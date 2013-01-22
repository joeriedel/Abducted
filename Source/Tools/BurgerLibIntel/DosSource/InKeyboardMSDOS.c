#include "InInput.h"

#if defined(__MSDOS__)
#include "MmMemory.h"
#include "ClStdLib.h"
#include "KeyCodes.h"
#include "MsDos.h"
#include <dos.h>
#include <stdlib.h>

static Word BurgerKeyStart;					/* Internal global for KeyboardKbhit */
static Word BurgerKeyEnd;					/* Internal global for KeyboardKbhit */
static Word BurgerKeyBuffer[KEYBUFFSIZE];	/* Keyboard buffer */
static void far *Int15Vec;		/* Offset to old int 15 handler */
static Word8 LastCode;			/* Last scan code hit */
static Bool OneTime;			/* Only set up once */
static Bool CapsLock;

/**********************************

	Post a keyboard event into the ASCII queue

**********************************/

static void BURGERCALL BurgerPostKey(Word NewKey)
{
	Word Temp;
	if (NewKey) {			/* Was a key passed? */
		Temp = BurgerKeyEnd;		/* Get the end mark */
		BurgerKeyBuffer[Temp] = NewKey;	/* Save the char */
		Temp = (Temp+1)&(KEYBUFFSIZE-1);	/* Wrap around */
		if (Temp!=BurgerKeyStart) {	/* Wrapped around? */
			BurgerKeyEnd = Temp;	/* Nope, accept the char */
		}
	}
}

/**********************************

	Wait for a key down event, if the event
	is an extended key then translate to
	Burgerlib extended ASCII

**********************************/

Word BURGERCALL KeyboardGetch(void)
{
	Word NewKey;
	do {
		if (BurgerKeyStart==BurgerKeyEnd) {
			while (!KeyboardKbhit()) {}				/* Wait until a keydown */
		}
		NewKey = BurgerKeyBuffer[BurgerKeyStart];
		BurgerKeyStart = (BurgerKeyStart+1)&(KEYBUFFSIZE-1);	/* Accept the key */
	} while (NewKey&0x8000);									/* Ignore Key up events */
	NewKey = NewKey&0xFF;						/* Isolate the ASCII */
	if (KeyboardGetchCallBack) {
		NewKey = KeyboardGetchCallBack(NewKey);		/* Keyboard filter? */
	}
	return NewKey;	/* Convert to proper ASCII code */
}

/**********************************

	See if a key down event is pending from the keyboard

**********************************/

Word KeyboardKbhit(void)
{
	Word Start;
	Word Temp;		/* Temp event record */
	KeyboardCallPollingProcs();
	Start = BurgerKeyStart;		/* Get the starting index */
	if (Start!=BurgerKeyEnd) {	/* Anything in the buffer? */
		do {
			Temp = BurgerKeyBuffer[Start];		/* Scan for a key down event */
			if (!(Temp&0x8000) && (Temp&0xFF)) {		/* Key down? */
				return Temp&0xFF;				/* Accept it */
			}
			Start = (Start+1)&(KEYBUFFSIZE-1);	/* Next key */
		} while (Start!=BurgerKeyEnd);			/* End of the buffer? */
	}
	return 0;		/* No event */
}

/**********************************

	See if a key down event is pending from the keyboard

**********************************/

Word BURGERCALL KeyboardGet2(void)
{
	Word Start;
	Word Temp;		/* Temp event record */

	KeyboardCallPollingProcs();
	Start = BurgerKeyStart;		/* Get the starting index */
	if (Start!=BurgerKeyEnd) {	/* Anything in the buffer? */
		Temp = BurgerKeyBuffer[Start];		/* Scan for a key down event */
		BurgerKeyStart = (Start+1)&(KEYBUFFSIZE-1);	/* Next key */
		return Temp;
	}
	return 0;		/* No event */
}

/**********************************

	This table will take a scan code referance given by a
	call to INT 0x16 and convert it to my own format of ASCII codes
	DOS Only

**********************************/

