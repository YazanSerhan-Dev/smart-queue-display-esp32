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
const char* url = "http://192.168.33.2:8088/queue";

// ===== LCD =====
// You said 0x27 worked ✅
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== SETTINGS =====
const uint32_t POLL_MS = 2000;   // refresh every 2 seconds

// keep last values so we only redraw when something changes
int lastW = -1, lastA = -1, lastC = -1;

static int parseValueAfter(const String& s, const char key) {
  // Finds "W:" or "A:" or "C:" and reads the number after it
  int idx = s.indexOf(String(key) + ":");
  if (idx < 0) return -1;
  idx += 2; // skip "X:"
  // skip spaces if any
  while (idx < (int)s.length() && s[idx] == ' ') idx++;

  String num;
  while (idx < (int)s.length() && isDigit((unsigned char)s[idx])) {
    num += s[idx++];
  }
  if (num.length() == 0) return -1;
  return num.toInt();
}

static void printPadded(int col, int row, const String& text) {
  lcd.setCursor(col, row);
  String t = text;
  if (t.length() > 16) t = t.substring(0, 16);
  lcd.print(t);
  // clear rest of line
  for (int i = t.length(); i < 16; i++) lcd.print(" ");
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
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  printPadded(0, 1, "WiFi OK");
  delay(600);
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
    printPadded(0, 0, "Server error");
    printPadded(0, 1, "Retrying...");
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  Serial.println("Payload:");
  Serial.println(payload);

  int W = parseValueAfter(payload, 'W');
  int A = parseValueAfter(payload, 'A');
  int C = parseValueAfter(payload, 'C');

  // If parsing failed, show raw (debug)
  if (W < 0 && A < 0 && C < 0) {
    printPadded(0, 0, "Bad payload");
    printPadded(0, 1, payload);
    return;
  }

  // Update LCD only if something changed (prevents flicker)
  if (W != lastW || A != lastA || C != lastC) {
    lastW = W; lastA = A; lastC = C;

    printPadded(0, 0, "Waiting: " + String(W));
    printPadded(0, 1, "Arrived: " + String(A));
    // If you prefer Completed instead:
    // printPadded(0, 1, "Done: " + String(C));
  }
}


