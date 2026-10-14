// Host-side tests for the hardware-independent parts of the RP2040 HID
// sketches (inputs.h, ir_gesture.h). Run via pedals/tests/run.sh.
//
// Each scenario drives raw pedal/IR signals through the same debounce ->
// gesture -> key-slot pipeline the firmware's loop() uses, at 1 ms steps, and
// records every press/release a fake Keyboard sees.

#include <cstdint>
#include <cstdio>
#include <functional>
#include <vector>

#include "inputs.h"
#include "ir_gesture.h"

// volume_gesture.h only exists in sketches that pair the IR sensor with an
// ultrasonic ranger (e.g. rp2040-ir-ultrasonic-hid); __has_include lets this
// one shared test file skip those scenarios for sketches without it (e.g.
// rp2040-ir-hid) instead of needing a second copy of the whole file.
#if __has_include("volume_gesture.h")
#include "volume_gesture.h"
#define HAVE_VOLUME_GESTURE 1
#else
#define HAVE_VOLUME_GESTURE 0
#endif

// Arduino Keyboard codes for F13..F16, and F20/F21 where volume_gesture.h
// exists.
constexpr uint8_t F13 = 0xF0, F14 = 0xF1, F15 = 0xF2, F16 = 0xF3;
#if HAVE_VOLUME_GESTURE
constexpr uint8_t F20 = 0xF7, F21 = 0xF8;
#endif

// Mirrors the defaults in config.h.
constexpr uint32_t PEDAL_DEBOUNCE_MS = 20;
constexpr uint32_t IR_DEBOUNCE_MS = 10;
constexpr IrGestureConfig IR_CONFIG = {300, 150, 20, F15, F16};

struct KeyEvent {
  uint32_t t;
  uint8_t key;
  bool down;
};

struct FakeKeyboard {
  std::vector<KeyEvent> log;
  uint8_t held[6] = {};
  uint32_t now = 0;
  bool overflow = false;

  void press(uint8_t k) {
    for (uint8_t &slot : held) {
      if (slot == k)
        return;
    }
    for (uint8_t &slot : held) {
      if (slot == 0) {
        slot = k;
        log.push_back({now, k, true});
        return;
      }
    }
    overflow = true;
  }

  void release(uint8_t k) {
    for (uint8_t &slot : held) {
      if (slot == k) {
        slot = 0;
        log.push_back({now, k, false});
      }
    }
  }

  bool isHeld(uint8_t k) const {
    for (uint8_t slot : held) {
      if (slot == k)
        return true;
    }
    return false;
  }
};

using Signal = std::function<bool(uint32_t)>;

Signal off() {
  return [](uint32_t) { return false; };
}

Signal on(uint32_t start, uint32_t end) {
  return [=](uint32_t t) { return t >= start && t < end; };
}

Signal both(Signal a, Signal b) {
  return [=](uint32_t t) { return a(t) || b(t); };
}

// Toggles every `period` ms inside [start, end): comparator chatter.
Signal chatter(uint32_t start, uint32_t end, uint32_t period) {
  return [=](uint32_t t) {
    return t >= start && t < end && ((t - start) / period) % 2 == 0;
  };
}

struct Inputs {
  Signal p1 = off(), p2 = off(), ir = off();
};

// Runs the combined-firmware pipeline from t0 for `duration` ms. Signal time
// is relative to t0 so wraparound can be tested.
FakeKeyboard simulate(const Inputs &in, uint32_t duration, uint32_t t0 = 0) {
  FakeKeyboard kbd;
  Debouncer p1, p2, irSensor;
  IrGesture ir;
  uint8_t reportedP1 = 0, reportedP2 = 0, reportedIr = 0;

  p1.reset(in.p1(0), t0);
  p2.reset(in.p2(0), t0);
  irSensor.reset(in.ir(0), t0);
  ir.begin(IR_CONFIG, irSensor.stable, t0);

  for (uint32_t t = 0; t <= duration; ++t) {
    const uint32_t now = t0 + t;
    kbd.now = t;
    const bool p1Down = p1.update(in.p1(t), now, PEDAL_DEBOUNCE_MS);
    const bool p2Down = p2.update(in.p2(t), now, PEDAL_DEBOUNCE_MS);
    ir.update(irSensor.update(in.ir(t), now, IR_DEBOUNCE_MS), now);

    syncKeySlot(kbd, reportedP1, p1Down ? F13 : 0);
    syncKeySlot(kbd, reportedP2, p2Down ? F14 : 0);
    syncKeySlot(kbd, reportedIr, ir.key());
  }
  return kbd;
}

