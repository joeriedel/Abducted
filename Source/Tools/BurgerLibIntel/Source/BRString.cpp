/**********************************

	This is the Burgerlib C++ string
	class. I will not trigger exceptions
	on memory errors. However, I do take great
	care in making sure that the class
	structure is in a valid state at all times

**********************************/

#include <ctype.h>
#include "BRString.h"
#include "CLStdLib.h"
#include "StString.h"

static const char NullString[1] = {0};
char BRString::Dummy;

/**********************************

	Init to an empty string

**********************************/

void BURGERCALL BRStringInit(BRString_t *Output)
{
	Output->DataPtr = Output->Raw;	/* Init the string pointer */
	Output->StrLength = 0;			/* No data present */
	Output->Flags = 0;				/* Case sensitive */
	Output->Raw[0] = 0;				/* Init the "C" string */
}

/**********************************

	Initialize a BString with a BString

**********************************/

void BURGERCALL BRStringInitBRString(BRString_t *Output,const BRString_t *Input)
{
	Word Len;
	char *WorkPtr;
	Len = Input->StrLength;		/* Get the source length */
	Output->StrLength=(Word16)Len; 		/* Save the new length */
	Output->Flags = 0;					/* Case sensitive */
	if (Len<sizeof(Output->Raw)) {		/* Buffer big enough? */
		WorkPtr = Output->Raw;
		FastMemCpy(WorkPtr,Input->DataPtr,Len+1);	/* Copy the string */
	} else {
		WorkPtr = StrCopy(Input->DataPtr);	/* Make a true string copy */
		if (!WorkPtr) {			/* Oh oh... */
			WorkPtr = Output->Raw;
			Output->StrLength = 0;		/* Failsafe! */
			WorkPtr[0] = 0;
		}
	}
	Output->DataPtr = WorkPtr;		/* Set the pointer */
}

/**********************************

	Initialize a BString by using a subsection of
	a different BString

**********************************/

void BURGERCALL BRStringInitBRStringRange(BRString_t *Output,const BRString_t *Input,Word Start,Word End)
{
	Word Len;
	char *WorkPtr;
	Output->Flags = 0;					/* Case sensitive */
	Len = Input->StrLength;		/* Get the source length */
	if (End>Len) {				/* Clamp the end of the string */
		End = Len;				/* Make sure it fits */
	}
	if (Start>=End) {			/* Valid range? */
		goto KillIt;
	}
	Len = End-Start;			/* Length of the new string */
	Output->StrLength=(Word16)Len;		/* Save the new length */
	if (Len>=sizeof(Output->Raw)) {		/* Buffer big enough? */
		WorkPtr = (char *)AllocAPointer(Len+1);	/* Make a true string copy */
		if (!WorkPtr) {			/* Oh oh... */
KillIt:;
			Output->DataPtr = Output->Raw;		/* Failsafe */
			Output->StrLength = 0;		/* Zark the string */
			Output->Raw[0] = 0;
			return;
		}
	} else {
		WorkPtr = Output->Raw;			/* Use the default buffer */
	}
	WorkPtr[Len] = 0;
	Output->DataPtr = WorkPtr;		/* Set the pointer */
	FastMemCpy(WorkPtr,Input->DataPtr+Start,Len);	/* Copy the string */
}

/**********************************

	Initialize a BString with a "C" string

**********************************/

void BURGERCALL BRStringInitPChar(BRString_t *Output,const char *Input)
{
	Word Len;
	char *WorkPtr;
	Output->Flags = 0;					/* Case sensitive */
	if (!Input) {		/* No input? */
		goto KillIt;
	}
	Len = strlen(Input);		/* Get the source length */
	Output->StrLength=(Word16)Len;		/* Save the new length */
	if (Len<sizeof(Output->Raw)) {		/* Buffer big enough? */
		WorkPtr = Output->Raw;
		Output->DataPtr = WorkPtr;		/* Set the pointer */
		FastMemCpy(WorkPtr,Input,Len+1);	/* Copy the string */
	} else {
		WorkPtr = StrCopy(Input);	/* Make a true string copy */
		if (!WorkPtr) {			/* Oh oh... */
KillIt:;
			WorkPtr = Output->Raw;
			Output->StrLength = 0;		/* Failsafe! */
			WorkPtr[0] = 0;
		}
		Output->DataPtr = WorkPtr;
	}
}

/**********************************

	Initialize a BString by using a subsection of
	a different "C" string

**********************************/

void BURGERCALL BRStringInitPCharRange(BRString_t *Output,const char *Input,Word Start,Word End)
{
	Word Len;
	char *WorkPtr;
	Output->Flags = 0;					/* Case sensitive */
	if (!Input) {				/* Invalid string? */
		goto KillIt;			/* Kill the string now */
	}
	Len = strlen(Input);		/* Get the source length */
	if (End>Len) {				/* Clamp the end of the string */
		End = Len;				/* Make sure it fits */
	}
	if (Start>=End) {			/* Valid range? */
		goto KillIt;
	}
	Len = End-Start;			/* Length of the new string */
	Output->StrLength=(Word16)Len;	/* Save the new length */
	if (Len>=sizeof(Output->Raw)) {		/* Buffer big enough? */
		WorkPtr = (char *)AllocAPointer(Len+1);	/* Make a true string copy */
		if (!WorkPtr) {			/* Oh oh... */
KillIt:;
			Output->DataPtr = Output->Raw;		/* Failsafe */
			Output->StrLength = 0;		/* Zark the string */
			Output->Raw[0] = 0;
			return;
		}
	} else {
		WorkPtr = Output->Raw;			/* Use the default buffer */
	}
	WorkPtr[Len] = 0;
	Output->DataPtr = WorkPtr;		/* Set the pointer */
	FastMemCpy(WorkPtr,Input+Start,Len);	/* Copy the string */
}

