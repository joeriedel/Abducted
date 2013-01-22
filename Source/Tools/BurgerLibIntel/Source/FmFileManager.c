#include "FmFile.h"
#include <BREndian.hpp>
#include "MmMemory.h"
#include "ClStdLib.h"
#include "StString.h"
#include <stdlib.h>
#include "McMac.h"

#ifdef __cplusplus
extern "C" {
static void ANSICALL ReleasePrefixs(void);
}
#endif

/**********************************

	Prefix handle array

**********************************/

void ** PrefixHandles[PREFIXMAX];

/**********************************

	Release memory on shutdown

**********************************/

static Bool SetPfxInited;

static void ANSICALL ReleasePrefixs(void)
{
	Word i;
	void ***Table;
	i = 0;
	Table = PrefixHandles;
	do {
		DeallocAHandle(Table[0]);	/* Release the memory */
		Table[0] = 0;				/* Gone */
		++Table;
	} while (++i<PREFIXMAX);
}



/**********************************

	This routine will get the modification time and date
	from a file.

**********************************/

Word BURGERCALL GetFileModTime(const char *FileName,TimeDate_t *Output)
{
	char PathName[FULLPATHSIZE];

	ExpandAPathToBufferNative(PathName,FileName);	/* Convert to native pathname */
	return GetFileModTimeNative(PathName,Output);	/* Perform the operation */
}

/**********************************

	This routine will get the time and date
	from a file.
	Note, this routine is Operating system specfic!!!

**********************************/

#if !defined(__MSDOS__) && !defined(__MAC__) && !defined(__WIN32__) && !defined(__MACOSX__)
Word BURGERCALL GetFileModTimeNative(const char *FileName,TimeDate_t *Output)
{
	return TRUE;		/* Error! */
}
#endif

/**********************************

	This routine will get the creation time and date
	from a file.

**********************************/

Word BURGERCALL GetFileCreateTime(const char *FileName,TimeDate_t *Output)
{
	char PathName[FULLPATHSIZE];

	ExpandAPathToBufferNative(PathName,FileName);	/* Convert to native pathname */
	return GetFileCreateTimeNative(PathName,Output);	/* Perform the operation */
}

/**********************************

	This routine will get the time and date
	from a file.
	Note, this routine is Operating system specfic!!!

**********************************/

#if !defined(__MSDOS__) && !defined(__MAC__) && !defined(__WIN32__) && !defined(__MACOSX__)
Word BURGERCALL GetFileCreateTimeNative(const char *FileName,TimeDate_t *Output)
{
	return TRUE;		/* Error! */
}
#endif

/**********************************

	Check if a file exists, if so, then
	return TRUE

**********************************/

Word BURGERCALL DoesFileExist(const char *FileName)
{
	char PathName[FULLPATHSIZE];
	
	ExpandAPathToBufferNative(PathName,FileName);
	return DoesFileExistNative(PathName);
}

/**********************************

	Determine if a file exists.
	I will return TRUE if the specified path
	is a path to a file that exists, if it doesn't exist
	or it's a directory, I return FALSE.
	Note : I do not check if the file havs any data in it.
	Just the existance of the file.

**********************************/

#if !defined(__MSDOS__) && !defined(__MAC__) && !defined(__WIN32__) && !defined(__MACOSX__)
Word BURGERCALL DoesFileExistNative(const char *FileName)
{
	FILE *fp;

	fp = fopen(FileName,"rb");	/* Get file info */
	if (!fp) {
		return FALSE;		/* Bad file! */
	}
	fclose(fp);
	return TRUE;		/* File exists */
}

#endif

/**********************************

	Compare two FileDate_t records and
	return -1 if the first is less than the second.
	return 0 if they are the same and 1 if the second
	is less than the first.

**********************************/

int BURGERCALL CompareTimeDates(const TimeDate_t *First,const TimeDate_t *Second)
{
	if (First->Year!=Second->Year) {	/* Check years */
		goto TestYear;
	}
	if (First->Month!=Second->Month) {	/* Check months */
		goto TestMonth;
	}
	if (First->Day!=Second->Day) {		/* Check days */
		goto TestDay;
	}
	if (First->Hour!=Second->Hour) {	/* Check hours */
		goto TestHour;
	}
	if (First->Minute!=Second->Minute) {	/* Check minutes */
		goto TestMinute;
	}
	if (First->Second!=Second->Second) {	/* Check seconds */
		goto TestSecond;
	}
	if (First->Milliseconds!=Second->Milliseconds) {
		goto TestMilli;
	}
	return 0;		/* They are a match!! */

TestYear:
	if (First->Year<Second->Year) {	/* Test for less or more */
		goto Less;
	}
	goto More;

TestMonth:
	if (First->Month<Second->Month) {
		goto Less;
	}
	goto More;

TestDay:
	if (First->Day<Second->Day) {
		goto Less;
	}
	goto More;

TestHour:
	if (First->Hour<Second->Hour) {
		goto Less;
	}
	goto More;

TestMinute:
	if (First->Minute<Second->Minute) {
		goto Less;
	}
	goto More;

TestSecond:
	if (First->Second<Second->Second) {
		goto Less;
	}
	goto More;

TestMilli:
	if (First->Milliseconds<Second->Milliseconds) {
Less:
		return -1;		/* First is < than Second */
	}
More:;
	return 1;		/* First is > than Second */
}

