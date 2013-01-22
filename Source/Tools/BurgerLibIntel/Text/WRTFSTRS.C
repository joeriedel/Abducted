#include "Text.h"

/*******************************

	Write out the string array

*******************************/

void WriteFileStrings(FILE *fp,Byte **Array,Word MaxWidth)
{
	Word Count;	 /* Entries per line */

	if (!Array[0]) {
		return;
	}
	Count = 0;		/* Init entry count */
	for (;;) {		/* Valid entry? */
		WriteFileString(fp,Array[0]);	/* Print the entry */
		++Array;			/* Inc the pointer index */
		if (!Array[0]) {
			return;
		}
		fputc(',',fp);	/* Append a comma */
		if (++Count>=MaxWidth) {		 /* Do I need a CR? */
			fputc('\n',fp);	/* Print the CR */
			Count = 0;		/* Reset the count */
		}
	}
}

