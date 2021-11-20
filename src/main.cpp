#include <Arduino.h>
#include <EEPROM.h>
#include "ESACom.hpp"
#include "taskServer.h"

ESACom esa(&Serial2);




void setup() 
{
  setCpuFrequencyMhz(80);


  Serial.begin(115200);
  Serial2.begin(115200);


  //initialize EEPROM with predefined size for esp32
  EEPROM.begin(1);


  xTaskCreate(
  taskServer,   // Task function. 
  "taskServer",     // name of task. 
  8196,       // Stack size of task 
  NULL,        // parameter of the task 
  3,           // priority of the task 
  NULL      // Task handle to keep track of created task 
  );          // pin task to core x




  int speed = EEPROM.read(0);
  Serial.println(speed);
  if(speed)
  {
    esa.setSpeed(speed);
    Serial.println(speed);
  }

}




void loop() 
{
    if(esa.rxHandler())
    {
      Serial.print("Speed: ");
      Serial.println(esa.getStatus().speed/1000.f);
      Serial.print("SoC: ");
      Serial.println(esa.getStatus().soc);
      Serial.print("Eco: ");
      Serial.println(esa.getStatus().eco);
      Serial.print("Light: ");
      Serial.println(esa.getStatus().light);
    }
}