/**********************************

	Create a directory path using a Burgerlib path name
	Return FALSE if successful, or TRUE if an error

**********************************/

Word BURGERCALL CreateDirectoryPath(const char *FileName)
{
	char PathName[FULLPATHSIZE];

	ExpandAPathToBufferNative(PathName,FileName);	/* Convert to native path */
	return CreateDirectoryPathNative(PathName);		/* Create the path */
}

/**********************************

	Create a directory path using a Burgerlib path name
	but exclude the LAST entry so that I can make the directory
	tree but exclude the filename for the end of the directory
	Return FALSE if successful, or TRUE if an error

**********************************/

Word BURGERCALL CreateDirectoryPath2(const char *FileName)
{
	char PathName[FULLPATHSIZE];
	char FooName[FULLPATHSIZE];
	char *TempPtr;
	
	strncpy(FooName,FileName,FULLPATHSIZE-1);		/* Make a copy of the string */
	FooName[FULLPATHSIZE-1]=0;
	TempPtr = strrchr(FooName,':');					/* Is there a filename? */
	if (!TempPtr) {
		ExpandAPathToBufferNative(PathName,FileName);	/* Convert to native path as is */
	} else {
		TempPtr[0] = 0;									/* Kill the colon */
		ExpandAPathToBufferNative(PathName,FooName);	/* Convert to native path without filename */
	}
	return CreateDirectoryPathNative(PathName);		/* Create the path */
}

/**********************************

	Create a directory path using an operating system native name
	Return FALSE if successful, or TRUE if an error

**********************************/

#if !defined(__MSDOS__) && !defined(__MAC__) && !defined(__WIN32__) && !defined(__MACOSX__)
Word BURGERCALL CreateDirectoryPathNative(const char *FileName)
{
	return TRUE;
}
#endif

/**********************************

	Open a directory for scanning
	Return an error if the directory doesn't exist

**********************************/

#if !defined(__MSDOS__) && !defined(__MAC__) && !defined(__WIN32__) && !defined(__MACOSX__)
Word BURGERCALL OpenADirectory(DirectorySearch_t *Input,const char *Name)
{
	return TRUE;		/* Error! */
}
#endif

/**********************************

	Get the next directory entry
	Return FALSE if the entry is valid, TRUE if
	an error occurs.

**********************************/

#if !defined(__MSDOS__) && !defined(__MAC__) && !defined(__WIN32__) && !defined(__MACOSX__)
Word BURGERCALL GetADirectoryEntry(DirectorySearch_t *Input)
{
	return TRUE;		/* Error! */
}
#endif

/**********************************

	Get the next directory entry and check it's
	file extension. If a match is found return FALSE.
	Otherwise keep searching for a match and return TRUE if
	one cannot be found.

**********************************/

Word BURGERCALL GetADirectoryEntryExtension(DirectorySearch_t *Input,const char *ExtPtr)
{
	if (!GetADirectoryEntry(Input)) {			/* Get an entry */
		do {
			if (!Input->Dir && Input->FileSize) {		/* Non-directory files */
				char *MyExtPtr;
				MyExtPtr = StrGetFileExtension(Input->Name);	/* Get the extension */
				if (MyExtPtr) {
					if (!stricmp(MyExtPtr,ExtPtr)) {	/* Match the extension? */
						return FALSE;			/* I got a match */
					}
				}
			}
		} while (!GetADirectoryEntry(Input));
	}
	return TRUE;
}

/**********************************

	Close a directory that's being scanned

**********************************/

#if !defined(__MSDOS__) && !defined(__MAC__) && !defined(__WIN32__) && !defined(__MACOSX__)
void BURGERCALL CloseADirectory(DirectorySearch_t *Input)
{
}
#endif

/**********************************

	Open a file using a native path

**********************************/

#if !defined(__MAC__) && !defined(__MACOSX__)
FILE * BURGERCALL OpenAFile(const char *FileName,const char *Type)
{
	char NewFileName[FULLPATHSIZE];		/* New filename from ExpandAPathToBufferNative */

	ExpandAPathToBufferNative(NewFileName,FileName);	/* Expand a filename */
	return fopen(NewFileName,Type);		/* Open using standard fopen */
}
#endif

/**********************************

	Copy a file using full pathnames

**********************************/

Word BURGERCALL CopyAFile(const char *DestName,const char *SourceName)
{
	char Dest[FULLPATHSIZE];
	char Src[FULLPATHSIZE];

	ExpandAPathToBufferNative(Dest,DestName);		/* Expand the path to a full filename */
#if defined(__MAC__)
	MacVRefNum2 = MacVRefNum;	/* The mac needs these hidden vars */
	MacDirID2 = MacDirID;
#endif
	ExpandAPathToBufferNative(Src,SourceName);	/* Expand the source path */
	return CopyAFileNative(Dest,Src);		/* Copy the file */
}

/**********************************

	Copy a file using native pathnames

**********************************/

