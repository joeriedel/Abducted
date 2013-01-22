/**********************************

	MacOS version

**********************************/

#include "FmFile.h"

#if defined(__MAC__)
#include "MmMemory.h"
#include "ClStdLib.h"
#include "McMac.h"
#include "StString.h"
#include <stdlib.h>
#include <Files.h>
#include <Script.h>
#include <Resources.h>
#include <Processes.h>
#include <Folders.h>

#define DIRCACHESIZE 8			/* Number of cache entries */
typedef struct ExpandCache_t {
	void **NameHandle;			/* Handle to the original directory name */
	Word32 HitTick;			/* Last time hit (For purging) */ 
	long DirID;					/* Directory ID */
	Word NameLength;
	short VRefNum;				/* Volume reference number */
	short padding;				/* Crap for PPC padding */
} ExpandCache_t;

static ExpandCache_t DirCache[DIRCACHESIZE];
static Word ExpandClean;

/**********************************

	MacOS X version (Used for StdCLib)
	I am in hell.
	
	I need to see if I want the BOOT volume. If it is not,
	then I prepend a FAKE name of /Volumes to the name

**********************************/

#if TARGET_API_MAC_CARBON && !defined(__MSL__)

typedef struct BootLocals_t {
	Word16 MacOSXBootNameSize;	/* Length of the boot volume name */
	Word8 GotBootName;			/* TRUE if I tested for the boot volume name */
	Word8 Padding;
	char MacOSXBootName[256];	/* Boot volume name in the format ":FooBar" (Not zero terminated) */
} BootLocals_t;

static BootLocals_t ExLocals;

static void BURGERCALL MacOSXExpandAPathToBuffer(char *Output2,const char *FileName)
{
	char PathName[FULLPATHSIZE];
	char OutName[FULLPATHSIZE];
	char *Output;
	char *FilePtr;		/* Pointer to input buffer */
	Word8 Temp;
	
	ExpandAPathToBuffer(PathName,FileName);		/* Resolve prefixes */
	Output = OutName;
	
	/* Now, is this a fully qualified name? */
	
	if (((Word8 *)PathName)[0]==':') {		/* First char is ':' for a qualified pathname */
	
		/* Look for the volume name */
		
		FilePtr = strchr(PathName+1,':');
		if (!FilePtr) {
			FilePtr = PathName;		/* Odd, not a full pathname. */
		} else {
			BootLocals_t *LocalPtr;
		
			LocalPtr = &ExLocals;
			
			/* Did I get the boot volume name yet? */
			/* I need to ask the OS what is the name of the /boot volume */
			
			if (!LocalPtr->GotBootName) {				/* Once I have it, it doesn't change */
				char *NameTemp;
				LocalPtr->GotBootName = TRUE;
				NameTemp = GetAVolumeName(0);	/* Get the name of the boot volume */
				if (NameTemp) {
					if (((Word8 *)NameTemp)[0]) {	/* Valid name */
						Word Max;
						Max = strlen(NameTemp)-1;
						if (Max>sizeof(LocalPtr->MacOSXBootName)) {		/* Failsafe */
							Max = sizeof(LocalPtr->MacOSXBootName);
						}
						LocalPtr->MacOSXBootNameSize = Max;
						memcpy(LocalPtr->MacOSXBootName,NameTemp,Max);
					}
					DeallocAPointer(NameTemp);		/* Release the GetAVolumeName() pointer */
				}
			}
			
			/* Is this on the boot volume? */
			/* Also test for the special case of :Foo vs :FooBar */
			
			{
				Word Index;
				Index = LocalPtr->MacOSXBootNameSize;

				/* Test for boot name match */
				
				if (memicmp(LocalPtr->MacOSXBootName,PathName,Index) ||
				
					/* Test for :Foo matching :FooBar (The next char should be ':' or 0) */
					
					(PathName[Index]!=':' && PathName[Index])) {
					
					strcpy(Output,"/Volumes");		/* Look in the mounted volumes folder */
					Output+=8;						/* At the end of /Volumes */
					FilePtr = PathName;				/* Append the full name */
				}
			}
		}
	} else {
		FilePtr = PathName;		/* Odd, not a full pathname. */
	}
	
	/* Convert the rest of the path */
	/* Colons to slashes */
	
	Temp = ((Word8 *)FilePtr)[0];
	if (Temp) {
		do {
			++FilePtr;
			if (Temp==':') {
				Temp = '/';		/* Unix style */
			}
			
			Output[0] = Temp;
			++Output;
			Temp = ((Word8 *)FilePtr)[0];
		} while (Temp);
		Output-=2;
		
		/* A trailing slash assumes more to follow, get rid of it */
		
		if (Output[0]!='/') {
			++Output;		/* Remove trailing slash */
		}
	}
	Output[0] = 0;			/* Terminate the "C" string */

	/* Now, convert the filename into UTF8 encoded ASCII */
	/* I use MacRoman 8 internally, but OSX / StdCLib uses UTF8 */
	
	{
		CFStringRef StringRef;
		StringRef = CFStringCreateWithCString(0,OutName,kCFStringEncodingMacRoman);	/* Convert to a string ref */
		CFStringGetCString(StringRef,Output2,FULLPATHSIZE,kCFStringEncodingUTF8);	/* Perform the conversion */
		CFRelease(StringRef);														/* Dispose of the string ref */
	}
}
#endif

/**********************************

	Dispose of my cache on system exit

**********************************/

void BURGERCALL MacOSPurgeDirCache(void)
{
	Word i;
	ExpandCache_t *CachePtr;
	ExpandClean = 2;			/* I am shutting down */
	i = DIRCACHESIZE;
	CachePtr = DirCache;
	do {
		DeallocAHandle(CachePtr->NameHandle);
		CachePtr->NameHandle = 0;
		++CachePtr;
	} while (--i);
}

/**********************************

	This routine will get the time and date
	from a file.
	Note, this routine is Operating system specfic!!!

**********************************/

Word BURGERCALL GetFileModTimeNative(const char *FileName,TimeDate_t *Output)
{
	HParamBlockRec Temp;		/* MacOS file parameter block */
	char Foo[256];				/* Pascal string version */

	CStr2PStr(Foo,FileName);		/* Convert to a PASCAL string */
	FastMemSet(&Temp,0,sizeof(Temp));	/* Init the struct */
	Temp.fileParam.ioVRefNum = MacVRefNum;	/* Pass the volume reference */
	Temp.fileParam.ioDirID = MacDirID;		/* Pass the directory reference */
	Temp.fileParam.ioNamePtr = (StringPtr)&Foo[0];	/* Get the filename (PASCAL) */

	if (!PBHGetFInfoSync(&Temp)) {		/* Get the file info */
		MacOSFileSecondsToTimeDate(Output,Temp.fileParam.ioFlMdDat);
		return FALSE;
	}
	FastMemSet(Output,0,sizeof(TimeDate_t));	/* Zap it */
	return TRUE;
}

