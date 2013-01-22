#include "Text.h"

/****************************************

	Get a key from the keyboard and draw a cursor if needed

****************************************/

Word WaitCursorKey(void)
{
	Word Temp;
	do {
		Temp = GetACursorKey();	 /* Wait for a keystroke */
	} while (!Temp);
	return Temp;
}
