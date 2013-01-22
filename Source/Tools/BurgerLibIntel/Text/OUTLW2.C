#include "Text.h"

/****************************************

	Prints an unsigned long number.
	Also prints lead spaces or zeros if needed

****************************************/

static LongWord TensTable[] = {
1,				/* Table to quickly div by 10 */
10,
100,
1000,
10000,
100000,
1000000,
10000000,
100000000,
1000000000
};

void OutLongWord2(LongWord Val,Word MinSize,Word LeadChar)
{
	Word Index;		 /* Index to TensTable */
	LongWord BigNum;	/* Temp for TensTable value */
	Word Letter;		/* ASCII char */
	Word Printing;		/* Flag for printing */

	Index = 10;		 /* 10 digits to process */
	Printing = FALSE;	/* Not printing yet */
	do {
		--Index;		/* Dec index */
		BigNum = TensTable[Index];	/* Get div value in local */
		Letter = '0';			 /* Init ASCII value */
		while (Val>=BigNum) {	 /* Can I divide? */
			Val-=BigNum;		/* Remove value */
			++Letter;			 /* Inc ASCII value */
		}
		if (Printing || Letter!='0' || !Index) {	/* Already printing? */
			Printing = TRUE;		/* Force future printing */
			OutChar(Letter);		/* Also must print on last char */
		} else if (Index<MinSize) {
			OutChar(LeadChar);		/* Pad the output? */
		}
	} while (Index);		/* Any more left? */
}

