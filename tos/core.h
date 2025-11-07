#ifndef __CORE__
#define __CORE__

#define INTRO_STATE   1
#define DEEP_SLEEP_STATE 2
#define SETTING_DIFFICULTY 3
#define GAME_STATE  4
#define STAGE2_STATE  5
#define FINAL_STATE   6 

/* core business logic  */

void initCore(); //setup iniziali
void intro(); //introduzione --> welcome to tos
void deep_sleep();
void set_difficulty();
void game_state();
void stage1();
void stage2();
void finalize();
void sequenceShuffle();

#endif