/**********************************

	Init to a BString of a single character

**********************************/

void BURGERCALL BRStringInitChar(BRString_t *Output,char Input)
{
	char *WorkPtr;
	Word Len;
	WorkPtr = Output->Raw;			/* Get the dest pointer */
	Len = 0;
	WorkPtr[0] = Input;		/* Store the char in the string */
	WorkPtr[1] = (char)Len;
	if (Input) {			/* Valid string? */
		Len = 1;			/* 1 char long */
	}
	Output->Flags = 0;				/* Case sensitive */
	Output->StrLength = (Word16)Len;	/* Save the string's length */
	Output->DataPtr = WorkPtr;		/* Store the string pointer */
}

/**********************************

	Init to a BString filled with a single character

**********************************/

void BURGERCALL BRStringInitChar2(BRString_t *Output,char Input,Word FillSize)
{
	char *WorkPtr;
	Word InByte;
	Output->Flags = 0;					/* Case sensitive */
	InByte = (Word8)Input;
	if (InByte && FillSize) {		/* Valid input? */
		if (FillSize<sizeof(Output->Raw)) {
			WorkPtr = Output->Raw;			/* Use the internal buffer */
		} else {
			WorkPtr = (char *)AllocAPointer(FillSize+1);	/* Allocate a buffer */
			if (!WorkPtr) {			/* Oh crap */
				goto KillIt;
			}
		}
		Output->StrLength = (Word16)FillSize;	/* Save the new length */
		Output->DataPtr = WorkPtr;			/* Save the pointer */
		WorkPtr[FillSize] = 0;		/* Zero terminate */
		FastMemSet(WorkPtr,InByte,FillSize);	/* Fill the string */
		return;
	}
KillIt:;
	Output->DataPtr = Output->Raw;		/* Failsafe */
	Output->StrLength = 0;
	Output->Raw[0] = 0;
}

/**********************************

	Initialize a BString with two concatenated "C" strings

**********************************/

void BURGERCALL BRStringInitPChar2(BRString_t *Output,const char *Input1,const char *Input2)
{
	Word Len1,Len2;
	Word Total;
	char *WorkPtr;

	Output->Flags = 0;					/* Case sensitive */
	if (!Input1) {			/* Get the length of string #1 */
		Len1 = 0;
	} else {
		Len1 = strlen(Input1);
	}

	if (!Input2) {			/* Get the length of string #2 */
		Len2 = 0;
	} else {
		Len2 = strlen(Input2);
	}
	Total = Len1+Len2;		/* Size of the finished string */

	Output->StrLength=(Word16)Total;			/* Save the new length */
	if (Total<sizeof(Output->Raw)) {		/* Buffer big enough? */
		WorkPtr = Output->Raw;				/* Use my buffer */
	} else {
		WorkPtr = (char *)AllocAPointer(Total+1);	/* Get a buffer */
		if (!WorkPtr) {
			Output->DataPtr = Output->Raw;		/* Oh oh... */
			Output->StrLength = 0;		/* Failsafe! */
			Output->Raw[0] = 0;
			return;
		}
	}
	Output->DataPtr = WorkPtr;		/* Set the pointer */
	WorkPtr[Total] = 0;		/* End the string */
	FastMemCpy(WorkPtr,Input1,Len1);	/* Copy the string */
	FastMemCpy(WorkPtr+Len1,Input2,Len2);	/* Copy the next string */
}

/**********************************

	Initialize a BString with three concatenated "C" strings

**********************************/

void BURGERCALL BRStringInitPChar3(BRString_t *Output,const char *Input1,const char *Input2,const char *Input3)
{
	Word Len1,Len2,Len3;
	Word Total;
	char *WorkPtr;

	Output->Flags = 0;					/* Case sensitive */
	if (!Input1) {			/* Get the length of string #1 */
		Len1 = 0;
	} else {
		Len1 = strlen(Input1);
	}

	if (!Input2) {			/* Get the length of string #2 */
		Len2 = 0;
	} else {
		Len2 = strlen(Input2);
	}

	if (!Input3) {			/* Get the length of string #3 */
		Len3 = 0;
	} else {
		Len3 = strlen(Input3);
	}
	Total = Len1+Len2+Len3;		/* Size of the finished string */

	Output->StrLength=(Word16)Total;				/* Save the new length */
	if (Total<sizeof(Output->Raw)) {		/* Buffer big enough? */
		WorkPtr = Output->Raw;				/* Use my buffer */
	} else {
		WorkPtr = (char *)AllocAPointer(Total+1);	/* Get a buffer */
		if (!WorkPtr) {
			Output->DataPtr = Output->Raw;		/* Oh oh... */
			Output->StrLength = 0;		/* Failsafe! */
			Output->Raw[0] = 0;
			return;
		}
	}
	Output->DataPtr = WorkPtr;		/* Set the pointer */
	WorkPtr[Total] = 0;		/* End the string */
	FastMemCpy(WorkPtr,Input1,Len1);	/* Copy the string */
	WorkPtr+=Len1;
	FastMemCpy(WorkPtr,Input2,Len2);	/* Copy the next string */
	WorkPtr+=Len2;
	FastMemCpy(WorkPtr,Input3,Len3);
}

