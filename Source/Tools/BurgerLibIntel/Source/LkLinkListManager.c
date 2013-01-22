/**********************************

	Linked list manager

**********************************/

#include "LkLinkList.h"
#include "MmMemory.h"
#include "ClStdLib.h"
#include <string.h>

#undef LinkedListAddNewEntryEnd
#undef LinkedListAddNewEntryBegin
#undef LinkedListAddNewEntryAfter
#undef LinkedListAddNewEntryBefore
#undef LinkedListAddNewEntryProcEnd
#undef LinkedListAddNewEntryProcBegin
#undef LinkedListAddNewEntryProcAfter
#undef LinkedListAddNewEntryProcBefore
#undef LinkedListAddNewEntryMemEnd
#undef LinkedListAddNewEntryMemBegin
#undef LinkedListAddNewEntryMemAfter
#undef LinkedListAddNewEntryMemBefore
#undef LinkedListAddNewEntryStringEnd
#undef LinkedListAddNewEntryStringBegin
#undef LinkedListAddNewEntryStringAfter
#undef LinkedListAddNewEntryStringBefore

/**********************************

	The linked list manager works this way.
	A LinkedList_t is the root entry. It contains all the
	entries of the list and all calls use this root
	entry as a starting point.

	The LinkedListEntry_t's are where the magic happens.
	It contains a doublely linked list with zero pointing
	to the end of the list. The root LinkedList_t structure
	is where you can find the beginning and the ending
	of the entire list quickly.

	Each LinkedListEntry_t also has a pointer to the generic data
	that is contained and also a pointer to a function to
	discard the data once the entry itself is deleted. The
	function pointer could be 0 for no action taken or a 1
	meaning that the data can be deleted with a simple call to
	DeallocAPointer(). The data could also be pointing to the end
	of the base LinkedListEntry_t structure if the LinkedListEntry_t
	structure is part of a larger structure.

	All Linked list manager calls are pointer based. There is
	no support of handles. However the Data field could be a
	handle and you can pass your own data destructor routine
	to discard the handle. There is a convience routine to add
	data if the data is a handle. But you have to typecast
	the void * to a void ** manually when you access the data

**********************************/

/**********************************

	Dispose of a dual pointer linked list entry

**********************************/

void BURGERCALL LinkedListEntryDeallocProc(LinkedListEntry_t *Input)
{
	DeallocAPointer(Input->Data);
	DeallocAPointer(Input);
}

/**********************************

	Don't dispose of a linked list entry

**********************************/

void BURGERCALL LinkedListEntryDeallocNull(LinkedListEntry_t * /* Input */)
{
}

/**********************************

	Init a linked list to a default state

**********************************/

void BURGERCALL LinkedListInit(LinkedList_t *Input)
{
	Input->First = 0;
	Input->Last = 0;
	Input->Count = 0;
}

/**********************************

	Dispose of an entire linked list

**********************************/

void BURGERCALL LinkedListDestroy(LinkedList_t *Input)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = Input->First;		/* Is there data here? */
	if (EntryPtr) {					/* Yep, kill it */
		do {
			LinkedListEntry_t *Next;
			Next = EntryPtr->Next;	/* Prefetch the next entry */
			if (EntryPtr->KillProc) {	/* Is there a destructor? */
				EntryPtr->KillProc(EntryPtr);	/* Call the proc */
			} else {
				DeallocAPointer(EntryPtr);
			}
			EntryPtr = Next;				/* Next entry */
		} while (EntryPtr);					/* Any more? */
	}
	Input->First = 0;					/* Zap the data */
	Input->Last = 0;
	Input->Count = 0;
}

/**********************************

	Allocate a linked list

**********************************/

LinkedList_t * BURGERCALL LinkedListNew(void)
{
	return (LinkedList_t *)AllocAPointerClear(sizeof(LinkedList_t));
}

/**********************************

	Dispose of a linked list

**********************************/

void BURGERCALL LinkedListDelete(LinkedList_t *Input)
{
	if (Input) {
		LinkedListDestroy(Input);		/* Kill the list */
		DeallocAPointer(Input);			/* Dispose of the structure */
	}
}

/**********************************

	Dispose of the first entry in the linked list

**********************************/

void BURGERCALL LinkedListDeleteFirstEntry(LinkedList_t *Input)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = Input->First;
	if (EntryPtr) {
		LinkedListDeleteEntry(Input,EntryPtr);
	}
}

/**********************************

	Dispose of the last entry in the linked list

**********************************/

void BURGERCALL LinkedListDeleteLastEntry(LinkedList_t *Input)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = Input->Last;
	if (EntryPtr) {
		LinkedListDeleteEntry(Input,EntryPtr);
	}
}

/**********************************

	Given a data pointer, delete an entry

**********************************/

