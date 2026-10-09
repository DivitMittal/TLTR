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

Each half gets its own build. The left half is hardcoded as master, so it is
the one that must be plugged into USB.

```bash
qmk compile -kb beekeeb/piantor_tltr -km tltr -e TLTR_HALF=left  -e TARGET=beekeeb_piantor_tltr_tltr_left
qmk compile -kb beekeeb/piantor_tltr -km tltr -e TLTR_HALF=right -e TARGET=beekeeb_piantor_tltr_tltr_right
```

Or using make (from the `qmk_firmware` directory):

```bash
make beekeeb/piantor_tltr:tltr TLTR_HALF=left  TARGET=beekeeb_piantor_tltr_tltr_left
make beekeeb/piantor_tltr:tltr TLTR_HALF=right TARGET=beekeeb_piantor_tltr_tltr_right
```

The compiled firmware will be at:

```
.build/beekeeb_piantor_tltr_tltr_left.uf2
.build/beekeeb_piantor_tltr_tltr_right.uf2
```

### Flashing

Flash each half separately, with only that half connected over USB:

1. Put the half into bootloader mode:
   - Press the BOOT button while plugging in the USB cable, OR
   - Use the `QK_BOOT` key on the TLTR layer (only reaches the half that is plugged in)

2. The half will appear as a USB mass storage device

3. Copy the matching `.uf2` file to the mounted drive:

   ```bash
   cp .build/beekeeb_piantor_tltr_tltr_left.uf2 /Volumes/RPI-RP2/   # left half
   cp .build/beekeeb_piantor_tltr_tltr_right.uf2 /Volumes/RPI-RP2/  # right half
   ```

4. The half will automatically reboot with the new firmware
