# Ciudad Autoadaptable

Proyecto final del curso **Ingeniería de Sistemas Autoadaptables (ISA262)**: una maqueta de *smart city* que simula un cruce vial con dos semáforos. El objetivo es implementar el mismo sistema en tres niveles crecientes de complejidad (CPS bajo / medio / alto), usando todas las entradas y salidas de la maqueta.

## Estructura del repo

- **`docs/`** — copia local, literal, del sitio del curso ([isa262.davinsony.com](https://isa262.davinsony.com)). Cada `.md` tiene un `source:` con la URL original. Empieza por [`docs/README.md`](docs/README.md) (índice) y [`docs/GAPS.md`](docs/GAPS.md) (temas que el sitio no cubre, sobre todo nivel alto).
- **`assets/`** — recursos descargados: código de ejemplo, diapositivas de clase, PDFs, esquemáticos, proyectos Wokwi.
- **`proyecto/`** — una carpeta por nivel de implementación (`nivel_bajo/`, `nivel_medio/`, `nivel_alto/`), cada una con su código y su propio README de qué debería pasar al simularla. `proyecto/sim/` tiene las pruebas: un arnés nativo (`run.sh`), guiones para el simulador Wokwi (`wokwi/run_wokwi.sh`) y el entrenamiento del nivel alto (`entrenar.sh`).

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

Hay dos destinos y cada uno necesita su propia opción de compilación, porque en el ESP32-S3 el `Serial` del sketch puede ir al UART0 o al USB nativo:

**Para Wokwi** (los `code.bin`/`code.elf` que están en el repo): sin `CDCOnBoot`. El monitor serial de Wokwi está conectado al UART0 (`esp:TX`/`esp:RX` en `diagram.json`); con `CDCOnBoot=cdc` el sketch corre pero no se ve ni una línea de telemetría ni acepta comandos (verificado con `wokwi-cli`: solo aparece el arranque de la ROM).

```bash
arduino-cli compile --fqbn esp32:esp32:esp32s3 proyecto/<nivel> --export-binaries
cp proyecto/<nivel>/build/esp32.esp32.esp32s3/<nivel>.ino.bin proyecto/<nivel>/code.bin
cp proyecto/<nivel>/build/esp32.esp32.esp32s3/<nivel>.ino.elf proyecto/<nivel>/code.elf
```

**Para la placa física**: con `CDCOnBoot=cdc`, que manda `Serial` por el mismo USB con el que se programa. Sin esa opción la placa corre pero no se ve nada en el puerto.

```bash
arduino-cli compile --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc" proyecto/<nivel>
arduino-cli upload -p <puerto> --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc" proyecto/<nivel>
```

Si `esptool` no logra reiniciar la placa solo ("Failed to connect... No serial data received", pasa en algunos puertos USB con este firmware), los tres sketches aceptan el comando serie `BOOTLOADER`, que reinicia la placa en modo de carga; después se graba con `--before no-reset --after watchdog-reset`. Desde Windows, con los binarios de `proyecto/placa/`:

```powershell
python -c "import serial,time; s=serial.Serial('COM9',115200); s.dtr=True; s.write(b'BOOTLOADER\n'); time.sleep(0.5)"
python -m esptool --chip esp32s3 --port COM9 --before no-reset --after watchdog-reset --baud 460800 write-flash 0x0 proyecto\placa\bootloader.bin 0x8000 proyecto\placa\partitions.bin 0xe000 proyecto\placa\boot_app0.bin 0x10000 proyecto\placa\nivel_medio.bin
```

(El puerto COM puede cambiar al reiniciar; la telemetría por USB solo sale si el programa que abre el puerto levanta DTR, como hace `miniterm`.) La alternativa manual es mantener BOOT, tocar RESET y soltar BOOT.

Los sketches usan `Serial.setTxTimeoutMs(0)` dentro de `#if ARDUINO_USB_CDC_ON_BOOT`, porque ese método solo existe en la clase `HWCDC` (USB) y no en `HardwareSerial` (UART); así el mismo código compila en las dos variantes (core esp32 3.3.11).

Librerías necesarias en `~/Arduino/libraries/`: `LiquidCrystal I2C` (gestor de librerías) y `TimerMEF` (copiar el código de [`docs/software/timer-mef.md`](docs/software/timer-mef.md) a `TimerMEF/TimerMEF.h`).

## Rúbrica de la entrega

Ver [`docs/evaluacion/entrega.md`](docs/evaluacion/entrega.md). Resumen de pesos: nivel bajo 10 pts, nivel medio 20 pts, nivel alto 30 pts, demostración 20 pts, propuestas a nivel alto 10 pts, presentación 10 pts.