#if !defined(__MAC__) && !defined(__MACOSX__)
Word BURGERCALL CopyAFileNative(const char *DestName,const char *SourceName)
{
	FILE *f1,*f2;
	Word32 l;
	Word8 *buffer;
	Word Result;
		
	Result = TRUE;			/* Assume error */
	buffer = (Word8 *)AllocAPointer(65536);
	if (buffer) {
		f1 = fopen(SourceName,"rb");		/* Open the source file */
		if (f1) {
			f2 = fopen(DestName,"wb");		/* Open the dest file */
			if (f2) {
				Word32 Length;
				Length = fgetfilesize(f1);	/* Get the size of the source file */
				if (Length) {				/* Shall I copy anything? */
					do {
						Word32 Chunk;
						Chunk = Length;		/* Base chunk */
						if (Chunk>65536) {
							Chunk = 65536;		/* Only copy the chunk */
						}
						l = fread(buffer,1,Chunk,f1);		/* Read data */
						if (l!=Chunk) {
							break;
						}
						l = fwrite(buffer,1,Chunk,f2);		/* Write data */
						if (l!=Chunk) {
							break;
						}
						Length -= Chunk;
					} while (Length);			/* Any data left? */
				}
				if (!Length) {					/* All data copied? */
					Result = FALSE;				/* No error (So far) */
				}
				if (fclose(f2)) {		/* Did the file have an error in closing? */
					Result = TRUE;		/* Crap. */
				}
			}
			fclose(f1);					/* Close the source file */
		}
		DeallocAPointer(buffer);		/* Release the copy buffer */
	}
	return Result;
}
#endif

/**********************************

	Delete a file using full pathnames

**********************************/

Word BURGERCALL DeleteAFile(const char *FileName)
{
	char Dest[FULLPATHSIZE];

	ExpandAPathToBufferNative(Dest,FileName);		/* Expand the path to a full filename */
	return DeleteAFileNative(Dest);		/* Copy the file */
}

/**********************************

	Delete a file using native file system

**********************************/

#if !defined(__MAC__) && !defined(__WIN32__) && !defined(__MSDOS__) && !defined(__MACOSX__)
Word BURGERCALL DeleteAFileNative(const char *FileName)
{
	remove(FileName);
	return TRUE;
}
#endif

/**********************************

	Rename a file using full pathnames

**********************************/

Word BURGERCALL RenameAFile(const char *NewName,const char *OldName)
{
	char Dest[FULLPATHSIZE];
	char Src[FULLPATHSIZE];

	ExpandAPathToBufferNative(Dest,NewName);		/* Expand the path to a full filename */
#if defined(__MAC__)
	MacVRefNum2 = MacVRefNum;	/* The mac needs these hidden vars */
	MacDirID2 = MacDirID;
#endif
	ExpandAPathToBufferNative(Src,OldName);	/* Expand the source path */
	return RenameAFileNative(Dest,Src);		/* Rename or move the file */
}

#if !defined(__MAC__) && !defined(__MACOSX__)
Word BURGERCALL RenameAFileNative(const char *DestName,const char *SourceName)
{
	rename(SourceName,DestName);
	return TRUE;
}
#endif

/**********************************

	This is tricky.

	Using the rules for a GS/OS type pathname, expand a path
	into a FULL pathname native to the GS/OS file system.

	Directory delimiters are colons only.
	If the path starts with a colon, then it is a full pathname starting with a volume name.
	If the path starts with ".D" then it is a full pathname starting with a drive number.
	If the path starts with a "$:","*:" or "@:" then use special prefix numbers 32-34
	If the path starts with 0: through 31: then use prefix 0-31.
	Otherwise prepend the pathname with the contents of prefix 8 ("Default")

	If the path after the prefix is removed is a period then POP the number of
	directories from the pathname for each period present after the first.
	Example "..:PrevDir:File:" will go down one directory and up the directory PrevDir

	All returned pathnames will have a trailing colon

**********************************/

