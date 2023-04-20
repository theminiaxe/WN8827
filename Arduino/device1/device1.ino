#include <SoftwareSerial.h>

//SoftwareSerial XBee(1,0); // RX, TX
SoftwareSerial XBee(2,3); // RX, TX

#include "DHT.h"

#define DHTPIN 7
#define DHTTYPE DHT11

#define LEDPIN 8
#define ROOMLEDPIN 9

String DeviceID = "D01";
int pot = 0;

int send_data(String DevID, String DevDataType, String DevData);
int receive_data(String MessageToConfirm="NONE");
float get_pot();
void process_data(String Data);

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
  float new_pot = get_pot();

  // add Header
  String tempMsg = DeviceID + "TEMP"+ String(temp);
  // Send data to the coordinator
  float sentTime = millis();
  send_data(DeviceID, "TEMP", String(temp));
  if (new_pot != pot)
  {
    pot = new_pot;
    send_data(DeviceID, "TECO", String(pot));
  }
  
  while (millis() - sentTime < 10000)
  {
    receive_data("NONE");
  }
  digitalWrite(LEDPIN, LOW);
}

int receive_data(String MessageToConfirm)
{
  int MessageConfirmed = 0;
  int MultipleMessageFlag = 1;
  if (XBee.available()) // If data comes in from XBee, we process the data
  {
    String rawRead = XBee.readString(); // read message from XCTU
    if (rawRead.length() > 3){
      for (int i = 0; i < rawRead.length() -3 && rawRead.length() > 3 && MultipleMessageFlag == 1; i++)
      {      
        //indicates a message has been found addressed to this device. More than this single message may have been buffered on serial port, so further interrogationr required
        if (rawRead.substring(i, i+3) == DeviceID)
        {
          String dataToProcess = "";  
          //Skip past device ID and Data ID, and look for a 'D' indicating that more than one message has been buffered    
          int nextD = 0;
          for (int k = i+7; k < rawRead.length(); k++)
          {
            if (rawRead[k] == 'D')
            {
              Serial.println("D found");
              nextD = k;
              break;
            }
            else if (rawRead[k] == '\r')
            {
              Serial.println("Carriage return found");
              nextD = k;
              break;
            }
            else if (rawRead[k] == '\n')
            {
              Serial.println("New line found");
              nextD = k-2;
              break;
            }
            Serial.print(k);
          }
          //indicates there may be a message, and that rawRead should be broken up
          if (nextD != 0)
          {
            dataToProcess = rawRead.substring(i,nextD);
            rawRead = rawRead.substring(nextD, rawRead.length());
            Serial.println("Data to Process " + dataToProcess);
            Serial.println("New raw read " + rawRead);
            i = 0;
          }
          else
          {
            dataToProcess = rawRead.substring(i,rawRead.length()-2);
            MultipleMessageFlag = 0;
            Serial.println("Existing raw read " + rawRead);
            i = 0;
          }
          if (MessageToConfirm == "NONE")
          {
            Serial.println("No message to confirm - data will instead be sent to a relevant processing function");
            process_data(dataToProcess);
          }
          else if (MessageToConfirm == dataToProcess)
          {
            Serial.println("Message confirmed");
            MessageToConfirm = "NONE";
            MessageConfirmed = 1;
          }
          else 
          {
            Serial.println("To compare" + MessageToConfirm + " with " + dataToProcess);
          }

        }
        else {
          //Serial.println(rawRead.substring(i, i+3));
          //Serial.println(rawRead.length());
        }
      }
    }
  }
  return MessageConfirmed;
}

int send_data(String DevID, String DevDataType, String DevData) {
  //do some rudimentary checking to ensure input parameters conform to messaging standard
  if (DevID.length() != 3) {
    //Serial.print("DevID Contains too many characters");
    return 3;
  }
  else if (DevID[0] != 'D')
  if (DevDataType.length() != 4) {
    //Serial.print("Error in DevDataType, invalid length");
    return 3;
  }
  for (int i = 1; i <= 2; i++) {
    if ((DevID[i] < '0') || (DevID[i] > '9')) {
      //Serial.print("DevID Contains invalid characters");
      return 3;
      //valid DevID
    }
  }
  for (int i = 0; i <= 3; i++) {
    if (((DevDataType[i] < 'A') || (DevDataType[i] > 'z')) || ((DevDataType[i] > 'Z') && (DevDataType[i] < 'a')))
    {
      //Serial.print("Error in DevDataType, invalid character");
      //Serial.println(DevDataType[i]);
      return 3;
    }
  }
  for (int i = 0; i < DevData.length(); i++)
  {
    if ((DevData[i] < '0' || DevData[i] > '9') && (DevData[i] != '.')) {
      //Serial.print("DevData contains invalid characters");
      //Serial.println(DevData[i]);
      return 3;
    }
  }
  String Message = DevID + DevDataType + DevData;
  int timeOutAttempts = 100;
  int MsgConfirmation = 0;
  for (int i = 0; i < 5 &&  !MsgConfirmation; i++)
  {
    XBee.println(Message);
    while (!MsgConfirmation && timeOutAttempts>0)
    {
      MsgConfirmation = receive_data(Message);
      timeOutAttempts--;
      delay(100);
      //Do nothing, we want confirmation that the previous message has been received, and don't want to perform any action until that time.
    }
  }
  if (!MsgConfirmation)
  {
    Serial.println("No confirmation of transmission was received");
  }
  return 1;
}

void process_data(String Data) {
  if (Data.substring(3,7) == "TEMP")
  {
    Serial.println("Processing data related to temp");
    if (Data[7] == 'H')
    {
      Serial.println("H - turning on");
      digitalWrite(LEDPIN, HIGH);
    }
    else if (Data[7] == "L")
    {
      Serial.println("L - turning off");
      digitalWrite(LEDPIN, LOW);
    }
  }
  else if (Data.substring(3,7) == "ROOM")
  {
    Serial.println("Processing data related to room");
    if (Data[7] == '1')
    {
      Serial.println("Room occupied, turning LED on");
      digitalWrite(ROOMLEDPIN, HIGH);
    }
    else if (Data[7] == '0')
    {
      Serial.println("Room not occupied - turning LED off");
      digitalWrite(ROOMLEDPIN, LOW);
    }
  }  
  else
  {
    Serial.println(Data.substring(3,7) + " did not match a condition");
  }
}

float get_pot(){
  float rawPot = analogRead(A1);
  float reducedPot = rawPot/50.0 + 15;
  return reducedPot;
}
