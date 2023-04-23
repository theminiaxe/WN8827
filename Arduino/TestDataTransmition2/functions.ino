int receive_data(String MessageToConfirm) {
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
        SerialPrint(rawRead, DEBUG);
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
            SerialPrint("Looking for additional messages", DEBUG);
            //indicates that there is another message after this one
            if (rawRead[k] == 'D')
            {
              SerialPrint("D found", DEBUG);
              nextD = k;
              break;
            }
          }
          //indicates that there may have been multiple messaged buffered by the XBee
          //store the 'first message' in the temporary variable, then remove it from the bufferred string to avoid repeatedly processing
          if (nextD != 0)
          {
            dataToProcess = rawRead.substring(i,nextD);
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
            SerialPrint("Existing raw read " + rawRead,DEBUG);
            i = -1;
          }
          //the device has received a message confirming receipt of a previously sent data
          if (MessageToConfirm == dataToProcess)
          {
            SerialPrint("Message confirmed", DEBUG);
            MessageToConfirm = "NONE";
            MessageConfirmed = 1;
          }
          else 
          {
            SerialPrint("Processing data", DEBUG);
            process_data(dataToProcess);
          }

        }
        else {
          SerialPrint("Debugging rawRead update " +  rawRead.substring(i, i+3), DEBUG);
          SerialPrint(rawRead.length(), DEBUG);
        }
      }
    }
  }
  return MessageConfirmed;
}

int send_data(String DevID, String DevDataType, String DevData) {
  //do some rudimentary checking to ensure input parameters conform to messaging standard
  if (DevID.length() != 3) {
    SerialPrint("DevID Contains too many characters", DEBUG);
    return 3;
  }
  else if (DevID[0] != 'D')
  if (DevDataType.length() != 4) {
    SerialPrint("Error in DevDataType, invalid length", DEBUG);
    return 3;
  }
  for (int i = 1; i <= 2; i++) {
    if ((DevID[i] < '0') || (DevID[i] > '9')) {
      SerialPrint("DevID Contains invalid characters", DEBUG);
      return 3;
      //valid DevID
    }
  }
  for (int i = 0; i <= 3; i++) {
    if (((DevDataType[i] < 'A') || (DevDataType[i] > 'z')) || ((DevDataType[i] > 'Z') && (DevDataType[i] < 'a')))
    {
      SerialPrint("Error in DevDataType, invalid character", DEBUG);
      SerialPrint(DevDataType[i], DEBUG);
      return 3;
    }
  }
  for (int i = 0; i < DevData.length(); i++)
  {
    if ((DevData[i] < '0' || DevData[i] > '9') && (DevData[i] != '.')) {
      SerialPrint("DevData contains invalid characters", DEBUG);
      SerialPrint(DevData[i], DEBUG);
      return 3;
    }
  }
  String Message = DevID + DevDataType + DevData;
 
  int MsgConfirmation = 0;
  for (int i = 0; i < 5 &&  !MsgConfirmation; i++)
  {
    XBee.println(Message);
    int timeOutAttempts = 100;
    SerialPrint("Message Sent - " + Message, DEBUG);
    while (!MsgConfirmation && timeOutAttempts>0)
    {
      SerialPrint("*******************************************************" + Message, DEBUG);
      MsgConfirmation = receive_data(Message);
      timeOutAttempts--;
      delay(200);
      //Do nothing, we want confirmation that the previous message has been received, and don't want to perform any action until that time.
    }
  }
  if (!MsgConfirmation)
  {
    SerialPrint("No confirmation of transmission was received", DEBUG);
  }
  return 1;
}

void process_data(String Data) {
  if (Data.substring(3,7) == "TEMP")
  {
    SerialPrint("Processing data related to temp", DEBUG);
    if (Data[7] == 'H')
    {
      SerialPrint("H - turning on", DEBUG);
      digitalWrite(LEDPIN, HIGH);
    }
    else if (Data[7] == "L")
    {
      SerialPrint("L - turning off", DEBUG);
      digitalWrite(LEDPIN, LOW);
    }
  }
  else if (Data.substring(3,7) == "ROOM")
  {
    SerialPrint("Processing data related to room", DEBUG);
    if (Data[7] == '1')
    {
      SerialPrint("Room occupied, turning LED on", DEBUG);
      digitalWrite(ROOMLEDPIN, HIGH);
    }
    else if (Data[7] == '0')
    {
      SerialPrint("Room not occupied - turning LED off", DEBUG);
      digitalWrite(ROOMLEDPIN, LOW);
    }
  }  
  else
  {
    SerialPrint((Data.substring(3,8) + "From " + Data +"  did not match a condition ------------------------------------------------------------------------------------------------------------------------"), DEBUG);
  }
}

float get_pot(int analogPin){
  float rawPot = analogRead(analogPin);
  float reducedPot = map(rawPot, 0,1023,15,35);
  return reducedPot;
}

void SerialPrint(String toPrint, int Debug) {
  if (Debug) {
    Serial.println(toPrint);
  }
}

void SerialPrint(int toPrint, int Debug) {
  if (Debug) {
    Serial.println(toPrint);
  }
}

void SerialPrint(unsigned int toPrint, int Debug) {
  if (Debug) {
    Serial.println(toPrint);
  }
}

void SerialPrint(float toPrint, int Debug) {
  if (Debug) {
    Serial.println(toPrint);
  }
}