void BURGERCALL ExpandAPathToBuffer(char *DestPath,const char *FileName)
{
	Word Length;		/* Length of the input filename */
	Word Index;			/* Number if FileName chars to discard since it's a prefix */
	Word PrefixNum;		/* Prefix to prepend */
	void **PrefixHand;	/* Handle to a prefix */

	PrefixHand = 0;			/* Make SURE it's inited! */
	if (!FileName) {		/* Prevent a crash by using a bogus pathname */
		FileName = "";		/* for a passed null pointer */
	}

	Length = strlen(FileName);		/* How long is the pathname? */
	if (!Length) {			/* Any data input? */
		goto DefaultPrefix;	/* Prepend... */
	}

	/* There is data in the pathname, check if it is a prefix */

	PrefixNum = ((Word8 *)FileName)[0];	/* Get the first char of the filename */
	Index = 1;			/* Assume the prefix string length is 1 (A zero) */

	/* I use the input path as is... */

	if (PrefixNum==':') {	/* Full pathname starting with volume name? */
		goto AsIs;			/* Don't prepend a prefix!! */
	}
	if (PrefixNum=='.' && (((Word8 *)FileName)[1]=='D'||((Word8 *)FileName)[1]=='d')) {	/* Drive number? */
		goto AsIs;
	}

/* Determine which prefix to use, if any */

	if (PrefixNum=='$') {		/* System folder? */
		PrefixNum = PREFIXSYSTEM;
	} else if (PrefixNum=='*') {	/* Boot volume? */
		PrefixNum = PREFIXBOOT;
	} else if (PrefixNum=='@') {	/* Prefs folder? */
		PrefixNum = PREFIXPREFS;
	} else if (PrefixNum<'0' || PrefixNum>=('9'+1)) {		/* Prefix 0-31 */
DefaultPrefix:;
		PrefixNum = 8;		/* Use the default prefix */
		goto GotPrefix;
	} else {			/* Decode prefix 0-31 */
		PrefixNum = PrefixNum&0xF;		/* Isolate the prefix (0-9) */
		for (;;) {
			Word Temp;
			if ((Length==Index) ||		/* At the end? */
				((Temp = ((Word8 *)FileName)[Index])==':')) {	/* End the prefix here? */
				if (PrefixNum>=PREFIXMAX) {	/* Is the prefix valid? */
					goto DefaultPrefix;	/* Filename */
				}
				break;		/* It's good! */
			}
			if (Temp<'0' || Temp>=('9'+1)) {	/* Valid digit? */
				goto DefaultPrefix;		/* It's a filename that starts with a number */
			}
			PrefixNum = PrefixNum*10;	/* Shift up the previous number */
			Temp = Temp&0xF;			/* 0-9 */
			PrefixNum = PrefixNum+Temp;	/* Total prefix number */
			++Index;					/* Next value */
		}
	}

/* Remove the prefix ascii */
/* Index >=1 */

	if (Length!=Index) {		/* If there are chars left (Test for $,*,@) */
		if (((Word8 *)FileName)[Index]==':') {	/* Must be followed by a colon */
			++Index;		/* Remove the colon */
		}
	}
	Length = Length-Index;			/* Zap the prefix length */
	FileName = FileName+Index;		/* Zap the prefix char(s) */

/* Now I have a prefix number, let's prepend the prefix to the path */
/* Requires PrefixNum to be a valid prefix number */

GotPrefix:;
	Index = 1;			/* Assume only a zero byte ("C" end byte) (Empty prefix) */
	PrefixHand = PrefixHandles[PrefixNum];		/* Is there a prefix attached? */
	if (PrefixHand) {		/* Nope... */
		Index = GetAHandleSize(PrefixHand);		/* How many bytes for the prefix? */
	}

/* If the filename starts with a period, then remove it and pop the prefix */
/* for each following period */
/* Note : I assume that a NULL PrefixHand is also joined by Index=1 */

	if (((Word8 *)FileName)[0] == '.') {	/* Starts with a period */
		++FileName;		/* Discard the period */
		--Length;		/* Remove from length */
		--Index;		/* Remove final zero from prefix */
		while (((Word8 *)FileName)[0]=='.') {	/* Shall I pop a prefix? */
			if (Index) {			/* Prefix already empty? */
				Word8 *PrefixPtr;

				--Index;			/* Index to the last char */
				PrefixPtr = (Word8 *)(*PrefixHand);
				do {
					--Index;		/* Go to previous char */
				} while ((Index!=(Word)-1) && PrefixPtr[Index]!=':');
				++Index;		/* Grab the final colon */
			}
			++FileName;		/* Discard the period */
			--Length;		/* Shorten the length */
		}
		if (((Word8 *)FileName)[0]==':') {		/* Discard any colon after the periods */
			++FileName;
			--Length;
		}
		++Index;		/* Restore final zero for new prefix */
	}
/* Now let's prepend the prefix (If any) and the pathname to a result */
/* Requires Index==1 to not use a prefix */

AsIs:;

	/* At this point Length == length of the filename string to append */
	/* Index = length of the prefix WITH the trailing zero */

	--Index;		/* Remove the trailing zero */
	if (Index && PrefixHand) {	/* Any ASCII? */
		FastMemCpy(DestPath,PrefixHand[0],Index);	/* Copy the prefix */
	}
	if (Length) {	/* Any text for the path? */
		FastMemCpy(&DestPath[Index],FileName,Length);	/* Copy the path AFTER the prefix */
	}
	Index = Index+Length;		/* Get the end of the path */
	if (Index) {			/* Is there any text? */
		if (((Word8 *)DestPath)[Index-1] != ':') {	/* Trailing colon? */
			DestPath[Index] = ':';		/* End the path with a colon */
			++Index;			/* +1 to the length */
		}
	}
	DestPath[Index] = 0;		/* Add the terminating zero */
}

/**********************************

	This routine will take a generic path, expand it fully
	and then set the current working directory to the highest
	allowable directory I can attain without error. This is so that
	I can get around the limitation of 256 character pathnames on some
	operating systems (MacOS for 255 PStrings, DOS for 264 total pathname length)

**********************************/

#if !defined(__MSDOS__) && !defined(__MAC__) && !defined(__WIN32__) && !defined(__BEOS__) && !defined(__MACOSX__)
void BURGERCALL ExpandAPathToBufferNative(char *Output,const char *FileName)
{
	char PathName[FULLPATHSIZE];
	char *FilePtr;		/* Pointer to input buffer */
	Word8 Temp;

	ExpandAPathToBuffer(PathName,FileName);		/* Resolve prefixes */
	
	FilePtr = (char *)PathName;		/* Copy to running pointer */
	Temp = ((Word8 *)FilePtr)[0];
	if (Temp) {
		do {
			++FilePtr;
			if (Temp==':') {	/* Convert to a UNIX format */
				Temp = '/';
			}
			Output[0] = (char)Temp;
			++Output;
			Temp = ((Word8 *)FilePtr)[0];
		} while (Temp);
		Output-=2;
		if (((Word8 *)Output)[0]!='/') {
			++Output;		/* Remove trailing slash */
		}
	}
	Output[0] = 0;			/* Terminate the "C" string */
}
#endif

/**********************************

	Expand a path to a full qualified Burgerlib path.
	Make a copy of the string and return the copy

**********************************/

