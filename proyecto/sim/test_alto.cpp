// Escenarios de simulacion para nivel_alto.ino (agente Q-learning) con tiempo virtual.
#include "Arduino.h"
#include "LiquidCrystal_I2C.h"
#include "prototypes_alto.h"
#include "nivel_alto.ino"
#include <vector>
#include <string>

using namespace sim;
static int tests = 0, fails = 0;
#define CHECK(cond, msg) do { tests++; if (cond) printf("  PASS %s\n", msg); else { fails++; printf("  FAIL %s\n", msg); } } while (0)
#define CHECK_NEAR(val, exp, tol, msg) do { double _v = (val), _e = (exp); tests++; if (fabs(_v - _e) <= (tol)) printf("  PASS %s (%.3f)\n", msg, _v); else { fails++; printf("  FAIL %s: medido %.3f, esperado %.3f\n", msg, _v, _e); } } while (0)
static long violaciones = 0;

char faseLuces() {
  bool g1 = pin_level[LG1], y1 = pin_level[LY1], r1 = pin_level[LR1];
  bool g2 = pin_level[LG2], y2 = pin_level[LY2], r2 = pin_level[LR2];
  if (g1 && r2 && !y1 && !r1 && !g2 && !y2) return 'A';
  if (y1 && r2 && !g1 && !r1 && !g2 && !y2) return 'B';
  if (r1 && g2 && !g1 && !y1 && !r2 && !y2) return 'C';
  if (r1 && y2 && !g1 && !y1 && !r2 && !g2) return 'D';
  if (!r1 && !r2 && !g1 && !g2) return 'N';
  return '?';
}
void tick() {
  loop(); now_ms += 1;
  int s1 = pin_level[LR1] + pin_level[LY1] + pin_level[LG1];
  int s2 = pin_level[LR2] + pin_level[LY2] + pin_level[LG2];
  char f = faseLuces();
  if (pin_level[LG1] && pin_level[LG2]) violaciones++;
  if (f != 'N' && (s1 != 1 || s2 != 1)) violaciones++;
}
double t() { return now_ms / 1000.0; }
void runFor(double s) { unsigned long fin = now_ms + (unsigned long)(s * 1000); while (now_ms < fin) tick(); }
double runUntilPhase(char f, double maxS = 60) { double t0 = t(); while (faseLuces() != f && t() - t0 < maxS) tick(); return t() - t0; }
double measurePhase() { char f = faseLuces(); double t0 = t(); while (faseLuces() == f && t() - t0 < 120) tick(); return t() - t0; }
void enviar(const char* s) { for (const char* p = s; *p; p++) serial_in.push_back(*p); }
void ambienteNormal() {
  analog_value[LDR1] = 3282; analog_value[LDR2] = 3282; analog_value[CO2] = 3282;
  for (int p : {CNY1, CNY2, CNY3, CNY4, CNY5, CNY6}) pin_level[p] = HIGH;
  pin_level[P1] = HIGH; pin_level[P2] = HIGH;
  random_value = 500;  // random(1000)=500 >= 100 -> nunca explora, salvo que el escenario lo cambie
}