/* Remap table for scan codes 00-7F (No prefix) */

static const Word8 NormMap[128] = {
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
	0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
	0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F
};

/* Remap table for scan codes with 0E0H prefix */

static const Word8 E0Map[128] = {
	0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
	0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x6A,0x6B,0x7F,0x7F,
	0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x6D,0x7F,0x7F,0x7F,0x7F,0x7F,
	0x7F,0x7F,0x7F,0x7F,0x7F,0x6C,0x7F,0x6D,0x6F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
	0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x60,0x61,0x62,0x7F,0x63,0x7F,0x64,0x7F,0x65,
	0x66,0x67,0x68,0x69,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
	0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
	0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F
};

/* Remap table for scan codes with 0E1 prefix */

static const Word8 E1Map[128] = {
	0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
	0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x6E,0x7F,0x7F,
	0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
	0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
	0x7F,0x7F,0x7F,0x7F,0x7F,0x6E,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
	0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
	0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
	0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F
};

/* Remap from scan codes to ASCII */

static const Word8 ControlKeys[0x70] = {
	0x00,	//SC_NONE			0x00
	0x1B,	//SC_ESCAPE		0x01
	'1',	//SC_1			0x02
	'2',	//SC_2			0x03
	'3',	//SC_3			0x04
	'4',	//SC_4			0x05
	'5',	//SC_5			0x06
	'6',	//SC_6			0x07
	'7',	//SC_7			0x08
	'8',	//SC_8			0x09
	'9',	//SC_9			0x0A
	'0',	//SC_0			0x0B
	0x1f,	//SC_UNDERSCORE	0x0C
	0x00,	//SC_PLUS			0x0D
	ASCII_BACKSPACE,	//SC_BACKSPACE	0x0E
	ASCII_TAB,	//SC_TAB			0x0F
	'Q'-64,	//SC_Q			0x10
	'W'-64,	//SC_W			0x11
	'E'-64,	//SC_E			0x12
	'R'-64,	//SC_R			0x13
	'T'-64,	//SC_T			0x14
	'Y'-64,	//SC_Y			0x15
	'U'-64,	//SC_U			0x16
	'I'-64,	//SC_I			0x17
	'O'-64,	//SC_O			0x18
	'P'-64,	//SC_P			0x19
	0x1D,	//SC_RIGHTBRACE	0x1A
	0x1B,	//SC_LEFTBRACE	0x1B
	0x0D,	//SC_RETURN		0x1C
	0x00,	//SC_LEFTCONTROL	0x1D
	'A'-64,	//SC_A			0x1E
	'S'-64,	//SC_S			0x1F
	'D'-64,	//SC_D			0x20
	'F'-64,	//SC_F			0x21
	'G'-64,	//SC_G			0x22
	'H'-64,	//SC_H			0x23
	'J'-64,	//SC_J			0x24
	'K'-64,	//SC_K			0x25
	'L'-64,	//SC_L			0x26
	0x00,	//SC_COLON		0x27
	0x00,	//SC_QUOTE		0x28
	0x00,	//SC_TILDE		0x29
	0x00,	//SC_LEFTSHIFT	0x2A
	0x00,	//SC_VERTBAR		0x2B
	'Z'-64,	//SC_Z			0x2C
	'X'-64,	//SC_X			0x2D
	'C'-64,	//SC_C			0x2E
	'V'-64,	//SC_V			0x2F
	'B'-64,	//SC_B			0x30
	'N'-64,	//SC_N			0x31
	'M'-64,	//SC_M			0x32
	0x00,	//SC_COMMA		0x33
	0x00,	//SC_PERIOD		0x34
	0x00,	//SC_SLASH		0x35
	0x00,	//SC_RIGHTSHIFT	0x36
	'*',	//SC_KEYPADASTERISK	0x37
	0x00,	//SC_LEFTALT		0x38
	' ',	//SC_SPACE		0x39
	0x00,	//SC_CAPSLOCK		0x3A
	ASCII_F1,	//SC_F1			0x3B
	ASCII_F2,	//SC_F2			0x3C
	ASCII_F3,	//SC_F3			0x3D
	ASCII_F4,	//SC_F4			0x3E
	ASCII_F5,	//SC_F5			0x3F
	ASCII_F6,	//SC_F6			0x40
	ASCII_F7,	//SC_F7			0x41
	ASCII_F8,	//SC_F8			0x42
	ASCII_F9,	//SC_F9			0x43
	ASCII_F10,	//SC_F10			0x44
	0x00,	//SC_NUMLOCK		0x45
	ASCII_SCROLLLOCK,	//SC_SCROLLLOCK	0x46
	0x00,	//SC_KEYPAD7		0x47
	0x00,	//SC_KEYPAD8		0x48
	0x00,	//SC_KEYPAD9		0x49
	0x00,	//SC_KEYPADMINUS	0x4A
	0x00,	//SC_KEYPAD4		0x4B
	0x00,	//SC_KEYPAD5		0x4C
	0x00,	//SC_KEYPAD6		0x4D
	0x00,	//SC_KEYPADPLUS	0x4E
	0x00,	//SC_KEYPAD1		0x4F
	0x00,	//SC_KEYPAD2		0x50
	0x00,	//SC_KEYPAD3		0x51
	0x00,	//SC_KEYPAD0		0x52
	0x00,	//SC_KEYPADPERIOD	0x53
	0x00,	/* 0x54 */
	0x00,	/* 0x55 */
	0x00,	/* 0x56 */
	ASCII_F11,	//SC_F11			0x57
	0x00,	/* 0x58 */
	ASCII_F12,	//SC_F12			0x59
	0x00,	/* 0x5A */
	0x00,	/* 0x5B */
	0x00,	/* 0x5C */
	0x00,	/* 0x5D */
	0x00,	/* 0x5E */
	0x00,	/* 0x5F */
	ASCII_HOME,	//SC_HOME	0x60	/* 0xE0,0x47 */
	ASCII_UPARROW,	//SC_UPARROW		0x61	/* 0xE0,0x48 */
	ASCII_PAGEUP,	//SC_PAGEUP		0x62	/* 0xE0,0x49 */
	ASCII_LEFTARROW,	//SC_LEFTARROW	0x63	/* 0xE0,0x4B */
	ASCII_RIGHTARROW,	//SC_RIGHTARROW	0x64	/* 0xE0,0x4D */
	ASCII_END,	//SC_END			0x65	/* 0xE0,0x4F */
	ASCII_DOWNARROW,	//SC_DOWNARROW	0x66	/* 0xE0,0x50 */
	ASCII_PAGEDOWN,	//SC_PAGEDOWN		0x67	/* 0xE0,0x51 */
	ASCII_INSERT,	//SC_INSERT		0x68	/* 0xE0,0x52 */
	ASCII_DELETE,	//SC_DELETE		0x69	/* 0xE0,0x53 */
	0x0D,	//SC_KEYPADENTER	0x6A	/* 0xE0,0x1C */
	0x00,	//SC_RIGHTCONTROL	0x6B	/* 0xE0,0x1D */
	'/',	//SC_KEYPADSLASH	0x6C	/* 0xE0,0x35 */
	ASCII_PRINTSCREEN,	//SC_PRINTSCREEN	0x6D	/* 0xE0,0x2A,0x37 */
	ASCII_PAUSE,	//SC_PAUSE		0x6E	/* 0xE1,0x1D,0x45 */
	0x00	//SC_RIGHTALT		0x6F	/* 0xE0,0x38 */
};

