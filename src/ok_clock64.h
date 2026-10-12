// 64-bit micros() and millis() for Arduino code.

#pragma once

#include <Arduino.h>
#include <stdint.h>

// inline for speed and ISR-safety (conform to parent's memory region, etc).
#define OKT64_INLINE inline __attribute__((always_inline))

// avoid conflicts with other functions by these names; import with
// `using namespace clock64;` or use a `clock64::` prefix on each call.
namespace ok_clock64 {

// This wrapper is the same for all platforms
double secsd() { return micros64() * 1e-6; }

#if defined(ARDUINO_ARCH_ESP32)

  //
  // ESP32
  // esp_timer_get_time() is lovely, and there's hardware 64-bit div
  //

  #include <esp_timer.h>

  OKT64_INLINE int64_t IRAM_ATTR micros64() { return esp_timer_get_time(); }
  OKT64_INLINE int64_t IRAM_ATTR millis64() { return micros64() / 1000; }

#elif defined(ARDUINO_ARCH_ESP8266)

  //
  // ESP8266
  // has its own micros64(), but 64-bit div is slow; use an algorithm for /1000
  //

  OKT64_INLINE int64_t IRAM_ATTR micros64() { return ::micros64(); }
  int64_t millis64();
  #define OKT64_ESP8266_MILLIS64  // activate code in ok_clock64.cpp

#elif defined(ARDUINO_NRF52_ADAFRUIT)

  //
  // Adafruit's nRF52 core based on FreeRTOS
  // has 
  // NOTE: if dwt_enable() is used, these will diverge from micros().
  //

  OKT64_INLINE uint64_t ticks64() {
    TimeOut_t ts;
    if (__get_IPSR()) {
      vTaskInternalSetTimeOutState(&ts);  // skip critical section if in ISR
    } else {
      vTaskSetTimeOutState(&ts);
    }
    uint64_t const hi = (uint64_t)(uint32_t)ts.xOverflowCount << 32;
    return hi | ts.xTimeOnEntering;
  }

  OKT64_INLINE int64_t micros64() {
    return ticks64() * 1000000 / configTICK_RATE_HZ;
  }

  OKT64_INLINE int64_t millis64() {
    return ticks64() * 1000 / configTICK_RATE_HZ;
  }

#elif defined(ARDUINO_ARCH_ZEPHYR)

  //
  // Zephyr RTOS based cores (incl. RP2040 Zephyr core)
  //

  #include <zephyr/kernel.h>
  #include <zephyr/sys/time_units.h>

  OKT64_INLINE int64_t micros64() {
    #if defined(CONFIG_TIMER_HAS_64BIT_CYCLE_COUNTER)
      return k_cyc_to_us_floor64(k_cycle_get_64());
    #else
      return k_ticks_to_us_floor64(k_uptime_ticks());
    #endif
  }

  OKT64_INLINE int64_t millis64() { return k_uptime_get(); }

#elif defined(ARDUINO_ARCH_RP2040)

  //
  // RP2040 or RP2350[A/B], earlephilhower/rpi-pico OR mbed but NOT Zephyr
  // (the Zephyr core doesn't export Pico-SDK symbols; it's handled above)
  //

  #include <pico/stdlib.h>

  OKT64_INLINE int64_t micros64() { return time_us_64(); }
  OKT64_INLINE int64_t millis64() { return micros64() / 1000; }

#else

  int64_t micros64();
  int64_t millis64();
  #define OKT64_GENERIC_MICRO64 1
  #define OKT64_GENERIC_MILLIS64 1

#endif

}  // namespace ok_clock64
