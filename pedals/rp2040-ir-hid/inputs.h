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