static const Word8 NormKeys[0x70] = {
	0x00,	//SC_NONE			0x00
	0x1B,	//SC_ESCAPE		0x01
	'1',	//SC_1			0x02
	'2',	//SC_2			0x03
	'3',	//SC_3			0x04
	'4',	//SC_4			0x05
	'5',	//SC_5			0x06
	'6',	//SC_6			0x07
	'7',	//SC_7			0x08
	'8',	//SC_8			0x09
	'9',	//SC_9			0x0A
	'0',	//SC_0			0x0B
	'-',	//SC_UNDERSCORE	0x0C
	'=',	//SC_PLUS			0x0D
	ASCII_BACKSPACE,	//SC_BACKSPACE	0x0E
	ASCII_TAB,	//SC_TAB			0x0F
	'q',	//SC_Q			0x10
	'w',	//SC_W			0x11
	'e',	//SC_E			0x12
	'r',	//SC_R			0x13
	't',	//SC_T			0x14
	'y',	//SC_Y			0x15
	'u',	//SC_U			0x16
	'i',	//SC_I			0x17
	'o',	//SC_O			0x18
	'p',	//SC_P			0x19
	']',	//SC_RIGHTBRACE	0x1A
	'[',	//SC_LEFTBRACE	0x1B
	0x0D,	//SC_RETURN		0x1C
	0x00,	//SC_LEFTCONTROL	0x1D
	'a',	//SC_A			0x1E
	's',	//SC_S			0x1F
	'd',	//SC_D			0x20
	'f',	//SC_F			0x21
	'g',	//SC_G			0x22
	'h',	//SC_H			0x23
	'j',	//SC_J			0x24
	'k',	//SC_K			0x25
	'l',	//SC_L			0x26
	';',	//SC_COLON		0x27
	'\'',	//SC_QUOTE		0x28
	'`',	//SC_TILDE		0x29
	0x00,	//SC_LEFTSHIFT	0x2A
	'\\',	//SC_VERTBAR		0x2B
	'z',	//SC_Z			0x2C
	'x',	//SC_X			0x2D
	'c',	//SC_C			0x2E
	'v',	//SC_V			0x2F
	'b',	//SC_B			0x30
	'n',	//SC_N			0x31
	'm',	//SC_M			0x32
	',',	//SC_COMMA		0x33
	'.',	//SC_PERIOD		0x34
	'/',	//SC_SLASH		0x35
	0x00,	//SC_RIGHTSHIFT	0x36
	'*',	//SC_KEYPADASTERISK	0x37
	0x00,	//SC_LEFTALT		0x38
	' ',	//SC_SPACE		0x39
	0x00,	//SC_CAPSLOCK		0x3A
	ASCII_F1,	//SC_F1			0x3B
	ASCII_F2,	//SC_F2			0x3C
	ASCII_F3,	//SC_F3			0x3D
	ASCII_F4,	//SC_F4			0x3E
	ASCII_F5,	//SC_F5			0x3F
	ASCII_F6,	//SC_F6			0x40
	ASCII_F7,	//SC_F7			0x41
	ASCII_F8,	//SC_F8			0x42
	ASCII_F9,	//SC_F9			0x43
	ASCII_F10,	//SC_F10			0x44
	0x00,	//SC_NUMLOCK		0x45
	ASCII_SCROLLLOCK,	//SC_SCROLLLOCK	0x46
	'7',	//SC_KEYPAD7		0x47
	'8',	//SC_KEYPAD8		0x48
	'9',	//SC_KEYPAD9		0x49
	'-',	//SC_KEYPADMINUS	0x4A
	'4',	//SC_KEYPAD4		0x4B
	'5',	//SC_KEYPAD5		0x4C
	'6',	//SC_KEYPAD6		0x4D
	'+',	//SC_KEYPADPLUS	0x4E
	'1',	//SC_KEYPAD1		0x4F
	'2',	//SC_KEYPAD2		0x50
	'3',	//SC_KEYPAD3		0x51
	'0',	//SC_KEYPAD0		0x52
	'.',	//SC_KEYPADPERIOD	0x53
	0x00,	/* 0x54 */
	0x00,	/* 0x55 */
	0x00,	/* 0x56 */
	ASCII_F11,	//SC_F11			0x57
	0x00,	/* 0x58 */
	ASCII_F12,	//SC_F12			0x59
	0x00,	/* 0x5A */
	0x00,	/* 0x5B */
	0x00,	/* 0x5C */
	0x00,	/* 0x5D */
	0x00,	/* 0x5E */
	0x00,	/* 0x5F */
	ASCII_HOME,	//SC_HOME	0x60	/* 0xE0,0x47 */
	ASCII_UPARROW,	//SC_UPARROW		0x61	/* 0xE0,0x48 */
	ASCII_PAGEUP,	//SC_PAGEUP		0x62	/* 0xE0,0x49 */
	ASCII_LEFTARROW,	//SC_LEFTARROW	0x63	/* 0xE0,0x4B */
	ASCII_RIGHTARROW,	//SC_RIGHTARROW	0x64	/* 0xE0,0x4D */
	ASCII_END,	//SC_END			0x65	/* 0xE0,0x4F */
	ASCII_DOWNARROW,	//SC_DOWNARROW	0x66	/* 0xE0,0x50 */
	ASCII_PAGEDOWN,	//SC_PAGEDOWN		0x67	/* 0xE0,0x51 */
	ASCII_INSERT,	//SC_INSERT		0x68	/* 0xE0,0x52 */
	ASCII_DELETE,	//SC_DELETE		0x69	/* 0xE0,0x53 */
	0x0D,	//SC_KEYPADENTER	0x6A	/* 0xE0,0x1C */
	0x00,	//SC_RIGHTCONTROL	0x6B	/* 0xE0,0x1D */
	'/',	//SC_KEYPADSLASH	0x6C	/* 0xE0,0x35 */
	ASCII_PRINTSCREEN,	//SC_PRINTSCREEN	0x6D	/* 0xE0,0x2A,0x37 */
	ASCII_PAUSE,	//SC_PAUSE		0x6E	/* 0xE1,0x1D,0x45 */
	0x00	//SC_RIGHTALT		0x6F	/* 0xE0,0x38 */
};

