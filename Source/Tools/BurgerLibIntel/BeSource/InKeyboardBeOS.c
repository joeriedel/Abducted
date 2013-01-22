#include "InInput.h"

/**********************************

	BeOS version

**********************************/

#if defined(__BEOS__)

#include "LWMemory.h"
#include "LWStdLib.h"
#include "KeyCodes.h"
#include <Application.h>

static volatile Word BurgerKeyStart;		/* Internal global for KeyboardKbhit */
static volatile Word BurgerKeyEnd;			/* Internal global for KeyboardKbhit */
static Word8 BurgerKeyBuffer[KEYBUFFSIZE];	/* Keyboard buffer */

/**********************************

	Remap a BeOS scan code to a Buregerlib scan code

**********************************/

static Word8 NewCodes[128] = {
	0x7f,SC_ESCAPE,SC_F1,SC_F2,SC_F3,SC_F4,SC_F5,SC_F6,	// 00-07
	SC_F7,SC_F8,SC_F9,SC_F10,SC_F11,SC_F12,SC_PRINTSCREEN,SC_SCROLLLOCK,	// 08-0F
	SC_PAUSE,SC_TILDE,SC_1,SC_2,SC_3,SC_4,SC_5,SC_6,	// 10-17
	SC_7,SC_8,SC_9,SC_0,SC_MINUS,SC_EQUALS,SC_BACKSPACE,SC_INSERT,	// 18-1F
	SC_HOME,SC_PAGEUP,SC_NUMLOCK,SC_KEYPADSLASH,SC_KEYPADASTERISK,SC_KEYPADMINUS,SC_TAB,SC_Q,	// 20-27
	SC_W,SC_E,SC_R,SC_T,SC_Y,SC_U,SC_I,SC_O,	// 28-2F
	SC_P,SC_LEFTBRACKET,SC_RIGHTBRACKET,SC_BACKSLASH,SC_DELETE,SC_END,SC_PAGEDOWN,SC_KEYPAD7,	// 30-37
	SC_KEYPAD8,SC_KEYPAD9,SC_KEYPADPLUS,SC_CAPSLOCK,SC_A,SC_S,SC_D,SC_F,	// 38-3F
	SC_G,SC_H,SC_J,SC_K,SC_L,SC_COLON,SC_QUOTE,SC_RETURN,	// 40-47
	SC_KEYPAD4,SC_KEYPAD5,SC_KEYPAD6,SC_LEFTSHIFT,SC_Z,SC_X,SC_C,SC_V,	// 48-4F
	SC_B,SC_N,SC_M,SC_COMMA,SC_PERIOD,SC_SLASH,SC_RIGHTSHIFT,SC_UPARROW,	// 50-57
	SC_KEYPAD1,SC_KEYPAD2,SC_KEYPAD3,SC_KEYPADENTER,SC_LEFTCONTROL,SC_LEFTALT,SC_SPACE,SC_RIGHTALT,	// 58-5F
	SC_RIGHTCONTROL,SC_LEFTARROW,SC_DOWNARROW,SC_RIGHTARROW,SC_KEYPAD0,SC_KEYPADPERIOD,0x7f,0x7f,	// 60-67
	0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,	// 68-6F
	0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,	// 70-77
	0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f		// 78-7F
};

