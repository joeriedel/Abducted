/**********************************

	Burgerlib String Manager
	Written by Bill Heineman

**********************************/

#include "StString.h"
#include "MmMemory.h"
#include "ClStdLib.h"
#include "LkLinkList.h"
#include "PfPrefs.h"

#undef StrCopy
#undef StrCopyPad
#undef StrCopyHandle
#undef StrCopyPadHandle

/**********************************

	Header:
		Remove spaces from the beginning of a string

	Synopsis:
		Starting from the beginning of a string, see if the character is a space. If
		so, then the character is removed by copying the rest of the string up. This is
		repeated until there are no more spaces at the beginning of the string or the
		string is empty.

	Input:
		Input = Pointer to the "C" string to remove beginning spaces from. Do not pass
			a null pointer.

	Also:
		StrStripLeadingWhiteSpace(), StrStripTrailingSpaces(), StrStripLeading()

**********************************/

void BURGERCALL StrStripLeadingSpaces(char* Input)
{
	Word8 Temp;				/* Temp var */
	char *TempPtr;

	Temp = ((Word8 *)Input)[0];
	if (Temp==' ') {						/* Is there a leading space? */
		TempPtr = Input;					/* Save the starting point of the string */
		++Input;							/* Accept the char */
		do {
			Temp = ((Word8 *)Input)[0];		/* Fetch the next char */
			++Input;
		} while (Temp==' ');				/* Look for the end of the spaces */

		if (Temp) {							/* End of the string? */
			do {
				TempPtr[0] = (char)Temp;	/* Now, copy the string to the beginning of */
				++TempPtr;					/* the buffer */
				Temp = ((Word8 *)Input)[0];	/* Accept the next char */
				++Input;
			} while (Temp);					/* Did I copy the ending zero? */
		}
		TempPtr[0] = (char)Temp;			/* Store the final zero */
	}
}

/**********************************

	Header:
		Remove whitespace from the beginning of a string

	Synopsis:
		Starting from the beginning of a string, see if the character is a space or a tab. If
		so, then the character is removed by copying the rest of the string up. This is
		repeated until there are no more spaces or tabs at the beginning of the string or the
		string is empty.

	Input:
		Input = Pointer to the "C" string to remove beginning whitespace from. Do not pass
			a null pointer.

	Also:
		StrStripLeadingSpaces(), StrStripTrailingSpaces(), StrStripLeading()

**********************************/

void BURGERCALL StrStripLeadingWhiteSpace(char* Input)
{
	Word8 Temp;			/* Temp var */
	char *TempPtr;

	Temp = ((Word8 *)Input)[0];
	if (Temp==' ' || Temp=='\t') {			/* Is there a leading whitespace? */
		TempPtr = Input;					/* Save the starting point of the string */
		++Input;							/* Accept the char */
		do {
			Temp = ((Word8 *)Input)[0];		/* Fetch the next char */
			++Input;
		} while (Temp==' ' || Temp=='\t');	/* Look for the end of the whitespace */

		if (Temp) {							/* End of the string? */
			do {
				TempPtr[0] = (char)Temp;	/* Now, copy the string to the beginning of */
				++TempPtr;					/* the buffer */
				Temp = ((Word8 *)Input)[0];	/* Accept the next char */
				++Input;
			} while (Temp);					/* Did I copy the ending zero? */
		}
		TempPtr[0] = (char)Temp;			/* Store the final zero */
	}
}

/**********************************

	Header:
		Remove spaces from the end of a string

	Synopsis:
		Starting from the end of a string but before the ending zero, see if the last
		character is a space. If so, then the character is removed by zeroing it out and the
		process begins again until the string is empty or a non space character is at the
		end of the "C" string. For speed purposes, only a single zero is ever written to the
		"C" string. Do not assume that all the spaces that were removed were actually
		overwritten with zeros.

	Input:
		Input = Pointer to the "C" string to remove ending spaces from. Do not pass
			a null pointer.

	Also:
		StrStripTrailingWhiteSpace(), StrStripLeadingSpaces(), StrStripTrailing()

**********************************/

