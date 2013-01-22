#include "InInput.h"

#if defined(__MAC__)
#include "MmMemory.h"
#include "ClStdLib.h"
#include "TkTick.h"
#include "GrGraphics.h"
#include "ThThreads.h"
#include "KeyCodes.h"
#include <stdlib.h>
#include <Events.h>
#include "McMac.h"
#include <DrawSprocket.h>
#include <Devices.h>
#include <DiskInit.h>
#include <LowMem.h>
#include <stdio.h>
#include <InputSprocket.h>
#include <TextUtils.h>

/**********************************

	Globals

**********************************/

#define INITIALDELAY 30
#define REPEATDELAY 3

Word KeyModifiers;		/* If a key is read, pass back the keyboard modifiers */
Word ScanCode;			/* Scan code of key last read */
Word MacLastKeyDown;		/* Post keydown event */
static Word32 MacLastKeyDownTime;	/* Time before key was pressed */
static Word32 MacKeyTimer;

typedef struct InputKeysDesc_t {
//	const char *KeyName;		/* String to match to */
	Word8 SprocketIndex;			/* Sprocket index */
	Word8 ScanCode;				/* Scan code for Burgerlib */
	Word8 AsciiCode;
	Word8 ShiftCode;
	Word8 ControlCode;
} InputKeysDesc_t;

#define F(x,y) (y)

