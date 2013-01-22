#include "McMac.h"

/**********************************

	These are all MacOS specific routines.
	Since the CodeWarrior linker doesn't suck and only
	links in what you actually use, I can
	put all of my routines in a single source file instead
	of putting them in seperate files.
	
**********************************/

#if defined(__MAC__)
#include <DrawSprocket.h>		/* Link in the sprockets */
#include <InputSprocket.h>
#include <Gestalt.h>
#include <Movies.h>
#include <Fonts.h>
#include <Sound.h>
#include <Folders.h>
#include <Traps.h>
#include <Devices.h>
#include <Resources.h>
#include <LowMem.h>
#include <Disks.h>
#include <stdlib.h>
#include "FmFile.h"
#include "ClStdLib.h"
#include "MmMemory.h"
#include "InInput.h"
#include "StString.h"

#if TARGET_API_MAC_CARBON
#include <locale.h>
#include <time.h>
#include <fcntl.h>
#include <CGDirectDisplay.h>

/* Used for the redirection layer */

#ifdef __cplusplus
extern "C" {
#endif

extern OSErr PBControlSync(ParmBlkPtr paramBlock);
extern OSErr PBStatusSync(ParmBlkPtr paramBlock);
extern OSErr Control(short refNum,short csCode,const void *csParamPtr);
extern QHdrPtr GetDrvQHdr(void);
extern OSErr MacOpenDriver(ConstStr255Param name,short *drvrRefNum);
extern OSErr Eject(ConstStr63Param volName,short vRefNum);
extern OSErr GetDriverInformation(DriverRefNum refNum,UnitNumber *unitNum,DriverFlags *flags,
	DriverOpenCount *count,StringPtr name,RegEntryID *device,CFragSystem7Locator *driverLoadLocation,
	CFragConnectionID *fragmentConnID,DriverEntryPointPtr *fragmentMain,DriverDescription *driverDesc);

#ifdef __cplusplus
}
#endif

#endif

/* Help's a bit to NOT inline these. */

GDHandle VideoDevice;		/* Video device for graphic output */
#if !TARGET_API_MAC_CARBON
struct CGrafPort *VideoGWorld;	/* Grafport to offscreen buffer */
struct GrafPort *VideoWindow;	/* Window to display to */
#else
struct OpaqueGrafPtr *VideoGWorld;	/* Grafport to offscreen buffer */
struct OpaqueWindowPtr *VideoWindow;	/* Window to display to */
#endif
#if TARGET_RT_MAC_CFM
DSpContextReference MacContext;	/* Reference to a reserved DrawSprocket reference */
Bool MacDrawSprocketActive;		/* Has draw sprocket been started up? */
#endif

Bool MacUseBackBuffer = TRUE;	/* Which buffer is active */
Bool MacSystemTaskFlag;		/* If TRUE, then SystemTask() is called */
MacEventInterceptProcPtr MacEventIntercept;	/* Intercept for DoMacEvent() */
short MacVRefNum;		/* Volume reference used by Mac OS */
long MacDirID;			/* Directory reference used by MacOS */
short MacVRefNum2;		/* Volume reference used by copy and rename */
long MacDirID2;			/* Directory reference used by copy and rename */
short MacCacheVRefNum;	/* Cached VRef num for currently logged directory */
long MacCacheDirID;		/* Cached Dir ID for currently logged directory */

typedef struct Version_t {
#if TARGET_RT_MAC_CFM
	Word InputSprocketVersion;	/* InputSprocket version */
	Word DrawSprocketVersion;	/* DrawSprocket version */
#endif
	LibRef_t *InterfacePtr;		/* Reference to InterfaceLib if needed */
	LibRef_t *DriverLoaderLib;	/* Reference to DriverLoaderLib if needed */
#if TARGET_API_MAC_CARBON
	MacOSXFramework_t FoundationRef;	/* Foundation.framework */
	/* Functions for hybrid code */
	OSErr (*PBControlSync)(ParmBlkPtr paramBlock);
	OSErr (*PBStatusSync)(ParmBlkPtr paramBlock);
	OSErr (*Control)(short refNum,short csCode,const void *csParamPtr);
	QHdrPtr (*GetDrvQHdr)(void);
	OSErr (*MacOpenDriver)(ConstStr255Param name,short *drvrRefNum);
	OSErr (*Eject)(ConstStr63Param volName,short vRefNum);
	OSErr (*GetDriverInformation)(DriverRefNum refNum,UnitNumber *unitNum,DriverFlags *flags,
		DriverOpenCount *count,StringPtr name,RegEntryID *device,CFragSystem7Locator *driverLoadLocation,
		CFragConnectionID *fragmentConnID,DriverEntryPointPtr *fragmentMain,DriverDescription *driverDesc);

	CGDisplayErr (*CGSetDisplayTransferByTable)(CGDirectDisplayID display,CGTableCount tableSize,
		const CGGammaValue *redTable,const CGGammaValue *greenTable,const CGGammaValue *blueTable);
	CGDisplayErr (*CGGetDisplayTransferByTable)(CGDirectDisplayID display,CGTableCount capacity,
		CGGammaValue *redTable,CGGammaValue *greenTable,CGGammaValue *blueTable,CGTableCount *sampleCount);

#endif

	Word OSVersion;				/* MacOS Version */
	Word SoundManagerVersion;	/* Sound manager version */
	Word8 SoundManagerVerFlag;	/* TRUE if sound manager version is valid */
	Word8 InputSprocketVersFlag;	/* TRUE if InputSprocket version was obtained */
	Word8 DrawSprocketVersFlag;	/* TRUE if DrawSprocket version was obtained */
	Word8 LibClean;				/* TRUE if a library clean up function is installed */
} Version_t;

#if TARGET_RT_MAC_CFM
static const Word8 DebugDrawSpocketName[] = "\pDrawSprocketDebugLib";
static const Word8 DrawSprocketName[] = "\pDrawSprocketLib";
#endif

static Version_t VersionLocals;		/* Internal variables */

/**********************************

	If MacOSGetInterfaceLib() is called, this is called
	to release the instance of InterfaceLib
	
**********************************/

static void ANSICALL MacOSLibClean(void)
{
	Version_t *VersionPtr;
	VersionPtr = &VersionLocals;			/* Get the locals */
	if (VersionPtr->InterfacePtr) {			/* Was the lib loaded? */
		LibRefDelete(VersionPtr->InterfacePtr);	/* Get rid of it */
		VersionPtr->InterfacePtr = 0;		/* Clean it */
	}
	if (VersionPtr->DriverLoaderLib) {			/* Was the lib loaded? */
		LibRefDelete(VersionPtr->DriverLoaderLib);	/* Get rid of it */
		VersionPtr->DriverLoaderLib = 0;		/* Clean it */
	}
#if TARGET_API_MAC_CARBON
	if (VersionPtr->FoundationRef.LibBundle) {
		MacOSXFrameworkDestroy(&VersionPtr->FoundationRef);
	}
	VersionPtr->PBControlSync = 0;
	VersionPtr->PBStatusSync = 0;
	VersionPtr->Control = 0;
	VersionPtr->GetDrvQHdr = 0;
	VersionPtr->MacOpenDriver = 0;
	VersionPtr->Eject = 0;
	VersionPtr->CGSetDisplayTransferByTable = 0;
	VersionPtr->CGGetDisplayTransferByTable = 0;
#endif
}

/**********************************

	Detect if a MacOS trap number exists
	Return TRUE if so
	This call is for Classic MacOS, do not bother
	calling this function in MacOS X or CarbonLib
	
**********************************/

Word BURGERCALL MacOSIsTrapAvailable(int TrapNum) 
{
#if !TARGET_API_MAC_CARBON		/* Classic only */
	TrapType trType;
	
	trType = OSTrap;			/* OS trap! */
	if (TrapNum & 0x0800) {		/* What trap is it? */
		if (((TrapNum & 0x03FF) >= 0x0200) &&
			(GetToolboxTrapAddress(_InitGraf) == GetToolboxTrapAddress(_InitGraf+0x200))) {
			return FALSE;
		}
		trType = ToolTrap;		/* Toolbox trap */
	}
	
	if (NGetTrapAddress(TrapNum,trType) != GetToolboxTrapAddress(_Unimplemented)) {
		return TRUE;		/* The trap is present */
	}
#endif
	return FALSE;	/* Sorry! */
}

