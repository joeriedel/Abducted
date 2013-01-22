#include "InInput.h"

#if defined(__MAC__)
#include "McMac.h"
#include "GrGraphics.h"
#include "TkTick.h"
#include "ClStdLib.h"
#include "ThThreads.h"
#include <CursorDevices.h>
#include <InputSprocket.h>
#include <Traps.h>
#include <DrawSprocket.h>

#if TARGET_API_MAC_CARBON

/* Prototypes not present in Carbon, But I can call them anyways */

#ifdef __cplusplus
extern "C" {
#endif

extern OSErr CursorDeviceNextDevice(CursorDevicePtr * ourDevice);
extern OSErr CursorDeviceMoveTo(CursorDevicePtr ourDevice,long absX,long absY);

#ifdef __cplusplus
}
#endif

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
#endif

/**********************************

	Read mouse data for Classic
	called via a timer task. Uses low level
	memory to access the cursor position.
	Please note : I am using low memory access since this is classic
	only.

**********************************/

static Word BURGERCALL MouseReadIRQ(void *Input)
{
	MacInput_t *LocalPtr;
	LocalPtr = (MacInput_t *)Input;
	if (MousePresent) {			/* Is the mouse even enabled??? */
		int TempX,TempY;

		/* Ok, since we don't have InputSprocket (Or it's deactivated) */
		
		TempX = ((short *)0x82E)[0];		/* Get the mouse location */
		TempY = ((short *)0x82C)[0];
				
		/* Now I have the mouse position, let's process it */
		
		LocalPtr->LastMouseDeltaX += TempX-LocalPtr->LastMouseX;		/* Get distance traveled */
		LocalPtr->LastMouseDeltaY += TempY-LocalPtr->LastMouseY;

		LocalPtr->LastMouseX = TempX;		/* Save the position */
		LocalPtr->LastMouseY = TempY;
		
		/* Let's try getting the buttons with the cursor device manager */

		{
			Word Temp,Temp2;
					
			if (LocalPtr->CursorDevicePresent) {		/* Present? */
				CursorDevicePtr TheCursor;
			
				Temp = 0;
				TheCursor = 0;
				if (!CursorDeviceNextDevice(&TheCursor) && TheCursor) {
					do {
						Temp |= TheCursor->buttons;
						CursorDeviceNextDevice(&TheCursor);
					} while (TheCursor);
				}
			} else {
		
		/* Oh, you really suck, let's use Direct OS memory! */
		
				Temp = ((Word8 *)0x172)[0];	/* Read the button from MacOS */
			}

			Temp2 = (LocalPtr->LastMouseButton^Temp)&Temp;	/* Mouse down events */
			MouseClicked |= Temp2;
			LocalPtr->LastMouseButton = Temp;			/* Save the mouse button */
		}
	}
	return FALSE;
}

/**********************************

	Read the mouse input if present
	but using InputSprocket events

**********************************/

#if TARGET_RT_MAC_CFM

