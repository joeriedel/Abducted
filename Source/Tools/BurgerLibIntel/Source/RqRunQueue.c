#include "RqRunQueue.h"
#include "MmMemory.h"
#include "PfPrefs.h"
#include "ClStdLib.h"

/**********************************

	Allocate and initialize a Master RunQueue_t
	structure, return the structure or a NULL if error

**********************************/

RunQueue_t ** BURGERCALL MasterRunQueueNew(void)
{
	RunQueue_t **MasterQueueHandle;

	MasterQueueHandle = (RunQueue_t **)AllocAHandle(sizeof(RunQueue_t));
	if (MasterQueueHandle) {
		MasterRunQueueInit(MasterQueueHandle);		/* Init the queue */
	}
	return MasterQueueHandle;		/* Return the handle */
}

/**********************************

	Using a preexisting RunQueue_t struct, initialize the data

**********************************/

void BURGERCALL MasterRunQueueInit(RunQueue_t **MasterQueueHandle)
{
	RunQueue_t *RunQueuePtr;
	if (MasterQueueHandle) {		/* Valid handle? */
		RunQueuePtr = *MasterQueueHandle;
		RunQueuePtr->NextQueue = MasterQueueHandle;	/* Init the handles */
		RunQueuePtr->PrevQueue = MasterQueueHandle;
		RunQueuePtr->Proc = 0;			/* No code proc */
		RunQueuePtr->TimeQuantum = 0;	/* How many times to call myself */
		RunQueuePtr->ElapsedTime = 0;	/* Time elapsed */
		RunQueuePtr->Priority = 0;		/* Low priority */
		RunQueuePtr->IDNum = (Word)-1;	/* Master RunQueue_t */
	}
}


/**********************************

	Dispose of the entire contents of a Master
	RunQueue_t handle and then dispose of the handle

**********************************/

void BURGERCALL MasterRunQueueDelete(RunQueue_t **MasterQueueHandle)
{
	if (MasterQueueHandle) {
		MasterRunQueueDestroy(MasterQueueHandle);		/* Discard the contents */
		DeallocAHandle((void **)MasterQueueHandle);	/* Dispose of the memory */
	}
}


/**********************************

	Dispose of the entire contents of a Master
	RunQueue_t handle.

**********************************/

void BURGERCALL MasterRunQueueDestroy(RunQueue_t **MasterQueueHandle)
{
	RunQueue_t **NextHandle;
	if (MasterQueueHandle) {		/* Valid? */
		goto Middle;			/* Begin the code */

		do {
			RunQueueDelete(NextHandle);		/* Dispose of the record */
Middle:
			NextHandle = (*MasterQueueHandle)->NextQueue;	/* Next one */
		} while (NextHandle!=MasterQueueHandle);		/* All out? */
	}
}


/**********************************

	Traversing the linked list, execute the proc for each
	RunQueue_t struct. If the TimeQuantum input is zero, then only
	execute code that allows time to pass.

	Note : Code in a Proc can NEVER call DeallocARunQueue(), if
		must call DeallocARunQueueDefer() to yield the same results.
	Note : If the time quantum is -1 then I can only execute the code
		once, and the code COULD be a deferred delete, so make sure
		I cache the NextQueue handle before I call the proc to
		prevent the memory from disappearing under me.
	Note : A TimeQuantum in the RunQueue_t means that I execute forever.
		This allows subroutines called as DSP code.

**********************************/

void BURGERCALL MasterRunQueueExecute(RunQueue_t **MasterQueueHandle,Word TimeQuantum)
{
	RunQueue_t **NextHandle;
	RunQueue_t *WorkPtr;
	RunQueue_t **WorkHandle;

	if (MasterQueueHandle) {		/* Is the master handle ok? */
		NextHandle = (*MasterQueueHandle)->NextQueue;	/* Get the next one */
		if (NextHandle!=MasterQueueHandle) {
			do {
				WorkHandle = NextHandle;	/* I use this handle */
				WorkPtr = *WorkHandle;		/* Deref the handle */
				NextHandle = WorkPtr->NextQueue;		/* Cache the NEXT handle */
				if (WorkPtr->TimeQuantum==(Word)-1) {	/* Special case? */
					WorkPtr->ElapsedTime = TimeQuantum;		/* Return the elapsed time */
					WorkPtr->Proc((void **)WorkHandle,RQTIME);	/* Call code */
				} else if (TimeQuantum) {		/* Any time elapsed? */
					WorkPtr->ElapsedTime += TimeQuantum;	/* Adjust timer */
					while (WorkPtr->ElapsedTime>=WorkPtr->TimeQuantum) {	/* Time up? */
						WorkPtr->ElapsedTime-=WorkPtr->TimeQuantum;	/* Remove the time */
						WorkPtr->Proc((void **)WorkHandle,RQTIME);	/* Call the proc */
						WorkPtr = *WorkHandle;		/* Deref again */
						if (WorkPtr->TimeQuantum==(Word)-1) {		/* No more? */
							break;		/* End now! */
						}
					}
				}
			} while (NextHandle!=MasterQueueHandle);	/* No more? */
		}
	}
}