static const Word8 ShiftKeys[0x70] = {
	0x00,	//SC_NONE			0x00
	0x1B,	//SC_ESCAPE		0x01
	'!',	//SC_1			0x02
	'@',	//SC_2			0x03
	'#',	//SC_3			0x04
	'$',	//SC_4			0x05
	'%',	//SC_5			0x06
	'^',	//SC_6			0x07
	'&',	//SC_7			0x08
	'*',	//SC_8			0x09
	'(',	//SC_9			0x0A
	')',	//SC_0			0x0B
	'_',	//SC_UNDERSCORE	0x0C
	'+',	//SC_PLUS			0x0D
	ASCII_BACKSPACE,	//SC_BACKSPACE	0x0E
	ASCII_TAB,	//SC_TAB			0x0F
	'Q',	//SC_Q			0x10
	'W',	//SC_W			0x11
	'E',	//SC_E			0x12
	'R',	//SC_R			0x13
	'T',	//SC_T			0x14
	'Y',	//SC_Y			0x15
	'U',	//SC_U			0x16
	'I',	//SC_I			0x17
	'O',	//SC_O			0x18
	'P',	//SC_P			0x19
	'}',	//SC_RIGHTBRACE	0x1A
	'{',	//SC_LEFTBRACE	0x1B
	0x0D,	//SC_RETURN		0x1C
	0x00,	//SC_LEFTCONTROL	0x1D
	'A',	//SC_A			0x1E
	'S',	//SC_S			0x1F
	'D',	//SC_D			0x20
	'F',	//SC_F			0x21
	'G',	//SC_G			0x22
	'H',	//SC_H			0x23
	'J',	//SC_J			0x24
	'K',	//SC_K			0x25
	'L',	//SC_L			0x26
	':',	//SC_COLON		0x27
	'"',	//SC_QUOTE		0x28
	'~',	//SC_TILDE		0x29
	0x00,	//SC_LEFTSHIFT	0x2A
	'|',	//SC_VERTBAR		0x2B
	'Z',	//SC_Z			0x2C
	'X',	//SC_X			0x2D
	'C',	//SC_C			0x2E
	'V',	//SC_V			0x2F
	'B',	//SC_B			0x30
	'N',	//SC_N			0x31
	'M',	//SC_M			0x32
	'<',	//SC_COMMA		0x33
	'>',	//SC_PERIOD		0x34
	'?',	//SC_SLASH		0x35
	0x00,	//SC_RIGHTSHIFT	0x36
	'*',	//SC_KEYPADASTERISK	0x37
	0x00,	//SC_LEFTALT		0x38
	' ',	//SC_SPACE		0x39
	0x00,	//SC_CAPSLOCK		0x3A
	ASCII_F1,	//SC_F1			0x3B
	ASCII_F2,	//SC_F2			0x3C
	ASCII_F3,	//SC_F3			0x3D
	ASCII_F4,	//SC_F4			0x3E
	ASCII_F5,	//SC_F5			0x3F
	ASCII_F6,	//SC_F6			0x40
	ASCII_F7,	//SC_F7			0x41
	ASCII_F8,	//SC_F8			0x42
	ASCII_F9,	//SC_F9			0x43
	ASCII_F10,	//SC_F10			0x44
	0x00,	//SC_NUMLOCK		0x45
	ASCII_SCROLLLOCK,	//SC_SCROLLLOCK	0x46
	'7',	//SC_KEYPAD7		0x47
	'8',	//SC_KEYPAD8		0x48
	'9',	//SC_KEYPAD9		0x49
	'-',	//SC_KEYPADMINUS	0x4A
	'4',	//SC_KEYPAD4		0x4B
	'5',	//SC_KEYPAD5		0x4C
	'6',	//SC_KEYPAD6		0x4D
	'+',	//SC_KEYPADPLUS	0x4E
	'1',	//SC_KEYPAD1		0x4F
	'2',	//SC_KEYPAD2		0x50
	'3',	//SC_KEYPAD3		0x51
	'0',	//SC_KEYPAD0		0x52
	'.',	//SC_KEYPADPERIOD	0x53
	0x00,	/* 0x54 */
	0x00,	/* 0x55 */
	0x00,	/* 0x56 */
	ASCII_F11,	//SC_F11			0x57
	0x00,	/* 0x58 */
	ASCII_F12,	//SC_F12			0x59
	0x00,	/* 0x5A */
	0x00,	/* 0x5B */
	0x00,	/* 0x5C */
	0x00,	/* 0x5D */
	0x00,	/* 0x5E */
	0x00,	/* 0x5F */
	ASCII_HOME,	//SC_HOME	0x60	/* 0xE0,0x47 */
	ASCII_UPARROW,	//SC_UPARROW		0x61	/* 0xE0,0x48 */
	ASCII_PAGEUP,	//SC_PAGEUP		0x62	/* 0xE0,0x49 */
	ASCII_LEFTARROW,	//SC_LEFTARROW	0x63	/* 0xE0,0x4B */
	ASCII_RIGHTARROW,	//SC_RIGHTARROW	0x64	/* 0xE0,0x4D */
	ASCII_END,	//SC_END			0x65	/* 0xE0,0x4F */
	ASCII_DOWNARROW,	//SC_DOWNARROW	0x66	/* 0xE0,0x50 */
	ASCII_PAGEDOWN,	//SC_PAGEDOWN		0x67	/* 0xE0,0x51 */
	ASCII_INSERT,	//SC_INSERT		0x68	/* 0xE0,0x52 */
	ASCII_DELETE,	//SC_DELETE		0x69	/* 0xE0,0x53 */
	0x0D,	//SC_KEYPADENTER	0x6A	/* 0xE0,0x1C */
	0x00,	//SC_RIGHTCONTROL	0x6B	/* 0xE0,0x1D */
	'/',	//SC_KEYPADSLASH	0x6C	/* 0xE0,0x35 */
	ASCII_PRINTSCREEN,	//SC_PRINTSCREEN	0x6D	/* 0xE0,0x2A,0x37 */
	ASCII_PAUSE,	//SC_PAUSE		0x6E	/* 0xE1,0x1D,0x45 */
	0x00	//SC_RIGHTALT		0x6F	/* 0xE0,0x38 */
};


