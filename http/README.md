# Diplomová práca – HTTP klient/server pre IoT bezdrôtovú senzorickú sieť (WSN)

> ⚠️ Táto implementácia bola vytvorená v rámci diplomovej práce ako demonštračný príklad nezabezpečenej komunikácie v prostredí IoT. Slúžila na identifikáciu základných zraniteľností, testovanie možných útokov a poskytla východiskový rámec pre porovnanie s bezpečnými spôsobmi prenosu údajov.

## Architektúra systému

- **ESP32 D1 R32** (klient) zbiera dáta zo senzorov a odosiela ich cez HTTP POST na server.
- **Raspberry Pi 4** (server) prijíma tieto údaje cez jednoduchú Flask aplikáciu a zapisuje ich do SQLite databázy.

## Použité senzory

- `DHT22` – meranie teploty a vlhkosti
- `VEML7700` – meranie intenzity osvetlenia

## Obsah repozitára

- `http_client.ino` - kód pre ESP32, ktorý zbiera údaje zo senzorov a odosiela ich cez HTTP POST vo formáte JSON.
- `server.py` - Flask server pre Raspberry Pi, ktorý prijíma JSON dáta, ukladá ich do databázy `senzory.db` a zobrazuje výstup v konzole.

## Požiadavky

- Hardvér: ESP32 D1 R32, Raspberry Pi 4, DHT22, VEML7700
- Softvér: Arduino IDE, Python 3.x

## Knižnice

- **Pre ESP32:**
  - `WiFi.h` – pripojenie na Wi-Fi sieť
  - `HTTPClient.h` – HTTP komunikácia
  - `DHT.h` – senzor teploty a vlhkosti DHT22
  - `Wire.h` – I2C komunikácia
  - `Adafruit_VEML7700.h` – senzor intenzity osvetlenia VEML7700

- **Pre Raspberry Pi:**
  - `Flask` – webový framework pre spracovanie HTTP požiadaviek
  - `sqlite3` – databáza na ukladanie údajov
  - `datetime` – modul na prácu s dátumom a časom
 
> 💡 **Odporúčanie:** Na Raspberry Pi je vhodné spustiť server v samostatnom virtuálnom prostredí (napr. pomocou `venv`), aby sa predišlo konfliktom medzi knižnicami.

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
```

## Spustenie servera

Na spustenie serverovej časti stačí zadať nasledujúci príkaz do terminálu zariadenia (Raspberry Pi) s nainštalovaným Pythonom:

```bash
python3 server.py
