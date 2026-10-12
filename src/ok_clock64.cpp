#include "ok_clock64.h"

#include <Arduino.h>

// Most of clock64.h is inline in the header; exceptions live here.

#if OKT64_ESP8266_MILLIS64

  // Long enough to be put out-of-line. IRAM_ATTR to be ISR-safe.
  int64_t IRAM_ATTR millis64() {
    // The same logic as core millis(), without truncation to 32 bits:
    // https://raw.githubusercontent.com/esp8266/Arduino/master/cores/esp8266/core_esp8266_wiring.cpp
    uint64_t const magic_hi = 0x00418937, magic_lo = 0x4bc6a7f0;
    uint64_t const us = ::micros64();
    uint32_t const us_lo = (uint32_t) us, us_hi = (uint32_t)(us >> 32);
    uint64_t acc = (us_lo * magic_lo) >> 32;
    acc += us_lo * magic_hi;
    acc += us_hi * magic_lo;
    return int64_t((acc >> 32) + us_hi * magic_hi);  // convert to signed
  }

#endif  // OKT64_ESP8266_MILLIS64