void BURGERCALL LinkedListDeleteEntryByData(LinkedList_t *Input,const void *DataPtr)
{
	LinkedListEntry_t *EntryPtr;

	EntryPtr = Input->First;		/* Is there a list? */
	if (EntryPtr) {
		do {
			if (EntryPtr->Data==DataPtr) {	/* Match? */
				goto ByeBye;			/* Jump to kill code below */
			}
			EntryPtr = EntryPtr->Next;	/* Next entry please... */
		} while (EntryPtr);				/* More left? */
	}
	return;
ByeBye:
	LinkedListDeleteEntry(Input,EntryPtr);		/* Entry found */
}

/**********************************

	Dispose of a single entry from the list

**********************************/

void BURGERCALL LinkedListDeleteEntry(LinkedList_t *Input,LinkedListEntry_t *EntryPtr)
{
	LinkedListEntry_t *Next;
	LinkedListEntry_t *Prev;

	Next = EntryPtr->Next;	/* Prefetch the next entry */
	Prev = EntryPtr->Prev;	/* Prefetch the prev entry */
	Input->Count--;			/* 1 less */
	if (Input->First == EntryPtr) {		/* Was this the first one? */
		Input->First = Next;			/* Set the new root */
	}
	if (Input->Last == EntryPtr) {		/* Was this the last one? */
		Input->Last = Prev;				/* Set the new tail */
	}
	if (Prev) {					/* Shall I unlink? */
		Prev->Next = Next;		/* Set the new next link */
	}
	if (Next) {
		Next->Prev = Prev;		/* Set the new prev link */
	}

		/* Now kill the data if needed */

	if (EntryPtr->KillProc) {	/* Is there a destructor? */
		EntryPtr->KillProc(EntryPtr);	/* Call the proc */
	} else {
		DeallocAPointer(EntryPtr);			/* Kill the entry itself */
	}
}

/**********************************

	Remove a single entry from the list
	but don't actually discard its contents!

**********************************/

void BURGERCALL LinkedListRemoveEntry(LinkedList_t *Input,LinkedListEntry_t *EntryPtr)
{
	LinkedListEntry_t *Next;
	LinkedListEntry_t *Prev;

	Next = EntryPtr->Next;	/* Prefetch the next entry */
	Prev = EntryPtr->Prev;	/* Prefetch the prev entry */
	Input->Count--;			/* 1 less */
	if (Input->First == EntryPtr) {		/* Was this the first one? */
		Input->First = Next;			/* Set the new root */
	}
	if (Input->Last == EntryPtr) {		/* Was this the last one? */
		Input->Last = Prev;				/* Set the new tail */
	}
	if (Prev) {					/* Shall I unlink? */
		Prev->Next = Next;		/* Set the new next link */
	}
	if (Next) {
		Next->Prev = Prev;		/* Set the new prev link */
	}
}

/**********************************

	Determine if the list contains a piece of data

**********************************/

Word BURGERCALL LinkedListContains(const LinkedList_t *Input,const void *Data)
{
	const LinkedListEntry_t *EntryPtr;

	EntryPtr = Input->First;		/* Is there a list? */
	if (EntryPtr) {
		do {
			if (EntryPtr->Data==Data) {	/* Match? */
				return TRUE;			/* Entry found */
			}
			EntryPtr = EntryPtr->Next;	/* Next entry please... */
		} while (EntryPtr);				/* More left? */
	}
	return FALSE;		/* Does not contain the entry */
}

/**********************************

	Find the nth entry in the list

**********************************/

LinkedListEntry_t * BURGERCALL LinkedListGetEntry(const LinkedList_t *Input,Word EntryNum)
{
	LinkedListEntry_t *EntryPtr;

	if (Input->Count>EntryNum) {	/* Invalid count? */
		EntryPtr = Input->First;	/* Get the first entry */
		if (EntryNum) {				/* Should I traverse? */
			do {
				EntryPtr = EntryPtr->Next;	/* Follow the list */
			} while (--EntryNum);	/* All done? */
		}
		return EntryPtr;			/* Found it! */
	}
	return 0;
}

/**********************************

	Traverse the linked list until an entry matches the supplied data
	Returns 0 if the data is not in the list

**********************************/

LinkedListEntry_t * BURGERCALL LinkedListGetEntryByData(const LinkedList_t *Input,const void *DataPtr)
{
	LinkedListEntry_t *EntryPtr;

	EntryPtr = Input->First;	/* Get the first entry */
	if (EntryPtr) {				/* Should I traverse? */
		do {
			if (EntryPtr->Data==DataPtr) {
				break;
			}
			EntryPtr = EntryPtr->Next;	/* Follow the list */
		} while (EntryPtr);	/* All done? */
	}
	return EntryPtr;
}

/**********************************

	Find the nth entry in the list and
	return the data attached

**********************************/

