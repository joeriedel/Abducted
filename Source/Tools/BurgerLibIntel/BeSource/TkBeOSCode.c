/**********************************

	(BeOS specific version)

**********************************/

#include "TkTick.h"

#if defined(__BEOS__)

#include <Be.h>

/**********************************

	Tick counter for a 60 hertz
	timer

**********************************/

static Bool Inited;
static double Base;			/* Constant for time elapsed since ReadTick() began */

Word32 BURGERCALL ReadTick(void)
{
	double Foo;				/* Temp */
	if (Inited) {			/* Already initialized? */
		Foo = system_time()-Base;	/* Get the elapsed time */
		Foo = Foo*(1.0/(1000000.0/TICKSPERSEC));
		return (Word32)Foo;			/* Return the time value */
	}
	Base = system_time();	/* Get the base time */
	Inited = TRUE;			/* Mark as initialized */
	return 0;				/* Return zip for now */
}

#endif
