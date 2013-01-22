#include "Text.h"

/*******************************

	Write out a binary number

*******************************/

void WriteFileBinary(FILE *fp,Word Val)
{
	Word Mask;		/* Bit mask */

	Mask = 0x8000;	/* Init the bit mask */
	do {
		fputc((Mask&Val) ? '1' : '0',fp);		/* Print the char */
		Mask>>=1;			/* Shift down the mask by 1 bit */
	} while (Mask);		 /* All done? */
}

