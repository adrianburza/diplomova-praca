#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_VEML7700.h>

// Wi-Fi nastavenie
const char* ssid = "ASUS";
const char* password = "mustang123";
const uint8_t bssid[] = { 0x70, 0x4D, 0x7B, 0x51, 0xBA, 0xA0 };
const char* serverUrl = "http://192.168.2.51/";

// Statická IP konfigurácia
IPAddress staticIP(192, 168, 2, 149);
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 2, 1);
IPAddress secondaryDNS(0, 0, 0, 0);

// Nastavenie pinov pre senzory
#define DHTPIN 14
#define DHTTYPE DHT22
#define RED_LED 13
#define GREEN_LED 12
#define SDA_PIN 21
#define SCL_PIN 22

DHT dht(DHTPIN, DHTTYPE);
Adafruit_VEML7700 veml7700;

void setup() {
  Serial.begin(115200);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  Wire.begin(SDA_PIN, SCL_PIN);
  dht.begin();

  if (!veml7700.begin()) {
    Serial.println("Chyba: VEML7700 sa nepodarilo inicializovať!");
    while (true)
      ;
  }

  Serial.println("Nastavujem statickú IP adresu...");
  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Nastavenie statickej IP zlyhalo!");
  } else {
    Serial.println("Statická IP adresa nastavená.");
  }

  Serial.print("Pripájam sa na WiFi...");
  WiFi.begin(ssid, password, 0, bssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi pripojené: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Automatická obnova WiFi spojenia
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi odpojené – pokúšam sa znova pripojiť...");
    WiFi.begin(ssid, password, 0, bssid);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi znovu pripojené.");
    } else {
      Serial.println("\nZlyhalo pripojenie. Skúsim neskôr.");
      delay(5000);
      return;
    }
  }

  digitalWrite(RED_LED, HIGH);
  delay(500);

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float lux = veml7700.readLux();

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  delay(500);

  if (isnan(h) || isnan(t) || isnan(lux)) {
    Serial.println("Chyba pri čítaní senzorov!");
    return;
  }

  String json = "{";
  json += "\"teplota\":" + String(t, 1) + ",";
  json += "\"vlhkost\":" + String(h, 1) + ",";
  json += "\"lux\":" + String(lux, 1) + ",";
  json += "}";

  Serial.println("Odosielam dáta:");
  Serial.println(json);

  // Opakovanie HTTP požiadavky v prípade zlyhania
  const int maxPokusov = 3;
  bool uspesne = false;

  for (int i = 0; i < maxPokusov && !uspesne; i++) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    int responseCode = http.POST(json);

    if (responseCode == 200) {
      Serial.println("Dáta uložené do databázy.");
      uspesne = true;
    } else {
      Serial.print("Chyba (pokus ");
      Serial.print(i + 1);
      Serial.print("): ");
      Serial.println(responseCode);
      delay(3000);
    }

    String response = http.getString();
    Serial.println("Odpoveď servera:");
    Serial.println(response);

    http.end();
  }

  digitalWrite(GREEN_LED, LOW);
  delay(10000);
}
