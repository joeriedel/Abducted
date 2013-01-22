#include "Text.h"

/*******************************

	Grab a floating point number from the input

*******************************/

float FetchFileFloat(FILE *fp)
{
	float Val;
	Val = 0.0;				/* Init variable */
	fscanf(fp,"%g",&Val);	/* Use "C" to scan the floating point number */
	return Val;				/* Return the value */
}