void BURGERCALL StrStripTrailingSpaces(char* Input)
{
	Word8 Temp;
	char *TempPtr;

	Temp = ((Word8 *)Input)[0];				/* Get the first character */
	if (Temp) {								/* Is there a string? */
		TempPtr = Input;					/* Init the zap pointer */
		do {
			++Input;						/* Accept the char */
			if (Temp!=' ') {				/* Not a space? */
				TempPtr=Input;				/* Last VALID char */
			}
			Temp = ((Word8 *)Input)[0];
		} while (Temp);						/* All done? */
		TempPtr[0] = (char)Temp;			/* Zap the final char */
	}
}

/**********************************

	Header:
		Remove whitespace from the end of a string

	Synopsis:
		Starting from the end of a string but before the ending zero, see if the last
		character is a space or tab. If so, then the character is removed by zeroing it
		out and the process begins again until the string is empty or a non space or tab
		character is at the end of the "C" string. For speed purposes, only a single zero
		is ever written to the "C" string. Do not assume that all the spaces and tab that
		were removed were actually overwritten with zeros.

	Input:
		Input = Pointer to the "C" string to remove ending spaces and tabs from. Do not pass
			a null pointer.

	Also:
		StrStripTrailingSpaces(), StrStripLeadingSpaces(), StrStripTrailing()

**********************************/

void BURGERCALL StrStripTrailingWhiteSpace(char* Input)
{
	Word8 Temp;
	char *TempPtr;

	Temp = ((Word8 *)Input)[0];				/* Get the first character */
	if (Temp) {								/* Is there a string? */
		TempPtr = Input;					/* Init the zap pointer */
		do {
			++Input;						/* Accept the char */
			if (Temp!=' ' && Temp!='\t') {	/* Not whitespace? */
				TempPtr=Input;				/* Last VALID char */
			}
			Temp = ((Word8 *)Input)[0];
		} while (Temp);						/* All done? */
		TempPtr[0] = (char)Temp;			/* Zap the final char */
	}
}

/**********************************

	Header:
		Remove spaces from the beginning and end of a string

	Synopsis:
		Starting from the beginning of a string, see if the character is a space. If so
		then the character is removed by copying the rest of the string up. This is repeated
		until there are no more spaces at the beginning of the string or the string is empty.
		Then the process is repeated but from the end of the string. The resulting string will
		not have any space characters at the beginning or the end.

	Input:
		Input = Pointer to the "C" string to remove beginning and ending spaces from. Do not pass
			a null pointer.

	Also:
		StrStripTrailingSpaces(), StrStripLeadingSpaces(), StrStripLeadingAndTrailingWhiteSpace()

**********************************/

void BURGERCALL StrStripLeadingAndTrailingSpaces(char* Input)
{
	Word8 Temp;				/* Temp var */
	char *TempPtr;
	char *EndPtr;

	EndPtr = Input;							/* Save the starting point of the string */
	Temp = ((Word8 *)Input)[0];
	++Input;								/* Accept the char */
	if (Temp==' ') {						/* Is there a leading space? */
		do {
			Temp = ((Word8 *)Input)[0];		/* Fetch the next char */
			++Input;
		} while (Temp==' ');				/* Look for the end of the spaces */
	}

	if (Temp) {								/* End of the string? */
		TempPtr = EndPtr;					/* Begin storing here */
		do {
			TempPtr[0] = (char)Temp;		/* Now, copy the string to the beginning of */
			++TempPtr;						/* the buffer */
			if (Temp!=' ') {				/* Is this a forbidden last char? */
				EndPtr = TempPtr;
			}
			Temp = ((Word8 *)Input)[0];		/* Accept the next char */
			++Input;
		} while (Temp);						/* No more string? */
	}
	EndPtr[0] = (char)Temp;					/* Store the final zero */
}

/**********************************

	Header:
		Remove whitespace from the beginning and end of a string

	Synopsis:
		Starting from the beginning of a string, see if the character is whitespace. If so
		then the character is removed by copying the rest of the string up. This is repeated
		until there are no more whitespace at the beginning of the string or the string is empty.
		Then the process is repeated but from the end of the string. The resulting string will
		not have any whitespace characters at the beginning or the end.

	Input:
		Input = Pointer to the "C" string to remove beginning and ending whitespace from. Do not pass
			a null pointer.

	Also:
		StrStripTrailingWhiteSpace(), StrStripLeadingWhiteSpace(), StrStripLeadingAndTrailingSpaces()

**********************************/

