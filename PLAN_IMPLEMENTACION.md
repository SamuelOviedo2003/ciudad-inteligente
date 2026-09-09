# Plan de implementación — Ciudad Autoadaptable (ISA262)

> Análisis del estado actual del repo (`ciudad-inteligente-main`, descargado sin historial git) y plan concreto para completar los niveles medio y alto exigidos por [`docs/evaluacion/entrega.md`](docs/evaluacion/entrega.md).

## 1. Qué hay hecho hoy

### 1.1 Documentación (`docs/`, `assets/`)
Es un mirror completo y bien organizado del sitio del curso (isa262.davinsony.com): teoría de CPS, MEF con tiempo relativo (`TimerMEF.h`), pinout de la maqueta, librerías, ejemplos de Arduino/ESP32/Wokwi. Ya incluye un análisis propio de vacíos en [`docs/GAPS.md`](docs/GAPS.md) que sigue siendo válido — lo confirmo y lo uso como base de este plan (sección 2).

### 1.2 `proyecto/nivel_bajo/` — implementación de nivel bajo
Revisé [`nivel_bajo.ino`](proyecto/nivel_bajo/nivel_bajo.ino) y su [README](proyecto/nivel_bajo/README.md):

- **Semáforos**: MEF de 4 fases con `TimerMEF.h`, tiempos fijos (verde 5 s / amarillo 2 s), nunca reacciona a sensores ni botones. Esto es correcto para "nivel bajo" = **Generación 0** (lazo cerrado con setpoints predeterminados, no gestiona cambios, ver [`docs/concepto/cps-nivel-bajo.md`](docs/concepto/cps-nivel-bajo.md)).
- **LCD**: rota 4 anuncios cada 3 s (CO2, LDR1/LDR2, conteo CNY1–6, estado P1/P2), refrescando cada 0.3 s. Así se **usan todas las entradas** de la maqueta (cumple el requisito "dar uso a todas las entradas y salidas") sin que afecten el control — es una separación limpia entre "lo que se mide" y "lo que se decide", apropiada para ilustrar que en nivel bajo la medición no se traduce en adaptación.
- Constantes de calibración de CO2 y pines tomados literalmente de `esp_pruebas.ino`, consistente con el hardware documentado.
- Reutiliza `TimerMEF.h`, que es la librería exacta que pide el curso para MEF con tiempo relativo — buena práctica, evita `delay()`.

**Veredicto**: la implementación de nivel bajo es sólida y defendible tal cual está. No encontré errores funcionales. Dos observaciones menores (no bloqueantes):

1. El comentario del pin `P1`/`P2` dice "sin resistencia externa... pull-up interno" — coherente con `INPUT_PULLUP`, pero difiere de `esp_pruebas.ino` que usa `INPUT` simple. Confirmar en la maqueta física (si existe) o mantener así si solo se usa el simulador Wokwi.
2. El pinout de `hardware/ciudad.md` tiene a `LDR1=13/LDR2=12`, mientras que `esp_pruebas.ino` y `nivel_bajo.ino` usan `LDR1=12/LDR2=13` (intercambiados). Es el inconsistencia ya señalada en `docs/GAPS.md` punto 5 — no afecta la lógica (son simétricos), pero conviene decidir una convención única antes de escribir nivel medio/alto para no arrastrar la confusión.

### 1.3 Lo que falta físicamente en el repo
`proyecto/` solo tiene `nivel_bajo/`. **No existen `proyecto/nivel_medio/` ni `proyecto/nivel_alto/`** aunque el [`README.md`](README.md) raíz ya los menciona como si existieran. Ese es el trabajo pendiente real, no solo documentación.

## 2. Gaps confirmados (heredados de `docs/GAPS.md` + los míos)

