import pyrebase
from firebase import firebase
import serial
import time
from datetime import datetime
import re

from twilio.rest import Client as messagealert
account_sid = "ACa4986226cc489195135524c5f0d40f1a" #replace with your account_sid
auth_token = "ceba2cf279e44e7b3ea3f975e1c24e18" #replace with your auth token
twilioclient = messagealert(account_sid, auth_token)

ser = serial.Serial('/dev/ttyUSB0', 9600) # configure Xbee port

config = {
'apiKey': "AIzaSyC6MlFy4Btor9bQ4a_RHh1fJhuefeWwjAo",
'authDomain': "rpitest2-263a3.firebaseapp.com",
'databaseURL': "https://rpitest2-263a3-default-rtdb.firebaseio.com/",
'projectId': "rpitest2-263a3",
'storageBucket': "rpitest2-263a3.appspot.com",
'messagingSenderId': "712072402216"
}

#Initialise firebase
firebase = pyrebase.initialize_app(config)
#Get a reference to the database service
db = firebase.database()

def update_firebase_temperature(temperature, DevID, current_time, label):
        if temperature is not None:
                data = {"label":current_time, "value":temperature}
                results = db.child("Temperature").child(DevID).child(label).set(data)
        else:
                print('Failed to get reading. Try Again!')

def update_firebase_temperature_control(temperature, DevID, current_time, label):
        if temperature is not None:
                data = {"label":current_time, "value":temperature}
                results = db.child("TemperatureControl").child(DevID).child(label).set(data)
        else:
                print('Failed to get reading. Try Again!')

def update_firebase_humidity(humidity, DevID, current_time, label):
        if humidity is not None:
                data = {"label":current_time, "value":humidity}
                results = db.child("Humidity").child(DevID).child(label).set(data)
        else:
                print('Failed to get reading. Try Again!')

def update_firebase_light(light, DevID, current_time, label):
        if light is not None:
                data = {"label":current_time, "value":light}
                results = db.child("Light").child(DevID).child(label).set(data)
        else:
                print('Failed to get reading. Try Again!')

def update_firebase_room_occupancy(room_occupancy, DevID, current_time, label):
        if room_occupancy is not None:
                data = {"label":current_time, "value":room_occupancy}
                results = db.child("Room_Occupancy").child(DevID).child(label).set(data)
        else:
                print('Failed to get reading. Try Again!')

def update_firebase_keypad_lock_status(keypad_lock_status, DevID, current_time, label):
        if keypad_lock_status is not None:
                data = {"label":current_time, "value":keypad_lock_status}
                results = db.child("Lock_Status").child(DevID).child(label).set(data)
        else:
                print('Failed to get reading. Try Again!')

