# RP2040 IR HID Sketch

Two digital IR proximity/obstacle sensors on a Waveshare RP2040-Zero, exposed as a native USB HID keyboard.

The firmware only classifies physical gestures and emits otherwise-unused function keys. It never sends shortcuts, media keys or modifiers; host software (Kanata, Karabiner-Elements, ...) decides what each key means.

This is the IR-only variant, useful for tuning and testing the IR gesture state machine on its own. `../rp2040-pedal-ir-hid/` (on the `feature/rp2040-pedals-ir-hid` branch) is the combined pedal + IR firmware and shares the same gesture code.

## Wiring

| Function      | RP2040-Zero pin |
| ------------- | --------------- |
| IR left OUT   | GP29            |
| IR right OUT  | GP28            |
| IR VCC (both) | 3V3             |
| IR GND (both) | GND             |

GP28 and GP29 are ordinary exposed GPIOs on the RP2040-Zero (on a Raspberry Pi Pico GP29 is wired to VSYS sensing, which is why some docs call it unavailable; that does not apply here). The pins are configured as plain `INPUT` because the modules drive OUT themselves through their comparator and on-board pull-up.

## HID map

| Gesture                | Key   |
| ---------------------- | ----- |
| IR left hold           | `F15` |
| IR right hold          | `F16` |
| IR left -> right swipe | `F17` |
| IR right -> left swipe | `F18` |

Holds keep their key down for as long as the sensor stays active. Swipes are a single tap (key down, then key up `IR_SWIPE_TAP_MS` later).

## Timing model

| Setting                      | Default | Meaning                                                                |
| ---------------------------- | ------- | ---------------------------------------------------------------------- |
| `IR_DEBOUNCE_MS`             | 10      | Raw OUT must be stable this long before the sensor state changes       |
| `IR_HOLD_THRESHOLD_MS`       | 300     | A lone sensor must stay active this long to become a hold              |
| `IR_SWIPE_WINDOW_MS`         | 250     | Maximum onset gap between first and second sensor for a swipe          |
| `IR_MIN_SWIPE_SEPARATION_MS` | 20      | Onset gaps shorter than this have no clear direction and are discarded |
| `IR_REARM_MS`                | 150     | Both sensors must stay clear this long after a gesture before the next |
| `IR_SWIPE_TAP_MS`            | 20      | How long a swipe key is held down                                      |

- **Debounce**: LM393-style comparators chatter when an object sits near the threshold. Each sensor is debounced independently with the same delay, so the order in which they fire is preserved.
- **Pending**: when one sensor fires, nothing is sent yet, because it may be the start of a swipe.
- **Swipe**: if the other sensor fires between `IR_MIN_SWIPE_SEPARATION_MS` and `IR_SWIPE_WINDOW_MS` after the first one, that is a swipe in that direction. The first sensor may already have cleared (a hand passing over both).
- **Hold**: if the first sensor stays continuously active for `IR_HOLD_THRESHOLD_MS` with no second sensor, it becomes a hold. Once committed, the other sensor is ignored until the hold ends.
- **Brief activations**: a single sensor that clears before the hold threshold, with no second sensor inside the swipe window, emits nothing. That filters hands just passing nearby.
- **Ambiguous**: both sensors firing within `IR_MIN_SWIPE_SEPARATION_MS` (a hand approaching head-on), or the second sensor arriving after the swipe window, emits nothing.
- **Re-arm**: after any swipe, hold or ambiguous gesture, both sensors must be clear for `IR_REARM_MS` before a new gesture is accepted. One hand movement therefore produces at most one event, even if the hand lingers in front of the sensors. Sensors already active at power-up are treated the same way.

`IR_SWIPE_WINDOW_MS` must not exceed `IR_HOLD_THRESHOLD_MS`: a pending sensor turns into a hold at the threshold, after which it can no longer become a swipe. The trade-off is that holds are reported `IR_HOLD_THRESHOLD_MS + IR_DEBOUNCE_MS` after the hand arrives.

The state machine is documented at the top of `ir_gesture.h`.

## Configuration

All settings live in `config.h` and can be overridden without editing files:

```bash
arduino-cli compile --fqbn rp2040:rp2040:waveshare_rp2040_zero \
  --build-property compiler.cpp.extra_flags="-DIR_ACTIVE_STATE=HIGH -DIR_HOLD_THRESHOLD_MS=400" \
  pedals/rp2040-ir-hid
```

| Setting                        | Default                 |
| ------------------------------ | ----------------------- |
| `IR_LEFT_PIN` / `IR_RIGHT_PIN` | `29` / `28`             |
| `IR_PIN_MODE`                  | `INPUT`                 |
| `IR_ACTIVE_STATE`              | `LOW`                   |
| `IR_LEFT_HOLD_KEY` ...         | `KEY_F15` ... `KEY_F18` |
| `HID_POLL_INTERVAL_MS`         | `1`                     |
| `USB_MANUFACTURER`             | `"TLTR"`                |
| `USB_PRODUCT`                  | `"TLTR IR HID"`         |
| `DEBUG_SERIAL`                 | `0`                     |

The build fails with a `static_assert` if pins collide or fall outside GPIO 0-29, if two events share a key, if a key is `0` or a modifier, if an active state is not `LOW`/`HIGH`, or if the timing values contradict each other.

`DEBUG_SERIAL=1` prints gesture state changes over the USB serial port. HID works the same with or without it.
