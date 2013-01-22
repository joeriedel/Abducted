#include "InInput.h"
#include "MmMemory.h"
#include "ClStdLib.h"
#include "KeyCodes.h"

#if defined(__WIN32__)
#include "W9Win95.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

static Word PollingRecurse;					/* Recursion flag for polling procs */
static PollProcs_t **MasterKbHitProcList;	/* Master polled procs */

static ScanEntry_t ScanCodeNames[] = {
	{"Escape",SC_ESCAPE},
	{"Esc",SC_ESCAPE},
	{"0",SC_0},
	{"1",SC_1},
	{"2",SC_2},
	{"3",SC_3},
	{"4",SC_4},
	{"5",SC_5},
	{"6",SC_6},
	{"7",SC_7},
	{"8",SC_8},
	{"9",SC_9},
	{"A",SC_A},
	{"B",SC_B},
	{"C",SC_C},
	{"D",SC_D},
	{"E",SC_E},
	{"F",SC_F},
	{"G",SC_G},
	{"H",SC_H},
	{"I",SC_I},
	{"J",SC_J},
	{"K",SC_K},
	{"L",SC_L},
	{"M",SC_M},
	{"N",SC_N},
	{"O",SC_O},
	{"P",SC_P},
	{"Q",SC_Q},
	{"R",SC_R},
	{"S",SC_S},
	{"T",SC_T},
	{"U",SC_U},
	{"V",SC_V},
	{"W",SC_W},
	{"X",SC_X},
	{"Y",SC_Y},
	{"Z",SC_Z},
	{"F1",SC_F1},
	{"F2",SC_F2},
	{"F3",SC_F3},
	{"F4",SC_F4},
	{"F5",SC_F5},
	{"F6",SC_F6},
	{"F7",SC_F7},
	{"F8",SC_F8},
	{"F9",SC_F9},
	{"F10",SC_F10},
	{"F11",SC_F11},
	{"F12",SC_F12},
	{"-",SC_MINUS},
	{"=",SC_PLUS},
	{"BakSpc",SC_BACKSPACE},
	{"Tab",SC_TAB},
	{"[",SC_LEFTBRACE},
	{"]",SC_RIGHTBRACE},
	{";",SC_SEMICOLON},
	{"'",SC_QUOTE},
	{"`",SC_TILDE},
	{"\\",SC_BACKSLASH},
	{",",SC_COMMA},
	{".",SC_PERIOD},
	{"/",SC_SLASH},
	{"Enter",SC_RETURN},
	{"LCtrl",SC_LEFTCONTROL},
	{"RCtrl",SC_RIGHTCONTROL},
	{"LShift",SC_LEFTSHIFT},
	{"RShift",SC_RIGHTSHIFT},
	{"LAlt",SC_LEFTALT},
	{"RAlt",SC_RIGHTALT},
	{"Space",SC_SPACE},
	{"CapLck",SC_CAPSLOCK},
	{"NumLck",SC_NUMLOCK},
	{"ScrLck",SC_SCROLLLOCK},
	{"Pause",SC_PAUSE},
	{"Up",SC_UPARROW},
	{"Down",SC_DOWNARROW},
	{"Left",SC_LEFTARROW},
	{"Right",SC_RIGHTARROW},
	{"Insert",SC_INSERT},
	{"Delete",SC_DELETE},
	{"Home",SC_HOME},
	{"End",SC_END},
	{"PgUp",SC_PAGEUP},
	{"PgDn",SC_PAGEDOWN},
	{"PrtScn",SC_PRINTSCREEN},
	{"KPad0",SC_KEYPAD0},
	{"KPad1",SC_KEYPAD1},
	{"KPad2",SC_KEYPAD2},
	{"KPad3",SC_KEYPAD3},
	{"KPad4",SC_KEYPAD4},
	{"KPad5",SC_KEYPAD5},
	{"KPad6",SC_KEYPAD6},
	{"KPad7",SC_KEYPAD7},
	{"KPad8",SC_KEYPAD8},
	{"KPad9",SC_KEYPAD9},
	{"KPad*",SC_KEYPADASTERISK},
	{"KPad-",SC_KEYPADMINUS},
	{"KPad+",SC_KEYPADPLUS},
	{"KPad.",SC_KEYPADPERIOD},
	{"KPad/",SC_KEYPADSLASH},
	{"KPdEnt",SC_KEYPADENTER},
	{"KPadEnt",SC_KEYPADENTER},
	{"KPadEnter",SC_KEYPADENTER},
	{0,(Word)-1}
};