/**********************************

	Used for debugging only, it will dump a chain of
	RunQueue_t structs

**********************************/

static Word BURGERCALL DumpQueuesProc(RunQueue_t **Input)
{
	char OneLine[60];
	RunQueue_t *WorkPtr;

	WorkPtr = *Input;
	OneLine[0] = '$';
	OneLine[10] = '$';
	OneLine[20] = '$';
	OneLine[53] = '\n';
	OneLine[9] = ' ';
	OneLine[19] = ' ';
	OneLine[29] = ' ';
	OneLine[35] = ' ';
	OneLine[41] = ' ';
	OneLine[47] = ' ';
	OneLine[54] = 0;
	LongWordToAsciiHex2((Word32)Input,&OneLine[1],ASCIINONULL|ASCIILEADINGZEROS|8);
	LongWordToAsciiHex2((Word32)WorkPtr->PrevQueue,&OneLine[11],ASCIINONULL|ASCIILEADINGZEROS|8);
	LongWordToAsciiHex2((Word32)WorkPtr->NextQueue,&OneLine[21],ASCIINONULL|ASCIILEADINGZEROS|8);
	LongWordToAscii2((Word32)WorkPtr->IDNum,&OneLine[30],ASCIINONULL|ASCIILEADINGZEROS|5);
	LongWordToAscii2((Word32)WorkPtr->TimeQuantum,&OneLine[36],ASCIINONULL|ASCIILEADINGZEROS|5);
	LongWordToAscii2((Word32)WorkPtr->ElapsedTime,&OneLine[42],ASCIINONULL|ASCIILEADINGZEROS|5);
	LongWordToAscii2((Word32)WorkPtr->Priority,&OneLine[48],ASCIINONULL|ASCIILEADINGZEROS|5);
	DebugXString(OneLine);
	return FALSE;		/* Do not abort */
}

void BURGERCALL MasterRunQueueDump(RunQueue_t **MasterQueueHandle)
{
	DebugXString("List of active RunQueue_t's\n"
	"  Handle     Prev      Next    ID   Time  Elaps Prior\n");
/*  "$00000000 $00000000 $00000000 00000 00000 00000 00000 */
	RunQueuePoll(MasterQueueHandle,(RunQueuePollCallback_t) DumpQueuesProc,0);
}


/**********************************

	Allocate a RunQueue_t handle and insert it into
	a linked list.

**********************************/

Word BURGERCALL RunQueueCallProc(RunQueue_t **RunQueueHandle,Word Event)
{
	if (RunQueueHandle) {
		return (*RunQueueHandle)->Proc((void **)RunQueueHandle,Event);
	}
	return FALSE;		/* I didn't do anything special */
}


/**********************************

	Default RunQueue_t code procedure

**********************************/

Word BURGERCALL RunQueueDefaultProc(void ** /* RunQueueHandle */,Word /* Event */)
{
	return FALSE;		/* Pass through everything */
}



/**********************************

	Allocate a new RunQueue_t struct

**********************************/

RunQueue_t ** BURGERCALL RunQueueNew(RunQueue_t **MasterQueueHandle,Word MemSize,
	Word TimeQuantum,Word Priority,Word IDNum,RunQueueProc_t Proc)
{
	RunQueue_t **WorkHandle;
	RunQueue_t *WorkPtr;

	if (!Proc) {		/* Use default? */
		Proc = RunQueueDefaultProc;		/* Assign a default proc */
	}
	if (MasterQueueHandle && MemSize>=sizeof(RunQueue_t)) {
		WorkHandle = (RunQueue_t **)AllocAHandleClear(MemSize);	/* Get memory */
		if (WorkHandle) {			/* Got it? */
			WorkPtr = *WorkHandle;		/* Deref the handle */
			WorkPtr->TimeQuantum = TimeQuantum;	/* Set the defaults */
			WorkPtr->IDNum = IDNum;
			WorkPtr->Proc = Proc;
			RunQueueLink(MasterQueueHandle,WorkHandle,Priority);
			Proc((void **)WorkHandle,RQINIT);	/* Call the init code (If any) */
			return WorkHandle;		/* Return the new record */
		}
	}
	return 0;			/* No good! */
}


/**********************************

	Dispose of a RunQueue_t struct immediately,
	Warning : Never use this routine to dispose of a MasterRunQueue.
		That would be bad. Use MasterRunQueueDelete() instead.

**********************************/

void BURGERCALL RunQueueDelete(RunQueue_t **RunQueueHandle)
{
	if (RunQueueHandle) {		/* Valid handle? */
		RunQueueUnlink(RunQueueHandle);		/* Unlink it */
		(*RunQueueHandle)->Proc((void **)RunQueueHandle,RQDESTROY);	/* Call proc */
		DeallocAHandle((void **)RunQueueHandle);	/* Dispose of the memory */
	}
}

/**********************************

	This is a RunQueue_t proc that will dispose of my own handle.
	It is called ONLY by MasterQueueExecute() since the handle input
	will be disposed of.

**********************************/

