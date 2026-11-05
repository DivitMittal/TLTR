"""Build a .kicad_pcb from a Design with KiCad's pcbnew Python module.

Must run under KiCad's interpreter (use scripts/kicad python3 ...). Board
coordinates in the design are relative to the top-left corner of the outline;
the outline itself is placed at ORIGIN so the board sits on the drawing sheet.
"""

from __future__ import annotations

import math
import zlib
from pathlib import Path

import pcbnew

from .design import Design
from .libs import stock_footprint_dir, symbol_pin_names, unconnected_net_name
from .schematic import resolve_symbol

ORIGIN = (50.0, 50.0)
SHEET = "/"  # local labels on the root sheet become "/<name>" nets
VIA_DIAMETER, VIA_DRILL = 0.8, 0.4  # hobby-fab friendly (JLCPCB/PCBWay standard)


def _v(x: float, y: float) -> "pcbnew.VECTOR2I":
    return pcbnew.VECTOR2I(pcbnew.FromMM(ORIGIN[0] + x), pcbnew.FromMM(ORIGIN[1] + y))


_LAYERS = {
    "F.Cu": "F_Cu", "B.Cu": "B_Cu", "F.SilkS": "F_SilkS", "B.SilkS": "B_SilkS",
    "F.Fab": "F_Fab", "B.Fab": "B_Fab", "Edge.Cuts": "Edge_Cuts",
}


def _layer(board, name: str) -> int:
    if name not in _LAYERS:
        raise ValueError(f"unsupported layer {name!r}; expected one of {sorted(_LAYERS)}")
    return getattr(pcbnew, _LAYERS[name])


def _load_footprint(lib_id: str, project_dir: Path):
    lib, name = lib_id.split(":", 1)
    local = project_dir / f"{lib}.pretty"
    libdir = local if local.is_dir() else stock_footprint_dir() / f"{lib}.pretty"
    fp = pcbnew.FootprintLoad(str(libdir), name)
    if fp is None:
        raise FileNotFoundError(f"footprint {lib_id} not found in {libdir}")
    fp.SetFPID(pcbnew.LIB_ID(lib, name))
    return fp


def build_board(design: Design, pcb_path: Path) -> None:
    project_dir = pcb_path.parent
    # Seed KiCad's UUID generator so every item (and the footprint order on
    # save, which follows UUIDs) is reproducible; CI checks for drift.
    pcbnew.KIID.SeedGenerator(zlib.crc32(design.name.encode()))
    # Start from an empty file so LoadBoard picks up the .kicad_pro written by
    # build.py (design rules, net classes) instead of built-in defaults.
    pcb_path.write_text('(kicad_pcb (version 20260206) (generator "kigen"))\n')
    board = pcbnew.LoadBoard(str(pcb_path))

    nets = {}

    def net(name: str):
        if name not in nets:
            nets[name] = pcbnew.NETINFO_ITEM(board, name)
            board.Add(nets[name])
        return nets[name]

    _outline(board, design)

    for part in design.parts:
        if not part.on_board or part.pcb is None:
            continue
        fp = _load_footprint(part.footprint, project_dir)
        fp.SetReference(part.ref)
        fp.SetValue(part.value)
        fp.SetPath(pcbnew.KIID_PATH("/" + design.uid("symbol", part.ref)))
        if hasattr(fp, "SetDNP"):
            fp.SetDNP(part.dnp)
        for key, value in part.fields.items():
            fp.SetField(key, value)
            fp.GetField(key).SetVisible(False)
        if part.footprint.startswith("MountingHole:"):
            fp.Reference().SetVisible(False)
        x, y, rot = part.pcb
        board.Add(fp)
        fp.SetPosition(_v(x, y))
        fp.SetOrientationDegrees(rot)
        pin_names = symbol_pin_names(resolve_symbol(design, part.symbol))
        for pad in fp.Pads():
            num = pad.GetNumber()
            if num in part.pins:
                pad.SetNet(net(SHEET + part.pins[num]))
            elif num in pin_names:
                # eeschema gives every no-connect pin its own named net
                pad.SetNet(net(unconnected_net_name(part.ref, num, pin_names[num])))
        missing = set(part.pins) - {p.GetNumber() for p in fp.Pads()}
        if missing:
            raise ValueError(f"{part.ref}: pins {sorted(missing)} have no pad on {part.footprint}")
    nets = {k[len(SHEET):]: v for k, v in nets.items() if k.startswith(SHEET)}

    for t in design.tracks:
        for (x1, y1), (x2, y2) in zip(t.points, t.points[1:]):
            tr = pcbnew.PCB_TRACK(board)
            tr.SetStart(_v(x1, y1))
            tr.SetEnd(_v(x2, y2))
            tr.SetWidth(pcbnew.FromMM(t.width))
            tr.SetLayer(_layer(board, t.layer))
            tr.SetNet(nets[t.net])
            board.Add(tr)

    for net_name, (x, y) in design.vias:
        via = pcbnew.PCB_VIA(board)
        via.SetPosition(_v(x, y))
        via.SetWidth(pcbnew.FromMM(VIA_DIAMETER))
        via.SetDrill(pcbnew.FromMM(VIA_DRILL))
        via.SetNet(nets[net_name])
        board.Add(via)

    for z in design.zones:
        _zone(board, design, z, nets[z.net])

    for t in design.texts:
        txt = pcbnew.PCB_TEXT(board)
        txt.SetText(t.text)
        txt.SetLayer(_layer(board, t.layer))
        txt.SetTextSize(pcbnew.VECTOR2I(pcbnew.FromMM(t.size), pcbnew.FromMM(t.size)))
        txt.SetTextThickness(pcbnew.FromMM(t.size * 0.15))
        txt.SetPosition(_v(*t.at))
        txt.SetTextAngleDegrees(t.angle)
        if t.layer.startswith("B."):
            txt.SetMirrored(True)
        board.Add(txt)

    tb = board.GetTitleBlock()
    tb.SetTitle(design.title)
    tb.SetRevision(design.revision)
    for i, c in enumerate(design.comments):
        tb.SetComment(i, c)

    # Island removal consults the connectivity graph; without a fresh build
    # every B.Cu fill fragment looks unconnected and is discarded.
    board.BuildConnectivity()
    pcbnew.ZONE_FILLER(board).Fill(board.Zones())
    pcbnew.SaveBoard(str(pcb_path), board)


