#!/usr/bin/env python3
"""Genera los prototipos que el IDE de Arduino agrega solo, para poder compilar
un .ino con g++ (las funciones se usan antes de definirse)."""
import re
import sys

KEYWORDS = {"if", "for", "while", "switch", "else", "return", "case", "do"}
# Acepta la llave de apertura sola al final de la linea o una funcion completa en una linea.
pat = re.compile(r"^([A-Za-z_][\w\s\*&:<>]*?)\s+\**([A-Za-z_]\w*)\s*\(([^;{}]*)\)\s*\{(\s*|.*\}\s*)$")
out = []
for line in open(sys.argv[1], encoding="utf-8"):
    m = pat.match(line.rstrip("\n"))
    if not m:
        continue
    ret, name, args = m.groups()[:3]
    if name in KEYWORDS or ret.split()[0] in KEYWORDS:
        continue
    out.append(f"{ret} {name}({args});")
print("\n".join(out))
