// Declaracion SWITCH con LED Rojo en ESP32
// Definimos etiquetas
#define PIN_RED 5 // Pin led rojo

// Declaramos variables
byte brillo = 0;
char paso = 5; // Paso para el aumento y disminución del brillo

void setup()
{
  // Configuramos los pines
  pinMode(PIN_RED, OUTPUT); // Definimos PIN_RED como una salida
}

void loop()
{
  analogWrite(PIN_RED, brillo); // Definimos el brillo a PIN_RED
  brillo = brillo + paso;       // Aumentamos o disminuimos el brillo
  switch (brillo)
  {           // Dependiendo del valor de brillo
  case 0:     // Si es 0
    paso = 5; // Vamos a aumentar el brillo
    break;
  case 255:    // Si es 255
    paso = -5; // Vamos a disminuir el brillo
    break;
  }
  delay(50); // Esperamos 50 ms
}
