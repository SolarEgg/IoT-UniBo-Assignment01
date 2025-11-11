#ifndef __CONFIG__
#define __CONFIG__

// #define __DEBUG__
#define NUM_BUTTONS 4
#define NUM_LED 4

//Pulsanti
#define BUT01_PIN 2
#define BUT02_PIN 3
#define BUT03_PIN 4
#define BUT04_PIN 5

//Led verdi
#define LED01_PIN 6
#define LED02_PIN 7
#define LED03_PIN 8
#define LED04_PIN 9

//Led Rosso --> PWM
#define LEDS_PIN 10 

//Potenziometro
#define POT_PIN A2

//LCD
#define SCL_PIN A5
#define SDA_PIN A4


#define MAX_TIME_IN_INTRO_STATE 10000
#define T1  120000 


#endif