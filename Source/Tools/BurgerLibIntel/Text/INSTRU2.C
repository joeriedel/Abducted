#include "Text.h"

/************************************

	Handle input into an entry struct

************************************/

static Word (*IPCallBack)(DoInputCB_t Type,LongWord Operand);	/* Pointer to callback */
static Word (*OldGetch)(Word InKey);		/* Previous getch callback */

/************************************

	This will allow interception of keyboard
	events so that the GLOBAL keys can be processed.
	Only used as a MyGetch callback.

************************************/

static Word SpecialGetch(Word InKey)
{
	if (OldGetch) {			/* Was there an old routine? */
		InKey=OldGetch(InKey);		/* Call it */
	}
	InKey = IPCallBack(IPKeyHit,InKey);	/* Call my callback */
	switch (InKey) {			/* Special case key? */
	case 0xC8:					/* Move up */
		InKey=1000;
		break;
	case 13:					/* Accept? */
		InKey=1002;
		break;
	case 0xD0:					/* Move down */
	case 9:
		InKey=1001;
	}
	return InKey;				/* Return the key */
}

/************************************

	This will allow background tasks to handle interactive
	text entry to change events on the screen.
	Only used as a MyKbhit Callback

************************************/

static void SpecialKbhit(void)
{
	IPCallBack(IPIdle,0);		/* Call the idle proc */
}

/************************************

	This is a fake callback if the input
	struct routine is called without a valid
	callback pointer.

************************************/

static Word BogusCallBack(DoInputCB_t Type,LongWord Operand)
{
	if (Type==IPKeyHit) {			/* Only the keyhit returns anything */
		return (Word)Operand;
	}
	return 0;
}

/************************************

	Accept input from an array of input controls
	such as line edits, number choices etc...
	Use a callback to allow the user to enhance
	the screen with active displays and allow
	to change from the keyboard defaults.

************************************/

Word InputStruct2(EntryList_t *EntryPtr,Word (*CallBack)(DoInputCB_t,LongWord))
{
	Word Entries;		/* Number of entries into the struct */
	EntryList_t *WorkEntryPtr;	/* Work pointer */
	Word Result;		/* Result to return */
	Word Active;		/* Currently active entry */

	Entries = 0;		/* Init the entry count */
	WorkEntryPtr = EntryPtr;		/* Copy the pointer to work pointer */
	while (WorkEntryPtr->TextX!=-1) {	/* Valid? */
		++Entries;
		++WorkEntryPtr;
	}
	if (!Entries) {		 /* No entries in list? */
		return FALSE;		/* Canceled! */
	}
	DrawInputStruct(EntryPtr,Entries);		/* Draw all the entries */
	Active = 0;		 /* Make the first one active */
	if (!CallBack) {
		CallBack = BogusCallBack;
	}
	IPCallBack = CallBack;		/* Init the callback */

	KeyboardAddRoutine(SpecialKbhit,0);
	OldGetch = KeyboardGetchCallBack;
	KeyboardGetchCallBack = SpecialGetch;

	for (;;) {
		IPCallBack(IPDrawOn,Active);		/* Draw any extra data */
  		Result = DoInput(&EntryPtr[Active]);	/* Pass the entry ptr */
		IPCallBack(IPDrawOff,Active);
  		DrawOneInputStruct(&EntryPtr[Active]);	/* Redraw the control */
		switch (Result) {
		case 0:					/* Canceled? */
			KeyboardGetchCallBack = 0;
			Result = FALSE;		/* It's ok */
			break;
		case 1000:
			if (!Active) {		/* At top? */
				Active = Entries-1;
			} else {
				--Active;		/* Next one */
			}
			continue;
		case 1001:
			++Active;			/* Next one down */
			if (Active>=Entries) {
				Active = 0;		/* Back to the top */
			}
			continue;
		default:
			if (Result>=10000) {
				break;
			}
			if (Result>=2000) {	/* Jump to control */
				Active=Result-2000;	/* Get control # */
			}
			continue;
		case 1002:				/* Accept code */
			KeyboardGetchCallBack = 0;
			Result = TRUE;
		}
		KeyboardRemoveRoutine(SpecialKbhit,0);
		KeyboardGetchCallBack = OldGetch;
		return Result;			/* Return answer */
	}
}

