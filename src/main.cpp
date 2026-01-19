#include <Arduino.h>

// COMMON ANODE: LOW=ON, HIGH=OFF
const int ON  = HIGH;
const int OFF = LOW;

// Put YOUR GPIO mapping here (must be UNIQUE for each segment)
int SEG_A = 22;   // you said 22=a
int SEG_B = 23;   // <-- you must find/fix this (currently missing)
int SEG_C = 18;   // you said 18=c (then 23 cannot be c)
int SEG_D = 17;   // you said 17=d
int SEG_E = 16;   // you said 16=e
int SEG_F = 21;   // you said 21=f
int SEG_G = 19;   // you said 19=g

int segPins[7];   // will be filled in setup

//          a b c d e f g   (1 means segment should be ON)
const uint8_t digits[10][7] = {
  {1,1,1,1,1,1,0}, // 0
  {0,1,1,0,0,0,0}, // 1
  {1,1,0,1,1,0,1}, // 2
  {1,1,1,1,0,0,1}, // 3
  {0,1,1,0,0,1,1}, // 4
  {1,0,1,1,0,1,1}, // 5
  {1,0,1,1,1,1,1}, // 6
  {1,1,1,0,0,0,0}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,1,0,1,1}  // 9
};

void showDigit(int n) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segPins[i], digits[n][i] ? ON : OFF);
  }
}

void setup() {
  // Fill in order a,b,c,d,e,f,g
  segPins[0] = SEG_A;
  segPins[1] = SEG_B;
  segPins[2] = SEG_C;
  segPins[3] = SEG_D;
  segPins[4] = SEG_E;
  segPins[5] = SEG_F;
  segPins[6] = SEG_G;

  // If any segment pin is still -1, stop here (avoid weird output)
  for (int i = 0; i < 7; i++) {
    if (segPins[i] < 0) {
      // Blink onboard LED? (optional) For now just hang.
      while (true) { delay(1000); }
    }
  }

  for (int i = 0; i < 7; i++) {
    pinMode(segPins[i], OUTPUT);
    digitalWrite(segPins[i], OFF);
  }
}

void loop() {
  for (int n = 0; n <= 9; n++) {
    showDigit(n);
    delay(1000);
  }
}

