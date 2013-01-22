/********************************

	Burgerlib Memory manager

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
#include "PfPrefs.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(__MAC__)
#include <MacMemory.h>
#endif

/********************************

	If burgerlib is a directly linked library, 
	then I only keep the pointer to the filenames.
	If it is a SHARED library, I need to make a COPY of the
	filename since libraries can be loaded and unloaded
	at will and if a library is unloaded and THEN the
	memory is released, I won't cause a memory
	exception error if I print out the filename
	since the filename is a local copy instead of in
	memory of a purged library. Yes, it's wasteful, but
	there was no other real solution that didn't cause massive
	long term problems.
	
	Thankfully, the debug memory manager is only for debug builds
	and not release builds. So the memory waste is under control.
	
********************************/

#if _SHAREDLIB
#define REFNAMEPTRS 0
#else
#define REFNAMEPTRS 1
#endif

/* This structure is used to debug the memory allocations. */

#define HANDLELINEFLAG 0x80000000UL

typedef struct DebugMe_t {
	void **HandleRef;		/* Handle to reference */
	Word32 NewLineNum;	/* Line in the source (0x80000000 = handle) */
#if REFNAMEPTRS
	const char *Source;		/* Pointer to the name */
#else
	char Source[56];	/* Source line */
#endif
} DebugMe_t;

static Word DebugRecurse;	/* Atomic counter to prevent recursion for DebugAddSourceLine() */
static Word DebugCount;		/* Number of debug entries active */
static Word DebugMaxCount;	/* Size of the debug entries handle */
static DebugMe_t **DebugMeHand;	/* Handle to the debug entries */
static Word32 AllocatedMemSize;	/* Total amount of memory allocated */

#if defined(__MAC__)
Word32 MaxMemSize;				/* Maximum memory the program will take (4M) */
Word32 MinReserveSize=0x40000;	/* Minimum memory for OS (256K) */
#else
Word32 MaxMemSize = 0x400000;		/* Maximum memory the program will take (4M) */
Word32 MinReserveSize=0x10000;	/* Minimum memory for OS (64K) */
#endif
Word MinHandles = 500;				/* Number of handles to create (500) */
MemPurgeProcPtr MemPurgeCallBack;	/* Callback before memory purge */

/* If this is TRUE, then I use the native mac meory system */

#if !__MACUSENATIVEMEM__

#ifdef __cplusplus
extern "C" {
static void ANSICALL DeInitMemory(void);
}
#endif

MyHandle UsedHand1 = {		/* Pointer to the used handle list */
	0,0,(0xFFFEUL<<16)|HANDLELOCK|HANDLEFIXED,&UsedHand2,&UsedHand2};
MyHandle UsedHand2 = {
	0,0,(0xFFFEUL<<16)|HANDLELOCK|HANDLEFIXED,&UsedHand1,&UsedHand1};
MyHandle *FreeHands;		/* Pointer to the free handle list */
MyHandle PurgeHands = {		/* Phony master handle to purge list */
	0,0,(0xFFFEUL<<16)|0,&PurgeHands,&PurgeHands};
MyHandle FreeMemHands = {
	0,0,(0xFFFEUL<<16)|0,&FreeMemHands,&FreeMemHands};
MyHandle PurgeHandleFiFo = {	/* Purged handle linked list */
	0,0,(0xFFFEUL<<16)|0,&PurgeHandleFiFo,&PurgeHandleFiFo,&PurgeHandleFiFo,&PurgeHandleFiFo};
Word8 *MemoryBlockList;		/* Extra handle list */

/********************************

	Allocate and release the superblock using direct
	Win95 memory calls

********************************/

#if defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static void *FooAlloc(Word32 Size)
{
	void *Result;
	Result = GlobalAlloc(GMEM_NOT_BANKED,Size);		/* Get memory from Windows */
	if (Result) {
		VirtualLock(Result,Size);					/* Prevent swapping */
	}
	return Result;									/* Return the memory */
}

static void FooFree(void *Mem)
{
	if (Mem) {										/* Valid pointer? */
		Word32 Size;
		Size = GlobalSize(Mem);						/* Get the size of the memory chunk */
		VirtualUnlock(Mem,Size);					/* Allow VM to control this memory */
		GlobalFree(Mem);							/* Release it to windows */
	}
}

#define malloc FooAlloc
#define free FooFree

/********************************

	Allocate and release the superblock using direct
	MacOS memory calls

********************************/

#elif defined(__MAC__)

static INLINECALL void FooFree(void *Mem)
{
	if (Mem) {						/* Check if ok */
		DisposePtr(Mem);			/* Release the memory */
	}
}
#define malloc(x) (void*)NewPtr(x)
#define free FooFree
#endif

/**********************************

	Shut down my memory manager by releasing the superblock(s)

**********************************/

static void ANSICALL DeInitMemory(void)
{
	Word8 *TempPtr;

	if (UsedHand1.NextHandle==&UsedHand2) {
		TempPtr = MemoryBlockList;			/* Dispose of all the memory */
		if (TempPtr) {
			do {
				Word8 *Next;
				Next = ((Word8 **)TempPtr)[0];
				free(TempPtr);
				TempPtr = Next;
			} while (TempPtr);
			MemoryBlockList = 0;
		}
	}
#if _DEBUG
	else {
		if (DebugTraceFlag&DEBUGTRACE_MEMORYLEAKS) {
			DebugXString("Memory leak dump\n");
			DumpHandles();
		}
	}
#endif
}

/**********************************

	Create more handle records
	I assume FreeHands is NULL

**********************************/

static MyHandle * MakeNewHandles(void)
{
	MyHandle *New;
	MyHandle *FreeH;
	Word i;

	New = (MyHandle *)malloc((500*sizeof(MyHandle))+sizeof(Word8 *));	/* Get memory from system */
	if (New) {
		((Word8 **)New)[0]=MemoryBlockList;		/* Get the root pointer */
		MemoryBlockList = (Word8 *)New;			/* Store the new master pointer */
		New = (MyHandle *)(((Word8 *)New)+sizeof(Word8 *));	/* Adjust past the pointer */
		MinHandles = MinHandles+500;			/* Alert system to the new handle count */
		i = 500-1;								/* Add handles to the list */
		FreeH = 0;
		New = New+(500-1);						/* Index to the last one */
		do {
			New->Flags = (0xABCDUL<<16);		/* Bad ID */
			New->NextHandle = FreeH;			/* Link in the list */
			FreeH = New;						/* New parent */
			--New;								/* Next handle */
		} while (--i);							/* All done? */
		FreeHands = FreeH;						/* Save the new free handle list */
		return New;								/* Return the free handle */
	}
	Fatal("Out of malloc memory for Handles!\n");	/* Error!! */
	return 0;
}

/**********************************

	Returns the total allocated memory used by pointers and handles in bytes.
	
**********************************/

Word32 BURGERCALL GetTotalAllocatedMem(void)
{
	return AllocatedMemSize;
}

/**********************************

	Add a range of memory to the free memory list.
	Note, If this memory "Touches" another free memory entry. I
	merge them together.

**********************************/

