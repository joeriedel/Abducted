#include "Text.h"
#include <stdio.h>

/****************************************

	Prints a floating point number with only 2 digits
	after the decimal point (Monetary floats)

****************************************/

void SOutMoneyFloat2(Byte *TempBuffer,float Val,Word MinSize)
{
	char StreamBuffer[10];
	sprintf(StreamBuffer,"%%%u.2f",MinSize);
	sprintf((char *)TempBuffer,StreamBuffer,Val); /* Convert to ASCII */
}

