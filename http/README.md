# Diplomová práca – HTTP klient/server pre IoT senzorickú sieť

Tento projekt demonštruje základné fungovanie jednoduchej senzorickej siete využívajúcej **HTTP protokol bez zabezpečenia**. 
Slúži ako referenčný príklad, ktorý poukazuje na bezpečnostné riziká pri prenose údajov bez šifrovania a autentifikácie v IoT systémoch.

## Architektúra systému

- **ESP32 D1 R32** (klient) zbiera dáta zo senzorov a odosiela ich cez HTTP POST na server.
- **Raspberry Pi 4** (server) prijíma tieto údaje cez jednoduchú Flask aplikáciu a zapisuje ich do SQLite databázy.

## Použité senzory

- `DHT22` – meranie teploty a vlhkosti
- `VEML7700` – meranie intenzity osvetlenia

## Cieľ riešenia

- Demonštrovať prenos údajov bez šifrovania a autentifikácie
- Poskytnúť východiskový bod pre porovnanie s bezpečnými variantmi komunikácie

## Obsah repozitára

- `http_client.ino` - Kód pre ESP32. Zbiera údaje zo senzorov a odosiela ich cez HTTP POST ako JSON.
- `server.py` - Flask server, ktorý prijíma JSON, zapisuje ho do `senzory.db` a zobrazuje výstup v konzole.

## Požiadavky

- Hardvér: ESP32, DHT22, VEML7700
- Softvér: Arduino IDE, Python 3.x
- Knižnice:
  - Pre ESP32: `WiFi.h`, `HTTPClient.h`, `DHT.h`, `Adafruit_VEML7700`
  - Pre Raspberry Pi: `Flask`, `sqlite3`

## Vytvorenie databázy

V rámci diplomovej práce bola databázová tabuľka vytvorená manuálne pomocou grafického nástroja DB Browser for SQLite. 
Alternatívne je možné tabuľku jednoducho vytvoriť pomocou nasledovného SQL príkazu:

```sql
CREATE TABLE merania (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    teplota REAL,
    vlhkost REAL,
    intenzita_osvetlenia REAL,
    datum_merania TEXT
);