void BURGERCALL StrStripLeadingAndTrailingWhiteSpace(char* Input)
{
	Word8 Temp;				/* Temp var */
	char *TempPtr;
	char *EndPtr;

	EndPtr = Input;							/* Save the starting point of the string */
	Temp = ((Word8 *)Input)[0];
	++Input;								/* Accept the char */
	if ((Temp==' ') || (Temp =='\t')) {		/* Is there a leading space? */
		do {
			Temp = ((Word8 *)Input)[0];		/* Fetch the next char */
			++Input;
		} while ((Temp==' ') || (Temp=='\t'));	/* Look for the end of the spaces */
	}

	if (Temp) {								/* End of the string? */
		TempPtr = EndPtr;					/* Begin storing here */
		do {
			TempPtr[0] = (char)Temp;		/* Now, copy the string to the beginning of */
			++TempPtr;						/* the buffer */
			if (Temp!=' ' && Temp!='\t') {	/* Is this a forbidden last char? */
				EndPtr = TempPtr;
			}
			Temp = ((Word8 *)Input)[0];		/* Accept the next char */
			++Input;
		} while (Temp);						/* No more string? */
	}
	EndPtr[0] = (char)Temp;					/* Store the final zero */
}

/**********************************

	Header:
		Remove all characters except those in a list

	Synopsis:
		The Input string is scanned and every character that is not in the ListPtr "C"
		string will be removed and compacted. The resulting string consists only of
		characters that are found in the ListPtr "C" string.

		This code is case sensitive.

	Input:
		Input = Pointer to the "C" string to purge. Do not pass
			a null pointer.
		ListPtr = Pointer to the "C" string that contains the valid characters to allow.

	Also:
		StrStripAll(), StrStripLeadingSpaces(), StrStripTrailingSpaces()

**********************************/

void BURGERCALL StrStripAllBut(char* Input,const char* ListPtr)
{
	char *TempPtr;
	Word i;
	Word8 Temp;

	if (((Word8 *)Input)[0]) {		/* Any input */
		TempPtr = Input;		/* Destination pointer */
		if (((Word8 *)ListPtr)[0]) {		/* Is there a keep pointer? */
			do {
				Temp = ((Word8 *)Input)[0];	/* Upper case the input */
				i = 0;
				do {
					if (((Word8 *)ListPtr)[i]==Temp) {
						TempPtr[0] = Temp;		/* Accept the char */
						++TempPtr;
						break;
					}
					++i;
				} while (((Word8 *)ListPtr)[i]);	/* Any more? */
				++Input;
			} while (((Word8 *)Input)[0]);
		}
		TempPtr[0] = 0;		/* Terminate the string */
	}
}

/**********************************

	Header:
		Remove all characters that match those in a list

	Synopsis:
		The Input string is scanned and every character that is in the ListPtr "C"
		string will be removed and compacted. The resulting string consists only of
		characters that are not found in the ListPtr "C" string.

		This code is case sensitive.

	Input:
		Input = Pointer to the "C" string to purge. Do not pass
			a null pointer.
		ListPtr = Pointer to the "C" string that contains the characters to remove.
			Do not pass a null pointer.

	Also:
		StrStripAllBut(), StrStripLeadingSpaces(), StrStripTrailingSpaces()

**********************************/

void BURGERCALL StrStripAll(char* Input,const char* ListPtr)
{
	char *TempPtr;
	Word i;
	Word8 Temp;

	if (((Word8 *)Input)[0] && ((Word8 *)ListPtr)[0]) {		/* Any input */
		TempPtr = Input;		/* Destination pointer */
		do {
			Temp = ((Word8 *)Input)[0];	/* Upper case the input */
			i = 0;
			do {
				if (((Word8 *)ListPtr)[i]==Temp) {
					goto SkipIt;
				}
				++i;
			} while (((Word8 *)ListPtr)[i]);	/* Any more? */
			TempPtr[0] = (char)Temp;	/* Accept the char */
			++TempPtr;
SkipIt:
			++Input;
		} while (((Word8 *)Input)[0]);
		TempPtr[0] = 0;		/* Terminate the string */
	}
}

