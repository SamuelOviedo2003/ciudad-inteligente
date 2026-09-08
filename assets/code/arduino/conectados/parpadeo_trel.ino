// Parpadeo con tiempo relativo
// Definimos etiquetas
#define PIN_BLUE      2   // Pin led azul  

// Declaramos constantes
const unsigned long TBlink = 500; // Constante para el tiempo de parpadeo de 500ms

// Declaramos variables
bool encendido = false;
unsigned long tini, tact, trel;

void setup() {
  // Configuramos los pines
  pinMode(PIN_BLUE, OUTPUT); // Definimos PIN_BLUE como una salida
  // Limpieza de la salida por seguridad
  digitalWrite(PIN_BLUE, LOW); // Apagamos PIN_BLUE
  tini = millis(); // Iniciamos la marca de tiempo 
}

void loop() {
  tact = millis();    // Tomamos el tiempo actual
  trel = tact-tini;   // Calculamos el tiempo que a pasado desde tini
  if(trel > TBlink){  // Si ha pasado más tiempo que TBlink
    tini = tact;      // Actualizamos el tini
    encendido = !encendido; // Cambiamo el estado de encendido
  }
  digitalWrite(PIN_BLUE, encendido);   // Enviamos información a PIN_BLUE 
}
