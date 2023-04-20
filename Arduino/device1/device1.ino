#include <SoftwareSerial.h>

//SoftwareSerial XBee(1,0); // RX, TX
SoftwareSerial XBee(2,3); // RX, TX

#include "DHT.h"

#define DHTPIN 7
#define DHTTYPE DHT11

#define LEDPIN 8
#define ROOMLEDPIN 9

String DeviceID = "D01";
float pot = 0;

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
  float humi = dht.readHumidity();
  float new_pot = get_pot();

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
  Serial.println("Loop finished");
  digitalWrite(LEDPIN, LOW);
}

int receive_data(String MessageToConfirm)
{
//flag to confirm whether or not a confirmation of receipt has been received by the arduino
  int MessageConfirmed = 0;
//flag indicating that there might be multiple messages waiting to be processed
  int MultipleMessageFlag = 1;
  if (XBee.available()) // If data comes in from XBee, we process the data
  {
    String rawRead = XBee.readString(); // read the entirety of the available data from the XBee to a String for processing
    if (rawRead.length() > 3){ //check required to confirm that data is a reasonable length for processing, where data is less than three, for loop will effectively break.
      //iterate through the entirety of the received data, checking to see whether it might contain messaged destined for this device
      //if it's detected that there aren't multiple messages, will stop once there are no longer multiple messages to search
      for (int i = 0; i < rawRead.length() -3 && rawRead.length() > 3 && MultipleMessageFlag == 1; i++)
      {      
        //indicates a message has been found addressed to this device. More than this single message may have been buffered on serial port, so further interrogationr required
        if (rawRead.substring(i, i+3) == DeviceID)
        {
          //Declare a temporary variable to contain the data to be processed, not just the entire string
          String dataToProcess = "";  
          //Serial.println("Debug - " + rawRead);
          //An integer used to index the potential start of the next available message    
          int nextD = 0;
          //Skip past device ID and Data ID, and look for a 'D' indicating that more than one message has been buffered
          for (int k = i+7; k < rawRead.length(); k++)
          {
            //Serial.println("Looking for additional messages");
            //indicates that there is another message after this one
            if (rawRead[k] == 'D')
            {
              Serial.println("D found");
              nextD = k;
              break;
            }
            //\r follows a valid message
            // else if (rawRead[k] == '\r')
            // {
            //   Serial.println("Carriage return found");
            //   nextD = k;
            //   break;
            // }
            // //\n follows a \r, not expecting this to ever be triggered, but is here to catch in the event of a malformed message
            // else if (rawRead[k] == '\n')
            // {
            //   Serial.println("New line found");
            //   nextD = k-2;
            //   break;
            // }
            //Serial.print(k); //Debug
          }
          //indicates that there may have been multiple messaged buffered by the XBee
          //store the 'first message' in the temporary variable, then remove it from the bufferred string to avoid repeatedly processing
          if (nextD != 0)
          {
            dataToProcess = rawRead.substring(i,nextD-2);
            rawRead = rawRead.substring(nextD, rawRead.length());
            Serial.println("Data to Process " + dataToProcess);
            Serial.println("New raw read " + rawRead);
            i = -1;
          }
          //indicates that only one message was buffered, and as such no further processing is required.
          //for simplicity of later code, the string is read into the temporary variable so that only that variable
          //needs to be passed on to later functions
          else
          {
            dataToProcess = rawRead.substring(i,rawRead.length());
            if (dataToProcess[(dataToProcess.length())-2] == '\r')
            {
              dataToProcess = dataToProcess.substring(0,dataToProcess.length()-2);
            }
            MultipleMessageFlag = 0;
            Serial.println("Existing raw read " + rawRead);
            i = -1;
          }
          //the device has received a message confirming receipt of a previously sent data
          if (MessageToConfirm == dataToProcess)
          {
            Serial.println("Message confirmed");
            MessageToConfirm = "NONE";
            MessageConfirmed = 1;
          }
          else 
          {
            Serial.println("Processing data");
            process_data(dataToProcess);
          }

        }
        else {
          //Serial.println("Debugging rawRead update " +  rawRead.substring(i, i+3));
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
 
  int MsgConfirmation = 0;
  for (int i = 0; i < 5 &&  !MsgConfirmation; i++)
  {
    XBee.println(Message);
    int timeOutAttempts = 100;
    Serial.println("Message Sent - " + Message);
    while (!MsgConfirmation && timeOutAttempts>0)
    {
      Serial.println("*******************************************************" + Message);
      MsgConfirmation = receive_data(Message);
      timeOutAttempts--;
      delay(200);
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
    Serial.println(Data.substring(3,8) + "From " + Data +"  did not match a condition");
  }
}

float get_pot(){
  float rawPot = analogRead(A1);
  float reducedPot = map(rawPot, 0,1023,15,35);
  return reducedPot;
}
