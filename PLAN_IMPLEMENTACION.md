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

Arquitectura (basada en el patrón ya usado por `assets/code/wokwi/websocket-serial.py`, adaptado):

```
ESP32 (USB-Serial, 115200 baud, líneas JSON)
   │  TX: {"modo":"CONGESTION","cny":[1,1,0,0,1,0],"co2":612,"ldr":[820,750]}
   │  RX: {"cmd":"SET_MODO","valor":"ECO"} / {"cmd":"CLIMA","lluvia":true}
   ▼
puente_serial.py  (en el computador, pyserial)
   │  reenvía telemetría → POST a un endpoint HTTP/WebSocket
   │  consulta una fuente externa periódicamente → escribe comandos de vuelta al ESP32
   ▼
Internet: endpoint de prueba (isa.requestcatcher.com o un servidor propio) +
          fuente externa real (ej. API pública de clima tipo Open-Meteo, sin API key)
```

Qué información viaja en cada sentido (esto es lo que "amplifica la autoadaptabilidad", como pide la rúbrica):

- **ESP32 → PC → internet**: telemetría (modo activo, conteo de vehículos, CO2, luz ambiente) para un dashboard/log remoto — igual en espíritu a lo que hace `clase/sistemas-conectados.md` con Ubidots para la maqueta de clima, pero aplicado a la ciudad.
- **Internet → PC → ESP32**: datos que el ESP32 no puede medir por sí mismo y que cambian su comportamiento — el más simple y defendible es **clima real de la ciudad** (lluvia/visibilidad) desde una API pública gratuita: si llueve, el puente le manda `{"cmd":"CLIMA","lluvia":true}` y el ESP32 extiende el tiempo de amarillo y el tiempo de cruce peatonal (frenado más largo en piso mojado). Esto es exactamente "auto-ajuste" + "conciencia del contexto" con una variable que **no está disponible localmente en la maqueta**, justificando por qué hace falta el puente serial-internet y no basta con los sensores propios.

Esto se implementa como **Serial + PC**, no WiFi directo del ESP32 (que sería trivialmente igual al nivel bajo con más pasos) — es la diferencia que la rúbrica pide explícitamente frente al ejemplo de `esp_send_data.ino`/`esp_get_data.ino`.

### 3.4 Entregable de nivel medio
- `proyecto/nivel_medio/nivel_medio.ino` (MEF extendida con SOM + lectura/escritura por `Serial`)
- `proyecto/nivel_medio/puente_serial.py` (script en el computador)
- `proyecto/nivel_medio/README.md` (qué modos existen, cómo se disparan, cómo correr el puente)

## 4. Nivel alto — propuesta ("de alguna manera")

Como el curso no define nivel alto, hay que **elegir y justificar** una técnica. Con ESP32-S3 (suficiente RAM/CPU para tablas pequeñas, sin necesitar aceleración ML dedicada) y el hardware disponible, propongo dos mejoras combinables, ambas defendibles como Generación 2–4 ([`docs/concepto/cps-nivel-bajo.md`](docs/concepto/cps-nivel-bajo.md)):

### 4.1 Aprendizaje por refuerzo tabular (Q-learning) — Generación 2
- **Estado**: ocupación agregada por vía (conteo CNY 0–3), franja horaria/luz (LDR), si hay petición peatonal.
- **Acción**: duración del próximo verde para la vía activa (elegir entre 3–4 valores discretos, ej. 3 s / 5 s / 8 s).
- **Recompensa**: negativa por vehículos esperando (CNY de la vía en rojo) y por espera peatonal larga; positiva por throughput (vehículos que pasan, aproximado por transiciones CNY de detectado→libre).
- El sistema **aprende de su propia experiencia** (ver [`docs/concepto/cps-nivel-bajo.md`](docs/concepto/cps-nivel-bajo.md), Generación 2) ajustando la tabla Q en cada ciclo, en vez de tener reglas fijas como en nivel medio.
- La tabla Q se puede **persistir y refinar** aprovechando el puente serial del nivel medio: el ESP32 exporta episodios/recompensas por serial, un script en Python (`entrenador.py`) hace el ajuste (o entrena offline con más cómputo) y devuelve una tabla Q actualizada — así el nivel alto **reutiliza y extiende** la infraestructura del nivel medio en vez de descartarla, lo cual es un argumento fuerte para la presentación ("de nivel medio a alto: la nube deja de ser un simple canal de datos y pasa a ser parte del ciclo de aprendizaje").

