/**********************************

	DOS version

**********************************/

#include "FmFile.h"

#if defined(__MSDOS__)
#include "MsDos.h"
#include "ClStdLib.h"
#include "MmMemory.h"
#include "PfPrefs.h"
#include <string.h>
#include <dos.h>
#include <io.h>

/**********************************

	This routine will get the time and date
	from a file.
	Note, this routine is Operating system specfic!!!

**********************************/

Word32 DoWorkDOSMod(const char *Referance);

#pragma aux DoWorkDOSMod = \
	"XOR ECX,ECX"		/* Assume bogus time */ \
	"MOV EAX,03D00H"	/* Open file */ \
	"INT 021H"			/* Call messy dos */ \
	"JC Foo"			/* Oh oh... */ \
	"PUSH EAX"			/* Save the file handle */ \
	"MOV EBX,EAX"		/* Copy to EBX */ \
	"MOV EAX,05700H"	/* Get the file date */ \
	"INT 021H"			/* Call messy dos */ \
	"POP EBX"			/* Restore the file handle */ \
	"JNC Good"			/* Good read? */ \
	"XOR EDX,EDX"		/* Zark the time since it was bad */ \
	"XOR ECX,ECX" \
	"Good:"				/* Reenter */ \
	"SHL EDX,16"		/* Move the date to the upper 16 bits */ \
	"AND ECX,0FFFFH"	/* Mask off the time */ \
	"OR ECX,EDX"		/* Merge into ECX */ \
	"MOV EAX,03E00H"	/* Close the file and dispose of the handle */ \
	"INT 021H" \
	"Foo:"				/* Exit with result in ECX */ \
	parm [edx]			/* Filename in EDX */ \
	modify [eax ebx ecx edx]	/* I blast these */ \
	value [ecx]			/* Return in ECX */

Word BURGERCALL GetFileModTimeNative(const char *FileName,TimeDate_t *Output)
{
	Word32 Temp;

	if (AreLongFilenamesAllowed()) {		/* Win95? */
		Regs16 MyRegs;

	/* This code does NOT work on CD's or Networks */
	/* oh crud... */

#if 0
		MyRegs.ax = 0x7143;	/* Get file attributes */
		MyRegs.bx = 4;		/* Get last modified time */
		MyRegs.cx = 0;		/* Normal access requested */
		Temp = GetRealBufferPtr();	/* Local buffer */
		MyRegs.dx = Temp;			/* Pass the filename buffer */
		MyRegs.ds = (Temp>>16);		/* Get the segment */
		strcpy(GetRealBufferProtectedPtr(),FileName);
		Int86x(0x21,&MyRegs,&MyRegs);	/* Call Win95 */
		if (MyRegs.flags&1) {			/* Error? */
			goto FooBar;
		}
		Temp = (Word32)MyRegs.di;		/* Get the date and time */
		Temp = (Temp<<16)|MyRegs.cx;
#else
		/* This works on all devices */
		Word16 Ref;
		MyRegs.ax = 0x716C;			/* Open with long filenames */
		MyRegs.bx = 0x0000;			/* Read */
		MyRegs.cx = 0x0000;
		MyRegs.dx = 0x0001;			/* Open the file */
		MyRegs.di = 0x0000;
		Temp = GetRealBufferPtr();	/* Local buffer */
		MyRegs.si = static_cast<Word16>(Temp);			/* Pass the filename buffer */
		MyRegs.ds = static_cast<Word16>(Temp>>16);		/* Get the segment */
		strcpy(static_cast<char *>(GetRealBufferProtectedPtr()),FileName);
		Int86x(0x21,&MyRegs,&MyRegs);	/* Call Win95 */
		if (MyRegs.flags&1) {		/* Error? */
			goto FooBar;
		}
		Ref = MyRegs.ax;
		MyRegs.ax = 0x5700;		/* Read access time */
		MyRegs.bx = Ref;
		Int86x(0x21,&MyRegs,&MyRegs);
		Temp = MyRegs.flags;
		MyRegs.bx = Ref;
		MyRegs.ax = 0x3E00;		/* Close the file */
		Int86x(0x21,&MyRegs,&MyRegs);
		if (Temp&1) {			/* Error getting file time? */
			goto FooBar;
		}
		Temp = (Word32)MyRegs.dx;		/* Get the date and time */
		Temp = (Temp<<16)|MyRegs.cx;
#endif
	} else {
		Temp = DoWorkDOSMod(FileName);		/* Call DOS to perform the action */
		if (!Temp) {
			goto FooBar;		/* Error? */
		}
	}
	Output->Second = (Word8)((Temp&0x1F)<<1);	/* Get the seconds */
	Temp=Temp>>5;
	Output->Minute = (Word8)(Temp&0x3F);		/* Get the minute */
	Temp=Temp>>6;
	Output->Hour = (Word8)(Temp&0x1F);		/* Get the hour */
	Temp=Temp>>5;
	Output->Day = (Word8)(Temp&0x1F);		/* Get the day */
	Temp=Temp>>5;
	Output->Month = (Word8)(Temp&0xF);		/* Get the month */
	Temp=Temp>>4;
	Output->Year = (Word16)(Temp+1980);		/* Get the year */
	Output->Milliseconds = 0;
	return FALSE;
FooBar:
	FastMemSet(Output,0,sizeof(TimeDate_t));	/* Clear it on error */
	return TRUE;		/* Error */
}

