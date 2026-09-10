// Corre un nivel (el .ino que indique NIVEL_INO) contra el modelo de trafico y
// escribe una linea CSV de metricas por patron y semilla. Se compila una vez
// por nivel (ver entrenar.sh) para comparar medio contra alto con el mismo trafico.
// Uso: comparar <etiqueta> [semillas] [segundos por corrida]
#include "Arduino.h"
#include "LiquidCrystal_I2C.h"
#include PROTOTIPOS
#include NIVEL_INO
#include "trafico.h"
#include <cstdio>

using namespace sim;
static const double DT = 0.005;

int main(int argc, char **argv) {
  const char *etiqueta = argc > 1 ? argv[1] : "nivel";
  int semillas = argc > 2 ? atoi(argv[2]) : 3;
  double segundos = argc > 3 ? atof(argv[3]) : 1200;

  analog_value[LDR1] = 3282; analog_value[LDR2] = 3282; analog_value[CO2] = 3282;
  pin_level[P1] = P_REPOSO; pin_level[P2] = P_REPOSO;
  for (int p : {42, 41, 40, 39, 38, 37}) pin_level[p] = HIGH;
  setup();
#ifdef NIVEL_ALTO
  epsilon = 0;  // en la comparacion el agente explota lo aprendido (sigue aprendiendo con alpha)
#endif
  std::mt19937 rngExplora(99);
  Trafico tr;
  for (int pi = 0; pi < NUM_PATRONES; pi++) {
    for (int s = 0; s < semillas; s++) {
      tr.reiniciar(5000 + 100 * pi + s, PATRONES[pi].tasa1, PATRONES[pi].tasa2, TASA_PEATON);
      unsigned long fin = now_ms + (unsigned long)(segundos * 1000);
      while (now_ms < fin) {
        random_value = rngExplora() % 1000;
        tr.paso(now_ms / 1000.0, DT);
        loop();
        now_ms += (unsigned long)(DT * 1000);
        serial_out.clear();
      }
      // etiqueta, patron, semilla, cola media, espera media por vehiculo (s), vehiculos atendidos, descartados, espera media peaton (s), peatones
      printf("%s,%s,%d,%.3f,%.2f,%ld,%ld,%.2f,%ld\n", etiqueta, PATRONES[pi].nombre, s, tr.colaMedia(), tr.esperaMediaVehiculo(),
             tr.salieron, tr.descartados, tr.esperaMediaPeaton(), tr.peatones);
    }
  }
  return 0;
}
