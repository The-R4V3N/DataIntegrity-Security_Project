#include "Arduino.h"
#include <esp32-hal.h>
#include "bsp.h"
#include "led_driver.h"
#include "temp_sensor.h"

bool LED_is_ON = false;

void setup()
{
  Serial.begin(BAUD_RATE);
  led_driver_init(LED_PIN);
  temp_sensor_init();
  (void)led_driver_set_state(LED_DRIVER_LOW);
}

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
      float temp = temp_sensor_read();

      Serial.println("The Core Temperature is: " + String(temp) + " °C");
    }
    else
    {
      Serial.println("Unknown command");
    }
  }
}
