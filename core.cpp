/*
 * This file contains the main game logic (the "core")
 * for the "Turn on the Sequence!" (TOS) game.
 * It manages game states, timers, scoring, and hardware.
 */

#include "HardwareSerial.h"
#include "core.h"
#include "Arduino.h"
#include "kernel.h"      // For state management (changeState(), etc.)
#include "input.h"       // For button presses (isButtonPressed(), etc.)
#include "config.h"      // For pin definitions and game settings (T1, etc.)
#include "LiquidCrystal_I2C.h" // For the LCD screen
#include <avr/sleep.h>  // For deep sleep mode
#include <time.h>       // Not strictly used, but good for random

// --- Global Game Variables ---
int ledPins[NUM_LED] = {LED01_PIN, LED02_PIN, LED03_PIN, LED04_PIN};
float F = 0.15;         // Factor to reduce time (set by difficulty)
int level = 1;          // Difficulty level (1-4)
int sequence[NUM_BUTTONS]; // Array to store the correct sequence
int playerComb[NUM_BUTTONS]; // Array to store the player's inputs
int playerIndex = 0;    // How many buttons the player has pressed this round
unsigned long timeAvailable = T1; // Max time for the round (starts at T1)
unsigned long roundStartTime = 0; // When the player's turn started
int score = 0;
bool lost = false;      // Flag to check if the game was lost

// --- Static variables for non-blocking timers ---
static unsigned long lastFadeTime = 0;    // For the intro LED fade
static unsigned long lastPotReadTime = 0; // (Not used, but was here)
static unsigned long waitStartTime = 0;   // When a non-blocking wait starts
static unsigned long waitDuration = 0;  // How long the wait should be
static int nextState = INTRO_STATE;     // Where to go after a wait finishes

// Initialize the LCD library (address 0x27, 16 chars, 4 lines)
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 4);

// Empty function, needed for sleep mode to wake up
void wakeUp() {}

// --- Setup ---
// Called once at the beginning
void initCore() {
  Serial.begin(9600); // Start serial comms (for debugging)
  randomSeed(analogRead(A1)); // Seed the random number generator

  // Set all LED pins to OUTPUT mode
  pinMode(LED01_PIN, OUTPUT);
  pinMode(LED02_PIN, OUTPUT);
  pinMode(LED03_PIN, OUTPUT);
  pinMode(LED04_PIN, OUTPUT);
  pinMode(LEDS_PIN, OUTPUT); // The red LED

  lcd.init();      // Initialize the LCD
  lcd.backlight(); // Turn on the LCD backlight
}

// === NON-BLOCKING WAIT FUNCTIONS ===

// Helper function to start a wait
// Tells the game to pause (go to STATE_WAIT) for 'duration' ms,
// and then go to 'stateAfterWait'.
void wait(unsigned long duration, int stateAfterWait) {
  waitStartTime = millis();    // Record the start time
  waitDuration = duration;     // Save how long to wait
  nextState = stateAfterWait;  // Save where to go next
  changeState(STATE_WAIT);   // Change to the waiting state
}

// The actual WAIT state (called by the main loop)
// This function just checks if the wait time is over.
void state_wait() {
  // Check if (current time) - (start time) > (how long to wait)
  if (millis() - waitStartTime > waitDuration) {
    // Time is up!
    resetInput();        // Clear any accidental button presses
    changeState(nextState); // Go to the state we planned to
    return;
  }

  // Special case: "Game Over" wait
  // The red LED turns off after 2s, but the wait continues for 10s.
  if (nextState == INTRO_STATE) {
    if (millis() - waitStartTime > 2000) {
      analogWrite(LEDS_PIN, 0); // Turn off red LED
    }
  }
}


// --- GAME STATES ---