int main(int argc, char** argv) {
  std::string esc = argc > 1 ? argv[1] : "baseline";
  ambienteNormal();
  printf("== %s ==\n", esc.c_str());
  if (esc == "baseline") {
    setup(); tick();
    CHECK(faseLuces() == 'A', "arranca en fase A");
    CHECK(agente[0].estado == 0 && agente[0].accion == mejorAccion(0, 0), "via 1 sin trafico: estado 0 y la mejor accion de la tabla");
    CHECK_NEAR(measurePhase(), ACCION_VERDE[agente[0].accion], 0.01, "A dura lo que dice la accion elegida");
    CHECK_NEAR(measurePhase(), 2, 0.01, "B = 2 s (regla fija)");
    CHECK_NEAR(measurePhase(), ACCION_VERDE[agente[1].accion], 0.01, "C dura lo que dice la accion elegida");
    CHECK_NEAR(measurePhase(), 2, 0.01, "D = 2 s");
    CHECK(mejorAccion(0, 0) == 0 && mejorAccion(1, 0) == 0, "con las vias vacias la tabla prefiere el verde corto");
    CHECK(violaciones == 0, "sin violaciones de luces");
  } else if (esc == "tabla_inicial") {
    pin_level[CNY1] = LOW; pin_level[CNY2] = LOW; pin_level[CNY3] = LOW;  // via 1 llena, via 2 vacia
    setup(); tick();
    CHECK(agente[0].estado == 3, "via 1: cola propia 3, otra 0 -> estado 3");
    CHECK_NEAR(measurePhase(), 8, 0.01, "via 1 llena: elige el verde largo (8 s)");
    measurePhase();
    CHECK(agente[1].estado == 12, "via 2: cola propia 0, otra 3 -> estado 12");
    CHECK_NEAR(measurePhase(), 3, 0.01, "via 2 vacia con la otra llena: verde corto (3 s)");
    runFor(0.5);  // en D: cambiar el contexto antes de que la via 1 vuelva a decidir
    for (int p : {CNY1, CNY2, CNY3}) pin_level[p] = HIGH;
    analog_value[CO2] = 1000;  // eco
    measurePhase();  // resto de D
    CHECK_NEAR(measurePhase(), ACCION_VERDE[mejorAccion(0, 16)], 0.01, "con CO2 alto y vias vacias dura lo que la tabla dice para el estado 16");
    CHECK(agente[0].estado == 16, "el bit eco se refleja en el estado (16)");
  } else if (esc == "recompensa") {
    pin_level[CNY1] = LOW;  // un vehiculo en via 1
    setup(); tick();
    double verde = ACCION_VERDE[agente[0].accion];
    CHECK(agente[0].accion == mejorAccion(0, 1), "cola 1 -> la mejor accion de la tabla para el estado 1");
    runFor(1.5); pin_level[CNY1] = HIGH;  // el vehiculo sale a los 1.5 s
    measurePhase();  // termina A
    printf("  verde=%.0f pasaron=%d verdeVacio=%.3f espera=%.3f\n", verde, agente[0].pasaron, agente[0].verdeVacio, agente[0].espera);
    CHECK(agente[0].pasaron == 1, "cuenta 1 vehiculo que paso (transicion detectado -> libre)");
    CHECK_NEAR(agente[0].verdeVacio, verde - 1.5, 0.02, "el resto del verde cuenta como via vacia");
    CHECK_NEAR(agente[0].espera, 1.5, 0.02, "1.5 vehiculo-segundos de espera visible (el carro estuvo 1.5 s)");
    CHECK(agente[0].pendiente, "el paso queda abierto hasta la proxima decision de la via 1");
    pin_level[CNY4] = LOW; pin_level[CNY5] = LOW;  // dos esperando en via 2 durante el resto del ciclo
    runUntilPhase('A'); tick();  // cierra el paso de la via 1
    printf("  paso cerrado: pasaron=%d verdeVacio=%.3f espera=%.3f r=%.3f\n", agente[0].pasaronPaso, agente[0].verdeVacioPaso, agente[0].esperaPaso, agente[0].recompensa);
    CHECK(agente[0].esperaPaso > 1.5, "la espera del paso incluye a los que esperaron en la otra via durante el resto del ciclo");
    CHECK_NEAR(agente[0].duracionPaso, verde + 2 + ACCION_VERDE[agente[1].accion] + 2, 0.02, "el paso duro un ciclo completo (A+B+C+D)");
    CHECK_NEAR(agente[0].recompensa, (0.5 * agente[0].pasaronPaso - 0.1 * agente[0].esperaPaso - 0.1 * agente[0].verdeVacioPaso) * 10 / agente[0].duracionPaso, 0.001, "r = (0.5*pasaron - 0.1*espera - 0.1*verdeVacio) normalizado a 10 s");
    CHECK(!agente[0].pendiente, "y el paso queda cerrado");
  } else if (esc == "aprendizaje") {
    setup(); tick();
    float q0 = Q[0][0][0];
    float maxSiguiente = Q[0][0][mejorAccion(0, 0)];  // s' sera el mismo estado 0; se toma antes de actualizar
    measurePhase();  // A (3 s, vacia): r = -0.9
    runUntilPhase('A'); tick();  // segunda decision de la via 1: cierra la transicion anterior
    float esperado = q0 + ALPHA * (agente[0].recompensa + GAMMA * maxSiguiente - q0);
    printf("  Q[0][0][0]: %.4f -> %.4f (r=%.2f)\n", q0, Q[0][0][0], agente[0].recompensa);
    CHECK_NEAR(Q[0][0][0], esperado, 0.0005, "actualizacion Q-learning: q += alpha*(r + gamma*max q' - q)");
    CHECK(agente[0].recompensa < 0, "un verde con la via vacia tiene recompensa negativa");
    for (int i = 0; i < 40; i++) { runUntilPhase('A'); measurePhase(); }
    printf("  tras 40 ciclos vacios: Q[0][0] = %.3f %.3f %.3f\n", Q[0][0][0], Q[0][0][1], Q[0][0][2]);
    CHECK(mejorAccion(0, 0) == 0, "con las vias siempre vacias sigue prefiriendo el verde corto (el largo castiga mas)");
    CHECK(Q[0][0][0] > Q[0][0][2], "y el verde largo vale menos que el corto en ese estado");
    CHECK(violaciones == 0, "sin violaciones de luces mientras aprende");
  } else if (esc == "exploracion") {
    setup(); tick();
    random_value = 50;  // random(1000)=50 < 100 -> explora; random(3)=50%3=2 -> accion 8 s
    runUntilPhase('A', 30); measurePhase();  // termina la A actual
    runUntilPhase('A'); tick();
    CHECK(agente[0].exploro && agente[0].accion == 2, "con epsilon explora y elige una accion distinta a la mejor");
    CHECK_NEAR(measurePhase(), 8, 0.01, "el verde explorado dura 8 s");
    runFor(1.2);
    CHECK(serial_out.find("explora1=1") != std::string::npos, "la telemetria reporta la exploracion");
  } else if (esc == "reglas_fijas") {
    setup(); tick();
    enviar("LLUVIA=1\nDET_REMOTO=5\n"); tick();
    measurePhase();
    CHECK_NEAR(measurePhase(), 3, 0.01, "amarillo con lluvia = 3 s (regla, no la decide el agente)");
    CHECK_NEAR(measurePhase(), 5, 0.01, "verde 3 s + 2 s por la otra maqueta congestionada (regla de red sobre la decision)");
    enviar("LLUVIA=0\nDET_REMOTO=0\n"); tick();
    runUntilPhase('A'); runFor(0.5);
    pin_level[P1] = LOW;
    double resto = measurePhase(); pin_level[P1] = HIGH;
    CHECK_NEAR(resto + 0.5, 2, 0.02, "peaton con via libre: el verde se corta al verde minimo (2 s), por encima del agente");
    runUntilPhase('C'); tick();  // en C se borra la peticion peatonal (que tiene prioridad sobre el nocturno)
    analog_value[LDR1] = 300; analog_value[LDR2] = 300; runFor(0.5);
    CHECK(modoNocturno, "nocturno sigue siendo una regla fija");
    analog_value[LDR1] = 3282; analog_value[LDR2] = 3282; tick();
    CHECK(!modoNocturno && faseLuces() == 'A', "sale del nocturno a fase A");
    CHECK(violaciones == 0, "sin violaciones de luces");
  } else if (esc == "memoria") {
    setup(); tick();
    float inicial = Q[0][0][0];
    CHECK(!tablaDesdeFlash && sim::prefs.count("q") == 0, "sin nada en flash arranca con la tabla inicial (entrenada o heuristica)");
    for (int i = 0; i < 6; i++) { runUntilPhase('A'); measurePhase(); }  // 12 decisiones -> se guardo sola a las 10
    CHECK(sim::prefs.count("q") == 1 && sim::prefs["q"].size() == sizeof(Q), "tras 10 decisiones la tabla se guardo en flash (768 bytes)");
    enviar("Q_SAVE\n"); tick();
    float aprendido = Q[0][0][0];
    CHECK(fabs(aprendido - inicial) > 0.01, "el valor ya difiere de la tabla inicial (aprendio)");
    inicializarQ();                        // simula el reinicio: la RAM vuelve a la tabla inicial...
    CHECK_NEAR(Q[0][0][0], inicial, 0.0001, "(RAM reiniciada a la tabla inicial)");
    cargarQ();                             // ...y setup() la recupera de la flash
    CHECK_NEAR(Q[0][0][0], aprendido, 0.0001, "cargarQ() recupera lo aprendido: la experiencia sobrevive al reinicio");
    CHECK(tablaDesdeFlash, "y lo reporta (nvs=1)");
    serial_out.clear(); enviar("Q_RESET\n"); tick();
    CHECK(sim::prefs.count("q") == 0 && fabs(Q[0][0][0] - inicial) < 0.0001 && !tablaDesdeFlash, "Q_RESET borra la flash y vuelve a la tabla inicial");
    CHECK(serial_out.find("Q_RESET ok") != std::string::npos, "Q_RESET responde");
    serial_out.clear(); enviar("Q_DUMP\n"); tick();
    int lineas = 0; size_t pos = 0; while ((pos = serial_out.find("\nQ ", pos)) != std::string::npos) { lineas++; pos++; }
    CHECK(lineas == 64 && serial_out.find("Q fin") != std::string::npos, "Q_DUMP vuelca 2 vias x 32 estados y termina en 'Q fin'");
    enviar("EPSILON=1\n"); tick();
    int exploraciones = 0;
    for (int i = 0; i < 5; i++) { runUntilPhase('A'); tick(); exploraciones += agente[0].exploro; measurePhase(); }
    CHECK(exploraciones == 5, "con EPSILON=1 todas las decisiones exploran");
    enviar("EPSILON=0\n"); tick();
    runUntilPhase('A'); tick(); measurePhase(); runUntilPhase('A'); tick();
    CHECK(!agente[0].exploro, "con EPSILON=0 nunca explora");
  } else if (esc == "telemetria_lcd") {
    pin_level[CNY1] = LOW; pin_level[CNY4] = LOW; pin_level[CNY5] = LOW;
    setup(); tick(); runFor(20);
    size_t p = serial_out.find("nivel=alto"); size_t e = serial_out.find("\r\n", p);
    std::string linea = serial_out.substr(p, e - p);
    printf("  %s\n", linea.c_str());
    for (const char* k : {"nivel=alto", "modo=", "fase=", "dur=", "det=", "s1=", "a1=", "explora1=", "r1=", "rtotal1=", "q1=", "s2=", "q2=", "nvs=0", "eps=0.10"}) {
      char msg[64]; snprintf(msg, sizeof msg, "telemetria contiene %s", k);
      CHECK(linea.find(k) != std::string::npos, msg);
    }
    printf("  desbordes LCD: %d, clears: %d\n", lcd.overflows, lcd.clears);
    for (auto& s : lcd.overflow_samples) printf("    %s\n", s.c_str());
    CHECK(lcd.overflows == 0 && lcd.clears == 0, "LCD sin desbordes ni clear()");
    anuncioActual = 0; mostrarAnuncio();
    printf("  pantalla agente: '%s' | '%s' | '%s' | '%s'\n", lcd.line(0).c_str(), lcd.line(1).c_str(), lcd.line(2).c_str(), lcd.line(3).c_str());
  } else { printf("escenario desconocido\n"); return 2; }
  printf("  -> %d/%d OK, violaciones: %ld\n", tests - fails, tests, violaciones);
  return fails ? 1 : 0;
}
