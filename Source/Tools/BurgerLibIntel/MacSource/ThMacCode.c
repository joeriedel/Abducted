/**********************************

	This is the code to handle threads for Classic MacOS

**********************************/

#include "ThThreads.h"

#if defined(__MAC__)
#include "MmMemory.h"
#include <Timer.h>

/* Actual contents of the TimerTask_t structure */

struct TimerTask_t {
	TMTask Header;		/* Timer header */
	Word Active;		/* TRUE if active */
	Word8 Padding;		/* Not used */
#if !TARGET_RT_MAC_CFM
	long A5Register;	/* 680x0 A5 register */
#endif
	TimerTaskProc Proc;	/* Proc to call every time this fires */
	void *ProcData;		/* Data for the called proc */
	Word Period;		/* Millisecond timer */
};

#if !TARGET_RT_MAC_CFM
#ifdef __68K__
#pragma parameter __D0 MySetA5(__D1)
long MySetA5(long a5)= {0x200D, 0x2A41};
#pragma parameter MySetA52(__D1)
void MySetA52(long a5)= {0x2A41};
#pragma parameter __D0 MyGetA5
long MyGetA5(void)= {0x200d};
#endif

#pragma parameter TimerTaskCallback(__A1)
#endif

/**********************************

	This is called by my timer task from
	MacOS classic

**********************************/

static pascal void TimerTaskCallback(TMTask *tmTask)
{
	/* Classic 68000 has to fix the A5 frame */
	
#if !TARGET_RT_MAC_CFM
	TimerTask_t *TaskPtr= (TimerTask_t *)tmTask; /* get the pointer to this Time Manager task record from a1 */
	long old_a5= MySetA5(TaskPtr->A5Register);		/* set our a5 world */
#else
	TimerTask_t *TaskPtr= (TimerTask_t *) tmTask;	/* CFM is no problem */
#endif
	
	/* Call the task. If the task returns FALSE (No error) */
	/* Continue firing */
	
	if (!TaskPtr->Proc(TaskPtr->ProcData)) {
		PrimeTime((QElemPtr)&TaskPtr->Header, TaskPtr->Period);
	} else {
		TaskPtr->Active = FALSE;		/* It's shut down */
	}
#if !TARGET_RT_MAC_CFM
	MySetA52(old_a5); /* restore whatever a5 was on enter */
#endif
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
#if !TARGET_RT_MAC_CFM
		Output->A5Register = MyGetA5();
#endif
//		Output->Header.tmCount= 0;
//		Output->Header.tmWakeUp= 0;
//		Output->Header.tmReserved= 0;
		Output->Header.tmAddr= NewTimerUPP((TimerProcPtr)TimerTaskCallback);
		InsTime((QElemPtr)&Output->Header);
		if (Active) {
			PrimeTime((QElemPtr)&Output->Header,Period);
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
		RmvTime((QElemPtr)&Input->Header);
		if (Input->Header.tmAddr) {			/* Failsafe */
			DisposeTimerUPP(Input->Header.tmAddr);	/* Release the pointer */
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
	if (Flag != OldFlag) {
		RmvTime((QElemPtr)&Input->Header);
		Input->Active = Flag;
		InsTime((QElemPtr)&Input->Header);
		if (Flag) {
			PrimeTime((QElemPtr)&Input->Header,Input->Period);
		}
	}
	return OldFlag;
}

/**********************************

	Return the active flag of a timer task

**********************************/

Word BURGERCALL TimerTaskGetPeriod(const TimerTask_t *Input)
{
	return Input->Period;
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
	if (Period != OldPeriod) {
		Input->Period = Period;
		if (Input->Active) {		/* Is the timer running? */
			RmvTime((QElemPtr)&Input->Header);	/* Stop it */
			InsTime((QElemPtr)&Input->Header);	/* Put it back */
			PrimeTime((QElemPtr)&Input->Header,Period);	/* Start it */
		}
	}
	return OldPeriod;
}

#endif