// INTRO_STATE: "Welcome" screen
void intro() {
  
  // This code runs only ONCE when entering this state
  if (isJustEnteredInState()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Welcome to TOS!");
    lcd.setCursor(0, 1);
    lcd.print("Press B1");
    score = 0; // Reset score
  }

  // Non-blocking fade for the red LED
  if (millis() - lastFadeTime > 15) {
    static int currIntensity = 0;
    static int fadeAmount = 5;
    analogWrite(LEDS_PIN, currIntensity);
    currIntensity = currIntensity + fadeAmount;
    if (currIntensity == 0 || currIntensity == 255) {
      fadeAmount = -fadeAmount; // Reverse fade direction
    }
    lastFadeTime = millis();
  }

  // Check for 10-second timeout
  if (getCurrentTimeInState() > MAX_TIME_IN_INTRO_STATE) {
    changeState(DEEP_SLEEP_STATE);
  
  // Check if Button 1 is pressed
  } else if (isButtonPressed(0)) { // Button 0 is B1
    lcd.clear();
    resetInput(); // Clear the button press
    changeState(SETTING_DIFFICULTY);
  }
}

// DEEP_SLEEP_STATE: Powers down the Arduino
void deep_sleep() {

  if (isJustEnteredInState()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.println("Going sleep mode");
    lcd.setCursor(0, 1);
    lcd.print("Press B1 to wake");
    analogWrite(LEDS_PIN, 0); // Turn off red LED
    delay(100); // Small delay to let LCD finish printing
  }

  // --- Enter Deep Sleep ---
  // The button interrupt (from input.cpp) is already set up to wake it.
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode(); // Arduino stops here until an interrupt

  // --- Waking up ---
  // Code execution resumes here after B1 is pressed
  sleep_disable();

  // Quick flash to show it woke up
  digitalWrite(LED01_PIN, HIGH);
  delay(100);
  digitalWrite(LED01_PIN, LOW);

  resetInput(); // Clear the wake-up button press
  lcd.clear();
  changeState(INTRO_STATE); // Go back to the intro
}

// Helper function to show difficulty on LCD
void displayDifficulty() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Difficulty : ");
  lcd.print(level);
  lcd.setCursor(0, 1);
  lcd.print("Fattore: "); // "Fattore" means "Factor"
  lcd.print(F);
}

// SETTING_DIFFICULTY state: Player uses potentiometer
void set_difficulty() {

  static int lastLevel = -1; // Remembers the last level shown
  static bool entryClickCleaned = false; // Flag for initial press

  // Runs once on state entry
  if (isJustEnteredInState()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set difficulty");
    lcd.setCursor(0, 1);
    lcd.print("Turn Pot & press B1");
    delay(500); // Wait to ignore the button press that entered this state
    resetInput();
    entryClickCleaned = true; // OK to read B1 now
  }

  // Read the potentiometer
  int analogValue = analogRead(POT_PIN);
  // Map the 0-1023 value to a 1-4 level
  level = map(analogValue, 0, 1023, 1, 4);

  // Set the time-reduction factor F based on the level
  switch (level) {
    case 1: F = 0.25; break;
    case 2: F = 0.50; break;
    case 3: F = 0.75; break;
    case 4: F = 1.00; break;
    default: F = 0.25; break;
  }

  // Only update the LCD if the level actually changed
  if (level != lastLevel) {
    displayDifficulty();
    lastLevel = level;
  }

  // Check if B1 is pressed (and the initial press was cleared)
  if (entryClickCleaned && isButtonPressed(0)) {
    resetInput();             // Clear this press
    analogWrite(LEDS_PIN, 0); // Turn off red LED
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.println("Go !");

    // Use the non-blocking wait to show "Go!" for 1 sec
    // and then go to STATE_START_ROUND
    wait(1000, STATE_START_ROUND);
  }
}

// GAME_STATE: Player is actively playing
void game_state() {
  
  unsigned long now = millis();

  // Check for T1 timeout
  if (now - roundStartTime > timeAvailable) {
    lost = true;
    lcd.clear();
    lcd.print("TEMPO SCADUTO"); // "TIME'S UP"
    delay(2000); // OK to use delay here, game is over
    changeState(FINAL_STATE);
    return;
  }

  // Read button inputs (this is non-blocking)
  playerInput();

  // Check if player has pressed all 4 buttons
  if (playerIndex >= NUM_BUTTONS) {
    
    if (checkCombination()) {
      // --- ROUND WIN ---
      score++;
      showGoodMessage();
      
      // Reduce the time for the next round
      timeAvailable = (unsigned long)((float)timeAvailable * (1.0f - F));
      
      // Wait 1.5s ("Good!" message) then start next round
      wait(1500, STATE_START_ROUND);
      return;

    } else {
      // --- ROUND LOSE (Wrong sequence) ---
      lost = true;
      lcd.clear();
      lcd.print("SEQUENZA SBAGLIATA!"); // "WRONG SEQUENCE!"
      delay(2000); // OK to use delay here
      changeState(FINAL_STATE);
      return;
    }
  }
}


