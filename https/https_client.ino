#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_VEML7700.h>

// Wi-Fi nastavenia
const char* ssid = "ASUS";
const char* password = "mustang123";
const uint8_t bssid[] = { 0x70, 0x4D, 0x7B, 0x51, 0xBA, 0xA0 };

// HTTPS URL
const char* serverUrl = "https://192.168.2.51/";

// CA certifikát
const char* ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIICODCCAd2gAwIBAgIUFzcoG3u+7q3ygImSZZ2DbtATILwwCgYIKoZIzj0EAwIw
cTELMAkGA1UEBhMCU0sxEzARBgNVBAgMCk5pdHJpYW5za3kxDjAMBgNVBAcMBU5p
dHJhMSEwHwYDVQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQxCzAJBgNVBAsM
AkRQMQ0wCwYDVQQDDARNeUNBMB4XDTI1MDMyOTA4MzQwNVoXDTI2MDMyOTA4MzQw
NVowcTELMAkGA1UEBhMCU0sxEzARBgNVBAgMCk5pdHJpYW5za3kxDjAMBgNVBAcM
BU5pdHJhMSEwHwYDVQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQxCzAJBgNV
BAsMAkRQMQ0wCwYDVQQDDARNeUNBMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE
ua/oRBVOBcLMeG7ROzC2DZOGf5rhXM5GWoDIGRI2Tri0vJBBETwcr4at+svAhRkZ
xclP/1n6e2lcHNcHUNE3aqNTMFEwHQYDVR0OBBYEFDpdl6MZB0n0O6iDqjgWnY4c
mkSbMB8GA1UdIwQYMBaAFDpdl6MZB0n0O6iDqjgWnY4cmkSbMA8GA1UdEwEB/wQF
MAMBAf8wCgYIKoZIzj0EAwIDSQAwRgIhAKvzv6b3T8RBYUt+V4HO6SOKTY81vszI
E05BBHqVrcENAiEAlsSNyxC9QeCvAuE7n0WQJNLF5MS6ViuLJv2BZjcinvM=
-----END CERTIFICATE-----
)EOF";

// Piny a senzory
#define DHTPIN 14
#define DHTTYPE DHT22
#define RED_LED 13
#define GREEN_LED 12
#define SDA_PIN 21
#define SCL_PIN 22

DHT dht(DHTPIN, DHTTYPE);
Adafruit_VEML7700 veml7700;
WiFiClientSecure client;

void setup() {
  Serial.begin(115200);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  Wire.begin(SDA_PIN, SCL_PIN);
  dht.begin();

  if (!veml7700.begin()) {
    Serial.println("VEML7700 sa nepodarilo inicializovať!");
    while (1)
      ;
  }

  Serial.println("Nastavujem statickú IP...");
  IPAddress staticIP(192, 168, 2, 149);
  IPAddress gateway(192, 168, 2, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(192, 168, 2, 1);
  IPAddress secondaryDNS(0, 0, 0, 0);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Statická IP konfigurácia zlyhala.");
  }

  Serial.println("Pripájam sa na WiFi...");
  WiFi.begin(ssid, password, 0, bssid);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi pripojené!");
  Serial.print("IP adresa ESP32: ");
  Serial.println(WiFi.localIP());

  client.setCACert(ca_cert);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi odpojené! Pokúšam sa znova pripojiť...");
    WiFi.begin(ssid, password, 0, bssid);
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\nNepodarilo sa pripojiť. Skúsim neskôr.");
      delay(5000);
      return;
    }
    Serial.println("\nWiFi znovu pripojené!");
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
  json += "\"teplota\":" + String(t, 2) + ",";
  json += "\"vlhkost\":" + String(h, 2) + ",";
  json += "\"lux\":" + String(lux, 2);
  json += "}";

  Serial.println("Odosielam: " + json);

  bool uspesne = false;
  const int maxPokusov = 3;

  for (int i = 0; i < maxPokusov && !uspesne; i++) {
    HTTPClient https;
    https.begin(client, serverUrl);
    https.addHeader("Content-Type", "application/json");
    https.addHeader("X-API-KEY", "KZt7gA93!YpLd^RQwX5bM0z@nVh8eC2s");
    int httpCode = https.POST(json);

    if (httpCode == 200) {
      Serial.println("Úspešne uložené do DB");
      uspesne = true;
    } else {
      Serial.print("Chyba pri odosielaní dát (pokus ");
      Serial.print(i + 1);
      Serial.print("): ");
      Serial.println(httpCode);
      delay(3000);
    }

    String response = https.getString();
    Serial.println("Odpoveď servera:");
    Serial.println(response);
    https.end();
  }

  digitalWrite(GREEN_LED, LOW);
  delay(10000);
}