# Nivel bajo

Semáforo de tiempos fijos (CPS generación 0: siempre la misma secuencia, no reacciona a los sensores) + LCD con anuncios rotativos que usan todas las entradas de la maqueta.

## Cómo correrlo

Abrir `diagram.json` en VS Code (extensión Wokwi) → *Start Simulation*.

## Qué debería pasar

**Semáforos** — ciclo de 4 fases, se repite solo, sin parar:

| Fase | Duración | Semáforo 1 | Semáforo 2 |
|---|---|---|---|
| A | 5 s | Verde | Rojo |
| B | 2 s | Amarillo | Rojo |
| C | 5 s | Rojo | Verde |
| D | 2 s | Rojo | Amarillo |

Nunca dos luces del mismo semáforo encendidas a la vez. Nunca ambos en verde a la vez.

**LCD** — cambia de anuncio cada 3 s, en este orden, y cada uno se actualiza en vivo (cada 0.3 s) mientras está en pantalla:

1. `PONGASE TAPABOCAS` / `CO2: __ ppm`
2. `LUZ AMBIENTE` / valores de LDR1 y LDR2 (0–4095, mover los potenciómetros del simulador)
3. `VEHICULOS EN VIA` / `Detectados: _/6` (clic sostenido en los sensores CNY del simulador baja el contador)
4. `BOTON PEATONAL` / `P1: SI|NO`, `P2: SI|NO` (mantener presionado el botón correspondiente; `#define P_ACTIVO` en el código dice qué nivel cuenta como presionado: HIGH en la maqueta física (medido, pull-down externo), LOW en Wokwi (botones a tierra con pull-up interno))

**Lo que NO debe pasar**: tocar sensores o botones no debe alterar la secuencia ni los tiempos del semáforo — solo se refleja en el LCD.

## Archivos

- `nivel_bajo.ino` — código fuente
- `code.bin`, `code.elf` — compilados para `esp32:esp32:esp32s3` sin `CDCOnBoot` (la variante para Wokwi; para la placa física ver el README raíz)
- `diagram.json`, `wokwi.toml` — configuración del simulador
