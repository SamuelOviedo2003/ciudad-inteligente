# Nivel alto

CPS Generación 2: la misma maqueta y las mismas reglas de seguridad de `nivel_medio`, pero la duración del verde de cada vía ya no la fija una regla escrita a mano. La elige un **agente de aprendizaje por refuerzo** (Q-learning tabular, uno por vía) que observa el tráfico, mide el resultado de cada decisión y corrige su propia tabla. El sistema aprende de su experiencia y muestra en el LCD qué sabe y por qué decide, que es lo que [`docs/concepto/cps-nivel-bajo.md`](../../docs/concepto/cps-nivel-bajo.md) llama autoconciencia y autoadaptación en la Generación 2.

## Qué decide el agente y qué no

| | Lo decide el agente | Regla fija (self-regulation) |
|---|---|---|
| Verde de cada vía | Sí: 3, 5 u 8 s según lo aprendido | |
| Amarillo | | 2 s, 3 s con lluvia real (dato de internet) |
| Verde mínimo y espera peatonal | | 2 s mínimo; el peatón cruza a los 6 s como mucho |
| Modo nocturno | | Ambos LDR oscuros, con histéresis |
| Coordinación con la otra maqueta | | +2 s de verde si la otra reporta congestión (`DET_REMOTO`) |

Las reglas fijas están por encima del agente a propósito: son los límites que el sistema no cruza aunque la recompensa lo tiente. Eso también es parte de la definición del curso (auto-regulación: "definir objetivos, identificar riesgos y ajustar el comportamiento sin perder consistencia").

## Cómo aprende

- **Estado** (32 por vía): cola propia (0 a 3 CNY detectados) × cola de la otra vía (0 a 3) × CO2 alto (sí/no).
- **Acción** (3): duración del próximo verde de esa vía, 3, 5 u 8 s.
- **Paso**: desde que empieza el verde de la vía hasta que vuelve a empezar (un ciclo completo).
- **Recompensa** del paso, normalizada a 10 s porque los ciclos duran distinto según la acción: 0.5 por cada vehículo que salió durante el verde propio (transición detectado → libre en sus CNY), menos 0.1 por cada vehículo-segundo de espera visible en las dos vías durante todo el paso (lo que un semáforo quiere minimizar), menos 0.1 por cada segundo de verde propio con la vía vacía. Sin la normalización el agente aprendía que los ciclos cortos "cuestan menos" solo por ser cortos.
- **Actualización**: Q-learning clásico, `Q[s][a] += α·(r + γ·max Q[s'] − Q[s][a])`, con α = 0.1 y γ = 0.8, cerrada cuando la misma vía vuelve a decidir (ahí se conoce el estado siguiente). Exploración ε-greedy con ε = 0.1: una de cada diez decisiones prueba una acción distinta a la mejor conocida, y el LCD lo anuncia ("explora").
- **Tabla inicial**: `tabla_q.h`, entrenada offline por `proyecto/sim/entrenar.sh` con **este mismo código** corriendo en el PC contra un modelo de tráfico (3.000 episodios de 20 min, cinco patrones de tráfico, α bajando de 0.2 a 0.01, ε de 0.3 a 0.05, más de 500.000 decisiones). El ESP32 arranca sabiendo y sigue afinando en vivo. Si `tabla_q.h` no existe, arranca con una heurística suave.

## Qué aprendió y cómo se compara

Política aprendida (verde en segundos; filas = cola propia, columnas = cola de la otra vía), la misma para las dos vías:

```
propia 0 | 3 3 3 3
propia 1 | 3 3 3 3
propia 2 | 3 5 3 3
propia 3 | 8 8 8 8
```

Nadie le escribió esa regla: salió de la recompensa. Comparación con el mismo tráfico (detalle y metodología en [`entrenamiento.md`](entrenamiento.md)), espera media por vehículo en segundos:

