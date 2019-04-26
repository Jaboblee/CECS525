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

inputbyte = ''
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
				     

def timecheck(channel):
	time.sleep(.01)
	mode = gpio.input(channel)
	global last_tx
	global rxing
	global bits
	global inputbyte
	global byte_rxd
	prev = last_tx
	now = time.time()
	last_tx = now
	if (mode == 1):
		print('Rising')
		if (rxing == 0):
			rxing = 1
			return
		else:
			if (bits == 9):
				bits = 0
				rxing = 0
				byte_rxd = 1
				checkParity(inputbyte)
				inputbyte = ''
				return
			else:
				bits_since=periods(now,prev)
				if bits + bits_since > 8:
					for i in range(0, 8 - bits):
						inputbyte += str(0)
					bits = 0
					rxing = 1
					byte_rxd = 1
					checkParity(inputbyte)
					inputbyte = ''
					return
				for i in range(bits,bits+bits_since):
					inputbyte += str(0)
				bits += bits_since
				
	else:
		print('Falling')
		if (bits == 9):
			bits = 0
			rxing = 0
			byte_rxd = 1
			checkParity(inputbyte)
			inputbyte = ''
			return
		else:
			bits_since=periods(now,prev)
			if bits + bits_since > 8:
				for i in range(0, 8 - bits):
					inputbyte += str(1)
				bits = 0
				rxing = 1
				byte_rxd = 1
				checkParity(inputbyte)
				inputbyte = ''
				return
			for i in range(bits,bits+bits_since):
				inputbyte += str(1)
			bits += bits_since
	#time

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
	gpio.add_event_detect(rx, gpio.BOTH, callback=timecheck, bouncetime=30)
	gpio.output(tx,gpio.HIGH)

#serIn = serial.Serial('/dev/ttyAMA0', baudrate=300, timeout=1)

try:
	setup()
	time.sleep(2)
	lase_s('abc',tx)
	print(readbyte())
	print()
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

