#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== WIFI =====
const char* ssid = "-";
const char* password = "-";

// ===== SERVER URL =====
// Your working URL from the monitor:
const char* url = "-";

// ===== LCD =====
// 0x27 worked ✅
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== SETTINGS =====
const uint32_t POLL_MS = 2000;   // refresh every 2 seconds

// keep last values so we only redraw when something changes
int lastToday = -9999;
int lastServed = -9999;

// ---------- helpers ----------
static int parseValueAfter(const String& s, const char key) {
  int idx = s.indexOf(String(key) + ":");
  if (idx < 0) return -1;
  idx += 2;

  while (idx < (int)s.length() && s[idx] == ' ') idx++;

  bool neg = false;
  if (idx < (int)s.length() && s[idx] == '-') { // ✅ support negative
    neg = true;
    idx++;
  }

  String num;
  while (idx < (int)s.length() && isDigit((unsigned char)s[idx])) {
    num += s[idx++];
  }
  if (num.length() == 0) return -1;

  int v = num.toInt();
  return neg ? -v : v;
}

static void printPadded(int col, int row, const String& text) {
  lcd.setCursor(col, row);
  String t = text;
  if (t.length() > 16) t = t.substring(0, 16);
  lcd.print(t);
  for (int i = t.length(); i < 16; i++) lcd.print(" ");
}

static void showServerError(int httpCode, const String& errText) {
  printPadded(0, 0, "Server error");
  // keep it short for 16 chars
  String line2 = (httpCode != 0) ? ("HTTP:" + String(httpCode)) : errText;
  if (line2.length() > 16) line2 = line2.substring(0, 16);
  printPadded(0, 1, line2);
}

void setup() {
  Serial.begin(115200);
  delay(300);

  // I2C pins for ESP32
  Wire.begin(21, 22);
  Wire.setClock(100000);

  // LCD init
  lcd.init();
  lcd.backlight();

  printPadded(0, 0, "Bistro Queue");
  printPadded(0, 1, "Connecting...");

  // WiFi connect
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start > 20000) break; // 20s timeout
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
    printPadded(0, 1, "WiFi OK");
  } else {
    Serial.println("\nWiFi connect failed!");
    printPadded(0, 0, "WiFi FAILED");
    printPadded(0, 1, "Check SSID");
  }

  delay(800);
}

void loop() {
  static uint32_t lastPoll = 0;
  if (millis() - lastPoll < POLL_MS) return;
  lastPoll = millis();

  if (WiFi.status() != WL_CONNECTED) {
    printPadded(0, 0, "WiFi LOST");
    printPadded(0, 1, "Reconnecting..");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    return;
  }

  HTTPClient http;
  http.begin(url);

  int code = http.GET();
  Serial.print("HTTP code: ");
  Serial.println(code);

  if (code != 200) {
    if (code <= 0) {
      String err = http.errorToString(code);
      Serial.print("HTTP error: ");
      Serial.println(err);
      showServerError(0, err);
    } else {
      showServerError(code, "");
    }
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  Serial.println("Payload:");
  Serial.println(payload);

  // We keep server format: W:<todayMade> A:<servedToday> C:<unused>
  int todayMade  = parseValueAfter(payload, 'W');
  int servedToday = parseValueAfter(payload, 'A');

  // If parsing failed, show raw (debug)
  if (todayMade < 0 && servedToday < 0) {
    printPadded(0, 0, "Bad payload");
    printPadded(0, 1, payload);
    return;
  }

  // Update LCD only if something changed (prevents flicker)
  if (todayMade != lastToday || servedToday != lastServed) {
    lastToday = todayMade;
    lastServed = servedToday;

    // ✅ NEW TEXT (your new idea)
    printPadded(0, 0, "Today: " + String(todayMade));
    printPadded(0, 1, "Served: " + String(servedToday));
  }
}



