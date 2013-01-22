/**********************************

	Burgerlib Input Manager
	Joystick specific code

	Copyright Bill Heineman
	All rights reserved.
	Written by Bill Heineman

**********************************/

#include "InInput.h"
#include "TkTick.h"

Word JoystickPercent[MAXJOYNUM][AXISCOUNT];	/* Cache for percentages */

/**********************************

	Header:
		Number of joysticks present

	Synopsis:
		After JoystickInit() is called, this is set to the
		number of joysticks detected. If either the joystick services
		are not present or there are no joysticks installed, this value
		is set to zero. If this is zero, all joystick input routines will
		return default values and not attempt to actually talk
		to the joystick hardware.

	Also:
		JoystickInit()

**********************************/

Word JoystickPresent = FALSE;

/**********************************

	Header:
		Last read joystick/joypad state

	Synopsis:
		Whenever JoystickReadButtons() is called, it's value is also
		stored here for future use and to detect joystick button changes.

		Note: It isn't really recommended to use this table for detecting
		joystick down events since this is only updated on calls to
		JoystickReadButtons(). Use JoystickLastButtonsDown instead.

	Also:
		JoystickReadButtons()

**********************************/

Word32 JoystickLastButtons[MAXJOYNUM];

/**********************************

	Header:
		Last read joystick/joypad down state

	Synopsis:
		Whenever JoystickReadButtons() is called, a check is made for buttons
		that had been released and then pressed. This can be done via a background
		task so that all joystick down events can be captured even though they may
		not be sampled via a simple polling routine.

		If you act on an event, you must clear the event since JoystickReadButtons() only
		sets these flags but doesn't clear them.

	Also:
		JoystickReadButtons()

**********************************/

Word32 JoystickLastButtonsDown[MAXJOYNUM];

/**********************************

	Header:
		Boundaries for the analog joystick

	Synopsis:
		Analog joysticks are converted to a digital form but checking the
		analog data versus the boundaries found in this table. This allows the user
		to hint the sensitivity of the game to their liking.

	Platform:
		In GS/OS and MS-DOS, where the analog joystick driver is built into Burgerlib.
		This table contains data for joystick calibration. You may have to call JoystickSetCenter(),
		JoystickSetMin() and JoystickSetMax() to set the calibration information. All other
		platforms get the calibration information from the operating system or generic
		user settings.

	Also:
		JoystickReadButtons()

**********************************/

Word JoystickBoundaries[MAXJOYNUM][AXISENTRIES*AXISCOUNT];

/**********************************

	Header:
		Autorepeat a joystick

	Synopsis:
		Sometimes it would be good to simulate an autorepeat feature in a joystick fire
		button. For each event that can have this feature, allocate a JoyAutoRepeat_t and
		initialize it with zero. Then call this function with data read from
		JoystickReadButtons() masked with just the bits for the event. If TRUE is
		returned, then process the joystick button press. This is useful for
		rapid fire games.

		The structure needs to have these constants set. InitialTick has the number of
		ticks to wait before auto firing and RepeatTick has the firing rate.

	Input:
		Input = Pointer to a JoyAutoRepeat_t structure for state storage
		JoyBits = Bit mask for either a single or group of joystick bits to process

	Returns:
		Returns TRUE if the keydown should be processed by the application

	Also:
		JoystickReadButtons(), JoystickLastButton, JoystickLastButtonDown

**********************************/

#define LASTSTATE 1
#define SECONDDELAY 2
#define WAITFORKEYUP 4
#define INITIALIZED 8

Word BURGERCALL JoyAutoRepeater(JoyAutoRepeat_t *Input,Word32 JoyBits)
{
	Word Delay;
	Word32 NewMark;

	NewMark = ReadTick();		/* Get the current time mark */

	if (!(Input->HeldDown&INITIALIZED)) {	/* Initialized? */
		Input->TimeMark = NewMark;			/* Reset the timer */
		if (Input->JoyBits & JoyBits) {		/* Initially held down? */
			Input->HeldDown |= WAITFORKEYUP;	/* No response until key up */
		}
		Input->HeldDown |= INITIALIZED;		/* Don't do this again... */
	}

	if (Input->JoyBits & JoyBits) {	/* Is it held down? */
		if (Input->HeldDown & WAITFORKEYUP) {
			return FALSE;
		}
		if (!(Input->HeldDown & LASTSTATE)) {	/* Is this a key down? (First time) */
			Input->HeldDown |= LASTSTATE;	/* Mark as down */
			Input->TimeMark = NewMark;		/* Reset the timer */
			return TRUE;			/* Event... */
		}
		if (Input->HeldDown & SECONDDELAY) {		/* Second time through */
			Delay = Input->RepeatTick;		/* Repeater delay */
		} else {
			Delay = Input->InitialTick;		/* Initial delay */
		}
		if ((NewMark-Input->TimeMark) < (Word32)Delay) {	/* Not enough time yet? */
			return FALSE;			/* Return no event */
		}
		Input->TimeMark += Delay;		/* Make it an atomic delay */
		if ((NewMark-Input->TimeMark) >= (Word32)Delay) {	/* Hmm still active? */
			Input->TimeMark = NewMark;		/* Failsafe for wrap around of timer */
		}
		Input->HeldDown |= SECONDDELAY;	/* Use the second delay time from now on */
		return TRUE;			/* Event... */
	}
	Input->HeldDown &= ~(WAITFORKEYUP|LASTSTATE|SECONDDELAY);	/* Clear the held down bit */
	Input->TimeMark = NewMark;	/* Reset the time mark */
	return FALSE;
}

