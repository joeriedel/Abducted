#include "text.h"

/****************************************

	Draw a group of input controls on the screen

****************************************/

void DrawInputStruct(EntryList_t *EntryPtr,Word Entries)
{
	do {
		DrawOneInputStruct(EntryPtr);	/* Draw it */
		++EntryPtr;						/* Next one */
	} while (--Entries);				/* All done? */
}
