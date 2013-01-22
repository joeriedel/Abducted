#include "InInput.h"

#if defined(__MACOSX__)
#include <stdbool.h>
#include "McMac.h"
#include "GrGraphics.h"
#include "TkTick.h"
#include "ClStdLib.h"
#include "ThThreads.h"
#include <DrawSprocket/DrawSprocket.h>

/* List of events I support for OS X */

static const EventTypeSpec EventTypes[5] = {
	{kEventClassMouse,kEventMouseDown},
	{kEventClassMouse,kEventMouseUp},
	{kEventClassMouse,kEventMouseWheelMoved},
	{kEventClassMouse,kEventMouseMoved},
	{kEventClassMouse,kEventMouseDragged}
};

/**********************************

	Carbon event handler for mouse events
	Use for MacOS X

**********************************/

static pascal OSStatus MouseProc(EventHandlerCallRef inHandlerCallRef,EventRef inEvent,void *inUserData)
{
	MacInput_t *LocalPtr;
	Word32 EventKind;
	
	LocalPtr = (MacInput_t *)inUserData;		/* Pointer to local variables */
	EventKind = GetEventKind(inEvent);			/* Type of event accepted */
	switch (EventKind) {
	
	/* Mouse up or down events */
	
	case kEventMouseDown:
	case kEventMouseUp:
		{
			UInt16 whichButton;		/* Mouse button returned */
			Word Mask;
			GetEventParameter(inEvent,kEventParamMouseButton,
				typeMouseButton,0,sizeof(whichButton),0,&whichButton);
			Mask = whichButton-1;	/* Convert 1-32 to 0-31 */
			if (Mask<32) {			/* Which button */
				Mask = 1<<Mask;		/* Create the mouse button mask */
				if (EventKind==kEventMouseDown) {		/* Mouse down */
					LocalPtr->LastMouseButton |= Mask;
					MouseClicked |= Mask;				/* Set the click event */
				} else {
					LocalPtr->LastMouseButton &= ~Mask;	/* Mouse up */
				}
			}
		}
		break;
		
	/* Mouse movement */
	
	case kEventMouseMoved:
	case kEventMouseDragged:
		{
			Point Where;
			GetEventParameter(inEvent,kEventParamMouseDelta,
				typeQDPoint,0,sizeof(Where),0,&Where);
			LocalPtr->LastMouseDeltaX += Where.h;
			LocalPtr->LastMouseDeltaY += Where.v;

			GetEventParameter(inEvent,kEventParamMouseLocation,
				typeQDPoint,0,sizeof(Where),0,&Where);
			LocalPtr->LastMouseX = Where.h;
			LocalPtr->LastMouseY = Where.v;
		}
		break;
		
	/* Mouse wheel movement */
	
	case kEventMouseWheelMoved:
		{
			long Movement;
			GetEventParameter(inEvent,kEventParamMouseWheelDelta,
				typeLongInteger,0,sizeof(Movement),0,&Movement);
			LocalPtr->LastMouseDeltaZ += Movement;
		}
		break;
	}
	return eventNotHandledErr;
}

/**********************************

	Detect the mouse
	Return true if a mouse is present

**********************************/

Word BURGERCALL MouseInit(void)
{
	MacInput_t *LocalPtr;

	if (MousePresent) {		/* Why are we installing again? */
		return MousePresent;
	}
	LocalPtr = &MacInputLocals;
	
	if (!LocalPtr->MouseEventRef) {
		EventHandlerUPP MouseProcUPP;
		MouseProcUPP = NewEventHandlerUPP(MouseProc);
		LocalPtr->MouseCarbonProc = MouseProcUPP;
		InstallEventHandler(GetApplicationEventTarget(),MouseProcUPP,5,EventTypes,&MacInputLocals,&LocalPtr->MouseEventRef);
	}
	/* Set the bounds if not already set */
	
	if (!LocalPtr->MouseMaxX) {		/* Not initialized? */
		if (ScreenWidth) {
			LocalPtr->MouseMaxX = ScreenWidth;	/* Init the bounds rect */
			LocalPtr->MouseMaxY = ScreenHeight;
		} else {
			LocalPtr->MouseMaxX = 4096;	/* Init the bounds rect */
			LocalPtr->MouseMaxY = 3072;
		}
	}
	MousePresent = TRUE;	/* Mouse services are on, allow the IRQ to service */
	return TRUE;
}

/**********************************

	Release the mouse
	
**********************************/

void BURGERCALL MouseDestroy(void)
{
	MacInput_t *LocalPtr;
	
	LocalPtr = &MacInputLocals;
	
	/* Release my InputSprocket timer task */
	
	MousePresent = 0;						/* Shut off mouse services IRQ */
	
	/* MacOS X Carbon events */
	
	if (LocalPtr->MouseEventRef) {
		RemoveEventHandler(LocalPtr->MouseEventRef);
		LocalPtr->MouseEventRef = 0;
	}
	if (LocalPtr->MouseCarbonProc) {
		DisposeEventHandlerUPP((EventHandlerUPP)LocalPtr->MouseCarbonProc);
		LocalPtr->MouseCarbonProc = 0;
	}

	/* Shut down */
	
	LocalPtr->LastMouseButton = 0;			/* Make sure I am zero'd out */
	LocalPtr->LastMouseX = 0;
	LocalPtr->LastMouseY = 0;
	LocalPtr->LastMouseDeltaX = 0;
	LocalPtr->LastMouseDeltaY = 0;
	LocalPtr->LastMouseDeltaZ = 0;
}

