#include "text.h"

/****************************************

	Draw the initial data found inside the input struct
	Each control has a differant way of being drawn

****************************************/

void DrawOneInputStruct(EntryList_t *EntryPtr)
{
	Word Len;			/* Temp length */
	TextListCtrl_t *ListPtr;		/* Pointer to list of strings */

	SetXY(EntryPtr->TextX,EntryPtr->TextY);		/* Move the text cursor */
	Len = EntryPtr->Width;
	ListPtr = (TextListCtrl_t *)EntryPtr->DestBuffer;
	switch(EntryPtr->InputType) {		/* What type of control? */
	case ELLowerCase:		/* Input string? */
	case ELUpperCase:
		OutString((Byte*)ListPtr);		/* Print the default */
		--Len;							/* Adjust length for zero */
		break;
	case ELNumberEntry:
		OutLongWord(*ListPtr->DestWord);	/* Print the number */
		break;
	case ELFloatEntry:
		OutMoneyFloat(*(float *)ListPtr->DestWord);
		break;
	case ELMultiListEntry: {
		Word i;				/* Local variables */
		Byte **StrPtr;
		Word Val;
		OutSpace();			/* Initial space */
		StrPtr = &ListPtr->Strings[0];	/* Get my running pointer */
		Val = *ListPtr->DestWord;
		i = 1;
		do {
			if (i!=1) {			/* Not the first time? */
				OutString((Byte *)" / ");
			}
			if (i&Val) {
				TextInverse = TRUE;	/* Show in inverse text */
			}
			OutString(*StrPtr);		/* Draw the string */
			TextInverse = FALSE;	/* Normal text */
			++StrPtr;
			i<<=1;
		} while (*StrPtr);		/* More? */
		OutSpace();			/* Final space */
		return;
	}
	case ELCharListEntry:
		OutBackSpace();		/* Clear the left parenthesis */
		OutSpace();
		OutString(ListPtr->Strings[*ListPtr->DestWord]);	/* Print text */
		++Len;				/* Clear the right parenthesis */
	}
	if (Len) {			/* Pad the spaces? */
		TabToSpace(EntryPtr->TextX+Len);
	}
}