char * BURGERCALL ExpandAPath(const char *FileName)
{
	char PathName[FULLPATHSIZE];
	
	ExpandAPathToBuffer(PathName,FileName);
	return StrCopy(PathName);
}

/**********************************

	Expand a path to a full qualified Burgerlib path.
	Make a copy of the string and return the copy

**********************************/

char * BURGERCALL ExpandAPathNative(const char *FileName)
{
	char PathName[FULLPATHSIZE];
	
	ExpandAPathToBufferNative(PathName,FileName);
	return StrCopy(PathName);
}


/**********************************

	Return the contents of a prefix
	I MUST return a valid pointer, even if I only can return a NULL string.
	Returning a zero means an error occured (Out of memory)

**********************************/

char * BURGERCALL GetAPrefix(Word PrefixNum)
{
	void **TempHand;		/* Prefix handle */
	void *TempPtr;			/* Pointer to result buffer */
	Word32 Length;		/* Size of result buffer */

	if (PrefixNum<PREFIXMAX) {		/* Is the prefix valid? */
		TempHand = PrefixHandles[PrefixNum];	/* Get the handle */
		if (!TempHand) {			/* No data in the prefix? */
			return (char *)AllocAPointerClear(1);	/* Allocate a 1 byte pointer (Cleared) */
		}
		Length = GetAHandleSize(TempHand);		/* How much memory? */
		TempPtr = AllocAPointer(Length);		/* Get the memory */
		if (TempPtr) {			/* Memory no gotten? */
			FastMemCpy(TempPtr,*TempHand,Length);	/* Copy the memory */
		}
		return (char *)TempPtr;		/* Return the prefix pointer (Or zero) */
	}
	NonFatal("Invalid prefix number\n");		/* FooBar */
	return 0;		/* Oh oh... */
}

/**********************************

	Set the contents of a prefix

**********************************/

Word BURGERCALL SetAPrefix(Word PrefixNum,const char *PrefixName)
{
	void **TempHand;
	char NewPath[FULLPATHSIZE];

	if (!SetPfxInited) {
		SetPfxInited = TRUE;
		atexit(ReleasePrefixs);
	}

	if (PrefixNum<PREFIXMAX) {		/* Is the prefix valid? */
		TempHand = 0;		/* Zap the prefix */
		if (!PrefixName || !((Word8 *)PrefixName)[0]) {		/* Valid prefix? */
			goto NewPfx;
		}
		ExpandAPathToBuffer(NewPath,PrefixName);	/* Convert to full pathname */
		if (!((Word8 *)NewPath)[0]) {					/* Blank string? */
			goto NewPfx;				/* Kill the prefix */
		}
		TempHand = StrCopyHandle(NewPath);	/* Get HANDLE for new prefix */
		if (TempHand) {					/* Did I get the memory? */
NewPfx:
			DeallocAHandle(PrefixHandles[PrefixNum]);	/* Was there a path already? */
			PrefixHandles[PrefixNum] = TempHand;	/* Save the new handle */
			return 0;					/* No error */
		}
		return (Word)-1;		/* Can't finish operation */
	}
	NonFatal("Invalid prefix number\n");		/* FooBar */
	return (Word)-1;		/* Oh oh... */
}


/**********************************

	Remove the last entry of a prefix
	(Moves up one sub-directory)
	I assume that GetAPrefix will return me either a null pointer
	or a pointer to a string that ends with a ':'

**********************************/

void BURGERCALL PopAPrefix(Word PrefixNum)
{
	char *TempPtr;		/* Pointer to prefix */
	Word i;				/* Temp index */
	char *WorkPtr;		/* Work pointer */

	TempPtr = GetAPrefix(PrefixNum);	/* Get the current prefix */
	if (TempPtr) {				/* Valid? */
		i = strlen(TempPtr);	/* Get the length of the string */
		if (i<3) {			/* Too short? */
			TempPtr[0] = 0;	/* Force a null string */
		} else {			/* ":A:" is the shortest I can accept */
			i-=2;					/* Index to the final char */
			WorkPtr = TempPtr+i;	/* Point to the final char (Skip last colon) */
			do {
				if (((Word8 *)WorkPtr)[0] == ':') {	/* Delimiter? */
					break;		/* Break out of the loop and save */
				}
			} while (--WorkPtr!=TempPtr);	/* At the beginning? */
			WorkPtr[0] = 0;		/* Terminate the string */
		}
		SetAPrefix(PrefixNum,TempPtr);	/* Set the new prefix */
		DeallocAPointer(TempPtr);		/* Discard of the old pointer */
	}
}

/**********************************

	Change a directory using long filenames
	This only accepts Native OS filenames

**********************************/

#if !defined(__MSDOS__) && !defined(__MAC__) && !defined(__WIN32__) && !defined(__MACOSX__)

Word BURGERCALL ChangeADirectory(const char *DirName)
{
	return (Word)-1;	/* Error! */
}

#endif

/**********************************

	Write a longword to a file stream

**********************************/

void BURGERCALL fwritelong(Word32 Val,FILE *fp)
{
	fwrite(&Val,1,4,fp);		/* Save the long word */
}

/**********************************

	Write a longword to a file stream

**********************************/

void BURGERCALL fwritelongrev(Word32 Val,FILE *fp)
{
	Word32 Temp;
	Temp = Burger::SwapEndian(Val);
	fwrite(&Temp,1,4,fp);		/* Save the long word */
}


