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

double F; //Scale factor
int sequence[NUM_BUTTONS];
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

  static int myComb[NUM_BUTTONS];
  static int cont;
  static int timeAvailable;
  static bool lost = NULL;
  int timeElapsed = getCurrentTimeInState();
  
  if (isJustEnteredInState()){
    lcd.setCursor(0, 0); // Set the cursor on the third column and first row.  
    sequenceShuffle();
    lcd.print("Sequenza : ");    
    for(i = 0 ; i < NUM_BUTTONS ; i++)
      lcd.print(sequence[i]);

    cont = 0; //internal counter of myComb array
    timeAvailable = T1; //time available to
  }
  
  //Se il tempo per indovinare è scaduto, oppure ho sbagliato la sequenza --> GAME OVER
  if(timeElapsed >= timeAvailable){
    lcd.clear();
    lcd.print("TEMPO SCADUTO");
    delay(1000);
    lcd.clear();
  } else if(lost){
    lcd.print("SEQUENZA SBAGLIATA");
  }

  //Se ho indovinato la sequenza per tempo, incremento lo score e diminuisco il tempo disponibile del un fattore F
  if(lost != NULL && lost == false){
    lcd.clear();
    lcd.print("SEQUENZA CORRETTA");
  }



  for(i = 0 ; i < NUM_BUTTONS; i++){

    if(isButtonPressed(i)){
      Serial.print("Hai premuto il bottone "); //Debug
      Serial.println(i);
      digitalWrite(LED01_PIN, HIGH);
      delay(200);
      digitalWrite(LED01_PIN, LOW);
      myComb[cont] = i + 1;
      cont++;
      resetInput();
    }

  }

  if(cont == NUM_BUTTONS){

    for(i = 0 ; i < NUM_BUTTONS ; i++){
      if(myComb[i] != sequence[i])
        lost = true;
    }

    lost = false;

  }


}

void sequenceShuffle(){
  int arr[NUM_BUTTONS]={1, 2, 3, 4};  
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

void finalize(){
  if (isJustEnteredInState()){
    Serial.println("Finalize...");
  }
  changeState(INTRO_STATE);
}
