// Prueba de puente serial - Ciudad Autoadaptable
// Copia de nivel_bajo.ino que ADEMAS emite telemetria por Serial en JSON,
// una linea por segundo, para validar proyecto/puente_serial/puente_serial.py
// entre dos maquetas. No lee nada de Serial, no cambia el comportamiento del
// semaforo: sigue siendo Generacion 0 (nivel bajo), solo se le agrego una salida.

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include "TimerMEF.h"

// --- Pines (segun esp_pruebas.ino) ---
#define LDR1 12
#define LDR2 13
#define CO2 14
#define P1 1
#define P2 2
#define CNY1 42
#define CNY2 41
#define CNY3 40
#define CNY4 39
#define CNY5 38
#define CNY6 37
#define LR1 5
#define LY1 4
#define LG1 6
#define LR2 7
#define LY2 15
#define LG2 16

// --- Calibracion CO2 (identica a esp_pruebas.ino) ---
const float DC_GAIN = 8.5;
const float ZERO_POINT_VOLTAGE = 0.265;
const float REACTION_VOLTAGE = 0.059;
const float CO2Curve[3] = {2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTAGE / (2.602 - 3))};

LiquidCrystal_I2C lcd(0x27, 20, 4);

// --- Tiempos fijos del semaforo (nivel bajo: nunca cambian) ---
const double T_VERDE = 5;
const double T_AMARILLO = 2;

enum FaseSemaforo { FASE_A, FASE_B, FASE_C, FASE_D };
FaseSemaforo fase = FASE_A;
Timer tFase;

// --- Anuncios rotativos del LCD ---
const int NUM_ANUNCIOS = 4;
int anuncioActual = 0;
Timer tAnuncio;
const double T_ANUNCIO = 3;

Timer tRefresco;
const double T_REFRESCO = 0.3;

// --- Telemetria por Serial (lo unico nuevo frente a nivel_bajo.ino) ---
Timer tTelemetria;
const double T_TELEMETRIA = 1; // segundos

void setup() {
  pinMode(P1, INPUT_PULLUP);
  pinMode(P2, INPUT_PULLUP);
  pinMode(CNY1, INPUT);
  pinMode(CNY2, INPUT);
  pinMode(CNY3, INPUT);
  pinMode(CNY4, INPUT);
  pinMode(CNY5, INPUT);
  pinMode(CNY6, INPUT);

  pinMode(LR1, OUTPUT);
  pinMode(LY1, OUTPUT);
  pinMode(LG1, OUTPUT);
  pinMode(LR2, OUTPUT);
  pinMode(LY2, OUTPUT);
  pinMode(LG2, OUTPUT);

  apagarSemaforos();

  Serial.setTxTimeoutMs(0); // no bloquear el loop si nadie esta leyendo el USB CDC (puente apagado)
  Serial.begin(115200); // debe coincidir con --baud de puente_serial.py
  lcd.init();
  lcd.backlight();

  tFase = 0;
  tAnuncio = 0;
  tRefresco = 0;
  tTelemetria = 0;
  aplicarFase();
  mostrarAnuncio();
}

void loop() {
  actualizarSemaforo();
  actualizarAnuncio();
  actualizarRefresco();
  actualizarTelemetria();
}

// --- Semaforo: MEF de 4 fases, tiempos fijos, no depende de sensores ---
void apagarSemaforos() {
  digitalWrite(LR1, LOW);
  digitalWrite(LY1, LOW);
  digitalWrite(LG1, LOW);
  digitalWrite(LR2, LOW);
  digitalWrite(LY2, LOW);
  digitalWrite(LG2, LOW);
}

void aplicarFase() {
  apagarSemaforos();
  switch (fase) {
    case FASE_A: digitalWrite(LG1, HIGH); digitalWrite(LR2, HIGH); break;
    case FASE_B: digitalWrite(LY1, HIGH); digitalWrite(LR2, HIGH); break;
    case FASE_C: digitalWrite(LR1, HIGH); digitalWrite(LG2, HIGH); break;
    case FASE_D: digitalWrite(LR1, HIGH); digitalWrite(LY2, HIGH); break;
  }
}

void actualizarSemaforo() {
  double duracion = (fase == FASE_A || fase == FASE_C) ? T_VERDE : T_AMARILLO;
  if (tFase > duracion) {
    fase = (FaseSemaforo)((fase + 1) % 4);
    tFase = 0;
    aplicarFase();
  }
}

