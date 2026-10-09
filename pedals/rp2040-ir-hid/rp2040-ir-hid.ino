// RP2040 IR HID: one digital IR proximity sensor as a USB HID keyboard.
//
// The firmware only classifies physical gestures and emits otherwise-unused
// function keys (F15-F16 by default); the host decides what they mean.
// See README.md for wiring, timing model and tuning.

#include <Keyboard.h>
#include <USB.h>
#include <tusb.h>

#include "config.h"
#include "inputs.h"
#include "ir_gesture.h"

// Picked up by the arduino-pico USB descriptor builder.
int usb_hid_poll_interval = HID_POLL_INTERVAL_MS;

Debouncer irSensor;
IrGesture ir;
constexpr IrGestureConfig IR_CONFIG = {
    IR_HOLD_THRESHOLD_MS, IR_REARM_MS, IR_TAP_MS, IR_HOLD_KEY, IR_TAP_KEY,
};

uint8_t reportedIrKey = 0; // key the host currently sees from the IR slot
bool hostReady = false;

bool readIr() { return digitalRead(IR_PIN) == IR_ACTIVE_STATE; }

void setup() {
  USB.disconnect();
  USB.setManufacturer(USB_MANUFACTURER);
  USB.setProduct(USB_PRODUCT);
  USB.connect();

#if DEBUG_SERIAL
  Serial.begin(115200);
#endif

  pinMode(IR_PIN, IR_PIN_MODE);

  const uint32_t now = millis();
  irSensor.reset(readIr(), now);
  ir.begin(IR_CONFIG, irSensor.stable, now);

  Keyboard.begin();
}

void loop() {
  const uint32_t now = millis();

  // Gesture recognition keeps running while the host is away so the state
  // machine never sees a time jump when USB comes back.
  const bool active = irSensor.update(readIr(), now, IR_DEBOUNCE_MS);

#if DEBUG_SERIAL
  const IrState before = ir.state;
#endif
  ir.update(active, now);
#if DEBUG_SERIAL
  if (ir.state != before) {
    Serial.printf("%lu ir active=%d state %d -> %d key=0x%02x\n",
                  (unsigned long)now, active, before, ir.state, ir.key());
  }
#endif

  // tud_ready() is mounted-and-not-suspended and, unlike USB.HIDReady(),
  // never blocks. Keyboard.press/release wait for the endpoint themselves.
  if (!tud_ready()) {
    hostReady = false;
    return;
  }

  if (!hostReady) {
    // After (re-)enumeration or resume the host has forgotten every key, but
    // the Keyboard library's report still remembers them. Clear that report
    // once, then let the slot below replay whatever is still held.
    Keyboard.releaseAll();
    reportedIrKey = 0;
    hostReady = true;
  }

  syncKeySlot(Keyboard, reportedIrKey, ir.key());
}