Word BURGERCALL MouseReadInputSprocket(void *Input)
{
	MacInput_t *LocalPtr;
	LocalPtr = (MacInput_t *)Input;
	if (MousePresent) {			/* Is the mouse even enabled??? */

		/* Let's do it the InputSprocket way! */

		if (LocalPtr->InputSprocketActive &&
			LocalPtr->InputSprocketInited &&
			(LocalPtr->Flags & MACINITINPUTMOUSE)) {
			
			/* Note: InputSprocket is interrupt safe but not reentrant */
			/* So, obtain the lock to see if I am calling InputSprocket and */
			/* skip if I'm inside ISp */
			
			if (!MacInputLockInputSprocket()) {		/* Locked out? */
				ISpElementEvent EventHit;
				Bool HitMe;
				Word i;
				i = LocalPtr->MiceCount;			/* Any mice found? */
				if (i) {
					MacRatEntry_t *WorkPtr;
					WorkPtr = LocalPtr->MiceLists;		/* For each mouse */
					do {
						for (;;) {
							Word j;
							
							/* See if an event was passed to me... */
							HitMe = 0;
							if (ISpElementList_GetNextEvent(WorkPtr->MouseRef,sizeof(EventHit),&EventHit,&HitMe)) {
								break;
							}
							if (!HitMe) {		/* No event? */
								break;
							}
							j = 0;				/* Ok, where shall I pass the event */
							do {
								if (EventHit.element == WorkPtr->MouseElements[j]) {		/* Was it a valid entry? */
									if (j<8) {				/* Button? */
										Word Mask;
										Mask = 1<<j;		/* Mask for button */
										if (EventHit.data) {
											MouseClicked |= Mask;		/* I clicked... */
											LocalPtr->LastMouseButton |= Mask;		/* Set the state */
										} else {
										 	LocalPtr->LastMouseButton &= ~Mask;		/* Clear the state */
										}
										break;
									}
									if (j==8) {					/* X motion? */
										int XMotion;
										XMotion = ((int)EventHit.data)/163;
										LocalPtr->LastMouseDeltaX += XMotion;
										XMotion = LocalPtr->LastMouseX + XMotion;
										if (XMotion<0) {
											XMotion = 0;
										}
										if ((Word)XMotion>=VideoTrueScreenWidth) {
											XMotion = VideoTrueScreenWidth-1;
										}
										LocalPtr->LastMouseX = XMotion;
										break;
									}
									if (j==9) {					/* Y motion */
										int YMotion;
										YMotion = ((int)EventHit.data)/163;
										LocalPtr->LastMouseDeltaY -= YMotion;
										YMotion = LocalPtr->LastMouseY - YMotion;
										if (YMotion<0) {
											YMotion = 0;
										}
										if ((Word)YMotion>=VideoTrueScreenHeight) {
											YMotion = VideoTrueScreenHeight-1;
										}
										LocalPtr->LastMouseY = YMotion;
										break;
									}
									/* It's the wheel */
									LocalPtr->LastMouseDeltaZ -= ((int)EventHit.data)>>12;
									break;
								}
							} while (++j<11);
						}
						++WorkPtr;
					} while (--i);
				}
				MacInputUnlockInputSprocket();
			}

			/* It appears that InputSprocket give mouse ticks at a */
			/* different rate than the cursor */
			/* So if I am running in a window on the desktop, I need */
			/* this STUPID hack to make sure that the cursor x/y matches */
			/* the X/Y on the screen */
			
			{
				Point OutPoint;
				OutPoint.h = LocalPtr->LastMouseX;
				OutPoint.v = LocalPtr->LastMouseY;

				((long *)0x82C)[0] = ((long *)&OutPoint)[0];	/* Set the mouse location */
				((long *)0x828)[0] = ((long *)&OutPoint)[0];	/* Set the mouse temp location */
				((short *)0x8CE)[0] = 0xFFFF;	/* SetCursorNew Alert MacOS the cursor was updated */
//				((Word8 *)0x8CF)[0] = 255;	/* CrsrCouple */
			}
			return FALSE;
		}
		MouseReadIRQ(LocalPtr);		/* Fallback, since InputSprocket was disabled */
	}
	return FALSE;
}
#endif

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

	/* Check if I can use the CursorDevice calls */

#if !TARGET_API_MAC_CARBON
	LocalPtr->LastMouseX = ((short *)0x82E)[0];		/* Get the mouse location */
	LocalPtr->LastMouseY = ((short *)0x82C)[0];

	if (GetToolboxTrapAddress(_CursorDeviceDispatch) != GetToolboxTrapAddress(_Unimplemented)) {
		CursorDevicePtr TheCursor;
		LocalPtr->CursorDevicePresent = TRUE;		/* I use Cursor Device calls */

		/* Now, you ask yourself, why am I calling this function here? It */
		/* makes no sense, well the Timer IRQ proc calls this function */
		/* and if had never been called before, it COULD call the */
		/* shared library manager or Gestalt */
		/* Now, calling Gestalt or the shared library manager */
		/* during an interrupt is a really really really BAD idea */
		/* Placing this call here, will make SURE that the library is */
		/* loaded and primed */
					
		TheCursor = 0;				/* Init the variable */
		CursorDeviceNextDevice(&TheCursor);		/* Get the next device */
	}
