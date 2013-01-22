/**********************************

	Linked list manager

**********************************/

#ifndef __LKLINKLIST_H__
#define __LKLINKLIST_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct LinkedListEntry_t;
typedef void (BURGERCALL *LinkedListDeleteProcPtr)(struct LinkedListEntry_t *);
typedef Word (BURGERCALL *LinkedListTraverseProcPtr)(void *);

typedef struct LinkedListEntry_t {
	struct LinkedListEntry_t *Next;		/* Pointer to the next entry */
	struct LinkedListEntry_t *Prev;		/* Pointer to the previous entry */
	void *Data;							/* Pointer to the data */
	LinkedListDeleteProcPtr KillProc;	/* Destructor */
} LinkedListEntry_t;

typedef struct LinkedList_t {
	LinkedListEntry_t *First;			/* First entry in the list */
	LinkedListEntry_t *Last;			/* Last entry in the list */
	Word Count;							/* Number of entries in the list */
} LinkedList_t;

#define LINKLIST_ABORT 1
#define LINKLIST_DELETE 2

typedef int (BURGERCALL *LinkedListSortProc)(const void *,const void *);

extern void BURGERCALL LinkedListEntryDeallocProc(LinkedListEntry_t *Input);
extern void BURGERCALL LinkedListEntryDeallocNull(LinkedListEntry_t *Input);
extern void BURGERCALL LinkedListInit(LinkedList_t *Input);
extern void BURGERCALL LinkedListDestroy(LinkedList_t *Input);
extern LinkedList_t * BURGERCALL LinkedListNew(void);
extern void BURGERCALL LinkedListDelete(LinkedList_t *Input);
extern void BURGERCALL LinkedListDeleteFirstEntry(LinkedList_t *Input);
extern void BURGERCALL LinkedListDeleteLastEntry(LinkedList_t *Input);
extern void BURGERCALL LinkedListDeleteEntryByData(LinkedList_t *Input,const void *DataPtr);
extern void BURGERCALL LinkedListDeleteEntry(LinkedList_t *Input,LinkedListEntry_t *EntryPtr);
extern void BURGERCALL LinkedListRemoveEntry(LinkedList_t *Input,LinkedListEntry_t *EntryPtr);
extern Word BURGERCALL LinkedListContains(const LinkedList_t *Input,const void *Data);
extern LinkedListEntry_t * BURGERCALL LinkedListGetEntry(const LinkedList_t *Input,Word EntryNum);
extern LinkedListEntry_t * BURGERCALL LinkedListGetEntryByData(const LinkedList_t *Input,const void *DataPtr);
extern void * BURGERCALL LinkedListGetEntryData(const LinkedList_t *Input,Word EntryNum);
extern Word BURGERCALL LinkedListFindString(const LinkedList_t *Input,const char *TextPtr);
extern LinkedListEntry_t * BURGERCALL LinkedListFindStringEntry(const LinkedList_t *Input,const char *TextPtr);
extern LinkedListEntry_t *BURGERCALL LinkedListTraverseForward(LinkedList_t *Input,LinkedListTraverseProcPtr Proc);
extern LinkedListEntry_t *BURGERCALL LinkedListTraverseBackward(LinkedList_t *Input,LinkedListTraverseProcPtr Proc);
extern void BURGERCALL LinkedListSort(LinkedList_t *Input,LinkedListSortProc Proc);
extern void BURGERCALL LinkedListAddEntryEnd(LinkedList_t *Input,LinkedListEntry_t *EntryPtr);
extern void BURGERCALL LinkedListAddEntryBegin(LinkedList_t *Input,LinkedListEntry_t *EntryPtr);
extern void BURGERCALL LinkedListAddEntryAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,LinkedListEntry_t *NewPtr);
extern void BURGERCALL LinkedListAddEntryBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,LinkedListEntry_t *NewPtr);
extern void BURGERCALL LinkedListAddNewEntryEnd(LinkedList_t *Input,void *Data);
extern void BURGERCALL LinkedListAddNewEntryBegin(LinkedList_t *Input,void *Data);
extern void BURGERCALL LinkedListAddNewEntryAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data);
extern void BURGERCALL LinkedListAddNewEntryBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data);
extern void BURGERCALL LinkedListAddNewEntryProcEnd(LinkedList_t *Input,void *Data,LinkedListDeleteProcPtr Kill);
extern void BURGERCALL LinkedListAddNewEntryProcBegin(LinkedList_t *Input,void *Data,LinkedListDeleteProcPtr Kill);
extern void BURGERCALL LinkedListAddNewEntryProcAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,LinkedListDeleteProcPtr Kill);
extern void BURGERCALL LinkedListAddNewEntryProcBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,LinkedListDeleteProcPtr Kill);
extern void BURGERCALL LinkedListAddNewEntryMemEnd(LinkedList_t *Input,void *Data);
extern void BURGERCALL LinkedListAddNewEntryMemBegin(LinkedList_t *Input,void *Data);
extern void BURGERCALL LinkedListAddNewEntryMemAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data);
extern void BURGERCALL LinkedListAddNewEntryMemBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data);
extern void BURGERCALL LinkedListAddNewEntryStringEnd(LinkedList_t *Input,const char *Data);
extern void BURGERCALL LinkedListAddNewEntryStringBegin(LinkedList_t *Input,const char *Data);
extern void BURGERCALL LinkedListAddNewEntryStringAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,const char *Data);
extern void BURGERCALL LinkedListAddNewEntryStringBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,const char *Data);
extern void BURGERCALL DebugLinkedListAddNewEntryEnd(LinkedList_t *Input,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryBegin(LinkedList_t *Input,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryProcEnd(LinkedList_t *Input,void *Data,LinkedListDeleteProcPtr Kill,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryProcBegin(LinkedList_t *Input,void *Data,LinkedListDeleteProcPtr Kill,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryProcAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,LinkedListDeleteProcPtr Kill,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryProcBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,LinkedListDeleteProcPtr Kill,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryMemEnd(LinkedList_t *Input,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryMemBegin(LinkedList_t *Input,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryMemAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryMemBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryStringEnd(LinkedList_t *Input,const char *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryStringBegin(LinkedList_t *Input,const char *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryStringAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,const char *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryStringBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,const char *Data,const char *File,Word Line);

#if _DEBUG
#define LinkedListAddNewEntryEnd(x,y) DebugLinkedListAddNewEntryEnd(x,y,__FILE__,__LINE__)
#define LinkedListAddNewEntryBegin(x,y) DebugLinkedListAddNewEntryBegin(x,y,__FILE__,__LINE__)
#define LinkedListAddNewEntryAfter(x,y,z) DebugLinkedListAddNewEntryAfter(x,y,z,__FILE__,__LINE__)
#define LinkedListAddNewEntryBefore(x,y,z) DebugLinkedListAddNewEntryBefore(x,y,z,__FILE__,__LINE__)
#define LinkedListAddNewEntryProcEnd(x,y,z) DebugLinkedListAddNewEntryProcEnd(x,y,z,__FILE__,__LINE__)
#define LinkedListAddNewEntryProcBegin(x,y,z) DebugLinkedListAddNewEntryProcBegin(x,y,z,__FILE__,__LINE__)
#define LinkedListAddNewEntryProcAfter(x,y,z,w) DebugLinkedListAddNewEntryProcAfter(x,y,z,w,__FILE__,__LINE__)
#define LinkedListAddNewEntryProcBefore(x,y,z,w) DebugLinkedListAddNewEntryProcBefore(x,y,z,w,__FILE__,__LINE__)
#define LinkedListAddNewEntryMemEnd(x,y) DebugLinkedListAddNewEntryMemEnd(x,y,__FILE__,__LINE__)
#define LinkedListAddNewEntryMemBegin(x,y) DebugLinkedListAddNewEntryMemBegin(x,y,__FILE__,__LINE__)
#define LinkedListAddNewEntryMemAfter(x,y,z) DebugLinkedListAddNewEntryMemAfter(x,y,z,__FILE__,__LINE__)
#define LinkedListAddNewEntryMemBefore(x,y,z) DebugLinkedListAddNewEntryMemBefore(x,y,z,__FILE__,__LINE__)
#define LinkedListAddNewEntryStringEnd(x,y) DebugLinkedListAddNewEntryStringEnd(x,y,__FILE__,__LINE__)
#define LinkedListAddNewEntryStringBegin(x,y) DebugLinkedListAddNewEntryStringBegin(x,y,__FILE__,__LINE__)
#define LinkedListAddNewEntryStringAfter(x,y,z) DebugLinkedListAddNewEntryStringAfter(x,y,z,__FILE__,__LINE__)
#define LinkedListAddNewEntryStringBefore(x,y,z) DebugLinkedListAddNewEntryStringBefore(x,y,z,__FILE__,__LINE__)
#endif

#define LinkedListGetSize(x) (x)->Count
#define LinkedListGetFirst(x) (x)->First
#define LinkedListGetLast(x) (x)->Last
#define LinkedListGetFirstData(x) (x)->First->Data
#define LinkedListGetLastData(x) (x)->Last->Data

#ifdef __cplusplus
}
#endif

#endif