/**********************************

	Detect Quicktime Power Plug is present
	This is really old, only for Quicktime 2.5 do you even care.
		
**********************************/

Word BURGERCALL MacOSIsQuickTimePowerPlugAvailable(void)
{
#if defined(__POWERPC__)
	long gestaltAnswer;
	if (!Gestalt(gestaltQuickTimeFeatures,&gestaltAnswer)) {
		if ((gestaltAnswer & 1) && EnterMovies) {
			return TRUE;		/* Quicktime is in PowerPC */
		}
	}
#endif
	return FALSE;			/* 68k? */
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

	Return the Sound Manager version
	Note : I optimized this to only call the OS once
	
**********************************/

Word BURGERCALL MacOSGetSoundManagerVersion(void)
{
	NumVersion nver;
	Word Result;
	Version_t *VersionPtr;

	VersionPtr = &VersionLocals;			/* Get the locals */
	if (!VersionPtr->SoundManagerVerFlag) {
		VersionPtr->SoundManagerVerFlag = TRUE;		/* I got the version */
#if !TARGET_RT_MAC_CFM								/* 68K trap only */
		if (GetToolTrapAddress(_SoundDispatch) != GetToolTrapAddress(_Unimplemented))
#else
		if (SndSoundManagerVersion)			/* Check the pointer for CFM */
#endif
		{
			nver = SndSoundManagerVersion();		/* Get the version */
			Result = (nver.majorRev<<8)&0xFF00;		/* 8.8 version number */
			Result |= (nver.minorAndBugRev)&0xFF;
			VersionPtr->SoundManagerVersion = Result;		/* Store the version */
		}
	}
	return VersionPtr->SoundManagerVersion;		/* Return the version */
}

/**********************************

	Return the Input Sprocket version
	Note : I optimized this to only call the OS once
	
**********************************/

Word BURGERCALL MacOSGetInputSprocketVersion(void)
{
#if TARGET_RT_MAC_CFM
	NumVersion nver;
	Word Result;
	Version_t *VersionPtr;

	VersionPtr = &VersionLocals;			/* Get the locals */
	if (!VersionPtr->InputSprocketVersFlag) {
		VersionPtr->InputSprocketVersFlag = TRUE;	/* I got the version */
		if (ISpGetVersion) {		/* Code even present? */

		/* Call inputsprocket and get the version */

			nver = ISpGetVersion();
			Result = (nver.majorRev<<8)&0xFF00;
			Result |= (nver.minorAndBugRev)&0xFF;
			VersionPtr->InputSprocketVersion = Result;
		}
	}
	return VersionPtr->InputSprocketVersion;
#else
	return 0;		/* Not available in 68K traps */
#endif
}


/**********************************

	Return the Draw Sprocket version
	Note : DSpGetVersion() only appeared in 1.7
	as a result, to get the version from earlier versions
	I need to read the version resource found in the 
	library itself.
	
**********************************/

Word BURGERCALL MacOSGetDrawSprocketVersion(void)
{
#if TARGET_RT_MAC_CFM
	/* See if it is a really old version */
	Version_t *VersionPtr;

	VersionPtr = &VersionLocals;
	
	if (!VersionPtr->DrawSprocketVersFlag) {
		VersionPtr->DrawSprocketVersFlag = TRUE;
		if (!DSpGetVersion) {

			/* must be 1.1.4 or earlier so look at extension */

			short resFileSave;		/* Current resource file reft */
			short resFileDSp;		/* DrawSprocket's resource reference */
			short vRefDSp;			/* Volume for extensions folder */
			long dirIDDSp;			/* Directory for extensions folder */
			FSSpec fsSpecDSp;
			OSErr err;
			Handle hVersion;		/* Version data */
			UInt8 OldResLoad;
			
			OldResLoad = LMGetResLoad();	/* Get the previous setting */
			resFileSave = CurResFile();		/* Get the current resource ref */

			SetResLoad(FALSE);				/* Don't load in all the preloaded resources */
			HGetVol(0,&vRefDSp,&dirIDDSp);	/* Get my current folder */
			
			resFileDSp = HOpenResFile(vRefDSp,dirIDDSp,DebugDrawSpocketName,fsRdPerm);		/* Is it in my directory? */
			err = ResError();									/* Error? */
			if (err == fnfErr) {								/* Let's try again */
				resFileDSp = HOpenResFile(vRefDSp,dirIDDSp,DrawSprocketName,fsRdPerm);		/* Try the release version */
				err = ResError();								/* Error? */
				if (err == fnfErr) {							/* Try the system folder */
				
					/* At this time, I need to search the extensions folder */
					
					FindFolder(kOnSystemDisk, kExtensionFolderType, kDontCreateFolder, &vRefDSp, &dirIDDSp);

					/* Release build */
					
					FSMakeFSSpec(vRefDSp, dirIDDSp,DrawSprocketName,&fsSpecDSp);		/* Try the release version */

					resFileDSp = FSpOpenResFile(&fsSpecDSp,fsRdPerm);
					err = ResError();
					if (fnfErr == err) {
						
						/* Try the debug build */
						FSMakeFSSpec (vRefDSp, dirIDDSp, DebugDrawSpocketName, &fsSpecDSp);
						resFileDSp = FSpOpenResFile (&fsSpecDSp, fsRdPerm);
						
						/* Here is where I give up */
						err = ResError();
					}
				}
			}

			/* Do we have a resource open? */
			if (!err && (resFileDSp!=-1)) {
				SetResLoad(TRUE);						/* Ok, let's actually load something */
				hVersion = GetResource('vers',1); 		/* Get the version resource */
				if (hVersion && !ResError()) {		/* Valid? */
					Word8 *WorkPtr;
					Word Result;
					WorkPtr = ((Word8 **)hVersion)[0];	/* Deref the handle */
					Result = WorkPtr[0]<<8;				/* Major version */
					Result |= WorkPtr[1];				/* Minor version */
					VersionPtr->DrawSprocketVersion = Result;
					ReleaseResource(hVersion);			/* Release the version resource */
				}
				UseResFile(resFileSave);				/* Restore the resource file */
				CloseResFile(resFileDSp);				/* Close the lib's version resource file */
			}
			
			UseResFile(resFileSave);	/* Set the resource file to original setting */
			SetResLoad(OldResLoad);		/* Set the ResLoad flag to original setting */
		} else {
		
		/* Let's do it the easy way! */
		
			NumVersion nver;
			Word Result2;
		
			nver = DSpGetVersion();						/* Get the version */
			Result2 = (nver.majorRev<<8)&0xFF00;		/* Merge the version number */
			Result2 |= (nver.minorAndBugRev)&0xFF;
			VersionPtr->DrawSprocketVersion = Result2;	/* Store it */
		}
	}
	return VersionPtr->DrawSprocketVersion;		/* Not here */

#else
	return 0;			/* Oh oh..., Don't care in classic 68K */
#endif
}

/**********************************

	Return TRUE if AppleShare is present
	
**********************************/

static const Word16 AppleShareVer[] = {
	0x000,0x350,0x360,0x361,0x362,
	0x363,0x370,0x372,0x380,0x381,
	0x382,0x383};

Word BURGERCALL MacOSGetAppleShareVersion(void)
{
	long gestaltAnswer;
	Word Index;

	if (!Gestalt(gestaltAFPClient,&gestaltAnswer)) {		/* Detect Appleshare */
		Index = gestaltAnswer&gestaltAFPClientVersionMask;
		if (Index>11) {
			Index = 11;
		}
		return AppleShareVer[Index];		/* Return the version */
	}
	return FALSE;		/* Not present at all */
}

/**********************************

	Return TRUE if Altivec is present
	
**********************************/

Word BURGERCALL MacOSIsAltivecPresent(void)
{
#if defined(__POWERPC__)
	/* Carbon/Classic version */
	
	long gestaltAnswer;
	if (!Gestalt(gestaltPowerPCProcessorFeatures,&gestaltAnswer)) {
		if (gestaltAnswer & (1<<gestaltPowerPCHasVectorInstructions)) {
			return TRUE;
		}
	}
#endif
	return FALSE;
}

/**********************************

	Return the string of the extensions folder
	
**********************************/

void BURGERCALL MacOSSetExtensionsPrefix(Word PrefixNum)
{
	short vRef;			/* Volume for extensions folder */
	long dirID;			/* Directory for extensions folder */
	char *NewPath;
	FSSpec MyFsSpec;
	
	if (!FindFolder(kOnSystemDisk, kExtensionFolderType, kDontCreateFolder, &vRef, &dirID)) {
		FSMakeFSSpec(vRef,dirID,"\p", &MyFsSpec);		/* Try the release version */
		NewPath = GetFullPathFromMacFSSpec(&MyFsSpec);
		if (NewPath) {
			SetAPrefix(PrefixNum,NewPath);		/* Set the prefix to the directory */
			DeallocAPointer((void *)NewPath);	/* Release the path */
		}
	}
}

/**********************************

	Initialize the basic MacOS tools
	
**********************************/

void BURGERCALL MacOSInitTools(void)
{
#if !TARGET_API_MAC_CARBON
	MaxApplZone();					/* Expand the heap so code segments load at the top */
	InitGraf((Ptr)&qd.thePort);		/* Init the graphics system */
	InitFonts();					/* Init the font manager */
	InitWindows();					/* Init the window manager */
	InitMenus();					/* Init the menu manager */
	TEInit();						/* Init text edit */
	InitDialogs(0);					/* Init the dialog manager */
#endif
}

/**********************************

	This will use a generic pathname and open a Macintosh
	file. It has all the virtues and problems of HOpen();

**********************************/

long BURGERCALL MacOpenFileForRead(const char *PathName)
{
	char FileName[FULLPATHSIZE];
	Word8 NameBuffer[256];
	short RefNum;

	ExpandAPathToBufferNative(FileName,PathName);		/* Create the directories */
	CStr2PStr((char *)NameBuffer,FileName);
	if (!HOpen(MacVRefNum,MacDirID,NameBuffer,fsRdPerm,&RefNum)) {	/* Open the file */
		return RefNum;
	}
	return -1;			/* Uh oh... */
}

/**********************************

	Open a mac control panel
	I will first scan for a match based on
	the creator type. This way, if it's renamed
	in another language, I can find it.
	
**********************************/

Word BURGERCALL MacOpenControlPanel(Word32 type,const char *defaultname,Word Continue)
{
	short vref;		/* Volume ref for control panel directory */
	long dirID;		/* Directory ID for control panels */
	CInfoPBRec pb;	/* Directory scanner */
	Str255 name;
	char cname[256];
	int i;
	
	/* Is there a control panels folder? */
	
	if (FindFolder(kOnSystemDisk,kControlPanelFolderType,kDontCreateFolder,&vref,&dirID)) {
		strcpy(ErrorMsg,"The control panels folder can't be found");
		return TRUE;		/* Can't find the folder */
	}
	
	i = 1;
	for (;;) {
		pb.hFileInfo.ioCompletion = 0;
		pb.hFileInfo.ioResult = noErr;
		pb.hFileInfo.ioNamePtr = name;
		pb.hFileInfo.ioVRefNum = vref;
		pb.hFileInfo.ioFDirIndex = i;
		pb.hFileInfo.ioDirID = dirID;
		if (PBGetCatInfoSync(&pb)) {						/* Not found? */
			return MacLaunch(vref,dirID,defaultname,Continue);		/* Launch the filename then */
		}
		if ((pb.hFileInfo.ioFlFndrInfo.fdType == type) &&		/* Match the memory type? */
			(pb.hFileInfo.ioFlFndrInfo.fdCreator == 'APPC' || pb.hFileInfo.ioFlFndrInfo.fdCreator == 'APPL')) {
			PStr2CStr(cname,(char *)name);
			return MacLaunch(vref,dirID,cname,Continue);
		}
		++i;		/* Continue */
	}
}

/**********************************

	Launch an app

**********************************/

Word BURGERCALL MacLaunch(short vref, long dirID, const char *name,Word Continue)
{
	LaunchParamBlockRec pb;		/* Must be persistant memory */
	FSSpec spec;
	Bool ignore1, ignore2;
	Str255 pname;
	
	CStr2PStr((char *)pname,name);
	if (FSMakeFSSpec(vref,dirID,pname,&spec)) {
		sprintf(ErrorMsg,"Can't find the file %s",name);
		return TRUE;
	}
	if (ResolveAliasFile(&spec,TRUE,&ignore1,&ignore2)) {
		sprintf(ErrorMsg,"Can't parse the alias %s",name);
		return TRUE;
	}
	if (Continue) {
		Continue = launchContinue|launchNoFileFlags;		/* Stay alive */
	} else {
		Continue = launchNoFileFlags;						/* Kill me now! */
	}
	FastMemSet(&pb,0,sizeof(pb));
//	pb.reserved1 = 0;
//	pb.reserved2 = 0;
	pb.launchBlockID = extendedBlock;
	pb.launchEPBLength = extendedBlockLen;
//	pb.launchFileFlags = 0;
	pb.launchControlFlags = Continue;						/* Kill me now! */
	pb.launchAppSpec = &spec;
//	pb.launchAppParameters = 0;
	if (LaunchApplication(&pb)) {		/* This should quit my application */
		// if we return from launchapplication something broke
		sprintf(ErrorMsg,"Can't execute the file %s",name);
		return TRUE;
	}
	return FALSE;		/* Successful launch */
}

/**********************************

	Open a single CD tray

**********************************/

void BURGERCALL MacOSEjectCD(void)
{
#if !TARGET_API_MAC_CARBON
	Word i;
	UnitNumber unitNum;		/* Driver unit number */
	DriverFlags flags;		/* Driver flags */
	DriverOpenCount count;	/* Number of times it's open */
	RegEntryID device;		/* Device registration entry */
	Str255 drvrName;		/* Driver's name */
	CFragSystem7Locator driverLoadLocation;	/* Driver fragment location */
	CFragConnectionID fragmentConnID;		/* Driver connection ID */
	DriverEntryPointPtr fragmentMain;		/* Driver main code entry */
	DriverDescription driverDesc;			/* Type of driver */
	HParamBlockRec pb;						/* Used to talk to the driver */
	char foo[256];		/* Volume name converted to a "C" string */
	Str255 volName;		/* Buffer for the device's volume name */
	
	/* Scan all active devices and look for an empty one */
	
	{
		DrvQElPtr drvQElem;
		QHdrPtr DrvrPtr;
		DrvrPtr = GetDrvQHdr();
		if (DrvrPtr) {
			drvQElem = (DrvQElPtr)(DrvrPtr->qHead);		/* Get the first record */
			if (drvQElem) {
				do {
					if (!GetDriverInformation(drvQElem->dQRefNum,&unitNum,&flags,&count, drvrName, &device, &driverLoadLocation, &fragmentConnID, &fragmentMain, &driverDesc )) {
						PStr2CStr(foo,(char *)drvrName);
						if (!stricmp(foo,".AppleCD") || !stricmp(foo,".AppleDVD")) {
						
							/* Scan the mounted volumes... */
							/* for the device, a CD can have multiple partitions */
							/* on it, so all must be released */
						
							i = 1;
							for (;;) {
								FastMemSet(&pb,0,sizeof(pb));		/* Blank */
								pb.volumeParam.ioNamePtr = volName;
								pb.volumeParam.ioVolIndex = i;
								if (PBHGetVInfoSync(&pb)) {		/* Get the mounted volume */
									break;
								}
								if (pb.volumeParam.ioVDrvInfo == drvQElem->dQDrive) {
									goto NoEject;				/* No good */
								}
								++i;
							}
							/* no volume found for this CD */
							/* Eject the tray */
	
							if (!Eject(NULL, drvQElem->dQDrive )) {
								return;
							}
NoEject:;	
						}
					}
					drvQElem = (DrvQElPtr)drvQElem->qLink;
				} while (drvQElem);
			}
		}
	}
	
	/* Ok, there must be a CD in the drive */
	/* Eject the first CD I find */
	
	i = 1;
	for (;;) {
		FastMemSet(&pb,0,sizeof(pb));
		pb.volumeParam.ioNamePtr = volName;
		pb.volumeParam.ioVolIndex = i;
		if (PBHGetVInfoSync(&pb)) {
			break;		/* Oops... give up */
		}
		if (!GetDriverInformation( pb.volumeParam.ioVDRefNum, &unitNum, &flags, &count, drvrName, &device, &driverLoadLocation, &fragmentConnID, &fragmentMain, &driverDesc )) {
			PStr2CStr(foo,(char *)drvrName);
			if (!stricmp(foo, ".AppleCD") || !stricmp(foo,".AppleDVD")) {
				if (!UnmountVol(volName,pb.volumeParam.ioVRefNum)) {
					if (!Eject(volName,pb.volumeParam.ioVDrvInfo)) {
						return;
					}
				}
			}
		}
		++i;
	}
	
	/* Ok, there is no drive, let's see if it's an empty drive. */
	/* If, so, then send the eject command */
	
	{
		ParamBlockRec pb2;
		short DriverID;
		FastMemSet(&pb2,0,sizeof(pb2));
		pb2.cntrlParam.csCode = kEject;
		if (!MacOpenDriver("\p.AppleCD",&DriverID)) {
			pb2.cntrlParam.ioVRefNum = DriverID;
			pb2.cntrlParam.ioCRefNum = DriverID;
			if (!PBControlSync(&pb2)) {
				return;
			}
		}
		
	/* Now drives may call themselves a "DVD" drive */
	/* Handle this case as well */
	
		if (!MacOpenDriver("\p.AppleDVD",&DriverID)) {
			pb2.cntrlParam.ioVRefNum = DriverID;
			pb2.cntrlParam.ioCRefNum = DriverID;
			if (!PBControlSync(&pb2)) {
				return;
			}
		}
	}
#else

/**********************************

	Ok boys and girls, this level of hell exists
	since I want to actually talk to the CD ROM driver
	and OS X and OS 9 are TOTALLY different.
	
	First, Let's try OS 9, the code is the same as above except
	that I have to manually link in InterfaceLib and DriverLoaderLib

**********************************/

	if (MacOSGetOSVersion()<0x1000) {
		Word i;
		UnitNumber unitNum;		/* Driver unit number */
		DriverFlags flags;		/* Driver flags */
		DriverOpenCount count;	/* Number of times it's open */
		RegEntryID device;		/* Device registration entry */
		Str255 drvrName;		/* Driver's name */
		CFragSystem7Locator driverLoadLocation;	/* Driver fragment location */
		CFragConnectionID fragmentConnID;		/* Driver connection ID */
		DriverEntryPointPtr fragmentMain;		/* Driver main code entry */
		DriverDescription driverDesc;			/* Type of driver */
		HParamBlockRec pb;						/* Used to talk to the driver */
		char foo[256];		/* Volume name converted to a "C" string */
		Str255 volName;		/* Buffer for the device's volume name */
		
		/* Scan all active devices and look for an empty one */

		DrvQElPtr drvQElem;
		QHdrPtr DrvrPtr;
										
		DrvrPtr = GetDrvQHdr();
		if (DrvrPtr) {
			drvQElem = (DrvQElPtr)(DrvrPtr->qHead);		/* Get the first record */
			if (drvQElem) {
				do {
					if (!GetDriverInformation(drvQElem->dQRefNum,&unitNum,&flags,&count, drvrName, &device, &driverLoadLocation, &fragmentConnID, &fragmentMain, &driverDesc )) {
						PStr2CStr(foo,(char *)drvrName);
						if (!stricmp(foo,".AppleCD") || !stricmp(foo,".AppleDVD")) {
								
							/* Scan the mounted volumes... */
							/* for the device, a CD can have multiple partitions */
							/* on it, so all must be released */
							
							i = 1;
							for (;;) {
								FastMemSet(&pb,0,sizeof(pb));		/* Blank */
								pb.volumeParam.ioNamePtr = volName;
								pb.volumeParam.ioVolIndex = i;
								if (PBHGetVInfoSync(&pb)) {		/* Get the mounted volume */
									break;
								}
								if (pb.volumeParam.ioVDrvInfo == drvQElem->dQDrive) {
									goto NoEject;				/* No good */
								}
								++i;
							}
							/* no volume found for this CD */
							/* Eject the tray */

							if (!Eject(NULL, drvQElem->dQDrive )) {
								goto ReturnNow;
							}
		NoEject:;
						}
					}
					drvQElem = (DrvQElPtr)drvQElem->qLink;
				} while (drvQElem);
			}
		}
		/* Ok, there must be a CD in the drive */
		/* Eject the first CD I find */
			
		i = 1;
		for (;;) {
			FastMemSet(&pb,0,sizeof(pb));
			pb.volumeParam.ioNamePtr = volName;
			pb.volumeParam.ioVolIndex = i;
			if (PBHGetVInfoSync(&pb)) {
				break;		/* Oops... give up */
			}
			if (!GetDriverInformation( pb.volumeParam.ioVDRefNum, &unitNum, &flags, &count, drvrName, &device, &driverLoadLocation, &fragmentConnID, &fragmentMain, &driverDesc )) {
				PStr2CStr(foo,(char *)drvrName);
				if (!stricmp(foo, ".AppleCD") || !stricmp(foo,".AppleDVD")) {
					if (!UnmountVol(volName,pb.volumeParam.ioVRefNum)) {
						if (!Eject(volName,pb.volumeParam.ioVDrvInfo)) {
							goto ReturnNow;
						}
					}
				}
			}
			++i;
		}
			
		/* Ok, there is no drive, let's see if it's an empty drive. */
		/* If, so, then send the eject command */
			
		{
			ParamBlockRec pb2;
			short DriverID;
			FastMemSet(&pb2,0,sizeof(pb2));
			pb2.cntrlParam.csCode = kEject;
			if (!MacOpenDriver("\p.AppleCD",&DriverID)) {
				pb2.cntrlParam.ioVRefNum = DriverID;
				pb2.cntrlParam.ioCRefNum = DriverID;
				if (!PBControlSync(&pb2)) {
//					goto ReturnNow;
				}
			}
				
			/* Now drives may call themselves a "DVD" drive */
			/* Handle this case as well */
			
			if (!MacOpenDriver("\p.AppleDVD",&DriverID)) {
				pb2.cntrlParam.ioVRefNum = DriverID;
				pb2.cntrlParam.ioCRefNum = DriverID;
				if (!PBControlSync(&pb2)) {
					goto ReturnNow;
				}
			}
ReturnNow:;
		}
	} else {

/**********************************

	Since I am OS X or greater, I must use the "NextStep" driver
	protocol. Why can't life be easy?
	
**********************************/

	/* First, make some fake typedefs for CoreFoundation routines */
	
		typedef struct OSObject *io_object_t;
		typedef io_object_t io_iterator_t;
		typedef io_object_t io_registry_entry_t;
		typedef char *io_name_t;
		typedef	int kern_return_t;
		typedef unsigned int natural_t;
		typedef natural_t port_name_t;
		typedef port_name_t port_t;
		typedef port_t mach_port_t;
		
	/* Now, make function prototypes for the Mach-O routines */
	
		typedef kern_return_t (*IOMasterPortPtr)(mach_port_t bootstrapPort, mach_port_t * masterPort);
		typedef CFMutableDictionaryRef (*IOBSDNameMatchingPtr)(mach_port_t masterPort, unsigned int options, const char *bsdName);
		typedef kern_return_t (*IOServiceGetMatchingServicesPtr)(mach_port_t masterPort, CFDictionaryRef matching, io_iterator_t *existing);
		typedef io_object_t (*IOIteratorNextPtr)(io_iterator_t iterator);
		typedef boolean_t (*IOObjectConformsToPtr)(io_object_t object, const io_name_t);
		typedef kern_return_t (*IORegistryEntryGetParentEntryPtr)(io_registry_entry_t entry, const io_name_t plane, io_registry_entry_t * parent);
		typedef kern_return_t (*IOObjectReleasePtr)(io_object_t object);
	
	/* Here are the actual routines */
		
		IOMasterPortPtr IOMasterPort;
		IOBSDNameMatchingPtr IOBSDNameMatching;
		IOServiceGetMatchingServicesPtr IOServiceGetMatchingServices;
		IOIteratorNextPtr IOIteratorNext;
		IOObjectConformsToPtr IOObjectConformsTo;
		IORegistryEntryGetParentEntryPtr IORegistryEntryGetParentEntry;
		IOObjectReleasePtr IOObjectRelease;
		
		OSStatus err;
		CFBundleRef bundleRef = 0;
		FSRef frameworksFolderRef;
		CFURLRef baseURL = 0;
		CFURLRef bundleURL = 0;
		
		err = coreFoundationUnknownErr;
		
		if (!FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType,TRUE,&frameworksFolderRef)) {
			baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
			if (baseURL) {
				bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL,CFSTR("IOKit.framework"),FALSE);
				if (bundleURL) {
					bundleRef = CFBundleCreate(kCFAllocatorSystemDefault,bundleURL);
					if (bundleRef) {
		    			if (CFBundleLoadExecutable(bundleRef)) {
							//	Bundle loaded in "bundleRef"
							mach_port_t masterPort;
							kern_return_t kernErr;
							int i;

							/* Get the function pointers */
							
							IOMasterPort = (IOMasterPortPtr)CFBundleGetFunctionPointerForName(bundleRef, CFSTR("IOMasterPort"));
							IOBSDNameMatching = (IOBSDNameMatchingPtr)CFBundleGetFunctionPointerForName(bundleRef, CFSTR("IOBSDNameMatching"));
							IOServiceGetMatchingServices = (IOServiceGetMatchingServicesPtr)CFBundleGetFunctionPointerForName(bundleRef, CFSTR("IOServiceGetMatchingServices"));
							IOIteratorNext = (IOIteratorNextPtr)CFBundleGetFunctionPointerForName(bundleRef, CFSTR("IOIteratorNext"));
							IOObjectConformsTo = (IOObjectConformsToPtr)CFBundleGetFunctionPointerForName(bundleRef, CFSTR("IOObjectConformsTo"));
							IORegistryEntryGetParentEntry = (IORegistryEntryGetParentEntryPtr)CFBundleGetFunctionPointerForName(bundleRef, CFSTR("IORegistryEntryGetParentEntry"));
							IOObjectRelease = (IOObjectReleasePtr)CFBundleGetFunctionPointerForName(bundleRef, CFSTR("IOObjectRelease"));
							
							if (!IOMasterPort(0,&masterPort)) {
								i = 1;
								for (;;) {
									HParamBlockRec	pb;
									GetVolParmsInfoBuffer volInfo;
									FSVolumeRefNum vRefNum = 0;
									HFSUniStr255 volNameUni;
									io_object_t theObj, parentObj;
									
									Word bIsCDROM = FALSE;
									char buffer[256];
									int j;

									if (FSGetVolumeInfo(kFSInvalidVolumeRefNum,i,&vRefNum,kFSVolInfoNone,0, &volNameUni,0)) { // Ran Out
										break;		/* Exit now */
									}
									
									//	Convert volNameUni to C string
									for (j = 0; j < volNameUni.length; ++j) {
										buffer[j] = (char)volNameUni.unicode[j];
									}
									buffer[volNameUni.length] = '\0';
									
									FastMemSet(&pb,0,sizeof(pb));
									pb.ioParam.ioVRefNum = vRefNum;
									pb.ioParam.ioBuffer = (Ptr)&volInfo;
									pb.ioParam.ioReqCount = sizeof(volInfo);

									if (!PBHGetVolParmsSync(&pb)) {
										if (volInfo.vMVersion >= 4 && volInfo.vMDeviceID) {	// Old Version or no vMDeviceID
											char *devName;
											io_iterator_t theIterator;
											devName = StrCopy((char *)volInfo.vMDeviceID);
											
											//	Found a drive... see if it's a CD...oi!
											if (!IOServiceGetMatchingServices(masterPort, IOBSDNameMatching(masterPort, 0, devName), &theIterator)) {
												theObj = IOIteratorNext(theIterator);
												if (theObj) {
													do {
														for (;;) {
															if (IOObjectConformsTo(theObj, "IOCDMedia") || IOObjectConformsTo(theObj, "IODVDMedia")) {
																bIsCDROM = TRUE;
																IOObjectRelease(theObj);
																break;
															}
															
															kernErr = IORegistryEntryGetParentEntry(theObj, "IOService", &parentObj);
															IOObjectRelease(theObj);
															if (kernErr) {
																break;
															}
															theObj = parentObj;
														}
														theObj = IOIteratorNext(theIterator);
													} while (theObj);
												}
												IOObjectRelease(theIterator);
											}
											DeallocAPointer(devName);		/* Kill the device name */
											if (bIsCDROM) {
												//	Eject
												Str255 str;
												CStr2PStr((char *)str,buffer);
												if (!UnmountVol(str, vRefNum)) {		/* Pass the volume name */
													break;			/* Eject it! */
												}
											}
										}
									}
									++i;
								}
							}
						}
						CFRelease(bundleRef);
					}
					CFRelease(bundleURL);
				}
				CFRelease(baseURL);
			}
		}
	}
