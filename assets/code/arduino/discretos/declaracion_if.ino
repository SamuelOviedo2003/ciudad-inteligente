// Declaracion IF
// Definimos etiquetas
#define PIN_BATERIA   A0  // Pin voltaje de la bateria
#define PIN_RED       6   // Pin led rojo

void setup() {
  // Configuramos los pines
  pinMode(PIN_RED, OUTPUT);   // Definimos PIN_RED como una salida
  // Limpieza de la salida por seguridad
  digitalWrite(PIN_RED, LOW); // Apagamos PIN_RED
}

void loop() {
  if(analogRead(PIN_BATERIA)<700){  // Comparamos si el PIN_BATERIA es menor a 700
    digitalWrite(PIN_RED, HIGH);    // Prendemos PIN_RED  
  }else{                            // De lo contrario
    digitalWrite(PIN_RED, LOW);     // Apagamos PIN_RED  
  }
}
