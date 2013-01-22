#include "Text.h"

/****************************************

	Outputs an unsigned long number into a text buffer.
	This will not print leading spaces.

****************************************/

void SOutLongWord(Byte *TextString,LongWord Val)
{
	CaptureText_t Temp;
	CaptureText(&Temp,TextString);
	OutLongWord2(Val,0,0);
	FinishCapture(&Temp);
}

