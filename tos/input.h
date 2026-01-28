#ifndef __INPUT__
#define __INPUT__
#include "EnableInterrupt.h" // Library for interrupts

void initInput();
void resetInput();
bool isButtonPressed(int buttonIndex);

#endif
