#include "Text.h"

/****************************************

	Convience routine to print a hex SHORT

****************************************/

void OutHexShort(Word Hex)
{
	OutHexByte(Hex>>8);			/* Print the upper 8 bits */
	OutHexByte(Hex);				/* Print the lower 8 bits */
}