static Word BURGERCALL RemapScanCode(Word ScanCode)
{
	if (ScanCode<128) {		 			/* Failsafe */
		return NewCodes[ScanCode];		/* Return the code */
	}
	return 127;							/* Bad code */
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

	Wait for a keyboard event, if the event
	is an extended key then translate to
	Burgerlib extended ASCII

**********************************/

Word BURGERCALL KeyboardGetch(void)
{
	Word NewKey;
	while (BurgerKeyStart==BurgerKeyEnd) {	/* No keys? */
		sleep(10);							/* Yield CPU time */
	}
	NewKey = BurgerKeyBuffer[BurgerKeyStart];	/* Get the Key */
	BurgerKeyStart = (BurgerKeyStart+1)&(KEYBUFFSIZE-1);	/* Accept the key */
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
	KeyboardCallPollingProcs();			/* Null event handler */
	Temp = BurgerKeyStart;		/* Get the starting index */
	if (Temp!=BurgerKeyEnd) {	/* Anything in the buffer? */
		return BurgerKeyBuffer[Temp];		/* Return the key code hit */
	}
	return 0;		/* No event */
}

/**********************************

	Set for keyboard interception

**********************************/

void BURGERCALL InterceptKey(void)
{
	BurgerKeyEnd = 0;
	BurgerKeyStart = 0;
	FastMemSet((void *)KeyArray,0,sizeof(KeyArray));
}


/**********************************

	Here is the BeOS monitor thread routines

**********************************/

class TWin: public BWindow
{
public:
	TWin(BRect frame);
	virtual bool QuitRequested();
};

class TApp : public BApplication
{
public:
	TApp();
	BWindow *MyWin;
};

class TView: public BView
{
public:
	TView(BRect frame);
	virtual void MessageReceived(BMessage *msg);
	virtual void KeyDown(const char *bytes,int32 numBytes);
	virtual void KeyUp(const char *bytes,int32 numBytes);
	void ParseScanCodes(void);
	int32 ViewScanCode;
	Word ViewKeyCode;
};

TView::TView(BRect frame) : BView(frame,"Test View",B_FOLLOW_ALL,0)
{
}

/**********************************

	This is a convience routine to get
	the ASCII code and the scan code of a
	keystroke

**********************************/

void TView::ParseScanCodes(void)
{
	BMessage *msg;

	ViewScanCode = 0;					/* Assume a bogus scan code */
	msg = Window()->CurrentMessage();	/* Get the last message processed */
	if (msg) {
		if (msg->FindInt32("key",&ViewScanCode)!=B_OK) {	/* Get the "key" parm (Scancode) */
			ViewScanCode = 0;			/* Failsafe */
		}
	}

	switch (ViewKeyCode) {				/* From the ASCII code, convert the extra */
										/* codes to Burgerlib ASCII */
	case B_ENTER:
		ViewKeyCode = ASCII_ENTER;
		break;
	case B_LEFT_ARROW:
		ViewKeyCode = ASCII_LEFTARROW;
		break;
	case B_RIGHT_ARROW:
		ViewKeyCode = ASCII_RIGHTARROW;
		break;
	case B_UP_ARROW:
		ViewKeyCode = ASCII_UPARROW;
		break;
	case B_DOWN_ARROW:
		ViewKeyCode = ASCII_DOWNARROW;
		break;
	case B_INSERT:
		ViewKeyCode = ASCII_INSERT;
		break;
	case B_DELETE:
		ViewKeyCode = ASCII_DELETE;
		break;
	case B_HOME:
		ViewKeyCode = ASCII_HOME;
		break;
	case B_END:
		ViewKeyCode = ASCII_END;
		break;
	case B_PAGE_UP:
		ViewKeyCode = ASCII_PAGEUP;
		break;
	case B_PAGE_DOWN:
		ViewKeyCode = ASCII_PAGEDOWN;
		break;

	case B_FUNCTION_KEY:						/* Function keys are a special case in BeOS */
		switch (ViewScanCode) {
		case B_F1_KEY:
			ViewKeyCode = ASCII_F1;
			break;
		case B_F2_KEY:
			ViewKeyCode = ASCII_F2;
			break;
		case B_F3_KEY:
			ViewKeyCode = ASCII_F3;
			break;
		case B_F4_KEY:
			ViewKeyCode = ASCII_F4;
			break;
		case B_F5_KEY:
			ViewKeyCode = ASCII_F5;
			break;
		case B_F6_KEY:
			ViewKeyCode = ASCII_F6;
			break;
		case B_F7_KEY:
			ViewKeyCode = ASCII_F7;
			break;
		case B_F8_KEY:
			ViewKeyCode = ASCII_F8;
			break;
		case B_F9_KEY:
			ViewKeyCode = ASCII_F9;
			break;
		case B_F10_KEY:
			ViewKeyCode = ASCII_F10;
			break;
		case B_F11_KEY:
			ViewKeyCode = ASCII_F11;
			break;
		case B_F12_KEY:
			ViewKeyCode = ASCII_F12;
			break;
		case B_SCROLL_KEY:
			ViewKeyCode = ASCII_SCROLLLOCK;
			break;
		case B_PRINT_KEY:
			ViewKeyCode = ASCII_PRINTSCREEN;
			break;
		case B_PAUSE_KEY:
			ViewKeyCode = ASCII_PAUSE;
			break;
		}
		break;
	}
}

/**********************************

	Process a KeyDown event

**********************************/

void TView::KeyDown(const char *bytes,int32 numbytes)
{
	if (numbytes==1) {		/* I only process single byte commands */
		ViewKeyCode = ((Word8 *)bytes)[0];	/* Get the ASCII code */
		ParseScanCodes();					/* Get the scan code if any */
		BurgerPostKey(ViewKeyCode);			/* Post the ASCII code */
		KeyArray[RemapScanCode(ViewScanCode)]|=3;	/* Handle the key array event */
	}
}

/**********************************

	Process a KeyUp event

**********************************/

void TView::KeyUp(const char *bytes,int32 numbytes)
{
	if (numbytes==1) {		/* I only process single byte commands */
		ViewKeyCode = ((Word8 *)bytes)[0];		/* Get the ASCII code */
		ParseScanCodes();
		KeyArray[RemapScanCode(ViewScanCode)]&=~1;	/* Clear the key array event */
	}
}

/**********************************

	Process unmapped keys and keyboard modifiers

**********************************/

void TView::MessageReceived(BMessage *msg)
{
	int32 Temp;
	int32 Temp2;

	switch (msg->what) {
//	case B_MODIFIERS_CHANGED:
//		if (msg->FindInt32("modifiers",&Temp)==B_OK) {
//			if (Temp&B_SHIFT_KEY) {
//			}
//		}
//		break;
	case B_UNMAPPED_KEY_DOWN:
	case B_UNMAPPED_KEY_UP:
		if (msg->FindInt32("key",&Temp)==B_OK) {		/* Get the scan code */

			switch(Temp) {			/* Special cases for mapped burger keys */
			case 0x22:
				Temp2 = ASCII_SCROLLLOCK;
				break;
			case 0x49:				/* Keypad center */
				Temp2 = '5';		/* Keypad center is not mapped */
				break;
			case 0x5D:
				Temp2 = ASCII_PAUSE;
				break;
			default:
				Temp2 = Temp;		/* Use the scan code as is */
			}
			BurgerPostKey(Temp2);	/* Post the key event */
			Temp = RemapScanCode(Temp);
			if (msg->what==B_UNMAPPED_KEY_DOWN) {
				KeyArray[Temp]|=3;	/* Press the key */
			} else {
				KeyArray[Temp]&=~1;	/* Release the key */
			}
		}
		break;
	}
}

TWin::TWin(BRect frame) : BWindow(frame, "Test App", B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE)
{
	TView *mView;
	AddChild(mView = new TView(frame));
	mView->MakeFocus();
}

TApp :: TApp() : BApplication("application/x-vnd.Burger.TestApp")
{
	BRect r(50,50,640+50,480+50);
	MyWin = new TWin(r);
	MyWin->Show();
}

/**********************************

	Event to shut down an app

**********************************/

bool TWin::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


/**********************************

	This is the code that is called by spawn_thread()
	to start execution of the real thread code.

	If the real thread shuts down, I need to post a quit message

**********************************/

typedef struct Proc_t {
	int argc;
	char **argv;
	int (*Proc)(int,char **);
} Proc_t;

static int32 BURGERCALL BeCallMain(void *ProcPtr)
{
	exit(((Proc_t *)ProcPtr)->Proc(0,0));
	return 0;
}

/**********************************

	To properly run an app in BeOS, you need to have a
	"Monitor" thread that actually spawns the game.
	This thread will then receive state change events
	to change the display mode etc as well as get the input
	data from the input focus.

	This is backwards from most code where the main
	thread spawns monitor threads for input. Here the
	input threads are the main threads and the game is the slave.

	This code fragment will allow a Burgerlib app to quickly
	fake a master thread into a slave thread.

	Sigh.

**********************************/

int BURGERCALL BeOSSpawnMain(int (*MainCode)(int,char **),int argc, char **argv)
{
	thread_id mythread;		/* Spawned thread for the main code */
	Proc_t MyProc;			/* Parm list to pass through */

	MyProc.argc = argc;		/* Save the parms into a message */
	MyProc.argv = argv;
	MyProc.Proc = MainCode;

	be_app = new TApp();	/* Create my main thread */

	/* Here is where I start the code for the game thread */

	mythread = spawn_thread(BeCallMain,"BurgerApp",B_DISPLAY_PRIORITY,&MyProc);
	resume_thread(mythread);		/* Begin execution here */

	be_app->Run();					/* Here is where BeOS begins... */
	delete be_app;					/* I'm done!!! */
	be_app = 0;
	return 0;						/* Return any result codes */
}


#endif
