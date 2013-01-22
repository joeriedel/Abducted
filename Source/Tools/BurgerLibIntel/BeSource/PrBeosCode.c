#include "PrProfile.h"
#if defined(__BEOS__)

/**********************************

	Does the hardware exist to support the
	Profile Manager?

**********************************/

Word BURGERCALL ProfileIsAvailable(void)
{
	return TRUE;
}

#endif
