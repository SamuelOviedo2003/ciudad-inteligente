// Ciudad Autoadaptable - Nivel alto (CPS Generacion 2)
// Misma maqueta y mismas reglas de seguridad que nivel_medio (amarillo fijo,
// verde minimo, prioridad peatonal, modo nocturno, lluvia, coordinacion con la
// otra maqueta por el puente serial), pero la duracion del verde de cada via
// ya no sale de una regla escrita a mano: la elige un agente de aprendizaje
// por refuerzo (Q-learning tabular, uno por via) que observa el trafico,
// mide el resultado de cada decision (recompensa) y corrige su tabla. El
// sistema aprende de su propia experiencia y muestra en el LCD lo que sabe y
// por que decide (autoconciencia visible), que es lo que define la
// Generacion 2 en docs/concepto/cps-nivel-bajo.md.
// Pines y calibracion de CO2 identicos a nivel_bajo/nivel_bajo.ino

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include <Preferences.h>
#include "TimerMEF.h"
#ifdef ARDUINO_ARCH_ESP32
#include "soc/rtc_cntl_reg.h" // comando BOOTLOADER (reinicio en modo de carga)
#endif
#ifndef SIN_TABLA_ENTRENADA
#include "tabla_q.h"  // tabla entrenada offline por proyecto/sim/entrenar.sh
#endif

// --- Pines (idénticos a nivel_bajo, misma maqueta) ---
#define LDR1 12 // LDR semaforo 1, pin A0
#define LDR2 13 // LDR semaforo 2, pin A1
#define CO2 14  // Sensor CO2, pin A3
#define P1 1    // Boton peatonal semaforo 1
#define P2 2    // Boton peatonal semaforo 2
#define CNY1 42 // Infrarrojo via 1
#define CNY2 41
#define CNY3 40
#define CNY4 39 // Infrarrojo via 2
#define CNY5 38
#define CNY6 37
#define LR1 5
#define LY1 4
#define LG1 6
#define LR2 7
#define LY2 15
#define LG2 16

// Nivel electrico de un CNY que SI detecta vehiculo: en Wokwi y en la maqueta
// fisica van a tierra con pull-up (reposo HIGH, activo LOW; verificado en las
// placas el 2026-09-09).
#define CNY_ACTIVO LOW
// Nivel electrico de un boton peatonal presionado. En la maqueta fisica
// (medido el 2026-09-09 en las dos placas) los botones tienen pull-down externo:
// reposo = LOW, presionado = HIGH. En Wokwi (diagram.json) van a tierra con el
// pull-up interno, o sea al reves: para simular ahi, poner LOW. P_MODO acompana
// a la constante (sin pull-up interno cuando hay pull-down externo).
#define P_ACTIVO HIGH
#define P_MODO (P_ACTIVO == LOW ? INPUT_PULLUP : INPUT)

// --- Calibracion CO2 (identica a nivel_bajo) ---
const float DC_GAIN = 8.5;
const float ZERO_POINT_VOLTAGE = 0.265;
const float REACTION_VOLTAGE = 0.059;
const float CO2Curve[3] = {2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTAGE / (2.602 - 3))};

LiquidCrystal_I2C lcd(0x27, 20, 4);

// --- Factor de aceleracion para la demo: divide todos los tiempos del
// semaforo (verde, amarillo, esperas) sin tocar la logica. Con 1 los tiempos
// son los reales; con 2 un ciclo completo dura la mitad y se alcanzan a ver
// varias decisiones del agente en pocos minutos frente a la clase. ---
const double ACELERACION = 1;
double seg(double s) { return s / ACELERACION; }

