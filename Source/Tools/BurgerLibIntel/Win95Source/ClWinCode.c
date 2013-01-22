#include "ClStdLib.h"

/**********************************

	Win95 version

**********************************/

#if defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include "PfPrefs.h"
#include "FmFile.h"
#include "MmMemory.h"
#include "StString.h"

/**********************************

	This routine will scan the operating system for files dropped into this
	applications icon. If any are present, each and every file will
	be passed to a routine to handle the event.
	If no file is present, do nothing.
	Once all the files are gone, return.

	The procedure returns FALSE for no error and the scan will continue,
	if an error is returned, the scan will abort.

**********************************/

/**********************************

	I have to go through serious pain since Win95
	gives me the SHORT version of the filename.
	I need to manually expand it to the long form.

**********************************/

Word BURGERCALL SystemProcessFilenames(SystemProcessCallBackProc Proc)
{
	char *InputPtr;
	Word i;
	Word FirstPass;
	Word Result;
	char TempBuffer[260+256];

	Result = TRUE;
	FirstPass = TRUE;
	InputPtr = GetCommandLine();
	if (InputPtr) {
		do {
			InputPtr = ParseBeyondWhiteSpace(InputPtr);
			if (InputPtr[0]==0) {
				break;
			}
			if (InputPtr[0]=='"') {
				InputPtr = GetAParsedString(InputPtr,TempBuffer,sizeof(TempBuffer));
			} else {
				Word c;
				i = 0;
				do {
					TempBuffer[i] = InputPtr[0];
					++InputPtr;
					++i;
					c = InputPtr[0];
				} while (c && c!=32 && c!=9);
				TempBuffer[i] = 0;
			}
			if (!FirstPass) {
				WIN32_FIND_DATA Name;
				char *New;
				New = ConvertNativePathToPath(TempBuffer);
				if (New) {
					SetAPrefix(8,New);
					DeallocAPointer(New);
					PopAPrefix(8);		/* Prefix 8 has the work directory */
					FindFirstFile(TempBuffer,&Name);
					FindClose(&Name);
					Proc(Name.cFileName);
					Result = FALSE;
				}
			} else {
				/* This is a silly hack */
				/* It seems that under win95, I can't determine */
				/* whether I was invoked by the command line or */
				/* by dragging files to my icon unless the first */
				/* parm has the ".exe" extension in it */

				char *Str1;
				Str1 = StrGetFileExtension(TempBuffer);
				if (!Str1) {		/* No extension? */
					break;			/* Command line! */
				}
				if (stricmp(Str1,"exe")) {	/* Not .exe? */
					break;					/* Command line */
				}
				FirstPass = FALSE;			/* Assume this is a window launch */
			}
			InputPtr = ParseBeyondWhiteSpace(InputPtr);
		} while (InputPtr[0]);
	}
	return Result;
}

/**********************************

	By invoking DEEP magic, I will divine the version
	of QuickTimeX that is present.

	Returned values.
	0	    No QuickTime installed
	0x211   QuickTime 2.1.1 installed
	0x212	QuickTime 2.1.2 installed

**********************************/

Word BURGERCALL GetQuickTimeVersion(void)
{
	Word Ver;		/* Version to return */
	char *Data;		/* Pointer to version info */
	char PathName[MAX_PATH];
	Word32 Length;
	DWORD ZeroLong;
	Word ZeroWord;
	char *VerData;	/* Running ascii pointer */
	char *QueryPtr;

	Ver = 0;		/* I assume version 0! */
	ZeroLong = 0;
	ZeroWord = 0;
	if (GetSystemDirectory(PathName,MAX_PATH)) {	/* Get system directory */
		strcat(PathName,"\\QTIM32.DLL");			/* Get the Quicktime DLL */
		Length = GetFileVersionInfoSize(PathName,&ZeroLong);
		if (Length) {		/* Any data */
			QueryPtr = "\\StringFileInfo\\040904E4\\ProductVersion";
			goto GotFile;			/* It's Quicktime 2.0 - 3.0 */
		}
		ZeroLong = 0;
		GetSystemDirectory(PathName,MAX_PATH);	/* Get system directory */
		strcat(PathName,"\\QuickTime.qts");		/* Is this 4.0? */
		Length = GetFileVersionInfoSize(PathName,&ZeroLong);
		if (Length) {
			QueryPtr = "\\StringFileInfo\\040904B0\\FileVersion";
GotFile:
			Data = (char *)AllocAPointer(Length);	/* Get the data */
			if (Data) {
				if (GetFileVersionInfo(PathName,0,Length,Data)) {
					if (VerQueryValue(Data,QueryPtr,(void **)&VerData,&ZeroWord)) {
						Ver = AsciiToLongWord2(VerData,&VerData)<<8;
						if (VerData[0]=='.') {
							Length = AsciiToLongWord2(VerData+1,&VerData);
							if (Length>=16) {
								Length = 15;
							}
							Ver |= Length<<4;
							if (VerData[0]=='.') {
								Length = AsciiToLongWord(VerData+1);
								if (Length>=16) {
									Length = 15;
								}
								Ver |= Length;
							}
						}
					}
				}
				DeallocAPointer(Data);	/* Release the info pointer */
			}
		}
	}
	return Ver;		/* Return the QuickTime version */
}

/**********************************

	Load and launch a web page from an address string

**********************************/

Word BURGERCALL LaunchURL(const char *URLPtr)
{
	if (ShellExecute(GetDesktopWindow(),"open",URLPtr,0,0,SW_SHOW)==0) {
		return TRUE;	/* I died */
	}
	return FALSE;		/* I launched */
}

/**********************************

	Attempt to load in a shared library or DLL using
	the standard paths. Return NULL if it fails
	Please note, in Win95, passing in just the
	DLL name without a full path will allow
	the SYSTEM DLL's to be loaded, so, to
	see if you want to load a system DLL, I check
	the pathname if it has a ':' in it.

**********************************/

LibRef_t * BURGERCALL LibRefInit(const char *LibName)
{
	char Pathname[FULLPATHSIZE];
	if (strchr(LibName,':')) {		/* Burgerlib path? */
		ExpandAPathToBufferNative(Pathname,LibName);		/* Convert to Windows */
		LibName = Pathname;			/* Use this instead */
	}
	return (LibRef_t *)LoadLibrary(LibName);		/* Load the library from Windows */
}

/**********************************

	Release a shared library

**********************************/

void BURGERCALL LibRefDelete(LibRef_t *LibRef)
{
	if (LibRef) {
		FreeLibrary((HINSTANCE)LibRef);		/* Release the windows lib */
	}
}

/**********************************

	Return a function pointer to a procedure or data
	contained within a shared library

**********************************/

void *BURGERCALL LibRefGetProc(LibRef_t *LibRef,const char *ProcName)
{
	if (LibRef) {
		return GetProcAddress((HINSTANCE)LibRef,ProcName);
	}
	return 0;
}

#endif
