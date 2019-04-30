import RPi.GPIO as gpio
from laser import *

l = laser(17, 8, 1, 2, 11, 13)

try:
        while (1):
                if (len(l.inputbuff) != 0):
                                for c in l.inputbuff:
                                        print(c,end='')
                                l.inputbuff = []
		
except KeyboardInterrupt:
	l.close()

