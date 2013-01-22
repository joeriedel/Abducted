/**********************************

	MacOS version of the memory manager

**********************************/

#include "MmMemory.h"

/* Release all the debug macros */

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

#if defined(__MAC__)
#define __USEFASTALLOC__ 1
#else
#define __USEFASTALLOC__ 0
#endif

#if __MACUSENATIVEMEM__
#include <MacMemory.h>
#include <Errors.h>
#include <stdlib.h>
#include "ClStdLib.h"

#ifdef __cplusplus
extern "C" {
static void DeInitMemory(void);
}
#endif

static Word32 AllocatedMemSize;

#if __USEFASTALLOC__

/**********************************

	This is a pool based memory manager.
	Only for pointers.

**********************************/

#if _DEBUG
#define NEW_MEM_DEBUG
#endif

#define NEW_MEMPOOL_SIZE_COUNT 20			/* Number of chunk sizes */
#define MEMSTATUSSIZE (sizeof(MemStatus_t)-4)

typedef struct MemStatus_t {
	struct MemStatus_t *NextPtr;	/* Linked list of chunks */
	struct MemStatus_t *PrevPtr;	/* Previous chunk */
	Word32 ChunksRemaining;		/* Number of chunks available in this group */
	Word32 TotalChunks;			/* Total number of chunks present (Can be smaller than max) */

	struct FastPtr_t *FreePtr;		/* Pointer to first free memory */
	Word32 PoolSizeIndex;			/* Pool size index */
	Word32 RealChunkSize;			/* Size of each chunk with header */
	Word32 Padding;				/* Make 16 byte aligned */

	Word8 PoolData[4];				/* Memory! */
} MemStatus_t;

typedef struct MemModule_t {
	MemStatus_t *PoolPtrs[NEW_MEMPOOL_SIZE_COUNT];	/* Starting chunk size for the list */
#ifdef NEW_MEM_DEBUG
	Word32 PoolTotalBytes;	/* Memory allocated for pools */
	Word32 DirectTotalBytes;	/* Memory allocated via NewPtr */
	Word TrueAllocCount;		/* Number of allocs */
	Word PoolAllocCount[NEW_MEMPOOL_SIZE_COUNT];	/* Number of allocations per pool */
#endif
} MemModule_t;

/* This structure must be 16 bytes in size, NO MORE, NO LESS!!! */
/* This is the header for each and every pointer */

typedef struct FastPtr_t {
	Word32 Signature;				/* Signature for sanity check */
	Word32 Size;					/* Size of memory allocated */
	MemStatus_t *MemStatusPtr;		/* Which module is my parent */
	struct FastPtr_t *NextPtr;		/* Forward link in free list */
} FastPtr_t;

#define FASTSIG 0xD5AA				/* 16 bits for single PowerPC instruction */

/* Size of each chunk with FastPtr_t included */

typedef struct ChunkDesc_t {
	Word ChunkSize;			/* Size of each chunk (Real size) */
	Word32 ChunkAlloc;	/* Total memory to allocate */
	Word ChunkCount;		/* Number of chunks present */
} ChunkDesc_t;

/* Chunk sizes I support */
/* These MUST be ascending order or the binary search will fail */

/* Formula for figuring out the size of the buffer needed */

//#define BIG(size,count) ((size+16)*count)+32)

#define V1 16		/* 255 -> 8192 */
#define V2 32		/* 170 -> 8192 */
#define V3 48		/* 127 -> 8192 (32) */
#define V4 64		/* 102 -> 8192 */
#define V5 80		/* 85 -> 8192 */
#define V6 96		/* 72 -> 8192 (96) */
#define V7 112		/* 63 -> 8192 (96) */
#define V8 128		/* 56 -> 8192 (96) */
#define V9 192		/* 39 -> 8192 (48) */
#define V10 256		/* 30 -> 8192 */
#define V11 512		/* 23 -> 12176 (400) */
#define V12 768		/* 16 -> 12576 */
#define V13 1024	/* 16 -> 16672 */
#define V14 2048	/* 16 -> 33056 */
#define V15 4096	/* 16 -> 65792 */
#define V16 8192	/* 16 -> 131328 */
#define V17 12576	/* 16 -> 201504 */
#define V18 16672	/* 16 -> 267040 */
#define V19 24576	/* 16 -> 393504 */
#define V20 33056	/* 16 -> 529184 */

