// This code was written with the use of Deepseek on March 15, 2028

#include <AccelStepper.h>

AccelStepper stepper(1, 6, 5);

#define ENABLE_PIN 7

// Motion control variables
long  targetPosition = 0;
float currentSpeed = 1000;
float currentAcceleration = 800;
const float abs_MAX = 4000;
bool motorEnabled = true;
bool motionComplete = false;

// For serial command processing
String serialCommand = "";
bool commandComplete = false;

// Function prototypes (declare all functions before they're used)
void processSerialCommands();
void parseCommand(String cmd);
void runNormalMotion();
// void runUltraSlowMotion();
// void parseUltraSlowCommand(String cmd);
void setTargetPosition(long position);
void setSpeed(int speed);
void setAcceleration(int acceleration);
// void setupUltraSlowMotion(long steps, unsigned long intervalMs, int direction);
void emergencyStop();

void checkLimitSwitches();
void printStatus();
void printHelp();

void setup() {
  Serial.begin(9600);

  // Initialize stepper
  stepper.setEnablePin(ENABLE_PIN);
  stepper.enableOutputs();

  // Initialize stepper
  stepper.setMaxSpeed(currentSpeed);
  stepper.setAcceleration(currentAcceleration);

  // Initialize optional inputs
  // pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
  // pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP);
  // pinMode(HOME_SENSOR_PIN, INPUT_PULLUP);

  Serial.println("Stepper Motor Control System Ready");
  Serial.println("Type 'help' for available commands");
  printHelp();
}

void loop() {
  // Process Serial Cmds
  processSerialCommands();

  // Different Motion Modes
  runNormalMotion();

  // Provide periodic status updates (every 5 seconds)
  static unsigned long lastStatusTime = 0;
  if (millis() - lastStatusTime > 5000) {
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
    Serial.print("Speed must be between 1 and 4000");
    Serial.println(abs_MAX);
  }
}

void setAcceleration(int acceleration) {
  currentAcceleration = acceleration;
  stepper.setAcceleration(acceleration);
  Serial.print("Acceleration set to: ");
  Serial.print(acceleration);
  Serial.println(" steps/sec²");
}

void emergencyStop() {
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
  Serial.println("-------------");
}

void printHelp() {
  Serial.println("Available Commands:");
  Serial.println("  move [steps]     - Move to absolute position");
  Serial.println("  speed [steps/s]  - Set maximum speed");
  Serial.println("  accel [steps/s²] - Set acceleration");
  Serial.println("  stop             - Emergency stop");
  Serial.println("  home             - Run homing sequence");
  Serial.println("  enable           - Enable motor");
  Serial.println("  disable          - Disable motor");
  Serial.println("  ultra [steps] [interval_ms] [dir] - Ultra-slow mode");
  Serial.println("  help             - Show this help");
}