/**********************************

	Initialize a BString with four concatenated "C" strings

**********************************/

void BURGERCALL BRStringInitPChar4(BRString_t *Output,const char *Input1,const char *Input2,const char *Input3,const char *Input4)
{
	Word Len1,Len2,Len3,Len4;
	Word Total;
	char *WorkPtr;

	Output->Flags = 0;					/* Case sensitive */
	if (!Input1) {			/* Get the length of string #1 */
		Len1 = 0;
	} else {
		Len1 = strlen(Input1);
	}

	if (!Input2) {			/* Get the length of string #2 */
		Len2 = 0;
	} else {
		Len2 = strlen(Input2);
	}

	if (!Input3) {			/* Get the length of string #3 */
		Len3 = 0;
	} else {
		Len3 = strlen(Input3);
	}

	if (!Input4) {			/* Get the length of string #3 */
		Len4 = 0;
	} else {
		Len4 = strlen(Input4);
	}
	Total = Len1+Len2+Len3+Len4;		/* Size of the finished string */

	Output->StrLength=(Word16)Total;			/* Save the new length */
	if (Total<sizeof(Output->Raw)) {		/* Buffer big enough? */
		WorkPtr = Output->Raw;				/* Use my buffer */
	} else {
		WorkPtr = (char *)AllocAPointer(Total+1);	/* Get a buffer */
		if (!WorkPtr) {
			Output->DataPtr = Output->Raw;		/* Oh oh... */
			Output->StrLength = 0;		/* Failsafe! */
			Output->Raw[0] = 0;
			return;
		}
	}
	Output->DataPtr = WorkPtr;		/* Set the pointer */
	WorkPtr[Total] = 0;		/* End the string */
	FastMemCpy(WorkPtr,Input1,Len1);	/* Copy the string */
	WorkPtr+=Len1;
	FastMemCpy(WorkPtr,Input2,Len2);	/* Copy the next string */
	WorkPtr+=Len2;
	FastMemCpy(WorkPtr,Input3,Len3);
	WorkPtr+=Len3;
	FastMemCpy(WorkPtr,Input4,Len4);
}

/**********************************

	Dispose of the string

**********************************/

void BURGERCALL BRStringDestroy(BRString_t *Input)
{
	if (Input->DataPtr!=Input->Raw) {		/* Never initialized? */
		DeallocAPointer(Input->DataPtr);	/* Kill it off */
	}							/* Don't bother with any more clean up */
}

/**********************************

	Dispose of an allocated BRString_t string

**********************************/

void BURGERCALL BRStringDelete(BRString_t* Input)
{
	if (Input) {
		BRStringDestroy(Input);
		DeallocAPointer(Input);
	}
}

/**********************************

	Copy a BRString to a BString

**********************************/

void BURGERCALL BRStringCopyBRString(BRString_t *Output,const BRString_t *Input)
{
	if (Output!=Input) {		/* Am I copying myself? */
		Word Len;
		char *WorkPtr;

		WorkPtr = Output->DataPtr;
		if (WorkPtr!=Output->Raw) {		/* Discard previous memory */
			DeallocAPointer(WorkPtr);
		}

		Len = Input->StrLength;		/* Length of the new string */
		Output->StrLength = (Word16)Len;		/* Copy the new length */
		if (Len<sizeof(Output->Raw)) {
			WorkPtr = Output->Raw;
		} else {
			WorkPtr = (char*)AllocAPointer(Len+1);		/* New buffer */
			if (!WorkPtr) {
				Output->DataPtr = Output->Raw;		/* Oh oh... */
				Output->StrLength = 0;		/* Failsafe! */
				Output->Raw[0] = 0;
				return;
			}
		}
		Output->DataPtr = WorkPtr;
		FastMemCpy(WorkPtr,Input->DataPtr,Len+1);	/* Copy the string */
	}
}

/**********************************

	Copy a "C" to a BString
	Note : This is tricky, since sometimes the input
	string is a subsection of the output string.

**********************************/

