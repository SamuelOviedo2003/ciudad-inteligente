// Declaracion IF de un LED Rojo con ESP32
// Definimos etiquetas
#define PIN_LDR  12  // Pin voltaje de la bateria
#define PIN_RED  5   // Pin led rojo

void setup() {
  // Configuramos los pines
  pinMode(PIN_RED, OUTPUT);   // Definimos PIN_RED como una salida
  // Limpieza de la salida por seguridad
  digitalWrite(PIN_RED, LOW); // Apagamos PIN_RED
}

void loop() {
  if(analogRead(PIN_LDR)>700){      // Comparamos si el PIN_LDR es mayor a 700
    digitalWrite(PIN_RED, HIGH);    // Prendemos PIN_RED  
  }else{                            // De lo contrario
    digitalWrite(PIN_RED, LOW);     // Apagamos PIN_RED  
  }
}
