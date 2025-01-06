// Include necessary libraries
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "SparkFun_AS3935.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "credentials.h"  // Include credentials header

// Define AS3935 lightning sensor modes and interrupt types
#define INDOOR 0x12 
#define OUTDOOR 0xE
#define LIGHTNING_INT 0x08
#define DISTURBER_INT 0x04
#define NOISE_INT 0x01

// Initialize WiFi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int con = 0;

SparkFun_AS3935 lightning;

// Interrupt pin for lightning detection 
const int lightningInt = 32; 
int spiCS = 25; //SPI chip select pin

// This variable holds the number representing the lightning or non-lightning
// event issued by the lightning detector. 
int intVal = 0; //Interrupt
//int noise = 2; // Value between 1-7 
//int disturber = 2; // Value between 1-10

// Values for modifying the IC's settings. All of these values are set to their
// default values. 
byte noiseFloor = 2; // Value between 1-7 reduce noise default:2
byte watchDogVal = 7; // Value between 1-10 reeuce disturber default:2
byte spike = 2;   // 1-11 reject false positives by reducing the spike default:2
byte lightningThresh = 1;

void setup_wifi() {
  // WiFi connection setup
  delay(10);
  //connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);  // Using ssid from credentials.h

  WiFi.begin(ssid, password);  // Using credentials from credentials.h

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  // MQTT message callback handler
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

}

void reconnect() {
    // MQTT reconnection logic
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {  // Using MQTT credentials from credentials.h
        client.publish("esp32/as3935/status", String(con).c_str(), true);
        con++;
        if(con==10) con = 1; 
    } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
    } 
  }

void setup()
{
  // Initialize sensor and communication
  pinMode(lightningInt, INPUT); 

  // Start serial communication
  Serial.begin(9600); 
  Serial.println("AS3935 Franklin Lightning Detector"); 

  SPI.begin(); 

  if( !lightning.beginSPI(spiCS, 2000000) ){ 
    Serial.println ("Lightning Detector did not start up, freezing!"); 
    while(1); 
  }
  else
    Serial.println("Schmow-ZoW, Lightning Detector Ready!");

  // Configure lightning sensor settings
  lightning.setIndoorOutdoor(OUTDOOR);     // Set to outdoor mode
  lightning.maskDisturber(false);          // Don't mask disturbers

  int maskVal = lightning.readMaskDisturber();
  Serial.print("Are disturbers being masked: "); 
  if (maskVal == 1)
    Serial.println("YES"); 
  else if (maskVal == 0)
    Serial.println("NO"); 

  lightning.setNoiseLevel(noiseFloor);     // Set noise floor

  int noiseVal = lightning.readNoiseLevel();
  Serial.print("Noise Level is set at: ");
  Serial.println(noiseVal);

  // Watchdog threshold setting can be from 1-10, one being the lowest. Default setting is
  // two. If you need to check the setting, the corresponding function for
  // reading the function follows.    

  lightning.watchdogThreshold(watchDogVal); // Set watchdog

  int watchVal = lightning.readWatchdogThreshold();
  Serial.print("Watchdog Threshold is set to: ");
  Serial.println(watchVal);

  // Spike Rejection setting from 1-11, one being the lowest. Default setting is
  // two. If you need to check the setting, the corresponding function for
  // reading the function follows.    
  // The shape of the spike is analyzed during the chip's
  // validation routine. You can round this spike at the cost of sensitivity to
  // distant events. 

  lightning.spikeRejection(spike);         // Set spike rejection

  int spikeVal = lightning.readSpikeRejection();
  Serial.print("Spike Rejection is set to: ");
  Serial.println(spikeVal);

  // This setting will change when the lightning detector issues an interrupt.
  // For example you will only get an interrupt after five lightning strikes
  // instead of one. Default is one, and it takes settings of 1, 5, 9 and 16.   
  // Followed by its corresponding read function. Default is zero. 

  lightning.lightningThreshold(lightningThresh); // Set strike threshold

  uint8_t lightVal = lightning.readLightningThreshold();
  Serial.print("The number of strikes before interrupt is triggerd: "); 
  Serial.println(lightVal); 

  // Initialize WiFi and MQTT
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //lightning.resetSettings();
}

void loop()
{
  // Maintain MQTT connection
  if (!client.connected()) {
      reconnect();
  }
  //client.loop();

   // Check for lightning events
  if(digitalRead(lightningInt) == HIGH){
    intVal = lightning.readInterruptReg();
    
    // Handle different interrupt types
    if(intVal == NOISE_INT){
      // Handle noise detection
      Serial.println("Noise."); 
      // Too much noise? Uncomment the code below, a higher number means better
      // noise rejection.
      //lightning.setNoiseLevel(noise); 
    }
    else if(intVal == DISTURBER_INT){
      // Handle disturber detection
      Serial.println("Disturber."); 
      // Too many disturbers? Uncomment the code below, a higher number means better
      // disturber rejection.
      //lightning.watchdogThreshold(disturber);  
    }
    else if(intVal == LIGHTNING_INT){
      // Handle lightning strike
      Serial.println("Lightning Strike Detected!"); 
      // Lightning! Now how far away is it? Distance estimation takes into
      // account any previously seen events in the last 15 seconds. 
      byte distance = lightning.distanceToStorm(); 
      Serial.print("Approximately: "); 
      Serial.print(distance); 
      Serial.println("km away!"); 
      byte energy = lightning.lightningEnergy();
      Serial.print("Lightning Energy: "); 
      Serial.println(energy);
      
      // Publish lightning data to MQTT
      client.publish("esp32/as3935/lightningdistance", String(distance).c_str(), true);
      client.publish("esp32/as3935/lightningenergy", String(energy).c_str(), true);
      
    }
  }
  delay(100); // Prevent busy waiting
}