void BURGERCALL BRStringCopyPChar(BRString_t *Output,const char *Input)
{
	char *OhFoo;
	Word Len;
	char *WorkPtr;
	char RatBert[256];

	if (!Input) {				/* Zap the input? */
		if (Output->DataPtr!=Output->Raw) {		/* Kill the previous string if needed */
			DeallocAPointer(Output->DataPtr);
		}
		Output->DataPtr = Output->Raw;			/* Empty now */
		Output->Raw[0] = 0;
		Output->StrLength = 0;
	} else {
		Len = strlen(Input);		/* Length of the new string */

		/* This is to prevent a string overwrite if you are using */
		/* yourself as input */

		OhFoo = 0;					/* Assume I don't have to copy */
		if ((Word32)(Input-Output->DataPtr)<Output->StrLength) {	/* Is this in my dest string already? */
			if (Len<sizeof(RatBert)) {		/* Copy to a local buffer? */
				strcpy(RatBert,Input);		/* Yep! */
				Input = RatBert;			/* Use this buffer instead */
			} else {
				OhFoo = StrCopy(Input);		/* Copy the string outright */
				Input=OhFoo;				/* Use this as input */
			}
		}

		/* Dispose of any previous memory */

		WorkPtr = Output->DataPtr;
		if (WorkPtr!=Output->Raw) {		/* Discard previous memory */
			DeallocAPointer(WorkPtr);
		}
		Output->StrLength = (Word16)Len;		/* Copy the new length */

		if (Len<sizeof(Output->Raw)) {
			WorkPtr = Output->Raw;			/* New buffer */
		} else {
			WorkPtr = (char*)AllocAPointer(Len+1);		/* New buffer */
			if (!WorkPtr) {
				Output->DataPtr = Output->Raw;		/* Oh oh... */
				Output->StrLength = 0;		/* Failsafe! */
				Output->Raw[0] = 0;
				DeallocAPointer(OhFoo);
				return;
			}
		}
		Output->DataPtr = WorkPtr;
		FastMemCpy(WorkPtr,Input,Len+1);	/* Copy the string */
		DeallocAPointer(OhFoo);
	}
}

/**********************************

	Copy a char to a BString

**********************************/

void BURGERCALL BRStringCopyChar(BRString_t *Output,char Input)
{
	Word Len;
	char *WorkPtr;

	WorkPtr = Output->DataPtr;
	if (WorkPtr!=Output->Raw) {			/* Discard previous memory */
		DeallocAPointer(WorkPtr);
	}

	WorkPtr = Output->Raw;
	WorkPtr[0] = Input;
	Len = 1;		/* Length of the new string */
	if (!Input) {
		Len = 0;
	}
	WorkPtr[Len] = 0;
	Output->DataPtr = WorkPtr;
	Output->StrLength = (Word16)Len;		/* Copy the new length */
}

/**********************************

	Append a BRString with another BRString

**********************************/

void BURGERCALL BRStringAppendBRString(BRString_t *Output,const BRString_t *Input)
{
	Word Len1,Len2;
	Word Total;
	char *WorkPtr;
	char *DestPtr;

	Len2 = Input->StrLength;
	if (Len2) {						/* Do I bother? */
		Len1 = Output->StrLength;			/* Get the lengths of the two strings */
		Total = Len1+Len2;			/* New string size */

		Output->StrLength = (Word16)Total;	/* Save the new length */
		WorkPtr = Output->DataPtr;			/* Get the dest buffer */
		if (Total<sizeof(Output->Raw)) {
			FastMemCpy(WorkPtr+Len1,Input->DataPtr,Len2+1);	/* Simple concatination */
		} else {
			DestPtr = StrCopyPad(WorkPtr,Len2);				/* Make a new string */
			if (DestPtr) {
				FastMemCpy(DestPtr+Len1,Input->DataPtr,Len2+1);
				Output->DataPtr = DestPtr;					/* Save the new string (Must be AFTER the copy) */
			} else {
				Output->DataPtr = Output->Raw;				/* Zap the string */
				Output->Raw[0] = 0;
				Output->StrLength = 0;
			}
			if (WorkPtr!=Output->Raw) {						/* Get rid of the old string */
				DeallocAPointer(WorkPtr);
			}
		}
	}
}

/**********************************

	Append a BString with a "C" string

**********************************/

void BURGERCALL BRStringAppendPChar(BRString_t *Output,const char *Input)
{
	Word Len1,Len2;
	Word Total;
	char *WorkPtr;
	char *DestPtr;

	if (Input) {						/* Do I even bother? */
		Len2 = strlen(Input);			/* Get the length */
		if (Len2) {
			Len1 = Output->StrLength;	/* Get the lengths of the two strings */
			Total = Len1+Len2;			/* New string size */

			Output->StrLength = (Word16)Total;	/* Save the new length */
			WorkPtr = Output->DataPtr;			/* Get the dest buffer */
			if (Total<sizeof(Output->Raw)) {
				FastMemCpy(WorkPtr+Len1,Input,Len2+1);	/* Simple concatination */
			} else {
				DestPtr = StrCopyPad(WorkPtr,Len2);				/* Make a new string */
				if (DestPtr) {
					Output->DataPtr = DestPtr;					/* Save the new string (Must be AFTER the copy) */
					FastMemCpy(DestPtr+Len1,Input,Len2+1);
				} else {
					Output->DataPtr = Output->Raw;				/* Zap the string */
					Output->Raw[0] = 0;
					Output->StrLength = 0;
				}
				if (WorkPtr!=Output->Raw) {						/* Get rid of the old string */
					DeallocAPointer(WorkPtr);
				}
			}
		}
	}
}

/**********************************

	Append a BString with a char

**********************************/

