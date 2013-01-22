#include "ClStdLib.h"
#include "PfPrefs.h"
#include "FmFile.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/**********************************

	Set DEBUGTRACE_ flags to true to enable
	debugging spew

**********************************/

Word DebugTraceFlag;

/**********************************

	If TRUE then all NonFatal errors are fatal ones

**********************************/

Word BombFlag = FALSE;

/**********************************

	If TRUE then I am in the middle of a shutdown

**********************************/

Word IAmExiting;		/* TRUE if in a shutdown state */

/**********************************

	Last error that occured

**********************************/

char ErrorMsg[512];	/* System error string */


/********************************

	A FATAL error has occured, print message, then die!

********************************/

void ANSICALL Fatal(const char *FatalMsg,...)
{
	va_list Args;
	if (FatalMsg && FatalMsg!=ErrorMsg) {			/* Message to print? */
		va_start(Args,FatalMsg);
		vsprintf(ErrorMsg,FatalMsg,Args);		/* Create the message */
		va_end(Args);
	}

	if (!IAmExiting) {			/* Already in exit()? */
		IAmExiting = TRUE;		/* Don't call again! */
		if (FatalMsg) {			/* Is there a message? */
			DebugXString(ErrorMsg);		/* Print it */
		}
		exit(55);			/* Exit to OS */
	}
}

/********************************

	A Non-FATAL error has occured, save message, but if the
	BombFlag is set, then die anyways.

********************************/

void ANSICALL NonFatal(const char *Message,...)
{
	va_list Args;
	if (Message) {		/* No message, no error! */
		va_start(Args,Message);		/* Start parm passing */
		vsprintf(ErrorMsg,Message,Args);		/* Create the message */
		va_end(Args);			/* End parm passing */
		if (BombFlag) {			/* Bomb on ANY Error? */
			Fatal(ErrorMsg);		/* Force a fatal error */
		}
	}
}

/********************************

	Set the state of the bomb flag

********************************/

Word BURGERCALL SetErrBombFlag(Word Flag)
{
	Word Old;
	Old = BombFlag;
	BombFlag = Flag;
	return Old;
}

/********************************

	Enter the debugger if present
	You should NEVER make this call in a release build

********************************/

#if !defined(__MSDOS__) && !defined(__WIN32__) && !defined(__MAC__)
void BURGERCALL Halt(void)
{
}

#endif

/**********************************

	Save bogus memory into a debug file
	Do NOT quit on error!!!

**********************************/

static Word JunkCount = 1;
static char SaveName[]= "8:Junk00000";

void BURGERCALL SaveJunk(const void *AckPtr,Word32 Length)
{
	Word i;

	i = SetErrBombFlag(FALSE);		/* Prevent quit on error */
	LongWordToAscii2(JunkCount,&SaveName[6],5);
	SaveAFile(SaveName,AckPtr,Length);	/* Save the file, ignore errors */
	++JunkCount;
	SetErrBombFlag(i);		/* Exit */
}

/**********************************

	Print a hex character to standard out

**********************************/

void BURGERCALL PrintHexDigit(Word Val)
{
	char Foo[2];
	Foo[0] = NibbleToAscii[Val&0xF];
	Foo[1] = 0;
	printf(Foo);		/* Print the char */
}

/**********************************

	Print a hex byte to standard out

**********************************/

void BURGERCALL PrintHexByte(Word Val)
{
	PrintHexDigit(Val>>4);
	PrintHexDigit(Val);
}

/**********************************

	Print a hex short to standard out

**********************************/

void BURGERCALL PrintHexShort(Word Val)
{
	PrintHexByte(Val>>8);
	PrintHexByte(Val);
}

/**********************************

	Print a Word32 to the debug port

**********************************/

void BURGERCALL DebugXshort(short i)
{
	char TempBuffer[16];
	longToAscii(i,TempBuffer);
	DebugXString(TempBuffer);
}

/**********************************

	Print a hex longword to standard out

**********************************/

void BURGERCALL PrintHexLongWord(Word32 Val)
{
	PrintHexShort(Val>>16);
	PrintHexShort(Val);
}

/**********************************

	Print a Word32 to the debug port

**********************************/

void BURGERCALL DebugXShort(Word16 i)
{
	char TempBuffer[16];
	LongWordToAscii(i,TempBuffer);
	DebugXString(TempBuffer);
}


/**********************************

	Print a Word32 to the debug port

**********************************/

void BURGERCALL DebugXlong(long i)
{
	char TempBuffer[16];
	longToAscii(i,TempBuffer);
	DebugXString(TempBuffer);
}

/**********************************

	Print a Word32 to the debug port

**********************************/

void BURGERCALL DebugXLongWord(Word32 i)
{
	char TempBuffer[16];
	LongWordToAscii(i,TempBuffer);
	DebugXString(TempBuffer);
}


/**********************************

	Print a floating point number

**********************************/

void BURGERCALL DebugXDouble(double i)
{
	char TempBuffer[32];
	sprintf(TempBuffer,"%f",i);
	DebugXString(TempBuffer);
}

/**********************************

	Print a system pointer

**********************************/

void BURGERCALL DebugXPointer(const void *p)
{
	char TempBuffer[32];
	sprintf(TempBuffer,"%p",p);
	DebugXString(TempBuffer);
}

/********************************

	A Non-FATAL error has occured, save message, but if the
	BombFlag is set, then die anyways.

********************************/

void ANSICALL DebugXMessage(const char *String,...)
{
	va_list Args;
	char TempBuffer[30000];		/* Had better not be too big! */
	if (String) {		/* No message, no error! */
		va_start(Args,String);		/* Start parm passing */
		vsprintf(TempBuffer,String,Args);		/* Create the message */
		va_end(Args);			/* End parm passing */
		DebugXString(TempBuffer);	/* Send off the message */
	}
}

/**********************************

	Print a string to a file

**********************************/

#if !defined(__WIN32__)

void BURGERCALL DebugXString(const char *String)
{
	FILE *fp;
	Word i;

	i = strlen(String);
	if (i) {
		fp = OpenAFile("9:LogFile.Txt","a");
		if (fp) {
			fwrite(String,1,i,fp);		/* Send the string to the log file AS IS */
			fclose(fp);
		}
	}
}

#endif

/**********************************

	Display a dialog to alert the user
	of a possible error condition or message.

**********************************/

#if !defined(__MAC__) && !defined(__WIN32__) && !defined(__BEOS__)

void BURGERCALL OkAlertMessage(const char *Title,const char *Message)
{
	DebugXString("Alert message : ");
	DebugXString(Title);
	DebugXString(", ");
	DebugXString(Message);
	DebugXString("\n");
}

#endif

/**********************************

	Display a dialog to alert the user
	of a possible error condition or message.

**********************************/

#if !defined(__MAC__) && !defined(__WIN32__) && !defined(__BEOS__)

Word BURGERCALL OkCancelAlertMessage(const char *Title,const char *Message)
{
	DebugXString("Cancel alert message : ");
	DebugXString(Title);
	DebugXString(", ");
	DebugXString(Message);
	DebugXString("\n");
	return FALSE;		/* Always cancel! */
}

#endif