#define F(x,y) y+16,x,((x-MEMSTATUSSIZE)/(y+16))		/* Divide from memory chunk */
#define G(x,y) y+16,((x*(y+16))+MEMSTATUSSIZE),x		/* Mul by static chunk count */

static const ChunkDesc_t MemChunkCount[NEW_MEMPOOL_SIZE_COUNT]={
	G(255, V1),G(170, V2),G(127, V3),G(102, V4),G( 85, V5),
	G( 72, V6),G( 63, V7),G( 56, V8),G( 39, V9),G( 30,V10),
	G( 23,V11),G( 16,V12),G( 16,V13),G( 16,V14),G( 16,V15),
	G( 16,V16),G( 16,V17),G( 16,V18),G( 16,V19),G( 16,V20)
};

#undef F
#undef G

static MemModule_t MyMem;			/* Memory module */

#if __USEFASTALLOC__
static Word8 MemDying;				/* TRUE if I called exit() */
#endif

/**********************************

	Using a binary search, find which pool
	table to use.

**********************************/

static const Word8 PoolFast[257] = {
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	 9
};

static INLINECALL Word FindPoolSize(Word32 Size)
{
	if (Size<=V16) {
		if (Size<=V12) {
			if (Size<=V11) {
				return 10;			/* 257-512 */
			}
			return 11;				/* 513-768 */
		}
		if (Size<=V14) {
			if (Size<=V13) {
				return 12;
			}
			return 13;
		}
		if (Size<=V15) {
			return 14;
		}
		return 15;
	}
	if (Size<=V18) {
		if (Size<=V17) {
			return 16;
		}
		return 17;
	}
	if (Size<=V20) {
		if (Size<=V19) {
			return 18;
		}
		return 19;
	}
	return (Word)-1;
}

/**********************************

	Return the size of a pointer

**********************************/

static INLINECALL Word32 GetPointerSize(MemModule_t *Root,void* ptr)
{
	if (ptr) {	//spec requires we handle delete of null pointers
		FastPtr_t *FastPtr;
		FastPtr = (FastPtr_t *)(((Word8 *)ptr)-16);
		if (FastPtr->Signature==FASTSIG) {		/* Is this valid? */
			return FastPtr->Size;
		}
		/* Call MacOS for this */
		return GetPtrSize((Ptr)FastPtr);
	}
	return 0;
}

/**********************************

	Move a memstatus pointer to the front of the list
	(Usually when I delete some memory)

**********************************/

static INLINECALL void MemStatusMoveToFront(MemStatus_t *MemStatPtr, MemStatus_t **SlotPtr)
{
	// !!!! assumes that prevptr != NULL !!!!
	MemStatus_t *Prev;
	MemStatus_t *Next;
	MemStatus_t *HeadPtr;
	
	Next = MemStatPtr->NextPtr;
	Prev = MemStatPtr->PrevPtr;
	HeadPtr = SlotPtr[0];
	if (Next) {			/* Is there a next link? */
		Next->PrevPtr = Prev;
	}
	Prev->NextPtr = Next;
	MemStatPtr->NextPtr = HeadPtr;
	HeadPtr->PrevPtr = MemStatPtr;
	SlotPtr[0] = MemStatPtr;
	MemStatPtr->PrevPtr = 0;		/* Front of the list */
}

/**********************************

	Init the memory pool

**********************************/

