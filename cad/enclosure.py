# /// script
# requires-python = ">=3.10"
# dependencies = ["build123d==0.13.0"]
# ///
"""Parametric two-part enclosure (tray + lid) for a generated KiCad board.

    uv run cad/enclosure.py hardware/pedal-hub/mechanical.json

Everything board-specific comes from the mechanical.json the KiCad generator
writes: outline, mounting holes, edge cutouts and component heights. Outputs
land next to this script in out/<name>/ as STEP (for CAD) and STL (for the
slicer), plus a fit-check assembly STEP when the board STEP exists.

Frame: X = board x, Y = board height - board y (front edge at Y=0, back edge
at Y=H), Z up from the outside of the tray floor.

Fastening: four M3 screws pass through the lid bosses and the PCB into M3
heat-set inserts in the tray standoffs, so one set of screws clamps both the
board and the lid.
"""

from __future__ import annotations

import json
import sys
from dataclasses import dataclass
from pathlib import Path

from build123d import (
    Align,
    Axis,
    Box,
    Compound,
    Cylinder,
    ExportSVG,
    LineType,
    Location,
    Pos,
    Rot,
    Text,
    extrude,
    export_step,
    export_stl,
    fillet,
    import_step,
)

ALIGN_MIN = (Align.MIN, Align.MIN, Align.MIN)
CENTER_BOTTOM = (Align.CENTER, Align.CENTER, Align.MIN)


@dataclass(frozen=True)
class Params:
    wall: float = 2.4
    floor: float = 2.5
    lid: float = 2.0
    gap: float = 0.4  # board edge to inner wall (sides and front)
    back_gap: float = 0.2  # connectors sit against the back wall
    corner_r: float = 3.0
    standoff_extra: float = 2.0  # clearance under the longest lead
    top_clearance: float = 1.5  # above the tallest part
    insert_d: float = 4.0  # M3 heat-set insert hole (for 4.2-4.6 mm OD inserts)
    insert_depth: float = 5.7
    standoff_d: float = 7.5
    screw_clear_d: float = 3.4
    screw_head_d: float = 6.0
    screw_head_depth: float = 1.0
    boss_d: float = 7.0
    lip: float = 1.2  # lid locating lip thickness
    lip_h: float = 3.0
    lip_clearance: float = 0.25
    feet_d: float = 10.5  # recesses for 10 mm rubber bumpers
    feet_depth: float = 1.0