/**********************************

	This routine will get the time and date
	from a file.
	Note, this routine is Operating system specfic!!!

**********************************/

Word BURGERCALL GetFileCreateTimeNative(const char *FileName,TimeDate_t *Output)
{
	HParamBlockRec Temp;
	char Foo[256];

	CStr2PStr(Foo,FileName);		/* Convert to a PASCAL string */
	FastMemSet(&Temp,0,sizeof(Temp));	/* Init the struct */
	Temp.fileParam.ioVRefNum = MacVRefNum;	/* Pass the volume reference */
	Temp.fileParam.ioDirID = MacDirID;		/* Pass the directory reference */
	Temp.fileParam.ioNamePtr = (StringPtr)&Foo[0];	/* Get the filename (PASCAL) */

	if (!PBHGetFInfoSync(&Temp)) {		/* Get the file info */
		MacOSFileSecondsToTimeDate(Output,Temp.fileParam.ioFlCrDat);	/* Convert to date */
		return FALSE;
	}
	FastMemSet(Output,0,sizeof(TimeDate_t));		/* Zap it */
	return TRUE;
}

/**********************************

	Determine if a file exists.
	I will return TRUE if the specified path
	is a path to a file that exists, if it doesn't exist
	or it's a directory, I return FALSE.
	Note : I do not check if the file havs any data in it.
	Just the existance of the file.

**********************************/

Word BURGERCALL DoesFileExistNative(const char *FileName)
{
	HParamBlockRec Temp;
	char Foo[256];

	CStr2PStr(Foo,FileName);		/* Convert to a PASCAL string */
	FastMemSet(&Temp,0,sizeof(Temp));	/* Init the struct */
	Temp.fileParam.ioVRefNum = MacVRefNum;	/* Pass the volume reference */
	Temp.fileParam.ioDirID = MacDirID;		/* Pass the directory reference */
	Temp.fileParam.ioNamePtr = (StringPtr)&Foo[0];	/* Get the filename (PASCAL) */

	if (!PBHGetFInfoSync(&Temp)) {		/* Get the file info */
		return TRUE;
	}
	return FALSE;		/* Failure */
}

/**********************************

	Open a directory for scanning
	Return an error if the directory doesn't exist

**********************************/

Word BURGERCALL OpenADirectory(DirectorySearch_t *Input,const char *Name)
{
	char PathName[FULLPATHSIZE];
	CInfoPBRec MyRec;
	Word8 Bogus[256];

	ExpandAPathToBufferNative(PathName,Name);	/* Convert to native path */
	Input->VRefNum = MacVRefNum;			/* Get the mac volume */
	Input->DirID = MacDirID;				/* Get the mac directory ID */
	CStr2PStr((char *)&Bogus[0],PathName);	/* Convert to pascal */

	MyRec.dirInfo.ioCompletion = 0;			/* Set up for a check */
	MyRec.dirInfo.ioNamePtr = &Bogus[0];
	MyRec.dirInfo.ioVRefNum = Input->VRefNum;
	MyRec.dirInfo.ioDrDirID = Input->DirID;
	MyRec.dirInfo.ioFDirIndex = 0;
	if (!PBGetCatInfoSync(&MyRec)) {			/* Check the directory */
		if (MyRec.dirInfo.ioFlAttrib&0x10) {	/* Is it a directory? */
			Input->DirID = MyRec.dirInfo.ioDrDirID;	/* Save the true ID */
			Input->Index = 1;					/* Init the file index */
			return FALSE;					/* I'm good to go! */
		}
	}
	return TRUE;		/* Oh drat */
}

/**********************************

	Get the next directory entry
	Return FALSE if the entry is valid, TRUE if
	an error occurs.

**********************************/

Word BURGERCALL GetADirectoryEntry(DirectorySearch_t *Input)
{
	CInfoPBRec MyRec;
	Word8 Bogus[256];		/* Temp name buffer */

	MyRec.dirInfo.ioCompletion = 0;
	MyRec.dirInfo.ioNamePtr = Bogus;
	MyRec.dirInfo.ioVRefNum = Input->VRefNum;	/* Pass the open directory */
	MyRec.dirInfo.ioDrDirID = Input->DirID;
	MyRec.dirInfo.ioFDirIndex = Input->Index;
	if (!PBGetCatInfoSync(&MyRec)) {		/* Get the entry */
		Input->Dir = FALSE;
		Input->Locked = FALSE;
		Input->System = FALSE;
		Input->Hidden = FALSE;
		if (MyRec.dirInfo.ioFlAttrib&0x10) {	/* Is this a directory */
			Input->Dir = TRUE;
			Input->FileSize = 0;
			Input->FileType = 0;
			Input->AuxType = 0;
		} else {
			Input->Dir = FALSE;
			Input->FileSize = MyRec.hFileInfo.ioFlLgLen;
			Input->FileType = MyRec.hFileInfo.ioFlFndrInfo.fdType;
			Input->AuxType = MyRec.hFileInfo.ioFlFndrInfo.fdCreator;
		}
		if (MyRec.dirInfo.ioFlAttrib&0x01) {
			Input->Locked = TRUE;
		}
//		if (MyRec.dirInfo.ioACUser & 0x03) {	/* Can't see folder/file */
//			Input->Hidden = TRUE;
//		}
		MacOSFileSecondsToTimeDate(&Input->Create,MyRec.hFileInfo.ioFlCrDat);
		MacOSFileSecondsToTimeDate(&Input->Modify,MyRec.hFileInfo.ioFlMdDat);
		++Input->Index;			/* Next index */
		PStr2CStr(&Input->Name[0],(char *)&Bogus[0]);	/* Copy the filename */
		return FALSE;
	}
	return TRUE;			/* Got it! */
}

/**********************************

	Close a directory that's being scanned

**********************************/

void BURGERCALL CloseADirectory(DirectorySearch_t *Input)
{
}

/**********************************

	Open a file using a native path

**********************************/

