#include "Text.h"

/*********************************

	Print a string of FORMATTED text using
	a script. Vars is a list of pointers to variables
	that can be printed inside of the string.
	Output commands are as follows:

	~A/n55 = Set the text color to 55
	~B = Beep the speaker (Control G sound)
	~C"STRING"~ = Print a centered string (End with '~', \r or \n)
	~E = Clear to end of line
	~F1 = Print a floating point number (0.00 style) "Count" from table
	~G1,4 = Print a floating point number (0.00 style) "Count from table
			and pad with spaces "Count2" for left justification
	~L = Clear screen
	~N = Clear to EOL, then CR and LF
	~P = Clear to end of page
	~RF80 = Repeat a char "Count" times
	~S1 = Print string number "Count" from table
	~T14 = Tab to X coord "Count"
	~U1,0 = Print Word number "Count" from table but space justify it
	~W1 = Print Word number "Count" from table
	~X0,0 = Jump to coord X,Y
	~Z = Reset the text color
	~~ = Print a '~' char
	~@ = No operation (Used as delimiter for centering)

*********************************/

void OutFormatted2(Byte *StrPtr,void **Vars)
{
	Word Val;			/* Temp ASCII char */
	Word Temp1,Temp2;	/* Temp vars */

	for (;;) {				/* Stay until end of string */
		Val = StrPtr[0];	/* Get string char */
		++StrPtr;           /* Accept it */
		if (!Val) {			/* End of string? */
			return;			/* Exit */
		}
		if (Val!='~') {		/* Not a format code? */
			OutChar(Val);	/* Print the char */
			continue;		/* Restart the loop */
		}
		Val=StrPtr[0];		/* Get the format code */
		++StrPtr;			/* Accept it */
		switch (Val) {      /* Dispatch */
		case 'A':	 		/* Set the text attribute */
			TextAttrib = ((Word)StrPtr[0])<<8;		/* Set the text color */
			++StrPtr;
			break;
		case 'B':
			ConsoleBeep();
			break;
		case 'C':		/* Print a centered string */
			Temp1 = 0;	/* Assume zero length */
			for (;;) {
				Temp2 = StrPtr[Temp1];		/* Get char */
				if (Temp2=='~' || Temp2=='\r' || Temp2=='\n') {
					break;		/* Found the end of the string! */
				}
				++Temp1;		/* Inc string length */
			}
			StrPtr[Temp1] = 0;	/* Make a true end of string */
			OutCenter(StrPtr);	/* Print the centered string */
			StrPtr[Temp1] = Temp2;	/* Restore the string */
			StrPtr+=Temp1;		/* Adjust string pointer */
			if (Temp2=='~') {	/* Ignore the '~' char? */
				++StrPtr;		/* Skip it */
			}
			break;
		case 'E':			/* Just clear to end of line */
			ClearEOL();
			break;
		case 'F':			/* Print a floating point number */
			StrPtr = ScanWord(StrPtr,&Temp1);
			OutMoneyFloat(*((float **)Vars)[Temp1]);
			break;
		case 'G':
			StrPtr = ScanWord(StrPtr,&Temp1)+1;
			StrPtr = ScanWord(StrPtr,&Temp2);
			OutMoneyFloat2(*((float **)Vars)[Temp1],Temp2);
			break;
		case 'L':			/* Clear the screen */
			ClearScreen();
			break;
		case 'N':			/* CR LF, with a clear to end of line */
			ClearEOL();		/* Clear to end of line */
			OutCRLF();		/* Print the CR LF */
			break;
		case 'P':			/* Clear to end of page */
			ClearEOP();
			break;
		case 'R':			/* Repeat a char "X" number of times */
			Temp1 = StrPtr[0];		/* Get the char */
			StrPtr = ScanWord(StrPtr+1,&Temp2);	/* Get the count */
			OutChars(Temp1,Temp2);	/* Fill the screen */
			break;
		case 'S':
			StrPtr = ScanWord(StrPtr,&Temp1);
			OutString(((Byte **)Vars)[Temp1]);
			break;
		case 'T':			/* Tab to specific X coord */
			StrPtr = ScanWord(StrPtr,&Temp1);	/* Get the X */
			TabToSpace(Temp1);	/* Tab to the X coord */
			break;
		case 'U':
			StrPtr = ScanWord(StrPtr,&Temp1)+1;
			StrPtr = ScanWord(StrPtr,&Temp2);
			OutLongWord2(*((Word **)Vars)[Temp1],Temp2,' ');
			break;
		case 'W':			/* Print a Word variable */
			StrPtr = ScanWord(StrPtr,&Temp1);	/* Which entry? */
			OutLongWord(*((Word **)Vars)[Temp1]);	/* Print the entry */
			break;
		case 'X':			/* Jump to X/Y coord */
			StrPtr = ScanWord(StrPtr,&Temp1)+1;	/* Parse the X */
			StrPtr = ScanWord(StrPtr,&Temp2);   /* Parse the Y */
			SetXY(Temp1,Temp2);			/* Go there */
			break;
		case 'Z':
			TextAttrib = 0x1700;		/* Normal text mode */
			break;
		case '~':			/* Print the '~' char */
			OutChar('~');
		default:			/* @ and others */
			break;			/* Don't do anything */
		}
	}
}
