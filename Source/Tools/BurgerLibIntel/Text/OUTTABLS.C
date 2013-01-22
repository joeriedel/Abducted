#include "Text.h"
#include <string.h>

/****************************************

	Print a list of zero terminated strings
	with each string prefixed by a minimum number
	of spaces and suffixed by a clear to end of line.
	A double 0 will terminate the list

****************************************/

void OutTabbedList(Byte *List,Word MinTab)
{
	while (List[0]) {			/* Entry valid? */
		TabToSpace(MinTab);		/* Send the spaces */
		OutString(List);		/* Print the entry */
		ClearEOL();				/* Clear to the end of the line */
		OutCRLF();				/* Next line please */
		List+=(strlen((char *)List)+1);	/* Index to the next string */
	}
}
