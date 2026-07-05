# RP2040 Dual Pedal HID Sketch

This sketch turns an RP2040 board with native USB device support into a dual-pedal HID keyboard for `pedal-thoughtd` workflows.

It is intended for Waveshare RP2040-class boards and uses the `arduino-pico` core's built-in USB HID keyboard support so the board can expose a low-latency keyboard interface over native USB without an extra library dependency.

For this sketch, prefer `arduino-pico` over the Arduino `mbed` RP2040 core. `arduino-pico` keeps HID on the Pico SDK USB path, ships a built-in `Keyboard` implementation for RP2040 boards, and matches the sketch's `1 ms` HID poll interval tuning without adding another HID stack layer.

The default `GP14` / `GP15` mapping is meant for the common Waveshare RP2040 Zero and Waveshare RP2040 One wiring path, while still staying override-friendly for other RP2040 boards.

The sketch sets the HID poll interval to `1 ms` to match the low-latency pedal use case while staying on the core's default Pico SDK USB stack.

Both the debounce window and HID poll interval can be overridden at compile time, which is useful if a specific pedal switch or RP2040 board needs different tuning.

## Wiring

The default mapping targets Waveshare RP2040 Zero and Waveshare RP2040 One boards using `GP14` and `GP15`.

- pedal 1 tip -> `GP14`
- pedal 2 tip -> `GP15`
- both sleeves -> `GND`

Official pin references:

- Waveshare RP2040 Zero: `https://www.waveshare.com/wiki/RP2040-Zero`
- Waveshare RP2040 One: `https://www.waveshare.com/wiki/RP2040-One`

The sketch uses `INPUT_PULLUP` by default, so each switch reads:

- not pressed -> `HIGH`
- pressed -> `LOW`

By default, each pedal reports its own HID chord. Pedal 1 uses the left-pedal chord pair (`F19`, `Ctrl+F19`), and pedal 2 uses the right-pedal chord pair (`Ctrl+F13`, `Ctrl+Shift+F13`).

Override the default pins at compile time for another RP2040 board if needed:

```c
#define PEDAL1_PIN 2
#define PEDAL2_PIN 3
```

`PEDAL1_PIN` and `PEDAL2_PIN` must both stay in the `0` to `255` range.

The sketch rejects builds where `PEDAL1_PIN` and `PEDAL2_PIN` are set to the same pin, which helps catch broken dual-pedal override combinations before flashing.

Pin overrides outside the `0` to `255` range are rejected at compile time so a bad board-specific pin remap cannot overflow the sketch's `uint8_t` pin storage.

Override the debounce window if your pedals need more or less switch filtering:

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

## Default key

The default HID key is `F13`, which is usually unused by normal keyboards and can be rebound in host software.

Override it at compile time if needed:

```c
#define PEDAL_KEYCODE HID_KEY_F20
```

`PEDAL_KEYCODE` must stay in the `1` to `255` range. The sketch rejects `0` because that is the HID "no event" code and would otherwise make the pedal appear non-functional.

`PEDAL_KEYCODE` is only valid in the default merged-key mode; if `PEDAL_SEPARATE_KEYS=1` is enabled, the sketch rejects a `PEDAL_KEYCODE` override at compile time instead of silently ignoring it.

Override the HID poll interval if needed:

```c
#define PEDAL_HID_POLL_INTERVAL_MS 2
```

The HID poll interval override must stay in the `1` to `255` range; zero, negative, or larger values are rejected at compile time.

Enable the generic Waveshare preset if you want the sketch to keep the default `GP14` / `GP15` wiring and also advertise Waveshare-oriented USB descriptor strings without defining each string separately:

```c
#define PEDAL_BOARD_WAVESHARE_RP2040 1
```

The Waveshare preset macros must be set to `0` or `1`, and the sketch still rejects enabling both board-specific presets at the same time.

That preset fills in:

- `PEDAL_USB_MANUFACTURER` as `"Waveshare"`
- `PEDAL_USB_PRODUCT` as `"Waveshare RP2040 Dual Pedal HID"`

If you want the USB product string to name the exact Waveshare board without setting it manually, use one of the board-specific presets instead:

```c
#define PEDAL_BOARD_WAVESHARE_RP2040_ZERO 1
```

```c
#define PEDAL_BOARD_WAVESHARE_RP2040_ONE 1
```

Those board-specific presets still imply the generic Waveshare preset and fill in:

- `PEDAL_USB_MANUFACTURER` as `"Waveshare"`
- `PEDAL_USB_PRODUCT` as `"Waveshare RP2040 Zero Dual Pedal HID"` or `"Waveshare RP2040 One Dual Pedal HID"`

Override the USB descriptor strings directly if you want a different product name on the host:

```c
#define PEDAL_USB_MANUFACTURER "Waveshare"
#define PEDAL_USB_PRODUCT "Dual Pedal HID"
```

`PEDAL_USB_MANUFACTURER` and `PEDAL_USB_PRODUCT` must not be empty strings; the sketch rejects empty USB descriptor overrides at compile time.

Those overrides use the `arduino-pico` core's `USB.h` API and cause the sketch to disconnect and reconnect USB once during `setup()` so the host picks up the new descriptor strings.

## Optional separate-key mode

If you want the two pedals to act independently on the host, enable separate-key mode at compile time:

```c
#define PEDAL_SEPARATE_KEYS 1
```

`PEDAL_SEPARATE_KEYS` must be set to `0` or `1`; other values are rejected at compile time.

In that mode, pedal 1 defaults to `F19` with no modifiers and `Ctrl+F19` on double-tap-and-hold. Pedal 2 defaults to `Ctrl+F13` and `Ctrl+Shift+F13` on double-tap-and-hold.

