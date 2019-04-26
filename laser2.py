import RPi.GPIO as gpio
import binascii
import serial
import sys
import time
import threading

frequency = 1  #Hz, baud
d_bits = 8      #data bits
p_bits = 1      #parity bits
s_bits = 2      #stop bits

p = 1 / frequency
teststring = 'abcdefghijklmnopqrstuvwxyz'
tx = 11
rx = 13

inputbuff = []
inputbyte = ''
t = 0
rxing = 0
last_tx = 0
bits = 0
byte_rxd = 0

def periods(now, prev):
	global p
	return int((now - prev) / p)

def checkParity(byte):
	bytestr = ''
	numOnes = 0
	count = 0
	for c in byte:
		count += 1
		bytestr += str(c)
		if c == str(1):
			numOnes += 1
		if count == 8:
			if numOnes % 2 == 0:
				print('byte = ', bytestr)
				getAscii(bytestr)
			else:
				print('parity error')

def getAscii(byte):
	i = 8
	ascii = 0
	for c in byte:
		i -= 1
		if i != 0:
			if c == str(1):
				ascii += (2 ** i)
		else:
			print('character = ', chr(ascii))
				     

def rising(channel):
	mode = gpio.input(channel)
	global last_tx
	global rxing
	global bits
	global inputbyte
	global byte_rxd
	global p
	global t
	prev = last_tx
	now = time.time()
	last_tx = now
	print('Rising')
	if (rxing == 0):
		rxing = 1
		t = threading.Timer(p,uart_read())
		t.start()

def uart_read():
	global p
	global bits
	global rxing
	global inputbyte
	global inputbuff
	global rx
	print('b'+inputbyte)
	if (bits < 10):
		bits += 1
		inputbyte += str(gpio.input(rx))

		time.sleep(p)#threading.Timer(p,uart_read()).start
		uart_read()
	else:
		bits = 0
		print(inputbyte)
		inputbuff.append(inputbyte)
		rxing = 0
		
		

def lase_c(char, channel):
	char = bin(int(binascii.hexlify(char),16))[2:]
#	print(char)
	sendstart(channel)
#	for bit in char:
#		print(bit)

		
	numOnes = 0
	bitcount = 0
	for bit in reversed(char):
		bitcount += 1
		print(bit)
		if (bit == str(1)):
			numOnes += 1
			gpio.output(channel,gpio.LOW)
		elif (bit == str(0)):
			gpio.output(channel,gpio.HIGH)
		time.sleep(p)
	if (numOnes % 2 != 0):
		gpio.output(channel,gpio.LOW)
	else:
		gpio.output(channel,gpio.HIGH)
	time.sleep(p)

	for i in range(1,bitcount):
		gpio.output(channel,gpio.HIGH)
		time.sleep(p)
	sendstop(channel)

def lase_s(strng, channel):
	for c in strng:
		char = bytes(c,'UTF-8')
		lase_c(char, channel)
def readbyte():
	global inputbyte
	global byte_rxd
	while (byte_rxd != 1):
		pass
	byte_rxd = 0
	return inputbyte


def sendstart(channel):
	gpio.output(tx,gpio.LOW)
	time.sleep(p)
	return

def sendstop(channel):
	gpio.output(tx,gpio.HIGH)
	time.sleep(p*s_bits)
	return

def setup():
	gpio.setmode(gpio.BOARD)
	gpio.setup(tx, gpio.OUT)
	gpio.setup(rx, gpio.IN)
	gpio.add_event_detect(rx, gpio.RISING, callback=rising, bouncetime=10)
	gpio.output(tx,gpio.HIGH)

#serIn = serial.Serial('/dev/ttyAMA0', baudrate=300, timeout=1)

setup()

try:
	time.sleep(5)

	gpio.output(11,gpio.LOW)
	time.sleep(p)
	gpio.output(11,gpio.LOW)
	time.sleep(p)
	gpio.output(11,gpio.HIGH)
	time.sleep(p)
	gpio.output(11,gpio.HIGH)
	time.sleep(p)
	gpio.output(11,gpio.HIGH)
	time.sleep(p)
	gpio.output(11,gpio.HIGH)
	time.sleep(p)
	gpio.output(11,gpio.LOW)
	time.sleep(p)
	gpio.output(11,gpio.LOW)
	time.sleep(p)
	gpio.output(11,gpio.LOW)
	time.sleep(p)
	gpio.output(11,gpio.HIGH)
	time.sleep(p)
	gpio.output(11,gpio.HIGH)
	time.sleep(p)

	while (len(inputbuff) == 0):
		pass

	print(inputbuff[0])
	
	gpio.output(11,gpio.HIGH)
	gpio.cleanup()
#try:
#	while (1):
#		print(gpio.input(rx))
#		if (gpio.input(tx) == gpio.HIGH):
#			gpio.output(tx,gpio.LOW)
#		else: gpio.output(tx, gpio.HIGH)
#		time.sleep(.5)
#                for i in teststring:
#                        serIn.write(i)
#                        sys.stdout.write(serIn.read(1).encode('hex'))
#                        sys.stdout.flush()
#                        print(serIn.read(1).encode('hex'), end='')
		
except KeyboardInterrupt:
	gpio.output(11,gpio.HIGH)
	gpio.cleanup()