static INLINECALL void MemStatusInit(MemStatus_t *Input,MemModule_t *Root,Word SizeIndex,Word PoolSize)
{
	Word32 ChunkSize;		/* Size of each chunk */
	FastPtr_t *FreePtr;
	
	Input->NextPtr = 0;
	Input->PrevPtr = 0;
	Input->ChunksRemaining=PoolSize;
	Input->TotalChunks=PoolSize;
	
	FreePtr = (FastPtr_t *)(Input->PoolData);
	Input->FreePtr = FreePtr;
	Input->PoolSizeIndex = SizeIndex;			/* Save the index */
	
	ChunkSize=MemChunkCount[SizeIndex].ChunkSize;		/* Get the base allocation size */
	Input->RealChunkSize = ChunkSize;	/* Adjust for headers */
	
#ifdef NEW_MEM_DEBUG
	Root->PoolTotalBytes+=MemChunkCount[SizeIndex].ChunkAlloc;		/* Add the amount of memory allocated */
	++Root->TrueAllocCount;				/* Inc the total allocation count */
	++Root->PoolAllocCount[SizeIndex];	/* Inc the count for this size */
#endif

	do {
		FastPtr_t *Next;
		Next = (FastPtr_t *)((Word8 *)FreePtr+ChunkSize);
		FreePtr->Signature = FASTSIG;
		FreePtr->Size = 0;
		FreePtr->MemStatusPtr = Input;
		if (PoolSize==1) {
			Next = 0;
		}
		FreePtr->NextPtr = Next;
		FreePtr = Next;
	} while (--PoolSize);
}

/**********************************

	Allocate memory for a mem pool and add it to the list

**********************************/

static INLINECALL MemStatus_t * MemStatusNew(MemModule_t *Root,Word32 SizeIndex)
{
	MemStatus_t *MemPtr;
	MemStatus_t *PrevPoolPtr;
	Word32 AllocSize;
	Word32 PoolSize;
	
	PoolSize=MemChunkCount[SizeIndex].ChunkCount;		/* Number of chunks to make */
	AllocSize = MemChunkCount[SizeIndex].ChunkAlloc;	/* Memory to allocate */
	
	MemPtr=(MemStatus_t *)AllocAPointer(AllocSize);		/* Get the memory */
	if (MemPtr) {		/* Got memory?? */
		MemStatusInit(MemPtr,Root,SizeIndex,PoolSize);		/* Init the data */
	
		PrevPoolPtr = Root->PoolPtrs[SizeIndex];	/* Set the new previous pointer */
		MemPtr->PrevPtr = 0; 						/* Previous link */
		MemPtr->NextPtr = PrevPoolPtr;				/* Zap the next link */ 
		Root->PoolPtrs[SizeIndex] = MemPtr;			/* Save the new root pointer */
		if (PrevPoolPtr) {							/* Was there a link? */
			PrevPoolPtr->PrevPtr = MemPtr;			/* Set the new previous link */
		}
	}
	return MemPtr;								/* Return the new pool */
}

/**********************************

	Dispose of memory and unlink the referance

**********************************/

static void MemStatusDelete(MemStatus_t *MemStatPtr,MemModule_t *Root)
{
	Word i;
	/* dispose of the pool data */
	i = MemStatPtr->PoolSizeIndex;		/* Which is the root pool? */
#ifdef NEW_MEM_DEBUG
	Root->PoolTotalBytes-=MemChunkCount[i].ChunkAlloc; 	/* Remove the memory allocation */
	Root->TrueAllocCount--;			/* Remove count from total */
	Root->PoolAllocCount[i]--;			/* Remove count from size list */
#endif
	/* remove from the list */
	{
		MemStatus_t *NextPtr;
		MemStatus_t *PrevPtr;
		NextPtr = MemStatPtr->NextPtr;
		PrevPtr = MemStatPtr->PrevPtr;
		if (NextPtr) {
			NextPtr->PrevPtr = PrevPtr;
		}

		if (PrevPtr) {
			PrevPtr->NextPtr = NextPtr; 
		} else {	//no prev, so it's at the head, so reset Root
			Root->PoolPtrs[i]= NextPtr;			/* Clear out the index */
		}
	}
	/* dispose of the pool */
	DeallocAPointer(MemStatPtr);		/* Kill the memory */
}