FILE * BURGERCALL OpenAFile(const char *FileName,const char *Type)
{
	Word8 NameBuffer[1];		/* Buffer for blank name */
	char NewFileName[FULLPATHSIZE];		/* New filename from ExpandAPathToBufferNative */
	
#if TARGET_API_MAC_CARBON && !defined(__MSL__)
	if (MacOSGetOSVersion()>=0xA00) {
		MacOSXExpandAPathToBuffer(NewFileName,FileName);
		return fopen(NewFileName,Type);		/* Do it the MacOSX way */
	}
#endif

	/* Get the FSSpec of the file */
	
	ExpandAPathToBufferNative(NewFileName,FileName);	/* Expand a filename */
	if (MacVRefNum!=MacCacheVRefNum || MacDirID!=MacCacheDirID) {
		NameBuffer[0] = 0;
		if (!HSetVol(&NameBuffer[0],MacVRefNum,MacDirID)) {		/* Set the directory for fopen */
			MacCacheVRefNum = MacVRefNum;
			MacCacheDirID = MacDirID;
		}
	}

	/* Don't use the leading colon */
	FileName = NewFileName;
	if (((Word8 *)FileName)[0]==':') {
		++FileName;
	}
	return fopen(FileName,Type);	/* Open using standard fopen */
}

/**********************************

	Copy the contents of a fork
	Buffer is assumed to be 64K

**********************************/

static Word CopyFork(short f1,short f2,Word8 *buffer)
{
	Word32 Length;
	long LenMem;
	
	Length = 0;
	if (!GetEOF(f1,&LenMem)) {	/* Get the size of the source file */
		Length = LenMem;
	}
	if (Length) {				/* Shall I copy anything? */
		do {
			Word32 Chunk;
			long l;
			Chunk = Length;		/* Base chunk */
			if (Chunk>65536) {
				Chunk = 65536;		/* Only copy the chunk */
			}
			l = Chunk;
			if (FSRead(f1,&l,buffer)) {		/* Read data */
				break;
			}
			if (l!=Chunk) {
				break;
			}
			if (FSWrite(f2,&l,buffer)) {		/* Write data */
				break;
			}
			if (l!=Chunk) {
				break;
			}
			Length -= Chunk;
		} while (Length);			/* Any data left? */
	}
	if (!Length) {					/* All data copied? */
		return FALSE;				/* No error (So far) */
	}
	return TRUE;					/* Failure */
}

/**********************************

	Copy a file using native pathnames

**********************************/

Word BURGERCALL CopyAFileNative(const char *DestName,const char *SourceName)
{
	FSSpec SrcSpec;
	FSSpec DestSpec;
	short f1,f2;
	Word8 *buffer;
	Word Result;
	Str255 WorkName;
	OSErr err;
	
	Result = TRUE;			/* Assume error */
	buffer = (Word8 *)AllocAPointer(65536);
	if (buffer) {
	
		/* Make the source FSSpec */
		
		CStr2PStr((char *)WorkName,SourceName);
		if (!FSMakeFSSpec(MacVRefNum,MacDirID,WorkName,&SrcSpec)) {
		
		/* Make the dest FSSpec */
		
			CStr2PStr((char *)WorkName,DestName);
			err = FSMakeFSSpec(MacVRefNum2,MacDirID2,WorkName,&DestSpec);
			if (!err || err==fnfErr) {			/* File not found is ok */
			
			/* Copy the data fork */
			
				if (!FSpOpenDF(&SrcSpec,fsRdPerm,&f1)) {
					FSpCreate(&DestSpec,'????','BINA',smSystemScript);
					if (!FSpOpenDF(&DestSpec,fsWrPerm,&f2)) {
						if (!CopyFork(f1,f2,buffer)) {
							Result = FALSE;
						}
						if (FSClose(f2)) {
							Result = TRUE;
						}
					}
					FSClose(f1);
				}
				
			/* If ok, then copy the resource fork */
			
				if (!Result) {
					if (!FSpOpenRF(&SrcSpec,fsRdPerm,&f1)) {
						if (!FSpOpenRF(&DestSpec,fsWrPerm,&f2)) {
							if (!CopyFork(f1,f2,buffer)) {
								Result = FALSE;
							}
							if (FSClose(f2)) {
								Result = TRUE;
							}
						}
						FSClose(f1);
					}
					
			/* If still ok, then copy the finder's data */
			
					if (!Result) {
						FInfo Info;
						Result = TRUE;
						if (!FSpGetFInfo(&SrcSpec,&Info)) {
							if (!FSpSetFInfo(&DestSpec,&Info)) {
								Result = FALSE;
							}
						}
					}
				}
			}
		}
		DeallocAPointer(buffer);		/* Release the copy buffer */
	}
	return Result;			/* Return the end result */
}

/**********************************

	Delete a file using native file system

**********************************/

Word BURGERCALL DeleteAFileNative(const char *FileName)
{
	Word8 NameBuffer[256];		/* Pascal string buffer */

	CStr2PStr((char *)NameBuffer,FileName);		/* Convert string to pascal type */
	return HDelete(MacVRefNum,MacDirID,NameBuffer);		/* Delete the file */
}

/**********************************

	Rename a file using native pathnames

**********************************/

Word BURGERCALL RenameAFileNative(const char *DestName,const char *SourceName)
{
	FSSpec SrcSpec;
	Word Result;
	Str255 WorkName;
	
	Result = TRUE;			/* Assume error */
	
	/* Make the source FSSpec */
	
	CStr2PStr((char *)WorkName,SourceName);
	if (!FSMakeFSSpec(MacVRefNum,MacDirID,WorkName,&SrcSpec)) {
		CStr2PStr((char *)WorkName,DestName);
		if (!FSpRename(&SrcSpec,WorkName)) {
			Result = FALSE;
		}
	}
	return Result;			/* Return the end result */
}

/**********************************

	Create a directory path using an operating system native name
	Return FALSE if successful, or TRUE if an error

**********************************/

