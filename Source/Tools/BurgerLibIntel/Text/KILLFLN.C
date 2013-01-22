#include "Text.h"

/*******************************

	Kill a line of data

*******************************/

void KillFileLine(FILE *fp)
{
	Word Val;
	do {
		Val = fgetc(fp);		/* Get a char from the input stream */
	} while (Val!=10 && Val!=EOF);	/* EOL or EOF? */
}

