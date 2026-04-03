#include <AccelStepper.h>

// Define stepper connections
// Using AccelStepper with DRIVER interface (1)
// STEP = pin 6, DIR = pin 5
AccelStepper stepper(1, 6, 5);

// Define enable pin for CL86T driver
#define ENABLE_PIN 7

// Shake table parameters
long shakeAmplitude = 4000;  // Number of steps for each direction (adjust as needed)
int shakeDelay = 500;         // Delay between direction changes (milliseconds)

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  Serial.println("Shake Table Controller Started");
  
  // Set up enable pin
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW);  // LOW typically enables CL86T driver
  
  // Configure stepper motor
  stepper.setMaxSpeed(2000);      // Maximum steps per second
  stepper.setAcceleration(500);    // Steps per second squared
  stepper.setSpeed(1000);          // Initial speed
  
  Serial.println("Motor configured. Type commands:");
  Serial.println("F - Forward");
  Serial.println("B - Backward");
  Serial.println("S - Shake mode (alternating directions)");
  Serial.println("E - Enable motor");
  Serial.println("D - Disable motor");
  Serial.println("+ - Increase amplitude");
  Serial.println("- - Decrease amplitude");
  Serial.println("> - Increase speed");
  Serial.println("< - Decrease speed");
}

void loop() {
  // Check for serial commands
  if (Serial.available()) {
    char command = Serial.read();
    
    switch (command) {
      case 'F':
      case 'f':
        Serial.println("Moving forward (CW)");
        moveForward();
        break;
        
      case 'B':
      case 'b':
        Serial.println("Moving backward (CCW)");
        moveBackward();
        break;
        
      case 'S':
      case 's':
        Serial.println("Starting shake mode...");
        shakeMode();
        break;
        
      case 'E':
      case 'e':
        digitalWrite(ENABLE_PIN, LOW);
        Serial.println("Motor enabled");
        break;
        
      case 'D':
      case 'd':
        digitalWrite(ENABLE_PIN, HIGH);
        stepper.stop();
        Serial.println("Motor disabled");
        break;
        
      case '+':
        shakeAmplitude += 500;
        Serial.print("Amplitude increased to: ");
        Serial.println(shakeAmplitude);
        break;
        
      case '-':
        shakeAmplitude = max(500, shakeAmplitude - 500);
        Serial.print("Amplitude decreased to: ");
        Serial.println(shakeAmplitude);
        break;
        
      case '>':
        stepper.setMaxSpeed(stepper.maxSpeed() + 200);
        Serial.print("Max speed increased to: ");
        Serial.println(stepper.maxSpeed());
        break;
        
      case '<':
        stepper.setMaxSpeed(max(500, stepper.maxSpeed() - 200));
        Serial.print("Max speed decreased to: ");
        Serial.println(stepper.maxSpeed());
        break;
        
      default:
        Serial.println("Unknown command. Use F, B, S, E, D, +, -, >, <");
        break;
    }
  }
  
  // Run the stepper (must be called frequently)
  stepper.run();
}

// Function to move forward (CW)
void moveForward() {
  stepper.enableOutputs();
  stepper.moveTo(shakeAmplitude);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  stepper.disableOutputs();
}

// Function to move backward (CCW)
void moveBackward() {
  stepper.enableOutputs();
  stepper.moveTo(-shakeAmplitude);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  stepper.disableOutputs();
}

// Shake mode - alternates between forward and backward
void shakeMode() {
  stepper.enableOutputs();
  Serial.println("Shaking... Press any key to stop");
  
  long targetPosition = 0;
  bool direction = true;
  
  // Continue shaking until serial input is received
  while (!Serial.available()) {
    if (direction) {
      targetPosition = shakeAmplitude;
    } else {
      targetPosition = -shakeAmplitude;
    }
    
    stepper.moveTo(targetPosition);
    
    // Run until reaching target position
    while (stepper.distanceToGo() != 0 && !Serial.available()) {
      stepper.run();
    }
    
    // Change direction
    direction = !direction;
    
    // Small pause at extremes
    delay(shakeDelay);
    
    // Check for stop command during shake
    if (Serial.available()) {
      break;
    }
  }
  
  // Clear any waiting serial input
  while (Serial.available()) {
    Serial.read();
  }
  
  stepper.stop();
  Serial.println("Shake mode stopped");
  stepper.disableOutputs();
}