#include <Arduino.h>
#include <Wire.h>


//ports
// A4 SCL
// A6 SDA
const int NUMBER_OF_INPUTS = 8;
const int switchInputs[] = {PIN_A0,PIN_A1,PIN_A2,PIN_A3,PIN_A5,PIN_A7,PIN_B0,PIN_B1};


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
}

void setup() {
  // put your setup code here, to run once:
  Wire.begin(2);                // join i2c bus with address #2
  Wire.onRequest(requestEvent); // register event

  for(int i=0; i<NUMBER_OF_INPUTS; i++)
  {
    pinMode(switchInputs[i], INPUT);
  }     

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
}


