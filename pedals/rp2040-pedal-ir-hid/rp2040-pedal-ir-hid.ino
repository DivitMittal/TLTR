// RP2040 Pedal + IR HID: two foot pedals and two digital IR proximity sensors
// as one USB HID keyboard.
//
// The firmware only recognises physical events and emits otherwise-unused
// function keys (F13-F18 by default); the host decides what they mean.
// Pedals and IR run independently: each owns its own key slot, so an IR swipe
// tap never releases a held pedal key and vice versa.
// See README.md for wiring, timing model and tuning.

#include <Keyboard.h>
#include <USB.h>
#include <tusb.h>

#include "config.h"
#include "inputs.h"
#include "ir_gesture.h"

// Picked up by the arduino-pico USB descriptor builder.
int usb_hid_poll_interval = HID_POLL_INTERVAL_MS;

constexpr uint8_t PEDAL_PINS[] = {PEDAL1_PIN, PEDAL2_PIN};
constexpr uint8_t PEDAL_KEYS[] = {PEDAL1_KEY, PEDAL2_KEY};
constexpr size_t PEDAL_COUNT = sizeof(PEDAL_PINS) / sizeof(PEDAL_PINS[0]);

constexpr IrGestureConfig IR_CONFIG = {
    IR_HOLD_THRESHOLD_MS, IR_SWIPE_WINDOW_MS, IR_MIN_SWIPE_SEPARATION_MS,
    IR_REARM_MS,          IR_SWIPE_TAP_MS,    IR_LEFT_HOLD_KEY,
    IR_RIGHT_HOLD_KEY,    IR_SWIPE_LR_KEY,    IR_SWIPE_RL_KEY,
};

Debouncer pedals[PEDAL_COUNT];
Debouncer irLeft;
Debouncer irRight;
IrGesture ir;

// Keys the host currently sees from each slot (0 = none).
uint8_t reportedPedalKeys[PEDAL_COUNT] = {};
uint8_t reportedIrKey = 0;
bool hostReady = false;

bool readPedal(size_t i) {
  return digitalRead(PEDAL_PINS[i]) == PEDAL_ACTIVE_STATE;
}
bool readIrLeft() { return digitalRead(IR_LEFT_PIN) == IR_ACTIVE_STATE; }
bool readIrRight() { return digitalRead(IR_RIGHT_PIN) == IR_ACTIVE_STATE; }

void setup() {
  USB.disconnect();
  USB.setManufacturer(USB_MANUFACTURER);
  USB.setProduct(USB_PRODUCT);
  USB.connect();

  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    pinMode(PEDAL_PINS[i], PEDAL_PIN_MODE);
  }
  pinMode(IR_LEFT_PIN, IR_PIN_MODE);
  pinMode(IR_RIGHT_PIN, IR_PIN_MODE);

  const uint32_t now = millis();
  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    pedals[i].reset(readPedal(i), now);
  }
  irLeft.reset(readIrLeft(), now);
  irRight.reset(readIrRight(), now);
  ir.begin(IR_CONFIG, irLeft.stable, irRight.stable, now);

  Keyboard.begin();
}

void loop() {
  const uint32_t now = millis();

  // Inputs and gestures keep running while the host is away so nothing sees a
  // time jump when USB comes back.
  bool pedalDown[PEDAL_COUNT];
  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    pedalDown[i] = pedals[i].update(readPedal(i), now, PEDAL_DEBOUNCE_MS);
  }

  const bool left = irLeft.update(readIrLeft(), now, IR_DEBOUNCE_MS);
  const bool right = irRight.update(readIrRight(), now, IR_DEBOUNCE_MS);

  ir.update(left, right, now);

  // tud_ready() is mounted-and-not-suspended and, unlike USB.HIDReady(),
  // never blocks. Keyboard.press/release wait for the endpoint themselves.
  if (!tud_ready()) {
    hostReady = false;
    return;
  }

  if (!hostReady) {
    // After (re-)enumeration or resume the host has forgotten every key, but
    // the Keyboard library's report still remembers them. Clear that report
    // once, then let the slots below replay whatever is still held.
    Keyboard.releaseAll();
    for (size_t i = 0; i < PEDAL_COUNT; ++i) {
      reportedPedalKeys[i] = 0;
    }
    reportedIrKey = 0;
    hostReady = true;
  }

  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    syncKeySlot(Keyboard, reportedPedalKeys[i],
                pedalDown[i] ? PEDAL_KEYS[i] : 0);
  }
  syncKeySlot(Keyboard, reportedIrKey, ir.key());
}
