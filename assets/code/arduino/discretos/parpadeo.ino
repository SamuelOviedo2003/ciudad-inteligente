// Parpadeo
// Definimos etiquetas
#define PIN_BLUE      2   // Pin led azul  

// Declaramos constantes
const unsigned long TBlink = 500; // Constante para el tiempo de parpadeo de 500ms

void setup() {
  // Configuramos los pines
  pinMode(PIN_BLUE, OUTPUT); // Definimos PIN_BLUE como una salida
  // Limpieza de la salida por seguridad
  digitalWrite(PIN_BLUE, LOW); // Apagamos PIN_BLUE
}

void loop() {
  digitalWrite(PIN_BLUE, HIGH);   // Prendemos PIN_BLUE
  delay(TBlink);                  // Esperamos TBlink ms
  digitalWrite(PIN_BLUE, LOW);    // Apagamos PIN_BLUE
  delay(TBlink);                  // Esperamos TBlink ms
}
