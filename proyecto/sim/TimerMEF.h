// Copia literal de la libreria del curso (docs/software/timer-mef.md), la misma
// que se instala en ~/Arduino/libraries/TimerMEF/ para compilar con arduino-cli.
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
