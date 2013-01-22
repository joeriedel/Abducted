#include "ThThreads.h"
#include "TkTick.h"
#include "MmMemory.h"
#include "InInput.h"

/**********************************

	This is a simple Mutex implementation
	Note: Since this relies on processor locking,
	I wrote this all in assembly

**********************************/

#if !defined(__POWERPC__) && !defined(__INTEL__) && !defined(__68K__)

/**********************************

	Lock the mutex, if the mutex is already locked,
	return TRUE, otherwise return FALSE if
	you obtained the lock.

**********************************/

Word MutexLock(Mutex_t *Input)
{
	if (!Input[0]) {
		Input[0] = TRUE;
		return FALSE;
	}
	return TRUE;
}

/**********************************

	Release the lock.
	Never release a lock that you didn't
	obtain.

**********************************/

void MutexUnlock(Mutex_t *Input)
{
	Input[0] = FALSE;
}

#endif

/**********************************

	This is the code to handle timer tasks for
	non-threaded systems

**********************************/

#if !defined(__MAC__) && !defined(__WIN32__)

/* Actual contents of the TimerTask_t structure */

struct TimerTask_t {
	Word32 TimeMark;	/* Elapsed time marker */
	TimerTaskProc Proc;	/* Proc to call every time this fires */
	void *ProcData;		/* Data for the called proc */
	Word Period;		/* Millisecond timer */
	Word Active;		/* TRUE if active */
};

/**********************************

	This is called by my timer task from
	MacOS classic

**********************************/

static void BURGERCALL TimerTaskCallback(void *tmTask)
{
	Word32 Mark;
	TimerTask_t *TaskPtr;

	TaskPtr = (TimerTask_t*)tmTask;
	Mark = ReadTickMilliseconds();
	if ((Mark-TaskPtr->TimeMark)>TaskPtr->Period) {
		TaskPtr->TimeMark = Mark;
		if (TaskPtr->Proc(TaskPtr->ProcData)) {
			TaskPtr->Active = FALSE;		/* It's shut down */
			KeyboardRemoveRoutine(TimerTaskCallback,TaskPtr);
		}
	}
}

/**********************************

	Create a new timer task

**********************************/

TimerTask_t * BURGERCALL TimerTaskNew(Word Period,TimerTaskProc Proc,void *ProcData,Word Active)
{
	TimerTask_t *Output;
	if (!Period) {
		Period = 1;
	}
	Output = (TimerTask_t *)AllocAPointerClear(sizeof(TimerTask_t));
	if (Output) {
		Output->Proc = Proc;
		Output->ProcData = ProcData;
		Output->Active = Active;
		Output->Period = Period;
//		Output->TimeMark = 0;
		if (Active) {
			Output->TimeMark = ReadTickMilliseconds();
			KeyboardAddRoutine(TimerTaskCallback,Output);
		}
	}
	return Output;
}

/**********************************

	Dispose of a timer task

**********************************/

void BURGERCALL TimerTaskDelete(TimerTask_t *Input)
{
	if (Input) {
		if (Input->Active) {
			KeyboardRemoveRoutine(TimerTaskCallback,Input);
		}
		DeallocAPointer(Input);
	}
}

/**********************************

	Return the active flag of a timer task

**********************************/

Word BURGERCALL TimerTaskGetActive(const TimerTask_t *Input)
{
	return Input->Active;
}

/**********************************

	Start/stop a timer task

**********************************/

Word BURGERCALL TimerTaskSetActive(TimerTask_t *Input,Word Flag)
{
	Word OldFlag;
	if (Flag) {
		Flag = TRUE;		/* Force a 1 for a non-zero value */
	}
	OldFlag = Input->Active;
	if (Flag != OldFlag) {
		Input->Active = Flag;
		if (OldFlag) {
			KeyboardRemoveRoutine(TimerTaskCallback,Input);
		} else {
			Input->TimeMark = ReadTickMilliseconds();
			KeyboardAddRoutine(TimerTaskCallback,Input);
		}
	}
	return OldFlag;
}

/**********************************

	Return the active flag of a timer task

**********************************/

Word BURGERCALL TimerTaskGetPeriod(const TimerTask_t *Input)
{
	return Input->Period;		/* Return the time quantum */
}

/**********************************

	Start/stop a timer task

**********************************/

Word BURGERCALL TimerTaskSetPeriod(TimerTask_t *Input,Word Period)
{
	Word OldPeriod;
	if (!Period) {
		Period = 1;
	}
	OldPeriod = Input->Period;
	Input->Period = Period;
	return OldPeriod;
}

#endif
