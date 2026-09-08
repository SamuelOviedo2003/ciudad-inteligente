---
source: https://isa262.davinsony.com/clase/sistemas-discretos.slides
title: Taller ~ Sistemas discretos slides
---

# Taller ~ Sistemas discretos slides

# Taller en sistemas discretos

# Parte 1 : Fundamentos de Arduino
Página oficial : [arduino.cc](https://www.arduino.cc/)

## Contenidos
1. Introduccion
2. Arduino MEGA2560
3. Variables en Arduino
4. Operadores típicos en Arduino
5. Estructura de un programa en Arduino
6. Comandos más usados en Arduino
7. _Statements_ más comunes en Arduino

## ¿Qué es Arduino?

Arduino es una plataforma electrónica de código abierto basada en hardware y software fáciles de usar. Está pensada para cualquiera que haga proyectos interactivos.

## Algunas [placas de desarrollo](https://www.arduino.cc/en/hardware)

### Arduino UNO
[Imagen: No description has been provided for this image]

### Arduino MEGA
[Imagen: No description has been provided for this image]

### Arduino MKR1000
[Imagen: No description has been provided for this image]

## ¿Qué lenguage de programación se usa en Arduino?

Arduino es un "sabor" de C++ donde sus principales diferencias son el almacenamiento en memoria.
| Diferencias | Memoria | Instrucciones |
|---|---|---|
| Computador | 2GB | 32-bit / 64-bit |
| Arduino UNO | 2kB | 8-bit |

## Variables en Arduino
| Nombre | Tamaño | Rango sin signo `unsigned` | Rango con signo | Ejemplo |
|---|---|---|---|---|
| `boolean` | 1 bit | false / true | ~no aplica~ | `boolean state = false ;` |
| `char` | 8 bits | 0 ... 255 | -128 ... 127 | `char myChar = 65;`$^1$ |
| `byte` | 8 bits | 0 ... 255 | ~no aplica~ | `byte myByte = B00000111;`$^2$ |
| `int`$^3$ | 16 bits | 0 ... 65 535 | -32 768 ... 32 767 | `int counter = 0;` |
| `long` | 32 bits | 0 ... 4 294 967 295 | -2 147 483 848 ... 2 147 483 847 | `long number = 20000;` |
| `float`$^4$ | 32 bits | ~no aplica~ | -3.4028235e+38 ... 3.4028235e+38 | `float temperature = 37.5;` |
1. Verificar la [tabla ASCII](https://www.asciitable.com/asciifull.gif).
2. `B` indica notacion binaria.
3. Hay otros dos nombre equivalentes `word` = `unsigned int` y `short` = `int`.
4. Revisar la documentación de arduino para el tipo de dato `float` [aquí](https://www.arduino.cc/reference/en/language/variables/data-types/float/).

**Arduino** ya no es Rey
# Espressif Systems
Espressif Systems Co., Ltd. es una empresa china de semiconductores que cotiza en bolsa y con sede en Shanghai. Se centra en el desarrollo y venta de chips y módulos de comunicación de unidades de microcontroladores inalámbricos que se utilizan en Internet de las cosas.

| Característica | Arduino Mega 2560 | ESP32-S3 |  |
|---|---|---|---|
| **Microcontrolador** | ATmega2560 (8 bits) | Xtensa® LX7 de 32 bits, doble núcleo a 240 MHz |  |
| **Frecuencia de reloj** | 16 MHz | Hasta 240 MHz |  |
| **Memoria Flash** | 256 KB | Hasta 16 MB (dependiendo del módulo) |  |
| **SRAM** | 8 KB | 512 KB internos |  |
| **EEPROM** | 4 KB | No integrada (puede emularse en Flash) |  |
| **Wi-Fi** | No | Sí, Wi-Fi 802.11 b/g/n de 2.4 GHz |  |
| **Bluetooth** | No | Sí, Bluetooth 5 (LE) |  |
| **GPIOs** | 54 digitales (15 PWM), 16 analógicos | Hasta 45 GPIOs |  |
| **ADC** | 16 canales de 10 bits | 2 ADC SAR de 12 bits, hasta 20 canales |  |

| Característica | Arduino Mega 2560 | ESP32-S3 |  |
|---|---|---|---|
| **PWM** | 15 salidas PWM | 8 canales PWM |  |
| **Interfaz USB** | USB tipo B (con chip FTDI) | USB OTG integrado |  |
| **Voltaje de operación** | 5V | 3.3V |  |
| **Consumo energético** | Moderado | Bajo, con modos de suspensión profunda |  |
| **Soporte AI/ML** | No | Sí, con instrucciones para aceleración de IA |  |
| **Soporte USB nativo** | No | Sí, USB OTG |  |
| **PSRAM** | No | Hasta 8 MB externos |  |
| **Co-procesador ULP** | No | Sí, RISC-V de ultra bajo consumo |  |

[Imagen: No description has been provided for this image]

## Operadores aritmeticos
| Símbolo | Descripción |
|---|---|
| `=` | Asignación |
| `+` | Adición |
| `-` | Sustracción |
| `*` | Multiplicación |
| `/` | División |
| `%` | Módulo |

## Operadores de comparación
| Símbolo | Descripción |
|---|---|
| `==` | Igual a ($x$ es igual a $y$?) |
| `!=` | Diferente de ($x$ es diferente de $y$?) |
| `< ` | Menor que |
| `> ` | Mayor que |
| `<=` | Menor o igual que |
| `>=` | Mayor o igual que |

## Operadores booleanos
| Símbolo | Descripción |
|---|---|
| `&&` | Operador _Y_ |
| `\|\|` | Operador _O_ |
| `!` | Negación |

## Acumuladores
| Símbolo | Descripción | Ejemplo | Equivalente |
|---|---|---|---|
| `++` | Incremento | `y = x++;` | `y = x+1;` |
| `--` | Decremento | `y = x--;` | `y = x-1;` |
| `+=` | Asignación con suma | `y += x;` | `y = y+x;` |
| `-=` | Asignación con resta | `y -= x;` | `y = y-x;` |
| `*=` | Asignación con multiplicación | `y *= x;` | `y = y*x;` |
| `/=` | Asignación con división | `y /= x;` | `y = y/x;` |

## Estructura de un programa en Arduino
```
/* 1:  Declaración de libreria      */  #include <SFEMP3Shield.h>
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
/* 8:  Subrutina de ejecución inf.  */  void loop(){...}
```

## Arduino IDE
Para descargar el software ir a la [página oficial](https://www.arduino.cc/en/software) (las siguintes imágenes son de la versión _Legacy_ 1.8.X)
[Imagen: No description has been provided for this image]

### Botones
| Botones | Atajo | Descripción |
|---|---|---|
| [Imagen: verify.png] | `Ctrl`+`R` | Verificar (compilar) el código |
| [Imagen: upload.png] | `Ctrl`+`U` | Subir el código al microcontrolador |
| [Imagen: new.png] | `Ctrl`+`N` | Nuevo |
| [Imagen: open.png] | `Ctrl`+`O` | Abrir |
| [Imagen: save.png] | `Ctrl`+`S` | Guardar |
| [Imagen: monitor.png] | `Ctrl`+`Shift`+`M` | Monitor Serial |

## Comandos comúnmente utilizados
- [pinMode](https://www.arduino.cc/reference/en/language/functions/digital-io/pinmode/)
- [digitalWrite](https://www.arduino.cc/reference/en/language/functions/digital-io/digitalwrite/)
- [digitalRead](https://www.arduino.cc/reference/en/language/functions/digital-io/digitalread/)
- [delay](https://www.arduino.cc/reference/en/language/functions/time/delay/)

## Agreguemos las tarjetas
En el Arduino IDE 'archivo' > 'propiedades' y agregamos la siguiente URL para placas extra.
[https://espressif.github.io/arduino-esp32/package_esp32_index.json](https://espressif.github.io/arduino-esp32/package_esp32_index.json)

## Ejemplo 1.1 - Parpadeo
- 💾 [Código arduino para el parpadeo en Arduino MEGA](../../assets/code/arduino/discretos/parpadeo.ino)
- 💾 [Código arduino para el parpadeo en ESP32 S3](../../assets/code/esp32/discretos/esp_parpadeo.ino)

## Declaración IF
- Se usa en conjunto con operadores de comparación o funciones que retornen un booleano.
- Verificar si la condición se cumple, de cumplirse, ejecuta las acciones deseadas y luego continua con el programa.
### Sintaxis
```
if (condition) { //Do something here
}
else if (othercondition){ //Do something else if the first condition wasn’t met but the othercondition was met
}
else { //Do something here in other case
}
```

## Ejemplo 1.2 - Declaración IF con entrada externa
- 💾 [Código arduino para la declaración _if_ en Arduino MEGA](../../assets/code/arduino/discretos/declaracion_if.ino)
- 💾 [Código arduino para la declaración _if_ en ESP32 S3](../../assets/code/esp32/discretos/esp_declaracion_if.ino)

## Declaración SWITCH
- Permite tener diferentes acciones dependiendo de los valores de un variable.
- Es similar a tener multiples `if` y `else if` para la misma variable con diferentes valores.
- Cada caso es un posible valor para la variable puede tener y se termina con `break`.
### Sintaxis
```
switch ( var ){
   case 0: 
      //Do something here if var is equal to zero
      break;
   case 1:
      //Do something here if var is equal to one
      break;
   case 2:
      //Do something here if var is equal to two
   break;
}
```

## Ejemplo 1.3 - Declaración SWITCH
- 💾 [Código arduino para la declaración _switch_ en Arduino MEGA](../../assets/code/arduino/discretos/declaracion_switch.ino)
- 💾 [Código arduino para la declaración _switch_ en ESP32 S3](../../assets/code/esp32/discretos/esp_declaracion_switch.ino)

# Parte 2 : Introducción a sensores

## Contenidos
1. Introducción
2. Sensores analógicos
3. Sensores digitales
4. Sensores especializados
**¿Que diferencias hay entre estos sensores?**

## Sensores analógicos
- Generalmente poseen una salida en voltaje. $0V - 5V$
- Requieren un conversor analógicos - digital (ADC)
- Los Arduinos tienen integrados un ADC de 10 bits
$$2^{10\text{ bits}} = 1024\text{ valores} \qquad \text{(incluyendo el 0 hasta 1023)}$$
- Los ESP32 tienen integrados un ADC de 12 bits
$$2^{12\text{ bits}} = 4096\text{ valores} \qquad \text{(incluyendo el 0 hasta 4095)}$$
- Se usa la función `analogRead(PIN)` para su adquisición

### Potentiometer (ejemplo sensor analógico)
[Imagen: potentiometer.jpg]

### Foto-resistencia (ejemplo sensor analógico)
**LDR:** _Light Depedent Resistor_
[[Imagen: ldr.jpg]](http://www.reuk.co.uk/wordpress/electric-circuit/light-dependent-resistor/)

## Sensores digitales - binarios
- Funcionan como un suiche
- Poseen un 1 bit de información lo que les permite tener solo dos estados:
| Nivel | Voltaje | Función |
|---|---|---|
| HIGH | 5V | ON |
| LOW | 0V | OFF |
[[Imagen: voltage-levels.png]](https://daumemo.com/several-methods-on-how-to-connect-different-voltage-signal-lines-together/)
- Se usa la función `digitalRead(PIN)` para la adquisición de la información

### Suiche (ejemplo sensor binario)
[Imagen: No description has been provided for this image] (src: switch.jpg)

### ¿Cómo funciona un micro suiche?

```python
display(IFrame(
    "https://www.youtube-nocookie.com/embed/JmUinwXsQc4?controls=0",
    width="100%",
    height="450px"))
```
[Video incrustado: https://www.youtube-nocookie.com/embed/JmUinwXsQc4?controls=0]

## Sensores especializados
- Son sensores digitales que usan señales pulsadas
- La informacion esta codificada en un flujo de 1 bit
- Se usan librerias en Arduino para cada sensor
- Usualmente el sensor tiene un microprocesador que gestiona el protocolo
### Protocolos utilizados
- One-wire
- UART
- I2C
- SPI

## ¿Qué tipos de sensores de temperatura son estos?
| PT100$^1$ | DHT11$^2$ | Termostato$^3$ |
|---|---|---|
| [Imagen: No description has been provided for this image] | [Imagen: No description has been provided for this image] | [Imagen: No description has been provided for this image] |

1. Sensor analógico, resistencia dependiente de la temperatura [PT100](https://www.grainger.com/product/36P558).
2. Sensor especializado de temperatura DHT11.
3. Sensor binario, [suiche bimetálico](https://es.made-in-china.com/co_thermostat-china/product_RS-03-Bimetallic-Thermostat-for-Dish-Dryer-etc-Amt-250-10A-Thermal-Switch_ryneeereg.html).

## Ejemplos de sensores en Arduino
### Lectura de sensor analógico - fotoresistencia
- 💾 [Lectura en bits de Fotoresistencia](../../assets/code/arduino/discretos/fotoresistencia_bits.ino)
- 💾 [Lectura en voltios de Fotoresistencia](../../assets/code/arduino/discretos/fotoresistencia_volts.ino)
### Lectura de sensor especializado - DHT11
- 💾 [Lectura de temperatura y humedad DHT11](../../assets/code/arduino/smart_weather/lectura_temperatura_humedad.ino)

# Parte 3: Máquinas de Estados Finitos
También llamados **"Automatas"** de estados finitos.
_Modela el comportamiento de un sistema con un número limitado de modos o estados_

## Las MEF desde la auto-regulación
La máquina de estados finitos o MEF, también llamada autómata finito, está relacionada con el órgano de control del sistema. A diferencia de lo que se puede pensar, la MEF es una abstracción de las decisiones que el control tomará para satisfacer todas las necesidades o requerimientos en el proceso. Las MEF son representadas por diagramas y pueden ser programadas en cualquier lenguaje de programación.

## Secador de manos
**Ejemplo de un sistema "autonomo"**

```python
display(IFrame(
    "https://www.youtube-nocookie.com/embed/wZmwFvlBzyE?controls=0",
    width="100%",
    height="450px"))
```
[Video incrustado: https://www.youtube-nocookie.com/embed/wZmwFvlBzyE?controls=0]

## Secador de manos
Antes de pensar en la MEF que controla el secador, responde las siguientes preguntas:
- ¿Tiene sensores y actuadores el secador?
- ¿Cuál es el sensor?
- ¿Cuál es el actuador?

Efectivamente el secador tiene un sensor, un detector de presencia para sabe si hay o no una mano que quiere ser secada y tiene un actuador, un ventilador que genera un flujo fuerte de aire, secando así las manos.

## Construcción de la MEF de control
### Diagrama de flujo de materia
[Imagen: flujo-secador.gv.svg](../../assets/diagrams/flujo-secador.gv.svg)

### Descripción del proceso
_El proceso del secador inicia cuando una persona acerca la mano a la zona de secado. Si se detecta un objeto en la zona de secado, el secador generará un flujo de aire que terminará secando el objeto_.
### Requerimientos del secador
- _Para su correcto funcionamiento, el secador debe tener un sensor que detecte la presencia de un objeto a ser secado, y de un ventilador para generar el secado_
- _El sensor será denominado S, y responderá a la pregunta ¿estoy detectando presencia? con verdadero (1) o falso (0)_
- _El actuador será denominado V, y esperará una respuesta verdadera (1) o falsa (0) a la pregunta ¿debo ventilar?_

## Reglas de control
Una vez tenemos los requerimientos del sistema podemos proceder al paso a paso, en donde los escribiremos usando la estructura SIEEE (o _IFTTT_).
- _Si_ el sensor detecta presencia _entonces_ ventilar.
- _Si no_ detecta presencia el sensor _entonces no_ ventilar.
Estos pasos pueden escribirse también de la siguiente forma:
- _Si S entonces V_.
- _Si no S entonces no V_.

## MEF del control del secador
[Imagen: secador-mef.gv.svg](../../assets/diagrams/secador-mef.gv.svg)

## Anátomia de una MEF
| Elemento | Representación | Descripción |
|---|---|---|
| Estado | Círculo | Define el comportamiento de la máquina y genera la orden a cada actuador. |
| Transición | Flecha | Son los cambios de **estado** generados por la **expresión** que la acompaña. |
| Expresión | Ecuación Booleana | También llamadas **condiciones**, son las reglas que se deben cumplir para generar la transición. |
| Transición Inicial | Flecha que no proviene de un **estado** | Toda MEF tiene un **estado** inicial que se indica con esta flecha. |
| Evento | No se representa | Cambios en las variables que hacen parte de las expresiones de la MEF |

## Las acciones de una MEF
Las acciones pueden ser ubicadas:
- En el estado (dentro del círculo) generando una salida persistente.
- En la transición (en la flecha) generando una salida transitoria.

```python
sourcefile = 'secador-mef2.gv'
gv = open(sourcefile)
dot = Source(gv.read(),format="svg",filename = sourcefile)
dot.render(sourcefile,view=False);
```

Adicional a los elementos clásico de las MEF, las MEF híbridas tienen los siguientes elementos:
| Elemento | Descripción |
|---|---|
| Contador | Son registros que varían de forma ascendente o descendente dependiendo del interés del diseño. La variación es transitoria, por lo que se genera en la transición. |
| Temporizador | Estos nos permiten conocer el tiempo y tomar decisiones basados en él. |
| Time-out | Son temporizadores que evitan que el proceso permanezca en un estado en caso de fallo de la transición principal. |
Si volvemos a ver el vídeo del secado de manos, podemos notar que el ventilador no se apaga inmediatamente luego de que se quita la mano. Por lo que podemos pensar que el control usa temporizadores. Un MEF con un temporizador (**T**) se vería como sigue:
[Imagen: secador-mef2.gv.svg](../../assets/diagrams/secador-mef2.gv.svg)
Si leemos la MEF, diremos:
- El secador empieza en "Espera" con el ventilador apagado.
- Si detecta un objeto con el sensor (`S=1` o simplemente `S`), entonces empieza a "Secar" encendiendo el ventilador (`V=1`).
- Si está "Secando" y deja de detectar el objeto (`S=0` o también `!S`), entonces inicializa un temporizador en cero (`/T=0`) y continúa con el ventilador encendido (`V=1`) esperando "Parar".
- Si está esperando "Parar" y se cumple el tiempo en el temporizador de más de 2 segundos (`T > 2`), entonces apaga el ventilador (`V=0`) y queda a la "Espera".
Para entender mejor la anatomia de las MEF, podemos ver el siguiente vídeo:

```python
display(IFrame(
    "https://www.youtube-nocookie.com/embed/5KoRMjeFCZo?controls=0",
    width="100%",
    height="450px"))
```
[Video incrustado: https://www.youtube-nocookie.com/embed/5KoRMjeFCZo?controls=0]

## Definición formal de Automata
Un automata (máquina) de estados finitos esta definido por una tupla de 5 elementos:
$$M = (S,I,f,S_0,F)$$
donde:
- $S$ es el conjunto de estados finitos.
- $I$ es el conjunto de expresiones de entrada.
- $f:S\times I->S$ es la función de transición entre cada par estado-expresión hacia el siguiente estado.
- $S_0 \subset S$ es el estado inicial (solo uno).
- $F\subseteq S$ es el conjunto de estados finales.

## Ejemplo de la definición formal
Tomemos la siguiente definición:
$$\begin{align}
S &= \left\{1,2,3,4\right\} \\
I &= \left\{a,b\right\} \\
f &= \left\{(1,a,2),(1,b,4),(2,a,3),(2,b,4),(3,a,3),(3,b,3),(4,a,2),(4,b,4)\right\} \\
S_0 &= \left\{1\right\} \\
F &= \left\{4\right\} 
\end{align}$$
Que resulta en:
[Imagen: No description has been provided for this image]

## Ejercicio: Torniquete
¿Cómo sería la MEF de un torniqute?
[Imagen: torniquete.png]

## Ejercicio: Torniquete
Construir la definición formal a partir de la MEF
[Imagen: No description has been provided for this image]

$$\begin{align}
S = &\left\{Bloqueado,Desbloqueado\right\} \\
I = &\left\{Mover,Moneda\right\} \\
f = &\left\{\right. (Bloqueado,Mover,Bloqueado), \\ &(Bloqueado,Moneda,Desbloqueado), \\
    &(Desbloqueado,Moneda,Desbloqueado), \\ &(Desbloqueado,Mover,Bloqueado)\left. \right\} \\
S_0 = &\left\{Bloqueado\right\} \\
F = &\left\{\right\} 
\end{align}$$

> Contenido extraído de la presentación de diapositivas (Jupyter/Reveal.js) enlazada desde la página de taller (docs/taller/discretos.md) como "Diapositivas"/"PDF". Copia HTML original guardada en assets/slides/sistemas-discretos.slides.html. Algunas imágenes ilustrativas del deck (fotos de placas, iconos de botones, capturas del IDE) no se pudieron descargar por no tener URL absoluta identificable y se dejan solo como descripción [Imagen: ...].

