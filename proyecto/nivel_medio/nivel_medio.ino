// Ciudad Autoadaptable - Nivel medio (CPS Generacion 1)
// Mismos semaforos y misma maqueta que nivel_bajo, pero ahora los sensores SI
// cambian el comportamiento: los setpoints (duracion de verde/amarillo) se
// ajustan segun el contexto (auto-ajuste + conciencia del contexto), y el
// sistema recibe por Serial informacion que no puede medir por si mismo
// (clima real, vía proyecto/nivel_medio/puente_serial.py <-> internet), que
// amplia su capacidad de autoadaptacion.
// Pines y calibracion de CO2 identicos a nivel_bajo/nivel_bajo.ino

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include "TimerMEF.h"
#ifdef ARDUINO_ARCH_ESP32
#include "soc/rtc_cntl_reg.h" // comando BOOTLOADER (reinicio en modo de carga)
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

// Nivel electrico que un CNY reporta cuando SI detecta un vehiculo. En el
// diagrama de Wokwi los CNY van a tierra con pull-up (reposo = HIGH, al
// "presionar"/detectar bajan a LOW) -> aqui va LOW. Al pasar a la maqueta
// fisica, verificar que nivel entrega el modulo CNY70 real al detectar un
// objeto y ajustar unicamente esta constante si hace falta.
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

// --- Setpoints base: en nivel bajo eran fijos, en nivel medio son el punto
// de partida sobre el que se calculan los tiempos reales de cada fase. ---
const double T_VERDE_BASE = 5;      // segundos
const double T_AMARILLO_BASE = 2;   // segundos
const double BONUS_CONGESTION = 3;  // seg. extra de verde si la via tiene trafico
const double BONUS_ECO = 2;         // seg. extra de verde si el CO2 esta alto (menos frenadas/arrancadas)
const double BONUS_LLUVIA = 1;      // seg. extra de amarillo si el puente reporta lluvia real
const double BONUS_RED = 2;         // seg. extra de verde si la OTRA maqueta (via internet) esta muy congestionada
const int UMBRAL_CONGESTION = 2;    // CNY activos (de 3) para considerar una via "congestionada"
const int UMBRAL_CONGESTION_RED = 4; // conteo remoto (0-6) para considerar congestionada a la otra maqueta
const int UMBRAL_CO2_ECO = 800;     // ppm
// Histeresis del modo nocturno: entra cuando ambos LDR bajan de UMBRAL_NOCHE_ENTRA
// y solo sale cuando alguno supera UMBRAL_NOCHE_SALE. Con un solo umbral, el
// ruido del ADC alrededor de ese valor hacia entrar y salir del nocturno varias
// veces por segundo, reiniciando el ciclo en fase A cada vez.
// Valores medidos en las maquetas fisicas (2026-09-09, luz de habitacion):
// placa A ~250/330, placa B ~1550/300; tapadas con la mano bajan a 13-200. Con
// el umbral anterior (800) la placa A quedaba en nocturno de dia. En Wokwi el
// potenciometro de LDR debe bajar de ~4 % (value < 37) para simular la noche.
const int UMBRAL_NOCHE_ENTRA = 150;  // lectura LDR (0-4095)
const int UMBRAL_NOCHE_SALE = 250;
const double VERDE_MINIMO = 2;       // seg.: un peaton nunca corta un verde antes de esto
const double MAX_ESPERA_PEATON = 6;  // seg.: con trafico, el peaton espera como mucho esto (self-regulation)

enum FaseSemaforo { FASE_A, FASE_B, FASE_C, FASE_D };
FaseSemaforo fase = FASE_A;
Timer tFase;
double duracionFaseActual = T_VERDE_BASE;