def _outline_points(design: Design, steps: int = 8) -> list[tuple[float, float]]:
    w, h = design.board_size
    r = design.corner_radius
    pts = []
    corners = ((w - r, r, -90), (w - r, h - r, 0), (r, h - r, 90), (r, r, 180))
    for cx, cy, start in corners:
        for i in range(steps + 1):
            a = math.radians(start + 90 * i / steps)
            pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    return pts


def _outline(board, design: Design) -> None:
    w, h = design.board_size
    r = design.corner_radius
    edge = _layer(board, "Edge.Cuts")
    width = pcbnew.FromMM(0.1)

    def seg(a, b):
        s = pcbnew.PCB_SHAPE(board, pcbnew.SHAPE_T_SEGMENT)
        s.SetStart(_v(*a))
        s.SetEnd(_v(*b))
        s.SetLayer(edge)
        s.SetWidth(width)
        board.Add(s)

    def arc(center, start, angle):
        s = pcbnew.PCB_SHAPE(board, pcbnew.SHAPE_T_ARC)
        s.SetCenter(_v(*center))
        s.SetStart(_v(*start))
        s.SetArcAngleAndEnd(pcbnew.EDA_ANGLE(angle, pcbnew.DEGREES_T), True)
        s.SetLayer(edge)
        s.SetWidth(width)
        board.Add(s)

    seg((r, 0), (w - r, 0))
    seg((w, r), (w, h - r))
    seg((w - r, h), (r, h))
    seg((0, h - r), (0, r))
    if r > 0:
        arc((w - r, r), (w - r, 0), 90)
        arc((w - r, h - r), (w, h - r), 90)
        arc((r, h - r), (r, h), 90)
        arc((r, r), (0, r), 90)


def _zone(board, design: Design, z, net) -> None:
    zone = pcbnew.ZONE(board)
    # SetLayer() alone leaves a fresh zone's fill bookkeeping on F.Cu, so a
    # B.Cu zone "fills" to zero area; an explicit layer set avoids that.
    layers = pcbnew.LSET()
    layers.AddLayer(_layer(board, z.layer))
    zone.SetLayerSet(layers)
    zone.SetNet(net)
    zone.SetAssignedPriority(z.priority)
    zone.SetMinThickness(pcbnew.FromMM(z.min_width))
    zone.SetLocalClearance(pcbnew.FromMM(0.3))
    zone.SetThermalReliefGap(pcbnew.FromMM(0.4))
    zone.SetThermalReliefSpokeWidth(pcbnew.FromMM(0.6))
    zone.SetPadConnection(pcbnew.ZONE_CONNECTION_THERMAL)
    pts = z.polygon if z.polygon is not None else _outline_points(design)
    outline = zone.Outline()
    outline.NewOutline()
    for x, y in pts:
        outline.Append(_v(x, y))
    board.Add(zone)
