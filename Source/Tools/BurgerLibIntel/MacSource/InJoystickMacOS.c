/**********************************

	MacOS version of joystick input.
	I use InputSpocket to read the joystick info

**********************************/

#include "InInput.h"

#if defined(__MAC__)
#include "ClStdLib.h"
#include "TkTick.h"
#include "McMac.h"
#include "MmMemory.h"
#include "ThThreads.h"
#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif

MacInput_t MacInputLocals;		/* Local state data of MacInput */

#if TARGET_RT_MAC_CFM
#include <InputSprocket.h>

#define ELEMENTBUTTON 0		/* Indexs to element arrays */
#define ELEMENTPAD 20		/* 2 pads */
#define ELEMENTAXIS 22		/* 6 axis */
#define ELEMENTCOUNT 28		/* Total */

typedef struct JoyDesc_t {
	ISpElementReference Elements[ELEMENTCOUNT];	/* Input sprocket references */
	unsigned long ElementData[ELEMENTCOUNT];			/* Actual data returned */
} JoyDesc_t;

extern Word MacLastKeyDown;		/* Post keydown event */

/* This is the enumeration of input buttons to */
/* Burgerlib button flags */

static const Word32 ElementFlag[ELEMENTPAD-ELEMENTBUTTON] = {	/* Button count */
	PadButton1,
	PadButton2,
	PadButton3,
	PadButton4,
	PadButton5,
	PadButton6,
	PadButton7,
	PadButton8,
	PadButton9,
	PadButton10,
	PadButton11,
	PadButton12,
	PadButton13,
	PadButton14,
	PadButton15,
	PadButton16,
	PadButton17,
	PadButton18,
	PadButton19,
	PadButton20
};

/* Translate an inputsprocket pad (0-8) into pad bits */

static const Word32 PadValues[2][9] = {	/* Joypad/hat bits */
	{0,PadLeft,PadLeft+PadUp,PadUp,PadUp+PadRight,
	PadRight,PadRight+PadDown,PadDown,PadDown+PadLeft},
	{0,PadHatLeft,PadHatLeft+PadHatUp,PadHatUp,PadHatUp+PadHatRight,
	PadHatRight,PadHatRight+PadHatDown,PadHatDown,PadHatDown+PadHatLeft}
};

/**********************************

	Init inputsprocket

**********************************/

Word BURGERCALL MacInputInit(MacInput_t *Input,Word Flags)
{
	if (!Input->InputSprocketInited) {		/* Not started? */
		if (MacOSGetInputSprocketVersion()>=0x130) {	/* 1.3 or better? */
			if (Input->InputSprocketInited || !ISpStartup()) {					/* Start up input sprocket */
				Input->Flags |= Flags;				/* Set the requester */
				Input->InputSprocketInited = TRUE;	/* Set the flag for later shutdown */
				Input->InputSprocketActive = TRUE;	/* Enable sprockets */
				return FALSE;	/* I'm ok */
			}
		}
		return TRUE;			/* Error in startup */
	}
	Input->Flags |= Flags;					/* Set the requester */
	return FALSE;				/* I'm ok */
}

/**********************************

	Release inputsprocket

**********************************/

void BURGERCALL MacInputDestroy(MacInput_t *Input,Word Flags)
{
	Flags = Input->Flags & (~Flags);
	Input->Flags = Flags;
	if (!Flags) {
		if (Input->InputSprocketInited) {		/* Is input sprocket started up? */
			Input->InputSprocketInited = FALSE;	/* Ack the flag */
			Input->InputSprocketActive = FALSE;
			ISpShutdown();			/* Release input sprocket services */
		}
	}
}

/**********************************

	Obtain the Mutex lock to call InputSprocket

**********************************/

Word BURGERCALL MacInputLockInputSprocket(void)
{
	return MutexLock(&MacInputLocals.InputSprocketMutex);
}

/**********************************

	Release the Mutex lock to call InputSprocket

**********************************/

void BURGERCALL MacInputUnlockInputSprocket(void)
{
	MutexUnlock(&MacInputLocals.InputSprocketMutex);
}

/**********************************

	Put Inputsprocket to sleep

**********************************/