#else
	if (MacOSGetOSVersion()<0x1000) {				/* Only classic has it */
		CursorDevicePtr TheCursor;

		LocalPtr->LastMouseX = ((short *)0x82E)[0];		/* Get the mouse location */
		LocalPtr->LastMouseY = ((short *)0x82C)[0];
		LocalPtr->CursorDevicePresent = TRUE;		/* I use Cursor Device calls */
		TheCursor = 0;				/* Init the variable */
		CursorDeviceNextDevice(&TheCursor);		/* Get the next device */
	}
#endif
	
	/* Check if InputSprocket is available */
	
	if (!MacInputInit(LocalPtr,MACINITINPUTMOUSE)) {	/* 1.3 or better? */
#if TARGET_RT_MAC_CFM
		unsigned long TempLong;
		MacInputLockInputSprocket();
		FastMemSet(&LocalPtr->MiceLists[0],0,sizeof(LocalPtr->MiceLists));
		if (!ISpDevices_ExtractByClass(kISpDeviceClass_Mouse,0,&TempLong,0)) {		/* How many mice are present? */
			Word i;
			i = TempLong;
			if (i) {			/* Any mice detected? */
				ISpDeviceReference Rats[4];			/* Rats to load */
				if (i>4) {
					i = 4;	/* I'll only support 4 mice */
				}
				LocalPtr->MiceCount = i;
				if (!ISpDevices_ExtractByClass(kISpDeviceClass_Mouse,i,&TempLong,Rats)) {	/* Get the devices */
					Word j;
					MacRatEntry_t *JoyDevPtr;
					/* At this point, I have all the mice devices, now parse out the info */
					j = 0;
					JoyDevPtr = LocalPtr->MiceLists;
					do {

						/* For each device, scan the input elements and find a burgerlib */
						/* match, assign it a button, pad or axis id */

						if (!ISpDevice_GetElementList(Rats[j],&JoyDevPtr->MouseRef)) {
							unsigned long ElementCount;
							ISpElementReference ElementBuf[500];
							if (!ISpElementList_Extract(JoyDevPtr->MouseRef,500,&ElementCount,ElementBuf)) {
								if (ElementCount) {
									Word TempIndex;
									Word ButtonIndex;

									TempIndex = 0;
									ButtonIndex = 0;	/* No buttons found */
									do {
										ISpElementInfo ElementInfo;
										ISpElementReference CurrentRef;

										CurrentRef = ElementBuf[TempIndex];
										if (!ISpElement_GetInfo(CurrentRef,&ElementInfo)) {
											/* I've got input, what kind of input is it? */

											switch (ElementInfo.theKind) {
											case kISpElementKind_Button:		/* Simple button */
												if (ButtonIndex<8) {
													JoyDevPtr->MouseElements[ButtonIndex] = CurrentRef;
													++ButtonIndex;
												}
												break;

											case kISpElementKind_Delta:
												switch (ElementInfo.theLabel) {
												case kISpElementLabel_Delta_X:
													JoyDevPtr->MouseElements[8] = CurrentRef;
													break;
												case kISpElementLabel_Delta_Y:
													JoyDevPtr->MouseElements[9] = CurrentRef;
													break;
												case kISpElementLabel_Delta_Z:
													JoyDevPtr->MouseElements[10] = CurrentRef;
													break;
												}
												break;
											}
										}
									} while (++TempIndex<ElementCount);
								}
							}
						}
						++JoyDevPtr;
					} while (++j<i);			/* All devices scanned */
				}
			}
		}
		ISpDevices_ActivateClass(kISpDeviceClass_Mouse);	/* Enable mice */
		MacInputUnlockInputSprocket();
		MousePresent = TRUE;		/* Allow the IRQ to call InputSprocket */
		LocalPtr->MouseTimerProc = TimerTaskNew(16,MouseReadInputSprocket,LocalPtr,TRUE);
#endif
	}
