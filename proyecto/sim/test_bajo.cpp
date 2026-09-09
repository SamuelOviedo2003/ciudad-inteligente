// Escenarios de simulacion para nivel_bajo.ino con tiempo virtual.
#include "Arduino.h"
#include "LiquidCrystal_I2C.h"
#include "prototypes_bajo.h"
#include "nivel_bajo.ino"
#include <vector>
#include <string>

using namespace sim;
static int tests = 0, fails = 0;
#define CHECK(cond, msg) do { tests++; if (cond) printf("  PASS %s\n", msg); else { fails++; printf("  FAIL %s\n", msg); } } while (0)
#define CHECK_NEAR(val, exp, tol, msg) do { double _v = (val), _e = (exp); tests++; if (fabs(_v - _e) <= (tol)) printf("  PASS %s (%.3f s)\n", msg, _v); else { fails++; printf("  FAIL %s: medido %.3f s, esperado %.3f s\n", msg, _v, _e); } } while (0)
static long violaciones = 0;

char faseLuces() {
  bool g1 = pin_level[LG1], y1 = pin_level[LY1], r1 = pin_level[LR1];
  bool g2 = pin_level[LG2], y2 = pin_level[LY2], r2 = pin_level[LR2];
  if (g1 && r2 && !y1 && !r1 && !g2 && !y2) return 'A';
  if (y1 && r2 && !g1 && !r1 && !g2 && !y2) return 'B';
  if (r1 && g2 && !g1 && !y1 && !r2 && !y2) return 'C';
  if (r1 && y2 && !g1 && !y1 && !r2 && !g2) return 'D';
  return '?';
}
void tick() {
  loop(); now_ms += 1;
  int s1 = pin_level[LR1] + pin_level[LY1] + pin_level[LG1];
  int s2 = pin_level[LR2] + pin_level[LY2] + pin_level[LG2];
  if ((pin_level[LG1] && pin_level[LG2]) || s1 != 1 || s2 != 1) violaciones++;
}
double t() { return now_ms / 1000.0; }
void runFor(double s) { unsigned long fin = now_ms + (unsigned long)(s * 1000); while (now_ms < fin) tick(); }
double measurePhase() { char f = faseLuces(); double t0 = t(); while (faseLuces() == f && t() - t0 < 120) tick(); return t() - t0; }
void enviar(const char* s) { for (const char* p = s; *p; p++) serial_in.push_back(*p); }

int main(int argc, char** argv) {
  std::string esc = argc > 1 ? argv[1] : "baseline";
  analog_value[LDR1] = 0; analog_value[LDR2] = 0; analog_value[CO2] = 0;  // Wokwi: potenciometros en 0 (nivel_bajo no fija value)
  for (int p : {CNY1, CNY2, CNY3, CNY4, CNY5, CNY6}) pin_level[p] = HIGH;
  pin_level[P1] = HIGH; pin_level[P2] = HIGH;
  printf("== %s ==\n", esc.c_str());
  if (esc == "baseline") {
    setup(); tick();
    CHECK(faseLuces() == 'A', "arranca en A");
    CHECK_NEAR(measurePhase(), 5, 0.01, "A = 5 s");
    CHECK_NEAR(measurePhase(), 2, 0.01, "B = 2 s");
    CHECK_NEAR(measurePhase(), 5, 0.01, "C = 5 s");
    CHECK_NEAR(measurePhase(), 2, 0.01, "D = 2 s");
    // sensores y botones cambiando a lo loco: no deben alterar nada
    for (int p : {CNY1, CNY2, CNY3, CNY4, CNY5, CNY6}) pin_level[p] = LOW;
    pin_level[P1] = LOW; pin_level[P2] = LOW; analog_value[LDR1] = 4095; analog_value[CO2] = 1000;
    CHECK_NEAR(measurePhase(), 5, 0.01, "A sigue 5 s con todos los sensores activos");
    CHECK_NEAR(measurePhase(), 2, 0.01, "B sigue 2 s");
    CHECK(violaciones == 0, "sin violaciones de luces");
  } else if (esc == "telemetria") {
    setup(); pin_level[CNY1] = LOW; pin_level[P2] = LOW;  // despues de setup: pinMode(INPUT_PULLUP) del mock pone HIGH
    tick(); runFor(1.2);
    size_t p = serial_out.find("{"); size_t e = serial_out.find("}", p);
    std::string j = serial_out.substr(p, e - p + 1);
    printf("  json: %s\n", j.c_str());
    CHECK(j.find("\"det\":1") != std::string::npos, "det cuenta CNY en LOW como detectado");
    CHECK(j.find("\"cny\":[0,1,1,1,1,1]") != std::string::npos, "cny[] va crudo (1 = libre, 0 = detectado): distinto criterio que det");
    CHECK(j.find("\"p2\":1") != std::string::npos, "p2=1 al presionar");
  } else if (esc == "remoto") {
    setup(); tick();
    enviar("{\"fase\":\"C\",\"co2\":-1,\"ldr\":[1,2],\"cny\":[1,1,1,1,1,1],\"det\":3,\"p1\":0,\"p2\":0}\n"); tick();
    CHECK(detectadosRemoto == 3, "lee det=3 de la otra maqueta");
    anuncioActual = 2; mostrarAnuncio();
    printf("  LCD: '%s' / '%s'\n", lcd.line(0).c_str(), lcd.line(1).c_str());
    CHECK(lcd.line(1) == "Detectados: 3/12", "LCD suma local + remoto sobre 12");
    enviar("fragmento sin llave\n"); tick();
    CHECK(detectadosRemoto == 3, "ignora lineas que no empiezan con {");
  } else if (esc == "lcd") {
    for (int p : {CNY1, CNY2, CNY3, CNY4, CNY5, CNY6}) pin_level[p] = LOW;
    analog_value[LDR1] = 4095; analog_value[LDR2] = 4095; analog_value[CO2] = 1000;
    setup(); tick(); runFor(13);
    printf("  desbordes de linea: %d, clears en 13 s: %d\n", lcd.overflows, lcd.clears);
    for (auto& s : lcd.overflow_samples) printf("    %s\n", s.c_str());
    CHECK(lcd.overflows == 0, "ningun texto se pasa de 20 columnas");
  } else { printf("escenario desconocido\n"); return 2; }
  printf("  -> %d/%d OK, violaciones: %ld\n", tests - fails, tests, violaciones);
  return fails ? 1 : 0;
}
