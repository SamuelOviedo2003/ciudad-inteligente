---
source: https://isa262.davinsony.com/software/esp/
title: Configuración: Placas ESP32
---

# Configuración: Placas ESP32

Es necesario transformar el Arduino IDE en una estación de trabajo compatible con la arquitectura de **Espressif Systems**. A diferencia de las placas AVR, el ESP32 requiere una cadena de herramientas específica para su núcleo Xtensa®.
## Configuración del Repositorio de Tarjetas
El IDE no reconoce el ESP32 de forma nativa. Debemos indicarle dónde encontrar las definiciones de hardware mediante el gestor de URLs de tarjetas adicionales:
1. Diríjase a **Archivo > Preferencias**.
2. En el campo **Gestor de URLs Adicionales de Tarjetas**, pegue el siguiente enlace oficial: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. Haga clic en **OK** para indexar el repositorio.
## Instalación del Core ESP32
Una vez vinculado el repositorio, debemos descargar los binarios del compilador:
1. Acceda a **Herramientas > Placa > Gestor de tarjetas…**
2. En la barra de búsqueda, escriba **“ESP32”**.
3. Seleccione el paquete de **Espressif Systems** e instale la versión estable más reciente. Este proceso descargará las bibliotecas de Wi-Fi, Bluetooth y los sistemas de archivos (SPIFFS/LittleFS).
[Imagen: Instalar Placa ESP32] (imagen local: ../../assets/images/esp-placa.webp)
## Selección del Perfil de Hardware
El ecosistema ESP32 es vasto (DevKit V1, WROOM, S3, C3). Para un prototipado estándar, seleccione:
- **Placa:** [“ESP32 S3 Dev Module”](https://docs.espressif.com/projects/esp-idf/en/v4.4.3/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1.html) que es modelo que tenemos en la maqueta de la ciudad adaptable
## El Puente USB-UART
Si al conectar la placa no aparece un puerto COM activo, es probable que requiera el driver del puente USB-Serial. La mayoría de los módulos comerciales utilizan el chip **CP2102** o el **CH340**. Sin este controlador, la comunicación bidireccional entre el computador y el SoC será inexistente.
