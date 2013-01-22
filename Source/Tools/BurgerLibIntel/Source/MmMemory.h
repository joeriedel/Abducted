/*******************************

	Burger's Universal library WIN95 version
	This is for Watcom 10.5 and higher...
	Also support for MSVC 4.0

*******************************/

#ifndef __MMMEMORY_H__
#define __MMMEMORY_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

/* Private equate */

#if defined(__MAC__)
#define __MACUSENATIVEMEM__ 1		/* Set to TRUE to use MacOS memory management */
#else
#define __MACUSENATIVEMEM__ 0		/* Never CHANGE this, the other version need this to be zero! */
#endif

/* Align all memory to this boundary */

#if defined(__WIN32__) || defined(__MAC__) || defined(__BEOS__)
#define MEMORYPADDING 16
#else
#define MEMORYPADDING 4
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Public data */

#define HANDLELOCK 0x80		/* Lock flag */
#define HANDLEFIXED 0x40	/* fixed memory flag */
#define HANDLEMALLOC 0x20	/* Memory was Malloc'd */
#define HANDLEINUSE 0x10	/* True if handle is used */

#define REZ_MEM_ID 0xFFF1	/* Resource manager memory ID */
#define DEBUG_MEM_ID 0xFFF2	/* Memory debugger memory ID */
#define DEAD_MEM_ID 0xFFFE	/* Dead memory space ID */

typedef struct MyHandle {
	void *MemPtr;			/* Pointer to true memory (Must be first!) */
	Word32 Length;		/* Length of memory */
	Word32 Flags;			/* Memory flags or parent used handle */
	struct MyHandle *NextHandle;	/* Next handle in the chain */
	struct MyHandle *PrevHandle;	/* Previous handle in the chain */
	struct MyHandle *NextPurge;		/* Next handle in purge list */
	struct MyHandle *PrevPurge;		/* Previous handle in the purge list */
} MyHandle;

/* Memory handlers */

#define MMStageCompact 0
#define MMStagePurge 1

typedef void (BURGERCALL *MemPurgeProcPtr)(Word Stage);

extern Word32 MaxMemSize;		/* Maximum memory the program will take (4M) */
extern Word32 MinReserveSize;	/* Minimum memory for OS (64K) */
extern Word MinHandles;			/* Number of handles to create (500) */
extern MemPurgeProcPtr MemPurgeCallBack;	/* Callback before memory purging */
extern void BURGERCALL InitMemory(void);	/* Call this FIRST! */
#define AllocAHandle(MemSize) AllocAHandle2(MemSize,0)
extern void ** BURGERCALL AllocAHandle2(Word32 MemSize,Word Flag);
extern void ** BURGERCALL AllocAHandleClear(Word32 MemSize);
extern void BURGERCALL DeallocAHandle(void **MemHandle);
extern void * BURGERCALL AllocAPointer(Word32 MemSize);
extern void * BURGERCALL AllocAPointerClear(Word32 MemSize);
extern void BURGERCALL DeallocAPointer(void *MemPtr);
extern void ** BURGERCALL ReallocAHandle(void **MemHandle);
extern void ** BURGERCALL FindAHandle(void *MemPtr);
extern void * BURGERCALL LockAHandle(void **MemHandle);
extern void BURGERCALL UnlockAHandle(void **MemHandle);
extern void BURGERCALL CompactHandles(void);
extern void BURGERCALL PurgeAHandle(void **MemHandle);
extern Word BURGERCALL PurgeHandles(Word32 MemSize);
extern Word32 BURGERCALL GetAHandleSize(void **MemHandle);
extern Word32 BURGERCALL GetAPointerSize(void *MemPtr);
extern Word BURGERCALL GetAHandleLockedState(void **MemHandle);
extern void BURGERCALL SetAHandleLockedState(void **MemHandle,Word State);
extern Word32 BURGERCALL GetTotalFreeMem(void);
extern Word32 BURGERCALL GetTotalAllocatedMem(void);
extern void BURGERCALL SetHandlePurgeFlag(void **MemHandle,Word Flag);
extern void BURGERCALL DumpHandles(void);
extern void * BURGERCALL ResizeAPointer(void *Mem,Word32 Size);
extern void ** BURGERCALL ResizeAHandle(void **Mem,Word32 Size);
extern void * BURGERCALL MemoryNewPointerCopy(const void *Mem,Word32 Size);
extern void ** BURGERCALL MemoryNewHandleCopy(void **Mem);
extern void BURGERCALL DebugAddSourceLine(void **MemHandle,const char *Source,Word32 Line,Word IsPointer);
extern Word BURGERCALL DebugRemoveSourceLine(void **MemHandle,const char *Source,Word LineNum);
extern void BURGERCALL DebugGetSourceLineInfo(void **MemHanel,char **Source,Word32 *Line);
extern void ** BURGERCALL DebugAllocAHandle2(Word32 MemSize,Word Flag,const char *Source,Word LineNum);
extern void ** BURGERCALL DebugAllocAHandleClear(Word32 MemSize,const char *Source,Word LineNum);
extern void BURGERCALL DebugDeallocAHandle(void **MemHandle,const char *Source,Word LineNum);
extern void * BURGERCALL DebugAllocAPointer(Word32 MemSize,const char *Source,Word LineNum);
extern void * BURGERCALL DebugAllocAPointerClear(Word32 MemSize,const char *Source,Word LineNum);
extern void BURGERCALL DebugDeallocAPointer(void *MemPtr,const char *Source,Word LineNum);
extern void ** BURGERCALL DebugReallocAHandle(void **MemHandle,const char *Source,Word LineNum);
extern void * BURGERCALL DebugResizeAPointer(void *Mem,Word32 Size,const char *Source,Word LineNum);
extern void ** BURGERCALL DebugResizeAHandle(void **Mem,Word32 Size,const char *Source,Word LineNum);
extern void * BURGERCALL DebugMemoryNewPointerCopy(const void *Mem,Word32 Size,const char *Source,Word LineNum);
extern void ** BURGERCALL DebugMemoryNewHandleCopy(void **Mem,const char *Source,Word LineNum);
extern Word BURGERCALL DebugMemoryIsPointerValid(void *MemPtr);
extern Word BURGERCALL DebugMemoryIsHandleValid(void **MemHandle);

