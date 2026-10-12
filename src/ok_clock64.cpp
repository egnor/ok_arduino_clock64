#include "ok_clock64.h"

#include <Arduino.h>

// Most of clock64.h is inline in the header; exceptions live here.

#if OKC64_ESP8266_MILLIS64

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

#endif  // OKC64_ESP8266_MILLIS64

#if OKC64_GENERIC_MILLIS64

  // borrow a technique from (older) Linux kernel source:
  // https://github.com/torvalds/linux/blob/v6.6/include/linux/cnt32_to_63.h
  // WARNING:
  // - the code must be called at least once per half-period (24.8 days)
  // - the code must not be preempted for anything approaching that period
  static volatile uint32_t state;  // bit 31 = micros() bit 31; bits 0-30 = hi

  int64_t millis64() {
    uint32_t state_copy = state;
    uint32_t const lo = (uint32_t)millis();
    if ((state_copy ^ lo) & 0x80000000u) {
      state_copy = (state_copy ^ 0x80000000u) + (state_copy >> 31);
      state = state_copy;
    }
    return ((int64_t)(state_copy & 0x7FFFFFFFu) << 32) | lo;
  }

#endif  // OKC64_GENERIC_MILLIS64
