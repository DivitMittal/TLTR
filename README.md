<h1 align='center'>TLTR</h1>
<div align='center'>
    <img title='TLTR' src='./assets/logo.png' alt='Logo for the layout' height='250px'/>
</div>

---

<div align='center'>
    <img src="https://github.com/DivitMittal/TLTR/actions/workflows/.github/workflows/flake-check.yml/badge.svg" alt="nix-flake-check"/>
</div>

---

A bespoke cross-platform multi-layer keyboard layout for programmers, i.e., it optimizes for:

1. Minimal mouse/trackpad dependency
2. Convenient numbers & symbols access
3. Execution of complex keyboard shortcuts w/o cumbersome finger gymnastics.

---

| Layers                                                                                | Functionality                  |
| ------------------------------------------------------------------------------------- | ------------------------------ |
| [Colemak Mod-DH,Wide,Angle](https://github.com/ColemakMods/mod-dh?tab=readme-ov-file) | English letters & punctuations |
| TL                                                                                    | Navigation & Modifiers keys    |
| TR                                                                                    | Numbers & Symbols              |
| TLTR                                                                                  | Mouse, Media & Display control |
| QWERTY                                                                                | Backwards Compatibility        |

<div align='center'>
    <img title='Layers' src='./assets/tltr.svg' alt='Layers of the keyboard layout visualized'/>
</div>

---

## Can be deployed via:

1. [Kanata](https://github.com/jtroo/kanata/)

   1. macOS
      - Dependencies:
        1. [Karabiner-DriverKit](https://github.com/pqrs-org/Karabiner-DriverKit-VirtualHIDDevice/blob/main/dist/Karabiner-DriverKit-VirtualHIDDevice-3.1.0.pkg)
        2. [Shortcuts](https://www.wikipedia.com/en/articles/Shortcuts_(Apple))
   2. Windows
      - Dependencies:
        1. [nircmd](https://www.nirsoft.net/utils/nircmd.html)
        2. [InterceptionDriver](https://github.com/oblitum/Interception)
   3. \*nix

1. [Karabiner-Elements](https://github.com/pqrs-org/Karabiner-Elements)

   1. macOS

---

## Supported & Tested on:

- [ANSI US QWERTY](<https://en.m.wikipedia.org/wiki/File:ANSI_US_QWERTY_(Windows).svg>) keyboards
- macOS, \*nix, & Windows systems

---

## Physical implementation

### Standard Staggered ANSI US 60% Keyboard

![ANSI US Keyboard Image](./assets/ansi_us_keyboard.jpeg)

 ### Ergo-Split Keyboard

![Split Keyboard Image](./assets/split_keyboard.jpeg)

---

**Inspired by:** [Seniply](https://github.com/stevep99/seniply)