/**********************************

	Allocate memory from a pool

**********************************/

static void* MemStatusAlloc(MemStatus_t *Input,MemModule_t *Root,Word SizeIndex,Word32 Size)
{
	FastPtr_t *FastPtr;
	
	FastPtr = Input->FreePtr;
	if (FastPtr) {
		Input->FreePtr = FastPtr->NextPtr;	/* Remove from the list */
		FastPtr->Size = Size;				/* Save the memory chunk size */
		{
			MemStatus_t *PrevPtr;
			PrevPtr = Input->PrevPtr;		/* Am I the first? */
			if (--Input->ChunksRemaining && PrevPtr) {	/* Make this the head? */
				MemStatus_t *NextPtr;
				
				/* Unlink me from the chain */
				
				NextPtr = Input->NextPtr;
				PrevPtr->NextPtr = NextPtr;	/* Fix the link */
				if (NextPtr) {
					NextPtr->PrevPtr = PrevPtr;
				}
				
				/* Insert me at the head */
				NextPtr = Root->PoolPtrs[SizeIndex];	/* Set the new previous pointer */
				Input->PrevPtr = 0; 					/* Previous link */
				Input->NextPtr = NextPtr;				/* Zap the next link */ 
				Root->PoolPtrs[SizeIndex] = Input;		/* Save the new root pointer */
				NextPtr->PrevPtr = Input;				/* Set the new previous link */
			}
		}
		return ((Word8 *)FastPtr+16);		/* Return pointer to data */
	}
	return 0;							/* Memory full? */
}

/**********************************

	Release memory to a pool

**********************************/

static INLINECALL void MemStatusDealloc(MemStatus_t *Input,MemModule_t *Root,FastPtr_t *FastPtr)
{
	FastPtr->NextPtr = Input->FreePtr;
	Input->FreePtr = FastPtr;
	Input->ChunksRemaining++;				/* Add one to the free list */
}

/**********************************

	Initialize the memory manager

**********************************/

static INLINECALL void MemModuleInit(MemModule_t *Root)
{
	FastMemSet(Root,0,sizeof(MemModule_t));
}

/**********************************

	Kill the memory manager

**********************************/

static INLINECALL void MemModuleDestroy(MemModule_t *Root)
{
	Word i;
	MemStatus_t **WorkPtr;
	
	// dispose of all pools
	i = NEW_MEMPOOL_SIZE_COUNT;
	WorkPtr = Root->PoolPtrs;
	do {
		MemStatus_t *TempPtr;
		TempPtr = WorkPtr[0];
		if (TempPtr) {			/* Anything here? */
			do {
				MemStatusDelete(TempPtr,Root);	/* Kill the pool */
				TempPtr = WorkPtr[0];
			} while (TempPtr);				/* More? */
		}
		++WorkPtr;
	} while (--i);
}

/**********************************

	Allocate memory from Burgerlib

**********************************/

void* AllocAPointer(Word32 size)
{
	Word SizeIndex;			/* Chunk size */
	MemStatus_t *MemPtr;
	void *out;
	MemModule_t *Root;
	
	if (size) {
		Root = &MyMem;
		if (size>V10) {
			SizeIndex = FindPoolSize(size);		/* Is this in a pool? */
			if (SizeIndex==(Word)-1) {
				goto Surrender;
			}
		} else {
			SizeIndex = PoolFast[size];
		}		
		/* Traverse the pools */
		
		do {
			MemPtr=Root->PoolPtrs[SizeIndex];			/* Get the root pointer */
			if (MemPtr) {
				do {
					/* try to alloc in this pool */
					if (MemPtr->ChunksRemaining) {		/* Any chunks free? */
						return MemStatusAlloc(MemPtr,Root,SizeIndex,size);		/* Allocate me! */
					}
					MemPtr = MemPtr->NextPtr;		/* Follow the pool */
				} while (MemPtr);
			}
				
			/* Hmm, no more memory, allocate a new pool */
			
			MemPtr=MemStatusNew(Root,SizeIndex);		/* Create the memory pool */
			if (MemPtr) {			/* Am I screwed? */
				return MemStatusAlloc(MemPtr,Root,SizeIndex,size);
			}
		} while (++SizeIndex<NEW_MEMPOOL_SIZE_COUNT);

Surrender:;
		/* Try a standard allocate */
		
		out=NewPtr(size+16);
#if !TARGET_API_MAC_CARBON
		if (!out) {
			out=NewPtrSys(size+16);
		}
#endif
		if (out) {
#ifdef NEW_MEM_DEBUG
			Root->DirectTotalBytes+=size;
#endif
			((FastPtr_t *)out)->Signature=0;		/* Bad signature */
			out = (Word8 *)out+16;
		}
		return out;			/* Return the memory */
	}
	return 0;				/* Error! */
}

