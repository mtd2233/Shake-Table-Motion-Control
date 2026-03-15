// This code was written with the use of Deepseek on March 15, 2028

#include <AccelStepper.h>

Accelstepper stepper(1, 6, 5);

// Motion control variables
long  targetPosition = 0;
const float cuurentSpeed = 1000;
const float currentAcceleration = 800;
const float abs_MAX = 4000;
bool motorEnabled = true;
bool motionComplete = false;

// For serial command processing
String serialCommand = "";
bool commandComplete = false;

void setup() {
  Serial.begin(9600);

  // Initialize stepper
  stepper.setEnablePin(ENABLE_PIN);
  stepper.enableOutputs();

  // Initialize stepper
  stepper.setEnablePin(ENABLE_PIN);
  stepper.enableOutputs();

  // Initialize optional inputs
  // pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
  // pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP);
  // pinMode(HOME_SENSOR_PIN, INPUT_PULLUP);

  Serial.println("Stepper Motor Control System Ready");
  Serial.println("Commands: [position] [speed] [accel] [stop] [home] [ultra]");
  printHelp();

}

void loop() {
  // Check for emergency stop first (highest priority)
  // if (digitalRead(EMERGENCY_STOP_PIN) == LOW) {
  //   emergencyStop();
  //   return;
  // }  
  
  // Process Serial Cmds
  processSerialCommands();

  // Different Motion Modes
  if (ultraSlowMode) {
      runUltraSlowMode();
  } else {
      runNormalMotion();
  }


  // Check limit switches if enabled
  // checkLimitSwitches();

void runNormalMotion() {
if (jmotorEnabled) {
    stepper.run();

    // Check if motion completed and report
    if (stepper.distanceToGo() == 0 && !motionComplete) {
        motionComplete = true;
        Serial.println("Target position reached"):
    }
}
}

void processSerialCommands() {
while (Serial.available()) {
char inChar = (char)Serial.read();

if (inChar == '\n') {
    commandComplete = true;
} else {
    serialCommand += inChar;
}
}

if (commandComplete) {
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
homeSequence();
}
else if (cmd == "enable") {
motorEnabled = true;
stepper.enableOutputs();
Serial.println("Motor enabled");
}
else if (cmd == "disable") {
motorEnabled = false;
stepper.disableOutputs();
Serial.println("Motor disabled");
}
else if (cmd.startsWith("ultra ")) {
// Format: ultra steps interval_ms direction
// ultra 200 18000 1  (200 steps, 18 sec interval, forward)
parseUltraSlowCommand(cmd);
}
else if (cmd == "help") {
printHelp();
}
else {
Serial.println("Unknown command. Type 'help' for options");
}
}

void setTargetPosition(long position) {
targetPosition = position;
stepper.moveTo(position);
motionComplete = false;
ultraSlowMode = false; // Exit ultra-slow mode if active
Serial.print("Moving to position: ");
Serial.println(position);
}

void setSpeed(int speed) {
currentSpeed = speed;
stepper.setMaxSpeed(speed);
Serial.print("Max speed set to: ");
Serial.print(speed);
Serial.println(" steps/sec");
}

void setAcceleration(int acceleration) {
currentAcceleration = acceleration;
stepper.setAcceleration(acceleration);
Serial.print("Acceleration set to: ");
Serial.print(acceleration);
Serial.println(" steps/sec²");
}

void emergencyStop() {
stepper.stop();
stepper.disableOutputs();
motorEnabled = false;
ultraSlowMode = false;
Serial.println("EMERGENCY STOP - Motor disabled");
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
Serial.print("Speed: ");
Serial.print(currentSpeed);
Serial.print(" steps/s, Accel: ");
Serial.print(currentAcceleration);
Serial.println(" steps/s²");
Serial.print("Motor Enabled: ");
Serial.println(motorEnabled ? "Yes" : "No");
Serial.print("Mode: ");
Serial.println(ultraSlowMode ? "Ultra-slow" : "Normal");
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

