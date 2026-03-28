// This code was written with the use of Deepseek on March 15, 2028

#include <AccelStepper.h>

AccelStepper stepper(1, 6, 5);

#define ENABLE_PIN 7

// Motion control variables
long  targetPosition = 3200;
float currentSpeed = 2000;
float currentAcceleration = 800;
const float abs_MAX = 4000;
bool motorEnabled = true;
bool motionComplete = false;

// For back-and-forth motion
bool movingForward = true;
bool backAndForthEnabled = false;
unsigned long lastPositionChangeTime = 0;
const unsigned long PAUSE_BETWEEN_MOVE = 1000; // 1sec pause at each end

// For serial command processing
String serialCommand = "";
bool commandComplete = false;

// Function prototypes (declare all functions before they're used)
void processSerialCommands();
void parseCommand(String cmd);
void runNormalMotion();
void runBackAndForthMotion();
void setTargetPosition(long position);
void setSpeed(int speed);
void setAcceleration(int acceleration);
void emergencyStop();
void startBackAndForth();
void stopBackAndForth();
void checkLimitSwitches();
void printStatus();
void printHelp();

void setup() {
  Serial.begin(9600);

  // Configure step and direction pins (required for AccelStepper)
  pinMode(6, OUTPUT);   // PUL Pin
  pinMode(5, OUTPUT);   // DIR Pin
  pinMode(ENABLE_PIN, OUTPUT);

  // Initialize stepper
  stepper.setEnablePin(ENABLE_PIN);
  stepper.enableOutputs();

  // Set default motion parameters
  stepper.setMaxSpeed(currentSpeed);
  stepper.setAcceleration(currentAcceleration);

  // Initialize optional inputs
  // pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
  // pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP);
  // pinMode(HOME_SENSOR_PIN, INPUT_PULLUP);

  // Set initial target position
  stepper.moveTo(targetPosition);

  Serial.println("Stepper Motor Control System Ready");
  Serial.println("Type 'help' for available commands");
  printHelp();

  printStatus();
}

void loop() {
  // Process Serial Cmds
  processSerialCommands();

  // Different Motion Modes
  // Run appropriate motion mode
  if (backAndForthEnabled) {
    runBackAndForthMotion();
  } else {
    runNormalMotion();
  }

  // Provide periodic status updates (every 5 seconds)
  static unsigned long lastStatusTime = 0;
  if (millis() - lastStatusTime > 500000) {
    printStatus();
    lastStatusTime = millis();
  }

  // Check limit switches if enabled
  // checkLimitSwitches();

}

void runNormalMotion() {
    if (motorEnabled) {
        stepper.run();

    // Check if motion completed and report
    if (stepper.distanceToGo() == 0 && !motionComplete) {
        motionComplete = true;
        Serial.println("Target position reached");
        printStatus(); // Shows final position
    }
  }
}

void runBackAndForthMotion() {
  if (!motorEnabled) 
    return;

  stepper.run();

  // Check if reach current target
  if (stepper.distanceToGo() = 0 ) {
    if (millis()- lastPositionChangeTime >= PAUSE_BETWEEN_MOVE) {
      // Toggle dir
      if (movingForward) {
        // Reached forward position, now go back
        stepper.moveTo(0);
        movingForward = true;
        Serial.println("Reach forward limit - moving back to 0");
      } else {
        // Reached back position, now go forward
        stepper.moveTo(targetPosition);
        movingForward = true;
        Serial.println("Reach back limit - moving forward");
      }
      lastPositionChangeTime = millis();
    }
  }
}

void processSerialCommands() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();

    // Handle both \n and \r\n from serial monitor
    if (inChar == '\n') {
      if (serialCommand.length() > 0) {
        commandComplete = true;
      }
    }
    else if (inChar == '\r') {
      // Ignore carriage return, wait for newline
      continue;
    }
    else {
      serialCommand += inChar;
    }
  }

  if (commandComplete) {
    // Echo the command back for confirmation
    Serial.print("Executing: ");
    Serial.println(serialCommand);

    parseCommand(serialCommand);
    serialCommand = "";
    commandComplete = false;
  }
}

// Implementation of Serial Cmds

void parseCommand(String cmd) {
  cmd.trim();
  cmd.toLowerCase();

  if (cmd.startsWith("move ")) {
    // Format: move 3200
    long pos = cmd.substring(5).toInt();
    setTargetPosition(pos);
  }
  else if (cmd.startsWith("speed ")) {
    // Format: speed 2000
    int spd = cmd.substring(6).toInt();
    setSpeed(spd);
  }
  else if (cmd.startsWith("accel ")) {
    // Format: accel 800
    int acc = cmd.substring(6).toInt();
    setAcceleration(acc);
  }
    else if (cmd == "backandforth" || cmd == "baf") {
    startBackAndForth();
  }
  else if (cmd == "stopbackandforth" || cmd == "stopbaf") {
    stopBackAndForth();
  }
  else if (cmd == "stop") {
    emergencyStop();
  }
  else if (cmd == "home") {
    Serial.println("Homing sequence not impletmented yet");
  }
  else if (cmd == "enable") {
    motorEnabled = true;
    stepper.enableOutputs();
    Serial.println("Motor enabled");
    printStatus();
  }
  else if (cmd == "disable") {
    motorEnabled = false;
    stepper.disableOutputs();
    Serial.println("Motor disabled");
    printStatus();
  }
  else if (cmd == "status") {
    printStatus();
  }
  else if (cmd == "help") {
    printHelp();
  }
  else {
    Serial.println("Unknown command. Type 'help' for options");
  }
}

