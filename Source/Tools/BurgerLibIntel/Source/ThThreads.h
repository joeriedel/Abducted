/**********************************

	Thread manager

**********************************/

#ifndef __BRTHREADS_H__
#define __BRTHREADS_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Thread manager */

typedef Word32 Mutex_t;
typedef struct TimerTask_t TimerTask_t;
typedef Word (BURGERCALL *TimerTaskProc)(void *DataPtr);

extern Word BURGERCALL MutexLock(Mutex_t *Input);
extern void BURGERCALL MutexUnlock(Mutex_t *Input);
extern TimerTask_t * BURGERCALL TimerTaskNew(Word Period,TimerTaskProc Proc,void *ProcData,Word Active);
extern void BURGERCALL TimerTaskDelete(TimerTask_t *Input);
extern Word BURGERCALL TimerTaskGetActive(const TimerTask_t *Input);
extern Word BURGERCALL TimerTaskSetActive(TimerTask_t *Input,Word Flag);
extern Word BURGERCALL TimerTaskGetPeriod(const TimerTask_t *Input);
extern Word BURGERCALL TimerTaskSetPeriod(TimerTask_t *Input,Word Period);

#ifdef __cplusplus
}
#endif


#endif

