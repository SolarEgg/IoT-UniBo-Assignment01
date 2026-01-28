#include "kernel.h"
#include "config.h"
#include "core.h"
#include "input.h" 

void setup() {
  initCore(); //
  initInput(); //setup input pins
  changeState(INTRO_STATE);
}

void loop(){ 
  updateStateTime();
  
  switch (getCurrentState()) { 
    case INTRO_STATE:
      intro();
      break;
    
    case DEEP_SLEEP_STATE:
      deep_sleep();
      break;
    
    case SETTING_DIFFICULTY:
      set_difficulty();
      break;
      
    case STATE_START_ROUND: 
      startNewRound();
      break;

    case STATE_SHOW_SEQUENCE: 
      state_show_sequence();
      break;

    case STATE_WAIT: 
      state_wait();
      break;

    case GAME_STATE:
      game_state();
      break;
      
    case FINAL_STATE:
      finalize();
      break;
  }
}

