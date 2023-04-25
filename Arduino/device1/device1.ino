#include <SoftwareSerial.h>

//SoftwareSerial XBee(1,0); // RX, TX
SoftwareSerial XBee(2,3); // RX, TX

#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11


#define ROOMLEDPIN 16
#define LEDPIN 15
#define POTPIN 14
//Define a flag to be used to indicate whether code is being debugged
//this will be used largely to determine whether or not to print data
//to serial
#define DEBUG 1

String DeviceID = "D01";

float pot = 0;


int send_data(String DevID, String DevDataType, String DevData);
int receive_data(String MessageToConfirm="NONE");
float get_pot(int analogPin);
void process_data(String Data);
//Prints toPrint serial if Debug flag is set to 1, else done nothing
void SerialPrint(String toPrint, int Debug);
void SerialPrint(int toPrint, int Debug);
void SerialPrint(unsigned int toPrint, int Debug);
void SerialPrint(float toPrint, int Debug);

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  XBee.begin(9600);
  if (DEBUG) {
    Serial.begin(9600);
  }
  pinMode(LEDPIN, OUTPUT);
  pinMode(ROOMLEDPIN, OUTPUT);
  randomSeed(analogRead(5));
  dht.begin();
}

void loop() {
  float temp = dht.readTemperature(); // read temperature data
  float humi = dht.readHumidity();
  float new_pot = get_pot(POTPIN);

  // add Header
  String tempMsg = DeviceID + "TEMP"+ String(temp);
  // Send data to the coordinator
  
  send_data(DeviceID, "TEMP", String(temp));
  send_data(DeviceID, "HUMI", String(humi));
  if (new_pot != pot)
  {
    pot = new_pot;
    send_data(DeviceID, "TECO", String(pot));
  }
  float sentTime = millis();
  //continue to look for data coming in on the XBee for 10 seconds before reading in more data and sending it to the controller
  while (millis() - sentTime < 10000)
  {
    receive_data("NONE");
  }
  SerialPrint("Loop finished", DEBUG);
  digitalWrite(LEDPIN, LOW);
}