static Word Remap(Word Input)
{
	if (Input<0x70) {
		Bool Val;
		if (KeyArray[SC_LEFTCONTROL]&1 || KeyArray[SC_RIGHTCONTROL]) {
			return ControlKeys[Input]+(Input<<8);
		}
		Val = CapsLock;
		if (KeyArray[SC_LEFTSHIFT]&1 || KeyArray[SC_RIGHTSHIFT]&1) {
			Val ^= 1;		/* Reverse the caps flag */
		}
		if (Val) {
			return ShiftKeys[Input]+(Input<<8);
		}
		return NormKeys[Input]+(Input<<8);
	}
	return 0;
}

/**********************************

	This is a local IBM DOS routine to take a scan code
	and convert it into ASCII

**********************************/

#define ScanToAscii(Input) Lookup[Input&0xFF]

/**********************************

	Wait for a keyboard event, if the event
	is an extended key then translate to
	Burgerlib extended ASCII

**********************************/

#if 0
Word16 GrabFromDosKey(void);
#pragma aux GrabFromDosKey = \
	"MOV AH,010H"	/* Call BIOS to read the keyboard */ \
	"INT 016H" \
	value [ax] \
	modify [eax];

Word BURGERCALL KeyboardGetch(void)
{
	Word NewKey;
	Word Temp;
	Temp = GrabFromDosKey();	/* Call BIOS */
	NewKey = Temp&0xFF;			/* Isolate the ASCII */
 	if (!NewKey || NewKey>=0xE0) {	/* No BIOS ascii translation? */
 		NewKey = ScanToAscii(Temp>>8);
	}
	if (MyGetchCallBack) {		/* Is there a callback? */
		NewKey = MyGetchCallBack(NewKey);
	}
	return NewKey;
}