#if HAVE_VOLUME_GESTURE
using DistanceSignal = std::function<uint16_t(uint32_t)>;

DistanceSignal constantDistance(uint16_t cm) {
  return [=](uint32_t) { return cm; };
}

// Distance holds at the first step's value, then changes at each later step.
DistanceSignal stepDistance(std::vector<std::pair<uint32_t, uint16_t>> steps) {
  return [=](uint32_t t) {
    uint16_t value = DISTANCE_OUT_OF_RANGE;
    for (const auto &step : steps) {
      if (t >= step.first) {
        value = step.second;
      }
    }
    return value;
  };
}

// Mirrors the defaults in config.h.
constexpr VolumeGestureConfig VOL_CONFIG = {10, 14, 25, 21, 40, 150, 30, F20,
                                            F21};

FakeKeyboard simulateVolume(const DistanceSignal &distance, uint32_t duration,
                            uint32_t t0 = 0) {
  FakeKeyboard kbd;
  VolumeGesture vol;
  uint8_t reported = 0;

  vol.begin(VOL_CONFIG, t0);

  for (uint32_t t = 0; t <= duration; ++t) {
    const uint32_t now = t0 + t;
    kbd.now = t;
    vol.update(distance(t), now);
    syncKeySlot(kbd, reported, vol.key());
  }
  return kbd;
}
#endif

int failures = 0;
const char *currentTest = "";

