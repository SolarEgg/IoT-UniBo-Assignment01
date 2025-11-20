
#include "HardwareSerial.h"
#include "core.h"
#include "Arduino.h"
#include "kernel.h"      
#include "input.h"       
#include "config.h"      
#include "LiquidCrystal_I2C.h" 
#include <avr/sleep.h>  
#include <time.h>       

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
bool lost = false;      

// --- Static variables for  timers ---
static unsigned long lastFadeTime = 0;    // For the intro LED 
static unsigned long waitStartTime = 0;   
static unsigned long waitDuration = 0;  
static int nextState = INTRO_STATE;     

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 4);

// Empty function, needed for sleep mode to wake up
void wakeUp() {}

// --- Setup ---

// Called once at the beginning
void initCore() {
  Serial.begin(9600); 
  randomSeed(analogRead(A1)); 

  pinMode(LED01_PIN, OUTPUT);
  pinMode(LED02_PIN, OUTPUT);
  pinMode(LED03_PIN, OUTPUT);
  pinMode(LED04_PIN, OUTPUT);
  pinMode(LEDS_PIN, OUTPUT); 

  lcd.init();    
  lcd.backlight(); 
}

// === NON-BLOCKING WAIT FUNCTIONS ===

/* Helper function to start a wait
Tells the game to pause (go to STATE_WAIT) for 'duration' ms,
and then go to 'stateAfterWait'*/
void wait(unsigned long duration, int stateAfterWait) {
  waitStartTime = millis();    
  waitDuration = duration;     
  nextState = stateAfterWait;  
  changeState(STATE_WAIT);   
}

// This function just checks if the wait time is over.
void state_wait() {
  // Check if (current time) - (start time) > (how long to wait)
  if (millis() - waitStartTime > waitDuration) {
    resetInput();       
    changeState(nextState); 
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

  // fade for the red LED
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
  
  // Check if B1 is pressed
  } else if (isButtonPressed(0)) {

    lcd.clear();
    resetInput();
    changeState(SETTING_DIFFICULTY);
  }
}

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
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode(); 

  // --- Waking up ---
  if (isButtonPressed(0)) {
    sleep_disable();
    lcd.clear();
    resetInput(); 
    changeState(INTRO_STATE); //SETTING_DIFFICULTY
  }
}

void displayDifficulty() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Difficulty : ");
  lcd.print(level);
  lcd.setCursor(0, 1);
  lcd.print("Factor: "); // "Fattore" means "Factor"
  lcd.print(F);
}

// SETTING_DIFFICULTY by using potentiometer
void set_difficulty() {

  static int lastLevel = -1; 
  static bool entryClickCleaned = false; 

  if (isJustEnteredInState()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set difficulty");
    lcd.setCursor(0, 1);
    lcd.print("Turn Pot & press B1");
    delay(500); 
    resetInput();
    entryClickCleaned = true; 
  }

  
  int analogValue = analogRead(POT_PIN);
  // Map to translate 0-1023 into 1-4 level
  level = map(analogValue, 0, 1023, 1, 4);

  switch (level) {
 case 1: 
        F = 100.0;     
        timeAvailable = T1; 
        break;
case 2: 
        F = 150.0;     
        timeAvailable = T1 - 1000; 
        break;
case 3: 
        F = 250.0;     
        timeAvailable = T1 - 2000; 
        break;
case 4: 
        F = 500.0;    
        timeAvailable = T1 - 3000; 
        break;
default: 
        F = 250.0; 
        timeAvailable = T1; 
        break;
  }
  // Only update the LCD if the level actually changed
  if (level != lastLevel) {
    displayDifficulty();
    lastLevel = level;
  }

  if (entryClickCleaned && isButtonPressed(0)) {
    resetInput();             
    analogWrite(LEDS_PIN, 0); 
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.println("Go !");

    wait(1000, STATE_START_ROUND);
  }
}

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

  playerInput();

  if (playerIndex >= NUM_BUTTONS) {
    
    if (checkCombination()) {
      // --- ROUND WIN ---
      score++;
      showGoodMessage();
      
      // AGGIUNTO MODIFICA: Reduce the time for the next round
    unsigned long newTimeUL = timeAvailable - (unsigned long)F;

  if (newTimeUL < MIN_TIME_AVAILABLE) {
    timeAvailable = MIN_TIME_AVAILABLE;
    } else {
      timeAvailable = newTimeUL;
  }
      // --- INIZIO MODIFICA: AGGIUNTA PRINT SERIALE ---
            Serial.print("Round vinto. Nuovo tempo disponibile (ms): ");
            Serial.println(timeAvailable);
            // --- FINE MODIFICA ---
      // Wait 1.5s, shows "Good!"  then start next round
      wait(1500, STATE_START_ROUND);
      return;

    } else {
      // Wrong sequence
      lost = true;
      lcd.clear();
      lcd.print("SEQUENZA SBAGLIATA!"); 
      delay(2000); 
      changeState(FINAL_STATE);
      return;
    }
  }
}


// Helper Functions

void sequenceShuffle() {
  int arr[NUM_BUTTONS] = {1, 2, 3, 4};
  int temp, j;
  // Fisher-Yates algorithm to shuffle an array
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

void showSequence() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sequence: "); 
  lcd.setCursor(0, 1);
  for (int i = 0; i < NUM_BUTTONS; i++) {
    lcd.print(sequence[i]);
    lcd.print(" ");
  }
  
  wait(3000, STATE_SHOW_SEQUENCE);
}


void state_show_sequence() {

  lcd.clear(); // Clear the sequence from the screen
    
  resetInput(); // Clear any buttons pressed during the player wait
  
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
  //timeAvailable = T1;  commentata perchè il tempo viene modificato in game_state
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