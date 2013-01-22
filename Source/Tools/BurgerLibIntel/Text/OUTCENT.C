#include "Text.h"
#include <string.h>

/****************************************

	Prints out system status header to current user

****************************************/

void OutCenter(Byte *StrPtr)
{
	TabToSpace((TextWidth-strlen((char *)StrPtr))/2);
	OutString(StrPtr);
}
