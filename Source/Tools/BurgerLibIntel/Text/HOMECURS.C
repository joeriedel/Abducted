#include "Text.h"

/****************************************

	Convience routine to home the text cursor
	without clearing the screen

****************************************/

void HomeCursor(void)
{
	OutChar(0x1E);		/* Send the home command */
}