/**********************************

	Release memory to burgerlib

**********************************/

void DeallocAPointer(void* ptr)
{
	MemStatus_t *MemPtr;
	MemModule_t *Root;
	
	if (ptr && !MemDying) {			/* Pointer ok? */
		FastPtr_t *FastPtr;
		FastPtr = (FastPtr_t *)(((Word8 *)ptr)-16);
		Root = &MyMem;
		if (FastPtr->Signature!=FASTSIG) {		/* Is this valid? */
		/* couldn't find it in a pool, so let's try to dispose of it */
#ifdef NEW_MEM_DEBUG
			Root->DirectTotalBytes-=GetPtrSize((Ptr)FastPtr);
#endif
			DisposePtr((Ptr)FastPtr);
		} else {
			MemPtr = FastPtr->MemStatusPtr;
			MemStatusDealloc(MemPtr,Root,FastPtr);
			/* if this was the only chunk in the pool, then kill the pool */
			if (MemPtr->ChunksRemaining>=MemPtr->TotalChunks) {
				MemStatusDelete(MemPtr,Root);		/* Release the memory */
			} else if (MemPtr->PrevPtr) {
				MemStatusMoveToFront(MemPtr,&Root->PoolPtrs[MemPtr->PoolSizeIndex]);
			}
		}
	}
}


/**********************************

	Using a pointer to memory, reallocate the size and copy
	the contents.
	If I request a zero length buffer, I just deallocate the input
	pointer, if the input pointer is null, I just allocate a fresh pointer.

**********************************/

void *ResizeAPointer(void *Mem,Word32 size)
{
	void *NewMem;

	if (!Mem) {
		return AllocAPointer(size);		/* Allocate new memory */
	}

	if (!size) {				/* No memory? */
		DeallocAPointer(Mem);	/* Release the memory */
		return 0;
	}

	NewMem=AllocAPointer(size);	/* Allocate the new memory */
	if (NewMem) {
		Word32 OldMem;
		OldMem = GetPointerSize(&MyMem,Mem);
		if (OldMem<size) {		/* Use the lesser of the two */
			size = OldMem;
		}
		FastMemCpy(NewMem,Mem,size);	/* Copy the contents */
	}
	DeallocAPointer(Mem);	/* Release the previous memory */
	return NewMem;			/* Return the new pointer */
}


void * BURGERCALL DebugAllocAPointer(Word32 Size,const char *Source,Word LineNum)
{
	void *Result;
	if (Size) {		/* No memory requested? */
		Result = AllocAPointer(Size);
		if (Result) {		/* Valid pointer? */
			DebugAddSourceLine((void **)Result,Source,LineNum,TRUE);
			return Result;
		}
		NonFatal("Out of memory, I need %ld bytes\n",Size);		/* Too bad! */
	}
	return 0;		/* No memory to get */
}

#if 0
void * operator new(Word32 size)
{	
	return AllocAPointer(size);
}

void operator delete(void *ptr)
{
	DeallocAPointer(ptr);
}
#endif

#if defined(__cplusplus) && 0
void* operator new[](Word size)
{
	return operator new(size);
}

void operator delete[](void* ptr)
{
	operator delete(ptr);
}
#endif

#endif


