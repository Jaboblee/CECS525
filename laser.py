import RPi.GPIO as gpio
import serial
import sys
import time

frequency = 10 #Hz
p = 1 / frequency
teststring = 'abcdefghijklmnopqrstuvwxyz'
tx = 11
rx = 13

def timecheck(channel):
        if (gpio.input(channel) == 0):
                print('Falling')
        else:
                print('Rising')
	#time

def lase(char, channel):
        sendstart(channel)
        
        sendstop(channel)

def sendstart(channel):
        gpio.output(tx,gpio.LOW)
        time.sleep(p)

def sendstop(channel):
        gpio.output(tx,gpio.LOW)
        time.sleep(p)

def setup():
        gpio.setmode(gpio.BOARD)
        gpio.setup(tx, gpio.OUT)
        gpio.setup(rx, gpio.IN)
        gpio.add_event_detect(rx, gpio.BOTH, callback=timecheck, bouncetime=40)
        gpio.output(tx,gpio.LOW)

#serIn = serial.Serial('/dev/ttyAMA0', baudrate=300, timeout=1)

setup()
try:
	while (1):
		print(gpio.input(rx))
		if (gpio.input(tx) == gpio.HIGH):
			gpio.output(tx,gpio.LOW)
		else: gpio.output(tx, gpio.HIGH)
		time.sleep(.5)
#                for i in teststring:
#                        serIn.write(i)
#                        sys.stdout.write(serIn.read(1).encode('hex'))
#                        sys.stdout.flush()
#                        print(serIn.read(1).encode('hex'), end='')
		
except KeyboardInterrupt:
	gpio.output(11,gpio.HIGH)
	gpio.cleanup()

