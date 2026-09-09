# Simulación nativa (sin Wokwi ni hardware)

Compila `nivel_bajo.ino` y `nivel_medio.ino` tal cual están, en el computador, contra un mock mínimo de la API de Arduino (`Arduino.h`, `LiquidCrystal_I2C.h`, `Wire.h`) y la `TimerMEF.h` real del curso. El reloj es virtual: cada vuelta de `loop()` avanza 1 ms, así que 5 minutos de semáforo se simulan en milisegundos y los tiempos salen exactos. Sirve para comprobar la lógica de control (fases, duraciones, modos, peatones, comandos por serial) cada vez que se toca el firmware, antes de abrir Wokwi o cargar la placa.

```bash
proyecto/sim/run.sh
```

Necesita `g++` y `python3`; para las pruebas del puente, `pyserial` (el script usa `uv run --with pyserial` si `uv` está instalado). Salida: una lista de `PASS`/`FAIL` por escenario y el total de fallos. Cada escenario corre en un proceso aparte porque el `.ino` tiene estado global.

## Qué cubre

- **Invariantes en todos los escenarios**: nunca dos verdes a la vez, nunca dos luces del mismo semáforo (salvo en nocturno, donde solo hay amarillos).
- **Nivel bajo**: ciclo A 5 s / B 2 s / C 5 s / D 2 s; los sensores y botones no alteran los tiempos; formato del JSON de telemetría; recepción del `det` de la otra maqueta.
- **Nivel medio**: ciclo base; bonos de congestión, ECO, lluvia y conteo remoto aplicados al entrar a la fase; caducidad del conteo remoto; nocturno (parpadeo de 0.5 s, exige ambos LDR, histéresis ante ruido); peatón (verde mínimo, corte con vía libre, espera máxima con tráfico, memoria fuera de fase, botón sostenido, prioridad sobre el nocturno); `PING`/`PONG` y el indicador "PC: CONECTADO"; tope del buffer serial; que ningún texto del LCD pase de 20 columnas ni se use `lcd.clear()` en el refresco; y el valor inicial de los potenciómetros de `diagram.json` (0 a 1023, no porcentaje).
- **Nivel alto**: tabla inicial (verde según cola propia, ajena y CO2), conteo de vehículos que pasan y recompensa, actualización Q-learning exacta, exploración ε-greedy, reglas fijas por encima del agente (lluvia, `DET_REMOTO`, peatón, nocturno), telemetría del agente y LCD.
- **Puente** (`test_puente.py`): parseo de la telemetría, cuántas publicaciones hace a ntfy.sh por minuto, y que el arranque no reenvíe el historial del tema.

## Qué no cubre

El tiempo que tarda el LCD por I2C (en el mock es instantáneo), el comportamiento del USB CDC, el ADC real, ni nada de Wokwi. Las conversiones numéricas de `TimerMEF.h` se ejecutan en x86, no en Xtensa. Un escenario que pasa aquí puede fallar en la placa por polaridad de un sensor o por cableado; para eso están `esp_pruebas.ino` y las notas de los READMEs de cada nivel.

## Agregar un escenario

En `test_medio.cpp` (o `test_bajo.cpp`), un `else if (esc == "nombre")` con los helpers `runFor`, `runUntilPhase`, `measurePhase`, `enviar` y las macros `CHECK`/`CHECK_NEAR`, y el nombre en la lista de `run.sh`. Las entradas se fijan escribiendo `pin_level[...]` (digital) o `analog_value[...]` (ADC), y lo que el firmware escribe por `Serial` queda en `serial_out`.

## Wokwi automatizado (`wokwi/`)

Los mismos comportamientos, pero corriendo el binario real dentro del simulador Wokwi, sin abrir VS Code: `wokwi/run_wokwi.sh` usa [`wokwi-cli`](https://github.com/wokwi/wokwi-cli/releases) con guiones YAML que presionan botones, mueven potenciómetros, escriben en el serial y comprueban los GPIO de los LEDs y el texto de la telemetría. Necesita un token gratuito de wokwi.com/dashboard/ci en `WOKWI_CLI_TOKEN`.

```bash
WOKWI_CLI_TOKEN=wok_... proyecto/sim/wokwi/run_wokwi.sh
```

- `medio_modos.yaml`: arranque en fase A con las seis luces correctas, `PING`/`PONG`, `LLUVIA=1`, `DET_REMOTO=5`, congestión en vía 1 con dos CNY, la A siguiente dura 10 s y el amarillo 3 s, ECO al bajar el potenciómetro de CO2 (C dura 9 s), y vuelta a `NORMAL`.
- `medio_nocturno_peaton.yaml`: peatón con vía libre y con tráfico, nocturno con solo amarillos, peatón que interrumpe el nocturno, reingreso a los 20 s, histéresis (una LDR en ~900 no saca del nocturno, en ~1230 sí).
- `bajo.yaml`: ciclo A/B/C con los pines correctos y sensores que se reflejan en la telemetría sin alterar el ciclo.
- `alto_agente.yaml`: el agente elige 8 s con la vía llena y 3 s enfrente, un CNY que se suelta cuenta como vehículo, `Q_SAVE` escribe 768 bytes en la flash simulada, `Q_DUMP` vuelca 64 estados, `EPSILON=1` fuerza exploración, `Q_RESET` responde.

Los binarios que usa son los `code.bin` del repo, compilados sin `CDCOnBoot` (ver el README raíz). Las simulaciones van en serie: con dos a la vez el servidor cierra la conexión. El LCD no se puede capturar como imagen con esta API (sale en blanco aunque el bus I2C esté activo, comprobado con un analizador lógico: 46 mil transiciones de SDA en 3 s); para ver el LCD hay que abrir el diagrama en VS Code.