// --- Modos de operacion (SOM): cambian que setpoints se usan ---
// Prioridad explicita entre modos: PEATONAL > NOCTURNO > (congestion/eco/lluvia,
// que no son modos aparte, solo ajustan la duracion dentro del ciclo normal).
// Sin esta regla, un peaton que presiona el boton de noche quedaba ignorado
// porque el modo nocturno nunca llamaba a la logica peatonal.
bool modoNocturno = false;
bool nocturnoSuspendido = false; // true mientras se atiende a un peaton de noche
Timer tSuspenderNocturno;
const double SUSPENSION_NOCTURNO = 20; // seg: tiempo para un ciclo completo antes de reevaluar oscuridad
bool lluvia = false; // llega por Serial desde puente_serial.py (clima real de internet)

// --- Coordinacion con la OTRA maqueta de ciudad, vía internet (no USB directo
// como en nivel_bajo): puente_serial.py publica el conteo local en un topico
// de ntfy.sh y recibe de vuelta el de la otra maqueta. Es la "amplificacion de
// autoadaptabilidad" especifica de nivel medio: la vía extiende su verde no
// solo por su propio trafico, sino por saber que la otra interseccion de la
// ciudad esta congestionada, sin cablear las dos maquetas entre si. ---
int detectadosRemoto = 0; // ultimo conteo (0-6) recibido de la otra maqueta
// El puente de la otra maqueta republica su conteo al menos cada 2 min
// (latido). Si en 5 min no llega nada, la otra maqueta o su puente se
// apagaron: se descarta el conteo para no seguir extendiendo el verde por una
// congestion que ya nadie confirma.
unsigned long ultimoRemotoMs = 0;
const unsigned long CADUCIDAD_REMOTO_MS = 300000;

// --- Peticion peatonal. La pulsacion se memoriza (como el boton de un cruce
// real) en cualquier fase y modo; se atiende cuando su semaforo esta en verde:
// se corta el verde si la via esta libre, o se espera hasta un maximo si hay
// trafico, pero nunca antes de VERDE_MINIMO. La peticion se borra cuando el
// semaforo del peaton llega a rojo (ahi cruza). ---
bool peaton1Pedido = false;
bool peaton2Pedido = false;
bool finDeFaseForzado = false; // un peaton pidio terminar la fase actual
Timer tEspera1;
Timer tEspera2;

// --- Modo demanda (auto-optimizacion): una via con autos conserva el verde
// mientras la otra este vacia (no hay a quien darle paso), y una via vacia
// cede su verde en cuanto la otra tiene un auto, apenas cumple VERDE_MINIMO.
// Con las dos vias vacias o las dos con autos, ciclo normal. El peaton sigue
// por encima: gestionarPeatonX corta un verde sostenido como cualquier otro. ---
bool verdeSostenido = false; // el verde actual se mantiene por demanda (LCD/telemetria)

// --- Parpadeo del modo nocturno (ambos amarillos, independientes) ---
Timer tBlink1;
Timer tBlink2;
bool blink1 = false;
bool blink2 = false;
const double T_BLINK = 0.5;

// --- Anuncios rotativos del LCD (ahora muestran decisiones, no solo lecturas) ---
const int NUM_ANUNCIOS = 5;
int anuncioActual = 0;
Timer tAnuncio;
const double T_ANUNCIO = 3;
Timer tRefresco;
const double T_REFRESCO = 0.3;

// --- Enlace con el computador (puente_serial.py) ---
Timer tTelemetria;
const double T_TELEMETRIA = 1;
unsigned long ultimoPingMs = 0;
bool puenteVisto = false; // sin esto, ultimoPingMs=0 se leia como "conectado" los primeros 5 s
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

  // Placa fisica: compilar con CDCOnBoot=cdc (Serial = HWCDC, el USB nativo);
  // setTxTimeoutMs(0) evita que el loop se bloquee si el puerto USB esta
  // abierto pero nadie lo lee. Wokwi: compilar SIN esa opcion, porque su
  // monitor serial esta en el UART0 y con cdc no se ve nada del sketch.
#if ARDUINO_USB_CDC_ON_BOOT
  Serial.setTxTimeoutMs(0);