static void ReleaseMemRange(void *MemPtr,Word32 Length,MyHandle *Parent)
{
	MyHandle *Scan;
	MyHandle *Prev;

	Scan = &FreeMemHands;
	Length = (Length+(MEMORYPADDING-1))&(~(MEMORYPADDING-1));			/* Round to nearest longword */
	Prev = Scan->PrevHandle;
	if (Prev!=Scan) {			/* No handles in the list? */
		/* I will now scan free memory until I find the free memory */
		/* handle after the memory to be freed */

		do {
			if ((Word8 *)MemPtr>=(Word8 *)Prev->MemPtr) {	/* After free memory? */
				break;		/* Scan = handle */
			}
			Prev = Prev->PrevHandle;
		} while (Prev!=Scan);

		/* Scan has the free memory handle AFTER the memory */

		Scan = Prev->NextHandle;	/* Get the previous handle */

		/* See if this free memory is just an extension of the previous */

		{
			Word8 *EndPtr;
			EndPtr = (Word8 *)Prev->MemPtr;		/* Get the free memory */
			EndPtr = EndPtr+Prev->Length;		/* Get the end of the block */

			if (EndPtr==(Word8 *)MemPtr) {		/* End matches? */
				Prev->Flags = (Word32) Parent;	/* Set the new parent handle */
				Prev->Length = Prev->Length+Length;	/* Set the new free length */
				if (Scan!=&FreeMemHands) {			/* Last handle? */
					EndPtr = EndPtr+Length;			/* Recalc the new end pointer */
					if (EndPtr==(Word8 *)Scan->MemPtr) {		/* Filled in two free mems? */
						Prev->Length = Prev->Length+Scan->Length;	/* Add it */
						Prev->NextHandle = Scan->NextHandle;	/* Remove second handle */
						Scan->NextHandle->PrevHandle = Prev;
						Scan->Flags = (0xABCDUL<<16);	/* Bad ID */
						Scan->NextHandle = FreeHands;	/* Link in the list */
						FreeHands = Scan;				/* New parent */
					}
				}
				return;
			}

		/* Check If I should merge with the next fragment */

			EndPtr = (Word8 *)MemPtr;		/* Get the current memory fragment */
			EndPtr = EndPtr+Length;
			if (EndPtr==(Word8 *)Scan->MemPtr) {	/* Touch next handle */
				Scan->Flags = (Word32) Parent;	/* New parent */
				Scan->Length = Scan->Length+Length;	/* New length */
				Scan->MemPtr = MemPtr;		/* New start pointer */
				return;
			}
		}

		/* It is not mergable... I need to create a handle */

		{
			MyHandle *NewHand;
			NewHand = FreeHands;		/* Get a new handle */
			if (!NewHand) {
				NewHand = MakeNewHandles();
			} else {
				FreeHands = NewHand->NextHandle;	/* Follow the chain */
			}
			NewHand->MemPtr = MemPtr;			/* New pointer */
			NewHand->Length = Length;			/* Free mem length */
			NewHand->Flags = (Word32)Parent;	/* Set the new parent */
			NewHand->NextHandle = Scan;			/* Forward handle */
			NewHand->PrevHandle = Prev;			/* Previous handle */
			NewHand->NextPurge = 0;
			NewHand->PrevPurge = 0;
			Prev->NextHandle = NewHand;			/* Link me in */
			Scan->PrevHandle = NewHand;
		}
		return;
	}

	/* There is no free memory, create the singular entry */

	Prev = FreeHands;		/* Get a new handle */
	if (!Prev) {
		Prev = MakeNewHandles();
	} else {
		FreeHands = Prev->NextHandle;	/* Follow the chain */
	}
	Prev->MemPtr = MemPtr;			/* Create free entry */
	Prev->Length = Length;
	Prev->Flags = (Word32)Parent;	/* Mark the parent handle */
	Prev->NextHandle = Scan;
	Prev->PrevHandle = Scan;
	Prev->NextPurge = 0;
	Prev->PrevPurge = 0;
	Scan->NextHandle = Prev;
	Scan->PrevHandle = Prev;
}

/**********************************

	Remove a range of memory from the free memory pool
	I assume that the memory range is either attached to the
	end of a free memory segment or the end of a free memory
	segment.

	If not, bad things will happen!

**********************************/

static INLINECALL void GrabMemRange(void *MemPtr,Word32 Length,MyHandle *Parent,MyHandle *Scan)
{
	Length = (Length+(MEMORYPADDING-1))&(~(MEMORYPADDING-1));
	if (!Scan) {
		Scan = FreeMemHands.NextHandle;

	/* I will now scan free memory until I find the free memory */
	/* handle after the memory to be freed */

		do {
			if ((Word8 *)MemPtr>=(Word8 *)Scan->MemPtr) {	/* After free memory? */
	 			Word8 *EndPtr;
				EndPtr = (Word8 *)Scan->MemPtr;
				EndPtr = EndPtr + Scan->Length;
				if ((Word8 *)MemPtr<EndPtr) {
					goto FoundIt;
				}
			}
			Scan = Scan->NextHandle;
		} while (Scan!=&FreeMemHands);
		Fatal("Requested memory range to free is not in the free list\n");
	}
	/* I found a possible memory are to allocate from */

FoundIt:;
	Scan->Flags = (Word32)Parent;		/* Let's mark the parent entry */
	if (Scan->MemPtr == MemPtr) {		/* From the end of the data? */
		if (Scan->Length==Length) {		/* Full match... */
			MyHandle *Prev;
			MyHandle *Next;
			Prev = Scan->PrevHandle;	/* Unlink the free memory chunk */
			Next = Scan->NextHandle;
			Next->PrevHandle = Prev;
			Prev->NextHandle = Next;
			Scan->Flags = (0xABCDUL<<16);	/* Bad ID */
			Scan->NextHandle = FreeHands;	/* Link in the list */
			FreeHands = Scan;			/* New parent */
			return;				/* Exit ok */
		}
		Scan->Length = Scan->Length-Length;		/* Calc new length */
		Scan->MemPtr = ((Word8 *)MemPtr)+Length;	/* Calc new beginning */
		return;
	}

	/* Memory is from end to the beginning */

	Scan->Flags = (Word32)Parent;
	Scan->Length = (Word8 *)MemPtr-(Word8 *)Scan->MemPtr;	/* New length */
}

/**********************************

	Initialize the burgerlib memory manager

**********************************/

