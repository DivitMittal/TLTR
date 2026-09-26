#pragma once

// Two-sensor IR gesture classifier. Takes debounced sensor states and the
// current time, and reports which key (if any) should be held right now.
// No Arduino dependencies so pedals/tests/ can exercise it on the host.

#include <stdint.h>

enum IrState : uint8_t {
  IR_IDLE,
  IR_LEFT_PENDING,
  IR_RIGHT_PENDING,
  IR_LEFT_HOLD,
  IR_RIGHT_HOLD,
  IR_SWIPE,
  IR_WAIT_CLEAR,
};

struct IrGestureConfig {
  uint32_t holdThresholdMs;
  uint32_t swipeWindowMs;
  uint32_t minSwipeSeparationMs;
  uint32_t rearmMs;
  uint32_t swipeTapMs;
  uint8_t leftHoldKey;
  uint8_t rightHoldKey;
  uint8_t swipeLeftToRightKey;
  uint8_t swipeRightToLeftKey;
};

struct IrGesture {
  IrGestureConfig cfg;
  IrState state;
  uint32_t since;     // onset in *_PENDING/SWIPE, last activity in WAIT_CLEAR
  bool firstReleased; // *_PENDING: S has cleared at least once
  uint8_t swipeKey;   // SWIPE: which swipe key is being tapped

  // Sensors already active at boot cannot be classified reliably, so start by
  // waiting for them to clear.
  void begin(const IrGestureConfig &config, bool left, bool right,
             uint32_t now) {
    cfg = config;
    state = IR_IDLE;
    since = now;
    firstReleased = false;
    swipeKey = 0;
    if (left || right) {
      enterWaitClear(now);
    }
  }

  void update(bool left, bool right, uint32_t now) {
    switch (state) {
    case IR_IDLE:
      if (left && right) {
        enterWaitClear(now);
      } else if (left || right) {
        state = left ? IR_LEFT_PENDING : IR_RIGHT_PENDING;
        since = now;
        firstReleased = false;
      }
      break;

    case IR_LEFT_PENDING:
    case IR_RIGHT_PENDING: {
      const bool leftFirst = state == IR_LEFT_PENDING;
      const bool first = leftFirst ? left : right;
      const bool other = leftFirst ? right : left;
      const uint32_t dt = now - since;

      if (!first) {
        firstReleased = true;
      }

      if (other) {
        if (dt < cfg.minSwipeSeparationMs || dt > cfg.swipeWindowMs) {
          enterWaitClear(now);
        } else {
          state = IR_SWIPE;
          since = now;
          swipeKey =
              leftFirst ? cfg.swipeLeftToRightKey : cfg.swipeRightToLeftKey;
        }
      } else if (!firstReleased && dt >= cfg.holdThresholdMs) {
        state = leftFirst ? IR_LEFT_HOLD : IR_RIGHT_HOLD;
      } else if (firstReleased && dt > cfg.swipeWindowMs) {
        // Nothing was emitted, so there is nothing to re-arm from unless S
        // came back (flicker) and still needs to clear.
        if (first) {
          enterWaitClear(now);
        } else {
          state = IR_IDLE;
        }
      }
      break;
    }

    case IR_LEFT_HOLD:
      if (!left) {
        enterWaitClear(now);
      }
      break;

    case IR_RIGHT_HOLD:
      if (!right) {
        enterWaitClear(now);
      }
      break;

    case IR_SWIPE:
      if (now - since >= cfg.swipeTapMs) {
        enterWaitClear(now);
      }
      break;

    case IR_WAIT_CLEAR:
      if (left || right) {
        since = now;
      } else if (now - since >= cfg.rearmMs) {
        state = IR_IDLE;
      }
      break;
    }
  }

  // Key the IR subsystem wants held right now (0 = none).
  uint8_t key() const {
    switch (state) {
    case IR_LEFT_HOLD:
      return cfg.leftHoldKey;
    case IR_RIGHT_HOLD:
      return cfg.rightHoldKey;
    case IR_SWIPE:
      return swipeKey;
    default:
      return 0;
    }
  }

  void enterWaitClear(uint32_t now) {
    state = IR_WAIT_CLEAR;
    since = now;
  }
};
