#include "Text.h"

/*******************************

	Remove whitespace from an input stream

*******************************/

void KillFileWhiteSpace(FILE *fp)
{
	Word Val;
	do {
		Val = fgetc(fp);			/* Get a char from input stream */
	} while (Val==' ' || Val==9);	/* Space or tab? */
	ungetc(Val,fp);		 /* Restore the non-whitespace char */
}
