// MEF de parpadeo condicionado a bajos niveles de bateria
// Definimos estados
#define BOK   0
#define POFF  1
#define PON   2

// Definimos etiquetas
#define PIN_GREEN     3   // Pin led verde
#define PIN_RED       6   // Pin led rojo
#define PIN_BATERIA   A0  // Pin voltaje de la bateria

// Declaramos constantes
const unsigned long TOn  = 2000; // Constante para el tiempo de encendido 2000 ms
const unsigned long TOff = 1000; // Constante para el tiempo de apagado   1000 ms

// Declaramos variables
unsigned int estado = BOK;
unsigned long tini, tact, trel;

void setup() {
  // Configuramos los pines
  pinMode(PIN_RED, OUTPUT); // Definimos PIN_RED como una salida
  // Limpieza de la salida por seguridad
  digitalWrite(PIN_RED, LOW); // Apagamos PIN_RED
  tini = millis(); // Iniciamos la marca de tiempo
}

void loop() {
  int S = analogRead(PIN_BATERIA); // Leemos la información de PIN_BATERIA
  tact = millis();    // Tomamos el tiempo actual
  trel = tact - tini; // Calculamos el tiempo que a pasado desde tini
  switch (estado) {
    case BOK:
      digitalWrite(PIN_RED, LOW);
      if (S < 600) estado = POFF;
      break;
    case POFF:
      digitalWrite(PIN_RED, LOW);
      if (S > 650) estado = BOK;
      if (trel > TOff){
        estado = PON;
        tini = tact;
      }
      break;
    case PON:
      digitalWrite(PIN_RED, HIGH);
      if (S > 650) estado = BOK;
      if (trel > TOn){
        estado = POFF;
        tini = tact;
      }
      break;
  }
}