void BURGERCALL InitMemory(void)
{
	Word32 Size;		/* Temp size */
	Word32 Max;		/* Size of the ENTIRE buffer */
	Word32 Swing;		/* Swing index */
	MyHandle *Scan;		/* Running Handle ptr */
	Word i;
	
	AllocatedMemSize = 0;		/* No memory allocated yet */
	
	if (MemoryBlockList) {		/* Already initialized? */
		NonFatal("Memory Manager already started!\n");
		return;
	}

	Scan = (MyHandle *)malloc(MinReserveSize);	/* Enough for the OS? */
	if (!Scan) {				/* No good! */
		Fatal("Can't allocate minimum OS memory chunk\n");
	}

	Max = MaxMemSize;			/* Assume maximum allowed */
#if defined(__MAC__)
	if (!Max) {
		Max = MaxBlock();
		MaxMemSize = Max;
	}
#endif

	MemoryBlockList = (Word8 *)malloc(Max); /* Get the maximum? */
	if (!MemoryBlockList) {		/* Got the memory? */
		Swing = MaxMemSize/2;		/* Start with this motion */
		Max = 0;					/* No memory found yet */
		for (;;) {
			Size = Max+Swing;		/* Attempt this size */
			MemoryBlockList = (Word8 *)malloc(Size);	/* Try to allocate it */
			if (MemoryBlockList) {
				free(MemoryBlockList);
				Max = Size;			/* This is acceptable! */
			} else {
				MaxMemSize = Size;	/* Can't get bigger than this! */
			}
			Swing = (MaxMemSize-Max)/2;	/* Get half the differance */
			if (Swing<1024) {		/* Close enough? */
				MemoryBlockList = (Word8 *)malloc(Max);
				break;
			}
		}
	}
	free(Scan);			/* Release OS memory */

	if (!MemoryBlockList) {		/* Somehow I got an error! */
		Fatal("Can't allocate super chunk\n");	/* Die! */
	}
	atexit(DeInitMemory);		/* Allow shutdown */

	if (Max<0x10000) {			/* Not enough memory for any good */
		Fatal("Super chunk is less than 64K bytes\n");
	}

	((Word32 *)MemoryBlockList)[0] = 0;	/* No next link */
	Scan = (MyHandle *)(MemoryBlockList+4);	/* Init the free handle list */
	Max -=4;
	i = ((Word)Scan)&(MEMORYPADDING-1);		/* Get the start pad value */
	if (i) {		/* Not long word aligned!?! */
		Max = Max-i;		/* Remove the pad adjustment */
		i = MEMORYPADDING-i;	/* Make padding value */
		Scan = (MyHandle *)(((char *)Scan)+i);
	}

	Max = Max & (~(MEMORYPADDING-1));	/* Truncate the size */
	MaxMemSize = Max;	/* Save the adjusted heap size */

	Size = MinHandles*sizeof(MyHandle);	/* Memory needed for handles */
	if (Size>=Max) {	/* You've got to be kidding me!! */
		Fatal("Can't allocate default handle array\n");	/* Die! */
	}


/* Link all the handles in the free handle list */

	{
		MyHandle *FreeH;
		FreeH = 0;		/* Index to the first free handle */
		i = MinHandles;
		Scan = Scan+(i-1);
		do {
			Scan->Flags = (0xABCDUL<<16);	/* Bad ID */
			Scan->NextHandle = FreeH;		/* Link in the list */
			FreeH = Scan;					/* New parent */
			--Scan;
		} while (--i);
		FreeHands = FreeH;
		Scan = Scan+(MinHandles+1);		/* Point to the last entry */
	}
	Max = Max-Size;		/* Reduce the free memory */

/* Create the initial footprint for the handles */

	UsedHand1.MemPtr = 0;		/* First memory byte */
	UsedHand1.Length = (Word32)Scan;		/* Length to first chunk of memory */
	UsedHand1.NextHandle = &UsedHand2;		/* Init the links */
	UsedHand1.PrevHandle = &UsedHand2;

	UsedHand2.MemPtr = ((char *)Scan)+Max;	/* Last byte to use */
	UsedHand2.Length = 0xFFFFFFFF - (Word32)UsedHand2.MemPtr; /* Length to top */
	UsedHand2.NextHandle = &UsedHand1;
	UsedHand2.PrevHandle = &UsedHand1;

	PurgeHands.NextHandle = &PurgeHands;		/* No handles are purged */
	PurgeHands.PrevHandle = &PurgeHands;
	FreeMemHands.NextHandle = &FreeMemHands;	/* No handles are free */
	FreeMemHands.PrevHandle = &FreeMemHands;
	PurgeHandleFiFo.NextHandle = &PurgeHandleFiFo;	/* No handles are free */
	PurgeHandleFiFo.PrevHandle = &PurgeHandleFiFo;
	PurgeHandleFiFo.NextPurge = &PurgeHandleFiFo;	/* No handles are to be purged */
	PurgeHandleFiFo.PrevPurge = &PurgeHandleFiFo;

	ReleaseMemRange(Scan,Max,&UsedHand1);		/* Create the default free list */

	/* If this is the debug version, I will allocate the debug tracking */
	/* buffer first to prevent fragmentation caused by the debug manager */

#if _DEBUG
	DebugMeHand = (DebugMe_t **)AllocAHandle2(sizeof(DebugMe_t)*MinHandles,0);
	if (DebugMeHand) {
		DebugMaxCount = MinHandles;
	}
#endif
}

/********************************

	Allocates a block of memory
	I allocate from the top down if fixed and bottom up if movable
	This routine handles all the magic for memory purging and
	allocation.

********************************/

void ** BURGERCALL AllocAHandle2(Word32 MemSize,Word Flag)
{
	MyHandle *Scan;			/* Current handle checking */
	MyHandle *NewHandlePtr;	/* New handle to create */
	Word Stage;				/* Search stage */

	if (MemSize) {		/* Don't allocate an empty handle! */
		/* This is for the case that someone allocated memory */
		/* without first initializing the memory manager */

		if (MemoryBlockList) {				/* Not initialized? */
			NewHandlePtr = FreeHands;		/* Get a new handle */
			if (!NewHandlePtr) {
				NewHandlePtr = MakeNewHandles();	/* Get a handle */
				if (!NewHandlePtr) {
					return 0;
				}
			} else {
				FreeHands = NewHandlePtr->NextHandle;	/* Follow the chain */
			}
			NewHandlePtr->NextPurge = 0;
			NewHandlePtr->PrevPurge = 0;
			NewHandlePtr->Length = MemSize;		/* Save the handle size */
			NewHandlePtr->Flags = Flag&(~HANDLEMALLOC);		/* Save the default attributes */
			Stage = 0;			/* Init data memory search stage */
			MemSize = (MemSize+(MEMORYPADDING-1)) & (~(MEMORYPADDING-1));		/* Round up */

			if (Flag&HANDLEFIXED) {

			/* Scan from the top down for fixed handles */
			/* Increases odds for compaction success */

				for (;;) {
					Scan = FreeMemHands.PrevHandle;

				/* Find the memory, LastScan has the handle the memory */
				/* will occupy BEFORE, Scan is the prev handle before the new one */

					if (Scan != &FreeMemHands) {		/* No free memory? */
						do {
							if (Scan->Length>=MemSize) {
								MyHandle *Prev;
								MyHandle *Next;

								Prev = (MyHandle *)Scan->Flags;	/* Get the parent handle */
								Next = Prev->NextHandle;		/* Next handle */

								NewHandlePtr->PrevHandle = Prev;
								NewHandlePtr->NextHandle = Next;
								Prev->NextHandle = NewHandlePtr;
								Next->PrevHandle = NewHandlePtr;

								NewHandlePtr->MemPtr = (void **)(((Word8 *)Scan->MemPtr)+(Scan->Length-MemSize));
								GrabMemRange(NewHandlePtr->MemPtr,MemSize,Prev,Scan);
								
								/* Update the global allocated memory count. */
								AllocatedMemSize += NewHandlePtr->Length;
			
								return (void **)NewHandlePtr;		/* Good allocation! */
							}
							Scan=Scan->PrevHandle;			/* Look at next handle */
						} while (Scan != &FreeMemHands);	/* End of the list? */
					}
					if (Stage&1) {
						if (!PurgeHandles(MemSize)) {			/* Purge all handles */
							break;
						}
					} else {
						CompactHandles();		/* Pack memory together */
					}
					++Stage;
				}

			} else {

				/* Scan from the bottom up for movable handles */
				/* Increases odds for compaction success */

				for (;;) {
					Scan = FreeMemHands.NextHandle;	/* Get next index */

				/* Find the memory, LastScan has the handle the memory */
				/* will occupy AFTER, Scan is the next handle after the new one */

					if (Scan != &FreeMemHands) {		/* Already stop? */
						do {
							if (Scan->Length>=MemSize) {
								MyHandle *Prev;
								MyHandle *Next;

								Prev = (MyHandle *)Scan->Flags;	/* Get the parent handle */
								Next = Prev->NextHandle;

								NewHandlePtr->PrevHandle = Prev;
								NewHandlePtr->NextHandle = Next;
								Prev->NextHandle = NewHandlePtr;
								Next->PrevHandle = NewHandlePtr;

								NewHandlePtr->MemPtr = Scan->MemPtr;
								GrabMemRange(Scan->MemPtr,MemSize,NewHandlePtr,Scan);
								
								/* Update the global allocated memory count. */
								AllocatedMemSize += NewHandlePtr->Length;
								
								return (void **)NewHandlePtr;		/* Good allocation! */
							}
							Scan=Scan->NextHandle;			/* Look at next handle */
						} while (Scan != &FreeMemHands);	/* End of the list? */
					}
		//			if (Stage&1) {
						if (!PurgeHandles(MemSize)) {			/* Purge all handles */
							if (Stage) {
								break;
							}
							CompactHandles();		/* Pack memory together */
							Stage = 1;
						}
		//			} else {
		//				CompactHandles();		/* Pack memory together */
		//			}
		//			++Stage;
				}
			}

	/* I failed in my quest for memory, exit as a miserable loser */

			NewHandlePtr->Flags = (0xABCDUL<<16);	/* Bad ID */
			NewHandlePtr->NextHandle = FreeHands;	/* Link in the list */
			FreeHands = NewHandlePtr;			/* New parent */
			if (BombFlag) {
				DumpHandles();					/* Print a memory dump */
			}
AllocFailure:;
			NonFatal("Out of memory, I need %ld bytes\n",MemSize);		/* Too bad! */
			return 0;		/* Exit */
		}
		
		/* I'm going to try to get memory from somewhere else. */
		/* This is a last resort! */
		
		NewHandlePtr = (MyHandle *)malloc(MemSize+sizeof(MyHandle));
		if (!NewHandlePtr) {		/* Error? */
			goto AllocFailure;
		}
		
		/* Update the global allocated memory count. */
		AllocatedMemSize += MemSize;
		
		NewHandlePtr->Length = MemSize;
		NewHandlePtr->Flags = Flag|HANDLEMALLOC;	/* It was Malloc'd */
		NewHandlePtr->PrevHandle = 0;	/* Force crash */
		NewHandlePtr->NextHandle = 0;
		NewHandlePtr->NextPurge = 0;
		NewHandlePtr->PrevPurge = 0;
		NewHandlePtr->MemPtr = NewHandlePtr+1;	/* Point to true memory */
		return (void **)NewHandlePtr;		/* Return the handle */
	}
	return 0;		/* Return nil */
}



