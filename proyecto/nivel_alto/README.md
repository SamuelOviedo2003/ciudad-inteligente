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
- **Recompensa**, al terminar el verde: vehículos que pasaron (cada transición detectado → libre en los CNY propios cuenta uno) menos 0.3 por cada segundo de verde con la vía vacía menos 0.4 por cada vehículo que quedó esperando en la otra vía.
- **Actualización**: Q-learning clásico, `Q[s][a] += α·(r + γ·max Q[s'] − Q[s][a])`, con α = 0.1 y γ = 0.8, cerrada cuando la misma vía vuelve a decidir (ahí se conoce el estado siguiente). Exploración ε-greedy con ε = 0.1: una de cada diez decisiones prueba una acción distinta a la mejor conocida, y el LCD lo anuncia ("explora").
- **Tabla inicial**: una heurística suave (verde más largo cuanto más cola propia, más corto cuanto más cola ajena) para que arranque razonable. El aprendizaje la va reemplazando. La tabla entrenada offline (`tabla_q.h`) y la comparación con nivel medio llegan en el siguiente paso del plan.

## Cómo correrlo

Igual que nivel medio: abrir `diagram.json` con la extensión Wokwi (puerto RFC2217 `4002`) o cargar la placa. El puente `../nivel_medio/puente_serial.py` sirve tal cual (mismo protocolo, misma telemetría más los campos del agente); pasarle `rfc2217://localhost:4002`.

`ACELERACION` en el código divide todos los tiempos del semáforo para la demo (con 2, un ciclo dura la mitad y se ven más decisiones por minuto) sin tocar la lógica. Los binarios del repo están con 1.

## Qué debería pasar

- Con las vías vacías, el agente elige el verde corto (3 s) y la recompensa sale negativa (verde desperdiciado): el valor de esa acción baja ciclo a ciclo y se ve en el LCD y en la telemetría.
- Sostener los tres CNY de una vía: esa vía elige 8 s; la otra, viendo tres esperando enfrente, elige 3 s.
- Soltar un CNY durante el verde cuenta como un vehículo que pasó y sube la recompensa.
- Pantalla 0 del LCD: `AGENTE VIA n #decisiones` / `cola x otra y [eco]` / los tres valores Q del estado actual / `verde Ns explota|explora r±`.
- Telemetría: la de nivel medio más `nivel=alto s1= a1= explora1= r1= rtotal1= q1=a/b/c` (y lo mismo para la vía 2).
- Peatón, nocturno, lluvia y `DET_REMOTO` se comportan exactamente como en nivel medio.

## Archivos

- `nivel_alto.ino` — código fuente del ESP32
- `code.bin`, `code.elf` — compilados sin `CDCOnBoot` (variante Wokwi; para la placa ver el README raíz)
- `diagram.json`, `wokwi.toml` — mismo cableado que los otros niveles, puerto `4002`

Pruebas: `proyecto/sim/run.sh` (escenarios `sim_alto`: tabla inicial, recompensa, actualización Q, exploración, reglas fijas por encima del agente, telemetría y LCD).
