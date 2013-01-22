#include "LsList.h"
#include "ClStdLib.h"
#include "MmMemory.h"
#include "PfPrefs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**********************************

	Initialize a ListFixed_t structure

**********************************/

void BURGERCALL ListFixedInit(ListFixed_t *Input,char *FirstItem,Word Size,Word Count)
{
	Input->FirstItemTextPtr = FirstItem;	/* Save the pointer to the array */
	Input->ItemSize = Size;		/* Save the size of each array element */
	Input->NumItems = Count;	/* Number of items */
}


/**********************************

	Find a string in a ListFixed_t array
	and return the index number

	Use a case insensitive search

**********************************/

Word BURGERCALL ListFixedFind(const ListFixed_t *Input,const char *ItemText)
{
	char *WorkPtr;		/* Data pointer */
	Word i;
	Word Count;

	WorkPtr = Input->FirstItemTextPtr;	/* Get the array pointer */
	if (WorkPtr) {			/* Is the pointer valid? */
		Count = Input->NumItems;
		if (Count) {	/* Are there any entries? */
			i = 0;		/* Start at the begining */
			do {
				if (!stricmp(WorkPtr,ItemText)) {		/* Match? */
					return i;		/* Return it! */
				}
				WorkPtr += Input->ItemSize;		/* Next string */
			} while (++i<Count);	/* Done yet? */
		}
	}
	return (Word)-1;		/* Bogus dude! */
}


/**********************************

	Given an index into the list, return
	the pointer to the string.

**********************************/

char * BURGERCALL ListFixedGetString(const ListFixed_t *Input,Word Index)
{
	char *Result;

	Result = Input->FirstItemTextPtr;		/* Is there a pointer? */
	if (Result && Index<Input->NumItems) {	/* In bounds? */
		Result = Result + (Index * Input->ItemSize);	/* Index to the string */
		if (Result[0]) {		/* Is the string valid? */
			return Result;		/* Return the string */
		}
	}
	return 0;		/* String not found */
}

/**********************************

	Initialize a ListStatic_t structure

**********************************/

void BURGERCALL ListStaticInit(ListStatic_t *Input,char **FirstItem,Word Size,Word Count)
{
	Input->FirstItemTextArrayPtr = FirstItem;	/* Save the pointer to the char array */
	Input->ItemSize = Size;		/* Size of each element in the array */
	Input->NumItems = Count;	/* Number of items in the array */
}


/**********************************

	Find a string in a ListStatic_t array
	and return the index number

	Use a case insensitive search

**********************************/

Word BURGERCALL ListStaticFind(const ListStatic_t *Input,const char *ItemText)
{
	char **WorkPtrPtr;
	Word i;
	Word Count;

	WorkPtrPtr = Input->FirstItemTextArrayPtr;	/* Get the char * */
	if (WorkPtrPtr) {			/* Valid? */
		Count = Input->NumItems;
		if (Count) {	/* Valid char pointer? */
			i = 0;		/* Init the count */
			do {
				if (!stricmp(WorkPtrPtr[0],ItemText)) {		/* Match? */
					return i;			/* Return the index */
				}
				WorkPtrPtr = (char **) (((char *)WorkPtrPtr)+Input->ItemSize);	/* Next entry */
			} while (++i<Count);		/* All done? */
		}
	}
	return (Word)-1;	/* Bogus dude! */
}


/**********************************

	Given an index into the list, return
	the pointer to the string.

**********************************/

char * BURGERCALL ListStaticGetString(const ListStatic_t *Input,Word Index)
{
	char **WorkPtrPtr;
	WorkPtrPtr = Input->FirstItemTextArrayPtr;
	if (WorkPtrPtr && Index<Input->NumItems) {
		return ((char **) ((char *)WorkPtrPtr + (Index * Input->ItemSize)) )[0];
	}
	return 0;		/* No good! */
}


/**********************************

	Initialize a ListDynamic_t structure

**********************************/

void BURGERCALL ListDynamicInit(ListDynamic_t *Input)
{
	Input->ArrayHandle = 0;		/* No handle present */
	Input->NumItems = 0;		/* No data present */
}

