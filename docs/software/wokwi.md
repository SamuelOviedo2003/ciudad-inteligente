---
source: https://isa262.davinsony.com/software/wokwi/
title: Configuracion: Simulador Wokwi
---

# Configuracion: Simulador Wokwi

Para los talleres de curso estaremos maquetas física para las personas que por algun motivo no pueden asistir a las clases presenciales podremos usar el simulador **WOKWI** como extensión de **VS Code**.
## Instalación de WOKWI
Esta instalación supone que ya se tiene instalado **VS Code**. Dentro de este iremos a extensiones y buscaremos [“_Wokwi Simulator_”](https://marketplace.visualstudio.com/items?itemName=wokwi.wokwi-vscode).
[Imagen: Extension en VS Code] (imagen local: ../../assets/images/wokwi-vs-extension.webp)
Haremos clic en instalar. Una vez instalado presionaremos `F1` y seleccionaremos `Wokwi: Request a new License` iremos al navegador a la siguiente página ([https://wokwi.com/license](https://wokwi.com/license)).
[Imagen: Licencia para Wokwi] (imagen local: ../../assets/images/wokwi-get-license.webp)
Para obtener la licencia, debemos registrarnos/logearnos. Una vez estamos logeados, podemos hacer clic en `GET YOUR LICENSE`.
[Imagen: Licencia activada] (imagen local: ../../assets/images/wokwi-license-activated.webp)
Seguimos las intrucciones de la página y deberemos tener el ambiente de simulación listo.
## Instalación de Arduino
El simulador utiliza unos archivos generados por arduino `.hex` y `.elf`. Por lo que para generarlo necesitaremos el compilador que viene con el [Arduino IDE](index.md).
Para extraer los códigos necesarios para **Wokwi** de cada versión de Arduino usar el [tutorial](hexelf.md).
## Estructura de archivo
Para correr una simulación deberemos tener los siguiente 4 archivos:
```txt
wokwi.toml
diagram.json
code.elf
code.hex
```
- **wokwi.toml** Este archivo tiene la configuración del simulador, aquí se podrían cambiar los nombres de los archivos generador por Arduino.
```toml
[wokwi]
version = 1
firmware = 'code.hex'
elf = 'code.elf'
```
- **diagram.json** Contiene la descripción de todo los elementos de hardware que componen la simulación. Si se desea modificar el diagram.json la mejor forma es modificarlo en el [sitio web](https://wokwi.com/) de **Wokwi**
- **code.hex** y **code.elf** Son los archivos que se generan con el Arduino IDE, mirar [el tutorial](hexelf.md).
## Correr el simulador
Para probar el simulador descaguemos los siguientes 4 archivos:
- [wokwi.toml](../../assets/code/wokwi/blink/wokwi.toml)
- [diagram.json](../../assets/code/wokwi/blink/diagram.json) (clic derecho _guardar enlace como_ o similar)
- [blink.hex](../../assets/code/wokwi/blink/blink.hex) (clic derecho _guardar enlace como_ o similar)
- [blink.elf](../../assets/code/wokwi/blink/blink.elf)
Una vez descargados en la misma carpeta, abrir en VS Code `diagram.json` y presionamos _start simulation_:
[Imagen: Iniciar simulación] (imagen local: ../../assets/images/wokwi-start-simulation.webp)
Con esto tendremos la simulación funcionando:
[Imagen: Animación de la simulación] (imagen local: ../../assets/images/wokwi-blink-animation.webp)
