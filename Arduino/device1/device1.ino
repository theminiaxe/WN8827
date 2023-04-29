#include <SoftwareSerial.h>
#include "DHT.h"

//define a time between messages in milliseconds
#define TBM 30000
//string to be used as a unique identifier for messages to/from this device
String DeviceID = "D01";
//Define a flag to be used to indicate whether code is being debugged this will be used largely to determine whether or not to print data to serial
#define DEBUG 1

//these need to be changed depending on the shield being used 
//SoftwareSerial XBee(1,0); // RX, TX
SoftwareSerial XBee(2,3); // RX, TX

//Configure DHT sensor
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//pins used to simulate the output of an airconditioner - not necessarily used in every sketch but required for compilation due to implementation of function file, not ideal but will do in this instance
#define LEDPINRED 16 //current temp is lower than desired - heating
#define LEDPINGREEN 17 //airconditioner is off, desired temp reached or house locked
#define LEDPINBLUE 18 //current temp is higher than desired - cooling
//pins used to simulate control of an airconditioner
#define POTPIN 14 //reads from potentiometer to mimic a temperature controlled dial with values mapped from 15 - 35
float pot = 0; //float used to store ongoing value from potentiometer outside of each loop
//pin used to simulate a light in a room
#define ROOMLEDPIN 15

int send_data(String DevID, String DevDataType, String DevData);
int receive_data(String MessageToConfirm="NONE");
float get_pot(int analogPin);
void process_data(String Data);
//Prints toPrint serial if Debug flag is set to 1, else do nothing
void SerialPrint(String toPrint, int Debug);
void SerialPrint(int toPrint, int Debug);
void SerialPrint(unsigned int toPrint, int Debug);
void SerialPrint(float toPrint, int Debug);

void setup() {
  XBee.begin(9600);
  //Having had issues with certain shields and simultaneous use of software serial and serial,
  //have used DEBUG to allow any serial references to be ignored when DEBUG is 0
  if (DEBUG) {
    Serial.begin(9600);
  }
  pinMode(LEDPINRED, OUTPUT);
  pinMode(LEDPINGREEN, OUTPUT);
  pinMode(LEDPINBLUE, OUTPUT);
  pinMode(ROOMLEDPIN, OUTPUT);
  dht.begin();
}

void loop() {
  float temp = dht.readTemperature(); // read temperature data
  float humi = dht.readHumidity(); //read humidity data
  float new_pot = get_pot(POTPIN); //get the current value from the potentiometer
  
  //Check to see whether the value read from the potentiometer has changed since last loop
  if (new_pot != pot) //if value has changed since last loop, send new value
  {
    pot = new_pot;
    send_data(DeviceID, "TECO", String(pot));
  }
  send_data(DeviceID, "TEMP", String(temp)); //Send temperature data to coordinator
  send_data(DeviceID, "HUMI", String(humi)); //Send humidity data to coordinator
  

  float sentTime = millis(); //set a timestamp from which we can identify when a specific time period has passed, at which point loop can end and data can be resent
  //continue to look for data coming in on the XBee for TBM/1000 seconds before reading in more data and sending it to the controller
  while (millis() - sentTime < TBM)
  {
    receive_data("NONE");
  }
  SerialPrint("Loop finished", DEBUG);
  //digitalWrite(LEDPIN, LOW);
}