/**********************************

	Write a 16 bit short to a file stream

**********************************/

void BURGERCALL fwriteshort(Word16 Val,FILE *fp)
{
	fwrite(&Val,1,2,fp);		/* Save the long word */
}
/**********************************

	Write a 16 bit short to a file stream

**********************************/

void BURGERCALL fwriteshortrev(Word16 Val,FILE *fp)
{
	Word16 Temp;
	Temp = Burger::SwapEndian(Val);
	fwrite(&Temp,1,2,fp);		/* Save the long word */
}
/**********************************

	Write a "C" string with the terminating zero to a file stream

**********************************/

void BURGERCALL fwritestr(const char *Val,FILE *fp)
{
	fwrite(Val,1,strlen(Val)+1,fp);		/* Save the long word */
}

/**********************************

	Read a longword from a file stream

**********************************/

Word32 BURGERCALL fgetlong(FILE *fp)
{
	Word32 Val;
	fread(&Val,1,4,fp);		/* Save the long word */
	return Val;
}
/**********************************

	Read a longword from a file stream

**********************************/

Word32 BURGERCALL fgetlongrev(FILE *fp)
{
	Word32 Val;
	fread(&Val,1,4,fp);		/* Save the long word */
	return Burger::SwapEndian(Val);
}
/**********************************

	Reade a short from a file stream

**********************************/

short BURGERCALL fgetshort(FILE *fp)
{
	short Val;
	fread(&Val,1,2,fp);		/* Save the long word */
	return Val;
}
/**********************************

	Reade a short from a file stream

**********************************/

short BURGERCALL fgetshortrev(FILE *fp)
{
	short Val;
	fread(&Val,1,2,fp);		/* Save the long word */
	return Burger::SwapEndian(Val);
}
/**********************************

	Read a "C" string from a file stream

**********************************/

Word BURGERCALL fgetstr(char *Input,Word Length,FILE *fp)
{
	Word Val;
	char *EndPtr;

	EndPtr = Input;	/* Set the maximum buffer size */
	EndPtr = EndPtr+Length;
	--EndPtr;
	for (;;) {		/* Stay until either zero or EOF */
		Val = fgetc(((FILE *)fp));		/* Get the char from the file */
		if (!Val || Val==(Word)EOF) {		/* Exit? */
			break;
		}
		if (Input<EndPtr) {		/* Allowable here? */
			Input[0] = static_cast<char>(Val);
			++Input;		/* Inc the input pointer */
		}
	}
	if (Length) {		/* Any space in buffer? */
		Input[0] = 0;	/* Add ending zero */
	}
	if (Val) {		/* EOF? */
		return FALSE;
	}
	return TRUE;	/* Natural zero */
}

/**********************************

	Save a file to disk

**********************************/

Word BURGERCALL SaveAFile(const char *FileName,const void *Input,Word32 Length)
{
	FILE *fp;

	fp = OpenAFile(FileName,"wb");	/* Open the file */
	if (fp) {
		return SaveAFileFP(fp,Input,Length);	/* Save using the reference */
	}
	NonFatal("Can't open the file to save to\n");	/* Can't open the file */
	return FALSE;
}

/**********************************

	Save a file to disk

**********************************/

Word BURGERCALL SaveAFileNative(const char *FileName,const void *Input,Word32 Length)
{
	FILE *fp;

	fp = fopen(FileName,"wb");	/* Open the file */
	if (fp) {
		return SaveAFileFP(fp,Input,Length);
	}
	NonFatal("Can't open the file to save to\n");
	return FALSE;
}

/**********************************

	Save a file to disk using a previously opened file

**********************************/

Word BURGERCALL SaveAFileFP(FILE *Filefp,const void *Input,Word32 Length)
{
	Word32 NewLength;
	int Err;

	if (Filefp) {		/* Valid file structure? */
		NewLength = fwrite(Input,1,Length,Filefp);	/* Write the file contents */
		Err = fclose(Filefp);		/* Close the file */
		if (NewLength==Length && !Err) {	/* Error? */
			return TRUE;		/* It's ok! */
		}
	}
	NonFatal("Error in saving file contents\n");
	return FALSE;		/* I didn't save it right... */
}

/**********************************

	Save a file to disk

**********************************/

Word BURGERCALL SaveATextFile(const char *FileName,const void *Input,Word32 Length)
{
	FILE *fp;

	fp = OpenAFile(FileName,"w");	/* Open the file */
	if (fp) {
		return SaveAFileFP(fp,Input,Length);	/* Save using the reference */
	}
	NonFatal("Can't open the file to save to\n");	/* Can't open the file */
	return FALSE;
}

/**********************************

	Save a file to disk

**********************************/

Word BURGERCALL SaveATextFileNative(const char *FileName,const void *Input,Word32 Length)
{
	FILE *fp;

	fp = fopen(FileName,"w");	/* Open the file */
	if (fp) {
		return SaveAFileFP(fp,Input,Length);
	}
	NonFatal("Can't open the file to save to\n");
	return FALSE;
}

/**********************************

	Load a file to disk

**********************************/

