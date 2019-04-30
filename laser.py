import RPi.GPIO as gpio
import binascii
import serial
import sys
import time
import threading

class periodic(threading.Thread):
	def __init__(self,time,hfunction,pauseEvent,startEvent):
		threading.Thread.__init__(self)
		self.time = time
		self.hfunction = hfunction
		self.stopped = pauseEvent
		self.enabled = startEvent
	
	def run(self):
		while self.enabled.wait():
			while not self.stopped.wait(self.time):
				self.hfunction()		     
class laser():
        def __init__(self,frequency,databits,paritybits,stopbits,tx,rx):
                self.frequency = frequency
                self.p = 1/frequency
                self.d_bits = databits
                self.p_bits = paritybits
                self.s_bits = stopbits
                self.tx = tx
                self.rx = rx
                self.inputbuff = []
                self.outputbuff = []
                self.inputbyte = ''
                self.t = 0
                self.rxing = 0
                self.bits = 0
                self.numOnes = 0
                gpio.setmode(gpio.BOARD)
                gpio.setup(self.tx, gpio.OUT)
                gpio.setup(self.rx, gpio.IN)
                gpio.add_event_detect(self.rx, gpio.RISING, callback=self.interrupt, bouncetime=10)
                gpio.output(tx,gpio.HIGH)
                self.pauseFlag = threading.Event()
                self.enableFlag = threading.Event()
                self.enableFlag.set()
                self.pauseFlag.set()
                self.t = periodic(self.p, self.uart_read, self.pauseFlag, self.enableFlag)
                self.t.start()

        def __del__(self):
                self.pauseFlag.set()
                self.enableFlag.clear()
                self.t._stop()
                gpio.output(self.tx,gpio.HIGH)
                gpio.cleanup() 

        def interrupt(self,channel):
                mode = gpio.input(channel)
                if (self.rxing == 0 and mode == 1):
                        #print('Start')
                        self.rxing = 1
                        self.bits = 0
                        self.inputbyte = ''
                        self.numOnes = 0
                        time.sleep(self.p/2)
                        self.pauseFlag.clear()

        def uart_read(self):
                #print('B'+inputbyte)
                if (self.bits < 10):
                        self.bits += 1
                        bit = gpio.input(self.rx)
                        if (bit == 1):  self.numOnes += 1
                        self.inputbyte += str(bit)
                else:
                        #print(inputbyte)
                        if (self.numOnes % 2 == 0):
                                if (self.inputbyte != ''):
                                        self.inputbyte = chr(int(self.inputbyte[7::-1],2))
                                        self.inputbuff.append(self.inputbyte)
                        else:
                                print('Data Corrupted')
                        self.pauseFlag.set()
                        self.rxing = 0	

        def lase_c(self,char):
                char = bin(int(binascii.hexlify(char),16))[2:]

                self.sendstart()
                        
                numOnes = 0
                bitcount = 0
                for bit in reversed(char):
                        bitcount += 1
                        if (bit == str(1)):
                                numOnes += 1
                                gpio.output(self.tx,gpio.LOW)
                        elif (bit == str(0)):
                                gpio.output(self.tx,gpio.HIGH)
                        time.sleep(self.p)
                
                for i in range(bitcount,8):
                        gpio.output(self.tx,gpio.HIGH)
                        time.sleep(self.p)

                if (numOnes % 2 != 0):
                        gpio.output(self.tx,gpio.LOW)
                else:
                        gpio.output(self.tx,gpio.HIGH)
                time.sleep(self.p)
                
                self.sendstop()

        def lase_s(self,strng):
                for c in strng:
                        char = bytes(c,'UTF-8')
                        self.lase_c(char)

        def sendstart(self):
                gpio.output(self.tx,gpio.LOW)
                time.sleep(self.p)
                return

        def sendstop(self):
                gpio.output(self.tx,gpio.HIGH)
                time.sleep(self.p*self.s_bits)
                return
        def close(self):
                self.__del__()



