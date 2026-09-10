// Escenarios de simulacion para nivel_medio.ino con tiempo virtual.
// Uso: ./sim_medio <escenario>   (un escenario por proceso, porque el .ino
// tiene estado global y setup() solo se puede llamar una vez).
#include "Arduino.h"
#include "LiquidCrystal_I2C.h"
#include "prototypes_medio.h"
#include "nivel_medio.ino"
static const int LDR_OSCURO = UMBRAL_NOCHE_ENTRA / 2;  // lectura LDR claramente "de noche"
#include <vector>
#include <string>

using namespace sim;

static int tests = 0, fails = 0;
#define CHECK(cond, msg) do { tests++; if (cond) printf("  PASS %s\n", msg); else { fails++; printf("  FAIL %s\n", msg); } } while (0)
#define CHECK_NEAR(val, exp, tol, msg) do { double _v = (val), _e = (exp); tests++; if (fabs(_v - _e) <= (tol)) printf("  PASS %s (%.3f s)\n", msg, _v); else { fails++; printf("  FAIL %s: medido %.3f s, esperado %.3f s\n", msg, _v, _e); } } while (0)

static long violaciones = 0;
struct Evento { double t; char fase; };
static std::vector<Evento> eventos;
static char ultimaFase = 0;

char faseLuces() {
  bool g1 = pin_level[LG1], y1 = pin_level[LY1], r1 = pin_level[LR1];
  bool g2 = pin_level[LG2], y2 = pin_level[LY2], r2 = pin_level[LR2];
  if (g1 && r2 && !y1 && !r1 && !g2 && !y2) return 'A';
  if (y1 && r2 && !g1 && !r1 && !g2 && !y2) return 'B';
  if (r1 && g2 && !g1 && !y1 && !r2 && !y2) return 'C';
  if (r1 && y2 && !g1 && !y1 && !r2 && !g2) return 'D';
  if (!r1 && !r2 && !g1 && !g2) return 'N';  // nocturno: solo amarillos (o nada)
  return '?';
}

void tick(int ms = 1) {
  loop();
  now_ms += ms;
  int s1 = pin_level[LR1] + pin_level[LY1] + pin_level[LG1];
  int s2 = pin_level[LR2] + pin_level[LY2] + pin_level[LG2];
  char f = faseLuces();
  if (pin_level[LG1] && pin_level[LG2]) violaciones++;
  if (f != 'N' && (s1 != 1 || s2 != 1)) violaciones++;
  if (f != ultimaFase) { eventos.push_back({now_ms / 1000.0, f}); ultimaFase = f; }
}

double t() { return now_ms / 1000.0; }
void runFor(double s) { unsigned long fin = now_ms + (unsigned long)(s * 1000); while (now_ms < fin) tick(); }
double runUntilPhase(char f, double maxS = 60) { double t0 = t(); while (faseLuces() != f && t() - t0 < maxS) tick(); return t() - t0; }
double measurePhase() { char f = faseLuces(); double t0 = t(); while (faseLuces() == f && t() - t0 < 120) tick(); return t() - t0; }
void enviar(const char* s) { for (const char* p = s; *p; p++) serial_in.push_back(*p); }
void printEventos() { printf("  eventos:"); for (auto& e : eventos) printf(" %c@%.3f", e.fase, e.t); printf("\n"); }

void ambienteNormal() {
  analog_value[LDR1] = 3276; analog_value[LDR2] = 3276; analog_value[CO2] = 3276;  // 80% de 4095
  for (int p : {CNY1, CNY2, CNY3, CNY4, CNY5, CNY6}) pin_level[p] = HIGH;          // pull-up, nada detectado
  pin_level[P1] = P_REPOSO; pin_level[P2] = P_REPOSO;
}

