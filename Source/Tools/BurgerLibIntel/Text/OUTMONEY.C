#include "Text.h"
#include <stdio.h>

/****************************************

	Prints a floating point number with only 2 digits
	after the decimal point (Monetary floats)

****************************************/

void OutMoneyFloat(float Val)
{
	Byte TempBuffer[20];
	sprintf((char *)TempBuffer,"%.2f",Val); /* Convert to ASCII */
	OutString(TempBuffer);			/* Send out the string */
}