/**********************************

	Find a string in a ListStatic_t array
	and return the index number

	Use a case insensitive search

**********************************/

Word BURGERCALL ListDynamicFind(const ListDynamic_t *Input,const char *ItemText)
{
	Word i,Count;
	char **WorkPtrPtr;

	Count = Input->NumItems;			/* Any entries? */
	if (Count) {
		WorkPtrPtr = ((char ***)Input->ArrayHandle)[0];		/* Deref the handle */
		i = 0;		/* Reset index */
		do {
			if (!stricmp(WorkPtrPtr[0],ItemText)) {	/* Match? */
				return i;		/* Return the index */
			}
			++WorkPtrPtr;		/* Next pointer */
		} while (++i<Count);		/* All done? */
	}
	return (Word)-1;		/* Bogus dude! */
}

/**********************************

	Given an index to the array, return the
	pointer to the string

**********************************/

char * BURGERCALL ListDynamicGetString(const ListDynamic_t *Input,Word Index)
{
	if (Index < Input->NumItems) {		/* Valid input? */
		return ((char ***)Input->ArrayHandle)[0][Index];	/* Return the pointer */
	}
	return 0;		/* No good! */
}


/**********************************

	Destroy the contents of a ListDynamic_t structure

**********************************/

void BURGERCALL ListDynamicDestroy(ListDynamic_t *Input)
{
	Word Count;
	char **WorkPtrPtr;

	Count = Input->NumItems;
	if (Count) {			/* Any entries found? */
		WorkPtrPtr = (char **)(Input->ArrayHandle[0]);	/* Get the handle pointer */
		do {
			DeallocAPointer(WorkPtrPtr[0]);		/* Dispose of the memory */
			++WorkPtrPtr;						/* Next pointer */
		} while (--Count); 						/* Count down */
		DeallocAHandle(Input->ArrayHandle);		/* Get rid of the parent */
		Input->ArrayHandle = 0;					/* No handle */
		Input->NumItems = 0;					/* No count */
	}
}

/**********************************

	Add a new "C" string into a ListDynamic_t array

**********************************/

void BURGERCALL ListDynamicAdd(ListDynamic_t *Input,char *ItemText)
{
	char *TempPtr;		/* Pointer to the new added string */
	Word Len;

	if (ItemText) {		/* Valid input pointer */
		Len = strlen(ItemText)+1;
		TempPtr = (char *)AllocAPointer(Len);		/* Get space for the new string */
		if (TempPtr) {						/* Did I get the memory? */
			Input->ArrayHandle = ResizeAHandle(Input->ArrayHandle,(Input->NumItems+1)*sizeof(char *));
			if (Input->ArrayHandle) {					/* Did it resize? */
				((char ***)Input->ArrayHandle)[0][Input->NumItems] = TempPtr;
				++Input->NumItems;				/* Increase the item count */
				FastMemCpy(TempPtr,ItemText,Len);	/* Copy the string */
				return;						/* Cool! */
			}
			DeallocAPointer(TempPtr);		/* Dispose of the memory */
			Input->NumItems = 0;			/* No data is present */
		}
	}
}


/**********************************

	Find a string in the dynamic list and
	then remove it

**********************************/

void BURGERCALL ListDynamicRemoveString(ListDynamic_t *Input,const char *ItemText)
{
	Word Index;
	Index = ListDynamicFind(Input,ItemText);	/* Find the string */
	if (Index!=(Word)-1) {			/* Found? */
		ListDynamicRemoveIndex(Input,Index);	/* Remove it */
	}
}

/**********************************

	Remove an item from the dynamic list

**********************************/

void BURGERCALL ListDynamicRemoveIndex(ListDynamic_t *Input,Word Index)
{
	char **WorkPtr;
	Word Count;

	if (Index<Input->NumItems) {	/* Valid index? */
		WorkPtr = (char **)(Input->ArrayHandle[0]);
		DeallocAPointer((void *)(WorkPtr[Index]));	/* Dispose of the data */

		Input->NumItems--;
		Count = Input->NumItems-Index;	/* Number of bytes to move */
		if (Count) {
			WorkPtr = (char **)(Input->ArrayHandle[0]);	/* In case it moved! */
			memmove(&WorkPtr[Index],&WorkPtr[Index+1],Count*sizeof(char *));
		}
		Input->ArrayHandle = ResizeAHandle(Input->ArrayHandle,Input->NumItems*sizeof(char *));
	}
}


