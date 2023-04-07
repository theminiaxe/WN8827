import pyrebase
from firebase import firebase
import serial
import time
from datetime import datetime
import re

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

def update_firebase_temperature(temperature, current_time, label):
        if temperature is not None:
                data = {"label":current_time, "value":temperature}
                results = db.child("Temperature").child(label).set(data)
                time.sleep(4)
        else:
                print('Failed to get reading. Try Again!')
                time.sleep(4)

def update_firebase_humidity(humidity, current_time, label):
        if humidity is not None:
                data = {"label":current_time, "value":humidity}
                results = db.child("Humidity").child(label).set(data)
                time.sleep(4)
        else:
                print('Failed to get reading. Try Again!')
                time.sleep(4)

def update_firebase_light(light, current_time, label):
        if light is not None:
                data = {"label":current_time, "value":light}
                results = db.child("Light").child(label).set(data)
                time.sleep(4)
        else:
                print('Failed to get reading. Try Again!')
                time.sleep(4)

def update_firebase_room_occupancy(room_occupancy, current_time, label):
        if room_occupancy is not None:
                data = {"label":current_time, "value":room_occupancy}
                results = db.child("Room_Occupancy").child(label).set(data)
                time.sleep(4)
        else:
                print('Failed to get reading. Try Again!')
                time.sleep(4)

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
#	<re.Match object; span=(0, 3), match='002'>
#	<re.Match object; span=(3, 7), match='HUMI'>
#	Device ID:  002
#	Device Data Type:  HUMI
#	Device Data:  46.00
#	12
#	Confirmation sent 2023-04-08 09:44:26.236539
# Example Output (Return Value) - Successful reception of valid message
#	Message = ('001', 'TEMP', '34.58')
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
		if (len(Data) <= 2 and (c_char < "0" or c_char > "9")):
			return False
		Data += c_char
		if len(Data) == 7:
			DevID = re.search("^\d{3}", Data)
			DevDataType = re.search("(?<=^\d{3})[A-Z]{4}", Data)
			if (DevID is None or DevDataType is None):
				print("*****************************************************")
				return False
			# 	#Regex matches
	DevID = re.search("^\d{3}", Data)
	print(DevID)
	DevDataType = re.search("(?<=^\d{3})[A-Z]{4}", Data)
	print(DevDataType) 
	DevData = re.search("(?<=^\d{3}[A-Z]{4})([0-9.]+)$", Data)
	#(?<=^b'\d{3}[A-Z]{4})([0-9.]+)(?=\\\\r\\\\n'$)
	if DevID and DevDataType and DevData:
		print("Device ID: ", DevID.group(0))
		print("Device Data Type: ", DevDataType.group(0))
		print("Device Data: ", DevData.group(0))
		string_encode = (DevID.group(0) + DevDataType.group(0) + DevData.group(0)).encode('utf-8')
		print(ser.write(string_encode)) #send data to Arduino
		print("Confirmation sent", datetime.now())
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
	# 	Data = str(SensorData)
	# 	print(Data)
	# 	#When converted to string, message is of the form b'message\r\n' (when sent with Xbee.println)
	# 	#Regex matches
	# 	DevID = re.search("(?<=^b')\d{3}", Data)
	# 	print(DevID)
	# 	DevDataType = re.search("(?<=^b'\d{3})[A-Z]{4}", Data)
	# 	print(DevDataType) 
	# 	DevData = re.search("(?<=^b'\d{3}[A-Z]{4})([0-9.]+)(?=\\\\r\\\\n'$)", Data)
	# 	#(?<=^b'\d{3}[A-Z]{4})([0-9.]+)(?=\\\\r\\\\n'$)
	# 	print(DevData)
	# 	#Confirm that message was appropriately formatted, and that DevID, DevDataType and DevData were able to be extracted from the message
	# 	if DevID and DevDataType and DevData:
	# 		print("Device ID: ", DevID.group(0))
	# 		print("Device Data Type: ", DevDataType.group(0))
	# 		print("Device Data: ", DevData.group(0))
	# 		#----- send a confirmation ------
	# 		#string = "Data received"
	# 		#string_encode = Data.encode() #encode to right format
	# 		string_encode = (DevID.group(0) + DevDataType.group(0) + DevData.group(0)).encode('utf-8')
	# 		print(ser.write(string_encode)) #send data to Arduino
	# 		print("Confirmation sent", datetime.now())
	# 		return DevID.group(0), DevDataType.group(0), DevData.group(0)
	# 	else:
	# 		return None
	# else:
	# 	return None


# Continuously read and print data
while True:
	try:

		Message = read_XBEE()
		if Message != False:
			print(Message)
		else:
			print("Invalid input received and dropped")
	except KeyboardInterrupt:
		break

ser.close()
print("done")