#endif
}

/**********************************

	Send a "Quit" event to the requested process
	Note : Maintain a timeout since not all apps
	support the "Quit" apple event
	
**********************************/

void BURGERCALL MacOSKillProcess(struct ProcessSerialNumber *victim)
{
	OSErr err;
	AEAddressDesc target;
	AppleEvent theEvent;
	Word waits;

	/* Create a "Quit" event */

	if (!AECreateDesc(typeProcessSerialNumber, (Ptr)victim, sizeof(ProcessSerialNumber), &target)) {
		err = AECreateAppleEvent('aevt', 'quit', &target, kAutoGenerateReturnID, kAnyTransactionID, &theEvent);
		AEDisposeDesc(&target);
		if (!err) {
			/* Send the "Quit" event */
			AESend(&theEvent, 0, kAENoReply + kAENeverInteract, kAENormalPriority, kAEDefaultTimeout,0,0);
			AEDisposeDesc(&theEvent);
		}
	}

	/* Give some CPU time for the event to trigger */

	waits = 7;
	do {
		EventRecord event;
		if (!WaitNextEvent(everyEvent,&event,180,0)) {
			break;
		}
	} while (--waits);
}

/**********************************

	Send a "Quit" event to every other app
	however, don't kill myself and kill the finder last
	
**********************************/

