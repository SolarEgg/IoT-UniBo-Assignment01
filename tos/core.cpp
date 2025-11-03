#include "core.h"
#include "Arduino.h"
#include "kernel.h"
#include "input.h"
#include "config.h"
#include "LiquidCrystal_I2C.h"

#define MAX_TIME_IN_INTRO_STATE 10000
#define T1 = 10000
#include <time.h>

double F; //Fattore di scala
int sequenza[4];
int i;

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

  int dt = getCurrentTimeInState();
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
    lcd.setCursor(0, 0); // Set the cursor on the third column and first row.  
    generaSequenza();
    lcd.print("Sequenza : ");    
    for(i = 0 ; i < 4 ; i++)
      lcd.print(sequenza[i]);
    
  }
  /* change the state if button 0 is pressed */
  if (isButtonPressed(0)){
    digitalWrite(LED01_PIN, HIGH);
  }
}

void generaSequenza(){
  for(int j = 0 ; j < 4 ; j++){
    sequenza[j] = random(1,5); //Genera un numero casuale tra 1 e 4
    Serial.println(sequenza[j]);
  }
}

void finalize(){
  if (isJustEnteredInState()){
    Serial.println("Finalize...");
  }
  changeState(INTRO_STATE);
}