static const InputKeysDesc_t KeyDesc[] = {
{F("1",18),SC_1,'1','!','1'},
{F("2",19),SC_2,'2','@','2'},
{F("3",20),SC_3,'3','#','3'},
{F("4",21),SC_4,'4','$','4'},
{F("5",23),SC_5,'5','%','5'},
{F("6",22),SC_6,'6','^','6'},
{F("7",26),SC_7,'7','&','7'},
{F("8",28),SC_8,'8','*','8'},
{F("9",25),SC_9,'9','(','9'},
{F("0",29),SC_0,'0',')','0'},
{F("A",0),SC_A,'a','A','A'-64},
{F("B",11),SC_B,'b','B','B'-64},
{F("C",8),SC_C,'c','C','C'-64},
{F("D",2),SC_D,'d','D','D'-64},
{F("E",14),SC_E,'e','E','E'-64},
{F("F",3),SC_F,'f','F','F'-64},
{F("G",5),SC_G,'g','G','G'-64},
{F("H",4),SC_H,'h','H','H'-64},
{F("I",34),SC_I,'i','I','I'-64},
{F("J",38),SC_J,'j','J','J'-64},
{F("K",40),SC_K,'k','K','K'-64},
{F("L",37),SC_L,'l','L','L'-64},
{F("M",46),SC_M,'m','M','M'-64},
{F("N",45),SC_N,'n','N','N'-64},
{F("O",31),SC_O,'o','O','O'-64},
{F("P",35),SC_P,'p','P','P'-64},
{F("Q",12),SC_Q,'q','Q','Q'-64},
{F("R",15),SC_R,'r','R','R'-64},
{F("S",1),SC_S,'s','S','S'-64},
{F("T",17),SC_T,'t','T','T'-64},
{F("U",32),SC_U,'u','U','U'-64},
{F("V",9),SC_V,'v','V','V'-64},
{F("W",13),SC_W,'w','W','W'-64},
{F("X",7),SC_X,'x','X','X'-64},
{F("Y",16),SC_Y,'y','Y','Y'-64},
{F("Z",6),SC_Z,'z','Z','Z'-64},
{F("=",24),SC_EQUALS,'=','+','='},
{F("-",27),SC_MINUS,'-','_','-'},
{F("]",30),SC_RIGHTBRACE,']','}',']'},
{F("[",33),SC_LEFTBRACE,'[','{','['},
{F("'",39),SC_QUOTE,'\'','"','\''},
{F(";",41),SC_SEMICOLON,';',':',';'},
{F("\\",42),SC_BACKSLASH,'\\','|','\\'},
{F(",",43),SC_COMMA,',','<',','},
{F("/",44),SC_SLASH,'/','?','/'},
{F(".",47),SC_PERIOD,'.','>','.'},
{F("`",50),SC_TILDE,'`','~','`'},
{F("F1",114),SC_F1,ASCII_F1,ASCII_F1,ASCII_F1},
{F("F2",112),SC_F2,ASCII_F2,ASCII_F2,ASCII_F2},
{F("F3",91),SC_F3,ASCII_F3,ASCII_F3,ASCII_F3},
{F("F4",110),SC_F4,ASCII_F4,ASCII_F4,ASCII_F4},
{F("F5",88),SC_F5,ASCII_F5,ASCII_F5,ASCII_F5},
{F("F6",89),SC_F6,ASCII_F6,ASCII_F6,ASCII_F6},
{F("F7",90),SC_F7,ASCII_F7,ASCII_F7,ASCII_F7},
{F("F8",92),SC_F8,ASCII_F8,ASCII_F8,ASCII_F8},
{F("F9",93),SC_F9,ASCII_F9,ASCII_F9,ASCII_F9},
{F("F10",101),SC_F10,ASCII_F10,ASCII_F10,ASCII_F10},
{F("F11",95),SC_F11,ASCII_F11,ASCII_F11,ASCII_F11},
{F("F12",103),SC_F12,ASCII_F12,ASCII_F12,ASCII_F12},
{F("F13",97),SC_PRINTSCREEN,ASCII_PRINTSCREEN,ASCII_PRINTSCREEN,ASCII_PRINTSCREEN},
{F("F14",99),SC_SCROLLLOCK,ASCII_SCROLLLOCK,ASCII_SCROLLLOCK,ASCII_SCROLLLOCK},
{F("F15",105),SC_PAUSE,ASCII_PAUSE,ASCII_PAUSE,ASCII_PAUSE},
{F("Left",67),SC_LEFTARROW,ASCII_LEFTARROW,ASCII_LEFTARROW,ASCII_LEFTARROW},
{F("Right",64),SC_RIGHTARROW,ASCII_RIGHTARROW,ASCII_RIGHTARROW,ASCII_RIGHTARROW},
{F("Down",69),SC_DOWNARROW,ASCII_DOWNARROW,ASCII_DOWNARROW,ASCII_DOWNARROW},
{F("Up",72),SC_UPARROW,ASCII_UPARROW,ASCII_UPARROW,ASCII_UPARROW},
{F("Left",115),SC_LEFTARROW,ASCII_LEFTARROW,ASCII_LEFTARROW,ASCII_LEFTARROW},
{F("Right",116),SC_RIGHTARROW,ASCII_RIGHTARROW,ASCII_RIGHTARROW,ASCII_RIGHTARROW},
{F("Down",117),SC_DOWNARROW,ASCII_DOWNARROW,ASCII_DOWNARROW,ASCII_DOWNARROW},
{F("Up",118),SC_UPARROW,ASCII_UPARROW,ASCII_UPARROW,ASCII_UPARROW},
{F("Clear",68),SC_NUMLOCK},
{F("Num =",74),SC_KEYPADEQUALS,'=','=','='},
{F("Num /",70),SC_KEYPADSLASH,'/','/','/'},
{F("Num *",65),SC_KEYPADASTERISK,'*','*','*'},
{F("Num .",63),SC_KEYPADPERIOD,'.','.','.'},
{F("Num -",73),SC_KEYPADMINUS,'-','-','-'},
{F("Num +",66),SC_KEYPADPLUS,'+','+','+'},
{F("Enter",52),SC_KEYPADENTER,13,13,13},
{F("Enter",71),SC_KEYPADENTER,13,13,13},
{F("Num 0",75),SC_KEYPAD0,'0','0','0'},
{F("Num 1",76),SC_KEYPAD1,'1','1','1'},
{F("Num 2",77),SC_KEYPAD2,'2','2','2'},
{F("Num 3",78),SC_KEYPAD3,'3','3','3'},
{F("Num 4",79),SC_KEYPAD4,'4','4','4'},
{F("Num 5",80),SC_KEYPAD5,'5','5','5'},
{F("Num 6",81),SC_KEYPAD6,'6','6','6'},
{F("Num 7",82),SC_KEYPAD7,'7','7','7'},
{F("Num 8",83),SC_KEYPAD8,'8','8','8'},
{F("Num 9",84),SC_KEYPAD9,'9','9','9'},
{F("Tab",48),SC_TAB,9,9,9},
{F("Space",49),SC_SPACE,32,32,32},
{F("Escape",53),SC_ESCAPE,27,27,27},
{F("Return",36),SC_RETURN,13,13,13},
{F("Help",106),SC_INSERT,ASCII_INSERT,ASCII_INSERT,ASCII_INSERT},
{F("Home",107),SC_HOME,ASCII_HOME,ASCII_HOME,ASCII_HOME},
{F("Page Up",108),SC_PAGEUP,ASCII_PAGEUP,ASCII_PAGEUP,ASCII_PAGEUP},
{F("Del",109),SC_DELETE,ASCII_DELETE,ASCII_DELETE,ASCII_DELETE},
{F("End",111),SC_END,ASCII_END,ASCII_END,ASCII_END},
{F("Page Down",113),SC_PAGEDOWN,ASCII_PAGEDOWN,ASCII_PAGEDOWN,ASCII_PAGEDOWN},
{F("Delete",51),SC_BACKSPACE,ASCII_BACKSPACE,ASCII_BACKSPACE,ASCII_BACKSPACE},
{F("Shift",55),SC_LEFTSHIFT},
{F("Caps Lock",56),SC_CAPSLOCK},
{F("Option",57),SC_LEFTOPTION},
{F("Control",58),SC_LEFTCONTROL},
{F("Right Shift",59),SC_RIGHTSHIFT},
{F("Right Option",60),SC_RIGHTOPTION},
{F("Right Control",61),SC_RIGHTCONTROL},
{F("\21",54),SC_LEFTOPENAPPLE},
{F("PPP",21),SC_4,'4','$','4'},
//{F("¤",10),SC_SPACE,'5',32,32},
//{F("Fn",62),SC_SPACE,'0',32,32},
//{F("Yen",85),SC_SPACE,'1',32,32},
//{F("Ro",86),SC_SPACE,'2',32,32},
//{F("Num ,"87),SC_SPACE,'3',32,32},
//{F("Eisuu",94),SC_SPACE,'4',32,32},
//{F("Kana,96),SC_SPACE,'5',32,32},
{F(0,255)}
};

