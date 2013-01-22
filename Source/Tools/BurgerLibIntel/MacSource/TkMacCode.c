#include "TkTick.h"

#if defined(__MAC__)
#include <DateTimeUtils.h>
#include <LowMem.h>
#include <Timer.h>
#include <Gestalt.h>
#include "Ll64Bit.h"
#include "ClStdLib.h"

#if TARGET_API_MAC_CARBON
#define LMGetTicks() TickCount()
#else
#define LMGetTicks() ((volatile Word32 *)0x16A)[0]
#endif

/**********************************

	Retrieve the current system time
	
**********************************/

void BURGERCALL TimeDateGetCurrentTime(TimeDate_t *Input)
{
	DateTimeRec MyDate;
	GetTime(&MyDate);
	Input->Year = MyDate.year;
	Input->Milliseconds = 0;
	Input->Month = MyDate.month;
	Input->Day = MyDate.day;
	Input->DayOfWeek = MyDate.dayOfWeek-1;	/* Start the day at zero */
	Input->Hour = MyDate.hour;
	Input->Minute = MyDate.minute;
	Input->Second = MyDate.second;
}

/**********************************

	Get the 60hz timer
	
**********************************/

Word32 ReadTick(void)
{
	return LMGetTicks();		/* Call the mac native function */
}


/**********************************

	Accurate timers for Burgerlib
	Original code by Matt Slot <fprefect@ambrosiasw.com>
	Optimized and included into Burgerlib by Bill Heineman <burger@contrabandent.com>
	
**********************************/

/**********************************

	Here is the "C" reference code. This works fine

**********************************/

#if 0	
Word32 BURGERCALL ReadTickMicroseconds(void)
{
	UnsignedWide wide;
	Microseconds(&wide);	/* Get the value from MacOS */
	return wide.lo;			/* Return just the low 32 bits */
}

Word32 BURGERCALL ReadTickMilliseconds(void)
{
	unsigned long long wide;
	Microseconds((UnsignedWide *)&wide);	/* Get the time in microseconds */
	return (Word32)((wide / 1000ULL));		
}
#endif

/**********************************

	For 68K macs, I just use Microseconds, it's accurate and fast
	I rewrote the routines in 68K asm for all the speed I can muster

**********************************/

#if !defined(__POWERPC__)

/**********************************

	Here is the above code but written in 68K assembly.
	Of course this means that I can only use Metrowerks 68K compilers
	to compile this. If anyone cares in the future (It is 1999 after all)
	then I'll support the alternate compilers.

**********************************/

extern void __rt_divu64(void);		// A Metrowerks internal function

asm Word32 BURGERCALL ReadTickMicroseconds(void)
{
	0xA193			// _Microseconds
	rts				// Get out NOW!
}


asm Word32 BURGERCALL ReadTickMilliseconds(void)
{	
	0xA193				// _Microseconds
	subq.w    #8,a7		// Space for result of divide
	pea	1000			// Div by 1000
	clr.l	-(a7)		// 64 bit
	move.l	d0,-(a7)	// Save the returned value
	move.l	a0,-(a7)
	pea		16(a7)		// Address for the result
	jsr		__rt_divu64	// 64 bit divide
	move.l	4(a0),d0	// Get the low 32 bits of the result
	lea      28(a7),a7	// Fix the stack
	rts					// Exit
}

#else