#endif
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

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
    actualizarSemaforo();
  }
  actualizarAnuncio();
  actualizarRefresco();
  actualizarTelemetria();
}

// --- Semaforo: mismos 4 estados de nivel bajo, pero la duracion de cada uno
// se recalcula al entrar a la fase segun sensores y modos activos ---
void apagarSemaforos() {
  digitalWrite(LR1, LOW);
  digitalWrite(LY1, LOW);
  digitalWrite(LG1, LOW);
  digitalWrite(LR2, LOW);
  digitalWrite(LY2, LOW);
  digitalWrite(LG2, LOW);
}

// CNY: en la maqueta van a tierra con pull-up, es decir que en reposo (nada
// detectado) leen HIGH y solo bajan a LOW cuando detectan un objeto. Por eso
// "detectado" se define como LOW, no HIGH (si se invirtiera, la via se veria
// "congestionada" todo el tiempo con solo dejar la maqueta quieta).
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
    case FASE_A: // S1 verde, S2 rojo
      digitalWrite(LG1, HIGH); digitalWrite(LR2, HIGH);
      duracionFaseActual = T_VERDE_BASE;
      if (contarVehiculos1() >= UMBRAL_CONGESTION) duracionFaseActual += BONUS_CONGESTION;
      if (leerCO2ppm() > UMBRAL_CO2_ECO) duracionFaseActual += BONUS_ECO;
      if (detectadosRemoto >= UMBRAL_CONGESTION_RED) duracionFaseActual += BONUS_RED;
      break;
    case FASE_B: // S1 amarillo, S2 rojo
      digitalWrite(LY1, HIGH); digitalWrite(LR2, HIGH);
      duracionFaseActual = T_AMARILLO_BASE + (lluvia ? BONUS_LLUVIA : 0);
      break;
    case FASE_C: // S1 rojo, S2 verde
      digitalWrite(LR1, HIGH); digitalWrite(LG2, HIGH);
      duracionFaseActual = T_VERDE_BASE;
      if (contarVehiculos2() >= UMBRAL_CONGESTION) duracionFaseActual += BONUS_CONGESTION;
      if (leerCO2ppm() > UMBRAL_CO2_ECO) duracionFaseActual += BONUS_ECO;
      if (detectadosRemoto >= UMBRAL_CONGESTION_RED) duracionFaseActual += BONUS_RED;
      break;
    case FASE_D: // S1 rojo, S2 amarillo
      digitalWrite(LR1, HIGH); digitalWrite(LY2, HIGH);
      duracionFaseActual = T_AMARILLO_BASE + (lluvia ? BONUS_LLUVIA : 0);
      break;
  }
}

void actualizarSemaforo() {
  gestionarDemanda();
  gestionarPeaton1();
  gestionarPeaton2();
  if (finDeFaseForzado || (tFase > duracionFaseActual && !verdeSostenido)) {
    finDeFaseForzado = false;
    fase = (FaseSemaforo)((fase + 1) % 4);
    tFase = 0;
    aplicarFase();
  }
}

// Memoriza las pulsaciones en cualquier fase y modo (incluido el nocturno).
void registrarPeticionesPeatonales() {
  if (botonPresionado(P1) && !peaton1Pedido) { peaton1Pedido = true; tEspera1 = 0; }
  if (botonPresionado(P2) && !peaton2Pedido) { peaton2Pedido = true; tEspera2 = 0; }
}

// Termina la fase actual en este mismo ciclo del loop (ver actualizarSemaforo).
void cortarFase() { finDeFaseForzado = true; }

// Modo demanda: solo actua en los verdes (A: via 1, C: via 2). Sostiene el
// verde de la via que tiene autos si la otra esta vacia, y corta el verde de
// una via vacia si la otra tiene autos. Si el verde sostenido deja de serlo
// (llego un auto a la otra via) y ya cumplio su duracion, termina en el acto.
void gestionarDemanda() {
  verdeSostenido = false;
  if (fase != FASE_A && fase != FASE_C) return;
  int propia = (fase == FASE_A) ? contarVehiculos1() : contarVehiculos2();
  int otra = (fase == FASE_A) ? contarVehiculos2() : contarVehiculos1();
  if (propia > 0 && otra == 0) verdeSostenido = true;
  else if (propia == 0 && otra > 0 && tFase >= VERDE_MINIMO) cortarFase();
}

