from flask import Flask, request
import sqlite3
from datetime import datetime

app = Flask(__name__)

db_path = "/var/lib/grafana/senzory.db"

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
            VALUES (?, ?, ?, ?)
        '''
        values = (teplota, vlhkost, intenzita, datum_merania)

        print(f"SQL: {sql_query}")
        print(f"Dáta: {values}")

        cursor.execute(sql_query, values)
        conn.commit()
        conn.close()

        print("Dáta uložené do senzory.db")

    except Exception as e:
        print(f"Chyba pri ukladaní do databázy: {e}")

@app.route('/', methods=['POST', 'GET'])
def prijmi_data():
    if request.method == 'POST':
        json_data = request.get_json(silent=True)
        if json_data:
            print(f"Prijatý JSON: {json_data}")
            uloz_do_db(json_data)
            return "Uložené do databázy", 200
        else:
            print("Prázdny alebo neplatný JSON")
            return "Neplatný formát", 400

    # GET – kontrolné zobrazenie
    return "<h1>Server beží – čaká na dáta z ESP32</h1>", 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=80)