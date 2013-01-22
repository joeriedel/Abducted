/**********************************

	Code Profiling manager
	
**********************************/

#include "PrProfile.h"
#include "TkTick.h"
#include "MmMemory.h"

Profile_t *ProfileRoot;			/* Root list entry for the profile manager */

/**********************************

	Get the current CPU type and feature list.
	This is DIFFERENT for each processor family.

	Intel series will get the manufacturer, family and whether
	it supports MMX and or FPU operations.

	Power PC processors will return the family (601/603/604/750)
	and whether or not it supports ALTIVEC instructions.

**********************************/

#if !defined(__INTEL__) && !defined(__MAC__)

void CPUFeaturesGet(CPUFeatures_t *CPUPtr)
{
	CPUPtr->CPUFamily = CPU_UNKNOWN;
	CPUPtr->MMXFamily = MMX_NONE;
	CPUPtr->FPUFamily = FPU_NONE;
	CPUPtr->Vendor = VENDOR_UNKNOWN;
	CPUPtr->VerboseName[0] = 0;
}

#endif

/**********************************

	Create a new Profile_t struct

**********************************/

Profile_t * BURGERCALL ProfileNew(const char *Name)
{
	Profile_t *MyProfile;
	MyProfile = (Profile_t *)AllocAPointer(sizeof(Profile_t));	/* Allocate it */
	if (MyProfile) {			/* Success? */
		ProfileInit(MyProfile,Name);	/* Initialize the data */
	}
	return MyProfile;		/* Return NULL or the pointer */
}

/**********************************

	Initialize a Profile_t struct

**********************************/

void BURGERCALL ProfileInit(Profile_t *Input,const char *Name)
{
	Input->Next = ProfileRoot;
	ProfileRoot = Input;
	Input->Initialized = TRUE;
	Input->Name = Name;
	ProfileReset(Input);
}

/**********************************

	Delete a Profile_t struct

**********************************/

void BURGERCALL ProfileDelete(Profile_t *Input)
{
	ProfileDestroy(Input);		/* Dispose of the profile contents */
	DeallocAPointer(Input);		/* Dispose of the memory */
}

/**********************************

	Remove a profile from a linked list

**********************************/

void BURGERCALL ProfileDestroy(Profile_t *Input)
{
	Profile_t *Work;
	Profile_t *Prev;

	Work = ProfileRoot;		/* Get the linked list */
	if (Work) {				/* Is there a list? */
		if (Work==Input) {	/* Is this the root? */
			ProfileRoot = Work->Next;	/* Unlink from the root */
		} else {
			for (;;) {
				Prev = Work;		/* Previous link */
				Work = Work->Next;	/* Next link */
				if (!Work) {		/* No more? */
					break;
				}
				if (Work==Input) {	/* Did I find it? */
					Prev->Next = Work->Next;	/* Unlink it */
					break;
				}
			}
		}
	}
}

/**********************************

	This should be called when a piece of code is
	entered so it can be properly benchmarked.

**********************************/

void BURGERCALL ProfileEntry(Profile_t *Input)
{
	Word32 New;
	Word32 Temp;
	if (!Input->Initialized) {		/* Linked in? */
		ProfileInit(Input,Input->Name);	/* Reset the structure */
	}
	if (!Input->RecurseCount) {		/* Is it the first time? */
		++Input->HitCount;			/* I entered */
		New = ReadTickMicroseconds();	/* Get the time mark */
		Temp = New - Input->Mark;	/* Get the elapsed time */
		Input->TimeOut += Temp;		/* Add it */
		Input->Mark = New;			/* Save the new time mark */
	}
	++Input->RecurseCount;			/* Prevent recursion */
}

/**********************************

	When a piece of code is exiting, call
	this routine to properly benchmark the exit

**********************************/

void BURGERCALL ProfileExit(Profile_t *Input)
{
	Word32 New,Temp;
	if (Input->RecurseCount) {		/* Have I been entered? */
		--Input->RecurseCount;		/* Lower recursion flag */
		if (!Input->RecurseCount) {	/* Done? */
			New = ReadTickMicroseconds();	/* Get the time */
			Temp = New - Input->Mark;	/* Get the elapsed time */
			Input->TimeIn += Temp;		/* Add it */
			Input->Mark = New;		/* Save the new mark */
		}
	}
}

/**********************************

	Return the amount of elapsed time
	inside of a profiled routine

**********************************/

double BURGERCALL ProfileGetSecondsIn(const Profile_t *Input)
{
	double Result;
	Result = (double)Input->TimeIn;	/* Convert to a double */
	Result *= (1.0/1000000.0);		/* Scale to seconds */
	return Result;
}

/**********************************

	Return the amount of elapsed time
	outside of a profiled routine

**********************************/

double BURGERCALL ProfileGetSecondsOut(const Profile_t *Input)
{
	double Result;
	Result = (double)Input->TimeOut;	/* Convert to a double */
	Result *= (1.0/1000000.0);		/* Scale to seconds */
	return Result;
}

/**********************************

	Return the amount of elapsed time
	inside of a profiled routine

**********************************/

double BURGERCALL ProfileGetMicrosecondsIn(const Profile_t *Input)
{
	return (double)Input->TimeIn;	/* Convert to a double */
}

/**********************************

	Return the amount of elapsed time
	outside of a profiled routine

**********************************/

double BURGERCALL ProfileGetMicrosecondsOut(const Profile_t *Input)
{
	return (double)Input->TimeOut;	/* Convert to a double */
}

/**********************************

	Reset a profile structures
	time values

**********************************/

void BURGERCALL ProfileReset(Profile_t *Input)
{
	Input->TimeIn = 0;		/* Zap time elapsed */
	Input->TimeOut = 0;
	Input->HitCount = 0;		/* Zap the call count */
	Input->RecurseCount = 0;	/* Zap the recursion flag */
	Input->Mark = ReadTickMicroseconds();		/* Zap the current time mark */
}

/**********************************

	Reset all of the linked profile structures

**********************************/

void BURGERCALL ProfileResetAll(void)
{
	Profile_t *Work;

	Work = ProfileRoot;		/* Use the linked list */
	if (Work) {
		do {
			ProfileReset(Work);		/* Reset the current link */
			Work = Work->Next;		/* Next link */
		} while (Work);			/* Any more? */
	}
}

/**********************************

	Does the hardware exist to support the
	Profile Manager?

**********************************/

#if !defined(__WIN32__) && !defined(__MAC__) && !defined(__BEOS__)

Word BURGERCALL ProfileIsAvailable(void)
{
	/* All others don't support it */
	return FALSE;
}

#endif

