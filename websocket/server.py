import asyncio
import websockets
import sqlite3
import json
import ssl
from datetime import datetime, timedelta
import logging
from time import time

logging.basicConfig(level=logging.INFO)

# Nastavenia
SECRET_KEY = "KZt7gA93!YpLd^RQwX5bM0z@nVh8eC2s"
DB_PATH = "/var/lib/grafana/senzory.db"
MAX_MESSAGES_PER_MIN = 12
MAX_ATTEMPTS = 5
BLOCK_TIME = 300  # sekundy = 5 minút

# Brute-force ochrana
auth_attempts = {}   # { ip: [count, first_try_time] }
blocked_ips = {}     # { ip: unblock_time }

# Overenie JWT
def over_jwt(token):
    return token == SECRET_KEY

# DB Ukladanie
def uloz_do_db(data):
    try:
        teplota = float(data.get("teplota"))
        vlhkost = float(data.get("vlhkost"))
        lux = float(data.get("lux"))
        datum = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        conn = sqlite3.connect(DB_PATH)
        cursor = conn.cursor()
        cursor.execute('''
            INSERT INTO merania (teplota, vlhkost, intenzita_osvetlenia, datum_merania)
            VALUES (?, ?, ?, ?)''', (teplota, vlhkost, lux, datum))
        conn.commit()
        conn.close()

        logging.info(f"Uložené: T={teplota}, V={vlhkost}, LUX={lux}, D={datum}")
    except Exception as e:
        logging.error(f"Chyba pri ukladaní do DB: {e}")

# Rate Limiter
class RateLimiter:
    def __init__(self, max_per_minute):
        self.timestamps = []
        self.limit = max_per_minute

    def allowed(self):
        now = datetime.now()
        self.timestamps = [t for t in self.timestamps if (now - t).total_seconds() < 60]
        if len(self.timestamps) >= self.limit:
            return False
        self.timestamps.append(now)
        return True

# WebSocket Handler
async def handler(websocket):
    client_ip = websocket.remote_address[0]
    logging.info(f"Pripojenie z {client_ip}")
    limiter = RateLimiter(MAX_MESSAGES_PER_MIN)
    now = time()

    # Skontroluj, či IP nie je blokovaná
    if client_ip in blocked_ips and now < blocked_ips[client_ip]:
        logging.warning(f"IP {client_ip} je blokovaná")
        await websocket.send("Vaša IP je dočasne blokovaná.")
        await websocket.close()
        return

    try:
        raw_token = await websocket.recv()
        logging.info(f"Prijatý token: {raw_token}")

        if not over_jwt(raw_token):
            logging.warning(f"Neplatný token od {client_ip}")

            # Eviduj pokus
            attempts = auth_attempts.get(client_ip, [0, now])
            attempts[0] += 1
            auth_attempts[client_ip] = attempts

            if attempts[0] >= MAX_ATTEMPTS:
                blocked_ips[client_ip] = now + BLOCK_TIME
                logging.warning(f"IP {client_ip} zablokovaná na 5 minút")
                auth_attempts.pop(client_ip, None)

            await websocket.send("Neplatný alebo chýbajúci token.")
            await websocket.close()
            return

        # Pri úspechu resetuj pokusy
        auth_attempts.pop(client_ip, None)
        await websocket.send("Autentifikácia OK. Posielaj JSON dáta.")

        while True:
            message = await websocket.recv()

            if not limiter.allowed():
                logging.warning(f"Príliš veľa správ od {client_ip}, odpojenie.")
                await websocket.send("Limit správ prekročený – spojenie ukončené.")
                await websocket.close()
                return

            try:
                json_data = json.loads(message)
                if all(k in json_data for k in ("teplota", "vlhkost", "lux")):
                    uloz_do_db(json_data)
                    await websocket.send("Dáta prijaté a uložené.")
                else:
                    await websocket.send("Chýbajúce hodnoty v JSON.")
            except json.JSONDecodeError:
                await websocket.send("Neplatný JSON formát.")

    except websockets.exceptions.ConnectionClosed:
        logging.info(f"Spojenie z {client_ip} ukončené.")

# Spustenie servera
async def main():
    ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    ssl_context.load_cert_chain('/etc/https/certs/server.crt', '/etc/https/private/server.key')

    async with websockets.serve(handler, "0.0.0.0", 443, ssl=ssl_context):
        logging.info("WebSocket TLS server beží na porte 443")
        await asyncio.Future()

if __name__ == "__main__":
    asyncio.run(main())