#define CHECK(cond)                                                          \
  do {                                                                       \
    if (!(cond)) {                                                           \
      std::printf("FAIL [%s] %s:%d: %s\n", currentTest, __FILE__, __LINE__,  \
                   #cond);                                                   \
      ++failures;                                                            \
    }                                                                        \
  } while (0)

int presses(const FakeKeyboard &kbd, uint8_t key) {
  int n = 0;
  for (const KeyEvent &e : kbd.log) {
    if (e.key == key && e.down)
      ++n;
  }
  return n;
}

// Time of the n-th (0-based) press or release of `key`, or UINT32_MAX.
uint32_t eventTime(const FakeKeyboard &kbd, uint8_t key, bool down, int n = 0) {
  for (const KeyEvent &e : kbd.log) {
    if (e.key == key && e.down == down && n-- == 0)
      return e.t;
  }
  return UINT32_MAX;
}

// Exactly one press/release pair of `key` and nothing else at all.
bool onlyTap(const FakeKeyboard &kbd, uint8_t key) {
  return kbd.log.size() == 2 && kbd.log[0].key == key && kbd.log[0].down &&
         kbd.log[1].key == key && !kbd.log[1].down;
}

void test(const char *name, const std::function<void()> &body) {
  currentTest = name;
  body();
}

int main() {
  test("pedal 1 hold", [] {
    Inputs in;
    in.p1 = on(100, 600);
    FakeKeyboard k = simulate(in, 1000);
    CHECK(onlyTap(k, F13));
    CHECK(eventTime(k, F13, true) == 100 + PEDAL_DEBOUNCE_MS);
    CHECK(eventTime(k, F13, false) == 600 + PEDAL_DEBOUNCE_MS);
  });

  test("pedal 2 hold", [] {
    Inputs in;
    in.p2 = on(100, 600);
    FakeKeyboard k = simulate(in, 1000);
    CHECK(onlyTap(k, F14));
  });

  test("both pedals held together", [] {
    Inputs in;
    in.p1 = on(100, 800);
    in.p2 = on(105, 600);
    FakeKeyboard k = simulate(in, 1000);
    CHECK(presses(k, F13) == 1 && presses(k, F14) == 1);
    CHECK(eventTime(k, F14, false) > eventTime(k, F13, true));
    CHECK(eventTime(k, F14, false) < eventTime(k, F13, false));
    CHECK(k.log.size() == 4);
  });

  test("pedal contact bounce", [] {
    Inputs in;
    in.p1 = both(chatter(100, 115, 2), on(115, 600));
    FakeKeyboard k = simulate(in, 1000);
    CHECK(onlyTap(k, F13));
  });

  test("IR hold", [] {
    Inputs in;
    in.ir = on(100, 1000);
    FakeKeyboard k = simulate(in, 1500);
    CHECK(onlyTap(k, F15));
    CHECK(eventTime(k, F15, true) == 100 + IR_DEBOUNCE_MS + 300);
    CHECK(eventTime(k, F15, false) == 1000 + IR_DEBOUNCE_MS);
  });

  test("IR tap", [] {
    Inputs in;
    in.ir = on(100, 250);
    FakeKeyboard k = simulate(in, 1000);
    CHECK(onlyTap(k, F16));
    CHECK(eventTime(k, F16, true) == 250 + IR_DEBOUNCE_MS);
    CHECK(eventTime(k, F16, false) - eventTime(k, F16, true) == 20);
  });

  test("brief flicker still taps", [] {
    Inputs in;
    in.ir = on(100, 115);
    FakeKeyboard k = simulate(in, 1000);
    CHECK(onlyTap(k, F16));
  });

  test("a tap does not stop a later hold", [] {
    Inputs in;
    in.ir = both(on(100, 150), on(400, 1000));
    FakeKeyboard k = simulate(in, 1500);
    CHECK(presses(k, F16) == 1 && presses(k, F15) == 1);
    CHECK(eventTime(k, F16, false) < eventTime(k, F15, true));
  });

  test("short glitch during a hold does not release", [] {
    Inputs in;
    in.ir = both(on(100, 600), on(605, 1000));
    FakeKeyboard k = simulate(in, 1500);
    CHECK(onlyTap(k, F15));
  });

  test("comparator chatter before a hold", [] {
    Inputs in;
    in.ir = both(chatter(100, 200, 3), on(200, 1000));
    FakeKeyboard k = simulate(in, 1500);
    CHECK(onlyTap(k, F15));
  });

  test("chatter alone emits nothing", [] {
    Inputs in;
    in.ir = chatter(100, 1000, 4);
    CHECK(simulate(in, 1500).log.empty());
  });

  test("two separate taps", [] {
    Inputs in;
    in.ir = both(on(100, 200), on(700, 800));
    FakeKeyboard k = simulate(in, 1200);
    CHECK(presses(k, F16) == 2 && k.log.size() == 4);
  });

  test("a re-touch during re-arm is swallowed entirely", [] {
    // IR clears at 160 (debounced) after the first tap; it returns at 200,
    // inside the 150 ms re-arm interval, and then stays active far longer
    // than the hold threshold. None of that produces a second event: the
    // sensor must go fully clear and re-arm before it can be classified
    // again.
    Inputs in;
    in.ir = both(on(100, 150), on(200, 1000));
    FakeKeyboard k = simulate(in, 1500);
    CHECK(onlyTap(k, F16));
  });

  test("pedal 1 stays held through an IR tap", [] {
    Inputs in;
    in.p1 = on(50, 1500);
    in.ir = on(500, 650);
    FakeKeyboard k = simulate(in, 2000);
    CHECK(presses(k, F13) == 1 && presses(k, F16) == 1);
    CHECK(eventTime(k, F16, false) < eventTime(k, F13, false));
    CHECK(eventTime(k, F13, false) == 1500 + PEDAL_DEBOUNCE_MS);
    CHECK(k.log.size() == 4);
  });

  test("pedal 2 stays held through an IR hold", [] {
    Inputs in;
    in.p2 = on(50, 1500);
    in.ir = on(300, 1000);
    FakeKeyboard k = simulate(in, 2000);
    CHECK(presses(k, F14) == 1 && presses(k, F15) == 1);
    CHECK(eventTime(k, F15, false) < eventTime(k, F14, false));
    CHECK(k.log.size() == 4);
  });

  test("both pedals held through an IR tap", [] {
    Inputs in;
    in.p1 = on(50, 1500);
    in.p2 = on(60, 1500);
    in.ir = on(500, 650);
    FakeKeyboard k = simulate(in, 2000);
    CHECK(presses(k, F13) == 1 && presses(k, F14) == 1 &&
          presses(k, F16) == 1);
    CHECK(k.log.size() == 6 && !k.overflow);
  });

  test("sensor active at boot waits for clear", [] {
    Inputs in;
    in.ir = both(on(0, 800), on(1000, 1500));
    FakeKeyboard k = simulate(in, 2000);
    CHECK(onlyTap(k, F15));
    CHECK(eventTime(k, F15, true) == 1000 + IR_DEBOUNCE_MS + 300);
  });

  test("millis() wraparound", [] {
    Inputs in;
    in.p1 = on(100, 900);
    in.ir = both(on(100, 250), on(1000, 1600));
    FakeKeyboard k = simulate(in, 2000, UINT32_MAX - 500);
    CHECK(presses(k, F13) == 1 && presses(k, F16) == 1 &&
          presses(k, F15) == 1);
    CHECK(k.log.size() == 6);
  });

#if HAVE_VOLUME_GESTURE
  test("near zone repeats volume-up taps", [] {
    FakeKeyboard k = simulateVolume(constantDistance(5), 1000);
    CHECK(presses(k, F20) >= 5);
    CHECK(presses(k, F21) == 0);
    CHECK(eventTime(k, F20, true) == 150);
    CHECK(eventTime(k, F20, false) - eventTime(k, F20, true) == 30);
  });

  test("far zone repeats volume-down taps", [] {
    FakeKeyboard k = simulateVolume(constantDistance(30), 1000);
    CHECK(presses(k, F21) >= 5);
    CHECK(presses(k, F20) == 0);
  });

  test("dead zone between near and far emits nothing", [] {
    CHECK(simulateVolume(constantDistance(18), 1000).log.empty());
  });

  test("exactly at the far threshold enters the far zone", [] {
    FakeKeyboard k = simulateVolume(constantDistance(25), 1000);
    CHECK(presses(k, F21) >= 1);
  });

  test("missing echo emits nothing", [] {
    CHECK(
        simulateVolume(constantDistance(DISTANCE_OUT_OF_RANGE), 1000)
            .log.empty());
  });

  test("beyond max range emits nothing", [] {
    CHECK(simulateVolume(constantDistance(45), 1000).log.empty());
  });

  test("near zone hysteresis tolerates creeping back towards the threshold",
       [] {
         DistanceSignal d = stepDistance({{0, 8}, {400, 12}});
         FakeKeyboard k = simulateVolume(d, 800);
         CHECK(presses(k, F20) >= 1);
         CHECK(presses(k, F21) == 0);
       });

  test("leaving the near zone's hysteresis band stops the taps", [] {
    DistanceSignal d = stepDistance({{0, 8}, {400, 16}});
    FakeKeyboard k = simulateVolume(d, 1000);
    int pressesAfterExit = 0;
    for (const KeyEvent &e : k.log) {
      if (e.down && e.t > 430) {
        ++pressesAfterExit;
      }
    }
    CHECK(pressesAfterExit == 0);
  });

  test("far zone hysteresis tolerates creeping back towards the threshold",
       [] {
         DistanceSignal d = stepDistance({{0, 26}, {400, 23}});
         FakeKeyboard k = simulateVolume(d, 800);
         CHECK(presses(k, F21) >= 1);
         CHECK(presses(k, F20) == 0);
       });

  test("volume millis() wraparound", [] {
    FakeKeyboard k =
        simulateVolume(constantDistance(5), 500, UINT32_MAX - 250);
    CHECK(presses(k, F20) >= 1);
  });
#endif

  if (failures != 0) {
    std::printf("%d check(s) failed\n", failures);
    return 1;
  }
  std::printf("all gesture tests passed\n");
  return 0;
}
