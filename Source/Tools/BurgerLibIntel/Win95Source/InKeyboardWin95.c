#define DIRECTINPUT_VERSION 0x700
#include "InInput.h"

#if defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include "MmMemory.h"
#include "ClStdLib.h"
#include "KeyCodes.h"
#include "W9Win95.h"
#include "GrGraphics.h"
#include "PlPalette.h"

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)
#endif

static Word RWMMouseWheel;					/* Register mouse wheel support */
static Word BurgerKeyStart;					/* Internal global for KeyboardKbhit */
static Word BurgerKeyEnd;					/* Internal global for KeyboardKbhit */
static Word BurgerKeyBuffer[KEYBUFFSIZE];	/* Keyboard buffer */

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

	This is a local Windows 95 routine to take a scan code
	and convert it into ASCII

**********************************/

static INLINECALL Word ScanToAscii(MSG *Message)
{
	Word Input;
	Input = Message->wParam;
	switch (Input) {		/* Windows 95 Virtual key code */
	case VK_HOME:
		return ASCII_HOME|(SC_HOME<<8);
	case VK_UP:
		return ASCII_UPARROW|(SC_UPARROW<<8);
	case VK_PRIOR:
		return ASCII_PAGEUP|(SC_PAGEUP<<8);
	case VK_LEFT:
		return ASCII_LEFTARROW|(SC_LEFTARROW<<8);
	case VK_RIGHT:
		return ASCII_RIGHTARROW|(SC_RIGHTARROW<<8);
	case VK_END:
		return ASCII_END|(SC_END<<8);
	case VK_DOWN:
		return ASCII_DOWNARROW|(SC_DOWNARROW<<8);
	case VK_NEXT:
		return ASCII_PAGEDOWN|(SC_PAGEDOWN<<8);
	case VK_INSERT:
		return ASCII_INSERT|(SC_INSERT<<8);
	case VK_DELETE:
		return ASCII_DELETE|(SC_DELETE<<8);
	case VK_PAUSE:
		return ASCII_PAUSE|(SC_PAUSE<<8);
	case VK_F1:
		return ASCII_F1|(SC_F1<<8);
	case VK_F2:
		return ASCII_F2|(SC_F2<<8);
	case VK_F3:
		return ASCII_F3|(SC_F3<<8);
	case VK_F4:
		return ASCII_F4|(SC_F4<<8);
	case VK_F5:
		return ASCII_F5|(SC_F5<<8);
	case VK_F6:
		return ASCII_F6|(SC_F6<<8);
	case VK_F7:
		return ASCII_F7|(SC_F7<<8);
	case VK_F8:
		return ASCII_F8|(SC_F8<<8);
	case VK_F9:
		return ASCII_F9|(SC_F9<<8);
	case VK_F10:
		return ASCII_F10|(SC_F10<<8);
	case VK_F11:
		return ASCII_F11|(SC_F11<<8);
	case VK_F12:
		return ASCII_F12|(SC_F12<<8);
	case VK_SCROLL:
		return ASCII_SCROLLLOCK|(SC_SCROLLLOCK<<8);
	case VK_PRINT:
		return ASCII_PRINTSCREEN|(SC_PRINTSCREEN<<8);
	case VK_CONTROL:
		if ((Message->lParam&0x01FF0000)==(0x1D<<16)) {
			return SC_LEFTCONTROL<<8;
		}
		return SC_RIGHTCONTROL<<8;
	case VK_SHIFT:
		if ((Message->lParam&0x00FF0000)==(0x36<<16)) {
			return SC_RIGHTSHIFT<<8;
		}
		return SC_LEFTSHIFT<<8;
	case VK_MENU:
		if ((Message->lParam&0x01FF0000)==(0x038<<16)) {
			return SC_LEFTALT<<8;
		}
		return SC_RIGHTALT<<8;
	}
	return 0;
}

#if 0
/**********************************

	Wait for a keyboard event, if the event
	is an extended key then translate to
	Burgerlib extended ASCII

**********************************/

Word BURGERCALL KeyboardGetch(void)
{
	Word NewKey;
	do {
		ProcessSystemEvents();		/* Allow windows event posting */
	} while (BurgerKeyStart==BurgerKeyEnd);	/* No keys? */
	NewKey = BurgerKeyBuffer[BurgerKeyStart];
	BurgerKeyStart = (BurgerKeyStart+1)&(KEYBUFFSIZE-1);
	if (MyGetchCallBack) {			/* Is there a callback? */
		NewKey = MyGetchCallBack(NewKey);
	}
	return NewKey;			/* Return the key */
}

