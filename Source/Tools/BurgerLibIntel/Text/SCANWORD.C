#include "Text.h"

/*********************************

	Scan an ASCII string for an unsigned int and
	return the result in *Result.
	Return the Byte * to the first non numeric
	ASCII char.

*********************************/

Byte *ScanWord(Byte *StrPtr,Word *Result)
{
	Word Val;
	Word Answer;

	Answer = 0;			/* Assume numeric value is zero */
	for (;;) {
		Val = StrPtr[0]-'0';		/* Get an ASCII char (Convert to bin) */
		if (Val>=10) {				/* Non-numeric? */
			*Result = Answer;		/* Return the previous result */
			return StrPtr;			/* Return the current string pointer */
		}
		Answer = (Answer*10)+Val;	/* Add in the new numeric constant */
		++StrPtr;					/* Accept the ASCII char */
	}								/* Loop forever */
}
