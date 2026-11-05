# Hardware

Electronics (KiCad 10) and 3D-printable enclosures (build123d) for TLTR
accessories. Every KiCad file here is **generated**: edit the generator, then
regenerate. The committed `.kicad_*` files are what you open in KiCad, send to
a fab, or inspect with the Konnect MCP.

| Board | What | Firmware |
| --- | --- | --- |
| [`pedal-hub/`](pedal-hub/) | RP2040-Zero carrier with two 6.35 mm footswitch jacks | [`pedals/rp2040-dual-pedal-hid`](../pedals/rp2040-dual-pedal-hid/) |

![pedal hub enclosure](../cad/out/pedal-hub/pedal-hub-preview.svg)

## Layout

```
hardware/
  kigen/               generator library: S-expression I/O, schematic writer, pcbnew board builder
  <board>/generate.py  the design: parts, nets, placement, routing, enclosure data
  <board>/*.kicad_*    generated KiCad 10 project (schematic, board, project-local libraries)
  <board>/docs/        schematic PDF/SVG, top/bottom copper renders
  <board>/<board>.step bare-board STEP from KiCad
  <board>/mechanical.json  outline, holes, edge cutouts, heights -> cad/enclosure.py
cad/
  enclosure.py         parametric tray + lid (build123d)
  out/<board>/         tray/lid STEP + STL, fit-check assembly STEP, preview SVG
scripts/
  kicad                run kicad-cli / KiCad's python3: native install, else the kicad/kicad:10.0 container
  hardware             regenerate everything (boards, checks, exports, enclosures)
```

## Regenerate

```sh
scripts/hardware                                   # all boards + enclosures
scripts/kicad python3 hardware/pedal-hub/generate.py --export   # one board
uv run cad/enclosure.py hardware/pedal-hub/mechanical.json      # one enclosure
```

`generate.py` fails unless all of these are clean:

- `kicad-cli sch erc` (all severities)
- `kicad-cli pcb drc --schematic-parity` (all severities, including unconnected items)

Reports go to `.validation/<board>/` (gitignored). The
[`hardware-check`](../.github/workflows/hardware-check.yml) workflow runs the
same generators in the `kicad/kicad:10.0` container and fails if the committed
KiCad files drift from their generators.

**Requirements:** a native KiCad 10, or Docker for the container fallback. On
macOS nixpkgs marks KiCad broken, so install KiCad.app or run a Docker VM, such
as `colima start`. Enclosures need [uv](https://docs.astral.sh/uv/). It
installs `build123d` from the script's inline metadata.

## Why generated KiCad, not atopile

atopile 0.15 announces itself as the last CLI release. Its successor is a
hosted app that needs `ato auth login`. Plain KiCad files plus a small
deterministic generator keep the design reviewable in git, buildable offline
and in CI, and openable in the KiCad GUI and in [Konnect](https://github.com/mixelpixx/Konnect)
for hand edits. If you hand-edit a board in KiCad, port the change back into
`generate.py`, or stop regenerating that board.

## Pedal hub

- **U1** Waveshare RP2040-Zero on pin headers. Only the two side rows are used
  (rows 15.24 mm apart, 2.54 mm pitch). USB-C is flush with the back edge.
- **J1/J2** Neutrik NMJ6HCD2 6.35 mm (¼″) stereo jacks. Tip goes to the GPIO.
  Ring and sleeve go to GND, so TS footswitches and tip-to-ring TRS pedals both
  work.
- **R1/R2 330 Ω + C1/C2 100 nF** series RC on each input. It limits ESD/RF
  pickup on long pedal cables. τ = 33 µs, invisible to the 25 ms firmware
  debounce.
- 2-layer, all through-hole. Signals are on B.Cu; F.Cu is an unbroken ground
  plane. Four M3 holes are 4 mm from the corners.

**BOM:** 1× RP2040-Zero, 2× 1×9 2.54 mm male headers, 2× NMJ6HCD2, 2× 330 Ω
¼ W, 2× 100 nF disc ceramic. Enclosure: 4× M3×20 cap screws, 4× M3 heat-set
inserts (4.0 mm hole), and optionally 4× 10 mm rubber feet.

### Verify before you order

Not all of these come from measurements:

| Item | Source | Status |
| --- | --- | --- |
| RP2040-Zero row spacing 15.24 mm, pin order | community footprint (CountParadox/RP2040-Zero-Kicad) cross-checked with Waveshare pinout | verify with calipers |
| NMJ6HCD2 mouth on the footprint's +X side | stock KiCad footprint, ferrule outline | check datasheet |
| Jack axis 6.6 mm and USB-C centre 5.1 mm above the PCB | datasheet-style estimates | measure the parts; edit `cutouts` in `generate.py` |
| Pedals use 6.35 mm plugs | assumption | for 3.5 mm pedals swap `JACK_FP` (e.g. `Jack_3.5mm_CUI_SJ1-3523N_Horizontal`) and re-route |

The enclosure prints without supports: the tray stands upright and the lid
prints top-down. The back-wall holes are bridged circles.