### 4.2 Coordinación multiagente entre los dos semáforos — Generación 3/4
- Actualmente los dos semáforos son controlados por una sola MEF central. Para nivel alto, modelarlos como **dos agentes** (uno por semáforo) que negocian localmente: cada uno "ofrece" su nivel de congestión (conteo CNY) al otro (por variable compartida en el mismo microcontrolador, o por I2C/Serial si se dividen en dos ESP32) y el que tiene mayor congestión gana la extensión de verde, sin una regla central fija que decida — esto es **auto-organización sin principio predefinido** (Generación 4 en su forma más simple) y da pie a una propuesta de escalamiento en la presentación: con más cruces, el patrón se generaliza a una red de agentes que negocian entre sí (sistema de sistemas real).

### 4.3 Qué se necesitaría para llegar más lejos (para la sección de "propuestas" de la presentación, 10 pts)
Con esto ya se resuelve la sección "propuestas para llevar el sistema al nivel alto" que pide la rúbrica:
- **Más sensores por vía** (cámaras o más CNY) para un estado más rico que el conteo binario actual.
- **Modelo entrenado offline** (aprendizaje por refuerzo profundo o un modelo de tráfico simulado en SUMO/Webots — este último ya aparece listado en [`docs/software/index.md`](docs/software/index.md) sin explicación, es la pista más probable de qué esperaba el curso) para pretrain antes de desplegar en el ESP32.
- **Red de intersecciones real**: replicar el patrón de negociación multiagente entre varios cruces (sistema de sistemas), no solo dos semáforos de un mismo cruce.
- **Aprendizaje continuo en la nube**: usar el puente serial-internet no solo para clima, sino para subir el historial de recompensas a un servicio (Ubidots u otro) y re-entrenar periódicamente, cerrando el ciclo edge-cloud.

### 4.4 Entregable de nivel alto
- `proyecto/nivel_alto/nivel_alto.ino` (Q-learning tabular + negociación entre las dos vías + reutiliza el puente serial de nivel medio)
- `proyecto/nivel_alto/entrenador.py` (opcional: ajusta/exporta la tabla Q offline)
- `proyecto/nivel_alto/README.md` (qué aprende el sistema, cómo verlo aprender en la demo, tabla Q inicial vs. tabla Q tras N ciclos)

## 5. Plan de trabajo sugerido (equipo de 3)

| Persona | Entregable | Depende de |
|---|---|---|
| A | Nivel bajo (ya está) + limpieza de la inconsistencia de pines (sección 1.2) | — |
| B | Nivel medio: MEF con SOM (3.1–3.2) | Nivel bajo como base |
| B o C | Puente serial-PC-internet (3.3) | En paralelo a la MEF de nivel medio |
| C | Nivel alto: Q-learning + negociación (4.1–4.2) | Nivel medio funcionando (reutiliza el puente) |
| Todos | Presentación: comparación nivel a nivel + propuestas (sección 4.3) + demo en vivo del nivel alto | Todo lo anterior |

## 6. Checklist frente a la rúbrica

- [x] Nivel bajo implementado y usando todas las E/S (`proyecto/nivel_bajo/`)
- [ ] Nivel medio implementado (SOM + serial-internet) — `proyecto/nivel_medio/` (por crear)
- [ ] Nivel alto implementado (Q-learning + negociación) — `proyecto/nivel_alto/` (por crear)
- [ ] Presentación con comparación de los 3 niveles
- [ ] Propuestas concretas para llevar el sistema más allá del nivel alcanzado (sección 4.3 ya da el contenido)
- [ ] Demo en vivo del nivel más alto alcanzado
- [ ] Resolver antes de programar: convención única de pines LDR1/LDR2 (sección 1.2, punto 2)