/**********************************

	Allocate a POINTER, I create a handle with 4
	bytes extra data and I place the handle at the beginning

**********************************/

void * BURGERCALL AllocAPointer(Word32 Size)
{
	Word32 *ThePointer;
	void **TheHandle;

	if (Size) {
		TheHandle = AllocAHandle2(Size+MEMORYPADDING,HANDLEFIXED);		/* Get the memory */
		if (TheHandle) {			/* Valid? */
			ThePointer = (Word32 *)TheHandle[0];	/* Deref the memory! */
			ThePointer[0] = (Word32)TheHandle;	/* Save the handle in memory */
	 		return (void *) (((Word8 *)ThePointer)+MEMORYPADDING);		/* Return the memory pointer */
		}
	}
	return 0;			/* Can't do it!! */
}

/********************************

	Dispose of a memory handle into the free handle pool

********************************/

void BURGERCALL DeallocAHandle(void **MemHandle)
{
	if (MemHandle) {		/* Valid pointer? */
	
		/* Subtract from global size. */
		AllocatedMemSize -= ((MyHandle *)MemHandle)->Length;
	
		if (!(((MyHandle *)MemHandle)->Flags&HANDLEMALLOC)) {
			if (MemoryBlockList) {		/* Not initialized? */
				MyHandle *Next;
				MyHandle *Prev;

				Next = ((MyHandle *)MemHandle)->NextPurge;	/* Get the links */
				if (Next) {
					Prev = ((MyHandle *)MemHandle)->PrevPurge;
					Prev->NextPurge = Next;	/* Set the previous link */
					Next->PrevPurge = Prev;	/* Link the next handle */
				}
				Next = ((MyHandle *)MemHandle)->NextHandle;	/* Get the links */
				Prev = ((MyHandle *)MemHandle)->PrevHandle;

				Prev->NextHandle = Next;	/* Set the previous link */
				Next->PrevHandle = Prev;	/* Link the next handle */

				Next = (MyHandle *)((MyHandle *)MemHandle)->MemPtr;	/* Get pointer */
				if (Next) {		/* Was this handle holding memory? */
					ReleaseMemRange(Next,((MyHandle*)MemHandle)->Length,Prev);
				}
				((MyHandle *)MemHandle)->Flags = (0xABCDUL<<16);	/* Bad ID */
				((MyHandle *)MemHandle)->NextHandle = FreeHands;	/* Link in the list */
				FreeHands = ((MyHandle *)MemHandle);			/* New parent */
			}
			return;
		}
		free(MemHandle);		/* Just release the memory */
	}
}

/**********************************

	Release a POINTER, I created a handle with 4
	bytes extra data and I place the handle at the beginning

**********************************/

void BURGERCALL DeallocAPointer(void *MemPtr)
{
	if (MemPtr) {			/* Null pointer?!? */
		DeallocAHandle(((void ***)((Word8 *)MemPtr-MEMORYPADDING))[0]);
	}
}

/**********************************

	Using a pointer to memory, reallocate the size and copy
	the contents.
	If I request a zero length buffer, I just deallocate the input
	pointer, if the input pointer is null, I just allocate a fresh pointer.

**********************************/

void * BURGERCALL ResizeAPointer(void *Mem,Word32 size)
{
	if (!Mem) {			/* No input pointer? */
		if (size) {		/* Do I want any memory? */
			return AllocAPointer(size);	/* Just get fresh memory */
		}
		return 0;		/* Return a null pointer */
	}

	if (!size) {			/* No memory requested? */
		DeallocAPointer(Mem);	/* Release the memory */
		return 0;			/* Return a null pointer */
	}

	/* Convert the pointer back into a handle and perform the operation */

	Mem = ResizeAHandle(((void ***)((Word8 *)Mem-MEMORYPADDING))[0],size+MEMORYPADDING);	/* Resize the handle */
	if (Mem) {			/* Successful? */
		void *ThePointer;		/* Temp pointer */

		ThePointer = ((void **)Mem)[0];	/* Deref the memory! */
		((void **)ThePointer)[0] = Mem;	/* Save the handle in memory */
 		return (((Word8 *)ThePointer)+MEMORYPADDING);		/* Return the memory pointer */

	}
	return 0;		/* Oh oh... */
}

/**********************************

	Using a handle to memory, reallocate the size and copy
	the contents. If the input handle is null, then just allocate a
	new handle, if the size requested is null then discard the input
	handle.

**********************************/

