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

const int resetLength = 240;


const int writePin = 13; 
const int readPin = 2; //on uno can only be 2 or 3

volatile int timeStep = 0; 

int data; 

void setup() {
 Timer1.initialize(10000); //(100hz);
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
  if (!ignoreInterrupt) {
    shouldRead = true;
  }
}

void loop() {
  int currentTimeStep = timeStep;
  
  if (shouldRead) {
    if (currentTimeStep>=B1 && currentTimeStep<=B2) {
      Serial.println("SELF");
    } else if (currentTimeStep>=C1 && currentTimeStep<=C2) {
      Serial.println("BEAT");
    } else if (currentTimeStep>=D1 && currentTimeStep<=D2) {
      Serial.println("TOUCH");
    } else {
      Serial.println(timeStep);
    }
    shouldRead = false;
  }
}

void timerISR() { 
  if (0==timeStep) {
    //go high
    ignoreInterrupt = true;
    digitalWrite(writePin, HIGH); 
  } else if (3==timeStep) {
    //go low, then read every 2*timeStepSize
    digitalWrite(writePin, LOW);     
    ignoreInterrupt = false;
  } else if (resetLength==timeStep) {
    //timeStep = -1;
    timeStep = -1;
  }
  timeStep++;
}
