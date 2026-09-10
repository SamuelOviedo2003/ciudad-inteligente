# Nivel medio

CPS Generación 1: los mismos dos semáforos y la misma maqueta de `nivel_bajo`, pero ahora los setpoints (duración de verde/amarillo) **se ajustan en tiempo real** según el contexto (auto-ajuste + conciencia del contexto), y el sistema recibe por Serial datos que no puede medir por sí mismo — clima real, y el estado de la **otra maqueta de ciudad**, ambos vía internet — que amplían su capacidad de autoadaptación — ver [`docs/concepto/cps-nivel-bajo.md`](../../docs/concepto/cps-nivel-bajo.md) y la rúbrica en [`docs/evaluacion/entrega.md`](../../docs/evaluacion/entrega.md).

A diferencia de `nivel_bajo` (donde las dos maquetas físicas se conectan directo por USB al mismo computador, ver [`proyecto/puente_serial/`](../puente_serial/)), aquí las dos maquetas **no necesitan estar en el mismo computador ni cerca físicamente**: se coordinan a través de internet, que es justo lo que pide la rúbrica de este nivel.

## Diferencias frente a nivel bajo

| | Nivel bajo | Nivel medio |
|---|---|---|
| Duración de verde/amarillo | Fija siempre (5 s / 2 s) | Se recalcula al entrar a cada fase según sensores |
| Sensores CNY (tráfico) | Solo se muestran en el LCD | Si hay ≥2 detectados en una vía, extienden su verde +3 s |
| CO2 | Solo se muestra | Si supera el umbral, extiende el verde (modo ECO) para reducir frenadas/arrancadas |
| LDR (luz ambiente) | Solo se muestra | Si ambos están oscuros, reemplaza el ciclo normal por parpadeo nocturno (ambos amarillos), salvo que un peatón esté pidiendo cruzar (ver prioridades abajo) |
| Botón peatonal | Solo se muestra SI/NO | La pulsación se memoriza en cualquier fase; cuando su semáforo está en verde lo corta si la vía está libre (nunca antes de 2 s de verde) o espera como mucho 6 s si hay tráfico |
| Comunicación | Ninguna | Serial con un script en el computador (`puente_serial.py`) que reenvía telemetría a internet, trae clima real que ajusta el amarillo, y conecta con la otra maqueta vía internet |
| Coordinación entre maquetas | No existe (cada semáforo es independiente) | Si la otra maqueta (en cualquier computador con internet) reporta congestión alta, esta vía también extiende su verde +2 s |

## Cómo correrlo

1. Abrir `diagram.json` en VS Code (extensión Wokwi) → *Start Simulation*. Usa el mismo cableado que `nivel_bajo` (misma maqueta), en el puerto RFC2217 `4001` (distinto al de nivel bajo, `4000`, para poder tener ambos simuladores abiertos a la vez en la demo comparativa). Los potenciómetros de LDR1, LDR2 y CO2 vienen con `"value": "820"` en `diagram.json` (el atributo va de 0 a 1023 según la documentación de Wokwi, así que 820 es ~80 %; una versión anterior tenía `80`, que es un 8 % y dejaba el sistema arrancando en nocturno y ECO). Si igual arrancan abajo, **subir los tres antes de hacer nada más**, o el sistema arranca directo en modo nocturno (el semáforo nunca muestra verde, solo parpadea amarillo) y en modo ECO.
2. En una terminal aparte:
   ```bash
   cd proyecto/nivel_medio
   pip install pyserial
   python3 puente_serial.py
   ```
   El script se conecta por defecto a `rfc2217://localhost:4001` (el simulador). Con una ESP32 física, pasar el puerto serie real como argumento, ej. `python3 puente_serial.py /dev/tty.usbserial-0001`.
