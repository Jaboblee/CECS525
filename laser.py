import RPi.GPIO as gpio
import binascii
import serial
import sys
import time

frequency = 1  #Hz, baud
d_bits = 8      #data bits
p_bits = 1      #parity bits
s_bits = 2      #stop bits

p = 1 / frequency
teststring = 'abcdefghijklmnopqrstuvwxyz'
tx = 11
rx = 13

inputbyte = '00000000'
rxing = 0
last_tx = 0
bits = 0
byte_rxd = 0

def periods(now, prev):
        return int((now - prev) % p)

def timecheck(channel):
        time.sleep(.01)
        global last_tx
        global rxing
        global bits
        global inputbyte
        global byte_rxd
        prev = last_tx
        now = time.time()
        last_tx = now
        if (gpio.input(channel) == 1):
                print('Rising')
                if (rxing == 0):
                        rxing = 1
                        return
                else:
                        if (bits == 9):
                                bits = 0
                                rxing = 0
                                byte_rxd = 1
                                return
                        else:
                                bits_since=periods(now,prev)
                                
                                for i in range(bits,bits+bits_since):
                                        inputbyte[i] = 0
                                bits += bits_since
                                #if bits > 9
                                
        else:
                print('Falling')
	#time

def lase_c(char, channel):
        char = bin(int(binascii.hexlify(char),16))
        print(char)
        sendstart(channel)
        for bit in char[1:]:
                print(bit)
                
        for bit in reversed(char[1:]):  #Not Functioning
                print('1',end='')
                if (bit == 1):
                        gpio.output(channel,gpio.LOW)
                else:
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
        gpio.output(tx,gpio.LOW)

#serIn = serial.Serial('/dev/ttyAMA0', baudrate=300, timeout=1)

setup()
lase_s('a',tx)
print(readbyte())
print()
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

