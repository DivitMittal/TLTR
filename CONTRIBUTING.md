# Contributing

Contributions are welcome: bug reports, layout improvements, new platform support, and Nix packaging fixes.

## Setup

```sh
nix develop
```

This provides `keymap`, `kanata`, `alejandra`, and other tools needed for development.

## Guidelines

- Nix files: format with `nix fmt`.
- Karabiner JSON: format with `nix fmt` (prettier via treefmt).
- Keep platform-specific logic isolated to its respective directory (`kanata/`, `karabiner/`, `qmk/`).
- Run `nix flake check` before submitting.
- When changing key behaviors, update the keymap drawing source in `keymap-drawer/tltr.yml` to match.

## Submitting Changes

1. Fork the repo and create a branch: `feat/description` or `fix/description`.
2. Keep commits atomic and use Conventional Commits format.
3. Open a PR against `master` with a clear description of what changed and why.

## Reporting Issues

Open a GitHub issue with:

- Deployment method (Kanata, Karabiner-Elements, or QMK).
- OS and, if applicable, physical keyboard model.
- Kanata or QMK version (`kanata --version`, `qmk --version`).
- The layer and key sequence involved.
- Expected vs actual behavior.