3. **Antes de la demo con las dos maquetas**: en `puente_serial.py`, cambiar `TOPIC_RED` por un nombre propio del equipo (el de por defecto es público y cualquiera podría estar usándolo). Correr el script en cada computador con un `id_maqueta` distinto como segundo argumento, ej. en la maqueta A: `python3 puente_serial.py rfc2217://localhost:4001 A`, y en la maqueta B: `python3 puente_serial.py rfc2217://localhost:4002 B`. Para probar esto con un solo computador, basta con tener las dos simulaciones de Wokwi abiertas a la vez (una en el puerto `4001`, ver `wokwi.toml`, y otra ciudad en otro puerto) y correr dos instancias del script, una por cada una.

## Qué debería pasar

**Semáforos** — mismo ciclo de 4 fases de nivel bajo (A/B/C/D), pero la duración de cada una cambia según el contexto en el momento de entrar a esa fase:

- **Congestión**: mantener presionado (clic sostenido en Wokwi) 2 o 3 sensores CNY de una vía antes de que empiece su verde → esa fase dura 3 s más.
- **Modo ECO**: **bajar** el potenciómetro de CO2 (la fórmula del sensor da más ppm mientras más bajo está el potenciómetro, es contraintuitivo) hasta que supere `UMBRAL_CO2_ECO` en el código → el verde de ambas vías se extiende. Con el potenciómetro alto (valor por defecto) da -1 (fuera de rango) y ECO queda apagado.
- **Modo nocturno**: bajar ambos potenciómetros de LDR por debajo de `UMBRAL_NOCHE_ENTRA` (150; en Wokwi el potenciómetro tiene que bajar de `value` 37, un 4 %) → los semáforos dejan de ciclar y ambos amarillos parpadean cada 0.5 s, hasta que alguno de los dos supere `UMBRAL_NOCHE_SALE` (250). Los umbrales se bajaron de 800/1000 tras medir las maquetas físicas con luz de habitación (una placa lee 250/330, la otra 1550/300; tapadas bajan a 13-200). La histéresis entre los dos umbrales evita que el ruido del ADC alrededor de un solo valor haga entrar y salir del modo varias veces por segundo (en simulación, con un solo umbral, lo hacía 25 veces en 5 s, reiniciando el ciclo cada vez).
- **Petición peatonal**: presionar P1 en cualquier momento; la pulsación queda memorizada (el LCD muestra "(esperando)") como en el botón de un cruce real. Se atiende cuando S1 está en verde: si la vía 1 no tiene autos detectados, el verde se corta en cuanto cumple `VERDE_MINIMO` (2 s, para que un botón sostenido o pegado no deje a la vía 1 sin paso); si hay autos, espera hasta que se libere o hasta `MAX_ESPERA_PEATON` (6 s desde la pulsación), lo que ocurra primero. La petición se borra cuando S1 llega a rojo (fase C), que es cuando el peatón cruza; si se presiona durante C ya está en rojo y no queda nada pendiente. P2 es simétrico con S2 (verde en C, cruce en A).
- **Lluvia (dato de internet)**: cuando `puente_serial.py` consulta el clima real y detecta precipitación en la ubicación configurada, manda `LLUVIA=1` por serial → el amarillo se extiende 1 s en ambas vías. Para probarlo sin depender del clima real del día, se puede simular escribiendo manualmente en el monitor serial del simulador: `LLUVIA=1` (o `LLUVIA=0` para desactivarlo).
- **Congestión en la otra maqueta (dato de internet, la coordinación entre las dos ciudades)**: cuando la otra maqueta reporta 4 o más CNY detectados (`UMBRAL_CONGESTION_RED`), el puente le manda `DET_REMOTO=<n>` a esta → el verde de la vía activa se extiende 2 s más (`BONUS_RED`), aunque en esta maqueta no haya tráfico local. Para probarlo sin la otra maqueta real, se puede simular escribiendo `DET_REMOTO=5` directo en el monitor serial del simulador.

**LCD** — igual que en nivel bajo rota 5 pantallas cada 3 s (con refresco cada 0.3 s, sobreescribiendo las 4 filas rellenadas a 20 columnas en vez de borrar la pantalla, para que no parpadee en el LCD real), pero ahora explica **por qué** se está comportando así: modo activo + fase + duración aplicada + si el puente con el computador está conectado (pantalla 0), estado del modo ECO (pantalla 1), estado del modo nocturno y sus umbrales (pantalla 2), congestión por vía **más el conteo de la otra maqueta** (pantalla 3), y estado de los botones peatonales + clima recibido (pantalla 4). Si el modo nocturno está activo, el LCD lo muestra de inmediato sin esperar la rotación.

