// Codigo del maestro para probar la simulacion de la maqueta smart weather
/* 

Este codigo es utilizado para probar las conexiones del 
simulador maestro a esclavo. Este es el código del 
maestro, el cual enviará ordenes al esclavo dependiendo
de la lectura de su entradas. 

*/

// Definimos librerias
#include "DHT.h"

// Definimos etiquetas
// #define DHTTYPE DHT11     // Usamos el sensor DHT11 en la maqueta
#define DHTTYPE DHT22     // Usamos el sensor DHT22 en el simulador
#define PIN_BATERIA   A0  // Pin voltaje de la bateria
#define PIN_DHT       9   // Pin sensor de temperatura
#define PIN_COOLER    8   // Pin ventilador
#define PIN_RED       6   // Pin led rojo
#define PIN_GREEN     3   // Pin led verde
#define PIN_BLUE      2   // Pin led azul  

DHT dht(PIN_DHT, DHTTYPE); // Se crea el objeto dht
float T,H,V;
int msg;

void medir(){
  T = dht.readTemperature();
  H = dht.readHumidity();
  int valEntrada = analogRead(PIN_BATERIA);  // Leemos la información de la bateria
  V = (float)valEntrada*9.2/1023.0;  // Convertimos la lectura en voltios
}

void setup() {
  Serial.begin(115200); // Iniciamos la comunicacion serial
  pinMode(PIN_COOLER, OUTPUT);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
}

void loop() { // Ciclo infinito
  medir();

  if(V<5){
    digitalWrite(PIN_RED,HIGH);
    digitalWrite(PIN_GREEN,LOW);
    digitalWrite(PIN_BLUE,LOW);
    msg = 1;
  }else if(V<7){
    digitalWrite(PIN_RED,LOW);
    digitalWrite(PIN_GREEN,HIGH);
    digitalWrite(PIN_BLUE,LOW);
    msg = 2;
  }
  else{
    digitalWrite(PIN_RED,LOW);
    digitalWrite(PIN_GREEN,LOW);
    digitalWrite(PIN_BLUE,HIGH);
    msg = 3;
  }

  if(T>30){
    digitalWrite(PIN_COOLER,LOW);
  }else{
    digitalWrite(PIN_COOLER,LOW);
    msg = -msg;
  }

  Serial.println(msg);
  
  delay(1000); // Esperamos 1000 ms y repetimos      
}