#undef F

static Word BurgerKeyStart;					/* Internal global for KeyboardKbhit */
static Word BurgerKeyEnd;					/* Internal global for KeyboardKbhit */
static Word BurgerKeyBuffer[KEYBUFFSIZE];	/* Keyboard buffer */
static const Word ExtraKey[7] = {
	cmdKey,shiftKey,controlKey,alphaLock,
	rightShiftKey,optionKey|rightOptionKey,rightControlKey};
static const Word ExtraKeyScan[7] = {
	SC_LEFTOPENAPPLE,SC_LEFTSHIFT,SC_LEFTCONTROL,SC_CAPSLOCK,
	SC_RIGHTSHIFT,SC_RIGHTOPTION,SC_RIGHTCONTROL};
static Word ExtraPrev[7];

/**********************************

	Refresh the main screen on an update or drag event

**********************************/

static void RefreshScreen(void)
{
	if (VideoWindow && !VideoFullScreen && MacUseBackBuffer) {
		UpdateAndPageFlip();
	}
}

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

	Carbon event handler for mouse events

**********************************/

#if TARGET_API_MAC_CARBON

static const EventTypeSpec EventTypes[4] = {
	{kEventClassKeyboard,kEventRawKeyDown},
	{kEventClassKeyboard,kEventRawKeyRepeat},
	{kEventClassKeyboard,kEventRawKeyUp},
	{kEventClassKeyboard,kEventRawKeyModifiersChanged}
};
static Word OldMods;

static pascal OSStatus CarbonKeyProc(EventHandlerCallRef inHandlerCallRef,EventRef inEvent,void *inUserData)
{
	MacInput_t *LocalPtr;
	Word32 EventKind;
	Word32 ModMe;
	
	LocalPtr = (MacInput_t *)inUserData;		/* Pointer to local variables */
	ModMe = (Word32)-1;
	EventKind = GetEventKind(inEvent);			/* Type of event accepted */
	switch (EventKind) {
	
	/* Mouse up or down events */
	
	case kEventRawKeyDown:
	case kEventRawKeyRepeat:
	case kEventRawKeyUp:
		{
			Word32 KeyCode;
			Word8 MacChar;
			Word Mask;
			
			GetEventParameter(inEvent,kEventParamKeyCode,
				typeUInt32,0,sizeof(KeyCode),0,&KeyCode);
			GetEventParameter(inEvent,kEventParamKeyMacCharCodes,
				typeChar,0,sizeof(MacChar),0,&MacChar);
			GetEventParameter(inEvent,kEventParamKeyModifiers,
				typeUInt32,0,sizeof(ModMe),0,&ModMe);

			Mask = KeyCode;
			if (Mask<128) {			/* Which button */
				Word Post;
				EventRecord FakeRecord;
				Word OldMod;

				FakeRecord.message = MacChar|(Mask<<8);		/* Pass through */
				FakeRecord.modifiers = 0;
				OldMod = KeyModifiers;
				Post = FixMacKey(&FakeRecord)|(Mask<<8);	/* Convert to ASCII */
				KeyModifiers = OldMod;

				MacLastKeyDown = 0;		/* Not held down */
				if (EventKind!=kEventRawKeyUp) {		/* Mouse down */
					KeyArray[Mask] |= 3;
					BurgerPostKey(Post);
				} else {
					KeyArray[Mask] &= (~1);
					BurgerPostKey(Post|0x8000);
				}
			}

		}
		break;
	case kEventRawKeyModifiersChanged:
		GetEventParameter(inEvent,kEventParamKeyModifiers,
			typeUInt32,0,sizeof(ModMe),0,&ModMe);
		break;
	}
	
	/* Check which modifier key was changed */
	
	if (ModMe!=(Word32)-1) {
		Word part;
		Word Mod;
		Mod = ModMe;
		part = 0;
		do {
			if (Mod&ExtraKey[part]) {
				if (!ExtraPrev[part]) {
					ExtraPrev[part] = TRUE;
					KeyArray[ExtraKeyScan[part]] |= 3;
					BurgerPostKey(ExtraKeyScan[part]<<8);
				}
			} else {
				if (ExtraPrev[part]) {
					ExtraPrev[part] = FALSE;
					BurgerPostKey((ExtraKeyScan[part]<<8)|0x8000);
				}
				KeyArray[ExtraKeyScan[part]] &= ~1;
			}
		} while (++part<7);
	}
	return eventNotHandledErr;
}
#endif

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
	DoMacEvent(everyEvent,0);		/* Try other events... */
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
	DoMacEvent(everyEvent,0);		/* Try other events... */
	Start = BurgerKeyStart;		/* Get the starting index */
	if (Start!=BurgerKeyEnd) {	/* Anything in the buffer? */
		Temp = BurgerKeyBuffer[Start];		/* Scan for a key down event */
		BurgerKeyStart = (Start+1)&(KEYBUFFSIZE-1);	/* Next key */
		return Temp;
	}
	return 0;		/* No event */
}

