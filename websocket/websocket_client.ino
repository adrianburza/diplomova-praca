#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_VEML7700.h>
#include <esp_system.h>
#include <esp_heap_caps.h>

using namespace websockets;

// Wi-Fi nastavenie
const char* ssid = "ASUS";
const char* password = "mustang123";
const uint8_t bssid[] = { 0x70, 0x4D, 0x7B, 0x51, 0xBA, 0xA0 };

// JWT Token
const char* jwt_token = "KZt7gA93!YpLd^RQwX5bM0z@nVh8eC2";

// WebSocket Server (wss)
const char* ws_server = "wss://192.168.2.51/";

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

// Nastavenie pinov pre senzory
#define DHTPIN 14
#define DHTTYPE DHT22
#define RED_LED 13
#define GREEN_LED 12
#define SDA_PIN 21
#define SCL_PIN 22

DHT dht(DHTPIN, DHTTYPE);
Adafruit_VEML7700 veml7700;
WebsocketsClient client;
bool authenticated = false;

void setup() {
  Serial.begin(115200);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  Wire.begin(SDA_PIN, SCL_PIN);
  dht.begin();

  if (!veml7700.begin()) {
    Serial.println("VEML7700 sa nepodarilo inicializovať!");
    while (true)
      ;
  }

  Serial.println("Nastavujem statickú IP adresu...");
  IPAddress staticIP(192, 168, 2, 149);
  IPAddress gateway(192, 168, 2, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(192, 168, 2, 1);
  IPAddress secondaryDNS(0, 0, 0, 0);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Nastavenie statickej IP zlyhalo!");
  } else {
    Serial.println("Statická IP adresa nastavená.");
  }

  Serial.print("Pripájam sa na WiFi...");
  WiFi.begin(ssid, password, 0, bssid);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi pripojené: ");
  Serial.println(WiFi.localIP());

  client.setCACert(ca_cert);

  client.onMessage([](WebsocketsMessage message) {
    Serial.print("[WS] Server: ");
    Serial.println(message.data());
    if (message.data().startsWith("Autentifikácia OK")) {
      authenticated = true;
      Serial.println("[WS] Overenie OK!");
    }
  });

  client.onEvent([](WebsocketsEvent event, String data) {
    if (event == WebsocketsEvent::ConnectionOpened) {
      Serial.println("[WS] Pripojený – posielam JWT...");
      client.send(jwt_token);
    } else if (event == WebsocketsEvent::ConnectionClosed) {
      Serial.println("[WS] Odpojenie");
      authenticated = false;
    } else if (event == WebsocketsEvent::GotPing) {
      Serial.println("[WS] Ping prijatý");
    }
  });

  if (!client.connect(ws_server)) {
    Serial.println("Nepodarilo sa pripojiť k WebSocket serveru!");
  }
}

void loop() {
  // Automatické obnovenie WiFi spojenia
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
      Serial.println("\nZlyhalo opätovné pripojenie. Skúsim neskôr.");
      delay(5000);
      return;
    }
  }

  // Automatické obnovenie WebSocket spojenia
  if (!client.available()) {
    Serial.println("[WS] WebSocket odpojený – pokúšam sa znova pripojiť...");
    if (client.connect(ws_server)) {
      Serial.println("[WS] WebSocket znovu pripojený.");
      client.send(jwt_token);
    } else {
      Serial.println("[WS] Neúspešné pripojenie k serveru.");
      delay(5000);
      return;
    }
  }

  client.poll();

  if (!authenticated) {
    delay(1000);
    return;
  }

  digitalWrite(RED_LED, HIGH);
  delay(200);

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  float lux = veml7700.readLux();

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  delay(200);

  if (isnan(t) || isnan(h) || isnan(lux)) {
    Serial.println("Chyba pri čítaní dát zo senzorov!");
    return;
  }

  String json = "{";
  json += "\"teplota\":" + String(t, 2) + ",";
  json += "\"vlhkost\":" + String(h, 2) + ",";
  json += "\"lux\":" + String(lux, 2);
  json += "}";

  Serial.println("Odosielam: " + json);
  client.send(json);
  digitalWrite(GREEN_LED, LOW);

  delay(10000);
}