#if TARGET_API_MAC_CARBON
	else if (!LocalPtr->MouseEventRef && MacOSGetOSVersion()>=0x1000) {
		EventHandlerUPP MouseProcUPP;
		MouseProcUPP = NewEventHandlerUPP(MouseProc);
		LocalPtr->MouseCarbonProc = MouseProcUPP;
		InstallEventHandler(GetApplicationEventTarget(),MouseProcUPP,5,EventTypes,&MacInputLocals,&LocalPtr->MouseEventRef);
	}
#endif
	else {
		LocalPtr->MouseTimerProc = TimerTaskNew(16,MouseReadIRQ,LocalPtr,TRUE);
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

	if (LocalPtr->MouseTimerProc) {
		TimerTaskDelete(LocalPtr->MouseTimerProc);	/* Kill the timer IRQ thread */
		LocalPtr->MouseTimerProc = 0;
	}
	
	if (LocalPtr->Flags & MACINITINPUTMOUSE) {
#if TARGET_RT_MAC_CFM
		if (LocalPtr->InputSprocketActive) {
			ISpDevices_DeactivateClass(kISpDeviceClass_Mouse);	/* Disable mice support */
		}
#endif
		MacInputDestroy(LocalPtr,MACINITINPUTMOUSE);	/* Release my data */
	}

	/* MacOS X Carbon events */
	
#if TARGET_API_MAC_CARBON
	if (LocalPtr->MouseEventRef) {
		RemoveEventHandler((EventHandlerRef)LocalPtr->MouseEventRef);
		LocalPtr->MouseEventRef = 0;
	}
	if (LocalPtr->MouseCarbonProc) {
		DisposeEventHandlerUPP((EventHandlerUPP)LocalPtr->MouseCarbonProc);
		LocalPtr->MouseCarbonProc = 0;
	}
#endif

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
#if TARGET_RT_MAC_CFM
	if (MacContext) {		/* Is DrawSprocket active? */
		DSpContext_LocalToGlobal(MacContext,&MousePoint);
	} else 
#endif
		{
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
		
		if (LocalPtr->MouseEventRef || LocalPtr->MouseTimerProc) {		/* Using input sprocket? */
			LocalPtr->LastMouseX = x;			/* Simply update my internal position */
			LocalPtr->LastMouseY = y;
			LocalPtr->LastMouseDeltaX = 0;		/* Kill any delta motion */
			LocalPtr->LastMouseDeltaY = 0;
			return;
		}
		
	/* Now I have the cursor in global coordinates, now send it to the mouse driver */
	
		if (MacOSGetOSVersion()<0x1000) {		/* Only works in Classic */
			Point MousePoint;

			if (LocalPtr->CursorDevicePresent) {	/* Shall I use devices? */
				CursorDevicePtr TheCursor;
			
				TheCursor = 0;				/* Init the variable */
				CursorDeviceNextDevice(&TheCursor);		/* Get the next device */
				if (TheCursor) {		/* Got a live one? */
					do {
						CursorDeviceMoveTo(TheCursor,x,y);	/* Set the position */
						CursorDeviceNextDevice(&TheCursor);	/* Move all devices */
					} while (TheCursor);		/* More devices? */
					return;				/* Exit now! */
				}
			}
	
		/* Do it the old fashioned way! */

			MousePoint.h = x;
			MousePoint.v = y;

			((long *)0x82C)[0] = ((long *)&MousePoint)[0];	/* Set the mouse location */
			((long *)0x828)[0] = ((long *)&MousePoint)[0];	/* Set the mouse temp location */
			((short *)0x8CE)[0] = 0xFFFF;	/* SetCursorNew Alert MacOS the cursor was updated */
//			((Word8 *)0x8CF)[0] = 255;	/* CrsrCouple */
		}
	}
}

#endif
