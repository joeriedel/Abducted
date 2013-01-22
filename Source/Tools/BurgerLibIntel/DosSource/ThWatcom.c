/***************************

	Watcom "C" version

***************************/

#include "ThThreads.h"

#if defined(__WATCOMC__)

/**********************************

	Lock the mutex, if the mutex is already locked,
	return TRUE, otherwise return FALSE if
	you obtained the lock.

**********************************/

extern Word FooMutexLock(Mutex_t *Input);

#pragma aux FooMutexLock = \
	"push	edx" \
	"mov	edx,eax" \
	"mov	eax,1" \
	"xchg	eax,[edx]" \
	"pop	edx" \
	parm [eax] \
	value [eax]

Word BURGERCALL MutexLock(Mutex_t *Input)
{
	return FooMutexLock(Input);
}

/**********************************

	Release the lock.
	Never release a lock that you didn't
	obtain.

**********************************/

extern void FooMutexUnlock(Mutex_t *Input);

#pragma aux FooMutexUnlock = \
	"push	edx" \
	"mov	edx,eax" \
	"xor	eax,eax" \
	"xchg	eax,[edx]" \
	"pop	edx" \
	parm [eax] \
	value [eax]

void BURGERCALL MutexUnlock(Mutex_t *Input)
{
	FooMutexUnlock(Input);
}

#endif