## Notas de diseño importantes

- **Polaridad de los sensores CNY**: en el diagrama de Wokwi van a tierra con pull-up, es decir que en reposo (nada detectado) leen HIGH y bajan a LOW cuando detectan un objeto (verificado siguiendo el cableado en `diagram.json`). Todo el código de control usa `vehiculoDetectado(pin) = (digitalRead(pin) == CNY_ACTIVO)`, nunca `digitalRead()` crudo, con `CNY_ACTIVO` como una sola constante al inicio de `nivel_medio.ino` — si se invirtiera, cualquier vía se vería "congestionada" todo el tiempo con solo dejar la maqueta quieta (nadie tocando los sensores = todos en HIGH = 3/3 "detectados"). En Wokwi esto significa que **sostener presionado el botón = vehículo detectado**. **Al pasar a la maqueta física, verificar qué nivel entrega el módulo CNY70 real al detectar un objeto y, si es distinto, cambiar únicamente `#define CNY_ACTIVO`** (una sola línea, no hay que tocar la lógica de control).
- **Polaridad de los botones peatonales**: **medida en las maquetas físicas el 2026-09-09**: los botones tienen pull-down externo, en reposo leen LOW y presionados HIGH, así que `#define P_ACTIVO HIGH` y el pin se configura como `INPUT` (sin pull-up interno, vía `P_MODO`). Con la constante anterior (`LOW`, la de Wokwi) el sistema creía que siempre había un peatón pidiendo: S1 nunca mostraba más de 2 s de verde y el modo nocturno nunca entraba, que es exactamente lo que se vio en la maqueta. En Wokwi (`diagram.json`) los botones van a tierra con el pull-up interno, al revés: para simular ahí hay que poner `P_ACTIVO LOW` antes de compilar `code.bin` (los `code.bin` del repo están compilados así). Los CNY sí coinciden en las dos plataformas (`CNY_ACTIVO LOW`), y el LCD físico es de 20x4, como asume el código.
- **Prioridad entre modos**: no hay una máquina de modos formal (congestión, ECO y lluvia solo ajustan la duración dentro del ciclo normal, nunca colisionan entre sí porque cada vía calcula su propio bono al entrar a su fase). El único conflicto real es **peatonal vs. nocturno**: si no se resolviera explícitamente, un peatón que presiona el botón de noche quedaría ignorado, porque el modo nocturno reemplaza todo el ciclo normal (donde vive la lógica peatonal). La regla implementada es **peatonal > nocturno**: si se presiona P1 o P2 mientras está en modo nocturno, se interrumpe de inmediato, se retoma el ciclo normal en la fase A, y el modo nocturno queda suspendido 20 s (un ciclo completo) antes de poder volver a activarse, dándole tiempo al peatón de cruzar.

**Puente serial (consola de `puente_serial.py`)**:
```
[ESP32 -> PC] {'modo': 'NORMAL', 'fase': 'A', 'dur': 5.0, 'co2': 612, 'det': 2, ...}
[PC -> ESP32] LLUVIA=1 (clima real de internet)
[PC -> ESP32] DET_REMOTO=5 (otra maqueta, via internet)
```

## Coordinación con la otra maqueta, vía internet

