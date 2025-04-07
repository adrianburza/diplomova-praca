from flask import Flask, request, jsonify
from flask_limiter import Limiter
from flask_limiter.util import get_remote_address
import sqlite3
from datetime import datetime
from time import time

app = Flask(__name__)

API_KEY = "KZt7gA93!YpLd^RQwX5bM0z@nVh8eC2s"

db_path = "/var/lib/grafana/senzory.db"

# Inicializácia limiteru (rate limit)
limiter = Limiter(get_remote_address, app=app)

# Ochrana proti bruteforce (dočasné blokovanie IP)
attempts = {}  # ip: [count, first_time]
blocked = {}  # ip: unblock_time
MAX_ATTEMPTS = 5
BLOCK_DURATION = 300  # sekundy (5 minút)

# Kontrola blokovaných IP pred každým requestom
@app.before_request
def check_ip_block():
    ip = request.remote_addr
    now = time()
    if ip in blocked and now < blocked[ip]:
        return jsonify({"error": "Prístup zablokovaný pre IP."}), 429

# Uloženie do DB
def uloz_do_db(data):
    try:
        teplota = float(data.get("teplota"))
        vlhkost = float(data.get("vlhkost"))
        intenzita = float(data.get("lux"))
        datum_merania = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()

        sql_query = '''
            INSERT INTO merania (teplota, vlhkost, intenzita_osvetlenia, datum_merania)
            VALUES (?, ?, ?, ?)'''
        values = (teplota, vlhkost, intenzita, datum_merania)

        cursor.execute(sql_query, values)
        conn.commit()
        conn.close()
        print("Dáta uložené do DB:", values)

    except Exception as e:
        print(f"Chyba pri ukladaní: {e}")

# Hlavný endpoint
@app.route('/', methods=['POST', 'GET'])
@limiter.limit("10 per minute")  # rate limit
def prijmi_data():
    ip = request.remote_addr
    now = time()

    if request.method == 'POST':
        # Autentifikácia API kľúčom
        key = request.headers.get("X-API-KEY")
        if key != API_KEY:
            attempts.setdefault(ip, [0, now])
            attempts[ip][0] += 1
            if attempts[ip][0] >= MAX_ATTEMPTS:
                blocked[ip] = now + BLOCK_DURATION
                print(f"IP {ip} blokovaná na 5 minút")
            return jsonify({"error": "Neplatný API kľúč"}), 403

        # Správna autentifikácia - reset pokusov
        if ip in attempts:
            del attempts[ip]

        # Prijmi a ulož JSON
        json_data = request.get_json(silent=True)
        if json_data:
            print(f"Prijatý JSON: {json_data}")
            uloz_do_db(json_data)
            return "Uložené", 200
        else:
            return "Neplatný JSON", 400

    return "<h1>HTTPS server aktívny – čaká na dáta z ESP32</h1>", 200

# HTTPS server
if __name__ == '__main__':
    app.run(
        host='0.0.0.0',
        port=443,
        ssl_context=('/etc/https/certs/server.crt', '/etc/https/private/server.key')
    )