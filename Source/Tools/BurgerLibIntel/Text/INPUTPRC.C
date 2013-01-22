#include "Text.h"
#include <stdio.h>

/****************************************

	Input an ASCII string and convert it to a
	floating point number. Return TRUE if acceptable,
	FALSE if canceled, or 2 if invalid input

****************************************/

Boolean InputPrice(float *Answer)
{
	Byte TempBuffer[20];
	Word Len;
	Word Dots;
	Word Fracs;

	while (InputLine(TempBuffer,sizeof(TempBuffer),ILFloat)) {
		Len = 0;					/* Init index */
		Dots = 0;				 /* Init period count */
		Fracs = 0;
		while (TempBuffer[Len]) {	 /* Valid char? */
			if (TempBuffer[Len]=='.') {	 /* Period? */
				++Dots;		 /* Count the periods */
			} else if (Dots) {
				++Fracs;		/* Digits AFTER the decimal point */
			}
			++Len;			/* Inc index */
		}
		/* Only allow numbers with ONE decimal point and less than */
		/*	THREE digits after the decimal point */
		/* In other words, only VALID monetary numbers! */

		if (Dots<2 && Fracs<3 && (sscanf((char *)TempBuffer,"%g",Answer) == 1)) {
			return TRUE;		/* Acceptable? */
		}
		OutBackSpaces(Len);		/* Erase the old input to accept a new */
		OutSpaces(Len);		/* line of input */
		OutBackSpaces(Len);
		OutChar(7);			/* Beep */
	}
	return FALSE;			/* User canceled */
}

