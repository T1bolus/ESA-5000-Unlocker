#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>

#include "webOTA.hpp"


const char* ssid = "ESP32";
const char* password = "12345678";

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80); 
extern ESACom esa;


void indexPage();
void setOptionPages();
extern "C" uint8_t temprature_sens_read();

void taskServer(void *pvParameters)
{
  WiFi.softAP(ssid, password, 1, 0 , 1);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  
  if (MDNS.begin("esa.unlocker")) //http://esa.unlocker/
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", HTTP_GET, indexPage);
  server.on("/", HTTP_POST, setOptionPages);

  //otaUpdate
  server.on("/update", HTTP_GET, updatePage);
  server.on("/update", HTTP_POST, updateStatus, updateUpload);

  server.onNotFound
  (
    []()
    {
        server.send(404, "text/plain", "Not found");
        Serial.println("404 Request!");
    }
  );

  server.begin();


  while(true)
  {
    server.handleClient();
    delay(250);
  }
}


void indexPage()
{
  uint8_t temp_farenheit = temprature_sens_read();
  //convert farenheit to celcius
  double temp = ( temp_farenheit - 32 ) / 1.8;
  
  String message = "<html><head><title>ESA5000 Unlocker</title></head>";
  message += "<body style=\"text-align: center; font-size: 2.5em; background-color: black; color: white;\">";
  message += "<h1>ESA5000 Unlocker</h1>";
  message += "<form method=\"POST\">";
  message += "<span>";
  message += "Speed in km/h: ";
  message += "</span><br>";
  message += "<span style=\"font-size: 0.4em;\">";
  message += "<input type=\"number\" value=\"25\" name=\"speed\">";
  message += "<br>";
  message += "Internal Temperature ESP32: ";
  message += temp;
  message += " °C<br>";
  message += "Online since: ";
  message += (uint32_t)(esp_timer_get_time() / (1000 * 1000 * 60));
  message += "  minutes<br><br>";
  message += "<input type=\"submit\" value=\"Set options\">";
  message += "</form>";
  message += "</span>";
  message += "</body></html>";
  server.send(200, "text/html; charset=utf-8", message);
}

void setOptionPages()
{
    //Read parameters and set it
    int speed = server.arg("speed").toInt();
    if(speed > 255) speed = 255;


    EEPROM.write(0, speed);
    EEPROM.commit();

    
    esa.setSpeed(speed);

    Serial.println(speed);

    indexPage();
}