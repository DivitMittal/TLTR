#pragma once

// Compile-time configuration for the RP2040 IR + ultrasonic HID sketch. Every
// macro can be overridden with -D through arduino-cli's
// compiler.cpp.extra_flags build property; see README.md.
//
// Validation uses static_assert rather than #if because LOW/HIGH/INPUT/... are
// enum values in arduino-pico, which the preprocessor silently treats as 0.

#include <Arduino.h>
#include <Keyboard.h>

// ---- Pins (Waveshare RP2040-Zero; GP27/GP28/GP29 are real exposed GPIOs) ---

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

#ifndef ULTRASONIC_TRIG_PIN
#define ULTRASONIC_TRIG_PIN 29
#endif

#ifndef ULTRASONIC_ECHO_PIN
#define ULTRASONIC_ECHO_PIN 27
#endif

// ---- HID keys (Arduino Keyboard codes, KEY_F13..KEY_F24 are 0xF0..0xFB) ----

#ifndef IR_HOLD_KEY
#define IR_HOLD_KEY KEY_F15
#endif

#ifndef IR_TAP_KEY
#define IR_TAP_KEY KEY_F16
#endif

#ifndef VOL_UP_KEY
#define VOL_UP_KEY KEY_F20
#endif

#ifndef VOL_DOWN_KEY
#define VOL_DOWN_KEY KEY_F21
#endif

// ---- Timing (milliseconds unless noted) -------------------------------------

// Comparator chatter around the potentiometer threshold is filtered here.
#ifndef IR_DEBOUNCE_MS
#define IR_DEBOUNCE_MS 10
#endif

// The sensor must stay active this long to count as a hold rather than a tap.
#ifndef IR_HOLD_THRESHOLD_MS
#define IR_HOLD_THRESHOLD_MS 300
#endif

// The sensor must stay clear this long after a gesture before the next one.
#ifndef IR_REARM_MS
#define IR_REARM_MS 150
#endif

// How long the tap key is held down for its single tap.
#ifndef IR_TAP_MS
#define IR_TAP_MS 20
#endif

// How often loop() starts a new ultrasonic ping. Must clear MAX_ECHO_US (plus
// margin) between pings so a slow/absent echo can't bleed into the next one.
#ifndef PING_INTERVAL_MS
#define PING_INTERVAL_MS 60
#endif

// How long to wait for an echo before treating the ping as "nothing in
// range" (microseconds). Scaled to VOL_MAX_RANGE_CM rather than the sensor's
// full ~400cm spec range, so an empty room re-triggers quickly.
#ifndef MAX_ECHO_US
#define MAX_ECHO_US 5000
#endif

// Hand distance (cm) that counts as "near" (volume up) and "far" (volume
// down), each with its own release threshold so a hand resting near a
// boundary can't flicker between zones. nearReleaseCm > nearCm;
// farReleaseCm < farCm.
#ifndef VOL_NEAR_CM
#define VOL_NEAR_CM 10
#endif

#ifndef VOL_NEAR_RELEASE_CM
#define VOL_NEAR_RELEASE_CM 14
#endif

#ifndef VOL_FAR_CM
#define VOL_FAR_CM 25
#endif

#ifndef VOL_FAR_RELEASE_CM
#define VOL_FAR_RELEASE_CM 21
#endif

// Readings beyond this (or a missing echo) are "nothing in range": no zone,
// no volume change.
#ifndef VOL_MAX_RANGE_CM
#define VOL_MAX_RANGE_CM 40
#endif

// While a zone holds, how often a volume key is tapped and for how long.
#ifndef VOL_PULSE_INTERVAL_MS
#define VOL_PULSE_INTERVAL_MS 150
#endif

#ifndef VOL_PULSE_ON_MS
#define VOL_PULSE_ON_MS 30
#endif

// ---- USB --------------------------------------------------------------------

#ifndef HID_POLL_INTERVAL_MS
#define HID_POLL_INTERVAL_MS 1
#endif

#ifndef USB_MANUFACTURER
#define USB_MANUFACTURER "TLTR"
#endif

#ifndef USB_PRODUCT
#define USB_PRODUCT "TLTR IR Ultrasonic HID"
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

constexpr long INPUT_PINS[] = {IR_PIN, ULTRASONIC_TRIG_PIN,
                               ULTRASONIC_ECHO_PIN};
constexpr long EVENT_KEYS[] = {IR_HOLD_KEY, IR_TAP_KEY, VOL_UP_KEY,
                               VOL_DOWN_KEY};

