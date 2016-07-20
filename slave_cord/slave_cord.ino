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
int timeState =-1; 
#define SETUP 1
#define READ_WRITE 2
#define END_READ 3


const int writePin = 4;
const int readPin = 2; //on uno can only be 2 or 3

const int lOut = 6;
const int sOut = 5;
const int saberRead = 3; // can only be 2 or 3 for interrupt;
const int usDelay = 10;
const float accelThreshold = 1.7; 
volatile int timeStep = 0;
volatile bool synchronized = false;
volatile int wire_State;
int data;
int opState =0; 
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
bool pulseBuffer = false; 
bool goodConnection = false; 
bool hasBeenWritten = false;
int timeSinceRestart = 0;

void readISR() {
  if (!ignoreInterrupt) {
    synchronized = true;
    timeStep = 0;
    timeSinceRestart = 0;
    ; 
  }
}

//void readSaber() {
//  //sendPulse = true;
//  if (!ignoreSaberInterrupt) {
//    sendPulse = true;
//  }
//}


//1 // timeStep: 0 -> A0: setup S High, L Low, disable interrupt, no writing to master

//2 // timeStep 20 -> B0: setup S low, B High,
// timeStep 27 -> B1: enable interrupt, if high, send to master, (-> then disable interrupt so that only read once?)
// timeStep 33 -> B2: disable interrupts
//3 // timeStep 40 -> C0: setup S Low, B low
// timeStep 47 -> C1: enable interrupts, if High, send to master
// timeStep 53 -> C2: disable interrupts.
//4 // timeStep 60 -> D0: setup S Low, B low (probably unecessary)
// timeStep 67 -> D1: enable interrupts, if High, send to master
// timeStep 73 -> D2: disable interrupts.
// timeStep 80 -> D3: timeStep = -1;
int connectionCount = 0;
void loop() {

  //unsigned long startTime = micros();

  if (synchronized) {
    int t = timeStep;

    ////////////
    // handle pulse buffer for scoring touch
    ///////////
    
    if(pulseBuffer && t >= D1 && t <= D2){
      sendPulse = true; 
       
      Serial.println("pulseBuffer used"); 
    } 
    else{  
    ///////////////////////////
    //read saber routine
    ///////////////////////////////
    if (!ignoreSaber) {
      for(int i = 0; i < 2; i++){ 
        int saberVal = digitalRead(saberRead);
        if(saberVal == 0){ 
          goodConnection = false; 
          break; 
        }
        else{ 
          goodConnection = true;  
        } 
      }
    } 

    
   ///////////
   // check timing of hit, & then check accelerometer.  
   ////////////
   
    if(goodConnection){ 
      goodConnection = false; 
       ///////////
       // if hit happens during "touch" period check accelerometer
       // simple version, just checks all three axis to see if they are above threshold. 
       // if above threshold, pulsBuffer is true; 
       if (timeStep >= D1 && timeStep <= D2) {
        Serial.println("timing correct"); 
          if (saberAccel.available()) {
            saberAccel.read();
            if (accelThreshold < abs(saberAccel.cx) 
            || accelThreshold < abs(saberAccel.cy) 
            || accelThreshold < abs(saberAccel.cz) ) {
              pulseBuffer = true; 
              Serial.println("passed threshold"); 
            }
            else{ 
              pulseBuffer = false; 
            }
          }
        }
      else{ 
        
      Serial.print("bad Sync "); 
      Serial.println(timeStep);
        sendPulse = false; //  true;
      }
     }
    } 
    
    
    ////////////////////////////
    // Send pulse routine 
    /////////////////////////////
    if (!ignoreSaber && sendPulse) {
      Serial.println("sending");
        //noInterrupts();
        ignoreInterrupt = true;
        ignoreSaber = true;
        digitalWrite(writePin, HIGH);
        sendPulse = false;
        pulseBuffer = false;
        delayMicroseconds(usDelay);
        digitalWrite(writePin, LOW);
        //delay(9);
        
        ignoreInterrupt = false;


        // Serial.println(timeStep);
      }

    //////////////////////////////////
    // Set pins routine
    ////////////////////////////////// 
//1 // timeStep: 0 -> A0: setup S High, L Low, disable interrupt, no writing to master

//2 // timeStep 20 -> B0: setup S low, B High,
// timeStep 27 -> B1: enable interrupt, if high, send to master, (-> then disable interrupt so that only read once?)
// timeStep 33 -> B2: disable interrupts
//3 // timeStep 40 -> C0: setup S Low, B low
// timeStep 47 -> C1: enable interrupts, if High, send to master
// timeStep 53 -> C2: disable interrupts.
//4 // timeStep 60 -> D0: setup S Low, B low (probably unecessary)
// timeStep 67 -> D1: enable interrupts, if High, send to master
// timeStep 73 -> D2: disable interrupts.
// timeStep 80 -> D3: timeStep = -1;
    
    // Saber set HIGH, Lame Low
    // Saber does not read.
    if(timeStep >= A0 && timeStep < B0 && A0 != timeState){ 
      ignoreSaber = true;
      digitalWrite(sOut, HIGH);
      digitalWrite(lOut, LOW);
      timeState = A0; 
    } 

      //saber out Set LOW, Lame set HIGH
    else if(timeStep >= B0 && timeStep < B1 && B0 != timeState ){ 
      digitalWrite(sOut, LOW);
      digitalWrite(lOut, HIGH);
      timeState = B0; 
    }
      // Allow saber read Interrrupt action
      // short period to
    else if(timeStep >= B1 && timeStep < B2 && B1 != timeState){ 
      // Serial.println("in self touch");
      ignoreSaber = false;
      timeState = B1; 
    } 
    else if(timeStep >= B2 && timeStep < C0 && B2 != timeState){ 
        ignoreSaber = true;
        timeState = B2;
    } 
    else if(timeStep >= C0 && timeStep < C1 && C0 != timeState ){ 
        digitalWrite(sOut, LOW);
        digitalWrite(lOut, LOW);
        timeState = C0;
    } 
    else if(timeStep >= C1 && timeStep < C2 && C1 != timeState){ 
        ignoreInterrupt = true;
        ignoreSaber = true;
        digitalWrite(writePin, HIGH);
        sendPulse = false;
        pulseBuffer = false;
        delayMicroseconds(usDelay);
        digitalWrite(writePin, LOW);
        //delay(9);
        
        ignoreInterrupt = false;
        Serial.println("c1 sent"); 
        ignoreSaber = false;

        timeState = C1; 
        
      
    } 
      else if(timeStep >= C2 && timeStep < D0 && C2 != timeState){
        ignoreSaber = true;
        timeState = C2;
      } 

      //    D0 operation unecessary because pin state in C0 and D0 is the same.
      //    case D0:
      //      digitalWrite(sOut, LOW);
      //      digitalWrite(lOut, LOW);
      //      break;

      else if(timeStep >= D1 && timeStep < D2 && D1 != timeState){
        ignoreSaber = false;
        timeState = D1;
      } 

     else if(timeStep >= D2 && timeStep < D3 && D2 != timeState){
        ignoreSaber = true;
        timeState = D2; 
     } 

      else if(timeStep >= D3){
        timeStep = -1;
        
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




