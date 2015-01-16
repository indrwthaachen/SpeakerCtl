"""
SpeakerCtl controls NuPro(R) loudspeakers via infrared

Copyright (c) 2014, Gerrit Wyen <gerrit.wyen@rwth-aachen.de>, Florian Heese <heese@ind.rwth-aachen.de>
Institute of Communication Systems and Data Processing
RWTH Aachen University, Germany

"""

from helper import exec_cmd_single,exec_cmd_all
from PySide import QtCore, QtGui
from helper import enumerate_serial_ports, exitEvent
from functools import partial
import time
import config
import sys

class SpeakerWindow(QtGui.QDialog):

	def __init__(self, speaker):
		super(SpeakerWindow, self).__init__()

		self.setWindowTitle("speakerCtl")
		self.speakerName = speaker
		self.groupBox = QtGui.QGroupBox("Speaker: " + speaker)


		exec_cmd_single(speaker, "disableMissCheck")
		signature = exec_cmd_single(speaker, "Signature")
		volume = exec_cmd_single(speaker, "getVolume")
		lastLimitHit = exec_cmd_single(speaker, "?")
		currentInput = exec_cmd_single(speaker, "getInput")

		print "Got as Current Input: ", currentInput

		volumeLayout = QtGui.QHBoxLayout()
		volumeDescLabel = QtGui.QLabel("Current Volume: ")
		self.volumeLabel = QtGui.QLabel(volume)
		self.slider = QtGui.QSlider()
		self.slider.setMinimum(-80)
		self.slider.setMaximum(-1)
		print "Using volume: ",volume
		
		volumeInt = None
		try:
			volumeInt = int(volume)
		except:
			pass

		if isinstance(volumeInt, int) == False:
			self.slider.setEnabled(False) 
		else:
			self.slider.setValue(volumeInt)

		self.slider.valueChanged.connect(self.setSliderValueLabel)

		resetButton = QtGui.QPushButton("Reset Volume")
		resetButton.clicked.connect(self.resetSpeakerClicked)
		volumeLayout.addWidget(volumeDescLabel)
		volumeLayout.addWidget(self.volumeLabel)
		volumeLayout.addWidget(self.slider)
		volumeLayout.addWidget(resetButton)

		newVolumeLayout = QtGui.QHBoxLayout()
		self.newVolumeDescLabel = QtGui.QLabel("New Volume: ")
		self.newVolumeLabel = QtGui.QLabel(volume)
		newVolumeLayout.addWidget(self.newVolumeDescLabel)
		newVolumeLayout.addWidget(self.newVolumeLabel)

		iconLayout = QtGui.QHBoxLayout()
		self.iconLabel = QtGui.QLabel("Input:")
		self.inputComboBox = QtGui.QComboBox()

		self.inputComboBox.addItem("USB")
		self.inputComboBox.addItem("AUX")
		self.inputComboBox.addItem("SPDIF")


		self.inputComboBox.setCurrentIndex(int(currentInput))

		iconLayout.addWidget(self.iconLabel)
		iconLayout.addWidget(self.inputComboBox)

		limitLayout = QtGui.QHBoxLayout()
		limitDescLabel = QtGui.QLabel("Last Limit Hit: ")
		self.limitLabel = QtGui.QLabel(lastLimitHit)
		limitLayout.addWidget(limitDescLabel)
		limitLayout.addWidget(self.limitLabel)

		signatureLayout = QtGui.QHBoxLayout()
		signatureDescLabel = QtGui.QLabel("Signature: ")
		self.signatureLabel = QtGui.QLabel(signature)
		signatureLayout.addWidget(signatureDescLabel)
		signatureLayout.addWidget(self.signatureLabel)

		mainLayout = QtGui.QVBoxLayout()


		mainLayout.addLayout(signatureLayout)
		mainLayout.addLayout(limitLayout)

		mainLayout.addLayout(volumeLayout)
		mainLayout.addLayout(newVolumeLayout)

		mainLayout.addLayout(iconLayout)

		self.saveSpeakerButton = QtGui.QPushButton("Save")
		self.saveSpeakerButton.setDefault(True)

		self.saveSpeakerButton.clicked.connect(self.saveSpeakerClicked)

		mainLayout.addWidget(self.saveSpeakerButton)

		self.groupBox.setLayout(mainLayout)



		windowLayout = QtGui.QHBoxLayout()
		windowLayout.addWidget(self.groupBox)

		self.setLayout(windowLayout) 
		self.show()

	def refreshData(self):
		signature = exec_cmd_single(self.speakerName, "Signature")
		volume = exec_cmd_single(self.speakerName, "getVolume")
		lastLimitHit = exec_cmd_single(self.speakerName, "?")
		currentInput = exec_cmd_single(self.speakerName, "getInput")

		self.signatureLabel.setText(signature)
		self.volumeLabel.setText(volume)
		self.limitLabel.setText(lastLimitHit)

		self.inputComboBox.setCurrentIndex(int(currentInput))


		volumeInt = None
		try:
			volumeInt = int(volume)
		except:
			pass

		if isinstance(volumeInt, int) == True:
			self.slider.setEnabled(True) 
			self.slider.setValue(volumeInt)

	def saveSpeakerClicked(self):
		newInput = self.inputComboBox.currentText()
		newVolume = self.slider.value()

		print "Setting new input: ",newInput
		exec_cmd_single(self.speakerName, str(newInput))
		exec_cmd_single(self.speakerName, "setVolume "+str(newVolume))

		self.refreshData()

	def resetSpeakerClicked(self):
		exec_cmd_single(self.speakerName, "resetVolume")

		self.refreshData()

	def setSliderValueLabel(self):
		newVolume = self.slider.value()
		self.newVolumeLabel.setText(str(newVolume))
		

