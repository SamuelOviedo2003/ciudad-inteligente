// Entrena la tabla Q de nivel_alto.ino contra el modelo de trafico, con el
// MISMO codigo del agente que corre en el ESP32 (el .ino se incluye tal cual),
// y exporta el resultado a nivel_alto/tabla_q.h.
// Uso: entrenar <ruta de salida tabla_q.h> [episodios por patron]
#include "Arduino.h"
#include "LiquidCrystal_I2C.h"
#include "prototypes_alto.h"
#include "nivel_alto.ino"
#include "trafico.h"
#include <cstdio>
#include <ctime>

using namespace sim;
static const double DT = 0.005;  // 5 ms por vuelta del loop

static long visitas[2][32][3];
static int decisionesVistas[2] = {0, 0};

static void correr(Trafico &tr, double segundos, std::mt19937 &rngExplora) {
  unsigned long fin = now_ms + (unsigned long)(segundos * 1000);
  while (now_ms < fin) {
    random_value = rngExplora() % 1000;  // aleatoriedad real para la exploracion del agente
    tr.paso(now_ms / 1000.0, DT);
    loop();
    for (int v = 0; v < 2; v++) {
      if (agente[v].decisiones != decisionesVistas[v]) { decisionesVistas[v] = agente[v].decisiones; visitas[v][agente[v].estado][agente[v].accion]++; }
    }
    now_ms += (unsigned long)(DT * 1000);
    serial_out.clear();
  }
}

