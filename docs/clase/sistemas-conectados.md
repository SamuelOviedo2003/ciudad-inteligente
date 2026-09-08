---
source: https://isa262.davinsony.com/clase/sistemas-conectados.slides
title: Taller ~ Sistemas conectados slides
---

# Taller ~ Sistemas conectados slides

# Taller en sistemas conectados

# Parte 1: MEF en Arduino
Para implementar MEF en Arduino debemos estudiar un concepto clave y es el tiempo relativo.

# Tiempo relativo
El concepto de tiempo relativo no permite sabe cuanto tiempo a pasado desde un evento anterior a través de una sustracción.
- Permite el procesamiento de tiempo
- Permite tener multiples tareas en un microcontrolador
- Mejora el rendimiento de del microcontrolador

## Tiempo relativo con un reloj
- ¿ Para poder responder a la siguiente pregunta que necesito saber ?
  - ¿ Cuantos minutos han pasado desde que inicio la clase ?

- Necesito saber el tiempo "absoluto" actual.
- Necesito saber el tiempo "absoluto" del inicio de la clase (el evento).

## Tiempo relativo con Arduino
En Arduino existen dos funciones que están relacionadas con el tiempo:
- [`millis()`](https://www.arduino.cc/reference/en/language/functions/time/millis/) que retorna el tiempo en milisegundos que el Arduino lleva encendido.
- [`micros()`](https://www.arduino.cc/reference/en/language/functions/time/micros/) que retorna el tiempo en microsegundos que el Arduino lleva encendido.
Estás funciones nos permiten tener encuenta retardo sin frenar la ejecución del código. Para usar el tiempo relativo necesitamos $3$ variables:
- `unsigned long tini;` Tiempo de inicio, esta variable se actualiza cuando ocurre el inicio del evento para el cual queremos contar tiempo.
- `unsigned long tact;` Tiempo actual, esta variable se actualiza en cada ciclo de ejecución (en el `loop()`)
- `unsigned long trel;` Diferencia de tiempo entre el tiempo actual y el tiempo de inicio, esta variable se actualiza a discresión de las necesidades del programa.

## Ejemplo de Parpadeo con tiempo relativo
💾 [Código Parpadeo con tiempo relativo](../../assets/code/arduino/conectados/parpadeo_trel.ino)

## Estructura de un código MEF en Arduino
```
/* 1:  Declaración de libreria      */  #include <SFEMP3Shield.h>
/* 2.1:  Definición de ESTADOS      */  #define SOFF 0
/* 2:  Definición de etiquetas      */  #define LEDPIN 3
/* 3:  Declaración de constantes    */  const unsigned int contMax = 10;
/* 4:  Declaración de variables     */  float temperature = 0;
/* 5:  Declaración de subrutinas    */  void readSensor(){
                                            int y = analogRead(1);
                                            temperature = 100.0*y/1023.0;
                                        }
/* 6:  Declaración de funciones     */  int sum(int x, int y){
                                            return x + y;
                                        }
/* 7:  Subrutina de configuración   */  void setup(){...}
/* 8:  Subrutina de ejecución inf.  */  void loop(){...
/* 8.1:  Se define dentro de loop 
         la MEF en un Switch Case */        switch(state){}
                                        }
```

## MEF de seguridad de la bateria
Hacer un MEF que el "LED Rojo" apagada mientras haya bateria en el sistema, en caso de faltar bateria, el "LED Rojo" parpadeará con el siguiente patrón : 2 segundos encendido y 1 segundo apagado.

[Imagen: mef-parpadeo.gv.svg](../../assets/diagrams/mef-parpadeo.gv.svg)
💾 [Código MEF de seguridad de la bateria](../../assets/code/arduino/conectados/parpadeo_mef.ino)

## MEF de seguridad de la bateria con SOS
Usar el patrón de SOS en morse para el patrón de parpadeo.
[Imagen: SOS]

### Solución MEF SOS
[Imagen: mef-sos.gv.svg](../../assets/diagrams/mef-sos.gv.svg)
💾 [Código MEF de seguridad con SOS](../../assets/code/arduino/conectados/mef_sos.ino)

# Estación Meteorológica
Se tiene la siguiente maqueta, que simulará una estación meteorológica, que posee diferentes protocolos de comunicación que permiten robustez y adaptabilidad.
[Imagen: No description has been provided for this image]
Se desea contruir un sistema inalambrico de monitoreo de temperatura y humedad, resistente a diferentes situaciones.

## Características
### Entradas
- Sensor de temperatura y humedad
- Medida de voltaje
### Salidas
- LED multicolor RGB
- _Cooler_ (ventilador)
### Comunicación
- USB-Serial
- Xbee-Serial
- Wifi

## Arquitectura de un sistema IOT
[Imagen: No description has been provided for this image]

## Arquitectura de la maqueta
[Imagen: No description has been provided for this image]

## Códigos de ejemplo
- 💾 [Ver Maqueta](../hardware/monitoreo.md)

## Request catcher ([enlace](https://isa.requestcatcher.com))

```python
display(IFrame(
    "https://isa.requestcatcher.com/",
    frameborder="0",
    width="100%",
    height="450px"))
```
[Video incrustado: https://isa.requestcatcher.com/?frameborder=0]

## Dashboard en ubidots

```python
display(IFrame(
    "https://stem.ubidots.com/app/dashboards/public/dashboard/w6eBcObiKIIZT2QVpUCny-MpLgiaxeTU5Ug7eS7KXDk?nonavbar=true&embed=true",
    frameborder="0",
    width="800px",
    height="450px"))
```
[Video incrustado: https://stem.ubidots.com/app/dashboards/public/dashboard/w6eBcObiKIIZT2QVpUCny-MpLgiaxeTU5Ug7eS7KXDk?nonavbar=true&embed=true?frameborder=0]

## Creación de una cuenta academica
Para la creación de una cuenta academica en Ubidots hacerlo en [Ubidots STEM](https://www.ubidots.com/stem).
[Imagen: ubidots.png]

> Contenido extraído de la presentación de diapositivas (Jupyter/Reveal.js) enlazada desde la página de taller (docs/taller/conectados.md) como "Diapositivas"/"PDF". Copia HTML original guardada en assets/slides/sistemas-conectados.slides.html. Algunas imágenes (arquitectura IoT, maqueta, sos.png, ubidots.png) no se pudieron descargar por no tener URL absoluta identificable y se dejan solo como descripción [Imagen: ...].

