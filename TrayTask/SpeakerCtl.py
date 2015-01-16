"""
SpeakerCtl controls NuPro(R) loudspeakers via infrared

Copyright (c) 2014, Gerrit Wyen <gerrit.wyen@rwth-aachen.de>, Florian Heese <heese@ind.rwth-aachen.de>
Institute of Communication Systems and Data Processing
RWTH Aachen University, Germany

"""

import pythoncom
import win32serviceutil
import win32service
import win32event
import servicemanager
import socket
import time
import sys

from thread import start_new_thread
from threading import Thread, RLock
from serial import Serial
from tcpDaemon import tcpDaemonMain
from PySide.QtCore import QSettings

import config
from qtTrayicon import qt_systray_main
from helper import openComPorts, closeComPorts, exitEvent

def main():

	guiThread = Thread(target=qt_systray_main, args=())
	guiThread.daemon = True
	guiThread.start()

	tcpThread = Thread(target=tcpDaemonMain, args=())
	tcpThread.daemon = True
	tcpThread.start()


	while True:
		if exitEvent.is_set():
			closeComPorts()
			break;
		time.sleep(1);
		



class AppServerSvc (win32serviceutil.ServiceFramework):
	_svc_name_ = "SpeakerCtlService"
	_svc_display_name_ = "SpeakerCtl Service"

	def __init__(self,args):
		win32serviceutil.ServiceFramework.__init__(self,args)
		self.hWaitStop = win32event.CreateEvent(None,0,0,None)
		socket.setdefaulttimeout(60)

	def SvcStop(self):
		self.ReportServiceStatus(win32service.SERVICE_STOP_PENDING)
		win32event.SetEvent(self.hWaitStop)

	def SvcDoRun(self):
		servicemanager.LogMsg(servicemanager.EVENTLOG_INFORMATION_TYPE,
							  servicemanager.PYS_SERVICE_STARTED,
							  (self._svc_name_,''))
		main()




if __name__ == '__main__':

	config.qsettings = QSettings(config.qSettingsCompany, config.qSettingsName)

	if config.runAsService:
		win32serviceutil.HandleCommandLine(AppServerSvc)
	else:
		main()








