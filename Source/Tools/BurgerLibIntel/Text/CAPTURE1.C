#include "Text.h"

/********************************

	This converts a "Printed" text string into
	an offline string.

********************************/

void CaptureFormatted(Byte *TextPtr,Byte *FormattedStr)
{
	CaptureText_t Temp;
	CaptureText(&Temp,TextPtr);	/* Begin text capturing */
	OutFormatted(FormattedStr);		/* Print the string */
	FinishCapture(&Temp);			/* Clean up after myself */
}

