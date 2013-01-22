#define DIRECTINPUT_VERSION 0x700
#include "InInput.h"

/**********************************

	Win95 version of the Joystick Manager

**********************************/

#if defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>			/* I need some windows stuff */
#include <mmsystem.h>			/* Joystick reading is performed under multimedia functions */

static JOYINFOEX PrivJoyInfo;	/* Win95 local data struct */
static Word PrivJoyNumber;		/* Joystick number of last read in cache */

/**********************************

	Read the absolute value of an axis

**********************************/

Word BURGERCALL JoystickReadAbs(Word Axis,Word Which)
{
	Word32 Value;
	Word32 Range;
	Word *Boundary;

	if (Which<JoystickPresent) {			/* Valid joystick number? */
		if (PrivJoyNumber!=Which) {			/* Different joystick to read? */
			JoystickReadNow(Which);
		}
		switch (Axis) {						/* Get the requested axis */
		case 1:
			Value = PrivJoyInfo.dwYpos;
			break;
		case 2:
			Value = PrivJoyInfo.dwZpos;
			break;
		case 3:
			Value = PrivJoyInfo.dwRpos;
			break;
		case 4:
			Value = PrivJoyInfo.dwUpos;
			break;
		case 5:
			Value = PrivJoyInfo.dwVpos;
			break;
		default:
			Value = PrivJoyInfo.dwXpos;
		}
		Boundary = &JoystickBoundaries[Which][Axis*AXISENTRIES];	/* Force in the bounds */
		Range = Boundary[AXISMAX]-Boundary[AXISMIN];
		if (!Range) {		/* Prevent divide by zero */
			Range = 1;
		}
		Value = (Value*255)/Range;		/* Convert the value to a range of 0-255 */
		return (Word)Value;				/* Return 0-255 */
	}
	return 128;						/* Bogus value */
}

/**********************************

	Read the joystick info now.
	This forces the data to be current.

**********************************/

void BURGERCALL JoystickReadNow(Word Which)
{
	if (Which<JoystickPresent) {			/* Is the joystick available? */
		PrivJoyInfo.dwSize = sizeof(PrivJoyInfo);
		PrivJoyInfo.dwFlags = (JOY_RETURNX|JOY_RETURNY|JOY_RETURNZ|JOY_RETURNR|JOY_RETURNU|JOY_RETURNV|
			JOY_RETURNPOVCTS|JOY_RETURNBUTTONS|JOY_RETURNCENTERED);	/* I need this info */
		joyGetPosEx(JOYSTICKID1+Which,&PrivJoyInfo);	/* Read it in */
		PrivJoyNumber = Which;		/* Mark the joystick # read */
	}
}

/**********************************

	Read the joystick buttons and
	convert the analog input into digital
	directions

**********************************/

#define POV_SLOP 1500 // 15 degrees

