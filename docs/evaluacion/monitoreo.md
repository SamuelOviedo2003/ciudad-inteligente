---
source: https://isa262.davinsony.com/evaluacion/monitoreo/
title: Monitoreo climático en red
---

# Monitoreo climático en red

Se tiene una estación con un sistema redundante de medidas de temperatura y humedad, es decir, cuenta con un sistema principal (maestro) y un sistema de respaldo (esclavo).
| [Imagen: Maqueta Smart Weather] (imagen local: ../../assets/schematics/smart-weather.webp) | [Imagen: Maqueta Smart Weather] (imagen local: ../../assets/schematics/smart-weather.webp) |
|---|---|
Más información de las maquetas [aquí](../hardware/monitoreo.md)
## Descripción del problema
- El sistema maestro emitirá datos a la nube siempre que este pueda.
- Cómo medida de seguridad cuando la temperatura del sistema maestro supere un cierto valor (30°C), emitirá una señal de alerta “A” al sistema secundario via ZigBee (protocolo de comunicación) 3 veces con una espera entre mensajes de 3 segundos. Para identificar de manera visual que los sistemas estan en modo alerta, se mostrará un parpadeo en un led rojo (LR, 500ms encendido y 500ms apagado). Si el sistema maestro regresa a rangos normales (inferior a 27°C) enviará un mensajes “O” de OK al sistema esclavo para que este deje de enviar datos siguiendo la misma dinámicas, enviar 3 veces con una espera de 3 segundos.
- El sistema esclavo solo emitirá datos a la nube si recibe el mensaje de alerta “A” del maestro, y lo hará hasta recibir el OK “O”, en caso de recibir el OK, dejara de enviar datos.
### Tarea
- Construir e implementar en Arduino, MEF o MEFs para cada uno de los dos sistemas (maestro y esclavo) de manera que se cumplan las condiciones presentadas.
# Generación 0
Solo tenemos un modo de operación en donde tenemos un lazo cerrado que une sensores y actuadores que permite regular el sistema. La funcionalidad no cambia.
# Generación 1
En la generación 1 podemos tener diferentes modos de operación que cambian o se suichean entre los estados del problema.
