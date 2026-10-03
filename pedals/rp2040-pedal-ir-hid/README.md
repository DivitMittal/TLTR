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

## Timing model

| Setting                      | Default | Meaning                                                                |
| ---------------------------- | ------- | ---------------------------------------------------------------------- |
| `PEDAL_DEBOUNCE_MS`          | 20      | Pedal contact must be stable this long before it counts                |
| `IR_DEBOUNCE_MS`             | 10      | Raw IR OUT must be stable this long before the sensor state changes    |
| `IR_HOLD_THRESHOLD_MS`       | 300     | A lone sensor must stay active this long to become a hold              |
| `IR_SWIPE_WINDOW_MS`         | 250     | Maximum onset gap between first and second sensor for a swipe          |
| `IR_MIN_SWIPE_SEPARATION_MS` | 20      | Onset gaps shorter than this have no clear direction and are discarded |
| `IR_REARM_MS`                | 150     | Both sensors must stay clear this long after a gesture before the next |
| `IR_SWIPE_TAP_MS`            | 20      | How long a swipe key is held down                                      |

Everything is non-blocking: `loop()` samples all four inputs on every pass using `millis()` timestamps and small state machines, with no `delay()`.

- **Pedal debounce**: a pedal key goes down `PEDAL_DEBOUNCE_MS` after the contact settles, and up `PEDAL_DEBOUNCE_MS` after release. There are no pedal gestures in this firmware.
- **IR debounce**: LM393-style comparators chatter when an object sits near the threshold. Each sensor is debounced independently with the same delay, so the order in which they fire is preserved.
- **Pending**: when one IR sensor fires, nothing is sent yet, because it may be the start of a swipe.
- **Swipe**: if the other sensor fires between `IR_MIN_SWIPE_SEPARATION_MS` and `IR_SWIPE_WINDOW_MS` after the first one, that is a swipe in that direction and one key tap is sent. The first sensor may already have cleared (a hand passing over both). No hold key is sent for the same movement.
- **Hold**: if the first sensor stays continuously active for `IR_HOLD_THRESHOLD_MS` with no second sensor, it becomes a hold. Once committed, the other sensor is ignored until the hold ends; a hold never turns into a swipe.
- **Brief activations**: a single sensor that clears before the hold threshold, with no second sensor inside the swipe window, emits nothing. That filters hands just passing nearby.
- **Ambiguous**: both sensors firing within `IR_MIN_SWIPE_SEPARATION_MS` (a hand approaching head-on), or the second sensor arriving after the swipe window has expired, emits nothing. There is no "both sensors" gesture.
- **Re-arm**: after any swipe, hold or ambiguous gesture, both sensors must be clear for `IR_REARM_MS` before a new gesture is accepted. One hand movement therefore produces at most one IR event, even if the hand lingers. Sensors already active at power-up are treated the same way.

`IR_SWIPE_WINDOW_MS` must not exceed `IR_HOLD_THRESHOLD_MS`: a pending sensor turns into a hold at the threshold, after which it can no longer become a swipe. The trade-off is that IR holds are reported `IR_HOLD_THRESHOLD_MS + IR_DEBOUNCE_MS` (310 ms) after the hand arrives. Lower the threshold for snappier holds at the cost of a shorter swipe window.

The IR state machine is documented at the top of `ir_gesture.h`.

## USB behaviour

- Enumerates as `TLTR` / `TLTR Pedal + IR HID`, using the `arduino-pico` built-in `Keyboard` library on the default Pico SDK USB stack.
- HID poll interval is 1 ms (`HID_POLL_INTERVAL_MS`), matching the pedal-only sketch.
- Input processing continues while the host is unplugged, asleep or re-enumerating. When the host comes back the firmware clears its HID report once and replays whatever pedal or IR hold is still active, so keys neither stick nor get lost. A swipe tap that completes while the host is away is dropped.

## Configuration

All settings live in `config.h` and can be overridden without editing files:

```bash
arduino-cli compile --fqbn rp2040:rp2040:waveshare_rp2040_zero \
  --build-property compiler.cpp.extra_flags="-DIR_ACTIVE_STATE=HIGH -DPEDAL_DEBOUNCE_MS=25" \
  pedals/rp2040-pedal-ir-hid
```

| Setting                                  | Default                 |
| ---------------------------------------- | ----------------------- |
| `PEDAL1_PIN` / `PEDAL2_PIN`              | `14` / `15`             |
| `IR_LEFT_PIN` / `IR_RIGHT_PIN`           | `29` / `28`             |
| `PEDAL_PIN_MODE`                         | `INPUT_PULLUP`          |
| `IR_PIN_MODE`                            | `INPUT`                 |
| `PEDAL_ACTIVE_STATE`                     | `LOW`                   |
| `IR_ACTIVE_STATE`                        | `LOW`                   |
| `PEDAL1_KEY` / `PEDAL2_KEY`              | `KEY_F13` / `KEY_F14`   |
| `IR_LEFT_HOLD_KEY` / `IR_RIGHT_HOLD_KEY` | `KEY_F15` / `KEY_F16`   |
| `IR_SWIPE_LR_KEY` / `IR_SWIPE_RL_KEY`    | `KEY_F17` / `KEY_F18`   |
| `HID_POLL_INTERVAL_MS`                   | `1`                     |
| `USB_MANUFACTURER`                       | `"TLTR"`                |
| `USB_PRODUCT`                            | `"TLTR Pedal + IR HID"` |
| `DEBUG_SERIAL`                           | `0`                     |