/**********************************

	Header:
		Set the boundaries for the joystick

	Synopsis:
		To simulate a digital joypad with a analog joystick, you need to
		define a bounds rect that has the dead area for digital motion.
		Burgerlib has an acceptable default but you can override it with this
		call.

	Input:
		Axis = Which analog axis to affect
		Percent = Percentage from center point for dead zone (20% is normal)
		Which = Which joystick device to affect

	Also:
		JoystickReadButtons()

**********************************/

void BURGERCALL JoystickSetDigital(Word Axis,Word Percent,Word Which)
{
	Word *BoundPtr;
	Word Distance;

	if (Which<MAXJOYNUM && Axis<AXISCOUNT) {
		JoystickPercent[Which][Axis] = Percent;			/* Save for later */
		BoundPtr = &JoystickBoundaries[Which][Axis*AXISENTRIES];	/* Get pointer to record */
		Distance = BoundPtr[AXISCENTER]-BoundPtr[AXISMIN];		/* Get distance from center */
		Distance = (Percent*Distance)/100;				/* Get the percentage */
		BoundPtr[AXISLESS] = BoundPtr[AXISCENTER]-Distance;	/* Save the lower value */
		Distance = BoundPtr[AXISMAX] - BoundPtr[AXISCENTER];	/* Get distance from center */
		Distance = (Percent*Distance)/100;
		BoundPtr[AXISMORE] = BoundPtr[AXISCENTER]+Distance;	/* Save the upper value */
	}
}

/**********************************

	Header:
		The joystick boundaries have changed

	Synopsis:
		Assuming the JoystickBoundaries table
		has been read from disk or any other source,
		recalibrate the joystick to these settings.
		This way, you can save the player's preferences to disk.

	Also:
		JoystickSetMin(), JoystickSetMax(), JoystickSetCenter()

**********************************/

void BURGERCALL JoystickBoundariesChanged(void)
{
}

/**********************************

	Header:
		Calibrate the joystick minimum

	Synopsis:
		For versions of Burgerlib that have built in analog joystick
		drivers, read the raw joystick analog data and use the reading
		as the minimum setting for that axis. You must have already shown some
		sort of message to the use to ask that the joystick is in the upper left
		corner before calling this routine.

	Input:
		Axis = Analog axis for the joystick
		Which = Which joystick device to read

	Platform:
		Only call for MS-DOS or GS/OS versions of Burgerlib. This routine
		does nothing on all other platforms.

	Also:
		JoystickSetMax(), JoystickSetCenter()

**********************************/

void BURGERCALL JoystickSetMin(Word /* Axis */,Word /* Which */)
{
}

/**********************************

	Header:
		Calibrate the joystick maximum

	Synopsis:
		For versions of Burgerlib that have built in analog joystick
		drivers, read the raw joystick analog data and use the reading
		as the maximum setting for that axis. You must have already shown some
		sort of message to the use to ask that the joystick is in the lower right
		corner before calling this routine.

	Input:
		Axis = Analog axis for the joystick
		Which = Which joystick device to read

	Platform:
		Only call for MS-DOS or GS/OS versions of Burgerlib. This routine
		does nothing on all other platforms.

	Also:
		JoystickSetMin(), JoystickSetCenter()

**********************************/

void BURGERCALL JoystickSetMax(Word /* Axis */,Word /* Which */)
{
}

/**********************************

	Header:
		Calibrate the joystick center

	Synopsis:
		For versions of Burgerlib that have built in analog joystick
		drivers, read the raw joystick analog data and use the reading
		as the center setting for that axis. You must have already shown some
		sort of message to the use to ask that the joystick is in the center
		before calling this routine.

	Input:
		Axis = Analog axis for the joystick
		Which = Which joystick device to read

	Platform:
		Only call for MS-DOS or GS/OS versions of Burgerlib. This routine
		does nothing on all other platforms.

	Also:
		JoystickSetMin(), JoystickSetMax()

**********************************/

void BURGERCALL JoystickSetCenter(Word /* Axis */,Word /* Which */)
{
}

/**********************************

	Header:
		Read an analog joystick axis as delta

	Synopsis:
		Get information from the device and axis of a joystick and return
		the value of the axis at this moment in time. The value returned is -128
		to 127 with -128 being left/up and 127 being right/down.

		Since reading a joystick may be slow, the function JoystickReadNow() will
		actually perform the read and keep all the data in a cache so that the
		subsequent calls will have very little overhead. If you must be certain that
		data is freshly read, call JoystickReadNow() before this call to make sure the
		data is fresh.

	Input:
		Axis = Analog axis for the joystick
		Which = Which joystick device to read

	Also:
		JoystickReadAbs(), JoystickReadButtons(), JoystickReadNow()

**********************************/

