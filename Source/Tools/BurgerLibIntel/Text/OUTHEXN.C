#include "Text.h"

/****************************************

	Convience routine to print a hex NIBBLE

****************************************/

void OutHexNibble(Word Nibble)
{
	OutChar(HexAsc(Nibble));	/* Print the converted char */
}

