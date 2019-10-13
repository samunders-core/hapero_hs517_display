#ifndef UNIT_TEST

#include "Arduino.h"
#include "mcp_can.h"
#include "print_utils.h"
#include "CanBusMessage.h"
#include "lcd_decoder.h"

char keys, shift, menu, transmitted = 0;
char previousMessage[9] = {0x00}, currentState[9] = {0x00};

#ifdef ARDUINO_ARCH_AVR
#include <Adafruit_GFX.h>
#include "Fonts/FreeMono12pt7b.h"
#include <UTFTGLUE.h>
#include "SharedPinsTouchScreen.h"

SharedPinsTouchScreen touch(240, 320, A1, A2);
int16_t touchSampleCount = 0;
Adafruit_GFX_Button buttons[4], messageButton, trayButton;
UTFTGLUE display(0x9341, A2, A1, A3, A4, A0); // ifdef-ed since ESP8266 has only 1 analog input

#define BLACK 0x0000
#define RED 0xF800
#define GREEN 0x07E0
#define YELLOW 0xFFE0
#define GREY 0x8410
#define WHITE 0xFFFF
uint16_t messageColors[5] = {WHITE, GREEN, YELLOW, GREY, RED};

void drawPortraitButtons(uint16_t w, uint16_t h) {
  buttons[0].initButton(&display, w/4 - 1, w/4 - 1, w/2 - 2, w/2 - 2, WHITE, BLACK, YELLOW, "", 2);
  buttons[2].initButton(&display, 3*w/4 + 1, w/4 - 1, w/2 - 2, w/2 - 2, WHITE, BLACK, YELLOW, "", 2);
  buttons[1].initButton(&display, w/4 - 1, h - w/4, w/2 - 2, w/2 - 2, WHITE, BLACK, YELLOW, "", 2);
  buttons[3].initButton(&display, 3*w/4 + 1, h - 3*w/8, w/2 - 2, w/4 - 2, WHITE, BLACK, YELLOW, "", 2);
  for (uint8_t i = 0; i < sizeof(buttons)/sizeof(Adafruit_GFX_Button); i++) {
      buttons[i].drawButton();
  }
  messageButton.initButton(&display, w/2, h/2, w, h - w/2 - 2 - w/2 - 4, WHITE, BLACK, YELLOW, "", 2);
  messageButton.drawButton();
  trayButton.initButton(&display, 3*w/4 + 1, h - w/8, w/2 - 2, w/4 - 2, WHITE, BLACK, YELLOW, "tray", 1);
  trayButton.drawButton();
  display.fillTriangle(w/6, w/4 - 1, 2*w/6 - 1, w/6, 2*w/6 - 1, 2*w/6 - 1, GREEN);
  display.fillTriangle(4*w/6 - 1, 2*w/6 - 1, 5*w/6, 2*w/6 - 1, 3*w/4, w/6, GREEN);
  display.fillTriangle(w/6 - 1, h - 2*w/6 + 1, 2*w/6, h - 2*w/6 + 1, w/4, h - w/6, GREEN);
  display.fillTriangle(5*w/6, h - 3*w/8, 4*w/6 + 1, h - 3*w/8 + w/12, 4*w/6 + 1, h - 3*w/8 - w/12, GREEN);
}

void drawMessage(char * msg, uint16_t textColor = BLACK) {
  display.setColor(BLACK);
  display.print(previousMessage, 20, 160 - 14);
  display.setColor(BLACK == textColor ? messageColors[menu] : textColor);
  display.print(msg, 20, 160 - 14);
  strcpy(previousMessage, msg);
}

static const uint8_t D0   = 0;
static const uint8_t D1   = 1;
static const uint8_t D2   = 2;
static const uint8_t D3   = 3;
static const uint8_t D4   = 4;
static const uint8_t D5   = 5;
static const uint8_t D6   = 6;
static const uint8_t D7   = 7;
static const uint8_t D8   = 8;
static const uint8_t D9   = 9;
static const uint8_t D10  = 10;
static const uint8_t D11  = 11;
static const uint8_t D12  = 12;
static const uint8_t D13  = 13;
static const uint8_t D14  = 14;
static const uint8_t D15  = 15;
#else
void drawMessage(char * msg) {}
#endif

byte can_speed = CanBusMessage::UNINITIALIZED;
MCP_CAN CAN0(D10);     // Set CS to pin 10
CanBusMessage rx_msgs[2], tx_msgs[2];
unsigned long pressedAtMillis = 0, intervalMillis = 300;
unsigned long wasted = 0;

void setup() {
  Serial.begin(115200);
  for (byte rv = CAN0.begin(CAN_100KBPS); CAN_OK != rv; ) {
    printfTo(Serial, "Error %d Initializing MCP2515...", rv);
    while(1);
  }
  pinMode(D2, INPUT);
  Serial.println("MCP2515 Initialized Successfully!");
  can_speed = CAN_100KBPS;
#ifdef ARDUINO_ARCH_AVR
  pinMode(A0, INPUT_PULLUP);  // required for correct touch operation
  digitalWrite(A0, HIGH);
  pinMode(A0, OUTPUT);
  display.InitLCD(PORTRAIT);
  display.clrScr();
  display.setFont(&FreeMono12pt7b);
  drawPortraitButtons(240, 320);
  display.setTextSize(2);
  drawMessage("5 t b y ");
#endif
}

