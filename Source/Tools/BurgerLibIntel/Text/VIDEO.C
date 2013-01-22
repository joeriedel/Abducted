#include "Text.h"
#include <string.h>
#include <stdlib.h>

/****************************************

	This contains all the code to handle console
	output via WYSE 30 emulation.
	By Bill Heineman 4/3/95

****************************************/

#define DefaultColor 0x1700	 /* Color for the CGA screen */
Word TextX;					 /* Current console text X coord */
Word TextY;					 /* Current console text Y coord */
static Boolean ESCHit;			/* Was an ESC char sent? (WYSE 30) */
static Boolean PosHit;			/* Was a position char sent? (WYSE 30) */
Word TextAttrib = DefaultColor; /* Attribute byte for CGA screens */
Word TextWidth = 80;			/* Width of video console */
Word TextHeight = 25;			/* Height of video console */
Word TextInverse = FALSE;		/* Inverse text color */
Short *TextScreen;

/****************************************

	Clear from the current TextX coord to
	the right edge of the screen with spaces
	in the TextAttrib color.

	No variables affected.

****************************************/

static void ConsoleCLSEOL(void)
{
	Word Stuff;	 /* Short to store on the screen */
	Word Count;	 /* Number of chars to erase */
	Short *Screenad;	/* Pointer to the screen */

	if (TextX<TextWidth) {	/* Is there space on the right side? */
		Screenad = TextScreen;
		if (!Screenad) {
			Screenad = (Short *)&ZeroBase[0xB8000];
		}
		Screenad = Screenad+((TextY*TextWidth)+TextX);	/* Make pointer */
		Count = TextWidth-TextX;	/* Number of spaces to print */
		if (TextInverse) {
			Stuff=((TextAttrib>>4)&0x0700);
			Stuff|=((TextAttrib<<4)&0x7000);
			Stuff|=' ';
		} else {
			Stuff = ' '|TextAttrib;		/* Print the char */
		}
		do {
			*Screenad = Stuff;		/* Draw a char */
			++Screenad;			 /* Inc pointer */
		} while (--Count);			/* Count down */
	}
}

/****************************************

	Clears from the current TextX and TextY
	to the bottom right corner of the text screen.

	No variables affected.

****************************************/

static void ConsoleCLSEOP(void)
{
	Word OldX;
	Word OldY;
	if (TextY<TextHeight) {
		OldX = TextX;		 /* Save the TextX and TextY */
		OldY = TextY;
		do {
			ConsoleCLSEOL();	/* Clear to current end of line */
			TextX = 0;			/* Force future lines to clear completely */
		} while (++TextY<TextHeight);	 /* All done? */
		TextX = OldX;		 /* Restore the TextX and TextY */
		TextY = OldY;
	}
}

/****************************************

	Home the text cursor position.

	Affects TextX and TextY;

****************************************/

static void ConsoleHome(void)
{
	TextX = 0;			/* Move the cursor to the upper left corner */
	TextY = 0;
}

/****************************************

	Move the cursor one char to the left.

	Affects TextX and TextY.

****************************************/

static void ConsoleBS(void)
{
	if (TextX) {		/* Not at the left edge? */
		--TextX;		/* Move back one */
	} else {
		if (TextY) {	/* At the top? */
			--TextY;	/* Move up one */
			TextX = TextWidth-1;	/* Move the cursor to the far right */
		}
	}
}

/****************************************

	Move the cursor to the far left.

	Affects TextX.

****************************************/

static void ConsoleCR(void)
{
	TextX = 0;		/* Minimum X */
}

/****************************************

	Move the cursor down one line, scroll the screen
	up one line if TextY is at the bottom.

	Affects TextY.

****************************************/

static void ConsoleLF(void)
{
	Word OldX;				/* Temp for text X */
	Short *Screenad;
	if (++TextY>=TextHeight) { /* Off the bottom after going down one? */
		TextY = TextHeight-1;		 /* Failsafe for text Y */
		Screenad = TextScreen;
		if (!Screenad) {
			Screenad = (Short *)&ZeroBase[0xB8000];
		}
		memcpy((char*)Screenad,(char *)(Screenad+TextWidth),(TextHeight-1)*2*TextWidth);	/* Move the memory */
		OldX = TextX;		 /* Save TextX */
		TextX = 0;			/* Force CLSEOL to zap the whole line */
		ConsoleCLSEOL();	/* Clear the bottom line */
		TextX = OldX;		 /* Restore TextX */
	}
}

/****************************************

	Home the cursor and clear the screen completely.

	Affects TextX and TextY.

****************************************/