/**********************************

	See if a key is pending from the keyboard

**********************************/

Word16 CheckForDosKey(void);
#pragma aux CheckForDosKey = \
	"MOV AH,011H"	/* Call BIOS to read the keyboard */ \
	"INT 016H"		\
	"JNZ Foo"		\
	"XOR EAX,EAX"	\
	"Foo:"			\
	value [ax]		\
	modify [eax];

Word BURGERCALL KeyboardKbhit(void)
{
	Word Temp,NewKey;

	KeyboardCallPollingProcs();
	Temp = CheckForDosKey();		/* Assembly to check pending key */
	NewKey = Temp&0xFF;
	if (Temp) {
		if (!NewKey || NewKey>=0xE0) {
			NewKey = ScanToAscii(Temp>>8);
		}
	}
	return NewKey;
}
#endif

/**********************************

	Keyboard intercept routine to track whether you are holding
	down multiple keys (For games)
	Note : The Input value cannot be changed!!
		I can only or it with 0x80 but that's it!

**********************************/

static Word __loadds KeyParser(Word Input)
{
	Word8 NewChar;		/* Current input */
	Word8 Prefix;		/* Previous prefix code */
	Word Temp;			/* Temp value */

	NewChar = (Word8)Input;		/* Isolate the scan code */
	if (NewChar != 0xE0 && NewChar!=0xE1) {	/* Valid key? */
		Temp = Input&0x7F;	/* Mask the scan code index */
		Prefix = LastCode;	/* Get the prefix code */

		if (Prefix==0xE0) {		/* Remap the scan code to Burgerlib */
			Temp = E0Map[Temp];
		} else if (Prefix==0xE1) {
			Temp = E1Map[Temp];
		} else {
			Temp = NormMap[Temp];
		}

	/* Set and clear element in KeyArray */

		if (NewChar>=128) {		/* Key up event? */
			KeyArray[Temp]&=(~1);
			BurgerPostKey(Remap(Temp)|0x8000);
		} else {
			KeyArray[Temp]|=3;	/* Key down */
			if (Temp==SC_CAPSLOCK) {
				CapsLock ^= 1;
			}
			BurgerPostKey(Remap(Temp));
		}

/* The following code removes the pause key, Ctrl-C and Ctrl-Break */
/* from the input stream to prevent system crashes by bad keycodes */
/* CL has the previous prefix code */

#if 0
		if (Prefix==0xE0) {
			if (NewChar==0x46) {	/* Ctrl-Break (E0,46) */
				goto ZapKey;		/* Force to key up */
			}
		} else if (Prefix==0xE1) {
			if (NewChar==0x1D) {
				goto ZapKey;
			}
			if (NewChar==0x45) {
				LastCode = 0;
				goto ZapKey;
			}
		} else if (NewChar==0x2E) {		/* Possibly a Ctrl-C */
			if (ZeroBase[0x40*16+0x17]&4 ||		/* Holding down control */
				ZeroBase[0x40*16+0x18]&1) {
ZapKey:
				return Input|0x80;
			}
		}
#endif
	}
	LastCode = NewChar;		/* Save code for next time */
	return Input|0x80;
}

