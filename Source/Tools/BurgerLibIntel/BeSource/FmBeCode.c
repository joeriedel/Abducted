/**********************************

	BEOS version
	If I am asked for a drive number
	I assume 0-63 are SCSI drives,
	64-127 are floppy drives
	128-191 are ide drives
	Drive numbers are NOT recommended for BeOS

**********************************/

#include "FmFile.h"

#if defined(__BEOS__)
#include "LWMemory.h"
#include "LWStdLib.h"
#include <Be.h>

/**********************************

	BEOS version
	If I am asked for a drive number
	I assume 0-63 are SCSI drives,
	64-127 are floppy drives
	128-191 are ide drives
	Drive numbers are NOT recommended for BeOS

**********************************/

void BURGERCALL ExpandAPathToBufferNative(char *Output, const char *FileName)
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
			if (Temp==':') {
				Temp = '/';
			}
			Output[0] = Temp;
			++Output;
			Temp = ((Word8 *)FilePtr)[0];
		} while (Temp);
		Output-=2;
		if (Output[0]!='/') {
			++Output;		/* Remove trailing slash */
		}
	}
	Output[0] = 0;			/* Terminate the "C" string */
}

/**********************************

	Set the initial default prefixs for a power up state
	*: = Boot volume
	$: = System folder
	@: = Prefs folder
	8: = Default directory
	9: = Application directory

**********************************/

extern char **argv_save;

void BURGERCALL SetDefaultPrefixs(void)
{
	char *TempPtr;

	TempPtr = ConvertNativePathToPath("");	/* This covers all versions */
	if (TempPtr) {
		SetAPrefix(8,TempPtr);		/* Set the standard work prefix */
		DeallocAPointer(TempPtr);	/* Release the memory */
	}

	TempPtr = ConvertNativePathToPath(argv_save[0]);
	if (TempPtr) {
		SetAPrefix(9,TempPtr);		/* Set the application prefix */
		DeallocAPointer(TempPtr);
		PopAPrefix(9);		/* Remove the application's name */
	}

	SetAPrefix(PREFIXBOOT,":boot:");

	{
		char Buffer[256];
		find_directory(B_BEOS_SYSTEM_DIRECTORY,0,0,Buffer,sizeof(Buffer));
		TempPtr = ConvertNativePathToPath(Buffer);
		if (TempPtr) {
			SetAPrefix(PREFIXSYSTEM,TempPtr);
			DeallocAPointer(TempPtr);
		}

		find_directory(B_USER_SETTINGS_DIRECTORY,0,0,Buffer,sizeof(Buffer));
		TempPtr = ConvertNativePathToPath(Buffer);
		if (TempPtr) {
			SetAPrefix(PREFIXPREFS,TempPtr);
			DeallocAPointer(TempPtr);
		}
	}
}



#endif