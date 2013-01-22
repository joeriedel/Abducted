#include "Text.h"

/****************************************

	Output a single char and repeat it until
	the count is zero.

****************************************/

void OutChars(Word Letter,Word Count)
{
	if (Count) {		/* Any to process? */
		do {
			OutChar(Letter);
		} while (--Count);
	}
}

