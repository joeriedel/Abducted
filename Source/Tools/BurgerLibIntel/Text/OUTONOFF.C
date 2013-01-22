#include "Text.h"

/****************************************

	Just prints the word "ON" or "OFF" depending on state

****************************************/

void OutOnOff(Word Flag)
{
	if (Flag) {		 /* On? */
		OutString((Byte *)" ON");		/* Print ON */
	} else {
		OutString((Byte *)"OFF");		/* Print OFF */
	}
}