def build(mech: dict, p: Params = Params()):
    board = mech["board"]
    enc = mech["enclosure"]
    bw, bh, bt = board["width"], board["height"], board["thickness"]

    standoff_h = enc["lead_protrusion_below_pcb"] + p.standoff_extra
    pcb_z = p.floor + standoff_h  # underside of the PCB
    inner_top = pcb_z + bt + enc["tallest_component_above_pcb"] + p.top_clearance

    # interior footprint (board plus clearances), origin at board x=0, Y=0 front
    ix0, ix1 = -p.gap, bw + p.gap
    iy0, iy1 = -p.gap, bh + p.back_gap
    ox0, oy0 = ix0 - p.wall, iy0 - p.wall
    ow, od = (ix1 - ix0) + 2 * p.wall, (iy1 - iy0) + 2 * p.wall

    def rounded_box(x, y, w, d, h, z, r):
        b = Box(w, d, h, align=ALIGN_MIN)
        b = fillet(b.edges().filter_by(Axis.Z), radius=r)
        return Pos(x, y, z) * b

    tray = rounded_box(ox0, oy0, ow, od, inner_top, 0, p.corner_r + p.wall)
    tray -= rounded_box(ix0, iy0, ix1 - ix0, iy1 - iy0, inner_top, p.floor, p.corner_r)

    def to_cad(x, y):
        return x, bh - y

    holes = [to_cad(h["x"], h["y"]) for h in mech["mounting_holes"]]
    for hx, hy in holes:
        tray += Pos(hx, hy, p.floor) * Cylinder(p.standoff_d / 2, standoff_h, align=CENTER_BOTTOM)
        tray -= Pos(hx, hy, pcb_z - p.insert_depth) * Cylinder(p.insert_d / 2, p.insert_depth, align=CENTER_BOTTOM)

    for c in mech.get("cutouts", []):
        tray -= _cutout(c, p, bw, bh, pcb_z + bt, ix0, ix1, iy0, iy1)

    vents = enc.get("vents") or []
    if vents:
        tray -= _vents(p, vents, ix0, ix1, oy0, od, pcb_z, inner_top)

    # rubber feet recesses, inset from the outer corners
    inset = p.feet_d / 2 + 3
    for fx in (ox0 + inset, ox0 + ow - inset):
        for fy in (oy0 + inset, oy0 + od - inset):
            tray -= Pos(fx, fy, 0) * Cylinder(p.feet_d / 2, p.feet_depth, align=CENTER_BOTTOM)

    # ---- lid: plate + locating lip + bosses that clamp the PCB
    lid = rounded_box(ox0, oy0, ow, od, p.lid, inner_top, p.corner_r + p.wall)
    c = p.lip_clearance
    lip_outer = rounded_box(ix0 + c, iy0 + c, ix1 - ix0 - 2 * c, iy1 - iy0 - 2 * c, p.lip_h,
                            inner_top - p.lip_h, p.corner_r - c)
    lip_inner = rounded_box(ix0 + c + p.lip, iy0 + c + p.lip, ix1 - ix0 - 2 * (c + p.lip),
                            iy1 - iy0 - 2 * (c + p.lip), p.lip_h, inner_top - p.lip_h,
                            max(p.corner_r - c - p.lip, 0.5))
    lid += lip_outer - lip_inner
    boss_h = inner_top - (pcb_z + bt)
    for hx, hy in holes:
        lid += Pos(hx, hy, pcb_z + bt) * Cylinder(p.boss_d / 2, boss_h, align=CENTER_BOTTOM)
        lid -= Pos(hx, hy, pcb_z + bt) * Cylinder(p.screw_clear_d / 2, boss_h + p.lid, align=CENTER_BOTTOM)
        lid -= Pos(hx, hy, inner_top + p.lid - p.screw_head_depth) * Cylinder(
            p.screw_head_d / 2, p.screw_head_depth, align=CENTER_BOTTOM)
    # the lip must not collide with back-edge cutouts' material either
    for cut in mech.get("cutouts", []):
        lid -= _cutout(cut, p, bw, bh, pcb_z + bt, ix0, ix1, iy0, iy1)

    if enc.get("lid_text"):
        label = Text(enc["lid_text"], font_size=8, align=(Align.CENTER, Align.CENTER))
        lid -= Pos(bw / 2, bh / 2, inner_top + p.lid - 0.6) * extrude(label, 0.6)

    info = {
        "outer_size_mm": [round(ow, 2), round(od, 2), round(inner_top + p.lid, 2)],
        "pcb_underside_z": round(pcb_z, 2),
        "screw": f"4x M3 x {screw_length(inner_top + p.lid - p.screw_head_depth - (pcb_z - p.insert_depth + 1.0))} "
                 "(cap head) + 4x M3 heat-set insert",
    }
    return tray, lid, pcb_z, info


def screw_length(span: float) -> int:
    """Shortest stock M3 length that does not bottom out in the insert hole."""
    for length in (16, 20, 25, 30, 35, 40):
        if length >= span - 4.5 and length <= span:
            return length
    return int(span)