`puente_serial.py` publica el conteo local de vehículos (`det`, de la telemetría) en un tema de [ntfy.sh](https://ntfy.sh) (servicio público gratuito de pub/sub por HTTP, sin cuenta ni API key) y revisa cada 10 s si la otra maqueta publicó algo nuevo en ese mismo tema — si es así, se lo manda al ESP32 como `DET_REMOTO=<n>`. Cada maqueta se identifica con un `id_maqueta` (`A`/`B`, segundo argumento del script) para que cada una ignore sus propios mensajes.

ntfy.sh tiene límites por IP (60 peticiones de ráfaga, luego una cada 5 s, y 250 mensajes publicados por día), así que el puente publica **solo cuando el conteo cambia**, nunca más seguido que cada 5 s, y si no cambia manda un latido cada 2 min. Al arrancar lee el historial del tema solo para ubicarse, sin reenviarlo al ESP32 (el tema guarda 12 h de mensajes). Del lado del ESP32, si en 5 min no llega ningún `DET_REMOTO`, el conteo remoto vuelve a 0: la otra maqueta o su puente se apagaron y no tiene sentido seguir extendiendo el verde por ella. Para la demo esto no se nota: al presionar los CNY de la otra maqueta el cambio se publica en el acto y llega en el siguiente sondeo (≤10 s). Se puede ver el canal en vivo abriendo `https://ntfy.sh/<TOPIC_RED>` en un navegador, que sirve como "dashboard" de la coordinación durante la presentación.

**Importante**: el tema por defecto (`TOPIC_RED` en el código) es público y adivinable — cualquiera que sepa el nombre puede publicar o leer ahí. Cambiarlo por uno propio del equipo antes de la demo (ej. agregar su usuario de GitHub al nombre) para no mezclar datos con otro grupo del curso usando el valor por defecto.

## Protocolo Serial (115200 baudios)

- **ESP32 → PC**, cada 1 s, una línea de texto `clave=valor` separada por espacios (`modo=NORMAL fase=A dur=5.0 co2=612 ldr1=... cny1=... det=2 det_remoto=0 p1=... peaton1_espera=... lluvia=... nocturno=...`). Los `cny1..cny6` ya vienen interpretados (`1` = vehículo detectado, `0` = libre), no el nivel eléctrico crudo; `det` es la suma de ambas vías locales. `puente_serial.py` convierte la línea completa a JSON antes de reenviarla a internet, y usa el campo `det` para publicarlo en ntfy.sh.
- **PC → ESP32**: `PING` (responde `PONG`, usado solo para que el LCD muestre "PC: CONECTADO"), `LLUVIA=1` / `LLUVIA=0`, `DET_REMOTO=<n>` (conteo de la otra maqueta, recibido vía ntfy.sh).

## Archivos

- `nivel_medio.ino` — código fuente del ESP32
- `puente_serial.py` — puente Serial ↔ Internet (telemetría, clima real, y coordinación con la otra maqueta vía ntfy.sh)
- `diagram.json`, `wokwi.toml` — configuración del simulador (mismo cableado que `nivel_bajo`)

## Compilar y subir a una ESP32-S3 real

```bash
arduino-cli compile --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc" proyecto/nivel_medio
arduino-cli upload -p <puerto> --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc" proyecto/nivel_medio
```

**`CDCOnBoot=cdc` es obligatorio en hardware real** (validado subiendo el código a dos ESP32-S3 físicas): por defecto la placa deja el `Serial` del sketch en el UART clásico, no en el puerto USB nativo, así que sin esta opción el ESP32 arranca y corre bien, pero no se ve absolutamente nada por el puerto USB (ni telemetría ni respuesta a comandos) — parece "colgado" sin estarlo.

**Y es exactamente al revés en Wokwi**: su monitor serial está en el UART0, así que el `code.bin` del repo se compila **sin** `CDCOnBoot` (comando en el README raíz). Un binario con `cdc` corre en Wokwi pero no muestra telemetría ni acepta comandos (verificado con `wokwi-cli`). No copiar el binario de la placa al simulador ni al revés.

Requiere la librería `TimerMEF.h` y `LiquidCrystal_I2C` instaladas (ver [`docs/software/librerias.md`](../../docs/software/librerias.md) y [`docs/software/timer-mef.md`](../../docs/software/timer-mef.md)), igual que `nivel_bajo`. `TimerMEF.h` no está en el gestor de librerías de Arduino — copiar el contenido de [`docs/software/timer-mef.md`](../../docs/software/timer-mef.md) a `~/Documents/Arduino/libraries/TimerMEF/TimerMEF.h`.
