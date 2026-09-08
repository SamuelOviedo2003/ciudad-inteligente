---
source: https://isa262.davinsony.com/discreto/esp32/
title: Expressif ESP
---

# Expressif ESP

Tras analizar la arquitectura de **Arduino**, es imperativo discutir la evolución natural hacia sistemas más robustos. Si **Arduino** es el estándar de aprendizaje, el **ESP32** es la herramienta de rendimiento para la ingeniería del siglo XXI.
## La Superioridad del **ESP32**
Desarrollado por Espressif Systems, el **ESP32** no es solo una placa de desarrollo, sino un **SoC (System on Chip)** diseñado específicamente para el Internet de las Cosas (IoT). A diferencia del **Arduino** Uno (basado en un microcontrolador de 8 bits a 16 MHz), el **ESP32** integra un procesador **Xtensa® Dual-Core de 32 bits** con frecuencias de hasta 240 MHz.
| Característica | **Arduino** Uno | **ESP32** |
|---|---|---|
| **Arquitectura** | 8-bit (Single Core) | 32-bit (Dual Core) |
| **Conectividad** | Ninguna (requiere shields) | Wi-Fi & Bluetooth (Dual Mode) |
| **SRAM** | 2 KB | 520 KB |
| **Voltaje Lógico** | 5 V | 3.3 V |
## Ventajas Competitivas
1. **Multitarea Real:** Su arquitectura de doble núcleo permite asignar procesos críticos (como el stack de red) a un núcleo, mientras el código de usuario se ejecuta en el otro, evitando latencias.
2. **Conectividad Nativa:** La integración de Wi-Fi y Bluetooth (Classic y BLE) elimina la necesidad de hardware adicional y reduce drásticamente el costo total del proyecto.
3. **Periféricos Avanzados:** Incluye sensores táctiles capacitivos, sensores de efecto Hall y una resolución de ADC de 12 bits, superando los 10 bits convencionales de **Arduino**.
Para nosotros, la transición al **ESP32** representa el paso del prototipado básico al despliegue de soluciones conectadas y escalables.