byte elapsed(unsigned long& since, unsigned long deltaMillis, unsigned long now, unsigned long newSince = ~0) {
  byte result = now >= since ? deltaMillis <= (now - since) : 0;
  return result && ((since = newSince) || 1);
}

char keyPressed(char key) {
  if ('a' == key || 'A' == key) { // left
    keys |= 0x01;
  } else if ('s' == key || 'S' == key) { // down
    keys |= 0x02;
  } else if ('w' == key || 'W' == key) { //up
    keys |= 0x04;
  } else if ('d' == key || 'D' == key) { // right
    keys |= 0x08;
  } else if ('0' == key || '1' == key || '2' == key || '4' == key) {
    menu = key - '0';
  } else {
    keys = 0x00;
  }
  shift = key >= 'A' && key <= 'Z'; // simulate long holding of button by SHIFT+press
  return key;
}

void messageReceived(unsigned long now) {
  if (transmitted || memcmp(&rx_msgs[0], &rx_msgs[1], sizeof(CanBusMessage))) {
    if (0xDE == rx_msgs[0].bytes[0]) {
      print_8_DE_bytes(Serial, rx_msgs[0].bytes);
      char buffer[9] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00};
      print_4_DE_bytes(buffer, rx_msgs[0].bytes);
      drawMessage(buffer);
    } else {
      rx_msgs[0].to(Serial);
    }
    // Serial.print(" received at ");
    // Serial.print(now, DEC);
    Serial.println();
    memcpy(&rx_msgs[1], &rx_msgs[0], sizeof(CanBusMessage));
    transmitted = 0;
  }
}

void messageSent(unsigned long now) {
  if (memcmp(&tx_msgs[0], &tx_msgs[1], sizeof(CanBusMessage))) {  // notify only on change
    memcpy(&tx_msgs[1], &tx_msgs[0], sizeof(CanBusMessage));
    tx_msgs[0].to(Serial);
    Serial.print(" sent at ");
    Serial.print(now, DEC);
    Serial.print(", ");
    Serial.flush();
    transmitted = 1;
  }
  tx_msgs[0].reset();
}

// GOAL: at least 3x per second refresh LCD
void loop() {
  pinMode(D2, INPUT);
  uint8_t canReceiveReady = !digitalRead(D2);
  pinMode(D2, OUTPUT);
  unsigned long now = millis();
  if (canReceiveReady && rx_msgs[0].recv(CAN0).size() != CanBusMessage::UNINITIALIZED) {
    messageReceived(now);
  } else if (tx_msgs[0].send(CAN0, tx_msgs[0].size()).size() != CanBusMessage::UNINITIALIZED) {  // was and still is set
    messageSent(now);
  } else if (elapsed(pressedAtMillis, intervalMillis, now, now)) {
#ifdef ARDUINO_ARCH_AVR
    TSPoint point = touch.getPoint(touchSampleCount);
    int16_t z = point.z;
    if (intervalMillis == 130 || z < 10 && (touchSampleCount > 0 || z > 0)) {
      intervalMillis = 5; // first getPoint after send-receive returns 0!!!
      touchSampleCount += z;
      return;
    }
    touchSampleCount = 0;
#else
    int16_t z = 0;
#endif
    if (!shift) {
      keys = 0x00;
    }
    intervalMillis = 333;
    if (z >= 10) {
#ifdef ARDUINO_ARCH_AVR
      if (messageButton.contains(point.x, point.y)) {
        menu = menu ? (2 * menu) % 8 : 1; // cycle through 0, 1, 2, 4
        intervalMillis = 130; // already waited for 10x 5ms
      } else if (trayButton.contains(point.x, point.y)) {
        keys = 0x01 | 0x08;
        menu = 0;
        intervalMillis = 130;
      } else {
        for (uint8_t i = 0; i < sizeof(buttons)/sizeof(Adafruit_GFX_Button); ++i) {
          if (buttons[i].contains(point.x, point.y)) {
            keys |= (1 << i);
            intervalMillis = 130;
            break;
          }
        }
      }
#endif
    } else {
      for (; Serial.available() > 0; intervalMillis = 200) {
        char key = keyPressed(Serial.read());
        if ('i' == key || 'I' == key) {
          tx_msgs[0].id(0x7C);
          tx_msgs[0].set(0, 0);
          return;
        }
      }
    }
    tx_msgs[0].id(0x7C);
    tx_msgs[0].set(0, 0xDE);
    tx_msgs[0].set(1, keys);
    tx_msgs[0].set(2, menu);
  } else if (0) { // debug
    char buffer[10];
    sprintf(buffer, "%08ld", ++wasted);
    drawMessage(buffer);
  }
}

#endif
