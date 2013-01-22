/**********************************

	Memory pool manager
	Create a array of structures and quickly allocate and
	deallocate from this array pool. You can allocate from a static
	array, a resizable dynamic array or an array that is just 
	a looping pointer
	
**********************************/

#include "MmMemory.h"

/**********************************

	Here is how it works. I maintain a giant chunk of
	memory that is all the memory I manage. I also have
	a resizable handle that is a list of pointers to
	the memory chunks. To allocate memory, I simple take
	a pointer from the handle array and return that.
	If I am out of pointers I can either take a pointer
	that was already assigned (MEMPOOL_LOOP), allocate
	more memory (MEMPOOL_DYNAMIC) or surrender (MEMPOOL_STATIC)
	Since I can allocate more memory on the fly, the buffer has a
	hidden 16 byte structure at the start which is a linked list of memory
	chunks that have been allocated since the creation of the
	memory pool. I will traverse the linked list on shutdown to 
	release all memory that was allocated.
	
	Note : I cannot release memory at runtime. Only when destroying
	the structure.
	
**********************************/

#define MEMPOOLBLOCKSIZE 512

/**********************************

	Init a MemPool_t structure
	Return TRUE on an error
	Only a MEMPOOL_DYNAMIC structure can be
	empty at startup. LOOPED and STATIC must
	have the memory at creation
	ChunkSize = Size of each memory piece. Will be rounded up to the nearest 4 (0,4,8)
	InitialSize = Number of memory chunks to allocate
	
**********************************/

Word BURGERCALL MemPoolInit(MemPool_t *Input,Word InitialSize,Word ChunkSize,MemPoolBehaviour_e Behaviour)
{
	/* Init all the entries */
	
	ChunkSize = (ChunkSize+3)&(~3);		/* Chunks must be longword aligned */
	
	Input->ArrayHandle = 0;		/* No memory present */
	Input->RootMem = 0;
	Input->ArraySize = InitialSize;	/* Set the initial size */
	Input->Count = 0;	
	Input->ChunkSize = ChunkSize;
	Input->Behaviour = Behaviour;		/* How to allocate the memory */

	if (ChunkSize) {					/* Must be valid chunk size */
		if (!InitialSize) {				/* No data present? */
			if (Behaviour!=MEMPOOL_DYNAMIC) {		/* Must have something!! */
				return TRUE;			/* Error */
			}
			return FALSE;				/* An empty dynamic structure is OK */
		}
		
		/* I want some memory to start with */
		
		{
			void ***ArrayHandle;		/* Temp handle list */
			
			ArrayHandle = (void ***)AllocAHandle(sizeof(void*)*InitialSize);	/* Array of pointers */
			if (ArrayHandle) {
				Word8 *SrcPtr;
				SrcPtr = (Word8 *)AllocAPointer(MEMORYPADDING+(ChunkSize*InitialSize));	/* Initial memory */
				if (SrcPtr) {			/* Allocation ok? */
					void **DestPtr;
					Input->ArrayHandle = ArrayHandle;
					Input->RootMem = SrcPtr;		/* Mark the linked list */
					((Word8 **)SrcPtr)[0] = 0;		/* End of the linked list */
					
					SrcPtr += MEMORYPADDING;		/* Index to the actual array */
					DestPtr = ArrayHandle[0];		/* Pointer to the array of pointers */
					do {
						DestPtr[0] = SrcPtr;		/* Create the array of pointers */
						++DestPtr;
						SrcPtr+=ChunkSize;			/* Skip to the next chunk */
					} while (--InitialSize);
					return FALSE;					/* No errors */
				}
				DeallocAHandle((void **)ArrayHandle);		/* Crud... */
			}
		}
		Input->ArraySize = 0;	/* Force an error */
	}
	return TRUE;				/* Error! */
}

/**********************************

	Create a new MemPool_t structure

**********************************/

MemPool_t * BURGERCALL MemPoolNew(Word InitialSize,Word ChunkSize,MemPoolBehaviour_e Behaviour)
{
	MemPool_t *Input;
	Input = (MemPool_t *)AllocAPointer(sizeof(MemPool_t));		/* Create the structure */
	if (Input) {
		if (!MemPoolInit(Input,InitialSize,ChunkSize,Behaviour)) {	/* Init it */
			return Input;
		}
		DeallocAPointer(Input);		/* Oh oh... */
	}
	return 0;			/* Out of memory! */
}

/**********************************

	Destroy a MemPool_t structure

**********************************/