void * BURGERCALL LoadAFile(const char *FileName,Word32 *Length)
{
	FILE *fp;		/* File referance */

	fp = OpenAFile(FileName,"rb");	/* Open the file */
	if (fp) {
#if _DEBUG
		if (DebugTraceFlag&DEBUGTRACE_FILELOAD) {
			DebugXMessage("Loading file %s.\n",FileName);
		}
#endif
		return LoadAFileFP(fp,Length);
	}
	NonFatal("File %s was not found\n",FileName);	/* File doesn't exist */
	if (Length) {		/* Shall I return the file length? */
		Length[0] = 0;	/* Ok */
	}
	return 0;		/* Return the result */
}
/**********************************

	Load a file to disk

**********************************/

void * BURGERCALL LoadAFileNative(const char *FileName,Word32 *Length)
{
	FILE *fp;		/* File referance */

	fp = fopen(FileName,"rb");	/* Open the file */
	if (fp) {
#if _DEBUG
		if (DebugTraceFlag&DEBUGTRACE_FILELOAD) {
			DebugXString("Loading native file ");
			DebugXString(FileName);
			DebugXString(".\n");
		}
#endif
		return LoadAFileFP(fp,Length);
	}
	NonFatal("File not found\n");	/* File doesn't exist */
	if (Length) {		/* Shall I return the file length? */
		Length[0] = 0;	/* Ok */
	}
	return 0;		/* Return the result */
}
/**********************************

	Load a file to disk

**********************************/

void * BURGERCALL LoadAFileFP(FILE *Filefp,Word32 *Length)
{
	Word32 FileLength;		/* Length to load */
	void *Result;

	Result = 0;		/* Init result */
	FileLength = 0;	/* Init length */

	if (Filefp) {	/* Valid file reference */
		FileLength = fgetfilesize(Filefp);		/* Get the file length */
		if (!FileLength) {
			NonFatal("Empty file\n");
		} else {
			Result = AllocAPointer(FileLength);	/* Get memory */
			if (Result) {
				if (fread(Result,1,FileLength,Filefp)!=FileLength) {	/* Write the file contents */
					DeallocAPointer(Result);		/* Release the memory */
					Result = 0;			/* Zap the pointer */
					FileLength = 0;
					NonFatal("Error in loading file\n");
				}
			}
		}
		fclose(Filefp);		/* Close the file */
	}
	if (Length) {		/* Shall I return the file length? */
		Length[0] = FileLength;	/* Ok */
	}
	return Result;		/* Return the result */
}
/**********************************

	Load a file to disk

**********************************/

void ** BURGERCALL LoadAFileHandle(const char *FileName)
{
	FILE *fp;		/* File referance */

	fp = OpenAFile(FileName,"rb");	/* Open the file */
	if (fp) {
		return LoadAFileHandleFP(fp);
	}
	NonFatal("File %s was not found\n",FileName);	/* File doesn't exist */
	return 0;		/* Return the result */
}
/**********************************

	Load a file from disk

**********************************/

void ** BURGERCALL LoadAFileHandleNative(const char *FileName)
{
	FILE *fp;		/* File referance */

	fp = fopen(FileName,"rb");	/* Open the file */
	if (fp) {
		return LoadAFileHandleFP(fp);
	}
	NonFatal("File not found\n");	/* File doesn't exist */
	return 0;		/* Return the result */
}

/**********************************

	Load a file from disk

**********************************/

void ** BURGERCALL LoadAFileHandleFP(FILE *Filefp)
{
	Word32 FileLength;		/* Length to load */
	Word32 ReadIn;
	void **Result;

	Result = 0;		/* Init result */

	if (Filefp) {	/* Valid file reference */
		FileLength = fgetfilesize(Filefp);		/* Get the file length */
		if (!FileLength) {
			NonFatal("Empty file\n");
		} else {
			Result = AllocAHandle(FileLength);	/* Get memory */
			if (Result) {
				ReadIn = fread(LockAHandle(Result),1,FileLength,Filefp);
				UnlockAHandle(Result);
				if (ReadIn!=FileLength) {	/* Write the file contents */
					DeallocAHandle(Result);		/* Release the memory */
					Result = 0;			/* Zap the pointer */
					FileLength = 0;
					NonFatal("Error in loading file\n");
				}
			}
		}
		fclose(Filefp);		/* Close the file */
	}
	return Result;		/* Return the result */
}

/**********************************

	Get a file's Auxtype

**********************************/

Word32 BURGERCALL GetAnAuxType(const char *FileName)
{
	char TempPtr[FULLPATHSIZE];

	ExpandAPathToBufferNative(TempPtr,FileName);	/* Get the true path */
	return GetAnAuxTypeNative(TempPtr);	/* Call the function */
}

/**********************************

	Get a file's FileType

**********************************/

Word32 BURGERCALL GetAFileType(const char *FileName)
{
	char TempPtr[FULLPATHSIZE];

	ExpandAPathToBufferNative(TempPtr,FileName);	/* Get the true path */
	return GetAFileTypeNative(TempPtr);	/* Call the function */
}

/**********************************

	Set a file's AuxType

**********************************/

void BURGERCALL SetAnAuxType(const char *FileName,Word32 AuxType)
{
	char TempPtr[FULLPATHSIZE];
	
	ExpandAPathToBufferNative(TempPtr,FileName);	/* Get the true path */
	SetAnAuxTypeNative(TempPtr,AuxType);
}

/**********************************

	Set a file's FileType

**********************************/

