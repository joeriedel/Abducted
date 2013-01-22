#include "Text.h"

/****************************************

	Convience routine to set the position of the
	text cursor.

****************************************/

void SetXY(Word x,Word y)
{
	OutChar(27);
	OutChar('=');
	OutChar(y+32);
	OutChar(x+32);
}

