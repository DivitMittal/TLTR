#pragma once

// Hardware-independent input helpers shared by the RP2040 HID sketches.
// No Arduino dependencies so pedals/tests/ can exercise them on the host.

#include <stdint.h>

// Non-blocking debouncer: the stable state only follows the raw reading once
// the raw reading has stopped changing for `windowMs`.
struct Debouncer {
  bool stable;
  bool lastRaw;
  uint32_t changedAt;

  void reset(bool raw, uint32_t now) {
    stable = raw;
    lastRaw = raw;
    changedAt = now;
  }

  bool update(bool raw, uint32_t now, uint32_t windowMs) {
    if (raw != lastRaw) {
      lastRaw = raw;
      changedAt = now;
    }
    if (raw != stable && now - changedAt >= windowMs) {
      stable = raw;
    }
    return stable;
  }
};

// Each logical input owns one key slot and only ever releases the key it
// pressed itself. This is what keeps an IR tap from releasing a held pedal
// key (never use releaseAll() to end a single gesture).
// `kbd` is the Arduino Keyboard object, or a fake in host tests.
template <typename Kbd>
void syncKeySlot(Kbd &kbd, uint8_t &reported, uint8_t desired) {
  if (reported == desired) {
    return;
  }
  if (reported != 0) {
    kbd.release(reported);
  }
  if (desired != 0) {
    kbd.press(desired);
  }
  reported = desired;
}