/**********************************

	Convert a keydown record into an ASCII record
	This code is MacOS specific

**********************************/

Word FixMacKey(EventRecord *Event)
{
	Word NewKey;

	NewKey = Event->message & 0xff;			/* Mask off the ASCII */
	ScanCode = (Event->message>>8) & 0xff;	/* Get the scan code */
	KeyModifiers = Event->modifiers;	/* Save modifiers in global */
	switch (NewKey) {
	default:
		if ((KeyModifiers&(alphaLock|shiftKey)) == (alphaLock|shiftKey)) {
			if (NewKey>='A' && NewKey<='Z') {
				NewKey += 32;
			}
		}
		break;
	case 0x10:						/* Unknown key */
		switch (ScanCode) {			/* By using the scan code, I will convert to ASCII */
		case SC_F1:
			NewKey = ASCII_F1;
			break;
		case SC_F2:
			NewKey = ASCII_F2;
			break;
		case SC_F3:
			NewKey = ASCII_F3;
			break;
		case SC_F4:
			NewKey = ASCII_F4;
			break;
		case SC_F5:
			NewKey = ASCII_F5;
			break;
		case SC_F6:
			NewKey = ASCII_F6;
			break;
		case SC_F7:
			NewKey = ASCII_F7;
			break;
		case SC_F8:
			NewKey = ASCII_F8;
			break;
		case SC_F9:
			NewKey = ASCII_F9;
			break;
		case SC_F10:
			NewKey = ASCII_F10;
			break;
		case SC_F11:
			NewKey = ASCII_F11;
			break;
		case SC_F12:
			NewKey = ASCII_F12;
			break;
		case SC_PRINTSCREEN:
			NewKey = ASCII_PRINTSCREEN;
			break;
		case SC_PAUSE:
			NewKey = ASCII_PAUSE;
			break;
		case SC_SCROLLLOCK:
			NewKey = ASCII_SCROLLLOCK;
			break;
		case SC_BACKSPACE:
			NewKey = ASCII_BACKSPACE;
			break;
		}
		break;
	case 0x01:
		if (ScanCode==SC_HOME) {
			NewKey = ASCII_HOME;
		}
		break;
	case 0x03:
		if (ScanCode==SC_KEYPADENTER) {
			NewKey = ASCII_ENTER;
		}
		break;
	case 0x04:
		if (ScanCode==SC_END) {
			NewKey = ASCII_END;
		}
		break;
	case 0x05:
		if (ScanCode==SC_INSERT) {
			NewKey = ASCII_INSERT;
		}
		break;
	case 0x0D:
		if (ScanCode==SC_PAGEUP) {
			NewKey = ASCII_PAGEUP;
		}
		break;
	case 0x0C:
		if (ScanCode==SC_PAGEDOWN) {
			NewKey = ASCII_PAGEDOWN;
		}
		break;
	case 0x1B:
		if (ScanCode==SC_NUMLOCK) {
			NewKey = 0;
		}
		break;
	case 0x1C:			/* Convert Mac ASCII to something more sensible */
		NewKey = ASCII_LEFTARROW;
		break;
	case 0x1D:
		NewKey = ASCII_RIGHTARROW;
		break;
	case 0x1E:
		NewKey = ASCII_UPARROW;
		break;
	case 0x1F:
		NewKey = ASCII_DOWNARROW;
		break;
	case 0x7F:
		NewKey = ASCII_DELETE;
		break;
	}
	return NewKey;		/* Return ASCII */
}