/**********************************

	On PowerPC machines, we try several methods:
		* DriverServicesLib is available on all PCI PowerMacs, and perhaps
			some NuBus PowerMacs. If it is, we use UpTime() : Overhead = 2.1 탎ec.
		* The PowerPC 601 has a built-in "real time clock" RTC, and we fall
			back to that, accessing it directly from asm. Overhead = 1.3 탎ec.
		* Later PowerPCs have an accurate "time base register" TBR, and we 
			fall back to that, access it from PowerPC asm. Overhead = 1.3 탎ec.
		* We can also try Microseconds() which is emulated : Overhead = 36 탎ec.

	On PowerPC machines, we avoid the following:
		* OpenTransport is available on all PCI and some NuBus PowerMacs, but it
			uses UpTime() if available and falls back to Microseconds() otherwise.
		* InputSprocket is available on many PowerMacs, but again it uses
			UpTime() if available and falls back to Microseconds() otherwise.

	Another PowerPC note: certain configurations, especially 3rd party upgrade
	cards, may return inaccurate timings for the CPU or memory bus -- causing
	skew in various system routines (up to 20% drift!). The VIA chip is very
	accurate, and it's the basis for the Time Manager and Microseconds().
	Unfortunately, it's also very slow because the MacOS has to (a) switch to
	68K and (b) poll for a VIA event.

	We compensate for the drift by calibrating a floating point scale factor
	between our fast method and the accurate timer at startup, then convert
	each sample quickly on the fly. I'd rather not have the initialization 
	overhead -- but it's simply necessary for accurate timing. You can drop
	it down to 30 ticks if you prefer, but that's as low as I'd recommend.

**********************************/

typedef AbsoluteTime (*UpTimeProcPtr)(void);

#define WideToDouble(w) ((double) (w).hi * (65536.0*65536.0) + (double) (w).lo)
#define RTCToNano(w)	((double) (w).hi * PowerPCBillion + (double) (w).lo)
#define WideTo64bit(w)	(*(unsigned long long *) &(w))

#ifdef __cplusplus
extern "C" {
#endif
extern void PollRTC601(LongWord64_t *Output);
extern void PollTBR603(LongWord64_t *Output);
extern void BurgerInitTimers(void);
#ifdef __cplusplus
}
#endif

Word PowerPCTimeMethod;			/* How shall I read the timer? 0-3 */
double PowerPCScale = 1000000.0;		/* Standard scale */
double PowerPCScale2 = 1000.0;			/* Microsecond accuracy */
UpTimeProcPtr PowerPCUpTime;			/* UpTime pointer */
Fixed32 PowerPCFScale,PowerPCFScale2;
double PowerPCBillion = 1000000000.0;
double PowerPCThousand = 1000.0;
double PowerPCFix = 65536.0*65536.0*65536.0*16.0;

/**********************************

	Determine what timing method to use. 
	UpTime, RTC, TBR or Microseconds()

**********************************/

void BurgerInitTimers(void) 
{
	Word Method;
	long result;
	
	Method = 4;				/* Assume Microseconds() */
	if (!Gestalt(gestaltNativeCPUtype, &result)) {
		if (result == gestaltCPU601) {
			Method = 3;		/* Use 601 method */
		} else if (result > gestaltCPU601) {
			Method = 2;		/* Use 603+ method */
		}
	}

	/* See if UpTime is present in DriverServicesLib */

	if (Method==4) {
		PowerPCUpTime = (UpTimeProcPtr)LibRefGetFunctionInLib("DriverServicesLib", "UpTime");

	/* If no DriverServicesLib, use Gestalt() to get the processor type. 
	   Only NuBus PowerMacs with old System Software won't have DSL, so
	   we know it should either be a 601 or 603. */

		if (PowerPCUpTime) {
			Method = 1;
		}
	}
	PowerPCTimeMethod = Method;		/* Save the method */
	
	/* Now calculate a scale factor to keep us accurate. */

	if (Method!=4) {
		Word32 Mark,Mark2;
		unsigned long long usec1, usec2;
		UnsignedWide wide;

		/* Wait for the beginning of the very next tick */

		Mark2 = LMGetTicks();		/* Get the anchor */
		do {
			Mark = LMGetTicks();	/* Check ... */
		} while (Mark2==Mark);		/* Time? */
					
		/* Poll the selected timer and prepare it (since we have time) */

		if (Method==1) {
			wide = PowerPCUpTime();
			usec1 = WideTo64bit(wide);
		} else if (Method==2) {
			PollTBR603((LongWord64_t *)&wide);
			usec1 = WideTo64bit(wide);
		} else {
			PollRTC601((LongWord64_t *)&wide);
			usec1 = RTCToNano(wide);
		}
			
		/* Wait for the exact 60th second to roll over */
		
		do {
			Mark2 = LMGetTicks();
		} while ((Mark2-Mark)<60);

		/* Poll the selected timer again and prepare it  */

		if (Method==1) {
			wide = PowerPCUpTime();
			usec2 = WideTo64bit(wide);
		} else if (Method==2) {
			PollTBR603((LongWord64_t *)&wide);
			usec2 = WideTo64bit(wide);
		} else {
			PollRTC601((LongWord64_t *)&wide);
			usec2 = RTCToNano(wide);
		}
		
		/* Calculate a scale value that will give microseconds per second.
			Remember, there are actually 60.15 ticks in a second, not 60.  */
		
		PowerPCScale = 60000000.0 / ((usec2 - usec1) * 60.15);
		PowerPCScale2 = PowerPCScale/1000.0;
		PowerPCFScale = PowerPCScale*65536.0*65536.0;
		PowerPCFScale2 = PowerPCScale2*65536.0*65536.0;
	}
}

