#include "TkTick.h"

#if defined(__WIN32__)
#include "InInput.h"
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "W9Win95.h"
#include <mmsystem.h>

/**********************************

	I create a thread that gets CPU time about 60 times
	a second, but since I can't rely on this I then
	use GetTickCount() to get the TRUE elapsed time

**********************************/

Word TickWakeUpFlag;				/* TRUE if I should make wake up messages */
static volatile Word32 TimeCount;	/* Inc every 1/60th of a second */
static DWORD TimerID;				/* Thread ID */
static HANDLE ThreadHandle;			/* Thread handle */
static Bool Started;				/* True if started */

/*********************************

	This thread handles the timer

**********************************/

#define TICKINMILLS 0x10AA		/* 16.6667 Milliseconds in 24.8 fixed */

static DWORD WINAPI MyIrq(LPVOID /* Foo */)
{
	Word32 Mark;		/* Tick mark in 24.8 fixed point */
	Word32 Delay;		/* Delay in 24.8 fixed point */

	Mark = GetTickCount()<<8;		/* Init the timer */
	for (;;) {
		SleepEx(16,FALSE);			/* Sleep for 1/60th of a second */
		Delay = (GetTickCount()<<8)-Mark;	/* Fixed32 point elapsed time */
		if (Delay>=TICKINMILLS) {		/* Time up? */
			do {
				++TimeCount;			/* Inc tick value */
				Mark+=TICKINMILLS;		/* Adjust timer */
				Delay-=TICKINMILLS;
			} while (Delay>=TICKINMILLS);
			if (TickWakeUpFlag && Win95MainWindow && GetForegroundWindow()==Win95MainWindow) {
				PostMessage((HWND)Win95MainWindow,WM_USER,0,TimeCount);
			}
		}
	}
#if !defined(__WATCOMC__)
	return 0;		/* Will never exit */
#endif
}

/*********************************

	Shut down the timer thread

**********************************/

static void ANSICALL MyShutdown(void)
{
	CloseHandle(ThreadHandle);	/* Kill the thread */
	ThreadHandle = 0;			/* Release */
}

/**********************************

	Read the current system tick value

**********************************/

Word32 BURGERCALL ReadTick(void)
{
	if (Started) {		/* Already started? */
		return TimeCount;	/* Read from the Interrupt system */
	}
	atexit(MyShutdown);		/* Dispose on exit */
	ThreadHandle = CreateThread(0,1024,MyIrq,0,0,&TimerID);
	SetThreadPriority(ThreadHandle,THREAD_PRIORITY_HIGHEST);
	Started = TRUE;			/* I'm started */
	return TimeCount;		/* Return tick count */
}

/**********************************

	Wait for a number of system ticks

**********************************/

void BURGERCALL WaitTicks(Word TickCount)
{
	Word32 NewTick;		/* Temp */

	KeyboardKbhit();				/* Handle any pending events */
	NewTick = ReadTick();	/* Read the timer */
	if ((NewTick-LastTick)<(Word32)TickCount) {	/* Should I wait? */
		do {
			Word Old;
			Old = TickWakeUpFlag;
			TickWakeUpFlag = TRUE;	/* Allow wake up events */
			WaitMessage();		/* Sleep until a tick occurs */
			TickWakeUpFlag = Old;
			KeyboardKbhit();			/* Call the system task if needed */
			NewTick = ReadTick();	/* Read in the current time tick */
		} while ((NewTick-LastTick)<(Word32)TickCount);	/* Time has elapsed? */
	}
	LastTick = NewTick;		/* Mark the time */
}

/**********************************

	Retrieve the current system time
	
**********************************/

void BURGERCALL TimeDateGetCurrentTime(TimeDate_t *Input)
{
	SYSTEMTIME MyDate;
	GetLocalTime(&MyDate);		/* Call windows 95 */
	Input->Year = MyDate.wYear;
	Input->Milliseconds = MyDate.wMilliseconds;
	Input->Month = (Word8)MyDate.wMonth;
	Input->Day = (Word8)MyDate.wDay;
	Input->DayOfWeek = (Word8)MyDate.wDayOfWeek;
	Input->Hour = (Word8)MyDate.wHour;
	Input->Minute = (Word8)MyDate.wMinute;
	Input->Second = (Word8)MyDate.wSecond;
}	

/**********************************

	Read the time in microsecond increments

**********************************/

static Word GotMilk;	/* Got the frequency */
static double WinTicks = 1.0;	/* Frequency adjust */

Word32 BURGERCALL ReadTickMicroseconds(void)
{
	LARGE_INTEGER Temp;
	if (!GotMilk) {		/* Is the divisor initialized? */
		GotMilk = TRUE;
		if (QueryPerformanceFrequency(&Temp)) {	/* Get the constants */
			WinTicks = 1000000.0/(double)Temp.QuadPart;	/* Timer change */
		}
	}
	if (QueryPerformanceCounter(&Temp)) {	/* Get the timer from Win95 */
		return (Word32)((double)Temp.QuadPart*WinTicks);	/* Save the result */
	}
	return 0;		/* Just zap it! (Error) */
}

/**********************************

	Read the time in millisecond increments

**********************************/

Word32 BURGERCALL ReadTickMilliseconds(void)
{
	return timeGetTime();		/* Call windows 95/NT */
}

#endif