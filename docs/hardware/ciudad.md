---
source: https://isa262.davinsony.com/hardware/ciudad/
title: Ciudad Autoadaptable
---

# Ciudad Autoadaptable

Esta maqueta también conocida como _Smart City_ cuenta con multiples elementos de entrada y salida.
[Imagen: Maqueta Smart City] (imagen local: ../../assets/schematics/smart-city-labels.webp)
## Conexiones de entrada y salida
```cpp
/// Maqueta con ESP32
#define LDR1 13 // LDR Light sensor from traffic light 1 connected in pin A0
#define LDR2 12 // LDR Light sensor from traffic light 2 connected in pin A1
#define CO2 14  // CO2 sensor connected in pin A3
#define P1 1    // Traffic light 1 button connected in pin 1
#define P2 2    // Traffic light 2 button connected in pin 2
#define CNY1 42 // Infrared sensor 1 in traffic light 1 connected in pin 42
#define CNY2 41 // Infrared sensor 2 in traffic light 1 connected in pin 41
#define CNY3 40 // Infrared sensor 3 in traffic light 1 connected in pin 40
#define CNY4 39 // Infrared sensor 4 in traffic light 2 connected in pin 39
#define CNY5 38 // Infrared sensor 5 in traffic light 2 connected in pin 38
#define CNY6 37 // Infrared sensor 6 in traffic light 2 connected in pin 37
#define LR1 5   // Red traffic light 1 connected in pin 5
#define LY1 4   // Yellow traffic light 1 connected in pin 4
#define LG1 6   // Green traffic light 1 connected in pin 6
#define LR2 7   // Red traffic light 2 connected in pin 7
#define LY2 15  // Yellow traffic light 2 connected in pin 15
#define LG2 16  // Green traffic light 2 connected in pin 16
```
## Importante
- La pantalla se controla por I2C, la pantalla cuenta con una matrix de 16x4 caracteres.
- Los sensores infrarojos detectan la presencia de objetos blancos.
## Códigos de pruebas en ESP32
Para realizar las pruebas podemos descargar el siguiente código:
- [pruebas.ino](../../assets/code/esp32/smart_city/esp_pruebas.ino) las librerias pueden ser descargadas directamente desde el Arduino IDE, o pueden ser descargadas desde la página de [librerias](../software/librerias.md).
Las mismas pruebas las podemos realizar en el simulador [wokwi](../software/wokwi.md), descargando los siguientes 4 archivos en la misma carpeta y corriendo la simulación con VS Code:
- [wokwi.toml](../../assets/code/wokwi/ciudad/wokwi.toml)
- [diagram.json](../../assets/code/wokwi/ciudad/diagram.json) (clic derecho _guardar enlace como_ o similar)
- [code.bin](../../assets/code/wokwi/ciudad/code.bin) (clic derecho _guardar enlace como_ o similar)
- [code.elf](../../assets/code/wokwi/ciudad/code.elf)
Una versión en linea también esta disponible [aquí](https://wokwi.com/projects/431140613428347905), tener el cuenta que la compilación del código en la versión en linea depende de la disponibilidad de los servidores de la página.
## Códigos de ejemplo
- 💾 [Enviar datos](../../assets/code/esp32/smart_city/esp_send_data.ino)
- 💾 [Recibir datos](../../assets/code/esp32/smart_city/esp_get_data.ino)
