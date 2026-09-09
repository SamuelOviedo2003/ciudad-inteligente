// Ciudad Autoadaptable - Nivel bajo (CPS Generacion 0)
// Semaforos con tiempos fijos, siempre la misma secuencia (no reaccionan a los sensores).
// Todas las entradas/salidas de la maqueta se usan a traves de los anuncios en el LCD.
// Pines y calibracion de CO2 tomados de esp_pruebas.ino (assets/code/esp32/smart_city/esp_pruebas.ino)

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include "TimerMEF.h"

// --- Pines (segun esp_pruebas.ino) ---
#define LDR1 12 // LDR semaforo 1, pin A0
#define LDR2 13 // LDR semaforo 2, pin A1
#define CO2 14  // Sensor CO2, pin A3
#define P1 1    // Boton peatonal semaforo 1
#define P2 2    // Boton peatonal semaforo 2
#define CNY1 42 // Infrarrojo semaforo 1
#define CNY2 41
#define CNY3 40
#define CNY4 39 // Infrarrojo semaforo 2
#define CNY5 38
#define CNY6 37
#define LR1 5
#define LY1 4
#define LG1 6
#define LR2 7
#define LY2 15
#define LG2 16

// Nivel electrico de un boton peatonal presionado. En Wokwi los botones van a
// tierra y el pin usa el pull-up interno (reposo = HIGH, presionado = LOW). El
// codigo de pruebas del curso (esp_pruebas.ino) los lee como INPUT sin pull-up,
// asi que en la maqueta fisica pueden estar cableados al reves: verificar con
// ese sketch (muestra P1/P2 en el LCD) y ajustar solo esta constante.
#define P_ACTIVO LOW

// --- Calibracion CO2 (identica a esp_pruebas.ino) ---
const float DC_GAIN = 8.5;
const float ZERO_POINT_VOLTAGE = 0.265;
const float REACTION_VOLTAGE = 0.059;
const float CO2Curve[3] = {2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTAGE / (2.602 - 3))};

LiquidCrystal_I2C lcd(0x27, 20, 4);

// --- Tiempos fijos del semaforo (nivel bajo: nunca cambian) ---
const double T_VERDE = 5;    // segundos
const double T_AMARILLO = 2; // segundos

enum FaseSemaforo { FASE_A, FASE_B, FASE_C, FASE_D };
FaseSemaforo fase = FASE_A;
Timer tFase;

// --- Anuncios rotativos del LCD ---
const int NUM_ANUNCIOS = 4;
int anuncioActual = 0;
Timer tAnuncio;
const double T_ANUNCIO = 3; // segundos

// Refresco de los valores del anuncio actual (para que reaccione en vivo a los sensores/botones)
Timer tRefresco;
const double T_REFRESCO = 0.3; // segundos

// Telemetria por Serial (solo lectura, no controla nada del semaforo/LCD)
Timer tTelemetria;
const double T_TELEMETRIA = 1; // segundos

// Conteo combinado de vehiculos entre esta maqueta y la otra, via puente_serial.py
// (ver proyecto/puente_serial/). "remoto" es el ultimo valor recibido de la otra
// placa; si el puente no esta corriendo, se queda en 0 y el total es solo el local.
const int NUM_MAQUINAS = 2;
const int CNY_POR_MAQUINA = 6;
int detectadosRemoto = 0;
String bufferSerial = "";

