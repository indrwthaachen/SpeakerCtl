/*
SpeakerCtl can control NuPro(R) Speakers with IR and optical feedback 

Copyright (c) 2014, Gerrit Wyen <gerrit.wyen@rwth-aachen.de>, Florian Heese <heese@ind.rwth-aachen.de>
Institute of Communication Systems and Data Processing
RWTH Aachen University, Germany

*/


#include <Wire.h>
#include <IRremote.h>
#include "lcd_ST7032.h"
#include <EEPROM.h>
#include "speakerctl_config.h"

#define DEBUG 0

// receive buffer for bluetooth serial
#define BUF_SIZE 100
char receiveBuffer[BUF_SIZE];
int bytesReceived = 0;

// store timestamp of last limit hit event
long lastLimitHit = -1;

// keep track of current volume and input
int currentVolume = -100;
int currentInputEEPROM = 0x00;

int NameEEPROM = 0x10; // to 0x20
int NameLenEEPROM = 10;

int SetBluetoothNameEEPROM = 0x1B; 


// setup IR Input/Output
IRrecv irrecv(IRInputPin);
IRsend irsend; 
decode_results results;

#include "speakerctl.h"


// store light signature in order to detect unanticipated changes
int lastPersistentSignature[6];


// variable for storing the device name
char device_name[10];


int checkMissEnabled = 1;
int printLightDiff = 0;



void writeStringEEPROM(int addr, char *string, int len)
{
   for(int i=0; i<len; i++)
   {
     if(strlen(string) > i)
         EEPROM.write(addr+i, string[i]);
     else
         EEPROM.write(addr+i, 0);
   }  
}


void readStringEEPROM(int addr, char *string, int len)
{
  for(int i=0; i<len; i++)
  {
    string[i] = EEPROM.read(addr+i);  
  }
  string[len-1] = 0;
}

int currentInputCache = -1;

void setCurrentInput(int input)
{
  EEPROM.write(currentInputEEPROM, input);
  currentInputCache = input;
}

int getCurrentInput()
{
  if(currentInputCache == -1)
    currentInputCache = EEPROM.read(currentInputEEPROM);
  
  return currentInputCache;  
}

void restoreInput()
{
   switch(getCurrentInput())
  {
      case USB:
            setInputUSB();
            break;
      case SPDIF:
            setInputSPDIF();
            break;
      case AUX:
            setInputAUX();
            break;
  }
}

void setup() 
{
  
  // initialize bluetooth and regular serial connection  
  BTSerial.begin(9600);
  Serial.begin(9600);

  // set resolution of ADC
  analogReadRes(13); 
  delay(20);
  
  BTSerial.println("SpeakerRemote started");

  // read device name from EEPROM
  readStringEEPROM(NameEEPROM, device_name, NameLenEEPROM);
  
  // set bluetooth name if requested
  if(EEPROM.read(SetBluetoothNameEEPROM))
  {
    // Code for AT Commands of Bluetooth Module
    for(int i=0; i<10; i++)
    {
       
      Serial.println("LCD config...");
      
      BTSerial.print("AT+NAME");
      BTSerial.print(device_name);
      delay(500);
      while(BTSerial.available() > 0) Serial.println(BTSerial.read());
      delay(500);
     }
     
     EEPROM.write(SetBluetoothNameEEPROM, 0);
  }
   
  // switch off led
  pinMode(TeensyLedPin, OUTPUT);  
  digitalWrite(TeensyLedPin, LOW);


  pinMode(IRDiode, OUTPUT);  
  
  memset(receiveBuffer,0, BUF_SIZE);
  
  // attach Interrupt for handling IR Remote Inputs
  pinMode(IRInputPin, INPUT); 
  irrecv.enableIRIn(); // Start the receiver

 results.rawlen = 0;
 
 // restore Input from EEPROM
  restoreInput();
    
  int lcdRevision = 2;
  if(strcmp(device_name,"Nupro1") == 0)
    lcdRevision = 1;
    
  initLCD(lcdRevision);
 // showMessage("not initialized");
  showStatusLCD(currentVolume, getCurrentInput());

}


int resetVolume()
{  
  
 showMessage("initializing...");

 // send VolumeMinus 90-times
 for(int i=0; i<90; i++)
 {   
   sendIRVolumeMinus();
   BTSerial.print("busy ");    
   delay(108);
 }
 
  showStatusLCD(-80, getCurrentInput());
}




int compareLightSignatures(int signature1[], int signature2[])
{
  int diff = 0, diffSum=0;
  
  // calculate diff of two light signatures, square each compontent
  for(int i=0; i < 6; i++){
    diff = signature1[i]-signature2[i];
    diffSum += diff*diff;
  }
  
  return diffSum;  
}


int signatureCheckLimit(int signature[])
{
  
  int diffSum = compareLightSignatures(signature, limitSignature);  
  
  // check if difference is saller than threshold
  if(diffSum < signatureCompareThreshold)
      return true;
      
  return false;
  
}