// --- Reglas fijas (self-regulation): esto el agente NO lo decide ---
const double T_AMARILLO_BASE = 2;   // segundos
const double BONUS_LLUVIA = 1;      // seg. extra de amarillo si el puente reporta lluvia real
const double BONUS_RED = 2;         // seg. extra de verde si la OTRA maqueta (via internet) esta muy congestionada
const int UMBRAL_CONGESTION_RED = 4; // conteo remoto (0-6) para considerar congestionada a la otra maqueta
const int UMBRAL_CO2_ECO = 800;     // ppm: por encima, el agente ve el estado "eco"
// Valores medidos en las maquetas fisicas (2026-09-09, luz de habitacion):
// placa A ~250/330, placa B ~1550/300; tapadas con la mano bajan a 13-200. Con
// el umbral anterior (800) la placa A quedaba en nocturno de dia. En Wokwi el
// potenciometro de LDR debe bajar de ~4 % (value < 37) para simular la noche.
const int UMBRAL_NOCHE_ENTRA = 150;  // lectura LDR (0-4095)
const int UMBRAL_NOCHE_SALE = 250;
const double VERDE_MINIMO = 2;       // seg.: un peaton nunca corta un verde antes de esto
const double MAX_ESPERA_PEATON = 6;  // seg.: con trafico, el peaton espera como mucho esto

// --- Agente de aprendizaje por refuerzo (Q-learning tabular), uno por via ---
// Estado (32 por via): cola propia (0-3 CNY) x cola de la otra via (0-3) x CO2 alto (0/1).
// Accion (3): duracion del proximo verde de esa via.
// Un paso del agente va desde que empieza su verde hasta que vuelve a empezar
// (un ciclo completo). Recompensa del paso: menos la espera visible acumulada
// (vehiculos detectados en las dos vias, integrados en el tiempo, que es lo
// que un semaforo quiere minimizar), mas un premio por cada vehiculo que salio
// durante su verde (transicion detectado -> libre en sus CNY), menos un
// castigo por segundos de verde con la via vacia (para que no regale verde).
// Todo se normaliza a T_REF segundos: los pasos duran distinto segun la accion
// (un verde de 3 s hace ciclos mas cortos que uno de 8 s) y sin normalizar el
// agente aprenderia que los ciclos cortos "cuestan menos" solo por ser cortos.
const int NUM_ACCIONES = 3;
const double ACCION_VERDE[NUM_ACCIONES] = {3, 5, 8};
const int NUM_ESTADOS = 32;
float Q[2][NUM_ESTADOS][NUM_ACCIONES];
const float ALPHA = 0.1;    // tasa de aprendizaje en vivo
float alpha = ALPHA;        // el entrenamiento offline la va bajando para que la tabla converja
const float GAMMA = 0.8;    // cuanto pesa el futuro
const float EPSILON = 0.1;  // probabilidad de explorar una accion distinta a la mejor (valor inicial)
float epsilon = EPSILON;    // ajustable en vivo con "EPSILON=<0..1>" por serial, util en la demo
const float PESO_ESPERA = 0.1;       // por cada vehiculo-segundo visible esperando (ambas vias)
const float PESO_PASARON = 0.5;      // por cada vehiculo que salio durante el verde propio
const float PESO_VERDE_VACIO = 0.1;  // por cada segundo de verde propio con la via vacia
const double T_REF = 10;             // segundos a los que se normaliza la recompensa de cada paso

struct Agente {
  int estado = 0;             // estado observado al empezar el verde actual
  int accion = 1;             // accion elegida para el verde actual
  bool exploro = false;       // si la ultima decision fue exploracion
  bool pendiente = false;     // hay una transicion (estado, accion, r) sin cerrar con s'
  float recompensa = 0;       // recompensa del ultimo paso cerrado
  float recompensaTotal = 0;  // acumulada desde el arranque
  int decisiones = 0;
  int pasaron = 0;            // vehiculos que salieron durante el verde del paso actual
  double verdeVacio = 0;      // segundos de verde del paso actual con la via vacia
  double espera = 0;          // vehiculo-segundos visibles (ambas vias) en el paso actual
  int pasaronPaso = 0;        // los mismos tres, del ultimo paso cerrado (telemetria)
  double verdeVacioPaso = 0, esperaPaso = 0, duracionPaso = 0;
  unsigned long inicioPasoMs = 0;
  bool cnyPrev[3] = {false, false, false};
  unsigned long ultimoMuestreoMs = 0;
};
Agente agente[2];

// La tabla vive tambien en la memoria no volatil (NVS) del ESP32: lo aprendido
// sobrevive a un reinicio o a un corte de energia. Se guarda cada
// GUARDAR_CADA decisiones (no en cada una, para no desgastar la flash) y con
// el comando serial "Q_SAVE"; "Q_RESET" la borra y vuelve a la heuristica.
Preferences memoria;
bool tablaDesdeFlash = false;
int decisionesSinGuardar = 0;
const int GUARDAR_CADA = 10;

