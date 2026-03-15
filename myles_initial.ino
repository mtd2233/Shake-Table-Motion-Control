// This code was written with the use of Deepseek on March 15, 2028

#include <AccelStepper.h>

AccelStepper stepper(1, 6, 5);

void setup() {
  Serial.begin(9600);

  // --- Optional: Configure the Enable Pin ---
  // Uncomment these lines if you are using the ENABLE_PIN and want to control it.
  // stepper.setEnablePin(ENABLE_PIN);
  // stepper.enableOutputs(); // Enable the driver at start

  // --- Configure Motion Parameters ---
  // Set the maximum speed in steps per second.
  // Start with a lower value (e.g., 1000) and increase gradually.
  // Find the max reliable speed for your setup.
  stepper.setMaxSpeed(2000); // Steps per second

  // Set the acceleration in steps per second per second.
  // Crucial for smooth starts and stops, especially with a large motor [citation:4].
  stepper.setAcceleration(800); // Steps/s^2

  // Set the target position (in steps) for the motor to move to.
  // This will make the motor move 3200 steps (e.g., one full revolution if set to 3200 microsteps/rev).
  stepper.moveTo(3200);

  Serial.println("Setup complete. Motor will move to 3200 and back.");

}

void loop() {
  // This is the non-blocking run function. It must be called as frequently as possible.
  // It will step the motor if it needs to move to the target position.
  stepper.run();

  // Check if the motor has reached its current target position
  if (stepper.distanceToGo() == 0) {
    // If it has reached the target, reverse the direction for the next movement
    // by setting a new target opposite to the current position.
    stepper.moveTo(-stepper.currentPosition());

    // Optional: Print the new target for debugging
    Serial.print("Target reached. New target: ");
    Serial.println(stepper.targetPosition());
    
    // Small delay to visualize the stop before moving again
    delay(1000);
  }


}