/**********************************

	Header:
		Data for key scan codes

	Synopsis:
		When InterceptKey() is called, a keyboard monitor is activated
		which will check for all key strokes. There are a maximum of 128 keyboard
		scan codes that can be monitored. Each byte will contain the current status of
		the key by scan code. The lowest bit (mask with 0x01) will determine the
		keydown status at the current moment in time. Bit #1 (mask with 0x02) is set
		when the key is pressed but never cleared unless you call a function
		that clears the flag or KeyboardFlush() with flushes all events.

		This array is checked by several routines that use the keyboard as a game
		input device.

		The array is volatile since some keyboard monitors run as a separate thread.

	Also:
		KeyboardHasBeenPressed(), InterceptKey(), KeyboardHasBeenPressedClear()

**********************************/

volatile Word8 KeyArray[128];

/**********************************

	Header:
		Call back pointer for KeyboardGetch()

	Synopsis:
		When KeyboardGetch() is called, it will get a key from the keyboard cueue. If a
		function exists, it will call it which can do some post processing or global
		keyboard macro commands. Normally not used, but some text apps may use this
		to shut down a screen saver.

	Also:
		KeyboardGetch(), KeyboardKbhit()

**********************************/

KeyboardGetchCallBackPtr KeyboardGetchCallBack;	/* Key stealers */

/**********************************

	Header:
		Call all the call back functions

	Synopsis:
		This is an internal routine to call all of the keyboard polling procs.
		It is called by KeyboardKbhit(). If you don't use Burgerlib keyboard input but
		you still want to use the polling procs feature, call this function somewhere
		in your main loop. The overhead is minimal unless the polling procs were badly
		written.

	Also:
		KeyboardKbhit(), KeyboardGetch()

**********************************/

void BURGERCALL KeyboardCallPollingProcs(void)
{
	PollProcs_t **MyProc; 	/* Now the new and improved way! */

	if (!PollingRecurse) {
		PollingRecurse = TRUE;	/* Prevent recursion */
		MyProc = MasterKbHitProcList;	/* Get the master handle */
		if (MyProc) { 					/* Any in the list? */
			do {
				PollProcs_t **Next;
				Next = MyProc[0]->Next;		/* Next in the chain */
				MyProc[0]->Proc(MyProc[0]->Data);	/* Call the proc */
				MyProc=Next;				/* Next one */
			} while (MyProc); 				/* Any more? */
		}
		PollingRecurse = FALSE;				/* Allow recursion now */
	}
}

/**********************************

	Header:
		Add a KeyboardKbhit polling routine

	Synopsis:
		Given a proc pointer and a pointer to data to pass to the
		proc pointer, add this to the list of procs that are called with
		each call to KeyboardCallPollingProcs(). The pointer Data is not
		used by the polling manager itself. Only the polling proc uses the
		pointer for it's internal use.

		Before system shutdown, you must remove the proc with a call to
		RemoveKbhitPollRoutine() to release memory.

	Input:
		Proc = Pointer to a function of type void BURGERCALL Proc(void *)
		Data = Pointer that is passed to the Proc upon calling. Not used by the input manager.

	Also:
		KeyboardCallPollingProcs(), RemoveKbhitPollRoutine()

**********************************/

