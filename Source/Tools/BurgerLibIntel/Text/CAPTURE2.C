#include "Text.h"

/********************************

	This converts a "Printed" text string into
	an offline string.

********************************/

void CaptureFormatted2(Byte *TextPtr,Byte *FormattedStr,void **Vars)
{
	CaptureText_t Temp;
	CaptureText(&Temp,TextPtr);		/* Begin text capturing */
	OutFormatted2(FormattedStr,Vars);	/* Print the string */
	FinishCapture(&Temp);				/* Clean up after myself */
}

