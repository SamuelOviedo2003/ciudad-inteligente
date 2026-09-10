# Traspaso: validación en las maquetas físicas desde Windows

## Estado al 2026-09-10 (sesión en el Windows de David)

Lo que ya está hecho y commiteado en `main`:

- **Polaridad medida y corregida**: botones P1/P2 activos en HIGH (`P_ACTIVO HIGH` + `P_MODO` en los tres `.ino`); CNY correctos (LOW); LCD 20x4. Con la constante vieja cada verde se cortaba a los 2 s por un "peatón fantasma".
- **Umbrales LDR** bajados a 150/250 (con luz de habitación una placa lee 250/330 y la otra 1550/300).
- **Modo demanda en nivel medio**: la vía con autos conserva el verde mientras la otra esté vacía; la vía vacía cede el verde apenas cumple 2 s si la otra tiene un auto; el peatón sigue por encima. Telemetría `demanda=1`, LCD `DEMANDA`. Escenario `demanda` en `sim/test_medio.cpp`; `congestion` ahora pone tráfico en las dos vías.
- **Comando serie `BOOTLOADER`** en los tres `.ino`: reinicia en modo de carga, porque en este Windows esptool no logra auto-resetear las placas (ver README raíz, sección "Para la placa física").
- Arnés `sim/` independiente de la polaridad; `sim/run.sh` con 0 fallos (corrido en Windows con g++ de WinLibs vía winget, en Git Bash).
- Puente ntfy probado de punta a punta con las dos maquetas (`TOPIC_RED` propio).

**Qué hay cargado en las placas ahora mismo**: nivel medio con la polaridad corregida (commit 7d41206), **sin** modo demanda ni comando `BOOTLOADER`. Para cargar el `placa/nivel_medio.bin` actual hace falta una última vez el modo carga manual (mantener BOOT, tocar RESET, soltar BOOT) y luego `esptool --before no-reset --after watchdog-reset ...`; a partir de ahí ya sirve el comando `BOOTLOADER`.

**Binarios en `placa/`**: `nivel_medio.bin` y `nivel_bajo.bin` compilados con todo lo anterior (arduino-cli 1.5.1, core esp32 3.3.11, `CDCOnBoot=cdc`). `nivel_alto.bin` tiene la polaridad corregida pero **no** el comando `BOOTLOADER` (esa compilación se colgó); recompilar antes de usarlo.

**Pendientes**:
- Ver en hardware el modo demanda y la prueba del botón (en telemetría se confirmó el ciclo 7/2/7/2 y `p1=0 p2=0` en reposo, pero no una pulsación).
- Sensor de CO2: lee 17000-20000 ppm constantes (ADC ~0,25 V), modo ECO siempre activo (verdes de 7 s). Revisar sensor/cable.
- En una maqueta el CNY5 nunca registra (CNY6 sí). En la placa B (MAC 6B:D0:90) solo se vio responder P2.
- El nivel alto no tiene modo demanda (el agente elige 3/5/8 s por vía); decidir si se quiere ahí también.
- Los `code.bin` de Wokwi siguen con `P_ACTIVO LOW` porque el diagrama cablea los botones a tierra.

**Cómo leer/grabar desde Windows**: los COM cambian con el puerto USB (buscar VID 303A con pyserial). La telemetría por USB solo sale si el programa que abre el puerto levanta DTR (`miniterm` lo hace; con pyserial, `s.dtr = True`). Placas: A = MAC 34:85:18:43:53:5C, B = 34:85:18:6B:D0:90.

Este documento es para un Claude Code (o una persona) trabajando en el **Windows que tiene las dos maquetas conectadas**. Todo el desarrollo y la verificación en simulación se hizo en otra máquina (Linux); acá solo falta lo que necesita hardware real. Leer primero [`DEMO.md`](DEMO.md) para el contexto general y [`../README.md`](../README.md) para el repo.

## Situación

