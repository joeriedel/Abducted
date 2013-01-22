/**********************************

	MacOS version

**********************************/

#include "FmFile.h"

#if defined(__MACOSX__)
#include "MmMemory.h"
#include "ClStdLib.h"
#include "McMac.h"
#include "StString.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <crt_externs.h>
#include <CarbonCore/Files.h>
#include <CarbonCore/Script.h>
#include <CarbonCore/Resources.h>
#include <CoreFoundation/CFDate.h>
#include <HiServices/Processes.h>
#include <CarbonCore/Folders.h>
#include <AppKit/NSFileWrapper.h>
#include <Foundation/NSFileManager.h>
#include <Foundation/NSCalendarDate.h>
#include <Foundation/NSTimeZone.h>
#include <Foundation/NSPathUtilities.h>

/**********************************

	This routine will get the time and date
	from a file.
	Note, this routine is Operating system specfic!!!

**********************************/

Word BURGERCALL GetFileModTimeNative(const char *FileName,TimeDate_t *Output)
{
	Word Result;
	NSFileWrapper *myWrapper;
	NSDictionary *myAttributes;
	time_t myInterval;

	FastMemSet(Output,0,sizeof(TimeDate_t));	/* Zap it */
	Result = TRUE;

	/* Open a file wrapper */
	
	myWrapper = [[NSFileWrapper alloc] initWithPath: [NSString stringWithUTF8String:FileName]];
	if (myWrapper) {
		/* Ok, I seem to have this file, now get the attributes */
		
		myAttributes = [myWrapper fileAttributes];
		if (myAttributes) {
			/* Now get the time */
			myInterval = [[myAttributes objectForKey: NSFileModificationDate] timeIntervalSince1970];
			
			/* Convert to a time structure */
			
			if (!TimeDateFromANSITime(Output,myInterval)) {
				Result = FALSE;
			}
		}
	}
	return Result;		/* FALSE for ok */
}

/**********************************

	This routine will get the time and date
	from a file.
	Note, this routine is Operating system specfic!!!

**********************************/

Word BURGERCALL GetFileCreateTimeNative(const char *FileName,TimeDate_t *Output)
{
	FSRef myFileRef;
	FSCatalogInfo myCatalogInfo;

	if (!FSPathMakeRef((Word8 *)FileName,&myFileRef,0)) {
		if (!FSGetCatalogInfo(&myFileRef,kFSCatInfoCreateDate,&myCatalogInfo,0,0,0)) {
			MacOSXTimeDateFromUTCTime(Output,&myCatalogInfo.createDate);
			return FALSE;
		}
	}
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
	NSFileManager *myFileManager;

	/* Open a file wrapper */
	
	myFileManager = [NSFileManager alloc];
	if (myFileManager) {
		if ([myFileManager fileExistsAtPath:[NSString stringWithUTF8String:FileName]]) {
			return TRUE;
		}
	}
	return FALSE;
}

/**********************************

	Using a Burgerlib path, open a directory to scan it
	for filenames
	
**********************************/

Word BURGERCALL OpenADirectory(DirectorySearch_t *Input,const char *Name)
{
	char PathName[FULLPATHSIZE];
	NSFileManager *myFileManager;	
	NSString *TempString;
	NSArray *DirPtr;
	
	if (Input) {		/* Valid? */
		FastMemSet(Input,0,sizeof(DirectorySearch_t));		/* Blank it */
		myFileManager = [NSFileManager alloc];
		if (myFileManager) {
			ExpandAPathToBufferNative(PathName,Name);	/* Convert to native path */
			TempString = [NSString stringWithUTF8String:PathName];
			if (TempString) {
				DirPtr = [myFileManager directoryContentsAtPath:TempString];
				if (DirPtr) {
					NSEnumerator *WorkRec;
					WorkRec = [DirPtr objectEnumerator];
					if (WorkRec) {
						/* Retain the directory list */
						[WorkRec retain];
						Input->PathPrefix = StrCopyPad(PathName,256+8);
						strcat(Input->PathPrefix,"/");
						Input->PathEnd = &Input->PathPrefix[strlen(Input->PathPrefix)];
						Input->Enumerator = (void*)WorkRec;
						return FALSE;
					}
				}
			}
		}
	}
	return TRUE;
}

/**********************************

	Return a directory entry
	
**********************************/

