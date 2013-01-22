#include "Text.h"

/*******************************

	Grab a decimal number from the input

*******************************/

Word FetchFileWord(FILE *fp)
{
	Word Val;
	Val = 0;				/* Init the result */
	fscanf(fp,"%u",&Val);	/* Fetch a decimal number from input */
	return Val;				/* Return the result */
}

