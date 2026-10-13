# Automated tests for 64-bit clocks

Running a bench of hardware for 50 days would be inconvenient for development.
Instead, we have a set of targeted tests to try to verify behavior.

## Running the tests

1. Install a C/C++ toolchain (eg. `sudo apt install build-essential`)
2. Install [mise-en-place](https://mise.jdx.dev/)
3. In this directory, run `mise run tests`

If mise is hooked into the shell, you may also (after `mise install`)
use `pytest` directly.

## Test coverage, by platform

Build-only tests verify compilation for many targets ([build_only](build_only))

**ESP32:** Trivial wrapper, no runtime coverage at present.

**ESP8266:** `micros64()` is a trivial wrapper; the optimized divide-by-1000 for `millis64()` is tested locally ([esp8266_hostcc](esp8266_hostcc))

**nRF52 Adafruit core:** No runtime coverage at present.

**Zephyr cores:** (Mostly) trivial wrapper, no runtime coverage at present.

**RP2040**: Trivial wrapper, no runtime coverage at present.

**Generic implementation:** Rollover detection is tested locally ([generic_hostcc](generic_hostcc))

**To do:** Add emulated tests (using renode, Wokwi, rp2040.js, etc) for popular platforms (ESP32, RP2040, etc), "poking" near-rollover values into system timers and verifying rollover behavior.
