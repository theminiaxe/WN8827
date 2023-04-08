#include <SoftwareSerial.h>

//SoftwareSerial XBee(1,0); // RX, TX
SoftwareSerial XBee(2,3); // RX, TX

int send_data(String DevID, String DevDataType, String DevData);

void setup() {
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
  send_data("D03", "LIGH", String(avgSensorValue));
  delay(100);
  //mimic sending temperature as a second device - don't have another thermistor so will just send const value
  send_data("D03", "TEMP", "22.0");
  delay(random(10000,60000));
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
  if (XBee.available())
    XBee.readString();
  XBee.println(Message);
  delay(150);
  for (int i = 0; i < 10; i++) {
    if (XBee.available()) {
      String XBeeConfirmation = XBee.readString();
      Serial.println(XBeeConfirmation.length()); 
      Serial.println(XBeeConfirmation);  
      if (XBeeConfirmation == Message)
      {
        //Serial.println("Confirmed");
        return 0;
      }
    }
    Serial.print("No message available in response to - ");
    Serial.println(Message);
        //Serial.println(Message);
        delay(random(150,500));
        XBee.readString();
        XBee.println(Message);
        delay(150);
  }
  Serial.println("No confirmation of transmission was received");
  return 1;
}