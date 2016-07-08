//Master

#include <TimerOne.h>

#define A0 0
#define A1 5
#define A2 15

#define B0 20
#define B1 25
#define B2 38

#define C0 40
#define C1 45
#define C2 55

#define D0 60
#define D1 65
#define D2 78
#define D3 80

const int resetLength = D3; //Leave these the same for the moment, but in future allows for sync to be less often
const int cycleLength = D3;

const int writePinLeft  = 4;
const int readPinLeft = 2; //on uno can only be 2 or 3

const int writePinRight = 5;
const int readPinRight = 3;

volatile int timeStep = 0;
volatile int timeToReset = 0;
int data;

void setup() {
  Timer1.initialize(400); //(800hz);
  Timer1.attachInterrupt(timerISR);

  pinMode(writePinLeft, OUTPUT);
  digitalWrite(writePinLeft, LOW);

  pinMode(readPinLeft, INPUT);
  attachInterrupt(digitalPinToInterrupt(readPinLeft), readISRLeft, RISING);

  pinMode(writePinRight, OUTPUT);
  digitalWrite(writePinRight, LOW);

  pinMode(readPinRight, INPUT);
  attachInterrupt(digitalPinToInterrupt(readPinRight), readISRRight, RISING);

  Serial.begin(115200);
  Serial.println("Master V0.6 Started.");
}

volatile bool ignoreInterruptLeft = false;
volatile bool shouldReadLeft = false;

volatile bool ignoreInterruptRight = false;
volatile bool shouldReadRight = false;

void readISRLeft() {
  shouldReadLeft = false;
  if (!ignoreInterruptLeft) {
    shouldReadLeft = true;
  }
}


void readISRRight() {
  shouldReadRight = false;
  if (!ignoreInterruptRight) {
    shouldReadRight = true;
  }
}

void loop() {
  int currentTimeStep = timeStep;
  if (shouldReadLeft) {

    if (B1 <= currentTimeStep && B2 > currentTimeStep) {
      Serial.println("SELF LEFT");
    } else if (currentTimeStep >= C1 && currentTimeStep < C2) {
      Serial.println("BEAT LEFT");

    } else if (currentTimeStep >= D1 && currentTimeStep <= D2) {
      Serial.println("TOUCH LEFT");
    } else {
      Serial.println(currentTimeStep);
    }
    shouldReadLeft = false;
  }


  if (shouldReadRight) {
    if (A1 <= currentTimeStep && A2 > currentTimeStep) {
      Serial.println("BEAT RIGHT");
    } else if (B1 <= currentTimeStep && B2 > currentTimeStep) {
      Serial.println("TOUCH RIGHT");
    } else if (currentTimeStep >= D1 && currentTimeStep <= D2) {
      Serial.println("SELF RIGHT");
    } else {
      Serial.println("aaaa");
      Serial.println(currentTimeStep);
    }
    shouldReadRight = false;
  }
}

void timerISR() {

  //Reset for left side
  if (0 == timeToReset) {
    //go high
    ignoreInterruptLeft = true;
    shouldReadLeft = false;
    digitalWrite(writePinLeft, HIGH);
  } else if (3 == timeToReset) {
    digitalWrite(writePinLeft, LOW);
    ignoreInterruptLeft = false;

    //Reset for Right Side;
  } else if (C0 == timeToReset) {
    ignoreInterruptRight = true;
    shouldReadRight = false;
    digitalWrite(writePinRight, HIGH);
  } else if (C0+2 == timeToReset) {
    digitalWrite(writePinRight, LOW);
    ignoreInterruptRight = false;

  } else if (resetLength == timeToReset) {
    timeStep = -1;
    timeToReset = -1;
  }
  if (cycleLength == timeStep) {
    timeStep = -1;
  }
  timeStep++;
  timeToReset++;
}
