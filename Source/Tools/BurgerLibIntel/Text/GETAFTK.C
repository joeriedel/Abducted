#include "Text.h"
#include <ctype.h>

/*******************************

	Parse a token from the input stream

*******************************/

Boolean GetAFileToken(FILE *fp,Byte *Dest,Word MaxLen)
{
	Word i;
	Word Val;

	--MaxLen;
	KillFileWhiteSpace(fp); /* Remove leading whitespace */
	i = 0;
	do {
		Val = fgetc(fp);	/* Get a char */
		if (Val==EOF) {		/* End of input?? */
			return FALSE;	/* Abort now */
		}
		if (i<MaxLen) {
			Dest[i] = toupper(Val);	/* Save in dest string */
			++i;
		}
	} while (Val!='=' && Val!=' ' && Val!=9 && Val!=10);
	if (Val!='=') {		/* Remove the end char if not an '=' */
		--i;
		ungetc(Val,fp);
	}
	Dest[i] = 0;		/* Save final zero */
	return TRUE;
}

