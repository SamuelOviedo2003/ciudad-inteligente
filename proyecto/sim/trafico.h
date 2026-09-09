// Modelo de trafico para entrenar y comparar los niveles en el arnes nativo.
// Cada via tiene una cola de vehiculos: llegan al azar (Poisson) y, mientras su
// semaforo esta en verde, salen uno cada T_SALIDA segundos (el primero tras
// T_REACCION). Los tres CNY de cada via reflejan min(cola, 3), asi que el
// firmware ve exactamente lo que veria en la maqueta: presencia, no conteo.
// Tambien genera peatones que pulsan P1/P2 al azar y mide cuanto esperan
// hasta que su semaforo se pone en rojo.
#pragma once
#include "Arduino.h"
#include <random>

struct Trafico {
  std::mt19937 rng;
  double tasa[2] = {0, 0};   // llegadas por segundo por via
  double tasaPeaton = 0;     // pulsaciones por segundo por boton
  int cola[2] = {0, 0};
  double proximaSalida[2] = {0, 0};
  static constexpr int MAX_COLA = 6;  // los CNY solo ven 3; mas alla de eso el firmware no distingue
  static constexpr double T_SALIDA = 1.5, T_REACCION = 0.8, T_PULSACION = 0.3;
  // metricas
  double segundos = 0, colaSegundos = 0, esperaPeatonTotal = 0;
  long llegaron = 0, salieron = 0, descartados = 0, peatones = 0;
  bool peatonEsperando[2] = {false, false};
  double peatonDesde[2] = {0, 0}, soltarEn[2] = {0, 0};

  void reiniciar(unsigned semilla, double tasa1, double tasa2, double tasaP) {
    rng.seed(semilla);
    tasa[0] = tasa1; tasa[1] = tasa2; tasaPeaton = tasaP;
    cola[0] = cola[1] = 0;
    segundos = colaSegundos = esperaPeatonTotal = 0;
    llegaron = salieron = descartados = peatones = 0;
    peatonEsperando[0] = peatonEsperando[1] = false;
    aplicarPines();
  }

  bool azar(double prob) { return std::uniform_real_distribution<double>(0, 1)(rng) < prob; }

  // t en segundos de simulacion, dt el paso
  void paso(double t, double dt) {
    static const int LG[2] = {6, 16}, LR[2] = {5, 7}, P[2] = {1, 2};
    for (int v = 0; v < 2; v++) {
      if (azar(tasa[v] * dt)) {
        if (cola[v] < MAX_COLA) { cola[v]++; llegaron++; } else descartados++;
      }
      bool verde = sim::pin_level[LG[v]] == HIGH;
      if (!verde) {
        proximaSalida[v] = t + T_REACCION;
      } else if (cola[v] > 0 && t >= proximaSalida[v]) {
        cola[v]--; salieron++;
        proximaSalida[v] = t + T_SALIDA;
      }
      // peatones
      if (!peatonEsperando[v] && azar(tasaPeaton * dt)) {
        peatonEsperando[v] = true; peatonDesde[v] = t; soltarEn[v] = t + T_PULSACION;
        sim::pin_level[P[v]] = LOW;
      }
      if (sim::pin_level[P[v]] == LOW && t >= soltarEn[v]) sim::pin_level[P[v]] = HIGH;
      if (peatonEsperando[v] && t > soltarEn[v] && sim::pin_level[LR[v]] == HIGH) {
        esperaPeatonTotal += t - peatonDesde[v]; peatones++; peatonEsperando[v] = false;
      }
    }
    colaSegundos += (cola[0] + cola[1]) * dt;
    segundos += dt;
    aplicarPines();
  }

  void aplicarPines() {
    static const int CNY[2][3] = {{42, 41, 40}, {39, 38, 37}};
    for (int v = 0; v < 2; v++)
      for (int k = 0; k < 3; k++) sim::pin_level[CNY[v][k]] = (cola[v] > k) ? LOW : HIGH;
  }

  double colaMedia() const { return segundos > 0 ? colaSegundos / segundos : 0; }
  double esperaMediaVehiculo() const { return salieron > 0 ? colaSegundos / salieron : 0; }
  double esperaMediaPeaton() const { return peatones > 0 ? esperaPeatonTotal / peatones : 0; }
};

// Patrones de trafico (vehiculos/s por via) usados para entrenar y comparar.
struct Patron { const char* nombre; double tasa1, tasa2; };
static const Patron PATRONES[] = {
  {"poco trafico", 0.04, 0.04},
  {"hora pico via 1", 0.30, 0.05},
  {"hora pico via 2", 0.05, 0.30},
  {"ambas cargadas", 0.22, 0.22},
  {"desbalance suave", 0.15, 0.08},
};
static const int NUM_PATRONES = 5;
static const double TASA_PEATON = 1.0 / 120;  // una pulsacion cada 2 min por boton
