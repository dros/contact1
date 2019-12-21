#include <Arduino.h>
#include <Wire.h>

/********************************************************************/
// First we include the libraries
#define REQUIRESALARMS false
#include <OneWire.h> 
#include <DallasTemperature.h>
/********************************************************************/
// Data wire is plugged into pin 2 on the Arduino 
#define ONE_WIRE_BUS PIN_B2 
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature temperatureSensors(&oneWire);
/********************************************************************/

//ports
// A4 SCL
// A6 SDA
const int NUMBER_OF_INPUTS = 8;
const int switchInputs[] = {PIN_A0,PIN_A1,PIN_A2,PIN_A3,PIN_A5,PIN_A7,PIN_B0,PIN_B1};

const int NUMBER_OF_TEMPERATURE_SENSORS = 2;
float temperatures[NUMBER_OF_TEMPERATURE_SENSORS];

bool startTempRead = false;


void requestEvent()
{
  uint8_t readPort = 0;

  for(int i=0; i<NUMBER_OF_INPUTS; i++)
  {
    int input = digitalRead(switchInputs[i]);
    if(input)
    {
      readPort |= 1<<i;
    }
  }

  Wire.write(readPort);
  uint8_t *pUint8 = (uint8_t*)&(temperatures);
  for(unsigned int i=0; i<sizeof(temperatures); i++)
  {
    Wire.write(pUint8[i]);
  }
}


void readTempSensors()
{
  temperatureSensors.requestTemperatures();
  uint8_t deviceCount = temperatureSensors.getDeviceCount();
  temperatures[0] = temperatureSensors.getTempCByIndex(0);
  for(int i=0; i<NUMBER_OF_TEMPERATURE_SENSORS && i<deviceCount; i++)
  {
  //  temperatures[i] = temperatureSensors.getTempCByIndex(i);
  }
  
  startTempRead = false;
}

void setup() {
  // put your setup code here, to run once:
  Wire.begin(2);                // join i2c bus with address #2
  Wire.onRequest(requestEvent); // register event

  for(int i=0; i<NUMBER_OF_INPUTS; i++)
  {
    pinMode(switchInputs[i], INPUT);
  }     

  for(int i=0; i<NUMBER_OF_TEMPERATURE_SENSORS; i++)
  {
    temperatures[i] = 0.0;
  }
//  readTempSensors();
  temperatureSensors.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
  if(startTempRead)
  {
    readTempSensors();
  }
}