Word BURGERCALL GetADirectoryEntry(DirectorySearch_t *Input)
{
	NSEnumerator *WorkRec;
	
	
	if (Input) {
		/* Erase the internal data */
	
		FastMemSet(&Input->FileSize,0,(Word8 *)(&((DirectorySearch_t *)0)->Name[256])-(Word8*)(&((DirectorySearch_t*)0)->FileSize));

		WorkRec = (NSEnumerator *)Input->Enumerator;
		if (WorkRec) {
			NSString *Entry;
			Entry = [WorkRec nextObject];		/* Get the next entry */
			if (Entry) {
			
				/* Ok, I got the filename, but now I need to get the file description */
				
				FSRef MyRef;
				CFStringGetCString((CFStringRef)Entry,Input->Name,sizeof(Input->Name),kCFStringEncodingMacRoman);	/* Perform the conversion */
				strcpy(Input->PathEnd,[Entry cString]);
				if (!FSPathMakeRef((Word8 *)Input->PathPrefix,&MyRef,0)) {
				
					/* Get the juicy details */
					
					FSCatalogInfo Data;
					if (!FSGetCatalogInfo(&MyRef,kFSCatInfoGettableInfo,&Data,0,0,0)) {
						Word Foo;
						Word Flags;
						MacOSXTimeDateFromUTCTime(&Input->Create,&Data.createDate);
						MacOSXTimeDateFromUTCTime(&Input->Modify,&Data.contentModDate);
						Flags = Data.nodeFlags;
						if (Flags&kFSNodeLockedMask) {
							Foo = TRUE;
						} else {
							Foo = FALSE;
						}
						Input->Locked = Foo;
						if (Flags&kFSNodeIsDirectoryMask) {
							Foo = TRUE;
							Input->FileSize = 0;		/* Directories have no size */
						} else {
							Foo = FALSE;
							Input->FileSize = (Word32)Data.dataLogicalSize;
						}
						Input->Dir = Foo;
						if (Input->Name[0]=='.' || Data.finderInfo[8]&0x40) {
							Foo = TRUE;
						} else {
							Foo = FALSE;
						}
						Input->Hidden = Foo;
						Input->System = 0;		/* Not valid on the mac */
					}
				}
				return FALSE;
			}
		}
	}
	return TRUE;		/* Directory is over */
}

/**********************************

	Release an opened directory
	
**********************************/

void BURGERCALL CloseADirectory(DirectorySearch_t *Input)
{
	if (Input) {			/* Valid? */
		NSEnumerator *ArrayPtr;
		ArrayPtr = (NSEnumerator *)Input->Enumerator;
		if (ArrayPtr) {
			[ArrayPtr release];		/* Release the enumerator */
			Input->Enumerator = 0;
		}
		DeallocAPointer((void *)Input->PathPrefix);		/* Release the path */
		Input->PathPrefix = 0;
		Input->PathEnd = 0;
	}
}

/**********************************

	Open a file using a native path

**********************************/

FILE * BURGERCALL OpenAFile(const char *FileName,const char *Type)
{
	char NewFileName[FULLPATHSIZE];		/* New filename from ExpandAPathToBufferNative */
	
	ExpandAPathToBufferNative(NewFileName,FileName);	/* Expand a filename */
	return fopen(NewFileName,Type);		/* Do it the MacOSX way */
}

/**********************************

	Copy a file using native pathnames

**********************************/

Word BURGERCALL CopyAFileNative(const char *DestFile,const char *SrcFile)
{
	NSFileManager *myFileManager;	
	NSString *SrcString;
	NSString *DestString;
	myFileManager = [NSFileManager alloc];
	if (myFileManager) {
		SrcString = [NSString stringWithUTF8String:SrcFile];
		if (SrcString) {
			DestString = [NSString stringWithUTF8String:DestFile];
			if (DestString) {
				if ([myFileManager copyPath:SrcString toPath:DestString handler:nil]) {
					return FALSE;
				}
			}
		}		
	}
	return TRUE;
}

/**********************************

	Delete a file using native file system

**********************************/

Word BURGERCALL DeleteAFileNative(const char *FileName)
{
	OSErr Error;
	FSRef myFileRef;

	Error = FSPathMakeRef((Word8 *)FileName,&myFileRef,0);
	if (Error == fnfErr) {
		return FALSE;
	}
	if (!Error) {
		if (!FSDeleteObject(&myFileRef)) {
			return FALSE;
		}
	}
	return TRUE;		/* Oh oh... */
}

