#!/usr/bin/env bash
# Corre los guiones de automatizacion contra el simulador Wokwi real (wokwi-cli),
# usando los code.bin/code.elf de proyecto/nivel_bajo y proyecto/nivel_medio.
# Requiere wokwi-cli (https://github.com/wokwi/wokwi-cli/releases) y un token
# gratuito de https://wokwi.com/dashboard/ci en WOKWI_CLI_TOKEN.
# Las simulaciones van en serie: el servidor cierra la conexion si hay dos a la vez.
set -uo pipefail
cd "$(dirname "$0")"
: "${WOKWI_CLI_TOKEN:?Falta WOKWI_CLI_TOKEN (token de wokwi.com/dashboard/ci)}"
mkdir -p ../build/wokwi
fallos=0
correr() {
  local nivel=$1 guion=$2 timeout=$3
  echo "########## $guion ##########"
  wokwi-cli "../../$nivel" --timeout "$timeout" --scenario "$PWD/$guion.yaml" --serial-log-file "$PWD/../build/wokwi/$guion.serial.log" \
    | grep -E "^\[|Scenario|Timeout|rror|Unknown"
  local rc=${PIPESTATUS[0]}
  [ "$rc" -eq 0 ] || { echo "FALLO ($rc)"; fallos=$((fallos + 1)); }
  sleep 3
}
correr nivel_medio medio_modos 150000
correr nivel_medio medio_nocturno_peaton 150000
correr nivel_bajo bajo 60000
echo; echo "Guiones fallidos: $fallos"
[ "$fallos" -eq 0 ]