int readLightSignature(int *signature)
{  
  int samples = 70; // number of samples


  for(int i=0; i<6; i++)
    signature[i] = 0;
  
  for(int i=0; i<samples; i++) 
  {
    signature[0] += analogRead(DETECT1);
    signature[1] += analogRead(DETECT2);
    signature[2] += analogRead(DETECT3);
    signature[3] += analogRead(DETECT4);
    signature[4] += analogRead(DETECT5);
    signature[5] += analogRead(DETECT6);
    delay(1);
  }
    
  
  for(int i=0; i<6; i++)
  {
    signature[i] = signature[i]/samples; 
  }
}


void setNewVolume(int newVolume)
{
      int goPlus;
      int sigDiff;
 
      int orgSignature[6], newSignature[6];
    
     // check that volume is within bound
      if(newVolume >= -80 && newVolume <= 0)
      {

        // calculate diff between new and old volume        
        int diffVolume = newVolume - currentVolume;
        
        if(printLightDiff)
        {
          BTSerial.print("Volume Diff: ");
          BTSerial.print(diffVolume,DEC);
          BTSerial.print(" ");
        }
        
        // determine direction
        if(diffVolume < 0)
          goPlus = 0;
        else
          goPlus = 1;
        
        diffVolume = abs(diffVolume);
        delay(250);
        int currentInput = getCurrentInput();
        
        int missCounter = 0;
        
        for(int i=0; i<diffVolume; i++)
        {
            
            // read light signature before sending cmd
            readLightSignature(orgSignature);
           
            if(goPlus)
            {
             sendIRVolumePlus();
             showStatusLCD(currentVolume+i, currentInput);
            }
            else
            {
              sendIRVolumeMinus();
              showStatusLCD(currentVolume-i, currentInput);
            }
            //restoreInput();
            //delay(250);
            delay(300);
            // read light signature after sending cmd
            readLightSignature(newSignature);
            sigDiff = compareLightSignatures(orgSignature, newSignature);
            BTSerial.print(" . ");
             
            if(sigDiff < 3000 && checkMissEnabled) // check if volume change did actually occure
            {
               BTSerial.print("-- Detected Miss!!! --");
               BTSerial.print("LightDiff: ");
               BTSerial.print(sigDiff, DEC);
               BTSerial.print(" ");
               missCounter++;
               i--;     
            }
            
            if(missCounter > 10)
            {
               newVolume = -100;
               break; 
            }
        }
               
        
        currentVolume = newVolume;
     }
}


int processUserInput(char *cmd)
{
  
  if(strcmp(cmd, "isNupro") == 0)
  { 
    BTSerial.println("+Nupro");
  }
  else  
  if(strcmp(cmd, "NuproName") == 0)
  { 
    BTSerial.println(device_name);
  }
  else  
  if(strcmp(cmd, "scan") == 0)
  { 
 
    if(currentVolume == -100)
        BTSerial.println("not initialized"); 
    else
    {
      
      
    }
  }
  else
  if(strcmp(cmd, "disableMissCheck") == 0)
  { 
    checkMissEnabled = 0;
    BTSerial.println("ok");
  }
  else
  if(strcmp(cmd, "printLightDiff") == 0)
  {
    printLightDiff = 1;
    BTSerial.println("ok");
  }
  else
  if(strcmp(cmd, "resetVolume") == 0)
  {
    resetVolume();
    currentVolume = -80;
    irrecv.enableIRIn();
    BTSerial.println("ok");    
  }
  else
  if(strcmp(cmd, "getVolume") == 0)
  {    
    if(currentVolume != -100)
        BTSerial.println(currentVolume,DEC); 
    else
        BTSerial.println("not initialized"); 
  }
  else  
  if(strncmp(cmd, "setVolume", 9) == 0)
  {   
    
    int len = strlen(cmd);

    
    
    if(currentVolume == -100)
    {
      BTSerial.println("not initialized");
    }
    else if(len > 10)
    {
        int newVolume = atoi(cmd+10); // convert to int, skip chars
      
        setNewVolume(newVolume);
        
        // save light signature as persistent signature
        readLightSignature(lastPersistentSignature);
        irrecv.enableIRIn();
        BTSerial.println("Ok");
     
    }
  }
  else  
  if(strcmp(cmd, "Volume+") == 0)
  {
    delay(400);
    sendIRVolumePlus();

    // save new persistent light signature
    readLightSignature(lastPersistentSignature); 

    irrecv.enableIRIn();
    BTSerial.println("Ok");
  }
  else
  if(strcmp(cmd, "Volume-") == 0)
  {
     delay(400);
     sendIRVolumeMinus();

     // save new persistent light signature
     readLightSignature(lastPersistentSignature); 
     
     irrecv.enableIRIn();
     BTSerial.println("Ok");  
  }
  else
  if(strcmp(cmd, "getInput") == 0)
  {
     BTSerial.println(getCurrentInput());    
  }
  else
  if(strcmp(cmd, "USB") == 0)
  {
     delay(400);
     setInputUSB();
     BTSerial.println("Ok");    
     
     // write new INPUT setting to EEPROM
     setCurrentInput(USB);
     // save new persistent light signature
     readLightSignature(lastPersistentSignature);

  }
  else
  if(strcmp(cmd, "AUX") == 0)
  {
     delay(400);
     setInputAUX();
     BTSerial.println("Ok");   
    
     // write new INPUT setting to EEPROM
     setCurrentInput(AUX);
     // save new persistent light signature
     readLightSignature(lastPersistentSignature);

  }
  else
  if(strcmp(cmd, "SPDIF") == 0)
  {
     delay(400);
     
         
     setInputSPDIF();
     BTSerial.println("Ok"); 

     // write new INPUT setting to EEPROM
     setCurrentInput(SPDIF);
     // save new persistent light signature
     readLightSignature(lastPersistentSignature);

  }
  else
  if(strcmp(cmd, "MUTE") == 0)
  {
     delay(400);
     toggleMute();
     BTSerial.println("Ok"); 
     // save new persistent light signature
     readLightSignature(lastPersistentSignature);

  }
  else
  if(strcmp(cmd, "Signature") == 0)
  {
    int signature[6], newSignature[6];
    int sigDiff;
    
    // read and print current light signature
    readLightSignature(signature);
    BTSerial.print(signature[0], DEC);   
    BTSerial.print(" ");
    BTSerial.print(signature[1], DEC);   
    BTSerial.print(" ");
    BTSerial.print(signature[2], DEC);    
    BTSerial.print(" "); 
    BTSerial.print(signature[3], DEC); 
    BTSerial.print(" ");   
    BTSerial.print(signature[4], DEC);  
    BTSerial.print(" ");  
    BTSerial.print(signature[5], DEC);  
  
    // reread light signature to determine noise
    delay(100);
    readLightSignature(newSignature);
    sigDiff = compareLightSignatures(signature, newSignature);  
   
    BTSerial.print(" Noise: ");
    BTSerial.println(sigDiff, DEC);

  }
  else
  if(strcmp(cmd, "?") == 0)
  {
    // print ms since last time that limit was hit
    if(lastLimitHit == -1)
    {
      BTSerial.println("No limit hits since startup");
    }
    else
    {
      BTSerial.print("Last limit hit: ");
      BTSerial.print(millis()-lastLimitHit, DEC); 
      BTSerial.println(" ms ago");
    }
  }
  else
  if(strncmp(cmd, "SetName", 7) == 0)
  {
     strcpy(device_name, cmd+8);
     writeStringEEPROM(NameEEPROM, device_name, NameLenEEPROM); 
     EEPROM.write(SetBluetoothNameEEPROM, 1);
     BTSerial.println("Ok"); 

  }
  else
  {
     BTSerial.println("Error: Command not found");    
  }
  
  // update information on LCD
  showStatusLCD(currentVolume, getCurrentInput());
}



