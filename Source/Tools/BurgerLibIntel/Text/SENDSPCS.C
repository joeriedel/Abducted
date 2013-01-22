#include "Text.h"

/****************************************

	Convience routine to print a group of spaces

****************************************/

void OutSpaces(Word Count)
{
	do {
		OutChar(' ');	/* Print a space */
	} while (--Count);	/* Count down */
}

