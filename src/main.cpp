#include <Arduino.h>

constexpr uint8_t BUTTON_COUNT = 18;
constexpr uint32_t DEBOUNCE_TIME = 5; //ms

uint32_t lastButtonChange[BUTTON_COUNT];
bool buttonState[BUTTON_COUNT];

const uint8_t buttonPins[BUTTON_COUNT] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };

const uint8_t LED_R = 18;
const uint8_t LED_G = 19;
const uint8_t LED_B = 20;

const uint8_t ENC_A = 21;
const uint8_t ENC_B = 22;

enum Packet : uint8_t {
  BTN_DOWN = 0x01,
  BTN_UP   = 0x02,
  ENC_CW   = 0x03,
  ENC_CCW  = 0x04,
  SET_LED  = 0x20
};

bool lastA;
bool lastB;

void WritePacketValue(Packet packet, uint8_t value) {
  Serial.write(packet);
  Serial.write(value);
}

void ScanButtons() {
  uint32_t now = millis();
  for (int i = 0; i < BUTTON_COUNT; i++) {
    bool current = digitalRead(buttonPins[i]);

    if (current != buttonState[i]) {
      if (now-lastButtonChange[i] > DEBOUNCE_TIME) {
        buttonState[i] = current;  
        if (current == LOW) WritePacketValue(BTN_DOWN, i);
        else WritePacketValue(BTN_UP, i);
      }
    } else {
      lastButtonChange[i] = now;
    }
  }
}

void ScanEncoder() {
  bool a = digitalRead(ENC_A);
  bool b = digitalRead(ENC_B);
  if(a != lastA) {
    if(a == b) Serial.write(ENC_CW);
    else Serial.write(ENC_CCW);
  }
  lastA = a;
  lastB = b;
}

void ProcessSerial() {
  while (Serial.available()) {
    uint8_t cmd = Serial.read();
    switch(cmd) {
      case SET_LED: {
        while (Serial.available() < 3);
        uint8_t r = Serial.read();
        uint8_t g = Serial.read();
        uint8_t b = Serial.read();
        analogWrite(LED_R, r);
        analogWrite(LED_G, g);
        analogWrite(LED_B, b);
        break;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  for(int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    buttonState[i] = digitalRead(buttonPins[i]);
    lastButtonChange[i] = millis();
  }

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);

  lastA = digitalRead(ENC_A);
  lastB = digitalRead(ENC_B);
}

void loop() {
  ScanButtons();
  ScanEncoder();
  ProcessSerial();
}