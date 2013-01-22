#include "ClStdLib.h"

#if defined(__MAC__)
#include "FmFile.h"
#include "McMac.h"
#include "MmMemory.h"
#include "InInput.h"
#include <MacTypes.h>
#include <AppleEvents.h>
#include <Files.h>
#include <Gestalt.h>
#include <CodeFragments.h>
#include <InternetConfig.h>

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

	MacOS version, uses AppleEvents
	
**********************************/

/**********************************

	This is called from the Apple event handler
	The refcon is the pointer to the procedure to call
	
**********************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Foo_t {
	Word (*Proc)(const char *);		/* Function to call */
	Word Result;					/* Return value */
} Foo_t;

#ifdef __cplusplus
}
#endif

static pascal OSErr HandleOdoc(const AEDescList *aevt, AEDescList *reply,long refCon) 
{
	AEDesc FileListDescription;
	DescType MyFileType;
	long DataSize;
	AEKeyword MyKeyword;
	FSSpec TheFileSpec;
	Word i;
	Foo_t *FooPtr;
						
	/* First, I see if any files are present */
	
	FileListDescription.descriptorType = typeWildCard;		/* Any file */
	FileListDescription.dataHandle = 0;				/* No handle (Yet) */
	
	if (!AEGetKeyDesc(aevt,keyDirectObject,typeAEList,&FileListDescription)) {
		
		/* Now load each and every file */

		i = 1;
		FooPtr = (Foo_t *)refCon;
		while (!AEGetNthPtr(&FileListDescription,i,typeFSS,&MyKeyword,&MyFileType,(Ptr)&TheFileSpec,sizeof(FSSpec),&DataSize)) {
			char CurrentName[256];
			char *DirName;
			if (TheFileSpec.name[0]) {		/* Must have some name! */
				PStr2CStr(CurrentName,(char *)TheFileSpec.name);	/* Convert to "C" */
				TheFileSpec.name[0] = 0;	/* Zap the filename */
				DirName = GetFullPathFromMacFSSpec(&TheFileSpec);	/* Get the directory */
				if (DirName) {
					SetAPrefix(8,DirName);		/* Set my directory */
					DeallocAPointer(DirName);
					FooPtr->Result = FALSE;		/* Hit me! */
					if (FooPtr->Proc(CurrentName)) {	/* Process the file */
						break;
					}
				}
			}
			++i;		/* Next */
		}
	}
	AEDisposeDesc(&FileListDescription);		/* Release the file list */
	return noErr;
}

/**********************************

	Install Apple Events to monitor

**********************************/

Word BURGERCALL SystemProcessFilenames(SystemProcessCallBackProc Proc) 
{
	long result;				/* Gestalt temp */
	Word OldFlag;				/* Previous MacSystemTaskFlag */
	AEEventHandlerUPP OpenFileProc;		/* Current proc pointer */
	AEEventHandlerUPP PrevFileProc;		/* Previous proc pointer */
	long PrevRefCon;				/* Previous proc refcon */
	OSErr PrevErr;					/* Error getting the previous event handler */
	Foo_t FooData;					/* Data state */

	FooData.Result = TRUE;					/* Nothing processed */
	FooData.Proc = Proc;

	if (Proc && !Gestalt(gestaltAppleEventsAttr,&result)) {	/* Do I have apple events? */
		OpenFileProc = NewAEEventHandlerUPP(HandleOdoc);	/* Get the file */
		if (OpenFileProc) {
			Word i;
			
			/* Install the event handler */
			PrevErr = AEGetEventHandler(kCoreEventClass,kAEOpenDocuments,&PrevFileProc,&PrevRefCon,FALSE);
			AEInstallEventHandler(kCoreEventClass,kAEOpenDocuments,OpenFileProc,(long)&FooData, FALSE);
			i = 50;
			OldFlag = MacSystemTaskFlag;		/* Save */
			MacSystemTaskFlag = TRUE;			/* I WANT SystemTask() to be called */
			do {
				KeyboardGet();					/* Give some time to MacOS */
			} while (--i);
			MacSystemTaskFlag = OldFlag;	/* Restore the flag */
			AERemoveEventHandler(kCoreEventClass,kAEOpenDocuments,OpenFileProc,FALSE);
			DisposeAEEventHandlerUPP(OpenFileProc);	/* Kill my routine */
			if (!PrevErr) {
				AEInstallEventHandler(kCoreEventClass,kAEOpenDocuments,PrevFileProc,PrevRefCon, FALSE);
			}
		}
	}
	return FooData.Result;
}

/**********************************

	By invoking DEEP magic, I will divine the version
	of QuickTimeX that is present under MacOS

	Returned values.
	0	    No QuickTime installed
	0x211   QuickTime 2.1.1 installed
	0x212	QuickTime 2.1.2 installed

**********************************/

Word BURGERCALL GetQuickTimeVersion(void)
{
	long gestaltAnswer;

	if (!Gestalt(gestaltQuickTimeVersion,&gestaltAnswer)) {
		return (gestaltAnswer >> 16)&0xFFFF;	/* Major version */
	}
	return 0;		/* No quicktime */
}

/**********************************

	Load and launch a web page from an address string

**********************************/

Word BURGERCALL LaunchURL(const char *URLPtr)
{
	OSStatus err;
	ICInstance inst;
	long startSel;
	long endSel;
	
	err = -1;
	if (ICStart) {
		err = ICStart(&inst,'????');           // Use your creator code if you have one!
		if (err == noErr) {
#if !TARGET_API_MAC_CARBON
			err = ICFindConfigFile(inst, 0, nil);
			if (err == noErr)
#endif
			{
				startSel = 0;
				endSel = strlen(URLPtr);
				err = ICLaunchURL(inst, "\p", (char *)URLPtr,endSel, &startSel, &endSel);
			}
			ICStop(inst);
		}
	}
	return (err);
 }

/**********************************

	Attempt to load in a shared library or DLL using
	the standard paths. Return NULL if it fails

**********************************/

LibRef_t * BURGERCALL LibRefInit(const char *LibName) 
{
	Str255 TempName;			/* Copy of the "C" string as a PASCAL string */
	Str255 ErrStr;				/* Returned error code if any */
	Ptr EntryPtr;				/* Pointer to the fragment entry */
	CFragConnectionID ConnID;

	/* This code only works for CFM functions */
	CStr2PStr((char *)TempName,LibName);	
	if (!GetSharedLibrary(TempName,kCompiledCFragArch,kLoadCFrag, &ConnID,&EntryPtr, ErrStr)) {
		return (LibRef_t *)ConnID;
	}
	return 0;
}

/**********************************

	Release a shared library

**********************************/

void BURGERCALL LibRefDelete(LibRef_t *LibRef)
{
	if (LibRef) {
		CloseConnection((CFragConnectionID *)&LibRef);
	}
}

/**********************************

	Return a function pointer to a procedure or data
	contained within a shared library

**********************************/

void * BURGERCALL LibRefGetProc(LibRef_t *LibRef,const char *ProcName) 
{
	Str255 TempName;			/* Copy of the "C" string as a PASCAL string */
	Ptr ProcPtr;				/* Pointer to the function */		

	/* This code only works for CFM functions */
	if (LibRef) {
		CStr2PStr((char *)TempName,ProcName);
		if (!FindSymbol((CFragConnectionID)LibRef, TempName, &ProcPtr,0)) {
			return ProcPtr;
		}
	}
	return 0;
}

#endif