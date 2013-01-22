#include "ThThreads.h"
#include "TkTick.h"
#include "MmMemory.h"
#include "InInput.h"

/**********************************

	This is the code to handle timer tasks for
	non-threaded systems

**********************************/

#if defined(__WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

/* Actual contents of the TimerTask_t structure */

struct TimerTask_t {
	Word32 EventID;		/* Handle to the background thread */
	TimerTaskProc Proc;	/* Proc to call every time this fires */
	void *ProcData;		/* Data for the called proc */
	Word32 Period;		/* Millisecond timer */
	Word32 Active;		/* TRUE if active */
};

/**********************************

	This is called by my timer task from
	MacOS classic

**********************************/

static void CALLBACK TimerTaskCallback(Word /*uID*/,Word /*uMsg*/,DWORD tmTask,DWORD /*dw1*/,DWORD /*dw2*/)
{
	TimerTask_t *TaskPtr= (TimerTask_t *) tmTask;	/* CFM is no problem */
	
	/* Call the task. If the task returns FALSE (No error) */
	/* Continue firing */
	if (TaskPtr->Active) {
		if (TaskPtr->Proc(TaskPtr->ProcData)) {
			TaskPtr->Active = FALSE;		/* It's shut down */
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
		Word32 EventID;
		Output->Proc = Proc;
		Output->ProcData = ProcData;
		Output->Active = Active;
		Output->Period = Period;
//		Output->EventID = 0;
		EventID = timeSetEvent(Period,1000/60,TimerTaskCallback,(DWORD)Output,TIME_PERIODIC);
		if (EventID) {
			Output->EventID = EventID;
			return Output;
		}
		DeallocAPointer(Output);
	}
	return 0;
}

/**********************************

	Dispose of a timer task

**********************************/

void BURGERCALL TimerTaskDelete(TimerTask_t *Input)
{	
	if (Input) {
		Word32 EventID;
		EventID = Input->EventID;
		if (EventID) {
			timeKillEvent(EventID);
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
	OldFlag = Input->Active;
	Input->Active = Flag;
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
	if (Input->EventID) {
		timeKillEvent(Input->EventID);
	}
	Input->EventID = timeSetEvent(Period,1000/60,TimerTaskCallback,(DWORD)Input,TIME_PERIODIC);
	return OldPeriod;
}

#endif