int main(int argc, char** argv) {
  std::string esc = argc > 1 ? argv[1] : "baseline";
  ambienteNormal();
  printf("== %s ==\n", esc.c_str());

  if (esc == "baseline") {
    setup();
    tick();
    CHECK(faseLuces() == 'A', "arranca en fase A (S1 verde, S2 rojo)");
    const char* esperado = "BCDABCDA"; double dur[] = {2, 5, 2, 5, 2, 5, 2, 5};
    for (int i = 0; i < 8; i++) {
      double d = measurePhase();
      char msg[80]; snprintf(msg, sizeof msg, "fase %c dura %.0f s", esperado[i] == 'A' ? 'D' : esperado[i] - 1, dur[i] == 2 ? 5.0 : 2.0);
      (void)msg;
      CHECK(faseLuces() == esperado[i], "orden de fases A->B->C->D");
      (void)d;
    }
    eventos.clear(); runUntilPhase('A');
    CHECK_NEAR(measurePhase(), 5, 0.01, "A (verde S1) = 5 s");
    CHECK_NEAR(measurePhase(), 2, 0.01, "B (amarillo S1) = 2 s");
    CHECK_NEAR(measurePhase(), 5, 0.01, "C (verde S2) = 5 s");
    CHECK_NEAR(measurePhase(), 2, 0.01, "D (amarillo S2) = 2 s");
    CHECK(violaciones == 0, "nunca dos verdes a la vez ni dos luces del mismo semaforo");
  } else if (esc == "cny_polaridad") {
    setup(); tick();
    CHECK(contarVehiculos1() == 0 && contarVehiculos2() == 0, "con todos los CNY en HIGH (reposo) no hay vehiculos");
    pin_level[CNY1] = LOW; pin_level[CNY5] = LOW; tick();
    CHECK(contarVehiculos1() == 1 && contarVehiculos2() == 1, "CNY en LOW cuenta como vehiculo");
  } else if (esc == "congestion") {
    pin_level[CNY1] = LOW; pin_level[CNY2] = LOW;  // via 1 congestionada desde el arranque
    setup(); tick();
    CHECK_NEAR(measurePhase(), 8, 0.01, "A con 2 CNY de via 1 = 5+3 s");
    CHECK_NEAR(measurePhase(), 2, 0.01, "B sigue 2 s");
    CHECK_NEAR(measurePhase(), 5, 0.01, "C (via 2 libre) sigue 5 s");
    runFor(0.5);  // ya en D
    pin_level[CNY1] = HIGH; pin_level[CNY2] = HIGH; pin_level[CNY4] = LOW; pin_level[CNY5] = LOW; pin_level[CNY6] = LOW;
    measurePhase();  // resto de D
    CHECK_NEAR(measurePhase(), 5, 0.01, "A vuelve a 5 s al liberar via 1");
    measurePhase();
    CHECK_NEAR(measurePhase(), 8, 0.01, "C con 3 CNY de via 2 = 8 s");
    CHECK(violaciones == 0, "sin violaciones de luces");
  } else if (esc == "congestion_midfase") {
    setup(); tick();
    runFor(1.0);
    pin_level[CNY1] = LOW; pin_level[CNY2] = LOW;  // llega trafico 1 s despues de empezar el verde
    double resto = measurePhase();
    CHECK_NEAR(resto + 1.0, 5, 0.02, "trafico que llega a mitad de verde NO extiende esa fase (se decide al entrar)");
    printf("  (LCD anuncio 0 dira 'CONG1' pero dur: sigue en 5.0 hasta la proxima A)\n");
    measurePhase(); measurePhase(); measurePhase();
    CHECK_NEAR(measurePhase(), 8, 0.01, "la siguiente A si dura 8 s");
  } else if (esc == "eco") {
    analog_value[CO2] = 1000;
    setup(); tick();
    printf("  CO2 analog=1000 -> %.0f ppm (umbral %d)\n", leerCO2ppm(), UMBRAL_CO2_ECO);
    CHECK(leerCO2ppm() > UMBRAL_CO2_ECO, "con el potenciometro bajo el CO2 calculado supera el umbral ECO");
    CHECK_NEAR(measurePhase(), 7, 0.01, "A con ECO = 5+2 s");
    measurePhase();
    CHECK_NEAR(measurePhase(), 7, 0.01, "C con ECO = 5+2 s");
    analog_value[CO2] = 2400; printf("  CO2 analog=2400 -> %.0f ppm\n", leerCO2ppm());
    analog_value[CO2] = 2800; printf("  CO2 analog=2800 -> %.0f ppm\n", leerCO2ppm());
    analog_value[CO2] = 3276; printf("  CO2 analog=3276 (80%%) -> %.0f ppm\n", leerCO2ppm());
  } else if (esc == "wokwi_value_80") {
    // diagram.json pone "value": "80" en los 3 potenciometros. Segun docs.wokwi.com el
    // atributo va de 0 a 1023, asi que 80 => 80/1023*4095 = 320 en analogRead.
    analog_value[CO2] = 320;
    printf("  value=80 (viejo) -> analogRead=320: LDR<%d? %s ; CO2=%.0f ppm => arrancaba en nocturno + ECO\n", UMBRAL_NOCHE_ENTRA, 320 < UMBRAL_NOCHE_ENTRA ? "si" : "no", leerCO2ppm());
    analog_value[LDR1] = 3282; analog_value[LDR2] = 3282; analog_value[CO2] = 3282;  // value=820 -> 820/1023*4095
    setup(); tick();
    printf("  value=820 (actual) -> analogRead=3282: CO2=%.0f ppm\n", leerCO2ppm());
    CHECK(!modoNocturno && faseLuces() == 'A', "con value=820 el simulador arranca en modo normal (fase A)");
    CHECK(leerCO2ppm() < UMBRAL_CO2_ECO, "con value=820 el modo ECO arranca apagado");
  } else if (esc == "nocturno") {
    analog_value[LDR1] = LDR_OSCURO; analog_value[LDR2] = LDR_OSCURO;
    setup(); tick();
    CHECK(modoNocturno, "con ambos LDR oscuros entra en modo nocturno");
    int toggles = 0, prev = pin_level[LY1]; bool rojosVerdes = false;
    for (int i = 0; i < 3000; i++) { tick(); if (pin_level[LY1] != prev) { toggles++; prev = pin_level[LY1]; } if (pin_level[LR1] || pin_level[LR2] || pin_level[LG1] || pin_level[LG2]) rojosVerdes = true; }
    printf("  LY1 cambio %d veces en 3 s\n", toggles);
    CHECK(toggles >= 5 && toggles <= 7, "amarillo parpadea cada 0.5 s");
    CHECK(!rojosVerdes, "en nocturno no se enciende ningun rojo ni verde");
    analog_value[LDR1] = 3276; analog_value[LDR2] = 3276; tick();
    CHECK(!modoNocturno && faseLuces() == 'A', "al volver la luz retoma en fase A");
    CHECK_NEAR(measurePhase(), 5, 0.01, "y la A dura 5 s");
    analog_value[LDR1] = LDR_OSCURO; tick(); runFor(1);
    CHECK(!modoNocturno, "con un solo LDR oscuro NO entra en nocturno (exige ambos)");
  } else if (esc == "nocturno_histeresis") {
    setup(); tick();
    int entradas = 0; bool prev = modoNocturno;
    for (int i = 0; i < 5000; i++) {
      int v = ((i / 100) % 2) ? UMBRAL_NOCHE_ENTRA - 10 : UMBRAL_NOCHE_ENTRA + 10;  // LDR oscilando alrededor del umbral cada 100 ms
      analog_value[LDR1] = v; analog_value[LDR2] = v;
      tick();
      if (modoNocturno && !prev) entradas++;
      prev = modoNocturno;
    }
    printf("  entradas a nocturno en 5 s con LDR oscilando %d<->%d: %d\n", UMBRAL_NOCHE_ENTRA - 10, UMBRAL_NOCHE_ENTRA + 10, entradas);
    CHECK(entradas <= 1, "no debe entrar/salir de nocturno repetidamente con ruido en el umbral (FALLA = falta histeresis)");
  } else if (esc == "peaton_libre") {
    setup(); tick();
    runFor(1.0);
    pin_level[P1] = P_ACTIVO;
    double resto = measurePhase();
    pin_level[P1] = P_REPOSO;
    CHECK_NEAR(resto, 1, 0.01, "P1 a 1 s de verde con via libre: corta al cumplir el verde minimo (2 s)");
    CHECK(faseLuces() == 'B', "pasa a amarillo (B), no salta directo a rojo");
    CHECK_NEAR(measurePhase(), 2, 0.01, "amarillo normal de 2 s");
    runUntilPhase('A'); runFor(3.0);
    pin_level[P1] = P_ACTIVO; resto = measurePhase(); pin_level[P1] = P_REPOSO;
    CHECK_NEAR(resto, 0, 0.01, "P1 a 3 s de verde con via libre: corta de inmediato");
    CHECK(violaciones == 0, "sin violaciones de luces");
  } else if (esc == "peaton_trafico") {
    pin_level[CNY1] = LOW; pin_level[CNY2] = LOW;  // A = 8 s
    setup(); tick();
    runFor(1.0);
    pin_level[P1] = P_ACTIVO; runFor(0.3); pin_level[P1] = P_REPOSO;
    CHECK(peaton1Pedido, "con trafico el peaton queda 'esperando'");
    double resto = measurePhase();
    CHECK_NEAR(resto + 1.3, 7, 0.02, "con trafico (A=8 s) el peaton que pidio a 1 s cruza a los 6 s de espera (MAX_ESPERA_PEATON), antes de que termine el verde");
  } else if (esc == "peaton_fuera_de_fase") {
    setup(); tick();
    runUntilPhase('C'); runFor(1.0);
    pin_level[P1] = P_ACTIVO; runFor(0.5); pin_level[P1] = P_REPOSO;  // P1 durante C: S1 ya esta en rojo, el peaton cruza ahi
    CHECK(!peaton1Pedido, "una pulsacion de P1 durante C se atiende en el acto (S1 en rojo) y no queda pendiente");
    runUntilPhase('A');
    CHECK_NEAR(measurePhase(), 5, 0.01, "la siguiente A dura 5 s completos");
    runUntilPhase('D'); runFor(0.5);
    pin_level[P1] = P_ACTIVO; runFor(0.3); pin_level[P1] = P_REPOSO;  // P1 durante D: queda memorizado para la proxima A
    CHECK(peaton1Pedido, "una pulsacion de P1 durante D queda memorizada");
    runUntilPhase('A');
    CHECK_NEAR(measurePhase(), 2, 0.01, "y la siguiente A se corta al cumplir el verde minimo de 2 s");
  } else if (esc == "peaton_sostenido") {
    setup(); tick();
    pin_level[P1] = P_ACTIVO;  // boton pegado o sostenido
    double maxVerde = 0, totalVerde = 0; unsigned long t0 = now_ms; double actual = 0;
    while (now_ms - t0 < 30000) { tick(); if (faseLuces() == 'A') { actual += 0.001; totalVerde += 0.001; } else { if (actual > maxVerde) maxVerde = actual; actual = 0; } }
    printf("  en 30 s con P1 sostenido: verde S1 maximo %.3f s, total %.3f s\n", maxVerde, totalVerde);
    CHECK(maxVerde >= 1.0, "S1 conserva un verde minimo aunque P1 este pegado (FALLA = no hay verde minimo, la via 1 se queda sin paso)");
  } else if (esc == "nocturno_peaton") {
    analog_value[LDR1] = LDR_OSCURO; analog_value[LDR2] = LDR_OSCURO;
    setup(); tick(); runFor(2);
    CHECK(modoNocturno, "en nocturno");
    double tPress = t();
    pin_level[P1] = P_ACTIVO; runFor(0.3); pin_level[P1] = P_REPOSO;
    CHECK(!modoNocturno && faseLuces() != 'N', "P1 interrumpe el nocturno");
    while (!modoNocturno && t() - tPress < 60) tick();
    printf("  nocturno volvio %.2f s despues de la pulsacion (SUSPENSION_NOCTURNO=%.0f)\n", t() - tPress, SUSPENSION_NOCTURNO);
    CHECK_NEAR(t() - tPress, 20, 0.35, "vuelve al nocturno ~20 s despues");
    CHECK(violaciones == 0, "sin violaciones de luces durante la transicion");
  } else if (esc == "lluvia") {
    setup(); tick();
    enviar("LLUVIA=1\n"); tick();
    CHECK(lluvia, "LLUVIA=1 se recibe");
    measurePhase();
    CHECK_NEAR(measurePhase(), 3, 0.01, "B con lluvia = 2+1 s");
    measurePhase();
    CHECK_NEAR(measurePhase(), 3, 0.01, "D con lluvia = 2+1 s");
    enviar("LLUVIA=0\r\n"); tick();
    measurePhase();
    CHECK_NEAR(measurePhase(), 2, 0.01, "B vuelve a 2 s con LLUVIA=0");
  } else if (esc == "det_remoto") {
    setup(); tick();
    enviar("DET_REMOTO=5\n"); tick();
    CHECK(detectadosRemoto == 5, "DET_REMOTO=5 se recibe");
    measurePhase(); measurePhase(); measurePhase(); measurePhase();
    CHECK_NEAR(measurePhase(), 7, 0.01, "A con la otra maqueta congestionada = 5+2 s");
    enviar("DET_REMOTO=3\n"); tick();
    measurePhase(); measurePhase(); measurePhase();
    CHECK_NEAR(measurePhase(), 5, 0.01, "A vuelve a 5 s con DET_REMOTO=3");
    enviar("DET_REMOTO=6\n"); tick(); runFor(290);
    CHECK(detectadosRemoto == 6, "el conteo remoto sigue vigente a los 4.8 min");
    runFor(15);
    CHECK(detectadosRemoto == 0, "el conteo remoto caduca a los 5 min sin DET_REMOTO nuevo");
  } else if (esc == "ping") {
    setup(); tick(); runFor(1.0);
    anuncioActual = 0; mostrarAnuncio();
    printf("  LCD fila 3 a t=1 s sin ningun PING: '%s'\n", lcd.line(3).c_str());
    CHECK(lcd.line(3) == "PC: SIN CONEXION", "sin PING el LCD debe decir SIN CONEXION desde el arranque (FALLA = ultimoPingMs=0 se lee como 'conectado' los primeros 5 s)");
    runFor(5); anuncioActual = 0; mostrarAnuncio();
    CHECK(lcd.line(3) == "PC: SIN CONEXION", "a los 6 s sin PING dice SIN CONEXION");
    serial_out.clear(); enviar("PING\n"); tick();
    CHECK(serial_out.find("PONG") != std::string::npos, "PING responde PONG");
    anuncioActual = 0; mostrarAnuncio();
    CHECK(lcd.line(3) == "PC: CONECTADO", "tras PING dice CONECTADO");
    enviar("basura sin salto de linea de mas de doscientos caracteres ...........................................................................................................................................................................");
    runFor(1);
    printf("  bufferSerial tras basura sin \\n: %u bytes\n", bufferSerial.length());
    CHECK(bufferSerial.length() <= 200, "bufferSerial acotado a 200 bytes");
  } else if (esc == "telemetria") {
    pin_level[CNY1] = LOW; enviar("LLUVIA=1\n");
    setup(); tick(); runFor(2.2);
    size_t p = serial_out.find("modo=");
    size_t e = serial_out.find("\r\n", p);
    std::string linea = p == std::string::npos ? "" : serial_out.substr(p, e - p);
    printf("  linea: %s\n", linea.c_str());
    for (const char* k : {"modo=", "fase=", "dur=", "co2=", "ldr1=", "cny1=1", "det=1", "det_remoto=", "p1=", "peaton1_espera=", "lluvia=1", "nocturno=0"}) {
      char msg[64]; snprintf(msg, sizeof msg, "telemetria contiene %s", k);
      CHECK(linea.find(k) != std::string::npos, msg);
    }
  } else if (esc == "lcd") {
    for (int p : {CNY1, CNY2, CNY3, CNY4, CNY5, CNY6}) pin_level[p] = LOW;
    analog_value[CO2] = 1000; enviar("LLUVIA=1\nDET_REMOTO=6\n");
    setup(); tick(); runFor(16);
    printf("  modo mas largo: '%s' (%u chars)\n", modoActualTexto().c_str(), modoActualTexto().length());
    printf("  lcd.clear() en 16 s: %d (cada %.0f ms)\n", lcd.clears, 16000.0 / lcd.clears);
    analog_value[LDR1] = LDR_OSCURO; analog_value[LDR2] = LDR_OSCURO; runFor(1);
    printf("  desbordes de linea (>20 col): %d\n", lcd.overflows);
    for (auto& s : lcd.overflow_samples) printf("    %s\n", s.c_str());
    CHECK(lcd.overflows == 0, "ningun texto se pasa de 20 columnas");
    CHECK(lcd.clears == 0, "el refresco no usa lcd.clear() (sin parpadeo en el LCD real)");
    anuncioActual = 0; mostrarAnuncio();
    printf("  pantalla 0: '%s' | '%s' | '%s' | '%s'\n", lcd.line(0).c_str(), lcd.line(1).c_str(), lcd.line(2).c_str(), lcd.line(3).c_str());
  } else {
    printf("escenario desconocido\n"); return 2;
  }
  printEventos();
  printf("  -> %d/%d OK, violaciones de luces: %ld\n", tests - fails, tests, violaciones);
  return fails ? 1 : 0;
}
