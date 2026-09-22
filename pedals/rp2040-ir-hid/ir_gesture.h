#pragma once

// Two-sensor IR gesture classifier. Takes debounced sensor states and the
// current time, and reports which key (if any) should be held right now.
// No Arduino dependencies so pedals/tests/ can exercise it on the host.

#include <stdint.h>

enum IrState : uint8_t {
  IR_IDLE,
  IR_LEFT_PENDING,
  IR_RIGHT_PENDING,
  IR_SWIPE,
  IR_WAIT_CLEAR,
};

struct IrGestureConfig {
  uint32_t swipeWindowMs;
  uint32_t minSwipeSeparationMs;
  uint32_t rearmMs;
  uint32_t swipeTapMs;
  uint8_t swipeLeftToRightKey;
  uint8_t swipeRightToLeftKey;
};
