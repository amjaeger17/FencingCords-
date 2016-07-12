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
#define D2 77
#define D3 80


#define WAITING 0
#define GREEN_HIT 1
#define RED_HIT 2
#define BOTH_HIT 3

#define GREEN_SCORE 4 
#define RED_SCORE 5 
#define BOTH_SCORE 6 
#define V_PAUSE 7 
#define RESET 8 
int state =0 ; 


const int beatLockout = 30; // milliseconds
const int hitLockOut = 300;// 120milliseconds = 12000 microseconds. isr increments once ever 400 microseconds. ;
volatile long pastTime = 0; 
volatile long beatLoutOutCount = 0;
bool isBeatLockOut = false;
bool redHitFirst = false;
bool greenHitFirst = false;
bool redHit = false;
bool greenHit = false;
bool greenPoint = false;
bool redPoint = false;
bool scoreMode = false; 

const int redLight = 7;
const int greenLight = 8;
const int buzzer = 9;
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
  Timer1.initialize(400); //(2.5Khz) -  isr is in microseconds;
  Timer1.attachInterrupt(timerISR);

  pinMode(writePinLeft, OUTPUT);
  digitalWrite(writePinLeft, LOW);

  pinMode(readPinLeft, INPUT);
  attachInterrupt(digitalPinToInterrupt(readPinLeft), readISRLeft, RISING);

  pinMode(writePinRight, OUTPUT);
  digitalWrite(writePinRight, LOW);

  pinMode(readPinRight, INPUT);
  attachInterrupt(digitalPinToInterrupt(readPinRight), readISRRight, RISING);

  pinMode(redLight, OUTPUT);
  digitalWrite(redLight, LOW);

  pinMode(greenLight, OUTPUT);
  digitalWrite(greenLight, LOW);

  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);


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
      if(!greenHit){ 
      greenHit = true;
      } 
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
        if(!redHit){ 
          redHit = true; 
        }
      
    } else if (currentTimeStep >= D1 && currentTimeStep <= D2) {
      Serial.println("SELF RIGHT");
    } else {
      Serial.println("aaaa");
      Serial.println(currentTimeStep);
    }
    shouldReadRight = false;
  }

  switch (state){ 
    case WAITING: 
      //Serial.println("In Case Waiting"); 
      if(redHit && greenHit){ 
          state = BOTH_SCORE;  
      } 
      else if(redHit && !greenHit) { 
        state = RED_HIT;
        pastTime = millis(); 
        //digitalWrite(leftLight, HIGH); 
        
      }
      else if(!redHit && greenHit){
         state = GREEN_HIT;
         pastTime = millis();
         Serial.println("green Score");
        // digitalWrite(rightLight, HIGH);  
        
      }
      else{ 
        digitalWrite(greenLight, LOW);
        digitalWrite(redLight, LOW);
        break; 
      }
      break; 

//--------------------------------
     case GREEN_HIT: 
        if(pastTime < hitLockOut){ 
          if (redHit){ 
            state = BOTH_SCORE;  
          }
        }
        else {  
          state = GREEN_SCORE;
          Serial.println("going to green_score"); 
        }
       break; 

//-------------------------------------
      case RED_HIT: 
        if(pastTime < hitLockOut){ 
          if (greenHit){ 
            state = BOTH_SCORE; 
          }
        }
        else { 
          state = RED_SCORE;
        }
       break;

//---------------------------
    case BOTH_SCORE:
      digitalWrite(greenLight, HIGH);
      digitalWrite(redLight, HIGH);
      digitalWrite(buzzer, HIGH);
      state = V_PAUSE;
      pastTime =0;   
      break; 
      
//---------------------------------
    case GREEN_SCORE: 
    Serial.println("in green score"); 
     digitalWrite(greenLight, HIGH);
     digitalWrite(buzzer, HIGH);
     state = V_PAUSE;
     pastTime =0;
     break; 

//---------------------------------
    case RED_SCORE: 
     digitalWrite(redLight, HIGH);
     digitalWrite(buzzer, HIGH);
     state = V_PAUSE;
     pastTime =0;
     break; 

//-------------------------------
    case V_PAUSE: 
      if (pastTime >= 5000){ 
        pastTime = 0; 
        state = RESET;
      } 
        break; 
//-------------------------------
    case RESET:
      digitalWrite(greenLight, LOW);
      digitalWrite(redLight, LOW);
      digitalWrite(buzzer, LOW);
      redHit = false; 
      greenHit = false; 
      state = WAITING; 
      }
  
   
}
/////////////////////////////////////////
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
  } else if (C0 + 2 == timeToReset) {
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
  pastTime++; 
}