/**********************************

	Init the memory manager
	I use the Mac's memory manager so it's
	already initialized.

**********************************/

#if _DEBUG || __USEFASTALLOC__
static Word8 MemStarted;

static void DeInitMemory(void)
{
#if _DEBUG
	if (DebugTraceFlag&DEBUGTRACE_MEMORYLEAKS) {
		DebugXString("Memory leak dump\n");
		DumpHandles();
	}
#endif
#if __USEFASTALLOC__
	MemModuleDestroy(&MyMem);
	MemDying = TRUE;
#endif
}
#endif

void InitMemory(void)
{
#if __USEFASTALLOC__
	MemModuleInit(&MyMem);
#endif

#if _DEBUG || __USEFASTALLOC__
	if (!MemStarted) {
		MemStarted = TRUE;
		atexit(DeInitMemory);		/* Allow shutdown */
	}
#endif
}

/********************************

	Allocates a block of memory
	I allocate from the top down if fixed and bottom up if movable
	This routine handles all the magic for memory purging and
	allocation.

********************************/

void BURGERCALL **AllocAHandle2(Word32 MemSize,Word Flag)
{
	void **Result;
	if (!MemSize) {
		return 0;
	}
	Result = (void **)NewHandle(MemSize);		/* Get the memory (Unlocked and unpurgeable) */
	if (!Result) {
		OSErr err;
		Result = (void **)TempNewHandle(MemSize,&err);
		if (!Result) {
#if !TARGET_API_MAC_CARBON
			Result = (void **)NewHandleSys(MemSize);
			if (!Result)
#endif
			{
				NonFatal("Out of memory, I need %ld bytes\n",MemSize);	/* Too bad! */
				return 0;		/* Exit */
			}
		}
	}
//	if (Flag&HANDLEPURGEBITS) {			/* Enable purging */
//		HPurge(Result);			/* Enable the purge flag */
//	}
	if (Flag&(HANDLELOCK|HANDLEFIXED)) {		/* Lock the handle */
		HLock((Handle)Result);			/* Make it locked and fixed */
	}
	return Result;		/* Return the handle */
}

/**********************************

	Allocate a POINTER, I create a handle with 4
	bytes extra data and I place the handle at the beginning

**********************************/

#if !__USEFASTALLOC__
void * BURGERCALL AllocAPointer(Word32 Size)
{
	void *Result;
	if (Size) {		/* No memory requested? */
		Result = NewPtr(Size);
		if (Result) {		/* Valid pointer? */
			ReleaseMemoryData(Result,Size);
			return Result;
		}
		Result = NewPtrSys(Size);	/* Try again */
		if (Result) {
			ReleaseMemoryData(Result,Size);
			return Result;
		}
		NonFatal("Out of memory, I need %ld bytes\n",Size);		/* Too bad! */
	}
	return 0;		/* No memory to get */
}
#endif

/********************************

	Dispose of a memory handle into the free handle pool

********************************/

void BURGERCALL DeallocAHandle(void **MemHandle)
{
	if (MemHandle) {
		DisposeHandle((Handle)MemHandle);		/* Discard the handle */
	}
}

/**********************************

	Release a POINTER, I created a handle with 4
	bytes extra data and I place the handle at the beginning

**********************************/

#if !__USEFASTALLOC__
void BURGERCALL DeallocAPointer(void *MemPtr)
{
	if (MemPtr) {			/* Valid pointer? */
		DisposePtr(MemPtr);		/* Release the memory */
	}
}
#endif

/**********************************

	Using a pointer to memory, reallocate the size and copy
	the contents.
	If I request a zero length buffer, I just deallocate the input
	pointer, if the input pointer is null, I just allocate a fresh pointer.

**********************************/

