"""Symbol/footprint library access and generators for project-local parts."""

from __future__ import annotations

import os
from pathlib import Path

from .sexpr import Str, find, find_all, parse

_STOCK_SYMBOL_DIRS = [
    os.environ.get("KICAD10_SYMBOL_DIR", ""),
    "/usr/share/kicad/symbols",
    "/Applications/KiCad/KiCad.app/Contents/SharedSupport/symbols",
]
_STOCK_FOOTPRINT_DIRS = [
    os.environ.get("KICAD10_FOOTPRINT_DIR", ""),
    "/usr/share/kicad/footprints",
    "/Applications/KiCad/KiCad.app/Contents/SharedSupport/footprints",
]


def _first_dir(candidates: list[str], what: str) -> Path:
    for c in candidates:
        if c and Path(c).is_dir():
            return Path(c)
    raise FileNotFoundError(f"KiCad {what} library directory not found; set KICAD10_{what.upper()}_DIR")


def stock_symbol_dir() -> Path:
    return _first_dir(_STOCK_SYMBOL_DIRS, "symbol")


def stock_footprint_dir() -> Path:
    return _first_dir(_STOCK_FOOTPRINT_DIRS, "footprint")


_lib_cache: dict[str, list] = {}


def _load_symbol_lib(lib: str) -> list:
    if lib not in _lib_cache:
        path = stock_symbol_dir() / f"{lib}.kicad_sym"
        _lib_cache[lib] = parse(path.read_text())
    return _lib_cache[lib]


def stock_symbol(lib_id: str) -> list:
    """Return a stock symbol renamed to `Lib:Name`, with `extends` flattened.

    A schematic's lib_symbols section must hold self-contained symbols, so a
    derived symbol (e.g. 74AHCT1G125 extends 74LVC1G125) takes its parent's
    graphics and pins with its own properties.
    """
    lib, name = lib_id.split(":", 1)
    tree = _load_symbol_lib(lib)
    sym = _named_symbol(tree, name, lib_id)
    ext = find(sym, "extends")
    if ext is None:
        out = [c for c in sym]
        out[1] = Str(lib_id)
        return out
    parent = _named_symbol(tree, ext[1], lib_id)
    own_props = {p[1]: p for p in find_all(sym, "property")}
    out = ["symbol", Str(lib_id)]
    for child in parent[2:]:
        if isinstance(child, list) and child[0] == "property" and child[1] in own_props:
            out.append(own_props.pop(child[1]))
        elif isinstance(child, list) and child[0] == "symbol":
            unit = list(child)
            unit[1] = Str(str(child[1]).replace(str(ext[1]), name, 1))
            out.append(unit)
        else:
            out.append(child)
    # properties only the child defines go before the unit sub-symbols
    insert_at = next(i for i, c in enumerate(out) if isinstance(c, list) and c[0] == "symbol")
    for prop in own_props.values():
        out.insert(insert_at, prop)
        insert_at += 1
    return out


def _named_symbol(tree: list, name: str, lib_id: str) -> list:
    for s in find_all(tree, "symbol"):
        if s[1] == name:
            return s
    raise KeyError(f"symbol {lib_id!r} not found in stock library")


def symbol_pins(sym: list) -> dict[str, tuple[float, float, int]]:
    """Pin number -> (x, y, angle) in symbol coordinates (y up), unit 1 / body style 1."""
    pins: dict[str, tuple[float, float, int]] = {}
    for unit in find_all(sym, "symbol"):
        # sub-symbol names end in _<unit>_<style>; 0 means "all units/styles"
        unit_no, style = str(unit[1]).rsplit("_", 2)[-2:]
        if unit_no not in ("0", "1") or style not in ("0", "1"):
            continue
        for pin in find_all(unit, "pin"):
            at = find(pin, "at")
            number = find(pin, "number")[1]
            pins[str(number)] = (float(at[1]), float(at[2]), int(float(at[3])) if len(at) > 3 else 0)
    return pins


def symbol_pin_names(sym: list) -> dict[str, str]:
    """Pin number -> pin name ('' when the symbol leaves it unnamed or '~')."""
    names: dict[str, str] = {}
    for unit in find_all(sym, "symbol"):
        for pin in find_all(unit, "pin"):
            name = str(find(pin, "name")[1])
            names[str(find(pin, "number")[1])] = "" if name in ("~", "") else name
    return names


def unconnected_net_name(ref: str, pin_number: str, pin_name: str) -> str:
    """The net name eeschema gives a lone no-connect pin, mirrored on the board for parity."""
    if pin_name:
        return f"unconnected-({ref}-{pin_name}-Pad{pin_number})"
    return f"unconnected-({ref}-Pad{pin_number})"


def symbol_bbox(sym: list) -> tuple[float, float, float, float]:
    """Rough body+pin extents (xmin, ymin, xmax, ymax) in symbol coordinates."""
    xs: list[float] = []
    ys: list[float] = []

    def walk(node):
        if not isinstance(node, list):
            return
        if node and node[0] in ("start", "end", "xy", "center") and len(node) >= 3:
            try:
                xs.append(float(node[1]))
                ys.append(float(node[2]))
            except ValueError:
                pass
        for c in node:
            if isinstance(c, list) and c and c[0] != "property":
                walk(c)

    walk(sym)
    for x, y, _ in symbol_pins(sym).values():
        xs.append(x)
        ys.append(y)
    if not xs:
        return (-2.54, -2.54, 2.54, 2.54)
    return (min(xs), min(ys), max(xs), max(ys))


