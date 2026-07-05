#include <Keyboard.h>
#include <USB.h>

#ifndef PEDAL_BOARD_WAVESHARE_RP2040_ZERO
#define PEDAL_BOARD_WAVESHARE_RP2040_ZERO 0
#endif

#if PEDAL_BOARD_WAVESHARE_RP2040_ZERO != 0 && PEDAL_BOARD_WAVESHARE_RP2040_ZERO != 1
#error "PEDAL_BOARD_WAVESHARE_RP2040_ZERO must be 0 or 1"
#endif

#ifndef PEDAL_BOARD_WAVESHARE_RP2040_ONE
#define PEDAL_BOARD_WAVESHARE_RP2040_ONE 0
#endif

#if PEDAL_BOARD_WAVESHARE_RP2040_ONE != 0 && PEDAL_BOARD_WAVESHARE_RP2040_ONE != 1
#error "PEDAL_BOARD_WAVESHARE_RP2040_ONE must be 0 or 1"
#endif

#ifndef PEDAL_BOARD_WAVESHARE_RP2040
#define PEDAL_BOARD_WAVESHARE_RP2040 0
#endif

#if PEDAL_BOARD_WAVESHARE_RP2040 != 0 && PEDAL_BOARD_WAVESHARE_RP2040 != 1
#error "PEDAL_BOARD_WAVESHARE_RP2040 must be 0 or 1"
#endif

#if PEDAL_BOARD_WAVESHARE_RP2040_ZERO && PEDAL_BOARD_WAVESHARE_RP2040_ONE
#error "Enable only one Waveshare RP2040 board preset at a time"
#endif

#if PEDAL_BOARD_WAVESHARE_RP2040_ZERO || PEDAL_BOARD_WAVESHARE_RP2040_ONE
#undef PEDAL_BOARD_WAVESHARE_RP2040
#define PEDAL_BOARD_WAVESHARE_RP2040 1
#endif

#if PEDAL_BOARD_WAVESHARE_RP2040
#ifndef PEDAL_USB_MANUFACTURER
#define PEDAL_USB_MANUFACTURER "Waveshare"
#endif

#ifndef PEDAL_USB_PRODUCT
#if PEDAL_BOARD_WAVESHARE_RP2040_ZERO
#define PEDAL_USB_PRODUCT "Waveshare RP2040 Zero Dual Pedal HID"
#elif PEDAL_BOARD_WAVESHARE_RP2040_ONE
#define PEDAL_USB_PRODUCT "Waveshare RP2040 One Dual Pedal HID"
#else
#define PEDAL_USB_PRODUCT "Waveshare RP2040 Dual Pedal HID"
#endif
#endif
#endif

#ifndef PEDAL1_PIN
#define PEDAL1_PIN 14
#endif

#ifndef PEDAL2_PIN
#define PEDAL2_PIN 15
#endif

#if PEDAL1_PIN < 0 || PEDAL2_PIN < 0
#error "PEDAL1_PIN and PEDAL2_PIN must be in the 0-255 range"
#endif

#if PEDAL1_PIN > 255 || PEDAL2_PIN > 255
#error "PEDAL1_PIN and PEDAL2_PIN must be in the 0-255 range"
#endif

#if PEDAL1_PIN == PEDAL2_PIN
#error "PEDAL1_PIN and PEDAL2_PIN must be different pins"
#endif

#ifndef PEDAL_SEPARATE_KEYS
#define PEDAL_SEPARATE_KEYS 1
#endif

#if PEDAL_SEPARATE_KEYS != 0 && PEDAL_SEPARATE_KEYS != 1
#error "PEDAL_SEPARATE_KEYS must be 0 or 1"
#endif

#ifndef PEDAL_DEBOUNCE_MS
#define PEDAL_DEBOUNCE_MS 25
#endif

#if PEDAL_DEBOUNCE_MS < 0
#error "PEDAL_DEBOUNCE_MS must be zero or greater"
#endif

#ifndef PEDAL_HID_POLL_INTERVAL_MS
#define PEDAL_HID_POLL_INTERVAL_MS 1
#endif

