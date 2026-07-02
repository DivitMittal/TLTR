#ifndef PEDAL1_PIN
#define PEDAL1_PIN 2
#endif

#ifndef PEDAL2_PIN
#define PEDAL2_PIN 3
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

constexpr uint8_t PEDAL_PINS[] = {PEDAL1_PIN, PEDAL2_PIN};
constexpr size_t PEDAL_COUNT = sizeof(PEDAL_PINS) / sizeof(PEDAL_PINS[0]);
constexpr unsigned long DEBOUNCE_MS = PEDAL_DEBOUNCE_MS;

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
#define PEDAL_KEYCODE 0
#endif

#if PEDAL_KEYCODE < 0 || PEDAL_KEYCODE > 255
#error "PEDAL_KEYCODE must be in the 0-255 range"
#endif

#if PEDAL_SEPARATE_KEYS
#if defined(PEDAL_KEYCODE_OVERRIDE_DEFINED)
#error "PEDAL_KEYCODE cannot be overridden when PEDAL_SEPARATE_KEYS is enabled"
#endif

#ifndef PEDAL1_KEYCODE
#define PEDAL1_KEYCODE 1
#endif

#ifndef PEDAL2_KEYCODE
#define PEDAL2_KEYCODE 2
#endif

#if PEDAL1_KEYCODE < 1 || PEDAL1_KEYCODE > 255 || PEDAL2_KEYCODE < 1 || PEDAL2_KEYCODE > 255
#error "PEDAL1_KEYCODE and PEDAL2_KEYCODE must be in the 1-255 range"
#endif

#if PEDAL1_KEYCODE == PEDAL2_KEYCODE
#error "PEDAL1_KEYCODE and PEDAL2_KEYCODE must be different when PEDAL_SEPARATE_KEYS is enabled"
#endif

constexpr uint8_t PEDAL_KEYCODES[] = {PEDAL1_KEYCODE, PEDAL2_KEYCODE};

#ifndef PEDAL1_ALT_KEYCODE
#define PEDAL1_ALT_KEYCODE 3
#endif

#ifndef PEDAL2_ALT_KEYCODE
#define PEDAL2_ALT_KEYCODE 4
#endif

#if PEDAL1_ALT_KEYCODE < 1 || PEDAL1_ALT_KEYCODE > 255 || PEDAL2_ALT_KEYCODE < 1 || PEDAL2_ALT_KEYCODE > 255
#error "PEDAL1_ALT_KEYCODE and PEDAL2_ALT_KEYCODE must be in the 1-255 range"
#endif

#if PEDAL1_ALT_KEYCODE == PEDAL1_KEYCODE || PEDAL2_ALT_KEYCODE == PEDAL2_KEYCODE
#error "Each pedal's alternate keycode must differ from its primary keycode"
#endif

constexpr uint8_t PEDAL_ALT_KEYCODES[] = {PEDAL1_ALT_KEYCODE, PEDAL2_ALT_KEYCODE};

#ifndef PEDAL_MULTI_TAP_WINDOW_MS
#define PEDAL_MULTI_TAP_WINDOW_MS 250
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
unsigned long sequenceNumber = 0;

#if PEDAL_SEPARATE_KEYS
enum PedalGesture : uint8_t {
  GESTURE_IDLE,
  GESTURE_FIRST_DOWN,
  GESTURE_WAIT_SECOND,
  GESTURE_PRIMARY_HOLD,
  GESTURE_ALT_HOLD,
};

PedalGesture gestureState[PEDAL_COUNT];
unsigned long gestureTimer[PEDAL_COUNT];
uint8_t activeKeycode[PEDAL_COUNT];
uint8_t reportedKeycode[PEDAL_COUNT];
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

void emitEvent(const char* event) {
  Serial.print("EVT ");
  Serial.print(sequenceNumber++);
  Serial.print(" ");
  Serial.print(event);
  Serial.print(" ");
  Serial.println(millis());
}

#if PEDAL_SEPARATE_KEYS
void updatePedalGesture(size_t pedalIndex, bool pressed, unsigned long now) {
  switch (gestureState[pedalIndex]) {
    case GESTURE_IDLE:
      if (pressed) {
        gestureState[pedalIndex] = GESTURE_FIRST_DOWN;
        gestureTimer[pedalIndex] = now;
      }
      break;

    case GESTURE_FIRST_DOWN:
      if (!pressed) {
        gestureState[pedalIndex] = GESTURE_WAIT_SECOND;
        gestureTimer[pedalIndex] = now;
      } else if (now - gestureTimer[pedalIndex] >= MULTI_TAP_WINDOW_MS) {
        gestureState[pedalIndex] = GESTURE_PRIMARY_HOLD;
        activeKeycode[pedalIndex] = PEDAL_KEYCODES[pedalIndex];
      }
      break;

    case GESTURE_WAIT_SECOND:
      if (now - gestureTimer[pedalIndex] >= MULTI_TAP_WINDOW_MS) {
        if (pressed) {
          gestureState[pedalIndex] = GESTURE_FIRST_DOWN;
          gestureTimer[pedalIndex] = now;
        } else {
          gestureState[pedalIndex] = GESTURE_IDLE;
        }
      } else if (pressed) {
        gestureState[pedalIndex] = GESTURE_ALT_HOLD;
        activeKeycode[pedalIndex] = PEDAL_ALT_KEYCODES[pedalIndex];
      }
      break;

    case GESTURE_PRIMARY_HOLD:
    case GESTURE_ALT_HOLD:
      if (!pressed) {
        activeKeycode[pedalIndex] = 0;
        gestureState[pedalIndex] = GESTURE_IDLE;
      }
      break;
  }
}
#endif

void setup() {
  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    pinMode(PEDAL_PINS[i], PEDAL_PIN_MODE);
    stablePressed[i] = readPedalPressed(i);
    lastReadingPressed[i] = stablePressed[i];
    lastChangeAt[i] = millis();
#if PEDAL_SEPARATE_KEYS
    gestureState[i] = GESTURE_IDLE;
    gestureTimer[i] = lastChangeAt[i];
    activeKeycode[i] = 0;
    reportedKeycode[i] = 0;
#endif
  }

  Serial.begin(115200);
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

#if PEDAL_SEPARATE_KEYS
  const unsigned long now = millis();
  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    updatePedalGesture(i, stablePressed[i], now);
  }

  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    if (activeKeycode[i] == reportedKeycode[i]) {
      continue;
    }

    if (activeKeycode[i] != 0) {
      const char* pedalName = (i == 0) ? "P1" : "P2";
      if (activeKeycode[i] == PEDAL_KEYCODES[i]) {
        emitEvent(pedalName);
      } else {
        char eventBuf[16];
        snprintf(eventBuf, sizeof(eventBuf), "%s_ALT", pedalName);
        emitEvent(eventBuf);
      }
    } else if (reportedKeycode[i] != 0) {
      const char* pedalName = (i == 0) ? "P1" : "P2";
      char eventBuf[16];
      snprintf(eventBuf, sizeof(eventBuf), "%s_UP", pedalName);
      emitEvent(eventBuf);
    }
    reportedKeycode[i] = activeKeycode[i];
  }
#else
  const bool combinedPressed = anyStablePedalPressed();

  if (combinedPressed == reportedCombinedPressed) {
    return;
  }

  emitEvent(combinedPressed ? "DOWN" : "UP");
  reportedCombinedPressed = combinedPressed;
#endif
}