def _cutout(c, p: Params, bw, bh, pcb_top, ix0, ix1, iy0, iy1):
    z = pcb_top + c["z_above_pcb"]
    depth = p.wall + p.gap + 2.0  # through the wall with margin either side
    edge = c["edge"]
    if edge in ("back", "front"):
        x = c["pos"]
        y = (iy1 + p.wall / 2) if edge == "back" else (iy0 - p.wall / 2)
        rot = Rot(90, 0, 0)
    else:
        x = (ix0 - p.wall / 2) if edge == "left" else (ix1 + p.wall / 2)
        y = bh - c["pos"]
        rot = Rot(0, 90, 0)
    loc = Location((x, y, z))
    if c["shape"] == "circle":
        return loc * rot * Cylinder(c["diameter"] / 2, depth)
    w, h = c["width"], c["height"]
    if edge in ("back", "front"):
        body = Box(w, depth, h)
        axis = Axis.Y
    else:
        body = Box(depth, w, h)
        axis = Axis.X
    r = min(w, h) / 2 - 0.01 if c["shape"] == "slot" else c.get("radius", 0.5)
    return loc * fillet(body.edges().filter_by(axis), radius=r)


def _vents(p: Params, sides, ix0, ix1, oy0, od, pcb_z, inner_top):
    """Horizontal slots in the named side walls ("left"/"right"), above the PCB."""
    walls = {"left": ix0 - p.wall / 2, "right": ix1 + p.wall / 2}
    unknown = set(sides) - set(walls)
    if unknown:
        raise ValueError(f"vents: unsupported walls {sorted(unknown)}; use 'left' and/or 'right'")
    slots = []
    n = 6
    z0 = pcb_z + 3
    pitch = (inner_top - 3 - z0) / n
    span = od - 2 * (p.corner_r + p.wall + 4)
    for side_x in (walls[s] for s in sides):
        for i in range(n):
            s = Box(p.wall + 2, span, 1.6)
            s = fillet(s.edges().filter_by(Axis.X), radius=0.79)
            slots.append(Pos(side_x, oy0 + od / 2, z0 + i * pitch) * s)
    out = slots[0]
    for s in slots[1:]:
        out += s
    return out


def export_preview(tray, lid, path: Path) -> None:
    """Isometric line drawing of the tray with the lid lifted off, for docs."""
    lift = tray.bounding_box().size.Z + 15
    scene = Compound([tray, Pos(0, 0, lift) * lid])
    centre = scene.bounding_box().center()
    eye = centre + (-120, -160, 110)  # from the front-left, so back-wall cutouts show through the lid gap
    visible, hidden = scene.project_to_viewport(eye, viewport_up=(0, 0, 1), look_at=centre)
    size = max(*scene.bounding_box().size)
    svg = ExportSVG(scale=200 / size)
    svg.add_layer("hidden", line_color="darkgray", line_type=LineType.ISO_DOT)
    svg.add_layer("visible", line_color="black", line_weight=0.35)
    svg.add_shape(hidden, layer="hidden")
    svg.add_shape(visible, layer="visible")
    svg.write(str(path))


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__)
        return 2
    mech_path = Path(sys.argv[1]).resolve()
    mech = json.loads(mech_path.read_text())
    name = mech["enclosure"]["name"]
    out = Path(__file__).resolve().parent / "out" / name
    out.mkdir(parents=True, exist_ok=True)

    tray, lid, pcb_z, info = build(mech)
    export_step(tray, str(out / f"{name}-tray.step"))
    export_step(lid, str(out / f"{name}-lid.step"))
    export_stl(tray, str(out / f"{name}-tray.stl"))
    # print the lid upside down: flat top on the bed
    export_stl(Rot(180, 0, 0) * lid, str(out / f"{name}-lid.stl"))

    board_step = mech_path.parent / f"{name}.step"
    if board_step.exists():
        pcb = import_step(str(board_step))
        bb = pcb.bounding_box()
        # KiCad exports the board top-left at the drawing origin offset; move it
        # so the outline matches the enclosure frame.
        pcb = Pos(-bb.min.X, -bb.min.Y, pcb_z - bb.min.Z) * pcb
        export_step(Compound([tray, lid, pcb]), str(out / f"{name}-assembly.step"))

    export_preview(tray, lid, out / f"{name}-preview.svg")
    (out / f"{name}-enclosure.json").write_text(json.dumps(info, indent=2) + "\n")
    print(json.dumps(info, indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
