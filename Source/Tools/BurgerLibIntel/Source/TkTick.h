/*******************************

	Burger's Universal library WIN95 version
	This is for Watcom 10.5 and higher...
	Also support for MSVC 4.0

*******************************/

#ifndef __TKTICK_H__
#define __TKTICK_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct StreamHandle_t;

/* Time and events */

typedef struct TimeDate_t {	/* Used by FileModTime routines */
	Word32 Year;		/* Year 2002 */
	Word16 Milliseconds;	/* 0-999 */
	Word8 Month;		/* 1-12 */
	Word8 Day;		/* 1-31 */
	Word8 DayOfWeek;	/* 0-6 */
	Word8 Hour;		/* 0-23 */
	Word8 Minute;	/* 0-59 */
	Word8 Second;	/* 0-59 */
} TimeDate_t;

#define TICKSPERSEC 60
extern Word32 LastTick;
extern Word32 BURGERCALL ReadTick(void);
#define ResetLastTick() LastTick=ReadTick()
#define WaitEvent() WaitTicksEvent(0)
#define WaitOneTick() ResetLastTick(),WaitTick()
#define WaitTick() WaitTicks(1)
extern void BURGERCALL WaitTicks(Word TickCount);
extern Word BURGERCALL WaitTicksEvent(Word TickCount);
extern Word32 BURGERCALL ReadTickMicroseconds(void);
extern Word32 BURGERCALL ReadTickMilliseconds(void);
extern void BURGERCALL TimeDateGetCurrentTime(TimeDate_t *Input);
extern Word BURGERCALL TimeDateFromANSITime(TimeDate_t *Output,Word32 Input);
extern void BURGERCALL TimeDateTimeString(char *Output,const TimeDate_t *Input);
extern void BURGERCALL TimeDateTimeStringPM(char *Output,const TimeDate_t *Input);
extern void BURGERCALL TimeDateDateString(char *Output,const TimeDate_t *Input);
extern void BURGERCALL TimeDateDateStringVerbose(char *Output,const TimeDate_t *Input);
extern void BURGERCALL TimeDateStreamHandleRead(TimeDate_t *Output,struct StreamHandle_t *Input);
extern void BURGERCALL TimeDateStreamHandleWrite(const TimeDate_t *Input,struct StreamHandle_t *Output);

#ifdef __cplusplus
}
#endif

#endif
