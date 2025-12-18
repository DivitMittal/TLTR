# QMK Firmware for TLTR Layout

This directory contains the QMK firmware implementation of the TLTR keyboard layout for the Piantor keyboard.

## Overview

The QMK implementation provides hardware-level key remapping and advanced features that work independently of the operating system. This is ideal for users who want their layout to work consistently across different machines or operating systems without installing software.

## Target Keyboard

- **Keyboard**: Modified [Piantor](https://github.com/beekeeb/piantor) by beekeeb
- **Microcontroller**: RP2040 (Raspberry Pi Pico)
- **Layout**: 38-key split keyboard
- **Firmware directory**: `piantor_tltr/`

## Directory Structure

```
qmk/
└── piantor_tltr/          # Custom keyboard variant for TLTR
    ├── config.h           # Keyboard configuration
    ├── keyboard.json      # Keyboard metadata and layout
    ├── README.md          # Keyboard-specific documentation
    └── keymaps/
        └── tltr/
            └── keymap.c   # TLTR keymap implementation
```

## Building the Firmware

### Prerequisites

1. QMK CLI installed and set up:

   ```bash
   uv tool install qmk
   qmk setup
   ```

2. Symlink the custom keyboard to QMK firmware:
   ```bash
   ln -s /path/to/TLTR/qmk/piantor_tltr keyboards/beekeeb/piantor_tltr
   ```

### Compilation

```bash
qmk compile -kb beekeeb/piantor_tltr -km tltr
```

Or using make (from the `qmk_firmware` directory):

```bash
make beekeeb/piantor_tltr:tltr
```

The compiled firmware will be at:

```
.build/beekeeb_piantor_tltr_tltr.uf2
```

### Flashing

1. Put the keyboard into bootloader mode:
   - Press the BOOT button while plugging in the USB cable, OR
   - Use the `QK_BOOT` key on the TLTR layer

2. The keyboard will appear as a USB mass storage device

3. Copy the `.uf2` file to the mounted drive:

   ```bash
   cp .build/beekeeb_piantor_tltr_tltr.uf2 /Volumes/RPI-RP2/
   ```

4. The keyboard will automatically reboot with the new firmware
