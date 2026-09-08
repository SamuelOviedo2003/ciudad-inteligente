// Codigo del esclavo para probar la simulacion de la maqueta smart weather
/* 

Este codigo es utilizado para probar las conexiones del 
simulador maestro a esclavo. Este es el código del 
esclavo, el cual recibira ordenes al maestro dependiendo
de la lectura de las entradas de dicho maestro. 

*/

// Definimos etiquetas
#define PIN_COOLER    8   // Pin ventilador
#define PIN_RED       6   // Pin led rojo
#define PIN_GREEN     3   // Pin led verde
#define PIN_BLUE      2   // Pin led azul  

int msg = -1;

void setup() {
  Serial.begin(115200); // Iniciamos la comunicacion serial
  pinMode(PIN_COOLER, OUTPUT);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
}

void loop() { // Ciclo infinito
  if(Serial.available()){
    int aux = Serial.parseInt();
    if(aux!=0){
      msg = aux;
      Serial.print("Recibido msg: ");
      Serial.println(msg);
    }
  }

  int V = abs(msg);

  if(V==1){
    digitalWrite(PIN_RED,HIGH);
    digitalWrite(PIN_GREEN,LOW);
    digitalWrite(PIN_BLUE,LOW);
  }else if(V==2){
    digitalWrite(PIN_RED,LOW);
    digitalWrite(PIN_GREEN,HIGH);
    digitalWrite(PIN_BLUE,LOW);
  }
  else{
    digitalWrite(PIN_RED,LOW);
    digitalWrite(PIN_GREEN,LOW);
    digitalWrite(PIN_BLUE,HIGH);
  }

  if(msg>0){
    digitalWrite(PIN_COOLER,HIGH);
  }else{
    digitalWrite(PIN_COOLER,LOW);
  }    
}
