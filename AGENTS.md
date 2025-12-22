# CLAUDE.md

## Project Overview

TLTR is a bespoke cross-platform multi-layer 38-key keyboard layout optimized for programmers. It implements:

- A Colemak Mod-DH(Curl), Wide, Angle base layer for English typing
- TL layer for navigation and modifier keys
- TR layer for numbers and symbols
- TLTR layer for mouse, media, and display control

## Architecture

The project is organized by deployment method:

### Core Layers

- **kanata/**: Kanata configuration files (`.kbd` format) for cross-platform deployment
- **karabiner/**: Karabiner-Elements configuration (JSON) for macOS-specific deployment
- **qmk/**: QMK firmware for physical split keyboard implementation

### Build System

- **flake.nix**: Main Nix flake configuration
- **flake/**: Modular Nix configuration files:
  - `devshells.nix`: Development environment with nixd, alejandra formatter
  - `checks.nix`: Pre-commit hooks for code quality
  - `formatters.nix`: Tree formatting with alejandra (Nix), prettier (JSON)
  - `actions/`: GitHub Actions definitions in Nix

### Deployment Targets

1. **Software-based**: Kanata (cross-platform) or Karabiner-Elements (macOS)
2. **Hardware-based**: QMK firmware for Cantor/Piantor split keyboards

## Common Commands

### Development Environment

```bash
# Enter development shell
nix develop

# Format code
nix fmt

# Run all checks
nix flake check
```

### Keymap Visualization

```bash
# Generate keymap drawing (in dev shell)
keymap draw keymap-drawer/tltr.yml
```

### QMK Development

The QMK implementation is work-in-progress (see qmk/README.md). The current keymap in `qmk/keymaps/tltr/keymap.c` implements:

- Custom keycodes for Kanata-specific behaviors
- One-shot modifiers
- Layer momentary activation
- Fork key behaviors (context-dependent key actions)

## Project Structure Notes

- Physical keyboard config targets Cantor (Piantor) with Cherry MX1A Red switches
- Kanata configs support macOS (with Karabiner-DriverKit), Windows (with nircmd/InterceptionDriver), and Linux
- GitHub Actions automatically update flake.lock and generate keymap drawings
- Pre-commit hooks ensure code quality and prevent large file commits
