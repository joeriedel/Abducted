#include "Text.h"

/****************************************

	Convience routine to send a line feed and a carriage return

****************************************/

void OutCRLF(void)
{
	OutChar(13);			/* CR */
	OutChar(10);			/* LF */
}

