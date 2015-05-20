import serial
from struct import pack, unpack
from math import pi
import time

ser = serial.Serial("/dev/ttyACM0", 115200)

def getDouble():
	return unpack('d', ser.read(8))

def sendDouble(f):
	x = pack('d', f)
	for i in range(8):
		ser.write(x[i])

def main():
	ser.close()
	ser.open()
	# give the Arduino some time to 'boot'
	time.sleep(2.0)
	f = open('col1.csv', 'r')
	#mio = 2.345e-06
	#while(True):
	#	sendDouble(mio)
	#	print getDouble()
	for line in f:
		 sendDouble(float(line))
		 print getDouble()
	print "VAR:", getDouble()
	print "SKEW:", getDouble()
		 
if __name__ == '__main__':
	main()
# END OF FILE