/**********************************

	This routine will get the time and date
	from a file.
	Note, this routine is Operating system specfic!!!

**********************************/

Word BURGERCALL GetFileCreateTimeNative(const char *FileName,TimeDate_t *Output)
{
	Word32 Temp;
	Word Result;

	Result = FALSE;		/* If no dos support then don't return an error */
	if (AreLongFilenamesAllowed()) {		/* Win95? */
		Regs16 MyRegs;
		strcpy(static_cast<char *>(GetRealBufferProtectedPtr()),FileName);
		MyRegs.ax = 0x7143;	/* Get file attributes */
		MyRegs.bx = 8;		/* Get creation date/time */
		Temp = GetRealBufferPtr();	/* Local buffer */
		MyRegs.dx = static_cast<Word16>(Temp);		/* Pass the filename buffer */
		MyRegs.ds = static_cast<Word16>(Temp>>16);	/* Get the segment */
		Int86x(0x21,&MyRegs,&MyRegs);
		if (!(MyRegs.flags&1)) {
			Temp = (Word32)MyRegs.di;		/* Get the date and time */
			Temp = (Temp<<16)|MyRegs.cx;
			Output->Second = (Word8)((Temp&0x1F)<<1);	/* Get the seconds */
			Temp=Temp>>5;
			Output->Minute = (Word8)(Temp&0x3F);		/* Get the minute */
			Temp=Temp>>6;
			Output->Hour = (Word8)(Temp&0x1F);		/* Get the hour */
			Temp=Temp>>5;
			Output->Day = (Word8)(Temp&0x1F);		/* Get the day */
			Temp=Temp>>5;
			Output->Month = (Word8)(Temp&0xF);		/* Get the month */
			Temp=Temp>>4;
			Output->Year = (Word16)(Temp+1980);		/* Get the year */
			Output->Milliseconds = (Word16)MyRegs.si;	/* Get milliseconds */
			return FALSE;
		}
		Result = TRUE;		/* Error condition */
	}
	FastMemSet(Output,0,sizeof(TimeDate_t));	/* No DOS support */
	return Result;		/* Error! */
}

/**********************************

	Determine if a file exists.
	I will return TRUE if the specified path
	is a path to a file that exists, if it doesn't exist
	or it's a directory, I return FALSE.
	Note : I do not check if the file havs any data in it.
	Just the existance of the file.

**********************************/

Word32 DoWorkDOSExist(const char *Referance);

#pragma aux DoWorkDOSExist = \
	"MOV EAX,04300H"	/* Get file attributes */ \
	"INT 021H"			/* Call messy dos */ \
	"JNC Foo"			/* No error */ \
	"MOV ECX,0x18"		/* Force error */ \
	"Foo:" \
	parm [edx]			/* Filename in EDX */ \
	modify [eax ecx edx]	/* I blast these */ \
	value [ecx]			/* Return in ECX */

