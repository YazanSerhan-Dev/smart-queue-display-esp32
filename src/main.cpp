#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// ===== WIFI =====
const char* ssid = "-";
const char* password = "-";

// ===== MQTT =====
const char* mqttHost  = "-";     
const int   mqttPort  = 1883;
const char* mqttTopic = "bistro/reservations/today";

// ===== LCD =====
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== MQTT client =====
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

int lastToday = -1;
int lastServed = -1;

static void printPadded(uint8_t col, uint8_t row, const String& text) {
  lcd.setCursor(col, row);
  String t = text;
  if (t.length() > 16) t = t.substring(0, 16);
  lcd.print(t);
  for (int i = t.length(); i < 16; i++) lcd.print(" ");
}

static void show2(const String& l1, const String& l2) {
  printPadded(0, 0, l1);
  printPadded(0, 1, l2);
}

static void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  show2("WiFi connecting", "...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }

  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.localIP());
  show2("WiFi OK", WiFi.localIP().toString());
  delay(800);
}

static void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  JsonDocument doc;  

  DeserializationError err = deserializeJson(doc, payload, length);
  if (err) {
    show2("Bad JSON", "from server");
    return;
  }

  int today  = doc["today"]  | -1;
  int served = doc["served"] | -1;

  if (today != lastToday || served != lastServed) {
    lastToday = today;
    lastServed = served;
    show2("Today: " + String(today), "Served: " + String(served));
  }
}

static void connectMqtt() {
  mqtt.setServer(mqttHost, mqttPort);
  mqtt.setCallback(onMqttMessage);

  while (!mqtt.connected()) {
    String clientId = "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);

    Serial.print("[MQTT] connecting to ");
    Serial.print(mqttHost);
    Serial.print(":");
    Serial.println(mqttPort);

    show2("MQTT connect", mqttHost);

    if (mqtt.connect(clientId.c_str())) {
      Serial.println("[MQTT] connected!");
      mqtt.subscribe(mqttTopic, 1);
      Serial.print("[MQTT] subscribed to ");
      Serial.println(mqttTopic);

      show2("MQTT subscribed", "Waiting data");
      delay(700);
      } else {
        int rc = mqtt.state();
        Serial.print("[MQTT] failed rc=");
        Serial.println(rc);

        show2("MQTT failed", "rc=" + String(rc));
        delay(1500);
      }
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(21, 22);
  Wire.setClock(100000);

  lcd.init();
  lcd.backlight();

  show2("Bistro IoT LCD", "Booting...");
  delay(600);

  connectWiFi();
  connectMqtt();

  show2("Ready", "Wait update...");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] lost, reconnecting...");
    show2("WiFi LOST", "Reconnecting");
    connectWiFi();
    connectMqtt();
  }

  if (!mqtt.connected()) {
    Serial.println("[MQTT] lost, reconnecting...");
    show2("MQTT LOST", "Reconnecting");
    connectMqtt();
  }

  mqtt.loop();
}
