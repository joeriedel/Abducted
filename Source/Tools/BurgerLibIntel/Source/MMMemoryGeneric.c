/********************************

	Burgerlib Memory manager
	Generic routines
	
********************************/

#include "MmMemory.h"
#undef AllocAHandle
#undef AllocAHandle2
#undef AllocAHandleClear
#undef DeallocAHandle
#undef AllocAPointer
#undef AllocAPointerClear
#undef DeallocAPointer
#undef ReallocAHandle
#undef ResizeAPointer
#undef ResizeAHandle
#undef MemoryNewPointerCopy
#undef MemoryNewHandleCopy
#include "ClStdLib.h"

/********************************

	Debug intercept to allocate a handle

********************************/

void ** BURGERCALL DebugAllocAHandle2(Word32 MemSize,Word Flag,const char *Source,Word LineNum)
{
	void **NewMem;

	if (MemSize) {		/* Don't allocate an empty handle! */
		NewMem = AllocAHandle2(MemSize,Flag);	/* Get the memory */
		if (NewMem) {
			DebugAddSourceLine(NewMem,Source,LineNum,FALSE);
			return NewMem;
		}
		DebugXMessage("Memory allocation failed for %lu bytes in %s, line %u\n",MemSize,Source,LineNum);
	}
	return 0;		/* Return nil */
}

/**********************************

	Allocate a handle and then clear out
	the memory.

**********************************/

void ** BURGERCALL DebugAllocAHandleClear(Word32 MemSize,const char *Source,Word LineNum)
{
	void **TempPtr;
	TempPtr = DebugAllocAHandle2(MemSize,0,Source,LineNum);	/* Allocate the memory */
	if (TempPtr) {
		FastMemSet(TempPtr[0],0,MemSize);		/* Clear it out if successful */
	}
	return TempPtr;		/* Return the pointer */
}

/**********************************

	Allocate a pointer and then clear out
	the memory.

**********************************/

void * BURGERCALL DebugAllocAPointerClear(Word32 MemSize,const char *Source,Word LineNum)
{
	void *TempPtr;
	TempPtr = DebugAllocAPointer(MemSize,Source,LineNum);	/* Allocate the memory */
	if (TempPtr) {
		FastMemSet(TempPtr,0,MemSize);		/* Clear it out if successful */
	}
	return TempPtr;		/* Return the pointer */
}

/********************************

	Dispose of a memory handle into the free handle pool

********************************/

void BURGERCALL DebugDeallocAHandle(void **MemHandle,const char *Source,Word LineNum)
{
	if (MemHandle) {		/* Valid pointer? */
		if (!DebugRemoveSourceLine((void **)MemHandle,Source,LineNum)) {
			DeallocAHandle(MemHandle);
		}
	}
}

/**********************************

	Release a POINTER, I created a handle with 4
	bytes extra data and I place the handle at the beginning

**********************************/

void BURGERCALL DebugDeallocAPointer(void *MemPtr,const char *Source,Word LineNum)
{
	if (MemPtr) {			/* Null pointer?!? */
		if (!DebugRemoveSourceLine((void **)MemPtr,Source,LineNum)) {
			DeallocAPointer(MemPtr);
		}
	}
}

/**********************************

	Using a pointer to memory, reallocate the size and copy
	the contents.
	If I request a zero length buffer, I just deallocate the input
	pointer, if the input pointer is null, I just allocate a fresh pointer.

**********************************/

void * BURGERCALL DebugResizeAPointer(void *Mem,Word32 size,const char *Source,Word LineNum)
{
	void *NewMem;
	/* Convert the pointer back into a handle and perform the operation */

	if (Mem) {
		if (DebugRemoveSourceLine(static_cast<void **>(Mem),Source,LineNum)) {
			return 0;
		}
	}
	NewMem = ResizeAPointer(Mem,size);
	if (NewMem) {
		DebugAddSourceLine(static_cast<void **>(NewMem),Source,LineNum,TRUE);
	}
	return NewMem;
}

/**********************************

	Using a handle to memory, reallocate the size and copy
	the contents. If the input handle is null, then just allocate a
	new handle, if the size requested is null then discard the input
	handle.

**********************************/

