// Lectura de temperatura y humedad DTH11

// Definimos etiquetas
#include "DHT.h"
#define DHTTYPE DHT11     // Usamos el sensor DHT11
#define PIN_DHT 9         // Pin sensor de temperatura

DHT dht(PIN_DHT, DHTTYPE); // Se crea el objeto dht

void setup() {
  Serial.begin(115200); // Iniciamos la comunicacion serial
}

void loop() {
  float T = dht.readTemperature();
  Serial.print("Temperatura: ");
  Serial.print(T);
  float H = dht.readHumidity();
  Serial.print(", Humedad: ");
  Serial.print(H);
  Serial.println("");
  delay(200);                                  // Esperamos 200 ms y repetimos
}
