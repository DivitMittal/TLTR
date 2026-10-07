#pragma once

// Compile-time configuration for the RP2040 pedal + IR HID sketch. Every macro
// can be overridden with -D through arduino-cli's compiler.cpp.extra_flags
// build property; see README.md.
//
// Validation uses static_assert rather than #if because LOW/HIGH/INPUT/... are
// enum values in arduino-pico, which the preprocessor silently treats as 0.

#include <Arduino.h>
#include <Keyboard.h>

// ---- Pins (Waveshare RP2040-Zero; GP28 is a real exposed GPIO) ------------

#ifndef PEDAL1_PIN
#define PEDAL1_PIN 14
#endif

#ifndef PEDAL2_PIN
#define PEDAL2_PIN 15
#endif

// Pedals short the pin to GND with no external resistor, so the internal
// pull-up holds a released pedal HIGH.
#ifndef PEDAL_PIN_MODE
#define PEDAL_PIN_MODE INPUT_PULLUP
#endif

// Assumes the pedal's polarity switch is set to normally-open (pressed = short
// to GND). Flip to HIGH instead of rewiring if the switch is reversed.
#ifndef PEDAL_ACTIVE_STATE
#define PEDAL_ACTIVE_STATE LOW
#endif

#ifndef IR_PIN
#define IR_PIN 28
#endif

// The IR module actively drives OUT (comparator + on-board pull-up), so no
// internal pull is needed by default.
#ifndef IR_PIN_MODE
#define IR_PIN_MODE INPUT
#endif

// Logic level on OUT while an object is detected. Most LM393-style obstacle
// modules pull OUT low on detection; flip to HIGH if yours is inverted.
#ifndef IR_ACTIVE_STATE
#define IR_ACTIVE_STATE LOW
#endif

// ---- HID keys (Arduino Keyboard codes, KEY_F13..KEY_F24 are 0xF0..0xFB) ----

#ifndef PEDAL1_KEY
#define PEDAL1_KEY KEY_F13
#endif

#ifndef PEDAL2_KEY
#define PEDAL2_KEY KEY_F14
#endif

#ifndef IR_HOLD_KEY
#define IR_HOLD_KEY KEY_F15
#endif

#ifndef IR_TAP_KEY
#define IR_TAP_KEY KEY_F16
#endif

// ---- Timing (milliseconds) --------------------------------------------------

// Mechanical contact bounce on the pedal switches.
#ifndef PEDAL_DEBOUNCE_MS
#define PEDAL_DEBOUNCE_MS 20
#endif

// Comparator chatter around the potentiometer threshold is filtered here.
#ifndef IR_DEBOUNCE_MS
#define IR_DEBOUNCE_MS 10
#endif

// The IR sensor must stay active this long to count as a hold rather than a
// tap.
#ifndef IR_HOLD_THRESHOLD_MS
#define IR_HOLD_THRESHOLD_MS 300
#endif

// The IR sensor must stay clear this long after a gesture before the next.
#ifndef IR_REARM_MS
#define IR_REARM_MS 150
#endif

// How long the IR tap key is held down for its single tap.
#ifndef IR_TAP_MS
#define IR_TAP_MS 20
#endif

// ---- USB --------------------------------------------------------------------

#ifndef HID_POLL_INTERVAL_MS
#define HID_POLL_INTERVAL_MS 1
#endif

#ifndef USB_MANUFACTURER
#define USB_MANUFACTURER "TLTR"
#endif

#ifndef USB_PRODUCT
#define USB_PRODUCT "TLTR Pedal + IR HID"
#endif

// Optional state/key trace over USB CDC serial. HID never depends on it.
#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL 0
#endif

// ---- Validation -------------------------------------------------------------

constexpr bool isValidPin(long pin) { return pin >= 0 && pin <= 29; }

