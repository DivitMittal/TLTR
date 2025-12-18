# Piantor TLTR - Custom Keyboard Variant

This is a custom QMK keyboard configuration for the TLTR layout, based on the [Piantor](https://github.com/beekeeb/piantor) keyboard by beekeeb.

## Hardware

- **Name**: Piantor TLTR
- **Microcontroller**: RP2040 (Raspberry Pi Pico)
- **Switches**: Cherry MX1A Red (linear)
- **Layout**: 38-key split (6×2 + 5x1 + 2 thumb keys per side)
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
