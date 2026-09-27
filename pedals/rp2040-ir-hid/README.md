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