// Plain keyboard keys only: 0 means "no key", 0x80-0x87 are modifiers.
constexpr bool isValidKey(long key) {
  return key > 0 && key <= 255 &&
         !(key >= KEY_LEFT_CTRL && key <= KEY_RIGHT_GUI);
}

template <size_t N> constexpr bool allDistinct(const long (&values)[N]) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = i + 1; j < N; ++j) {
      if (values[i] == values[j]) {
        return false;
      }
    }
  }
  return true;
}

constexpr long INPUT_PINS[] = {PEDAL1_PIN, PEDAL2_PIN, IR_PIN};
constexpr long EVENT_KEYS[] = {PEDAL1_KEY, PEDAL2_KEY, IR_HOLD_KEY,
                               IR_TAP_KEY};

static_assert(isValidPin(PEDAL1_PIN) && isValidPin(PEDAL2_PIN) &&
                  isValidPin(IR_PIN),
              "Input pins must be RP2040 GPIOs 0-29");
static_assert(allDistinct(INPUT_PINS),
              "Input GPIO assignments must be distinct");
static_assert(PEDAL_PIN_MODE == INPUT || PEDAL_PIN_MODE == INPUT_PULLUP ||
                  PEDAL_PIN_MODE == INPUT_PULLDOWN,
              "PEDAL_PIN_MODE must be INPUT, INPUT_PULLUP or INPUT_PULLDOWN");
static_assert(PEDAL_ACTIVE_STATE == LOW || PEDAL_ACTIVE_STATE == HIGH,
              "PEDAL_ACTIVE_STATE must be LOW or HIGH");
static_assert(IR_PIN_MODE == INPUT || IR_PIN_MODE == INPUT_PULLUP ||
                  IR_PIN_MODE == INPUT_PULLDOWN,
              "IR_PIN_MODE must be INPUT, INPUT_PULLUP or INPUT_PULLDOWN");
static_assert(IR_ACTIVE_STATE == LOW || IR_ACTIVE_STATE == HIGH,
              "IR_ACTIVE_STATE must be LOW or HIGH");

static_assert(isValidKey(PEDAL1_KEY) && isValidKey(PEDAL2_KEY),
              "Pedal keys must be non-zero, non-modifier keycodes");
static_assert(isValidKey(IR_HOLD_KEY) && isValidKey(IR_TAP_KEY),
              "IR keys must be non-zero, non-modifier keycodes");
static_assert(allDistinct(EVENT_KEYS),
              "Every pedal and IR event must have its own HID key");

static_assert(PEDAL_DEBOUNCE_MS >= 0 && PEDAL_DEBOUNCE_MS <= 100,
              "PEDAL_DEBOUNCE_MS must be in the 0-100 range");
static_assert(IR_DEBOUNCE_MS >= 0 && IR_DEBOUNCE_MS <= 100,
              "IR_DEBOUNCE_MS must be in the 0-100 range");
static_assert(IR_HOLD_THRESHOLD_MS <= 5000,
              "IR_HOLD_THRESHOLD_MS must be at most 5000");
static_assert(IR_REARM_MS >= 0 && IR_REARM_MS <= 5000,
              "IR_REARM_MS must be in the 0-5000 range");
// The press and release must land in separate HID polls.
static_assert(IR_TAP_MS >= 2 * HID_POLL_INTERVAL_MS && IR_TAP_MS <= 500,
              "IR_TAP_MS must cover at least two HID polls and be at most 500");

static_assert(HID_POLL_INTERVAL_MS >= 1 && HID_POLL_INTERVAL_MS <= 255,
              "HID_POLL_INTERVAL_MS must be in the 1-255 range");
static_assert(sizeof(USB_MANUFACTURER) > 1 && sizeof(USB_PRODUCT) > 1,
              "USB descriptor strings must not be empty");
static_assert(DEBUG_SERIAL == 0 || DEBUG_SERIAL == 1,
              "DEBUG_SERIAL must be 0 or 1");
