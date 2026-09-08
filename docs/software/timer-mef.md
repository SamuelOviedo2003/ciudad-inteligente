---
source: https://isa262.davinsony.com/software/timer-mef/
title: TimerMEF.h
---

# TimerMEF.h

Para trabajar con tiempos en los microcontroladores es ideal evitar el uso de la función retraso `delay()` ya que esta interrumpe el hilo único que tiene el microcontrolador. Para resolver esta situación en el curso usaremos la siguiente libreria llamada “`TimerMEF.h` (enlace roto en el sitio original: `/arduino/libreria/TimerMEF.h` — no disponible)”, se puede copiar y pegar en una nueva pestaña del Arduino IDE, con el nombre “`TimerMEF.h` (enlace roto en el sitio original: `/arduino/libreria/TimerMEF.h` — no disponible)”:
```cpp
#ifndef TimerMEF_h
#define TimerMEF_h


#include "Arduino.h"


class Timer {
private:
  unsigned long _time;
public:
  Timer(double time) {
    unsigned long ms = (long)millis();
    _time = ms - time * 1000;
  }
  Timer(int time) {
    Timer(static_cast<double>(time));
  }
  Timer() {
    Timer(0);
  }
  double set(double time) {
    unsigned long ms = (long)millis();
    _time = ms - time * 1000;
    return ((double)(ms - _time)) / 1000;
  }
  double get() {
    unsigned long ms = (long)millis();
    return ((double)(ms - _time)) / 1000;
  }
  void operator=(double time) {
    set(time);
  }
  double operator()() {
    return get();
  }
  // User-defined conversion function to double (implicit conversion allowed)
  operator double() const {
    unsigned long ms = (long)millis();
    return (static_cast<double>(ms - _time)) / 1000;
  }
};


#endif
```
Para usarla solo debemos incluirla en el código principal de la siguiente forma:
```cpp
#include "TimerMEF.h" // Incluimos la libreria


Timer t;  // Creamos un temporizador
t = 0;    // Iniciamos un temporizador
if(t > 10) hagaAlgo(); // Usamos el temporizador como una variable double
```
## Ejemplo
Este [archivo](../../assets/code/wokwi/ejemplo-timer/ejemplo-timer.ino) muestra el uso de dos temporizadores en la maqueta de ciudad adaptable uno para el control de una luz roja que parpadea cada 1 segundo (0.5 segundos apagada y 0.5 segundos encendida), y la luz amarilla del otro semaforo que parpadea cada 0.6 segundos (0.3 segundos apagada y 0.3 segundos encendida).
