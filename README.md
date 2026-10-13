# OK 64-bit clocks for 32-bit Arduino boards

Implementations of `millis64()` and `micros64()` which return `int64_t` and never wrap around on plausible time scales.

First, consider these alternatives

- [millis64](https://github.com/RobTillaart/millis64) - another such library; simpler, with fewer guarantees
- [esp_timer_get_time()](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/esp_timer.html#esp__timer_8h_1ab38881d22858adab666843cee4ba6589) (ESP32), [micros64()](https://github.com/esp8266/Arduino/blob/1475ed7d49fef5c5167061ac76abb6eced9abda5/cores/esp8266/core_esp8266_features.h#L103) (ESP8266), [time_us_64()](https://www.raspberrypi.com/documentation/pico-sdk/hardware.html#group_hardware_timer_1ga7bbcd996dbd3dd4adc03a80272fb2fc3) (RP2xxx), [k_uptime_get()](https://docs.zephyrproject.org/latest/doxygen/html/group__clock__apis.html#gae3e992cd3257c23d5b26d765fcbb2b69) (Zephyr)
- [millis_64()](https://api-docs.esphome.io/time__64_8h_source) from ESPHome's HAL

This implementation has these features

- Uses hardware timers where possible
- Otherwise uses rollover detection (requires calls at least every ~24 days)
- Time access is thread-safe, multicore-safe and ISR-safe
- Overhead is minimized
- [Somewhat tested (!)](tests/README.md)

This implementation does NOT support

- 8-bit "classic" Arduino (eg. AVR)
- Real time clocks (RTC) or network time (NTP), it's just time-since-boot

## Usage example

```cpp
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

## Platform implementation notes

| platform | implementation | notable caveats |
| --- | --- | --- |
| [ESP32](https://docs.espressif.com/projects/arduino-esp32/) | `esp_timer_get_time()` | none |
| [ESP8266](https://arduino-esp8266.readthedocs.io/) | core `micros64()`<br>special div-1000 | like core `micros64()`, calls from high priority ISRs could get incoherent values |
| nRF52, [Adafruit core](https://github.com/nRF52-Arduino/nRF52_Arduino) | FreeRTOS ticks | <p>[like `micros()`](https://github.com/adafruit/Adafruit_nRF52_Arduino/issues/451), `micros64()` steps by ~970us; `enable_dwt()` improves `micros()` (with messy rollover) but not `micros64()`</p> <p>calls from ISRs above max-syscall priority could get incoherent values</p> |
| RP2xxx, [arduino-pico](https://arduino-pico.readthedocs.io/) or [mbed core](https://github.com/arduino/ArduinoCore-mbed) (not Zephyr) | `time_us_64()` | none |
| [Arduino on Zephyr](https://github.com/arduino/ArduinoCore-zephyr), w/ 64-bit cycle counter (STM32, RP2040, Renesas) | `k_cycle_get_64()`, `k_uptime_get()` | none |
| [Arduino on Zephyr](https://github.com/arduino/ArduinoCore-zephyr), no 64-bit cycle counter (nRF52, SAMD11, MGM240) | `k_uptime_ticks()`, `k_uptime_get()` | [like `micros()`](https://github.com/arduino/ArduinoCore-zephyr/blob/4fe592f214bd657159067e5517e7a5b596fc2cf8/cores/arduino/zephyrCommon.cpp#L17), `micros64()` steps by ~30us |
| Other | rollover detection | <p>only as thread/core/ISR-safe as platform `micros()` and `millis()`</p> <p>no more than 24 days may elapse between calls to `millis64()` or `micros64()`</p> <p>base `millis()` and `micros()` must stay synced within ~30min; some mbed Arduino targets (eg. Nano 33 BLE) can drift this far in a few months</p> |
