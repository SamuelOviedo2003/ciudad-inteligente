#!/usr/bin/env python3
"""Convierte build/comparacion.csv (salida de comparar.cpp) en nivel_alto/entrenamiento.md."""
import csv
import sys
from collections import defaultdict

filas = list(csv.reader(open(sys.argv[1])))
politica = open(sys.argv[2]).read() if len(sys.argv) > 2 else ""
niveles, patrones = [], []
datos = defaultdict(list)  # (nivel, patron) -> [(cola, espera_veh, atendidos, descartados, espera_peaton, peatones)]
for etiqueta, patron, _semilla, cola, espera, atendidos, descartados, espera_p, peatones in filas:
    if etiqueta not in niveles:
        niveles.append(etiqueta)
    if patron not in patrones:
        patrones.append(patron)
    datos[(etiqueta, patron)].append((float(cola), float(espera), int(atendidos), int(descartados), float(espera_p), int(peatones)))


def media(nivel, patron, i):
    v = datos[(nivel, patron)]
    return sum(x[i] for x in v) / len(v)


def media_total(nivel, i):
    return sum(media(nivel, p, i) for p in patrones) / len(patrones)


print("# Entrenamiento y comparación de niveles\n")
print("Generado por `proyecto/sim/entrenar.sh`. Los tres niveles corren el **mismo tráfico** (mismas semillas, mismos patrones, 20 min simulados por corrida) en el arnés nativo, con el modelo de `proyecto/sim/trafico.h`: llegadas Poisson por vía, un vehículo sale cada 1.5 s de verde, los CNY muestran presencia (mín(cola, 3)), y un peatón pulsa cada botón cada 2 min en promedio. Nivel bajo no aparece: sus tiempos son fijos y coinciden con nivel medio sin ningún bono.\n")
print("## Espera media por vehículo (s, menos es mejor)\n")
print("| Patrón | " + " | ".join(niveles) + " |")
print("|---|" + "---|" * len(niveles))
for p in patrones:
    print(f"| {p} | " + " | ".join(f"{media(n, p, 1):.1f}" for n in niveles) + " |")
print("| **promedio** | " + " | ".join(f"**{media_total(n, 1):.1f}**" for n in niveles) + " |")
print("\n## Cola media (vehículos esperando en las dos vías)\n")
print("| Patrón | " + " | ".join(niveles) + " |")
print("|---|" + "---|" * len(niveles))
for p in patrones:
    print(f"| {p} | " + " | ".join(f"{media(n, p, 0):.2f}" for n in niveles) + " |")
print("| **promedio** | " + " | ".join(f"**{media_total(n, 0):.2f}**" for n in niveles) + " |")
print("\n## Espera media del peatón (s) y vehículos atendidos por corrida\n")
print("| Nivel | Espera peatón | Atendidos | Descartados (cola llena) |")
print("|---|---|---|---|")
for n in niveles:
    print(f"| {n} | {media_total(n, 4):.1f} | {media_total(n, 2):.0f} | {media_total(n, 3):.1f} |")
if politica:
    print("\n## Política aprendida\n")
    print("```")
    print(politica.strip())
    print("```")
