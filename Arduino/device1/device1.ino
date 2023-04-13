#include <SoftwareSerial.h>

//SoftwareSerial XBee(1,0); // RX, TX
SoftwareSerial XBee(2,3); // RX, TX

#include "DHT.h"

#define DHTPIN 7
#define DHTTYPE DHT11

#define LEDPIN 8

String DeviceID = "D01";

int send_data(String DevID, String DevDataType, String DevData);

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  XBee.begin(9600);
  Serial.begin(9600);
  pinMode(LEDPIN, OUTPUT);
  randomSeed(analogRead(5));
  dht.begin();
}

void loop() {
  float temp = dht.readTemperature(); // read temperature data
  // add Header
  String tempMsg = DeviceID + "TEMP"+ String(temp);
  // Send data to the coordinator
  if (XBee.available() )
    XBee.readString();
  XBee.println(tempMsg);
  delay(350);
  if (XBee.available()) // If data comes in from XBee, we process the data
  {
    String rawRead = XBee.readString(); // read message from XCTU
    String header = rawRead.substring(0,3);
    Serial.println(header == DeviceID);
    int nextD = 0;
    for (int i = 0; i < rawRead.length() -3; i++)
    {      
      if (String(String(rawRead[i]) + String(rawRead[i+1]) + String(rawRead[i+2])) == header)
      {      
        Serial.println("Found message");
        if (rawRead[i+7] == 'H')
        {
          digitalWrite(LEDPIN ,HIGH);
        }
        if (rawRead[i+7] == 'L')
        {
          digitalWrite(LEDPIN ,LOW);
        }
      }
    }
  }
delay(1000);  
}