# PSEUDOCODE
#  -  Read input from XBEE using the pyserial library, one byte at a time to enable easier input validation
#  -  validate the input based on the expected sensors and format of messages being received
#  -  If valid
#  -    Broadcast received message as means of validating receipt (not pretty, but should do the trick)
#  -    return Device ID, Device Data Type (e,g, Temp, light, etc.) and Device Data Value
#  -  If Invalid
#  -    return False
# Assumptions
#  - Messages transmitted by devices on the network will be of the form \d{3}[A-Z]{4}[0-9.]+ i.e. 3 digits (device id), 4 characters (device data type), at least one integer or float value (device data)
# Example Output (Console) - Successful reception of valid message
#       <re.Match object; span=(0, 3), match='002'>
#       <re.Match object; span=(3, 7), match='HUMI'>
#       Device ID:  002
#       Device Data Type:  HUMI
#       Device Data:  46.00
#       12
#       Confirmation sent 2023-04-08 09:44:26.236539
# Example Output (Return Value) - Successful reception of valid message
#       Message = ('001', 'TEMP', '34.58')
def read_XBEE():
        Data = ""
        ValidMessage = True
        # SensorData = ser.read()
        while ValidMessage:
                c_char = ''
                for c in ser.read(1):
                        c_char = chr(c)
                if c_char == "\n" or c_char == "\r":
                        break
                if (len(Data) == 0 and c_char != "D"):
                        return False;
                if (len(Data) <= 2 and len(Data) >=1 and (c_char < "0" or c_char > "9")):
                        return False
                Data += c_char
                if len(Data) == 7:
                        DevID = re.search("^D\d{2}", Data)
                        DevDataType = re.search("(?<=^D\d{2})[A-Z]{4}", Data)
                        if (DevID is None or DevDataType is None):
                                #print("DEBUG - *****************************************************")
                                return False
                        #       #Regex matches
        DevID = re.search("^D\d{2}", Data)
        #print("Debug - ",DevID)
        DevDataType = re.search("(?<=^D\d{2})[A-Z]{4}", Data)
        #print("Debug - ",DevDataType)
        DevData = re.search("(?<=^D\d{2}[A-Z]{4})([0-9.]+)$", Data)
        #(?<=^b'\d{3}[A-Z]{4})([0-9.]+)(?=\\\\r\\\\n'$)
        if DevID and DevDataType and DevData:
                #print("Debug - Device ID: ", DevID.group(0))
                #print("Debug - Device Data Type: ", DevDataType.group(0))
                #print("Debug - Device Data: ", DevData.group(0))
                string_encode = (DevID.group(0) + DevDataType.group(0) + DevData.group(0)).encode('utf-8')

                print(ser.write(string_encode), " - ", datetime.now()) #send data to Arduino
                #print("Debug - Confirmation sent", datetime.now())
                return DevID.group(0), DevDataType.group(0), DevData.group(0)
        else:
                ValidMessage = False
        return False

        # Following reads input from serial one line at a time, but can be more prone to being unable to validate message when more then one message is received
        # in quick succession, where one of the messages does not terminate with a line feed (i.e. the two messages are concatenated)
        # can probably be removed at some point, was just part of the first draft
        # ser.flushInput()
        # #SensorData is read in as bytes, convert to string for easier processing with regex
        # if SensorData != None:
        #       Data = str(SensorData)
        #       print(Data)
        #       #When converted to string, message is of the form b'message\r\n' (when sent with Xbee.println)
        #       #Regex matches
        #       DevID = re.search("(?<=^b')\d{3}", Data)
        #       print(DevID)
        #       DevDataType = re.search("(?<=^b'\d{3})[A-Z]{4}", Data)
        #       print(DevDataType)
        #       DevData = re.search("(?<=^b'\d{3}[A-Z]{4})([0-9.]+)(?=\\\\r\\\\n'$)", Data)
        #       #(?<=^b'\d{3}[A-Z]{4})([0-9.]+)(?=\\\\r\\\\n'$)
        #       print(DevData)
        #       #Confirm that message was appropriately formatted, and that DevID, DevDataType and DevData were able to be extracted from the message
        #       if DevID and DevDataType and DevData:
        #               print("Device ID: ", DevID.group(0))
        #               print("Device Data Type: ", DevDataType.group(0))
        #               print("Device Data: ", DevData.group(0))
        #               #----- send a confirmation ------
        #               #string = "Data received"
        #               #string_encode = Data.encode() #encode to right format
        #               string_encode = (DevID.group(0) + DevDataType.group(0) + DevData.group(0)).encode('utf-8')
        #               print(ser.write(string_encode)) #send data to Arduino
        #               print("Confirmation sent", datetime.now())
        #               return DevID.group(0), DevDataType.group(0), DevData.group(0)
        #       else:
        #               return None
        # else:
        #       return None

DataTypes = ["TEMP", "HUMI", "LIGH", "ROOM", "TECO", "KEYP"]
SampleCounts = {}
for dt in DataTypes:
        SampleCounts[dt] = 0
MessageCache = {}

desiredTemp = 20
roomState = "0" #Default is empty - room unoccupied
lockState = "0" #Default is unlocked - i.e. people in the house

