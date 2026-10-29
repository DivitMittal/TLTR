// RP2040 IR + ultrasonic HID: one digital IR proximity sensor and one
// HC-SR04-style ultrasonic distance sensor as a USB HID keyboard.
//
// The firmware only classifies physical gestures and emits otherwise-unused
// function keys (F15/F16 for IR, F20/F21 for volume by default); the host
// decides what they mean. See README.md for wiring, timing model and tuning.

#include <Keyboard.h>
#include <USB.h>
#include <tusb.h>

#include "config.h"
#include "inputs.h"
#include "ir_gesture.h"
#include "volume_gesture.h"

// Picked up by the arduino-pico USB descriptor builder.
int usb_hid_poll_interval = HID_POLL_INTERVAL_MS;

Debouncer irSensor;
IrGesture ir;
constexpr IrGestureConfig IR_CONFIG = {
    IR_HOLD_THRESHOLD_MS, IR_REARM_MS, IR_TAP_MS, IR_HOLD_KEY, IR_TAP_KEY,
};

VolumeGesture vol;
constexpr VolumeGestureConfig VOL_CONFIG = {
    VOL_NEAR_CM,          VOL_NEAR_RELEASE_CM, VOL_FAR_CM,
    VOL_FAR_RELEASE_CM,   VOL_MAX_RANGE_CM,    VOL_PULSE_INTERVAL_MS,
    VOL_PULSE_ON_MS,      VOL_UP_KEY,          VOL_DOWN_KEY,
};

uint8_t reportedIrKey = 0;  // key the host currently sees from the IR slot
uint8_t reportedVolKey = 0; // key the host currently sees from the volume slot
bool hostReady = false;

bool readIr() { return digitalRead(IR_PIN) == IR_ACTIVE_STATE; }

// ---- Ultrasonic ranging -----------------------------------------------------
//
// A trigger pulse is started on a millis() cadence; the echo pulse width is
// timestamped by an interrupt on ULTRASONIC_ECHO_PIN using micros(), never by
// blocking on pulseIn(). loop() only starts pings and drains whichever result
// (echoed or timed out) is ready, so a slow or absent echo cannot stall the
// shared 1kHz HID loop.

volatile uint32_t echoRiseUs = 0;
volatile uint32_t echoDurationUs = 0;
volatile bool echoPending = false; // waiting for the falling edge
volatile bool echoReady = false;   // a full pulse width is available

void onEchoChange() {
  if (digitalRead(ULTRASONIC_ECHO_PIN)) {
    echoRiseUs = micros();
    echoPending = true;
  } else if (echoPending) {
    echoDurationUs = micros() - echoRiseUs;
    echoPending = false;
    echoReady = true;
  }
}

uint32_t lastPingAtMs = 0;
uint32_t pingSentAtUs = 0;
bool pingInFlight = false;

void startPing(uint32_t nowMs) {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  lastPingAtMs = nowMs;
  pingSentAtUs = micros();
  pingInFlight = true;
}

// Sets distanceCm and returns true once this ping's result (echoed or timed
// out) is ready; returns false while still waiting, without blocking.
bool pollDistance(uint16_t &distanceCm) {
  if (echoReady) {
    noInterrupts();
    const uint32_t durationUs = echoDurationUs;
    echoReady = false;
    interrupts();
    pingInFlight = false;
    // ~58us of round-trip travel per cm at the speed of sound.
    distanceCm = static_cast<uint16_t>(durationUs / 58);
    return true;
  }
  if (pingInFlight && (micros() - pingSentAtUs) > MAX_ECHO_US) {
    pingInFlight = false;
    distanceCm = DISTANCE_OUT_OF_RANGE;
    return true;
  }
  return false;
}

uint16_t lastDistanceCm = DISTANCE_OUT_OF_RANGE;

void setup() {
  USB.disconnect();
  USB.setManufacturer(USB_MANUFACTURER);
  USB.setProduct(USB_PRODUCT);
  USB.connect();

#if DEBUG_SERIAL
  Serial.begin(115200);
#endif

  pinMode(IR_PIN, IR_PIN_MODE);
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(ULTRASONIC_ECHO_PIN), onEchoChange,
                   CHANGE);

  const uint32_t now = millis();
  irSensor.reset(readIr(), now);
  ir.begin(IR_CONFIG, irSensor.stable, now);
  vol.begin(VOL_CONFIG, now);
  lastPingAtMs = now - PING_INTERVAL_MS; // ping immediately on first loop

  Keyboard.begin();
}

void loop() {
  const uint32_t now = millis();

  // Gesture recognition keeps running while the host is away so neither
  // state machine ever sees a time jump when USB comes back.
  const bool active = irSensor.update(readIr(), now, IR_DEBOUNCE_MS);

#if DEBUG_SERIAL
  const IrState irBefore = ir.state;
#endif
  ir.update(active, now);
#if DEBUG_SERIAL
  if (ir.state != irBefore) {
    Serial.printf("%lu ir active=%d state %d -> %d key=0x%02x\n",
                  (unsigned long)now, active, irBefore, ir.state, ir.key());
  }
#endif

  if (!pingInFlight && now - lastPingAtMs >= PING_INTERVAL_MS) {
    startPing(now);
  }
  uint16_t sample;
  if (pollDistance(sample)) {
    lastDistanceCm = sample;
  }
#if DEBUG_SERIAL
  const VolumeZone volBefore = vol.zone;
#endif
  vol.update(lastDistanceCm, now);
#if DEBUG_SERIAL
  if (vol.zone != volBefore) {
    Serial.printf("%lu vol distance=%u zone %d -> %d\n", (unsigned long)now,
                  lastDistanceCm, volBefore, vol.zone);
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
    reportedIrKey = 0;
    reportedVolKey = 0;
    hostReady = true;
  }

  syncKeySlot(Keyboard, reportedIrKey, ir.key());
  syncKeySlot(Keyboard, reportedVolKey, vol.key());
}