/**********************************

	Header:
		Remove characters from the end of a string

	Synopsis:
		Starting from the end of a string but before the ending zero, see if the
		character found is inside the string passed in ListPtr. If so, then the character
		is removed by zeroing it out and the process begins again until the string is
		empty or a character that is not in the list is found.

		This code is case sensitive.

	Input:
		Input = Pointer to the "C" string to remove ending characters from. Do not pass
			a null pointer.
		ListPtr = Pointer to the "C" string that contains the characters to remove.
		Do not pass a null pointer.

	Also:
		StrStripAllBut(), StrStripLeadingSpaces(), StrStripTrailingSpaces()

**********************************/

void BURGERCALL StrStripTrailing(char* Input,const char* ListPtr)
{
	char *EndPtr;
	Word i;
	Word8 Temp;

	if (((Word8 *)Input)[0] && ((Word8 *)ListPtr)[0]) {		/* Is there a string? */
		EndPtr = Input-1;	/* Init the zap pointer */
		do {
			i = 0;
			Temp = ((Word8 *)Input)[0];
			do {
				if (Temp==((Word8 *)ListPtr)[i]) {	/* In the list? */
					goto SkipIt;		/* Last VALID char */
				}
				++i;
			} while (((Word8 *)ListPtr)[i]);
			EndPtr = Input;		/* Don't zap this char? */
SkipIt:
			++Input;	/* Accept the char */
		} while (((Word8 *)Input)[0]);		/* All done? */
		EndPtr[1] = 0;	/* Zap the final char */
	}
}

/**********************************

	Header:
		Remove characters from the beginning of a string

	Synopsis:
		Starting from the beginning of a string, see if the character is in the supplied
		list. If so, then the character is removed by copying the rest of the string up.
		This is repeated until there are no more characters from the list at the
		beginning of the string or the string is empty.

		This code is case sensitive.

	Input:
		Input = Pointer to the "C" string to remove beginning characters from. Do not pass
			a null pointer.
		ListPtr = Pointer to the "C" string that contains the characters to remove.
		Do not pass a null pointer.

	Also:
		StrStripLeadingWhiteSpace(), StrStripTrailingSpaces(), StrStripLeading()

**********************************/

void BURGERCALL StrStripLeading(char* Input,const char* ListPtr)
{
	char *TempPtr;
	Word i,Temp;
	Word StopKilling;

	if (((Word8 *)Input)[0] && ((Word8 *)ListPtr)[0]) {	/* Any input */
		StopKilling = FALSE;
		TempPtr = Input;			/* Destination pointer */
		do {
			Temp = ((Word8 *)Input)[0];		/* Upper case the input */
			if (!StopKilling) {
				i = 0;
				do {
					if (((Word8 *)ListPtr)[i]==Temp) {
						goto SkipIt;
					}
					++i;
				} while (((Word8 *)ListPtr)[i]);	/* Any more? */
				StopKilling = TRUE;
			}
			TempPtr[0] = (char)Temp;	/* Accept the char */
			++TempPtr;
SkipIt:
			++Input;
		} while (((Word8 *)Input)[0]);
		TempPtr[0] = 0;		/* Terminate the string */
	}
}

/**********************************

	Header:
		Find whitespace, CR, LF or zero

	Synopsis:
		Parse a string until a whitespace,
		CR, LF or zero is found. Return the pointer at the
		point where the requested character is found.

	Input:
		Input = Pointer to a "C" string to parse

	Returns:
		Pointer to a TAB, Space, CR, LF or zero

	Also:
		ParseBeyondWhiteSpace()

**********************************/

char * BURGERCALL StrParseToDelimiter(const char *Input)
{
	Word Temp;		/* Temp storage */
	do {
		Temp = ((Word8 *)Input)[0];	/* Get a byte of input */
		++Input;
	} while (Temp!=32 && Temp!=9 && Temp && Temp!=13 && Temp!=10);		/* Space or TAB? */
	return (char*)--Input;		/* Return the result pointer */
}

