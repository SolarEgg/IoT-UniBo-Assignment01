#ifndef __CORE__
#define __CORE__

#define INTRO_STATE 1
#define DEEP_SLEEP_STATE 2
#define SETTING_DIFFICULTY 3
#define GAME_STATE 4        // Stato di gioco attivo
#define FINAL_STATE 5
#define STATE_WAIT 6        // STATO PAUSA (non-bloccante)
#define STATE_START_ROUND 7 // Stato che prepara il round
#define STATE_SHOW_SEQUENCE 8 // Stato che mostra la sequenza


/* core business logic */

void initCore(); //setup iniziali
void intro(); //introduzione --> welcome to tos
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