/**********************************

	Convert the scan code to ASCII

**********************************/

static Word MacFakeKey(Word ScanCodex)
{
	Word NewKey;
	Word Shift;
	Word Control;
	const InputKeysDesc_t *KeyPtr;
	
	NewKey = 0;				/* Init the output */
	KeyPtr = KeyDesc;
	ScanCode = ScanCodex;		/* Save the scan code */

	/* Scan for a match for the scan code */
	do {
		if (KeyPtr->ScanCode==ScanCodex) {		/* Found it! */
			Shift = FALSE;
			Control = FALSE;
			if (KeyArray[SC_LEFTCONTROL]&1 || KeyArray[SC_RIGHTCONTROL]&1) {
				Control = TRUE;			/* Control is held down */
			}
			if (KeyArray[SC_LEFTSHIFT]&1 || KeyArray[SC_RIGHTSHIFT]&1) {
				Shift = TRUE;			/* Shift is held down */
			}
			if (Control) {				/* Control has precedence */
				if (!Shift) {			/* Shift not held down? */
					NewKey = KeyPtr->ControlCode;	/* ASCII is a control code */
				}
			} else {
				NewKey = KeyPtr->AsciiCode;		/* Assume lower case */
				if (NewKey>='a' && NewKey<='z') {		/* Capslock affects uppercase only */
					if (KeyArray[SC_CAPSLOCK]&1) {		/* Reverse shift with Caps lock */
						Shift = Shift^TRUE;
					}
				}
				if (Shift) {					/* Upper case */
					NewKey = KeyPtr->ShiftCode;	/* Change it */
				}
			}
			break;		/* Abort now */
		}
		++KeyPtr;
	} while (KeyPtr->SprocketIndex!=255);
	
	/* Create the mac keyboard modifiers */
	
	if (Control) {
		Control = controlKey;
	}
	if (Shift) {
		Control |= shiftKey;
	}
	if (KeyArray[SC_CAPSLOCK]&1) {
		Control |= alphaLock;
	}
	if (KeyArray[SC_LEFTOPENAPPLE]&1 || KeyArray[SC_RIGHTOPENAPPLE]&1) {
		Control |= cmdKey;
	}
	if (KeyArray[SC_LEFTOPTION]&1 || KeyArray[SC_RIGHTOPTION]&1) {
		Control |= optionKey;
	}
	KeyModifiers = Control;			/* Save the keyboard modifiers */
	return (ScanCodex<<8)|NewKey;
}

#if TARGET_RT_MAC_CFM
Word BURGERCALL KeyboardReadInputSprocket(void *Input)
{
	ISpElementEvent EventHit;
	Bool HitMe;
	Word i;
	MacInput_t *LocalPtr;
	
	LocalPtr = (MacInput_t *)Input;
	if (LocalPtr->InputSprocketActive &&
		LocalPtr->InputSprocketInited &&
		(LocalPtr->Flags & MACINITINPUTKEY)) {

		/* Note: InputSprocket is interrupt safe but not reentrant */
		/* So, obtain the lock to see if I am calling InputSprocket and */
		/* skip if I'm inside ISp */
			
		if (!MacInputLockInputSprocket()) {		/* Locked out? */
			i = LocalPtr->KeyboardCount;
			if (i) {
				MacKeyEntry_t *WorkPtr;
				WorkPtr = LocalPtr->KeyLists;		/* For each mouse */
				do {
					for (;;) {
						Word j;
						Word MaxOne;
						/* See if an event was passed to me... */
						HitMe = 0;
						if (ISpElementList_GetNextEvent(WorkPtr->KeyboardRef,sizeof(EventHit),&EventHit,&HitMe)) {
							break;
						}
						if (!HitMe) {		/* No event? */
							break;
						}
						MaxOne = WorkPtr->ScanCodeCount;
						j = 0;				/* Ok, where shall I pass the event */
						do {
							if (EventHit.element == WorkPtr->KeyElements[j]) {		/* Was it a valid entry? */
								Word Mask;
								Word Post;
								Mask = WorkPtr->ScanCodes[j];		/* Mask for button */
								Post = MacFakeKey(Mask);
								MacLastKeyDown = 0;		/* Not held down */
								if (EventHit.data) {
									KeyArray[Mask] |= 3;
									BurgerPostKey(Post);
									if (Post&0xFF) {		/* Valid ASCII? */
										MacLastKeyDown = Post;					/* Set the key */
										MacLastKeyDownTime = ReadTick();		/* Time mark */
										MacKeyTimer = MacLastKeyDownTime;
									}
								} else {
									KeyArray[Mask] &= (~1);
									BurgerPostKey(Post|0x8000);
								}
								break;
							}
						} while (++j<MaxOne);
					}
					++WorkPtr;
				} while (--i);
			}
			if (MacLastKeyDown) {
				Word32 xx;
				xx = ReadTick();			/* This is reentrant on MacOS */
				if ((xx-MacLastKeyDownTime)>=INITIALDELAY) {
					if ((xx-MacKeyTimer)>=REPEATDELAY) {
						MacKeyTimer = xx;
						BurgerPostKey(MacLastKeyDown);
					}
				}
			}
			MacInputUnlockInputSprocket();
		}
	}
	return FALSE;
}
#else
Word BURGERCALL KeyboardReadInputSprocket(void *LocalPtr)
{
	return FALSE;
}
#endif