static void ConsoleCLS(void)
{
	ConsoleHome();			/* Home the cursor (Zap X and Y) */
	ConsoleCLSEOP();		/* Since the cursor is home'd, clear everything */
}

/****************************************

	Call bios to reset the video system to
	mode at startup

****************************************/

static Word OldMode;

static void FixVideo(void)
{
	CallInt10(OldMode);			 /* Init mode 3 */
}

/****************************************

	Call this routine after the video mode
	size is changed

****************************************/

void FixVideoWidthHeight(void)
{
	ConsoleCursorOff();			/* Turn off the cursor */
	TextScreen = (Short *)(0);
	TextWidth = *(ZeroBase+0x44A);
	TextHeight = *(ZeroBase+0x484)+1;
	ESCHit = FALSE;			/* Esc was NOT printed */
	PosHit = FALSE;			/* XY position sending NOT printed */
	TextInverse = FALSE;	/* Normal text */
	TextAttrib = DefaultColor;	/* White on blue text */
	ConsoleCLS();			/* Clear the screen */
}

/****************************************

	Init the video system to 80x25 video mode

****************************************/

void Set80x25(void)
{
	CallInt10(3);			/* Set the PC video mode */
	FixVideoWidthHeight();
}

/****************************************

	Init my video driver and WYSE emulation system
	also attach "FixVideo" to the exit chain so when
	the application quits, the screen will be reset.

	This sets ALL video variables to defaults

****************************************/

static Boolean HitMe=FALSE;	/* Allow call to atexit only once */

void InitVideo(void)
{
	if (!HitMe) {			/* Never called before? */
		HitMe = TRUE;		/* Mark it */
		OldMode = *(ZeroBase+0x449);
		atexit(FixVideo);	/* Attach FixVideo to the exit chain */
	}
	Set80x25();				/* Init 80 x 25 video */
}

/****************************************

	Place a char on the video screen
	This does NOT handle control chars, it will print them.

	Affects TextX and TextY.

****************************************/

static void ConsoleRawCout(Word Letter)
{
	Short *Screenad;

	Screenad = TextScreen;
	if (!Screenad) {
		Screenad = (Short *) &ZeroBase[0xB8000];
	}
	Screenad = Screenad + ((TextY*TextWidth)+TextX);
	if (TextInverse) {
		Letter|=((TextAttrib>>4)&0x0700);
		Letter|=((TextAttrib<<4)&0x7000);
	} else {
		Letter|=TextAttrib;		/* Print the char */
	}
	*Screenad = Letter;
	++TextX;							/* Next X coord */
	if (TextX>=TextWidth) {			 /* Off the edge? */
		ConsoleCR();					/* Print a CR and line feed */
		ConsoleLF();
	}
}

/****************************************

	This code will directly drive the video display
	using a WYSE 30 type terminal opcodes.

	It will affect all video variables.

****************************************/

void PutCharConsole(Word Letter)
{
	if (PosHit) {			 /* In the XY scan code already? */
		--PosHit;			 /* Next stage */
		Letter-=' ';		/* Convert ASCII to XY */
		if (PosHit) {		 /* First time through */
			TextY = Letter; /* Save the Y first*/
			return;
		}
		TextX = Letter;	 /* Save the X and exit */
		return;			 /* PosHit = 0 at this point */
	}
	if (ESCHit) {			 /* Was the last char an ESC? */
		ESCHit = FALSE;	 /* Force it clear */
		switch(Letter) {
		case '+':
			ConsoleCLS();	 /* Clear the console screen */
			break;
		case '=':			 /* Set XY command */
			PosHit = 2;	 /* Init XY accept state */
			break;
		case 'Y':			 /* Clear to end of page */
			ConsoleCLSEOP();
			break;
		case 'T':			 /* Clear to end of line */
			ConsoleCLSEOL();
		}
		return;			 /* Ignore all others */
	}
	if (Letter<0x20) {		/* Control character? */
		switch (Letter) {
		case 0x07:
			ConsoleBeep();		/* Beep the speaker */
			break;
		case 0x08:
			ConsoleBS();		/* Backspace */
			break;
		case 0x09:
			do {
				ConsoleRawCout(0x20);
			} while (TextX&3);
			break;
		case 0x0A:
			ConsoleLF();		/* Perform a linefeed */
			break;
		case 0x0C:
			ConsoleCLS();		 /* Clear the screen */
			break;
		case 0x0D:
			ConsoleCR();		/* Perform a carridge return */
			break;
		case 0x1E:
			ConsoleHome();		/* Home cursor */
			break;
		case 0x1B:
			ESCHit = TRUE;		/* Next time through assume ESC code */
		}
		return;
	}
	ConsoleRawCout(Letter);	 /* Print the char */
}

