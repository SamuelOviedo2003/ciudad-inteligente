---
source: https://isa262.davinsony.com/discreto/sintaxis/
title: Sintaxis en Arduino IDE
---

# Sintaxis en Arduino IDE

Analizaremos la estructura sintáctica que unifica a ambos ecosistemas. Aunque el hardware difiera sustancialmente, el **Entorno de Desarrollo Integrado (IDE) de Arduino** utiliza una abstracción de **C++**, proporcionando un marco de trabajo (_framework_) que estandariza la programación mediante una estructura de funciones predefinidas.
## La Estructura Fundamental: El “Sketch”
Todo programa en este entorno se denomina _sketch_ y se fundamenta en dos funciones obligatorias:
1. **`void setup()`**: Es el punto de entrada inicial. Se ejecuta una sola vez al energizar el microcontrolador o tras un _reset_. Aquí se inicializan protocolos de comunicación (como el puerto Serial) y se define la configuración de los pines (entrada o salida).
2. **`void loop()`**: Constituye el núcleo del programa. Es un bucle infinito donde reside la lógica operativa. La velocidad de ejecución de este ciclo depende de la frecuencia de reloj del procesador. Esta función es analoga a tener: `while(true){ // Tu código se ejecuta aquí}`
## Sintaxis y Abstracción de Hardware
La sintaxis del IDE se apoya en funciones de alto nivel que ocultan la complejidad de los registros internos del microcontrolador:
- **Manipulación de Pines:** Mediante `pinMode()`, `digitalWrite()` y `digitalRead()`, interactuamos con el hardware sin necesidad de álgebra de bits sobre registros de puerto. Estas funciones de base de Arduino pueden ser consultadas en su [página de documentación](https://docs.arduino.cc/programming/).
- **Gestión del Tiempo:** Se utilizan funciones como `delay(ms)` para pausas bloqueantes o `millis()` para obtener el tiempo transcurrido desde el inicio, técnica esencial para implementar multitarea no bloqueante.
## El Preprocesador y Librerías
Dada la naturaleza de C++, es fundamental el uso de directivas como `#include` para importar librerías que extienden las capacidades del sistema (por ejemplo, para manejar el stack TCP/IP del ESP32). Esto permite una modularidad académica, facilitando la reutilización de código y la interoperabilidad entre distintas arquitecturas de hardware.