class SettingsWindow(QtGui.QDialog):
	def __init__(self):
		super(SettingsWindow, self).__init__()

		self.createIconGroupBox()
	
		self.createTrayIcon()

		self.saveSettingsButton.clicked.connect(self.saveSettingsClicked)
		self.trayIcon.messageClicked.connect(self.messageClicked)
		mainLayout = QtGui.QVBoxLayout()
		mainLayout.addWidget(self.iconGroupBox)
		self.setLayout(mainLayout) 


		self.trayIcon.show()

		self.setWindowTitle("Systray")
		self.setIcon();

		# Create Update QTimer. Call every 5 seconds
		self.timer = QtCore.QTimer()
		self.timer.timeout.connect(self.updateSpeakerList)
		self.timer.start(5000)

	def updateSpeakerList(self):
		print "Updating Speakers List"
		print "Current Speakers: ", config.devDescList

		self.speaker_menu.clear()
		for speaker in config.devDescList:
			self.speaker_menu.addAction(speaker, partial(self.speakerClicked, speaker))

			
	def saveSettingsClicked(self):
		comList = []

		for box in self.comPortCheckBoxs:
			if box.checkState():
				comList.append(box.text())

		print "Saving Comports: ", comList
		config.qsettings.setValue('ComPorts', comList)
		config.updatePortsEvent = True

		self.hide()


	def resetAll(self):
		pass


	def closeEvent(self, event):
		self.hide()


	def setIcon(self):
		icon = QtGui.QIcon('nupro.ico')
		self.trayIcon.setIcon(icon)
		self.setWindowIcon(icon)

		self.trayIcon.setToolTip('Nupro')


	def showMessage(self):
		icon = QtGui.QSystemTrayIcon.MessageIcon(
				self.typeComboBox.itemData(self.typeComboBox.currentIndex()))
		self.trayIcon.showMessage(self.titleEdit.text(),
				self.bodyEdit.toPlainText(), icon,
				self.durationSpinBox.value() * 1000)

	def messageClicked(self):
		QtGui.QMessageBox.information(None, "Systray", "")

		
	def speakerClicked(self, speaker):
		print "Speaker Clicked: ", speaker

		try:
			self.speakerwindow.destroy()
		except:
			pass

		self.speakerwindow = SpeakerWindow(speaker)
		self.speakerwindow.show()



	def createIconGroupBox(self):
   
		self.iconGroupBox = QtGui.QGroupBox("Com Ports")

		self.comPortCheckBoxs = []


		comPortList = []

		comPortList = config.qsettings.value('ComPorts', type=str)
		if comPortList is None:
			comPortList = []

		print "Current Com Ports: ", comPortList


		for com in enumerate_serial_ports():
			box = QtGui.QCheckBox(com)
			if com in comPortList:
				box.setChecked(True)

			self.comPortCheckBoxs.append(box);


		vLayout = QtGui.QVBoxLayout()

		checkboxLayout = QtGui.QVBoxLayout()


		for box in self.comPortCheckBoxs:
			checkboxLayout.addWidget(box)



		self.saveSettingsButton = QtGui.QPushButton("Save")
		self.saveSettingsButton.setDefault(True)


		vLayout.addStretch()
		vLayout.addLayout(checkboxLayout)
		vLayout.addWidget(self.saveSettingsButton)
		self.iconGroupBox.setLayout(vLayout)




	def createActions(self):

		self.minimizeAction = QtGui.QAction("Mi&nimize", self, triggered=self.hide)
		self.maximizeAction = QtGui.QAction("Ma&ximize", self, triggered=self.showMaximized)
		self.resetAllAction = QtGui.QAction("&Reset All", self, triggered=self.resetAll)
		self.restoreAction = QtGui.QAction("&Settings", self, triggered=self.showNormal)

		self.quitAction = QtGui.QAction("&Quit", self, triggered=QtGui.qApp.quit)


	def createTrayIcon(self):

		self.createActions()

		self.trayIconMenu = QtGui.QMenu(self)

		self.speaker_menu = QtGui.QMenu("Speakers")

		for speaker in config.devDescList: 
			self.speaker_menu.addAction(speaker, lambda: self.speakerClicked(speaker) ) 

		self.trayIconMenu.addMenu(self.speaker_menu)

		self.trayIconMenu.addAction(self.restoreAction)
		self.trayIconMenu.addSeparator()
		self.trayIconMenu.addAction(self.quitAction)

		self.trayIcon = QtGui.QSystemTrayIcon(self)
		self.trayIcon.setContextMenu(self.trayIconMenu)


def qt_systray_main():
   
	app = QtGui.QApplication(sys.argv)

	if not QtGui.QSystemTrayIcon.isSystemTrayAvailable():
		QtGui.QMessageBox.critical(None, "Systray", "I couldn't detect any system tray on this system.")
		sys.exit(1)

	QtGui.QApplication.setQuitOnLastWindowClosed(False)

	window = SettingsWindow()
	window.hide()
	ret = app.exec_()
	exitEvent.set()
	time.sleep(10);
  

if __name__ == "__main__":
	qt_systray_main([])
	