/**********************************

	The actual keyboard interrupt glue
	I have to do it the hard way since
	INT 0x15 needs to return CARRY set on the stack

**********************************/

void DoInt15X32(void);		/* Prototype in "C" */

#pragma aux DoInt15X32 = \
"	PUSHFD"	\
"	CMP	AH,04FH"	/* Is this a keyboard filter request? */ \
"	JZ	SHORT Doit"	/* Yes, process it */ \
"	POPFD" \
"	JMP	FAR CS:[Int15Vec]"		/* Chain...	*/ \
"Doit:" \
"	CALL	KeyParser" \
"	POPFD" \
"	OR	BYTE PTR [ESP+8],1" /* Set the carry flag */ \
"	IRETD"

static void NewInt15(void)
{
	DoInt15X32();		/* Do the assembly glue */
}

/**********************************

	Use the X32 DOS extender

**********************************/

#if defined (__X32__)

/**********************************

	Remove the keyboard intercept routine

**********************************/

static Word32 RealInt15;		/* Pointer to real mode int 15 handler */

void BURGERCALL KeyboardDestroy(void)
{
	if (OneTime) {
		OneTime = FALSE;
		SetProtInt(0x15,Int15Vec);
		SetRealInt(0x15,RealInt15);
	}
}

/**********************************

	Activate the keyboard intercept routine

**********************************/

