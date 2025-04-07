#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_VEML7700.h>

// Wi-Fi nastavenie
const char* ssid = "ASUS";
const char* password = "mustang123";
const uint8_t bssid[] = { 0x70, 0x4D, 0x7B, 0x51, 0xBA, 0xA0 };

// MQTT broker
const char* mqtt_server = "192.168.2.51";
const int mqtt_port = 8883;
const char* mqtt_user = "sensor_5X96";
const char* mqtt_password = "jablko5X96";

// Certifikát certifikačnej autority
const char* ca_cert =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIB8zCCAZmgAwIBAgIUM1/ejkESOh2spA77p1zqwyISAvUwCgYIKoZIzj0EAwIw\n"
  "TzELMAkGA1UEBhMCU0sxEzARBgNVBAgMCk5pdHJpYW5za3kxDjAMBgNVBAcMBU5p\n"
  "dHJhMQswCQYDVQQKDAJEUDEOMAwGA1UEAwwFTXkgQ0EwHhcNMjUwMzE2MTA0OTM2\n"
  "WhcNMjYwMzE2MTA0OTM2WjBPMQswCQYDVQQGEwJTSzETMBEGA1UECAwKTml0cmlh\n"
  "bnNreTEOMAwGA1UEBwwFTml0cmExCzAJBgNVBAoMAkRQMQ4wDAYDVQQDDAVNeSBD\n"
  "QTBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABOdK+P3fSrnC5MTbvCOiOfjlTBXb\n"
  "5SWpfoUbSRnUvHcD2ZMeNgdMFtPIKN5INCyWWyJcwU6LdHRJfPB1yb66MKmjUzBR\n"
  "MB0GA1UdDgQWBBSfO0usj33+Mw6y1R3WiagjxLMBUTAfBgNVHSMEGDAWgBSfO0us\n"
  "j33+Mw6y1R3WiagjxLMBUTAPBgNVHRMBAf8EBTADAQH/MAoGCCqGSM49BAMCA0gA\n"
  "MEUCIE0uXNNftwdl82vwrONqEKJdktAxgICn40VZsvnzQ+WeAiEAhsabifi7Xxrp\n"
  "Sh7XVbPrWozHP65Odv+VHA1Uk4by7QY=\n"
  "-----END CERTIFICATE-----\n";

// Inicializácia WiFi a MQTT
WiFiClientSecure espClient;
PubSubClient client(espClient);

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
  Serial.println("ESP32 D1 R32 - Start");

  // Nastavenie statickej IP adresy
  IPAddress staticIP(192, 168, 2, 149);
  IPAddress gateway(192, 168, 2, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(192, 168, 2, 1);
  IPAddress secondaryDNS(0, 0, 0, 0);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Chyba: Nepodarilo sa nastaviť statickú IP adresu");
  } else {
    Serial.println("Statická IP adresa úspešne nastavená");
  }

  // Pripojenie k Wi-Fi
  Serial.print("Pripájam sa k Wi-Fi...");
  WiFi.begin(ssid, password, 0, bssid);

  int maxTries = 20;
  while (WiFi.status() != WL_CONNECTED && maxTries > 0) {
    delay(1000);
    Serial.print(".");
    maxTries--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi pripojené!");
    Serial.print("IP adresa: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Nepodarilo sa pripojiť k Wi-Fi.");
    return;
  }

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  Wire.begin(SDA_PIN, SCL_PIN);
  dht.begin();

  if (!veml7700.begin()) {
    Serial.println("Chyba: VEML7700 sa nepodarilo inicializovať!");
    while (1)
      ;
  }
  Serial.println("VEML7700 senzor inicializovaný");

  espClient.setCACert(ca_cert);
  client.setServer(mqtt_server, mqtt_port);
  connectMQTT();
}

void loop() {
  // Reštart Wi-Fi ak vypadne
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi odpojené, pripájam znova...");
    connectWiFi();
  }

  // Overenie pripojenia k MQTT brokeru, prípadne opätovné pripojenie
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  // DHT22 Meranie
  digitalWrite(RED_LED, HIGH);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  digitalWrite(RED_LED, LOW);

  // VEML7700 Meranie
  digitalWrite(GREEN_LED, HIGH);
  float lux = veml7700.readLux();
  digitalWrite(GREEN_LED, LOW);

  if (!isnan(h) && !isnan(t) && !isnan(lux)) {
    char msg[200];
    snprintf(msg, sizeof(msg), "{\"teplota\":%.2f,\"vlhkost\":%.2f,\"intenzita\":%.2f}", t, h, lux);
    Serial.print("Odosielanie údajov: ");
    Serial.println(msg);
    client.publish("senzory/datas", msg);
  }

  delay(10000);  // Cyklus merania každých 10 sekúnd
}

void connectWiFi() {
  // Nastavenie statickej IP po odpojení
  IPAddress staticIP(192, 168, 2, 149);
  IPAddress gateway(192, 168, 2, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(192, 168, 2, 1);
  IPAddress secondaryDNS(0, 0, 0, 0);

  WiFi.disconnect(true);
  delay(1000);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Nepodarilo sa znovu nastaviť statickú IP");
  }

  WiFi.begin(ssid, password, 0, bssid);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi pripojené!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nZlyhalo pripojenie k Wi-Fi.");
  }
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Pripájam sa k MQTT cez TLS...");

    client.setKeepAlive(60); 

    if (client.connect("ESP32_Client", mqtt_user, mqtt_password)) {
      Serial.println();
      Serial.println("Pripojené k MQTT brokeru cez TLS!");
      client.subscribe("senzory/#", 1);
    } else {
      Serial.print("Chyba, kód: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}