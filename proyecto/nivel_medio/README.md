# Nivel medio

CPS Generación 1: los mismos dos semáforos y la misma maqueta de `nivel_bajo`, pero ahora los setpoints (duración de verde/amarillo) **se ajustan en tiempo real** según el contexto (auto-ajuste + conciencia del contexto), y el sistema recibe por Serial un dato que no puede medir por sí mismo (clima real, vía internet) que amplía su capacidad de autoadaptación — ver [`docs/concepto/cps-nivel-bajo.md`](../../docs/concepto/cps-nivel-bajo.md) y la rúbrica en [`docs/evaluacion/entrega.md`](../../docs/evaluacion/entrega.md).

## Diferencias frente a nivel bajo

| | Nivel bajo | Nivel medio |
|---|---|---|
| Duración de verde/amarillo | Fija siempre (5 s / 2 s) | Se recalcula al entrar a cada fase según sensores |
| Sensores CNY (tráfico) | Solo se muestran en el LCD | Si hay ≥2 detectados en una vía, extienden su verde +3 s |
| CO2 | Solo se muestra | Si supera el umbral, extiende el verde (modo ECO) para reducir frenadas/arrancadas |
| LDR (luz ambiente) | Solo se muestra | Si ambos están oscuros, reemplaza el ciclo normal por parpadeo nocturno (ambos amarillos), salvo que un peatón esté pidiendo cruzar (ver prioridades abajo) |
| Botón peatonal | Solo se muestra SI/NO | Corta el verde actual si la vía está libre, o espera hasta un máximo de 12 s si hay tráfico |
| Comunicación | Ninguna | Serial con un script en el computador (`puente_serial.py`) que reenvía telemetría a internet y trae clima real que ajusta el amarillo |

## Cómo correrlo

1. Abrir `diagram.json` en VS Code (extensión Wokwi) → *Start Simulation*. Usa el mismo cableado que `nivel_bajo` (misma maqueta), en el puerto RFC2217 `4001` (distinto al de nivel bajo, `4000`, para poder tener ambos simuladores abiertos a la vez en la demo comparativa). Los potenciómetros de LDR1, LDR2 y CO2 ya vienen con un valor inicial alto (80%) puesto en `diagram.json` para arrancar en modo normal (de día, sin ECO) — si tu versión de Wokwi los ignora y arrancan en 0, **subir los tres antes de hacer nada más**, o el sistema arranca directo en modo nocturno (el semáforo nunca muestra verde, solo parpadea amarillo) y en modo ECO.
2. En una terminal aparte:
   ```bash
   cd proyecto/nivel_medio
   pip install pyserial
   python3 puente_serial.py
   ```
   El script se conecta por defecto a `rfc2217://localhost:4001` (el simulador). Con una ESP32 física, pasar el puerto serie real como argumento, ej. `python3 puente_serial.py /dev/tty.usbserial-0001`.

## Qué debería pasar

**Semáforos** — mismo ciclo de 4 fases de nivel bajo (A/B/C/D), pero la duración de cada una cambia según el contexto en el momento de entrar a esa fase:

- **Congestión**: mantener presionado (clic sostenido en Wokwi) 2 o 3 sensores CNY de una vía antes de que empiece su verde → esa fase dura 3 s más.
- **Modo ECO**: **bajar** el potenciómetro de CO2 (la fórmula del sensor da más ppm mientras más bajo está el potenciómetro, es contraintuitivo) hasta que supere `UMBRAL_CO2_ECO` en el código → el verde de ambas vías se extiende. Con el potenciómetro alto (valor por defecto) da -1 (fuera de rango) y ECO queda apagado.
- **Modo nocturno**: bajar ambos potenciómetros de LDR por debajo del umbral (`UMBRAL_OSCURIDAD`) → los semáforos dejan de ciclar y ambos amarillos parpadean cada 0.5 s, hasta que vuelva a haber luz.
- **Petición peatonal**: presionar P1 mientras S1 está en verde. Si la vía 1 no tiene autos detectados, el verde se corta de inmediato. Si hay autos, el LCD muestra "(esperando)" y el sistema garantiza el corte a más tardar en 12 s, haya o no tráfico.
- **Lluvia (dato de internet)**: cuando `puente_serial.py` consulta el clima real y detecta precipitación en la ubicación configurada, manda `LLUVIA=1` por serial → el amarillo se extiende 1 s en ambas vías. Para probarlo sin depender del clima real del día, se puede simular escribiendo manualmente en el monitor serial del simulador: `LLUVIA=1` (o `LLUVIA=0` para desactivarlo).