void BURGERCALL MacOSKillAllProcesses(void)
{
	ProcessSerialNumber MyAppNumber;		/* My apps own process number */
	ProcessSerialNumber current;			/* Current process being scanned */
	ProcessSerialNumber next;				/* Next process in chain */
	ProcessSerialNumber finder;				/* Finder process ID */
	Word foundFinder;

	GetCurrentProcess(&MyAppNumber);		/* Get my current app process number */

	next.highLongOfPSN = 0;				/* Start following the process list */
	next.lowLongOfPSN = kNoProcess;
	GetNextProcess(&next);				/* Get my number */

	/* Found another process? */
	
	if (next.highLongOfPSN || next.lowLongOfPSN != kNoProcess) {
		foundFinder = FALSE;
		do {
			Bool IsFlag;

			current = next;
			GetNextProcess(&next);		/* Preload the NEXT process since I may kill it now */

			SameProcess(&current,&MyAppNumber,&IsFlag);		/* Don't kill myself */
			if (!IsFlag) {

				/* If I have found the finder, the rest are easy. */
				/* Otherwise... */

				if (!foundFinder) {			/* Find it already? */
					Str31 processName;
					FSSpec procSpec;
					ProcessInfoRec infoRec;

					infoRec.processInfoLength = sizeof(ProcessInfoRec);
					infoRec.processName = (StringPtr)&processName;
					infoRec.processAppSpec = &procSpec;
					GetProcessInformation(&current,&infoRec);		/* What process is this? */
					if (infoRec.processSignature == 'MACS' && infoRec.processType == 'FNDR') {
						finder = current;			/* This is the finder! */
						foundFinder = TRUE;
						IsFlag = TRUE;
					} else {
						IsFlag = FALSE;
					}
				} else {
				
					/* You may ask yourself, why check for finder when the finder */
					/* is already found? The finder has multiple processes!!! */
					
					SameProcess(&current,&finder,&IsFlag);		/* Just do a compare */
				}
				
				/* Is this app not the finder? */
				
				if (!IsFlag) {						
					MacOSKillProcess(&current);		/* Kill it */
				}
			}
		} while (next.highLongOfPSN || next.lowLongOfPSN != kNoProcess);
	}

	/* Now, did I locate the finder? */

	if (foundFinder) {
		MacOSKillProcess(&finder);		/* Bye bye */
	}
}