Word BURGERCALL InputSetState(Word NewState)
{
	MacInput_t *LocalPtr;
	LocalPtr = &MacInputLocals;
	if (LocalPtr->InputSprocketInited) {
		if (LocalPtr->InputSprocketActive) {
			if (!NewState) {
				MacLastKeyDown = 0;
				KeyboardFlush();
				LocalPtr->InputSprocketActive = FALSE;	/* Kill IRQ */
				ISpSuspend();			/* Shut down services */
				KeyboardFlush();
				FlushEvents(everyEvent,0);
			}
			return TRUE;					/* Previous state was active */
		}
		if (NewState) {
			MacLastKeyDown = 0;
			KeyboardFlush();
			FlushEvents(everyEvent,0);
			if (!ISpResume()) {				/* Resume services */
				LocalPtr->InputSprocketActive = TRUE;		/* Allow IRQ */
			}
			KeyboardFlush();
			MacLastKeyDown = 0;
		}
	}
	return FALSE;				/* Previous state was inactive */
}

/**********************************

	Return the current state of InputSprocket
	
**********************************/

Word BURGERCALL InputGetState(void)
{
	MacInput_t *LocalPtr;
	LocalPtr = &MacInputLocals;
	if (LocalPtr->InputSprocketInited &&		/* Input sprocket enabled? */
		LocalPtr->InputSprocketActive) {		/* Get the state */
		return TRUE;
	}
	return FALSE;
}

/**********************************

	Init Joystick services

**********************************/