/**********************************

	Rename a file using native pathnames

**********************************/

Word BURGERCALL RenameAFileNative(const char *NewName,const char *OldName)
{
	if (!rename(OldName,NewName)) {
		return FALSE;
	}
	return TRUE;		/* Oh oh... */
}

/**********************************

	Create a directory path using an operating system native name
	Return FALSE if successful, or TRUE if an error

**********************************/

Word BURGERCALL CreateDirectoryPathNative(const char *FileName)
{
	NSFileManager *myFileManager;	
	NSString *TempString;
	NSString *Fragments[100];		/* Array of pathname fragments */
	Word i;
	
	/* Open a file wrapper */
	
	myFileManager = [NSFileManager alloc];
	if (myFileManager) {
		TempString = [NSString stringWithUTF8String:FileName];
		if (![myFileManager createDirectoryAtPath:TempString attributes:0]) {
			BOOL DirVal;
			
			if ([myFileManager fileExistsAtPath:TempString isDirectory:&DirVal]) {
				if (DirVal) {
					return FALSE;
				}
			}
			
			/* Ok, this is going to be fun, i will have to try to create each and every sub */
			/* directory part and then repeat the directory creation */
		
			i = 0;
			do {
				const char *CheckPtr;
				CheckPtr = [TempString cString];		/* Get the current string */
				if (!strcmp(CheckPtr,"/Volumes")) {		/* On a secondary drive? */
					if (i>=2) {
						--i;			/* Ignore the volume name */
						break;
					}
					return TRUE;		/* Can't create a volume name */
				}
				
				/* At the top of the directory? */
				
				if (!CheckPtr || !strcmp(CheckPtr,"/")) {
					if (i) {			/* Must have SOMETHING */
						break;
					}
					return TRUE;		/* Oh, forget it! */
				}
				Fragments[i] = TempString;
				TempString = [TempString stringByDeletingLastPathComponent];
			} while (++i<100);
			
			do {
				--i;
				if (![myFileManager createDirectoryAtPath:Fragments[i] attributes:0] && !i) {
					return TRUE;		/* Failure! */
				}
			} while (i);		/* All done? */
		}
		return FALSE;		/* Success! */
	}
	return TRUE;
}

/**********************************

	This routine will take a generic path, expand it fully
	and then set the current working directory to the highest
	allowable directory I can attain without error. This is so that
	I can get around the limitation of 256 character pathnames on some
	operating systems (MacOS for 255 PStrings, DOS for 264 total pathname length)


	MacOS X version (Used for StdCLib)
	I am in hell.
	
	I need to see if I want the BOOT volume. If it is not,
	then I prepend a FAKE name of /Volumes to the name

**********************************/

typedef struct BootLocals_t {
	Word16 MacOSXBootNameSize;	/* Length of the boot volume name */
	Word8 GotBootName;			/* TRUE if I tested for the boot volume name */
	Word8 Padding;
	char MacOSXBootName[256];	/* Boot volume name in the format ":FooBar" (Not zero terminated) */
} BootLocals_t;

static BootLocals_t ExLocals;

void BURGERCALL ExpandAPathToBufferNative(char *Output2,const char *FileName)
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
		
		/* A trailing slash assumes more to follow, get rid of it */
		--Output;
		if ((Output==OutName) ||		/* Only a '/'? (Skip the check then) */
			(Output[0]!='/')) {
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

/**********************************

	Change a directory using long filenames
	This only accepts Native OS filenames

**********************************/

Word BURGERCALL ChangeADirectory(const char *DirName)
{
	if (!chdir(DirName)) {
		return FALSE;
	}
	return (Word)-1;	/* Error! */
}

/**********************************

	Given a drive number, return in generic format
	the drive's name.

**********************************/

char * BURGERCALL GetAVolumeName(Word DriveNum)
{
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
		CFStringRef MyRef;
		
		i = Name.length;
		if (i) {
		
			/* Convert Unicode to a CFString */
			
			MyRef = CFStringCreateWithCharacters(0,Name.unicode,i);
			if (MyRef) {
				Result = (char *)AllocAPointer(i*2+8);		/* Buffer for the string */
				if (Result) {
					Result[0] = ':';
					if (CFStringGetCString(MyRef,Result+1,i*2+8,kCFStringEncodingMacRoman)) {
						CFRelease(MyRef);		/* Release the string */
						strcat(Result,":");		/* Append the final colon */
						return Result;			/* Return the string */
					}
					DeallocAPointer(Result);
				}
				CFRelease(MyRef);
			}
		}
	}
	return 0;
}

