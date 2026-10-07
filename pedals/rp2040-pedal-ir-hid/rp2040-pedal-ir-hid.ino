// RP2040 Pedal + IR HID: two foot pedals and one digital IR proximity sensor
// as one USB HID keyboard.
//
// The firmware only recognises physical events and emits otherwise-unused
// function keys (F13-F16 by default); the host decides what they mean.
// Pedals and IR run independently: each owns its own key slot, so an IR tap
// never releases a held pedal key and vice versa.
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
    IR_HOLD_THRESHOLD_MS, IR_REARM_MS, IR_TAP_MS, IR_HOLD_KEY, IR_TAP_KEY,
};

Debouncer pedals[PEDAL_COUNT];
Debouncer irSensor;
IrGesture ir;

// Keys the host currently sees from each slot (0 = none).
uint8_t reportedPedalKeys[PEDAL_COUNT] = {};
uint8_t reportedIrKey = 0;
bool hostReady = false;

bool readPedal(size_t i) {
  return digitalRead(PEDAL_PINS[i]) == PEDAL_ACTIVE_STATE;
}
bool readIr() { return digitalRead(IR_PIN) == IR_ACTIVE_STATE; }

void setup() {
  USB.disconnect();
  USB.setManufacturer(USB_MANUFACTURER);
  USB.setProduct(USB_PRODUCT);
  USB.connect();

#if DEBUG_SERIAL
  Serial.begin(115200);
#endif

  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    pinMode(PEDAL_PINS[i], PEDAL_PIN_MODE);
  }
  pinMode(IR_PIN, IR_PIN_MODE);

  const uint32_t now = millis();
  for (size_t i = 0; i < PEDAL_COUNT; ++i) {
    pedals[i].reset(readPedal(i), now);
  }
  irSensor.reset(readIr(), now);
  ir.begin(IR_CONFIG, irSensor.stable, now);

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
