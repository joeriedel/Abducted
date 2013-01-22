#include "InInput.h"

/**********************************

	True if a mouse is present

**********************************/

Word MousePresent = FALSE;

/**********************************

	Was the mouse "Clicked" between reads of ReadMouseButtons?

**********************************/

Word MouseClicked;		/* TRUE if clicked, FALSE if not */

/**********************************

	Generic OS code

**********************************/

#if !defined(__MSDOS__) && !defined(__MAC__) && !defined(__WIN32__) && !defined(__MACOSX__)

/**********************************

	Detect the mouse
	Return true if a mouse is present

**********************************/

Word BURGERCALL MouseInit(void)
{
	MousePresent = FALSE;	/* The OS makes a fake mouse */
	return MousePresent;
}

/**********************************

	Shut down the mouse manager
	
**********************************/

void BURGERCALL MouseDestroy(void)
{
}

/**********************************

	Read the buttons from the mouse

**********************************/

Word BURGERCALL MouseReadButtons(void)
{
	return 0;
}

/**********************************

	Read the mouse x and y
	in LOCAL coordinates!

**********************************/

void BURGERCALL MouseReadAbs(Word *x,Word *y)
{
	if (x) {
		*x = 0;
	}
	if (y) {
		*y = 0;
	}
}

/**********************************

	Read the mouse delta movement from the last reading

**********************************/

void BURGERCALL MouseReadDelta(int *x,int *y)
{
	if (x) {
		*x = 0;			/* Get distance traveled */
	}
	if (y) {
		*y = 0;
	}
}

/**********************************

	Sets the mouse movement bounds

**********************************/

void BURGERCALL MouseSetRange(Word x,Word y)
{
}

/**********************************

	Set the new mouse position

**********************************/

void BURGERCALL MouseSetPosition(Word x,Word y)
{
}

#endif

/**********************************

	Read the mouse wheel

**********************************/

#if !defined(__WIN32__) && !defined(__MAC__) && !defined(__MACOSX__)
int BURGERCALL MouseReadWheel(void)
{
	return 0;
}
#endif
