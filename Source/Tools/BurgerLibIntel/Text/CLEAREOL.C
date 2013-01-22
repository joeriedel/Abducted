#include "Text.h"

/****************************************

	Convience routine to clear the console from the
	current text cursor position to the end of the line

****************************************/

void ClearEOL(void)
{
	OutChar(27);
	OutChar('T');		/* Send the command sequence */
}