| # | Gap | Impacto en rúbrica |
|---|---|---|
| 1 | No hay código de nivel medio ni nivel alto | 20 + 30 + 20 (demo) = **70/100 pts sin cubrir** |
| 2 | "Nivel alto" no está definido en ningún lado del curso (ni técnica ni ejemplo) — el enunciado dice literalmente "de alguna manera" | Hay que **proponer y justificar** la técnica nosotros mismos (ver sección 4) |
| 3 | El patrón de "serial + internet" solo existe como ejemplos genéricos de WiFi directo (`esp_send_data.ino`, `esp_get_data.ino`) — **no** es comunicación serial con un computador que hable con internet, que es lo que pide la rúbrica de nivel medio | Hay que diseñar el puente serial↔PC↔internet nosotros (sección 3.3) |
| 4 | Scripts de puente ya descargados (`assets/code/wokwi/websocket-serial.py`, `monitoreo-master/sim2real.py`, `monitoreo-slave/real2sim.py`) no tienen protocolo documentado | Sirven de referencia de patrón (ver 3.3), pero hay que adaptarlos, no son plug-and-play para la ciudad |
| 5 | Inconsistencia de pines LDR1/LDR2 entre `hardware/ciudad.md` y el código de ejemplo | Definir una convención única antes de escribir nivel medio/alto |
| 6 | Sensor CO2 sin modelo/documentación real | Para el simulador no importa (es un potenciómetro en Wokwi); si hay maqueta física, verificar antes de la demo |

## 3. Nivel medio — Generación 1 (self-tuning + context-aware)

**Definición objetivo** ([`docs/concepto/cps-nivel-medio.md`](docs/concepto/cps-nivel-medio.md)): planeación de secuencias, más tiempo para decidir, puede optimizar o elegir entre criterios. Generación 1 ([`docs/concepto/cps-nivel-bajo.md`](docs/concepto/cps-nivel-bajo.md)): mismo lazo cerrado del nivel bajo, pero **los setpoints cambian según el entorno** (auto-ajuste + conciencia del contexto), mediante modos de operación (SOM) conmutados.

### 3.1 Adaptación local (con lo que ya hay en la maqueta)
Reemplazar los tiempos fijos de `T_VERDE`/`T_AMARILLO` por setpoints que cambian según sensores, definiendo modos de operación explícitos (SOM):

- **Modo NORMAL**: tiempos base (los del nivel bajo).
- **Modo CONGESTIÓN**: si `CNY1..3` (vía 1) o `CNY4..6` (vía 2) reportan ocupación sostenida, extender el verde de esa vía (p. ej. +3 s) y acortar la otra, dentro de un mínimo/máximo de seguridad.
- **Modo PEATONAL**: `P1`/`P2` fuerzan una fase de "todo en rojo" con tiempo fijo de cruce antes de reanudar la MEF (esto ya es adaptación real del comportamiento, a diferencia del nivel bajo donde P1/P2 solo se mostraban en el LCD).
- **Modo NOCTURNO / BAJA LUZ**: si `LDR1`/`LDR2` caen bajo un umbral, cambiar a modo intermitente (amarillo parpadeante en ambos, como un semáforo real de madrugada) — reutiliza directamente el patrón de `ejemplo-timer.ino` que ya está en `assets/code/wokwi/ejemplo-timer/ejemplo-timer.ino`.
- **Modo ECO** (usa CO2): si el CO2 supera un umbral, priorizar ciclos más largos de verde para reducir arrancadas/frenadas (menos tiempo en rojo acumulando vehículos encendidos).

Esto ya usa **todas** las entradas para *decidir*, no solo para *mostrar* — el salto real de nivel bajo a medio.

### 3.2 LCD como panel de estado
El LCD pasa de "anuncios rotativos" a mostrar el **modo de operación activo** y por qué se activó (ej. `MODO: CONGESTION` / `Via 1: 4/3 CNY`), más una pantalla con el estado de la conexión con el computador (sección 3.3). Sigue usando el mismo hardware, solo cambia el propósito: de informativo a **auto-conciencia visible** (el sistema comunica su propio estado).

### 3.3 Comunicación serial con el computador → internet (lo que pide explícitamente la rúbrica de nivel medio)

