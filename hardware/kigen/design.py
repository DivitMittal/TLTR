"""Design description shared by the schematic writer and the board builder.

A design is a flat list of parts. Each part names its KiCad symbol and
footprint, maps symbol pin numbers to net names, and carries its schematic
position and board placement. Both outputs are derived from this one table,
so the schematic and PCB cannot drift apart; `kicad-cli pcb drc
--schematic-parity` re-checks that claim on every build.
"""

from __future__ import annotations

import uuid
from dataclasses import dataclass, field


@dataclass
class Part:
    ref: str
    symbol: str  # "Lib:Name" from the stock libraries or a project library
    value: str
    footprint: str = ""  # "Lib:Name"; empty for schematic-only symbols
    pins: dict[str, str] = field(default_factory=dict)  # pin number -> net; missing pins get a no-connect flag
    sch: tuple[float, float] = (0.0, 0.0)  # schematic position, mm (snapped to 1.27 grid)
    pcb: tuple[float, float, float] | None = None  # x, y, rotation relative to the board origin
    dnp: bool = False
    fields: dict[str, str] = field(default_factory=dict)  # extra hidden schematic fields (MPN, Notes, ...)
    in_bom: bool = True
    on_board: bool = True


@dataclass
class Track:
    net: str
    points: list[tuple[float, float]]
    width: float
    layer: str = "B.Cu"


@dataclass
class Zone:
    net: str
    layer: str
    polygon: list[tuple[float, float]] | None = None  # None = whole board outline
    priority: int = 0
    min_width: float = 0.25


@dataclass
class Text:
    text: str
    at: tuple[float, float]
    layer: str = "F.SilkS"
    size: float = 1.0
    angle: float = 0.0


@dataclass
class Design:
    name: str  # project / file stem
    title: str
    revision: str
    board_size: tuple[float, float]
    corner_radius: float = 3.0
    parts: list[Part] = field(default_factory=list)
    tracks: list[Track] = field(default_factory=list)
    zones: list[Zone] = field(default_factory=list)
    texts: list[Text] = field(default_factory=list)
    custom_symbols: dict = field(default_factory=dict)  # "Lib:Name" -> symbol S-expression (list)
    custom_footprints: dict = field(default_factory=dict)  # "Lib:Name" -> footprint S-expression (list)
    mechanical: dict = field(default_factory=dict)  # extra data for the CAD enclosure
    comments: list[str] = field(default_factory=list)  # schematic title-block comments
    paper: str = "A4"

    def uid(self, *key: str) -> str:
        """Deterministic UUID so regenerated files diff cleanly."""
        return str(uuid.uuid5(uuid.NAMESPACE_URL, "kigen:" + self.name + ":" + ":".join(key)))

    def nets(self) -> list[str]:
        names = {n for p in self.parts for n in p.pins.values()}
        return sorted(names)

    def part(self, ref: str) -> Part:
        for p in self.parts:
            if p.ref == ref:
                return p
        raise KeyError(f"no part {ref!r} in design {self.name!r}")
