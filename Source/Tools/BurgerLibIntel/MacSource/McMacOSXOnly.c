#include "McMac.h"

/**********************************

	These are all MacOS specific routines.
	Since the CodeWarrior linker doesn't suck and only
	links in what you actually use, I can
	put all of my routines in a single source file instead
	of putting them in seperate files.
	
**********************************/

#if defined(__MACOSX__)
#include <stdbool.h>
#include <sysctl.h>
#include <stdlib.h>
#include <Foundation/NSAutoreleasePool.h>
#include <Carbon/Carbon.h>
#include <DrawSprocket/DrawSprocket.h>

typedef struct Version_t {
	Word DrawSprocketVersion;	/* DrawSprocket version */
	Word OSVersion;				/* MacOS Version */
	Word DrawSprocketVersFlag;	/* TRUE if DrawSprocket version was obtained */
} Version_t;

static Version_t VersionLocals;		/* Internal variables */

MacEventInterceptProcPtr MacEventIntercept;	/* Intercept for DoMacEvent() */

/**********************************

	Return TRUE if Altivec is present
	
**********************************/

Word BURGERCALL MacOSIsAltivecPresent(void)
{
	int mib[2];
	unsigned long gHasAltivec;
	unsigned long len;

	/* Create a machine information block */
	
	mib[0] = CTL_HW;
	mib[1] = HW_VECTORUNIT;
	len = sizeof(gHasAltivec);
	sysctl(mib,2,&gHasAltivec, &len,0,0);		/* Ask for Altivec */
	if (gHasAltivec) {							/* Found it */
		return TRUE;
	}
	return FALSE;
}

/**********************************

	Return the MacOS version present
	Note : I optimized this to only call Gestalt once
	
**********************************/

Word BURGERCALL MacOSGetOSVersion(void)
{
	long gestaltAnswer;
	Word Version;
	Version_t *VersionPtr;

	VersionPtr = &VersionLocals;			/* Get the locals */
	Version = VersionPtr->OSVersion;		/* Get the version */
	if (!Version) {							/* Not called? */
		if (Gestalt(gestaltSystemVersion,&gestaltAnswer)) {		/* Get the version */
			gestaltAnswer = 0;				/* Should NEVER execute */
		}
		Version = gestaltAnswer&0xFFFF;		/* Get the version of the OS */
		VersionPtr->OSVersion = Version;	/* Save in the global */
	}
	return Version;							/* Return the version found */
}

/**********************************

	Return the Draw Sprocket version
	
**********************************/

Word BURGERCALL MacOSGetDrawSprocketVersion(void)
{
	Version_t *VersionPtr;

	VersionPtr = &VersionLocals;
	if (!VersionPtr->DrawSprocketVersFlag) {
		NumVersion nver;
		Word Result2;
		
		VersionPtr->DrawSprocketVersFlag = TRUE;
		
		/* Let's do it the easy way! */
		
		nver = DSpGetVersion();						/* Get the version */
		Result2 = (nver.majorRev<<8)&0xFF00;		/* Merge the version number */
		Result2 |= (nver.minorAndBugRev)&0xFF;
		VersionPtr->DrawSprocketVersion = Result2;	/* Store it */
	}
	return VersionPtr->DrawSprocketVersion;		/* Not here */
}

/**********************************

	Initialize the basic MacOS tools
	
**********************************/

static NSAutoreleasePool *MyPool;		/* Autorelease class */

static void ANSICALL MacOSReleaseTools(void)
{
	if (MyPool) {			/* Was this started? */
		[MyPool release];	/* Release the class */
		MyPool = 0;
	}
}

void BURGERCALL MacOSInitTools(void)
{
	if (!MyPool) {			/* Not initialized? */
		MyPool = [[NSAutoreleasePool alloc] init];	/* Init me */
		atexit(&MacOSReleaseTools);			/* Allow shutdown */
	}
}

#endif

#if defined(__MACOSX__) || (defined(__MAC__) && TARGET_API_MAC_CARBON)

#include "TkTick.h"

#if defined(__MACOSX__)
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFDate.h>
#include <CarbonCore/Folders.h>
#else
#include <CFBundle.h>
#include <CFDate.h>
#include <Folders.h>
#endif

