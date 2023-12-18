#include "Arduino.h"
#include <esp32-hal.h>
#include "bsp.h"
#include "led_driver.h"
#include "temp_sensor.h"
#include <WiFi.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

bool LED_is_ON = false;

/**
 * @brief Initializes the setup for the server.
 *
 * This function sets up the serial communication, initializes the LED driver,
 * initializes the temperature sensor, WIFI is used to activate the internal tempsensor otherwhise the tempsensor return always 53.3 degres, and sets the LED driver state to low.
 */
void setup()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.begin(BAUD_RATE);
  led_driver_init(LED_PIN);
  temp_sensor_init();
  (void)led_driver_set_state(LED_DRIVER_LOW);
}

/**
 * @brief The main loop for the server.
 *
 * This function is the main loop for the server. It checks for serial input and
 * executes the appropriate command.
 */
void loop()
{
  if (Serial.available() > 0)
  {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "toggle_led_state")
    {
      if (LED_is_ON)
      {
        led_driver_set_state(LED_DRIVER_LOW);
        LED_is_ON = false;
        Serial.println("LED turned off");
      }
      else
      {
        led_driver_set_state(LED_DRIVER_HIGH);
        LED_is_ON = true;
        Serial.println("LED turned on");
      }
    }
    else if (command == "read_temp")
    {
      float temp = temperatureRead();

      Serial.println("The Core Temperature is: " + String(temp) + " °C");
    }
    else
    {
      Serial.println("Unknown command");
    }
  }
}
