#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>



/********************************************************************/
// First we include the libraries
#define REQUIRESALARMS false
#include <OneWire.h> 
#include <DallasTemperature.h>
/********************************************************************/
// Data wire is plugged into pin 2 on the Arduino 
#define ONE_WIRE_BUS D2 
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature temperatureSensors(&oneWire);
/********************************************************************/

const int NUMBER_OF_TEMPERATURE_SENSORS = 2;
float temperatures[NUMBER_OF_TEMPERATURE_SENSORS];

void readTempSensors()
{
  temperatureSensors.requestTemperatures();
  uint8_t deviceCount = temperatureSensors.getDeviceCount();
  temperatures[0] = temperatureSensors.getTempCByIndex(0);
  for(int i=0; i<NUMBER_OF_TEMPERATURE_SENSORS && i<deviceCount; i++)
  {
  //  temperatures[i] = temperatureSensors.getTempCByIndex(i);
  }
}



void setup() {
  // put your setup code here, to run once:

  for(int i=0; i<NUMBER_OF_TEMPERATURE_SENSORS; i++)
  {
    temperatures[i] = 0.0;
  }
//  readTempSensors();
  temperatureSensors.begin();

  readTempSensors();
}

void loop() {
  // put your main code here, to run repeatedly:
}