void * BURGERCALL LinkedListGetEntryData(const LinkedList_t *Input,Word EntryNum)
{
	LinkedListEntry_t *EntryPtr;

	if (Input->Count>EntryNum) {	/* Invalid count? */
		EntryPtr = Input->First;	/* Get the first entry */
		if (EntryNum) {				/* Should I traverse? */
			do {
				EntryPtr = EntryPtr->Next;	/* Follow the list */
			} while (--EntryNum);	/* All done? */
		}
		return EntryPtr->Data;		/* Found it! */
	}
	return 0;
}

/**********************************

	Search a linked list of STRINGS and
	return the NUMBER of the entry found
	Return -1 if no entry is found

**********************************/

Word BURGERCALL LinkedListFindString(const LinkedList_t *Input,const char *TextPtr)
{
	LinkedListEntry_t *EntryPtr;
	if (TextPtr) {
		EntryPtr = LinkedListGetFirst(Input);		/* Check the list for a string */
		if (EntryPtr) {				/* Any data in the list? */
			Word i;
			i = 0;
			do {
				if (!stricmp(TextPtr,(char *)EntryPtr->Data)) {		/* Does this match? */
					return i;
				}
				++i;
				EntryPtr = EntryPtr->Next;			/* Follow the list */
			} while (EntryPtr);			/* Still more */
		}
	}
	return (Word)-1;		/* You suck */
}

/**********************************

	Search a linked list of STRINGS and
	return the LinkedListEntry_t *of the entry found
	Return NULL if no entry is found

**********************************/

LinkedListEntry_t * BURGERCALL LinkedListFindStringEntry(const LinkedList_t *Input,const char *TextPtr)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = 0;
	if (TextPtr) {
		EntryPtr = LinkedListGetFirst(Input);		/* Check the list for a string */
		if (EntryPtr) {				/* Any data in the list? */
			do {
				if (!stricmp(TextPtr,(char *)EntryPtr->Data)) {		/* Does this match? */
					break;
				}
				EntryPtr = EntryPtr->Next;			/* Follow the list */
			} while (EntryPtr);			/* Still more */
		}
	}
	return EntryPtr;		/* Return the match or NULL */
}


/**********************************

	Iterate from the beginning to the end of the list

**********************************/

LinkedListEntry_t *BURGERCALL LinkedListTraverseForward(LinkedList_t *Input,LinkedListTraverseProcPtr Proc)
{
	LinkedListEntry_t *EntryPtr;
	Word Result;

	EntryPtr = Input->First;		/* Is there a list? */
	if (EntryPtr) {
		do {
			LinkedListEntry_t *Next;
			Result = Proc(EntryPtr->Data);	/* Call the function */
			Next = EntryPtr->Next;			/* Get the next entry */
			if (Result&LINKLIST_DELETE) {	/* Dispose of the entry? */
				LinkedListDeleteEntry(Input,EntryPtr);
			}
			if (Result&LINKLIST_ABORT) {	/* Abort the traversal? */
				break;
			}
			EntryPtr = Next;		/* Next entry please... */
		} while (EntryPtr);			/* More left? */
	}
	return EntryPtr;		/* Entry I stopped on */
}

/**********************************

	Iterate from the end to the beginning of the list

**********************************/

LinkedListEntry_t *BURGERCALL LinkedListTraverseBackward(LinkedList_t *Input,LinkedListTraverseProcPtr Proc)
{
	LinkedListEntry_t *EntryPtr;
	Word Result;

	EntryPtr = Input->Last;		/* Is there a list? */
	if (EntryPtr) {
		do {
			LinkedListEntry_t *Prev;
			Result = Proc(EntryPtr->Data);	/* Call the function */
			Prev = EntryPtr->Prev;			/* Get the next entry */
			if (Result&LINKLIST_DELETE) {	/* Dispose of the entry? */
				LinkedListDeleteEntry(Input,EntryPtr);
			}
			if (Result&LINKLIST_ABORT) {	/* Abort the traversal? */
				break;
			}
			EntryPtr = Prev;		/* Next entry please... */
		} while (EntryPtr);			/* More left? */
	}
	return EntryPtr;		/* Entry I stopped on */
}

/**********************************
	
	Merges src1/size1 and src2/size2 to dest
	Both Size1 and Size2 MUST be non-zero
	
**********************************/