void BURGERCALL BRStringAppendChar(BRString_t *Output,char Input)
{
	Word Len1;
	Word Total;
	char *WorkPtr;
	char *DestPtr;

	if (Input) {					/* Do I even bother? */
		Len1 = Output->StrLength;	/* Get the lengths of the two strings */
		Total = Len1+1;				/* New string size */

		Output->StrLength = (Word16)Total;	/* Save the new length */
		WorkPtr = Output->DataPtr;		/* Get the dest buffer */
		if (Total<sizeof(Output->Raw)) {
			WorkPtr[Len1] = Input;
			WorkPtr[Total] = 0;		/* Simple concatination */
		} else {
			DestPtr = StrCopyPad(WorkPtr,1);	/* Make a new string */
			if (!DestPtr) {
				DestPtr = Output->Raw;
				Len1 = 0;
				Output->StrLength = 0;
			}
			Output->DataPtr = DestPtr;		/* Save the new string (Must be AFTER the copy) */
			DestPtr[Len1] = Input;
			DestPtr[Len1+1] = 0;
			if (WorkPtr!=Output->Raw) {			/* Get rid of the old string */
				DeallocAPointer(WorkPtr);		/* Bye bye */
			}
		}
	}
}

/**********************************

	Return the result of strcmp

**********************************/

int BURGERCALL BRStringCompareBRString(const BRString_t *Input1,const BRString_t *Input2)
{
	int Result;
	if (Input1->Flags || Input2->Flags) {		/* Case insensitive? */
		Result = stricmp(Input1->DataPtr,Input2->DataPtr);
	} else {
		Result = strcmp(Input1->DataPtr,Input2->DataPtr);
	}
	return Result;
}

int BURGERCALL BRStringComparePChar(const BRString_t *Input1,const char *Input2)
{
	int Result;
	if (!Input2) {
		Input2 = NullString;
	}
	if (Input1->Flags) {
		Result = stricmp(Input1->DataPtr,Input2);
	} else {
		Result = strcmp(Input1->DataPtr,Input2);
	}
	return Result;
}

int BURGERCALL BRStringCompareChar(const BRString_t *Input1,char Input2)
{
	char TempStr[2];
	int Result;
	TempStr[0] = Input2;
	TempStr[1] = 0;
	if (Input1->Flags) {
		Result = stricmp(Input1->DataPtr,TempStr);
	} else {
		Result = strcmp(Input1->DataPtr,TempStr);
	}
	return Result;
}

/**********************************

	Test for string equality

**********************************/

Word BURGERCALL BRStringEqBString(const BRString_t *Input1,const BRString_t *Input2)
{
	if (!BRStringCompareBRString(Input1,Input2)) {
		return TRUE;
	}
	return FALSE;
}

Word BURGERCALL BRStringEqPChar(const BRString_t *Input1,const char *Input2)
{
	if (!BRStringComparePChar(Input1,Input2)) {
		return TRUE;
	}
	return FALSE;
}

Word BURGERCALL BRStringEqChar(const BRString_t *Input1,char Input2)
{
	if (!BRStringCompareChar(Input1,Input2)) {
		return TRUE;
	}
	return FALSE;
}

/**********************************

	Test for string inequality

**********************************/

Word BURGERCALL BRStringNeBString(const BRString_t *Input1,const BRString_t *Input2)
{
	if (BRStringCompareBRString(Input1,Input2)) {
		return TRUE;
	}
	return FALSE;
}

Word BURGERCALL BRStringNePChar(const BRString_t *Input1,const char *Input2)
{
	if (BRStringComparePChar(Input1,Input2)) {
		return TRUE;
	}
	return FALSE;
}

Word BURGERCALL BRStringNeChar(const BRString_t *Input1,char Input2)
{
	if (BRStringCompareChar(Input1,Input2)) {
		return TRUE;
	}
	return FALSE;
}

/**********************************

	Is string 1 < String 2?

**********************************/

Word BURGERCALL BRStringLtBString(const BRString_t *Input1,const BRString_t *Input2)
{
	if (BRStringCompareBRString(Input1,Input2)<0) {
		return TRUE;
	}
	return FALSE;
}

Word BURGERCALL BRStringLtPChar(const BRString_t *Input1,const char *Input2)
{
	if (BRStringComparePChar(Input1,Input2)<0) {
		return TRUE;
	}
	return FALSE;
}

Word BURGERCALL BRStringLtChar(const BRString_t *Input1,char Input2)
{
	if (BRStringCompareChar(Input1,Input2)<0) {
		return TRUE;
	}
	return FALSE;
}

/**********************************

	Is string 1 <= String 2?

**********************************/

Word BURGERCALL BRStringLeBString(const BRString_t *Input1,const BRString_t *Input2)
{
	if (BRStringCompareBRString(Input1,Input2)<=0) {
		return TRUE;
	}
	return FALSE;
}

Word BURGERCALL BRStringLePChar(const BRString_t *Input1,const char *Input2)
{
	if (BRStringComparePChar(Input1,Input2)<=0) {
		return TRUE;
	}
	return FALSE;
}

Word BURGERCALL BRStringLeChar(const BRString_t *Input1,char Input2)
{
	if (BRStringCompareChar(Input1,Input2)<=0) {
		return TRUE;
	}
	return FALSE;
}

/**********************************

	Is string 1 > String 2?

**********************************/

Word BURGERCALL BRStringGtBString(const BRString_t *Input1,const BRString_t *Input2)
{
	if (BRStringCompareBRString(Input1,Input2)>0) {
		return TRUE;
	}
	return FALSE;
}

Word BURGERCALL BRStringGtPChar(const BRString_t *Input1,const char *Input2)
{
	if (BRStringComparePChar(Input1,Input2)>0) {
		return TRUE;
	}
	return FALSE;
}