int main(int argc, char **argv) {
  const char *salida = argc > 1 ? argv[1] : "tabla_q.h";
  int episodiosPorPatron = argc > 2 ? atoi(argv[2]) : 12;
  const double DURACION_EPISODIO = 1200;  // 20 min de trafico simulado

  analog_value[LDR1] = 3282; analog_value[LDR2] = 3282; analog_value[CO2] = 3282;
  pin_level[P1] = P_REPOSO; pin_level[P2] = P_REPOSO;
  setup();
  std::mt19937 rngExplora(7);
  Trafico tr;

  // Curriculum: rota los patrones; la exploracion baja de 0.3 a 0.05 y en la
  // mitad de los episodios el CO2 esta alto para que tambien aprenda esos estados.
  int total = episodiosPorPatron * NUM_PATRONES;
  for (int e = 0; e < total; e++) {
    const Patron &p = PATRONES[e % NUM_PATRONES];
    epsilon = 0.3 - 0.25 * e / (double)total;
    alpha = 0.2 - 0.19 * e / (double)total;  // de 0.2 a 0.01: promedia cada vez mas y deja de oscilar
    analog_value[CO2] = (e % 2) ? 1000 : 3282;
    tr.reiniciar(1000 + e, p.tasa1, p.tasa2, TASA_PEATON);
    correr(tr, DURACION_EPISODIO, rngExplora);
    if (e % NUM_PATRONES == NUM_PATRONES - 1 && (e / NUM_PATRONES + 1) % (episodiosPorPatron / 10 > 0 ? episodiosPorPatron / 10 : 1) == 0)
      fprintf(stderr, "  ronda %2d/%d  eps=%.2f  ultimo patron '%s': cola media %.2f, espera vehiculo %.1f s, R total v1 %.0f v2 %.0f\n",
              e / NUM_PATRONES + 1, episodiosPorPatron, epsilon, p.nombre, tr.colaMedia(), tr.esperaMediaVehiculo(),
              agente[0].recompensaTotal, agente[1].recompensaTotal);
  }

  // Las dos vias son simetricas (mismo ciclo visto desde cada lado), asi que la
  // tabla exportada es el promedio de los dos agentes: el doble de experiencia
  // por estado y la misma politica inicial para ambos. En vivo cada via sigue
  // afinando la suya por separado.
  for (int s = 0; s < NUM_ESTADOS; s++)
    for (int a = 0; a < NUM_ACCIONES; a++) Q[0][s][a] = Q[1][s][a] = 0.5f * (Q[0][s][a] + Q[1][s][a]);

  // Politica aprendida (sin eco) como grilla cola propia x cola ajena
  printf("Politica aprendida (verde en s; filas = cola propia 0..3, columnas = cola de la otra via 0..3)\n");
  for (int v = 0; v < 1; v++) {  // (promediada: es la misma para las dos vias)
    for (int eco = 0; eco < 2; eco++) {
      printf("  %s:\n", eco ? "con CO2 alto" : "sin CO2 alto");
      for (int propia = 0; propia < 4; propia++) {
        printf("    propia %d |", propia);
        for (int otra = 0; otra < 4; otra++) printf(" %d", (int)ACCION_VERDE[mejorAccion(v, propia + 4 * otra + 16 * eco)]);
        printf("\n");
      }
    }
  }
  // Cobertura: cuantas veces se visito cada par (estado, accion) sin CO2 alto
  long minV = 1L << 30, totalV = 0; int pocoVisitados = 0;
  for (int v = 0; v < 2; v++) for (int s = 0; s < 16; s++) for (int a = 0; a < 3; a++) { long n = visitas[v][s][a]; totalV += n; if (n < minV) minV = n; if (n < 20) pocoVisitados++; }
  printf("Cobertura (sin eco): %ld decisiones, minimo %ld visitas en un par (estado, accion), %d de 96 pares con menos de 20 visitas\n", totalV, minV, pocoVisitados);
  printf("Valores Q (sin eco) en estados clave [3 s, 5 s, 8 s]:\n");
  for (int s : {0, 3, 12, 15, 1, 4}) printf("  propia %d ajena %d: %.2f %.2f %.2f\n", s % 4, s / 4, Q[0][s][0], Q[0][s][1], Q[0][s][2]);
  int fallas = 0;
  auto verificar = [&](bool cond, const char *msg) { printf("  %s %s\n", cond ? "PASS" : "FAIL", msg); if (!cond) fallas++; };
  for (int v = 0; v < 1; v++) {
    verificar(ACCION_VERDE[mejorAccion(v, 3 + 4 * 0)] == 8, "cola propia 3, ajena 0 -> verde largo (8 s)");
    verificar(ACCION_VERDE[mejorAccion(v, 0 + 4 * 3)] == 3, "cola propia 0, ajena 3 -> verde corto (3 s)");
    verificar(ACCION_VERDE[mejorAccion(v, 0 + 4 * 0)] == 3, "vias vacias -> verde corto (3 s)");
    verificar(ACCION_VERDE[mejorAccion(v, 3 + 4 * 3)] >= 5, "ambas llenas -> no elige el verde corto");
  }

  FILE *f = fopen(salida, "w");
  if (!f) { perror(salida); return 1; }
  time_t ahora = time(nullptr); char fecha[32]; strftime(fecha, sizeof fecha, "%Y-%m-%d", localtime(&ahora));
  fprintf(f, "// Tabla Q entrenada offline por proyecto/sim/entrenar.cpp el %s.\n", fecha);
  fprintf(f, "// Mismo codigo del agente de nivel_alto.ino corriendo en el PC contra el modelo de\n");
  fprintf(f, "// trafico de proyecto/sim/trafico.h: %d episodios de %.0f min (%d patrones),\n",
          total, DURACION_EPISODIO / 60, NUM_PATRONES);
  fprintf(f, "// gamma %.2f, epsilon de 0.30 a 0.05, alpha de 0.20 a 0.01. Regenerar con proyecto/sim/entrenar.sh.\n", GAMMA);
  fprintf(f, "// Indice: [via][estado][accion], estado = colaPropia + 4*colaOtra + 16*eco, acciones = 3/5/8 s.\n");
  fprintf(f, "#pragma once\n#define TABLA_Q_ENTRENADA\nconst float TABLA_Q[2][%d][%d] = {\n", NUM_ESTADOS, NUM_ACCIONES);
  for (int v = 0; v < 2; v++) {
    fprintf(f, "  {\n");
    for (int s = 0; s < NUM_ESTADOS; s++) {
      fprintf(f, "    {%.4f, %.4f, %.4f},%s\n", Q[v][s][0], Q[v][s][1], Q[v][s][2],
              s % 16 == 0 ? (s == 0 ? " // cola propia 0..3, ajena 0..3" : " // igual, con CO2 alto") : "");
    }
    fprintf(f, "  },\n");
  }
  fprintf(f, "};\n");
  fclose(f);
  printf("Tabla escrita en %s (%d decisiones de entrenamiento)\n", salida, agente[0].decisiones + agente[1].decisiones);
  return fallas ? 1 : 0;
}