#if PEDAL_HID_POLL_INTERVAL_MS <= 0 || PEDAL_HID_POLL_INTERVAL_MS > 255
#error "PEDAL_HID_POLL_INTERVAL_MS must be in the 1-255 range"
#endif

#ifndef PEDAL_PRESSED_STATE
#define PEDAL_PRESSED_STATE LOW
#endif

#if PEDAL_PRESSED_STATE != LOW && PEDAL_PRESSED_STATE != HIGH
#error "PEDAL_PRESSED_STATE must be LOW or HIGH"
#endif

#ifndef PEDAL_PIN_MODE
#define PEDAL_PIN_MODE INPUT_PULLUP
#endif

#if PEDAL_PIN_MODE != INPUT && PEDAL_PIN_MODE != INPUT_PULLUP
#error "PEDAL_PIN_MODE must be INPUT or INPUT_PULLUP"
#endif

#ifdef PEDAL_USB_MANUFACTURER
constexpr char USB_MANUFACTURER_NAME[] = PEDAL_USB_MANUFACTURER;
static_assert(sizeof(USB_MANUFACTURER_NAME) > 1,
              "PEDAL_USB_MANUFACTURER must not be empty");
#endif

#ifdef PEDAL_USB_PRODUCT
constexpr char USB_PRODUCT_NAME[] = PEDAL_USB_PRODUCT;
static_assert(sizeof(USB_PRODUCT_NAME) > 1,
              "PEDAL_USB_PRODUCT must not be empty");
#endif

constexpr uint8_t PEDAL_PINS[] = {PEDAL1_PIN, PEDAL2_PIN};
constexpr size_t PEDAL_COUNT = sizeof(PEDAL_PINS) / sizeof(PEDAL_PINS[0]);
constexpr unsigned long DEBOUNCE_MS = PEDAL_DEBOUNCE_MS;

int usb_hid_poll_interval = PEDAL_HID_POLL_INTERVAL_MS;

#ifdef PEDAL_KEYCODE
#define PEDAL_KEYCODE_OVERRIDE_DEFINED 1
#endif

#ifdef PEDAL1_KEYCODE
#define PEDAL1_KEYCODE_OVERRIDE_DEFINED 1
#endif

#ifdef PEDAL2_KEYCODE
#define PEDAL2_KEYCODE_OVERRIDE_DEFINED 1
#endif

#ifndef PEDAL_KEYCODE
#define PEDAL_KEYCODE KEY_F13
#endif

#if PEDAL_KEYCODE <= 0 || PEDAL_KEYCODE > 255
#error "PEDAL_KEYCODE must be in the 1-255 range"
#endif

#if PEDAL_SEPARATE_KEYS
#if defined(PEDAL_KEYCODE_OVERRIDE_DEFINED)
#error "PEDAL_KEYCODE cannot be overridden when PEDAL_SEPARATE_KEYS is enabled"
#endif

#ifndef PEDAL1_KEYCODE
#define PEDAL1_KEYCODE KEY_F19
#endif

#ifndef PEDAL2_KEYCODE
#define PEDAL2_KEYCODE KEY_F13
#endif

#if PEDAL1_KEYCODE <= 0 || PEDAL1_KEYCODE > 255 || PEDAL2_KEYCODE <= 0 || PEDAL2_KEYCODE > 255
#error "PEDAL1_KEYCODE and PEDAL2_KEYCODE must be in the 1-255 range"
#endif

#if PEDAL1_KEYCODE == PEDAL2_KEYCODE
#error "PEDAL1_KEYCODE and PEDAL2_KEYCODE must be different when PEDAL_SEPARATE_KEYS is enabled"
#endif

constexpr uint8_t PEDAL_KEYCODES[] = {PEDAL1_KEYCODE, PEDAL2_KEYCODE};

// Double-tap-and-hold can reuse the primary function key with a different
// modifier chord.
#ifndef PEDAL1_ALT_KEYCODE
#define PEDAL1_ALT_KEYCODE KEY_F19
#endif

#ifndef PEDAL2_ALT_KEYCODE
#define PEDAL2_ALT_KEYCODE KEY_F13
#endif