enum FaseSemaforo { FASE_A, FASE_B, FASE_C, FASE_D };
FaseSemaforo fase = FASE_A;
Timer tFase;
double duracionFaseActual = 5;

// --- Modo nocturno y prioridad peatonal, iguales a nivel_medio ---
bool modoNocturno = false;
bool nocturnoSuspendido = false;
Timer tSuspenderNocturno;
const double SUSPENSION_NOCTURNO = 20;
bool lluvia = false; // llega por Serial desde puente_serial.py (clima real de internet)

int detectadosRemoto = 0; // ultimo conteo (0-6) recibido de la otra maqueta
unsigned long ultimoRemotoMs = 0;
const unsigned long CADUCIDAD_REMOTO_MS = 300000;

bool peaton1Pedido = false;
bool peaton2Pedido = false;
bool finDeFaseForzado = false;
Timer tEspera1;
Timer tEspera2;

Timer tBlink1;
Timer tBlink2;
bool blink1 = false;
bool blink2 = false;
const double T_BLINK = 0.5;

// --- Anuncios rotativos del LCD ---
const int NUM_ANUNCIOS = 6;
int anuncioActual = 0;
Timer tAnuncio;
const double T_ANUNCIO = 3;
Timer tRefresco;
const double T_REFRESCO = 0.3;

// --- Enlace con el computador (puente_serial.py) ---
Timer tTelemetria;
const double T_TELEMETRIA = 1;
unsigned long ultimoPingMs = 0;
bool puenteVisto = false;
const unsigned long TIMEOUT_PC_MS = 5000;
String bufferSerial = "";

void setup() {
  pinMode(P1, P_MODO);
  pinMode(P2, P_MODO);
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

  // Placa fisica: compilar con CDCOnBoot=cdc. Wokwi: compilar SIN esa opcion
  // (su monitor serial esta en el UART0). Ver README raiz.
#if ARDUINO_USB_CDC_ON_BOOT
  Serial.setTxTimeoutMs(0);
#endif
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  inicializarQ();
  memoria.begin("ciudad", false);
  cargarQ();

  tFase = 0;
  tAnuncio = 0;
  tRefresco = 0;
  tTelemetria = 0;
  tSuspenderNocturno = 0;
  aplicarFase();
  mostrarAnuncio();
}

void loop() {
  leerComandosSerial();
  caducarConteoRemoto();
  registrarPeticionesPeatonales();
  actualizarModoNocturno();
  if (modoNocturno) {
    actualizarParpadeoNocturno();
  } else {
    observarTrafico();
    actualizarSemaforo();
  }
  actualizarAnuncio();
  actualizarRefresco();
  actualizarTelemetria();
}

// =====================================================================
// Agente
// =====================================================================

// Tabla inicial. Si existe tabla_q.h (entrenada offline con este mismo codigo
// contra el modelo de trafico del arnes) se parte de ella: el ESP32 arranca ya
// sabiendo y sigue afinando en vivo. Si no, una heuristica suave (verde mas
// largo cuanto mas cola propia, mas corto cuanto mas cola ajena, algo mas largo
// con CO2 alto) para que el sistema arranque razonable y aprenda desde ahi.
void inicializarQ() {
#ifdef TABLA_Q_ENTRENADA
  memcpy(Q, TABLA_Q, sizeof(Q));
  return;
#endif
  for (int v = 0; v < 2; v++) {
    for (int s = 0; s < NUM_ESTADOS; s++) {
      int colaPropia = s % 4, colaOtra = (s / 4) % 4, eco = s / 16;
      double deseado = 3 + 1.5 * colaPropia - 0.5 * colaOtra + (eco ? 2 : 0);
      for (int a = 0; a < NUM_ACCIONES; a++) {
        Q[v][s][a] = -0.2 * fabs(ACCION_VERDE[a] - deseado);
      }
    }
  }
}

void cargarQ() {
  if (memoria.getBytesLength("q") != sizeof(Q)) return; // no hay tabla guardada (o es de otra version)
  memoria.getBytes("q", Q, sizeof(Q));
  tablaDesdeFlash = true;
}

