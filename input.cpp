/*
 * Handles all button inputs using interrupts and debouncing.
 */

#include "input.h"
#include "Arduino.h"
#include "config.h"
#include <EnableInterrupt.h> // Library for interrupts

// --- Debounce Time ---
// Ignores rapid, noisy signals from a button press.
// 300ms = ignore all signals for 0.3s after the first one.
#define BOUNCING_TIME 300

// Array of Arduino pin numbers for the buttons
uint8_t inputPins[NUM_BUTTONS] = {BUT01_PIN, BUT02_PIN, BUT03_PIN, BUT04_PIN};

// Flags to store which button was pressed.
// `true` = pressed, `false` = not pressed (or already processed).
bool buttonPressed[NUM_BUTTONS] = {false, false, false, false};

// Stores the time (in ms) of the last valid press for debouncing.
long lastButtonPressedTimestamps[NUM_BUTTONS];

// --- Interrupt Handler Functions ---

// Declare the main handler
void buttonHandler(int i);

// Wrapper functions for the interrupt.
// These call the main handler with the correct button index (0-3).
void buttonHandler0(){ buttonHandler(0); }
void buttonHandler1(){ buttonHandler(1); }
void buttonHandler2(){ buttonHandler(2); }
void buttonHandler3(){ buttonHandler(3); }

// Array of function pointers to store the handlers.
// Used to attach interrupts in a loop.
void (*buttonHandlers[NUM_BUTTONS])() = { 
  buttonHandler0, 
  buttonHandler1, 
  buttonHandler2, 
  buttonHandler3 
};

// Main Interrupt Service Routine (ISR).
// This function is called automatically when a button triggers an interrupt.
void buttonHandler(int i) {
  // Get current time
  long ts = millis(); 
  
  // Debounce logic:
  // Only accept the press if 300ms have passed since the last one.
  if (ts - lastButtonPressedTimestamps[i] > BOUNCING_TIME) {
    // This is a valid press.
    lastButtonPressedTimestamps[i] = ts; // Record the time
    buttonPressed[i] = true;             // Set the flag for the main loop
  }
}

// --- Setup Function ---

// This function is called once in setup().
void initInput() {
  // Loop through all buttons
  for (int i = 0; i < NUM_BUTTONS; i++) {
    
    // Set pin as an input and enable the internal pull-up resistor.
    // This keeps the pin HIGH until the button (wired to GND) is pressed.
    pinMode(inputPins[i], INPUT_PULLUP); 
    
    // Attach the interrupt.
    // Calls the handler when the pin signal FALLS (HIGH to LOW) on press.
    enableInterrupt(inputPins[i], buttonHandlers[i], FALLING); 
  }
}

// --- Helper Functions (used by the main loop) ---

// Resets all button flags to 'false'.
// Called by the main loop after processing inputs.
void resetInput() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttonPressed[i] = false;      
  }
}

// Checks if a specific button's flag is set to 'true'.
bool isButtonPressed(int buttonIndex) {
  return buttonPressed[buttonIndex];
}