#if PEDAL1_ALT_KEYCODE <= 0 || PEDAL1_ALT_KEYCODE > 255 || PEDAL2_ALT_KEYCODE <= 0 || PEDAL2_ALT_KEYCODE > 255
#error "PEDAL1_ALT_KEYCODE and PEDAL2_ALT_KEYCODE must be in the 1-255 range"
#endif

#ifndef PEDAL1_MODIFIERS
#define PEDAL1_MODIFIERS 0
#endif

#ifndef PEDAL1_ALT_MODIFIERS
#define PEDAL1_ALT_MODIFIERS KEY_LEFT_CTRL
#endif

#ifndef PEDAL2_MODIFIERS
#define PEDAL2_MODIFIERS KEY_LEFT_CTRL
#endif

#ifndef PEDAL2_ALT_MODIFIERS
#define PEDAL2_ALT_MODIFIERS KEY_LEFT_CTRL, KEY_LEFT_SHIFT
#endif

constexpr uint8_t PEDAL_ALT_KEYCODES[] = {PEDAL1_ALT_KEYCODE, PEDAL2_ALT_KEYCODE};

constexpr uint8_t PEDAL_MODIFIER_LISTS[][2] = {
    {PEDAL1_MODIFIERS},
    {PEDAL2_MODIFIERS},
};
constexpr uint8_t PEDAL_ALT_MODIFIER_LISTS[][2] = {
    {PEDAL1_ALT_MODIFIERS},
    {PEDAL2_ALT_MODIFIERS},
};

static_assert(PEDAL1_KEYCODE != PEDAL1_ALT_KEYCODE ||
                  PEDAL_MODIFIER_LISTS[0][0] != PEDAL_ALT_MODIFIER_LISTS[0][0] ||
                  PEDAL_MODIFIER_LISTS[0][1] != PEDAL_ALT_MODIFIER_LISTS[0][1],
              "Pedal 1 primary and alternate chords must differ");
static_assert(PEDAL2_KEYCODE != PEDAL2_ALT_KEYCODE ||
                  PEDAL_MODIFIER_LISTS[1][0] != PEDAL_ALT_MODIFIER_LISTS[1][0] ||
                  PEDAL_MODIFIER_LISTS[1][1] != PEDAL_ALT_MODIFIER_LISTS[1][1],
              "Pedal 2 primary and alternate chords must differ");

// A tap shorter than this becomes the first half of a double-tap; a press held
// longer resolves as a single press-and-hold. It is also the maximum gap
// between the first tap's release and the second press.
#ifndef PEDAL_MULTI_TAP_WINDOW_MS
#define PEDAL_MULTI_TAP_WINDOW_MS 150
#endif

#if PEDAL_MULTI_TAP_WINDOW_MS <= 0 || PEDAL_MULTI_TAP_WINDOW_MS > 60000
#error "PEDAL_MULTI_TAP_WINDOW_MS must be in the 1-60000 range"
#endif

constexpr unsigned long MULTI_TAP_WINDOW_MS = PEDAL_MULTI_TAP_WINDOW_MS;
#else
#if defined(PEDAL1_KEYCODE_OVERRIDE_DEFINED) || defined(PEDAL2_KEYCODE_OVERRIDE_DEFINED)
#error "PEDAL1_KEYCODE and PEDAL2_KEYCODE require PEDAL_SEPARATE_KEYS=1"
#endif
#endif

bool stablePressed[PEDAL_COUNT];
bool lastReadingPressed[PEDAL_COUNT];
unsigned long lastChangeAt[PEDAL_COUNT];
bool usbHidWasReady = false;

#if PEDAL_SEPARATE_KEYS
// Per-pedal tap/hold gesture recognizer. Each pedal resolves to either its
// primary keycode (single press-and-hold) or its alternate keycode
// (double-tap-and-hold), or nothing while a gesture is still being classified.
enum PedalGesture : uint8_t {
  GESTURE_IDLE,         // no pedal activity
  GESTURE_FIRST_DOWN,   // first press down, not yet classified as tap or hold
  GESTURE_WAIT_SECOND,  // first press was a short tap; awaiting a second press
  GESTURE_PRIMARY_HOLD, // single press-and-hold; primary key held
  GESTURE_ALT_HOLD,     // double-tap-and-hold; alternate key held
};