**Estado: implementado** en `proyecto/nivel_medio/nivel_medio.ino` + `proyecto/nivel_medio/puente_serial.py` (pendiente de compilar/probar en vivo, ver sección 3.4). Arquitectura realmente construida (inspirada en el patrón de `assets/code/wokwi/websocket-serial.py`, pero con protocolo propio en texto plano en vez de JSON sobre el cable, para no depender de una librería JSON en el ESP32):

```
ESP32 (Serial USB, 115200 baud, texto plano línea a línea)
   │  TX cada 1 s: "modo=CONGESTION fase=A dur=8.0 co2=612 ldr1=820 ldr2=750
   │                cny1=1 cny2=1 cny3=0 cny4=0 cny5=1 cny6=0 det=2 det_remoto=0
   │                p1=0 p2=1 peaton1_espera=0 peaton2_espera=1 lluvia=0 nocturno=0"
   │  RX: "PING" (responde "PONG") / "LLUVIA=1"/"LLUVIA=0" / "DET_REMOTO=<n>"
   ▼
puente_serial.py  (en el computador, pyserial + solo librerías estándar de Python)
   │  hilo_lector():  parsea la telemetría "clave=valor" → dict → la reenvía
   │                  como JSON por POST a ENDPOINT_TELEMETRIA (configurable,
   │                  None por defecto para no golpear el requestcatcher de
   │                  un tercero sin permiso) → y publica "det" en ntfy.sh
   │  hilo_clima():   cada 30 s consulta la API pública de Open-Meteo
   │                  (sin API key) para la ubicación configurada y, si
   │                  cambia el estado de lluvia, escribe "LLUVIA=1"/"LLUVIA=0"
   │  hilo_red():     cada 5 s revisa el tema de ntfy.sh compartido con la
   │                  OTRA maqueta; si publicó un conteo nuevo, escribe
   │                  "DET_REMOTO=<n>"
   │  hilo_ping():    cada 2 s escribe "PING" para que el LCD muestre
   │                  "PC: CONECTADO" (auto-conciencia de conectividad)
   ▼
Internet: Open-Meteo (clima real) + ntfy.sh (pub/sub gratis sin cuenta, canal
          con la otra maqueta) + endpoint propio opcional para telemetría
```

Qué información viaja en cada sentido (esto es lo que "amplifica la autoadaptabilidad", como pide la rúbrica):

- **ESP32 → PC → internet**: telemetría (modo activo, fase, duración aplicada, CNY, CO2, LDR, botones, espera peatonal, conteo local) que el puente convierte a JSON antes de reenviarla — mismo espíritu que `clase/sistemas-conectados.md` con Ubidots para la maqueta de clima, pero aplicado a la ciudad. El conteo local (`det`) también se publica en ntfy.sh para que lo vea la otra maqueta.
- **Internet → PC → ESP32**, dos fuentes distintas de información que el ESP32 no puede obtener con sus propios sensores:
  1. **Clima real** de la ubicación configurada (lluvia, vía Open-Meteo). Cuando el puente detecta lluvia, manda `LLUVIA=1` y `nivel_medio.ino` extiende `T_AMARILLO_BASE` en `BONUS_LLUVIA` (1 s) mientras dure.
  2. **Estado de la otra maqueta de ciudad**, en cualquier computador con internet (no necesita cable ni estar en el mismo lugar — a diferencia de `proyecto/puente_serial/`, que conecta las dos maquetas directo por USB solo para nivel bajo). Vía un tema de ntfy.sh: si la otra maqueta reporta congestión alta (`UMBRAL_CONGESTION_RED`), el puente manda `DET_REMOTO=<n>` y `nivel_medio.ino` extiende el verde en `BONUS_RED` (2 s), como si las dos intersecciones de la ciudad coordinaran su tráfico entre sí.

Esto es "auto-ajuste" + "conciencia del contexto" con variables externas, justificando por qué hace falta el puente serial-internet y no basta con los sensores propios de la maqueta ni con el puente USB directo de nivel bajo.

Esto se implementa como **Serial + PC**, no WiFi directo del ESP32 (que sería trivialmente igual al nivel bajo con más pasos) — es la diferencia que la rúbrica pide explícitamente frente al ejemplo de `esp_send_data.ino`/`esp_get_data.ino`.