/**********************************

	See if a key is pending from the keyboard

**********************************/

Word BURGERCALL KeyboardKbhit(void)
{
	Word Temp;
	ProcessSystemEvents();		/* Handle system events */
	KeyboardCallPollingProcs();					/* Null event handler */
	Temp = BurgerKeyStart;		/* Get the starting index */
	if (Temp!=BurgerKeyEnd) {	/* Anything in the buffer? */
		return BurgerKeyBuffer[Temp];		/* Return the key code hit */
	}
	return 0;		/* No event */
}
#else
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
			while (!KeyboardKbhit()) {
			}				/* Wait until a keydown */
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
	ProcessSystemEvents();
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

	ProcessSystemEvents();
	KeyboardCallPollingProcs();
	Start = BurgerKeyStart;		/* Get the starting index */
	if (Start!=BurgerKeyEnd) {	/* Anything in the buffer? */
		Temp = BurgerKeyBuffer[Start];		/* Scan for a key down event */
		BurgerKeyStart = (Start+1)&(KEYBUFFSIZE-1);	/* Next key */
		return Temp;
	}
	return 0;		/* No event */
}
#endif

/**********************************

	Convert Windows scan codes to my internal scan codes
	to properly index to KeyArray

**********************************/

static Word8 NormalScan[256] = {
	0,		/* 0x00 */
	SC_ESCAPE,	/* 0x01 */
	SC_1,	/* 0x02 */
	SC_2,	/* 0x03 */
	SC_3,	/* 0x04 */
	SC_4,	/* 0x05 */
	SC_5,	/* 0x06 */
	SC_6,	/* 0x07 */
	SC_7,	/* 0x08 */
	SC_8,	/* 0x09 */
	SC_9,	/* 0x0A */
	SC_0,	/* 0x0B */
	SC_MINUS,	/* 0x0C */
	SC_EQUALS,	/* 0x0D */
	SC_BACKSPACE,	/* 0x0E */
	SC_TAB,	/* 0x0F */
	SC_Q,	/* 0x10 */
	SC_W,	/* 0x11 */
	SC_E,	/* 0x12 */
	SC_R,	/* 0x13 */
	SC_T,	/* 0x14 */
	SC_Y,	/* 0x15 */
	SC_U,	/* 0x16 */
	SC_I,	/* 0x17 */
	SC_O,	/* 0x18 */
	SC_P,	/* 0x19 */
	SC_RIGHTBRACKET,	/* 0x1A */
	SC_LEFTBRACKET,	/* 0x1B */
	SC_RETURN,	/* 0x1C */
	SC_LEFTCONTROL,	/* 0x1D */
	SC_A,	/* 0x1E */
	SC_S,	/* 0x1F */
	SC_D,	/* 0x20 */
	SC_F,	/* 0x21 */
	SC_G,	/* 0x22 */
	SC_H,	/* 0x23 */
	SC_J,	/* 0x24 */
	SC_K,	/* 0x25 */
	SC_L,	/* 0x26 */
	SC_COLON,	/* 0x27 */
	SC_QUOTE,	/* 0x28 */
	SC_TILDE,	/* 0x29 */
	SC_LEFTSHIFT,	/* 0x2A */
	 0x2B,	/* 0x2B */
	SC_Z,	/* 0x2C */
	SC_X,	/* 0x2D */
	SC_C,	/* 0x2E */
	SC_V,	/* 0x2F */
	SC_B,	/* 0x30 */
	SC_N,	/* 0x31 */
	SC_M,	/* 0x32 */
	SC_COMMA,	/* 0x33 */
	SC_PERIOD,	/* 0x34 */
	SC_SLASH,	/* 0x35 */
	SC_RIGHTSHIFT,	/* 0x36 */
	SC_KEYPADASTERISK,	/* 0x37 */
	SC_LEFTALT,	/* 0x38 */
	 57,	/* 0x39 */
	 58,	/* 0x3A */
	 59,	/* 0x3B */
	 60,	/* 0x3C */
	 61,	/* 0x3D */
	 62,	/* 0x3E */
	 63,	/* 0x3F */
	 64,	/* 0x40 */
	 65,	/* 0x41 */
	 66,	/* 0x42 */
	 67,	/* 0x43 */
	 68,	/* 0x44 */
	SC_PAUSE,	/* 0x45 */
	 70,	/* 0x46 */
	 71,	/* 0x47 */
	 72,	/* 0x48 */
	 73,	/* 0x49 */
	 74,	/* 0x4A */
	 75,	/* 0x4B */
	 76,	/* 0x4C */
	 77,	/* 0x4D */
	 78,	/* 0x4E */
	 79,	/* 0x4F */
	 80,	/* 0x50 */
	 81,	/* 0x51 */
	 82,	/* 0x52 */
	 83,	/* 0x53 */
	 84,	/* 0x54 */
	 85,	/* 0x55 */
	 86,	/* 0x56 */
	 87,	/* 0x57 */
	 88,	/* 0x58 */
	 89,	/* 0x59 */
	 90,	/* 0x5A */
	 91,	/* 0x5B */
	 92,	/* 0x5C */
	 93,	/* 0x5D */
	 94,	/* 0x5E */
	 95,	/* 0x5F */
	 96,	/* 0x60 */
	 97,	/* 0x61 */
	 98,	/* 0x62 */
	 99,	/* 0x63 */
	100,	/* 0x64 */
	101,	/* 0x65 */
	102,	/* 0x66 */
	103,	/* 0x67 */
	104,	/* 0x68 */
	105,	/* 0x69 */
	106,	/* 0x6A */
	107,	/* 0x6B */
	108,	/* 0x6C */
	109,	/* 0x6D */
	110,	/* 0x6E */
	111,	/* 0x6F */
	112,	/* 0x70 */
	113,	/* 0x71 */
	114,	/* 0x72 */
	115,	/* 0x73 */
	116,	/* 0x74 */
	117,	/* 0x75 */
	118,	/* 0x76 */
	119,	/* 0x77 */
	120,	/* 0x78 */
	121,	/* 0x79 */
	122,	/* 0x7A */
	123,	/* 0x7B */
	124,	/* 0x7C */
	125,	/* 0x7D */
	126,	/* 0x7E */
	127,	/* 0x7F */
	0x00,	/* 0x80 */
	0x00,	/* 0x81 */
	0x00,	/* 0x82 */
	0x00,	/* 0x83 */
	0x00,	/* 0x84 */
	0x00,	/* 0x85 */
	0x00,	/* 0x86 */
	0x00,	/* 0x87 */
	0x00,	/* 0x88 */
	0x00,	/* 0x89 */
	0x00,	/* 0x8A */
	0x00,	/* 0x8B */
	0x00,	/* 0x8C */
	0x00,	/* 0x8D */
	0x00,	/* 0x8E */
	0x00,	/* 0x8F */
	0x00,	/* 0x90 */
	0x00,	/* 0x91 */
	0x00,	/* 0x92 */
	0x00,	/* 0x93 */
	0x00,	/* 0x94 */
	0x00,	/* 0x95 */
	0x00,	/* 0x96 */
	0x00,	/* 0x97 */
	0x00,	/* 0x98 */
	0x00,	/* 0x99 */
	0x00,	/* 0x9A */
	0x00,	/* 0x9B */
	SC_KEYPADENTER,	/* 0x9C */
	SC_RIGHTCONTROL,	/* 0x9D */
	0x00,	/* 0x9E */
	0x00,	/* 0x9F */
	0x00,	/* 0xA0 */
	0x00,	/* 0xA1 */
	0x00,	/* 0xA2 */
	0x00,	/* 0xA3 */
	0x00,	/* 0xA4 */
	0x00,	/* 0xA5 */
	0x00,	/* 0xA6 */
	0x00,	/* 0xA7 */
	0x00,	/* 0xA8 */
	0x00,	/* 0xA9 */
	0x00,	/* 0xAA */
	0x00,	/* 0xAB */
	0x00,	/* 0xAC */
	0x00,	/* 0xAD */
	0x00,	/* 0xAE */
	0x00,	/* 0xAF */
	0x00,	/* 0xB0 */
	0x00,	/* 0xB1 */
	0x00,	/* 0xB2 */
	0x00,	/* 0xB3 */
	0x00,	/* 0xB4 */
	SC_KEYPADSLASH,	/* 0xB5 */
	0x00,	/* 0xB6 */
	0x00,	/* 0xB7 */
	SC_RIGHTALT,	/* 0xB8 */
	0x00,	/* 0xB9 */
	0x00,	/* 0xBA */
	0x00,	/* 0xBB */
	0x00,	/* 0xBC */
	0x00,	/* 0xBD */
	0x00,	/* 0xBE */
	0x00,	/* 0xBF */
	0x00,	/* 0xC0 */
	0x00,	/* 0xC1 */
	0x00,	/* 0xC2 */
	0x00,	/* 0xC3 */
	0x00,	/* 0xC4 */
	SC_CLEAR,	/* 0xC5 */
	0x00,	/* 0xC6 */
	SC_HOME,	/* 0xC7 */
	SC_UPARROW,	/* 0xC8 */
	SC_PAGEUP,	/* 0xC9 */
	0x00,	/* 0xCA */
	SC_LEFTARROW,	/* 0xCB */
	0x00,	/* 0xCC */
	SC_RIGHTARROW,	/* 0xCD */
	0x00,	/* 0xCE */
	SC_END,	/* 0xCF */
	SC_DOWNARROW,	/* 0xD0 */
	SC_PAGEDOWN,	/* 0xD1 */
	SC_INSERT,	/* 0xD2 */
	SC_DELETE,	/* 0xD3 */
	0x00,	/* 0xD4 */
	0x00,	/* 0xD5 */
	0x00,	/* 0xD6 */
	0x00,	/* 0xD7 */
	0x00,	/* 0xD8 */
	0x00,	/* 0xD9 */
	0x00,	/* 0xDA */
	SC_LEFTOPTION,	/* 0xDB */
	SC_RIGHTOPTION,	/* 0xDC */
	0x00,	/* 0xDD */
	0x00,	/* 0xDE */
	0x00,	/* 0xDF */
	0x00,	/* 0xE0 */
	0x00,	/* 0xE1 */
	0x00,	/* 0xE2 */
	0x00,	/* 0xE3 */
	0x00,	/* 0xE4 */
	0x00,	/* 0xE5 */
	0x00,	/* 0xE6 */
	0x00,	/* 0xE7 */
	0x00,	/* 0xE8 */
	0x00,	/* 0xE9 */
	0x00,	/* 0xEA */
	0x00,	/* 0xEB */
	0x00,	/* 0xEC */
	0x00,	/* 0xED */
	0x00,	/* 0xEE */
	0x00,	/* 0xEF */
	0x00,	/* 0xF0 */
	0x00,	/* 0xF1 */
	0x00,	/* 0xF2 */
	0x00,	/* 0xF3 */
	0x00,	/* 0xF4 */
	0x00,	/* 0xF5 */
	0x00,	/* 0xF6 */
	0x00,	/* 0xF7 */
	0x00,	/* 0xF8 */
	0x00,	/* 0xF9 */
	0x00,	/* 0xFA */
	0x00,	/* 0xFB */
	0x00,	/* 0xFC */
	0x00,	/* 0xFD */
	0x00,	/* 0xFE */
	0x00	/* 0xFF */
};

