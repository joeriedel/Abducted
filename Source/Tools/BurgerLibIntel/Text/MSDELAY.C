#include "Text.h"

/********************************

	Time delay in milliseconds
	The heartbeat IRQ is 60 times a second
	so I apply a little math (Delay*60/1000) to convert
	milliseconds into ticks.

*********************************/

void MSDelay(LongWord Delay)
{
	Delay = Delay*3UL/50UL;	/* Do the math... */
	LastTick = ReadTick();		/* Reset the timer */
	WaitTicks(Delay);			/* Wait the proper delay */
}
