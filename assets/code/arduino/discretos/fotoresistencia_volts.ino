// Lectura en Voltios de Fotoresistencia

// Definimos etiquetas
#define PIN_BATERIA   A0  // Pin voltaje de la bateria

void setup() {
  Serial.begin(115200); // Iniciamos la comunicacion serial
}

void loop() {
  int sensorValue = analogRead(PIN_BATERIA);  // Leemos la información de A0
  // Cambiar a 9.2/1023.0 para coincidir con el indicador
  float voltaje = sensorValue * (5.0 / 1023.0); 
  Serial.println(voltaje);          // Enviamos por puerto Serial
  delay(10);                        // Esperamos 10 ms y repetimos      
}