/************************************

	Do the right thing for an event. Determine what kind of
	event it is, and call the appropriate routines.
	This is like TaskMaster for the IIgs but REALLY SCALED DOWN!

************************************/

Word DoMacEvent(Word Mask,EventRecord *Event)
{
	Word part;
	WindowRef window;
	EventRecord MyEvent;
	MacInput_t *LocalPtr;
	
	LocalPtr = &MacInputLocals;
	if (Mask) {			/* Should I get an event? */
		Event = &MyEvent;					/* Set to my internal pointer */
#if !TARGET_API_MAC_CARBON
		if (MacSystemTaskFlag || !VideoFullScreen) {	/* Call system task? */
			if (!GetNextEvent(Mask,Event)) {			/* Get all events */
				Event->what = nullEvent;
			}
		} else if (!GetOSEvent(Mask,Event)) {	/* Is there an event? */
			Event->what = nullEvent;		/* Force null event */
		}
#else
		if (!GetNextEvent(Mask,Event)) {
			Event->what = nullEvent;
		}
#endif
		if (LocalPtr->CenterMouseFlag) {
			MouseSetPosition(VideoTrueScreenWidth>>1,VideoTrueScreenHeight>>1);	/* Reset the mouse to the center */
			LocalPtr->CenterMouseFlag = FALSE;
		}
	}

#if TARGET_RT_MAC_CFM
	if (LocalPtr->InputSprocketActive && 
		LocalPtr->InputSprocketInited &&
		(LocalPtr->Flags&MACINITINPUTKEY)) {
		Word32 Mark;

		Mark = ReadTick();
		if ((Mark-LocalPtr->InputSprocketTick)>(TICKSPERSEC/4)) {	/* Time elapsed... */
			LocalPtr->InputSprocketTick = Mark;			/* Set the new mark */
			ISpTickle();								/* Tickle inputsprocket */
		}

		KeyboardReadInputSprocket(LocalPtr);	/* Force the keyboard to read */
		MouseReadInputSprocket(LocalPtr);		/* Make sure I yield some CPU time to mouse services */

	} else
#endif
#if TARGET_API_MAC_CARBON
	if (!LocalPtr->KeyEventRef)
#endif
	/* Handle the input the old fashioned way */
	{
		part = 0;
		do {
			if (Event->modifiers&ExtraKey[part]) {
				if (!ExtraPrev[part]) {
					ExtraPrev[part] = TRUE;
					KeyArray[ExtraKeyScan[part]] |= 3;
					BurgerPostKey(ExtraKeyScan[part]<<8);
				}
			} else {
				if (ExtraPrev[part]) {
					ExtraPrev[part] = FALSE;
					BurgerPostKey((ExtraKeyScan[part]<<8)|0x8000);
				}
				KeyArray[ExtraKeyScan[part]] &= ~1;
			}
		} while (++part<7);
	}

	/* Allow DrawSprocket to get events */

#if TARGET_RT_MAC_CFM
	if (MacDrawSprocketActive && Event->what!=nullEvent && MacOSGetOSVersion()<0x1000) {		/* Has draw sprocket been started up? */
		
		/* Note, don't call this in OSX, it allows Alt-Tab */
		
		Bool LeaveFlag;		/* Will drawsprocket eat this event? */
		DSpProcessEvent(Event,&LeaveFlag);
		if (LeaveFlag) {
			return TRUE;		/* Yumm! */
		}
	}
#endif

	if (MacEventIntercept) {				/* Is there an intercept routine */
		if (MacEventIntercept(Event)) {		/* Call the user's intercept routine */
			return TRUE;			/* I ate the evnet */
		}
	}

	/* Process all generic events */
	
	switch (Event->what) {
	case keyDown:
	case autoKey:
		if (!LocalPtr->KeyEventRef) {
			if (!LocalPtr->InputSprocketActive || !(LocalPtr->Flags&MACINITINPUTKEY)) {
				KeyArray[(Event->message>>8)&0xFF] |= 3;
				BurgerPostKey(FixMacKey(Event)|(Event->message&0x7F00));
			}
		}
		break;
	case keyUp:
		if (!LocalPtr->KeyEventRef) {
			if (!LocalPtr->InputSprocketActive || !(LocalPtr->Flags&MACINITINPUTKEY)) {
				KeyArray[(Event->message>>8)&0xFF] &= ~1;
				BurgerPostKey(FixMacKey(Event)|(Event->message&0x7F00)|0x8000);
			}
		}
		break;
	case kHighLevelEvent:
		AEProcessAppleEvent(Event);		/* Allow high level events if applicable */
		break;
	
	case mouseDown:		/* Pressed the mouse? */
		part = FindWindow(Event->where, &window);	/* Choose the hit */
		switch (part) {
		case inMenuBar:
			MenuSelect(Event->where);
			break;
				
#if !TARGET_API_MAC_CARBON
		case inSysWindow:					/* let the system handle the mouseDown */
			SystemClick(Event,window);		/* Pass on the event */
			break;
#endif

		case inDrag:						/* Pass screenBits.bounds to get all gDevices */
			RefreshScreen();
#if !TARGET_API_MAC_CARBON
			DragWindow(window,Event->where, &qd.screenBits.bounds);
#else
			DragWindow(window,Event->where,0);
#endif
			MouseSetRange(ScreenWidth,ScreenHeight);
			break;

		case inGrow:
#if !TARGET_API_MAC_CARBON
			GrowWindow(window,Event->where, &qd.screenBits.bounds);
#else
			GrowWindow(window,Event->where,0);
#endif
			break;

		case inGoAway:
			if (TrackGoAway(window,Event->where)) {		/* Handle the close box */
#if !TARGET_API_MAC_CARBON
				int Kind;
				Kind = ((WindowPeek) window)->windowKind;		/* Get the kind */
				if (Kind<0) {
					CloseDeskAcc(Kind);
				}
#endif
			}
			break;
				
		case inZoomIn:
		case inZoomOut:
			if (TrackBox(window,Event->where,part)) {	/* Track the zoom box */
				Rect FooRect2;
				SetPortWindowPort(window);				/* the window must be the current port... */
				GetPortBounds(GetWindowPort(window),&FooRect2);
				EraseRect(&FooRect2);	/* because of a bug in ZoomWindow */
				ZoomWindow(window,part,TRUE);	/* note that we invalidate and erase... */
#if !TARGET_API_MAC_CARBON
				InvalRect(&FooRect2);
#else
				InvalWindowRect(window,&FooRect2);	/* to make things look better on-screen */
#endif
				MouseSetRange(ScreenWidth,ScreenHeight);
			}
			break;
		}
		break;
	case activateEvt:
		if (Event->message && !VideoFullScreen) {
			WindowRef TempWindow2;
			TempWindow2 = (WindowRef)Event->message;
			if (Event->modifiers & activeFlag) {
				HiliteWindow(TempWindow2,TRUE);
				LocalPtr->Dormant = FALSE;
			} else {
				HiliteWindow(TempWindow2,FALSE);
				LocalPtr->Dormant = TRUE;
			}
		}
		break;
	case updateEvt:
		{
			WindowRef TempWindow;
			GrafPtr OldPort;
			GetPort(&OldPort);
			TempWindow = (WindowRef)Event->message;
			SetPortWindowPort(TempWindow);
			BeginUpdate(TempWindow);
			RefreshScreen();
			EndUpdate(TempWindow);
			SetPort(OldPort);
		}
		break;

#if !TARGET_API_MAC_CARBON
	case diskEvt:
		if ((Event->message>>16) != noErr ) {	/* Was there an error? */
			Point aPoint;
			aPoint.h = 20;
			aPoint.v = 20;
			DIBadMount(aPoint, Event->message);	/* Format the disk? */
		}
		break;
#endif
	}
	return FALSE;			/* No event processed */
}