void ** BURGERCALL ResizeAHandle(void **Mem,Word32 NewSize)
{
	MyHandle *NewMem;
	Word32 OldSize;

	if (!Mem) {		/* No previous handle? */
		if (NewSize) {		/* New memory requested? */
			return AllocAHandle2(NewSize,0);		/* Allocate the new memory */
		}
		return 0;
	}

	if (!NewSize) {		/* No memory requested? */
		DeallocAHandle(Mem);	/* Release the original */
		return 0;		/* Exit with nothing */
	}

	/* Try the easy way, just shrink the handle... */

	OldSize = ((MyHandle *)Mem)->Length;	/* Get length */
	if (NewSize==OldSize) {
		return Mem;		/* Return the handle intact */
	}

	if (NewSize<OldSize &&
		(!(((MyHandle *)Mem)->Flags & HANDLEMALLOC))) {		/* Handle will shrink?? */
		((MyHandle *)Mem)->Length = NewSize;		/* Set the new size */
		NewSize = (NewSize+(MEMORYPADDING-1))&(~(MEMORYPADDING-1));		/* Long word align */
		OldSize = (OldSize+(MEMORYPADDING-1))&(~(MEMORYPADDING-1));
		OldSize = OldSize-NewSize;		/* How many bytes to release? */
		if (OldSize) {			/* Will I release any memory? */
			Word8 *Start;
			Start = (Word8 *)((MyHandle *)Mem)->MemPtr;	/* Get start pointer */
			Start = Start+NewSize;
			ReleaseMemRange(Start,OldSize,(MyHandle *)Mem);
		}
		return Mem;		/* Return the smaller handle */
	}

	/* Handle is growing... */

	/* I have to do it the hard way!! */

	NewMem = (MyHandle *)AllocAHandle2(NewSize,((MyHandle *)Mem)->Flags);	/* Allocate the new memory */
	if (NewMem) {		/* Success! */
		if (NewSize<OldSize) {		/* Make sure I only copy the SMALLER of the two */
			OldSize = NewSize;		/* New size */
		}
		FastMemCpy(NewMem->MemPtr,*Mem,OldSize);	/* Copy the contents */
	}
	DeallocAHandle(Mem);	/* Release the previous memory */
	
	/* Both handles must be under BURGERLIB control. Very rare this is not true */
	
	if (NewMem) {			/* Did I get memory? */
		if (!(((MyHandle *)Mem)->Flags&HANDLEMALLOC) && !(NewMem->Flags&HANDLEMALLOC)) {			/* If this is BURGERLIB, return the SAME handle */
			MyHandle *NextFree;
			MyHandle *Next;
			MyHandle *Prev;
			NextFree = ((MyHandle *)Mem)->NextHandle;		/* Get the free handle entry (Set by DeallocAHandle()) */

			/* Copy the NEW handle data over the old handle data */
			
			((MyHandle *)Mem)->MemPtr = NewMem->MemPtr;
			((MyHandle *)Mem)->Length = NewMem->Length;
			((MyHandle *)Mem)->Flags = NewMem->Flags;
			Next = NewMem->NextHandle;
			Prev = NewMem->PrevHandle;
			((MyHandle *)Mem)->NextHandle = Next;
			((MyHandle *)Mem)->PrevHandle = Prev;
		
			/* Fix the linked list entries to point to the previous handle record */
		
			if (Next) {
				Next->PrevHandle = (MyHandle *)Mem;
			}
			if (Prev) {
				Prev->NextHandle = (MyHandle *)Mem;
			}
						
			Next = NewMem->NextPurge;	/* Purgeable? */
			if (Next) {
				Prev = NewMem->PrevPurge;	/* Backward link */
				Next->PrevPurge = Prev;	/* Unlink me from the list */
				Prev->NextPurge = Next;
				NewMem->NextPurge = 0;
				NewMem->PrevPurge = 0;
				
				((MyHandle*)Mem)->PrevPurge = &PurgeHandleFiFo;
				((MyHandle*)Mem)->NextPurge = PurgeHandleFiFo.NextPurge;
				PurgeHandleFiFo.NextPurge->PrevPurge = (MyHandle *)Mem;
				PurgeHandleFiFo.NextPurge = (MyHandle *)Mem;
			} else {
				((MyHandle *)Mem)->NextPurge = 0;
				((MyHandle *)Mem)->PrevPurge = 0;
			}

			/* This is FUCKING hack since I am tired and I just lost my */
			/* ability to care. */
			
			{
				MyHandle *Scan;
				Scan = FreeMemHands.PrevHandle;
				if (Scan != &FreeMemHands) {		/* No free memory? */
					do {
						if (Scan->Flags == (Word32)NewMem) {
							Scan->Flags = (Word32)Mem;
						}
						Scan = Scan->PrevHandle;
					} while (Scan != &FreeMemHands);	/* End of the list? */
				}
			}
			
			/* Now release the handle structure */

			NewMem->Flags = (0xABCDUL<<16);		/* Bad ID */
			NewMem->NextHandle = NextFree;		/* Link in the list */
			NewMem->PrevHandle = 0;
			FreeHands = NewMem;					/* New parent */
			NewMem = (MyHandle *)Mem;			/* Ok, the old entry is ok */
		}
	}
	return (void **)NewMem;			/* Return the new pointer */
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
	Word32 Size;		/* Size to allocate */
	Word32 Flags;

	if (*MemHandle) {	/* Handle already valid? */
		SetHandlePurgeFlag(MemHandle,FALSE);	/* Don't purge now */
		return MemHandle;	/* Leave now! */
	}
	Size = ((MyHandle *)MemHandle)->Length;		/* How much memory to allocate */
	Flags = ((MyHandle *)MemHandle)->Flags;
	DeallocAHandle(MemHandle);		/* Dispose of the old handle */
	return AllocAHandle2(Size,Flags);	/* Create a new one with the old size */
}

/**********************************

	Search the handle tree for a pointer, note that the pointer does
	NOT have to be the MemPtr, just in the domain of the handle
	Return 0 if the handle is not here.

**********************************/

void ** BURGERCALL FindAHandle(void *MemPtr)
{
	Word8 *ThePtr;		/* Temp pointer */
	MyHandle *List;		/* Current handle */

	List = UsedHand1.NextHandle;	/* Get the first handle */
	if (List!=&UsedHand2) {		/* Failsafe... */
		do {
			ThePtr = (Word8 *)List->MemPtr;	/* Get the handle's memory pointer */
			if (ThePtr>((Word8 *)MemPtr)) {	/* Is it too far? */
				break;			/* Abort now... */
			}
			ThePtr += List->Length;			/* Get the final byte address */
			if (ThePtr>((Word8 *)MemPtr)) {	/* In range? */
				return (void **)List;		/* This is the handle! */
			}
			List = List->NextHandle;
		} while (List!=&UsedHand2);					/* List still valid? */
	}
	NonFatal("Can't find handle\n");			/* Didn't find it... */
	return 0;							/* Return null */
}

/********************************

	Returns the size of a memory handle

********************************/

Word32 BURGERCALL GetAHandleSize(void **MemHandle)
{
	if (MemHandle) {
		return ((MyHandle *)MemHandle)->Length;
	}
	return 0;
}

/**********************************

	Returns the size of a memory pointer

**********************************/

Word32 BURGERCALL GetAPointerSize(void *MemPtr)
{
	if (MemPtr) {			/* Null pointer?!? */
		MyHandle *Hand;
		Hand = (MyHandle *)(((Word8 **)((Word8 *)MemPtr-MEMORYPADDING))[0]);
		return Hand->Length;
	}
	return 0;
}


/********************************

	Returns the total free space with purging
	This is accomplished by adding all the memory found in
	the free memory linked list and then adding all the memory
	in the used list that can be purged.

********************************/