Word BURGERCALL CreateDirectoryPathNative(const char *FileName)
{
	OSErr Err;			/* Possible error code */
	Word8 PName[256];	/* Pascal string */
	long NewDir;		/* Returned value */

	MacOSPurgeDirCache();		/* Kill off the cache since the directories may have changed */
	CStr2PStr((char *)PName,FileName);		/* Create the MacOS version */
	Err = DirCreate(MacVRefNum,MacDirID,PName,&NewDir);		/* Easy way! */
	if (!Err || Err== dupFNErr) {	/* Cool!! */
		return FALSE;				/* No error */
	}
	if (FileName[0]) {				/* Is there a filename? */
		const char *WorkPtr;
		WorkPtr = FileName;			/* Index past the first colon */
		if (((Word8 *)WorkPtr)[0]==':') {
			++WorkPtr;				/* Skip past the initial colon */
		}
		do {
			WorkPtr = strchr(WorkPtr,':');	/* Skip to the next colon */
			if (!WorkPtr) {		/* No colon found? */
				WorkPtr = strchr(FileName,0)-1;
			}
			++WorkPtr;				/* Accept the colon */
			PName[0] = (Word8)(WorkPtr-FileName);	/* Get the string length */
			Err = DirCreate(MacVRefNum,MacDirID,PName,&NewDir);		/* Easy way! */
		} while (((Word8 *)WorkPtr)[0]);			/* Still more string? */
		if (!Err || Err== dupFNErr) {	/* Cool!! */
			return FALSE;				/* No error */
		}
	}
	/* Ok see if I can create the directory tree */
	return Err;		/* Didn't do it! */
}

/**********************************

	This routine will take a generic path, expand it fully
	and then set the current working directory to the highest
	allowable directory I can attain without error. This is so that
	I can get around the limitation of 256 character pathnames on some
	operating systems (MacOS for 255 PStrings, DOS for 264 total pathname length)

**********************************/

void BURGERCALL ExpandAPathToBufferNative(char *Output,const char *FileNamex)
{
	char PathName[FULLPATHSIZE];
	Word Length;		/* Length of the final string */
	char *FilePtr;		/* Pointer to input buffer */
	char *WorkPtr;
	OSErr MyError;
	Word DirLength;		/* Length of the directory in bytes */
	ExpandCache_t *Biggest;

	MacVRefNum = 0;		/* Init the Macintosh volume numbers */
	MacDirID = 0;

	if (!ExpandClean) {
		ExpandClean = TRUE;
		atexit(MacOSPurgeDirCache);
	}
	ExpandAPathToBuffer(PathName,FileNamex);		/* Resolve prefixes */

	/* Since the MacOS is so slow in creating FSSpecs on 603 machines */
	/* I have to use a directory cache to see if have already determined */
	/* the volume ID and dir ref */
	
	/* Note, If I have a perfect match, then exit now, else */
	/* Set up myself for the partial match */
	
	DirLength = 0;			/* Assume the directory has no length */
	WorkPtr = strrchr(PathName,':');		/* Is there an ending colon? */
	Biggest = 0;
	if (WorkPtr) {
		WorkPtr[0] = 0;		/* End the string here */
		FilePtr = strrchr(PathName,':');		/* Colon before that? */
		WorkPtr[0] = ':';	/* Restore the string */
		if (FilePtr) {		/* Second colon? */
			ExpandCache_t *CachePtr;
			Word BiggestLen;
			Word i;
			
			DirLength = (FilePtr-PathName)+1;		/* Length of the string */
			if (DirLength>=2) {					/* Not a single char? */
				i = DIRCACHESIZE;
				CachePtr = DirCache;
				BiggestLen = 0;
				do {
					if (CachePtr->NameHandle && 		/* Should I bother checking? */
						CachePtr->NameLength<=DirLength) {
						if (!memicmp(static_cast<char *>(CachePtr->NameHandle[0]),PathName,CachePtr->NameLength)) {
							if (CachePtr->NameLength==DirLength) {	/* Perfect match? */
								MacVRefNum = CachePtr->VRefNum;		/* Get the cache */
								MacDirID = CachePtr->DirID;
								CachePtr->HitTick = ReadTick();
								strcpy(Output,PathName+DirLength);	/* Include a leading colon */
								DirLength = strlen(Output);		/* Length of the result */
								if (DirLength) {
									Output[DirLength-1] = 0;		/* Kill the trailing colon */
								}
								return;
							}
							if (CachePtr->NameLength>=BiggestLen) {
								Biggest = CachePtr;
								BiggestLen = CachePtr->NameLength;
							}
						}
					}
					++CachePtr;
				} while (--i);
			}
		}
	}
	
	/* First parse either the volume name or a .DXX device number */
	/* I either return a fully qualified volume name or I get the proper */
	/* MacVRefNum of the volume number to work with */

	/* Is this a volume name? */

	Length = 0;			/* Init length */
	WorkPtr = Output;				/* Save dest buffer */
	FilePtr = (char *)PathName;		/* Copy to running pointer */
	if (Biggest) {
		MacVRefNum = Biggest->VRefNum;		/* Get the cache */
		MacDirID = Biggest->DirID;
		FilePtr += Biggest->NameLength-1;	/* Point to the colon */
		Biggest->HitTick = ReadTick();
	} else {
		if (((Word8 *)FilePtr)[0] == ':') {	/* Fully qualified pathname? */
			FSSpec MyFileInfo;		/* I convert the volume name to device number by getting an FSSpec */
			do {
				++Length;
				if (((Word8 *)FilePtr)[Length]==':') {
					if (((Word8*)FilePtr)[Length+1]) {		/* End now? Just a volume name? */
						goto ParseDir;
					}
					break;
				}			
			} while (((Word8 *)FilePtr)[Length]);	/* Parse out the volume name */
			FilePtr[0] = Length;	/* Convert to a "P" String and kill the ':' */
			FSMakeFSSpec(0,0,(Word8 *)FilePtr,&MyFileInfo);	/* Get the volume number */
			MacVRefNum = MyFileInfo.vRefNum;	/* Save it */
			MacDirID = 2;			/* Root directories are always #2 */
			FilePtr[0] = ':';		/* Restore the colon */
			DirLength = 0;	/* Don't save */
			Length = 0;
			goto EndNow2;	/* Exit now */

ParseDir:;
			FilePtr[0] = Length;	/* Convert to a "P" String and kill the ':' */
			FSMakeFSSpec(0,0,(Word8 *)FilePtr,&MyFileInfo);	/* Get the volume number */
			MacVRefNum = MyFileInfo.vRefNum;	/* Save it */
			MacDirID = 2;			/* Root directories are always #2 */
			FilePtr[0] = ':';		/* Restore the colon */
			FilePtr = FilePtr+Length;		/* I will point to the next colon */

		/* Is this a device number? */

		} else if (((Word8 *)FilePtr)[0]=='.') {		/* Device number? */
			Word Temp;
			Temp = ((Word8 *)FilePtr)[1];			/* Get the second char */
			if (Temp>='a' && Temp<('z'+1)) {
				Temp &= 0xDF;
			}
			if (Temp=='D') {			/* Is it a 'D'? */
				Word DeviceNum;
				Length = 2;		/* Init numeric index */
				DeviceNum = 0;	/* Init drive number */
				for (;;) {
					Temp = ((Word8 *)FilePtr)[Length];	/* Get an ASCII char */
					if (Temp==':') {		/* Proper end of string? */
						char *FullPathPtr;
						DeviceNum = -1-DeviceNum;
						MacDirID = 0;
						MacVRefNum = DeviceNum;
						if (((Word8 *)FilePtr)[Length+1]) {		/* Is there more? */
							break;		/* Accept input and go to part 2 */
						}

						/* Just a device name? Parse the volume name and convert to fully */
						/* qualified pathname */

						FullPathPtr = static_cast<char *>(GetFullPathFromMacID(0,DeviceNum));	/* Get the volume name */
						if (FullPathPtr) {
							Word NewLength;
							NewLength = strlen(FullPathPtr);	/* Length MUST be set!! */
							if (NewLength>=2) {
								FastMemCpy(WorkPtr,FullPathPtr+1,NewLength-1);
								WorkPtr = WorkPtr+NewLength;
							}
							DeallocAPointer(FullPathPtr);
							DirLength = Length+1;
							goto EndNow2;				/* Copy the result and exit */
						}
						Output[0] = 0;
						return;
					}
					if (Temp<'0' || Temp>='9'+1) {	/* Numeric value? */
						Length = 0;		/* Do not accept ANY input! */
						break;			/* Go to phase 2 */
					}
					++Length;		/* Accept the char */
					Temp = Temp-'0';	/* Convert to bin */
					DeviceNum = DeviceNum*10;		/* Adjust previous value */
					DeviceNum = DeviceNum+Temp;		/* Make full decimal result */
				}									/* Loop until done */
				FilePtr = FilePtr+Length;		/* Discard accepted input */
			}
		}
	}
	/* Now follow the directory chain for all the sub-directories */
	/* MacVRefNum may contain a volume number already */

	if (((Word8 *)FilePtr)[0]) {		/* Sanity check */
		CInfoPBRec Foo;
		for (;;) {
			Length = 0;
			do {
				++Length;			/* Parse to the ending colon */
				if (((Word8 *)FilePtr)[Length]==':') {
					if (((Word8 *)FilePtr)[Length+1]) {		/* End of string? */
						goto ParseIt;			/* Keep the leading colon for a qualified pathname */
					}
					break;
				}
			} while (((Word8 *)FilePtr)[Length]);
			goto EndNowXX;	
ParseIt:;
			FastMemCpy(WorkPtr+1,FilePtr,Length);	/* Copy the string */
			WorkPtr[0] = Length;		/* Save the length byte */
			Foo.dirInfo.ioNamePtr = (Word8 *)WorkPtr;	/* Set up the DirInfo record */
			Foo.dirInfo.ioVRefNum = MacVRefNum;
			Foo.dirInfo.ioFDirIndex = 0;
			Foo.dirInfo.ioDrDirID = MacDirID;
			MyError = PBGetCatInfoSync(&Foo);		/* Get the directory ID number */
			if (!MyError) {					/* Ok? */
				FilePtr = FilePtr+Length;			/* Remove the parsed string */
				Length = 0;
				MacVRefNum = Foo.dirInfo.ioVRefNum;	/* Follow the links */
				MacDirID = Foo.dirInfo.ioDrDirID;
				Biggest = 0;				/* Force a path to be created */
				if (!((Word8 *)FilePtr)[0]) {
					break;
				}
				continue;
			}
			Length = strlen(FilePtr);	/* Reset length */
			break;		/* Exit now! */
		}
	}
EndNowXX:;
	DirLength = (FilePtr-PathName)+1;
EndNow:;
	if (Length) {
		FastMemCpy(WorkPtr,FilePtr,Length);
		WorkPtr = WorkPtr+Length;
	}
EndNow2:;
	WorkPtr[0] = 0;			/* Terminate the "C" string */
	if (DirLength && !Biggest && ExpandClean!=2) {
		ExpandCache_t *CachePtr;
		ExpandCache_t *SavePtr;
		Word i;
		Word32 Mark,Time;

		Mark = ReadTick();
		i = DIRCACHESIZE;
		CachePtr = DirCache;
		SavePtr = CachePtr;
		Time = 0;
		do {
			if (!CachePtr->NameHandle) {		/* Empty? */
				SavePtr = CachePtr;
				break;
			}
			if ((Mark-CachePtr->HitTick)>Time) {
				SavePtr = CachePtr;
				Time = Mark-CachePtr->HitTick;
			}
			++CachePtr;
		} while (--i);
		SavePtr->VRefNum = MacVRefNum;
		SavePtr->DirID = MacDirID;
		SavePtr->HitTick = Mark;
		if (PathName[DirLength-1]!=':') {
			++DirLength;
			PathName[DirLength] = ':';
		}
		SavePtr->NameLength = DirLength;		/* Save the strlen size */
		PathName[DirLength] = 0;				/* Make sure it's zero terminated */
		DeallocAHandle(SavePtr->NameHandle);
		SavePtr->NameHandle = StrCopyHandle(PathName);
	}
}

