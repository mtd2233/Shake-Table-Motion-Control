// This code was written with the use of Deepseek on March 15, 2028

#include <AccelStepper.h>

Accelstepper stepper(1, 6, 5);

// --- ADJUSTABLE PARAMETERS ---
float currentMaxSpeed = 200.0;
const float speedIncrement = 300.0;
const float ABSOLUTE_MAX = 4000.0; 
const float acceleration = 800.0;
long targetPosition = 30000;

enum State { MOVING, STOPPING, PAUSED };
State systemState = MOVING;
long savedTarget = 2000; 

void setup() {
  Serial.begin(9600);
  stepper.setMaxSpeed(currentMaxSpeed);
  stepper.setAcceleration(acceleration);
  stepper.moveTo(targetPosition);
  Serial.println("--- System Online ---");
  Serial.println("Commands: 's'=stop, 'r'=resume, [Number]=set speed");

}

void loop() {
  // 1. Handle Serial Commands
  if (Serial.available() > 0) {
    char firstChar = Serial.peek();
    
    if (isAlpha(firstChar)) {
      char cmd = Serial.read();
      if (cmd == 's' && systemState == MOVING) {
        savedTarget = stepper.targetPosition(); 
        stepper.stop();                         
        systemState = STOPPING;
        Serial.println("Stopping...");
      } 
      else if (cmd == 'r') {
        systemState = MOVING;
        stepper.moveTo(savedTarget);            
        Serial.println("Resuming!");
      }
    } 
    else if (isDigit(firstChar)) {
      float newSpeed = Serial.parseFloat();
      if (newSpeed > 0) {
        currentMaxSpeed = min(newSpeed, ABSOLUTE_MAX);
        stepper.setMaxSpeed(currentMaxSpeed);
        Serial.print("Speed Cap Updated: "); Serial.println(currentMaxSpeed);
      }
      // Correct way to clear the buffer of newlines/garbage
      while(Serial.available() > 0) { Serial.read(); }
    } else {
      // Discard unknown characters
      Serial.read();
    }
  }

  // 2. Movement Logic
  if (systemState == STOPPING) {
    if (stepper.distanceToGo() == 0) {
      systemState = PAUSED;
      Serial.println("Paused. Send 'r' to resume.");
    }
  } 
  else if (systemState == MOVING) {
    if (stepper.distanceToGo() == 0) {
      if (stepper.currentPosition() == 0) {
        currentMaxSpeed = min(currentMaxSpeed + speedIncrement, ABSOLUTE_MAX);
        stepper.setMaxSpeed(currentMaxSpeed);
        Serial.print("New Cycle Speed: "); Serial.println(currentMaxSpeed);
      }
      long nextPos = (stepper.currentPosition() == 0) ? targetPosition : 0;
      stepper.moveTo(nextPos);
      savedTarget = nextPos; // Update savedTarget so 'r' knows where to go
    }
  }

  stepper.run();

}