// Devuelve los bytes escritos (0 si la flash no acepto la escritura).
size_t guardarQ() {
  decisionesSinGuardar = 0;
  return memoria.putBytes("q", Q, sizeof(Q));
}

void borrarQ() {
  memoria.clear();
  inicializarQ();
  tablaDesdeFlash = false;
  decisionesSinGuardar = 0;
  for (int v = 0; v < 2; v++) {
    agente[v].pendiente = false;
    agente[v].recompensaTotal = 0;
    agente[v].decisiones = 0;
  }
}

// Vuelca la tabla completa por serial: una linea por (via, estado) con los
// tres valores Q. Sirve para ver que aprendio y para exportar tabla_q.h.
void volcarQ() {
  for (int v = 0; v < 2; v++) {
    for (int s = 0; s < NUM_ESTADOS; s++) {
      Serial.print("Q "); Serial.print(v + 1); Serial.print(" "); Serial.print(s);
      for (int a = 0; a < NUM_ACCIONES; a++) { Serial.print(" "); Serial.print(Q[v][s][a], 4); }
      Serial.println();
    }
  }
  Serial.println("Q fin");
}

int colaDeVia(int via) { return via == 0 ? contarVehiculos1() : contarVehiculos2(); }

int observarEstado(int via) {
  int colaPropia = colaDeVia(via);
  int colaOtra = colaDeVia(1 - via);
  int eco = (leerCO2ppm() > UMBRAL_CO2_ECO) ? 1 : 0;
  return colaPropia + 4 * colaOtra + 16 * eco;
}

int mejorAccion(int via, int estado) {
  int mejor = 0;
  for (int a = 1; a < NUM_ACCIONES; a++) {
    if (Q[via][estado][a] > Q[via][estado][mejor]) mejor = a;
  }
  return mejor;
}

// Se llama al empezar el verde de una via. Primero cierra el paso anterior de
// esa via (ya se conoce s' y toda la espera del ciclo), luego elige la accion.
double decidirVerde(int via) {
  Agente &ag = agente[via];
  int estado = observarEstado(via);
  if (ag.pendiente) {
    double dur = (millis() - ag.inicioPasoMs) / 1000.0;
    if (dur < 0.001) dur = 0.001;
    ag.recompensa = (PESO_PASARON * ag.pasaron - PESO_ESPERA * ag.espera - PESO_VERDE_VACIO * ag.verdeVacio) * (T_REF / dur);
    ag.recompensaTotal += ag.recompensa;
    ag.pasaronPaso = ag.pasaron; ag.verdeVacioPaso = ag.verdeVacio; ag.esperaPaso = ag.espera; ag.duracionPaso = dur;
    float maxSiguiente = Q[via][estado][mejorAccion(via, estado)];
    float &q = Q[via][ag.estado][ag.accion];
    q += alpha * (ag.recompensa + GAMMA * maxSiguiente - q);
    ag.pendiente = false;
  }
  ag.estado = estado;
  ag.exploro = (random(1000) < (long)(epsilon * 1000));
  ag.accion = ag.exploro ? (int)random(NUM_ACCIONES) : mejorAccion(via, estado);
  ag.decisiones++;
  if (++decisionesSinGuardar >= GUARDAR_CADA) guardarQ();
  ag.pasaron = 0;
  ag.verdeVacio = 0;
  ag.espera = 0;
  ag.ultimoMuestreoMs = millis();
  ag.inicioPasoMs = millis();
  for (int i = 0; i < 3; i++) ag.cnyPrev[i] = cnyDeVia(via, i);
  return ACCION_VERDE[ag.accion];
}

bool cnyDeVia(int via, int i) {
  static const int pines[2][3] = {{CNY1, CNY2, CNY3}, {CNY4, CNY5, CNY6}};
  return vehiculoDetectado(pines[via][i]);
}