/**********************************

	Header:
		Slice up a string into delimited parts

	Synopsis:
		Parse a string and return a list of
		unique strings parsed with whitespace delimiters.
		End of input is a zero, CR or LF.
		Quoted strings are accepted.
		The linked list is assume to be uninitialized. It will be
		initialized on exit so you must call LinkedListDestroy() on
		your list when you are done.
		The LinkedList_t's data is pointers to "C" strings. You
		cannot delete the strings since they were created with
		LinkedListAddNewEntryStringEnd(). You however can delete the
		entry and re-insert a string entry if you choose to modify the
		list in post processing.

	Input:
		ListPtr = Pointer to an uninitialized linked list
		Input = Pointer to a "C" string to parse (If NULL, the list is empty)

	Also:
		ParseBeyondWhiteSpace()

**********************************/

void BURGERCALL StrParse(LinkedList_t *ListPtr,const char *Input)
{
	char WorkBuffer[256];		/* Work buffer */
	LinkedListInit(ListPtr);
	if (Input) {				/* Failsafe */
		for (;;) {
			Word Temp;
			Input = ParseBeyondWhiteSpace(Input);		/* Kill whitespace */
			Temp = ((Word8 *)Input)[0];					/* End of line? */
			if (!Temp || Temp==10 || Temp==13) {
				break;
			}
			if (Temp=='"') {				/* Quoted string? */
				Input = GetAParsedString((char *)Input,WorkBuffer,sizeof(WorkBuffer));
			} else {
				char *End;
				End = StrParseToDelimiter(Input);
				Temp = End-Input;
				if (Temp>=sizeof(WorkBuffer)) {
					Temp = sizeof(WorkBuffer)-1;
				}
				FastMemCpy(WorkBuffer,Input,Temp);
				WorkBuffer[Temp]=0;
				Input = End;			/* Accept the text */
			}
			if (((Word8 *)WorkBuffer)[0]) {		/* Anything here? */
				LinkedListAddNewEntryStringEnd(ListPtr,WorkBuffer);
			}
		}
	}
}

/**********************************

	Header:
		Get the name the user has called the computer

	Synopsis:
		Some computer owners have the option to give their computer a whimsical
		name. This routine will retreive that name. If the buffer is too small to
		receive the name, the name will be truncated. If for some reason a name can't
		be found or the operating system doesn't support naming, the name of "Computer"
		will be returned.

	Input:
		Output = Pointer to a buffer to receive the computer's name. Do not pass a null pointer.
		OutputSize = The size of the buffer in bytes.

	Platform:
		Win95/NT will call GetComputerName(). MacOS will get a copy of the OS string
		number -16413 from the system resource file, all others will just return the
		name of "Computer".

	Also:
		StrGetUserName()

**********************************/

#if !defined(__WIN32__) && !defined(__MAC__) && !defined(__MACOSX__)

void BURGERCALL StrGetComputerName(char* Output,Word OutputSize)
{
	if (OutputSize) {
		/* The name wasn't present, use the default */

		--OutputSize;
		strncpy(Output,"Computer",OutputSize);	/* Copy to buffer */
		Output[OutputSize] = 0;		/* Make SURE it's zero terminated! */
	}
}

#endif


/**********************************

	Header:
		Get the name of the current user

	Synopsis:
		When someone has logged onto a computer, that person had to give a name.
		This routine will retreive that name. If the buffer is too small to receive
		the name, the name will be truncated. If for some reason a name can't be found
		or the operating system doesn't support user logons, the name of "User" will
		be returned.

	Input:
		Output = Pointer to a buffer to receive the user's name. Do not pass a null pointer.
		OutputSize = The size of the buffer in bytes.

	Platform:
		Win95/NT will call GetUserName(). MacOS will get a copy of the OS string
		number -16096 from the system resource file, all others will just return the
		name of "User".

	Also:
		StrGetComputerName()

**********************************/

#if !defined(__WIN32__) && !defined(__MAC__) && !defined(__MACOSX__)