static void LinkedListMerge(LinkedListEntry_t **DestPtr,LinkedListEntry_t **Src1,LinkedListEntry_t **Src2,Word Size1, Word Size2,LinkedListSortProc Proc)
{
	LinkedListEntry_t *Ptr1,*Ptr2;
	
/* merge two parts of the unsorted array to the sorted array */

	Ptr1 = Src1[0];
	Ptr2 = Src2[0];
	if (Proc(Ptr1->Data,Ptr2->Data)<0) {		/* Which sort to use? */
mergefrom1:
		do {
			DestPtr[0] = Ptr1;	/* Copy one entry */
			++Src1;
			++DestPtr;
			if (!--Size1) {			/* Any more? */
				goto FlushSrc2;
			}
			Ptr1 = Src1[0];
		} while (Proc(Ptr1->Data,Ptr2->Data)<0);
	}
	do {
		DestPtr[0] = Ptr2;
		++Src2;
		++DestPtr;
		if (!--Size2) {
			goto FlushSrc1;
		}
		Ptr2 = Src2[0];
	} while (Proc(Ptr1->Data,Ptr2->Data)>=0);
	goto mergefrom1;

/* Src2 is empty, copy the rest of Src1 */

FlushSrc1:
	do {
		DestPtr[0] = Src1[0];
		++Src1;
		++DestPtr;
	} while (--Size1);
	return;

/* Src1 is empty, copy the rest of Src2 */

FlushSrc2:;
	do {	/* Dump the rest */
		DestPtr[0] = Src2[0];	/* Copy the rest of data */
		++Src2;
		++DestPtr;
	} while (--Size2);
	return;
}

/**********************************
	
	Sort the linked list by using a merge sort.
	I first, "flatten" the linked list into an array 
	of pointers and then perform the sort. Once
	the sort is done, I rebuild the linked list using the
	new sorted order.
	
**********************************/

void BURGERCALL LinkedListSort(LinkedList_t *Input,LinkedListSortProc Proc)
{
	LinkedListEntry_t **ArrayPtr;
	
	LinkedListEntry_t *Internal[2000];
	
	/* Simple sort? */
	
	if (Input->Count>=2) {				/* Should I even worry about a sort? */
	
		/* Two entries? */
		
		if (Input->Count==2) {			/* I guess so... */
			LinkedListEntry_t *First;
			LinkedListEntry_t *Second;
			First = Input->First;		/* Just a two entry sort, trivial... */
			Second = Input->Last;
			if (Proc(First->Data,Second->Data)>0) {		/* Worry? */
				Input->First = Second;					/* Perform a swap */
				Input->Last = First;
				Second->Next = First;		/* New first entry */
				Second->Prev = 0;
				First->Next = 0;			/* New last entry */
				First->Prev = Second;
			}
			return;							/* I am sorted */
		}
		
		/* Let's perform a merge sort */
		
		/* First get the buffer for the sort */
		
		if (Input->Count>1000) {
			ArrayPtr = (LinkedListEntry_t **)AllocAPointer(sizeof(LinkedListEntry_t *)*Input->Count*2);	/* Yikes!! */
			if (!ArrayPtr) {			/* Memory error! */
				return;					/* Forget about the sort! */
			}
		} else {
			ArrayPtr = Internal;		/* Use my internal buffer */
		}
		
		/* I need to first "flatten" the linked list */
		
		{	
			LinkedListEntry_t **WorkPtr1;
			LinkedListEntry_t *EntryPtr1;
			Word i1;
			WorkPtr1 = ArrayPtr;				/* Pointer to the allocated buffer */
			i1 = Input->Count;					/* Number of entries to upload */
			EntryPtr1 = Input->First;
			do {
				WorkPtr1[0] = EntryPtr1;		/* Store the entry */
				EntryPtr1 = EntryPtr1->Next;	/* Next one in the chain */
				++WorkPtr1;
			} while (--i1);					/* All done */
		}
		
		/* Now, we perform the merge sort */
		
		{
			LinkedListEntry_t **WorkPtr;
			{
				Word i;
				Word size;	/* Entry size to sort with */
				Word sort;	/* Sort count */
				LinkedListEntry_t **unsorted;
			    
				i = Input->Count;			/* How many entries are there? */	
				size = 1;						/* source size		(<<1 / loop)*/
				sort = 1;						/* iteration number (+1 / loop)*/
				WorkPtr = ArrayPtr;
				unsorted = ArrayPtr+i;
				do {
					{
						Word remaining;								/* Temp merge count */
						LinkedListEntry_t **src1,**src2,**dest;		/* Used by the sort */
						remaining = i>>sort;						/* How many times to try */
					
						/* pointers incremented by the merge */
						
						src1 = WorkPtr;								/* Sorted array */
						src2 = &WorkPtr[remaining<<(sort-1)];		/* Half point */
						dest = unsorted;	/* Dest array */
					
						/* merge paired blocks*/
						if (remaining) {	/* Any to sort? */
							do {
								LinkedListMerge(dest,src1,src2,size,size,Proc);	/* All groups equal size */
								dest = &dest[size+size];
								src1 = &src1[size];
								src2 = &src2[size];
							} while (--remaining);
						}
					
						/* copy or merge the leftovers */
						remaining = i&((size<<1)-1);	/* Create mask (1 bit higher) */
						if (remaining > size) {	/* one complete block and one fragment */
							LinkedListMerge(dest,&src2[size],src2,remaining-size,size,Proc);
						} else if (remaining) {	/* just a single sorted fragment */
							FastMemCpy(dest,src2,remaining*sizeof(LinkedListEntry_t *));	/* Copy it */
						}
					}
					
					/* get ready to sort back to the other array */
					
					size <<= 1;		/* Double the entry size */
					++sort;			/* Increase the shift size */
					{
						LinkedListEntry_t **temp;
						temp = WorkPtr;				/* Swap the pointers */
						WorkPtr = unsorted;
						unsorted = temp;
					}
				} while (size<i);
			}


		/* Now that I have the sorted list, Let's create the */
		/* new linked list */
			
			{
				LinkedListEntry_t *EntryPtr2;
				LinkedListEntry_t *Prev;
				LinkedListEntry_t *Next;
				Word i3;
				
				EntryPtr2 = WorkPtr[0];			/* Get the new root pointer */
				++WorkPtr;
				Input->First = EntryPtr2;		/* Set as the first one */
				i3 = Input->Count-1;			/* Traverse until the final link */
				Prev = 0;						/* Zap the previous link */
				do {
					Next = WorkPtr[0];
					++WorkPtr;
					EntryPtr2->Next = Next;		/* Store the links */
					EntryPtr2->Prev = Prev;
					Prev = EntryPtr2;			/* Follow the chains */
					EntryPtr2 = Next;			/* Next one */
				} while (--i3);					/* All done? */
				EntryPtr2->Next = 0;				/* The final link */
				EntryPtr2->Prev = Prev;
				Input->Last = EntryPtr2;			/* Save the ending link */
			}
		}
		if (ArrayPtr!=Internal) {
			DeallocAPointer(ArrayPtr);		/* Dispose of the linked list flattened data */
		}
	}
}

