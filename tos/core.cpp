#include "core.h"
#include "Arduino.h"
#include "kernel.h"
#include "input.h"
#include "config.h"

#define MAX_TIME_IN_INTRO_STATE 10000
#define MAX_TIME_IN_STAGE2_STATE 10000

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


  //Va inizializzato il display LCD
}

//Gestisce lo stato iniziale
void intro(){
  
  if (isJustEnteredInState()){
    Serial.println("Welcome to TOS! Press B1 to Start");
  }

  //Fading del led LS ---------------------------
  static int currIntensity = 0;
  static int fadeAmount = 5;

  analogWrite(LEDS_PIN, currIntensity);   
  currIntensity = currIntensity + fadeAmount;
  if (currIntensity == 0 || currIntensity == 255) {
    fadeAmount = -fadeAmount ; 
  }     

  //Scelta difficoltà --------------------
  int analogValue = analogRead(POT_PIN);
  static int level;
  if(analogValue <= 255)
    level = 1;
  else if(analogValue <= 511)
    level = 2;
  else if(analogValue <= 767)
    level = 3;
  else{
    level = 4;
  }

  int dt = getCurrentTimeInState();
  if (dt > MAX_TIME_IN_INTRO_STATE) //Se passano 10 secondi, vado in DEEP_SLEEP
    changeState(DEEP_SLEEP_STATE);

  else if(isButtonPressed(0)) //Se ho premuto il pulsante 1, vado in GAME_STATE
    Serial.println("stato 1 - DIFFICOLTA' SCELTA :" + level);
    //changeState(GAME_STATE);

}

void deep_sleep(){
  if (isJustEnteredInState()){
    Serial.println("Going into deep sleep mode..");
  }

  /* change the state if button 0 is pressed */
  if (isButtonPressed(0)){
    changeState(INTRO_STATE);          
  }
}


void game_state(){
  if (isJustEnteredInState()){
    Serial.println("Stage1..");
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
