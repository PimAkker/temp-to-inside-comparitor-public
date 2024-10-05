# Importing Libraries
import serial
import json
import time

arduino = serial.Serial(port='COM4', baudrate=115200, timeout=.1)
def read():
    time.sleep(0.05)
    data = arduino.readline()
    
    return data
while True:

    data = read().decode("utf-8") 
    if "coord" in data:
         data = json.loads(data)
         outsideTemp = data["main"]["temp"] -273
         outsideHum = data["main"]["humidity"]
         print("outside temperature: " + str(outsideTemp) + "\n" + "outside humidity: " + str(outsideHum))

    if "temperature" in data:
        data = json.loads(data)
        print("inside Temperature: " + str(data["temperature"]) +"\n" +"inside humidity: " + str(data["humidity"]))

   

    

