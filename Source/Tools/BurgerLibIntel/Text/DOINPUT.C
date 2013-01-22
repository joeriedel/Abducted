#include "Text.h"
#include <stdlib.h>

/****************************************

	Process all events for a text list control

****************************************/

static Word InputCharList(EntryList_t *EntryPtr)
{
	Word Result;
	Word Index;
	Word Max;
	TextListCtrl_t *ListPtr;

	ListPtr = (TextListCtrl_t *)EntryPtr->DestBuffer;	/* Get the pointer */
	Index = *ListPtr->DestWord;		/* Get the base value */
	Max = 0;
	do {
		++Max;
	} while (ListPtr->Strings[Max]);			/* Count the entries */

	for (;;) {
		SetXY(EntryPtr->TextX-1,EntryPtr->TextY);		/* Move the text cursor */
		Result = EntryPtr->Width;
		OutChar('(');
		OutString(ListPtr->Strings[*ListPtr->DestWord]);	/* Print text */
		OutChar(')');
		TabToSpace(EntryPtr->TextX+Result+1);

		Result = KeyboardWait();		/* Wait for key press */
		switch(Result) {
		case 27:				/* ESC */
			return FALSE;
		case 13:				/* CR (Accept) */
			return TRUE;
		default:
			if (Result>=1000) {	/* Special exit? */
				return Result;
			}
			if (Result>='1' && Result<('9'+1)) {	/* Go to number */
				Result-='1';
				if (Result<Max) {
					Index = Result;
				}
			}
			break;
		case 0xCB:		/* Left arrow */
			if (Index) {
				--Index;
			} else {
				Index = Max-1;
			}
			break;
		case 0xCD:		/* Right arrow */
			++Index;
			if (Index>=Max) {
				Index = 0;
			}
		}
		*ListPtr->DestWord = Index;		/* Save the new result in global */
	}
}

/****************************************

	Process all events for a multiple choice list control

****************************************/

static Word InputMultiList(EntryList_t *EntryPtr)
{
	Word Result;
	Word Index;
	Word Max;
	Word Val;
	Word i,j;
	Byte **StrPtr;
	TextListCtrl_t *ListPtr;

	ListPtr = (TextListCtrl_t *)EntryPtr->DestBuffer;	/* Get the pointer */
	Val = *ListPtr->DestWord;		/* Get the base value */
	Index = EntryPtr->Width;
	Max = 0;
	StrPtr = &ListPtr->Strings[0];
	do {
		++Max;
		++StrPtr;
	} while (*StrPtr);			/* Count the entries */

	for (;;) {
		SetXY(EntryPtr->TextX,EntryPtr->TextY);		/* Move the text cursor */
		i = 1;
		StrPtr = &ListPtr->Strings[0];
		j = 0;
		do {
			if (i!=1) {
				OutChar('/');
			}
			if (j==Index) {
				OutChar('(');
			} else {
				OutSpace();
			}
			if (i&Val) {
				TextInverse = TRUE;
			}
			OutString(*StrPtr);
			TextInverse = FALSE;
			if (j==Index) {
				OutChar(')');
			} else {
				OutSpace();
			}
			i<<=1;
			++j;
			++StrPtr;
		} while (*StrPtr);

		Result = KeyboardWait();		/* Wait for key press */
		switch(Result) {
		case 27:				/* ESC */
			return FALSE;
		case 13:				/* CR (Accept) */
			return TRUE;
		default:
			if (Result>=1000) {	/* Special exit? */
				return Result;
			}
			if (Result>='1' && Result<('9'+1)) {	/* Go to number */
				Result-='1';
				if (Result<Max) {
					Index = Result;
				}
			}
			break;
		case ' ':
			Val ^= (1<<Index);
			break;
		case 0xCB:		/* Left arrow */
			if (Index) {
				--Index;
			} else {
				Index = Max-1;
			}
			break;
		case 0xCD:		/* Right arrow */
			++Index;
			if (Index>=Max) {
				Index = 0;
			}
		}
		*ListPtr->DestWord = Val;		/* Save the new result in global */
		EntryPtr->Width = Index;
	}
}

/****************************************

	Draw the initial data found inside the input struct

****************************************/

Word DoInput(EntryList_t *EntryPtr)
{
	Word Result;
	TextListCtrl_t *ListPtr;

	SetXY(EntryPtr->TextX,EntryPtr->TextY);
	switch(EntryPtr->InputType) {
	case ELUpperCase:
		Result = InputLine((Byte *)EntryPtr->DestBuffer,EntryPtr->Width,ILInputValid);
		break;
	case ELLowerCase:
		Result = InputLine((Byte *)EntryPtr->DestBuffer,EntryPtr->Width,ILLowerCase|ILInputValid);
		break;
	case ELNumberEntry:
		ListPtr = (TextListCtrl_t *)EntryPtr->DestBuffer;
		SOutLongWord(ListPtr->Strings[0],*ListPtr->DestWord);
		Result = InputLine(ListPtr->Strings[0],EntryPtr->Width+1,ILNumeric|ILInputValid);
		*ListPtr->DestWord = atoi((char *)(ListPtr->Strings[0]));
		break;
	case ELFloatEntry:
		ListPtr = (TextListCtrl_t *)EntryPtr->DestBuffer;
		SOutMoneyFloat(ListPtr->Strings[0],*(float*)ListPtr->DestWord);
		Result = InputLine(ListPtr->Strings[0],EntryPtr->Width+1,ILFloat|ILInputValid);
		*(float*)ListPtr->DestWord = RoundCent(atof((char *)(ListPtr->Strings[0])));
		break;
	case ELMultiListEntry:
		Result = InputMultiList(EntryPtr);
		break;
	case ELCharListEntry:
		Result = InputCharList(EntryPtr);
		break;
	default:
		Result = FALSE;
	}
	return Result;
}