/**********************************

	Change a directory using long filenames
	This only accepts Native OS filenames

**********************************/

Word BURGERCALL ChangeADirectory(const char *DirName)
{
	Word8 NameBuffer[256];

	CStr2PStr((char *)NameBuffer,DirName);
	if (!HSetVol(NameBuffer,MacVRefNum,MacDirID)) {
		MacCacheVRefNum = 0;
		MacCacheDirID = 0;
		return FALSE;
	}
	return (Word)-1;	/* Error! */
}

/**********************************

	Given an open directory, read in the next filename
	Return TRUE when the entries dry up.

**********************************/

void BURGERCALL MacOSFileSecondsToTimeDate(TimeDate_t *Output,Word32 Time)
{
	DateTimeRec Temp2;
	SecondsToDate(Time,&Temp2);		/* Convert to date */
	Output->Milliseconds = 0;		/* MacOS doesn't use milliseconds */
	Output->Second = (Word8)Temp2.second;	/* Get the seconds */
	Output->Minute = (Word8)Temp2.minute;	/* Get the minute */
	Output->Hour = (Word8)Temp2.hour;		/* Get the hour */
	Output->Day = (Word8)Temp2.day;			/* Get the day */
	Output->Month = (Word8)Temp2.month;		/* Get the month */
	Output->Year = (Word16)Temp2.year;		/* Get the year */
	Output->DayOfWeek = (Word8)Temp2.dayOfWeek-1;	/* Day of the week */
}

