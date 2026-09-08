---
source: (índice local, generado a partir de https://isa262.davinsony.com)
title: Índice del sitio ISA262 (mirror local)
---

# Mapa del sitio — Ingeniería de Sistemas Autoadaptables (ISA262)

Copia local, literal, del sitio del curso [isa262.davinsony.com](https://isa262.davinsony.com), organizada para preparar la entrega final del proyecto "ciudad inteligente" (maqueta de tráfico urbano con niveles bajo/medio/alto de complejidad CPS).

Cada archivo `.md` incluye un frontmatter con `source:` (URL exacta de origen) y `title:`. Los enlaces internos entre páginas del sitio se reescribieron como rutas relativas a los `.md` locales. Los recursos descargables (código, diapositivas, PDFs, esquemáticos) están en [`../assets/`](../assets/).

Ver también [GAPS.md](GAPS.md) — temas de la rúbrica que no están cubiertos en detalle en el sitio y habrá que investigar aparte.

## Página principal

- [Sistemas Autoadaptables](index.md) — `/`

## Evaluación

- [Evaluación](evaluacion/index.md) — `/evaluacion/` (tabla de pesos: Seguimiento 30%, CPS nivel medio 25%, Monitoreo climático en red 15%, Entrega Final 30%)
- [Monitoreo climático en red](evaluacion/monitoreo.md) — `/evaluacion/monitoreo/` (enunciado del taller de sistema maestro/esclavo)
- [Entrega Final](evaluacion/entrega.md) — `/evaluacion/entrega/` (requisitos y rúbrica del proyecto final)
- [Solución (taller de monitoreo)](solucion/taller-solucion.md) — `/solucion/taller-solucion/` — **no estaba en la lista original pero se encontró enlazable desde el sitemap del sitio**; contiene una posible solución (MEF maestro/esclavo) al taller de `evaluacion/monitoreo.md`

## Talleres (resumen de cada sesión + diapositivas)

- [Sistemas discretos](taller/discretos.md) — `/taller/discretos/`
- [Sistemas conectados](taller/conectados.md) — `/taller/conectados/`
- [Sistemas continuos](taller/continuos.md) — `/taller/continuos/`

### Diapositivas de clase (contenido completo extraído de las presentaciones Jupyter/Reveal.js enlazadas desde los talleres)

- [Diapositivas: Sistemas discretos](clase/sistemas-discretos.md) — Arduino vs ESP32, variables, operadores, sensores analógicos/digitales/especializados, teoría formal de Máquinas de Estados Finitos (autómata, ejemplos secador de manos y torniquete)
- [Diapositivas: Sistemas conectados](clase/sistemas-conectados.md) — tiempo relativo (`millis()`/`micros()`), MEF con temporizador, MEF de seguridad de batería y variante SOS, estación meteorológica IoT (Ubidots, request catcher)
- [Diapositivas: Sistemas continuos](clase/sistemas-continuos.md) — realimentación, función de transferencia, modelado masa-resorte-amortiguador, modelado experimental de la planta de iluminación (LED+LDR en Arduino Mega), identificación de parámetros, diseño de controlador PI

## Sesión 1 — Sistemas discretos (18 de agosto)

- [Preparación de sesión 1](discreto/index.md) — `/discreto/`
- [Arduino](discreto/arduino.md) — `/discreto/arduino/`
- [Expressif ESP](discreto/esp32.md) — `/discreto/esp32/`
- [Sintaxis en Arduino IDE](discreto/sintaxis.md) — `/discreto/sintaxis/`
- [Máquinas de Estados Finitos](discreto/mef.md) — `/discreto/mef/`

## Sesión 2 — Sistemas conectados (25 de agosto)

- [Preparación de sesión 2](conectado/index.md) — `/conectado/`

## Sesión 3 — Sistemas continuos (27 de agosto)

- [Preparación de sesión 3](continuo/index.md) — `/continuo/`

## Software

- [Requerimientos en Software](software/index.md) — `/software/`
- [Configuración: Placas ESP32](software/esp.md) — `/software/esp/`
- [Configuración: Simulador Wokwi](software/wokwi.md) — `/software/wokwi/` — **clave para trabajar sin la maqueta física**: instalación de la extensión Wokwi en VS Code, licencia, estructura de archivos (`wokwi.toml`, `diagram.json`, `code.hex`, `code.elf`) y proyecto de prueba (Blink)
- [Extracción de archivos HEX/BIN & ELF](software/hexelf.md) — `/software/hexelf/`
- [Librerías para el Arduino IDE](software/librerias.md) — `/software/librerias/`
- [TimerMEF.h](software/timer-mef.md) — `/software/timer-mef/` — librería de temporización no bloqueante usada en toda la maqueta

## Hardware (pines y conexiones de cada maqueta)

- [Ciudad Autoadaptable](hardware/ciudad.md) — `/hardware/ciudad/` — **la maqueta del proyecto final**: pines de 2 semáforos (LDR, botones, 6 sensores infrarrojos CNY, LEDs rojo/amarillo/verde), sensor CO2, pantalla LCD I2C 16x4; enlace al proyecto Wokwi en línea
- [Monitoreo Autoadaptable](hardware/monitoreo.md) — `/hardware/monitoreo/` — maqueta maestro/esclavo (batería, DHT, cooler, LED RGB)
- [Iluminación Autoadaptable](hardware/iluminacion.md) — `/hardware/iluminacion/` — maqueta de fotoresistencia + LED de potencia (usada en el taller de sistemas continuos)

## Conceptos teóricos (CPS)

- [Introducción y conceptos claves](concepto/index.md) — `/concepto/` — definición de sistema autoadaptable y CPS
- [CPS de nivel bajo](concepto/cps-nivel-bajo.md) — `/concepto/cps-nivel-bajo/` — auto-ajuste, auto-adaptación, auto-regulación, auto-conciencia, autoconocimiento, conciencia del contexto, autoreproducción; generaciones 0–4
- [CPS de nivel medio](concepto/cps-nivel-medio.md) — `/concepto/cps-nivel-medio/` — diagrama de niveles bajo/medio/alto (complejidad vs. estratégico-operativo), características de nivel bajo y medio, referencia bibliográfica (Lee & Seshia)
- **No existe una página `concepto/cps-nivel-alto` en el sitio** — ver [GAPS.md](GAPS.md)

## Otras páginas del sitio (encontradas vía sitemap, no enlazadas desde la navegación ni desde el contenido de las páginas anteriores)

- [Enfoque Pedagógico](inicio/enfoque.md) — `/inicio/enfoque/` — metodología del curso (aprendizaje inverso, taxonomía de Bloom, cono de Edgar Dale); no es contenido técnico del proyecto pero se incluyó por completitud.
- **`/guia-clase/guia_clases/`, `/guia-clase/guia_1/`, `/guia-clase/guia_1b/`, `/guia-clase/guia_2/`** — encontradas en `sitemap-0.xml` del sitio pero **no se mirroraron**: su contenido trata sobre "electrónica de potencia" (fuentes de alimentación, diodos, rectificadores), aparentemente contenido huérfano de otro curso que reutiliza la misma plantilla del sitio. No tienen relación con el proyecto de ciudad inteligente ni están enlazadas desde ninguna página del curso ISA262.

## Recursos descargados (`../assets/`)

Total: 86 archivos descargados exitosamente (~15 MB). Un archivo (`TimerMEF.h` en `/arduino/libreria/TimerMEF.h`) dio error 404 en el sitio original — su código fuente completo igualmente quedó capturado de forma literal dentro de [software/timer-mef.md](software/timer-mef.md).

### Diapositivas y notebook (`assets/slides/`)
| Archivo | Origen (página que lo referencia) |
|---|---|
| `sistemas-discretos.slides.html` | [taller/discretos.md](taller/discretos.md) / [clase/sistemas-discretos.md](clase/sistemas-discretos.md) |
| `sistemas-conectados.slides.html` | [taller/conectados.md](taller/conectados.md) / [clase/sistemas-conectados.md](clase/sistemas-conectados.md) |
| `sistemas-continuos.slides.html` | [taller/continuos.md](taller/continuos.md) / [clase/sistemas-continuos.md](clase/sistemas-continuos.md) |
| `sistemas-continuos.ipynb` | [taller/continuos.md](taller/continuos.md) — notebook Jupyter original con el modelado de la planta de iluminación |

### PDFs (`assets/pdfs/`)
| Archivo | Origen |
|---|---|
| `extra.pdf` (38 páginas, inglés) | Enlazado como "Información EXTRA" en [clase/sistemas-continuos.md](clase/sistemas-continuos.md) — teoría de dinámica de sistemas (lags/delays, estabilidad, orden, modelo FOPDT) |
| `arduino-programming-cheat-sheet.pdf` | [software/librerias.md](software/librerias.md) |
| `acordeon-arduino.pdf` | [software/librerias.md](software/librerias.md) |
| *(no descargado: Sparkfun Arduino Cheat Sheet, el servidor de origen respondió 502; enlace externo se dejó tal cual en* [software/librerias.md](software/librerias.md)*)* | |

### Esquemáticos / fotos de las maquetas (`assets/schematics/`)
| Archivo | Origen |
|---|---|
| `smart-city-labels.webp` | [hardware/ciudad.md](hardware/ciudad.md) |
| `smart-city.webp` | [evaluacion/entrega.md](evaluacion/entrega.md) |
| `smart-weather.webp` | [hardware/monitoreo.md](hardware/monitoreo.md), [evaluacion/monitoreo.md](evaluacion/monitoreo.md) |
| `smart-lighting.webp` | [hardware/iluminacion.md](hardware/iluminacion.md) |

### Diagramas de MEF / control (`assets/diagrams/`, extraídos de las diapositivas)
| Archivo | Contenido | Origen |
|---|---|---|
| `mef-parpadeo.gv.svg` | MEF de seguridad de batería | [clase/sistemas-conectados.md](clase/sistemas-conectados.md) |
| `mef-sos.gv.svg` | MEF de parpadeo SOS | [clase/sistemas-conectados.md](clase/sistemas-conectados.md) |
| `ft.gv.svg` | Diagrama de bloques función de transferencia | [clase/sistemas-continuos.md](clase/sistemas-continuos.md) |
| `mass-spring.gv.svg` | Sistema masa-resorte | [clase/sistemas-continuos.md](clase/sistemas-continuos.md) |
| `mass-damp-spring.gv.svg` | Sistema masa-resorte-amortiguador | [clase/sistemas-continuos.md](clase/sistemas-continuos.md) |
| `flujo-secador.gv.svg` | Diagrama de flujo de materia (secador de manos) | [clase/sistemas-discretos.md](clase/sistemas-discretos.md) |
| `secador-mef.gv.svg` | MEF del secador de manos | [clase/sistemas-discretos.md](clase/sistemas-discretos.md) |
| `secador-mef2.gv.svg` | MEF del secador con temporizador | [clase/sistemas-discretos.md](clase/sistemas-discretos.md) |

### Capturas de pantalla de tutoriales (`assets/images/`)
Instalación de placas ESP32, extracción HEX/ELF, instalación del simulador Wokwi, y las imágenes del enfoque pedagógico (taxonomía de Bloom, cono de Edgar Dale) — referenciadas inline desde [software/esp.md](software/esp.md), [software/hexelf.md](software/hexelf.md), [software/wokwi.md](software/wokwi.md) e [inicio/enfoque.md](inicio/enfoque.md).

### Código fuente (`assets/code/`)

**Ciudad autoadaptable** (proyecto final) — `assets/code/esp32/smart_city/`: `esp_pruebas.ino`, `esp_send_data.ino`, `esp_get_data.ino` — enlazados desde [hardware/ciudad.md](hardware/ciudad.md).

**Proyecto Wokwi de la ciudad** — `assets/code/wokwi/ciudad/`: `wokwi.toml`, `diagram.json`, `code.bin`, `code.elf` — listos para simular la maqueta completa sin hardware físico (ver también el proyecto en línea enlazado desde [hardware/ciudad.md](hardware/ciudad.md): `https://wokwi.com/projects/431140613428347905`).

**Monitoreo climático (maestro/esclavo)** — `assets/code/arduino/smart_weather/` (9 archivos: lectura de batería, temperatura/humedad, actuadores, WiFi, envío de datos, eco serial, simuladores maestro/esclavo) y `assets/code/wokwi/monitoreo-master/`, `assets/code/wokwi/monitoreo-slave/` (archivos de simulación + `sim2real.py`/`real2sim.py` para puentear simulador↔hardware real) — enlazados desde [hardware/monitoreo.md](hardware/monitoreo.md).

**Solución de referencia del taller de monitoreo** — `assets/code/arduino/taller/mef_esclavo.ino`, `mef_maestro.ino` — enlazados desde [solucion/taller-solucion.md](solucion/taller-solucion.md).

**Ejemplos de las diapositivas de discretos/conectados/continuos** — `assets/code/arduino/discretos/` (parpadeo, if, switch, fotoresistencia), `assets/code/esp32/discretos/` (mismos ejemplos en ESP32), `assets/code/arduino/conectados/` (parpadeo con tiempo relativo, MEF de batería, MEF SOS), `assets/code/arduino/continuos/` (lazo abierto y lazo cerrado del sistema de iluminación) — enlazados desde [clase/sistemas-discretos.md](clase/sistemas-discretos.md), [clase/sistemas-conectados.md](clase/sistemas-conectados.md) y [clase/sistemas-continuos.md](clase/sistemas-continuos.md) respectivamente.

**Librerías Arduino** — `assets/code/arduino/librerias/` (`Adafruit_NeoPixel.zip`, `ArduinoHttpClient.zip`, `Arduino_JSON.zip`, `DHT.zip`, `LiquidCrystal_I2C.zip`, `WiFi101.zip`) — enlazadas desde [software/librerias.md](software/librerias.md).

**Wokwi — proyecto de prueba Blink** — `assets/code/wokwi/blink/` — enlazado desde [software/wokwi.md](software/wokwi.md) como tutorial de primer uso del simulador.

**Utilidades varias** — `assets/code/wokwi/ejemplo-timer/ejemplo-timer.ino` (ejemplo de `TimerMEF.h` con semáforo, ligado a [software/timer-mef.md](software/timer-mef.md)); `assets/code/wokwi/websocket-serial.py`, `assets/code/wokwi/sim2sim.py` (ligados a [software/librerias.md](software/librerias.md) y [hardware/monitoreo.md](hardware/monitoreo.md)).