/**********************************

	Load a MacOSX framework library bundle

**********************************/

Word BURGERCALL MacOSXFrameworkInit(MacOSXFramework_t *Input,const char *FramwWorkName)
{
	CFURLRef baseURL;			/* Base folder URL */
	CFURLRef bundleURL;			/* URL to the framework itself */
	CFBundleRef bundleRef;		/* Library reference */
	FSRef frameworksFolderRef;	/* Folder for the framework */
	Word Result;				/* Return value */
	
	/* Does the framework folder exist? (Only on a MacOS X install) */
	
	Result = TRUE;
	Input->LibBundle = 0;
	if (!FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType,TRUE,&frameworksFolderRef)) {

		/* Convert the FSRef into a URL */
		
		baseURL = CFURLCreateFromFSRef(0, &frameworksFolderRef);
		if (baseURL) {
			CFStringRef StringRef;

			/* Convert to a string ref */
			
			StringRef = CFStringCreateWithCString(0,FramwWorkName,kCFStringEncodingMacRoman);
			bundleURL = CFURLCreateCopyAppendingPathComponent(0,baseURL,StringRef,FALSE);
			CFRelease(StringRef);					/* Dispose of the string ref */

			if (bundleURL) {
				bundleRef = CFBundleCreate(0,bundleURL);
				if (bundleRef) {
					if (!CFBundleLoadExecutable(bundleRef)) {
						CFRelease(bundleRef);
					} else {
						Input->LibBundle = bundleRef;
						Result = FALSE;
					}
				}
				CFRelease(bundleURL);
			}
			CFRelease(baseURL);
		}
	}
	return Result;
}

/**********************************

	Get a proc pointer from a framework library

**********************************/

void * BURGERCALL MacOSXFrameworkGetProc(MacOSXFramework_t *Input,const char *ProcNam)
{
	void *Result;
	Result = 0;
	if (Input) {			/* Is the lib valid? */
		if (Input->LibBundle) {
			CFStringRef StringRef;

			/* Convert to a string ref */
			
			StringRef = CFStringCreateWithCString(0,ProcNam,kCFStringEncodingMacRoman);
			Result = CFBundleGetFunctionPointerForName(Input->LibBundle,StringRef);
			CFRelease(StringRef);		/* Dispose of the string ref */
		}
	}
	return Result;		/* Return 0 or the pointer */
}

/**********************************

	Release a MacOSX library

**********************************/

void BURGERCALL MacOSXFrameworkDestroy(MacOSXFramework_t *Input)
{
	if (Input) {			/* Is it valid? */
		if (Input->LibBundle) {
			CFRelease(Input->LibBundle);		/* Release the lib */
			Input->LibBundle = 0;				/* Invalidate the lib */
		}
	}
}

/**********************************

	Convert a UTCDateTime to a TimeDate_t

**********************************/

void BURGERCALL MacOSXTimeDateFromUTCTime(TimeDate_t *Output,const UTCDateTime *Input)
{
	LocalDateTime TempTime;
	CFGregorianDate TimeRec;
	double WorkTime;

	/* Convert to my time zone */
	
	ConvertUTCToLocalDateTime(Input,&TempTime);
	
	/* Merge the 48 bit time into a double */
	
	WorkTime = (double) ((double) TempTime.highSeconds * (65536.0*65536.0))
		+ (double)TempTime.lowSeconds;
	
	WorkTime-=kCFAbsoluteTimeIntervalSince1904;	/* Convert to CFDate format */
	
	TimeRec = CFAbsoluteTimeGetGregorianDate(WorkTime,0);
	Output->Year = (Word)TimeRec.year;
	Output->Month = (Word8)TimeRec.month;
	Output->Day = (Word8)TimeRec.day;
	Output->Hour = (Word8)TimeRec.hour;
	Output->Minute = (Word8)TimeRec.minute;
	Output->Second = (Word8)TimeRec.second;
	Output->Milliseconds = (Word16)(((Word)TempTime.fraction * 1000)>>16);
	Output->DayOfWeek = CFAbsoluteTimeGetDayOfWeek(WorkTime,0);
}

#endif