/**********************************

	Load InterfaceLib for manual linking
	
**********************************/

LibRef_t * BURGERCALL MacOSGetInterfaceLib(void)
{
	Version_t *VersionPtr;
	LibRef_t *Result;
	VersionPtr = &VersionLocals;
	Result = VersionPtr->InterfacePtr;
	if (!Result) {			/* Is interfacelib installed? */
		Result = LibRefInit("InterfaceLib");	/* Try to get the lib */
		VersionPtr->InterfacePtr = Result;		/* Can be zero on failure! */
		
		if (!VersionPtr->LibClean) {
			atexit(MacOSLibClean);
			VersionPtr->LibClean = TRUE;			/* I've checked */
		}
	}
	return Result;
}

/**********************************

	Load InterfaceLib for manual linking
	
**********************************/

LibRef_t * BURGERCALL MacOSDriverLoaderLib(void)
{
	Version_t *VersionPtr;
	LibRef_t *Result;
	VersionPtr = &VersionLocals;
	Result = VersionPtr->DriverLoaderLib;
	if (!Result) {			/* Is DriverLoaderLib installed? */
		Result = LibRefInit("DriverLoaderLib");	/* Try to get the lib */
		VersionPtr->DriverLoaderLib = Result;		/* Can be zero on failure! */
		
		if (!VersionPtr->LibClean) {
			atexit(MacOSLibClean);
			VersionPtr->LibClean = TRUE;			/* I've checked */
		}
	}
	return Result;
}