Word32 BURGERCALL GetTotalFreeMem(void)
{
	Word32 Free;		/* Running total */
	MyHandle *Scan;		/* Pointer to handle */

	Free = 0;			/* Init free size */

	/* Add all the free memory handles */

	Scan = FreeMemHands.NextHandle;	/* Follow the entire list */
	if (Scan!=&FreeMemHands) {			/* List valid? */
		do {
			Free += Scan->Length;	/* Just add it in */
			Scan = Scan->NextHandle;	/* Next one in chain */
		} while (Scan!=&FreeMemHands);	/* All done? */
	}

	/* Now traverse the used list for all purgable memory */

	Scan = UsedHand1.NextHandle;		/* Find all purgable memory */
	if (Scan!=&UsedHand2) {			/* Valid chain? */
		do {
			if (!(Scan->Flags&HANDLELOCK) &&	/* Unlocked and purgeable */
				(Scan->NextPurge)) {
				Word32 Temp;
				Temp = Scan->Length;		/* Round up the length */
				Temp = Temp+(MEMORYPADDING-1);
				Temp = Temp & (~(MEMORYPADDING-1));
				Free += Temp;		/* Add into the total */
			}
			Scan = Scan->NextHandle;		/* Next link */
		} while (Scan!=&UsedHand2);			/* All done? */
	}
	return Free;		/* Return the free size */
}

/********************************

	Set the lock flag to a given handle and return the data pointer

********************************/

void * BURGERCALL LockAHandle(void **TheHandle)
{
	if (TheHandle) {
		((MyHandle *)TheHandle)->Flags |= HANDLELOCK;	/* Lock the handle down */
		return TheHandle[0];
	}
	return 0;
}

/********************************

	Clear the lock flag to a given handle

********************************/

void BURGERCALL UnlockAHandle(void **TheHandle)
{
	if (TheHandle) {
		((MyHandle*)TheHandle)->Flags&=(~HANDLELOCK);	/* Clear the lock flag */
	}
}

/********************************

	Set the purge flag to a given handle

********************************/

void BURGERCALL SetHandlePurgeFlag(void **TheHandle,Word Flag)
{
	if (!(((MyHandle *)TheHandle)->Flags & HANDLEMALLOC)) {
		if (((MyHandle *)TheHandle)->NextPurge) {	/* Was it purgable? */

		/* Unlink from the purge fifo */

			((MyHandle *)TheHandle)->PrevPurge->NextPurge = ((MyHandle *)TheHandle)->NextPurge;
			((MyHandle *)TheHandle)->NextPurge->PrevPurge = ((MyHandle *)TheHandle)->PrevPurge;
		}
		/* Now is it purgable? */

		if (Flag) {
			((MyHandle*)TheHandle)->PrevPurge = &PurgeHandleFiFo;
			((MyHandle*)TheHandle)->NextPurge = PurgeHandleFiFo.NextPurge;
			PurgeHandleFiFo.NextPurge->PrevPurge = (MyHandle *)TheHandle;
			PurgeHandleFiFo.NextPurge = (MyHandle *)TheHandle;
			return;
		}
		((MyHandle*)TheHandle)->NextPurge = 0;
		((MyHandle*)TheHandle)->PrevPurge = 0;
	}
}

/********************************

	Get the current purge and lock flags of the handle

********************************/

Word BURGERCALL GetAHandleLockedState(void **MemHandle)
{
	return ((MyHandle *)MemHandle)->Flags & 0xFFFF;
}

/********************************

	Set the current purge and lock flags of the handle

********************************/

void BURGERCALL SetAHandleLockedState(void **MemHandle,Word Flag)
{
	((MyHandle *)MemHandle)->Flags = (((MyHandle *)MemHandle)->Flags&0xFFFF0000) | Flag;

	if (!(((MyHandle *)MemHandle)->Flags & HANDLEMALLOC)) {
		if (((MyHandle *)MemHandle)->NextPurge) {	/* Was it purgable? */

		/* Unlink from the purge fifo */

			((MyHandle *)MemHandle)->PrevPurge->NextPurge = ((MyHandle *)MemHandle)->NextPurge;
			((MyHandle *)MemHandle)->NextPurge->PrevPurge = ((MyHandle *)MemHandle)->PrevPurge;
		}
		/* Now is it purgable? */

		if (Flag&1) {
			((MyHandle*)MemHandle)->PrevPurge = &PurgeHandleFiFo;
			((MyHandle*)MemHandle)->NextPurge = PurgeHandleFiFo.NextPurge;
			PurgeHandleFiFo.NextPurge->PrevPurge = (MyHandle *)MemHandle;
			PurgeHandleFiFo.NextPurge = (MyHandle *)MemHandle;
			return;
		}
		((MyHandle*)MemHandle)->NextPurge = 0;
		((MyHandle*)MemHandle)->PrevPurge = 0;
	}
}

/********************************

	This routine will move a handle from the used list
	into the purged handle list. The handle is not discarded.
	This is the only way a handle can be placed into the purged list.

	I will call ReleaseMemRange() to alert the free memory
	list that I have free memory.

********************************/

void BURGERCALL PurgeAHandle(void **MemHandle)
{
	MyHandle *Prev;			/* Previous link */
	MyHandle *Next;

	if (MemHandle &&		/* Valid pointer? */
		((MyHandle *)MemHandle)->MemPtr &&
		!(((MyHandle *)MemHandle)->Flags&HANDLEMALLOC)) {		/* Not purged? */

		if (MemPurgeCallBack) {
			MemPurgeCallBack(MMStagePurge);	/* I will purge now! */
		}

		((MyHandle *)MemHandle)->Flags &= (~HANDLELOCK);	/* Force unlocked */

	/* Unlink from the purge list */

		Next = ((MyHandle *)MemHandle)->NextPurge;	/* Forward link */
		if (Next) {
			Prev = ((MyHandle *)MemHandle)->PrevPurge;	/* Backward link */
			Next->PrevPurge = Prev;	/* Unlink me from the list */
			Prev->NextPurge = Next;
			((MyHandle *)MemHandle)->NextPurge = 0;
			((MyHandle *)MemHandle)->PrevPurge = 0;
		}

	/* Unlink from the used list */

		Next = ((MyHandle *)MemHandle)->NextHandle;	/* Forward link */
		Prev = ((MyHandle *)MemHandle)->PrevHandle;	/* Backward link */
		Next->PrevHandle = Prev;	/* Unlink me from the list */
		Prev->NextHandle = Next;

	/* Move to the purged handle list */
	/* Don't harm the flags or the length!! */

		ReleaseMemRange(((MyHandle *)MemHandle)->MemPtr,((MyHandle *)MemHandle)->Length,Prev);	/* Release the memory */

		Prev = PurgeHands.NextHandle;	/* Get the first link */
		((MyHandle *)MemHandle)->MemPtr = 0;		/* Zap the pointer (Purge list) */
		((MyHandle *)MemHandle)->PrevHandle = &PurgeHands;	/* I am the parent */
		((MyHandle *)MemHandle)->NextHandle = Prev;	/* Link it to the purge list */
		Prev->PrevHandle = (MyHandle *)MemHandle;
		PurgeHands.NextHandle = (MyHandle*)MemHandle;	/* Make as the new head */
	}
}

/********************************

	Purges all handles that are purgable and are
	greater or equal to the level requested.
	I will call ReleaseMemRange() to alert the free memory
	list that I have free memory.

********************************/

