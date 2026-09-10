# Guion de demostración y presentación

Cubre lo que pide la rúbrica de [`docs/evaluacion/entrega.md`](../docs/evaluacion/entrega.md): las tres implementaciones, las diferencias y mejoras de nivel a nivel, la demostración del nivel más alto alcanzado y las propuestas para ir más allá.

## Antes de entrar al salón

1. Con la maqueta física (medido el 2026-09-09): los botones son activos en HIGH (`P_ACTIVO HIGH` en los tres `.ino`), los CNY activos en LOW y el LCD es de 20x4. Los binarios de `placa/` ya están compilados así; los `code.bin` de Wokwi siguen con `P_ACTIVO LOW` porque el diagrama cablea los botones a tierra.
2. En `nivel_medio/puente_serial.py`, cambiar `TOPIC_RED` por un nombre propio del equipo. El mismo script sirve para los tres niveles que hablan serial (medio y alto).
3. Probar el puente con las dos maquetas a la vez y ver llegar `DET_REMOTO` de una a otra. Abrir `https://ntfy.sh/<TOPIC_RED>` en el navegador: es el canal en vivo entre las dos ciudades y sirve como pantalla durante la charla.
4. Si el nivel alto va a correr en la maqueta, dejarlo encendido un rato antes: la tabla que trae ya está entrenada, pero las decisiones que tome en el salón se van guardando en la flash y se pueden mostrar con `Q_DUMP`.
5. Plan B sin maqueta: los tres niveles corren en Wokwi (VS Code) con los `code.bin` del repo, uno por puerto (`4000`, `4001`, `4002`), y se pueden tener abiertos a la vez.

## Nivel bajo (2 min): Generación 0

Qué decir: un lazo cerrado con setpoints fijos. Mide todo, no decide nada.

Qué mostrar:
- El ciclo A/B/C/D siempre igual: verde 5 s, amarillo 2 s.
- El LCD rotando las cuatro pantallas con CO2, luz, vehículos y botones.
- Tapar un CNY o presionar un botón: el LCD lo refleja, el semáforo ni se entera. Ese es el punto.

## Nivel medio (4 min): Generación 1, modos de operación y puente a internet

Qué decir: los mismos sensores ahora cambian los setpoints (auto-ajuste) según el contexto (conciencia del contexto), mediante modos de operación. Y el sistema recibe y envía información de internet por el serial con el computador, que es lo que pide la rúbrica.

Qué mostrar, en este orden:
1. Demanda: tapar un CNY de la vía 1 y dejar la vía 2 vacía; la vía 1 se queda en verde (el LCD dice `DEMANDA`) hasta que se tape un CNY de la vía 2, y ahí cede el paso. Con un CNY tapado en cada vía, tapar dos de la vía 1 antes de su verde: el LCD dice `CONG1` y `dur:8.0s`.
2. Peatón: presionar P1 en cualquier momento; queda `(esperando)` y el verde de S1 se corta a los 2 s como mínimo.
3. Nocturno: tapar las dos LDR; ambos amarillos parpadean. Presionar un botón: el peatón tiene prioridad y el ciclo vuelve 20 s.
4. Internet: con el puente corriendo, el LCD dice `PC: CONECTADO`; el clima real de Medellín llega como `LLUVIA=1` y alarga el amarillo. Tapar cuatro CNY en la otra maqueta: en esta llega `DET_REMOTO` y el verde se alarga 2 s aunque acá no haya tráfico. Enseñar el canal de ntfy.sh en el navegador.

Frase para el jurado: nivel bajo mide, nivel medio decide con reglas que nosotros escribimos.

## Nivel alto (6 min): Generación 2, el sistema aprende de su experiencia

Qué decir: ya nadie escribe la regla del verde. Un agente por vía observa el tráfico, prueba, mide el resultado y corrige su tabla. Las reglas de seguridad (amarillo, verde mínimo, peatón, nocturno) quedan por encima del agente a propósito: eso es auto-regulación. Y el sistema muestra lo que sabe y por qué decide: autoconciencia.

