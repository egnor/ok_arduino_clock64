// stub Arduino.h for host testing of ESP8266 ok_clock64

#pragma once

#include <stdint.h>

#define ARDUINO_ARCH_ESP8266 1

#define IRAM_ATTR /* empty */

uint64_t micros64();
