#include "kernel.h"

//keep track of the game flow

int currentState;
long enteredStateTime;
long currentTimeInState;
bool firstCheckInState; //flag is true if it's the first cycle

void updateStateTime(){
  currentTimeInState = millis() - enteredStateTime;
}

int getCurrentState(){
  return currentState;
}

long getCurrentTimeInState(){
  return currentTimeInState;
}

long getEnteredStateTime(){
  return enteredStateTime;
}

bool isJustEnteredInState(){
  bool com = firstCheckInState;
  firstCheckInState = false;
  return com;
}

void changeState(int new_state){
  currentState = new_state;
  enteredStateTime = millis();
  firstCheckInState = true;
}

void logMsg(const String& msg){
  #ifdef __DEBUG__
  Serial.println(msg);
  #endif
}

