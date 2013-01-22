#include "Text.h"
#include <string.h>

/****************************************

	This will input a line of text from the console
	and return whether the line was canceled
	or not (TRUE = VALID)
	Max = sizeof(Buffer), as a result, I must assume that the
	string can be one less than the buffer size since I must
	place a zero at the end of the string for "C"

****************************************/

Word (*InputLineCallBack)(void);

Word InputLine(Byte *Buffer,Word Max,Boolean EchoFlag)
{
	Word Index;	 /* Index to the data buffer */
	Word Temp;		/* Temp ASCII char */

	--Max;			/* Compensate for zero terminator */
	OutSpaces(Max);	/* Clear the input area */
	OutBackSpaces(Max);	/* Fix the cursor */
	Index = 0;			/* Init the dest index */

	if (EchoFlag&ILInputValid) {		/* Shall I print the input string? */
		OutString(Buffer);				/* Print the input */
		Index = strlen((char *)Buffer);			/* Place the cursor at the end */
	}
	if (!InputLineCallBack) {			/* Failsafe for bogus callback pointer */
		EchoFlag &= ~ILCallBack;
	}

	for (;;) {		/* Stay forever! */
		Buffer[Index] = 0;
		Temp = 0;
		if (EchoFlag&ILCallBack) {
			Temp = InputLineCallBack();
		}
		if (!Temp) {
			Temp = GetACursorKey();	 /* Get user input */
			if (!Temp) {
				continue;
			}
		}
		if (!(EchoFlag&ILLowerCase)) {	/* Force to upper case? */
			if (Temp>='a' && Temp<('z'+1)) {
				Temp -= 32;		/* Convert to upper case */
			}
		}
		OutSpace();				/* Erase the cursor */
		OutBackSpace();			/* Position at the char */
		if (Temp>=1000) {
			return Temp;
		}
		if (Temp==0xCB) {		/* If left arrow */
			Temp = 8;			/* Then convert to a backspace */
		}
		if (Temp<' ') {	 /* Control char? */
			if (Temp==0x1b) {	 /* ESC? */
				if (Index) {
					OutBackSpaces(Index);	/* Back up over the string */
					OutSpaces(Index);	/* Erase the string */
					OutBackSpaces(Index);	/* Move the text cursor to the beginning */
				}
				return FALSE;	 /* Abort! */
			}
			if (Temp==13) {	 /* CR? */
				if (Index) {
					return TRUE;	/* There is a valid string */
				}
				return FALSE;	 /* No input at all! */
			}
			if (Temp==8 && Index) {
				OutSpace();
				OutBackSpaces(2);
				--Index;
			}
		} else if (Temp<128) {				/* Printable character... */
			if (!Index && Temp==' ') {	/* Don't allow leading spaces */
				continue;
			}
			if (EchoFlag&ILNumeric) {	 /* Only allow numeric input? */
				if (Temp!='.') {
					if (Temp<'0' || Temp >= ('9'+1)) {
						continue;	 /* Not a number */
					}
				} else {
					if (!(EchoFlag&(ILFloat-ILNumeric))) {
						continue;		 /* Not a floating point number */
					}
				}
			}
			if (Index<Max) {		/* Allowable? */
				if (EchoFlag&ILNoEcho) {		/* Echo normally? */
					OutChar('*');		 /* Hidden char for password */
				} else {
					OutChar(Temp);		/* Print the char */
				}
				Buffer[Index] = Temp;	 /* Store the char in the buffer */
				++Index;				/* Add 1 to length */
			}
		}
	}
}

