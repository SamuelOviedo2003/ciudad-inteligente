# Entrenamiento y comparación de niveles

Generado por `proyecto/sim/entrenar.sh`. Los tres niveles corren el **mismo tráfico** (mismas semillas, mismos patrones, 20 min simulados por corrida) en el arnés nativo, con el modelo de `proyecto/sim/trafico.h`: llegadas Poisson por vía, un vehículo sale cada 1.5 s de verde, los CNY muestran presencia (mín(cola, 3)), y un peatón pulsa cada botón cada 2 min en promedio. Nivel bajo no aparece: sus tiempos son fijos y coinciden con nivel medio sin ningún bono.

## Espera media por vehículo (s, menos es mejor)

| Patrón | nivel medio | nivel alto (tabla heuristica) | nivel alto (tabla entrenada) |
|---|---|---|---|
| poco trafico | 3.6 | 3.5 | 3.9 |
| hora pico via 1 | 9.3 | 13.6 | 8.3 |
| hora pico via 2 | 10.0 | 12.3 | 9.5 |
| ambas cargadas | 9.2 | 11.0 | 9.6 |
| desbalance suave | 5.1 | 5.1 | 5.5 |
| **promedio** | **7.4** | **9.1** | **7.3** |

## Cola media (vehículos esperando en las dos vías)

| Patrón | nivel medio | nivel alto (tabla heuristica) | nivel alto (tabla entrenada) |
|---|---|---|---|
| poco trafico | 0.29 | 0.28 | 0.30 |
| hora pico via 1 | 2.91 | 3.90 | 2.59 |
| hora pico via 2 | 3.19 | 3.62 | 3.05 |
| ambas cargadas | 3.89 | 4.53 | 3.93 |
| desbalance suave | 1.10 | 1.12 | 1.27 |
| **promedio** | **2.28** | **2.69** | **2.23** |

## Espera media del peatón (s) y vehículos atendidos por corrida

| Nivel | Espera peatón | Atendidos | Descartados (cola llena) |
|---|---|---|---|
| nivel medio | 1.5 | 324 | 12.6 |
| nivel alto (tabla heuristica) | 1.5 | 310 | 28.3 |
| nivel alto (tabla entrenada) | 1.9 | 324 | 13.2 |

## Política aprendida

```
Politica aprendida (verde en s; filas = cola propia 0..3, columnas = cola de la otra via 0..3)
  sin CO2 alto:
    propia 0 | 3 3 3 3
    propia 1 | 3 3 3 3
    propia 2 | 3 5 3 3
    propia 3 | 8 8 8 8
  con CO2 alto:
    propia 0 | 3 3 3 3
    propia 1 | 3 3 3 3
    propia 2 | 5 3 3 5
    propia 3 | 8 8 8 8
Cobertura (sin eco): 256084 decisiones, minimo 207 visitas en un par (estado, accion), 0 de 96 pares con menos de 20 visitas
Valores Q (sin eco) en estados clave [3 s, 5 s, 8 s]:
  propia 0 ajena 0: -5.34 -5.46 -5.84
  propia 3 ajena 0: -8.89 -8.50 -7.58
  propia 0 ajena 3: -10.24 -11.18 -11.60
  propia 3 ajena 3: -14.30 -13.73 -11.97
  propia 1 ajena 0: -5.10 -5.60 -5.69
  propia 0 ajena 1: -6.69 -7.52 -7.90
  PASS cola propia 3, ajena 0 -> verde largo (8 s)
  PASS cola propia 0, ajena 3 -> verde corto (3 s)
  PASS vias vacias -> verde corto (3 s)
  PASS ambas llenas -> no elige el verde corto
Tabla escrita en ../nivel_alto/tabla_q.h (514077 decisiones de entrenamiento)
```
