#!/usr/bin/env bash
# Entrena la tabla Q de nivel alto offline (con el mismo codigo del ESP32 corriendo
# en el PC contra proyecto/sim/trafico.h), la exporta a nivel_alto/tabla_q.h y
# compara nivel medio, nivel alto sin entrenar y nivel alto entrenado con el
# mismo trafico. Resultado: nivel_alto/entrenamiento.md.
# Uso: proyecto/sim/entrenar.sh [episodios por patron] [semillas de comparacion]
set -euo pipefail
cd "$(dirname "$0")"
EPISODIOS=${1:-600}
SEMILLAS=${2:-3}
mkdir -p build
CXX="g++ -std=gnu++17 -O2 -Wall -Wno-unused-function -I. -Ibuild"
python3 gen_prototypes.py ../nivel_medio/nivel_medio.ino > build/prototypes_medio.h
python3 gen_prototypes.py ../nivel_alto/nivel_alto.ino > build/prototypes_alto.h

echo "== entrenando ($EPISODIOS episodios por patron) =="
$CXX -DSIN_TABLA_ENTRENADA -I../nivel_alto -o build/entrenar entrenar.cpp Arduino.cpp
build/entrenar ../nivel_alto/tabla_q.h "$EPISODIOS" | tee build/entrenar.out

echo "== comparando ($SEMILLAS semillas x 5 patrones x 20 min) =="
$CXX -DNIVEL_INO='"nivel_medio.ino"' -DPROTOTIPOS='"prototypes_medio.h"' -I../nivel_medio -o build/comparar_medio comparar.cpp Arduino.cpp
$CXX -DNIVEL_ALTO -DSIN_TABLA_ENTRENADA -DNIVEL_INO='"nivel_alto.ino"' -DPROTOTIPOS='"prototypes_alto.h"' -I../nivel_alto -o build/comparar_alto_heur comparar.cpp Arduino.cpp
$CXX -DNIVEL_ALTO -DNIVEL_INO='"nivel_alto.ino"' -DPROTOTIPOS='"prototypes_alto.h"' -I../nivel_alto -o build/comparar_alto comparar.cpp Arduino.cpp
{
  build/comparar_medio "nivel medio" "$SEMILLAS"
  build/comparar_alto_heur "nivel alto (tabla heuristica)" "$SEMILLAS"
  build/comparar_alto "nivel alto (tabla entrenada)" "$SEMILLAS"
} > build/comparacion.csv
python3 resumen_comparacion.py build/comparacion.csv build/entrenar.out > ../nivel_alto/entrenamiento.md
cat ../nivel_alto/entrenamiento.md
