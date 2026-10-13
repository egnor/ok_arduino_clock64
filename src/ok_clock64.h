// 64-bit micros() and millis() for Arduino code.

#pragma once

#include <Arduino.h>
#include <stdint.h>

// inline for speed and ISR-safety (conform to parent's memory region, etc).
#define OKC64_INLINE inline __attribute__((always_inline))

// avoid conflicts with other functions by these names; import with
// `using namespace clock64;` or use a `clock64::` prefix on each call.
namespace ok_clock64 {

int64_t millis64();
int64_t micros64();
OKC64_INLINE double secsd() { return micros64() * 1e-6; }  // Universal

#if defined(ARDUINO_ARCH_ESP32)

  //
  // ESP32: use esp_timer_get_time()
  //
  // safety: esp_timer_get_time() is documented ISR-safe, and is stateless;
  // these wrappers are in IRAM and __divdi3 is in ROM
  //

  #include <esp_timer.h>

  OKC64_INLINE int64_t IRAM_ATTR micros64() { return esp_timer_get_time(); }
  OKC64_INLINE int64_t IRAM_ATTR millis64() { return micros64() / 1000; }

#elif defined(ARDUINO_ARCH_ESP8266)

  //
  // ESP8266: use platform micros64(), with a fast algorithm for /1000
  //
  // safety: single core, no threads, micros64() reads ISR-updated statics
  //
  // inherited caveat: an ISR preempting the timer could get incoherent data
  //

  OKC64_INLINE int64_t IRAM_ATTR micros64() { return ::micros64(); }
  #define OKC64_ESP8266_MILLIS64 1  // activate code in ok_clock64.cpp

#elif defined(ARDUINO_NRF52_ADAFRUIT)

  //
  // Adafruit's nRF52 core based on FreeRTOS: use FreeRTOS timing
  //
  // safety: vTaskSetTimeOutState() uses a critical section to be thread-safe
  // and core-safe; in an ISR we use the internal no-critical-section variant;
  // the timer that updates state masks interrupts up to max-syscall priority
  //
  // caveats:
  // - an ISR preempting the timer could get incoherent data (unlikely)
  // - if dwt_enable() is used, these will diverge from micros()
  //

  OKC64_INLINE uint64_t ticks64() {
    TimeOut_t ts;
    if (__get_IPSR()) {
      // in ISR: use the internal version without the critical section
      vTaskInternalSetTimeOutState(&ts);
    } else {
      // takes a critical section; thread-safe, not ISR-safe (hence the if)
      vTaskSetTimeOutState(&ts);
    }
    uint64_t const hi = (uint64_t)(uint32_t)ts.xOverflowCount << 32;
    return hi | ts.xTimeOnEntering;
  }

  OKC64_INLINE int64_t micros64() {
    return ticks64() * 1000000 / configTICK_RATE_HZ;
  }

  OKC64_INLINE int64_t millis64() {
    return ticks64() * 1000 / configTICK_RATE_HZ;
  }

#elif defined(ARDUINO_ARCH_ZEPHYR)

  //
  // Zephyr RTOS based cores (incl. RP2040 Zephyr core): use Zephyr timing
  //
  // safety: k_uptime_ticks() uses an interrupt-masking spinlock to be
  // thread-safe and ISR-safe (and core-safe, if multicore is ever added)
  //
  // caveat: if 64-bit cycle counters are unavailable, resolution is coarse 
  //

  #include <zephyr/kernel.h>
  #include <zephyr/sys/time_units.h>

  OKC64_INLINE int64_t micros64() {
    #if defined(CONFIG_TIMER_HAS_64BIT_CYCLE_COUNTER)
      return k_cyc_to_us_floor64(k_cycle_get_64());
    #else
      return k_ticks_to_us_floor64(k_uptime_ticks());
    #endif
  }

  OKC64_INLINE int64_t millis64() { return k_uptime_get(); }

#elif defined(ARDUINO_ARCH_RP2040)

  //
  // RP2040 or RP2350[A/B], earlephilhower/rpi-pico OR mbed but NOT Zephyr
  // (the Zephyr core doesn't export Pico-SDK symbols; it's handled above):
  // use platform (Pico-SDK) time_us_64()
  //
  // safety: time_us_64() reads registers directly and is stateless
  //

  OKC64_INLINE int64_t micros64() { return time_us_64(); }
  OKC64_INLINE int64_t millis64() { return micros64() / 1000; }

#else

  //
  // Generic implementation: detect rollover
  //
  // safety: as safe as platform millis() and micros() and compiler arithmetic;
  // the rollover detect uses a lock-free algorithm based on a static volatile
  //
  // caveats:
  // - one of micros64() or millis64() must be called every half-cycle (~24days)
  // - micros() and millis() must not diverge by more than ~35min
  //

  #define OKC64_GENERIC_MILLIS64 1

  OKC64_INLINE int64_t micros64() {
    int64_t const approx = millis64() * 1000;
    return approx + (int32_t)((uint32_t)micros() - (uint32_t)approx);
  }

#endif

}  // namespace ok_clock64