/**********************************

	Add an entry to the end of the list
	I assume that the Data and KillProc entries
	are filled in.

**********************************/

void BURGERCALL LinkedListAddEntryEnd(LinkedList_t *Input,LinkedListEntry_t *EntryPtr)
{
	LinkedListEntry_t *Other;
	Other = Input->Last;			/* Get the last entry */
	EntryPtr->Next = 0;			/* No forward entry */
	EntryPtr->Prev = Other;		/* Set the last entry */
	if (!Other) {				/* Was there a valid previous entry? */
		Input->First = EntryPtr;	/* This is the FIRST and LAST */
	} else {
		Other->Next = EntryPtr;		/* Add in the link */
	}
	Input->Last = EntryPtr;			/* Set the new last pointer */
	Input->Count++;				/* Entry added */
}

/**********************************

	Add an entry to the beginning of the list
	I assume that the Data and KillProc entries
	are filled in.

**********************************/

void BURGERCALL LinkedListAddEntryBegin(LinkedList_t *Input,LinkedListEntry_t *EntryPtr)
{
	LinkedListEntry_t *Other;
	Other = Input->First;		/* Get the first entry */
	EntryPtr->Next = Other;		/* Set the next entry */
	EntryPtr->Prev = 0;			/* No previous entry */
	if (!Other) {				/* Was there a valid next entry? */
		Input->Last = EntryPtr;	/* This is the FIRST and LAST */
	} else {
		Other->Prev = EntryPtr;	/* Add in the link */
	}
	Input->First = EntryPtr;	/* Set the new first pointer */
	Input->Count++;				/* Entry added */
}

/**********************************

	Add an entry after a specific entry in a list

**********************************/

void BURGERCALL LinkedListAddEntryAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,LinkedListEntry_t *NewPtr)
{
	LinkedListEntry_t *Other;
	Other = EntryPtr->Next;		/* Prepare for link in */
	EntryPtr->Next = NewPtr;	/* New forward link */
	NewPtr->Next = Other;		/* Set forward entry */
	NewPtr->Prev = EntryPtr;	/* Set prev entry */
	if (!Other) { 				/* Was there a valid previous entry? */
		Input->Last = NewPtr;	/* This is the FIRST and LAST */
	} else {
		Other->Prev = NewPtr;	/* Add in the link */
	}
	Input->Count++;				/* Entry added */
}

/**********************************

	Add an entry before a specific entry in a list

**********************************/

void BURGERCALL LinkedListAddEntryBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,LinkedListEntry_t *NewPtr)
{
	LinkedListEntry_t *Other;
	Other = EntryPtr->Prev;		/* Prepare for link in */
	EntryPtr->Prev = NewPtr;	/* New forward link */
	NewPtr->Next = EntryPtr;	/* Set forward entry */
	NewPtr->Prev = Other;		/* Set prev entry */
	if (!Other) { 				/* Was there a valid previous entry? */
		Input->First = NewPtr;	/* This is the FIRST and LAST */
	} else {
		Other->Next = NewPtr;	/* Add in the link */
	}
	Input->Count++;				/* Entry added */
}

