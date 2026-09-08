// Codigo ejemplo para obtener datos de URL con ESP32
#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your network credentials
const char *ssid = "ISA251";
const char *password = "PASSWORD";

// URL to send GET request to
const char *serverUrl = "http://example.com";

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

    // Send HTTP GET request
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;

        http.begin(serverUrl);             // Specify the URL
        int httpResponseCode = http.GET(); // Make the request

        if (httpResponseCode > 0)
        {
            String response = http.getString(); // Get the response payload
            Serial.println("Response:");
            Serial.println(response);
        }
        else
        {
            Serial.print("Error on sending GET: ");
            Serial.println(httpResponseCode);
        }

        http.end(); // Free resources
    }
}

void loop()
{
    // Nothing in loop
}
