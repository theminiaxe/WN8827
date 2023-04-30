#include <SoftwareSerial.h>
#include "DHT.h"

//define a time between messages in milliseconds
#define TBM 30000
//string to be used as a unique identifier for messages to/from this device
String DeviceID = "D02";
//Define a flag to be used to indicate whether code is being debugged this will be used largely to determine whether or not to print data to serial
#define DEBUG 1

//these need to be changed depending on the shield being used 
//SoftwareSerial XBee(1,0); // RX, TX
SoftwareSerial XBee(2,3); // RX, TX

#define PIRPIN 7

//pins used to simulate the output of an airconditioner - not necessarily used in every sketch but required for compilation due to implementation of function file, not ideal but will do in this instance
#define LEDPINRED 16 //current temp is lower than desired - heating
#define LEDPINGREEN 17 //airconditioner is off, desired temp reached or house locked
#define LEDPINBLUE 18 //current temp is higher than desired - cooling
//pins used to simulate control of an airconditioner
#define POTPIN 14 //reads from potentiometer to mimic a temperature controlled dial with values mapped from 15 - 35
float pot = 0; //float used to store ongoing value from potentiometer outside of each loop
//pin used to simulate a light in a room
#define ROOMLEDPIN 15
#define BUZZERPIN 19
int locked = 0;

#define PHOTOPIN 14

int send_data(String DevID, String DevDataType, String DevData);
void process_data(String Data);
int receive_data(String MessageToConfirm="NONE");

void setup() {
  pinMode(PIRPIN, INPUT);
  XBee.begin(9600);
  if (DEBUG) {
    Serial.begin(9600);
  }
}

// the loop routine runs over and over again forever:
void loop() {
  float totalSensorValues = 0; //store total of values read from photoresistor - used to calculate average
  float totalReadings = 100; //total readings to take from photoresistor before calculating average
  float avgSensorValue; //store the average sensor value and use that as our 'reading' of light
  for (int i = 0; i < totalReadings; i++) { //add totalReadings number of values from photoresistor with 1ms delay in between each reading
    totalSensorValues += analogRead(PHOTOPIN);
    delay(1);  // delay in between reads for stability
  }
  avgSensorValue = totalSensorValues / totalReadings;
  
  send_data(DeviceID, "LIGH", String(avgSensorValue)); //send the value of light read to the coordinator

  int pirVal = digitalRead(PIRPIN); //read from the PIR sensor, 1 indicates motion, 0 indicates no motion
  send_data(DeviceID, "ROOM", String(pirVal)); //send the value from the PIIR sensor to the coordinator

  float sentTime = millis(); //set a timestamp from which we can identify when a specific time period has passed, at which point loop can end and data can be resent
  //continue to look for data coming in on the XBee for TBM/1000 seconds before reading in more data and sending it to the controller
  while (millis() - sentTime < TBM)
  {
    receive_data("NONE");
  }
  SerialPrint("Loop finished", DEBUG);
}