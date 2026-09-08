// Codigo ejemplo para enviar datos a URL con ESP32
#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your network credentials
const char *ssid = "ISA251";
const char *password = "PASSWORD";

// Replace with your target URL
const char *serverUrl = "http://isa.requestcatcher.com/api/endpoint";

void setup()
{
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void loop()
{
  // Send HTTP POST request with JSON body
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    http.begin(serverUrl);                              // Your API endpoint
    http.addHeader("Content-Type", "application/json"); // Set content type to JSON

    // Create JSON payload
    String jsonPayload = "{\"key1\":\"value1\", \"key2\":123}";

    // Send the request
    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0)
    {
      String response = http.getString(); // Get response
      Serial.println("Response:");
      Serial.println(response);
    }
    else
    {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // Free resources
  }

  delay(2000);
}
