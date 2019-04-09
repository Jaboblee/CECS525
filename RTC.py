# -*- coding: utf-8 -*-
import smbus
import sys
import time
import RPi.GPIO as gpio			#need to check python 2
bus = smbus.SMBus(0)
address = 0x68

ampm = -1	#-1=24hr mode, 0=AM, 1=PM
fc = 'F'	#'f'= Fahrenheit, 'c'=Celsius
year = 0	#millennium

def settime(addr,x=-1,h=-1,m=-1,s=-1):
        if (s > -1):
                write = 0
                write = write | (s % 10)
                write = write | (s / 10)<<4
                bus.write_byte_data(addr,0,write)
        if (m > -1):
                write = 0
                write = write | (m % 10)
                write = write | (m / 10)<<4
                bus.write_byte_data(addr,1,write)
        if (h > -1):
                write = 0
                if (x >= 0 and h < 13):          #12hr mode, AM:x=1, PM:x=2
                        write = write | 0b1000000
                        if (x==1): write = write | 0b100000
                        write = write | (h % 10)
                        write = write | (h / 10)<<4
                elif (x == -1):                  #24hr mode, x=0
                        write = write | (h % 10)
                        write = write | (h / 10)<<4
                bus.write_byte_data(addr,2,write)
        #bus.write_byte_data(addr,3,2)

def setdate(addr,d=-1,m=-1,y=-1):
	global year
	if (d > -1):
		write = 0
		write = write | (d % 10)
		write = write | (d / 10)<<4
		bus.write_byte_data(addr,4,write)
	if (m > -1):
		write = 0
		write = write | (m % 10)
		write = write | (m / 10)<<4
		bus.write_byte_data(addr,5,write)
	if (y > -1):
		year = (y / 1000)*10
		y = y - (year * 100)
		year += (y / 100)
		y = y % 100
		write = 0
		write = write | (y % 10)
		write = write | (y / 10)<<4
		bus.write_byte_data(addr,6,write)

def gettime(addr):
	time = []
	byte = bus.read_byte_data(addr,0)
	time.append((byte & 0xF) + ((byte & 0xF0)>>4)*10)       #seconds
	byte = bus.read_byte_data(addr,1)
	time.append((byte & 0xF) + ((byte & 0xF0)>>4)*10)       #minutes
	byte = bus.read_byte_data(addr,2)
	if (byte & 0b1000000 == 0b1000000):                     #hours
		time.append((byte & 0xF) + ((byte & 0b10000)>>4)*10)
		time.append((byte & 0b100000 == 0b100000))
	else:
		time.append((byte & 0xF) + ((byte & 0b110000)>>4)*10)
	return time

def gettime_s(addr,format='HH:MM:SS'):
	time = []
	h = m = s = 0
	ampm = -1
	for char in format:
		if (char == 'H'):
			h += 1
		elif (char == 'M'):
			m += 1
		elif (char == 'S'):
			s += 1
	if (h > 0):
		byte = bus.read_byte_data(addr,2)
		time.append('')
		if (byte & 0b1000000 == 0b1000000):                     #hours
			if (h == 2):
				time[-1] = str((byte & 0x10)>>4) + str(byte & 0xF)
			elif (h == 1):
				if ((byte & 0x10) != 0):
					time[-1] = str((byte & 0x10)>>4)
				time[-1] += str(byte & 0xF)
			ampm = (byte & 0b100000 == 0b100000)
		else:
			if (h == 2):
				time[-1] = str((byte & 0x30)>>4) + str(byte & 0xF)
			elif (h == 1):
				if ((byte & 0x30) != 0):
					time[-1] = str((byte & 0x30)>>4)
				time[-1] += str(byte & 0xF)
	if (m != 0):
		byte = bus.read_byte_data(addr,1)
		time.append('')
		if (m == 2):
			time[-1] = str((byte & 0xF0)>>4) + str(byte & 0xF)       #minutes
		elif (m == 1):
			if ((byte & 0xF0) != 0):
				time[-1] = str((byte & 0xF0)>>4)
			time[-1] += str(byte & 0xF)       
	if (s != 0):
		byte = bus.read_byte_data(addr,0)
		time.append('')
		if (s == 2):
			time[-1] = str((byte & 0xF0)>>4) + str(byte & 0xF)       #seconds
		elif (s == 1):
			if ((byte & 0xF0) != 0):
				time[-1] = str((byte & 0xF0)>>4)
			time[-1] += str(byte & 0xF)    
	output = time[0]
	for s in time[1:]:
		output += ':'+ s
	if (ampm == 1):
		output += ' PM'
	elif (ampm == 0):
		output += ' AM'
	return output
	
def getdate(addr):
	global year
	date = []
	byte = bus.read_byte_data(addr,4)
	date.append((byte & 0xF) + ((byte & 0xF0)>>4)*10)       #days
	byte = bus.read_byte_data(addr,5)
	date.append((byte & 0xF) + ((byte & 0x10)>>4)*10)       #months
	if (byte & 0x80 == 0x80):
		year += 1
		byte = byte & 0x7F
		bus.write_byte_data(addr,5,byte)
	byte = bus.read_byte_data(addr,6)
	date.append((byte & 0xF) + ((byte & 0xF0)>>4)*10)       #years
	date[2] += (year*100)
	return date