#if TARGET_API_MAC_CARBON
MacOSXFramework_t * BURGERCALL MacOSGetFoundationFramework(void)
{
	Version_t *VersionPtr;
	MacOSXFramework_t *Result;
	VersionPtr = &VersionLocals;
	Result = &VersionPtr->FoundationRef;
	if (!Result->LibBundle) {			/* Is interfacelib installed? */
		if (MacOSXFrameworkInit(Result,"Foundation.framework")) {
			Result = 0;
		}
		if (!VersionPtr->LibClean) {
			atexit(MacOSLibClean);
			VersionPtr->LibClean = TRUE;			/* I've checked */
		}
	}
	return Result;
}
#endif


/*******************************

	Return a value from a control in 
	a dialog
	
*******************************/

Word BURGERCALL MacOSDialogControlGetValue(DialogRef dialog,Word item) 
{
	short itemType;
	Rect r;
	ControlHandle control;
		
	GetDialogItem(dialog,(short) item, &itemType, (Handle *) &control, &r);
	return GetControlValue(control);
}

/*******************************

	Set a value to a control in 
	a dialog
	
*******************************/

void BURGERCALL MacOSDialogControlSetValue(DialogRef dialog,Word item,Word Value) 
{
	short itemType;
	Rect r;
	ControlHandle control;
		
	GetDialogItem(dialog, (short)item, &itemType, (Handle *) &control, &r);
	SetControlValue(control,(short)Value);
}

/*******************************

	Reverse a value to a control in 
	a dialog
	
*******************************/

void BURGERCALL MacOSDialogControlToggleValue(DialogRef dialog,Word item) 
{
	short itemType;
	Rect r;
	ControlHandle controlmem;
	ControlHandle control;
		
	GetDialogItem(dialog,(short)item, &itemType, (Handle *) &controlmem, &r);
	control = controlmem;
	SetControlValue(control,!GetControlValue(control));
}


/**********************************

	Take a CFM function pointer and create a code stub that
	will allow this to be called from Mach-O.
	The memory is allocated with a call to NewPtr
	
**********************************/

#if TARGET_API_MAC_CARBON
void * BURGERCALL MacOSMachOToCFM(void *cfmfp)
{
	Word32 *mfp = (Word32*)NewPtr(6*sizeof(Word32));	/* Create the stub */
	if (mfp) {
		mfp[0] = 0x3D800000 | ((Word32)cfmfp >> 16);	/* lis r12,0 */
		mfp[1] = 0x618C0000 | ((Word32)cfmfp & 0xFFFF);	/* ori r12,r12,0 */
		mfp[2] = 0x800C0000;		/* lwz r0,0(r12) */
		mfp[3] = 0x804C0004;		/* lwz r2,4(r12) */
		mfp[4] = 0x7C0903A6;		/* mtctr r0 */
		mfp[5] = 0x4E800420;		/* bctr */
		MakeDataExecutable(mfp,6*sizeof(Word32));		/* Allow MacOS to execute code here */
	}
	return mfp;					/* Return the pointer */
}

/**********************************

	Release a code stub allocated by MacOSMachOToCFM()
	
**********************************/

void BURGERCALL MacOSMachOToCFMDelete(void *MachPtr)
{
	if (MachPtr) {
		DisposePtr(static_cast<char *>(MachPtr));
	}
}
#endif

#if TARGET_API_MAC_CARBON && defined(__MRC__) && 0

/**********************************

	This code is to allow StdCLib to work flawlessly
	with a CFM version of Burgerlib.
	
	You see, some MORON made StdCLib under Mac OS X a Mach-O only
	binary. So all the CFM calls are converted to Mach-O and back again.
	
	Why do I care you ask? Well functions like atexit() and sort() require
	a callback proc and all callback procs are using a true pointer which
	in CFM is a pointer to a TOC entry. Now, Mach-O binaries use a direct
	pointer to the proc. Therefore all ANSI callbacks will explode if
	you are not a Mach-O binary.
	
	Oh joy and yummy!
	
	And to add insult to injury, flags are also damaged beyond repair for some calls... :(

**********************************/

#ifdef __cplusplus
extern "C" {
#endif

/* Callback typedefs */

typedef void (*atexitProc)(void);
typedef int (*qsortProc)(const void *, const void *);

static Word8 LoadedStdCLib;			/* TRUE once the function pointers are set */

/* Here are all the functions in question */

static struct {
	Word Version;					/* MacOS version */
	int (*atexit)(atexitProc f);	/* Call to atexit in StdCLib */
	void (*qsort)(void *,size_t,size_t,qsortProc compar);	/* Call to qsort in StdCLib */
	void *(*bsearch)(const void*, const void*, size_t, size_t,qsortProc compar);	/* Call to bsearch in StdCLib */
	char *(*setlocale)(int category, const char *locale);
	int (*open)(const char *path, int oflag);
	int (*unlink)(const char *path);
	int (*remove)(const char *filename);
	int (*rename)(const char *oldname, const char *newname);
	FILE *(*fopen)(const char *filename, const char *mode);
	FILE *(*freopen)(const char *filename, const char *mode, FILE *stream);
	int (*setvbuf)(FILE *stream, char *buf, int mode, size_t size);
} Real;

#ifdef __cplusplus
}
#endif

/**********************************

	Get the REAL function calls from StdCLib

**********************************/

static void MacOSStdCLibCalls(void)
{
	LibRef_t *StdCLib;
	
	if (!LoadedStdCLib) {		/* Is it initialized? */
		LoadedStdCLib = TRUE;	/* Init me */
		StdCLib = LibRefInit("StdCLib");		/* Get StdCLib */
		if (StdCLib) {							/* Got it! */
			Real.Version = MacOSGetOSVersion();
			Real.atexit = LibRefGetProc(StdCLib,"atexit");		/* Init all the procs */
			Real.qsort = LibRefGetProc(StdCLib,"qsort");
			Real.bsearch = LibRefGetProc(StdCLib,"bsearch");
			Real.setlocale = LibRefGetProc(StdCLib,"setlocale");
			Real.open = LibRefGetProc(StdCLib,"open");
			Real.unlink = LibRefGetProc(StdCLib,"unlink");
			Real.remove = LibRefGetProc(StdCLib,"remove");
			Real.rename = LibRefGetProc(StdCLib,"rename");
			Real.fopen = LibRefGetProc(StdCLib,"fopen");
			Real.freopen = LibRefGetProc(StdCLib,"freopen");
			Real.setvbuf = LibRefGetProc(StdCLib,"setvbuf");
		}
	}
}