Word BURGERCALL RunQueueDeleteProc(void **RunQueueHandle,Word /* Event */)
{
	RunQueueDelete((RunQueue_t **)RunQueueHandle);	/* Dispose of myself */
	return TRUE;			/* I handled it! */
}


/**********************************

	Dispose of a RunQueue_t at a later time, this allows
	RunQueue_t routines to dispose of themselves safely so that
	programs that traverse linked lists won't be surprised.

**********************************/

void BURGERCALL RunQueueDeleteDefer(RunQueue_t **RunQueueHandle)
{
	RunQueue_t *WorkPtr;
	if (RunQueueHandle) {
		WorkPtr = *RunQueueHandle;
		WorkPtr->TimeQuantum = (Word)-1;	/* Force a call on main loop */
		WorkPtr->IDNum = 0;			/* Kill the ID number */
		WorkPtr->Proc = RunQueueDeleteProc;		/* Delete me next time! */
	}
}



/**********************************

	Insert a RunQueue_t struct into a linked list using
	a priority value.
	If it's 255, link to the beginning of the list.
	If it's 0, link to the end of the list.
	All others, must traverse the list to find its place.

**********************************/

void BURGERCALL RunQueueLink(RunQueue_t **MasterQueueHandle,RunQueue_t **RunQueueHandle,Word Priority)
{
	RunQueue_t **NextHandle;
	if (MasterQueueHandle && RunQueueHandle) {		/* Valid?? */
		if (Priority) {		/* Link to the end of the list? */
			/* Should I link AFTER MasterQueueHandle? */
			if (Priority>=255) {	/* Link to the beginning of the list? */
				Priority = 255;		/* Failsafe */
				MasterQueueHandle = (*MasterQueueHandle)->NextQueue;
			} else {

	/* Follow the list and link BEFORE the first handle that is LOWER */
	/* than the priority requested */

				NextHandle = (*MasterQueueHandle)->NextQueue;	/* Get the next one */
				if (NextHandle!=MasterQueueHandle) {
					do {
						if ((*NextHandle)->Priority<=Priority) {		/* Is the priority lower? */
							MasterQueueHandle = NextHandle;		/* Link BEFORE this? */
							break;
						}
						NextHandle = (*NextHandle)->NextQueue;	/* Next */
					} while (NextHandle!=MasterQueueHandle);	/* No more? */
				}
			}
		}

	/* Use the Handle "MasterQueueHandle" and link BEFORE it */

		(*RunQueueHandle)->NextQueue = MasterQueueHandle;
		(*RunQueueHandle)->PrevQueue = (*MasterQueueHandle)->PrevQueue;
		(*(*MasterQueueHandle)->PrevQueue)->NextQueue = RunQueueHandle;
		(*MasterQueueHandle)->PrevQueue = RunQueueHandle;
		(*RunQueueHandle)->Priority = Priority;
	}
}


/**********************************

	Remove a RunQueue struct from a linked list.
	When this routine is done, the RunQueue is no longer linked
	to its master list.

**********************************/

void BURGERCALL RunQueueUnlink(RunQueue_t **RunQueueHandle)
{
	RunQueue_t *WorkPtr;
	if (RunQueueHandle) {
		WorkPtr = *RunQueueHandle;		/* Deref my handle */
		(*WorkPtr->PrevQueue)->NextQueue = WorkPtr->NextQueue;	/* Unlink */
		(*WorkPtr->NextQueue)->PrevQueue = WorkPtr->PrevQueue;
	}
}

/**********************************

	Scan a list of RunQueues and return either
	a NULL if I hit the end of the list or a handle to
	a RunQueue that matches the IDNum
	If IDNum input is zero, then just traverse the list to the end.

**********************************/

RunQueue_t ** BURGERCALL RunQueueFindIDNum(RunQueue_t **RunQueueHandle,Word IDNum)
{
	Word TheID;
	if (RunQueueHandle) {		/* Valid handle? */
		goto Middle;
		do {
			if (!IDNum || 		/* Looking for all? */
				TheID==IDNum) {	/* Specific match? */
				return RunQueueHandle;		/* Return the current handle */
			}
Middle:
			RunQueueHandle = (*RunQueueHandle)->NextQueue;	/* Next one in chain */
			TheID = (*RunQueueHandle)->IDNum;	/* Get the ID number */
		} while (TheID!=(Word)-1);		/* Last member? */
	}
	return 0;		/* No more scanning */
}

/**********************************

	Traverse the linked list of RunQueue_t records and
	call a callback routine for each one that matches a specific
	ID. If the ID requested is 0, then scan all of them.

**********************************/

Word BURGERCALL RunQueuePoll(RunQueue_t **MasterQueueHandle,RunQueuePollCallback_t CallBack,Word IDNum)
{
	Word Result;
	Result = FALSE;
	do {
		MasterQueueHandle = RunQueueFindIDNum(MasterQueueHandle,IDNum);
		if (!MasterQueueHandle) {		/* Found one? */
			break;
		}
		Result = CallBack((void **)MasterQueueHandle);	/* Call the proc */
	} while (!Result);		/* Stay here? */
	return Result;		/* Did I abort? */
}