/**********************************

	Return the filesize of a file stream

**********************************/

Word32 BURGERCALL fgetfilesize(FILE *fp)
{
	long Mark;
	Word32 Length;

	Mark = ftell(fp);	/* Get the current position */
	fseek(fp,0,SEEK_END);		/* Seek to the end of the file */
	Length = ftell(fp);
	fseek(fp,Mark,SEEK_SET);
	return Length;
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
	char *TempPtr2;
	char ***Argv;
	char NameBuffer[2048];
	int *Argc;
	short MyVRef;		/* Internal volume references */
	long MyDirID;		/* Internal drive ID */
	FSSpec MySpec;		/* FSSpec of the current app */

	TempPtr = GetAVolumeName(0);		/* Get the boot volume name */
	if (TempPtr) {
		SetAPrefix(PREFIXBOOT,TempPtr);	/* Set the initial prefix */
		DeallocAPointer(TempPtr);		/* Deallocate the pointer */
	}

	TempPtr = getcwd(0,0);		/* This covers all versions */
	if (TempPtr) {
		TempPtr2 = ConvertNativePathToPath(TempPtr);
		if (TempPtr2) {
			SetAPrefix(8,TempPtr2);		/* Set the standard work prefix */
			DeallocAPointer(TempPtr2);	/* Release the memory */
		}
		free(TempPtr);
	}

	Argv = _NSGetArgv();
	Argc = _NSGetArgc();
	if (Argv && Argc) {
		if (Argc[0]) {
			TempPtr2 = ConvertNativePathToPath(Argv[0][0]);
			if (TempPtr2) {
				SetAPrefix(9,TempPtr2);		/* Set the standard work prefix */
				PopAPrefix(9);
				DeallocAPointer(TempPtr2);	/* Release the memory */
			}
		}		
	}
		
	if (!FindFolder(kOnSystemDisk,kSystemFolderType,kDontCreateFolder,&MyVRef,&MyDirID)) {
		Word8 BlankName[1];
		BlankName[0] = 0;
		if (!FSMakeFSSpec(MyVRef,MyDirID,BlankName,&MySpec)) {
			FSRef MyRef;
			if (!FSpMakeFSRef(&MySpec,&MyRef)) {
				if (!FSRefMakePath(&MyRef,(Word8 *)NameBuffer,sizeof(NameBuffer))) {
					TempPtr2 = ConvertNativePathToPath(NameBuffer);
					if (TempPtr2) {
						SetAPrefix(PREFIXSYSTEM,TempPtr2);		/* Set the standard work prefix */
						DeallocAPointer(TempPtr2);	/* Release the memory */
					}
				}
			}
		}
	}
	if (!FindFolder(kOnSystemDisk,kPreferencesFolderType,kDontCreateFolder,&MyVRef,&MyDirID)) {
		Word8 BlankName[1];
		BlankName[0] = 0;
		if (!FSMakeFSSpec(MyVRef,MyDirID,BlankName,&MySpec)) {
			FSRef MyRef;
			if (!FSpMakeFSRef(&MySpec,&MyRef)) {
				if (!FSRefMakePath(&MyRef,(Word8 *)NameBuffer,sizeof(NameBuffer))) {
					TempPtr2 = ConvertNativePathToPath(NameBuffer);
					if (TempPtr2) {
						SetAPrefix(PREFIXPREFS,TempPtr2);		/* Set the standard work prefix */
						DeallocAPointer(TempPtr2);	/* Release the memory */
					}
				}
			}
		}
	}
}

/**********************************

	Get a file's Filetype
	Only valid for GSOS and MacOS

**********************************/

