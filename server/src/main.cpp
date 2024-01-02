#include "security_manager.h"
#include "Arduino.h"
#include <esp32-hal.h>
#include "bsp.h"
#include "led_driver.h"
#include "temp_sensor.h"
#include <WiFi.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

bool LED_is_ON = false;

void setup()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.begin(BAUD_RATE);
  led_driver_init(LED_PIN);
  temp_sensor_init();
  (void)led_driver_set_state(LED_DRIVER_LOW);
}

void loop()
{
  if (Serial.available() > 0)
  {
    String receivedData = Serial.readStringUntil('\n');
    Serial.println("ESP32 Received data: " + receivedData);
    Serial.println();
    receivedData.trim();

    // Convert Hexadecimal String to Byte Array
    size_t byteLength = receivedData.length() / 2;
    unsigned char receivedDataBytes[byteLength];
    for (size_t i = 0; i < byteLength; i++)
    {
      String byteString = receivedData.substring(i * 2, (i * 2) + 2);
      receivedDataBytes[i] = (unsigned char)strtol(byteString.c_str(), NULL, 16);
    }

    // Print the received byte array for debugging
    Serial.print("ESP32 Received data (bytes): ");
    for (size_t i = 0; i < byteLength; i++)
    {
      Serial.printf("%02x", receivedDataBytes[i]);
    }
    Serial.println();

    // Decrypt the data
    unsigned char decryptedData[512]; // Adjust size as needed
    size_t decryptedDataLength;
    decrypt_aes256(receivedDataBytes, receivedData.length(), decryptedData, decryptedDataLength, (const unsigned char *)SECRET_KEY);
    Serial.println("ESP32 Decrypted data:  \n");

    // Print the decrypted data for debugging
    Serial.print("ESP32 Decrypted data: ");
    for (size_t i = 0; i < decryptedDataLength; i++)
    {
      Serial.printf("%02x", decryptedData[i]);
    }
    Serial.println();

    // Convert decrypted data back to String for further processing
    String decryptedDataStr = String((char *)decryptedData);
    Serial.println(decryptedDataStr) + "\n";

    // Find the delimiter in the decrypted data
    int delimiterIndex = decryptedDataStr.lastIndexOf(',');

    if (delimiterIndex == -1)
    {
      Serial.println("Delimiter not found in decrypted data.");
      return; // Exit if no delimiter is found in decrypted data
    }

    // Split the message into command and HMAC
    String command = receivedData.substring(0, delimiterIndex);
    String receivedHmac = receivedData.substring(delimiterIndex + 1);

    Serial.println("ESP32 Received command: " + command) + "\n";
    Serial.println("ESP32 Received HMAC: " + receivedHmac);
    +"\n";

    // Convert String to const char* before passing to the functions
    if (verifyHMAC(command.c_str(), receivedHmac.c_str()))
    {
      Serial.println("ESP32 HMAC Verified Successfully") + "\n";
      String response;

      if (command == "toggle_led_state")
      {
        if (LED_is_ON)
        {
          led_driver_set_state(LED_DRIVER_LOW);
          LED_is_ON = false;
          response = "LED turned off";
        }
        else
        {
          led_driver_set_state(LED_DRIVER_HIGH);
          LED_is_ON = true;
          response = "LED turned on";
        }
      }
      else if (command == "read_temp")
      {
        float temp = temperatureRead();
        response = "The Core Temperature is: " + String(temp) + " °C";
      }
      else
      {
        response = "Unknown command";
      }

      Serial.println("Response: " + response);

      // Send response with HMAC
      String responseHmac = computeHMAC_SHA256(response.c_str());
      Serial.println("Response HMAC: " + responseHmac) + "\n";
      Serial.println("Full response to send: " + response + "," + responseHmac) + "\n";
    }
    else
    {
      Serial.println("Invalid HMAC");
    }
  }
}