# ---------------------------------------------------------------------------
# Project-local symbol and footprint generators
# ---------------------------------------------------------------------------

def _effects(size=1.27, hide=False, justify=None):
    e = ["effects", ["font", ["size", size, size]]]
    if justify:
        e.append(["justify", justify])
    out = e
    return (["hide", "yes"], out) if hide else (out,)


def _prop(name, value, at, hide=False):
    items = ["property", Str(name), Str(value), ["at", at[0], at[1], 0]]
    items.extend(_effects(hide=hide))
    return items


def module_symbol(lib_id: str, left: list[tuple[str, str]], right: list[tuple[str, str]],
                  footprint: str, description: str, ref_prefix: str = "U") -> list:
    """Rectangular module symbol with passive pins on both sides.

    `left`/`right` are top-to-bottom (pin number, pin name) lists; an empty
    name leaves a gap. All pins are passive: the modules carry their own
    regulators, so ERC power-driver rules would only produce noise.
    """
    _, name = lib_id.split(":", 1)
    rows = max(len(left), len(right))
    half_h = (rows + 1) * 2.54 / 2
    half_h = round(half_h / 1.27) * 1.27
    half_w = 10.16
    top = (rows - 1) * 2.54 / 2
    unit_body = ["symbol", Str(f"{name}_0_1"),
                 ["rectangle", ["start", -half_w, half_h], ["end", half_w, -half_h],
                  ["stroke", ["width", 0.254], ["type", "default"]], ["fill", ["type", "background"]]]]
    unit_pins = ["symbol", Str(f"{name}_1_1")]
    for side, entries in ((-1, left), (1, right)):
        for i, (num, pname) in enumerate(entries):
            if not num:
                continue
            y = top - i * 2.54
            x = side * (half_w + 2.54)
            angle = 0 if side < 0 else 180
            unit_pins.append(["pin", "passive", "line", ["at", x, y, angle], ["length", 2.54],
                              ["name", Str(pname), *_effects()], ["number", Str(num), *_effects()]])
    return ["symbol", Str(lib_id),
            ["pin_names", ["offset", 1.016]],
            ["exclude_from_sim", "no"], ["in_bom", "yes"], ["on_board", "yes"],
            _prop("Reference", ref_prefix, (0, half_h + 1.27)),
            _prop("Value", name, (0, -half_h - 1.27)),
            _prop("Footprint", footprint, (0, 0), hide=True),
            _prop("Datasheet", "", (0, 0), hide=True),
            _prop("Description", description, (0, 0), hide=True),
            unit_body, unit_pins,
            ["embedded_fonts", "no"]]


def symbol_library(symbols: list[list]) -> list:
    """A .kicad_sym file holding the given symbols (names stripped of `Lib:`)."""
    out = ["kicad_symbol_lib", ["version", 20251024], ["generator", Str("kigen")],
           ["generator_version", Str("10.0")]]
    for s in symbols:
        s = list(s)
        s[1] = Str(str(s[1]).split(":", 1)[1])
        out.append(s)
    return out


def module_footprint(name: str, size: tuple[float, float], columns: list[tuple[float, list[str]]],
                     pitch: float, description: str, usb_marker: bool = True,
                     drill: float = 1.0, pad: float = 1.7) -> list:
    """Through-hole footprint for a castellated module mounted on pin headers.

    The origin is the module centre, USB end towards -Y. `columns` lists
    (x, [pad numbers top-to-bottom]); rows are centred on the origin.
    """
    w, h = size
    fp = ["footprint", Str(name), ["version", 20260206], ["generator", Str("kigen")],
          ["generator_version", Str("10.0")], ["layer", Str("F.Cu")],
          ["descr", Str(description)], ["tags", Str("module castellated pin header")],
          ["property", Str("Reference"), Str("REF**"), ["at", 0, -h / 2 - 1.5, 0], ["layer", Str("F.SilkS")],
           ["effects", ["font", ["size", 1, 1], ["thickness", 0.15]]]],
          ["property", Str("Value"), Str(name), ["at", 0, h / 2 + 1.5, 0], ["layer", Str("F.Fab")],
           ["effects", ["font", ["size", 1, 1], ["thickness", 0.15]]]],
          ["attr", "through_hole"]]
    for layer, width, grow in (("F.Fab", 0.1, 0.0), ("F.SilkS", 0.12, 0.11), ("F.CrtYd", 0.05, 0.5)):
        fp.append(["fp_rect", ["start", -w / 2 - grow, -h / 2 - grow], ["end", w / 2 + grow, h / 2 + grow],
                   ["stroke", ["width", width], ["type", "solid"]], ["fill", "no"], ["layer", Str(layer)]])
    if usb_marker:
        fp.append(["fp_text", "user", Str("USB"), ["at", 0, -h / 2 + 2, 0], ["layer", Str("F.Fab")],
                   ["effects", ["font", ["size", 1, 1], ["thickness", 0.15]]]])
    for x, pads in columns:
        top = -(len(pads) - 1) * pitch / 2
        for i, num in enumerate(pads):
            shape = "rect" if num == "1" else "circle"
            fp.append(["pad", Str(num), "thru_hole", shape, ["at", x, round(top + i * pitch, 4)],
                       ["size", pad, pad], ["drill", drill],
                       ["layers", Str("*.Cu"), Str("*.Mask")], ["remove_unused_layers", "no"]])
    fp.append(["embedded_fonts", "no"])
    return fp
