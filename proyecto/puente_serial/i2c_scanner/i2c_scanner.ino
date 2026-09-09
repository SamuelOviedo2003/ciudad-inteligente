// Diagnostico: escanea el bus I2C y reporta que direcciones responden.
// Util para confirmar la direccion real del LCD (nivel_bajo asume 0x27).

#include <Wire.h>

void setup() {
  Serial.setTxTimeoutMs(0);
  Serial.begin(115200);
  delay(1000);
  Wire.begin();
  Serial.println("Escaneando bus I2C...");
}

void loop() {
  int encontrados = 0;
  for (byte direccion = 1; direccion < 127; direccion++) {
    Wire.beginTransmission(direccion);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("Dispositivo I2C encontrado en 0x");
      if (direccion < 16) Serial.print("0");
      Serial.println(direccion, HEX);
      encontrados++;
    }
  }
  if (encontrados == 0) {
    Serial.println("No se encontro ningun dispositivo I2C.");
  }
  Serial.println("---");
  delay(3000);
}
