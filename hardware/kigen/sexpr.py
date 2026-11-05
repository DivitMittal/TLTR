"""Minimal S-expression reader/writer for KiCad files.

Quoted strings are kept as `Str` so they round-trip with quotes; bare atoms
stay plain `str`. The writer mirrors KiCad's own layout closely enough for
stable git diffs: a list whose children are all atoms goes on one line,
anything else is broken over indented lines.
"""

from __future__ import annotations


class Str(str):
    """A token that was (or must be) written in double quotes."""


def parse(text: str):
    tokens = _tokenize(text)
    pos = 0

    def read():
        nonlocal pos
        tok = tokens[pos]
        pos += 1
        if tok == "(":
            out = []
            while tokens[pos] != ")":
                out.append(read())
            pos += 1
            return out
        if tok == ")":
            raise ValueError("unbalanced ')' in S-expression")
        return tok

    result = read()
    if pos != len(tokens):
        raise ValueError("trailing data after S-expression")
    return result


def _tokenize(text: str):
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c in " \t\r\n":
            i += 1
        elif c in "()":
            out.append(c)
            i += 1
        elif c == '"':
            j = i + 1
            buf = []
            while text[j] != '"':
                if text[j] == "\\":
                    buf.append(text[j : j + 2])
                    j += 2
                else:
                    buf.append(text[j])
                    j += 1
            out.append(Str(_unescape("".join(buf))))
            i = j + 1
        else:
            j = i
            while j < n and text[j] not in ' \t\r\n()"':
                j += 1
            out.append(text[i:j])
            i = j
    return out


def _unescape(s: str) -> str:
    return s.replace('\\"', '"').replace("\\n", "\n").replace("\\\\", "\\")


def _escape(s: str) -> str:
    return s.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")


def _atom(a) -> str:
    if isinstance(a, Str):
        return '"' + _escape(a) + '"'
    if isinstance(a, bool):
        return "yes" if a else "no"
    if isinstance(a, float):
        return fmt(a)
    return str(a)


def fmt(v: float) -> str:
    """KiCad-style number: up to 4 decimals, no trailing zeros, no '-0'."""
    s = f"{v:.4f}".rstrip("0").rstrip(".")
    return "0" if s in ("-0", "") else s


def dumps(node, indent: int = 0) -> str:
    pad = "\t" * indent
    if not isinstance(node, list):
        return pad + _atom(node)
    if all(not isinstance(c, list) for c in node):
        return pad + "(" + " ".join(_atom(c) for c in node) + ")"
    head = []
    rest = node
    # keep leading atoms (keyword, name, ...) on the opening line
    while rest and not isinstance(rest[0], list):
        head.append(_atom(rest[0]))
        rest = rest[1:]
    lines = [pad + "(" + " ".join(head)]
    for child in rest:
        lines.append(dumps(child, indent + 1))
    lines.append(pad + ")")
    return "\n".join(lines)


def find(node, key):
    """First direct child list whose keyword is `key`."""
    for c in node:
        if isinstance(c, list) and c and c[0] == key:
            return c
    return None


def find_all(node, key):
    return [c for c in node if isinstance(c, list) and c and c[0] == key]
