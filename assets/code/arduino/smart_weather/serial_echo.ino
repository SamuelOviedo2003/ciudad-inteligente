// Comunicacion XBee por Serial1 a Serial
// El modulo de comunicación XBee requiere que
// el conector de alimentación este conectado y
// que el display de bateria marque más de 5.0 voltios

void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600);
}

void loop()
{
  if (Serial.available() > 0)
  {
    Serial1.write(Serial.read());
  }
  if (Serial1.available() > 0)
  {
    Serial.write(Serial1.read());
  }
}