Word BURGERCALL PurgeHandles(Word32 MemSize)
{
	MyHandle *Scan;
	Word Result;

	Result = FALSE;
	Scan = PurgeHandleFiFo.PrevPurge;	/* Index to the active handle list */
	if (Scan!=&PurgeHandleFiFo) {			/* No memory used? */
		do {			/* Follow the handle list */
			MyHandle *Next;
			Word32 TempLen;
			Next = Scan->PrevPurge;		/* Preload next link */
			TempLen = (Scan->Length+(MEMORYPADDING-1)) & (~(MEMORYPADDING-1));		/* Round up */
			PurgeAHandle((void **)Scan);		/* Force a purge */
			Result = TRUE;
			if (TempLen>=MemSize) {
				return TRUE;
			}
			MemSize-=TempLen;	/* Remove this... */
			Scan = Next;		/* Get the next link */
		} while (Scan!=&PurgeHandleFiFo);		/* At the end? */
	}
	return Result;
}

/********************************

	Packs all memory together.
	This doesn't alter the handle list in any way but it can move
	memory around to get rid of empty holes in the memory map.

********************************/

void BURGERCALL CompactHandles(void)
{
	MyHandle *Scan;
	Word CalledCallBack;

	Scan = UsedHand1.NextHandle;	/* Index to the active handle list */
	if (Scan==&UsedHand2) {			/* Failsafe */
		return;
	}
	CalledCallBack = TRUE;		/* Assume bogus */
	if (MemPurgeCallBack) {		/* Valid pointer? */
		CalledCallBack = FALSE;
	}
	do {	/* Skip all locked or fixed handles */
		if (!(Scan->Flags & (HANDLELOCK|HANDLEFIXED))) {
			Word32 Size;
			Word8 *StartMem;
			MyHandle *Last;

			Last = Scan->PrevHandle;		/* Get the previous handle */
			Size = (Last->Length+(MEMORYPADDING-1))&(~(MEMORYPADDING-1));	/* Pad to long word */
			StartMem = (Word8 *)Last->MemPtr + Size;
			Size = (Word8 *)Scan->MemPtr - StartMem;	/* Any space here? */
			if (Size) {		/* If there is free space, then pack them */
				void *TempPtr;
				if (!CalledCallBack) {		/* Hadn't called it yet? */
					CalledCallBack = TRUE;
					MemPurgeCallBack(MMStageCompact);	/* Tell the app */
				}
				TempPtr = Scan->MemPtr;		/* Save old address */
				Scan->MemPtr = StartMem;	/* Set new address */
				ReleaseMemRange(TempPtr,Scan->Length,Last);	/* Release the memory */
				GrabMemRange(StartMem,Scan->Length,Scan,0);		/* Grab the memory again */
				memmove(StartMem,TempPtr,Scan->Length);	/* Move */
			}
		}
		Scan = Scan->NextHandle;		/* Next handle in chain */
	} while (Scan!=&UsedHand2);
}

/**********************************

	Allocate a POINTER, I create a handle with 4
	bytes extra data and I place the handle at the beginning

**********************************/

void * BURGERCALL DebugAllocAPointer(Word32 Size,const char *Source,Word LineNum)
{
	Word32 *ThePointer;
	void **TheHandle;

	if (Size) {
		TheHandle = AllocAHandle2(Size+MEMORYPADDING,HANDLEFIXED);		/* Get the memory */
		if (TheHandle) {			/* Valid? */
			ThePointer = (Word32 *)TheHandle[0];	/* Deref the memory! */
			ThePointer[0] = (Word32)TheHandle;	/* Save the handle in memory */
			DebugAddSourceLine((void **)(((Word8 *)ThePointer)+MEMORYPADDING),Source,LineNum,TRUE);
	 		return (void *) (((Word8 *)ThePointer)+MEMORYPADDING);		/* Return the memory pointer */
		}
	}
	return 0;			/* Can't do it!! */
}

#endif


/**********************************

	Print a list of handles with records of free
	memory.

**********************************/

#if __MACUSENATIVEMEM__
void BURGERCALL DumpHandles(void)
{
	Word i;
	DebugMe_t *RefPtr;
	i = DebugCount;
	if (i) {
		DebugXMessage("Memory dump\n");
		++DebugRecurse;
		RefPtr = (DebugMe_t *)LockAHandle((void **)DebugMeHand);
		do {
			DebugXMessage("Memory $%08X (%d) Line %u, Source %s\n",RefPtr->HandleRef,RefPtr->NewLineNum>>31,RefPtr->NewLineNum&0x7FFFFFFF,RefPtr->Source);
			++RefPtr;
		} while (--i);
		UnlockAHandle((void **)DebugMeHand);
		--DebugRecurse;
	}
#if defined(__MAC__)
#if !TARGET_API_MAC_CARBON
	DebugXMessage(
		"MaxBlock() = %d\n"
		"TempFreeMem() = %d\n"
		"FreeMem() = %d\n"
		"FreeMemSys() = %d\n",MaxBlock(),TempFreeMem(),FreeMem(),FreeMemSys());
#else
	DebugXMessage(
		"MaxBlock() = %d\n"
		"TempFreeMem() = %d\n"
		"FreeMem() = %d\n",MaxBlock(),TempFreeMem(),FreeMem());
#endif
#endif
}
#else

static void BURGERCALL PrintHandles(MyHandle *Scan,MyHandle *Last,Word NoCheck)
{
	Word Count;
	char FooBar[256];

	Count = 1;		/* Init handle count */

	DebugXString("#     Handle    Addr   Attr  ID    Size     Prev     Next\n");
	if (NoCheck || Scan!=Last) {
		do {
			LongWordToAsciiHex2(Count,&FooBar[0],ASCIINONULL|ASCIILEADINGZEROS|4);
			FooBar[4] = ' ';
			LongWordToAsciiHex2((Word32)Scan,&FooBar[5],ASCIINONULL|ASCIILEADINGZEROS|8);
			FooBar[13] = ' ';
			LongWordToAsciiHex2((Word32)Scan->MemPtr,&FooBar[14],ASCIINONULL|ASCIILEADINGZEROS|8);
			FooBar[22] = ' ';
			LongWordToAsciiHex2((Word32)Scan->Flags,&FooBar[23],ASCIINONULL|ASCIILEADINGZEROS|4);
			FooBar[27] = ' ';
			LongWordToAsciiHex2((Word32)Scan->Flags>>16,&FooBar[28],ASCIINONULL|ASCIILEADINGZEROS|4);
			FooBar[32] = ' ';
			LongWordToAsciiHex2((Word32)Scan->Length,&FooBar[33],ASCIINONULL|ASCIILEADINGZEROS|8);
			FooBar[41] = ' ';
			LongWordToAsciiHex2((Word32)Scan->PrevHandle,&FooBar[42],ASCIINONULL|ASCIILEADINGZEROS|8);
			FooBar[50] = ' ';
			LongWordToAsciiHex2((Word32)Scan->NextHandle,&FooBar[51],ASCIINONULL|ASCIILEADINGZEROS|8);
#if !_DEBUG
			FooBar[59] = '\n';
			FooBar[60] = 0;
#else
			{
				Word32 LineNum;
				char *StrPtr;
				FooBar[59] = 0;
				DebugGetSourceLineInfo((void **)Scan,&StrPtr,&LineNum);
				if (StrPtr) {
					goto Coolness;
				}
				DebugGetSourceLineInfo((void **)((Word8 *)Scan->MemPtr+MEMORYPADDING),&StrPtr,&LineNum);
				if (StrPtr) {
Coolness:;
					strcpy(&FooBar[59]," File ");
					strcat(&FooBar[59],StrPtr);
					strcat(&FooBar[59],", Line ");
					StrPtr = &FooBar[strlen(FooBar)];
					StrPtr = LongWordToAscii(LineNum&0x7FFFFFFF,StrPtr);
					StrPtr[0] = '\n';
					StrPtr[1] = 0;
				} else {
					FooBar[59] = '\n';
					FooBar[60] = 0;
				}
			}
#endif
			DebugXString(FooBar);
			Scan = Scan->NextHandle;	/* Next handle in chain */
			++Count;
		} while (Scan!=Last);		/* All done? */
	}
}

