/**********************************

	Metrowerks for 680x0

**********************************/

#include "ThThreads.h"

#if defined(__MAC__) && !defined(__POWERPC__)

#include <ConditionalMacros.h>

#if TARGET_RT_MAC_CFM
#define rts rtd #4
#endif

/**********************************

	Lock the mutex, if the mutex is already locked,
	return TRUE, otherwise return FALSE if 
	you obtained the lock.
	
**********************************/

asm Word BURGERCALL MutexLock(Mutex_t *Input)
{
#if TARGET_RT_MAC_CFM
	movea.l	(a1),a5
#endif
	movea.l	4(a7),a0 	/* Get the address */
	tas		(a0)		/* Perform the mutex (Sets 0x80 as a byte) */
	bne.s	Bad			/* Was it already set? */
	moveq	#0,d0		/* I am accepting the mutex */
	rts					/* Done */
Bad:
	moveq	#1,d0		/* Oh oh... */
	rts					/* Done */
}

/**********************************

	Release the lock.
	Never release a lock that you didn't
	obtain.
	
**********************************/

void BURGERCALL MutexUnlock(Mutex_t *Input)
{
	Input[0] = 0;		/* Force it to be zero */
}

#endif