def getdate_s(addr,format='YYYY/MM/DD'):	#accepts M,MM,D,DD,Y,YY,YYY,YYYY in any config
	global year
	date = ['','','']
	m = d = y = 0
	pos = 0
	prevchar = 0
	for char in format:
		if (char == 'M'):
			m += 1
			if (char != prevchar):
				mpos = pos
				pos += 1
		elif (char == 'D'):
			d += 1
			if (char != prevchar):
				dpos = pos
				pos += 1
		elif (char == 'Y'):
			y += 1
			if (char != prevchar):
				ypos = pos
				pos += 1
		prevchar = char
	byte = bus.read_byte_data(addr,4)
	if (d == 2):													#days
		date[dpos] = str((byte & 0xF0)>>4) + str(byte & 0xF)
	elif (d == 1):
		if ((byte & 0xF0) != 0):
			date[dpos] = str((byte & 0xF0)>>4)
		date[dpos] +=  str(byte & 0xF)
	byte = bus.read_byte_data(addr,5)
	if (m == 2):													#months
		date[mpos] = str((byte & 0x10)>>4) + str(byte & 0xF)
	elif (m == 1):
		if ((byte & 0x10) != 0):
			date[mpos] = str((byte & 0x10)>>4)
		date[mpos] +=  str(byte & 0xF)	
	if (byte & 0x80 == 0x80):       #increase year if rollover detected
		year += 1
		byte = byte & 0x7F
		bus.write_byte_data(addr,5,byte)
	byte = bus.read_byte_data(addr,6)
	years = str(year)		#years
	if (y == 4):
		if (year > 9):
			date[ypos] = years
		else:
			date[ypos] = '0' + years
		date[ypos] += str((byte & 0xF0)>>4) + str(byte & 0xF)
	elif (y == 3):
		date[ypos] = years[-1]
		date[ypos] += str((byte & 0xF0)>>4) + str(byte & 0xF)
	elif (y == 2):
		date[ypos] = str((byte & 0xF0)>>4) + str(byte & 0xF)
	elif (y == 1):
		date[ypos] = str(((byte & 0xF0)>>4)*10 + (byte & 0xF) + (year*100))
		
	output = date[0]
	if (pos >= 2): output += '/' + date[1]
	if (pos >= 3): output += '/' + date[2]
	return output
	
def gettemp(addr,c):
        byte = bus.read_byte_data(addr,14)
        if (byte & 0x20 == 0):
                ctrl = byte | 0x20
                byte = bus.read_byte_data(addr,17)
                temp = byte & 0x7F
                if (byte & 0x80 == 0x80): temp = temp - 128
                byte = bus.read_byte_data(addr,18)
                bus.write_byte_data(addr,14,ctrl)
                if (byte & 0x80 == 0x80): temp = temp + 0.5
                if (byte & 0x40 == 0x40): temp = temp + 0.25
                if (c == 'c' or c == 'C'):
                        return temp
                elif (c == 'f' or c == 'F'):
                        temp = (temp * 1.8) + 32
                        return temp
        else:
                return 'b'
	
def settemp(c):
	global fc
	if (c == 'c' or c == 'C'): fc = 'C'
	elif (c == 'f' or c == 'F'): fc = 'F'
	
def timeupdate(channel):
	output = ''
	output += getdate_s(address,'MM/DD/YYYY')+' '
	output += gettime_s(address, 'HH:MM:SS')+' '
	temp = gettemp(address,fc)
	if (temp != 'b'):
		output += 'at '+str(temp)+'°'+fc
	sys.stdout.write(u'\u001b[s\u001b[H\u001b[2K')
	sys.stdout.write(output)
	sys.stdout.write(u'\u001b[u')
	sys.stdout.flush()

def reset(channel):
        global address
        addr = address
        settime(addr,0,12,0,0)
        setdate(addr,1,1,2018)

def bin(s):                     #convert to binary string
        return str(s) if s<=1 else bin(s>>1) + str(s&1)

#Set up
gpio.setmode(gpio.BCM)
gpio.setup(23, gpio.IN, pull_up_down=gpio.PUD_UP)
gpio.setup(24, gpio.IN, pull_up_down=gpio.PUD_DOWN)
gpio.add_event_detect(23, gpio.FALLING, callback=reset,bouncetime=300)
gpio.add_event_detect(24, gpio.RISING, callback=timeupdate,bouncetime=300)
bus.write_byte_data(address,14,0b11111011)

#Program Start
#settime(address,0,12,0,0)
#setdate(address,1,1,2018)
sys.stdout.write(u'\u001b[2J')
sys.stdout.write(u'\u001b[2;1H')

msg = ' (-1) for unchanged: '

while True:
	sys.stdout.write(u'\u001b[2;1H')
	inpt = raw_input('\rSet (t)ime, (d)ate, (f)ahrenheit, (c)elcius, (e)xit: ')
	if (inpt == 't' or inpt == 'T'):
		ampm = input('\r(-1)24Hr Mode, (0)AM, (1)PM: ')
		h = input('\rHour' + msg)
		m = input('\rMinute' + msg)
		s = input('\rSecond' + msg)
		settime(address,ampm,h,m,s)
	elif (inpt == 'd' or inpt == 'D'):
		y = input('\rYear' + msg)
		m = input('\rMonth' + msg)
		d = input('\rDay' + msg)
		setdate(address,d,m,y)
	elif (inpt == 'c' or inpt == 'C'):
		fc = 'C'
	elif (inpt == 'f' or inpt == 'F'):
		fc = 'F'
	elif (inpt == 'e' or inpt == 'E'):
		bus.write_byte_data(address,14,0b11111111)
		gpio.remove_event_detect(24)
		break
	sys.stdout.write(u'\u001b[2;1H\u001b[0J')
