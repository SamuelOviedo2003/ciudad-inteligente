// Conectar al Wifi
// Librerias requeridas
#include <SPI.h>
#include <WiFi101.h>

// Variables para la conexión WiFi
char ssid[] = "hotspot";     // Network SSID (name)
char pass[] = "isa20261";       // Network password
int status = WL_IDLE_STATUS;    // Network status

void printWifiStatus() {
  // Muestra el nombre de la red WiFi
  Serial.print("[WIFI] Conectado a SSID: ");
  Serial.println(WiFi.SSID());

  // Muestra la dirección IP
  IPAddress ip = WiFi.localIP();
  Serial.print("[WIFI] Dirección IP: ");
  Serial.println(ip);

  // Muestra la intensidad de señal:
  long rssi = WiFi.RSSI();
  Serial.print("[WIFI] Intensidad de señal (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void WiFiInit() {
  // Le damos tiempo al shield WiFi de iniciar:
  delay(1000);

  // Verificamos la presencia del shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("[WIFI] Shield not present");
    while (true); // No continuamos
  }

  String fv = WiFi.firmwareVersion();
  if (fv <= "1.1.0" )
    Serial.println("[WIFI] Por favor actualizar el firmware");

  // Intentando conectarse al WiFi:
  while (status != WL_CONNECTED) {
    Serial.print("[WIFI] Intento de connexión al SSID: ");
    Serial.println(ssid);
    // Conexión a una red por WPA/WPA2. 
    // Cambiar la siguiente linea si usa un WIFI abierto o red WEP:
    status = WiFi.begin(ssid, pass);

    // Esperando 10 segundos para conexión:
    delay(10000);
  }
  // Estamos conectados, mostramos la información de la conexión:
  printWifiStatus();
}


void setup() {
  Serial.begin(115200);
  WiFiInit(); //Initialize communications through WiFi Shield
}

void loop() {
}
