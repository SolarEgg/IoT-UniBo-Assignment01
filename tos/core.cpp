#include "core.h"
#include "Arduino.h"
#include "kernel.h"
#include "input.h"
#include "config.h"
#include "LiquidCrystal_I2C.h"

#define MAX_TIME_IN_INTRO_STATE 10000
#define MAX_TIME_IN_STAGE2_STATE 10000

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,20,4); 

//Setup
void initCore(){
  Serial.begin(9600); //Monitor seriale

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
  
  if (isJustEnteredInState()){
    lcd.setCursor(0,0); // Set the cursor on the third column and first row.
    lcd.print("Welcome to TOS!");
    lcd.setCursor(0,1);
    lcd.print("Press B1 to start");
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

  int dt = getCurrentTimeInState();
  if (dt > MAX_TIME_IN_INTRO_STATE) //Se passano 10 secondi, vado in DEEP_SLEEP
    changeState(DEEP_SLEEP_STATE);

  else if(isButtonPressed(0)){ //Se ho premuto il pulsante 1, vado in GAME_STATE
    lcd.clear();
    changeState(SETTING_DIFFICULTY);
  }

}

void deep_sleep(){
  if (isJustEnteredInState()){
    lcd.clear();
    lcd.setCursor(0, 0); // Set the cursor on the third column and first row.  
    lcd.println("Going in sleep mode");    
  }
  //Spengo il led rosso
  analogWrite(LEDS_PIN, 0);

  /* change the state if button 0 is pressed */
  if (isButtonPressed(0)){
    digitalWrite(LED01_PIN, HIGH);
    delay(100);
    digitalWrite(LED01_PIN, LOW);
    changeState(INTRO_STATE);          
  }
}

void set_difficulty(){

  int analogValue = analogRead(POT_PIN);
  static int level = 1;

  if (isJustEnteredInState()){
    lcd.setCursor(0, 0); // Set the cursor on the third column and first row.  
    lcd.println("Difficulty level : " + level);    
  }

   //Scelta difficoltà --------------------
  if(analogValue <= 255)
    level = 1;
  else if(analogValue <= 511)
    level = 2;
  else if(analogValue <= 767)
    level = 3;
  else{
    level = 4;
  }

  /* change the state if button 0 is pressed */
  if (isButtonPressed(0)){
    digitalWrite(LED01_PIN, HIGH);
    delay(100);
    digitalWrite(LED01_PIN, LOW);
    changeState(GAME_STATE);          
  }
  
}


void game_state(){
  if (isJustEnteredInState()){
    lcd.clear();
    lcd.setCursor(0, 0); // Set the cursor on the third column and first row.  
    lcd.println("Gioco Iniziato");      
    resetInput();
  }

  /* change the state if button 0 is pressed */
  if (isButtonPressed(0)){
    changeState(STAGE2_STATE);          
  }
}

void stage1(){
  if (isJustEnteredInState()){
    Serial.println("Stage2...");
  }

  /* change the state if button 1 is pressed or max time elapsed*/
  if (isButtonPressed(1) || getCurrentTimeInState() > MAX_TIME_IN_STAGE2_STATE){
    changeState(FINAL_STATE);          
  }
}

void stage2(){
  if (isJustEnteredInState()){
    Serial.println("Stage2...");
  }

  /* change the state if button 1 is pressed or max time elapsed*/
  if (isButtonPressed(1) || getCurrentTimeInState() > MAX_TIME_IN_STAGE2_STATE){
    changeState(FINAL_STATE);          
  }
}

void finalize(){
  if (isJustEnteredInState()){
    Serial.println("Finalize...");
  }
  changeState(INTRO_STATE);
}