void loop()
{ 
   
  int signature[6];
  
  readLightSignature(signature);

  // only check light signature for limit signature if persistent signature changed
  if(compareLightSignatures(lastPersistentSignature, signature) > 3000)
  {
    if(signatureCheckLimit(signature)){
        Serial.println("WARNING: Did hit Limit ! ");
        if(DEBUG) BTSerial.println("WARNING: Did hit Limit ! ");
        lastLimitHit = millis();
    }
    
  }
  
  // check if IR command was received that can be relayed
  if (irrecv.decode(&results)) {


     if (results.decode_type == NEC) {

       if(DEBUG)
       {
         BTSerial.print("Code: ");
         BTSerial.println(results.value, DEC);
       }
       
       switch(results.value)
       {
         case 0xBF20DF:
             if(currentVolume == -100 || currentVolume == -1) break;
             sendIRVolumePlus();
             irrecv.enableIRIn();
             currentVolume++;
             break;
         case 0xBF10EF:
             if(currentVolume == -100 || currentVolume == -80) break;
             sendIRVolumeMinus();
             irrecv.enableIRIn();
             currentVolume--;
             break;
         case 0xBF08F7:
             setCurrentInput(USB);
             setInputUSB();
             break;
         case 0xBF8877:
             setCurrentInput(AUX);
             setInputAUX();
             break;
         case 0xBF48B7:
             setCurrentInput(SPDIF);
             setInputSPDIF();
             break;
         case 0xBF00FF:
             resetVolume();
             currentVolume = -80;
             irrecv.enableIRIn();
             break;
       }
     }
     else
                 Serial.println("Received unknown IR Signal");

    irrecv.resume(); // receive the next value
    
     // update information on screen
     showStatusLCD(currentVolume, getCurrentInput());

  }
  


  // read the incoming bytes
  if (BTSerial.available() > 0) {
    
        receiveBuffer[bytesReceived] = BTSerial.read();
        bytesReceived++;
        
        if(bytesReceived >= BUF_SIZE){
            memset(receiveBuffer,0, BUF_SIZE);
            bytesReceived = 0;
        }  
            
        if(receiveBuffer[bytesReceived-1] == '\n'){
             receiveBuffer[bytesReceived-1] = 0;
             if(DEBUG)
             {
                BTSerial.print("Received via BT: ");
                BTSerial.println(receiveBuffer);
             }
             processUserInput(receiveBuffer);   
      
             memset(receiveBuffer,0, BUF_SIZE);
             bytesReceived = 0;       
        }
                   
  }
 
  
}