/**********************************

	Install the heartbeat task to periodacally
	refresh the KeyArray

**********************************/

void BURGERCALL KeyboardInit(void)
{
	MacInput_t *LocalPtr;
	
	LocalPtr = &MacInputLocals;
	FastMemSet((char *)&KeyArray[0],0,sizeof(KeyArray));	/* Blank the array */
	BurgerKeyStart = 0;		/* No keys in the keyboard array */
	BurgerKeyEnd = 0;
	MouseClicked = FALSE;	/* The mouse is not held down */
#if !TARGET_API_MAC_CARBON
	SetEventMask(LMGetSysEvtMask()|keyUpMask);
#endif
	/* Check if InputSprocket is available */
	
	if (!MacInputInit(LocalPtr,MACINITINPUTKEY)) {	/* 1.3 or better? */
#if TARGET_RT_MAC_CFM
		unsigned long TempLong;
		MacInputLockInputSprocket();
		FastMemSet(LocalPtr->KeyLists,0,sizeof(LocalPtr->KeyLists));
		if (!ISpDevices_ExtractByClass(kISpDeviceClass_Keyboard,0,&TempLong,0)) {		/* How many mice are present? */
			Word i;
			i = TempLong;
			if (i) {			/* Any mice detected? */
				ISpDeviceReference Keys[2];			/* Rats to load */
				if (i>2) {
					i = 2;	/* I'll only support 2 keyboards */
				}
				LocalPtr->KeyboardCount = i;
				if (!ISpDevices_ExtractByClass(kISpDeviceClass_Keyboard,i,&TempLong,Keys)) {	/* Get the devices */
					Word j;
					MacKeyEntry_t *JoyDevPtr;
					/* At this point, I have all the mice devices, now parse out the info */
					j = 0;
					JoyDevPtr = LocalPtr->KeyLists;
					do {

						/* For each device, scan the input elements and find a burgerlib */
						/* match, assign it a button, pad or axis id */

						if (!ISpDevice_GetElementList(Keys[j],&JoyDevPtr->KeyboardRef)) {
							unsigned long ElementCount;
							ISpElementReference ElementBuf[500];
							if (!ISpElementList_Extract(JoyDevPtr->KeyboardRef,500,&ElementCount,ElementBuf)) {
								if (ElementCount) {
									Word TempIndex;
									Word ButtonIndex;
									
									TempIndex = 0;
									ButtonIndex = 0;	/* No buttons found */
									do {
										ISpElementInfo ElementInfo;
										ISpElementReference CurrentRef;

										CurrentRef = ElementBuf[TempIndex];
										if (!ISpElement_GetInfo(CurrentRef,&ElementInfo)) {
											/* I've got input, what kind of input is it? */

											switch (ElementInfo.theKind) {
											case kISpElementKind_Button:		/* Simple button */
												if (ButtonIndex<128) {
													const InputKeysDesc_t *KeyPtr;
													KeyPtr = KeyDesc;
													do {
														if (KeyPtr->SprocketIndex==TempIndex) {
															JoyDevPtr->KeyElements[ButtonIndex] = CurrentRef;
															JoyDevPtr->ScanCodes[ButtonIndex] = KeyPtr->ScanCode;
															++ButtonIndex;
															break;
														}
														++KeyPtr;
													} while (KeyPtr->SprocketIndex!=255);
												}
												break;
											}
										}
									} while (++TempIndex<ElementCount);
									JoyDevPtr->ScanCodeCount = ButtonIndex;		/* Save the key count */
								}
							}
						}
						++JoyDevPtr;
					} while (++j<i);			/* All devices scanned */
				}
			}
		}
		ISpDevices_ActivateClass(kISpDeviceClass_Keyboard);	/* Enable keyboards */
		MacInputUnlockInputSprocket();
		LocalPtr->KeyboardTimerProc = TimerTaskNew(16,KeyboardReadInputSprocket,LocalPtr,TRUE);
#endif
	}
#if TARGET_API_MAC_CARBON
	else if (!LocalPtr->KeyEventRef) {
		EventHandlerUPP KeyProcUPP;
		KeyProcUPP = NewEventHandlerUPP(CarbonKeyProc);
		LocalPtr->KeyCarbonProc = KeyProcUPP;
		InstallEventHandler(GetApplicationEventTarget(),KeyProcUPP,4,EventTypes,&MacInputLocals,&LocalPtr->KeyEventRef);
	}
#endif
}

