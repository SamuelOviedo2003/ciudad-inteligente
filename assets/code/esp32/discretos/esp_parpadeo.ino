// Parpadeo de un LED Rojo con ESP32
// Definimos etiquetas
#define PIN_RED 5 // Pin led azul

// Declaramos constantes
const unsigned long TBlink = 500; // Constante para el tiempo de parpadeo de 500ms

void setup()
{
  // Configuramos los pines
  pinMode(PIN_RED, OUTPUT); // Definimos PIN_RED como una salida
  // Limpieza de la salida por seguridad
  digitalWrite(PIN_RED, LOW); // Apagamos PIN_RED
}

void loop()
{
  digitalWrite(PIN_RED, HIGH); // Prendemos PIN_RED
  delay(TBlink);                // Esperamos TBlink ms
  digitalWrite(PIN_RED, LOW);  // Apagamos PIN_RED
  delay(TBlink);                // Esperamos TBlink ms
}
