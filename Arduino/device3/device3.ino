#include <SoftwareSerial.h>
#include <Keypad.h>
//SoftwareSerial XBee(1,0); // RX, TX
SoftwareSerial XBee(2,3); // RX, TX

#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11

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

#define LEDPIN 16
#define ROOMLEDPIN 15
#define POTPIN 14
//Define a flag to be used to indicate whether code is being debugged
//this will be used largely to determine whether or not to print data
//to serial
#define DEBUG 1

String DeviceID = "D03";

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

void setup(){
  XBee.begin(9600);
  if (DEBUG) {
    Serial.begin(9600);
  }
  pinMode(LEDPIN, OUTPUT);
  dht.begin();
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
          delay(10000);
        }
        else if (input == UnlockPin) {
          Serial.println("Unlocking");
          XBee.readString();
          send_data(DeviceID, "KEYP", String("0"));
          delay(10000);
        }
        else {
          Serial.println("Invalid Pin");
          delay(10000);
        }
        input = "";
      }
    }
    else {
      input = "";
    }
  }
}