/**********************************

	Release the keyboard services

**********************************/

void KeyboardDestroy(void)
{
	MacInput_t *LocalPtr;
	LocalPtr = &MacInputLocals;

	if (LocalPtr->KeyboardTimerProc) {
		TimerTaskDelete(LocalPtr->KeyboardTimerProc);	/* Kill the timer IRQ thread */
		LocalPtr->KeyboardTimerProc = 0;
	}

	if (LocalPtr->Flags & MACINITINPUTKEY) {
#if TARGET_RT_MAC_CFM
		if (LocalPtr->InputSprocketActive) {
			ISpDevices_DeactivateClass(kISpDeviceClass_Keyboard);	/* Disable mice support */
		}
#endif
		MacInputDestroy(LocalPtr,MACINITINPUTKEY);	/* 1.3 or better? */
	}
#if TARGET_API_MAC_CARBON
	if (LocalPtr->KeyEventRef) {
		RemoveEventHandler((EventHandlerRef)LocalPtr->KeyEventRef);
		LocalPtr->KeyEventRef = 0;
	}
	if (LocalPtr->KeyCarbonProc) {
		DisposeEventHandlerUPP((EventHandlerUPP)LocalPtr->KeyCarbonProc);
		LocalPtr->KeyCarbonProc = 0;
	}
#endif
}

#endif
