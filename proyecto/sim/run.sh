#!/usr/bin/env bash
# Compila nivel_bajo.ino y nivel_medio.ino en el PC (mocks de Arduino, tiempo
# virtual) y corre todos los escenarios, mas las pruebas del puente Python.
# Uso: proyecto/sim/run.sh   (requiere g++ y python3; pyserial via uv o pip)
set -uo pipefail
cd "$(dirname "$0")"
export PYTHONDONTWRITEBYTECODE=1
mkdir -p build
python3 gen_prototypes.py ../nivel_medio/nivel_medio.ino > build/prototypes_medio.h
python3 gen_prototypes.py ../nivel_bajo/nivel_bajo.ino | grep -v nombreFase > build/prototypes_bajo.h
python3 gen_prototypes.py ../nivel_alto/nivel_alto.ino > build/prototypes_alto.h
g++ -std=gnu++17 -Wall -Wno-unused-function -I. -Ibuild -I../nivel_medio -o build/sim_medio test_medio.cpp Arduino.cpp || exit 1
g++ -std=gnu++17 -Wall -Wno-unused-function -I. -Ibuild -I../nivel_bajo -o build/sim_bajo test_bajo.cpp Arduino.cpp || exit 1
g++ -std=gnu++17 -Wall -Wno-unused-function -I. -Ibuild -I../nivel_alto -o build/sim_alto test_alto.cpp Arduino.cpp || exit 1

fallos=0
for e in baseline cny_polaridad congestion congestion_midfase eco wokwi_value_80 nocturno nocturno_histeresis \
         peaton_libre peaton_trafico peaton_fuera_de_fase peaton_sostenido nocturno_peaton lluvia det_remoto ping telemetria lcd; do
  build/sim_medio "$e" || fallos=$((fallos + 1))
done
for e in baseline telemetria remoto lcd; do
  build/sim_bajo "$e" || fallos=$((fallos + 1))
done
for e in baseline tabla_inicial recompensa aprendizaje exploracion reglas_fijas telemetria_lcd; do
  build/sim_alto "$e" || fallos=$((fallos + 1))
done
if command -v uv > /dev/null; then
  uv run --quiet --with pyserial python3 test_puente.py ../nivel_medio || fallos=$((fallos + 1))
else
  python3 test_puente.py ../nivel_medio || fallos=$((fallos + 1))
fi
echo
echo "Escenarios fallidos: $fallos"
[ "$fallos" -eq 0 ]
