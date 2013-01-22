#include "Text.h"
#include <string.h>

/****************************************

	Print a list of zero terminated strings
	with each string prefixed by a minimum number
	of spaces and suffixed by a clear to end of line.
	The minimum number of spaces is determined by
	centering the LARGEST string in the list.
	A double 0 will terminate the list.

****************************************/

void OutCenteredList(Byte *List)
{
	Byte *List2;
	Word Left;
	Word Temp;

	Left = 0;				/* Init the largest string */
	List2 = List;			 /* Save a running pointer */
	while (List2[0]) {		/* Any strings? */
		Temp = strlen((char *)List2);	 /* Get the string length */
		if (Left<Temp) {		/* Larger than previous largest? */
			Left = Temp;		/* Save new largest string */
		}
		List2 += Temp+1;	/* Index to next string */
	}
	Left = (TextWidth-Left)/2;	/* Center from largest string */
	OutTabbedList(List,Left);	 /* Print the list */
}

