"""
SpeakerCtl controls NuPro(R) loudspeakers via infrared

Copyright (c) 2014, Gerrit Wyen <gerrit.wyen@rwth-aachen.de>, Florian Heese <heese@ind.rwth-aachen.de>
Institute of Communication Systems and Data Processing
RWTH Aachen University, Germany

"""

import SocketServer
import time
import config
import sys
import select
from threading import Thread,RLock;
from serial import Serial
from helper import openComPorts,closeComPorts,exitEvent,exec_cmd_single,exec_cmd_all


class TCPHandler(SocketServer.BaseRequestHandler):
	"""
	TCP Server for handling requests
	"""

	def handle(self):


		while True:		

			self.request.setblocking(0)
			timeout_in_seconds = 60*60*2
			ready = select.select([self.request], [], [], timeout_in_seconds)
			if ready[0]:
				self.data = self.request.recv(1024)
			else:
				return
			print "{} wrote:".format(self.client_address[0])
			print self.data

			if self.data == '':
				break

			cmdParts = self.data.strip().split(' ')
			print cmdParts

			if  cmdParts[0] == 'getSpeakers':
				speakerList = ""
				for speaker in config.devDescList:
					speakerList += speaker + " "


				print "Sending " + speakerList
				self.request.send(speakerList+"\n")
			
			elif cmdParts[0] == 'all':

				cmd = ' '.join(cmdParts[1:])

				print "Sending each Speaker: [" + cmd + "]"
				cmd = cmd + "\n"

				
				response = exec_cmd_all(cmd)

				print "Sending back: ", response
				self.request.send(response+"\n")

			else:
				cmd = ' '.join(cmdParts[1:])
				print "Sending Speaker ["+cmdParts[0]+"]: [" + cmd + "]"
				
				if cmdParts[0] not in config.devDescList:
					response = "speaker does not exist"
				else:
					response = exec_cmd_single(cmdParts[0], cmd)

				print "Sending back: ", response
				self.request.send(response+"\n")


	

def tcpDaemonMain():	
	# Create the server, binding to localhost
	server = SocketServer.ThreadingTCPServer((config.tcpHost, config.tcpPort), TCPHandler)
	server.timeout = 2
	
	openComPorts()

	try:

		while True:

				
			print "Checking for Change.."
			if config.updatePortsEvent == True:
				print "ComPorts changed"
				closeComPorts()
				openComPorts()
				config.updatePortsEvent = False
			
			server.handle_request()

	except Exception, e:
		closeComPorts()
		sys.exit(0)