#if !__USEFASTALLOC__
void *ResizeAPointer(void *Mem,Word32 size)
{
	void *NewMem;

	if (!Mem) {
		return AllocAPointer(size);		/* Allocate new memory */
	}

	if (!size) {		/* No memory? */
		DisposePtr(Mem);	/* Release the memory */
		return 0;
	}

	SetPtrSize(Mem, size);		/* Try to get the Mac to resize the pointer */
	if (!MemError()) {			/* Did it work? */
		return Mem;				/* Yes, so just return the current pointer */
	}
	NewMem=NewPtr(size);	/* Allocate the new memory */
	if (!NewMem) {
		NewMem = NewPtrSys(size);
		if (!NewMem) {
			goto Crap2;
		}
	}
	ReleaseMemoryData(NewMem,size);
	{
		Word32 OldMem;
		OldMem = GetPtrSize(Mem);
		if (OldMem<size) {		/* Use the lesser of the two */
			size = OldMem;
		}
		FastMemCpy(NewMem,Mem,size);	/* Copy the contents */
	}
Crap2:;
	DisposePtr(Mem);	/* Release the previous memory */
	return NewMem;			/* Return the new pointer */
}
#endif

/**********************************

	Using a handle to memory, reallocate the size and copy
	the contents. If the input handle is null, then just allocate a
	new handle, if the size requested is null then discard the input
	handle.

**********************************/

void **ResizeAHandle(void **Mem,Word32 NewSize)
{
	void **NewMem;
	Word32 OldSize;
	SInt8 prevState;

	if (!Mem) {		/* No previous handle? */
		if (NewSize) {
			return AllocAHandle2(NewSize,0);	/* Allocate the new memory */
		}
		return 0;
	}

	if (!NewSize) {		/* No memory requested? */
		DeallocAHandle(Mem);	/* Release the orignal */
		return 0;
	}

	OldSize = GetAHandleSize(Mem);
	if (NewSize==OldSize) {		/* No change? */
		return Mem;				/* Return the old handle */
	}
	prevState = HGetState((Handle)Mem);	/* Get current locked/purgeable state of handle */
	HSetState((Handle)Mem, prevState & 0x3F);	/* Unlock handle and make it unpurgeable */
	SetHandleSize((Handle)Mem,NewSize);	/* Try to get the Mac to resize the handle */
	if (!MemError()) {			/* Did it work? */
		HSetState((Handle)Mem, prevState);	/* Restore handle's previous state */
		return Mem;				/* Yes, so return */
	}
	NewMem=AllocAHandle2(NewSize,0);	/* Allocate the new memory */
	if (NewMem) {
		if (OldSize<NewSize) {			/* Did the handle grow? */
			NewSize = OldSize;			/* Only copy the old data */
		}
		FastMemCpy(*NewMem,*Mem,NewSize);	/* Copy the contents */
	}
	HSetState((Handle)NewMem,prevState);
	DeallocAHandle(Mem);		/* Release the previous memory */
	return NewMem;				/* Return the new pointer */
}

/********************************

	If the handle was purged, reallocate memory to it.
	Note, the returned handle will REPLACE the handle you passed me.
	This code effectively disposes of the previous handle and allocates
	a new one of the old one's size. If the data is still intact then
	nothing happens

********************************/

void ** BURGERCALL ReallocAHandle(void **MemHandle)
{
	NonFatal("ReallocAHandle Not support on Mac");
	return 0;
}

/**********************************

	Search the handle tree for a pointer, note that the pointer does
	NOT have to be the MemPtr, just in the domain of the handle
	Return 0 if the handle is not here.

**********************************/

void ** BURGERCALL FindAHandle(void *MemPtr)
{
	void **Result;
	Result = (void **)RecoverHandle((char *)MemPtr);
	if (!Result) {
		NonFatal("Can't find handle\n");
	}
	return Result;
}

/********************************

	Returns the size of a memory handle

********************************/

Word32 GetAHandleSize(void **MemHandle)
{
	if (MemHandle) {
		return GetHandleSize((Handle)MemHandle);		/* Return the size */
	}
	return 0;
}

/********************************

	Returns the total free space with purging
	This is accomplished by adding all the memory found in
	the free memory linked list and then adding all the memory
	in the used list that can be purged.

********************************/

Word32 GetTotalFreeMem(void)
{
	return FreeMem();		/* Return the possible free memory */
}

