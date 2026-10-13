#include <Arduino.h>
#include <initializer_list>
#include <inttypes.h>
#include <ok_clock64.h>
#include <stdio.h>

#if !OKC64_ESP8266_MILLIS64
#error OKC64_ESP8266_MILLIS64 should be set here
#endif

using namespace ok_clock64;

uint64_t fake_micros64 = 0;
uint64_t micros64() { return fake_micros64; }

static void test_micros64_passthrough() {
  printf("#TEST# test_micros64_passthrough\n");

  fake_micros64 = 0x1122334455667788;
  int64_t const u = ok_clock64::micros64();
  if ((uint64_t) u != fake_micros64) {
    printf("#TEST-FAIL# %" PRIx64 " != %" PRIx64 "\n", u, fake_micros64);
  }
}

static void test_millis64_uses_micros64() {
  printf("#TEST# test_millis64_uses_micros64\n");

  for (uint64_t b = 0; b < 0x0040000000000000 - 2000; b = (b + 1) * 3) {
    printf("  %" PRIx64 " ~ +1000\n", b);
    for (uint64_t u = b; u < b + 1000; ++u) {
      fake_micros64 = u;
      int64_t const m = ok_clock64::millis64();
      if ((uint64_t) m != (u / 1000)) {
        printf("#TEST-FAIL# %" PRIu64 " != %" PRIu64 " / 1000\n", m, u);
      }
    }
  }
}

int main() {
  printf("#BEGIN-TESTS#\n");
  test_micros64_passthrough();
  test_millis64_uses_micros64();
  printf("#END-TESTS#\n");
  return 0;
}