- Tres niveles implementados y verificados en un arnés nativo (30 escenarios) y en el simulador Wokwi con el binario real (4 guiones). Ver `proyecto/sim/`.
- Las dos maquetas (ESP32-S3, USB nativo) aparecen en este Windows como **COM9 y COM10** ("USB Serial Device" en el Administrador de dispositivos). Python 3.14, `esptool` 5.4 y `pyserial` ya están instalados (`python -m esptool`, `python -m serial.tools.miniterm`).
- Los binarios para la placa ya están compilados en [`placa/`](placa/): `bootloader.bin` (va en 0x0), `partitions.bin` (0x8000), `boot_app0.bin` (0xe000), comunes a los tres niveles, y la aplicación de cada nivel `nivel_bajo.bin`, `nivel_medio.bin`, `nivel_alto.bin` (0x10000). Compilados con `esp32:esp32:esp32s3:CDCOnBoot=cdc`, que es la variante correcta para hardware (el `Serial` sale por el USB nativo). **No usar los `code.bin` de las carpetas de cada nivel en la placa**: esos son para Wokwi, sin CDC, y por USB no mostrarían nada.
- Si hubiera que recompilar (por ejemplo, para cambiar la polaridad de un sensor), hace falta `arduino-cli` con el core `esp32:esp32` 3.3.11 y las librerías `LiquidCrystal I2C` y `TimerMEF` (README raíz). Es una descarga grande; la alternativa es reportarle a David qué constante cambiar y que se recompile en la torre Linux y se suban binarios nuevos a `placa/`.

## Qué hay que averiguar (lo que ninguna simulación pudo)

1. **Polaridad de los botones P1/P2 y de los sensores CNY1..6 en la maqueta real.** El firmware asume que ambos van a tierra con pull-up: en reposo leen HIGH y activos LOW (`#define P_ACTIVO LOW`, `#define CNY_ACTIVO LOW` en los tres `.ino`). El código de pruebas del profesor (`assets/code/esp32/smart_city/esp_pruebas.ino`) sugiere que en la maqueta física podría ser al revés. Si está invertido y no se corrige, el nivel medio y el alto creen que siempre hay un peatón pidiendo y vehículos en todas partes.
2. **Si el LCD es de 20x4** (lo que asume el código y el simulador) **o de 16x4** (lo que dice la página del curso). Con 16x4 las filas 2 y 3 saldrían corridas cuatro columnas.
3. **La coordinación entre las dos maquetas por internet** (`DET_REMOTO`) de punta a punta con el puente real, que en simulación se probó con red falsa.
4. Que el nivel alto corra en la placa: telemetría `nivel=alto`, decisiones del agente con los sensores reales, y la tabla que sobrevive al reinicio (`nvs=1`).

## Pasos

Antes de nada, si hay ventanas de PowerShell con `esp_rfc2217_server` corriendo, cerrarlas (`Ctrl+C`): tienen tomados los puertos COM.

### 1. Grabar nivel medio en las dos placas

Desde la raíz del repo (una línea por placa, cambiando `COM9` por `COM10` para la segunda):

```powershell
python -m esptool --chip esp32s3 --port COM9 --baud 460800 write-flash 0x0 proyecto\placa\bootloader.bin 0x8000 proyecto\placa\partitions.bin 0xe000 proyecto\placa\boot_app0.bin 0x10000 proyecto\placa\nivel_medio.bin
```

Si esptool no logra conectar ("Failed to connect"), mantener presionado el botón BOOT de la placa, presionar y soltar RESET, soltar BOOT, y repetir el comando. Tras grabar, la placa se reinicia sola; el puerto USB puede desaparecer un segundo y volver.

### 2. Leer la telemetría en reposo y decidir la polaridad

Sin tocar ningún sensor ni botón:

```powershell
python -m serial.tools.miniterm COM9 115200
```

(salir con `Ctrl+]`). Cada segundo debe salir una línea como:

```
modo=NORMAL fase=A dur=5.00 co2=... ldr1=... ldr2=... cny1=0 cny2=0 cny3=0 cny4=0 cny5=0 cny6=0 det=0 det_remoto=0 p1=0 p2=0 peaton1_espera=0 ...
```

