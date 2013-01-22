#include "Text.h"

/****************************************

	Convience routine to clear the console from the
	current text cursor position to the end of the screen

****************************************/

void ClearEOP(void)
{
	OutChar(27);
	OutChar('Y');		/* Send the command sequence */
}