Qué mostrar:
1. Pantalla 0 del LCD: `AGENTE VIA 1 #n`, el estado que ve (`cola 2 otra 0`), sus tres valores Q y la decisión (`verde 8s explota r-3.1`).
2. Tapar los tres CNY de la vía 1: elige 8 s. Soltarlos y tapar los de la vía 2: la vía 1 pasa a 3 s y la vía 2 a 8 s. Nadie le escribió esa regla.
3. Soltar un CNY durante el verde: cuenta como un vehículo que pasó y sube la recompensa.
4. Por el serial (monitor de Wokwi o `puente_serial.py`): `EPSILON=0.5` para que explore a la vista (`explora` en el LCD), `Q_DUMP` para ver la tabla, `Q_SAVE` para guardarla en flash. Apagar y prender la maqueta: `nvs=1`, la experiencia sobrevivió.
5. La comparación con números, de [`nivel_alto/entrenamiento.md`](nivel_alto/entrenamiento.md): contra el mismo tráfico simulado, el agente iguala en promedio a las reglas de nivel medio (7.3 s frente a 7.4 s de espera por vehículo) y las mejora en las horas pico desbalanceadas (8.3 frente a 9.3 s). Sin entrenar daba 9.1 s. Decir cómo se entrenó: el mismo código del ESP32 corriendo en el computador contra un modelo de tráfico, medio millón de decisiones en un minuto.

Si preguntan por qué no aprende desde cero en el salón: un ciclo dura 10 a 20 s y hacen falta miles de decisiones; por eso se entrena offline y en el salón se muestra que sigue afinando (con `EPSILON` alto se ven cambios en los valores Q en pocos minutos).

## Diferencias de nivel a nivel (una diapositiva)

| | Nivel bajo | Nivel medio | Nivel alto |
|---|---|---|---|
| Generación CPS | 0 | 1 | 2 |
| Setpoints | Fijos | Cambian por reglas según contexto | Los aprende el agente |
| Sensores | Solo se muestran | Deciden (modos) | Deciden y además miden el resultado (recompensa) |
| Internet | No | Recibe clima y la otra maqueta, envía su conteo | Igual que medio |
| Lo que muestra el LCD | Lecturas | Modo activo y por qué | Lo que sabe, lo que eligió y cuánto le rindió |
| Espera media por vehículo (sim.) | igual a medio sin bonos | 7.4 s | 7.3 s, mejor en horas pico |
| Qué pasa si se reinicia | Nada que perder | Nada que perder | Recupera lo aprendido de la flash |

## Propuestas para llevar el sistema más allá (10 pts)

1. **Dos maquetas como agentes que negocian por radio (ESP-NOW).** Hoy las dos ciudades se coordinan por internet a través de los computadores. Con ESP-NOW los dos ESP32-S3 se pasan mensajes directo, sin router ni computador, a decenas de metros: cada intersección publica su cola y su próxima decisión, y la otra la incorpora a su estado. Ahí "sistema de sistemas" deja de ser una frase: dos agentes con hardware propio y paso de mensajes real (Generación 3 y 4: estrategia propia, auto-organización). Es la extensión natural de lo que ya existe y las dos maquetas físicas ya están.
2. **Reentrenar en la nube.** El puente ya sube la telemetría; subir también las recompensas, reentrenar la tabla en el servidor con el tráfico real acumulado y bajarla por el mismo serial. Cierra el ciclo edge-cloud.
3. **Estado más rico.** Hora del día y día de la semana (el ESP32 puede pedirlos por el puente), y más sensores por vía para contar en vez de detectar presencia: el límite que más le cuesta al agente hoy es que tres CNY no distinguen una cola de 3 de una de 10.
4. **Entrenar contra un simulador de tráfico real** (SUMO), en vez del modelo de colas simple de `proyecto/sim/trafico.h`, y con una red de varias intersecciones.
5. **Aprendizaje más allá de una tabla**: con más estados hace falta aproximar Q (una red pequeña cabe en el ESP32-S3), y coordinar varios agentes con un objetivo global en vez de recompensas locales.

## Preguntas que pueden hacer

- *¿Por qué el nivel medio no "planea secuencias" como dice la página del curso?* Porque los modos de operación son exactamente la definición de Generación 1 del curso y la rúbrica de nivel medio pide eso más el puente serial; la planeación de secuencias es lo que el agente de nivel alto empieza a hacer al elegir la duración de cada verde mirando ambas colas.
- *¿El agente puede hacer algo peligroso?* No: amarillo, verde mínimo, prioridad peatonal y nocturno son reglas fijas fuera de su alcance. Solo elige entre 3, 5 y 8 s de verde.
- *¿Cómo saben que funciona sin la maqueta?* Tres capas: un arnés nativo con 30 escenarios (`proyecto/sim/run.sh`), guiones automatizados sobre el simulador Wokwi con el binario real (`proyecto/sim/wokwi/run_wokwi.sh`), y la maqueta física.
