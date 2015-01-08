/*
Library for communication with ST7032 Dot Matrix LCD Controller/Driver over I2C
e.g. for GE-O1602F/WO1602F LCDs 

Copyright (c) 2014, Gerrit Wyen <gerrit.wyen@rwth-aachen.de>, Florian Heese <heese@ind.rwth-aachen.de>
Institute of Communication Systems and Data Processing
RWTH Aachen University, Germany

*/

const char Slave = 0x3E;
const char Comsend = 0x00;
const char Datasend = 0x40;
const char Line2 = 0xC0;


#define setBit(x,y) x |= (1<<y)


// encapsulate Wire for easier reuse
void sendCMDStart()
{
    	Wire.beginTransmission(Slave);
      	Wire.write(Comsend);
}

void sendCMD(int cmd) 
{
 	Wire.write(cmd);  
}

int sendCMDCommit()
{
        int ret;
  	ret = Wire.endTransmission(true);
        return ret;
}


int function_set(bool N, bool DH, bool IS)
{
	char cmd = 0b00110000;

	if(IS) setBit(cmd,0);
	if(DH) setBit(cmd,2);
	if(N) setBit(cmd,3);

	sendCMD(cmd);
}

int internal_osc_frequency(bool BS, bool F2, bool F1, bool F0)
{
	char cmd = 0b00010000;

	if(BS) setBit(cmd, 3);
	if(F2) setBit(cmd, 2);
	if(F1) setBit(cmd, 1);
	if(F0) setBit(cmd, 0);

	sendCMD(cmd);
}

int contrast_set(bool C3, bool C2, bool C1, bool C0)
{
	char cmd = 0b01110000;

	if(C3) setBit(cmd, 3);
	if(C2) setBit(cmd, 2);
	if(C1) setBit(cmd, 1);
	if(C0) setBit(cmd, 0);

	sendCMD(cmd);	
}

int power_icon_contrast_control(bool Ion, bool Bon, bool C5, bool C4)
{
	char cmd = 0b01010000;

	if(Ion) setBit(cmd, 3);
	if(Bon) setBit(cmd, 2);
	if(C5) setBit(cmd, 1);
	if(C4) setBit(cmd, 0);

	sendCMD(cmd);
}


int follower_control(bool Fon, bool Rab2, bool Rab1, bool Rab0)
{

	char cmd = 0b01100000;

	if(Fon) setBit(cmd, 3);
	if(Rab2) setBit(cmd, 2);
	if(Rab1) setBit(cmd, 1);
	if(Rab0) setBit(cmd, 0);

	sendCMD(cmd);
}

int display_on_off_control(bool D, bool C, bool B)
{
	char cmd = 0b00001000;

	if(D) setBit(cmd, 2);
	if(C) setBit(cmd, 1);
	if(B) setBit(cmd, 0);

	sendCMD(cmd);
}


int init_lcd(int lcdRevision)
{

        sendCMDStart();
	delay(50);

	bool TwoDisplayLines = 1;
	bool DoubleHeightFont = 0;
	bool ExtensionInstructionSelect = 1;

	function_set(TwoDisplayLines, DoubleHeightFont, ExtensionInstructionSelect);
	delayMicroseconds(27);
	function_set(TwoDisplayLines, DoubleHeightFont, ExtensionInstructionSelect);
	delayMicroseconds(27);

	bool biasSelection = 1; // 1/4
	bool F2=0,F1=0,F0=0; // Internal OSC frequency adjust; frame freq: 122Hz

	internal_osc_frequency(biasSelection, F2, F1, F0);
	delayMicroseconds(27);

        bool C0,C1,C2,C3,C4,C5;
        if(lcdRevision == 1)
	  C0=0, C1=1, C2=0, C3=0, C4=0, C5=1; // for old one
        else
	  C0=0, C1=1, C2=0, C3=1, C4=0, C5=1; // for new speakerctrls
	contrast_set(C3, C2, C1, C0);
	delayMicroseconds(27);

	bool IconDisplayOn = 0;
	bool BoosterCircuit=1;
	power_icon_contrast_control(IconDisplayOn, BoosterCircuit, C5, C4);
	delayMicroseconds(27);

	bool switchFollowerCircuitOn=1;
	bool Rab2=0,Rab1=1,Rab0=1; // V0 generator amplified ratio;
	follower_control(switchFollowerCircuitOn, Rab2, Rab1, Rab0);
	delay(210);

	bool displayOn = 1;
	bool cursorOn = 0;
	bool cursorBlinkOn = 0;
	display_on_off_control(displayOn, cursorOn, cursorBlinkOn);
	delayMicroseconds(27);

        sendCMDCommit();
}

void writeString(char *text)
{
	int n, ret;
        int len = strlen(text);    
	

	for(n=0;n<len;n++)
	{  
                  
                delayMicroseconds(100);
                Wire.beginTransmission(Slave);
    	        Wire.write(Datasend);
  		Wire.write(*text);  
                ret = Wire.endTransmission(true);
		text++;
	}

	

}

int clearDisplay()
{
        sendCMDStart();
        int cmd = 0b00000001;
          
        sendCMD(cmd);

        sendCMDCommit();
}

int newLine()
{
        sendCMDStart();
          
        sendCMD(0xC0);

        sendCMDCommit();
}

int returnHome()
{
        sendCMDStart();
        int cmd = 0b00000010;
          
        sendCMD(cmd);

        sendCMDCommit();
}

int entryModeSet()
{
        int increment = 1;
        int shiftDisplay = 0;
  
  
        sendCMDStart();
        int cmd = 0b00000100;
        
	if(increment) setBit(cmd, 1);
	if(shiftDisplay) setBit(cmd, 0);

        sendCMD(cmd);

        sendCMDCommit();
}




void showMessage(char *msg)
{
        returnHome();
        entryModeSet();
	writeString(msg);
}


enum INPUTS {USB, AUX, SPDIF};


void showStatusLCD(int volume, int input)
{
	char buffer[120];
        
       
        returnHome();
        entryModeSet();
        if(volume != -100)
          sprintf(buffer, "Volume: %d dB    ", volume);
        else
          sprintf(buffer, "not initialized", volume);
	writeString(buffer);
        newLine();
        
        switch(input)
        {
           case USB:
                    writeString("Input: USB   ");
                    break;
           case AUX:
                   writeString("Input: AUX   ");
                   break;
           case SPDIF:
                   writeString("Input: SPDIF   ");
                   break;
        }           
	delay(100);     

}



void initLCD(int lcdRevision)
{
	Wire.begin();
        init_lcd(lcdRevision);
        clearDisplay();
        delay(100); 
}