/**********************************

	Initialize a ListIntRange_t structure

**********************************/

void BURGERCALL ListIntRangeInit(ListIntRange_t *Input,int MinVal,int MaxVal)
{
	if (MinVal>MaxVal) {	/* Make sure min and max are sorted! */
		int Foo;			/* Swap them! */
		Foo = MaxVal;
		MaxVal = MinVal;
		MinVal = Foo;
	}
	Input->MinVal = MinVal;		/* Save the values */
	Input->MaxVal = MaxVal;
	Input->WorkString[0] = 0;	/* Init the work string */
}


/**********************************

	Given a string, convert it to an int.

**********************************/

Word BURGERCALL ListIntRangeFind(const ListIntRange_t * /* Input */,const char *ItemText)
{
	return AsciiToLongWord(ItemText);		/* Simple conversion */
}

/**********************************

	Given an index, convert it to ascii

**********************************/

char * BURGERCALL ListIntRangeGetString(ListIntRange_t *Input,Word Index)
{
	longToAscii(Index,Input->WorkString);
	return Input->WorkString;		/* Return the pointer to the string */
}

/**********************************

	Initialize a ListFixedRange_t structure

**********************************/

void BURGERCALL ListFixedRangeInit(ListFixedRange_t *Input,Fixed32 MinVal,Fixed32 MaxVal,Fixed32 StepVal)
{
	if (MinVal>MaxVal) {		/* Sort min and max */
		int Foo;				/* Swap'em */
		Foo = MaxVal;
		MaxVal = MinVal;
		MinVal = Foo;
	}
	Input->MinVal = MinVal;		/* Save the min */
	Input->MaxVal = MaxVal;
	Input->StepVal = StepVal;		/* Save the step */
	Input->WorkString[0] = 0;	/* Init the work string */
}
/**********************************

	Convert a string to fixed point

**********************************/

Fixed32 BURGERCALL ListFixedRangeFind(const ListFixedRange_t * /* Input */,const char *ItemText)
{
	float Temp;
	Temp = (float) atof(ItemText);		/* Convert the ASCII */
	Temp = Temp*65536.0f;
	return (Fixed32)((long)Temp);
}

/**********************************

	Convert a fixed point number into a string

**********************************/

char * BURGERCALL ListFixedRangeGetString(ListFixedRange_t *Input,Fixed32 Index)
{
	float Temp;
	Temp = (float)Index;
	Temp = Temp/65536.0f;
	sprintf(Input->WorkString,"%g",Temp);		/* Convert to ascii */
	return Input->WorkString;			/* Return the string */
}

/**********************************

	Initialize a ListFloatRange_t structure

**********************************/

void BURGERCALL ListFloatRangeInit(ListFloatRange_t *Input,float MinVal,float MaxVal,float Step)
{
	if (MinVal>MaxVal) {	/* Make SURE the min and max are sorted! */
		float Foo;
		Foo = MaxVal;		/* Swap them! */
		MaxVal = MinVal;
		MinVal = Foo;
	}
	Input->MinVal = MinVal;		/* Save the minimum for the range */
	Input->MaxVal = MaxVal;		/* Save the maximum for the range */
	Input->StepVal = Step;		/* Save the step value */
	Input->WorkString[0] = 0;	/* Nothing in the work string */
}

/**********************************

	Convert a string into an element in the list

**********************************/

float BURGERCALL ListFloatRangeFind(const ListFloatRange_t * /* Input */,const char *ItemText)
{
	return (float) atof(ItemText);		/* Convert the ASCII */
}

/**********************************

	Convert an index into a string
	for a floating point list

**********************************/

char * BURGERCALL ListFloatRangeGetString(ListFloatRange_t *Input,float Index)
{
	sprintf(Input->WorkString,"%g",Index);		/* Convert to ascii */
	return Input->WorkString;			/* Return the string */
}


