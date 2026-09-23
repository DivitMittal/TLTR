#pragma once

// Compile-time configuration for the RP2040 IR HID sketch. Every macro can be
// overridden with -D through arduino-cli's compiler.cpp.extra_flags build
// property; see README.md.
//
// Validation uses static_assert rather than #if because LOW/HIGH/INPUT/... are
// enum values in arduino-pico, which the preprocessor silently treats as 0.

#include <Arduino.h>
#include <Keyboard.h>

// ---- Pins (Waveshare RP2040-Zero; GP28/GP29 are real exposed GPIOs) --------

#ifndef IR_LEFT_PIN
#define IR_LEFT_PIN 29
#endif

#ifndef IR_RIGHT_PIN
#define IR_RIGHT_PIN 28
#endif

// The IR modules actively drive OUT (comparator + on-board pull-up), so no
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

#ifndef IR_LEFT_HOLD_KEY
#define IR_LEFT_HOLD_KEY KEY_F15
#endif

#ifndef IR_RIGHT_HOLD_KEY
#define IR_RIGHT_HOLD_KEY KEY_F16
#endif

#ifndef IR_SWIPE_LR_KEY
#define IR_SWIPE_LR_KEY KEY_F17
#endif

#ifndef IR_SWIPE_RL_KEY
#define IR_SWIPE_RL_KEY KEY_F18
#endif

// ---- Timing (milliseconds) --------------------------------------------------

// Comparator chatter around the potentiometer threshold is filtered here. The
// same delay applies to both sensors, so swipe ordering is preserved.
#ifndef IR_DEBOUNCE_MS
#define IR_DEBOUNCE_MS 10
#endif

// A lone sensor must stay active this long before it counts as a hold. Shorter
// activations produce nothing, so a hand passing by is ignored.
#ifndef IR_HOLD_THRESHOLD_MS
#define IR_HOLD_THRESHOLD_MS 300
#endif

// Maximum onset gap between the first and second sensor for a swipe.
#ifndef IR_SWIPE_WINDOW_MS
#define IR_SWIPE_WINDOW_MS 250
#endif

// Onset gaps shorter than this cannot be given a direction and are discarded.
#ifndef IR_MIN_SWIPE_SEPARATION_MS
#define IR_MIN_SWIPE_SEPARATION_MS 20
#endif

// Both sensors must stay clear this long after a gesture before the next one.
#ifndef IR_REARM_MS
#define IR_REARM_MS 150
#endif

// How long a swipe key is held down for its single tap.
#ifndef IR_SWIPE_TAP_MS
#define IR_SWIPE_TAP_MS 20
#endif

// ---- USB --------------------------------------------------------------------

#ifndef USB_MANUFACTURER
#define USB_MANUFACTURER "TLTR"
#endif

#ifndef USB_PRODUCT
#define USB_PRODUCT "TLTR IR HID"
#endif