PedalGesture gestureState[PEDAL_COUNT];
unsigned long gestureTimer[PEDAL_COUNT];  // press time in FIRST_DOWN, release time in WAIT_SECOND
uint8_t activeKeycode[PEDAL_COUNT];       // keycode the gesture wants held now (0 = none)
bool activeAlternate[PEDAL_COUNT];        // whether the active chord is the double-tap chord
uint8_t reportedKeycode[PEDAL_COUNT];     // keycode currently reported to the host (0 = none)
bool reportedAlternate[PEDAL_COUNT];      // whether the reported chord is the double-tap chord
#else
bool reportedCombinedPressed = false;
#endif

bool readPedalPressed(size_t pedalIndex) {
  return digitalRead(PEDAL_PINS[pedalIndex]) == PEDAL_PRESSED_STATE;
}

bool anyStablePedalPressed() {
  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    if (stablePressed[i]) {
      return true;
    }
  }

  return false;
}

void sendCombinedKeyboardState(bool pressed) {
  if (pressed) {
    Keyboard.press(PEDAL_KEYCODE);
    return;
  }

  Keyboard.release(PEDAL_KEYCODE);
}

#if PEDAL_SEPARATE_KEYS
static inline const uint8_t* pedalModifiers(size_t pedalIndex, bool alternate) {
  return alternate ? PEDAL_ALT_MODIFIER_LISTS[pedalIndex]
                   : PEDAL_MODIFIER_LISTS[pedalIndex];
}

void pressPedalModifiers(size_t pedalIndex, bool alternate) {
  const uint8_t* modifiers = pedalModifiers(pedalIndex, alternate);
  for (size_t i = 0; i < 2; ++i) {
    if (modifiers[i] != 0) {
      Keyboard.press(modifiers[i]);
    }
  }
}

void releasePedalModifiers(size_t pedalIndex, bool alternate) {
  const uint8_t* modifiers = pedalModifiers(pedalIndex, alternate);
  for (size_t i = 2; i > 0; --i) {
    if (modifiers[i - 1] != 0) {
      Keyboard.release(modifiers[i - 1]);
    }
  }
}

// Advance one pedal's tap/hold recognizer from its debounced pressed state and
// the current time, updating activeKeycode[pedalIndex] with the keycode that
// should be held (0 = nothing yet / released).
void updatePedalGesture(size_t pedalIndex, bool pressed, unsigned long now) {
  switch (gestureState[pedalIndex]) {
    case GESTURE_IDLE:
      if (pressed) {
        gestureState[pedalIndex] = GESTURE_FIRST_DOWN;
        gestureTimer[pedalIndex] = now;  // remember press time
      }
      break;

    case GESTURE_FIRST_DOWN:
      if (!pressed) {
        // Released before the window elapsed: a tap. Wait for a second press.
        gestureState[pedalIndex] = GESTURE_WAIT_SECOND;
        gestureTimer[pedalIndex] = now;  // remember release time
      } else if (now - gestureTimer[pedalIndex] >= MULTI_TAP_WINDOW_MS) {
        // Held past the window with no release: a single press-and-hold.
        gestureState[pedalIndex] = GESTURE_PRIMARY_HOLD;
        activeKeycode[pedalIndex] = PEDAL_KEYCODES[pedalIndex];
        activeAlternate[pedalIndex] = false;
      }
      break;

    case GESTURE_WAIT_SECOND:
      if (now - gestureTimer[pedalIndex] >= MULTI_TAP_WINDOW_MS) {
        // Window expired. A lone tap emits nothing; a late press starts fresh.
        if (pressed) {
          gestureState[pedalIndex] = GESTURE_FIRST_DOWN;
          gestureTimer[pedalIndex] = now;
        } else {
          gestureState[pedalIndex] = GESTURE_IDLE;
        }
      } else if (pressed) {
        // Second press within the window: double-tap-and-hold.
        gestureState[pedalIndex] = GESTURE_ALT_HOLD;
        activeKeycode[pedalIndex] = PEDAL_ALT_KEYCODES[pedalIndex];
        activeAlternate[pedalIndex] = true;
      }
      break;

    case GESTURE_PRIMARY_HOLD:
    case GESTURE_ALT_HOLD:
      if (!pressed) {
        activeKeycode[pedalIndex] = 0;
        activeAlternate[pedalIndex] = false;
        gestureState[pedalIndex] = GESTURE_IDLE;
      }
      break;
  }
}
#endif