void BURGERCALL StrGetUserName(char* Output,Word OutputSize)
{
	if (OutputSize) {
		/* The name wasn't present, use the default */

		--OutputSize;
		strncpy(Output,"User",OutputSize);	/* Copy to buffer */
		Output[OutputSize] = 0;		/* Make SURE it's zero terminated! */
	}
}

#endif

/**********************************

	Header:
		Get a pointer to the beginning of the XYZ extension

	Synopsis:
		A string is scanned until the last period is found. A pointer to the string
		fragment following the period is returned. If no period is found then a pointer
		to the terminating zero is returned. Filenames expected are... "ReadMe.txt",
		"ArtFile.gif" or "MyData.c". These examples will return pointers to ... "txt",
		"gif" or "c". The pointer is a pointer within the string. Do not dispose of it.

	Input:
		Input = Pointer to the "C" string to scan. Do not pass a null pointer.
		OutputSize = The size of the buffer in bytes.

	Returns:
		A pointer to the filename extension of terminating zero.

	Also:
		StrSetFileExtension()

**********************************/

char* BURGERCALL StrGetFileExtension(const char *Input)
{
	const char *WorkPtr;

	WorkPtr = 0;		/* No string found */
	if (((Word8 *)Input)[0]) {		/* Any data to parse? */
		do {
			if (((Word8 *)Input)[0]=='.') {	/* Hit a period? */
				WorkPtr = Input;	/* Save the place */
			}
			++Input;
		} while (((Word8 *)Input)[0]);		/* End of the string */
	}
	if (!WorkPtr) {		/* No match? */
		return (char *)Input;	/* Return end of string */
	}
	return (char *)WorkPtr+1;		/* Return the pointer beyond the period */
}

/**********************************

	Header:
		Replace the text after the last period for filename extensions

	Synopsis:
		A string is scanned until the last period is found. The text beyond the final
		period is discarded and the string pointed by NewExtension is appended to the Input
		filename. If no final period is found, then a period is appended and then
		the new extension is added. If NewExtension has a period as the first character,
		it will be ignored to prevent a double period from occuring in the final string.

		You must guarantee that the Input buffer has enough space to accomodate the new
		extension. This routine will not check for buffer overruns.

	Input:
		Input = Pointer to the "C" string to scan. Do not pass a null pointer. This will be modified
			with the new file extension.
		NewExtension = Pointer to a "C" string with the extension to apply.

	Also:
		StrGetFileExtension()

**********************************/

void BURGERCALL StrSetFileExtension(char* Input,const char* NewExtension)
{
	char *WorkPtr;

	WorkPtr = StrGetFileExtension(Input);	/* Get pointer to the char after the '.' */
	if (((Word8 *)WorkPtr)[0]) {	/* End of string? (No extension) */
		--WorkPtr;		/* Dispose of the period */
		WorkPtr[0] = 0;	/* Truncate the string */
	}
	if (NewExtension && ((Word8 *)NewExtension)[0]) {	/* Is there a new extension? */
		if (((Word8 *)NewExtension)[0]!='.') {		/* Does the new extension have a period? */
			WorkPtr[0] = '.';		/* Put the period back */
			++WorkPtr;
		}
		strcpy(WorkPtr,NewExtension);		/* Overwrite the extension */
	}
}


/**********************************

	Header:
		Make a copy of a "C" string

	Synopsis:
		If the input is zero, then a zero is returned. Otherwise, the "C" string pointed
		by Input will be copied into a buffer allocated by AllocAPointer(). The buffer
		is exactly the same size of the string. You must eventually dispose of the string
		with a call to DeallocAPointer().

	Input:
		Input = Pointer to the "C" string to copy. A null pointer will return zero.

	Returns:
		A pointer to the copy of the string. Or zero if a memory error occurs.

	Also:
		StrCopyHandle(), DeallocAPointer(), StrCopyPad()

**********************************/

char * BURGERCALL StrCopy(const char *Input)
{
	Word Length;
	char *Result;

	if (Input) {		/* Valid input? */
		Length = strlen(Input)+1;		/* Get the length */
		Result = (char *)AllocAPointer(Length);		/* Allocate the memory */
		if (Result) {
			FastMemCpy(Result,Input,Length);	/* Copy the string */
			return Result;
		}
	}
	return 0;	/* Return nothing */
}

