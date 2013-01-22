#include "Text.h"
#include <stdio.h>

/****************************************

	Prints a floating point number with only 2 digits
	after the decimal point (Monetary floats)

****************************************/

void OutMoneyFloat2(float Val,Word MinSize)
{
	char TempBuffer[20];
	char StreamBuffer[10];
	sprintf(StreamBuffer,"%%%u.2f",MinSize);
	sprintf(TempBuffer,StreamBuffer,Val); /* Convert to ASCII */
	OutString((Byte *)TempBuffer);			/* Send out the string */
}

