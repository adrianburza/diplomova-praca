# Diplomová práca – HTTPS klient/server pre IoT bezdrôtovú senzorickú sieť (WSN)

Táto implementácia bola vytvorená ako súčasť diplomovej práce a predstavuje bezpečný model komunikácie v rámci WSN s využitím **HTTPS protokolu**. Komunikácia medzi zariadeniami je **zabezpečená pomocou TLS šifrovania a API kľúča**.

## Architektúra systému

- **ESP32 D1 R32** (klient) zbiera údaje zo senzorov a odosiela ich vo formáte JSON cez HTTPS POST na server. Komunikácia je zabezpečená TLS certifikátom a autentifikovaná pomocou API kľúča v hlavičke `X-API-KEY`.
- **Raspberry Pi 4** (server) hostuje Flask aplikáciu, ktorá:
  - overuje API kľúč,
  - kontroluje počet pokusov o prístup (rate-limiting),
  - blokuje IP adresy po opakovaných neúspešných pokusoch o autentifikáciu,
  - prijíma dáta vo formáte JSON,
  - ukladá ich do SQLite databázy (`senzory.db`),
  - a zobrazuje odpovede v termináli.

## Použité senzory

- `DHT22` – meranie teploty a vlhkosti
- `VEML7700` – meranie intenzity osvetlenia

## Obsah repozitára

- `https_client.ino` – kód pre ESP32, ktorý bezpečne odosiela údaje zo senzorov na server.
- `server.py` – bezpečne prijíma, overuje a ukladá údaje na strane Raspberry Pi.

## Požiadavky

- Hardvér: ESP32 D1 R32, Raspberry Pi 4, DHT22, VEML7700
- Softvér: Arduino IDE, Python 3.x

## Knižnice
- **Pre ESP32:**
  - `WiFi.h` – pripojenie na Wi-Fi sieť
  - `WiFiClientSecure.h` – zabezpečená HTTPS komunikácia (TLS)
  - `HTTPClient.h` – HTTP(S) klient na odosielanie požiadaviek
  - `DHT.h` – čítanie údajov zo senzora DHT22
  - `Wire.h` – I2C komunikácia pre senzor osvetlenia
  - `Adafruit_VEML7700.h` – knižnica pre VEML7700 senzor intenzity osvetlenia

- **Pre Raspberry Pi:**
  - `Flask` – jednoduchý webový server v Pythone
  - `flask_limiter` – obmedzovanie počtu požiadaviek (rate-limiting)
  - `sqlite3` – databázový systém na ukladanie prijatých údajov


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

- Šifrovanie komunikácie cez TLS (HTTPS)
- Overenie servera cez CA certifikát
- Overovanie API kľúča v hlavičke `X-API-KEY`
- Rate-limiting: max. 10 žiadostí za minútu na IP
- Blokovanie IP po 5 chybných pokusoch o autentifikáciu na 5 minút