**LCD** — igual que en nivel bajo rota 5 pantallas cada 3 s (con refresco cada 0.3 s), pero ahora explica **por qué** se está comportando así: modo activo + fase + duración aplicada + si el puente con el computador está conectado (pantalla 0), estado del modo ECO (pantalla 1), estado del modo nocturno y sus umbrales (pantalla 2), congestión por vía (pantalla 3), y estado de los botones peatonales + clima recibido (pantalla 4). Si el modo nocturno está activo, el LCD lo muestra de inmediato sin esperar la rotación.

## Notas de diseño importantes

- **Polaridad de los sensores CNY**: en el diagrama de Wokwi van a tierra con pull-up, es decir que en reposo (nada detectado) leen HIGH y bajan a LOW cuando detectan un objeto (verificado siguiendo el cableado en `diagram.json`). Todo el código de control usa `vehiculoDetectado(pin) = (digitalRead(pin) == CNY_ACTIVO)`, nunca `digitalRead()` crudo, con `CNY_ACTIVO` como una sola constante al inicio de `nivel_medio.ino` — si se invirtiera, cualquier vía se vería "congestionada" todo el tiempo con solo dejar la maqueta quieta (nadie tocando los sensores = todos en HIGH = 3/3 "detectados"). En Wokwi esto significa que **sostener presionado el botón = vehículo detectado**. **Al pasar a la maqueta física, verificar qué nivel entrega el módulo CNY70 real al detectar un objeto y, si es distinto, cambiar únicamente `#define CNY_ACTIVO`** (una sola línea, no hay que tocar la lógica de control).
- **Prioridad entre modos**: no hay una máquina de modos formal (congestión, ECO y lluvia solo ajustan la duración dentro del ciclo normal, nunca colisionan entre sí porque cada vía calcula su propio bono al entrar a su fase). El único conflicto real es **peatonal vs. nocturno**: si no se resolviera explícitamente, un peatón que presiona el botón de noche quedaría ignorado, porque el modo nocturno reemplaza todo el ciclo normal (donde vive la lógica peatonal). La regla implementada es **peatonal > nocturno**: si se presiona P1 o P2 mientras está en modo nocturno, se interrumpe de inmediato, se retoma el ciclo normal en la fase A, y el modo nocturno queda suspendido 20 s (un ciclo completo) antes de poder volver a activarse, dándole tiempo al peatón de cruzar.

**Puente serial (consola de `puente_serial.py`)**:
```
[ESP32 -> PC] {'modo': 'NORMAL', 'fase': 'A', 'dur': 5.0, 'co2': 612, ...}
[PC -> ESP32] LLUVIA=1 (clima real de internet)
```

## Protocolo Serial (9600 baudios)

- **ESP32 → PC**, cada 1 s, una línea de texto `clave=valor` separada por espacios (`modo=NORMAL fase=A dur=5.0 co2=612 ldr1=... cny1=... p1=... peaton1_espera=... lluvia=... nocturno=...`). Los `cny1..cny6` ya vienen interpretados (`1` = vehículo detectado, `0` = libre), no el nivel eléctrico crudo. `puente_serial.py` convierte la línea completa a JSON antes de reenviarla a internet.
- **PC → ESP32**: `PING` (responde `PONG`, usado solo para que el LCD muestre "PC: CONECTADO"), `LLUVIA=1` / `LLUVIA=0`.

## Archivos

- `nivel_medio.ino` — código fuente del ESP32
- `puente_serial.py` — puente Serial ↔ Internet (clima real + telemetría)
- `diagram.json`, `wokwi.toml` — configuración del simulador (mismo cableado que `nivel_bajo`)

## Compilar (produce `code.bin`/`code.elf`, no incluidos aún)

```bash
arduino-cli compile --fqbn esp32:esp32:esp32s3 proyecto/nivel_medio --export-binaries
cp proyecto/nivel_medio/build/esp32.esp32.esp32s3/nivel_medio.ino.bin proyecto/nivel_medio/code.bin
cp proyecto/nivel_medio/build/esp32.esp32.esp32s3/nivel_medio.ino.elf proyecto/nivel_medio/code.elf
```

Requiere la librería `TimerMEF.h` y `LiquidCrystal_I2C` instaladas (ver [`docs/software/librerias.md`](../../docs/software/librerias.md) y [`docs/software/timer-mef.md`](../../docs/software/timer-mef.md)), igual que `nivel_bajo`. No se generaron binarios en este entorno porque no tiene `arduino-cli` ni el core de ESP32 instalados — compilar localmente antes de la demo.
