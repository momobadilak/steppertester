/*
  File steppertest1.ino

  License : The MIT License (MIT)

  Copyright (c) 2016 Andy Idsinga
  
  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the "Software"), 
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense, 
  and/or sell copies of the Software, and to permit persons to whom the 
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included 
  in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
  IN THE SOFTWARE.
*/

#include <Stepper.h>

/* 
   Motor related setup and constants

   Stepper class instance connected thusly
   arduino pin 4 > motor board AIN1
   arduino pin 5 > motor board AIN2
   adruino pin 6 > motor board BIN1
   arduino pin 7 > motor board BIN2
   
   Note, on the motor board there are two sets of outputs that go to the stepper
   motor, labeled "MOTORA" and "MOTORB" ..its unclear which pin is which regarding polarity
   or if this matters.
   Looking at the motor board with the VM+ pin at the upper left, this is how I'm wiring
   the motor outputs:
   motorA, top    : A-
   motorA, bottom : A+
   motorB, top    : B-
   motorB, bottom : B+
*/

const unsigned int MOTOR_A1_POS   = 4;
const unsigned int MOTOR_A2_NEG   = 5;
const unsigned int MOTOR_B1_POS   = 6;
const unsigned int MOTOR_B2_NEG   = 7;
const unsigned int MOTOR_STEPS    = 200;

Stepper stepper(MOTOR_STEPS,
		MOTOR_A1_POS,
		MOTOR_A2_NEG,
		MOTOR_B1_POS,
		MOTOR_B2_NEG);

/* other arduino pins mapped to local names */
const unsigned int SWITCH_ONOFF  = 3;

/*
  function protos for our project
 */
void stateMachineEntry();
void stateIdle();
void stateMotorControl();
void stateError();

/*
  misc protos
 */
void motorStandby();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(SWITCH_ONOFF,INPUT);

  /* 
     this is the "RPM" value - which doesn't appear to cause the actual RPM, but
     the delay time between steps. This could work out to the RPM if the motor
     physically supports it...

     I found this value through experimentation -- going much higher than this 
     for the ZJchao motor starts to do weird things. Caveat emptor!

     PS. it would be nice to figure out the exact math and mechanics of this
     but I'm in hacking mode at the moment...
  */
  stepper.setSpeed(200);
  
  transitionStateIdle();

  Serial.println("-setup()");
}

void loop() {

  stateMachineEntry();

  /* chores to do after each run of the state machine? */
}

/*
  coordinate various input events and actions via a simple
  finite state machine.
  This function is meant to be called from arduino's loop()
  and return to loop as fast as possible.
 */
const unsigned int ERRORSTATE        = 0;
const unsigned int IDLESTATE         = 1;
const unsigned int MOTORCONTROLSTATE = 2;

unsigned int gMainState              = IDLESTATE;

void stateMachineEntry() {

  switch(gMainState) {
  case IDLESTATE:
    stateIdle();
    break;
    
  case MOTORCONTROLSTATE:
    stateMotorControl();
    break;
    
  default:
    transitionStateError("weird, hit default case in stateMachineEntry()");
    break;
  }

  /* for debugging .. we can slow things down */
  //delay(500);
  
}


/*
  do all the things that need to be done
  on a transition into idle state
 */
void transitionStateIdle() {
  motorStandby();
  gMainState = IDLESTATE;
  Serial.println("switched off, now IDLE");
}

/* 
   in idle we're just checking to see what the state of the main
   ON / OFF switch is ..if it is on, we transition to MOTORCONTROLSTATE
   else we just hang out
*/
void stateIdle() {
  /* the switch is connected to arduino pin 3 */
  if(digitalRead(SWITCH_ONOFF) == HIGH) {
    return transitionStateMotorControl();
  }
}


/*
  do all the things that need to be done
  on a transition into idle state
 */
void transitionStateMotorControl() {
    /* transition to motor control state */
    gMainState = MOTORCONTROLSTATE;
    Serial.println("switched on, starting motor control");
}

void stateMotorControl() {

  /* short circuit if we need to transition states */
  if(digitalRead(SWITCH_ONOFF) == LOW) {
    return transitionStateIdle();
  }

  //Serial.println("step(200)");
  stepper.step(200);
}

/* 
   do all the stuff that has to happen
   when we transition into error state 
*/
void transitionStateError(const char* msg) {
  gMainState = ERRORSTATE;

  /* try to print the error message */
  Serial.println(msg);

  /* put the motor in standby to reduce current draw */
  motorStandby();
}

void stateError(const char* msg) {
  Serial.println("hanging...");
  while (1) {}
}

/*
  this function puts the motor in standby mode -- this is importnat and the current
  draw in this mode is minimal.
  see the table in the motor control IC datasheet
  https://www.adafruit.com/datasheets/TB6612FNG_datasheet_en_20121101.pdf
 */
void motorStandby() {
  Serial.println("+motorStandby");
  digitalWrite(MOTOR_A1_POS,LOW);
  digitalWrite(MOTOR_A2_NEG,LOW);
  digitalWrite(MOTOR_B1_POS,LOW);
  digitalWrite(MOTOR_B2_NEG,LOW);
}