// En cada vuelta del loop: los dos agentes acumulan la espera visible de
// ambas vias; el de la via en verde ademas cuenta los vehiculos que salen
// (detectado -> libre) y el tiempo de verde con su via vacia.
void observarTrafico() {
  unsigned long ahora = millis();
  int totalVisible = colaDeVia(0) + colaDeVia(1);
  for (int via = 0; via < 2; via++) {
    Agente &ag = agente[via];
    double dt = (ahora - ag.ultimoMuestreoMs) / 1000.0;
    ag.ultimoMuestreoMs = ahora;
    ag.espera += totalVisible * dt;
    bool enVerde = (via == 0 && fase == FASE_A) || (via == 1 && fase == FASE_C);
    if (!enVerde) continue;
    if (colaDeVia(via) == 0) ag.verdeVacio += dt;
    for (int i = 0; i < 3; i++) {
      bool actual = cnyDeVia(via, i);
      if (ag.cnyPrev[i] && !actual) ag.pasaron++;
      ag.cnyPrev[i] = actual;
    }
  }
}

// Al terminar el verde de una via el paso sigue abierto (falta la espera del
// resto del ciclo); se cierra en la proxima decidirVerde de esa via.
void cerrarVerde(int via) { agente[via].pendiente = true; }

// =====================================================================
// Semaforo
// =====================================================================
void apagarSemaforos() {
  digitalWrite(LR1, LOW);
  digitalWrite(LY1, LOW);
  digitalWrite(LG1, LOW);
  digitalWrite(LR2, LOW);
  digitalWrite(LY2, LOW);
  digitalWrite(LG2, LOW);
}

bool vehiculoDetectado(int pin) { return digitalRead(pin) == CNY_ACTIVO; }
bool botonPresionado(int pin) { return digitalRead(pin) == P_ACTIVO; }

int contarVehiculos1() {
  return vehiculoDetectado(CNY1) + vehiculoDetectado(CNY2) + vehiculoDetectado(CNY3);
}
int contarVehiculos2() {
  return vehiculoDetectado(CNY4) + vehiculoDetectado(CNY5) + vehiculoDetectado(CNY6);
}

void aplicarFase() {
  apagarSemaforos();
  switch (fase) {
    case FASE_A: // S1 verde, S2 rojo: el agente de la via 1 elige el verde
      digitalWrite(LG1, HIGH); digitalWrite(LR2, HIGH);
      duracionFaseActual = decidirVerde(0);
      if (detectadosRemoto >= UMBRAL_CONGESTION_RED) duracionFaseActual += BONUS_RED;
      duracionFaseActual = seg(duracionFaseActual);
      break;
    case FASE_B: // S1 amarillo, S2 rojo (regla fija)
      digitalWrite(LY1, HIGH); digitalWrite(LR2, HIGH);
      duracionFaseActual = seg(T_AMARILLO_BASE + (lluvia ? BONUS_LLUVIA : 0));
      break;
    case FASE_C: // S1 rojo, S2 verde: el agente de la via 2 elige el verde
      digitalWrite(LR1, HIGH); digitalWrite(LG2, HIGH);
      duracionFaseActual = decidirVerde(1);
      if (detectadosRemoto >= UMBRAL_CONGESTION_RED) duracionFaseActual += BONUS_RED;
      duracionFaseActual = seg(duracionFaseActual);
      break;
    case FASE_D: // S1 rojo, S2 amarillo (regla fija)
      digitalWrite(LR1, HIGH); digitalWrite(LY2, HIGH);
      duracionFaseActual = seg(T_AMARILLO_BASE + (lluvia ? BONUS_LLUVIA : 0));
      break;
  }
}

void actualizarSemaforo() {
  gestionarPeaton1();
  gestionarPeaton2();
  if (finDeFaseForzado || tFase > duracionFaseActual) {
    finDeFaseForzado = false;
    if (fase == FASE_A) cerrarVerde(0);
    if (fase == FASE_C) cerrarVerde(1);
    fase = (FaseSemaforo)((fase + 1) % 4);
    tFase = 0;
    aplicarFase();
  }
}

void registrarPeticionesPeatonales() {
  if (botonPresionado(P1) && !peaton1Pedido) { peaton1Pedido = true; tEspera1 = 0; }
  if (botonPresionado(P2) && !peaton2Pedido) { peaton2Pedido = true; tEspera2 = 0; }
}

void cortarFase() { finDeFaseForzado = true; }

void gestionarPeaton1() {
  if (fase == FASE_C) { peaton1Pedido = false; return; }
  if (fase != FASE_A || !peaton1Pedido) return;
  if (tFase < seg(VERDE_MINIMO)) return;
  bool viaLibre = (contarVehiculos1() == 0);
  bool esperoDemasiado = (tEspera1 > seg(MAX_ESPERA_PEATON));
  if (viaLibre || esperoDemasiado) cortarFase();
}

