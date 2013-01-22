#include "Text.h"

/****************************************

	Prints an unsigned long number to a string buffer.
	Also prints lead spaces or zeros if needed

****************************************/

void SOutLongWord2(Byte *TextString,LongWord Val,Word MinSize,Word LeadChar)
{
	CaptureText_t Temp;
	CaptureText(&Temp,TextString);
	OutLongWord2(Val,MinSize,LeadChar);
	FinishCapture(&Temp);
}