/**********************************

	Convert a filename from MacRoman to UTF8
	Used to allow MacRoman as the ASCII format while
	passing UTF8 to MacOS X
	
**********************************/

static void FixFileName(char *Output,const char *Input)
{
	CFStringRef StringRef;
	StringRef = CFStringCreateWithCString(0,Input,kCFStringEncodingMacRoman);	/* Convert to a string ref */
	CFStringGetCString(StringRef,Output,2048,kCFStringEncodingUTF8);			/* Perform the conversion */
	CFRelease(StringRef);														/* Dispose of the string ref */
}

/**********************************

	Intercept atexit and allow it to work under
	StdCLib under MacOSX
	
**********************************/

int atexit(atexitProc func)
{
	MacOSStdCLibCalls();		/* Make sure I'm loaded */
	if (Real.atexit) {			/* Valid? */
		if (Real.Version>=0xA00) {	/* Get the OS version */
			func = (atexitProc)MacOSMachOToCFM(func);	/* Make a Mach-O to CFM converter */
			if (!func) {
				return 0;
			}
		}
		return Real.atexit(func);		/* Call atexit() */
	}
	return 0;					/* Error */
}

/**********************************

	Intercept qsort and allow it to work under
	StdCLib under MacOSX
	
**********************************/

void qsort(void *base,size_t count,size_t size,qsortProc compar)
{
	MacOSStdCLibCalls();		/* Make sure I'm loaded */
	if (Real.qsort) {			/* Valid? */
		if (Real.Version<0xA00) {	/* Get the OS version */
			Real.qsort(base,count,size,compar);	/* Before OS 10.0 */
		} else {
			void *compar2;
			compar2 = MacOSMachOToCFM(compar);	/* Make the fragment call via Mach-O */
			if (compar2) {
				Real.qsort(base,count,size,compar2);	/* Call the function */
				MacOSXCFMToMachODelete(compar2);				/* Dispose of the code fragment */
			}
		}
	}
}

/**********************************

	Intercept bsearch and allow it to work under
	StdCLib under MacOSX
	
**********************************/

void *bsearch(const void*key, const void* base, size_t nmemb, size_t size,qsortProc compar)
{
	MacOSStdCLibCalls();		/* Make sure I'm loaded */
	if (Real.bsearch) {			/* Valid? */
		if (Real.Version<0xA00) {	/* Get the OS version */
			return Real.bsearch(key,base,nmemb,size,compar);	/* Before OS 10.0 */
		}
		{
			void *Result;
			void *compar2;
			compar2 = MacOSMachOToCFM(compar);	/* Make the fragment call via Mach-O */
			if (compar2) {
				Result = Real.bsearch(key,base,nmemb,size,compar2);	/* Call the function */
				MacOSXCFMToMachODelete(compar2);				/* Dispose of the code fragment */
				return Result;
			}
		}
	}
	return 0;
}

/**********************************

	Intercept setlocale and allow it to work under
	StdCLib under MacOSX
	
**********************************/

char *setlocale(int category, const char *locale)
{
	MacOSStdCLibCalls();		/* Make sure I'm loaded */
	if (Real.setlocale) {			/* Valid? */
		if (Real.Version>=0xA00) {
			--category;		/* Convert the category */
		}
		return Real.setlocale(category,locale);
	}
	return 0;
}

/**********************************

	Intercept open and allow it to work under
	StdCLib under MacOSX
	
**********************************/

int open(const char *path, int oflag)
{
	char NewName[2048];
	MacOSStdCLibCalls();		/* Make sure I'm loaded */
	if (Real.open) {			/* Valid? */
		if (Real.Version>=0xA00) {
			if (oflag&0x30) {
				return -1;
			}
			FixFileName(NewName,path);
			path = NewName;
			oflag = (oflag&0xF)|((oflag&0x700)<<1);
		}
		return Real.open(path,oflag);
	}
	return -1;
}

/**********************************

	Intercept unlink and allow it to work under
	StdCLib under MacOSX
	
**********************************/

int unlink(const char *path)
{
	char NewName[2048];
	MacOSStdCLibCalls();		/* Make sure I'm loaded */
	if (Real.unlink) {			/* Valid? */
		if (Real.Version>=0xA00) {
			FixFileName(NewName,path);
			path = NewName;
		}
		return Real.unlink(path);
	}
	return -1;
}

/**********************************

	Intercept remove and allow it to work under
	StdCLib under MacOSX
	
**********************************/

int remove (const char *filename)
{
	char NewName[2048];
	MacOSStdCLibCalls();		/* Make sure I'm loaded */
	if (Real.remove) {			/* Valid? */
		if (Real.Version>=0xA00) {
			FixFileName(NewName,filename);
			filename = NewName;
		}
		return Real.remove(filename);
	}
	return -1;
}

/**********************************

	Intercept rename and allow it to work under
	StdCLib under MacOSX
	
**********************************/

int rename (const char *oldname, const char *newname)
{
	char NewName[2048];
	char OldName[2048];
	MacOSStdCLibCalls();		/* Make sure I'm loaded */
	if (Real.rename) {			/* Valid? */
		if (Real.Version>=0xA00) {
			FixFileName(NewName,newname);
			FixFileName(OldName,oldname);
			oldname = OldName;
			newname = NewName;
		}
		return Real.rename(oldname,newname);
	}
	return -1;
}

/**********************************

	Intercept fopen and allow it to work under
	StdCLib under MacOSX
	
**********************************/

FILE *fopen(const char *filename, const char *mode)
{
	char NewName[2048];
	MacOSStdCLibCalls();		/* Make sure I'm loaded */
	if (Real.fopen) {			/* Valid? */
		if (Real.Version>=0xA00) {
			FixFileName(NewName,filename);
			filename = NewName;
		}
		return Real.fopen(filename,mode);
	}
	return 0;
}

/**********************************

	Intercept freopen and allow it to work under
	StdCLib under MacOSX
	
**********************************/

FILE *freopen (const char *filename, const char *mode, FILE *stream)
{
	char NewName[2048];
	MacOSStdCLibCalls();		/* Make sure I'm loaded */
	if (Real.freopen) {			/* Valid? */
		if (Real.Version>=0xA00) {
			FixFileName(NewName,filename);
			filename = NewName;
		}
		return Real.freopen(filename,mode,stream);
	}
	return 0;
}

/**********************************

	Intercept setvbuf and allow it to work under
	StdCLib under MacOSX
	
**********************************/

int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
	MacOSStdCLibCalls();		/* Make sure I'm loaded */
	if (Real.setvbuf) {			/* Valid? */
		if (Real.Version>=0xA00) {
			if (mode&0x40) {	// _IOLBF
				mode = 1;		// MacOSX _IOLBF
			}
			if (mode&4) {		// _IONBF
				mode = 2;		// MacOSX _IONBF
			}
		}
		return Real.setvbuf(stream,buf,mode,size);
	}
	return 0;
}

#endif

#if TARGET_API_MAC_CARBON

/**********************************

	This is a MacOS compatibility layer to be used
	under Classic in Carbon
	
**********************************/

OSErr PBControlSync(ParmBlkPtr paramBlock)
{
	Version_t *Locals;
	LibRef_t *InterfaceLibRef;
	
	Locals = &VersionLocals;
	if (Locals->PBControlSync) {			/* Did I already get the function pointer? */
		return Locals->PBControlSync(paramBlock);		/* Call it */
	}
	
	InterfaceLibRef = MacOSGetInterfaceLib();			/* Load in the interface lib */
	if (InterfaceLibRef) {
		Locals->PBControlSync = (OSErr (*)(ParmBlkPtr))LibRefGetProc(InterfaceLibRef,"PBControlSync");
		if (Locals->PBControlSync) {
			return Locals->PBControlSync(paramBlock);
		}
	}
	return -1;
}