void setTargetPosition(long position) {
  // Stop back-and-forth mode if it's running
  if (backAndForthEnabled) {
    stopBackAndForth();
  }

  // Re-enable motor if it was disabled
  if (!motorEnabled) {
    motorEnabled = true;
    stepper.enableOutputs();
    Serial.println("Auto-enabling motor for movement");
  }

  targetPosition = position;
  stepper.moveTo(position);
  motionComplete = false;

  
  Serial.print("Moving to position: ");
  Serial.println(position);
  Serial.print("Distance to go: ");
  Serial.println(stepper.distanceToGo());
}

void setSpeed(int speed) {
  if (speed > 0 && speed <= abs_MAX) {
    currentSpeed = speed;
    stepper.setMaxSpeed(speed);
    Serial.print("Max speed set to: ");
    Serial.print(speed);
    Serial.println(" steps/sec");
  } else {
    Serial.print("Speed must be between 1 and ");
    Serial.println(abs_MAX);
  }
}

void setAcceleration(int acceleration) {
  if (acceleration > 0) {
    currentAcceleration = acceleration;
    stepper.setAcceleration(acceleration);
    Serial.print("Acceleration set to: ");
    Serial.print(acceleration);
    Serial.println(" steps/sec²");
  } else {
    Serial.println("Acceleration must be greater than 0");
  }
}

void startBackAndForth() {
  if (backAndForthEnabled) {
    Serial.println("Back-and-forth motion already running");
    return;
  }
  
  // Re-enable motor if it was disabled
  if (!motorEnabled) {
    motorEnabled = true;
    stepper.enableOutputs();
    Serial.println("Auto-enabling motor for back-and-forth motion");
  }
  
  backAndForthEnabled = true;
  movingForward = true;
  lastPositionChangeTime = millis();
  
  // Start moving to target position
  stepper.moveTo(targetPosition);
  motionComplete = false;
  
  Serial.println("Back-and-forth motion enabled");
  Serial.print("Moving between 0 and ");
  Serial.print(targetPosition);
  Serial.println(" steps");
}

void stopBackAndForth() {
  if (!backAndForthEnabled) {
    return;
  }
  
  backAndForthEnabled = false;
  stepper.stop();
  
  Serial.println("Back-and-forth motion stopped");
  Serial.print("Stopped at position: ");
  Serial.println(stepper.currentPosition());
}

void emergencyStop() {
  // Stop back-and-forth mode
  if (backAndForthEnabled) {
    stopBackAndForth();
  }

  stepper.stop();           // Stop motion immediately
  stepper.disableOutputs(); // Disable motor power
  motorEnabled = false;     // Prevent further movement until enabled

  Serial.println("EMERGENCY STOP - Motor disabled");
  Serial.print("Stopped at position: ");
  Serial.println(stepper.currentPosition());

  // Resets any pending motion
  motionComplete = true;
}

void checkLimitSwitches() {
  // Add limit switch checking logic here
  /*
  if (digitalRead(LIMIT_SWITCH_PIN) == LOW) {
  emergencyStop();
  Serial.println("Limit switch triggered - Stopped");
  }
  */
}

void printStatus() {
  Serial.println("--- Status ---");
  Serial.print("Current Position: ");
  Serial.println(stepper.currentPosition());
  Serial.print("Target Position: ");
  Serial.println(targetPosition);
  Serial.print("Distance to Go: ");
  Serial.println(stepper.distanceToGo());
  Serial.print("Speed: ");
  Serial.print(currentSpeed);
  Serial.print(" steps/s, Accel: ");
  Serial.print(currentAcceleration);
  Serial.println(" steps/s²");
  Serial.print("Motor Enabled: ");
  Serial.println(motorEnabled ? "Yes" : "No");
  Serial.print("Motion Complete: ");
  Serial.println(motionComplete ? "Yes" : "No");
  Serial.print("Back-and-Forth Mode: ");
  Serial.println(backAndForthEnabled ? "Enabled" : "Disabled");
  Serial.println("-------------");
}

void printHelp() {
  Serial.println("Available Commands:");
  Serial.println("  move [steps]     - Move to absolute position");
  Serial.println("  speed [steps/s]  - Set maximum speed");
  Serial.println("  accel [steps/s²] - Set acceleration");
  Serial.println("  backandforth (baf)  - Start back-and-forth motion between 0 and target");
  Serial.println("  stopbackandforth    - Stop back-and-forth motion");
  Serial.println("  stop             - Emergency stop");
  Serial.println("  home             - Run homing sequence");
  Serial.println("  enable           - Enable motor");
  Serial.println("  disable          - Disable motor");
  Serial.println("  ultra [steps] [interval_ms] [dir] - Ultra-slow mode");
  Serial.println("  help             - Show this help");
}