Word BURGERCALL BRStringGtChar(const BRString_t *Input1,char Input2)
{
	if (BRStringCompareChar(Input1,Input2)>0) {
		return TRUE;
	}
	return FALSE;
}

/**********************************

	Is string 1 >= String 2?

**********************************/

Word BURGERCALL BRStringGeBString(const BRString_t *Input1,const BRString_t *Input2)
{
	if (BRStringCompareBRString(Input1,Input2)>=0) {
		return TRUE;
	}
	return FALSE;
}

Word BURGERCALL BRStringGePChar(const BRString_t *Input1,const char *Input2)
{
	if (BRStringComparePChar(Input1,Input2)>=0) {
		return TRUE;
	}
	return FALSE;
}

Word BURGERCALL BRStringGeChar(const BRString_t *Input1,char Input2)
{
	if (BRStringCompareChar(Input1,Input2)>=0) {
		return TRUE;
	}
	return FALSE;
}

/**********************************

	Return a single character from the string
	I will do bounds checking

**********************************/

char BURGERCALL BRStringGet(const BRString_t *Input,Word Where)
{
	if (Where<Input->StrLength) {			/* In bounds? */
		return Input->DataPtr[Where];		/* Fetch it */
	}
	return 0;								/* Return zilch */
}

/**********************************

	Modify a single character in the
	string, this routine will do bounds
	checking

**********************************/

void BURGERCALL BRStringPut(BRString_t *Output,Word Where,char Input)
{
	if (Where<Output->StrLength) {			/* In bounds? */
		Output->DataPtr[Where] = Input;		/* Store it */
	}
}

/**********************************

	Scan for a sub string

**********************************/

char * BURGERCALL BRStringstrstr(const BRString_t *Input1,const char *Input2)
{
	if (Input2) {
		if (!Input1->Flags) {		/* Case sensitive? */
			return strstr(Input1->DataPtr,Input2);
		}
		return stristr(Input1->DataPtr,Input2);		/* Case insensitive? */
	}
	return 0;
}

/**********************************

	Convert a string to lower case

**********************************/

void BURGERCALL BRStringToLowerCase(BRString_t *Input)
{
	Word i;
	i = Input->StrLength;					/* Get the length to convert */
	if (i) {
		Word8 *WorkPtr;
		WorkPtr = (Word8 *)Input->DataPtr;	/* Get the buffer pointer */
		do {
			Word Temp;
			Temp = WorkPtr[0];
			if (Temp>='A' && Temp<='Z') {	/* Upper case? */
				Temp += 32;					/* Make lower case */
				WorkPtr[0] = (Word8)Temp;	/* Save it */
			}
			++WorkPtr;					/* Next byte */
		} while (--i);					/* All done? */
	}
}

/**********************************

	Convert a string to upper case

**********************************/

void BURGERCALL BRStringToUpperCase(BRString_t *Input)
{
	Word i;
	i = Input->StrLength;					/* Get the length to convert */
	if (i) {
		Word8 *WorkPtr;
		WorkPtr = (Word8 *)Input->DataPtr;	/* Get the buffer pointer */
		do {
			Word Temp;
			Temp = WorkPtr[0];
			if (Temp>='a' && Temp<='z') {	/* Lower case? */
				Temp -= 32;					/* Make upper case */
				WorkPtr[0] = (Word8)Temp;	/* Save it */
			}
			++WorkPtr;					/* Next byte */
		} while (--i);					/* All done? */
	}
}

/**********************************

	Return true if the string contains
	anything

**********************************/

Word BURGERCALL BRStringValid(const BRString_t *Input)
{
	if (!Input->StrLength) {		/* Any data at all? */
		return FALSE;
	}
	return TRUE;
}

/**********************************

	Clear out the data in a BRString

**********************************/

void BURGERCALL BRStringClear(BRString_t *Input)
{
	char *WorkPtr;
	WorkPtr = Input->DataPtr;		/* Old data pointer */
	Input->Raw[0] = 0;				/* Kill the string */
	Input->StrLength = 0;			/* No length */
	Input->DataPtr = Input->Raw;	/* New pointer */
	if (WorkPtr!=Input->Raw) {		/* Dispose of the data */
		DeallocAPointer(WorkPtr);	/* Kill the old memory */
	}
}

/**********************************

	Extract the string into a buffer of a specific size
	(This will truncate the string if the buffer is too small)

**********************************/

Word BURGERCALL BRStringCopy(const BRString_t *Input,char *Output,Word MaxLen)
{
	if (MaxLen) {					/* Is there an output buffer? */
		Word Length;				/* Local */
		Length = Input->StrLength;	/* Get the size of the current string */
		if (MaxLen>Length) {		/* Set the maximum */
			MaxLen = Length;		/* Truncate */
		}
		--MaxLen;					/* Max size of the "C" string */
		Output[MaxLen] = 0;			/* Zero terminate */
		if (MaxLen) {
			FastMemCpy(Output,Input->DataPtr,MaxLen);	/* Copy the string */
		}
	}
	return MaxLen;
}

/**********************************

	Extract the string into a buffer of a specific size
	(This will truncate the string if the buffer is too small)

**********************************/