void gestionarPeaton2() {
  if (fase == FASE_A) { peaton2Pedido = false; return; }
  if (fase != FASE_C || !peaton2Pedido) return;
  if (tFase < seg(VERDE_MINIMO)) return;
  bool viaLibre = (contarVehiculos2() == 0);
  bool esperoDemasiado = (tEspera2 > seg(MAX_ESPERA_PEATON));
  if (viaLibre || esperoDemasiado) cortarFase();
}

void actualizarModoNocturno() {
  bool peatonPide = peaton1Pedido || peaton2Pedido;
  if (modoNocturno && peatonPide) {
    modoNocturno = false;
    nocturnoSuspendido = true;
    tSuspenderNocturno = 0;
    fase = FASE_A;
    tFase = 0;
    aplicarFase();
    return;
  }
  if (nocturnoSuspendido) {
    if (tSuspenderNocturno > seg(SUSPENSION_NOCTURNO)) {
      nocturnoSuspendido = false;
    } else {
      return;
    }
  }
  int l1 = analogRead(LDR1), l2 = analogRead(LDR2);
  bool oscuro = (l1 < UMBRAL_NOCHE_ENTRA) && (l2 < UMBRAL_NOCHE_ENTRA);
  bool claro = (l1 > UMBRAL_NOCHE_SALE) || (l2 > UMBRAL_NOCHE_SALE);
  if (oscuro && !modoNocturno) {
    modoNocturno = true;
    apagarSemaforos();
    blink1 = false;
    blink2 = false;
    tBlink1 = 0;
    tBlink2 = 0;
  } else if (claro && modoNocturno) {
    modoNocturno = false;
    fase = FASE_A;
    tFase = 0;
    aplicarFase();
  }
}

void actualizarParpadeoNocturno() {
  if (tBlink1 > T_BLINK) { blink1 = !blink1; tBlink1 = 0; digitalWrite(LY1, blink1); }
  if (tBlink2 > T_BLINK) { blink2 = !blink2; tBlink2 = 0; digitalWrite(LY2, blink2); }
}

float leerCO2ppm() {
  float volts = analogRead(CO2) * 3.3 / 4096.0;
  if (volts / DC_GAIN >= ZERO_POINT_VOLTAGE) return -1;
  return pow(10, ((volts / DC_GAIN) - CO2Curve[1]) / CO2Curve[2] + CO2Curve[0]);
}

// =====================================================================
// Puente serial con el computador (puente_serial.py)
// PC -> ESP32: "PING", "LLUVIA=1/0", "DET_REMOTO=<n>" (igual que nivel medio) y,
// nuevos en este nivel: "Q_DUMP" (volcar la tabla), "Q_SAVE" (guardarla en
// flash ya), "Q_RESET" (borrarla y volver a la heuristica), "EPSILON=<0..1>"
// (cuanto explora; subirlo en la demo hace visible el aprendizaje).
// =====================================================================
void leerComandosSerial() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') {
      bufferSerial.trim();
      procesarComando(bufferSerial);
      bufferSerial = "";
    } else if (c != '\r') {
      bufferSerial += c;
      if (bufferSerial.length() > 200) bufferSerial = "";
    }
  }
}

void procesarComando(String linea) {
  if (linea == "PING") {
    ultimoPingMs = millis();
    puenteVisto = true;
    Serial.println("PONG");
  } else if (linea == "BOOTLOADER") {
    // Reinicia en modo de carga por USB (ROM download), para grabar con
    // esptool --before no-reset sin tocar BOOT/RESET.
    Serial.println("BOOTLOADER ok");
#ifdef ARDUINO_ARCH_ESP32
    delay(50);
    REG_WRITE(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT);
    esp_restart();
#endif
  } else if (linea == "LLUVIA=1") {
    lluvia = true;
  } else if (linea == "LLUVIA=0") {
    lluvia = false;
  } else if (linea.startsWith("DET_REMOTO=")) {
    detectadosRemoto = linea.substring(11).toInt();
    ultimoRemotoMs = millis();
  } else if (linea == "Q_RESET") {
    borrarQ();
    Serial.println("Q_RESET ok");
  } else if (linea == "Q_SAVE") {
    size_t n = guardarQ();
    Serial.print("Q_SAVE ok "); Serial.println((int)n); // 768 = tabla completa escrita en flash
  } else if (linea == "Q_DUMP") {
    volcarQ();
  } else if (linea.startsWith("EPSILON=")) {
    float e = linea.substring(8).toFloat();
    epsilon = e < 0 ? 0 : (e > 1 ? 1 : e);
  }
}

