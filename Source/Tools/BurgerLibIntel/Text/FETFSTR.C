#include "Text.h"

/*******************************

	Grab a literal string from the input file

*******************************/

void FetchFileString(FILE *fp,Byte *Dest,Word MaxLen)
{
	Word i;
	Word Val;

	Dest[0] = 0;				/* Make SURE it's ready! */
	KillFileWhiteSpace(fp);		/* Remove leading whitespace */
	Val = fgetc(fp);			/* Get the first char */
	if (Val!='"') {				/* Must be quotes! */
		ungetc(Val,fp);			/* Put the char back */
		return;					/* No input */
	}
	i = 0;						/* Init index */
	for (;;) {
		Val = fgetc(fp);		/* Get a char */
		if (Val=='"') {			/* Quote? */
			Val = fgetc(fp);
			if (Val!='"') {	 /* Double quote? */
				ungetc(Val,fp); /* Put SECOND char back */
				break;			/* Exit */
			}
		}
		if (Val==EOF || Val==10) {	/* End of line or exit? */
			ungetc(Val,fp);	 /* Put char back */
			break;
		}
		if (Val==9) {			/* Convert tabs to spaces */
			Val = ' ';
		}
		if (i<MaxLen) {		 /* Must fit in buffer */
			Dest[i] = Val;		/* Save char in dest */
			++i;				/* Next index */
			Dest[i] = 0;		/* My special end of line character */
		}
	}
}

