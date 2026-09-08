---
source: https://isa262.davinsony.com/discreto/mef/
title: Máquinas de Estados Finitos
---

# Máquinas de Estados Finitos

En el desarrollo de sistemas embebidos, la eficiencia del código es tan vital como el hardware mismo. Hoy abordaremos un pilar de la computación teórica aplicado al control de sistemas: las **Máquinas de Estados Finitos (MEF o FSM, por sus siglas en inglés: _Finite State Machine_)**.
## Definición y Relevancia Académica
Una MEF es un modelo computacional abstracto que describe el comportamiento de un sistema a través de un número limitado de **estados**. El sistema solo puede encontrarse en un estado a la vez (estado actual) y cambia de uno a otro (transición) en respuesta a entradas externas o condiciones lógicas específicas.
En arquitecturas como Arduino o ESP32, las MEF son la solución profesional para evitar el uso de `delay()`, permitiendo un comportamiento determinista y reactivo.
## Componentes de una MEF en Código
Para implementar una MEF dentro del `loop()`, utilizamos habitualmente dos estructuras de C++:
1. **`enum` (Enumeraciones):** Define de manera legible los estados posibles (ej. `IDLE`, `SENSING`, `ALARM`).
2. **`switch-case`:** Estructura de control que evalúa el estado actual y ejecuta la lógica correspondiente.
```cpp
enum Estado {REPOSO, ACTIVO};
Estado estadoActual = REPOSO;


void loop() {
  switch (estadoActual) {
    case REPOSO:
      if (sensorDetectado()) estadoActual = ACTIVO;
      break;
    case ACTIVO:
      ejecutarAccion();
      if (tiempoCumplido()) estadoActual = REPOSO;
      break;
  }
}
```
## Ventajas en el Desarrollo
- **Determinismo:** Facilita la predicción del comportamiento del sistema ante cualquier evento.
- **Mantenibilidad:** El código se vuelve modular y escalable; añadir una nueva funcionalidad implica simplemente agregar un nuevo estado y sus transiciones.
- **Depuración:** Permite rastrear errores lógicos identificando en qué estado se “bloquea” el flujo.