### 3.4 Entregable de nivel medio

**Estado: código escrito, sin compilar ni probar todavía** (el canal de internet↔internet con ntfy.sh sí se probó de forma aislada, fuera del ESP32 — el publish/poll funciona).

- [x] `proyecto/nivel_medio/nivel_medio.ino` — MEF con SOM (congestión, ECO, nocturno, peatonal, lluvia, congestión de la otra maqueta) + protocolo Serial de la sección 3.3
- [x] `proyecto/nivel_medio/puente_serial.py` — puente Serial↔Internet (Open-Meteo + telemetría + coordinación con la otra maqueta vía ntfy.sh)
- [x] `proyecto/nivel_medio/README.md` — modos, cómo correr el puente (incluyendo con dos maquetas), protocolo Serial documentado
- [x] `proyecto/nivel_medio/diagram.json` / `wokwi.toml` — mismo cableado de `nivel_bajo`, puerto RFC2217 `4001` (distinto al `4000` de nivel bajo, para poder tener ambos simuladores abiertos a la vez en la demo comparativa)
- [ ] **Pendiente**: compilar con `arduino-cli` (no disponible en el entorno donde se escribió el código — hay que hacerlo localmente, comando en el README del proyecto), correr en Wokwi junto con `puente_serial.py` y confirmar que todos los modos (incluyendo la coordinación entre maquetas) se disparan como se documentó aquí y en el README

### 3.5 Correcciones tras revisión de código (importante)

Una segunda revisión del código (no solo del diseño) encontró dos defectos reales en `nivel_medio.ino`, ya corregidos:

1. **Polaridad de los sensores CNY invertida.** En la maqueta los CNY van a tierra con pull-up: en reposo (nada detectado) leen HIGH, y bajan a LOW al detectar un objeto. La primera versión del código sumaba `digitalRead()` crudo como "vehículos detectados", heredado de cómo `nivel_bajo` los usaba solo para *mostrar* un número (inofensivo ahí). Pero en nivel medio esa suma **decide** el modo congestión — con la polaridad al revés, una vía vacía (todos los CNY en HIGH) se leía como "3/3 congestionada" permanentemente, y el sistema nunca habría reaccionado de verdad al tráfico real. Se corrigió con un helper `vehiculoDetectado(pin) = (digitalRead(pin) == LOW)` usado en todo el código de control y en la telemetría.
2. **Conflicto sin resolver entre modo nocturno y petición peatonal.** El modo nocturno reemplazaba todo el ciclo normal (donde vive la lógica de `gestionarPeaton1/2`), así que un peatón que presionara el botón de noche quedaba completamente ignorado, sin ningún aviso. Se agregó una regla de prioridad explícita **peatonal > nocturno**: presionar P1/P2 durante la noche interrumpe el parpadeo, retoma el ciclo normal, y suspende el reingreso a modo nocturno por 20 s (un ciclo completo) para darle tiempo al peatón de cruzar. No hizo falta convertir esto en una máquina de modos formal — bastó una regla de prioridad puntual entre los dos únicos modos que de verdad se excluyen mutuamente (congestión/ECO/lluvia nunca chocan entre sí porque solo ajustan la duración dentro del ciclo normal, cada vía por separado).

Una prueba real en Wokwi (el semáforo nunca mostraba verde, los botones no parecían hacer nada) encontró un tercer problema, también corregido:

