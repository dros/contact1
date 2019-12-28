#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>

#include <Wire.h>

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

#include "../../../passwordFile.h"
// includes 
//const char* update_username = "xxx";
//const char* update_password = "xxx";
//const char* ssid = "xxx";
//const char* ssid_password = "xxx";
//const char* mqtt_server = "x.x.x.x";


const char* host = "esp8266-webupdate";
const char* update_path = "/firmware";
const char* deviceName = "cecilieHAN";

const int NUMBER_OF_TEMPERATURE_SENSORS = 2;
float temperatures[NUMBER_OF_TEMPERATURE_SENSORS];

const int DEBUG_COUNT = 10;
int debug[DEBUG_COUNT];

std::string lightControlTopic;
std::vector<std::string> lightTopics;
std::string lightDebugTopic;
std::string debugInfo;

const int NUMBER_OF_IO = 8;
bool lightsControl[NUMBER_OF_IO];
bool lightsControlLast[NUMBER_OF_IO];

WiFiClient espClient;
PubSubClient client(espClient);

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void WiFiConnect();
void MQTT_connect();
void MQTT_setup();
void MQTT_callback(char* topic, byte* payload, unsigned int length);

void webServerSetup();

void readTempSensors();
void handleContacts();
void readContacts();

void setup() {
  // put your setup code here, to run once:

  for(int i=0; i<NUMBER_OF_TEMPERATURE_SENSORS; i++)
  {
    temperatures[i] = 0.0;
  }
//  readTempSensors();
  temperatureSensors.begin();
  readTempSensors();

  // i2c
  Wire.begin();

  Serial.begin(9600);
  Serial.println();
  Serial.println("Booting Sketch...");
  WiFi.mode(WIFI_AP_STA);
  WiFiConnect();

  MDNS.begin(host);
  MDNS.addService("http", "tcp", 80);
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);

  webServerSetup();

  Serial.printf("HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);

  MQTT_setup();  


}

void loop() {
  WiFiConnect();
  httpServer.handleClient();
  MDNS.update();

  MQTT_connect();
  client.loop();

  handleContacts();
}

void WiFiConnect()
{
    if(!WiFi.isConnected()){
    Serial.println("WiFi connection failed.");
    WiFi.begin(ssid, password);
    while(WiFi.waitForConnectResult() != WL_CONNECTED){
      WiFi.begin(ssid, password);
      Serial.println("WiFi failed, retrying.");
    }
    String ipAddr = WiFi.localIP().toString();
    Serial.printf("IP address %s\n" , ipAddr.c_str());
    debugInfo = std::string("IP address ") + ipAddr.c_str();
  }
}


void readTempSensors()
{
  temperatureSensors.requestTemperatures();
  uint8_t deviceCount = temperatureSensors.getDeviceCount();
  for(int i=0; i<NUMBER_OF_TEMPERATURE_SENSORS && i<deviceCount; i++)
  {
    temperatures[i] = temperatureSensors.getTempCByIndex(i);
  }
}

void handleContacts()
{
  readContacts();
}

void readContacts()
{
 Wire.requestFrom(2, 1);    // request data from slave device #2
 uint8_t data[3] = {0,0,0};
 int byteIndex = 0;
 int bitIndex = 0;

 while(Wire.available())
 {
   if(byteIndex < 3)
   {
     data[byteIndex] = Wire.read(); // receive a byte as character        
   }
   byteIndex++;
 }

  int io = 0;
  for(io=0; io<NUMBER_OF_IO; io++)
  {
    byteIndex = io/8;
    int bitIndex = io % 8;
    if(data[byteIndex] & (1<< bitIndex))
    {
      lightsControl[io] = true;
    }
    else
    {
      lightsControl[io] = false;
    }
  }
}

void webServerSetup()
{
  httpServer.on("/stat", [](){

    std::string debugStr = std::string("Ligth status for ") + deviceName + std::string("\n");
    debugStr += "\n";
    debugStr += "Debug\n";
    
    for(int i=0; i<DEBUG_COUNT; i++)
    {
      String val = String(i) + String(" : ") + String(debug[i]) + String("\n"); 
      debugStr += val.c_str();
    }
    debugStr += "\n";

    httpServer.send(200, "text/plain", debugStr.c_str());
  });

  
  httpServer.begin();
}


void MQTT_setup()
{
  Serial.println("calling MQTT_setup");

  client.setServer(mqtt_server, 1883);
  client.setCallback(MQTT_callback);

  std::string topic = std::string("/house/light/") + std::string(deviceName);
  lightDebugTopic = topic + std::string("/debug");
  lightControlTopic = topic + std::string("/contact/");
  lightTopics.push_back(lightControlTopic + "light0");
  lightTopics.push_back(lightControlTopic + "light1");
  lightTopics.push_back(lightControlTopic + "light2");
  lightTopics.push_back(lightControlTopic + "light3");
  lightTopics.push_back(lightControlTopic + "light4");
  lightTopics.push_back(lightControlTopic + "light5");
  lightTopics.push_back(lightControlTopic + "light6");
  lightTopics.push_back(lightControlTopic + "light7");
  Serial.print(lightDebugTopic.c_str());
  Serial.println(" -- debug");
  Serial.print(lightControlTopic.c_str());
  Serial.println(" -- control");

}

void MQTT_connect()
{
  debug[0]++;
  if (!client.connected()) {
  debug[1]++;
    while (!client.connected()) {
  debug[2]++;
//      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect(deviceName)) {
  debug[3]++;
//        Serial.println("connected");
        // Once connected, publish an announcement...
        client.publish( lightDebugTopic.c_str(), debugInfo.c_str());

//        client.subscribe(lightControlTopic.c_str());

      } else {
  debug[4]++;
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
  }

}


void MQTT_callback(char* topic, byte* payload, unsigned int length) {
  std::string topicStr(topic);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
}
