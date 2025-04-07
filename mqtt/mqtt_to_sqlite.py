import paho.mqtt.client as mqtt
import json
import sqlite3
import ssl
from datetime import datetime

# MQTT nastavenia
mqtt_server = "192.168.2.51"
mqtt_port = 8883 
mqtt_topic = "senzory/datas"
mqtt_user = "sensor_5X96" 
mqtt_password = "jablko5X96"  
ca_cert_path = "/etc/mosquitto/certs/ca.crt"  

# SQLite databáza
db_path = "/var/lib/grafana/senzory.db"

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Pripojené k MQTT brokeru cez TLS")
        client.subscribe(mqtt_topic)
    elif rc == 5:
        print("Chyba: Nesprávne meno alebo heslo (RC=5)")
    else:
        print(f"Chyba pri pripojení: Kód {rc}")

def on_message(client, userdata, msg):
    try:
        # Dekódovanie prijatej správy
        payload = json.loads(msg.payload.decode())
        teplota = float(payload.get("teplota", 0.0))
        vlhkost = float(payload.get("vlhkost", 0.0))
        intenzita = float(payload.get("intenzita", 0.0))

        # Získanie aktuálneho dátumu a času
        datum_merania = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        # Debug výstup pred zápisom do databázy
        print(f"Prijaté dáta -> Teplota: {teplota}°C, Vlhkosť: {vlhkost}%, Intenzita: {intenzita} lx, Dátum: {datum_merania}")

        # Pripojenie k databáze a zápis údajov
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        sql_query = "INSERT INTO merania (teplota, vlhkost, intenzita_osvetlenia, datum_merania) VALUES (?, ?, ?, ?)"
        values = (teplota, vlhkost, intenzita, datum_merania)
        
        # Debug výstup SQL príkazu
        print(f"SQL dotaz: {sql_query}")
        print(f"Hodnoty: {values}")

        cursor.execute(sql_query, values)
        conn.commit()
        conn.close()

        print("Dáta boli úspešne uložené do databázy")

    except Exception as e:
        print(f"Chyba pri spracovaní dát: {e}")

# Inicializácia MQTT klienta s TLS
client = mqtt.Client()
client.username_pw_set(mqtt_user, mqtt_password)  # Pridanie mena a hesla
client.on_connect = on_connect
client.on_message = on_message

# Povolenie TLS a použitie CA certifikátu
client.tls_set(ca_certs=ca_cert_path, tls_version=ssl.PROTOCOL_TLSv1_2)
client.tls_insecure_set(False) 

# Pripojenie k MQTT brokeru cez TLS
try:
    client.connect(mqtt_server, mqtt_port, 60)
    print("Pripojenie k brokeru úspešné!")
except Exception as e:
    print(f"Chyba pri pripojení k MQTT brokeru: {e}")
    exit(1)

# Spustenie hlavnej slučky
client.loop_forever()