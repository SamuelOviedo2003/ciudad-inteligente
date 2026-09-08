// Lectura en Bits de Fotoresistencia

// Definimos etiquetas
#define PIN_BATERIA   A0  // Pin voltaje de la bateria

void setup() {
  Serial.begin(115200); // Iniciamos la comunicacion serial
}

void loop() {
  int sensorValue = analogRead(PIN_BATERIA);  // Leemos la información de FR
  Serial.println(sensorValue);                // Enviamos por puerto Serial
  delay(10);                                  // Esperamos 10 ms y repetimos      
}