/**********************************

	Read the tick value in microsecond accuracy
	
**********************************/


#if defined(__MRC__) /* assembly version of this depends on CW libraries */

#pragma options opt=off

Word32 ReadTickMicroseconds(void) 
{
	UnsignedWide wide;
	Word Temp;
	double Foo;
	
	/* Initialize globals the first time we are called */
	
	Temp = PowerPCTimeMethod;
	if (!Temp) {		/* Initialized? */
		BurgerInitTimers();
		Temp = PowerPCTimeMethod;
	}
	if (Temp==1) {
		/* Use DriverServices if it's available -- it's fast and compatible */
		wide = PowerPCUpTime();
		Foo = (double) ((unsigned long long *)&wide)[0];
		return (unsigned long long)(Foo*PowerPCScale);
	}
	if (Temp==2) {
		/* On a recent PowerPC, we poll the TBR directly */
		PollTBR603((LongWord64_t *)&wide);
		Foo = (double) ((unsigned long long *)&wide)[0];
		return (unsigned long long)(Foo*PowerPCScale);
	}
	if (Temp==3) {
		/* On a 601, we can poll the RTC instead */
		PollRTC601((LongWord64_t*)&wide);
		return (unsigned long long)(RTCToNano(wide) * PowerPCScale);
	}
	/* If all else fails, suffer the mixed mode overhead */
	Microseconds(&wide);
	return wide.lo;
}

/**********************************

	Read the tick value in millisecond accuracy
	
**********************************/

Word32 ReadTickMilliseconds(void)
{
	Word Temp;
	double Foo;
	UnsignedWide wide;

	/* Initialize globals the first time we are called */
	
	Temp = PowerPCTimeMethod;
	if (!Temp) {		/* Initialized? */
		BurgerInitTimers();
		Temp = PowerPCTimeMethod;
	}

	if (Temp==1) {
		/* Use DriverServices if it's available -- it's fast and compatible */
		wide = PowerPCUpTime();
		Foo = (double) ((unsigned long long *)&wide)[0];
		return (unsigned long long)(Foo*PowerPCScale2);
	}
	if (Temp==2) {
		/* On a recent PowerPC, we poll the TBR directly */
		PollTBR603((LongWord64_t *)&wide);
		Foo = (double) ((unsigned long long *)&wide)[0];
		return (unsigned long long)(Foo*PowerPCScale2);
	}
	if (Temp==3) {
		/* On a 601, we can poll the RTC instead */
		PollRTC601((LongWord64_t*)&wide);
		return (unsigned long long)(RTCToNano(wide) * PowerPCScale2);
	}
	/* If all else fails, suffer the mixed mode overhead */
	Microseconds(&wide);
	return (unsigned long long)(((double) WideTo64bit(wide)) * (1.0/1000.0));
}

#pragma options opt=speed

#endif
#endif
#endif