#if _DEBUG
#undef AllocAHandle
#define AllocAHandle(x) DebugAllocAHandle2(x,0,__FILE__,__LINE__)
#define AllocAHandle2(x,y) DebugAllocAHandle2(x,y,__FILE__,__LINE__)
#define AllocAHandleClear(x) DebugAllocAHandleClear(x,__FILE__,__LINE__)
#define DeallocAHandle(x) DebugDeallocAHandle(x,__FILE__,__LINE__)
#define AllocAPointer(x) DebugAllocAPointer(x,__FILE__,__LINE__)
#define AllocAPointerClear(x) DebugAllocAPointerClear(x,__FILE__,__LINE__)
#define DeallocAPointer(x) DebugDeallocAPointer(x,__FILE__,__LINE__)
#define ReallocAHandle(x) DebugReallocAHandle(x,__FILE__,__LINE__)
#define ResizeAPointer(x,y) DebugResizeAPointer(x,y,__FILE__,__LINE__)
#define ResizeAHandle(x,y) DebugResizeAHandle(x,y,__FILE__,__LINE__)
#define MemoryNewPointerCopy(x,y) DebugMemoryNewPointerCopy(x,y,__FILE__,__LINE__)
#define MemoryNewHandleCopy(x) DebugMemoryNewHandleCopy(x,__FILE__,__LINE__)
#define MemoryIsPointerValid(x) DebugMemoryIsPointerValid(x)
#define MemoryIsHandleValid(x) DebugMemoryIsHandleValid(x)
#else
#define MemoryIsPointerValid(x)
#define MemoryIsHandleValid(x)
#endif

extern MyHandle UsedHand1;		/* First used memory handle */
extern MyHandle UsedHand2;		/* Last used memory handle */
extern MyHandle PurgeHands;		/* Purged handle list */
extern MyHandle FreeMemHands;	/* Free handle list */
extern MyHandle PurgeHandleFiFo;	/* Purged handle linked list */

typedef enum { MEMPOOL_STATIC,MEMPOOL_DYNAMIC,MEMPOOL_LOOP} MemPoolBehaviour_e;

typedef struct MemPool_t {
	void ***ArrayHandle;		/* Handle to free memory list */
	Word8 *RootMem;				/* Pointer to the allocated memory */
	Word ArraySize;				/* Size of the free memory list */
	Word Count;					/* Number of entries used */
	Word ChunkSize;				/* Size of each entry in bytes */
	MemPoolBehaviour_e Behaviour;	/* Type of MemPool_t */
} MemPool_t;

extern Word BURGERCALL MemPoolInit(MemPool_t *Input,Word InitialSize,Word ChunkSize,MemPoolBehaviour_e Behaviour);
extern MemPool_t * BURGERCALL MemPoolNew(Word InitialSize,Word ChunkSize,MemPoolBehaviour_e Behaviour);
extern void BURGERCALL MemPoolDestroy(MemPool_t *Input);
extern void BURGERCALL MemPoolDelete(MemPool_t *Input);
extern void * BURGERCALL MemPoolAllocate(MemPool_t *Input);
extern void BURGERCALL MemPoolDeallocate(MemPool_t *Input,void *MemPtr);

#ifdef __cplusplus
}
#endif

#endif

