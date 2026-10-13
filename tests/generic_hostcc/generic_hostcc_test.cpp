#include <Arduino.h>
#include <inttypes.h>
#include <ok_clock64.h>
#include <stdio.h>

#if !OKC64_GENERIC_MILLIS64
#error OKC64_GENERIC_MILLIS64 should be set here
#endif

using namespace ok_clock64;

unsigned long fake_micros = 0, fake_millis = 0;
unsigned long micros() { return fake_micros; }
unsigned long millis() { return fake_millis; }

static void test_basic_millis64_micros64() {
  printf("#TEST# test_basic_millis64_micros64\n");

  // basic pass-through
  fake_millis = 123456;
  fake_micros = 123456789;
  uint64_t const m1 = millis64(), u1 = micros64();
  if (m1 != 123456) printf("#TEST-FAIL# %" PRIu64 " != 123456\n", m1);
  if (u1 != 123456789) printf("#TEST-FAIL# %" PRIu64 " != 123456789\n", u1);

  // even if micros() disagrees from millis() by a bunch, don't tweak it
  fake_millis = 123456;
  fake_micros = 234567890;
  uint64_t const m2 = millis64(), u2 = micros64();
  if (m2 != 123456) printf("#TEST-FAIL# %" PRIu64 " != 123456\n", m2);
  if (u2 != 234567890) printf("#TEST-FAIL# %" PRIu64 " != 234567890\n", u2);
}

static void test_millis64_rollover() {
  printf("#TEST# test_millis64_rollover\n");

  fake_millis = 0xFFFFFFFF;  // start at maximum millis() value
  uint64_t const m1 = millis64();
  uint32_t const h1 = m1 >> 32, l1 = (uint32_t) m1;
  if (l1 != 0xFFFFFFFF) printf("#TEST-FAIL# %" PRIx32 " != FFFFFFFF\n", l1);

  fake_millis = 0;  // simulate rollover back to 0
  uint64_t const m2 = millis64();
  uint32_t const h2 = m2 >> 32, l2 = (uint32_t) m2;
  if (l2 != 0) printf("#TEST-FAIL# %" PRIx32 " != 0\n", l2);
  if (h2 != h1 + 1) printf("#TEST-FAIL# %" PRIx32 " != %" PRIx32 "+\n", h2, h1);

  fake_millis = 0xFFFFFFFF;  // back up to maximum (no rollover)
  uint64_t const m3 = millis64();
  uint32_t const h3 = m3 >> 32, l3 = (uint32_t) m3;
  if (l3 != 0xFFFFFFFF) printf("#TEST-FAIL# %" PRIx32 " != FFFFFFFF\n", l3);
  if (h3 != h2) printf("#TEST-FAIL# %" PRIx32 " != %" PRIx32 "\n", h3, h2);

  fake_millis = 0x7FFFFFFF;  // rollover back to maximum rolled value
  uint64_t const m4 = millis64();
  uint32_t const h4 = m4 >> 32, l4 = (uint32_t) m4;
  if (l4 != 0x7FFFFFFF) printf("#TEST-FAIL# %" PRIx32 " != 7FFFFFFF\n", l4);
  if (h4 != h3 + 1) printf("#TEST-FAIL# %" PRIx32 " != %" PRIx32 "+\n", h4, h3);
}

static void test_micros64_uses_millis64() {
  printf("#TEST# test_micros64_uses_millis64\n");

  fake_millis = 0xFFFFFFFF;
  millis64();
  fake_millis = 0x12345678;  // ensure at least one rollover
  uint64_t const m = millis64(), um = m * 1000;

  fake_micros = (uint32_t) um;  // align micros with millis (modulo rollover)
  uint64_t const u1 = micros64();
  if (u1 != um) printf("#TEST-FAIL# %" PRIx64 " != %" PRIx64 "\n", u1, um);

  uint64_t const ui2 = um - 0x70000000;  // offset micros from millis value
  fake_micros = (uint32_t) ui2;
  uint64_t const u2 = micros64();
  if (u2 != ui2) printf("#TEST-FAIL# %" PRIx64 " != %" PRIx64 "\n", u2, ui2);

  uint64_t const ui3 = um + 0x70000000;  // offset micros in the other direction
  fake_micros = (uint32_t) ui3;
  uint64_t const u3 = micros64();
  if (u3 != ui3) printf("#TEST-FAIL# %" PRIx64 " != %" PRIx64 "\n", u3, ui3);
}

int main() {
  printf("#BEGIN-TESTS#\n");
  test_basic_millis64_micros64();  // must come before rollover tests
  test_millis64_rollover();
  test_micros64_uses_millis64();
  printf("#END-TESTS#\n");
  return 0;
}