void setup() {
#if defined(PEDAL_USB_MANUFACTURER) || defined(PEDAL_USB_PRODUCT)
  USB.disconnect();

#ifdef PEDAL_USB_MANUFACTURER
  USB.setManufacturer(USB_MANUFACTURER_NAME);
#endif

#ifdef PEDAL_USB_PRODUCT
  USB.setProduct(USB_PRODUCT_NAME);
#endif

  USB.connect();
#endif

  Serial.begin(115200);

  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    pinMode(PEDAL_PINS[i], PEDAL_PIN_MODE);
    stablePressed[i] = readPedalPressed(i);
    lastReadingPressed[i] = stablePressed[i];
    lastChangeAt[i] = millis();
#if PEDAL_SEPARATE_KEYS
    gestureState[i] = GESTURE_IDLE;
    gestureTimer[i] = lastChangeAt[i];
    activeKeycode[i] = 0;
    activeAlternate[i] = false;
    reportedKeycode[i] = 0;
    reportedAlternate[i] = false;
#endif
  }

  Keyboard.begin();
}

void loop() {
  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    const bool readingPressed = readPedalPressed(i);

    if (readingPressed != lastReadingPressed[i]) {
      lastReadingPressed[i] = readingPressed;
      lastChangeAt[i] = millis();
    }

    if ((millis() - lastChangeAt[i]) < DEBOUNCE_MS || readingPressed == stablePressed[i]) {
      continue;
    }

    stablePressed[i] = readingPressed;
  }

  // Keep debouncing live during enumeration, but do not mark any key state as
  // reported until the host is ready to receive HID reports.
  const bool usbHidReady = USB.HIDReady();

  if (!usbHidReady) {
    usbHidWasReady = false;
    return;
  }

  if (!usbHidWasReady) {
    // Force whatever is currently held to be replayed after any USB
    // disconnect/re-enumeration instead of waiting for another pedal change.
    // Gesture state is preserved, only the reported layer is cleared so a key
    // held across the disconnect is pressed again once HID is ready.
#if PEDAL_SEPARATE_KEYS
    for (size_t i = 0; i < PEDAL_COUNT; ++i) {
      reportedKeycode[i] = 0;
      reportedAlternate[i] = false;
    }
#else
    reportedCombinedPressed = false;
#endif

    usbHidWasReady = true;
  }

#if PEDAL_SEPARATE_KEYS
  // Advance each pedal's tap/hold recognizer, which sets the keycode it wants
  // held (primary for a single hold, alternate for a double-tap-and-hold).
  const unsigned long now = millis();
  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    updatePedalGesture(i, stablePressed[i], now);
  }

  bool stateChanged = false;
  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    if (activeKeycode[i] != reportedKeycode[i] ||
        activeAlternate[i] != reportedAlternate[i]) {
      stateChanged = true;
      break;
    }
  }

  if (!stateChanged) {
    return;
  }

  // Replay all active pedal chords together. Modifiers are report-global in USB
  // keyboard HID, so replaying as a set avoids releasing Ctrl/Shift out from
  // under another held pedal.
  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    if (reportedKeycode[i] != 0) {
      Keyboard.release(reportedKeycode[i]);
    }
  }
  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    if (reportedKeycode[i] != 0) {
      releasePedalModifiers(i, reportedAlternate[i]);
    }
  }
  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    if (activeKeycode[i] != 0) {
      pressPedalModifiers(i, activeAlternate[i]);
    }
  }
  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    if (activeKeycode[i] != 0) {
      Keyboard.press(activeKeycode[i]);
    }
    reportedKeycode[i] = activeKeycode[i];
    reportedAlternate[i] = activeAlternate[i];
  }
#else
  const bool combinedPressed = anyStablePedalPressed();

  if (combinedPressed == reportedCombinedPressed) {
    return;
  }

  sendCombinedKeyboardState(combinedPressed);
  reportedCombinedPressed = combinedPressed;
#endif
}
