// this sketch is not run, but verified to compile on various platforms

#include <Arduino.h>
#include <ok_clock64.h>

// https://github.com/adafruit/Adafruit_nRF52_Arduino/issues/653
#if defined(ARDUINO_NRF52_ADAFRUIT)
#include <Adafruit_TinyUSB.h>
#endif

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println(ok_clock64::secsd());
  Serial.println(ok_clock64::millis64() * 1e-3);
  Serial.println(ok_clock64::micros64() * 1e-6);
  delay(100);
}
