"""Write a flat KiCad 10 schematic from a Design.

Connectivity is expressed with local net labels placed exactly on pin
endpoints instead of drawn wires. That keeps the generator simple and the
sheet readable: every pin shows the name of the net it belongs to. Unused
pins get explicit no-connect flags so ERC stays meaningful.
"""

from __future__ import annotations

from .design import Design, Part
from .libs import stock_symbol, symbol_bbox, symbol_pins
from .sexpr import Str, dumps

GRID = 1.27
FILE_VERSION = 20260306


def snap(v: float) -> float:
    return round(round(v / GRID) * GRID, 4)


def _font(size=1.27):
    return ["font", ["size", size, size]]


def _label(net: str, x: float, y: float, angle: int, uid: str):
    justify = "left" if angle in (0, 90) else "right"
    return ["label", Str(net), ["at", x, y, angle],
            ["effects", _font(), ["justify", justify, "bottom"]], ["uuid", Str(uid)]]


def _property(name, value, x, y, hide=False, justify=None):
    node = ["property", Str(name), Str(value), ["at", x, y, 0]]
    if hide:
        node.append(["hide", "yes"])
    effects = ["effects", _font()]
    if justify:
        effects.append(["justify", justify])
    node.append(effects)
    return node


def resolve_symbol(design: Design, lib_id: str) -> list:
    if lib_id in design.custom_symbols:
        return design.custom_symbols[lib_id]
    return stock_symbol(lib_id)


def _text_anchor(bbox, pins) -> tuple[float, float, str]:
    """Pick a pin-free side of the symbol for its reference/value text."""
    xmin, ymin, xmax, ymax = bbox
    sides = {"right": False, "left": False, "top": False}
    for x, y, angle in pins.values():
        if angle == 180:
            sides["right"] = True
        elif angle == 0:
            sides["left"] = True
        elif angle == 270:
            sides["top"] = True
    if not sides["right"]:
        return (xmax + GRID, -(ymin + ymax) / 2, "left")
    if not sides["top"]:
        return ((xmin + xmax) / 2, -ymax - 2 * GRID, "center")
    return (xmax + GRID, -ymax - GRID, "left")


def build_schematic(design: Design) -> str:
    root = design.uid("sheet", "root")
    lib_symbols = {}
    for p in design.parts:
        if p.symbol not in lib_symbols:
            lib_symbols[p.symbol] = resolve_symbol(design, p.symbol)

    sch = ["kicad_sch", ["version", FILE_VERSION], ["generator", Str("kigen")],
           ["generator_version", Str("10.0")], ["uuid", Str(root)], ["paper", Str(design.paper)]]
    title = ["title_block", ["title", Str(design.title)], ["rev", Str(design.revision)]]
    for i, c in enumerate(design.comments, start=1):
        title.append(["comment", i, Str(c)])
    sch.append(title)
    sch.append(["lib_symbols", *lib_symbols.values()])

    for part in design.parts:
        sch.extend(_part_annotations(design, part, lib_symbols[part.symbol]))
    for part in design.parts:
        sch.append(_symbol_instance(design, part, lib_symbols[part.symbol], root))

    sch.append(["sheet_instances", ["path", Str("/"), ["page", Str("1")]]])
    sch.append(["embedded_fonts", "no"])
    return dumps(sch) + "\n"


def _pin_world(part: Part, px: float, py: float) -> tuple[float, float]:
    x0, y0 = snap(part.sch[0]), snap(part.sch[1])
    return (round(x0 + px, 4), round(y0 - py, 4))


def _part_annotations(design: Design, part: Part, sym: list) -> list:
    out = []
    pins = symbol_pins(sym)
    unknown = set(part.pins) - set(pins)
    if unknown:
        raise ValueError(f"{part.ref}: pins {sorted(unknown)} not on symbol {part.symbol}")
    for num, (px, py, angle) in sorted(pins.items()):
        x, y = _pin_world(part, px, py)
        if num in part.pins:
            out.append(_label(part.pins[num], x, y, (angle + 180) % 360, design.uid("label", part.ref, num)))
        else:
            out.append(["no_connect", ["at", x, y], ["uuid", Str(design.uid("nc", part.ref, num))]])
    return out


def _symbol_instance(design: Design, part: Part, sym: list, root: str) -> list:
    x0, y0 = snap(part.sch[0]), snap(part.sch[1])
    pins = symbol_pins(sym)
    tx, ty, justify = _text_anchor(symbol_bbox(sym), pins)
    ref_y = snap(y0 + ty - GRID) if justify != "center" else snap(y0 + ty - GRID)
    val_y = snap(ref_y + 2 * GRID)
    node = ["symbol", ["lib_id", Str(part.symbol)], ["at", x0, y0, 0], ["unit", 1], ["body_style", 1],
            ["exclude_from_sim", "no"], ["in_bom", "yes" if part.in_bom else "no"],
            ["on_board", "yes" if part.on_board else "no"], ["in_pos_files", "yes" if part.on_board else "no"],
            ["dnp", "yes" if part.dnp else "no"], ["uuid", Str(design.uid("symbol", part.ref))]]
    hide_ref = part.ref.startswith("#")
    j = None if justify == "center" else justify
    node.append(_property("Reference", part.ref, snap(x0 + tx), ref_y, hide=hide_ref, justify=j))
    node.append(_property("Value", part.value, snap(x0 + tx), val_y, hide=hide_ref, justify=j))
    node.append(_property("Footprint", part.footprint, x0, y0, hide=True))
    node.append(_property("Datasheet", "", x0, y0, hide=True))
    node.append(_property("Description", "", x0, y0, hide=True))
    for k, v in part.fields.items():
        node.append(_property(k, v, x0, y0, hide=True))
    for num in sorted(pins):
        node.append(["pin", Str(num), ["uuid", Str(design.uid("pin", part.ref, num))]])
    node.append(["instances", ["project", Str(design.name),
                               ["path", Str("/" + root), ["reference", Str(part.ref)], ["unit", 1]]]])
    return node
