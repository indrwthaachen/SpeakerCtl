"""
SpeakerCtl controls NuPro(R) loudspeakers via infrared

Copyright (c) 2014, Gerrit Wyen <gerrit.wyen@rwth-aachen.de>, Florian Heese <heese@ind.rwth-aachen.de>
Institute of Communication Systems and Data Processing
RWTH Aachen University, Germany

"""

import _winreg as winreg
import itertools
import config
import time
from serial import Serial
from threading import RLock, Event

exitEvent = Event()


cmdSleepDuration = 0.8
readDelayDuration = 0.5

def exec_cmd_single(speakerName, cmd):
	cmd = cmd + "\n"
	for speaker in config.speakers:

		if speaker['name'] == speakerName:

			if speaker['lock'].acquire(blocking=False) is False:
				return "busy"

			print "Found Speaker"

			if speaker['ioDev'].inWaiting() != 0: # clear old data
				speaker['ioDev'].read( speaker['ioDev'].inWaiting() )

			speaker['ioDev'].write(cmd)


			endTime = time.time()+2*60
			n = ""

			while time.time()<endTime:
				n += speaker['ioDev'].read( speaker['ioDev'].inWaiting() )

				if len(n) > 0 and n[-1] == '\n':
					break

				time.sleep(0.01)
			speaker['lock'].release() 

			return n.strip("\n")


def exec_cmd_all(cmd):
	for speaker in config.speakers:	
		if speaker['lock'].acquire(blocking=False) is False:
			return "one or more busy"
			
		if speaker['ioDev'].inWaiting() != 0: # clear old data
				speaker['ioDev'].read( speaker['ioDev'].inWaiting() )
		speaker['ioDev'].write(cmd)
		

	n=""
	for speaker in config.speakers:
		n += speaker['name'] + ": "

		endTime = time.time()+2*60
		
		while time.time()<endTime:
			n += speaker['ioDev'].read( speaker['ioDev'].inWaiting() )
			if len(n) > 0 and n[-1] == '\n':
				break
			time.sleep(0.01)

		speaker['lock'].release() 
		n=n.strip("\n")
		n += " | "
		
	return n.strip(" | ")

	
def closeComPorts():
	for speaker in config.speakers:
		speaker['ioDev'].close()

		
		
def openComPorts():
	# init IO devices
	comPorts = config.qsettings.value('ComPorts', type=str)
	config.devDescList = []
	failedComPorts = []
	if comPorts == None:
		return

	for comPort in comPorts:
		try:
			dev = Serial(comPort, timeout=20)
		except:
			try:
				dev = Serial(comPort, timeout=20)
			except:
				try:
					dev = Serial(comPort, timeout=20)
				except:
					continue

		print "Opening ", comPort

		while dev.inWaiting() != 0: # clear old stuff
			dev.read( dev.inWaiting() )

		name = getNameOfDev(dev)
		print "getNameOfDev: " + name

		if name != '':
			config.speakers.append( {'ioDev':dev, 'name':name, 'active':False, 'lock': RLock()} )
			config.devDescList.append(name)


def getNameOfDev(dev):
	dev.write("NuproName\n")

	if dev.inWaiting() == 0:
		time.sleep(1.9)
	n = ""
		
	while dev.inWaiting() != 0:
		n += dev.read(dev.inWaiting())
		time.sleep(0.3)
	
	return n.strip()
	

def enumerate_serial_ports():
	""" Read Win32 registry and yield serial (COM) ports
	"""
	path = 'HARDWARE\\DEVICEMAP\\SERIALCOMM'
	try:
		key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, path)
	except WindowsError:
		return

	for i in itertools.count():
		try:
			val = winreg.EnumValue(key, i)
			yield str(val[1])
		except EnvironmentError:
			break


