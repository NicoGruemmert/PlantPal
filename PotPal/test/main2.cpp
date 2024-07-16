	
#include "Arduino.h"

#define LED_GPIO 15

void setup()
{
  pinMode(LED_GPIO,OUTPUT);


}
void loop()
{
  static bool ledState = true;
  digitalWrite(LED_GPIO,ledState);
  ledState = !ledState;
  delay(500);

}