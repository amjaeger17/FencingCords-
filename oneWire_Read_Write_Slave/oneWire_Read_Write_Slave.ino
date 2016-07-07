//Slave

#include <TimerOne.h>


#define A0  0

#define B0 30
#define B1 35
#define B2 55

#define C0 60
#define C1 65
#define C2 85

#define D0 90
#define D1 95
#define D2 115
#define D3 120

#define SETUP 1
#define READ_WRITE 2
#define END_READ 3


const int writePin = 13;
const int readPin = 2; //on uno can only be 2 or 3

const int lOut = 5;
const int sOut = 4;
const int saberRead = 3; // can only be 2 or 3 for interrupt;
const int usDelay = 10;

volatile int timeStep = 0;
volatile bool synchronized = false;
volatile int wire_State;
int data;

void setup() {
  Timer1.initialize(10000); //(100hz);
  Timer1.attachInterrupt(timerISR);

  pinMode(writePin, OUTPUT);
  digitalWrite(writePin, LOW);

  pinMode(readPin, INPUT);
  pinMode(saberRead, INPUT);
  attachInterrupt(digitalPinToInterrupt(readPin), readISR, RISING);
  attachInterrupt(digitalPinToInterrupt(saberRead), readSaber, RISING);
  Serial.begin(115200);
  Serial.println("Slave V0.6 Started.");
}

volatile bool ignoreInterrupt = false;
volatile bool ignoreSaberInterrupt = false;
bool sendPulse = false;
bool hasBeenWritten = false;


void readISR() {
  if (!ignoreInterrupt) {
    synchronized = true;
    timeStep = 0;
  }
}

void readSaber() {
  if (!ignoreSaberInterrupt) {
    sendPulse = true;
  }
}


//1 // timeStep: 0 -> A0: setup S High, L Low, disable interrupt, no writing to master

//2 // timeStep 30 -> B0: setup S low, B High,
// timeStep 35 -> B1: enable interrupt, if high, send to master, (-> then disable interrupt so that only read once?)
// timeStep 55 -> B2: disable interrupts
//3 // timeStep 60 -> C0: setup S Low, B low
// timeStep 65 -> C1: enable interrupts, if High, send to master
// timeStep 85 -> C2: disable interrupts.
//4 // timeStep 90 -> D0: setup S Low, B low (probably unecessary)
// timeStep 95 -> D1: enable interrupts, if High, send to master
// timeStep 115 -> D2: disable interrupts.
// timeStep 120 -> D3: timeStep = -1;

void loop() {
  if (synchronized) {
    int t = timeStep;

    Serial.println(t);
    if (sendPulse) {
      ignoreInterrupt=true;
      digitalWrite(writePin, HIGH);
      sendPulse = false;
      delayMicroseconds(usDelay);
      digitalWrite(writePin, LOW);
      ignoreInterrupt=false;
    }
    switch (t) {

      // Saber set HIGH, Lame Low
      // Saber does not read.
      case A0:
        ignoreSaberInterrupt = true;
        digitalWrite(sOut, HIGH);
        digitalWrite(lOut, LOW);
        break;

      //saber out Set LOW, Lame set HIGH
      case B0:
        digitalWrite(sOut, LOW);
        digitalWrite(lOut, HIGH);
        break;

      // Allow saber read Interrrupt action
      // short period to
      case B1:
        ignoreSaberInterrupt = false;
        break;

      case B2:
        ignoreSaberInterrupt = true;
        break;

      case C0:
        digitalWrite(sOut, LOW);
        digitalWrite(lOut, LOW);
        break;

      case C1:
        ignoreSaberInterrupt = false;
        break;

      case C2:
        ignoreSaberInterrupt = true;
        break;

      //    D0 operation unecessary because pin state in C0 and D0 is the same.
      //    case D0:
      //      digitalWrite(sOut, LOW);
      //      digitalWrite(lOut, LOW);
      //      break;

      case D1:
        ignoreSaberInterrupt = false;
        break;

      case D2:
        ignoreSaberInterrupt = true;
        break;

      case D3:
        timeStep = -1;
        break;
    }
  } else {
    //unsynchronized
    //Serial.print(".");
  }
}

void timerISR() {
  timeStep++;
}