void BURGERCALL MemPoolDestroy(MemPool_t *Input)
{
	Word8 *WorkPtr;
	
	Input->ArraySize = 0;			/* Zap the variables */
	Input->Count = 0;
	Input->ChunkSize = 0;
	WorkPtr = Input->RootMem;		/* Get the linked list */
	if (WorkPtr) {
		Input->RootMem = 0;			/* Purged */
		do {
			Word8 *Next;
			Next = ((Word8 **)WorkPtr)[0];	/* Get the next linked pointer */
			DeallocAPointer(WorkPtr);		/* Dispose of this block */
			WorkPtr = Next;					/* Follow the chain */
		} while (WorkPtr);					/* All done? */
	}
	DeallocAHandle((void **)Input->ArrayHandle);
	Input->ArrayHandle = 0;					/* Blast it! */
}

/**********************************

	Delete a MemPool_t structure

**********************************/

void BURGERCALL MemPoolDelete(MemPool_t *Input)
{
	if (Input) {					/* Valid pointer? */
		MemPoolDestroy(Input);		/* Destroy the contents */
		DeallocAPointer(Input);		/* Destroy the pointer */
	}
}

/**********************************

	Allocate an entry from the memory pool
	If no more entries are found process according
	to the behaviour

**********************************/

void * BURGERCALL MemPoolAllocate(MemPool_t *Input)
{
	void *DataPtr;
	Word i;
	
	/* Try to get memory from the list (Most likely case) */
	
	i = Input->Count;			/* Get the used count */
	if (i<Input->ArraySize) {	/* Do I have a free entry? */
		DataPtr = Input->ArrayHandle[0][i];		/* Get the memory to return */
		++i;
		Input->Count = i;		/* Increase the count */
		return DataPtr;			/* Return the memory */
	}

	/* I am going to have to allocate more memory, how shall I do it? */
	
	switch (Input->Behaviour) {			/* Behaviour type */
	
	/* This will expand the buffer to get more memory */
	
	case MEMPOOL_DYNAMIC:
		DataPtr = AllocAPointer(MEMORYPADDING+(Input->ChunkSize*MEMPOOLBLOCKSIZE));	/* Get MEMPOOLBLOCKSIZE more chunks */
		if (DataPtr) {
			void ***ArrayHandle;
			((void **)DataPtr)[0] = Input->RootMem;		/* Link in the memory */
			Input->RootMem = (Word8*)DataPtr;			/* This is the new base memory */

			ArrayHandle = (void ***)ResizeAHandle((void **)Input->ArrayHandle,(Input->ArraySize+MEMPOOLBLOCKSIZE)*sizeof(void *));
			Input->ArrayHandle = ArrayHandle;
			if (ArrayHandle) {					/* Allocation ok? */
				Word ChunkSize;
				Word8 *SrcPtr;
				void **DestPtr;
				
				DataPtr = (void *)(((Word8 *)DataPtr)+MEMORYPADDING);
				i = Input->ArraySize;
				Input->ArraySize = i+MEMPOOLBLOCKSIZE;		/* Set the new array size */
				DestPtr = &ArrayHandle[0][i];		/* New point array */
				SrcPtr = (Word8 *)DataPtr;					/* First entry */
				ChunkSize = Input->ChunkSize;				/* Size of each memory chunk */
				i = MEMPOOLBLOCKSIZE;						/* The MEMPOOLBLOCKSIZE new entries */
				do {
					DestPtr[0] = SrcPtr;					/* Store the pointers */
					++DestPtr;								/* Next pointer */
					SrcPtr += ChunkSize;					/* Next structure entry */
				} while (--i);								/* All done? */
				Input->Count++;								/* Accept the first entry */
				return DataPtr;								/* Return the first entry's pointer */
			}
			MemPoolDestroy(Input);		/* I am SCREWED!!! */
		}
		break;

	/* Here, I just start back from the beginning */
	
	case MEMPOOL_LOOP:
		Input->Count = 0;
		return Input->ArrayHandle[0][0];		/* Get the memory to return */
	}
	
	/* Surrender */
	
	return 0;
}

/**********************************

	Release an entry back into the memory pool
	Note : LOOP doesn't release memory in the classic sense
	Make this the first memory to be allocated
	
**********************************/

void BURGERCALL MemPoolDeallocate(MemPool_t *Input,void *MemPtr)
{
	Word i;
	if (MemPtr) {			/* Is the memory even valid? */
		if (Input->Behaviour!=MEMPOOL_LOOP) {		/* Should not be called for this mode */
			i = Input->Count-1;						/* Adjust the used count */
			Input->ArrayHandle[0][i] = MemPtr;		/* Save the released memory */
			Input->Count = i;						/* Save the new count */
		}
	}
}
