#include "Text.h"

/*******************************

	Write out a literal text string

*******************************/

void WriteFileString(FILE *fp,Byte *SrcPtr)
{
	Word Temp;
	fputc('"',fp);			/* Print the opening quote */
	for (;;) {
		Temp=SrcPtr[0];	 /* Get a char from the input */
		if (!Temp) {
			break;
		}
		++SrcPtr;			 /* Inc index */
		if (Temp=='"') {	/* Quote char? */
			fputc(Temp,fp);	/* Print a double quote */
		}
		fputc(Temp,fp);		/* Print the char in the string */
	}
	fputc('"',fp);			 /* Send the ending quote */
}

