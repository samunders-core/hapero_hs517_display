#ifdef ARDUINO_ARCH_AVR

#include "TouchScreen.h"
#include "Arduino.h"

struct SharedPinsTouchScreen : TouchScreen {
  const uint16_t w, h;
  const uint8_t yp, xm; // TODO: save 2 bytes by making them protected in superclass

  SharedPinsTouchScreen(uint16_t w_, uint16_t h_, uint8_t yp_, uint8_t xm_) : TouchScreen(6, yp_, xm_, 7, 300), w(w_), h(h_), yp(yp_), xm(xm_) {}

  TSPoint getPoint(int16_t previous_z = 0) {
      TSPoint result = TouchScreen::getPoint();
      pinMode(yp, OUTPUT);      //restore shared pins
      pinMode(xm, OUTPUT);
      digitalWrite(yp, HIGH);   //because TFT control pins
      digitalWrite(xm, HIGH);
      result.x = map(result.x, 910, 265, 0, w);
      result.y = map(result.y, 950, 153, 0, h);
      result.z = previous_z + ((result.z > 20 && result.z < 1000) ? 1 : 0);
      return result;
  }
  /* touch PORTRAIT calibration:
  LEFT 896 RT 333 -2.53
  TOP 688 BOT 486 -0.63
  WID 239 HT 319
  x = map(p.x, LEFT, RT, 0, tft.width());
  y = map(p.y, TOP, BOT, 0, tft.height());
  with 3 samples 240x320:
  LEFT 910 RT 265
  TOP 950 BOT 153
  */
};

#endif