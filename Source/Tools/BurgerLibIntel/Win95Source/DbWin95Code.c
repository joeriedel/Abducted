#include "ClStdLib.h"

/********************************

	Win 95 version

********************************/

#if defined(__WIN32__)
#include "W9Win95.h"
#include "OcOSCursor.h"
#include "FmFile.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winbase.h>
#include <stdlib.h>

static Word Tested;
static int (__stdcall *DebugTest)();

void BURGERCALL Halt(void)
{
	if (!Tested) {			/* Did I get the reference */
		HINSTANCE KernelRef;
		KernelRef = LoadLibrary("kernel32.dll");	/* Load in the kernel reference */
		if (KernelRef) {			/* Valid reference? */
			DebugTest = GetProcAddress(KernelRef,"IsDebuggerPresent");	/* Get the routine */
		}
		Tested = TRUE;		/* I am cool... */
	}
	if (DebugTest) {		/* Is the pointer valid? */
		if (DebugTest()) {	/* Is the debugger present? */
			_asm int 3		/* Trap! */
		}
	}
}

/**********************************

	Win 95 version

**********************************/

void BURGERCALL OkAlertMessage(const char *Title,const char *Message)
{
	Word visible;

	// Make sure that the OS cursor is visible otherwise the user will
	// wonder what's up when he can't see the cursor to click the button
	visible = OSCursorIsVisible();
	OSCursorShow();
	SetForegroundWindow(GetDesktopWindow());
	MessageBox((HWND)Win95MainWindow,Message,Title,MB_OK);

	// Restore state
	if( !visible ) {
		OSCursorHide();
	}
}

/**********************************

	Win 95 version

**********************************/

Word BURGERCALL OkCancelAlertMessage(const char *Title,const char *Message)
{
	Word visible;
	Word result;

	// Make sure that the OS cursor is visible otherwise the user will
	// wonder what's up when he can't see the cursor to click the button
	visible = OSCursorIsVisible();
	OSCursorShow();
	SetForegroundWindow(GetDesktopWindow());
	result = MessageBox((HWND)Win95MainWindow,Message,Title,MB_ICONWARNING|MB_OKCANCEL) == IDOK;

	// Restore state
	if( !visible ) {
		OSCursorHide();
	}
	return result;
}

/**********************************

	Print a string to a file

**********************************/

static CRITICAL_SECTION Semi;
static Word Inited;
static void ANSICALL Cleanup(void)
{
	DeleteCriticalSection(&Semi);		/* Release the object */
	Inited=TRUE+1;		/* Force critical section to shut down */
}

void BURGERCALL DebugXString(const char *String)
{
	FILE *fp;
	Word i;

	if (!Inited) {
		InitializeCriticalSection(&Semi);	/* Init the section */
		Inited = TRUE;						/* Mark as such */
		atexit(Cleanup);					/* Clean up after myself */
	}
	if (Inited==TRUE) {						/* I was up fine? */
		EnterCriticalSection(&Semi);		/* Allow multiple threads to call me! */
	}

	i = strlen(String);
	if (i) {
		fp = OpenAFile("LogFile.Txt","a");
		if (fp) {
			fwrite(String,1,i,fp);		/* Send the string to the log file */
			fclose(fp);
		}
	}
	OutputDebugString(String);		/* Send to the developer studio console window */
	if (Inited==TRUE) {				/* Up fine? */
		LeaveCriticalSection(&Semi);
	}
}

#endif