#include "TkTick.h"
#include "InInput.h"
#include "ShStream.h"
#include "ClStdLib.h"
#include "PfPrefs.h"
#include <time.h>

static const char *WeekDays[7] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
static const char *Months[12] = {"January","February","March","April","May","June","July","August","September","October","November","December"};

/**********************************

	Time mark for ticker

**********************************/

Word32 LastTick;	/* Last tick hit for time quantum */

/**********************************

	Wait for a system event to occur...

**********************************/

Word BURGERCALL WaitTicksEvent(Word Delay)
{
	Word Temp;
	Word MouseBits;			/* Mouse bits on entry */
	Word JoyBits;			/* Joystick bits on entry */
	Word32 TimeMark;		/* I cache this since I want to make SURE */
							/* that the timing is as precise as can be */

	TimeMark = ReadTick();		/* Set the current time mark */
	LastTick = TimeMark;		/* Set the global time mark */

	MouseBits = MouseReadButtons();		/* Get the current state of the mouse */
	JoyBits = JoystickReadButtons(0);		/* Get the current state of the joypad */
	for (;;) {
		Temp = JoystickReadButtons(0);		/* Pressed a joypad button? */
		if (((Temp^JoyBits)&Temp) & (PadButton1|PadButton2|PadButton3|PadButton4|PadButton5|PadButton6)) {
			Temp = 1;				/* Joypad event */
			break;
		}
		JoyBits = Temp;			/* Save it */

		Temp = MouseReadButtons();		/* Read the mouse */
		if ((Temp^MouseBits)&Temp) {		/* Pressed a mouse button? */
			Temp = 1;
			break;
		}
		MouseBits = Temp;		/* Save it */

		Temp = KeyboardGet();		/* Try the keyboard */
		if (Temp) {
			break;		/* Return the key event */
		}
		WaitTicks(1);	/* Wait 1 tick (Possibly calling an OS taskswitch) */
		if (Delay) {	/* Can I timeout? */
			if ((LastTick-TimeMark)>=(Word32)Delay) {		/* Count down */
				Temp = 0;		/* Timeout exit */
				break;
			}
		}
	}
	return Temp;
}

/**********************************

	Wait for a number of system ticks

**********************************/

#if !defined(__WIN32__)
void BURGERCALL WaitTicks(Word TickCount)
{
	Word32 NewTick;		/* Temp */

	KeyboardKbhit();				/* Handle any pending events */
	NewTick = ReadTick();	/* Read the timer */
	if ((NewTick-LastTick)<(Word32)TickCount) {	/* Should I wait? */
		do {
			KeyboardKbhit();			/* Call the system task if needed */
			NewTick = ReadTick();	/* Read in the current time tick */
		} while ((NewTick-LastTick)<(Word32)TickCount);	/* Time has elapsed? */
	}
	LastTick = NewTick;		/* Mark the time */
}
#endif

/**********************************

	Retrieve the 60 hertz timer system time

**********************************/

#if !defined(__MSDOS__) && !defined(__WIN32__) && !defined(__MAC__) && !defined(__BEOS__)

Word32 BURGERCALL ReadTick(void)
{
#if CLOCKS_PER_SEC==TICKSPERSEC
	return clock();
#else
	return (clock()*TICKSPERSEC)/CLOCKS_PER_SEC;
#endif
}
#endif

/**********************************

	Read the current number of elapsed Microseconds

**********************************/

#if !defined(__WIN32__) && !defined(__MAC__)

Word32 BURGERCALL ReadTickMicroseconds(void)
{
#if CLOCKS_PER_SEC==1000000
	return clock();
#else
	return (clock()*1000000)/CLOCKS_PER_SEC;
#endif
}

#endif

/**********************************

	Read the current number of elapsed Milliseconds

**********************************/

#if !defined(__WIN32__) && !defined(__MAC__)

Word32 BURGERCALL ReadTickMilliseconds(void)
{
#if CLOCKS_PER_SEC==1000
	return clock();
#else
	return (clock()*1000)/CLOCKS_PER_SEC;
#endif
}

#endif

/**********************************

	Retrieve the current system time

**********************************/

#if !defined(__WIN32__) && !defined(__MAC__)
void BURGERCALL TimeDateGetCurrentTime(TimeDate_t *Input)
{
	time_t mytime;

	time(&mytime);			/* Get the unix time */
	TimeDateFromANSITime(Input,mytime);
}
#endif

/**********************************

	Convert a time_t to a TimeDate_t

**********************************/

Word BURGERCALL TimeDateFromANSITime(TimeDate_t *Output,Word32 Input)
{
	struct tm *TimePtr;
	TimePtr = localtime((time_t *)&Input);		/* Convert the time value to local time */
	if (TimePtr) {
		Output->Year = (Word16)(TimePtr->tm_year+1900);
		Output->Milliseconds = 0;		/* Not supported */
		Output->Month = (Word8)(TimePtr->tm_mon+1);
		Output->Day = (Word8)TimePtr->tm_mday;
		Output->DayOfWeek = (Word8)TimePtr->tm_wday;
		Output->Hour = (Word8)TimePtr->tm_hour;
		Output->Minute = (Word8)TimePtr->tm_min;
		Output->Second = (Word8)TimePtr->tm_sec;
		return FALSE;					/* Convert is ok */
	}
	FastMemSet(Output,0,sizeof(TimeDate_t));		/* Clear out the struct */
	return TRUE;
}

