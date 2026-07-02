# Dual Foot Pedal Sketches

Optional USB foot pedals for hands-free control. Works as a standalone accessory.

![Pedal image placeholder](../assets/pedals-placeholder.png)

## Sketches

- **uno-dual-pedal/**: Arduino Uno-compatible serial sketch for capture daemons
- **rp2040-dual-pedal-hid/**: RP2040 native-USB HID sketch for direct keyboard integration (Waveshare RP2040 Zero/One)

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
│  Ctrl+F17        │  Ctrl+F18        │
│                  │                  │
│  Double-Tap:     │  Double-Tap:     │
│  Ctrl+F19        │  Ctrl+F20        │
│                  │                  │
└──────────────────┴──────────────────┘
```

## Choose a sketch

### Arduino Uno dual pedal

Serial output for capture daemons or custom listeners.

- **Default pins**: D2 (Pedal 1), D3 (Pedal 2)
- **Output**: Serial events at 115200 baud
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
- **Output**: HID keyboard with Ctrl modifier
  - Single hold: Ctrl+F17 (P1), Ctrl+F18 (P2)
  - Double-tap: Ctrl+F19 (P1), Ctrl+F20 (P2)
- **Core**: `arduino-pico` (official RP2040 support)
- **Best for**: Standalone USB keyboard integration without daemon overhead

See `rp2040-dual-pedal-hid/README.md` for Waveshare setup, pin configuration, USB identity customization, and re-enumeration behavior.

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
- `PEDAL_DEBOUNCE_MS` — Debounce window (default 25ms)
- `PEDAL_MULTI_TAP_WINDOW_MS` — Gesture window (default 250ms)
- `PEDAL_SEPARATE_KEYS` — Per-pedal mode or combined (default 1)

See the `.ino` files for the full list of compile-time parameters and validation rules.
