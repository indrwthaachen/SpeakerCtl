/*
SpeakerCtl can control NuPro(R) Speakers with IR and optical feedback 

Copyright (c) 2014, Gerrit Wyen <gerrit.wyen@rwth-aachen.de>, Florian Heese <heese@ind.rwth-aachen.de>
Institute of Communication Systems and Data Processing
RWTH Aachen University, Germany

*/

void setInputUSB()
{
     irsend.sendNEC(0xBF08F7, 32);
     irsend.sendNEC(REPEAT, 0); 
     irrecv.enableIRIn();
}
     
 void setInputAUX()
 {
     irsend.sendNEC(0xBF8877, 32);
     irsend.sendNEC(REPEAT, 0); 
     irrecv.enableIRIn();
 }
     
 void setInputSPDIF()
 {
     irsend.sendNEC(0xBF8877, 32); // Set AUX
     irsend.sendNEC(REPEAT, 0); 
     delay(108);

     irsend.sendNEC(0xBF48B7, 32); // SPDIF two times -> skip optical
     irsend.sendNEC(REPEAT, 0); 
     delay(108);
     irsend.sendNEC(0xBF48B7, 32);
     irsend.sendNEC(REPEAT, 0); 
     
     irrecv.enableIRIn();
 }
 
 void toggleMute()
 {
     irsend.sendNEC(0xBF40BF, 32);
     irsend.sendNEC(REPEAT, 0); 
     irrecv.enableIRIn();
 }
 
 
 // TIME DIFFS; First Repeat 63ms, other Repeats 106

int sendIRVolumePlus()
{ 

   // +
   irsend.sendNEC(0xBF20DF, 32); 
   irsend.sendNEC(REPEAT, 0); 
   
}

int sendIRVolumeMinus()
{  
  //-
  irsend.sendNEC(0xBF10EF, 32);
  irsend.sendNEC(REPEAT, 0); 
}

