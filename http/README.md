# Diplomová práca – HTTP klient/server pre IoT bezdrôtovú senzorickú sieť (WSN)

> ⚠️ Táto implementácia bola vytvorená ako súčasť diplomovej práce a slúžila na demonštráciu nezabezpečeného prenosu údajov v IoT prostredí. Umožnila identifikovať základné zraniteľnosti, otestovať možné útoky a poslúžila ako východiskový bod pre porovnanie s bezpečnými variantmi komunikácie.

## Architektúra systému

- **ESP32 D1 R32** (klient) zbiera dáta zo senzorov a odosiela ich cez HTTP POST na server.
- **Raspberry Pi 4** (server) prijíma tieto údaje cez jednoduchú Flask aplikáciu a zapisuje ich do SQLite databázy.

## Použité senzory

- `DHT22` – meranie teploty a vlhkosti
- `VEML7700` – meranie intenzity osvetlenia

## Obsah repozitára

- `http_client.ino` - kód pre ESP32. Zbiera údaje zo senzorov a odosiela ich cez HTTP POST ako JSON.
- `server.py` - Flask server, ktorý prijíma JSON, zapisuje ho do `senzory.db` a zobrazuje výstup v konzole.

## Požiadavky

- Hardvér: ESP32, DHT22, VEML7700
- Softvér: Arduino IDE, Python 3.x
- Knižnice:
  - Pre ESP32: `WiFi.h`, `HTTPClient.h`, `DHT.h`, `Wire.h`, `Adafruit_VEML7700`
  - Pre Raspberry Pi: `Flask`, `sqlite3`
 
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
