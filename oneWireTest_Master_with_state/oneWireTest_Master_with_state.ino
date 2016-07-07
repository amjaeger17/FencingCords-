//Master 

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

const int resetLength = 120;
const int cycleLength = 120; 

const int writePin = 13; 
const int readPin = 2; //on uno can only be 2 or 3

volatile int timeStep = 0; 
volatile int timeToReset =0; 
int data; 

void setup() {
 Timer1.initialize(8000); //(100hz);
 Timer1.attachInterrupt(timerISR);  
 
 pinMode(writePin, OUTPUT);
 digitalWrite(writePin, LOW);
 
 pinMode(readPin, INPUT);
 attachInterrupt(digitalPinToInterrupt(readPin), readISR, RISING); 

 Serial.begin(115200);
 Serial.println("Master V0.6 Started.");
}

volatile bool ignoreInterrupt = false;
volatile bool shouldRead = false;
void readISR() {
  shouldRead = false; 
  if (!ignoreInterrupt) {
    shouldRead = true;
  }
}

void loop() {
  int currentTimeStep = timeStep;
  if (shouldRead) {
    
    if (B1 <= currentTimeStep && B2>currentTimeStep) {
      Serial.println("SELF");
    } else if (currentTimeStep>=C1 && currentTimeStep<C2) {
      Serial.println(currentTimeStep);
      Serial.println("BEAT");
    } else if (currentTimeStep>=D1 && currentTimeStep<=D2) {
      Serial.println(currentTimeStep);
      Serial.println("TOUCH");
    } else {
      Serial.println(currentTimeStep);
    }
    shouldRead = false;
  }

  //delayMicroseconds(15); 
}

void timerISR() { 
  if (0==timeToReset) {
    //go high
    ignoreInterrupt = true;
    shouldRead = false; 
    digitalWrite(writePin, HIGH); 
  } else if (3==timeToReset) {
    //go low, then read every 2*timeStepSize
    digitalWrite(writePin, LOW);     
    //ignoreInterrupt = false;
  } else if(20 == timeToReset){ 
    ignoreInterrupt = false;  
  }
  else if (resetLength==timeToReset) {
    timeStep = -1;
    timeToReset = -1;
  }
  if(cycleLength == timeStep){ 
    timeStep = -1; 
  }
  timeStep++;
  timeToReset++;
}
