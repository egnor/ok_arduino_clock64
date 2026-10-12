# OK 64-bit clocks for 32-bit Arduino boards

Implementations of `millis64()` and `micros64()` which return `int64_t` and never wrap around on plausible time scales.

First, consider these alternatives:

- [millis64](https://github.com/RobTillaart/millis64) - another such library, platform-independent, with fewer guarantees
- [esp_timer_get_time()](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/esp_timer.html#esp__timer_8h_1ab38881d22858adab666843cee4ba6589) (ESP32), [micros64()](https://github.com/esp8266/Arduino/blob/1475ed7d49fef5c5167061ac76abb6eced9abda5/cores/esp8266/core_esp8266_features.h#L103) (ESP8266), [time_us_64()](https://www.raspberrypi.com/documentation/pico-sdk/hardware.html#group_hardware_timer_1ga7bbcd996dbd3dd4adc03a80272fb2fc3) (RP2xxx), [k_uptime_get()](https://docs.zephyrproject.org/latest/doxygen/html/group__clock__apis.html#gae3e992cd3257c23d5b26d765fcbb2b69) (Zephyr)
- [millis_64()](https://api-docs.esphome.io/time__64_8h_source) from ESPHome's HAL

This implementation has these features

- Uses hardware timers where possible
- Otherwise uses rollover-sensing (requires calls at least every 49 days)
- Calls are thread-safe, multicore-safe and ISR-safe
- Overhead is minimized

This implementation does NOT support

- 8-bit "classic" Arduino (eg. AVR)
- Real time clocks (RTC) or network time (NTP), it's just time-since-boot

## Usage example

```
#include <ok_clock64.h>

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
}

void loop() {
  // clock64 functions are namespaced; use the namespace, or
  // add `using namespace ok_clock64;` to import them globally.
  int64_t const micros = ok_clock64::micros64();

  // Serial.print() can't do 64-bit numbers...
  Serial.print("micros64: hi=");
  Serial.print(uint32_t(micros >> 32));
  Serial.print(" lo=");
  Serial.println(uint32_t(micros));
  delay(100);
}
```

## Reference

**`ok_clock64::millis64()`**

Returns an `int64_t` with milliseconds since boot.

**`ok_clock64::micros64()`**

Returns an `int64_t` with microseconds since boot.

**`ok_clock64::secsd()`**

Returns a `double` with seconds since boot, in microsecond precision.

## Platform compatibility notes