void BURGERCALL KeyboardAddRoutine(KeyboardCallBack Proc,void *Data)
{
	PollProcs_t **NewEnt;

	NewEnt = (PollProcs_t **)AllocAHandle(sizeof(PollProcs_t));
	if (NewEnt) {
		PollProcs_t *NewPtr;
		NewPtr = NewEnt[0];		/* Deref the handle */
		NewPtr->Proc = Proc;	/* Save the proc */
		NewPtr->Data = Data;
		NewPtr->Next = MasterKbHitProcList;	/* Link it in */
		MasterKbHitProcList = NewEnt;	/* New master handle */
	}
}

/**********************************

	Header:
		Remove a Kbhit polling routine

	Synopsis:
		Given a proc pointer and a pointer to data to pass to the
		proc pointer, search the proc list and if a match is found, remove
		the proc from the list.

	Input:
		Proc = Pointer to a function of type void BURGERCALL Proc(void *)
		Data = Pointer that is passed to the Proc upon calling. Not used by the input manager.

	Also:
		KeyboardCallPollingProcs(), AddKbhitPollRoutine()

**********************************/

void BURGERCALL KeyboardRemoveRoutine(KeyboardCallBack Proc,void *Data)
{
	PollProcs_t **NewEnt;
	PollProcs_t **PrevEnt;

	NewEnt = MasterKbHitProcList;	/* Get the master handle */
	if (NewEnt) {				/* Is it valid? */
		PollProcs_t *NewPtr;

		PrevEnt = 0;		/* Assume no previous handle */
		do {
			NewPtr = NewEnt[0];		/* Deref the handle */
			if (NewPtr->Proc==Proc && NewPtr->Data==Data) {	/* Match? */
				if (PrevEnt) {		/* Was there a previous entry? */
					(*PrevEnt)->Next = NewPtr->Next;	/* Pass the link */
				} else {
					MasterKbHitProcList = NewPtr->Next;	/* New master link */
				}
				DeallocAHandle((void **)NewEnt);		/* Dispose of the handle */
				break;			/* Exit */
			}
			PrevEnt = NewEnt;		/* Save the handle as previous */
			NewEnt = NewPtr->Next;	/* Get the next one */
		} while (NewEnt);		/* Still more? */
	}
}


/********************************

	Clear the key event in the specific scan code.
	If the code is invalid, do nothing

********************************/

void BURGERCALL KeyboardClearKey(Word ScanCode)
{
	if (ScanCode<128) {
		KeyArray[ScanCode]&=1;		/* Remove the keyboard event */
	}
}

/********************************

	Check if a key is pressed at this very moment
	Return a -1 if not found or the SC_ scancode if so

********************************/

#if defined(__BIGENDIAN__)
#define MASK1 0x01000000UL
#define MASK2 0x00010000UL
#define MASK3 0x00000100UL
#else
#define MASK1 0x00000001UL
#define MASK2 0x00000100UL
#define MASK3 0x00010000UL
#endif

Word BURGERCALL KeyboardAnyPressed(void)
{
	Word32 TempVal;
	Word Key;
	Word8 *WorkPtr;

	WorkPtr = (Word8 *)&KeyArray[0];	/* Init the pointer */
	Key = 128/4;
	do {
		TempVal = ((Word32 *)WorkPtr)[0];		/* I am going to perform a Word32 */
		WorkPtr+=4;								/* test for speed */
		if (TempVal & 0x01010101UL) {			/* Test 4 in a row */
			goto Process;						/* A Hit? */
		}
	} while (--Key);			/* Count down */
	return (Word)-1;
Process:
	Key = WorkPtr-KeyArray-4;		/* Which key was pressed? */
	if (!(TempVal&MASK1)) {			/* Was it the first one? */
		++Key;
		if (!(TempVal&MASK2)) {		/* Second? */
			++Key;
			if (!(TempVal&MASK3)) {	/* Third? */
				++Key;				/* Must be the fourth */
			}
		}
	}
	return Key;
}

