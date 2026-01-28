#ifndef __CORE__
#define __CORE__

#define INTRO_STATE 1
#define DEEP_SLEEP_STATE 2
#define SETTING_DIFFICULTY 3
#define GAME_STATE 4   
#define FINAL_STATE 5
#define STATE_WAIT 6    
#define STATE_START_ROUND 7 
#define STATE_SHOW_SEQUENCE 8 

/* core business logic */

void initCore(); //Initial setups
void intro(); //
void deep_sleep();
void displayDifficulty();
void set_difficulty();
void game_state();

// --- funzioni per logica non-bloccante ---
void state_wait();
void state_show_sequence();
void wait(unsigned long duration, int stateAfterWait);


void finalize();
void sequenceShuffle();
void showSequence();
void startNewRound();
bool checkCombination();
void playerInput();
void showGoodMessage();

#endif