// P1 se atiende cuando S1 esta en verde (fase A): corta el verde si la via 1
// esta libre, o espera hasta MAX_ESPERA_PEATON si hay trafico; nunca antes de
// VERDE_MINIMO. En fase C S1 esta en rojo: el peaton cruza y la peticion se borra.
void gestionarPeaton1() {
  if (fase == FASE_C) { peaton1Pedido = false; return; }
  if (fase != FASE_A || !peaton1Pedido) return;
  if (tFase < VERDE_MINIMO) return;
  bool viaLibre = (contarVehiculos1() == 0);
  bool esperoDemasiado = (tEspera1 > MAX_ESPERA_PEATON);
  if (viaLibre || esperoDemasiado) cortarFase();
}

void gestionarPeaton2() {
  if (fase == FASE_A) { peaton2Pedido = false; return; }
  if (fase != FASE_C || !peaton2Pedido) return;
  if (tFase < VERDE_MINIMO) return;
  bool viaLibre = (contarVehiculos2() == 0);
  bool esperoDemasiado = (tEspera2 > MAX_ESPERA_PEATON);
  if (viaLibre || esperoDemasiado) cortarFase();
}

// --- Modo nocturno: si ambos LDR estan oscuros, se reemplaza el ciclo de 4
// fases por ambos amarillos parpadeando (como un semaforo real de madrugada).
// Prioridad: un peaton pidiendo cruzar interrumpe el nocturno de inmediato
// (si no, quedaria ignorado, ya que el nocturno no corre gestionarPeatonX). ---
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
    if (tSuspenderNocturno > SUSPENSION_NOCTURNO) {
      nocturnoSuspendido = false; // ya se le dio un ciclo completo al peaton, se reevalua la oscuridad
    } else {
      return; // no reevaluar oscuridad todavia, dejar correr el ciclo normal
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

// --- CO2 en ppm, misma formula que nivel_bajo/esp_pruebas.ino ---
float leerCO2ppm() {
  float volts = analogRead(CO2) * 3.3 / 4096.0;
  if (volts / DC_GAIN >= ZERO_POINT_VOLTAGE) return -1;
  return pow(10, ((volts / DC_GAIN) - CO2Curve[1]) / CO2Curve[2] + CO2Curve[0]);
}

// --- Puente serial con el computador (puente_serial.py) ---
// Protocolo de entrada (PC -> ESP32), una linea de texto por comando:
//   "PING"       -> el puente sigue vivo (responde "PONG", ver LCD anuncio 0)
//   "LLUVIA=1/0" -> clima real obtenido de internet por el puente
void leerComandosSerial() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') {
      bufferSerial.trim();
      procesarComando(bufferSerial);
      bufferSerial = "";
    } else if (c != '\r') {
      bufferSerial += c;
      if (bufferSerial.length() > 200) bufferSerial = ""; // linea corrupta o sin terminar, descartar
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
    // esptool --before no-reset sin tocar BOOT/RESET: en algunos puertos USB
    // el auto-reset de esptool no funciona con este firmware.
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

// Protocolo de salida (ESP32 -> PC), telemetria en texto plano cada
// T_TELEMETRIA segundos; puente_serial.py la convierte a JSON antes de
// reenviarla a internet.
void actualizarTelemetria() {
  if (tTelemetria > T_TELEMETRIA) {
    tTelemetria = 0;
    Serial.print("modo="); Serial.print(modoActualTexto());
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
    Serial.print(" demanda="); Serial.print(verdeSostenido ? 1 : 0);
    Serial.print(" p1="); Serial.print(botonPresionado(P1) ? 1 : 0);
    Serial.print(" p2="); Serial.print(botonPresionado(P2) ? 1 : 0);
    Serial.print(" peaton1_espera="); Serial.print(peaton1Pedido ? 1 : 0);
    Serial.print(" peaton2_espera="); Serial.print(peaton2Pedido ? 1 : 0);
    Serial.print(" lluvia="); Serial.print(lluvia ? 1 : 0);
    Serial.print(" nocturno="); Serial.println(modoNocturno ? 1 : 0);
  }
}

// Texto corto del/los modo(s) activos, para el LCD y para razonar la decision
String modoActualTexto() {
  if (modoNocturno) return "NOCTURNO";
  String s = "";
  if (verdeSostenido) s += "DEMANDA+";
  if (contarVehiculos1() >= UMBRAL_CONGESTION) s += "CONG1+";
  if (contarVehiculos2() >= UMBRAL_CONGESTION) s += "CONG2+";
  if (leerCO2ppm() > UMBRAL_CO2_ECO) s += "ECO+";
  if (lluvia) s += "LLUVIA+";
  if (detectadosRemoto >= UMBRAL_CONGESTION_RED) s += "RED+";
  if (s == "") return "NORMAL";
  s.remove(s.length() - 1); // quita el '+' final
  return s;
}

// --- LCD: en nivel bajo mostraba lecturas crudas; en nivel medio muestra el
// modo de operacion activo y POR QUE se activo (auto-conciencia visible) ---
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

// Escribe una fila completa (20 columnas, rellena o recorta) sin lcd.clear():
// borrar la pantalla cada 0.3 s se ve como parpadeo en un LCD real.
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
    case 0: // Modo de operacion activo y estado del puente con el computador
      filaLCD(0, "MODO DE OPERACION");
      filaLCD(1, modoActualTexto());
      filaLCD(2, String("Fase ") + "ABCD"[fase] + " dur:" + String(duracionFaseActual, 1) + "s");
      filaLCD(3, conectadoAlPuente() ? "PC: CONECTADO" : "PC: SIN CONEXION");
      break;
    case 1: { // CO2 / modo eco
      float ppm = leerCO2ppm();
      filaLCD(0, "CALIDAD DEL AIRE");
      filaLCD(1, String("CO2: ") + (int)ppm + " ppm");
      filaLCD(2, ppm > UMBRAL_CO2_ECO ? "MODO ECO: activo" : "MODO ECO: normal");
      filaLCD(3, "");
      break;
    }
    case 2: // LDR / modo nocturno
      filaLCD(0, "LUZ AMBIENTE");
      filaLCD(1, String("S1: ") + analogRead(LDR1));
      filaLCD(2, String("S2: ") + analogRead(LDR2));
      filaLCD(3, String("Noche<") + UMBRAL_NOCHE_ENTRA + " Dia>" + UMBRAL_NOCHE_SALE);
      break;
    case 3: // CNY / congestion por via
      filaLCD(0, "TRAFICO POR VIA");
      filaLCD(1, String("Via1: ") + contarVehiculos1() + "/3 " + (contarVehiculos1() >= UMBRAL_CONGESTION ? "CONGESTION" : ""));
      filaLCD(2, String("Via2: ") + contarVehiculos2() + "/3 " + (contarVehiculos2() >= UMBRAL_CONGESTION ? "CONGESTION" : ""));
      filaLCD(3, String("Otra maqueta: ") + detectadosRemoto + "/6");
      break;
    case 4: // Peatones y clima recibido por el puente
      filaLCD(0, "BOTON PEATONAL");
      filaLCD(1, String("P1:") + (botonPresionado(P1) ? "SI" : "NO") + (peaton1Pedido ? " (esperando)" : ""));
      filaLCD(2, String("P2:") + (botonPresionado(P2) ? "SI" : "NO") + (peaton2Pedido ? " (esperando)" : ""));
      filaLCD(3, lluvia ? "CLIMA: LLUVIA" : "CLIMA: despejado");
      break;
  }
}