// --- CO2 en ppm, misma formula que esp_pruebas.ino ---
float leerCO2ppm() {
  float volts = analogRead(CO2) * 3.3 / 4096.0;
  if (volts / DC_GAIN >= ZERO_POINT_VOLTAGE) return -1;
  return pow(10, ((volts / DC_GAIN) - CO2Curve[1]) / CO2Curve[2] + CO2Curve[0]);
}

// --- Anuncios: cada uno usa un grupo distinto de sensores de la maqueta ---
void actualizarAnuncio() {
  if (tAnuncio > T_ANUNCIO) {
    anuncioActual = (anuncioActual + 1) % NUM_ANUNCIOS;
    tAnuncio = 0;
    tRefresco = 0;
    mostrarAnuncio();
  }
}

void actualizarRefresco() {
  if (tRefresco > T_REFRESCO) {
    tRefresco = 0;
    mostrarAnuncio();
  }
}

void mostrarAnuncio() {
  lcd.clear();
  switch (anuncioActual) {
    case 0: {
      float ppm = leerCO2ppm();
      lcd.setCursor(0, 0); lcd.print("PONGASE TAPABOCAS");
      lcd.setCursor(0, 1); lcd.print("CO2: ");
      lcd.print((int)ppm);
      lcd.print(" ppm");
      break;
    }
    case 1: {
      int l1 = analogRead(LDR1);
      int l2 = analogRead(LDR2);
      lcd.setCursor(0, 0); lcd.print("LUZ AMBIENTE");
      lcd.setCursor(0, 1); lcd.print("Semaforo 1: "); lcd.print(l1);
      lcd.setCursor(0, 2); lcd.print("Semaforo 2: "); lcd.print(l2);
      break;
    }
    case 2: {
      // LOW = objeto detectado (los CNY van a tierra con pull-up), igual que en nivel_bajo.ino
      int detectados = (digitalRead(CNY1) == LOW) + (digitalRead(CNY2) == LOW) + (digitalRead(CNY3) == LOW) +
                       (digitalRead(CNY4) == LOW) + (digitalRead(CNY5) == LOW) + (digitalRead(CNY6) == LOW);
      lcd.setCursor(0, 0); lcd.print("VEHICULOS EN VIA");
      lcd.setCursor(0, 1); lcd.print("Detectados: "); lcd.print(detectados); lcd.print("/6");
      break;
    }
    case 3: {
      lcd.setCursor(0, 0); lcd.print("BOTON PEATONAL");
      lcd.setCursor(0, 1); lcd.print("P1:"); lcd.print(digitalRead(P1) == LOW ? "SI" : "NO");
      lcd.setCursor(9, 1); lcd.print("P2:"); lcd.print(digitalRead(P2) == LOW ? "SI" : "NO");
      break;
    }
  }
}

// --- Telemetria: una linea JSON por segundo, solo lectura, no controla nada ---
const char* nombreFase(FaseSemaforo f) {
  switch (f) {
    case FASE_A: return "A";
    case FASE_B: return "B";
    case FASE_C: return "C";
    case FASE_D: return "D";
  }
  return "?";
}

void actualizarTelemetria() {
  if (tTelemetria > T_TELEMETRIA) {
    tTelemetria = 0;
    enviarTelemetria();
  }
}

void enviarTelemetria() {
  float co2 = leerCO2ppm();
  int cny[6] = {
    digitalRead(CNY1), digitalRead(CNY2), digitalRead(CNY3),
    digitalRead(CNY4), digitalRead(CNY5), digitalRead(CNY6)
  };

  Serial.print("{\"fase\":\"");
  Serial.print(nombreFase(fase));
  Serial.print("\",\"co2\":");
  Serial.print((int)co2);
  Serial.print(",\"ldr\":[");
  Serial.print(analogRead(LDR1));
  Serial.print(",");
  Serial.print(analogRead(LDR2));
  Serial.print("],\"cny\":[");
  for (int i = 0; i < 6; i++) {
    Serial.print(cny[i]);
    if (i < 5) Serial.print(",");
  }
  Serial.print("],\"p1\":");
  Serial.print(digitalRead(P1) == LOW ? 1 : 0);
  Serial.print(",\"p2\":");
  Serial.print(digitalRead(P2) == LOW ? 1 : 0);
  Serial.println("}");
}