| Patrón | nivel medio (reglas a mano) | nivel alto entrenado |
|---|---|---|
| hora pico vía 1 | 9.3 | 8.3 |
| hora pico vía 2 | 10.0 | 9.5 |
| ambas cargadas | 9.2 | 9.6 |
| poco tráfico | 3.6 | 3.9 |
| promedio | 7.4 | 7.3 |

El agente iguala en promedio a las reglas diseñadas a mano y las mejora en las horas pico desbalanceadas, que es donde importa; en tráfico liviano las diferencias son de décimas. La tabla heurística sin entrenar da 9.1 s, así que el entrenamiento sí es lo que lo lleva ahí. Lo que no aprende el agente son los límites de seguridad, que siguen siendo reglas.

## Cómo correrlo

Igual que nivel medio: abrir `diagram.json` con la extensión Wokwi (puerto RFC2217 `4002`) o cargar la placa. El puente `../nivel_medio/puente_serial.py` sirve tal cual (mismo protocolo, misma telemetría más los campos del agente); pasarle `rfc2217://localhost:4002`.

`ACELERACION` en el código divide todos los tiempos del semáforo para la demo (con 2, un ciclo dura la mitad y se ven más decisiones por minuto) sin tocar la lógica. Los binarios del repo están con 1.

## Qué debería pasar

- Con las vías vacías, el agente elige el verde corto (3 s); la recompensa sale negativa (verde desperdiciado) y se ve en el LCD y en la telemetría.
- Sostener los tres CNY de una vía: esa vía elige 8 s; la otra, viendo tres esperando enfrente, elige 3 s.
- Soltar un CNY durante el verde cuenta como un vehículo que pasó y sube la recompensa.
- Pantalla 0 del LCD: `AGENTE VIA n #decisiones` / `cola x otra y [eco]` / los tres valores Q del estado actual / `verde Ns explota|explora r±`.
- Telemetría: la de nivel medio más `nivel=alto nvs= eps= s1= a1= explora1= r1= rtotal1= q1=a/b/c` (y lo mismo para la vía 2). `nvs=1` significa que la tabla se cargó de la flash al arrancar.
- Peatón, nocturno, lluvia y `DET_REMOTO` se comportan exactamente como en nivel medio.

## Memoria de la experiencia y comandos para la demo

La tabla Q se guarda en la memoria no volátil del ESP32 (NVS, vía `Preferences`) cada 10 decisiones, así que lo aprendido sobrevive a un reinicio o a desconectar la maqueta. Por el mismo serial del puente (o el monitor de Wokwi) se aceptan, además de `PING`, `LLUVIA=` y `DET_REMOTO=`:

| Comando | Efecto |
|---|---|
| `Q_DUMP` | Vuelca la tabla: 64 líneas `Q via estado q3 q5 q8` y `Q fin` |
| `Q_SAVE` | Guarda la tabla en flash ya; responde `Q_SAVE ok 768` (bytes escritos) |
| `Q_RESET` | Borra la flash y vuelve a la tabla entrenada de `tabla_q.h`; responde `Q_RESET ok` |
| `EPSILON=0.5` | Cambia cuánto explora (0 a 1). Subirlo durante la demo hace visible el aprendizaje; con 0 solo explota lo aprendido |

## Archivos

- `nivel_alto.ino` — código fuente del ESP32
- `tabla_q.h` — tabla Q entrenada offline (generada por `proyecto/sim/entrenar.sh`, no editar a mano)
- `entrenamiento.md` — resultados del entrenamiento y la comparación con nivel medio (generado)
- `code.bin`, `code.elf` — compilados sin `CDCOnBoot` (variante Wokwi; para la placa ver el README raíz)
- `diagram.json`, `wokwi.toml` — mismo cableado que los otros niveles, puerto `4002`

Pruebas: `proyecto/sim/run.sh` (escenarios `sim_alto`: tabla inicial, recompensa, actualización Q, exploración, reglas fijas por encima del agente, memoria en flash y comandos, telemetría y LCD) y `proyecto/sim/wokwi/run_wokwi.sh` (guion `alto_agente.yaml` contra el simulador real, incluida la escritura de la tabla en la flash simulada).