/**********************************

	Using a Macintosh directory ID and a volume referance number,
	return the full path that the ID generates.
	I need to use the current values to get the current
	directory name, then by traversing the directories PARENT entry, I can
	follow the tree BACKWARDS back to the root. So I must constantly be 
	prefixing my current data with the newly located PARENT entry until
	I get to the root entry.

	The Mac is brain dead.

**********************************/

void * BURGERCALL GetFullPathFromMacID(long dirID,short vRefNum) 
{
	CInfoPBRec FileRecord;	/* File record for passing to MacOS */
	void **TempHand;		/* Handle to first directory entry */
	char *TempPtr;			/* Pointer to first directory entry */
	char *TempPtr2;			/* Pointer to second directory entry */
	Word Length;			/* Length of final string */

	if (!ExpandClean) {
		ExpandClean = TRUE;
		atexit(MacOSPurgeDirCache);
	}
	TempHand = AllocAHandle(8192*2);		/* Allocate the twin buffers */
	if (!TempHand) {
		return 0;			/* Leave now */
	}
	TempPtr = static_cast<char *>(LockAHandle(TempHand));	/* Lock the twin buffers */
	TempPtr2 = &TempPtr[8192];			/* Point to the second buffer */ 

	FileRecord.dirInfo.ioVRefNum = vRefNum;	/* Get the magic mac value */
	FileRecord.dirInfo.ioDrParID = dirID;	/* Second magic mac value */
	FileRecord.dirInfo.ioFDirIndex = -1;	/* Query dir in ioDrDirID */

	TempPtr[0] = ':';
	TempPtr[1] = 0;
	do {
		/* Get the previous directory */
		FileRecord.dirInfo.ioDrDirID = FileRecord.dirInfo.ioDrParID;
		FileRecord.dirInfo.ioNamePtr = (StringPtr) TempPtr2;	/* Get dest pointer */
		if (PBGetCatInfoSync(&FileRecord)) {	/* Read directory info sync */
			DeallocAHandle(TempHand);			/* Kill the buffer */
			NonFatal("Error in PBGetCatInfo\n");
			return 0;			/* Sorry! */
		}
		Length = TempPtr2[0];	/* Get the string length */
		TempPtr2[0] = ':';		/* Replace the length with a path colon */
		strcpy(&TempPtr2[Length+1],TempPtr);	/* Concatinate the previous string */
		{
			char *TempX;
			TempX = TempPtr;		/* Swap the directory pointers */
			TempPtr = TempPtr2;
			TempPtr2 = TempX;
		}
	} while (FileRecord.dirInfo.ioDrDirID != fsRtDirID);	/* Hit the root already? */
	Length = strlen(TempPtr);			/* Get the length of the finished path */
	TempPtr2 = static_cast<char *>(AllocAPointer(Length+1));	/* Get buffer for final product */
	if (TempPtr2) {			/* Did I get the buffer? */
		FastMemCpy(TempPtr2,TempPtr,Length+1);	/* Copy the memory */
	}
	DeallocAHandle(TempHand);	/* Release temp buffers */
	return TempPtr2;			/* Return the buffer or NULL */
}

/**********************************
	
	Given a Macintosh FSSpec, return
	a Burgerlib Path
	
	This is a convience routine
	
**********************************/

char *BURGERCALL GetFullPathFromMacFSSpec(const FSSpec *Input)
{
	Word Len;
	Word Len2;
	char *Name;
	char *Result;
	
	/* First get the path from the parent directory and volume reference */
	/* number */
	
	Name = static_cast<char *>(GetFullPathFromMacID(Input->parID,Input->vRefNum));
	if (Name) {		/* Ok? */
		Len2 = Input->name[0];		/* Is there a filename? */
		if (Len2) {					/* Yep */
			Len = strlen(Name);		/* What's the length of the prefix */
			Result = (char *)AllocAPointer(Len+Len2+1);	/* Append the filename */
			if (Result) {			/* Memory ok? */
				strcpy(Result,Name);	/* Copy the prefix */
				FastMemCpy(Result+Len,(char*)&Input->name[1],Len2);	/* Append the filename */
				Result[Len+Len2]=0;		/* "C" string */
			}
			DeallocAPointer(Name);		/* Discard the prefix */
			Name = Result;		/* Return the result */
		}
	}
	return Name;		/* Result filename */
}

/**********************************

	MacOS ONLY!

	This will use a generic pathname and open a Macintosh
	resource file. It has all the virtues and problems of HOpenResFile();

**********************************/

short BURGERCALL OpenAMacResourceFile(const char *PathName,char Permission)
{
	char FileName[FULLPATHSIZE];
	Word8 NameBuffer[256];

	ExpandAPathToBufferNative(FileName,PathName);		/* Create the directories */
	CStr2PStr((char *)NameBuffer,FileName);
	return HOpenResFile(MacVRefNum,MacDirID,NameBuffer,Permission);	/* Open the file */
}

/**********************************

	MacOS ONLY!

	This will use a generic pathname and open a Macintosh
	resource file. It has all the virtues and problems of HOpenResFile();

**********************************/

Word BURGERCALL CreateAMacResourceFile(const char *PathName)
{
	char FileName[FULLPATHSIZE];
	Word8 NameBuffer[256];

	ExpandAPathToBufferNative(FileName,PathName);		/* Create the directories */
	CStr2PStr((char *)NameBuffer,FileName);
	HCreateResFile(MacVRefNum,MacDirID,NameBuffer);		/* Open the file */
	return ResError();
}

/**********************************

	Given a drive number, return in generic format
	the drive's name.

**********************************/

char * BURGERCALL GetAVolumeName(Word DriveNum)
{
#if TARGET_API_MAC_CARBON
	if (MacOSGetOSVersion()>=0x900) {		/* Can I do the MacOS 9/10 version? */
		FSVolumeInfoParam pb;
		HFSUniStr255 Name;
		
		pb.ioVRefNum = kFSInvalidVolumeRefNum;	/* I only want the name */
		pb.volumeIndex = DriveNum+1;			/* Drive starts with volume #1 */
		pb.whichInfo = kFSVolInfoNone;
		pb.volumeInfo = 0;
		pb.volumeName = &Name;
		pb.ref = 0;
		if (!PBGetVolumeInfoSync(&pb)) {		/* Get the data? */
			char *Result;
			Word i;
			i = Name.length;
			if (i) {
				Result = (char *)AllocAPointer(i+3);
				if (Result) {
					char *Dest;
					Word16 *Src;
					Src = Name.unicode;
					Dest = Result;
					Dest[0] = ':';
					++Dest;
					do {
						Dest[0] = (Word8)Src[0];
						++Src;
						++Dest;
					} while (--i);
					Dest[0] = ':';
					Dest[1] = 0;
					return Result;
				}
			}
		}
	}
#endif
	return static_cast<char *>(GetFullPathFromMacID(0,-1-DriveNum));	/* Convert to mac volume reference */
}