void BURGERCALL SetAFileType(const char *FileName,Word32 FileType)
{
	char TempPtr[FULLPATHSIZE];
	
	ExpandAPathToBufferNative(TempPtr,FileName);	/* Get the true path */
	SetAFileTypeNative(TempPtr,FileType);
}

/**********************************

	Set a file's FileType and Auxtype

**********************************/

void BURGERCALL FileSetFileAndAuxType(const char *FileName,Word32 FileType,Word32 AuxType)
{
	char TempPtr[FULLPATHSIZE];
	
	ExpandAPathToBufferNative(TempPtr,FileName);	/* Get the true path */
	FileSetFileAndAuxTypeNative(TempPtr,FileType,AuxType);
}

/**********************************

	Returns true if long filenames
	are allowed. (That is, filenames don't have to be 8.3)

**********************************/

#if !defined(__MSDOS__)
Word BURGERCALL AreLongFilenamesAllowed(void)
{
	return TRUE;
}
#endif

/**********************************

	Given a drive number, return in generic format
	the drive's name.

**********************************/

#if !defined(__MSDOS__) && !defined(__MAC__) && !defined(__WIN32__) && !defined(__MACOSX__)

char * BURGERCALL GetAVolumeName(Word DriveNum)
{
	return 0;
}

#endif

/**********************************

	Scanning all the dos volumes, I will search for
	a drive that is labeled by the input name.

	Note : I do NOT search floppy drives A: and B:
	this is to prevent an annoying error for bad media.

**********************************/

Word BURGERCALL FindAVolumeByName(const char *VolumeName)
{
	Word i;		/* Drive number */
	Word Result;	/* Compare result */
	Word ErrorState;
	char *TempPtr;	/* Pointer to name returned */

#if defined(__MSDOS__) || defined(__WIN32__)
#define LASTDRIVE 26
	i = 2;		/* Start at drive C: */
#else
#define LASTDRIVE 32
	i = 0;		/* Start at the first drive in the chain */
#endif

	ErrorState = SetErrBombFlag(FALSE);
	do {
		TempPtr = GetAVolumeName(i);	/* Convert to name */
		if (TempPtr) {
			Result = stricmp(TempPtr,VolumeName);	/* Compare */
			DeallocAPointer(TempPtr);		/* Release the pointer */
			if (!Result) {		/* Match? */
				SetErrBombFlag(ErrorState);		/* Restore the error flag */
				return i;		/* Return the drive number */
			}
		}
	} while (++i<LASTDRIVE);		/* All drives checked? */
	SetErrBombFlag(ErrorState);		/* Restore the error flag */
	return (Word)-1;		/* Too bad... */
}

/**********************************

	Return the filesize of a file stream

**********************************/

#if !defined(__MSDOS__) && !defined(__MAC__) && !defined(__WIN32__) && !defined(__MACOSX__)

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

#endif

/**********************************

	Set the initial default prefixs for a power up state
	*: = Boot volume
	$: = System folder
	@: = Prefs folder
	8: = Default directory
	9: = Application directory

**********************************/

#if !defined(__MSDOS__) && !defined(__MAC__) && !defined(__WIN32__) && !defined(__BEOS__) && !defined(__MACOSX__)

void BURGERCALL SetDefaultPrefixs(void)
{
	char *TempPtr;

	TempPtr = ConvertNativePathToPath("");	/* This covers all versions */
	if (TempPtr) {
		SetAPrefix(8,TempPtr);		/* Set the standard work prefix */
		DeallocAPointer(TempPtr);	/* Release the memory */
	}
}

#endif

#if !defined(__MAC__) && !defined(__MACOSX__)		/* MacOS helper functions */

/**********************************

	All other operating systems don't support Filetypes

**********************************/

Word32 BURGERCALL GetAFileTypeNative(const char * /*FileName*/)
{
	return 0;		/* Don't do anything! */
}

/**********************************

	All other operating systems don't support Auxtypes

**********************************/

Word32 BURGERCALL GetAnAuxTypeNative(const char * /*FileName */)
{
	return 0;		/* Don't do anything! */
}

/**********************************

	All other operating systems don't support Filetypes

**********************************/

void BURGERCALL SetAFileTypeNative(const char * /*FileName*/,Word32 /*FileType */)
{
}

/**********************************

	All other operating systems don't support Auxtypes

**********************************/

void BURGERCALL SetAnAuxTypeNative(const char * /*FileName*/,Word32 /* AuxType */)
{
}

/**********************************

	All other operating systems don't support Filetypes and Auxtypes

**********************************/

void BURGERCALL FileSetFileAndAuxTypeNative(const char * /*FileName*/,Word32 /* FileType */,Word32 /* AuxType */)
{
}

#endif


/**********************************

	Convert a native path into a generic path
	The format is differant for each platform.

**********************************/

#if !defined(__MAC__) && !defined(__MSDOS__) && !defined(__WIN32__) && !defined(__MACOSX__)

char * BURGERCALL ConvertNativePathToPath(const char *OldPath)
{
	Word Temp;				/* Ascii Temp */
	char TempPath[8192];	/* Handle to temp buffer */
	char *Output;			/* Running pointer to temp buffer */
	Word Length;			/* Length of finished string */

	Output = TempPath;		/* Get running pointer */

	Temp = OldPath[0];
	if (Temp) {
		do {
			if (Temp=='/') {
				Temp = ':';
			}
			Output[0] = Temp;
			++Output;
			++OldPath;
			Temp = OldPath[0];
		} while (Temp);
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