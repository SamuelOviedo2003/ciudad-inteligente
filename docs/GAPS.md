---
source: (análisis local, no es contenido del sitio)
title: Vacíos de contenido frente a la rúbrica de entrega
---

# Vacíos de contenido (GAPS)

Este documento lista temas mencionados en [evaluacion/entrega.md](evaluacion/entrega.md) (o implicados por su rúbrica) que **no están cubiertos con suficiente detalle** en ninguna otra página del sitio ni en los recursos descargados a `assets/`. No se investigó ni se rellenó contenido para estos puntos — solo se documenta que faltan, para investigarlos aparte.

## 1. CPS de nivel alto — no existe documentación en el sitio (30 de 100 puntos de la rúbrica)

La rúbrica de [evaluacion/entrega.md](evaluacion/entrega.md) pide (30 pts, el ítem de mayor peso):

> "Utilizando todas las entradas y salidas que poseé la maqueta, diseñar e implementar un sistema de sistemas autoadaptables de nivel alto, de alguna manera."

La sección de Conceptos Teóricos del sitio solo tiene tres páginas: [concepto/index.md](concepto/index.md) (introducción), [concepto/cps-nivel-bajo.md](concepto/cps-nivel-bajo.md) y [concepto/cps-nivel-medio.md](concepto/cps-nivel-medio.md). **No existe una página `concepto/cps-nivel-alto`** ni en la navegación del sitio ni en su `sitemap.xml`. El diagrama GraphViz en [concepto/cps-nivel-medio.md](concepto/cps-nivel-medio.md) sí dibuja un eje "Low level - CPS → Mid level - CPS → High level - CPS", pero el texto que lo acompaña solo describe nivel bajo y medio; nunca define qué caracteriza al nivel alto.

Las únicas pistas indirectas sobre "nivel alto" en todo el sitio son las descripciones de generaciones 2–4 en [concepto/cps-nivel-bajo.md](concepto/cps-nivel-bajo.md) (autoconciencia, aprendizaje de la propia experiencia, componentes intensivos en conocimiento, auto-organización sin principio predefinido) — son solo definiciones conceptuales de un párrafo cada una, sin técnicas, algoritmos ni ejemplos de implementación.

La frase de la rúbrica "**de alguna manera**" confirma que el propio curso deja abierto el "cómo" del nivel alto. El enunciado de la tarea de la entrega también lo señala explícitamente: *"En la presentación hacer propuestas para llevar el sistema al nivel alto. ¿Qué se necesitaría?"* — es decir, se espera que el estudiante **investigue y proponga** las técnicas, no que las encuentre documentadas en el sitio.

**A investigar aparte:** qué técnicas concretas (aprendizaje automático / machine learning aplicado a control de tráfico, sistemas multiagente, algoritmos de auto-organización, optimización en tiempo real, IA embebida en ESP32 — p. ej. TinyML — reglas adaptativas basadas en datos históricos, etc.) son viables de implementar sobre el hardware real disponible (ESP32 S3, ver [software/esp.md](software/esp.md)) para un sistema de semáforos de la maqueta "ciudad autoadaptable" ([hardware/ciudad.md](hardware/ciudad.md)).

## 2. Nivel medio: "comunicación serial con el computador para recibir/enviar información de internet" — solo hay ejemplos parciales, no un tutorial dedicado

La rúbrica (20 pts) pide específicamente:

> "...extendiendo el sistema con comunicación serial con el computador para recibir/enviar información de internet, que amplifique las capacidades de autoadaptabilidad del sistema."

Lo que sí existe:
- [hardware/ciudad.md](hardware/ciudad.md) enlaza `esp_send_data.ino` y `esp_get_data.ino` (en `assets/code/esp32/smart_city/`), pero son ejemplos genéricos de HTTP GET/POST **por WiFi directo del ESP32** (usando `WiFi.h`/`HTTPClient.h`), no comunicación **serial** con un computador que a su vez hable con internet.
- [clase/sistemas-conectados.md](clase/sistemas-conectados.md) sí documenta un patrón de nube (Ubidots, `isa.requestcatcher.com`) pero aplicado a la maqueta de **monitoreo climático** (`hardware/monitoreo.md`), no a la ciudad.
- `assets/code/wokwi/websocket-serial.py`, `assets/code/wokwi/monitoreo-master/sim2real.py` y `assets/code/wokwi/monitoreo-slave/real2sim.py` son scripts Python que puentean el puerto serial del simulador con algo externo, pero tampoco tienen documentación narrativa en ninguna página — solo aparecen como enlaces de descarga en [software/librerias.md](software/librerias.md) y [hardware/monitoreo.md](hardware/monitoreo.md), sin explicación de su protocolo o formato de datos.

**A investigar aparte:** cómo estructurar literalmente un puente serie (ESP32 ↔ USB-Serial ↔ script en el computador ↔ API/internet) para la ciudad autoadaptable, y qué formato de datos/protocolo usar (los scripts descargados no traen documentación, habría que leer su código fuente y probarlo).

## 3. Sensor de CO2 de la maqueta de la ciudad — sin documentación