/**********************************

	Return the filesize of a file stream

**********************************/

Word32 BURGERCALL fgetfilesize(FILE *fp)
{
#if defined(__MSL__) && 0
	long Length;
	if (!GetEOF(fp->handle,&Length)) {	/* Call MACOS to get the file size */
		return Length;		/* Return the length */
	}
	return 0;

#else		/* Generic ANSI version */
	long Mark;
	Word32 Length;

	Mark = ftell(fp);	/* Get the current position */
	fseek(fp,0,SEEK_END);		/* Seek to the end of the file */
	Length = ftell(fp);
	fseek(fp,Mark,SEEK_SET);
	return Length;
#endif
}

/**********************************

	Set the initial default prefixs for a power up state
	*: = Boot volume
	$: = System folder
	@: = Prefs folder
	8: = Default directory
	9: = Application directory

**********************************/

void BURGERCALL SetDefaultPrefixs(void)
{
	char *TempPtr;
	short MyVRef;		/* Internal volume references */
	long MyDirID;		/* Internal drive ID */
	ProcessInfoRec MyProcess;	/* My input process */
	ProcessSerialNumber MyNumber;	/* My Process serial number */
	FSSpec MySpec;		/* FSSpec of the current app */
	
	MacVRefNum = 0;		/* Init my Mac references to default */
	MacDirID = 0;

	TempPtr = ConvertNativePathToPath("");	/* This covers all versions */
	if (TempPtr) {
		SetAPrefix(8,TempPtr);		/* Set the standard work prefix */
		DeallocAPointer(TempPtr);	/* Release the memory */
	}

	TempPtr = GetAVolumeName(0);		/* Get the boot volume name */
	if (TempPtr) {
		SetAPrefix(PREFIXBOOT,TempPtr);	/* Set the initial prefix */
		DeallocAPointer(TempPtr);		/* Deallocate the pointer */
	}

	if (!FindFolder(kOnSystemDisk,kSystemFolderType,kDontCreateFolder,&MyVRef,&MyDirID)) {
		TempPtr = static_cast<char *>(GetFullPathFromMacID(MyDirID,MyVRef));
		if (TempPtr) {
			SetAPrefix(PREFIXSYSTEM,TempPtr);	/* Set the system folder */
			DeallocAPointer(TempPtr);
		}
	}
	if (!FindFolder(kOnSystemDisk,kPreferencesFolderType,kDontCreateFolder,&MyVRef,&MyDirID)) {
		TempPtr = static_cast<char *>(GetFullPathFromMacID(MyDirID,MyVRef));
		if (TempPtr) {
			SetAPrefix(PREFIXPREFS,TempPtr);	/* Set the prefs folder */
			DeallocAPointer(TempPtr);
		}
	}
	FastMemSet(&MyProcess,0,sizeof(ProcessInfoRec));
	FastMemSet(&MySpec,0,sizeof(MySpec));
	MyNumber.lowLongOfPSN = kCurrentProcess;
	MyNumber.highLongOfPSN = 0;
	MyProcess.processInfoLength = sizeof(MyProcess);	/* Init the record */
	MyProcess.processName = 0;			/* I don't want the name */
	MyProcess.processAppSpec = &MySpec;	/* Get the FSSpec */
	if (!GetProcessInformation(&MyNumber,&MyProcess)) {	/* Get info */
		TempPtr = static_cast<char *>(GetFullPathFromMacID(MyProcess.processAppSpec->parID,
			MyProcess.processAppSpec->vRefNum));
		if (TempPtr) {
			SetAPrefix(9,TempPtr);		/* Application's pathname */
			DeallocAPointer(TempPtr);	/* Release the pointer */
		}
	}
}

/**********************************

	Get a file's Filetype
	Only valid for GSOS and MacOS

**********************************/

Word32 BURGERCALL GetAFileTypeNative(const char *FileName)
{
	HParamBlockRec Temp;		/* MacOS file parameter block */
	char Foo[256];				/* Pascal string version */

	CStr2PStr(Foo,FileName);		/* Convert to a PASCAL string */
	FastMemSet(&Temp,0,sizeof(Temp));	/* Init the struct */
	Temp.fileParam.ioVRefNum = MacVRefNum;	/* Pass the volume reference */
	Temp.fileParam.ioDirID = MacDirID;		/* Pass the directory reference */
	Temp.fileParam.ioNamePtr = (StringPtr)&Foo[0];	/* Get the filename (PASCAL) */

	if (!PBHGetFInfoSync(&Temp)) {		/* Get the file information */
		return Temp.fileParam.ioFlFndrInfo.fdType;	/* Return the Aux Type */
	}
	NonFatal("Can't get file info\n");
	return 0;
}

/**********************************

	Get a file's Auxtype
	Only valid for GSOS and MacOS

**********************************/

Word32 BURGERCALL GetAnAuxTypeNative(const char *FileName)
{
	HParamBlockRec Temp;		/* MacOS file parameter block */
	char Foo[256];				/* Pascal string version */

	CStr2PStr(Foo,FileName);		/* Convert to a PASCAL string */
	FastMemSet(&Temp,0,sizeof(Temp));	/* Init the struct */
	Temp.fileParam.ioVRefNum = MacVRefNum;	/* Pass the volume reference */
	Temp.fileParam.ioDirID = MacDirID;		/* Pass the directory reference */
	Temp.fileParam.ioNamePtr = (StringPtr)&Foo[0];	/* Get the filename (PASCAL) */

	if (!PBHGetFInfoSync(&Temp)) {		/* Get the file information */
		return Temp.fileParam.ioFlFndrInfo.fdCreator;	/* Return the Aux Type */
	}
	NonFatal("Can't get file info\n");
	return 0;
}

/**********************************

	Set a file's Filetype
	Only valid for GSOS and MacOS

**********************************/