Word32 BURGERCALL JoystickReadButtons(Word Which)
{
	Word32 Cache;
	Word *Boundary;
	Word32 Result;

	if (Which>=JoystickPresent) {		/* Joystick now ready? */
		return 0;				/* No buttons for you! */
	}
	JoystickReadNow(Which);		/* Force the joystick to be read */
	Result = 0;		/* Init the return value */
	Cache = PrivJoyInfo.dwButtons;		/* Translate the buttons to Burgerlib */
	if (Cache&JOY_BUTTON1) {		/* Button A-D */
		Result = PadButton1;
	}
	if (Cache&JOY_BUTTON2) {
		Result += PadButton2;
	}
	if (Cache&JOY_BUTTON3) {
		Result += PadButton3;
	}
	if (Cache&JOY_BUTTON4) {
		Result += PadButton4;
	}

	if (Cache&JOY_BUTTON5) {		/* Extra buttons (Shift buttons) */
		Result += PadButton5;
	}
	if (Cache&JOY_BUTTON6) {
		Result += PadButton6;
	}
	if (Cache&JOY_BUTTON7) {
		Result += PadButton7;
	}
	if (Cache&JOY_BUTTON8) {
		Result += PadButton8;
	}
	if (Cache&JOY_BUTTON9) {
		Result += PadButton9;
	}
	if (Cache&JOY_BUTTON10) {
		Result += PadButton10;
	}
	if (Cache&JOY_BUTTON11) {
		Result += PadButton11;
	}
	if (Cache&JOY_BUTTON12) {
		Result += PadButton12;
	}
	if (Cache&JOY_BUTTON13) {
		Result += PadButton13;
	}
	if (Cache&JOY_BUTTON14) {
		Result += PadButton14;
	}
	if (Cache&JOY_BUTTON15) {
		Result += PadButton15;
	}
	if (Cache&JOY_BUTTON16) {
		Result += PadButton16;
	}
	if (Cache&JOY_BUTTON17) {
		Result += PadButton17;
	}
	if (Cache&JOY_BUTTON18) {
		Result += PadButton18;
	}
	if (Cache&JOY_BUTTON19) {
		Result += PadButton19;
	}
	if (Cache&JOY_BUTTON20) {
		Result += PadButton20;
	}

	/*	Window returns the position of the hi-hat (POV) as
		-1 (JOY_POVCENTERED) or a value between 0 (0 degrees)
		and 36000 (360 degrees). 0 is forward, 9000 is right, 18000
		is backward, and 27000 is left. Rather than saying the hat
		is pressed to the right only when the Windows POV value
		is exactly 9000 (90 degreese), we will say anything between
		15 degrees (forward plus some slop) and 165 degrees (backward
		minus some slop) means the hat is to the right.
	*/

	Cache = PrivJoyInfo.dwPOV;		/* Get the hi-hat value */
	if (Cache != JOY_POVCENTERED) { /* Convert to digital information */
		if ((Cache > JOY_POVLEFT + POV_SLOP && Cache <= 36000) || (Cache < JOY_POVRIGHT - POV_SLOP)) { /* we will consider the hat to be up if it is anywhere forward of left or right */
			Result += PadHatUp;
		}
		if ((Cache > JOY_POVFORWARD + POV_SLOP) && (Cache < JOY_POVBACKWARD - POV_SLOP)) { /* we will consider the hat to be to the ritgh up if it is anywhere to the right opf up or down */
			Result += PadHatRight;
		}
		if ((Cache > JOY_POVRIGHT + POV_SLOP) && (Cache < JOY_POVLEFT - POV_SLOP)) {
			Result += PadHatDown;
		}
		if ((Cache > JOY_POVBACKWARD + POV_SLOP) && (Cache < 36000 - POV_SLOP)) {
			Result += PadHatLeft;
		}
	}

	Cache = PrivJoyInfo.dwXpos;		/* Convert analog directions to digital info */
	Boundary = &JoystickBoundaries[Which][0];
	if (Cache<Boundary[AXISLESS]) {		/* Test X axis */
		Result += PadLeft;
	}
	if (Cache>=Boundary[AXISMORE]) {
		Result += PadRight;
	}

	Cache = PrivJoyInfo.dwYpos;			/* Test Y axis */
	if (Cache<Boundary[AXISLESS+(AXISENTRIES*1)]) {
		Result += PadUp;
	}
	if (Cache>=Boundary[AXISMORE+(AXISENTRIES*1)]) {
		Result += PadDown;
	}

	Cache = PrivJoyInfo.dwZpos;			/* Test throttle axis */
	if (Cache<Boundary[AXISLESS+(AXISENTRIES*2)]) {
		Result += PadThrottleUp;
	}
	if (Cache>=Boundary[AXISMORE+(AXISENTRIES*2)]) {
		Result += PadThrottleDown;
	}

	Cache = PrivJoyInfo.dwRpos;			/* Test rudder axis */
	if (Cache<Boundary[AXISLESS+(AXISENTRIES*3)]) {
		Result += PadTwistLeft;
	}
	if (Cache>=Boundary[AXISMORE+(AXISENTRIES*3)]) {
		Result += PadTwistRight;
	}
	Cache = (JoystickLastButtons[Which] ^ Result)&Result;
	JoystickLastButtonsDown[Which] |= Cache;
	JoystickLastButtons[Which] = Result;
	return Result;		/* Return digital information */
}

