#include "Text.h"
#include <stdio.h>

/****************************************

	Prints a floating point number with only 2 digits
	after the decimal point (Monetary floats)

****************************************/

void SOutMoneyFloat(Byte *TempBuffer,float Val)
{
	sprintf((char *)TempBuffer,"%.2f",Val); /* Convert to ASCII */
}