Override those defaults if needed:

```c
#define PEDAL1_KEYCODE HID_KEY_F15
#define PEDAL1_ALT_KEYCODE HID_KEY_F15
#define PEDAL1_MODIFIERS 0
#define PEDAL1_ALT_MODIFIERS KEY_LEFT_CTRL
#define PEDAL2_KEYCODE HID_KEY_F16
#define PEDAL2_ALT_KEYCODE HID_KEY_F16
#define PEDAL2_MODIFIERS KEY_LEFT_CTRL
#define PEDAL2_ALT_MODIFIERS KEY_LEFT_CTRL, KEY_LEFT_SHIFT
```

`PEDAL1_KEYCODE`, `PEDAL2_KEYCODE`, `PEDAL1_ALT_KEYCODE`, and `PEDAL2_ALT_KEYCODE` must stay in the `1` to `255` range. Primary and alternate gestures may reuse the same keycode when their modifier lists differ. Those per-pedal overrides are only valid when `PEDAL_SEPARATE_KEYS=1`; otherwise the sketch rejects them instead of silently staying in merged-key mode.

Install the supported `arduino-pico` core for `arduino-cli` with the same package index the Arduino IDE uses. If you want that third-party index to persist for future `arduino-cli` commands, add it to the CLI config once first:

```bash
arduino-cli config add board_manager.additional_urls \
  https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
```

You can then refresh the package index and install the core:

```bash
arduino-cli core update-index \
  --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

arduino-cli core install rp2040:rp2040 \
  --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
```

If you use the persistent config entry above, later `arduino-cli core update-index` and `arduino-cli core install rp2040:rp2040` commands do not need `--additional-urls` again.

You can then pass pin, debounce, polarity, mode, key, and Waveshare preset overrides from `arduino-cli` without editing the sketch. With the current `arduino-pico` boards package, use the explicit Waveshare board IDs instead of the generic Pico target:

```bash
arduino-cli compile \
  --fqbn rp2040:rp2040:waveshare_rp2040_zero \
  --build-property compiler.cpp.extra_flags="-DPEDAL_BOARD_WAVESHARE_RP2040_ZERO=1 -DPEDAL1_PIN=2 -DPEDAL2_PIN=3 -DPEDAL_DEBOUNCE_MS=15 -DPEDAL_PIN_MODE=INPUT -DPEDAL_PRESSED_STATE=HIGH -DPEDAL_SEPARATE_KEYS=1 -DPEDAL1_KEYCODE=HID_KEY_F15 -DPEDAL2_KEYCODE=HID_KEY_F16 -DPEDAL_USB_PRODUCT='\"Dual Pedal HID\"'" \
  arduino/rp2040-dual-pedal-hid
```

In that example, the Waveshare Zero preset provides the manufacturer string and the example still overrides the product string explicitly.

For a Waveshare RP2040 One, switch both the board target and the board preset macro to match:

```bash
--fqbn rp2040:rp2040:waveshare_rp2040_one
```

```text
-DPEDAL_BOARD_WAVESHARE_RP2040_ONE=1
```

Do not keep `PEDAL_BOARD_WAVESHARE_RP2040_ZERO=1` when targeting a Waveshare RP2040 One, or the sketch will still advertise the Zero-oriented default USB product string.

## Arduino IDE setup

1. Add `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json` to `Preferences -> Additional boards manager URLs`.
1. Install `Raspberry Pi RP2040/RP2350` by Earle F. Philhower, III in Boards Manager.
1. Do not use the Arduino `mbed` RP2040 boards package for this sketch; the intended path is `arduino-pico` so the built-in `Keyboard` HID support and Pico SDK USB stack stay available.
1. Open `rp2040-dual-pedal-hid.ino` in the Arduino IDE.
1. Select the matching Waveshare board target in `Tools -> Board`, such as `Waveshare RP2040 Zero` or `Waveshare RP2040 One`.
1. For the first upload, hold `BOOT` while plugging the board in, or on Waveshare boards press `RESET` first and then `BOOT`, release `RESET`, and then release `BOOT`, so the ROM bootloader appears as `RPI-RP2` and Arduino IDE can target `Tools -> Port -> UF2 Board`.
1. Leave `Tools -> USB Stack` on the default Pico SDK option so the built-in `Keyboard` library stays available.
1. Upload the sketch.

If the board is not detected after switching away from a previous TinyUSB-based sketch, enter the RP2040 bootloader once with `BOOT` plus `RESET` and upload again so the host re-enumerates it with the new USB stack. If Arduino IDE still cannot hand the sketch off automatically, export or locate the compiled `.uf2` and copy it to the `RPI-RP2` drive manually; Waveshare documents that the ROM bootloader accepts drag-and-drop UF2 flashing on these boards.

## Verification

1. Open a key event viewer on the host.
1. Hold a pedal while plugging in or resetting the board and verify the key appears once USB enumeration completes.
1. While holding a pedal, trigger a USB re-enumeration event such as unplugging and reconnecting the USB cable or resetting the board, and verify the held key is replayed once the host sees the board again.
1. In the default separate-key mode, hold pedal 1 and verify `F19` is held.
1. In the default separate-key mode, double-tap-and-hold pedal 1 and verify `Ctrl+F19` is held.
1. In the default separate-key mode, hold pedal 2 and verify `Ctrl+F13` is held.
1. In the default separate-key mode, double-tap-and-hold pedal 2 and verify `Ctrl+Shift+F13` is held.
1. Release each gesture and verify all function keys and modifiers are released.
