#include "Text.h"

/****************************************

	Get a key from the keyboard and draw a cursor if needed

****************************************/

static Byte Cursors[4] = {'|','/','-','\\'};
static LongWord Wowzers;
static Word LastCur;

Word GetACursorKey(void)
{
	Word Temp;
	LongWord NewTick;

	Temp = KeyboardGet();		/* Get user input */
	if (Temp) {
		OutSpace();			/* Erase the cursor on a key stroke */
		OutBackSpace();
	} else {
		NewTick = (ReadTick()>>1);
		if (NewTick!=Wowzers) {
			++LastCur;
		}
		Wowzers=NewTick;
		OutChar(Cursors[LastCur&3]);
		OutBackSpace();			/* Backspace over the cursor */
	}
	return Temp;
}