Word BURGERCALL BRStringPCopy(const BRString_t *Input,Word8 *Output,Word MaxLen)
{
	if (MaxLen) {					/* Is there an output buffer? */
		Word Length;				/* Local */
		if (MaxLen>256) {			/* Pascal strings maximum length */
			MaxLen = 256;
		}
		--MaxLen;					/* Max size of the "C" string */
		Length = Input->StrLength;	/* Get the size of the current string */
		if (MaxLen>Length) {		/* Set the maximum */
			MaxLen = Length;		/* Truncate */
		}
		Output[0] = (Word8)MaxLen;			/* Pascal length */
		if (MaxLen) {
			FastMemCpy(Output+1,Input->DataPtr,MaxLen);	/* Copy the string */
		}
	}
	return MaxLen;
}

/**********************************

	Remove a part of the string.
	This routine will not resize the buffer

**********************************/

Word BURGERCALL BRStringRemove(BRString_t *Input,Word Start,Word Len)
{
	Word MaxLen;
	char *WorkPtr;
	MaxLen = Input->StrLength;			/* Get the string's length */
	if (Start < MaxLen) {				/* Start off the end? */
		WorkPtr = &Input->DataPtr[Start];		/* Start pointer */
		MaxLen -= Start;				/* Real maximum number of bytes I can remove */
		if (Len >= MaxLen) {			/* Trucation? */
			WorkPtr[0] = 0;				/* End the string here */
			Input->StrLength = (Word16)Start;	/* Save the new length */
		} else {
			if (Len) {					/* Am I removing anything? */
				Input->StrLength -= (Word16)Len;		/* Adjust the length */
				MaxLen = (MaxLen-Len)+1;		/* +1 for the zero terminator */
				do {							/* I have the loop here to make sure I don't have a bad memcpy routine */
					WorkPtr[0] = WorkPtr[Len];	/* Copy the string */
					++WorkPtr;					/* Next byte */
				} while (--MaxLen);				/* All done? */
			}
		}
	}
	return Input->StrLength;					/* Return the new length */
}

/**********************************

	Insert a string into a current BRString_t
	If Start is beyond the end of the string, it's
	placed at the end of the string.
	If Text points to a "C" string that is smaller than
	MaxLen, then only the length of the "C" string is used

**********************************/

Word BURGERCALL BRStringInsert(BRString_t *Input,Word Start,const char *Text,Word MaxLen)
{
	Word Len;					/* Length of the work string */
	Word Temp;					/* ??? */
	char *WorkPtr;				/* Pointer to the ORIGINAL string */
	char *DestPtr;				/* Pointer to the new string */

	Len = Input->StrLength;		/* Current string size */

	if (Start>Len) {			/* Is the start at the END of the string? */
		Start = Len;			/* Force appending */
	}

	Temp = strlen(Text);		/* Maximum insertion */
	if (MaxLen>Temp) {
		MaxLen = Temp;			/* Duh! Don't insert more that I have */
	}

	if (MaxLen) {				/* Am I inserting anything? */
		Temp = Len+MaxLen;		/* New string length */
		WorkPtr = Input->DataPtr;		/* Get the source pointer */
		if (Temp<sizeof(Input->Raw)) {	/* Still in the main buffer? */
			DestPtr = Input->Raw;		/* Use the raw buffer */
		} else {
			DestPtr = StrCopyPad(WorkPtr,MaxLen);	/* Get a copy of the string */
			if (!DestPtr) {			/* Oh my god!!! */
				return Len;			/* I'm not touching the string! */
			}
		}
		Input->StrLength = (Word16)Temp;	/* Save the new length */
		Input->DataPtr = DestPtr;	/* Save the new pointer */

		if (DestPtr==WorkPtr) {		/* Same buffer? */
			DestPtr = WorkPtr+Start;		/* Start here... */
			memmove(DestPtr+MaxLen,DestPtr,(Len-Start)+1);	/* Make room in the center */
			FastMemCpy(DestPtr,Text,MaxLen);				/* Copy in the string */
		} else {
			FastMemCpy(DestPtr,WorkPtr,Start);	/* Copy the first part */
			DestPtr+=Start;
			FastMemCpy(DestPtr,Text,MaxLen);	/* Insert my string */
			DestPtr+=MaxLen;
			FastMemCpy(DestPtr,WorkPtr+Start,(Len-Start)+1);	/* Copy the end and the zero */
		}
		if (WorkPtr!=Input->Raw) {		/* Do I dispose of the old one? */
			DeallocAPointer(WorkPtr);
		}
		Len = Input->StrLength;		/* Save the new length */
	}
	return Len;			/* Return the new string length */
}

/**********************************

	Force the string to be a specific size. If the string
	grows, fill in the extra space with the pad character.

**********************************/

void BRStringLeft(BRString_t *Input,Word NewLen,char padch)
{
	Word OldLen;
	char *WorkPtr;
	char *DestPtr;

	OldLen = Input->StrLength;		/* Get the current length */
	Input->StrLength = (Word16)NewLen;	/* Set the new length */
	WorkPtr = Input->DataPtr;		/* Buffer pointer */
	if (NewLen > OldLen) {			/* Did it grow? */
		if (NewLen<sizeof(Input->Raw)) {
			DestPtr = Input->Raw;		/* Used the internal buffer */
		} else {
			DestPtr = StrCopyPad(WorkPtr,NewLen-OldLen);	/* Get a new string */
			if (!DestPtr) {
				Input->StrLength = 0;		/* No memory!!! */
				Input->Raw[0] = 0;
				Input->DataPtr = Input->Raw;
				return;
			}
		}
		Input->DataPtr = DestPtr;		/* Save the new buffer pointer */
		DestPtr[NewLen] = 0;			/* End it */
		FastMemSet(DestPtr + OldLen, padch, NewLen - OldLen);		/* Fill in the extra */
		if (WorkPtr!=Input->Raw) {		/* Get rid of the old string? */
			DeallocAPointer(WorkPtr);	/* Bye bye */
		}
	} else {
		WorkPtr[NewLen] = 0;			/* Set the end character for shrinkage */
	}
}

