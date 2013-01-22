#include "Text.h"

/****************************************

	Convert the hex nibble (0-F) into ASCII 0-9,A-F

****************************************/

Word HexAsc(Word Nibble)
{
	Nibble &= 0x0f;			/* Failsafe */
	if (Nibble>=10) {		/* A-F? */
		Nibble+=7;			/* Convert to A-F */
	}
	return (Nibble+'0');	/* Print the converted char */
}