/**********************************

	Add an entry to the end of the list

**********************************/

void BURGERCALL LinkedListAddNewEntryEnd(LinkedList_t *Input,void *Data)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t));
	if (EntryPtr) {
		EntryPtr->Data = Data;
		EntryPtr->KillProc = 0;
		LinkedListAddEntryEnd(Input,EntryPtr);
	}
}

/**********************************

	Add an entry to the beginning

**********************************/

void BURGERCALL LinkedListAddNewEntryBegin(LinkedList_t *Input,void *Data)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t));
	if (EntryPtr) {
		EntryPtr->Data = Data;
		EntryPtr->KillProc = 0;
		LinkedListAddEntryBegin(Input,EntryPtr);
	}
}

/**********************************

	Add a simple entry after an entry

**********************************/

void BURGERCALL LinkedListAddNewEntryAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data)
{
	LinkedListEntry_t *NewPtr;
	NewPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t));
	if (NewPtr) {
		NewPtr->Data = Data;
		NewPtr->KillProc = 0;
		LinkedListAddEntryAfter(Input,EntryPtr,NewPtr);
	}
}

/**********************************

	Add a simple entry before an entry

**********************************/

void BURGERCALL LinkedListAddNewEntryBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data)
{
	LinkedListEntry_t *NewPtr;
	NewPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t));
	if (NewPtr) {
		NewPtr->Data = Data;
		NewPtr->KillProc = 0;
		LinkedListAddEntryBefore(Input,EntryPtr,NewPtr);
	}
}


/**********************************

	Add an entry to the end of the list

**********************************/

void BURGERCALL LinkedListAddNewEntryProcEnd(LinkedList_t *Input,void *Data,LinkedListDeleteProcPtr Kill)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t));
	if (EntryPtr) {
		EntryPtr->Data = Data;
		EntryPtr->KillProc = Kill;
		LinkedListAddEntryEnd(Input,EntryPtr);
	}
}

/**********************************

	Add an entry to the beginning

**********************************/

void BURGERCALL LinkedListAddNewEntryProcBegin(LinkedList_t *Input,void *Data,LinkedListDeleteProcPtr Kill)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t));
	if (EntryPtr) {
		EntryPtr->Data = Data;
		EntryPtr->KillProc = Kill;
		LinkedListAddEntryBegin(Input,EntryPtr);
	}
}

/**********************************

	Add a simple entry after an entry

**********************************/

void BURGERCALL LinkedListAddNewEntryProcAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,LinkedListDeleteProcPtr Kill)
{
	LinkedListEntry_t *NewPtr;
	NewPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t));
	if (NewPtr) {
		NewPtr->Data = Data;
		NewPtr->KillProc = Kill;
		LinkedListAddEntryAfter(Input,EntryPtr,NewPtr);
	}
}

/**********************************

	Add a simple entry before an entry

**********************************/

void BURGERCALL LinkedListAddNewEntryProcBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,LinkedListDeleteProcPtr Kill)
{
	LinkedListEntry_t *NewPtr;
	NewPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t));
	if (NewPtr) {
		NewPtr->Data = Data;
		NewPtr->KillProc = Kill;
		LinkedListAddEntryBefore(Input,EntryPtr,NewPtr);
	}
}

/**********************************

	Add an entry to the end of the list

**********************************/

void BURGERCALL LinkedListAddNewEntryMemEnd(LinkedList_t *Input,void *Data)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t));
	if (EntryPtr) {
		EntryPtr->Data = Data;
		EntryPtr->KillProc = LinkedListEntryDeallocProc;
		LinkedListAddEntryEnd(Input,EntryPtr);
	}
}

/**********************************

	Add an entry to the beginning of the list

**********************************/

void BURGERCALL LinkedListAddNewEntryMemBegin(LinkedList_t *Input,void *Data)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t));
	if (EntryPtr) {
		EntryPtr->Data = Data;
		EntryPtr->KillProc = LinkedListEntryDeallocProc;
		LinkedListAddEntryBegin(Input,EntryPtr);
	}
}

/**********************************

	Add an entry after an entry

**********************************/

void BURGERCALL LinkedListAddNewEntryMemAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data)
{
	LinkedListEntry_t *NewPtr;
	NewPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t));
	if (NewPtr) {
		NewPtr->Data = Data;
		NewPtr->KillProc = LinkedListEntryDeallocProc;
		LinkedListAddEntryAfter(Input,EntryPtr,NewPtr);
	}
}

/**********************************

	Add an entry before an entry

**********************************/

void BURGERCALL LinkedListAddNewEntryMemBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data)
{
	LinkedListEntry_t *NewPtr;
	NewPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t));
	if (NewPtr) {
		NewPtr->Data = Data;
		NewPtr->KillProc = LinkedListEntryDeallocProc;
		LinkedListAddEntryBefore(Input,EntryPtr,NewPtr);
	}
}

/**********************************

	Add an entry to the end of the list

**********************************/

void BURGERCALL LinkedListAddNewEntryStringEnd(LinkedList_t *Input,const char *Data)
{
	LinkedListEntry_t *EntryPtr;
	Word Length;
	Length = strlen(Data)+1;
	EntryPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t)+Length);
	if (EntryPtr) {
		FastMemCpy((EntryPtr+1),Data,Length);
		EntryPtr->Data = EntryPtr+1;
		EntryPtr->KillProc = 0;
		LinkedListAddEntryEnd(Input,EntryPtr);
	}
}

/**********************************

	Add an entry to the beginning of the list

**********************************/

void BURGERCALL LinkedListAddNewEntryStringBegin(LinkedList_t *Input,const char *Data)
{
	LinkedListEntry_t *EntryPtr;
	Word Length;
	Length = strlen(Data)+1;
	EntryPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t)+Length);
	if (EntryPtr) {
		FastMemCpy((EntryPtr+1),Data,Length);
		EntryPtr->Data = EntryPtr+1;
		EntryPtr->KillProc = 0;
		LinkedListAddEntryBegin(Input,EntryPtr);
	}
}

/**********************************

	Add an entry after an entry

**********************************/

void BURGERCALL LinkedListAddNewEntryStringAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,const char *Data)
{
	LinkedListEntry_t *NewPtr;
	Word Length;
	Length = strlen(Data)+1;
	NewPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t)+Length);
	if (NewPtr) {
		FastMemCpy((NewPtr+1),Data,Length);
		NewPtr->Data = NewPtr+1;
		NewPtr->KillProc = 0;
		LinkedListAddEntryAfter(Input,EntryPtr,NewPtr);
	}
}

/**********************************

	Add an entry before an entry

**********************************/

void BURGERCALL LinkedListAddNewEntryStringBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,const char *Data)
{
	LinkedListEntry_t *NewPtr;
	Word Length;
	Length = strlen(Data)+1;
	NewPtr = (LinkedListEntry_t *)AllocAPointer(sizeof(LinkedListEntry_t)+Length);
	if (NewPtr) {
		FastMemCpy((NewPtr+1),Data,Length);
		NewPtr->Data = NewPtr+1;
		NewPtr->KillProc = 0;
		LinkedListAddEntryBefore(Input,EntryPtr,NewPtr);
	}
}