3. **Los potenciómetros de LDR1/LDR2/CO2 arrancan en 0 en Wokwi**, y con los umbrales del código eso dispara el modo nocturno **y** el modo ECO desde el primer instante de la simulación — el semáforo se queda parpadeando en amarillo para siempre (nunca llega a mostrar verde) hasta que alguien suba manualmente los tres potenciómetros. Esto explica el síntoma exacto de esa prueba: no es que el botón peatonal no funcione, es que el sistema arranca atascado en modo nocturno y ahí la lógica peatonal solo interviene a través de la regla de prioridad del punto 2 (que sí funciona, pero es fácil no notarla si no se sabe que está ahí). Se corrigió fijando un valor inicial del 80% en los tres potenciómetros en `proyecto/nivel_medio/diagram.json`, y documentando en el README que hay que subirlos manualmente si la versión de Wokwi de cada quien no respeta ese atributo. De paso se corrigió una instrucción del README que estaba al revés: por la fórmula del sensor, el modo ECO se activa **bajando** el potenciómetro de CO2, no subiéndolo.
4. **Constante de calibración explícita para la polaridad CNY.** Se agregó `#define CNY_ACTIVO LOW` al inicio de `nivel_medio.ino`, usada por `vehiculoDetectado()` en vez de un `LOW` implícito — así, al pasar a la maqueta física, si el módulo CNY70 real entrega el nivel contrario, el ajuste es cambiar una sola constante en vez de tocar la lógica de control en varios lugares.

## 4. Nivel alto — propuesta ("de alguna manera")

Como el curso no define nivel alto, hay que **elegir y justificar** una técnica. La primera versión de esta sección proponía Q-learning aprendiendo en vivo sobre el ESP32 **más** negociación multiagente entre las dos vías. Una segunda revisión (antes de escribir código) señaló que eso es demasiado alcance para lo que se puede mostrar en una demo de clase, y tiene razón — ver el porqué abajo. Esta es la versión recortada.

### 4.1 Por qué se recorta el alcance

- **El aprendizaje en vivo no se alcanza a ver en una demo de clase.** Un ciclo completo de semáforo (verde+amarillo de ambas vías) dura del orden de 14 s con los tiempos base de nivel medio. Una tabla Q necesita del orden de decenas a cientos de episodios para mostrar una mejora clara — eso son varios minutos a más de media hora de ejecución continua, más de lo que dura una presentación.
- **En Wokwi los CNY son botones que hay que sostener a mano.** No hay forma de simular tráfico automáticamente para generar esos episodios de entrenamiento sin que una persona esté presionando botones todo ese tiempo real.
- **Conclusión**: la tabla Q se **pre-entrena offline en Python**, contra un simulador de tráfico simple (no contra el hardware/Wokwi), y se carga ya entrenada en el ESP32. En la demo se muestra la política ya aprendida funcionando, más — si da tiempo — un modo con exploración activada y un factor de aceleración de tiempo para que se alcance a ver *algo* de ajuste en vivo.
- **La negociación multiagente entre las dos vías se retira como "nivel alto" implementado.** Dentro de un solo microcontrolador y un solo programa, dos vías "negociando" son, en la práctica, dos ramas de if/else con nombres bonitos — un evaluador puede verlo (con razón) como cosmético, no como una arquitectura multiagente real (que necesitaría procesos/hardware separados con paso de mensajes real). Baja a la sección 4.4 como propuesta de escalamiento, que es donde sí es defendible.

### 4.2 Aprendizaje por refuerzo tabular pre-entrenado — Generación 2

- **Estado** (por vía): congestión (conteo CNY 0–3, ya con la polaridad corregida — ver sección 3.5), si hay petición peatonal, si el CO2 está alto (modo eco). 4×2×2 = 16 estados por vía.
- **Acción**: duración del próximo verde para esa vía, elegida entre 3 valores discretos (3 s / 5 s / 8 s).
- **Recompensa**: positiva por vehículos que pasaron durante el verde (transición CNY detectado→libre), negativa por vehículos acumulados en la vía que quedó en rojo y por si un peatón tuvo que esperar.
- **Entrenamiento**: un script en Python (`simulador_trafico.py`) simula muchos ciclos de esta MEF con tráfico aleatorio (sin hardware ni Wokwi de por medio) y corre el ajuste de la tabla Q ahí, en segundos en vez de minutos/horas reales. El resultado (`tabla_q.h`, un arreglo de floats) se incluye directo en `nivel_alto.ino` como tabla ya entrenada — el ESP32 solo *ejecuta* la política aprendida (con algo de exploración residual si se quiere seguir afinando en vivo), no entrena desde cero en la demo.
- **Factor de aceleración para la demo**: una constante `ACELERACION` que divide todos los tiempos de fase (ej. de 14 s por ciclo a 3-4 s) para poder mostrar varios ciclos de decisión en poco tiempo frente a la clase, sin cambiar la lógica de decisión en sí.
- Esto sigue siendo Generación 2 (aprende de su propia experiencia y ajusta su comportamiento), solo que el "aprender" ocurre offline antes de la demo, no en vivo durante ella — una simplificación honesta y necesaria dado el hardware y el tiempo de clase disponibles, no una renuncia al concepto.

