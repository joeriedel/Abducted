#include "Text.h"

/*******************************

	Grab a binary number from the input

*******************************/

Word FetchFileBinary(FILE *fp)
{
	Word Val;
	Word Answer;

	KillFileWhiteSpace(fp);		 /* Remove leading whitespace */
	Answer = 0;				 /* Assume zilch */
	for (;;) {
		Val = fgetc(fp)-'0';		/* Get a char */
		if (Val<2) {
			Answer = (Answer<<1)|Val;	/* Shift up the result and add new digit */
		} else {
			ungetc(Val+'0',fp);	 /* Put char back */
			break;
		}
	}
	return Answer;			/* Return the number */
}

