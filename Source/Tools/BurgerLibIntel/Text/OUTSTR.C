#include "Text.h"

/****************************************

	Print a "C" string out to my internal
	video driver. This is the MAIN string
	print routine.

****************************************/

void OutString(Byte *StrPtr)
{
	Word Val;
	Val = StrPtr[0];
	if (Val) {
		do {
			OutChar(Val); /* Print the char */
			++StrPtr;			/* Next char */
			Val = StrPtr[0];
		} while (Val);
	}
}

