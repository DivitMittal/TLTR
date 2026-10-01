# RP2040 Pedal + IR HID Sketch

Two mechanical foot pedals and two digital IR proximity/obstacle sensors on a Waveshare RP2040-Zero, exposed as a single native USB HID keyboard.

The firmware only recognises physical events and emits one otherwise-unused function key per event. It never sends shortcuts, media keys or modifiers; host software (Kanata, Karabiner-Elements, ...) decides what each key means. No serial daemon is involved.

Related sketches:

- `../rp2040-ir-hid/`: the same IR gesture code without pedals, for tuning sensors in isolation.
- `../rp2040-dual-pedal-hid/`: the earlier pedal-only firmware with tap/double-tap chords, kept as a fallback and regression reference.

## Wiring

| Function      | RP2040-Zero pin |
| ------------- | --------------- |
| Pedal 1       | GP14            |
| Pedal 2       | GP15            |
| IR left OUT   | GP29            |
| IR right OUT  | GP28            |
| IR VCC (both) | 3V3             |
| IR GND (both) | GND             |
| Pedal returns | GND             |

```text
GP14 ---- pedal 1 switch ---- GND
GP15 ---- pedal 2 switch ---- GND
```

- **Pedals** use the RP2040's internal pull-up (`INPUT_PULLUP`), so no external resistors are needed. Set each pedal's polarity switch so that **released = open** and **pressed = short to GND**. If a pedal's switch is flipped the other way, either flip it back or rebuild with `-DPEDAL_ACTIVE_STATE=HIGH` (this applies to both pedals).
- **IR sensors** are read as plain `INPUT`: the modules drive OUT themselves through their comparator and on-board pull-up, so no internal pull is enabled.
- GP28 and GP29 are ordinary exposed GPIOs on the RP2040-Zero. (On a Raspberry Pi Pico GP29 is wired to VSYS sensing, which is why some docs call it unavailable; that does not apply here.)

## HID map

| Event                  | Key   | Behaviour                             |
| ---------------------- | ----- | ------------------------------------- |
| Pedal 1 hold           | `F13` | held while the pedal is down          |
| Pedal 2 hold           | `F14` | held while the pedal is down          |
| IR left hold           | `F15` | held while the left sensor is active  |
| IR right hold          | `F16` | held while the right sensor is active |
| IR left -> right swipe | `F17` | single tap                            |
| IR right -> left swipe | `F18` | single tap                            |

Pedals and IR are independent. Each input owns its own key and only ever releases that key, so for example `F13` stays held while an IR swipe taps `F17`. The firmware never uses `releaseAll()` to end a gesture.