### 4.3 Entregable de nivel alto
- `proyecto/nivel_alto/nivel_alto.ino` (ejecuta la política Q pre-entrenada por vía + factor de aceleración para la demo, reutiliza el puente serial de nivel medio)
- `proyecto/nivel_alto/simulador_trafico.py` (entrena la tabla Q offline contra tráfico simulado, exporta `tabla_q.h`)
- `proyecto/nivel_alto/README.md` (qué aprendió la política, cómo se entrenó, tabla Q antes/después de entrenar)

### 4.4 Qué se necesitaría para llegar más lejos (para la sección de "propuestas" de la presentación, 10 pts)
Con esto se resuelve la sección "propuestas para llevar el sistema al nivel alto" que pide la rúbrica:
- **Negociación multiagente real entre las dos vías**, con hardware separado (dos ESP32, uno por semáforo) que se pasan mensajes de verdad (I2C o Serial entre ellos) en vez de compartir variables en un mismo programa — así "sin principio de organización predefinido" (Generación 4) deja de ser solo un nombre.
- **Más sensores por vía** (cámaras o más CNY) para un estado más rico que el conteo actual.
- **Entrenamiento contra un simulador de tráfico más realista** (SUMO o Webots — este último ya aparece listado en [`docs/software/index.md`](docs/software/index.md) sin explicación, es la pista más probable de qué esperaba el curso) en vez del simulador simplificado de `simulador_trafico.py`.
- **Red de intersecciones real**: extender el patrón de negociación multiagente (con hardware real, ver primer punto) a varios cruces, no solo dos vías de un mismo cruce.
- **Aprendizaje continuo en la nube**: usar el puente serial-internet no solo para clima, sino para subir el historial de recompensas a un servicio (Ubidots u otro) y re-entrenar periódicamente, cerrando el ciclo edge-cloud, en vez de entrenar una sola vez offline antes de la demo.

## 5. Plan de trabajo sugerido (equipo de 3)

| Persona | Entregable | Depende de |
|---|---|---|
| A | Nivel bajo (ya está) + limpieza de la inconsistencia de pines (sección 1.2) | — |
| B | Nivel medio: MEF con SOM (3.1–3.2) | Nivel bajo como base |
| B o C | Puente serial-PC-internet (3.3) | En paralelo a la MEF de nivel medio |
| C | Nivel alto: entrenar `simulador_trafico.py` offline y cargar la tabla Q en `nivel_alto.ino` (4.2) | Nivel medio funcionando (reutiliza el puente) |
| Todos | Presentación: comparación nivel a nivel + propuestas (sección 4.4) + demo en vivo del nivel alto | Todo lo anterior |

## 6. Checklist frente a la rúbrica

- [x] Nivel bajo implementado y usando todas las E/S (`proyecto/nivel_bajo/`)
- [x] Nivel medio: código escrito y corregido (SOM + serial-internet, secciones 3.4–3.5) — `proyecto/nivel_medio/` — **falta compilar y probar en vivo**
- [ ] Nivel alto: tabla Q pre-entrenada offline + política ejecutándose en el ESP32 (sección 4.2) — `proyecto/nivel_alto/` (por crear)
- [ ] Presentación con comparación de los 3 niveles
- [ ] Propuestas concretas para llevar el sistema más allá del nivel alcanzado (sección 4.4 ya da el contenido)
- [ ] Demo en vivo del nivel más alto alcanzado
- [ ] Resolver antes de programar: convención única de pines LDR1/LDR2 (sección 1.2, punto 2)
