---
description: TLTR project overview, architecture, commands, and deployment targets
applyTo: "**/*"
---

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
- **qmk/**: QMK firmware for physical split keyboard implementation (Cantor/Piantor)

### Build System

- **flake.nix**: Main Nix flake configuration
- **flake/**: Modular Nix configuration:
  - `devshells.nix`: Development environment with nixd, alejandra formatter
  - `checks.nix`: Pre-commit hooks
  - `formatters.nix`: alejandra (Nix), prettier (JSON)
  - `actions/`: GitHub Actions definitions in Nix

### Deployment Targets

1. **Software-based**: Kanata (cross-platform) or Karabiner-Elements (macOS)
2. **Hardware-based**: QMK firmware for Cantor/Piantor split keyboards

## Common Commands

```bash
nix develop            # Enter development shell
nix fmt                # Format code
nix flake check        # Run all checks

# Keymap Visualization (in dev shell)
keymap draw keymap-drawer/tltr.yml
```

## Project Structure Notes

- Physical keyboard targets Cantor (Piantor) with Cherry MX1A Red switches
- Kanata configs support macOS (Karabiner-DriverKit), Windows (nircmd/InterceptionDriver), and Linux
- GitHub Actions automatically update flake.lock and generate keymap drawings
- QMK implementation is work-in-progress (see qmk/README.md)