/**********************************

	Force the string to be a specific size. If the string
	grows, fill in the extra space with the pad character.
	Note: I am padding the string from the "left" side. I.E.
	the string is moved to the right.

**********************************/

void BRStringRight(BRString_t *Input,Word NewLen,char padch)
{
	Word OldLen;
	char *WorkPtr;
	char *DestPtr;

	OldLen = Input->StrLength;		/* Get the current length */
	Input->StrLength = (Word16)NewLen;	/* Set the new length */
	WorkPtr = Input->DataPtr;		/* Buffer pointer */
	if (NewLen > OldLen) {			/* Did it grow? */
		if (NewLen<sizeof(Input->Raw)) {
			DestPtr = Input->Raw;		/* Used the internal buffer */
		} else {
			DestPtr = StrCopyPad(WorkPtr,NewLen-OldLen);	/* Get a new string */
			if (!DestPtr) {
				Input->StrLength = 0;		/* No memory!!! */
				Input->Raw[0] = 0;
				Input->DataPtr = Input->Raw;
				return;
			}
		}
		Input->DataPtr = DestPtr;		/* Save the new buffer pointer */
		memmove(DestPtr+NewLen-OldLen,DestPtr,OldLen+1);	/* Copy over the characters from the right */
		FastMemSet(DestPtr, padch, NewLen - OldLen);		/* Fill in the extra */
		if (WorkPtr!=Input->Raw) {		/* Get rid of the old string? */
			DeallocAPointer(WorkPtr);	/* Bye bye */
		}
	} else {
		memmove(WorkPtr,WorkPtr+OldLen-NewLen,NewLen+1);	/* Copy over the characters from the right */
	}
}

/**********************************

	Remove a specific char from the string

**********************************/

Word BURGERCALL BRStringRemoveChar(BRString_t *Input,char KillChar)
{
	Word8 *WorkPtr;
	Word8 *DestPtr;
	Word Length;
	Word Temp;

	Length = Input->StrLength;
	if (KillChar && Length) {			/* Should I bother? */
		WorkPtr = (Word8 *)Input->DataPtr;		/* Get the pointer */
		DestPtr = WorkPtr;
		if (Input->Flags) {				/* Case insensitive? */
			if ((Word8)KillChar>='A' && (Word8)KillChar<='Z') {
				KillChar += 32;		/* Force lower case */
			}
			do {
				Temp = WorkPtr[0];
				if (Temp>='A' && Temp<='Z') {		/* Force lower case */
					Temp += 32;
				}
				if (Temp!=(Word8)KillChar) {			/* Allow this char? */
					DestPtr[0] = WorkPtr[0];		/* Save the char */
					++DestPtr;
				}
				++WorkPtr;
			} while (--Length);
		} else {
			do {
				Temp = WorkPtr[0];					/* Get the char */
				++WorkPtr;							/* Accept it */
				if (Temp!=(Word8)KillChar) {
					DestPtr[0] = (Word8)Temp;		/* Keep this */
					++DestPtr;
				}
			} while (--Length);
		}
		DestPtr[0] = 0;								/* Zero terminate */
		Length = DestPtr-(Word8 *)Input->DataPtr;	/* Set the new length */
		Input->StrLength = (Word16)Length;		/* Save the new length if needed */
	}
	return Length;			/* Return the new length */
}

/**********************************

	Remove each and every char in the list

**********************************/

Word BURGERCALL BRStringRemoveChars(BRString_t *Input,const char *CharList)
{
	Word Temp;
	Temp = ((Word8 *)CharList)[0];
	if (Temp) {
		do {
			++CharList;			/* Accept the char */
			BRStringRemoveChar(Input,(char)Temp);	/* Remove this char */
			Temp = ((Word8 *)CharList)[0];		/* Next char in the list */
		} while (Temp);				/* Not the end of the list? */
	}
	return Input->StrLength;		/* Return the new length */
}

/**********************************

	Force compares to be case sensitive

**********************************/

Word BURGERCALL BRStringCaseSensitive(BRString_t *Input)
{
	Word OldFlag;
	OldFlag = Input->Flags;
	Input->Flags = FALSE;
	return OldFlag;
}

/**********************************

	Force compares to be case insensitive

**********************************/

Word BURGERCALL BRStringCaseInsensitive(BRString_t *Input)
{
	Word OldFlag;
	OldFlag = Input->Flags;
	Input->Flags = TRUE;
	return OldFlag;
}

/**********************************

	Force compares to be case sensitive if false

**********************************/

Word BURGERCALL BRStringSetCase(BRString_t *Input,Word State)
{
	Word OldFlag;
	OldFlag = Input->Flags;
	if (State) {
		Input->Flags = TRUE;
	} else {
		Input->Flags = FALSE;
	}
	return OldFlag;
}