void setup() {
  pinMode(P1, INPUT_PULLUP); // sin resistencia externa en el diagrama, se usa el pull-up interno
  pinMode(P2, INPUT_PULLUP); // presionado = LOW, suelto = HIGH
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

  Serial.setTxTimeoutMs(0); // no bloquear el loop si nadie esta leyendo el USB CDC
  Serial.begin(115200);
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
  leerSerialEntrante();
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
    case FASE_A: digitalWrite(LG1, HIGH); digitalWrite(LR2, HIGH); break; // S1 verde, S2 rojo
    case FASE_B: digitalWrite(LY1, HIGH); digitalWrite(LR2, HIGH); break; // S1 amarillo, S2 rojo
    case FASE_C: digitalWrite(LR1, HIGH); digitalWrite(LG2, HIGH); break; // S1 rojo, S2 verde
    case FASE_D: digitalWrite(LR1, HIGH); digitalWrite(LY2, HIGH); break; // S1 rojo, S2 amarillo
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

bool botonPresionado(int pin) { return digitalRead(pin) == P_ACTIVO; }

// CNY1..CNY6: en reposo (sin objeto) quedan en HIGH; LOW = objeto blanco detectado
int contarDetectadosLocal() {
  return (digitalRead(CNY1) == LOW) + (digitalRead(CNY2) == LOW) + (digitalRead(CNY3) == LOW) +
         (digitalRead(CNY4) == LOW) + (digitalRead(CNY5) == LOW) + (digitalRead(CNY6) == LOW);
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

// Vuelve a leer y redibujar el anuncio actual sin esperar a la siguiente rotacion,
// para que reaccione en vivo si se toca un sensor o boton mientras esta en pantalla.
void actualizarRefresco() {
  if (tRefresco > T_REFRESCO) {
    tRefresco = 0;
    mostrarAnuncio();
  }
}

void mostrarAnuncio() {
  lcd.clear();
  switch (anuncioActual) {
    case 0: { // CO2
      float ppm = leerCO2ppm();
      lcd.setCursor(0, 0); lcd.print("PONGASE TAPABOCAS");
      lcd.setCursor(0, 1); lcd.print("CO2: ");
      lcd.print((int)ppm);
      lcd.print(" ppm");
      break;
    }
    case 1: { // LDR1, LDR2
      int l1 = analogRead(LDR1);
      int l2 = analogRead(LDR2);
      lcd.setCursor(0, 0); lcd.print("LUZ AMBIENTE");
      lcd.setCursor(0, 1); lcd.print("Semaforo 1: "); lcd.print(l1);
      lcd.setCursor(0, 2); lcd.print("Semaforo 2: "); lcd.print(l2);
      break;
    }
    case 2: { // CNY1..CNY6 locales + los de la otra maqueta (recibidos por el puente serial)
      int total = contarDetectadosLocal() + detectadosRemoto;
      lcd.setCursor(0, 0); lcd.print("VEHICULOS EN VIA");
      lcd.setCursor(0, 1); lcd.print("Detectados: "); lcd.print(total);
      lcd.print("/"); lcd.print(NUM_MAQUINAS * CNY_POR_MAQUINA);
      break;
    }
    case 3: { // P1, P2 (con INPUT_PULLUP: presionado = LOW/0, suelto = HIGH/1)
      lcd.setCursor(0, 0); lcd.print("BOTON PEATONAL");
      lcd.setCursor(0, 1); lcd.print("P1:"); lcd.print(botonPresionado(P1) ? "SI" : "NO");
      lcd.setCursor(9, 1); lcd.print("P2:"); lcd.print(botonPresionado(P2) ? "SI" : "NO");
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

  Serial.print("{\"fase\":\"");
  Serial.print(nombreFase(fase));
  Serial.print("\",\"co2\":");
  Serial.print((int)co2);
  Serial.print(",\"ldr\":[");
  Serial.print(analogRead(LDR1));
  Serial.print(",");
  Serial.print(analogRead(LDR2));
  Serial.print("],\"cny\":[");
  Serial.print(digitalRead(CNY1)); Serial.print(",");
  Serial.print(digitalRead(CNY2)); Serial.print(",");
  Serial.print(digitalRead(CNY3)); Serial.print(",");
  Serial.print(digitalRead(CNY4)); Serial.print(",");
  Serial.print(digitalRead(CNY5)); Serial.print(",");
  Serial.print(digitalRead(CNY6));
  Serial.print("],\"det\":");
  Serial.print(contarDetectadosLocal());
  Serial.print(",\"p1\":");
  Serial.print(botonPresionado(P1) ? 1 : 0);
  Serial.print(",\"p2\":");
  Serial.print(botonPresionado(P2) ? 1 : 0);
  Serial.println("}");
}

// --- Recepcion de telemetria de la otra maqueta, reenviada por puente_serial.py ---
// Solo lee "det" (conteo local de la otra placa) para sumarlo al propio; ignora el resto.
void leerSerialEntrante() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      procesarLineaRemota(bufferSerial);
      bufferSerial = "";
    } else if (c != '\r') {
      bufferSerial += c;
      if (bufferSerial.length() > 200) bufferSerial = ""; // linea corrupta/sin terminar, descartar
    }
  }
}

void procesarLineaRemota(const String &linea) {
  if (!linea.startsWith("{")) return; // fragmento incompleto, ignorar
  int idx = linea.indexOf("\"det\":");
  if (idx == -1) return;
  detectadosRemoto = linea.substring(idx + 6).toInt();
}