The build fails with a `static_assert` if any two of the four input pins collide or fall outside GPIO 0-29, if two events share a key, if a key is `0` or a modifier, if an active state is not `LOW`/`HIGH`, if a pin mode is not an input mode, or if the timing values contradict each other.

`DEBUG_SERIAL=1` prints IR state changes over the USB serial port. HID works the same with or without it.

## Sensor tuning

Each IR module's potentiometer sets its detection distance (the comparator threshold). Most modules also have an LED that lights while an object is detected.

The exact module model is unverified, so the output polarity is a setting rather than an assumption:

1. Build and flash with the default `IR_ACTIVE_STATE=LOW`.
1. With nothing in front of the sensors, turn each potentiometer until its LED is reliably off, then back off a little from the point where it starts flickering. A flickering idle sensor will block re-arming.
1. Hold a hand at the intended distance and check the LED turns on cleanly.
1. Open a key viewer on the host (see below). Hold a hand over one sensor for about half a second: `F15`/`F16` should go down, and come back up when the hand leaves.
1. If instead a key is held while nothing is in front of the sensor (or nothing ever fires), the output polarity is inverted: rebuild with `-DIR_ACTIVE_STATE=HIGH`. No rewiring is needed.

## Build

Install `arduino-cli` (`brew install arduino-cli`, or `nix shell nixpkgs#arduino-cli`) and the community-maintained [`arduino-pico`](https://github.com/earlephilhower/arduino-pico) core by Earle F. Philhower, III:

```bash
arduino-cli config add board_manager.additional_urls \
  https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
arduino-cli core update-index
arduino-cli core install rp2040:rp2040
```

Build from the repository root, writing the UF2 outside the sketch:

```bash
arduino-cli compile --fqbn rp2040:rp2040:waveshare_rp2040_zero \
  --output-dir /tmp/tltr-pedal-ir-hid pedals/rp2040-pedal-ir-hid
```

This produces `/tmp/tltr-pedal-ir-hid/rp2040-pedal-ir-hid.ino.uf2`. Leave the core's USB stack on the default Pico SDK option; the built-in `Keyboard` library depends on it.

## Flash

1. Put the RP2040-Zero into the ROM bootloader: hold `BOOT` while plugging in USB, or with USB connected hold `BOOT`, press and release `RESET`, then release `BOOT`.
1. A drive named `RPI-RP2` appears.
1. Copy the UF2 onto it:

   ```bash
   cp /tmp/tltr-pedal-ir-hid/rp2040-pedal-ir-hid.ino.uf2 /Volumes/RPI-RP2/
   ```

1. The board reboots and enumerates as `TLTR Pedal + IR HID`.

To go back to the pedal-only firmware, flash `../rp2040-dual-pedal-hid/` the same way.

## Verify on macOS

1. Open Karabiner-EventViewer (ships with Karabiner-Elements; allow Input Monitoring if asked). Any other key event viewer works too.
1. Check System Information -> USB lists `TLTR Pedal + IR HID`. macOS may open the Keyboard Setup Assistant for the new keyboard; it can be closed.
1. Pedals: press and hold pedal 1 -> `f13` down, release -> `f13` up. Pedal 2 -> `f14`. Hold both -> both down together.
1. IR holds: hold over the left sensor -> `f15` down after about 0.3 s, up when the hand leaves. Right -> `f16`.
1. IR swipes: sweep left to right -> exactly one `f17` down/up pair, no `f15`/`f16`. Right to left -> `f18`.
1. Ambiguous: bring a flat hand straight down onto both sensors at once -> nothing.
1. Concurrency: hold pedal 1 and swipe -> `f13` stays down throughout while `f17` taps. Hold pedal 2 and hold over the right sensor -> `f14` and `f16` both held.
1. Re-enumeration: sleep and wake the Mac, and unplug/replug the board, with and without a pedal held. No key should stay stuck on the host; a pedal still physically down after wake is reported as held again and releases normally.

## Host tests

The gesture logic in `ir_gesture.h` and `inputs.h` has no Arduino dependencies. `../tests/run.sh` builds `../tests/gesture_tests.cpp` with the host compiler and replays pedal, IR and concurrency scenarios through the same debounce -> gesture -> key-slot pipeline this sketch uses. It also checks that the shared headers are identical across sketches.
