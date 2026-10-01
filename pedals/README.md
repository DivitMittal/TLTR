# Dual Foot Pedal Sketches

Optional USB foot pedals for hands-free control. Works as a standalone accessory.

![Dual foot pedal keymap](../assets/pedals.svg)

## Sketches

- **uno-dual-pedal/**: Arduino Uno-compatible serial sketch for capture daemons
- **rp2040-dual-pedal-hid/**: RP2040 native-USB HID sketch for direct keyboard integration (Waveshare RP2040 Zero/One)
- **rp2040-pedal-ir-hid/**: RP2040 native-USB HID sketch combining both pedals and both IR sensors, one unused function key per event
- **rp2040-ir-hid/**: RP2040 native-USB HID sketch for two digital IR proximity sensors (hold and swipe gestures)
- **tests/**: host-side tests for the shared RP2040 gesture code (`tests/run.sh`)

Both support compile-time tuning and validation, and both now include per-pedal gesture recognition:
- **Single hold**: press and hold (primary keycode)
- **Double-tap-and-hold**: quick press, release, press again while within the tap window (alternate keycode)

## Keymap

```
┌──────────────────┬──────────────────┐
│                  │                  │
│  Left Pedal (P1) │ Right Pedal (P2) │
│                  │                  │
├──────────────────┼──────────────────┤
│  Single Hold:    │  Single Hold:    │
│  F19             │  Ctrl+F13        │
│                  │                  │
│  Double-Tap:     │  Double-Tap:     │
│  Ctrl+F19        │  Ctrl+Shift+F13  │
│                  │                  │
└──────────────────┴──────────────────┘
```

## Choose a sketch

### Arduino Uno dual pedal

Serial output for capture daemons or custom listeners.

- **Default pins**: D2 (Pedal 1), D3 (Pedal 2)
- **Output**: Serial events at 115200 baud (host software maps these events to actions)
  - `EVT <sequence> P1 <timestamp>` — Pedal 1 primary hold
  - `EVT <sequence> P1_ALT <timestamp>` — Pedal 1 double-tap hold
  - `EVT <sequence> P1_UP <timestamp>` — Pedal 1 release
  - `EVT <sequence> P2 <timestamp>` — Pedal 2 primary hold
  - `EVT <sequence> P2_ALT <timestamp>` — Pedal 2 double-tap hold
  - `EVT <sequence> P2_UP <timestamp>` — Pedal 2 release
- **Best for**: Integration with `pedal-thoughtd` or custom serial handlers

See `uno-dual-pedal/README.md` for wiring, configuration, and flashing.

### RP2040 dual pedal HID

Native USB keyboard for direct OS integration—no daemon required.

- **Default board**: Waveshare RP2040 Zero / Waveshare RP2040 One
- **Default pins**: GP14 (Pedal 1), GP15 (Pedal 2)
- **Output**: HID keyboard chords
  - Single hold: F19 (P1), Ctrl+F13 (P2)
  - Double-tap: Ctrl+F19 (P1), Ctrl+Shift+F13 (P2)
- **Core**: `arduino-pico` (community-maintained RP2040 core by Earle F. Philhower, III)
- **Best for**: Standalone USB keyboard integration without daemon overhead

See `rp2040-dual-pedal-hid/README.md` for Waveshare setup, pin configuration, USB identity customization, and re-enumeration behavior.

### RP2040 IR HID

Two digital IR obstacle sensors as a native USB keyboard, for hands-free gestures.

- **Default board**: Waveshare RP2040-Zero
- **Default pins**: GP29 (IR left OUT), GP28 (IR right OUT)
- **Output**: one unused function key per gesture, no modifiers
  - Hold: F15 (left), F16 (right)
  - Swipe: F17 (left -> right), F18 (right -> left)
- **Best for**: tuning and testing the IR gesture state machine on its own

### RP2040 pedal + IR HID

The two pedals and the two IR sensors on one Waveshare RP2040-Zero, as a single native USB keyboard. Pedals are plain holds here (no double-tap chords), and every event has its own function key with no modifiers, so host remapping sees an unambiguous namespace.

- **Default board**: Waveshare RP2040-Zero
- **Default pins**: GP14 (Pedal 1), GP15 (Pedal 2), GP29 (IR left OUT), GP28 (IR right OUT)
- **Output**:
  - Pedal holds: F13 (P1), F14 (P2)
  - IR holds: F15 (left), F16 (right)
  - IR swipes: F17 (left -> right), F18 (right -> left)
- **Best for**: the full hands-and-feet controller; pedals and IR work concurrently

See `rp2040-pedal-ir-hid/README.md` for wiring, the timing model, sensor tuning, flashing, and verification.

See `rp2040-ir-hid/README.md` for wiring, the timing model, sensor tuning, and flashing.

## Build-time configuration

Both sketches validate configuration at compile time and support override via `arduino-cli --build-property compiler.cpp.extra_flags=`:

```bash
arduino-cli compile --board rp2040:rp2040:waveshare_rp2040_zero \
  --build-property compiler.cpp.extra_flags="-DPEDAL1_KEYCODE=72 -DPEDAL2_KEYCODE=73"
```

Common overrides:
- `PEDAL1_PIN`, `PEDAL2_PIN` — GPIO pin numbers
- `PEDAL1_KEYCODE`, `PEDAL2_KEYCODE` — Primary keycodes (1–255)
- `PEDAL1_ALT_KEYCODE`, `PEDAL2_ALT_KEYCODE` — Alternate keycodes (1–255)
- `PEDAL1_MODIFIERS`, `PEDAL2_MODIFIERS` — Primary chord modifiers
- `PEDAL1_ALT_MODIFIERS`, `PEDAL2_ALT_MODIFIERS` — Alternate chord modifiers
- `PEDAL_DEBOUNCE_MS` — Debounce window (default 25ms)
- `PEDAL_MULTI_TAP_WINDOW_MS` — Gesture window (default 150ms)
- `PEDAL_SEPARATE_KEYS` — Per-pedal mode or combined (default 1)

See the `.ino` files for the full list of compile-time parameters and validation rules.