// --- Game Logic Helper Functions ---

// Generates a new random sequence
void sequenceShuffle() {
  int arr[NUM_BUTTONS] = {1, 2, 3, 4};
  int temp, j;
  // Fisher-Yates shuffle algorithm
  for (int i = NUM_BUTTONS - 1; i > 0; i--) {
    j = random(0, i + 1);
    temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
  }
  // Copy the shuffled array to the global sequence
  for (int k = 0; k < NUM_BUTTONS; k++) {
    sequence[k] = arr[k];
  }
}

// Shows the sequence on the LCD
void showSequence() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sequenza: "); // "Sequence:"
  lcd.setCursor(0, 1);
  for (int i = 0; i < NUM_BUTTONS; i++) {
    lcd.print(sequence[i]);
    lcd.print(" ");
  }
  
  // Wait 3 seconds (showing the sequence)
  // then go to STATE_SHOW_SEQUENCE
  wait(3000, STATE_SHOW_SEQUENCE);
}

// STATE_SHOW_SEQUENCE: Runs *after* the 3s wait is over
void state_show_sequence() {
  
  // This state's only job is to prepare for the player's turn

  lcd.clear(); // Clear the sequence from the screen
    
  resetInput(); // Clear any buttons pressed *during* the wait
  
  // Turn off all green LEDs
  for (int i = 0; i < NUM_LED; i++) {
    digitalWrite(ledPins[i], LOW);
  }
  
  playerIndex = 0;           // Reset player's progress
  roundStartTime = millis(); // Start the T1 timer NOW
  lost = false;
  changeState(GAME_STATE); // Go to the main game state
}


// STATE_START_ROUND: Prepares the new round
void startNewRound() {
  if (lost) { // If you lost the last game
    score = 0;    // Reset score
    lost = false;
  }
  timeAvailable = T1; // Reset timer to its base value
  sequenceShuffle();  // Get a new sequence
  lcd.clear();
  showSequence();     // Show the sequence (which will start a 3s wait)
}

// Checks if the player's combination matches the sequence
bool checkCombination() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (playerComb[i] != sequence[i]) {
      return false; // Mismatch found
    }
  }
  return true; // All matched
}

// Reference to the variable in input.cpp
extern bool buttonPressed[NUM_BUTTONS];

// Reads player input (non-blocking)
void playerInput() {
  // Loop through all 4 buttons
  for (int i = 0; i < NUM_BUTTONS; i++) {
    
    // Check if the flag for this button is true
    if (isButtonPressed(i)) {
      
      // Only record if we haven't pressed 4 buttons yet
      if (playerIndex < NUM_BUTTONS) {
        // Save the press (button 0 -> 1, button 1 -> 2, etc.)
        playerComb[playerIndex] = i + 1;
        playerIndex++; // Move to the next spot in the array

        digitalWrite(ledPins[i], HIGH); // Turn on the corresponding LED

        // Reset ONLY the flag for the button that was pressed
        buttonPressed[i] = false;
      }
    }
  }
}

// Displays the "GOOD!" message after a successful round
void showGoodMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GOOD!");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);
}


// FINAL_STATE: Game Over screen
void finalize() {

  // Runs only ONCE on state entry
  if (isJustEnteredInState()) {
    
    // Turn off all green LEDs
    for (int i = 0; i < NUM_LED; i++) {
      digitalWrite(ledPins[i], LOW);
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Game Over");
    lcd.setCursor(0, 1);
    lcd.print("Final Score: ");
    lcd.print(score);
    
    analogWrite(LEDS_PIN, 255); // Turn on red LED (solid)
    
    // Wait for 10 seconds, then go back to the intro
    // The state_wait() function handles turning off the
    // red LED after 2 seconds.
    wait(10000, INTRO_STATE);
  }
}