// Enviar datos por wifi
/////////////////////////////////////////////////////////// WIFI
// Librerias requeridas
#include <SPI.h>
#include <WiFi101.h>

// Variables para la conexión WiFi
char ssid[] = "hotspot";     // Network SSID (name)
char pass[] = "isa20261";    // Network password
int status = WL_IDLE_STATUS; // Network status

void printWifiStatus()
{
  // Muestra el nombre de la red WiFi
  Serial.print("[WIFI]\tConectado a SSID: ");
  Serial.println(WiFi.SSID());

  // Muestra la dirección IP
  IPAddress ip = WiFi.localIP();
  Serial.print("[WIFI]\tDirección IP: ");
  Serial.println(ip);

  // Muestra la intensidad de señal:
  long rssi = WiFi.RSSI();
  Serial.print("[WIFI]\tIntensidad de señal (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void WiFiInit()
{
  // Le damos tiempo al shield WiFi de iniciar:
  delay(1000);

  // Verificamos la presencia del shield:
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("[WIFI]\tShield not present");
    while (true)
      ; // No continuamos
  }

  String fv = WiFi.firmwareVersion();
  if (fv <= "1.1.0")
    Serial.println("[WIFI]\tPor favor actualizar el firmware");

  // Intentando conectarse al WiFi:
  while (status != WL_CONNECTED)
  {
    Serial.print("[WIFI]\tIntento de connexión al SSID: ");
    Serial.println(ssid);
    // Conexión a una red por WPA/WPA2.
    // Cambiar la siguiente linea si usa un WIFI abierto o red WEP:
    status = WiFi.begin(ssid, pass);

    // Esperando 1 segundo para conexión:
    delay(1000);
  }
  // Estamos conectados, mostramos la información de la conexión:
  printWifiStatus();
}

/////////////////////////////////////////////////// SENDING DATA

#include <ArduinoHttpClient.h>

// Constantes
const unsigned long postingInterval = 2 * 1000; // Intervalo mínmo para el envio de datos 2*1000 ms
const unsigned int sensorCount = 3;             // Numero de datos a enviar

// Variables
char *host = "isa.requestcatcher.com"; // No incluir el https://
char *path = "/post";                  // Path
int port = 80;                         // Puerto

WiFiClient wifi;
HttpClient client = HttpClient(wifi, host, port);

String response;
int statusCode = 0;

void POST_send(int sensorCount, char *sensorNames[], float values[])
{
  String contentType = "application/json";
  String postData = "";
  String request = String(path) + "?";
  for (int idx = 0; idx < sensorCount; idx++)
  {
    if (idx != 0)
      request += "&";
    request += sensorNames[idx];
    request += "=";
    request += values[idx];
  }
  client.post(request, contentType, postData);
  statusCode = client.responseStatusCode();
  response = client.responseBody();
  Serial.print("status-code: ");
  Serial.println(statusCode);
}

char *nameArray[] = {"temperature", "humidity", "battery"}; // Nombres de las variables en la nube
float sensorValues[sensorCount];                            // Vector de valores
unsigned long lastConnectionTime = 0;                       // Marca temporal para el ultimo envio de datos

void POST(float T, float H, float B)
{
  sensorValues[0] = T;
  sensorValues[1] = H;
  sensorValues[2] = B;
  if ((millis() - lastConnectionTime) > postingInterval)
  {
    Serial.print("[SEND]\tEnviando datos > ");
    POST_send(sensorCount, nameArray, sensorValues); // REST call
    lastConnectionTime = millis();                   // Actualizamos la marca temporal
  }
}

//////////////////////////////////////////////////////// ARDUINO

void setup()
{

  // Comunicación
  Serial.begin(115200);
  WiFiInit(); // Iniciando la conexión WIFI
}

void loop()
{
  POST(1.0, 2.0, 3.0);
}
