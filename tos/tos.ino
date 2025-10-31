#include "kernel.h"
#include "config.h"
#include "core.h"
#include "input.h" 

void setup() {
  initCore();
  initInput();
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
  case GAME_STATE:
    game_state();
    break;
  case STAGE2_STATE:
    stage2();
    break;
  case FINAL_STATE:
    finalize();
    break;
  }
}

