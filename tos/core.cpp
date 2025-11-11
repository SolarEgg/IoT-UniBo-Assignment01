#include "HardwareSerial.h"
#include "core.h"
#include "Arduino.h"
#include "kernel.h"
#include "input.h"
#include "config.h"
#include "LiquidCrystal_I2C.h"

#define MAX_TIME_IN_INTRO_STATE 10000
#define T1  10000
#include <time.h>

int ledPins[NUM_LED] ={LED01_PIN, LED02_PIN, LED03_PIN};
float F;
int level;
int sequence[NUM_BUTTONS];
int playerComb[NUM_BUTTONS];
int playerIndex = 0;
unsigned long timeAvailable = T1;
unsigned long roundStartTime = 0;
int score = 0; 
bool lost = false;

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,16,4); 

//Setup
void initCore(){
  Serial.begin(9600); //Monitor seriale

  randomSeed(analogRead(0)); //Genera un numero randomico ogni volta che il programma runna

  //Led verdi :
  pinMode(LED01_PIN, OUTPUT);
  pinMode(LED02_PIN, OUTPUT);
  pinMode(LED03_PIN, OUTPUT);
  pinMode(LED04_PIN, OUTPUT);

  //Led rosso :
  pinMode(LEDS_PIN, OUTPUT);

  //Display LCD :
  lcd.init();
  lcd.backlight();
}

//Gestisce lo stato iniziale
void intro(){
  
  int dt = getCurrentTimeInState();
  
  if (isJustEnteredInState()){
    lcd.setCursor(0,0); // Set the cursor on the third column and first row.
    lcd.print("Welcome to TOS!");
    lcd.setCursor(0,1);
    lcd.print("Press B1");
  }

  //Fading del led LS ---------------------------
  static int currIntensity = 0;
  static int fadeAmount = 5;

  analogWrite(LEDS_PIN, currIntensity);   
  currIntensity = currIntensity + fadeAmount;
  if (currIntensity == 0 || currIntensity == 255) {
    fadeAmount = -fadeAmount ; 
  }     

  delay(15);

  if (dt > MAX_TIME_IN_INTRO_STATE) //Se passano 10 secondi, vado in DEEP_SLEEP
    changeState(DEEP_SLEEP_STATE);

  else if(isButtonPressed(0)){ //Se ho premuto il pulsante 1, vado in GAME_STATE
    lcd.clear();
    resetInput();
    changeState(SETTING_DIFFICULTY);
  }

}

void deep_sleep(){

  if (isJustEnteredInState()){
    lcd.clear();
    lcd.setCursor(0, 0); // Set the cursor on the third column and first row.  
    lcd.println("Going sleep mode");    
  }
  //Spengo il led rosso
  analogWrite(LEDS_PIN, 0);

  /* change the state if button 0 is pressed */
  if (isButtonPressed(0)){
    digitalWrite(LED01_PIN, HIGH);
    delay(100);
    digitalWrite(LED01_PIN, LOW);
    resetInput();
    lcd.clear();
    changeState(INTRO_STATE);          
  }
}

void set_difficulty(){

  int analogValue = analogRead(POT_PIN);
  static int level = 1;

  lcd.setCursor(0, 0);
  lcd.print("Difficulty : ");
  lcd.print(level);  // stampa il valore del livello come numero

  //Forse meglio uno switch
   //Scelta difficoltà --------------------
  if(analogValue <= 255) { 
    level = 1;
    F = 0.25;
  }
  else if(analogValue <= 511){
    level = 2;
    F = 0.50;
  }
  else if(analogValue <= 767){
    level = 3;
    F = 0.75;
  }
  else{
    level = 4;
    F = 1.00;
  }

  lcd.setCursor(0, 1);
  lcd.print("Fattore : ");
  lcd.print(F); 

  /* change the state if button 0 is pressed */
  if (isButtonPressed(0)){
    analogWrite(LEDS_PIN, 0); //Spengo il led rosso
    resetInput();
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.println("Go !");     
    delay(1000);
    lcd.clear();
    changeState(GAME_STATE);          
  }
  
}

void game_state(){

  if (isJustEnteredInState()){
    score=0;
    timeAvailable = T1; 
    startNewRound();
    return;
  }

  unsigned long elapsed = millis() - roundStartTime;
  //if the time has finished
  if(elapsed >= timeAvailable){
    lost=true;
    Serial.println("Scaduto il tempo"); //Debug
    changeState(FINAL_STATE);
    return;
  }

  playerInput();

  //if the player managed to complete the sequence: 
  if(playerIndex == NUM_BUTTONS){
    if(checkCombination()){
      //the sequence is correct
      score++;
      showGoodMessage();
      delay(500);
      timeAvailable= (unsigned long)( (float)timeAvailable *(1.0f-F)); //next level, less time
      startNewRound();
  }else{
    //the sequence was wrong
    lost=true;
    changeState(FINAL_STATE);
  }
    return;
  }

}


void sequenceShuffle(){
  int arr[NUM_BUTTONS]={1, 2, 3};  
  int temp, j;
  for(int i = NUM_BUTTONS-1 ; i > 0 ; i--){
    j= random(0,i+1);
    temp= arr[i];
    arr[i]=arr[j];
    arr[j]= temp; 
  }
  for(int k=0; k< NUM_BUTTONS; k++){
    sequence[k]=arr[k];
  }
}

void showSequence(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sequenza: ");
  lcd.setCursor(0,1);
  for(int i=0; i<NUM_BUTTONS; i++){
    lcd.print(sequence[i]);
    lcd.print(" ");
  }

  delay(3000);  //da valutare, è sufficiente per poter vedere la sequenza su lcd al momento.
}

void startNewRound(){

  sequenceShuffle(); //Genera una sequenza
  lcd.clear(); //Pulisci il display
  showSequence(); // Mostra sequenza sullo schermo

  playerIndex = 0;
  resetInput();

  //turn off all green led
  for (int i=0; i<NUM_LED;i++){
    digitalWrite(ledPins[i], LOW);
  }

 roundStartTime=millis();
 lost=false;

}



bool checkCombination(){
  for(int i=0; i< NUM_BUTTONS; i++){
    if (playerComb[i] != sequence[i]){
      return false;
    }
  }
  return true;
}


void playerInput(){

  for(int i=0; i< NUM_BUTTONS; i++){

    if(isButtonPressed(i)){
      
      resetInput();  // Reset flag subito    

      playerComb[playerIndex] = i+1;
      Serial.print("Premuto pulsante :");
      Serial.println(playerComb[playerIndex]);
      playerIndex++;
      digitalWrite(ledPins[i], HIGH);
      
    }
  }
}

void showGoodMessage(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("GOOD!");
  lcd.setCursor(0,1);
  lcd.print("Score: ");
  lcd.print(score);
}

void finalize(){

  static unsigned long startTime =0;

  if (isJustEnteredInState()){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Game Over");
    lcd.setCursor(0, 1);
    lcd.print("Final Score: ");
    lcd.print(score);
    analogWrite(LEDS_PIN, 255);
    startTime = millis();

  }
  
  //Red led on for two sec
  if(millis() - startTime > 2000){
    analogWrite(LEDS_PIN, 0);
  }

  //after 10 sec go back to intro
  if(millis() - startTime >= 10000){
    lcd.clear();
    
    //turn off all green led
    for (int i=0; i<NUM_LED;i++)
      digitalWrite(ledPins[i], LOW);

    changeState(INTRO_STATE);
  }

}
