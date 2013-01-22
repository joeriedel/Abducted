#include "Text.h"
#include <stdlib.h>

/****************************************

	Input an ASCII string and convert it to an
	unsigned int number. Return TRUE if acceptable,
	FALSE if canceled, or 2 if invalid input

****************************************/

Boolean InputNumber(Word *Answer)
{
	char TempBuffer[11];

	if (InputLine((Byte *)TempBuffer,sizeof(TempBuffer),ILNumeric)) {
		*Answer = atoi(TempBuffer);
		return TRUE;		/* Accepted */
	}
	return FALSE;		/* Canceled */
}

