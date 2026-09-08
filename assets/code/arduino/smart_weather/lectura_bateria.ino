// Lectura de la bateria

// Definimos etiquetas
#define PIN_BATERIA   A0  // Pin voltaje de la bateria

void setup() {
  Serial.begin(115200); // Iniciamos la comunicacion serial
}

void loop() {
  int valEntrada = analogRead(PIN_BATERIA);  // Leemos la información de la bateria
  float voltiosEntrada = (float)valEntrada*9.2/1023.0;  // Convertimos la lectura en voltios
  Serial.print("Bateria: ");                 // Enviamos por puerto Serial
  Serial.print(valEntrada);
  Serial.print("/1023 >> ");
  Serial.print(voltiosEntrada);       
  Serial.println(" voltios");                    
  delay(10);                                  // Esperamos 10 ms y repetimos      
}
