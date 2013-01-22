#include "Text.h"

/*********************************

	Convience routine to invoke formatted
	text output without passing a pointer to
	a list of variables.

*********************************/

void OutFormatted(Byte *StrPtr)
{
	OutFormatted2(StrPtr,0);
}
