// Verificar actuadores del Smart Weather

// Definimos etiquetas
#define PIN_COOLER    8   // Pin ventilador
#define PIN_RED       6   // Pin led rojo
#define PIN_GREEN     3   // Pin led verde
#define PIN_BLUE      2   // Pin led azul  

void setup() {
  Serial.begin(115200); // Iniciamos la comunicacion serial
}

void loop() {
  pinMode(PIN_COOLER, OUTPUT);
  digitalWrite(PIN_COOLER, HIGH);
  delay(1000);
  digitalWrite(PIN_COOLER, LOW);
  pinMode(PIN_RED, OUTPUT);
  digitalWrite(PIN_RED, HIGH);
  delay(1000);
  digitalWrite(PIN_RED, LOW);
  pinMode(PIN_GREEN, OUTPUT);
  digitalWrite(PIN_GREEN, HIGH);
  delay(1000);
  digitalWrite(PIN_GREEN, LOW);
  pinMode(PIN_BLUE, OUTPUT);
  digitalWrite(PIN_BLUE, HIGH);
  delay(1000);
  digitalWrite(PIN_BLUE, LOW);
}
