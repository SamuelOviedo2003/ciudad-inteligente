#define LR1 5   // Red traffic light 1 connected in pin 5
#define LY2 15  // Yellow traffic light 2 connected in pin 15

#include "TimerMEF.h"

Timer t_rojo;
int estado_rojo = 0;
bool rojo = 0;

Timer t_amar;
int estado_amar = 0;
bool amar = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LR1, OUTPUT);
  digitalWrite(LR1, 0);
  pinMode(LY2, OUTPUT);
  digitalWrite(LY2, 0);
  t_rojo = 0.0;
  t_amar = 0;
}

void loop() {
  switch (estado_rojo) {
    case 0:
      rojo = 0;
      if (t_rojo > 0.5) {
        estado_rojo = 1;
        t_rojo = 0;
      }
      break;
    case 1:
      rojo = 1;
      if (t_rojo > 0.5) {
        estado_rojo = 0;
        t_rojo = 0;
      }
      break;
    default:
      break;
  }
  switch (estado_amar) {
    case 0:
      amar = 0;
      if (t_amar > 0.3) {
        estado_amar = 1;
        t_amar = 0;
      }
      break;
    case 1:
      amar = 1;
      if (t_amar > 0.3) {
        estado_amar = 0;
        t_amar = 0;
      }
      break;
    default:
      break;
  }

  digitalWrite(LR1, rojo);
  digitalWrite(LY2, amar);
}
