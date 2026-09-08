# Ciudad Autoadaptable

Proyecto final del curso **Ingeniería de Sistemas Autoadaptables (ISA262)**: una maqueta de *smart city* que simula un cruce vial con dos semáforos. El objetivo es implementar el mismo sistema en tres niveles crecientes de complejidad (CPS bajo / medio / alto), usando todas las entradas y salidas de la maqueta.

## Estructura del repo

- **`docs/`** — copia local, literal, del sitio del curso ([isa262.davinsony.com](https://isa262.davinsony.com)). Cada `.md` tiene un `source:` con la URL original. Empieza por [`docs/README.md`](docs/README.md) (índice) y [`docs/GAPS.md`](docs/GAPS.md) (temas que el sitio no cubre, sobre todo nivel alto).
- **`assets/`** — recursos descargados: código de ejemplo, diapositivas de clase, PDFs, esquemáticos, proyectos Wokwi.
- **`proyecto/`** — una carpeta por nivel de implementación (`nivel_bajo/`, `nivel_medio/`, `nivel_alto/`), cada una con su código y su propio README de qué debería pasar al simularla.

## Maqueta: pines de entrada/salida

Ver [`docs/hardware/ciudad.md`](docs/hardware/ciudad.md). Resumen:

| Tipo | Elementos |
|---|---|
| Entradas | LDR1, LDR2 (luz ambiente), CO2, P1/P2 (botones peatonales), CNY1–CNY6 (infrarrojos, detectan objetos blancos) |
| Salidas | LR1/LY1/LG1, LR2/LY2/LG2 (2 semáforos), pantalla LCD I2C 20x4 |

Placa: **ESP32 S3 Dev Module**.

## Setup del entorno (macOS)

1. Arduino IDE (`brew install --cask arduino-ide`) + core ESP32 (URL de placas: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`) — ver [`docs/software/esp.md`](docs/software/esp.md).
2. Librerías del curso instaladas en `~/Documents/Arduino/libraries/` (ver [`docs/software/librerias.md`](docs/software/librerias.md) y [`docs/software/timer-mef.md`](docs/software/timer-mef.md)).
3. Extensión **Wokwi Simulator** en VS Code + licencia gratuita en [wokwi.com/license](https://wokwi.com/license) — no hay maqueta física, todo se prueba simulado. Ver [`docs/software/wokwi.md`](docs/software/wokwi.md).

## Cómo correr una simulación

Cada carpeta en `proyecto/<nivel>/` trae `wokwi.toml`, `diagram.json`, `code.bin` y `code.elf` listos. Abrir el `diagram.json` en VS Code con la extensión de Wokwi instalada y darle *Start Simulation*.

## Cómo compilar un nivel nuevo

```bash
arduino-cli compile --fqbn esp32:esp32:esp32s3 proyecto/<nivel> --export-binaries
cp proyecto/<nivel>/build/esp32.esp32.esp32s3/<nivel>.ino.bin proyecto/<nivel>/code.bin
cp proyecto/<nivel>/build/esp32.esp32.esp32s3/<nivel>.ino.elf proyecto/<nivel>/code.elf
```

## Rúbrica de la entrega

Ver [`docs/evaluacion/entrega.md`](docs/evaluacion/entrega.md). Resumen de pesos: nivel bajo 10 pts, nivel medio 20 pts, nivel alto 30 pts, demostración 20 pts, propuestas a nivel alto 10 pts, presentación 10 pts.
