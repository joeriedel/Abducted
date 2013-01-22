/**********************************

	Microsquish "C"

**********************************/

#include "ThThreads.h"

#if !defined(__MWERKS__) && defined(__WIN32__)
#pragma warning(disable:4035)

/**********************************

	Lock the mutex, if the mutex is already locked,
	return TRUE, otherwise return FALSE if 
	you obtained the lock.
	
**********************************/

__declspec(naked) Word BURGERCALL MutexLock(Mutex_t *Input)
{
	_asm {
	mov		eax,1			/* Set to true */
	xchg	eax,[ecx]		/* Swap (Return TRUE if already TRUE) */
	ret
	}
}

/**********************************

	Release the lock.
	Never release a lock that you didn't
	obtain.
	
**********************************/

__declspec(naked) void BURGERCALL MutexUnlock(Mutex_t *Input)
{
	_asm {
	xor		eax,eax			/* Zero it out */
	xchg	eax,[ecx]		/* Set to zero making sure that I am locked */
	ret
	}
}

#endif
