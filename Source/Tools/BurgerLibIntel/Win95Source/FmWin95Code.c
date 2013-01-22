/**********************************

	Win95 version

**********************************/

#include "FmFile.h"

#if defined(__WIN32__)
#include "ClStdLib.h"
#include "MmMemory.h"
#include "PfPrefs.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if !defined(__MWERKS__)
#include <io.h>
#include <stdlib.h>
#endif
#if defined(__WATCOMC__)
#define std
#endif

/**********************************

	This routine will get the time and date
	from a file.
	Note, this routine is Operating system specfic!!!

**********************************/

Word BURGERCALL GetFileModTimeNative(const char *FileName,TimeDate_t *Output)
{
	HANDLE fp;			/* Open file handle */
	FILETIME Temp;		/* Time returned by Windows */
	SYSTEMTIME Temp2;	/* Time after local time zone conversion */

	fp = CreateFile(FileName,0,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if (fp!=INVALID_HANDLE_VALUE) {				/* Can't open the file? */
		GetFileTime(fp,0,0,&Temp);				/* Get the time from the file */
		CloseHandle(fp);						/* Release the file */
		FileTimeToLocalFileTime(&Temp,&Temp);	/* Convert to local time */
		FileTimeToSystemTime(&Temp,&Temp2);		/* Convert the time to sections */

		Output->Milliseconds = (Word16)Temp2.wMilliseconds;	/* Get the milliseconds */
		Output->Second = (Word8)Temp2.wSecond;	/* Get the seconds */
		Output->Minute = (Word8)Temp2.wMinute;	/* Get the minute */
		Output->Hour = (Word8)Temp2.wHour;		/* Get the hour */
		Output->Day = (Word8)Temp2.wDay;			/* Get the day */
		Output->DayOfWeek = (Word8)Temp2.wDayOfWeek;	/* Weekday */
		Output->Month = (Word8)Temp2.wMonth;	/* Get the month */
		Output->Year = (Word16)Temp2.wYear;	/* Get the year */
		return FALSE;
	}
	FastMemSet(Output,0,sizeof(TimeDate_t));	/* Clear out the input */
	return TRUE;
}

/**********************************

	This routine will get the time and date
	from a file.
	Note, this routine is Operating system specfic!!!

**********************************/

Word BURGERCALL GetFileCreateTimeNative(const char *FileName,TimeDate_t *Output)
{
	HANDLE fp;
	FILETIME Temp;
	SYSTEMTIME Temp2;

	fp = CreateFile(FileName,0,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if (fp!=INVALID_HANDLE_VALUE) {		/* Can't open the file? */
		GetFileTime(fp,&Temp,0,0);		/* Get the time from the file */
		CloseHandle(fp);				/* Release the file */
		FileTimeToLocalFileTime(&Temp,&Temp);	/* Convert to local time */
		FileTimeToSystemTime(&Temp,&Temp2);	/* Convert the time to sections */

		Output->Milliseconds = (Word16)Temp2.wMilliseconds;
		Output->Second = (Word8)Temp2.wSecond;	/* Get the seconds */
		Output->Minute = (Word8)Temp2.wMinute;	/* Get the minute */
		Output->Hour = (Word8)Temp2.wHour;		/* Get the hour */
		Output->Day = (Word8)Temp2.wDay;			/* Get the day */
		Output->DayOfWeek = (Word8)Temp2.wDayOfWeek;	/* Weekday */
		Output->Month = (Word8)Temp2.wMonth;		/* Get the month */
		Output->Year = (Word16)Temp2.wYear;		/* Get the year */
		return FALSE;
	}
	FastMemSet(Output,0,sizeof(TimeDate_t));	/* Clear out the input */
	return TRUE;		/* Error */
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
	DWORD Output;

	Output = GetFileAttributes(FileName);	/* Get file info */
	if (Output & FILE_ATTRIBUTE_DIRECTORY /* || Output == -1 */ ) { /* -1 means error */
		return FALSE;		/* Bad file! */
	}
	return TRUE;		/* File exists */
}

/**********************************

	Open a directory for scanning
	Return an error if the directory doesn't exist

**********************************/

Word BURGERCALL OpenADirectory(DirectorySearch_t *Input,const char *Name)
{
	char PathName[FULLPATHSIZE];
	char PathName2[FULLPATHSIZE];
	Word i;

	ExpandAPathToBufferNative(PathName,Name);		/* Get the DOS filename */
	i = strlen(PathName);
	strcpy(PathName2,PathName);
	if (i && PathName[i-1]!='\\') {
		strcat(PathName2,"\\");
	}
	strcat(PathName2,"*.*");
	Input->FindHandle = FindFirstFile(PathName2,(WIN32_FIND_DATA *)&Input->MyFindT[0]);
	if (Input->FindHandle != INVALID_HANDLE_VALUE) {
		Input->HandleOk = 1;
		return FALSE;
	}
	return TRUE;
}

/**********************************

	Get the next directory entry
	Return FALSE if the entry is valid, TRUE if
	an error occurs.

**********************************/

typedef struct {
	Word32 Attrib;
	Word32 CreateLo;
	Word32 CreateHi;
	Word32 AccessLo;
	Word32 AccessHi;
	Word32 WriteLo;
	Word32 WriteHi;
	Word32 SizeLo;
	Word32 SizeHi;
	Word32 ReservedLo;
	Word32 ReservedHi;
	char FileName[260];
	char ShortName[13];
} WinDosData_t;

static void ConvertDosDate(TimeDate_t *Output,FILETIME *Temp)
{
	SYSTEMTIME Temp2;

	FileTimeToLocalFileTime(Temp,Temp);	/* Convert to local time */
	FileTimeToSystemTime(Temp,&Temp2);	/* Convert the time to sections */

	Output->Milliseconds = (Word16)Temp2.wMilliseconds;
	Output->Second = (Word8)Temp2.wSecond;	/* Get the seconds */
	Output->Minute = (Word8)Temp2.wMinute;	/* Get the minute */
	Output->Hour = (Word8)Temp2.wHour;		/* Get the hour */
	Output->Day = (Word8)Temp2.wDay;			/* Get the day */
	Output->DayOfWeek = (Word8)Temp2.wDayOfWeek;	/* Weekday */
	Output->Month = (Word8)Temp2.wMonth;		/* Get the month */
	Output->Year = (Word16)Temp2.wYear;		/* Get the year */
}


Word BURGERCALL GetADirectoryEntry(DirectorySearch_t *Input)
{
	Word Flags;
Again:
	if (Input->HandleOk==2) {
		if (!FindNextFile(Input->FindHandle,(WIN32_FIND_DATA *)&Input->MyFindT[0])) {
			return TRUE;
		}
	}
	Input->HandleOk = 2;
	strcpy(&Input->Name[0],&((WinDosData_t *)&Input->MyFindT[0])->FileName[0]);
	Flags = ((WinDosData_t *)&Input->MyFindT[0])->Attrib;
	Input->Dir = FALSE;
	Input->Hidden = FALSE;
	Input->System = FALSE;
	Input->Locked = FALSE;
	if (Flags & FILE_ATTRIBUTE_DIRECTORY) {
		Input->Dir = TRUE;
		if (!strcmp(".",Input->Name) || !strcmp("..",Input->Name)) {
			goto Again;
		}
	}
	if (Flags & FILE_ATTRIBUTE_HIDDEN) {
		Input->Hidden = TRUE;
	}
	if (Flags & FILE_ATTRIBUTE_SYSTEM) {
		Input->System = TRUE;
	}
	if (Flags & FILE_ATTRIBUTE_READONLY) {
		Input->Locked = TRUE;
	}

	ConvertDosDate(&Input->Create,(FILETIME *)&((WinDosData_t *)&Input->MyFindT[0])->CreateLo);
	ConvertDosDate(&Input->Modify,(FILETIME *)&((WinDosData_t *)&Input->MyFindT[0])->WriteLo);
	if (((WinDosData_t *)&Input->MyFindT[0])->SizeHi) {
		Input->FileSize = 0xFFFFFFFFUL;
	} else {
		Input->FileSize = ((WinDosData_t *)&Input->MyFindT[0])->SizeLo;
	}
	return FALSE;
}

/**********************************

	Close a directory that's being scanned

**********************************/

void BURGERCALL CloseADirectory(DirectorySearch_t *Input)
{
	if (Input->HandleOk) {
		FindClose(Input->FindHandle);
		Input->HandleOk = FALSE;
		Input->FindHandle = 0;
	}
}

/**********************************

	Delete a file using native file system

**********************************/

Word BURGERCALL DeleteAFileNative(const char *FileName)
{
	if (!DeleteFile(FileName)) {	/* Did it fail? */
		if (!RemoveDirectory(FileName)) {		/* Try to delete a directory */
			return TRUE;		/* I failed! */
		}
	}
	return FALSE;		/* Success! */
}

/**********************************

	Create a directory path using an operating system native name
	Return FALSE if successful, or TRUE if an error

**********************************/

static Word BURGERCALL DirCreate(const char *FileName)
{
	SECURITY_ATTRIBUTES MySec;
	MySec.nLength = sizeof(MySec);		/* Must set the size */
	MySec.lpSecurityDescriptor = 0;		/* No extra data */
	MySec.bInheritHandle = FALSE;		/* Don't do anything special */
	if (!CreateDirectory(FileName,&MySec)) {	/* Make the directory */
		if (GetLastError()!=ERROR_ALREADY_EXISTS) {		/* Already exist? */
			return TRUE;		/* Error */
		}
	}
	return FALSE;		/* Success! */
}

Word BURGERCALL CreateDirectoryPathNative(const char *FileName)
{
	char Old;		/* Marker for a filename */
	Word Err;		/* Error code */

	if (!DirCreate(FileName)) {		/* Easy way! */
		return FALSE;				/* No error */
	}
	/* Ok see if I can create the directory tree */
	if (FileName[0]) {			/* Is there a filename? */
		const char *WorkPtr;
		WorkPtr = FileName;
		if (WorkPtr[0] && WorkPtr[1]==':') {	/* Drive name? */
			WorkPtr+=2;			/* Skip the drive name */
		}
		if (WorkPtr[0] == '\\') {		/* Accept the first slash */
			++WorkPtr;
		}
		do {
			WorkPtr = strchr(WorkPtr,'\\');		/* Skip to the next colon */
			if (!WorkPtr) {			/* No colon found? */
				WorkPtr = strchr(FileName,0);
			}
			Old = WorkPtr[0];		/* Get the previous char */
			((char *)WorkPtr)[0] = 0;		/* End the string */
			Err = DirCreate(FileName);		/* Create the directory */
			((char *)WorkPtr)[0] = Old;		/* Restore the string */
			++WorkPtr;		/* Index past the char */
		} while (Old);		/* Still more string? */
		if (!Err) {			/* Cool!! */
			return FALSE;	/* No error */
		}
	}
	return TRUE;		/* Didn't do it! */
}

/**********************************

	This routine will take a generic path, expand it fully
	and then set the current working directory to the highest
	allowable directory I can attain without error. This is so that
	I can get around the limitation of 256 character pathnames on some
	operating systems (MacOS for 255 PStrings, DOS for 264 total pathname length)

**********************************/

void BURGERCALL ExpandAPathToBufferNative(char *Output,const char *FileName)
{
	char PathName[FULLPATHSIZE];
	Word Length;		/* Length of the final string */
	char *FilePtr;		/* Pointer to input buffer */
	Word DeviceNum;

	ExpandAPathToBuffer(PathName,FileName);		/* Resolve prefixes */

/**********************************

	DOS version and Win 95 version
	I prefer for all paths intended for DOS use
	a generic drive specifier before the working directory.
	The problem is that Volume LABEL are very difficult to parse
	and slow to access.

**********************************/

	/* First parse either the volume name of a .DXX device number */
	/* I hopefully will get a volume number since DOS prefers it */

	FilePtr = PathName;		/* Copy to running pointer */
	DeviceNum = (Word)-1;		/* Init the default drive number */
	if (((Word8 *)FilePtr)[0] == ':') {		/* Fully qualified pathname? */
		Word8 Temp,Temp2;
		Length = 0;		/* Init index to the volume name */
		do {
			++Length;		/* Parse to the next colon */
		} while (((Word8 *)FilePtr)[Length]!=':' && ((Word8 *)FilePtr)[Length]);
		Temp = ((Word8 *)FilePtr)[Length];
		Temp2 = ((Word8 *)FilePtr)[Length+1];		/* Save char in string */
		FilePtr[Length] = ':';
		FilePtr[Length+1] = 0;		/* Zap the entry */
		DeviceNum = FindAVolumeByName(FilePtr);	/* Find a volume */
		FilePtr[Length] = Temp;		/* Restore char in string */
		FilePtr[Length+1] = Temp2;
		if (DeviceNum == (Word)-1) {	/* Can't find the volume?!? */
			/* Since I didn't find the volume name, I'll assume it's */
			/* a network volume */
			Output[0] = '\\';
			++Output;
		} else {
			FilePtr = FilePtr+Length;		/* Accept the name */
			if (Temp) {			/* Remove the colon */
				++FilePtr;
			}
		}
	} else if (((Word8 *)FilePtr)[0] == '.') {
		Word Temp;
		Temp = ((Word8 *)FilePtr)[1];			/* Get the second char */
		if (Temp>='a' && Temp<('z'+1)) {
			Temp &= 0xDF;
		}
		if (Temp=='D') {			/* Is it a 'D'? */
			Length = 2;		/* Init numeric index */
			DeviceNum = 0;	/* Init drive number */
			for (;;) {
				Temp = ((Word8 *)FilePtr)[Length];	/* Get an ASCII char */
				if (Temp==':') {		/* Proper end of string? */
					++Length;
					break;
				}
				if (Temp<'0' || Temp>='9'+1) {	/* Numeric value? */
					Length = 0;		/* Do not accept ANY input! */
					DeviceNum = (Word)-1;	/* Force using the CWD */
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

	/* Now follow the directory chain for all the sub-directories */
	/* DeviceNum has the drive I wish to mount, if it is -1 then do */
	/* NOT alter the current working directory, I use what is there already */

	if (DeviceNum!=(Word)-1) {
		Output[0] = static_cast<char>(DeviceNum+'A');
		Output[1] = ':';
		Output[2] = '\\';
		Output+=3;
	}

	if (((Word8 *)FilePtr)[0]) {
		Word8 Temp;
		do {
			Temp = ((Word8 *)FilePtr)[0];
			++FilePtr;
			if (Temp==':') {
				Temp = '\\';
			}
			Output[0] = Temp;
			++Output;
		} while (Temp);
		Output-=2;
		if (((Word8 *)Output)[0]!='\\') {
			++Output;		/* Remove trailing slash */
		}
	}
	Output[0] = 0;			/* Terminate the "C" string */
}


/**********************************

	Change a directory using long filenames
	This only accepts Native OS filenames

**********************************/

Word BURGERCALL ChangeADirectory(const char *DirName)
{
	if (SetCurrentDirectory(DirName)) {
		return FALSE;		/* Success! */
	}
	return (Word)-1;		/* Error!! */
}

/**********************************

	Given a drive number, return in generic format
	the drive's name.

**********************************/

char * BURGERCALL GetAVolumeName(Word DriveNum)
{
	char *Result;		/* Return value */
	Word OldMode;		/* Previos error state */
	Word Length;		/* Length of volume name */
	char OutputName[64];	/* Buffer to copy to */
	char InputName[5];	/* Drive name template */

	InputName[0] = static_cast<char>('A'+DriveNum);	/* Create "C:\\" */
	InputName[1] = ':';
	InputName[2] = '\\';
	InputName[3] = 0;
	OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);	/* Don't show dialog */
	Result = 0;		/* Assume error */
	if (GetVolumeInformation(InputName,OutputName,sizeof(OutputName),
		0,0,0,0,0)) {		/* Get the volume name */
		Length = strlen(OutputName);	/* Length of string */
		if (Length) {
			Result = static_cast<char *>(AllocAPointer(Length+3));	/* Add 2 colons and a zero */
			if (Result) {
				Result[0] = ':';		/* Colon prefix */
				FastMemCpy(Result+1,OutputName,Length);
				Result[Length+1] = ':';	/* Colon suffix */
				Result[Length+2] = 0;	/* End it */
			}
		}
	}
	SetErrorMode(OldMode);		/* Restore the error code */
	return Result;			/* Return the pointer */
}

/**********************************

	Return the filesize of a file stream

**********************************/

Word32 BURGERCALL fgetfilesize(FILE *fp)
{
#if !defined(__MWERKS__)
	long Length;
	Length = filelength(fileno(fp));		/* Fast version for INTEL */
	if (Length!=-1) {
		return Length;
	}
	return 0;
#else
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

#define _argv __argv
#if defined(__MWERKS__)
#ifdef __cplusplus
extern "C" {
#endif
extern char **__argv;
#ifdef __cplusplus
}
#endif
#endif

void BURGERCALL SetDefaultPrefixs(void)
{
	char NameBuffer[MAX_PATH];
	char *TempPtr;

	TempPtr = ConvertNativePathToPath("");	/* This covers all versions */
	if (TempPtr) {
		SetAPrefix(8,TempPtr);		/* Set the standard work prefix */
		DeallocAPointer(TempPtr);	/* Release the memory */
	}

	TempPtr = ConvertNativePathToPath(_argv[0]);
	if (TempPtr) {
		SetAPrefix(9,TempPtr);		/* Set the application prefix */
		DeallocAPointer(TempPtr);
		PopAPrefix(9);		/* Remove the application's name */
	}

	GetSystemDirectory(NameBuffer,sizeof(NameBuffer));	/* System folder */
	TempPtr = ConvertNativePathToPath(NameBuffer);		/* Convert to Burgerlib */
	if (TempPtr) {
		SetAPrefix(PREFIXSYSTEM,TempPtr);		/* Set the system folder */
		TempPtr[3] = 0;					/* ".D3:xxxx" is now ".D3" */
		SetAPrefix(PREFIXBOOT,TempPtr);	/* Set the boot volume */
		DeallocAPointer(TempPtr);
	}
#if defined(__WATCOMC__) || defined(__MWERKS__)
	TempPtr = std::getenv("APPDATA");
#else
	TempPtr = getenv("APPDATA");
#endif
	if (TempPtr) {
		TempPtr = ConvertNativePathToPath(TempPtr);
	} else {
		GetWindowsDirectory(NameBuffer,sizeof(NameBuffer));	/* Get the windows folder */
		TempPtr = ConvertNativePathToPath(NameBuffer);
	}
	if (TempPtr) {
		SetAPrefix(PREFIXPREFS,TempPtr);		/* Set the prefs folder */
		DeallocAPointer(TempPtr);
	}
}

/**********************************

	Windows 95 version
	There is no volume name support in Windows 95
	Paths without a leading '\' are prefixed with
	the current working directory
	The Win95 version converts these types of paths

	C:\TEMP\TEMP2 = .D2:TEMP:TEMP2:
	TEMP = (Path):TEMP
	TEMP\TEMP2 = (Path):TEMP:TEMP2
	\TEMP = (Drive):TEMP

**********************************/

char * BURGERCALL ConvertNativePathToPath(const char *OldPath)
{
	Word Temp;				/* Ascii Temp */
	char TempPath[8192];	/* Handle to temp buffer */
	char *Output;			/* Running pointer to temp buffer */
	Word Length;			/* Length of finished string */

	Output = TempPath;		/* Get running pointer */


	/* Parse out the C: (If any) */
	{
		char FooHand[8192];		/* Handle to temp memory for full pathname */
		char *FooPtr;		/* Pointer to full pathname */
		char *Last;

		FooPtr =FooHand;	/* Lock it down */
		if (!OldPath[0]) {		/* No directory at all? */
			OldPath = ".";		/* Just get the current directory */
		}
		GetFullPathName(OldPath,8192,FooPtr,&Last);	/* Have window parse it out */

		if (FooPtr[0]=='\\' && FooPtr[1]=='\\') {	/* Network name? */
			Output[0] = ':';		/* Leading colon */
			++Output;				/* Accept it */
			FooPtr+=2;			/* Only return 1 colon */
		} else {
			Temp = FooPtr[0];		/* Get the drive letter */
			if (Temp>='a' && Temp<'z'+1) {	/* Upper case */
				Temp &= 0xDF;
			}
			Temp = Temp-'A';
			FooPtr += 3;			/* Accept the C:\ */

	/* At this point I have the drive number, create the drive number prefix */

			Output[0] = '.';		/* .D2 for C: */
			Output[1] = 'D';
			LongWordToAscii(Temp,&Output[2]);
			Output = Output+strlen(Output);
			Output[0] = ':';		/* Append a colon */
			++Output;
		}

	/* Now, just copy the rest of the path */
	/* I'll convert all backslashes to colons */

		Temp = FooPtr[0];
		if (Temp) {				/* Any more? */
			do {
				if (Temp=='\\') {	/* Convert directory holders */
					Temp = ':';		/* To generic paths */
				}
				Output[0] = static_cast<char>(Temp);	/* Save char */
				++Output;
				++FooPtr;			/* Accept char */
				Temp = FooPtr[0];	/* Next char */
			} while (Temp);			/* Still more? */
		}
	}

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