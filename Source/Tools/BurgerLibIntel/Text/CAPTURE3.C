#include "Text.h"

/********************************

	This converts a "Printed" text string into
	an offline string.

********************************/

static Byte *StringPtr;			/* Pointer to dest buffer */
static void Stream(Word Letter)
{
	*StringPtr = Letter;		/* Save the char */
	++StringPtr;				/* Inc the pointer */
}

void CaptureText(CaptureText_t *CapRec,Byte *TextPtr)
{
	CapRec->Code = OutCharVector;	/* Save the current state */
	CapRec->TextPtr = StringPtr;
	StringPtr = TextPtr;		/* Set a new state */
	OutCharVector = Stream;
}

void FinishCapture(CaptureText_t *CapRec)
{
	*StringPtr = 0;				/* Zero terminate the previous string */
	StringPtr = CapRec->TextPtr;	/* Restore the state */
	OutCharVector = CapRec->Code;
}


