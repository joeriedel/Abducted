#include "Text.h"

/****************************************

	Convience routine to print a group of spaces

****************************************/

void TabToSpace(Word NewX)
{
	if (TextX<NewX) {			/* Already at the proper text coord? */
		OutSpaces(NewX-TextX);	/* Print enough spaces to the tab */
	}
}