void BURGERCALL SetAFileTypeNative(const char *FileName,Word32 FileType)
{
	HParamBlockRec Temp;		/* MacOS file parameter block */
	char Foo[256];				/* Pascal string version */

	CStr2PStr(Foo,FileName);		/* Convert to a PASCAL string */
	FastMemSet(&Temp,0,sizeof(Temp));	/* Init the struct */
	Temp.fileParam.ioVRefNum = MacVRefNum;	/* Pass the volume reference */
	Temp.fileParam.ioDirID = MacDirID;		/* Pass the directory reference */
	Temp.fileParam.ioNamePtr = (StringPtr)&Foo[0];	/* Get the filename (PASCAL) */

	if (!PBHGetFInfoSync(&Temp)) {		/* Get the file information */
		Temp.fileParam.ioVRefNum = MacVRefNum;
		Temp.fileParam.ioDirID = MacDirID;
		Temp.fileParam.ioFlFndrInfo.fdType = FileType;	/* Return the Aux Type */
		if (!PBHSetFInfoSync(&Temp)) {		/* Get the file information */
			return;
		}
	}
	NonFatal("Can't set file info\n");
}

/**********************************

	Set a file's Auxtype
	Only valid for GSOS and MacOS

**********************************/

void BURGERCALL SetAnAuxTypeNative(const char *FileName,Word32 AuxType)
{
	HParamBlockRec Temp;		/* MacOS file parameter block */
	char Foo[256];				/* Pascal string version */

	CStr2PStr(Foo,FileName);		/* Convert to a PASCAL string */
	FastMemSet(&Temp,0,sizeof(Temp));	/* Init the struct */
	Temp.fileParam.ioVRefNum = MacVRefNum;	/* Pass the volume reference */
	Temp.fileParam.ioDirID = MacDirID;		/* Pass the directory reference */
	Temp.fileParam.ioNamePtr = (StringPtr)&Foo[0];	/* Get the filename (PASCAL) */

	if (!PBHGetFInfoSync(&Temp)) {		/* Get the file information */
		Temp.fileParam.ioVRefNum = MacVRefNum;
		Temp.fileParam.ioDirID = MacDirID;
		Temp.fileParam.ioFlFndrInfo.fdCreator = AuxType;	/* Return the Aux Type */
		if (!PBHSetFInfoSync(&Temp)) {		/* Get the file information */
			return;
		}
	}
	NonFatal("Can't set file info\n");
}

/**********************************

	Set a file's Auxtype
	Only valid for GSOS and MacOS

**********************************/

void BURGERCALL FileSetFileAndAuxTypeNative(const char *FileName,Word32 FileType,Word32 AuxType)
{
	HParamBlockRec Temp;		/* MacOS file parameter block */
	char Foo[256];				/* Pascal string version */

	CStr2PStr(Foo,FileName);		/* Convert to a PASCAL string */
	FastMemSet(&Temp,0,sizeof(Temp));	/* Init the struct */
	Temp.fileParam.ioVRefNum = MacVRefNum;	/* Pass the volume reference */
	Temp.fileParam.ioDirID = MacDirID;		/* Pass the directory reference */
	Temp.fileParam.ioNamePtr = (StringPtr)&Foo[0];	/* Get the filename (PASCAL) */

	if (!PBHGetFInfoSync(&Temp)) {		/* Get the file information */
		Temp.fileParam.ioVRefNum = MacVRefNum;
		Temp.fileParam.ioDirID = MacDirID;
		Temp.fileParam.ioFlFndrInfo.fdType = FileType;	/* Return the Aux Type */
		Temp.fileParam.ioFlFndrInfo.fdCreator = AuxType;	/* Return the Aux Type */
		if (!PBHSetFInfoSync(&Temp)) {		/* Get the file information */
			return;
		}
	}
	NonFatal("Can't set file and type info\n");
}


/**********************************

	Mac OS version
	There is full volume name support in MacOS :)
	Paths with a leading ':' are prefixed with
	the current working directory
	The MacOS version converts these types of paths

	BurgerDrive:TEMP:TEMP2 = :BurgerDrive:TEMP:TEMP2:
	:TEMP = (Path):TEMP
	:TEMP:TEMP2 = (Path):TEMP:TEMP2
	TEMP = :TEMP

**********************************/

char * BURGERCALL ConvertNativePathToPath(const char *OldPath)
{
	Word Temp;				/* Ascii Temp */
	char TempPath[8192];	/* Handle to temp buffer */
	char *Output;			/* Running pointer to temp buffer */
	Word Length;			/* Length of finished string */

	Output = TempPath;		/* Get running pointer */

	if (!OldPath[0] || OldPath[0]==':') {		/* Must I prefix with the current directory? */
		short VRefNum;
		char *TempPtr;

		VRefNum = MacVRefNum;		/* Get the volume reference */
		if (!VRefNum && !MacDirID) {	/* If both are zero then look up default */
			HGetVol(0,&VRefNum,&MacDirID);		/* Call OS */
			MacDirID = 0;				/* Hack to simulate GetVol() */
		}
		TempPtr = static_cast<char *>(GetFullPathFromMacID(MacDirID,VRefNum));	/* Get the directory */
		if (TempPtr) {		/* Did I get a path? */
			strcpy(Output,TempPtr);		/* Copy to output */
			Output = Output+strlen(Output);	/* Fix pointer */
			DeallocAPointer(TempPtr);	/* Release input */
		}
		if (OldPath[0]) {	/* Was there a leading colon? */
			++OldPath;		/* Accept the leading colon */
		}
	} else {
		Output[0] = ':';		/* Place a leading colon in the output */
		++Output;
	}

/* Now, just copy the rest of the path */

	Temp = OldPath[0];
	if (Temp) {				/* Any more? */
		do {
			Output[0] = Temp;	/* Save char */
			++Output;
			++OldPath;			/* Accept char */
			Temp = OldPath[0];	/* Next char */
		} while (Temp);			/* Still more? */
	}
	MacVRefNum = 0;		/* Ack the input */
	MacDirID = 0;

	/* The wrap up... */
	/* Make sure it's appended with a colon */

	Length = Output-TempPath;		/* How many bytes is the new path? */
	if (Length) {					/* Valid length? */
		if (TempPath[Length-1]!=':') {	/* Last char a colon? */
			TempPath[Length] = ':';		/* End with a colon! */
			++Length;			/* Increase length! */
		}
	}
	Output = (char *)AllocAPointer(Length+1);	/* Get final pointer */
	if (Output) {						/* Did I get the memory? */
		if (Length) {					/* Anything to copy? */
			FastMemCpy(Output,TempPath,Length);	/* Copy the string */
		}
		Output[Length] = 0;			/* End the string with zero */
	}
	return Output;			/* Return the final product */
}


#endif