/********************************

	Check if the requested key is currently or previously held down. If so, return TRUE
	If the key has not been pressed or the scan code is invalid, return FALSE

********************************/

Word BURGERCALL KeyboardHasBeenPressed(Word ScanCode)
{
	if (ScanCode<128 && KeyArray[ScanCode]&2) {
		return TRUE;
	}
	return FALSE;
}


/********************************

	Check if the requested key is currently or previously held down. If so, return TRUE
	If the key has not been pressed or the scan code is invalid, return FALSE

********************************/

Word BURGERCALL KeyboardHasBeenPressedClear(Word ScanCode)
{
	if (ScanCode<128) {
		Word Temp;
		Temp = KeyArray[ScanCode];
		if (Temp&2) {
			KeyArray[ScanCode] = static_cast<Word8>(Temp&1);
			return TRUE;
		}
	}
	return FALSE;
}

/********************************

	Check if the requested key is currently held down. If so, return TRUE
	If the key is not pressed or the scan code is invalid, return FALSE

********************************/

Word BURGERCALL KeyboardIsPressed(Word ScanCode)
{
	if (ScanCode<128) {
		return KeyArray[ScanCode]&1;
	}
	return FALSE;
}


/********************************

	Check if a key is pending and return zero if not, else the
	ASCII code in A

********************************/

Word BURGERCALL KeyboardGet(void)
{
	if (!KeyboardKbhit()) {	/* Key pending? */
		return 0;
	}
	return KeyboardGetch();	/* Return the key */
}


/********************************

	Check if a key is pending and return zero if not, else the
	ASCII code in A

********************************/

Word BURGERCALL KeyboardGetKeyLC(void)
{
	Word Key;
	if (!KeyboardKbhit()) {	/* Key pending? */
		return 0;
	}
	Key = KeyboardGetch();	/* Return the key */
	if (Key>='A' && Key<('Z'+1)) {		/* Convert to lower case */
		Key += 32;
	}
	return Key;
}
/********************************

	Check if a key is pending and return zero if not, else the
	ASCII code in A

********************************/

Word BURGERCALL KeyboardGetKeyUC(void)
{
	Word Key;
	if (!KeyboardKbhit()) {	/* Key pending? */
		return 0;
	}
	Key = KeyboardGetch();	/* Return the key */
	if (Key>='a' && Key<('z'+1)) {		/* Convert to upper case */
		Key = Key-32;
	}
	return Key;
}

/**********************************

	Flush the keyboard buffer

**********************************/

void BURGERCALL KeyboardFlush(void)
{
	while (KeyboardKbhit()) {
		KeyboardGetch();
	}
	FastMemSet((char *)&KeyArray[0],0,sizeof(KeyArray));
	MouseClicked = 0;		/* Flush the mouse */
}

/********************************

	Wait for a keypress, but call KeyboardKbhit to allow
	screen savers to kick in.

********************************/

Word BURGERCALL KeyboardWait(void)
{
	while (!KeyboardKbhit()) {
#if defined(__WIN32__)		/* Sleep the application until a key is pressed */
		Word Old;
		Old = TickWakeUpFlag;
		TickWakeUpFlag = TRUE;	/* Get wake up events */
		WaitMessage();		/* Any messages arrived? */
		TickWakeUpFlag = Old;
#endif
	}	/* Key pending? */
	return KeyboardGetch();	/* Return the key */
}

/**********************************

	Header:
		Convert a string to a scan code

	Synopsis:
		Given a pointer to a string describing a keyboard scan code. Look up
		the scan code and return the value. I return a -1 if there is no
		match.

	Input:
		StringPtr = Pointer to a "C" string

	Returns:
		Burgerlib scancode (Found in KeyCodes.h) or a -1 if no match is found.
		
	Also:
		KeyboardScanCodeToString()

**********************************/

