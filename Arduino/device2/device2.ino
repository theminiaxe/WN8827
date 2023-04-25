#include <SoftwareSerial.h>

//SoftwareSerial XBee(1,0); // RX, TX
SoftwareSerial XBee(2,3); // RX, TX

String DeviceID = "D02";
#define DEBUG 1

#define PIRPIN 7

#define LEDPIN 8
#define ROOMLEDPIN 9
#define POTPIN 14

int send_data(String DevID, String DevDataType, String DevData);
void process_data(String Data);
int receive_data(String MessageToConfirm="NONE");



void setup() {
  pinMode(PIRPIN, INPUT);
  XBee.begin(9600);
  Serial.begin(9600);
  randomSeed(analogRead(A5));
}

// the loop routine runs over and over again forever:
void loop() {
  int totalSensorValues = 0;
  int totalReadings = 100;
  for (int i = 0; i < totalReadings; i++) {
    totalSensorValues += analogRead(A0);
    delay(1);  // delay in between reads for stability
  }
  int avgSensorValue = totalSensorValues / totalReadings;
  
  send_data(DeviceID, "LIGH", String(avgSensorValue));
  //mimic sending temperature as a second device - don't have another thermistor so will just send const value
  send_data(DeviceID, "TEMP", "22.0");
  int pirVal = digitalRead(PIRPIN);
  send_data(DeviceID, "ROOM", String(pirVal));
    
    SerialPrint("****************************", DEBUG);
    float sentTime = millis();
  while (millis() - sentTime < 10000)
  {
    receive_data("NONE");
  }
  SerialPrint("****************************", DEBUG);
}