Interpretación:
- `cny1..cny6` y `p1`, `p2` **deben ser 0 en reposo**. Si salen en 1 sin que nadie toque nada, la polaridad está invertida para ese tipo de sensor: hay que cambiar `CNY_ACTIVO` o `P_ACTIVO` a `HIGH` en los tres `.ino` y recompilar (o pedirlo). Luego poner un objeto blanco sobre un CNY y presionar un botón para confirmar que pasan a 1.
- `ldr1`, `ldr2`: valores de 0 a 4095; tapar la LDR con la mano debe bajarlos. Si con luz normal están por debajo de 800, la maqueta entra en modo nocturno (`nocturno=1`, amarillos parpadeando); en ese caso reportar los valores para ajustar `UMBRAL_NOCHE_ENTRA` / `UMBRAL_NOCHE_SALE`.
- `co2`: `-1` significa fuera de rango de la fórmula (normal en aire limpio). Si sale un número enorme constante, reportarlo.
- Si no sale nada en 10 s: reiniciar la placa con el botón RESET; si sigue sin salir nada, revisar que se grabó el `.bin` de `placa/` y no un `code.bin` de las carpetas de nivel.

Anotar también qué muestra el LCD: si las cuatro filas se ven completas y alineadas (20x4) o si dos filas arrancan corridas (16x4). Una foto sirve.

### 3. Probar los modos con las manos sobre la maqueta

Con miniterm abierto en una placa: tapar dos CNY de una vía antes de su verde (`dur=8.00` en la siguiente fase de esa vía), presionar P1 (`peaton1_espera=1` y el verde de S1 se corta a los 2 s), tapar las dos LDR (`nocturno=1`). Escribir en miniterm `LLUVIA=1` y Enter (`lluvia=1`, amarillo de 3 s).

### 4. El puente con las dos maquetas

Antes, en `proyecto/nivel_medio/puente_serial.py` cambiar `TOPIC_RED` por un nombre propio del equipo. Luego, en dos terminales (cerrar miniterm primero, el puerto es de uno solo):

```powershell
python proyecto\nivel_medio\puente_serial.py COM9 A
python proyecto\nivel_medio\puente_serial.py COM10 B
```

Debe verse `[ESP32 -> PC] {...}` en cada una, el LCD de cada placa pasa a `PC: CONECTADO`, y al tapar cuatro CNY en la maqueta A, en menos de 15 s la B imprime `[PC -> ESP32] DET_REMOTO=4` y su siguiente verde dura 2 s más. Abrir `https://ntfy.sh/<TOPIC_RED>` en el navegador para ver los mensajes cruzar. Si aparecen errores 429 de ntfy.sh, reportarlo (hay límites por IP y el puente ya los respeta; no debería pasar).

### 5. Nivel alto

```powershell
python -m esptool --chip esp32s3 --port COM9 --baud 460800 write-flash 0x0 proyecto\placa\bootloader.bin 0x8000 proyecto\placa\partitions.bin 0xe000 proyecto\placa\boot_app0.bin 0x10000 proyecto\placa\nivel_alto.bin
```

(Solo cambia la aplicación en 0x10000; bootloader y particiones son los mismos, pero no estorba regrabarlos.)

En miniterm: la telemetría empieza con `nivel=alto`, y trae `s1= a1= explora1= r1= q1=`. Tapar los tres CNY de la vía 1: `a1=8`. Soltarlos y tapar los de la vía 2: `a1=3` y `a2=8`. Escribir `Q_DUMP` (64 filas y `Q fin`), `Q_SAVE` (`Q_SAVE ok 768`), presionar RESET y confirmar que la telemetría trae `nvs=1`. `EPSILON=0.5` hace que explore a la vista. Los comandos están descritos en [`nivel_alto/README.md`](nivel_alto/README.md).

## Qué reportar

Las líneas de telemetría en reposo de cada placa, la conclusión sobre la polaridad (y si se cambió alguna constante), qué LCD es, si `DET_REMOTO` cruzó entre las dos maquetas, y cualquier cosa que no coincida con lo que dice este documento o los READMEs. Si se cambia código, correr `proyecto/sim/run.sh` antes de hacer commit (necesita `g++` y `python3`; en Windows sirve WSL o Git Bash con MinGW; si no hay cómo, dejar el cambio en un commit aparte y avisar).
