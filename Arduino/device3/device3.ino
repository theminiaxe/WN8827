#include <SoftwareSerial.h>
#include <Keypad.h>
#include "DHT.h"

//define a time between messages in milliseconds
#define TBM 30000
//string to be used as a unique identifier for messages to/from this device
String DeviceID = "D03";
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
#define BUZZERPIN 19

#define ROW_NUM 4 //four rows
#define COLUMN_NUM 4 //four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

//Pins 2 and 3 are used for XBee with the particular shields I have - pin 13 repeatedly causes triggering of input
byte pin_rows[ROW_NUM] = {12, 11, 10, 9}; //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {8, 7, 6, 5}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

//Pin can equal anything as long as 'C' is not included, and as long as pins are same length
String LockPin = "1234";
String UnlockPin = "4321";
String input = "";
int locked = 0;

int send_data(String DevID, String DevDataType, String DevData);
int receive_data(String MessageToConfirm="NONE");
float get_pot(int analogPin);
void process_data(String Data);
//Prints toPrint serial if Debug flag is set to 1, else done nothing
void SerialPrint(String toPrint, int Debug);
void SerialPrint(int toPrint, int Debug);
void SerialPrint(unsigned int toPrint, int Debug);
void SerialPrint(float toPrint, int Debug);

void setup(){
  XBee.begin(9600);
  if (DEBUG) {
    Serial.begin(9600);
  }
  pinMode(BUZZERPIN, OUTPUT);
}

void loop(){
  char key = keypad.getKey();
  if (key){
    if (key != 'C') {
      input += key;
      if (input.length() == LockPin.length()){
        if (input == LockPin) {
          Serial.println("Locking");
          XBee.readString();
          send_data(DeviceID, "KEYP", String("1"));
          locked = 1;
          delay(1000);
        }
        else if (input == UnlockPin) {
          Serial.println("Unlocking");
          XBee.readString();
          send_data(DeviceID, "KEYP", String("0"));
          locked = 0;
          noTone(BUZZERPIN);
          delay(1000);
        }
        else {
          Serial.println("Invalid Pin");
          delay(1000);
        }
        input = "";
      }
    }
    else {
      input = "";
    }
  }
  if (locked) {
    receive_data();
  }
}
