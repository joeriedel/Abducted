#include "OcOSCursor.h"

/**********************************

	TRUE if an OS cursor is visible

**********************************/

Bool OSCursorVisibleFlag = TRUE;		/* I assume the cursor is visible */

/**********************************

	Last cursor set

**********************************/

Word OSCursorLastCursor;		/* Set by OSCursorSet */

/**********************************

	Return the current state of the visible flag

**********************************/

Word BURGERCALL OSCursorIsVisible(void)
{
	return OSCursorVisibleFlag;
}

/**********************************

	Return the current cursor shape number

**********************************/

Word BURGERCALL OSCursorNumber(void)
{
	return OSCursorLastCursor;
}

#if !defined(__MAC__) && !defined(__WIN32__)

/**********************************

	Load an OS resource and set the cursor to it

**********************************/

void BURGERCALL OSCursorSet(Word Curnum)
{
	OSCursorLastCursor = Curnum;
}

/**********************************

	Make an OS cursor visible

**********************************/

void BURGERCALL OSCursorShow(void)
{
	OSCursorVisibleFlag = TRUE;		/* It's visible */
}

/**********************************

	Make an OS cursor disappear

**********************************/

void BURGERCALL OSCursorHide(void)
{
	OSCursorVisibleFlag = FALSE;
}

/**********************************

	Reset an OS cursor to an arrow and show it

**********************************/

void BURGERCALL OSCursorReset(void)
{
	OSCursorVisibleFlag = TRUE;
	OSCursorLastCursor = 1;
}

/**********************************

	Is there an OS cursor?

**********************************/

Word BURGERCALL OSCursorPresent(void)
{
	return FALSE;		/* No cursor is truly present */
}

#endif