# Arduino Uno Dual Pedal Sketch

This sketch turns an Arduino Uno-compatible board into a simple serial event source for `pedal-thoughtd`.
It supports a dual-pedal wiring setup while preserving the existing serial protocol.

## Wiring

- pedal 1 tip -> Arduino `D2`
- pedal 2 tip -> Arduino `D3`
- both sleeves -> Arduino `GND`

The sketch uses `INPUT_PULLUP` by default, so each switch reads:

- not pressed -> `HIGH`
- pressed -> `LOW`

Either pedal counts as an active press. The sketch emits `DOWN` when the first pedal is pressed and `UP` only after both pedals are released.

Override the default pins at compile time for another Uno-compatible wiring layout if needed:

```c
#define PEDAL1_PIN 4
#define PEDAL2_PIN 5
```

`PEDAL1_PIN` and `PEDAL2_PIN` must both stay in the `0` to `255` range.

Override the debounce window if your switches need different filtering:

```c
#define PEDAL_DEBOUNCE_MS 15
```

Negative debounce values are rejected at compile time so a bad override cannot silently wrap into a huge delay.

If your pedal wiring uses a different input mode or active level, override those at compile time too:

```c
#define PEDAL_PIN_MODE INPUT
#define PEDAL_PRESSED_STATE HIGH
```

`PEDAL_PIN_MODE` must be `INPUT` or `INPUT_PULLUP`, and `PEDAL_PRESSED_STATE` must be `LOW` or `HIGH`; other values are rejected at compile time.

You can pass those overrides from `arduino-cli` without editing the sketch:

```bash
arduino-cli compile \
  --fqbn arduino:avr:uno \
  --build-property compiler.cpp.extra_flags="-DPEDAL1_PIN=4 -DPEDAL2_PIN=5 -DPEDAL_DEBOUNCE_MS=15 -DPEDAL_PIN_MODE=INPUT -DPEDAL_PRESSED_STATE=HIGH -DENHANCED_PROTOCOL=1" \
  arduino/uno-dual-pedal
```

The sketch now rejects builds where `PEDAL1_PIN` or `PEDAL2_PIN` falls outside the `0` to `255` range or where both are set to the same pin, which helps catch broken dual-pedal override combinations before flashing.

## Protocol

Default output at `115200` baud:

```text
DOWN
UP
```

Optional enhanced output:

```text
EVT <sequence> <DOWN|UP> <millis>
```

Enable it by compiling with:

```c
#define ENHANCED_PROTOCOL 1
```

`ENHANCED_PROTOCOL` must be set to `0` or `1`; other values are rejected at compile time.

## Flashing

1. Open `uno-dual-pedal.ino` in the Arduino IDE.
1. Select the correct Arduino Uno board and CH340-backed serial port.
1. Upload the sketch.
1. Open Serial Monitor at `115200` baud and verify `DOWN` when either pedal is pressed and `UP` after both are released.
