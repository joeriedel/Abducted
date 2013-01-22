#include "OcOSCursor.h"

#if defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "W9Win95.h"

/**********************************

	Load an OS resource and set the cursor to it

**********************************/

void BURGERCALL OSCursorSet(Word Curnum)
{
	if (Curnum!=OSCursorLastCursor) {
		HCURSOR curs;
		curs = LoadCursor((HINSTANCE)Win95Instance,MAKEINTRESOURCE(Curnum));	/* Application */
		if (curs) {				/* Did it load? */
			goto DoIt;
		}
		curs = LoadCursor(0,MAKEINTRESOURCE(Curnum));		/* Win95 */
		if (curs) {			/* Got it now! */
DoIt:
			OSCursorLastCursor = Curnum;
			SetCursor(curs);	/* Set it */
		}
	}
}

/**********************************

	Make an OS cursor visible

**********************************/

void BURGERCALL OSCursorShow(void)
{
	if (!OSCursorVisibleFlag) {
		int State;
		OSCursorVisibleFlag = TRUE;		/* It's visible */
		do {
			State=ShowCursor(TRUE);	/* Inc the flag */
		} while (State<0);		/* Make sure it's visible! */
	}
}

/**********************************

	Make an OS cursor disappear

**********************************/

void BURGERCALL OSCursorHide(void)
{
	if (OSCursorVisibleFlag) {
		int State;
		OSCursorVisibleFlag = FALSE;
		do {
			State=ShowCursor(FALSE);	/* Hide it */
		} while (State>=0);		/* Make sure it's hidden */
	}
}

/**********************************

	Reset an OS cursor to an arrow and show it

**********************************/

void BURGERCALL OSCursorReset(void)
{
	OSCursorSet((Word)IDC_ARROW);	/* Set to the arrow */
	OSCursorVisibleFlag = FALSE;	/* Force the cursor to be shown */
	OSCursorShow();				/* Make sure it's visible! */
}

/**********************************

	Is there an OS cursor?

**********************************/

Word BURGERCALL OSCursorPresent(void)
{
	return TRUE;
}

#endif