void caducarConteoRemoto() {
  if (detectadosRemoto > 0 && millis() - ultimoRemotoMs > CADUCIDAD_REMOTO_MS) {
    detectadosRemoto = 0;
  }
}

bool conectadoAlPuente() {
  return puenteVisto && (millis() - ultimoPingMs) < TIMEOUT_PC_MS;
}

// ESP32 -> PC: la telemetria de nivel medio mas lo que el agente sabe y decide.
void actualizarTelemetria() {
  if (tTelemetria > T_TELEMETRIA) {
    tTelemetria = 0;
    Serial.print("nivel=alto modo="); Serial.print(modoActualTexto());
    Serial.print(" fase="); Serial.print("ABCD"[fase]);
    Serial.print(" dur="); Serial.print(duracionFaseActual);
    Serial.print(" co2="); Serial.print((int)leerCO2ppm());
    Serial.print(" ldr1="); Serial.print(analogRead(LDR1));
    Serial.print(" ldr2="); Serial.print(analogRead(LDR2));
    Serial.print(" cny1="); Serial.print(vehiculoDetectado(CNY1));
    Serial.print(" cny2="); Serial.print(vehiculoDetectado(CNY2));
    Serial.print(" cny3="); Serial.print(vehiculoDetectado(CNY3));
    Serial.print(" cny4="); Serial.print(vehiculoDetectado(CNY4));
    Serial.print(" cny5="); Serial.print(vehiculoDetectado(CNY5));
    Serial.print(" cny6="); Serial.print(vehiculoDetectado(CNY6));
    Serial.print(" det="); Serial.print(contarVehiculos1() + contarVehiculos2());
    Serial.print(" det_remoto="); Serial.print(detectadosRemoto);
    Serial.print(" p1="); Serial.print(botonPresionado(P1) ? 1 : 0);
    Serial.print(" p2="); Serial.print(botonPresionado(P2) ? 1 : 0);
    Serial.print(" peaton1_espera="); Serial.print(peaton1Pedido ? 1 : 0);
    Serial.print(" peaton2_espera="); Serial.print(peaton2Pedido ? 1 : 0);
    Serial.print(" lluvia="); Serial.print(lluvia ? 1 : 0);
    Serial.print(" nocturno="); Serial.print(modoNocturno ? 1 : 0);
    Serial.print(" nvs="); Serial.print(tablaDesdeFlash ? 1 : 0);
    Serial.print(" eps="); Serial.print(epsilon);
    for (int v = 0; v < 2; v++) {
      Agente &ag = agente[v];
      Serial.print(" s"); Serial.print(v + 1); Serial.print("="); Serial.print(ag.estado);
      Serial.print(" a"); Serial.print(v + 1); Serial.print("="); Serial.print((int)ACCION_VERDE[ag.accion]);
      Serial.print(" explora"); Serial.print(v + 1); Serial.print("="); Serial.print(ag.exploro ? 1 : 0);
      Serial.print(" r"); Serial.print(v + 1); Serial.print("="); Serial.print(ag.recompensa);
      Serial.print(" rtotal"); Serial.print(v + 1); Serial.print("="); Serial.print(ag.recompensaTotal);
      Serial.print(" q"); Serial.print(v + 1); Serial.print("=");
      for (int a = 0; a < NUM_ACCIONES; a++) { if (a) Serial.print("/"); Serial.print(Q[v][ag.estado][a]); }
    }
    Serial.println();
  }
}

// Texto del contexto activo (las reglas fijas y lo que ve el agente)
String modoActualTexto() {
  if (modoNocturno) return "NOCTURNO";
  String s = "";
  if (leerCO2ppm() > UMBRAL_CO2_ECO) s += "ECO+";
  if (lluvia) s += "LLUVIA+";
  if (detectadosRemoto >= UMBRAL_CONGESTION_RED) s += "RED+";
  if (s == "") return "APRENDIENDO";
  s.remove(s.length() - 1);
  return s;
}

