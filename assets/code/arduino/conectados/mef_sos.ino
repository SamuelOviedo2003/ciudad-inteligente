// MEF de parpadeo con patron SOS
// Definimos estados
#define BOK   0
#define POFF  1
#define PON   2

// Definimos etiquetas
#define PIN_RED       6   // Pin led rojo
#define PIN_BATERIA   A0  // Pin voltaje de la bateria

// Declaramos constantes
int Patron[18] = {
  600, 600,
  600, 600,
  600, 600,
  2000, 600,
  2000, 600,
  2000, 600,
  600, 600,
  600, 600,
  600, 6000
}; // Constante para el patron SOS

// Declaramos variables
int i = 0;
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
  if (i > 17) i = 0;
  int S = analogRead(PIN_BATERIA); // Leemos la información de PIN_BATERIA
  tact = millis();    // Tomamos el tiempo actual
  trel = tact - tini; // Calculamos el tiempo que a pasado desde tini
  switch (estado) {
    case BOK:
      digitalWrite(PIN_RED, LOW);
      if (S < 600) estado = PON;
      break;
    case POFF:
      digitalWrite(PIN_RED, LOW);
      if (S > 650) {
        estado = BOK;
        i = 0;
      }
      if (trel > Patron[i]) {
        estado = PON;
        tini = tact;
        i++;
      }
      break;
    case PON:
      digitalWrite(PIN_RED, HIGH);
      if (S > 650) {
        estado = BOK;
        i = 0;
      }
      if (trel > Patron[i]) {
        estado = POFF;
        tini = tact;
        i++;
      }
      break;
  }
}