/********************************

	Set the lock flag to a given handle and return the data pointer

********************************/

void * BURGERCALL LockAHandle(void **TheHandle)
{
	if (TheHandle) {
		HLock((Handle)TheHandle);	/* Lock the handle down */
		return *TheHandle;
	}
	return 0;
}

/********************************

	Clear the lock flag to a given handle

********************************/

void BURGERCALL UnlockAHandle(void **TheHandle)
{
	if (TheHandle) {
		HUnlock((Handle)TheHandle);		/* Unlock the handle */
	}
}

/********************************

	Set the purge flag to a given handle

********************************/

void BURGERCALL SetHandlePurgeFlag(void **MemHandle,Word Flag)
{
	if (Flag) {			/* Enable purging */
		HPurge((Handle)MemHandle);
		return;
	}
	HNoPurge((Handle)MemHandle);		/* Disable purging */
}

/********************************

	Get the current purge and lock flags of the handle

********************************/

Word BURGERCALL GetAHandleLockedState(void **MemHandle)
{
	Word Result;
	Result = HGetState((Handle)MemHandle);	/* Get current locked/purgeable state of handle */
	return Result;
}

/********************************

	Set the current purge and lock flags of the handle

********************************/

void BURGERCALL SetAHandleLockedState(void **MemHandle,Word State)
{
	HSetState((Handle)MemHandle,State);	/* Get current locked/purgeable state of handle */
}

/********************************

	This routine will move a handle from the used list
	into the purged handle list. The handle is not discarded.
	This is the only way a handle can be placed into the purged list.

	I will call ReleaseMemRange() to alert the free memory
	list that I have free memory.

********************************/

void PurgeAHandle(void **MemHandle)
{
	if (MemPurgeCallBack) {
		MemPurgeCallBack(MMStagePurge);
	}
	HUnlock((Handle)MemHandle);			/* Make sure it's unlocked */
	EmptyHandle((Handle)MemHandle);		/* Purge the contents */
}

/********************************

	Purges all handles that are purgable and are
	greater or equal to the level requested.
	I will call ReleaseMemRange() to alert the free memory
	list that I have free memory.

********************************/

Word BURGERCALL PurgeHandles(Word32 MemSize)
{
	if (MemPurgeCallBack) {
		MemPurgeCallBack(MMStagePurge);
	}
	PurgeMem(MemSize);		/* Force a full purge */
	if (MemError()==memFullErr) {
		return FALSE;
	}
	return TRUE;
}

/********************************

	Packs all memory together.
	This doesn't alter the handle list in any way but it can move
	memory around to get rid of empty holes in the memory map.

********************************/

void CompactHandles(void)
{
	if (MemPurgeCallBack) {
		MemPurgeCallBack(MMStageCompact);
	}
	CompactMem(16000000);		/* Force a full compaction */
}

/**********************************

	Allocate a POINTER, I create a handle with 4
	bytes extra data and I place the handle at the beginning

**********************************/

#if !__USEFASTALLOC__
void * BURGERCALL DebugAllocAPointer(Word32 Size,const char *Source,Word LineNum)
{
	void *Result;
	if (Size) {		/* No memory requested? */
		Result = NewPtr(Size);
		if (Result) {		/* Valid pointer? */
			ReleaseMemoryData(Result,Size);
			DebugAddSourceLine((void **)Result,Source,LineNum,TRUE);
			return Result;
		}
		Result = NewPtrSys(Size);
		if (Result) {
			ReleaseMemoryData(Result,Size);
			DebugAddSourceLine((void **)Result,Source,LineNum,TRUE);
			return Result;
		}
		NonFatal("Out of memory, I need %ld bytes\n",Size);		/* Too bad! */
	}
	return 0;		/* No memory to get */
}
#endif


/**********************************

	Returns the total allocated memory used by pointers and handles in bytes.
	
**********************************/

Word32 BURGERCALL GetTotalAllocatedMem(void)
{
	return AllocatedMemSize;
}


#endif