static Word Recurse;		/* Prevent recursion for ProcessSystemEvents() !! */
Word Win95NoProcess;		/* If true, then don't process events */

/**********************************

	Convert a windows scancode to a burgerlib scan code
	
**********************************/

static INLINECALL Word Win95ToScan(Word32 ScanCode)
{
	Word Temp;
	Temp = (ScanCode>>16)&0x7F;	/* Isolate the scan code */
	if (ScanCode&0x1000000) {		/* Extended table? */
		Temp |= 0x80;
	}
	return NormalScan[Temp];	/* Convert to virtual scan code */

}

/**********************************

	Accept a key down event and pass ALL
	key down events into the event queue

**********************************/

static INLINECALL void Win95KeyDown(Word32 ScanCode)
{
	KeyArray[Win95ToScan(ScanCode)] |= 3;		/* It's pressed */
}

/**********************************

	Accept a key up event

**********************************/

static INLINECALL void Win95KeyUp(Word32 ScanCode)
{
	KeyArray[Win95ToScan(ScanCode)] &= ~1;		/* It's released */
}

/**********************************

	Allow burgerlib to process keyboard and mouse events

**********************************/

Word BURGERCALL Win95ProcessMessage(MSG *MessagePtr)
{
	int Delta;
	Word Key;

	if (MessagePtr->message==RWMMouseWheel) {		/* Needed for Win95 MouseWheel support */
		Delta = (short)MessagePtr->wParam;
		Win95MouseWheel+=Delta;
	}

	switch (MessagePtr->message) {
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:			/* Doogie wants keys */
		Key = ScanToAscii(MessagePtr);
		if (Key) {
			BurgerPostKey(Key);	/* Send the ASCII */
		}
		Win95KeyDown(MessagePtr->lParam);		/* Mark for KeyArray[] */
		if (MessagePtr->message==WM_SYSKEYDOWN) {
			if (MessagePtr->wParam == VK_MENU || MessagePtr->wParam == VK_F10) {
				return TRUE;		/* Don't dispatch */
			}
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Key = ScanToAscii(MessagePtr);
		if (Key) {
			BurgerPostKey(Key|0x8000);
		} else {
			BurgerPostKey((MessagePtr->wParam&0xFF)+(Win95ToScan(MessagePtr->lParam)<<8)+0x8000);
		}
		Win95KeyUp(MessagePtr->lParam);		/* Erase for KeyArray[] */
		break;
	case WM_CHAR:
	case WM_SYSCHAR:
		BurgerPostKey((MessagePtr->wParam&0xFF)+(Win95ToScan(MessagePtr->lParam)<<8));
		break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		Win95MouseButton |= 2;
		MouseClicked |= 2;
		
		/* Make sure that windows let's me get the mouse up event */
		/* This is to allow a mouse up event to be logged while in windowed */
		/* mode if the windows cursor moves out of the content region */
			
		if (MessagePtr->hwnd) {
			SetCapture(MessagePtr->hwnd);
		}
		break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONDBLCLK:
		Win95MouseButton |= 4;
		MouseClicked |= 4;
		if (MessagePtr->hwnd) {
			SetCapture(MessagePtr->hwnd);
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		Win95MouseButton |= 1;
		MouseClicked |= 1;
		if (MessagePtr->hwnd) {
			SetCapture(MessagePtr->hwnd);
		}
		break;
	case WM_RBUTTONUP:
		Win95MouseButton &= ~2;
		ReleaseCapture();
		break;
	case WM_MBUTTONUP:
		Win95MouseButton &= ~4;
		ReleaseCapture();
		break;
	case WM_LBUTTONUP:
		Win95MouseButton &= ~1;
		ReleaseCapture();
		break;
	case WM_MOUSEWHEEL:
		Delta = ((long)MessagePtr->wParam)>>16;
		Win95MouseWheel += Delta;
		break;
	case WM_MOUSEMOVE:
		Win95MouseX = (MessagePtr->lParam&0xFFFF);	/* Convert to local coordinates */
		Win95MouseY = (MessagePtr->lParam>>16);
		Win95MouseXDelta += Win95MouseX-Win95LastMouseX;
		Win95MouseYDelta += Win95MouseY-Win95LastMouseY;
		Win95LastMouseX = Win95MouseX;
		Win95LastMouseY = Win95MouseY;
		break;
	}
	return FALSE;
}

/**********************************

	Process window events for Win 95

**********************************/

void BURGERCALL ProcessSystemEvents(void)
{
	MSG msg;
	if (!Win95NoProcess &&	/* Can I process windows events? */
		!Recurse && 		/* Prevent recursion */
		!VideoPageLocked) {	/* Prevent a window appearing while DirectDraw */
							/* is active */
		Recurse = TRUE;		/* Don't recurse */
		while (PeekMessage(&msg,0,0,0,PM_REMOVE)) {	/* Check for next event */
			if (!Win95ProcessMessage(&msg)) {	/* Preprocess the message with Burgerlib */
				TranslateMessage(&msg);		/* Translate the keyboard (Localize) */
				DispatchMessage(&msg);		/* Pass to the window event proc */
			}
		}
		Recurse = FALSE;		/* Recursion is over */
	}
}

/**********************************

	Windows 95 uses a window def proc to scan
	the keyboard.

**********************************/

void BURGERCALL KeyboardInit(void)
{
	FastMemSet((void *)KeyArray,0,sizeof(KeyArray));	/* Blank the array */
	BurgerKeyStart = 0;		/* No keys in the keyboard array */
	BurgerKeyEnd = 0;
	MouseClicked = FALSE;	/* The mouse is not held down */
	RWMMouseWheel = RegisterWindowMessage("MSWHEEL_ROLLMSG");		/* Allow old Win95 mouse wheel support */
}

void BURGERCALL KeyboardDestroy(void)
{
}

#endif