static_assert(isValidPin(IR_PIN) && isValidPin(ULTRASONIC_TRIG_PIN) &&
                  isValidPin(ULTRASONIC_ECHO_PIN),
              "Input pins must be RP2040 GPIOs 0-29");
static_assert(allDistinct(INPUT_PINS),
              "Input GPIO assignments must be distinct");
static_assert(IR_PIN_MODE == INPUT || IR_PIN_MODE == INPUT_PULLUP ||
                  IR_PIN_MODE == INPUT_PULLDOWN,
              "IR_PIN_MODE must be INPUT, INPUT_PULLUP or INPUT_PULLDOWN");
static_assert(IR_ACTIVE_STATE == LOW || IR_ACTIVE_STATE == HIGH,
              "IR_ACTIVE_STATE must be LOW or HIGH");

static_assert(isValidKey(IR_HOLD_KEY) && isValidKey(IR_TAP_KEY) &&
                  isValidKey(VOL_UP_KEY) && isValidKey(VOL_DOWN_KEY),
              "IR and volume keys must be non-zero, non-modifier keycodes");
static_assert(allDistinct(EVENT_KEYS),
              "Every IR and volume event must have its own HID key");

static_assert(IR_DEBOUNCE_MS >= 0 && IR_DEBOUNCE_MS <= 100,
              "IR_DEBOUNCE_MS must be in the 0-100 range");
static_assert(IR_HOLD_THRESHOLD_MS <= 5000,
              "IR_HOLD_THRESHOLD_MS must be at most 5000");
static_assert(IR_REARM_MS >= 0 && IR_REARM_MS <= 5000,
              "IR_REARM_MS must be in the 0-5000 range");
// The press and release must land in separate HID polls.
static_assert(IR_TAP_MS >= 2 * HID_POLL_INTERVAL_MS && IR_TAP_MS <= 500,
              "IR_TAP_MS must cover at least two HID polls and be at most 500");

static_assert(PING_INTERVAL_MS >= 20 && PING_INTERVAL_MS <= 1000,
              "PING_INTERVAL_MS must be in the 20-1000 range");
static_assert(MAX_ECHO_US >= 1000 && MAX_ECHO_US <= 38000,
              "MAX_ECHO_US must be in the 1000-38000 range (HC-SR04 spec max)");
static_assert(VOL_NEAR_CM > 0 && VOL_NEAR_CM < VOL_NEAR_RELEASE_CM,
              "VOL_NEAR_CM must be positive and below VOL_NEAR_RELEASE_CM");
static_assert(VOL_NEAR_RELEASE_CM < VOL_FAR_RELEASE_CM,
              "VOL_NEAR_RELEASE_CM must be below VOL_FAR_RELEASE_CM so the "
              "zones cannot overlap");
static_assert(VOL_FAR_RELEASE_CM < VOL_FAR_CM,
              "VOL_FAR_RELEASE_CM must be below VOL_FAR_CM");
static_assert(VOL_FAR_CM <= VOL_MAX_RANGE_CM,
              "VOL_FAR_CM must not exceed VOL_MAX_RANGE_CM");
static_assert(VOL_MAX_RANGE_CM > 0 && VOL_MAX_RANGE_CM <= 400,
              "VOL_MAX_RANGE_CM must be in the 1-400 range (HC-SR04 spec max)");
// The press and release must land in separate HID polls, and must not
// outlast the cadence they repeat at.
static_assert(VOL_PULSE_ON_MS >= 2 * HID_POLL_INTERVAL_MS &&
                  VOL_PULSE_ON_MS < VOL_PULSE_INTERVAL_MS,
              "VOL_PULSE_ON_MS must cover at least two HID polls and stay "
              "below VOL_PULSE_INTERVAL_MS");
static_assert(VOL_PULSE_INTERVAL_MS <= 2000,
              "VOL_PULSE_INTERVAL_MS must be at most 2000");

static_assert(HID_POLL_INTERVAL_MS >= 1 && HID_POLL_INTERVAL_MS <= 255,
              "HID_POLL_INTERVAL_MS must be in the 1-255 range");
static_assert(sizeof(USB_MANUFACTURER) > 1 && sizeof(USB_PRODUCT) > 1,
              "USB descriptor strings must not be empty");
static_assert(DEBUG_SERIAL == 0 || DEBUG_SERIAL == 1,
              "DEBUG_SERIAL must be 0 or 1");
