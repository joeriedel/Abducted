/**********************************

	Metrowerks for Intel

**********************************/

#include "ThThreads.h"

#if defined(__MWERKS__) && defined(__WIN32__)

/**********************************

	Lock the mutex, if the mutex is already locked,
	return TRUE, otherwise return FALSE if 
	you obtained the lock.
	
**********************************/

asm Word BURGERCALL MutexLock(Mutex_t *Input)
{
	asm {
	mov 	edx,[esp+4]		/* Get the address */
	mov		eax,1			/* Set to true */
	xchg	eax,[edx]		/* Swap (Return TRUE if already TRUE) */
	ret
	}
}

/**********************************

	Release the lock.
	Never release a lock that you didn't
	obtain.
	
**********************************/

void BURGERCALL MutexUnlock(Mutex_t *Input)
{
	asm {
	mov 	edx,[esp+4]		/* Get the address */
	xor		eax,eax			/* Zero it out */
	xchg	eax,[edx]		/* Set to zero making sure that I am locked */
	ret
	}
}

#endif
