#include "OcOSCursor.h"

#if defined(__MAC__)
#include <QuickDraw.h>

static CCrsrHandle OSCursorPreviousCur;		/* Current color cursor for MacOS */

/**********************************

	Load an OS resource and set the cursor to it

**********************************/

void BURGERCALL OSCursorSet(Word Curnum)
{
	if (Curnum!=OSCursorLastCursor) {
		CCrsrHandle curs;		/* Mac cursor handle */
		curs = GetCCursor(Curnum);		/* Load the cursor */
		if (curs) {			/* Did it load? */
			OSCursorLastCursor = Curnum;
			SetCCursor(curs);	/* Set the cursor */
			if (OSCursorPreviousCur) {		/* Was there a cursor before? */
				DisposeCCursor(OSCursorPreviousCur);	/* Dispose of the cursor */
			}
			OSCursorPreviousCur = curs;		/* Save the cursor record to delete */
		}
	}
}

/**********************************

	Make an OS cursor visible

**********************************/

void BURGERCALL OSCursorShow(void)
{
	if (!OSCursorVisibleFlag) {
		OSCursorVisibleFlag = TRUE;		/* It's visible */
		ShowCursor();
	}
}

/**********************************

	Make an OS cursor disappear

**********************************/

void BURGERCALL OSCursorHide(void)
{
	if (OSCursorVisibleFlag) {
		OSCursorVisibleFlag = FALSE;
		HideCursor();
	}
}

/**********************************

	Reset an OS cursor to an arrow and show it

**********************************/

void BURGERCALL OSCursorReset(void)
{
	InitCursor();
	OSCursorVisibleFlag = TRUE;
	OSCursorLastCursor = 1;
}

/**********************************

	Is there an OS cursor?

**********************************/

Word BURGERCALL OSCursorPresent(void)
{
	return TRUE;
}

#endif