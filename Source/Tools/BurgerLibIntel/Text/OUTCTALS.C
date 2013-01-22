#include "Text.h"
#include <string.h>

/****************************************

	Print a list of zero terminated strings
	with each string prefixed by a minimum number
	of spaces and suffixed by a clear to end of line.
	The minimum number of spaces is determined by
	centering the AVERAGE length of all strings in the list.
	A double 0 will terminate the list.

****************************************/

void OutCenteredAverageList(Byte *List)
{
	Byte *List2;
	Word Left;
	Word Temp;
	Word Count;

	if (List[0]) {		/* Detect for divide by zero! */
		Left = 0;		/* Init total length */
		List2 = List;	 /* Init running pointer */
		Count = 0;		/* Init string count */
		do {
			Temp = strlen((char *)List2);	 /* Get the string length */
			Left+=Temp;			 /* Add to total */
			List2 += Temp+1;		/* Index to the next string */
			++Count;				/* Inc count */
		} while (List2[0]);
		Left = (TextWidth-(Left/Count))/2;	/* Center on average length */
		OutTabbedList(List,Left);	 /* Print the list */
	}
}