// =====================================================================
// LCD: la primera pantalla es la autoconciencia del agente (que estado ve,
// que valores tiene aprendidos, que eligio y por que); las demas son las de
// nivel medio.
// =====================================================================
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

void filaLCD(int fila, String texto) {
  if (texto.length() > 20) texto.remove(20);
  while (texto.length() < 20) texto += ' ';
  lcd.setCursor(0, fila);
  lcd.print(texto);
}

void mostrarAnuncio() {
  if (modoNocturno) {
    filaLCD(0, "MODO NOCTURNO");
    filaLCD(1, "Ambos semaforos");
    filaLCD(2, "parpadean amarillo");
    filaLCD(3, String("LDR1:") + analogRead(LDR1) + " LDR2:" + analogRead(LDR2));
    return;
  }
  switch (anuncioActual) {
    case 0: { // Agente de la via cuyo verde esta activo (o el ultimo que decidio)
      int via = (fase == FASE_C || fase == FASE_D) ? 1 : 0;
      Agente &ag = agente[via];
      int colaPropia = ag.estado % 4, colaOtra = (ag.estado / 4) % 4;
      filaLCD(0, String("AGENTE VIA ") + (via + 1) + " #" + ag.decisiones);
      filaLCD(1, String("cola ") + colaPropia + " otra " + colaOtra + (ag.estado >= 16 ? " eco" : ""));
      filaLCD(2, String("Q ") + String(Q[via][ag.estado][0], 1) + " " + String(Q[via][ag.estado][1], 1) + " " + String(Q[via][ag.estado][2], 1));
      filaLCD(3, String("verde ") + (int)ACCION_VERDE[ag.accion] + "s " + (ag.exploro ? "explora" : "explota") + " r" + String(ag.recompensa, 1));
      break;
    }
    case 1: // Contexto y estado del puente
      filaLCD(0, "NIVEL ALTO Q-LEARN");
      filaLCD(1, modoActualTexto());
      filaLCD(2, String("Fase ") + "ABCD"[fase] + " dur:" + String(duracionFaseActual, 1) + "s");
      filaLCD(3, conectadoAlPuente() ? "PC: CONECTADO" : "PC: SIN CONEXION");
      break;
    case 2: { // CO2
      float ppm = leerCO2ppm();
      filaLCD(0, "CALIDAD DEL AIRE");
      filaLCD(1, String("CO2: ") + (int)ppm + " ppm");
      filaLCD(2, ppm > UMBRAL_CO2_ECO ? "estado ECO: activo" : "estado ECO: normal");
      filaLCD(3, "");
      break;
    }
    case 3: // LDR / modo nocturno
      filaLCD(0, "LUZ AMBIENTE");
      filaLCD(1, String("S1: ") + analogRead(LDR1));
      filaLCD(2, String("S2: ") + analogRead(LDR2));
      filaLCD(3, String("Noche<") + UMBRAL_NOCHE_ENTRA + " Dia>" + UMBRAL_NOCHE_SALE);
      break;
    case 4: // Trafico por via y recompensa acumulada
      filaLCD(0, "TRAFICO POR VIA");
      filaLCD(1, String("Via1: ") + contarVehiculos1() + "/3 R" + String(agente[0].recompensaTotal, 1));
      filaLCD(2, String("Via2: ") + contarVehiculos2() + "/3 R" + String(agente[1].recompensaTotal, 1));
      filaLCD(3, String("Otra maqueta: ") + detectadosRemoto + "/6");
      break;
    case 5: // Peatones y clima recibido por el puente
      filaLCD(0, "BOTON PEATONAL");
      filaLCD(1, String("P1:") + (botonPresionado(P1) ? "SI" : "NO") + (peaton1Pedido ? " (esperando)" : ""));
      filaLCD(2, String("P2:") + (botonPresionado(P2) ? "SI" : "NO") + (peaton2Pedido ? " (esperando)" : ""));
      filaLCD(3, lluvia ? "CLIMA: LLUVIA" : "CLIMA: despejado");
      break;
  }
}