Word32 BURGERCALL GetAFileTypeNative(const char *FileName)
{
	NSFileWrapper *myWrapper;
	NSDictionary *myAttributes;

	/* Open a file wrapper */
	
	myWrapper = [[NSFileWrapper alloc] initWithPath: [NSString stringWithUTF8String:FileName]];
	if (myWrapper) {
		/* Ok, I seem to have this file, now get the attributes */
		
		myAttributes = [myWrapper fileAttributes];
		if (myAttributes) {
			/* Now get the time */
			NSNumber *Value;
			Value = [myAttributes objectForKey: NSFileHFSTypeCode];
			if (Value) {
				return [Value unsignedLongValue];
			}
		}
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
	NSFileWrapper *myWrapper;
	NSDictionary *myAttributes;

	/* Open a file wrapper */
	
	myWrapper = [[NSFileWrapper alloc] initWithPath: [NSString stringWithUTF8String:FileName]];
	if (myWrapper) {
		/* Ok, I seem to have this file, now get the attributes */
		
		myAttributes = [myWrapper fileAttributes];
		if (myAttributes) {
			/* Now get the time */
			NSNumber *Value;
			Value = [myAttributes objectForKey: NSFileHFSCreatorCode];
			if (Value) {
				return [Value unsignedLongValue];
			}
		}
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
	FSRef myFileRef;
	FSCatalogInfo myCatalogInfo;

	if (!FSPathMakeRef((Word8 *)FileName,&myFileRef,0)) {
		if (!FSGetCatalogInfo(&myFileRef,kFSCatInfoFinderInfo,&myCatalogInfo,0,0,0)) {
			((Word32 *)myCatalogInfo.finderInfo)[0] = FileType;
			if (!FSSetCatalogInfo(&myFileRef,kFSCatInfoFinderInfo,&myCatalogInfo)) {
				return;
			}
		}
	}
	NonFatal("Can't set file type\n");
}

/**********************************

	Set a file's Auxtype
	Only valid for GSOS and MacOS

**********************************/

void BURGERCALL SetAnAuxTypeNative(const char *FileName,Word32 AuxType)
{
	FSRef myFileRef;
	FSCatalogInfo myCatalogInfo;

	if (!FSPathMakeRef((Word8 *)FileName,&myFileRef,0)) {
		if (!FSGetCatalogInfo(&myFileRef,kFSCatInfoFinderInfo,&myCatalogInfo,0,0,0)) {
			((Word32 *)myCatalogInfo.finderInfo)[1] = AuxType;
			if (!FSSetCatalogInfo(&myFileRef,kFSCatInfoFinderInfo,&myCatalogInfo)) {
				return;
			}
		}
	}
	NonFatal("Can't set aux type\n");
}

/**********************************

	Set a file's Filetype and Auxtype
	Only valid for GSOS and MacOS

**********************************/

void BURGERCALL FileSetFileAndAuxTypeNative(const char *FileName,Word32 FileType,Word32 AuxType)
{
	FSRef myFileRef;
	FSCatalogInfo myCatalogInfo;

	if (!FSPathMakeRef((Word8 *)FileName,&myFileRef,0)) {
		if (!FSGetCatalogInfo(&myFileRef,kFSCatInfoFinderInfo,&myCatalogInfo,0,0,0)) {
			((Word32 *)myCatalogInfo.finderInfo)[0] = FileType;
			((Word32 *)myCatalogInfo.finderInfo)[1] = AuxType;
			if (!FSSetCatalogInfo(&myFileRef,kFSCatInfoFinderInfo,&myCatalogInfo)) {
				return;
			}
		}
	}
	NonFatal("Can't set file and aux type\n");
}

/**********************************

	Mac OSX version
	There is full volume name support in MacOS X :)
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

	if (OldPath[0]!='/') {		/* Must I prefix with the current directory? */
		if (!memcmp("./",OldPath,2)) {		/* Dispose of "current directory" */
			OldPath+=2;
		}
		Output[0] = '8';
		Output[1] = ':';
		Output+=2;
	} else {
		if (!memcmp(OldPath,"/Volumes/",9)) {
			Output[0] = ':';		/* Place a leading colon in the output */
			++Output;
			OldPath+=9;
		} else {
			const char *VolName;
			VolName = GetAVolumeName(0);
			if (VolName) {
				strcpy(Output,VolName);
				Output+=strlen(VolName);
				DeallocAPointer((void *)VolName);
			}
			++OldPath;
		}
	}

/* Now, just copy the rest of the path */

	Temp = OldPath[0];
	if (Temp) {				/* Any more? */
		do {
			if (Temp=='/') {
				Temp = ':';
			}
			Output[0] = Temp;	/* Save char */
			++Output;
			++OldPath;			/* Accept char */
			Temp = OldPath[0];	/* Next char */
		} while (Temp);			/* Still more? */
	}

	/* The wrap up... */
	/* Make sure it's appended with a colon */

	Length = Output-TempPath;		/* How many bytes is the new path? */
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