/**********************************

	Header:
		Make a copy of a "C" string with some padding

	Synopsis:
		If the input is zero, then a zero is returned. Otherwise, the "C" string pointed
		by Input will be copied into a buffer allocated by AllocAPointer(). The buffer
		is the same size of the string plus the padding value. The extra memory is not
		initialized but the string does terminate with a zero. You must eventually
		dispose of the string with a call to DeallocAPointer().

	Input:
		Input = Pointer to the "C" string to copy. A null pointer will return zero.
		Padding = Number of bytes to extend the buffer.

	Returns:
		A pointer to the copy of the string. Or zero if a memory error occurs.

	Also:
		StrCopyHandle(), DeallocAPointer(), StrCopy()

**********************************/

char * BURGERCALL StrCopyPad(const char *Input,Word Padding)
{
	Word Length;
	char *Result;

	if (Input) {		/* Valid input? */
		Length = strlen(Input)+1;		/* Get the length */
		Result = (char *)AllocAPointer(Length+Padding);		/* Allocate the memory */
		if (Result) {
			FastMemCpy(Result,Input,Length);	/* Copy the string */
			return Result;
		}
	}
	return 0;	/* Return nothing */
}

/**********************************

	Header:
		Make a copy of a "C" string

	Synopsis:
		If the input is zero, then a zero is returned. Otherwise, the "C" string pointed
		by Input will be copied into a buffer allocated by AllocAHandle(). The buffer
		is exactly the same size of the string. You must eventually dispose of the string
		with a call to DeallocAHandle().

	Input:
		Input = Pointer to the "C" string to copy. A null pointer will return zero.

	Returns:
		A handle to the copy of the string. Or zero if a memory error occurs.

	Also:
		StrCopy(), DeallocAHandle(), StrCopyPad()

**********************************/

void ** BURGERCALL StrCopyHandle(const char *Input)
{
	Word Length;
	void **Result;

	if (Input) {		/* Valid input? */
		Length = strlen(Input)+1;		/* Get the length */
		Result = AllocAHandle(Length);		/* Allocate the memory */
		if (Result) {
			FastMemCpy(Result[0],Input,Length);	/* Copy the string */
			return Result;
		}
	}
	return 0;	/* Return nothing */
}


/**********************************

	Header:
		Make a copy of a "C" string with some padding

	Synopsis:
		If the input is zero, then a zero is returned. Otherwise, the "C" string pointed
		by Input will be copied into a buffer allocated by AllocAHandle(). The buffer
		is the same size of the string plus the padding value. The extra memory is not
		initialized but the string does terminate with a zero. You must eventually
		dispose of the string with a call to DeallocAHandle().

	Input:
		Input = Pointer to the "C" string to copy. A null pointer will return zero.
		Padding = Number of bytes to extend the buffer.

	Returns:
		A handle to the copy of the string. Or zero if a memory error occurs.

	Also:
		StrCopyHandle(), DeallocAHandle(), StrCopyPad()

**********************************/

void ** BURGERCALL StrCopyPadHandle(const char *Input,Word Padding)
{
	Word Length;
	void **Result;

	if (Input) {		/* Valid input? */
		Length = strlen(Input)+1;		/* Get the length */
		Result = AllocAHandle(Length+Padding);		/* Allocate the memory */
		if (Result) {
			FastMemCpy(Result[0],Input,Length);	/* Copy the string */
			return Result;
		}
	}
	return 0;	/* Return nothing */
}

/**********************************

	Header:
		Make a copy of a "C" string (DebugVersion)

	Synopsis:
		If the input is zero, then a zero is returned. Otherwise, the "C" string pointed
		by Input will be copied into a buffer allocated by AllocAPointer(). The buffer
		is exactly the same size of the string. You must eventually dispose of the string
		with a call to DeallocAPointer().

	Input:
		Input = Pointer to the "C" string to copy. A null pointer will return zero.
		File = Pointer to the source filename
		Line = Line number in the source file

	Returns:
		A pointer to the copy of the string. Or zero if a memory error occurs.

	Also:
		StrCopy(), DebugStrCopyHandle(), DeallocAPointer(), DebugStrCopyPad()

**********************************/

