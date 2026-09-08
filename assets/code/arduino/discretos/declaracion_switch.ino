// Declaracion SWITCH
// Definimos etiquetas
#define PIN_GREEN     3   // Pin led verde

// Declaramos variables
byte brillo = 0;
char paso = 5; // Paso para el aumento y disminución del brillo

void setup() {
  // Configuramos los pines
  pinMode(PIN_GREEN, OUTPUT); // Definimos PIN_GREEN como una salida
}

void loop() {
  analogWrite(PIN_GREEN, brillo);           // Definimos el brillo a PIN_GREEN
  brillo = brillo + paso;                   // Aumentamos o disminuimos el brillo
  switch(brillo){   // Dependiendo del valor de brillo
    case 0:         // Si es 0 
      paso = 5;     // Vamos a aumentar el brillo
      break;
    case 255:       // Si es 255
      paso = -5;    // Vamos a disminuir el brillo
      break;
  }
  delay(50);            // Esperamos 50 ms
}