void ** BURGERCALL DebugResizeAHandle(void **Mem,Word32 NewSize,const char *Source,Word LineNum)
{
	void **NewMem;

	if (Mem) {
		if (DebugRemoveSourceLine(Mem,Source,LineNum)) {
			return 0;
		}
	}
	NewMem = ResizeAHandle(Mem,NewSize);
	if (NewMem) {
		DebugAddSourceLine(NewMem,Source,LineNum,FALSE);
	}
	return NewMem;
}

/**********************************

	Allocate a handle and clear the memory

**********************************/

void ** BURGERCALL AllocAHandleClear(Word32 MemSize)
{
	void **TempHand;

	TempHand = AllocAHandle2(MemSize,0);		/* Allocate the handle */
	if (TempHand) {
		FastMemSet(*TempHand,0,MemSize);	/* Clear it */
	}
	return TempHand;		/* Return the handle (Or error) */
}

/**********************************

	Allocate a pointer and then clear out
	the memory.

**********************************/

void * BURGERCALL AllocAPointerClear(Word32 MemSize)
{
	void *TempPtr;
	TempPtr = AllocAPointer(MemSize);	/* Allocate the memory */
	if (TempPtr) {
		FastMemSet(TempPtr,0,MemSize);		/* Clear it out if successful */
	}
	return TempPtr;		/* Return the pointer */
}

/**********************************

	Allocate memory and copy the contents
	of the pointer to the new memory

**********************************/

void * BURGERCALL MemoryNewPointerCopy(const void *Mem,Word32 Size)
{
	void *TempPtr;
	if (Size) {			/* Sanity check */
		TempPtr = AllocAPointer(Size);		/* Get memory */
		if (TempPtr) {						/* Valid? */
			if (Mem) {						/* Anything to copy? */
				FastMemCpy(TempPtr,Mem,Size);	/* Copy it! */
			}
			return TempPtr;					/* Return the new memory */
		}
	}
	return 0;			/* Get real! */
}

/**********************************

	Allocate memory and copy the contents
	of the pointer to the new memory

**********************************/

void * BURGERCALL DebugMemoryNewPointerCopy(const void *Mem,Word32 Size,const char *Source,Word LineNum)
{
	void *TempPtr;
	if (Size) {			/* Sanity check */
		TempPtr = DebugAllocAPointer(Size,Source,LineNum);		/* Get memory */
		if (TempPtr) {						/* Valid? */
			if (Mem) {						/* Anything to copy? */
				FastMemCpy(TempPtr,Mem,Size);	/* Copy it! */
			}
			return TempPtr;					/* Return the new memory */
		}
	}
	return 0;			/* Get real! */
}

/**********************************

	Allocate a handle and copy the contents
	of a supplied handle into it

**********************************/

void ** BURGERCALL MemoryNewHandleCopy(void **Mem)
{
	Word32 Size;
	void **TempHand;
	if (Mem) {							/* Handle valid? */
		Size = GetAHandleSize(Mem);		/* Size of the handle */
		if (Size) {						/* Valid size */
			TempHand = AllocAHandle2(Size,0);	/* Get the memory */
			if (TempHand) {
				void *SrcPtr;
				SrcPtr = Mem[0];		/* Is the handle purged? */
				if (SrcPtr) {			/* Nope */
					FastMemCpy(TempHand[0],SrcPtr,Size);	/* Copy the contents */
				}
				return TempHand;		/* Return the copy */
			}
		}
	}
	return 0;			/* Get real! */
}

/**********************************

	Allocate a handle and copy the contents
	of a supplied handle into it

**********************************/

void ** BURGERCALL DebugMemoryNewHandleCopy(void **Mem,const char *Source,Word LineNum)
{
	Word32 Size;
	void **TempHand;
	if (Mem) {							/* Handle valid? */
		Size = GetAHandleSize(Mem);		/* Size of the handle */
		if (Size) {						/* Valid size */
			TempHand = DebugAllocAHandle2(Size,0,Source,LineNum);	/* Get the memory */
			if (TempHand) {
				void *SrcPtr;
				SrcPtr = Mem[0];		/* Is the handle purged? */
				if (SrcPtr) {			/* Nope */
					FastMemCpy(TempHand[0],SrcPtr,Size);	/* Copy the contents */
				}
				return TempHand;		/* Return the copy */
			}
		}
	}
	return 0;			/* Get real! */
}


