#pragma once

// Single-sensor IR gesture classifier. Takes the debounced sensor state and
// the current time, and reports which key (if any) should be held right now.
// No Arduino dependencies so pedals/tests/ can exercise it on the host.
//
// Vocabulary: tap (brief activation) and hold (sustained activation). Every
// physical gesture gets exactly one classification; once committed it is
// never reinterpreted, and the classifier then waits for the sensor to clear.
//
// State transitions (dt = time since the sensor became active):
//
//   IDLE
//     sensor active ........................................ -> PENDING
//
//   PENDING (nothing emitted yet)
//     sensor clears, dt < holdThreshold .................... -> TAP
//     sensor continuously active, dt >= holdThreshold ....... -> HOLD
//
//   HOLD (hold key down)
//     sensor clears .......................................... -> WAIT_CLEAR
//
//   TAP (tap key down)
//     tapMs elapsed ........................................... -> WAIT_CLEAR
//
//   WAIT_CLEAR (nothing emitted)
//     sensor clear continuously for rearm .................... -> IDLE
//
// All times are unsigned millisecond differences, so millis() wraparound is
// harmless.

#include <stdint.h>

enum IrState : uint8_t {
  IR_IDLE,
  IR_PENDING,
  IR_HOLD,
  IR_TAP,
  IR_WAIT_CLEAR,
};

struct IrGestureConfig {
  uint32_t holdThresholdMs;
  uint32_t rearmMs;
  uint32_t tapMs;
  uint8_t holdKey;
  uint8_t tapKey;
};

struct IrGesture {
  IrGestureConfig cfg;
  IrState state;
  uint32_t since; // onset in PENDING/TAP, last activity in WAIT_CLEAR

  // A sensor already active at boot cannot be classified reliably, so start
  // by waiting for it to clear.
  void begin(const IrGestureConfig &config, bool active, uint32_t now) {
    cfg = config;
    state = IR_IDLE;
    since = now;
    if (active) {
      enterWaitClear(now);
    }
  }

  void update(bool active, uint32_t now) {
    switch (state) {
    case IR_IDLE:
      if (active) {
        state = IR_PENDING;
        since = now;
      }
      break;

    case IR_PENDING:
      if (!active) {
        state = IR_TAP;
        since = now;
      } else if (now - since >= cfg.holdThresholdMs) {
        state = IR_HOLD;
      }
      break;

    case IR_HOLD:
      if (!active) {
        enterWaitClear(now);
      }
      break;

    case IR_TAP:
      if (now - since >= cfg.tapMs) {
        enterWaitClear(now);
      }
      break;

    case IR_WAIT_CLEAR:
      if (active) {
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
    case IR_HOLD:
      return cfg.holdKey;
    case IR_TAP:
      return cfg.tapKey;
    default:
      return 0;
    }
  }

  void enterWaitClear(uint32_t now) {
    state = IR_WAIT_CLEAR;
    since = now;
  }
};
