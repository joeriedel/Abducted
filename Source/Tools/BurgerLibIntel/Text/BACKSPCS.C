#include "Text.h"

/****************************************

	Convience routine to print a group of backspaces

****************************************/

void OutBackSpaces(Word Count)
{
	do {
		OutChar(8);	 /* Print a backspace */
	} while (--Count);	/* Count down */
}

