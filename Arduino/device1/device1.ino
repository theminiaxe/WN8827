void setup() {
  // put your setup code here, to run once:

}

void loop() {
  float temp = dht.readTemperature(); // read temperature data
  // add Header
  tempMsg = "TEMP"+temp
  // Send data to the coordinator
  XBee.print(tempMsg);
  
  if (XBee.available()) // If data comes in from XBee, we process the data
{
  rawRead = XBee.read(); // read message from XCTU
  header = rawRead.substrig(0,3) // obtain the message header
  if (header = "LEDP")
  {
    if (rawRead.charAt(4) = "H")
    {
      digitalWrite( //pin1// ,HIGH)
      digitalWrite( //pin2// ,HIGH)
    }
        if (rawRead.charAt(4) = "L")
        {
      digitalWrite( //pin1// ,LOW)
      digitalWrite( //pin2// ,LOW)
    }
  }
}