OSErr PBStatusSync(ParmBlkPtr paramBlock)
{
	Version_t *Locals;
	LibRef_t *InterfaceLibRef;
	
	Locals = &VersionLocals;
	if (Locals->PBStatusSync) {			/* Did I already get the function pointer? */
		return Locals->PBStatusSync(paramBlock);		/* Call it */
	}
	
	InterfaceLibRef = MacOSGetInterfaceLib();			/* Load in the interface lib */
	if (InterfaceLibRef) {
		Locals->PBStatusSync = (OSErr(*)(ParmBlkPtr))LibRefGetProc(InterfaceLibRef,"PBStatusSync");
		if (Locals->PBStatusSync) {
			return Locals->PBStatusSync(paramBlock);
		}
	}
	return -1;
}

OSErr Control(short refNum,short csCode,const void *csParamPtr)
{
	Version_t *Locals;
	LibRef_t *InterfaceLibRef;
	
	Locals = &VersionLocals;
	if (Locals->Control) {			/* Did I already get the function pointer? */
		return Locals->Control(refNum,csCode,csParamPtr);		/* Call it */
	}
	
	InterfaceLibRef = MacOSGetInterfaceLib();			/* Load in the interface lib */
	if (InterfaceLibRef) {
		Locals->Control = (OSErr(*)(short,short,const void *))LibRefGetProc(InterfaceLibRef,"Control");
		if (Locals->Control) {
			return Locals->Control(refNum,csCode,csParamPtr);
		}
	}
	return -1;
}

QHdrPtr GetDrvQHdr(void)
{
	Version_t *Locals;
	LibRef_t *InterfaceLibRef;
	
	Locals = &VersionLocals;
	if (Locals->GetDrvQHdr) {			/* Did I already get the function pointer? */
		return Locals->GetDrvQHdr();		/* Call it */
	}
	
	InterfaceLibRef = MacOSGetInterfaceLib();			/* Load in the interface lib */
	if (InterfaceLibRef) {
		Locals->GetDrvQHdr = (QHdrPtr(*)(void))LibRefGetProc(InterfaceLibRef,"GetDrvQHdr");
		if (Locals->GetDrvQHdr) {
			return Locals->GetDrvQHdr();
		}
	}
	return 0;
}

OSErr MacOpenDriver(ConstStr255Param name,short *drvrRefNum)
{
	Version_t *Locals;
	LibRef_t *InterfaceLibRef;
	
	Locals = &VersionLocals;
	if (Locals->MacOpenDriver) {			/* Did I already get the function pointer? */
		return Locals->MacOpenDriver(name,drvrRefNum);		/* Call it */
	}
	
	InterfaceLibRef = MacOSGetInterfaceLib();			/* Load in the interface lib */
	if (InterfaceLibRef) {
		Locals->MacOpenDriver = (OSErr(*)(ConstStr255Param,short *))LibRefGetProc(InterfaceLibRef,"OpenDriver");
		if (Locals->MacOpenDriver) {
			return Locals->MacOpenDriver(name,drvrRefNum);
		}
	}
	return -1;
}

OSErr Eject(ConstStr63Param volName,short vRefNum)
{
	Version_t *Locals;
	LibRef_t *InterfaceLibRef;
	
	Locals = &VersionLocals;
	if (Locals->Eject) {			/* Did I already get the function pointer? */
		return Locals->Eject(volName,vRefNum);		/* Call it */
	}
	
	InterfaceLibRef = MacOSGetInterfaceLib();			/* Load in the interface lib */
	if (InterfaceLibRef) {
		Locals->Eject = (OSErr(*)(ConstStr63Param,short))LibRefGetProc(InterfaceLibRef,"Eject");
		if (Locals->Eject) {
			return Locals->Eject(volName,vRefNum);
		}
	}
	return -1;
}

OSErr GetDriverInformation(DriverRefNum refNum,UnitNumber *unitNum,DriverFlags *flags,
	DriverOpenCount *count,StringPtr name,RegEntryID *device,CFragSystem7Locator *driverLoadLocation,
	CFragConnectionID *fragmentConnID,DriverEntryPointPtr *fragmentMain,DriverDescription *driverDesc)
{
	Version_t *Locals;
	LibRef_t *DriverLoaderRef;
	
	Locals = &VersionLocals;
	if (Locals->GetDriverInformation) {			/* Did I already get the function pointer? */
		return Locals->GetDriverInformation(refNum,unitNum,flags,count,
			name,device,driverLoadLocation,fragmentConnID,fragmentMain,driverDesc);		/* Call it */
	}
	
	DriverLoaderRef = MacOSDriverLoaderLib();			/* Load in the interface lib */
	if (DriverLoaderRef) {
		Locals->GetDriverInformation = (OSErr(*)(DriverRefNum,UnitNumber*,DriverFlags *,DriverOpenCount *,StringPtr,
			RegEntryID *,CFragSystem7Locator*,CFragConnectionID *,DriverEntryPointPtr *,DriverDescription *))LibRefGetProc(DriverLoaderRef,"GetDriverInformation");
		if (Locals->GetDriverInformation) {
			return Locals->GetDriverInformation(refNum,unitNum,flags,count,
				name,device,driverLoadLocation,fragmentConnID,fragmentMain,driverDesc);
		}
	}
	return -1;
}

/**********************************

	This is a MacOS compatibility layer to be used
	under OSX in Carbon
	
**********************************/

CGDisplayErr CGSetDisplayTransferByTable(CGDirectDisplayID display,CGTableCount tableSize,
	const CGGammaValue *redTable,const CGGammaValue *greenTable,const CGGammaValue *blueTable)
{
	Version_t *Locals;
	MacOSXFramework_t *FrameWorkRef;
	
	Locals = &VersionLocals;
	if (Locals->CGSetDisplayTransferByTable) {			/* Did I already get the function pointer? */
		return Locals->CGSetDisplayTransferByTable(display,tableSize,redTable,greenTable,blueTable);		/* Call it */
	}
	
	FrameWorkRef = MacOSGetFoundationFramework();			/* Load in the interface lib */
	if (FrameWorkRef) {
		Locals->CGSetDisplayTransferByTable = (CGDisplayErr(*)(CGDirectDisplayID,CGTableCount,const CGGammaValue *,const CGGammaValue *,const CGGammaValue *))MacOSXFrameworkGetProc(FrameWorkRef,"CGSetDisplayTransferByTable");
		if (Locals->CGSetDisplayTransferByTable) {
			return Locals->CGSetDisplayTransferByTable(display,tableSize,redTable,greenTable,blueTable);
		}
	}
	return static_cast<CGDisplayErr>(-1);
}

CGDisplayErr CGGetDisplayTransferByTable(CGDirectDisplayID display,CGTableCount capacity,
	CGGammaValue *redTable,CGGammaValue *greenTable,CGGammaValue *blueTable,CGTableCount *sampleCount)
{
	Version_t *Locals;
	MacOSXFramework_t *FrameWorkRef;
	
	Locals = &VersionLocals;
	if (Locals->CGGetDisplayTransferByTable) {			/* Did I already get the function pointer? */
		return Locals->CGGetDisplayTransferByTable(display,capacity,redTable,greenTable,blueTable,sampleCount);		/* Call it */
	}
	
	FrameWorkRef = MacOSGetFoundationFramework();			/* Load in the interface lib */
	if (FrameWorkRef) {
		Locals->CGGetDisplayTransferByTable = (CGDisplayErr(*)(CGDirectDisplayID,CGTableCount,CGGammaValue *,CGGammaValue *,CGGammaValue *,CGTableCount *))MacOSXFrameworkGetProc(FrameWorkRef,"CGGetDisplayTransferByTable");
		if (Locals->CGGetDisplayTransferByTable) {
			return Locals->CGGetDisplayTransferByTable(display,capacity,redTable,greenTable,blueTable,sampleCount);
		}
	}
	return static_cast<CGError>(-1);
}


#endif

#endif