void BURGERCALL KeyboardInit(void)
{
	if (!OneTime) {	/* Already initialized? */
		OneTime=TRUE;
		atexit(KeyboardDestroy);	/* Add this to the shutdown vector */
		Int15Vec = GetProtInt(0x15);	/* Get the protected interrupt vector */
		RealInt15 = GetRealInt(0x15);	/* Get the real mode IRQ */
		SetBothInts(0x15,&NewInt15);	/* Set my interrupt code */
		FastMemSet((char *)&KeyArray[0],0,sizeof(KeyArray));	/* Blank the array */
		MouseClicked = FALSE;
		BurgerKeyStart = 0;
		BurgerKeyEnd = 0;
	}
}

/**********************************

	Use the Rational Systems DOS4GW
	DOS extender

**********************************/

#else

/**********************************

	Remove the keyboard intercept routine

**********************************/

void BURGERCALL KeyboardDestroy(void)
{
	if (OneTime) {
		OneTime = FALSE;
		_dos_setvect(0x15,(void (__interrupt __far *)())Int15Vec);	/* Restore the old vector */
	}
}

/**********************************

	Activate the keyboard intercept routine

**********************************/

void BURGERCALL KeyboardInit(void)
{
	if (!OneTime) {		/* Already initialized? */
		OneTime = TRUE;
		atexit(KeyboardDestroy);
		Int15Vec = _dos_getvect(0x15);	/* Get the old vector */
		_dos_setvect(0x15,(void (__interrupt __far *)())&NewInt15);	/* Set the new vector */
		FastMemSet((char *)&KeyArray[0],0,sizeof(KeyArray));	/* Blank the array */
		MouseClicked = FALSE;
		BurgerKeyStart = 0;
		BurgerKeyEnd = 0;
	}
}

#endif

#endif