[hardware/ciudad.md](hardware/ciudad.md) define el pin `#define CO2 14` pero ningún texto del sitio explica qué modelo de sensor es, cómo se lee (¿analógico? ¿I2C/UART como el MH-Z19 u otros sensores NDIR típicos?), su rango de valores o su calibración. `esp_pruebas.ino` (descargado en `assets/code/esp32/smart_city/esp_pruebas.ino`) podría tener pistas en el código, pero no hay una página que lo explique.

**A investigar aparte:** modelo exacto del sensor de CO2 instalado en la maqueta física y su librería/protocolo de lectura.

## 4. Pantalla LCD I2C de la ciudad — solo una línea de mención

[hardware/ciudad.md](hardware/ciudad.md) solo dice: *"La pantalla se controla por I2C, la pantalla cuenta con una matrix de 16x4 caracteres."* No hay dirección I2C, librería recomendada (aunque `LiquidCrystal_I2C.zip` está disponible en [software/librerias.md](software/librerias.md), no se documenta que sea la usada para esta pantalla específicamente), ni ejemplos de qué mostrar en ella.

## 5. Inconsistencia sin resolver en los comentarios del pinout de la ciudad

En el bloque de código de [hardware/ciudad.md](hardware/ciudad.md) los `#define` numéricos no coinciden con los comentarios que los acompañan, por ejemplo:

```cpp
#define LDR1 13 // LDR Light sensor from traffic light 1 connected in pin A0
#define LDR2 12 // LDR Light sensor from traffic light 2 connected in pin A1
#define CO2 14  // CO2 sensor connected in pin A3
```

El valor (`13`, `12`, `14`) no coincide con lo que dice el comentario (`A0`, `A1`, `A3`). Esto está copiado literalmente del sitio (no es un error de esta extracción) pero es una ambigüedad real que habrá que resolver **contra la maqueta física** antes de programar, ya que no se puede saber con certeza cuál de los dos valores (el `#define` o el comentario) es el correcto sin verificarlo en el hardware.

## 6. Sin esquemático eléctrico / diagrama de circuito de la maqueta de la ciudad

Todas las páginas de `hardware/` dan una lista de pines (`#define`) y una foto/ilustración de la maqueta (`assets/schematics/smart-city-labels.webp`, etc.) pero **ningún archivo del sitio es un esquemático eléctrico real** (tipo Fritzing/KiCad) que muestre cómo están cableados los componentes entre sí, ni las resistencias/pull-ups usadas para los LDR y sensores infrarrojos CNY. El `diagram.json` de Wokwi (`assets/code/wokwi/ciudad/diagram.json`) es lo más cercano a un esquemático real y sí es utilizable, pero no viene acompañado de una explicación textual de por qué está cableado así.

## 7. Dependencia de servicios externos que pueden dejar de estar disponibles

- El proyecto Wokwi en línea de la ciudad (`https://wokwi.com/projects/431140613428347905`, enlazado desde [hardware/ciudad.md](hardware/ciudad.md)) depende de que ese proyecto siga público en la cuenta del profesor; el sitio mismo advierte "la compilación del código en la versión en línea depende de la disponibilidad de los servidores de la página". Los archivos locales descargados (`assets/code/wokwi/ciudad/`) son el respaldo, pero pueden no reflejar cambios posteriores al proyecto en línea.
- El dashboard de Ubidots enlazado en [clase/sistemas-conectados.md](clase/sistemas-conectados.md) y el servicio `isa.requestcatcher.com` usado en los ejemplos de código son cuentas/servicios de terceros cuya disponibilidad no está garantizada a futuro.

## 8. No cubierto por ser fuera de alcance del curso (mencionado solo de pasada)

- **Webots** aparece listado como software del curso en [software/index.md](software/index.md) pero ninguna otra página lo vuelve a mencionar ni explica para qué se usaría (simulación robótica 3D; posiblemente para la componente de "nivel alto", pero no hay confirmación en el sitio).
- El PDF `extra.pdf` (38 páginas, en inglés, sobre dinámica de sistemas y control) descargado en `assets/pdfs/extra.pdf` es material de referencia adicional enlazado solo desde [clase/sistemas-continuos.md](clase/sistemas-continuos.md) ("Información EXTRA"); no se transcribió página por página en este mirror por ser material de apoyo secundario y extenso — está disponible completo como PDF si se necesita profundizar en teoría de control (retardos, estabilidad, orden del sistema, modelo FOPDT).

## Resumen

| Punto de la rúbrica | Cobertura en el sitio | Gap principal |
|---|---|---|
| Nivel bajo (10 pts) | Buena — MEF, timers, pinout completo, ejemplos de discretos | Ninguno relevante |
| Nivel medio (20 pts) | Parcial — teoría de MEF con tiempo relativo bien cubierta; comunicación serial+internet solo con ejemplos genéricos de WiFi, no específicos de la ciudad | Ver punto 2 |
| Nivel alto (30 pts) | **Prácticamente inexistente** — no hay página de concepto ni ejemplos | Ver punto 1 (el más importante) |
| Demostración nivel más alto (20 pts) | Depende de resolver el punto anterior | — |
| Propuestas para nivel alto (10 pts) | El propio sitio espera que el estudiante investigue esto | Ver punto 1 |
| Calidad de la presentación (10 pts) | No es contenido técnico, no aplica | — |
