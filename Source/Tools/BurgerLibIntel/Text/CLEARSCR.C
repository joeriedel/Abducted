#include "Text.h"

/****************************************

	Convience routine to clear the ENTIRE screen
	and home the text cursor to the upper left

****************************************/

void ClearScreen(void)
{
	OutChar(27);
	OutChar('+');	/* Send the command sequence */
}