int BURGERCALL JoystickReadDelta(Word Axis,Word Which)
{
	return JoystickReadAbs(Axis,Which)-128;		/* Convert absolute value to signed offset */
}

/**********************************

	Here are the stubs for the unsupported versions

**********************************/

#if defined(__MAC__)
#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>		/* I need TARGET_RT_MAC_CFM */
#endif
#endif

#if !defined(__MSDOS__) && !defined(__WIN32__) && !(defined(__MAC__) && TARGET_RT_MAC_CFM)

/**********************************

	Header:
		Read an analog joystick axis

	Synopsis:
		Get information from the device and axis of a joystick and return
		the value of the axis at this moment in time. The value returned is 0
		to 255 with 0 being left/up and 255 being right/down.

		Since reading a joystick may be slow, the function JoystickReadNow() will
		actually perform the read and keep all the data in a cache so that the
		subsequent calls will have very little overhead. If you must be certain that
		data is freshly read, call JoystickReadNow() before this call to make sure the
		data is fresh.

	Input:
		Axis = Analog axis for the joystick
		Which = Which joystick device to read

	Platform:
		Win95/98/NT uses the multimedia manager to read the joystick. MacOS uses InputSprocket
		and MS-DOS and GS/OS uses direct joystick drivers. MS-DOS and GS/OS must be calibrated
		for accurate results.

	Also:
		JoystickReadDelta(), JoystickReadButtons(), JoystickReadNow()

**********************************/

Word BURGERCALL JoystickReadAbs(Word Axis,Word Which)
{
	return 128;		/* Read the default */
}

/**********************************

	Header:
		Read an analog joystick

	Synopsis:
		Get the analog joystick information for a joystick device. This is an internal
		routine that will ask the operating system for the joystick info. This could be slow.
		The data will then be stored into an internal cache that can be read from JoystickReadAbs()
		or JoystickReadDelta().

	Input:
		Which = Which joystick device to read

	Platform:
		Win95/98/NT uses the multimedia manager to read the joystick. MacOS uses InputSprocket
		and MS-DOS and GS/OS uses direct joystick drivers. MS-DOS and GS/OS must be calibrated
		for accurate results.

	Also:
		JoystickReadDelta(), JoystickReadButtons(), JoystickReadAbs()

**********************************/

void BURGERCALL JoystickReadNow(Word Which)
{
}

/**********************************

	Header:
		Read an analog joystick

	Synopsis:
		Get the analog joystick information for a joystick device. This is an internal
		routine that will ask the operating system for the joystick info. This could be slow.
		The data will then be stored into an internal cache that can be read from JoystickReadAbs()
		or JoystickReadDelta().

	Input:
		Which = Which joystick device to read

	Platform:
		Win95/98/NT uses the multimedia manager to read the joystick. MacOS uses InputSprocket
		and MS-DOS and GS/OS uses direct joystick drivers. MS-DOS and GS/OS must be calibrated
		for accurate results.

	Also:
		JoystickReadDelta(), JoystickReadButtons(), JoystickReadAbs()

**********************************/

Word32 BURGERCALL JoystickReadButtons(Word Which)
{
	Word Buttons;
	Buttons = 0;			/* Read the buttons */
	if (Which<4) {
		JoystickLastButtons[Which] = Buttons;	/* Return the button */
	}
	return Buttons;			/* Return the button state */
}
/**********************************

	Header:
		Init joystick services

	Synopsis:
		Init the joystick services and detect if a joystick(s) is connected. No joystick manager
		call will operate properly unless this call is issued. You can call JoystickDestroy() to
		shut down the operation at any time.

		If you call this function again, the joysticks will be rescanned and your configuration
		may change. This is due to the fact that users could plug in a joystick at
		any time.

	Returns:
		The number of joysticks found or 0 if an error occurs.

	Platform:
		Win95/98/NT uses the multimedia manager to read the joystick. MacOS uses InputSprocket
		and MS-DOS and GS/OS uses direct joystick drivers. MS-DOS and GS/OS must be calibrated
		for accurate results.

	Also:
		JoystickDestroy()

**********************************/

Word BURGERCALL JoystickInit(void)
{
	JoystickPresent = 0;
	return FALSE;
}

/**********************************

	Header:
		Shut down joystick services

	Synopsis:
		Stop joystick scanning and release all resources attached to reading the joystick.
		Useful for manual shutdown and restart of joystick services on Win95 and MacOS

	Platform:
		Win95/98/NT uses the multimedia manager to read the joystick. MacOS uses InputSprocket
		and MS-DOS and GS/OS uses direct joystick drivers. MS-DOS and GS/OS must be calibrated
		for accurate results.

	Also:
		JoystickInit()

**********************************/

void BURGERCALL JoystickDestroy(void)
{
	JoystickPresent = 0;
}

#endif