char * BURGERCALL DebugStrCopy(const char *Input,const char *File,Word Line)
{
	Word Length;
	char *Result;

	if (Input) {		/* Valid input? */
		Length = strlen(Input)+1;		/* Get the length */
		Result = (char *)DebugAllocAPointer(Length,File,Line);		/* Allocate the memory */
		if (Result) {
			FastMemCpy(Result,Input,Length);	/* Copy the string */
			return Result;
		}
	}
	return 0;	/* Return nothing */
}

/**********************************

	Header:
		Make a copy of a "C" string with some padding

	Synopsis:
		If the input is zero, then a zero is returned. Otherwise, the "C" string pointed
		by Input will be copied into a buffer allocated by AllocAPointer(). The buffer
		is the same size of the string plus the padding value. The extra memory is not
		initialized but the string does terminate with a zero. You must eventually
		dispose of the string with a call to DeallocAPointer().

	Input:
		Input = Pointer to the "C" string to copy. A null pointer will return zero.
		Padding = Number of bytes to extend the buffer.

	Returns:
		A pointer to the copy of the string. Or zero if a memory error occurs.

	Also:
		StrCopyHandle(), DeallocAPointer(), StrCopy()

**********************************/

char * BURGERCALL DebugStrCopyPad(const char *Input,Word Padding,const char *File,Word Line)
{
	Word Length;
	char *Result;

	if (Input) {		/* Valid input? */
		Length = strlen(Input)+1;		/* Get the length */
		Result = (char *)DebugAllocAPointer(Length+Padding,File,Line);		/* Allocate the memory */
		if (Result) {
			FastMemCpy(Result,Input,Length);	/* Copy the string */
			return Result;
		}
	}
	return 0;	/* Return nothing */
}

/**********************************

	Header:
		Make a copy of a "C" string

	Synopsis:
		If the input is zero, then a zero is returned. Otherwise, the "C" string pointed
		by Input will be copied into a buffer allocated by AllocAHandle(). The buffer
		is exactly the same size of the string. You must eventually dispose of the string
		with a call to DeallocAHandle().

	Input:
		Input = Pointer to the "C" string to copy. A null pointer will return zero.

	Returns:
		A handle to the copy of the string. Or zero if a memory error occurs.

	Also:
		StrCopy(), DeallocAHandle(), StrCopyPad()

**********************************/

void ** BURGERCALL DebugStrCopyHandle(const char *Input,const char *File,Word Line)
{
	Word Length;
	void **Result;

	if (Input) {		/* Valid input? */
		Length = strlen(Input)+1;		/* Get the length */
		Result = DebugAllocAHandle2(Length,0,File,Line);		/* Allocate the memory */
		if (Result) {
			FastMemCpy(Result[0],Input,Length);	/* Copy the string */
			return Result;
		}
	}
	return 0;	/* Return nothing */
}


/**********************************

	Header:
		Make a copy of a "C" string with some padding

	Synopsis:
		If the input is zero, then a zero is returned. Otherwise, the "C" string pointed
		by Input will be copied into a buffer allocated by AllocAHandle(). The buffer
		is the same size of the string plus the padding value. The extra memory is not
		initialized but the string does terminate with a zero. You must eventually
		dispose of the string with a call to DeallocAHandle().

	Input:
		Input = Pointer to the "C" string to copy. A null pointer will return zero.
		Padding = Number of bytes to extend the buffer.

	Returns:
		A handle to the copy of the string. Or zero if a memory error occurs.

	Also:
		StrCopyHandle(), DeallocAHandle(), StrCopyPad()

**********************************/

void ** BURGERCALL DebugStrCopyPadHandle(const char *Input,Word Padding,const char *File,Word Line)
{
	Word Length;
	void **Result;

	if (Input) {		/* Valid input? */
		Length = strlen(Input)+1;		/* Get the length */
		Result = DebugAllocAHandle2(Length+Padding,0,File,Line);		/* Allocate the memory */
		if (Result) {
			FastMemCpy(Result[0],Input,Length);	/* Copy the string */
			return Result;
		}
	}
	return 0;	/* Return nothing */
}


