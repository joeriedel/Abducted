#include "Text.h"

/****************************************

	Convience routine to print a hex BYTE

****************************************/

void OutHexByte(Word Hex)
{
	OutHexNibble(Hex>>4);			/* Print the upper 4 bits */
	OutHexNibble(Hex);				/* Print the lower 4 bits */
}