Word BURGERCALL DoesFileExistNative(const char *FileName)
{
	Word32 Temp;

	if (AreLongFilenamesAllowed()) {	/* Win95? */
		Regs16 MyRegs;
		MyRegs.ax = 0x7143;		/* Get file attributes */
		MyRegs.bx = 0;			/* Get file attributes only */
		Temp = GetRealBufferPtr();	/* Local buffer */
		MyRegs.dx = static_cast<Word16>(Temp);			/* Pass the filename buffer */
		MyRegs.ds = static_cast<Word16>(Temp>>16);		/* Get the segment */
		strcpy(static_cast<char *>(GetRealBufferProtectedPtr()),FileName);
		Int86x(0x21,&MyRegs,&MyRegs);	/* Call Win95 */
		if (MyRegs.flags&1 || MyRegs.cx&0x18) {		/* Error? Or directory? */
			return FALSE;
		}
	} else {
		if (DoWorkDOSExist(FileName)&0x18) {		/* Call DOS to perform the action */
			return FALSE;		/* Error? */
		}
	}
	return TRUE;		/* File was found */
}

/**********************************

	Open a directory for scanning
	Return an error if the directory doesn't exist

**********************************/

Word BURGERCALL OpenADirectory(DirectorySearch_t *Input,const char *Name)
{
	Regs16 MyRegs;
	Word32 TempPath;
	Word8 *DataPtr;
	char PathName[FULLPATHSIZE];
	Word i;

	TempPath = GetRealBufferPtr();
	DataPtr = static_cast<Word8 *>(GetRealBufferProtectedPtr());
	ExpandAPathToBufferNative(PathName,Name);		/* Get the DOS filename */
	strcpy((char *)DataPtr+512,PathName);
	i = strlen(PathName);
	if (i && PathName[i-1]!='\\') {
		strcat((char *)DataPtr+512,"\\");
	}
	strcat((char *)DataPtr+512,"*.*");

	if (AreLongFilenamesAllowed()) {
		MyRegs.ax = 0x714E;			/* Read first */
		MyRegs.cx = 0x0010;			/* All directories and files */
		MyRegs.dx = static_cast<Word16>(TempPath+512);
		MyRegs.ds = static_cast<Word16>(TempPath>>16);
		MyRegs.di = static_cast<Word16>(TempPath);
		MyRegs.es = static_cast<Word16>(TempPath>>16);
		MyRegs.si = 1;				/* Return DOS time */
		Int86x(0x21,&MyRegs,&MyRegs);
		if (!(MyRegs.flags&1)) {
			Input->HandleOk = 10;
			Input->FileHandle = MyRegs.ax;
			return FALSE;
		}
		return TRUE;
	}
	if (!_dos_findfirst((char *)DataPtr+512,0x10,(struct find_t *)&Input->MyFindT[0])) {
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

static void ConvertDosDate(TimeDate_t *Output,Word32 Temp)
{
	Output->Second = (Word8)((Temp&0x1F)<<1);	/* Get the seconds */
	Temp=Temp>>5;
	Output->Minute = (Word8)(Temp&0x3F);		/* Get the minute */
	Temp=Temp>>6;
	Output->Hour = (Word8)(Temp&0x1F);		/* Get the hour */
	Temp=Temp>>5;
	Output->Day = (Word8)(Temp&0x1F);		/* Get the day */
	Temp=Temp>>5;
	Output->Month = (Word8)(Temp&0xF);		/* Get the month */
	Temp=Temp>>4;
	Output->Year = (Word16)(Temp+1980);		/* Get the year */
	Output->Milliseconds = 0;	/* Get milliseconds */
}

Word BURGERCALL GetADirectoryEntry(DirectorySearch_t *Input)
{
	Regs16 MyRegs;
	Word32 TempPath;
	Word8 *DataPtr;
	Word Flags;

	TempPath = GetRealBufferPtr();
	DataPtr = static_cast<Word8*>(GetRealBufferProtectedPtr());
Again:
	if (AreLongFilenamesAllowed()) {
		if (Input->HandleOk==11) {
			MyRegs.ax = 0x714F;
			MyRegs.bx = Input->FileHandle;
			MyRegs.di = static_cast<Word16>(TempPath);
			MyRegs.es = static_cast<Word16>(TempPath>>16);
			MyRegs.si = 1;			/* Return DOS time */
			Int86x(0x21,&MyRegs,&MyRegs);
			if (MyRegs.flags&1) {
				return TRUE;
			}
		}
		Input->HandleOk = 11;
		strcpy(Input->Name,((WinDosData_t *)DataPtr)->FileName);
		Input->Dir = FALSE;
		Input->Hidden = FALSE;
		Input->System = FALSE;
		Input->Locked = FALSE;
		Flags = ((WinDosData_t *)DataPtr)->Attrib;
		if (Flags & 0x10) {
			Input->Dir = TRUE;
			if (!strcmp(".",Input->Name) || !strcmp("..",Input->Name)) {
				goto Again;
			}
		}
		if (Flags & 0x01) {
			Input->Locked = TRUE;
		}
		if (Flags & 0x02) {
			Input->Hidden = TRUE;
		}
		if (Flags & 0x04) {
			Input->System = TRUE;
		}
		ConvertDosDate(&Input->Create,((WinDosData_t *)DataPtr)->CreateLo);
		ConvertDosDate(&Input->Modify,((WinDosData_t *)DataPtr)->WriteLo);
		if (((WinDosData_t *)DataPtr)->SizeHi) {
			Input->FileSize = 0xFFFFFFFFUL;
		} else {
			Input->FileSize = ((WinDosData_t *)DataPtr)->SizeLo;
		}
		return FALSE;
	}
	if (Input->HandleOk==2) {
		if (_dos_findnext((struct find_t *)&Input->MyFindT[0])) {
			return TRUE;
		}
	}
	Input->HandleOk = 2;
	strcpy(Input->Name,((struct find_t *)&Input->MyFindT[0])->name);

	Input->Dir = FALSE;
	Input->Hidden = FALSE;
	Input->System = FALSE;
	Input->Locked = FALSE;
	Flags = ((struct find_t *)&Input->MyFindT[0])->attrib;
	if (Flags & 0x10) {
		Input->Dir = TRUE;
		if (!strcmp(".",Input->Name) || !strcmp("..",Input->Name)) {
			goto Again;
		}
	}
	if (Flags & 0x01) {
		Input->Locked = TRUE;
	}
	if (Flags & 0x02) {
		Input->Hidden = TRUE;
	}
	if (Flags & 0x04) {
		Input->System = TRUE;
	}
	ConvertDosDate(&Input->Create,((Word32 *)&((struct find_t *)&Input->MyFindT[0])->wr_time)[0]);
	FastMemCpy(&Input->Modify,&Input->Create,sizeof(TimeDate_t));
	Input->FileSize = ((struct find_t *)&Input->MyFindT[0])->size;
	return FALSE;
}

/**********************************

	Close a directory that's being scanned

**********************************/

void BURGERCALL CloseADirectory(DirectorySearch_t *Input)
{
	Regs16 MyRegs;
	if (Input->HandleOk>=10) {		/* This can only be true if an extended directory */
		MyRegs.ax = 0x71A1;
		MyRegs.bx = Input->FileHandle;
		Int86x(0x21,&MyRegs,&MyRegs);
		Input->HandleOk = FALSE;
	} else if (Input->HandleOk) {	/* Dos mode? */
		_dos_findclose((struct find_t *)&Input->MyFindT[0]);
		Input->HandleOk = FALSE;
	}
}

/**********************************

	Delete a file using native file system

**********************************/

Word BURGERCALL DeleteAFileNative(const char *FileName)
{
	Regs16 Regs;		/* Used by DOS */
	Word32 RealBuffer;	/* Real pointer to buffer */
	Word LongOk;

	LongOk = AreLongFilenamesAllowed();
	RealBuffer = GetRealBufferPtr();	/* Get real memory */
	strcpy(static_cast<char *>(RealToProtectedPtr(RealBuffer)),FileName);	/* Copy path */

	if (LongOk) {
		Regs.ax = 0x7141;		/* Try it via windows */
		Regs.dx = (Word16)RealBuffer;
		Regs.ds = (Word16)(RealBuffer>>16);
		Regs.cx = 0;		/* Normal file */
		Regs.si = 0;		/* No wildcards are present */
		Int86x(0x21,&Regs,&Regs);		/* Delete the file */
		if (!(Regs.flags&1)) {		/* Error? */
			return FALSE;
		}
	}
	Regs.ax = 0x4100;		/* Try it the DOS 5.0 way */
	Regs.dx = (Word16)RealBuffer;
	Regs.ds = (Word16)(RealBuffer>>16);
	Int86x(0x21,&Regs,&Regs);
	if (Regs.flags&1) {	/* Error? */
		return TRUE;		/* Oh forget it!!! */
	}
	return FALSE;		/* Success!! */
}

/**********************************

	Create a directory path using an operating system native name
	Return FALSE if successful, or TRUE if an error

**********************************/

Word DoWorkDOSCrDir(const char *Referance);

#pragma aux DoWorkDOSCrDir = \
	"MOV EAX,03900H"	/* Create directory */ \
	"PUSH EDX"			/* Save the filename pointer */ \
	"INT 021H"			/* Call DOS */ \
	"POP EDX"			/* Restore the filename */ \
	"JNC Good"			/* Excellent!! */ \
	"MOV EAX,04300H"	/* Get the file attributes */ \
	"INT 021H"			/* Call DOS */ \
	"JC Bad"			/* File not found!! */ \
	"TEST ECX,010H"		/* Is this a preexisting directory? */ \
	"JNZ Good"			/* Excellent! */ \
	"Bad:" \
	"MOV EAX,EAX"		/* Error */ \
	"JMP Foo"			/* Exit */ \
	"Good:" \
	"XOR EAX,EAX"		/* No error */ \
	"Foo:" \
	parm [edx]			/* Filename in EDX */ \
	modify [eax ecx edx]	/* I blast these */ \
	value [eax]			/* Return in EAX */

static Word BURGERCALL DirCreate(const char *FileName)
{
	if (AreLongFilenamesAllowed()) {
		Regs16 MyRegs;
		Word32 Temp;
		MyRegs.ax = 0x7139;		/* Create long filename version */
		Temp = GetRealBufferPtr();
		MyRegs.dx = static_cast<Word16>(Temp);		/* Save the real memory pointer */
		MyRegs.ds = static_cast<Word16>(Temp>>16);
		strcpy(static_cast<char *>(GetRealBufferProtectedPtr()),FileName);
		Int86x(0x21,&MyRegs,&MyRegs);	/* Make the directory */
		if (!(MyRegs.flags&1)) {
			return FALSE;
		}
		MyRegs.ax = 0x7143;
		MyRegs.bx = 0;			/* Get attributes */
		MyRegs.dx = static_cast<Word16>(Temp);
		MyRegs.ds = static_cast<Word16>(Temp>>16);
		Int86x(0x21,&MyRegs,&MyRegs);
		if (!(MyRegs.flags&1) && MyRegs.cx & 0x10) {	/* Directory here? */
			return FALSE;		/* Directory already present */
		}
		return TRUE;		/* Error! */
	}
	return DoWorkDOSCrDir(FileName);		/* Dos 5.0 or previous */
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
	Regs16 Regs;		/* Used by DOS */
	Word32 RealBuffer;	/* Real pointer to buffer */
	Word LongOk;

	LongOk = AreLongFilenamesAllowed();		/* Flag for long filenames */

	RealBuffer = GetRealBufferPtr();	/* Get real memory */
	strcpy(static_cast<char *>(RealToProtectedPtr(RealBuffer)),DirName);	/* Copy path */

	if (LongOk) {				/* Win95 is present? */
		Regs.ax = 0x713B;		/* Try it via windows */
		Regs.dx = (Word16)RealBuffer;
		Regs.ds = (Word16)(RealBuffer>>16);
		Int86x(0x21,&Regs,&Regs);	/* Change the directory */
		if (!(Regs.flags&1)) {		/* Error? */
			return 0;
		}
	}
	Regs.ax = 0x3B00;		/* Try it the DOS 5.0 way */
	Regs.dx = (Word16)RealBuffer;
	Regs.ds = (Word16)(RealBuffer>>16);
	Int86x(0x21,&Regs,&Regs);
	if (Regs.flags&1) {	/* Error? */
		return (Word)-1;		/* Oh forget it!!! */
	}
	return 0;		/* Success!! */
}

/**********************************

	DOS Only!

	Returns true if Win95 is present and long filenames
	are allowed.

**********************************/

static Bool Checked;		/* True if already checked */
static Bool Allowed;		/* True if Win95 is present */

Word BURGERCALL AreLongFilenamesAllowed(void)
{
	Regs16 Regs;
	if (!Checked) {				/* Did I check? */
		Checked = TRUE;
		Regs.ax = 0x3000;		/* Get version */
		Int86x(0x21,&Regs,&Regs);
		if ((Word8)Regs.ax >= 7) {	/* Dos 7.0 */
			Word32 Seg;

			Seg = GetRealBufferPtr();	/* Get real memory */
 			if (Seg) {
				char *DestPtr;
				DestPtr = static_cast<char *>(GetRealBufferProtectedPtr());	/* Get protected */
				strcpy(DestPtr,".");	/* Copy the string */
				Regs.ax = 0x71A0;	/* Ask OS */
				Regs.es = (Word16)(Seg>>16);		/* Save the segment */
				Regs.ds = (Word16)(Seg>>16);
				Regs.dx = (Word16)Seg;	/* Save the offset */
				Regs.di = (Word16)Seg;
				Regs.cx = 256;			/* Buffer size */
				Int86x(0x21,&Regs,&Regs);	/* Call Win95 */
				if (!(Regs.flags&1)) {
					Allowed = TRUE;		/* Long filenames are OK */
				}
			}
		}
	}
	return Allowed;		/* Return the flag */
}

/**********************************

	Given a drive number, return in generic format
	the drive's name.

**********************************/

char * BURGERCALL GetAVolumeName(Word DriveNum)
{
	char *Result;		/* Return value */
	Regs16 Regs;		/* Intel registers */
	Word16 OldOff,OldSeg;	/* Previous DTA address */
	Word32 RealBuffer;	/* Real memory offset */
	char *RealPtr;			/* Protected real memory pointer */

	if (DriveNum>=26) {		/* Bad drive number!! */
		NonFatal("Invalid DOS drive number!\n");
		return 0;
	}

	Regs.ax = 0x2F00;		/* Get DTA address */
	Int86x(0x21,&Regs,&Regs);	/* Call DOS */
	OldOff = Regs.bx;		/* Save the old DTA address for later restoration */
	OldSeg = Regs.es;

	RealBuffer = GetRealBufferPtr();	/* Get some real memory */
	RealPtr = static_cast<char *>(RealToProtectedPtr(RealBuffer));	/* Convert to true pointer */

	Regs.ax = 0x1A00;		/* Set the DTA address */
	Regs.dx = (Word16)RealBuffer;
	Regs.ds = (Word16)(RealBuffer>>16);
	Int86x(0x21,&Regs,&Regs);	/* Call DOS */

	strcpy(RealPtr+256,"C:\*.*");	/* Copy the search string for labels */
	RealPtr[256] = static_cast<char>('A' + DriveNum);	/* Set the drive letter AFTER the fact */

	Regs.ax = 0x4e00;		/* Find first */
	Regs.cx = 0x0008;		/* Only look for volume labels */
	Regs.dx = (Word16)RealBuffer+256;	/* Pointer to search string */
	Regs.ds = (Word16)(RealBuffer>>16);
	Int86x(0x21,&Regs,&Regs);	/* Call DOS */
	if (Regs.flags&1) {		/* Error (No volume name) */
		goto Untitled;
	}

	RealPtr = RealPtr+30;	/* Pointer to the volume name */
	RealPtr[8] = RealPtr[9];		/* Remove the period for an 8.3 filename */
	RealPtr[9] = RealPtr[10];
	RealPtr[10] = RealPtr[11];
	RealPtr[11] = 0;		/* Make SURE it's terminated! */
	RealBuffer = strlen(RealPtr);	/* Size of the string */
	if (RealBuffer) {
		--RealPtr;			/* Prepend the colon */
		RealPtr[0] = ':';		/* Colon prefix */
		++RealBuffer;		/* Increase the length because of the colon */
	} else {
Untitled:
		RealPtr = ":UntitledA";		/* Generic */
		RealPtr[9] = static_cast<char>('A' + DriveNum);
		RealBuffer = 10;
	}
	Result = static_cast<char *>(AllocAPointer(RealBuffer+2));	/* Return buffer */
	if (Result) {
		FastMemCpy(Result,RealPtr,RealBuffer);	/* Result */
		Result[RealBuffer] = ':';		/* Colon suffix */
		Result[RealBuffer+1] = 0;		/* End the string */
	}

	Regs.ax = 0x1A00;		/* Restore the DTA address to the old value */
	Regs.ds = OldSeg;
	Regs.dx = OldOff;
	Int86x(0x21,&Regs,&Regs);	/* Call DOS */
	return Result;		/* Return my result */
}

/**********************************

	Return the filesize of a file stream

**********************************/

Word32 BURGERCALL fgetfilesize(FILE *fp)
{
	long Length;
	Length = filelength(fileno(fp));		/* Fast version for INTEL */
	if (Length!=-1) {
		return Length;
	}
	return 0;
}

/**********************************

	Set the initial default prefixs for a power up state
	*: = Boot volume
	$: = System folder
	@: = Prefs folder
	8: = Default directory
	9: = Application directory

**********************************/

#if defined(__WATCOMC__)
#ifdef __cplusplus
extern "C" {
#endif
extern char **_argv;		/* Used for Intel versions */
#ifdef __cplusplus
}
#endif
#endif

void BURGERCALL SetDefaultPrefixs(void)
{
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

	SetAPrefix(PREFIXBOOT,".D2");		/* Assume C: is the boot volume */
	SetAPrefix(PREFIXSYSTEM,"*:DOS");	/* C:\DOS */
	SetAPrefix(PREFIXPREFS,"8:");		/* Place prefs in the data folder */
}

/**********************************

	DOS version
	There is no volume name support in DOS
	Paths without a leading '\' are prefixed with
	the current working directory
	Paths with a drive letter but no leading \ will use
	the drive's current working directory
	If it a network path "\\" then dispose of the drive
	letter and use the pathname.
	The DOS version converts these types of paths

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
		Word DriveNum;		/* Logged drive number */
		Regs16 Regs;		/* Used for DOS calls */

		Temp = OldPath[0];		/* Get the possible drive letter */
		if (Temp && OldPath[1]==':') {	/* Could this be a drive letter? */
			if (Temp>='a' && Temp<'z'+1) {	/* Upper case */
				Temp &= 0xDF;
			}
			if (Temp>='A' && Temp<'Z'+1) {	/* Only A-Z are valid drive letters */
				DriveNum = Temp-'A';
				OldPath += 2;			/* Accept the C: */
				goto GotDriveNum;
			}
		}
		_dos_getdrive(&DriveNum);	/* Get the default drive number */
		--DriveNum;

	/* At this point I have the drive number, create the drive number prefix */

	GotDriveNum:;
		Output[0] = '.';		/* .D2 for C: */
		Output[1] = 'D';
		LongWordToAscii(DriveNum,&Output[2]);
		Output = Output+strlen(Output);
		Output[0] = ':';		/* Append a colon */
		++Output;

	/* Now if the path starts with a '\' then I assume it's fully qualified */
	/* otherwise, I must insert the working directory for the drive */
	/* I assume that DriveNum has the current requested drive number */

		if (OldPath[0]=='\\') {
			++OldPath;			/* Remove the delimiter and go to the next section */
			if (OldPath[0]=='\\') {		/* Network name */
				++OldPath;		/* Remove the second slash */
				Output=TempPath;	/* Reset the volume name */
				Output[0] = ':';	/* Insert volume name header */
				++Output;
			}
		} else {
			Word32 DosBuffer;		/* Real memory pointer to buffer */
			char *DosPtr;			/* Protected memory pointer to buffer */

			Regs.dx = static_cast<Word16>(DriveNum+1);	/* Requested drive */
			DosBuffer = GetRealBufferPtr();	/* Get real memory buffer */
			Regs.ds = (Word16) (DosBuffer>>16);	/* Pass to Dos call */
			Regs.si = (Word16) (DosBuffer&0xFFFF);
			Regs.ax = 0x7147;		/* First try long version */
			Int86x(0x21,&Regs,&Regs);	/* Call DOS */
			if (Regs.flags&1) {			/* Carry set?? */
				Regs.ax = 0x4700;		/* Try DOS 2.0 version */
				Int86x(0x21,&Regs,&Regs);	/* Get the working directory */
				if (Regs.flags&1) {
					goto NextState;
				}
			}
			DosPtr = static_cast<char *>(RealToProtectedPtr(DosBuffer));	/* Convert to my pointer */
			Temp = DosPtr[0];		/* Prefix the path */
			if (Temp) {
				do {
					if (Temp=='\\') {	/* Convert directory holders */
						Temp = ':';		/* To generic paths */
					}
					Output[0] = static_cast<char>(Temp);	/* Save char */
					++Output;
					++DosPtr;			/* Accept char */
					Temp = DosPtr[0];	/* Next char */
				} while (Temp);			/* Still more? */
				Output[0] = ':';			/* Add a final colon after the path */
				++Output;
			}
		}
	NextState:;

	/* Now, just copy the rest of the path */
	/* I'll convert all backslashes to colons */

		Temp = OldPath[0];
		if (Temp) {				/* Any more? */
			do {
				if (Temp=='\\') {	/* Convert directory holders */
					Temp = ':';		/* To generic paths */
				}
				Output[0] = static_cast<char>(Temp);	/* Save char */
				++Output;
				++OldPath;			/* Accept char */
				Temp = OldPath[0];	/* Next char */
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