# Continuously read and print data
while True:
        try:
                Message = read_XBEE()
                if Message != False:
                        current_time = datetime.now()
                        current_time_string = current_time.strftime("%x,%X")
                        ProcessingRequired = False
                        if (Message[0] + Message[1] in MessageCache.keys()):
                                #Message containing data of same type from save device has been received previously

                                if (current_time - MessageCache[Message[0] + Message[1]]).total_seconds() > 10:
                                        ProcessingRequired = True
                        else:
                                print("Debug - not in message cache", Message)
                                ProcessingRequired = True
                        MessageCache[Message[0] + Message[1]] = current_time
                        print(MessageCache[Message[0] + Message[1]])
                        print(Message)
                        if ProcessingRequired:
                                if Message[1] == "TEMP":
                                        print("Temperature case triggered")
                                        SampleCounts[Message[1]] += 1
                                        update_firebase_temperature(Message[2], Message[0], current_time_string, SampleCounts[Message[1]])
                                        if float(Message[2]) < (desiredTemp -2)  and lockState == "0": #Indicates the house is unlocked, and temp needs to cool down
                                                ser.write((Message[0]+Message[1]+"H").encode())
                                                print((Message[0]+Message[1]+"H"))
                                        elif float(Message[2]) > (desiredTemp + 2) and lockState == "0":
                                                print(ser.write((Message[0]+Message[1]+"L").encode()))
                                        else:
                                                ser.write((Message[0]+Message[1]+"G").encode())

                                elif Message[1] == "HUMI":
                                        SampleCounts[Message[1]] += 1
                                        update_firebase_humidity(Message[2], Message[0], current_time_string, SampleCounts[Message[1]])
                                        print("Humidity case triggered")
                                elif Message[1] == "LIGH":
                                        print("Light case triggered")
                                        SampleCounts[Message[1]] += 1
                                        update_firebase_light(Message[2], Message[0], current_time_string, SampleCounts[Message[1]])
                                        if float(Message[2]) > 100: #indicates that there is enough light outside, and that the artificial light doesn't need to be turned on
                                                need_light = False
                                        else:
                                                need_light = True
                                elif Message[1] == "TECO":
                                        print("Temperature control case triggered")
                                        update_firebase_temperature_control(Message[2], Message[0], current_time_string, SampleCounts[Message[1]])
                                        desiredTemp = float(Message[2])
                                        SampleCounts[Message[1]] += 1
                                elif Message[1] == "ROOM":
                                        print("Room Occupancy case triggered")
                                        #double check to see if state has actually changed
                                        roomState = Message[2]
                                        if lockState == "0": #House unlocked, we expect people - turn on light
                                                if roomState == "0":#Room unoccupied turn off led to mimic turning off light
                                                        ser.write(("D01ROOM0").encode())
                                                elif roomState == "1" and need_light: #Turn on the led to mimic a light
                                                        ser.write(("D01ROOM1").encode())
                                        elif lockState == "1": #House locked - motion detected but we don't necessarily want to turn light on
                                                if roomState == "1":
                                                        print("Motion detected but house locked - doing nothing - notification will be sent and nothing further will be done") #Might like to look into sending push notifications to phone here
                                                        #alert_message =twilioclient.messages.create(
                                                        #        body="Alert! There is movement!",
                                                        #        from_="+16205089817", # Your trial phone number
                                                        #        to="+61415709665" #Your mobile phone number
                                                        #)
                                                        #print(alert_message.sid)
                                        SampleCounts[Message[1]] += 1
                                        update_firebase_room_occupancy(Message[2], Message[0], current_time_string, SampleCounts[Message[1]])
                                elif Message[1] == "KEYP":
                                        print("Lock status case triggered")
                                        if Message[2] == "1": #Lock pin has been input
                                                ser.write(("D01ROOM0").encode()) #Turn off led simulating lights
                                                ser.write(("D01TEMPG").encode()) #Turn on green led indicating air conditioner off
                                        lockState = Message[2]
                                        #don't need to send any messages, just update state and firebase
                                        update_firebase_keypad_lock_status(Message[2], Message[0], current_time_string, SampleCounts[Message[1]])


                                else:
                                        print("No case was triggered")

                        else:
                                print("Debug - Not processing the following as it is expected to be an unnecessary retransmit", Message)
                else:
                        print("Debug - Invalid input received and dropped")
        except KeyboardInterrupt:
                break

ser.close()
print("done")

