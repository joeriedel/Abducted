#define DIRECTINPUT_VERSION 0x700
#include "InInput.h"

/**********************************

	Windows 95 version

**********************************/

#if defined(__WIN32__)
#include "W9Win95.h"
#include "GrGraphics.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

/**********************************

	Detect the mouse
	Return true if a mouse is present

**********************************/

Word BURGERCALL MouseInit(void)
{
	MousePresent = TRUE;	/* The OS makes a fake mouse */
	return MousePresent;
}

/**********************************

	Release the mouse

**********************************/

void BURGERCALL MouseDestroy(void)
{
}

/**********************************

	Read the buttons from the mouse

**********************************/

Word BURGERCALL MouseReadButtons(void)
{
	return Win95MouseButton;	/* Return the current mouse state */
}

/**********************************

	Read the mouse x and y
	in LOCAL coordinates!

**********************************/

void BURGERCALL MouseReadAbs(Word *x,Word *y)
{
	int Temp;
	if (x) {		/* Do I want it? */
		Temp = Win95MouseX;	/* Return the value */
		if (Temp<0) {
			Temp = 0;
		}
		if (Temp>=(int)ScreenWidth) {
			Temp = ScreenWidth-1;
		}
		x[0] = Temp;
	}
	if (y) {
		Temp = Win95MouseY;
		if (Temp<0) {
			Temp = 0;
		}
		if (Temp>=(int)ScreenHeight) {
			Temp = ScreenHeight-1;
		}
		y[0] = Temp;
	}
}

/**********************************

	Read the mouse delta movement from the last reading

**********************************/

void BURGERCALL MouseReadDelta(int *x,int *y)
{
	if (x) {
		x[0] = Win95MouseXDelta;			/* Get distance traveled */
	}
	if (y) {
		y[0] = Win95MouseYDelta;
	}
	Win95MouseXDelta = 0;
	Win95MouseYDelta = 0;
	{		/* Do I move the cursor to the center? */
		Word HalfX,HalfY;
		Word TempX,TempY;
		HalfX = VideoTrueScreenWidth>>1;		/* Get the screen center point */
		HalfY = VideoTrueScreenHeight>>1;
		TempX = Win95MouseX;
		TempY = Win95MouseY;
		if ((TempX<(HalfX>>1)) || (TempX>(HalfX+(HalfX>>1))) ||
			(TempY<(HalfY>>1)) || (TempY>(HalfY+(HalfY>>1)))) {
			MouseSetPosition(HalfX,HalfY);	/* Reset the mouse to the center */
		}
	}
}

/**********************************

	Return the mouse wheel value

**********************************/

int BURGERCALL MouseReadWheel(void)
{
	int Result;
	Result = Win95MouseWheel;		/* Get the mouse motion */
	Win95MouseWheel = 0;			/* Reset to zero */
	return Result;					/* Return the wheel value */
}

/**********************************

	Sets the mouse movement bounds

**********************************/

static Word8 Once;		/* Set to true once atexit is installed */

static void ANSICALL KillClipping(void)
{
	ClipCursor(0);		/* Release the cursor on exit */
}

void BURGERCALL MouseSetRange(Word x,Word y)
{
	if (!Once) {		/* Only once? */
		Once = TRUE;
		atexit(KillClipping);	/* Force proper shutdown */
	}
	
	if (VideoFullScreen) {
		RECT NewClip;
		NewClip.left = 0;		/* Set */
		NewClip.top = 0;
		NewClip.right = x;
		NewClip.bottom = y;
		Win95DestScreenX = 0;
		Win95DestScreenY = 0;
		ClipCursor(&NewClip);
	} else {
		POINT pnt;
		pnt.x = 0;	/* Init the point to 0,0 */
		pnt.y = 0;
		if (Win95MainWindow) {		/* Is there a valid window? */
			ClientToScreen((HWND)Win95MainWindow,&pnt);	/* Get the content rect point */
		}
		Win95DestScreenX = pnt.x;		/* Save the position in globals */
		Win95DestScreenY = pnt.y;
		ClipCursor(0);
	}
}

/**********************************

	Set the new mouse position

**********************************/

void BURGERCALL MouseSetPosition(Word x,Word y)
{
	if (MousePresent) {			/* Mouse detected? */
		x += Win95DestScreenX;		/* Set my new location */
		y += Win95DestScreenY;
		Win95MouseX = x;		/* Set my new location */
		Win95MouseY = y;
		Win95LastMouseX = x;
		Win95LastMouseY = y;
		SetCursorPos(x,y);	/* Set the position */
	}
}

#endif
