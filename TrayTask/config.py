"""
SpeakerCtl controls NuPro(R) loudspeakers via infrared

Copyright (c) 2014, Gerrit Wyen <gerrit.wyen@rwth-aachen.de>, Florian Heese <heese@ind.rwth-aachen.de>
Institute of Communication Systems and Data Processing
RWTH Aachen University, Germany

"""

#------user settings----

tcpHost = "localhost"
tcpPort = 9999
comPorts = [ '' ]  
runAsService = False

#-----------------------

qSettingsCompany = "RWTH IND"
qSettingsName = "SpeakerCtl"

qsettings = None
updatePortsEvent = False

speakers = []
devDescList = []