/**********************************

	Display all the memory

**********************************/

void BURGERCALL DumpHandles(void)
{
	Word32 Size;
	Size = GetTotalFreeMem();
	DebugXString("Total free mem with purging ");
	DebugXLongWord(Size);
	DebugXString("\nUsed handle list\n");
	
	/* Set the flag here, DebugXString COULD allocate memory on some */
	/* file systems for directory caches. */
	/* As a result, the recurse flag being set could case a handle that needs */
	/* to be tracked to be removed from the tracking list and therefore */
	/* flag an error that doesn't exist */
	
#if _DEBUG
	++DebugRecurse;
#endif
	PrintHandles(&UsedHand1,&UsedHand1,TRUE);
	DebugXString("Purged handle list\n");
	PrintHandles(PurgeHands.NextHandle,&PurgeHands,FALSE);
	DebugXString("Free memory list\n");
	PrintHandles(FreeMemHands.NextHandle,&FreeMemHands,FALSE);
#if defined(__MAC__)
	DebugXMessage(
		"MaxBlock() = %d\n"
		"TempFreeMem() = %d\n"
		"FreeMem() = %d\n"
		"FreeMemSys() = %d\n",MaxBlock(),TempFreeMem(),FreeMem(),FreeMemSys());
#endif
#if _DEBUG
	--DebugRecurse;			/* Release the tracking flag */
#endif
}
#endif

/**********************************

	Add a handle to the debug list

**********************************/

void BURGERCALL DebugAddSourceLine(void **MemHandle,const char *Source,Word32 LineNum,Word IsPointer)
{
	DebugMe_t *RefPtr;

	if (!DebugRecurse) {		/* Don't recurse! */
		++DebugRecurse;			/* Inc the atomic recurse flag */
		if (DebugCount>=DebugMaxCount) {	/* Need more memory? */
			DebugMaxCount+=256;			/* Bigger! */
			DebugMeHand = (DebugMe_t **)ResizeAHandle((void **)DebugMeHand,sizeof(DebugMe_t)*DebugMaxCount);
			if (!DebugMeHand) {
				DebugMaxCount = 0;	/* Discard all! */
				DebugCount = 0;
				--DebugRecurse;
				return;
			}
		}
		RefPtr = &DebugMeHand[0][DebugCount];	/* Get the pointer */
		RefPtr->HandleRef = MemHandle;		/* Fill in the new entry */
#if REFNAMEPTRS
		RefPtr->Source = Source;
#else
		strncpy(RefPtr->Source,Source,sizeof(RefPtr->Source));
		RefPtr->Source[sizeof(RefPtr->Source)-1]=0;
#endif
		if (IsPointer) {
			LineNum |= HANDLELINEFLAG;
		}
		RefPtr->NewLineNum = LineNum;
		++DebugCount;			/* 1 more valid */
		--DebugRecurse;			/* Undo my recursion */
	}
}

/**********************************
	
	Check if a handle is in the debug list and return TRUE
	if it is found in the list
	
**********************************/
	
Word BURGERCALL DebugMemoryIsHandleValid(void **MemHandle)
{	
	if (MemHandle) {			/* Null handle?!? */
		Word i;					/* Get the count to scan */
		DebugMe_t *RefPtr;
		
		i = DebugCount;			/* Get the count to scan */
		if (i) {
			RefPtr = DebugMeHand[0];	/* Deref the handle */
			do {
				if (!(RefPtr->NewLineNum&HANDLELINEFLAG)) {		/* Handle? */
					if (RefPtr->HandleRef==MemHandle) {		/* Match? */
						return TRUE;
					}
				}
				++RefPtr;		/* Next entry */
			} while (--i);		/* All done? */
		}
	}
	return FALSE;		/* Not in the list */
}

/**********************************
	
	Check if a handle is in the debug list and return TRUE
	if it is found in the list
	
**********************************/
	
Word BURGERCALL DebugMemoryIsPointerValid(void *MemPtr)
{
	Word i;					/* Get the count to scan */
	DebugMe_t *RefPtr;
	Word32 Length;
	
	i = DebugCount;			/* Get the count to scan */
	if (i) {
		RefPtr = DebugMeHand[0];	/* Deref the handle */
		do {
			if (RefPtr->NewLineNum&HANDLELINEFLAG) {		/* Pointer? */
				if ((void *)RefPtr->HandleRef<=MemPtr) {		/* Can I check? */
					Length = GetAPointerSize((void *)RefPtr->HandleRef);
					if ((((Word8 *)RefPtr->HandleRef)+Length)>=(Word8 *)MemPtr) {
						return TRUE;
					}
				}
			}
			++RefPtr;		/* Next entry */
		} while (--i);		/* All done? */
	}
	return FALSE;			/* Not found in the table */
}

/**********************************

	Remove a handle from the debug list

**********************************/

Word BURGERCALL DebugRemoveSourceLine(void **MemHandle,const char *Source,Word Line)
{
	Word i;
	DebugMe_t *RefPtr;
	DebugMe_t *RootPtr;

	if (!DebugRecurse) {		/* Recursion? */
		++DebugRecurse;			/* Inc the recursion flag */
		i = DebugCount;			/* Get the count to scan */
		if (i) {
			RefPtr = &DebugMeHand[0][i-1];	/* Deref the handle */
			do {
				if (RefPtr->HandleRef==MemHandle) {		/* Match? */
					--DebugCount;						/* Remove from the count */
					RootPtr = &DebugMeHand[0][DebugCount];	/* Move the last entry to this one */
					RefPtr->HandleRef = RootPtr->HandleRef;
#if REFNAMEPTRS
					RefPtr->Source = RootPtr->Source;
#else
					strcpy(RefPtr->Source,RootPtr->Source);
#endif
					RefPtr->NewLineNum = RootPtr->NewLineNum;
					
					if (!DebugCount) {			/* No more? */
						DeallocAHandle((void **)DebugMeHand);	/* Kill memory */
						DebugMeHand = 0;		/* Zap my vars */
						DebugMaxCount = 0;
					}
					--DebugRecurse;		/* Allow recursion */
					return FALSE;
				}
				--RefPtr;		/* Next entry */
			} while (--i);
		}
		DebugXMessage("Handle %08X was not in the debug list, File: %s, Line %d\n",MemHandle,Source,Line);
		--DebugRecurse;
		return TRUE;
	}
	return FALSE;
}

/**********************************

	Find a handle in the debug list

**********************************/

void BURGERCALL DebugGetSourceLineInfo(void **MemHandle,char **Source,Word32 *LineNum)
{
	Word i;
	DebugMe_t *RefPtr;
	i = DebugCount;
	if (i) {
		RefPtr = DebugMeHand[0];		/* Deref the handle */
		do {
			if (RefPtr->HandleRef==MemHandle) {		/* Match? */
				Source[0] = (char *)RefPtr->Source;	/* Print the source line */
				LineNum[0] = RefPtr->NewLineNum;
				return;
			}
			++RefPtr;					/* Next */
		} while (--i);
	}
	if (MemHandle==(void**)DebugMeHand) {		/* Is it my own? */
		Source[0] = __FILE__;
		LineNum[0] = 800;			/* Line for the ResizeAHandle */
		return;
	}
	Source[0] = 0;		/* Bad news!!! */
	LineNum[0] = 0;
}


