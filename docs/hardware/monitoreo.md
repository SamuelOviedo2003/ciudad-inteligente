---
source: https://isa262.davinsony.com/hardware/monitoreo/
title: Monitoreo Autoadaptable
---

# Monitoreo Autoadaptable

Esta maqueta cuenta con un elemento de entrada y uno salida.
[Imagen: Maqueta Smart Weather] (imagen local: ../../assets/schematics/smart-weather.webp)
## Conexiones de entrada y salida
```cpp
#define PIN_BATERIA   A0  // Pin voltaje de la bateria
#define PIN_DHT       9   // Pin sensor de temperatura
#define PIN_COOLER    8   // Pin ventilador
#define PIN_RED       6   // Pin led rojo
#define PIN_GREEN     3   // Pin led verde
#define PIN_BLUE      2   // Pin led azul
```
## Códigos de ejemplo en Arduino
A continuación se presentan algunos códigos de ejemplo para el uso básico de los elementos usados en la maqueta:
- **Sensores**
  - 💾 [Lectura de la bateria](../../assets/code/arduino/smart_weather/lectura_bateria.ino)
  - 💾 [Lectura de temperatura y humedad](../../assets/code/arduino/smart_weather/lectura_temperatura_humedad.ino)
- **Actuadores**
  - 💾 [Verificar actuadores del Smart Weather](../../assets/code/arduino/smart_weather/verificar_actuadores.ino)
- **Comunicación**
  - 💾 [Conectar al Wifi](../../assets/code/arduino/smart_weather/conectar_wifi.ino)
  - 💾 [Enviar datos por wifi](../../assets/code/arduino/smart_weather/enviar_datos.ino)
  - 💾 [Tomar datos y enviarlos por WiFi](../../assets/code/arduino/smart_weather/enviar_datos_reales.ino)
  - 💾 [Comunicación XBee por Serial1](../../assets/code/arduino/smart_weather/serial_echo.ino)
## Uso del simulador
Para usar el simulador wokwi necesitaremos los siguientes archivos. Para esta maqueta tendremos los archivos de prueba para el modo maestro, y para el modo esclavo.
**Maestro** [codigo arduino](../../assets/code/arduino/smart_weather/simulador_master.ino)
- [wokwi.toml](../../assets/code/wokwi/monitoreo-master/wokwi.toml)
- [diagram.json](../../assets/code/wokwi/monitoreo-master/diagram.json) (clic derecho _guardar enlace como_ o similar)
- [master.hex](../../assets/code/wokwi/monitoreo-master/master.hex) (clic derecho _guardar enlace como_ o similar)
- [master.elf](../../assets/code/wokwi/monitoreo-master/master.elf)
**Esclavo** [codigo arduino](../../assets/code/arduino/smart_weather/simulador_slave.ino)
- [wokwi.toml](../../assets/code/wokwi/monitoreo-slave/wokwi.toml)
- [diagram.json](../../assets/code/wokwi/monitoreo-slave/diagram.json) (clic derecho _guardar enlace como_ o similar)
- [slave.hex](../../assets/code/wokwi/monitoreo-slave/slave.hex) (clic derecho _guardar enlace como_ o similar)
- [slave.elf](../../assets/code/wokwi/monitoreo-slave/slave.elf)
Si queremos conectar los dos simuladores o un simulador con una maqueta podremos usar los siguiente codígos de Python:
- [sim2real.py](../../assets/code/wokwi/monitoreo-master/sim2real.py)
- [real2sim.py](../../assets/code/wokwi/monitoreo-slave/real2sim.py)
- [sim2sim.py](../../assets/code/wokwi/sim2sim.py) se pueden usar dos simuladores en VS Code usando dos ventanas diferentes.
