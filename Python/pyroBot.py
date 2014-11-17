import serial
import string
import re
from time import sleep
from string import whitespace


# Specify your serial port path (/dev/ttyAMA0 is default for Raspberry Pi)
serialport = serial.Serial("/dev/ttyAMA0", 9600, timeout=0)
serialport.flushInput()

steer = 0
speed = 0
left = 0
right = 0

def readData(data):
    dataItems = data.split(",")
    
    return dataItems


while True:
    # read data
    
    dataIn = serialport.read(30)
    data = readData(dataIn.strip())
    
    
    
    
    print "**** start ****"
    print data
    print len(data)
    print data[0]
    if len(data)>1:
        print data[1]
    print "***** end *****" 
    
    
    
    
    if len(data[0]) >0:
        speed = int(data[0])
     
    
    if len(data) >1:
        steer = int(data[1])
        
    if speed > 50:
        speed = 50
        
    if speed < -50:
        speed = -50
    
     
    if steer == 0:
        left = speed
        right = speed
    if steer > 0:
        left = speed/3
        right = speed

    if steer < 0:   
        left = speed
        right = speed/3
 
    print "left: " + str(left) + " right: " + str(right)
    
    
    dataOut = str(left) + "," + str(right) +"\n"
    
    
    
    print "Data out " + "|" + dataOut + "|"
    
    serialport.write(dataOut)
    sleep(0.125)
    
    

serial.close()



