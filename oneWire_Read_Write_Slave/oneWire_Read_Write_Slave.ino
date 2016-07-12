//Slave

#include <TimerOne.h>
#include <Wire.h> // Must include Wire library for I2C
#include <SFE_MMA8452Q.h> // Includes the SFE_MMA8452Q library

//Declaring Accelerometer:
MMA8452Q saberAccel ;


#define A0 0
#define A1 7
#define A2 13

#define B0 20
#define B1 27
#define B2 33

#define C0 40
#define C1 47
#define C2 53

#define D0 60
#define D1 67
#define D2 73
#define D3 80

#define SETUP 1
#define READ_WRITE 2
#define END_READ 3


const int writePin = 4;
const int readPin = 2; //on uno can only be 2 or 3

const int lOut = 6;
const int sOut = 5;
const int saberRead = 3; // can only be 2 or 3 for interrupt;
const int usDelay = 10;

volatile int timeStep = 0;
volatile bool synchronized = false;
volatile int wire_State;
int data;

void setup() {
  Timer1.initialize(400); //(2.5khz);
  Timer1.attachInterrupt(timerISR);

  pinMode(writePin, OUTPUT);
  digitalWrite(writePin, LOW);

  pinMode(readPin, INPUT);
  pinMode(saberRead, INPUT);

  pinMode(sOut, OUTPUT);
  digitalWrite(sOut, LOW);
  pinMode(lOut, OUTPUT);
  digitalWrite(lOut, LOW);

  attachInterrupt(digitalPinToInterrupt(readPin), readISR, RISING);

  saberAccel.init();
  Serial.begin(115200);
  Serial.println("Slave V0.6 Started.");

}

volatile bool ignoreInterrupt = false;
volatile bool ignoreSaber = false;
volatile bool sendPulse = false;
bool hasBeenWritten = false;
int timeSinceRestart = 0;

void readISR() {
  if (!ignoreInterrupt) {
    synchronized = true;
    timeStep = 0;
    timeSinceRestart = 0;
  }
}

//void readSaber() {
//  //sendPulse = true;
//  if (!ignoreSaberInterrupt) {
//    sendPulse = true;
//  }
//}


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
int connectionCount = 0;
void loop() {

  //unsigned long startTime = micros();

  if (synchronized) {
    int t = timeStep;
    //Serial.println(t);
    //Serial.println(timeSinceRestart);
    if (!ignoreSaber) {
      int saberVal = digitalRead(saberRead);
      if (1 == saberVal) {
        if (2 > connectionCount) {
          connectionCount++;
        }
        else if (t >= D1 && t <= D2) {
          Serial.println("timeing is right") ;
          if (saberAccel.available()) {
            saberAccel.read();
            Serial.println("timing is right"); 
            //          Serial.print(saberAccel.cx, 3);
            //          Serial.print("\t");
            //             Serial.print(saberAccel.cy, 3);
            //              Serial.print("\t");
            //               Serial.print(saberAccel.cz, 3);
            //              Serial.println("\t");
            if (1.7 < abs(saberAccel.cx) || 1.7 < abs(saberAccel.cy) || 1.7 < abs(saberAccel.cz) ) {
              Serial.println("Heavy");
              sendPulse = true;
            }
            else{ 
              sendPulse = false; 
            }
          }
        }
        else {
          connectionCount = 0;
          sendPulse = true;
          Serial.println("f*** you"); 
        }
      }
    }

    if (!ignoreSaber && sendPulse) {
      Serial.println("sending");
        //noInterrupts();
        ignoreInterrupt = true;
        ignoreSaber = true;
        digitalWrite(writePin, HIGH);
        sendPulse = false;
        delayMicroseconds(usDelay);
        digitalWrite(writePin, LOW);
        //delay(9);
        ignoreInterrupt = false;


        // Serial.println(timeStep);
      }


    switch (t) {
    // Saber set HIGH, Lame Low
    // Saber does not read.
    case A0:
      ignoreSaber = true;
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
        // Serial.println("in self touch");
        ignoreSaber = false;
        break;

      case B2:
        ignoreSaber = true;
        break;

      case C0:
        digitalWrite(sOut, LOW);
        digitalWrite(lOut, LOW);
        break;

      case C1:
        ignoreSaber = false;
        break;

      case C2:
        ignoreSaber = true;
        break;

      //    D0 operation unecessary because pin state in C0 and D0 is the same.
      //    case D0:
      //      digitalWrite(sOut, LOW);
      //      digitalWrite(lOut, LOW);
      //      break;

      case D1:
        ignoreSaber = false;
        break;

      case D2:
        ignoreSaber = true;
        break;

      case D3:
        timeStep = -1;
        break;
    }
  } else {
    //unsynchronized
    Serial.print(".");
  }
  if (600 < timeSinceRestart) {
    synchronized = false;
  }
  //unsigned long endTime = micros();
}

void timerISR() {
  timeStep++;
  timeSinceRestart++;
}