/**********************************

	Print the time

**********************************/

void BURGERCALL TimeDateTimeString(char *Output,const TimeDate_t *Input)
{
	Output = LongWordToAscii(Input->Hour,Output);
	Output[0] = ':';
	Output = LongWordToAscii2(Input->Minute,Output+1,ASCIILEADINGZEROS|2);
	Output[0] = ':';
	LongWordToAscii2(Input->Second,Output+1,ASCIILEADINGZEROS|2);
}

/**********************************

	Print the time in 12 hour AM/PM format

**********************************/

void BURGERCALL TimeDateTimeStringPM(char *Output,const TimeDate_t *Input)
{
	Word Hour;
	char PM;
	
	Hour = Input->Hour;
	if (Hour<12) {
		PM = 'A';
	} else {
		PM = 'P';
		Hour -= 12;
	}
	if (!Hour) {
		Hour = 12;
	}
	Output = LongWordToAscii(Hour,Output);
	Output[0] = ':';
	Output = LongWordToAscii2(Input->Minute,Output+1,ASCIILEADINGZEROS|2);
	Output[0] = ':';
	Output = LongWordToAscii2(Input->Second,Output+1,ASCIILEADINGZEROS|2);
	Output[0] = PM;
	Output[1] = 'M';
	Output[2] = 0;
}

/**********************************

	Print the date as 10/30/63

**********************************/

void BURGERCALL TimeDateDateString(char *Output,const TimeDate_t *Input)
{
	Output = LongWordToAscii(Input->Month,Output);
	Output[0] = '/';
	Output = LongWordToAscii2(Input->Day,Output+1,ASCIILEADINGZEROS|2);
	Output[0] = '/';
	Output = LongWordToAscii2(Input->Year%100,Output+1,ASCIILEADINGZEROS|2);
}

/**********************************

	Print the date as Sunday, October 30, 1963

**********************************/

void BURGERCALL TimeDateDateStringVerbose(char *Output,const TimeDate_t *Input)
{
	Word Failsafe;
	Failsafe = Input->DayOfWeek;
	if (Failsafe>=7) {
		Failsafe = 0;
	}
	strcpy(Output,WeekDays[Failsafe]);
	strcat(Output,", ");
	Failsafe = Input->Month-1;
	if (Failsafe>=12) {
		Failsafe = 0;
	}
	strcat(Output,Months[Failsafe]);
	Output = Output+strlen(Output);
	Output[0] = ' ';
	Output = LongWordToAscii(Input->Day,Output+1);
	Output[0] = ',';
	Output[1] = ' ';
	Output = LongWordToAscii(Input->Year,Output+2);
}

/**********************************

	Header:
		Restore a TimeDate_t record from a StreamHandle_t

	Synopsis:
		Initialize a TimeDate_t structure from a
		StreamHandle_t record. The data will be read from what was store from a call
		to TimeDateStreamHandleWrite(). It will read the data back irregardless of platform.

		This call is endian neutral.

	Input:
		Output = Pointer to an uninitialized TimeDate_t structure
		Input = Pointer to a valid StreamHandle_t structure initialized for input

	Returns:
		Nothing

	Also:
		TimeDateStreamHandleWrite(), StreamHandleInitGet(), TimeDate_t, StreamHandle_t


**********************************/

void BURGERCALL TimeDateStreamHandleRead(TimeDate_t *Output,struct StreamHandle_t *Input)
{
	Output->Year = StreamHandleGetShort(Input);
	Output->Milliseconds = static_cast<Word16>(StreamHandleGetShort(Input));
	Output->Month = static_cast<Word8>(StreamHandleGetByte(Input));
	Output->Day = static_cast<Word8>(StreamHandleGetByte(Input));
	Output->DayOfWeek = static_cast<Word8>(StreamHandleGetByte(Input));
	Output->Hour = static_cast<Word8>(StreamHandleGetByte(Input));
	Output->Minute = static_cast<Word8>(StreamHandleGetByte(Input));
	Output->Second = static_cast<Word8>(StreamHandleGetByte(Input));
}

/**********************************

	Header:
		Save a TimeDate_t record to a StreamHandle_t

	Synopsis:
		Take the contents of a TimeDate_t structure and write it out to a
		StreamHandle_t record. The data will be written so a call to TimeDateStreamHandleRead()
		will read the data back irregardless of platform.

		This call is endian neutral.

	Input:
		Input = Pointer to a valid TimeDate_t structure
		Output = Pointer to a valid StreamHandle_t structure initialized for output

	Returns:
		Nothing

	Also:
		TimeDateStreamHandleRead(), StreamHandleInitPut(), TimeDate_t, StreamHandle_t

**********************************/

void BURGERCALL TimeDateStreamHandleWrite(const TimeDate_t *Input,struct StreamHandle_t *Output)
{
	StreamHandlePutShort(Output,Input->Year);
	StreamHandlePutShort(Output,Input->Milliseconds);
	StreamHandlePutByte(Output,Input->Month);
	StreamHandlePutByte(Output,Input->Day);
	StreamHandlePutByte(Output,Input->DayOfWeek);
	StreamHandlePutByte(Output,Input->Hour);
	StreamHandlePutByte(Output,Input->Minute);
	StreamHandlePutByte(Output,Input->Second);
}
