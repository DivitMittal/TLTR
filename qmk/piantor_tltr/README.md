# Piantor TLTR - Custom Keyboard Variant

This is a custom QMK keyboard configuration for the TLTR layout, based on the [Piantor](https://github.com/beekeeb/piantor) keyboard by beekeeb.

## Hardware

- **Name**: Piantor TLTR
- **Microcontroller**: RP2040 (Raspberry Pi Pico)
- **Switches**: Cherry MX1A Red (linear)
- **Layout**: 42-key split (6×3 + 3 thumb keys per side)
- **Communication**: Serial USART full-duplex between halves
- **Bootloader**: RP2040 bootloader with double-tap reset

## Pin Configuration

### Split Communication

- **TX Pin**: GP0
- **RX Pin**: GP1
- **Mode**: Serial USART full-duplex

### Bootloader

- **Double-tap reset**: Enabled
- **Timeout**: 1000ms

## Keyboard Features

### Layers

1. **COLEMAK** (Base layer)
   - Colemak Mod-DH + Wide + Angle layout
   - Regular shift key (not one-shot)
   - Fork keys for context-sensitive behavior

2. **TL** (Left thumb layer)
   - Navigation keys (arrows, page up/down, tab)
   - Modifier keys (Alt, Ctrl, Shift, Meta)
   - One-shot modifier combinations (sWin, sHyp, sFn)

3. **TR** (Right thumb layer)
   - Numbers (0-9) with function key alternates (F1-F12)
   - Symbols and operators
   - Bracket pairs with shift-based switching

4. **TLTR** (Both thumbs layer)
   - Mouse movement and scrolling
   - Mouse buttons
   - Media controls (play/pause, volume, brightness)
   - Screen controls (lock, sleep)
   - System bootloader key

### Custom Features

#### Mouse Keys

- **Three speed modes**:
  - Default: 8 pixels per report (modestly fast)
  - Slow: 3 pixels per report
  - Precise: 1 pixel per report
- **Natural scrolling**: Reversed scroll directions
- **Constant speed**: No acceleration for predictable movement
- **Scroll mode**: Convert mouse movement to scroll wheel

#### Fork Keys

Keys that change behavior based on modifiers:

- `KC_DELF`: Delete when shifted, Backspace normally
- `KC_SLAF`: Backslash when shifted, Slash normally
- `KC_COMF`: Underscore when shifted, Comma normally
- `KC_DOTF`: Question when shifted, Period normally
- `KC_PGUF`: Home when shifted, Page Up normally
- `KC_PGDF`: End when shifted, Page Down normally
- Symbol forks: !/`, @/~, #/^, $/₹, &/|, etc.

#### One-Shot Modifiers

- **Tap**: Activates modifier for next key only
- **Hold**: Regular modifier behavior
- **Timeout**: 200ms to distinguish tap from hold
- **Custom combinations**:
  - `KC_OS_WIN`: Alt+Ctrl+Meta (window manager shortcuts)
  - `KC_OS_HYP`: Alt+Ctrl+Shift+Meta (hyper key)
  - `KC_OS_FN`: Function modifier for number→function key switching

#### OS Detection

Automatically detects the host OS on keyboard connection:

- **macOS/iOS**: Sets Unicode mode to macOS
- **Windows**: Sets Unicode mode to WinCompose
- **Linux**: Sets Unicode mode to Linux
- **Unknown**: Defaults to Linux mode

#### Caps Word

- **Trigger**: Double-tap shift or `CW_TOGG` key
- **Timeout**: 3000ms of inactivity
- **Behavior**: Capitalizes letters, allows numbers and navigation
- **Auto-release**: Deactivates on most punctuation keys

## Configuration Files

### config.h

Contains all keyboard-level configuration:

- Mouse key timing and speed settings
- Split keyboard communication pins
- Bootloader configuration
- Caps Word timeout
- One-shot modifier timeout (300ms)
- Tapping term (200ms)
- USB polling interval (1ms for low latency)

### keyboard.json

Defines the keyboard matrix and layout:

- Physical layout coordinates
- Matrix to key position mapping
- USB device identification
- Feature flags (mousekey, extrakey, bootmagic, etc.)

### keymaps/tltr/keymap.c

The complete TLTR keymap implementation:

- All four layer definitions
- Custom keycode implementations
- Mouse handling logic
- Fork key behavior
- One-shot modifier logic
- State tracking for layers and modifiers

## Building and Flashing

See the [QMK README](../README.md) for detailed build and flash instructions.

Quick commands:

```bash
# From qmk_firmware directory
qmk compile -kb beekeeb/piantor_tltr -km tltr

# Flash (after entering bootloader mode)
cp .build/beekeeb_piantor_tltr_tltr.uf2 /Volumes/RPI-RP2/
```

## Keymap Overview

```
┌─────────────────────────────────────────────┐
│           COLEMAK Base Layer                │
├─────┬───┬───┬───┬───┬───┬───┬───┬───┬───┬──┤
│ CW  │ Q │ W │ F │ P │ B │ J │ L │ U │ Y │' │
├─────┼───┼───┼───┼───┼───┼───┼───┼───┼───┼──┤
│ D/B │ A │ R │ S │ T │ G │ M │ N │ E │ I │O │
├─────┼───┼───┼───┼───┼───┼───┼───┼───┼───┼──┤
│     │ Z │ X │ C │ D │ V │ K │ H │,/_│./? │/ │
└─────┴───┴───┴───┼───┼───┼───┼───┼───┴───┴──┘
                  │TL │SFT│SPC│TR │
                  └───┴───┴───┴───┘
```

## Memory Optimizations

- 8-bit layer state (we only use 4 layers)
- Disabled unused features (music mode)
- USB polling interval optimized to 1ms

## Timing Configuration

All timing values match the Kanata implementation:

- **Tapping term**: 200ms
- **One-shot timeout**: 300ms
- **Caps Word timeout**: 3000ms
- **Mouse interval**: 16ms (60Hz update rate)

## Custom Keycodes Reference

| Keycode     | Tap       | Hold/Shift             |
| ----------- | --------- | ---------------------- |
| KC_DELF     | Backspace | Delete                 |
| KC_SLAF     | /         | \                      |
| KC_COMF     | ,         | \_                     |
| KC_DOTF     | .         | ?                      |
| KC_PGUF     | Page Up   | Home                   |
| KC_PGDF     | Page Down | End                    |
| KC_EXCF     | !         | `                      |
| KC_ATSF     | @         | ~                      |
| KC_OCTF     | #         | ^                      |
| KC_DOLF     | $         | ₹                      |
| KC_AMPF     | &         | \|                     |
| KC_CURF     | {         | }                      |
| KC_PARF     | (         | )                      |
| KC_SQRF     | [         | ]                      |
| KC_1F-KC_0F | Number    | Function key (with Fn) |
| KC_ASTF     | \*        | F11 (with Fn)          |
| KC_PERF     | %         | F12 (with Fn)          |

## Special Keys

| Key                  | Behavior                          |
| -------------------- | --------------------------------- |
| KC_MSLW              | Mouse slow mode (hold)            |
| KC_MPRE              | Mouse precise mode (hold)         |
| KC_MSCR              | Mouse scroll mode (hold)          |
| KC_MUP/MDN/MLFT/MRGT | Mouse/scroll directions           |
| KC_SCRE              | Tap: lock screen, Hold: sleep     |
| KC_MEDC              | Tap: play/pause, Hold: next track |

## Differences from Standard Piantor

This is a custom variant of the Piantor keyboard with:

1. Custom keymap optimized for the TLTR layout
2. Advanced mouse key implementation with speed modes
3. Extensive fork key system
4. Custom one-shot modifier combinations
5. OS detection and automatic Unicode configuration

## Support

For issues or questions:

- QMK general: [QMK Documentation](https://docs.qmk.fm/)
- TLTR layout: [Main Repository](../../README.md)
- Piantor hardware: [beekeeb/piantor](https://github.com/beekeeb/piantor)
