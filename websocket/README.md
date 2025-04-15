# Diplomová práca – WebSocket klient/server pre IoT bezdrôtovú senzorickú sieť (WSN)

Táto implementácia demonštruje fungovanie WSN s využitím protokolu **WebSocket, zabezpečeného pomocou TLS a autentifikácie prostredníctvom JWT tokenu**.

## Architektúra systému

- **ESP32 D1 R32** (klient) zhromažďuje údeje zo senzorov a odosiela ich cez WebSocket (wss) na server. Komunikácia je šifrovaná pomocou TLS a zabezpečená JWT tokenom.
- **Raspberry Pi 4** (server) prijíma prípojenia cez zabezpečený WebSocket (TLS), overuje platnosť tokenu, aplikuje rate-limiting a ochranu proti brute-force útokom. Po overení uloží prijaté dáta do SQLite databázy.

## Použité senzory

- `DHT22` – meranie teploty a vlhkosti
- `VEML7700` – meranie intenzity osvetlenia

## Obsah repozitára

- `websocket_client.ino` – kód pre ESP32, ktorý zbiera dáta zo senzorov a odosiela ich cez WebSocket (wss) ako JSON, vrátane TLS a JWT autentifikácie.
- `server.py` – asynchrónny WebSocket server, ktorý prijíma JSON údeje, overuje JWT, sleduje neúspešné pokusy a blokuje IP, vykonáva rate-limiting a uloží dáta do `senzory.db`.

## Požiadavky

- Hardvér: ESP32 D1 R32, Raspberry Pi 4, DHT22, VEML7700
- Softvér: Arduino IDE, Python 3.x

## Knižnice

- re ESP32:
  - `WiFi.h` – pripojenie na Wi-Fi sieť
  - `ArduinoWebsockets.h` – komunikácia cez WebSocket protokol
  - `DHT.h` – čítanie údajov zo senzora DHT22
  - `Wire.h` – I2C komunikácia pre senzor osvetlenia
  - `Adafruit_VEML7700.h` – knižnica pre senzor intenzity osvetlenia VEML7700

- Pre Raspberry Pi:
  - `websockets` – WebSocket server pre príjem a odosielanie dát
  - `ssl` – podpora TLS šifrovania komunikácie
  - `sqlite3` – práca s lokálnou SQLite databázou
  - `json` – spracovanie údajov vo formáte JSON
  - `datetime` – generovanie časových pečiatok pre merania
  - `asyncio` – asynchrónne spracovanie WebSocket spojení
  - `logging` – výpis stavových hlásení do konzoly
  - `time` – spracovanie časových operácií

> 💡 **Odporúčanie:** Na Raspberry Pi je vhodné spustiť server v samostatnom virtuálnom prostredí (napr. pomocou `venv`), aby sa predišlo konfliktom medzi knižnicami.

## Vytvorenie databázy

V rámci diplomovej práce bola databázová tabuľka vytvorená manuálne pomocou grafického nástroja DB Browser for SQLite. Alternatívne je možné tabuľku jednoducho vytvoriť pomocou nasledovného SQL príkazu:

```sql
CREATE TABLE merania (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    teplota REAL,
    vlhkost REAL,
    intenzita_osvetlenia REAL,
    datum_merania TEXT
);
```

## Spustenie servera

Na spustenie serverovej časti stačí zadať nasledujúci príkaz do terminálu zariadenia (Raspberry Pi) s nainštalovaným Pythonom:

```bash
python3 server.py
```

### TLS certifikáty

Pre správnu funkčnosť HTTPS servera `server.py`, je potrebné, aby boli TLS certifikáty uložené na definovaných systémových cestách. Skript očakáva:

- `/etc/https/certs/server.crt` (verejný certifikát)
- `/etc/https/private/server.key` (privátny kľúc)

Na strane klienta (ESP32) je zároveň do kódu vložený **CA certifikát**, ktorý slúži na overenie dôveryhodnosti servera pred nadviazaním zabezpečeného spojenia.

## Bezpečnostné prvky

- Šifrovanie pomocou TLS (wss)
- Overenie servera cez CA certifikát
- Overenie klienta cez JWT token
- Ochrana proti brute-force: po 5 neúspešných pokusoch je IP blokovaná na 5 minút
- Rate-limiting: max. 12 správ za minútu na jedno pripojenie
- Validácia prichádzajúcich JSON dát pred ich uložením