Word BURGERCALL KeyboardStringToScanCode(const char *StringPtr)
{
	ScanEntry_t *EntryPtr;
	EntryPtr = ScanCodeNames;		/* Pointer to the array */
	do {
		if (!stricmp(StringPtr,EntryPtr->NameStr)) {	/* Match? */
			return EntryPtr->ScanCode;		/* Return the match */
		}
		++EntryPtr;				/* Next entry */
	} while (EntryPtr->NameStr);		/* All done? */
	return (Word)-1;			/* Return the bogus entry code */
}

/**********************************

	Header:
		Convert a scan code into a string

	Synopsis:
		Given a Burgerlib keyboard scan code, return a string that best
		describes the scan code. If StringSize is 0, nothing will be done since
		the output buffer is invalid.

	Input:
		StringPtr = Pointer to a "C" string buffer to store the result
		StringSize = sizeof() the buffer to prevent overruns
		ScanCode = Valid Burgerlib scan code

	Returns:
		StringPtr will contain the valid string. A null string will be returned
		if the scan code is not recognized.
		
	Also:
		KeyboardStringToScanCode()

**********************************/

void BURGERCALL KeyboardScanCodeToString(char *StringPtr,Word StringSize,Word ScanCode)
{
	ScanEntry_t *EntryPtr;
	if (StringSize) {
		EntryPtr = ScanCodeNames;
		do {
			if (EntryPtr->ScanCode==ScanCode) {
				ScanCode = strlen(EntryPtr->NameStr);	/* Length of the string */
				if (ScanCode>=StringSize) {
					ScanCode = StringSize-1;			/* Maximum to copy */
				}
				FastMemCpy(StringPtr,EntryPtr->NameStr,ScanCode);	/* Copy the string */
				StringPtr[ScanCode] = 0;				/* Zero terminate */
				return;
			}
			++EntryPtr;
		} while (EntryPtr->NameStr);
		StringPtr[0] = 0;		/* Don't return the string */
	}
}

#if !defined(__MAC__)

/**********************************

	If input is active and stopping OS user interface
	code, this will allow input to go to a passive mode.

**********************************/

Word BURGERCALL InputSetState(Word /* ActiveFlag */)
{
	return FALSE;		/* Must always be passive mode */
}

/**********************************

	Return the current state of input
	
**********************************/

Word BURGERCALL InputGetState(void)
{
	return FALSE;		/* Always passive mode */
}

#endif


#if !defined(__MAC__) && !defined(__MSDOS__) && !defined(__WIN32__) && !defined(__BEOS__) && !defined(__MACOSX__)

/**********************************

	Wait for a keyboard event, if the event
	is an extended key then translate to
	Burgerlib extended ASCII

**********************************/

Word BURGERCALL KeyboardGetch(void)
{
	Word NewKey;
	NewKey = 0;
	if (KeyboardGetchCallBack) {
		NewKey = KeyboardGetchCallBack(NewKey);		/* Keyboard filter? */
	}
	return NewKey;	/* Convert to proper ASCII code */
}

/**********************************

	See if a key is pending from the keyboard

**********************************/

Word BURGERCALL KeyboardKbhit(void)
{
	KeyboardCallPollingProcs();
	return 0;		/* No event */
}

/**********************************

	Install the heartbeat task to periodacally
	refresh the KeyArray

**********************************/

void BURGERCALL KeyboardInit(void)
{
	FastMemSet((char *)&KeyArray[0],0,sizeof(KeyArray));	/* Blank the array */
	MouseClicked = FALSE;	/* The mouse is not held down */
}

void BURGERCALL KeyboardDestroy(void)
{
}

/**********************************

	Get a key from the keyboard buffer but include key up events

**********************************/

Word BURGERCALL KeyboardGet2(void)
{
	return KeyboardGet();
}

#endif