/**********************************

	Read the buttons from the mouse
	Interrupt safe

**********************************/

Word BURGERCALL MouseReadButtons(void)
{
	MacInput_t *LocalPtr;
	
	LocalPtr = &MacInputLocals;
	if (LocalPtr->MouseEventRef) {		/* OS X needs to yield CPU time */
		KeyboardKbhit();
	}
	return LocalPtr->LastMouseButton;	/* Return the current button state */
}

/**********************************

	Read the mouse wheel
	Interrupt safe

**********************************/

int BURGERCALL MouseReadWheel(void)
{
	MacInput_t *LocalPtr;
	int Temp;

	LocalPtr = &MacInputLocals;
	Temp = LocalPtr->LastMouseDeltaZ;		/* Get the last read Z value */
	LocalPtr->LastMouseDeltaZ = 0;			/* Reset the value */
	return Temp;							/* Return the motion */
}

/**********************************

	Read the mouse x and y
	in LOCAL coordinates!
	Interrupt safe

**********************************/

void BURGERCALL MouseReadAbs(Word *x,Word *y)
{
	MacInput_t *LocalPtr;

	LocalPtr = &MacInputLocals;
	if (x) {
		int Temp;
		Temp = LocalPtr->LastMouseX-LocalPtr->MouseXBase;	/* Clip the coordinates */
		if (Temp<0) {
			Temp = 0;
		}
		if (Temp>=LocalPtr->MouseMaxX) {
			Temp = LocalPtr->MouseMaxX-1;
		}
		x[0] = Temp;
	}
	if (y) {
		int Temp;
		Temp = LocalPtr->LastMouseY-LocalPtr->MouseYBase;
		if (Temp<0) {
			Temp = 0;
		}
		if (Temp>=LocalPtr->MouseMaxY) {
			Temp = LocalPtr->MouseMaxY-1;
		}
		y[0] = Temp;
	}
}

/**********************************

	Read the mouse delta movement from the last reading
	Interrupt safe

**********************************/

void BURGERCALL MouseReadDelta(int *x,int *y)
{
	MacInput_t *LocalPtr;

	LocalPtr = &MacInputLocals;
	if (x) {
		x[0] = LocalPtr->LastMouseDeltaX;	/* Get distance traveled */
	}
	if (y) {
		y[0] = LocalPtr->LastMouseDeltaY;
	}
	LocalPtr->LastMouseDeltaX = 0;			/* Reset distance traveled */
	LocalPtr->LastMouseDeltaY = 0;
	{		/* Do I move the cursor to the center? */
		Word HalfX,HalfY;
		Word TempX,TempY;
		HalfX = VideoTrueScreenWidth>>1;		/* Get the screen center point */
		HalfY = VideoTrueScreenHeight>>1;
		TempX = LocalPtr->LastMouseX;
		TempY = LocalPtr->LastMouseY;
		if ((TempX<(HalfX>>1)) || (TempX>(HalfX+(HalfX>>1))) ||
			(TempY<(HalfY>>1)) || (TempY>(HalfY+(HalfY>>1)))) {
			LocalPtr->CenterMouseFlag = TRUE;
		}
	}
}

/**********************************

	Sets the mouse movement bounds

**********************************/

void BURGERCALL MouseSetRange(Word x,Word y)
{
	MacInput_t *LocalPtr;
	Point MousePoint;
	
	LocalPtr = &MacInputLocals;
	LocalPtr->MouseMaxX = x;		/* Set the bounds to clamp to */
	LocalPtr->MouseMaxY = y;

	MousePoint.h = 0;
	MousePoint.v = 0;
	if (MacContext) {		/* Is DrawSprocket active? */
		DSpContext_LocalToGlobal(MacContext,&MousePoint);
	} else  {
		GrafPtr OldPort;
		if (VideoWindow) {		/* Failsafe */
			GetPort(&OldPort);		/* Get the previous video port */
			SetPortWindowPort(VideoWindow);	/* Set my main port */
		}
		LocalToGlobal(&MousePoint);	/* Read the mouse in LOCAL coordinates */
		if (VideoWindow) {
			SetPort(OldPort);		/* Restore the grafport */
		}
	}
	LocalPtr->MouseXBase = MousePoint.h;
	LocalPtr->MouseYBase = MousePoint.v;

}

/**********************************

	Set the new mouse position

**********************************/

void BURGERCALL MouseSetPosition(Word x,Word y)
{
	MacInput_t *LocalPtr;
	
	LocalPtr = &MacInputLocals;
	if (MousePresent) {				/* Mouse services initialized? */
		
		x += LocalPtr->MouseXBase;		/* Copy the point into a structure to send to MacOS */
		y += LocalPtr->MouseYBase;
		
		if (LocalPtr->MouseEventRef) {		/* Using input sprocket? */
			LocalPtr->LastMouseX = x;			/* Simply update my internal position */
			LocalPtr->LastMouseY = y;
			LocalPtr->LastMouseDeltaX = 0;		/* Kill any delta motion */
			LocalPtr->LastMouseDeltaY = 0;
			return;
		}
	}
}

#endif