/**********************************

	Detect joysticks and init default information

**********************************/

Word BURGERCALL JoystickInit(void)
{
	JOYCAPS joycaps;	/* Joystick capabilities */
	JOYINFO ji;		/* Initial joystick info */
	Word i;
	Word Count;		/* Maximum number of joysticks to check */
	Word Center;	/* Cache value */
	Word j;
	Word *Boundaries;	/* Pointer to calibration table */

	Count = joyGetNumDevs();	/* Any joystick device drivers present? */
	if (!Count) {				/* No? */
		JoystickPresent = 0;	/* Exit now */
		return 0;
	}
	if (Count>MAXJOYNUM) {
		Count = MAXJOYNUM;		/* I can only support 4 joysticks */
	}

	i = 0;
	Boundaries = &JoystickBoundaries[0][0];		/* Init pointer to struct */
	do {
		if (joyGetDevCaps(JOYSTICKID1+i,&joycaps,sizeof(JOYCAPS)) != JOYERR_NOERROR) {
			break;		/* Joystick not found! */
		}
		if (joyGetPos(JOYSTICKID1+i,&ji) != JOYERR_NOERROR) {	/* Is the joystick present? */
			break;
		}
		Boundaries[AXISMIN] = joycaps.wXmin;	/* Init center point */
		Boundaries[AXISMAX] = joycaps.wXmax;
		Center = (joycaps.wXmax-joycaps.wXmin)/2;
		Boundaries[AXISCENTER] = Center;
		Boundaries+=AXISENTRIES;

		Boundaries[AXISMIN] = joycaps.wYmin;
		Boundaries[AXISMAX] = joycaps.wYmax;
		Center = (joycaps.wYmax-joycaps.wYmin)/2;
		Boundaries[AXISCENTER] = Center;
		Boundaries+=AXISENTRIES;

		Boundaries[AXISMIN] = joycaps.wZmin;
		Boundaries[AXISMAX] = joycaps.wZmax;
		Center = (joycaps.wZmax-joycaps.wZmin)/2;
		Boundaries[AXISCENTER] = Center;
		Boundaries+=AXISENTRIES;

		Boundaries[AXISMIN] = joycaps.wRmin;
		Boundaries[AXISMAX] = joycaps.wRmax;
		Center = (joycaps.wRmax-joycaps.wRmin)/2;
		Boundaries[AXISCENTER] = Center;
		Boundaries+=AXISENTRIES;

		Boundaries[AXISMIN] = joycaps.wUmin;
		Boundaries[AXISMAX] = joycaps.wUmax;
		Center = (joycaps.wUmax-joycaps.wUmin)/2;
		Boundaries[AXISCENTER] = Center;
		Boundaries+=AXISENTRIES;

		Boundaries[AXISMIN] = joycaps.wVmin;
		Boundaries[AXISMAX] = joycaps.wVmax;
		Center = (joycaps.wVmax-joycaps.wVmin)/2;
		Boundaries[AXISCENTER] = Center;
		Boundaries+=AXISENTRIES;

		j = 0;
		do {
			JoystickSetDigital(j,20,i);		/* Create the digital bounds */
		} while (++j<AXISCOUNT);

	} while (++i<Count);		/* All of them checked? */
	JoystickPresent = i;		/* Save joysticks found */
	return i;
}

/**********************************

	Shut down the joystick manager

**********************************/

void BURGERCALL JoystickDestroy(void)
{
	JoystickPresent = 0;
}

#endif