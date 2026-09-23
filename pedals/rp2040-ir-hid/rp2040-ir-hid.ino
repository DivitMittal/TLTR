// RP2040 IR HID: two digital IR proximity sensors as a USB HID keyboard.
//
// The firmware only classifies physical gestures and emits otherwise-unused
// function keys (F15-F18 by default); the host decides what they mean.
// See README.md for wiring, timing model and tuning.

#include <Keyboard.h>
#include <USB.h>
#include <tusb.h>

#include "config.h"
#include "inputs.h"
#include "ir_gesture.h"

// Picked up by the arduino-pico USB descriptor builder.
int usb_hid_poll_interval = HID_POLL_INTERVAL_MS;

Debouncer irLeft;
Debouncer irRight;
IrGesture ir;
constexpr IrGestureConfig IR_CONFIG = {
    IR_HOLD_THRESHOLD_MS, IR_SWIPE_WINDOW_MS, IR_MIN_SWIPE_SEPARATION_MS,
    IR_REARM_MS,          IR_SWIPE_TAP_MS,    IR_LEFT_HOLD_KEY,
    IR_RIGHT_HOLD_KEY,    IR_SWIPE_LR_KEY,    IR_SWIPE_RL_KEY,
};

uint8_t reportedIrKey = 0; // key the host currently sees from the IR slot

bool readIrLeft() { return digitalRead(IR_LEFT_PIN) == IR_ACTIVE_STATE; }
bool readIrRight() { return digitalRead(IR_RIGHT_PIN) == IR_ACTIVE_STATE; }

void setup() {
  USB.disconnect();
  USB.setManufacturer(USB_MANUFACTURER);
  USB.setProduct(USB_PRODUCT);
  USB.connect();

  pinMode(IR_LEFT_PIN, IR_PIN_MODE);
  pinMode(IR_RIGHT_PIN, IR_PIN_MODE);

  const uint32_t now = millis();
  irLeft.reset(readIrLeft(), now);
  irRight.reset(readIrRight(), now);
  ir.begin(IR_CONFIG, irLeft.stable, irRight.stable, now);

  Keyboard.begin();
}

void loop() {
  const uint32_t now = millis();

  // Gesture recognition keeps running while the host is away so the state
  // machine never sees a time jump when USB comes back.
  const bool left = irLeft.update(readIrLeft(), now, IR_DEBOUNCE_MS);
  const bool right = irRight.update(readIrRight(), now, IR_DEBOUNCE_MS);

  ir.update(left, right, now);

  // tud_ready() is mounted-and-not-suspended and, unlike USB.HIDReady(),
  // never blocks. Keyboard.press/release wait for the endpoint themselves.
  if (!tud_ready()) {
    return;
  }

  syncKeySlot(Keyboard, reportedIrKey, ir.key());
}
