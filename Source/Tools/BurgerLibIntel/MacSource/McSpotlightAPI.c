/*________________________________________________________________________________
	File:		SpotlightAPI.c

	Contains:	Spotlight stub functions for user code.

	Written by:	Onyx Technology, Inc.

	Copyright:	Copyright © 1996-1998 by Onyx Technology, Inc.

	Change History (most recent first):

		1998.06.02	dth		added SLInitialize and SLStart.
		1998.03.16	dth		Added SLInit() to public interfaces.

________________________________________________________________________________*/

#include "McMac.h"

#if defined(__POWERPC__) && defined(__MAC__) && !_SHAREDLIB

/* These are internal to the Spotlight API. You MUST be running spotlight */
/* to access these */

#ifdef __cplusplus
extern "C" {
#endif
extern pascal signed short __initialize(struct CFragSystem7InitBlock *theInitBlock);
extern pascal void __start(void);
#ifdef __cplusplus
}
#endif

void SLInit(void)
{
}

void SLDisable(void)
{
}

void SLEnable(void)
{

}

void SLEnterInterrupt(void)
{

}

void SLLeaveInterrupt(void)
{

}

void SLResetLeaks(void)
{

}


pascal signed short SLInitialize(struct CFragSystem7InitBlock *theInitBlock)
{ 
	SLInit();
	return(__initialize(theInitBlock));
}

pascal void SLStart(void)
{
	SLInit();
	__start();
}

#endif