Word BURGERCALL JoystickInit(void)
{
	unsigned long TempLong;
	Word i;
	MacInput_t *LocalPtr;
	
	LocalPtr = &MacInputLocals;
	if (!MacInputInit(LocalPtr,MACINITINPUTJOYSTICK)) {	/* 1.3 or better? */
	
		/* Discard any previous data from a previous call */
		/* This will allow simple rescanning */

		DeallocAPointer(LocalPtr->JoystickDescriptionsArray);
		DeallocAPointer(LocalPtr->JoystickDeviceArray);
		LocalPtr->JoystickDescriptionsArray = 0;
		LocalPtr->JoystickDeviceArray = 0;
		{
			Word *Boundaries;
			i = 0;
			Boundaries = &JoystickBoundaries[0][0];		/* Init pointer to struct */
			do {
				Word j;
				j = 0;
				do {
					Boundaries[AXISMIN] = 0x100000;	/* Init center point */
					Boundaries[AXISMAX] = 0xF00000;
					Boundaries[AXISCENTER] = 0x800000;
					Boundaries+=AXISENTRIES;
					JoystickSetDigital(j,20,i);		/* Create the digital bounds */
				} while (++j<AXISCOUNT);
			} while (++i<MAXJOYNUM);		/* All of them checked? */
		}

		MacInputLockInputSprocket();				/* Don't allow mouse/keyboard IRQ's */
		
		if (ISpDevices_Extract(0,&TempLong,0)) {		/* How many devices are present? */
			goto Abort;
		}
		i = TempLong;				/* Save in register */
		LocalPtr->JoystickDeviceArray = (ISpDeviceReference *)AllocAPointer(sizeof(ISpDeviceReference)*i);
		if (!LocalPtr->JoystickDeviceArray) {		/* Memory allocation error? */
			goto Abort;
		}
		if (ISpDevices_Extract(i,&TempLong,LocalPtr->JoystickDeviceArray)) {	/* Get the devices */
			goto Abort;
		}

		/* Now remove all devices that are not mice or keyboards */

		JoystickPresent = i;		/* Save the device count */
		if (i) {
			ISpDeviceReference *ArrayPtr;
			ISpDeviceReference *ArrayPtr2;

			ArrayPtr = LocalPtr->JoystickDeviceArray;
			ArrayPtr2 = ArrayPtr;
			do {
				ISpDeviceDefinition ResultBuf;		/* Get the device definition */
				if (!ISpDevice_GetDefinition(ArrayPtr[0],sizeof(ISpDeviceDefinition),&ResultBuf)) {
					if (ResultBuf.theDeviceClass != kISpElementLabel_None) {
						if ((ResultBuf.theDeviceClass != kISpDeviceClass_Mouse) &&
							(ResultBuf.theDeviceClass != kISpDeviceClass_Keyboard)) {
							ArrayPtr2[0] = ArrayPtr[0];
							++ArrayPtr2;
						}
					}
				}
				++ArrayPtr;
			} while (--i);
			JoystickPresent = ArrayPtr2-LocalPtr->JoystickDeviceArray;
		}

		
		/* At this point I have all the devices I am going to use. Now */
		/* I will get the actual input methods and create a map for each one */

		i = JoystickPresent;
		if (i) {
			Word j;
			JoyDesc_t *JoyDevPtr;

			if (i>MAXJOYNUM) {			/* I can only have 4 input devices */
				i = MAXJOYNUM;
				JoystickPresent = MAXJOYNUM;
			}

			JoyDevPtr = (JoyDesc_t *)AllocAPointerClear(sizeof(JoyDesc_t)*i);
			if (!JoyDevPtr) {
				goto Abort;
			}
			j = 0;
			LocalPtr->JoystickDescriptionsArray = JoyDevPtr;
			do {

				/* For each device, scan the input elements and find a burgerlib */
				/* match, assign it a button, pad or axis id */

				ISpElementListReference MyElementList;
				if (!ISpDevice_GetElementList(LocalPtr->JoystickDeviceArray[j],&MyElementList)) {
					unsigned long ElementCount;
					ISpElementReference ElementBuf[500];
					if (!ISpElementList_Extract(MyElementList,500,&ElementCount,ElementBuf)) {
						if (ElementCount) {
							Word TempIndex;
							Word FooIndex;
							Word AxisIndex;
							Word ButtonIndex;

							TempIndex = 0;
							AxisIndex = 0;		/* No axis' found */
							ButtonIndex = 0;	/* No buttons found */
							do {
								ISpElementInfo ElementInfo;
								ISpElementReference CurrentRef;

								CurrentRef = ElementBuf[TempIndex];
								if (!ISpElement_GetInfo(CurrentRef,&ElementInfo)) {
									/* I've got input, what kind of input is it? */

									switch (ElementInfo.theKind) {
									case kISpElementKind_Button:		/* Simple button */
										if (ButtonIndex<20) {
											JoyDevPtr->Elements[ButtonIndex+ELEMENTBUTTON] = CurrentRef;
											++ButtonIndex;
										}
										break;

									/* Hat or pad */

									case kISpElementKind_DPad:			/* Hat/Pad */

									/* If the ID is a hat, place it in the hat slot first */

										if (ElementInfo.theLabel==kISpElementLabel_Pad_POV ||
											ElementInfo.theLabel==kISpElementLabel_Pad_POV_Horiz) {
											if (!JoyDevPtr->Elements[ELEMENTPAD+1]) {
												JoyDevPtr->Elements[ELEMENTPAD+1] = CurrentRef;
												break;
											}
										}
										FooIndex = 0;
										do {
											if (!JoyDevPtr->Elements[FooIndex+ELEMENTPAD]) {
												 JoyDevPtr->Elements[FooIndex+ELEMENTPAD] = CurrentRef;
												 break;
											}
										} while (++FooIndex<2);
										break;

									/* Handle joystick axis' */
									
									case kISpElementKind_Axis:			/* Joystick axis */
									case kISpElementKind_Delta:
										if (ElementInfo.theLabel==kISpElementLabel_Axis_XAxis) {
											if (!JoyDevPtr->Elements[ELEMENTAXIS]) {
												JoyDevPtr->Elements[ELEMENTAXIS] = CurrentRef;
												JoyDevPtr->ElementData[ELEMENTAXIS] = 0x7FFFFFFF;
												break;
											}
											if (!JoyDevPtr->Elements[ELEMENTAXIS+4]) {
												JoyDevPtr->Elements[ELEMENTAXIS+4] = CurrentRef;
												JoyDevPtr->ElementData[ELEMENTAXIS+4] = 0x7FFFFFFF;
												break;
											}
										} else if (ElementInfo.theLabel==kISpElementLabel_Axis_YAxis) {
											if (!JoyDevPtr->Elements[ELEMENTAXIS+1]) {
												JoyDevPtr->Elements[ELEMENTAXIS+1] = CurrentRef;
												JoyDevPtr->ElementData[ELEMENTAXIS+1] = 0x7FFFFFFF;
												break;
											}
											if (!JoyDevPtr->Elements[ELEMENTAXIS+5]) {
												JoyDevPtr->Elements[ELEMENTAXIS+5] = CurrentRef;
												JoyDevPtr->ElementData[ELEMENTAXIS+5] = 0x7FFFFFFF;
												break;
											}
										} else if (ElementInfo.theLabel==kISpElementLabel_Axis_ZAxis) {
											if (!JoyDevPtr->Elements[ELEMENTAXIS+2]) {
												JoyDevPtr->Elements[ELEMENTAXIS+2] = CurrentRef;
												JoyDevPtr->ElementData[ELEMENTAXIS+2] = 0x7FFFFFFF;
												break;
											}
										}
										/* Insert in the generic list */
										
										if (AxisIndex<AXISCOUNT) {
											do {
												if (!JoyDevPtr->Elements[AxisIndex+ELEMENTAXIS]) {
													JoyDevPtr->Elements[AxisIndex+ELEMENTAXIS] = CurrentRef;
													JoyDevPtr->ElementData[AxisIndex+ELEMENTAXIS] = 0x7FFFFFFF;
													break;
												}
											} while (++AxisIndex<AXISCOUNT);
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
		MacInputUnlockInputSprocket();
		return JoystickPresent;
	}
Abort:;
	MacInputUnlockInputSprocket();
	JoystickDestroy();
	return 0;
}

/**********************************

	Shut down joystick services

**********************************/

void BURGERCALL JoystickDestroy(void)
{
	MacInput_t *LocalPtr;
	LocalPtr = &MacInputLocals;
	MacInputDestroy(LocalPtr,MACINITINPUTJOYSTICK);
	DeallocAPointer(LocalPtr->JoystickDescriptionsArray);	/* Release the input sources */
	DeallocAPointer(LocalPtr->JoystickDeviceArray);		/* Release the devices */

	LocalPtr->JoystickDescriptionsArray = 0;
	LocalPtr->JoystickDeviceArray = 0;
	JoystickPresent = 0;		/* No joysticks are present */
}

/**********************************

	Convert the joystick info into digital data

**********************************/

Word32 BURGERCALL JoystickReadButtons(Word Which)
{
	Word32 Result;
	Result = 0;
	if (Which<JoystickPresent) {		/* Valid joystick? */
		JoyDesc_t *JoyPtr;
		Word i;

		JoystickReadNow(Which);			/* Force a data read */
		JoyPtr = MacInputLocals.JoystickDescriptionsArray+Which;	/* Index to the data */

		/* Read all the simple buttons */
		i = ELEMENTBUTTON;
		do {
			if (JoyPtr->ElementData[i]) {
				Result |= ElementFlag[i-ELEMENTBUTTON];
			}
		} while (++i<(ELEMENTBUTTON+20));

		/* Read in the pad/hat */
		i = ELEMENTPAD;
		do {
			if (JoyPtr->ElementData[i]) {
				Result |= PadValues[i-ELEMENTPAD][JoyPtr->ElementData[i]];
			}
		} while (++i<(ELEMENTPAD+2));

		/* Convert analog directions to digital info */
		
		{
			Word32 Cache;
			Word *Boundary;
			Boundary = &JoystickBoundaries[Which][0];
			if (!JoyPtr->Elements[ELEMENTPAD]) {		/* Only if a joypad is NOT present */
			
				if (JoyPtr->Elements[ELEMENTAXIS]) {	
					Cache = JoyPtr->ElementData[ELEMENTAXIS]>>8;		
					if (Cache<Boundary[AXISLESS]) {		/* Test X axis */
						Result |= PadLeft;
					}
					if (Cache>=Boundary[AXISMORE]) {
						Result |= PadRight;
					}
				}
			
				if (JoyPtr->Elements[ELEMENTAXIS+1]) {		
					Cache = (JoyPtr->ElementData[ELEMENTAXIS+1]>>8)^0xFFFFFF;			/* Test Y axis */
					if (Cache<Boundary[AXISLESS+(AXISENTRIES*1)]) {
						Result |= PadUp;
					}
					if (Cache>=Boundary[AXISMORE+(AXISENTRIES*1)]) {
						Result |= PadDown;
					}
				}
			}
				
			if (!JoyPtr->Elements[ELEMENTPAD+1]) {		/* Only if a joypad is NOT present */
				if (JoyPtr->Elements[ELEMENTAXIS+2]) {		
					Cache = JoyPtr->ElementData[ELEMENTAXIS+2]>>8;			/* Test throttle axis */
					if (Cache<Boundary[AXISLESS+(AXISENTRIES*2)]) {
						Result |= PadThrottleUp;
					}
					if (Cache>=Boundary[AXISMORE+(AXISENTRIES*2)]) {
						Result |= PadThrottleDown;
					}
				}
				if (JoyPtr->Elements[ELEMENTAXIS+3]) {
					Cache = JoyPtr->ElementData[ELEMENTAXIS+3]>>8;			/* Test rudder axis */
					if (Cache<Boundary[AXISLESS+(AXISENTRIES*3)]) {
						Result |= PadTwistLeft;
					}
					if (Cache>=Boundary[AXISMORE+(AXISENTRIES*3)]) {
						Result |= PadTwistRight;
					}
				}
			}
		}
		
		/* Find the button down events */
		JoystickLastButtonsDown[Which] |= (Result^JoystickLastButtonsDown[Which])&Result;
		JoystickLastButtons[Which] = Result;
	}
	return Result;
}

/**********************************

	Read raw input from the joystick

**********************************/

void BURGERCALL JoystickReadNow(Word Which)
{
	JoyDesc_t *JoyPtr;
	Word i;
	if (Which<JoystickPresent && !MacInputLockInputSprocket()) {		/* Valid joystick? */
		MacInput_t *LocalPtr;
		LocalPtr = &MacInputLocals;
		LocalPtr->JoystickLastRead = Which;
		JoyPtr = LocalPtr->JoystickDescriptionsArray+Which;
		i = 0;
		do {
			if (JoyPtr->Elements[i]) {		/* Is there an element here? */
				if (ISpElement_GetSimpleState(JoyPtr->Elements[i],&JoyPtr->ElementData[i])) {	/* Read it in */
					if (i<ELEMENTAXIS || i>=(ELEMENTAXIS+6)) {
						JoyPtr->ElementData[i] = 0;		/* Clear it on error */
					} else {
						JoyPtr->ElementData[i] = 0x7FFFFFFF;
					}
				}
			}
		} while (++i<ELEMENTCOUNT);
		MacInputUnlockInputSprocket();
	}
}

/**********************************

	Read analog joystick info

**********************************/

Word BURGERCALL JoystickReadAbs(Word Axis,Word Which)
{
	if (Which<JoystickPresent && Axis<AXISCOUNT) {
		JoyDesc_t *JoyPtr;
		Word32 Result;
		Word *Boundary;
		Word Range;
		MacInput_t *LocalPtr;
		LocalPtr = &MacInputLocals;
		
		if (LocalPtr->JoystickLastRead!=Which) {	/* Different device? */
			JoystickReadNow(Which);		/* Read it in */
		}
		JoyPtr = LocalPtr->JoystickDescriptionsArray+Which;
		Range = Axis+ELEMENTAXIS;
		if (JoyPtr->Elements[Range]) {		/* Valid device? */
			Result = JoyPtr->ElementData[Range]>>8;		/* Use the upper 24 bits */
			if ((Axis==1) || (Axis==5)) {
				Result ^= 0xFFFFFF;			/* Reverse the Y axis (Mac oddity) */
			}
			Boundary = &JoystickBoundaries[Which][Axis*AXISENTRIES];	/* Force in the bounds */
			Range = Boundary[AXISMAX]-Boundary[AXISMIN];
			if (!Range) {		/* Prevent divide by zero */
				Range = 1;
			}
			if (Result<Boundary[AXISMIN]) {		/* Below minimum? */
				if (Result<=0x100000) {			/* Dead space on edges */
					Result = 0x100000;
				}
				Boundary[AXISMIN] = Result;
				Boundary[AXISCENTER] = (Result+Boundary[AXISMAX])/2;
				JoystickSetDigital(Axis,JoystickPercent[Which][Axis],Which);
				return 0;
			}
			if (Result>Boundary[AXISMAX]) {
				if (Result>=0xF00000) {
					Result = 0xF00000;			/* Dead space on edges */
				}
				Boundary[AXISMAX] = Result;
				Boundary[AXISCENTER] = (Boundary[AXISMIN]+Result)/2;
				JoystickSetDigital(Axis,JoystickPercent[Which][Axis],Which);
				return 255;
			}
			Result = ((Result-Boundary[AXISMIN])*255)/Range;		/* Convert the value to a range of 0-255 */
			return (Word)Result;				/* Return 0-255 */
		}
	}
	return 128;			/* Bad input */
}

#else

Word BURGERCALL MacInputInit(MacInput_t *Input,Word Flags)
{
	return TRUE;				/* I'm not ok */
}

void BURGERCALL MacInputDestroy(MacInput_t *Input,Word Flags)
{
	Flags = Input->Flags & (~Flags);
	Input->Flags = Flags;
}

Word BURGERCALL InputSetState(Word NewState)
{
	return FALSE;
}

Word BURGERCALL InputGetState(void)
{
	return FALSE;
}

/**********************************

	Obtain the Mutex lock to call InputSprocket

**********************************/

Word BURGERCALL MacInputLockInputSprocket(void)
{
	return MutexLock(&MacInputLocals.InputSprocketMutex);
}

/**********************************

	Release the Mutex lock to call InputSprocket

**********************************/

void BURGERCALL MacInputUnlockInputSprocket(void)
{
	MutexUnlock(&MacInputLocals.InputSprocketMutex);
}

#endif
#endif