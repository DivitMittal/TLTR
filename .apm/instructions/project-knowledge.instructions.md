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
- **qmk/**: QMK firmware for physical split keyboard implementation (Cantor/Piantor)

### Build System

- **flake.nix**: Main Nix flake configuration
- **flake/**: Modular Nix configuration:
  - `devshells.nix`: Development environment with nixd, alejandra formatter
  - `checks.nix`: Pre-commit hooks
  - `formatters.nix`: alejandra (Nix), prettier (JSON)
  - `actions/`: GitHub Actions definitions in Nix

### Deployment Targets

1. **Software-based**: Kanata (cross-platform)
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
- Kanata configs support macOS (Karabiner-DriverKit), Windows (InterceptionDriver), and Linux
- GitHub Actions automatically update flake.lock and generate keymap drawings
- QMK implementation is work-in-progress (see qmk/README.md)

## Hardware (hardware/, cad/)

- `hardware/<board>/generate.py` is the source of truth for each KiCad 10 project (`hardware/kigen/` is the generator library). Never hand-edit generated `.kicad_*`, `mechanical.json` or `docs/`; change the generator and run `scripts/kicad python3 hardware/<board>/generate.py --export`.
- A generator must exit 0: ERC, DRC and `--schematic-parity` clean at all severities. CI (`hardware-check`) also fails when committed KiCad files drift from the generator. Board UUIDs are seeded, so regeneration is byte-stable.
- `scripts/kicad` runs native KiCad 10 when present, else the `kicad/kicad:10.0` container (nixpkgs KiCad is broken on Darwin). Reports land in gitignored `.validation/`.
- `cad/enclosure.py` (build123d, run with `uv run`) builds tray + lid from `mechanical.json`; outputs in `cad/out/<board>/`. Heights in `cutouts` are estimates; keep the "verify before you order" table in `hardware/README.md` honest.
- Konnect MCP (`apm.yml`) needs the `konnect` binary on PATH; treat an MCP listing as unverified until it answers for the target board.
