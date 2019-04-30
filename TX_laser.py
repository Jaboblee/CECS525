import RPi.GPIO as gpio
from getch import *
from laser import *

teststring = 'abcd'
l = laser(17, 8, 1, 2, 11, 13)

try:
	time.sleep(2)
	i = 0
	for char in teststring:
		l.lase_s(char)
		if (len(l.inputbuff) != 0):
			for c in l.inputbuff:
				print(c,end='')
				sys.stdout.flush()
			l.inputbuff = []

	while (1):
		x = getch()
		l.lase_s(x)
		if (len(l.inputbuff) != 0):
			for c in l.inputbuff:
				print(c,end='')
				sys.stdout.flush()
			l.inputbuff = []
			
	l.close()
	
except KeyboardInterrupt:
	l.close()



