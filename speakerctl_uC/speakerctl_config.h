/*
SpeakerCtl can control NuPro(R) Speakers with IR and optical feedback 

Copyright (c) 2014, Gerrit Wyen <gerrit.wyen@rwth-aachen.de>, Florian Heese <heese@ind.rwth-aachen.de>
Institute of Communication Systems and Data Processing
RWTH Aachen University, Germany

*/


#define DEVICE_NAME "nupro01"

// Pin Configuration
const int TeensyLedPin =  13;
const int IRInputPin =  0;   
const int IRDiode = 10;

const int DETECT1 = A0;
const int DETECT2 = A1;
const int DETECT3 = A2;
const int DETECT4 = A3;
const int DETECT5 = A6;
const int DETECT6 = A7;


#define BTSerial Serial3

int signatureCompareThreshold = 166442;


// Light Signature for LIMIT Detection
int limitSignature[] = { 746, 1492, 3388, 3280, 3499, 1766 };
// ------