/**********************************

	Add an entry to the end of the list

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryEnd(LinkedList_t *Input,void *Data,const char *File,Word Line)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t),File,Line);
	if (EntryPtr) {
		EntryPtr->Data = Data;
		EntryPtr->KillProc = 0;
		LinkedListAddEntryEnd(Input,EntryPtr);
	}
}

/**********************************

	Add an entry to the beginning

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryBegin(LinkedList_t *Input,void *Data,const char *File,Word Line)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t),File,Line);
	if (EntryPtr) {
		EntryPtr->Data = Data;
		EntryPtr->KillProc = 0;
		LinkedListAddEntryBegin(Input,EntryPtr);
	}
}

/**********************************

	Add a simple entry after an entry

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,const char *File,Word Line)
{
	LinkedListEntry_t *NewPtr;
	NewPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t),File,Line);
	if (NewPtr) {
		NewPtr->Data = Data;
		NewPtr->KillProc = 0;
		LinkedListAddEntryAfter(Input,EntryPtr,NewPtr);
	}
}

/**********************************

	Add a simple entry before an entry

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,const char *File,Word Line)
{
	LinkedListEntry_t *NewPtr;
	NewPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t),File,Line);
	if (NewPtr) {
		NewPtr->Data = Data;
		NewPtr->KillProc = 0;
		LinkedListAddEntryBefore(Input,EntryPtr,NewPtr);
	}
}


/**********************************

	Add an entry to the end of the list

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryProcEnd(LinkedList_t *Input,void *Data,LinkedListDeleteProcPtr Kill,const char *File,Word Line)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t),File,Line);
	if (EntryPtr) {
		EntryPtr->Data = Data;
		EntryPtr->KillProc = Kill;
		LinkedListAddEntryEnd(Input,EntryPtr);
	}
}

/**********************************

	Add an entry to the beginning

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryProcBegin(LinkedList_t *Input,void *Data,LinkedListDeleteProcPtr Kill,const char *File,Word Line)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t),File,Line);
	if (EntryPtr) {
		EntryPtr->Data = Data;
		EntryPtr->KillProc = Kill;
		LinkedListAddEntryBegin(Input,EntryPtr);
	}
}

/**********************************

	Add a simple entry after an entry

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryProcAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,LinkedListDeleteProcPtr Kill,const char *File,Word Line)
{
	LinkedListEntry_t *NewPtr;
	NewPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t),File,Line);
	if (NewPtr) {
		NewPtr->Data = Data;
		NewPtr->KillProc = Kill;
		LinkedListAddEntryAfter(Input,EntryPtr,NewPtr);
	}
}

/**********************************

	Add a simple entry before an entry

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryProcBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,LinkedListDeleteProcPtr Kill,const char *File,Word Line)
{
	LinkedListEntry_t *NewPtr;
	NewPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t),File,Line);
	if (NewPtr) {
		NewPtr->Data = Data;
		NewPtr->KillProc = Kill;
		LinkedListAddEntryBefore(Input,EntryPtr,NewPtr);
	}
}

/**********************************

	Add an entry to the end of the list

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryMemEnd(LinkedList_t *Input,void *Data,const char *File,Word Line)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t),File,Line);
	if (EntryPtr) {
		EntryPtr->Data = Data;
		EntryPtr->KillProc = LinkedListEntryDeallocProc;
		LinkedListAddEntryEnd(Input,EntryPtr);
	}
}

/**********************************

	Add an entry to the beginning of the list

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryMemBegin(LinkedList_t *Input,void *Data,const char *File,Word Line)
{
	LinkedListEntry_t *EntryPtr;
	EntryPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t),File,Line);
	if (EntryPtr) {
		EntryPtr->Data = Data;
		EntryPtr->KillProc = LinkedListEntryDeallocProc;
		LinkedListAddEntryBegin(Input,EntryPtr);
	}
}

/**********************************

	Add an entry after an entry

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryMemAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,const char *File,Word Line)
{
	LinkedListEntry_t *NewPtr;
	NewPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t),File,Line);
	if (NewPtr) {
		NewPtr->Data = Data;
		NewPtr->KillProc = LinkedListEntryDeallocProc;
		LinkedListAddEntryAfter(Input,EntryPtr,NewPtr);
	}
}

/**********************************

	Add an entry before an entry

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryMemBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,const char *File,Word Line)
{
	LinkedListEntry_t *NewPtr;
	NewPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t),File,Line);
	if (NewPtr) {
		NewPtr->Data = Data;
		NewPtr->KillProc = LinkedListEntryDeallocProc;
		LinkedListAddEntryBefore(Input,EntryPtr,NewPtr);
	}
}

/**********************************

	Add an entry to the end of the list

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryStringEnd(LinkedList_t *Input,const char *Data,const char *File,Word Line)
{
	LinkedListEntry_t *EntryPtr;
	Word Length;
	Length = strlen(Data)+1;
	EntryPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t)+Length,File,Line);
	if (EntryPtr) {
		FastMemCpy((EntryPtr+1),Data,Length);
		EntryPtr->Data = EntryPtr+1;
		EntryPtr->KillProc = 0;
		LinkedListAddEntryEnd(Input,EntryPtr);
	}
}

/**********************************

	Add an entry to the beginning of the list

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryStringBegin(LinkedList_t *Input,const char *Data,const char *File,Word Line)
{
	LinkedListEntry_t *EntryPtr;
	Word Length;
	Length = strlen(Data)+1;
	EntryPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t)+Length,File,Line);
	if (EntryPtr) {
		FastMemCpy((EntryPtr+1),Data,Length);
		EntryPtr->Data = EntryPtr+1;
		EntryPtr->KillProc = 0;
		LinkedListAddEntryBegin(Input,EntryPtr);
	}
}

/**********************************

	Add an entry after an entry

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryStringAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,const char *Data,const char *File,Word Line)
{
	LinkedListEntry_t *NewPtr;
	Word Length;
	Length = strlen(Data)+1;
	NewPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t)+Length,File,Line);
	if (NewPtr) {
		FastMemCpy((NewPtr+1),Data,Length);
		NewPtr->Data = NewPtr+1;
		NewPtr->KillProc = 0;
		LinkedListAddEntryAfter(Input,EntryPtr,NewPtr);
	}
}

/**********************************

	Add an entry before an entry

**********************************/

void BURGERCALL DebugLinkedListAddNewEntryStringBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,const char *Data,const char *File,Word Line)
{
	LinkedListEntry_t *NewPtr;
	Word Length;
	Length = strlen(Data)+1;
	NewPtr = (LinkedListEntry_t *)DebugAllocAPointer(sizeof(LinkedListEntry_t)+Length,File,Line);
	if (NewPtr) {
		FastMemCpy((NewPtr+1),Data,Length);
		NewPtr->Data = NewPtr+1;
		NewPtr->KillProc = 0;
		LinkedListAddEntryBefore(Input,EntryPtr,NewPtr);
	}
}
