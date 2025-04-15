# Diplomová práca – MQTT klient/server pre IoT bezdrôtovú senzorickú sieť (WSN)

Táto implementácia bola vytvorená ako súčasť diplomovej práce a demonštruje bezpečnú komunikáciu v IoT senzorickej sieti prostredníctvom protokolu **MQTT cez TLS**. Komunikácia je šifrovaná a autentifikovaná pomocou mena, hesla a CA certifikátu.

## Architektúra systému

- **ESP32 D1 R32** (klient) zbiera údeje zo senzorov a publikuje ich na zabezpečený MQTT broker cez port `8883` (TLS).
- **Raspberry Pi 4** (server) hostuje MQTT broker Mosquitto a skript, ktorý odoberá MQTT správy a ukladá údeje do SQLite databázy.

## Použité senzory

- `DHT22` – meranie teploty a vlhkosti
- `VEML7700` – meranie intenzity osvetlenia

## Cieľ riešenia

- Ukázať využitie MQTT protokolu pre šifrovaný a autentifikovaný prenos senzorických údajov

## Obsah repozitára

- `mqtt_client.ino` – kód pre ESP32, ktorý odosiela údaje zo senzorov na MQTT broker cez TLS
- `mqtt_to_sqlite.py` – Python skript, ktorý prijíma MQTT správy, spracúva JSON a zapisuje dáta do databázy `senzory.db`
- `mosquitto.conf` – konfigurácia MQTT brokera Mosquitto pre zabezpečené TLS spojenie a autentifikáciu

## Požiadavky

- Hardvér: ESP32 D1 R32, Raspberry Pi 4, DHT22, VEML7700
- Softvér: Arduino IDE, Python 3.x, Mosquitto MQTT broker

## Knižnice

- **Pre ESP32:**
  - `WiFi.h` – pripojenie na Wi-Fi sieť
  - `WiFiClientSecure.h` – šifrovaná komunikácia cez TLS
  - `PubSubClient.h` – MQTT klient pre ESP32
  - `DHT.h` – čítanie údajov zo senzora DHT22
  - `Wire.h` – I2C komunikácia pre senzor osvetlenia
  - `Adafruit_VEML7700.h` – knižnica pre senzor VEML7700

- **Pre Raspberry Pi:**
  - `paho-mqtt` – MQTT klient na príjem správ
  - `sqlite3` – práca s databázou SQLite
  - `ssl` – zabezpečená komunikácia
  - `json`, `datetime` – spracovanie formátu a času

## Konfigurácia MQTT brokera (Mosquitto)

Súbor `mosquitto.conf` zabezpečuje nasledovné:

- Autentifikáciu pomocou mena/hesla (`password_file`)
- Pravidlá prístupu pomocou ACL (`acl_file`)
- Šifrovanie pomocou TLS certifikátov (CA, server cert, server key)

## Spustenie servera (Python)

1. Uistite sa, že Mosquitto broker beží a akceptuje TLS pripojenie na porte `8883`
2. Spustite skript:

```bash
python3 mqtt_to_sqlite.py
```

## Bezpečnostné prvky

- Šifrovanie pomocou TLS (MQTT cez port 8883)
- Overenie klienta pomocou mena a hesla
- Overenie servera pomocou **CA certifikátu** uloženého na strane ESP32 a Python klienta
- Validácia JSON údajov pred ich uložením

## Vytvorenie databázy

SQLite tabuľka sa vytvorí pomocou:

```sql
CREATE TABLE merania (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    teplota REAL,
    vlhkost REAL,
    intenzita_osvetlenia REAL,
    datum_merania TEXT
);
```

> 💡 **Odporúčanie:** Pre záznam údajov a ladenie použite `mosquitto_sub` pre testovanie alebo `DB Browser for SQLite` na kontrolu obsahu databázy.
