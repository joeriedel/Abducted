#include "PfPrefs.h"
#include "ClStdLib.h"
#include "MmMemory.h"
#include "FmFile.h"

/**********************************

	Table to convert nibbles to ASCII chars

**********************************/

char NibbleToAscii[16] = {'0','1','2','3','4','5','6','7',
	'8','9','A','B','C','D','E','F'};

/**********************************

	Table to convert nibbles to ASCII chars
	with hex chars in lower case

**********************************/

char NibbleToAsciiLC[16] = {'0','1','2','3','4','5','6','7',
	'8','9','a','b','c','d','e','f'};


/****************************************

	Prints an unsigned long number.
	Also prints lead spaces or zeros if needed

****************************************/

static Word32 TensTable[] = {
1,				/* Table to quickly div by 10 */
10,
100,
1000,
10000,
100000,
1000000,
10000000,
100000000,
1000000000
};

char *BURGERCALL LongWordToAscii2(Word32 Val,char *AsciiPtr,Word Printing)
{
	Word Index;		 /* Index to TensTable */
	Word32 BigNum;	/* Temp for TensTable value */
	Word Letter;		/* ASCII char */

	Index = Printing&0xFF;
	if (!Index || Index>=11) {		/* Failsafe */
		Index = 10;		 /* 10 digits to process */
	}
	if (Index<10) {
		Val = Val%TensTable[Index];		/* Remove all unneeded bits */
	}
	do {
		--Index;		/* Dec index */
		BigNum = TensTable[Index];	/* Get div value in local */
		Letter = '0';			 /* Init ASCII value */
		while (Val>=BigNum) {	 /* Can I divide? */
			Val-=BigNum;		/* Remove value */
			++Letter;			 /* Inc ASCII value */
		}
		if (Printing&ASCIILEADINGZEROS || Letter!='0' || !Index) {	/* Already printing? */
			Printing |= ASCIILEADINGZEROS;		/* Force future printing */
			AsciiPtr[0] = static_cast<char>(Letter);		/* Also must print on last char */
			++AsciiPtr;
		}
	} while (Index);		/* Any more left? */
	if (!(Printing&ASCIINONULL)) {
		AsciiPtr[0] = 0;		/* Terminate the string */
	}
	return AsciiPtr;
}

/****************************************

	Converts a signed long number into an ASCII string

****************************************/

char *BURGERCALL longToAscii2(long Input,char *AsciiPtr,Word Printing)
{
	if (Input<0) {
		Input = -Input;
		AsciiPtr[0] = '-';
		++AsciiPtr;
	}
	return LongWordToAscii2(Input,AsciiPtr,Printing);
}

/****************************************

	Parses from an input stream and converts it to a longword
	I support both decimal and hex

****************************************/

Word32 BURGERCALL AsciiToLongWord2(const char *AsciiPtr,char **DestPtr)
{
	Word32 Value;		/* Value to return */
	Word Letter;		/* Temp ASCII char */
	Word Negate;		/* Flag for negation */
	Word Base;			/* Decimal base 10 or 16 */
	const char *NewResult;	/* Pointer to accepted chars */

	Negate = FALSE;		/* Don't negate the result */
	Base = 10;			/* Decimal input */
	NewResult = AsciiPtr;	/* Assume I don't accept any input */

	for (;;) {
		Letter = ((Word8 *)AsciiPtr)[0];	/* Get char */
		++AsciiPtr;
		if (Letter == ' ' || Letter == 9 || Letter == '+') {		/* Eat white space */
			continue;
		}
		if (Letter == '-') {	/* Negate it? */
			Negate ^= TRUE;
			continue;			/* Accept char */
		}
		if (Letter=='$') {	/* Hex input? */
			Base = 16;
			continue;		/* Accept char */
		}
		if (Letter=='0') {	/* 0X for "C" style input */
			Letter = ((Word8 *)AsciiPtr)[0];		/* Index to 'x' */
			if (Letter=='X' || Letter=='x') {
				Base = 16;
				++AsciiPtr;		/* Skip the x */
				break;	/* Don't zap whitespace */
			}
		}
		--AsciiPtr;
		break;
	}

	Value = 0;		/* Init result */
	for (;;) {
		Letter = ((Word8 *)AsciiPtr)[0];	/* Get char */
		if (Letter>='a' && Letter<('z'+1)) {
			Letter &= 0xDF;		/* Convert to upper case */
		}
		Letter -= '0';		/* 0-9 for ascii 0-9 */
		if (Letter>=10) {	/* Possible hex value? */
			if (Base==10 || Letter<0x11 || Letter>=0x17) {	/* Valid? */
				if (Negate&TRUE) {			/* Should the answer be negated? */
					Value = 0-Value;
				}
				if (DestPtr) {			/* Should I return the last accepted ASCII? */
					if (Negate&0x80) {	/* Has text been accepted? */
						do {
							Letter = ((Word8 *)NewResult)[0];	/* Remove whitespace */
							++NewResult;
						} while (Letter==' ' || Letter==9);
						--NewResult;
					}
					*DestPtr = (char *)NewResult;		/* Return the result pointer */
				}
				return Value;		/* Return result */
			}
			Letter-=7;		/* Make ASCII A-F into 10-15 */
		}
		Value = Value*Base;	/* Adjust for base */
		Value += Letter;	/* Add the new number */
		++AsciiPtr;		/* Accept */
		NewResult = AsciiPtr;	/* Save the new result value */
		Negate|=0x80;		/* Set flag */
	}
}

/**********************************

	Convert a longword into an ASCII
	string without leading characters
	and use a base 16 number system (Hex)

**********************************/

char *BURGERCALL LongWordToAsciiHex2(Word32 Val,char *AsciiPtr,Word Printing)
{
	Word Index;		 /* Maximum numbers to translate */
	Word Letter;		/* ASCII char */

	Index = Printing&0xFF;		/* 8 digits to process */
	if (!Index || Index>=9) {	/* Failsafe */
		Index = 8;
	}
	if (Index<8) {				/* Won't show certain digits? */
		Letter = 8-Index;		/* How many digits to chop? */
		Letter <<=2;			/* 4,8,12 ... 28 */
		Val <<= Letter;			/* Newly adjusted input */
	}
	do {
		--Index;		/* Dec index */
		Letter = Val>>28;
		if (Printing&ASCIILEADINGZEROS || Letter || !Index) {	/* Already printing? */
			Printing |= ASCIILEADINGZEROS;		/* Force future printing */
			AsciiPtr[0] = NibbleToAscii[Letter];	/* Also must print on last char */
			++AsciiPtr;
		}
		Val<<=4;			/* Shift to next digit */
	} while (Index);		/* Any more left? */
	if (!(Printing&ASCIINONULL)) {
		AsciiPtr[0] = 0;		/* Terminate the string */
	}
	return AsciiPtr;
}

/**********************************

	Follow a stream of text input until either a zero is found
	or an EOL is found.
	If a zero is found, return a pointer to the ZERO
	If an EOL is found, return a pointer to the text BEYOND
	the EOL, I also handle EOLs for Mac and PC text files.

**********************************/

char * BURGERCALL ParseBeyondEOL(const char *Input)
{
	Word Temp;		/* Temp storage */
	do {
		Temp = ((Word8 *)Input)[0];	/* Get a byte of input */
		if (!Temp) {		/* End now? */
			break;
		}
		++Input;			/* Accept the char */
		if (Temp==13) {		/* Mac or PC style EOL? */
			goto HitEOL;	/* Accept */
		}
	} while (Temp!=10);		/* Unix EOL? */
	return (char *)Input;		/* Return the result pointer */

HitEOL:
	if (((Word8 *)Input)[0]==10) {	/* Followed by a PC LF? */
		++Input;	/* Accept the LF as well */
	}
	return (char*)Input;	/* Exit now */
}

/**********************************

	Follow a stream of text input until a non white space character is found.

**********************************/

char * BURGERCALL ParseBeyondWhiteSpace(const char *Input)
{
	Word Temp;		/* Temp storage */
	do {
		Temp = ((Word8 *)Input)[0];	/* Get a byte of input */
		++Input;
	} while (Temp==32 || Temp==9);		/* Space or TAB? */
	return (char*)--Input;		/* Return the result pointer */
}

/**********************************

	Store a "C" string into a streamed output
	but don't copy the zero.
	Return a pointer to the end of the data stream.

**********************************/

char * BURGERCALL StoreAString(char *WorkPtr,const char *Input)
{
	Word Letter;		/* Temp ASCII letter */
	Letter = ((Word8 *)Input)[0];	/* Get first char */
	if (Letter) {		/* Exit now? */
		do {
			WorkPtr[0] = static_cast<char>(Letter);	/* Save the char */
			++WorkPtr;		/* Inc index */
			++Input;		/* Inc source index */
			Letter = ((Word8 *)Input)[0];	/* Get next char */
		} while (Letter);		/* More? */
	}
	return WorkPtr;			/* Return the final char index */
}


/**********************************

	Store a longword into a stream output text file
	as a decimal number

**********************************/

char * BURGERCALL StoreALongWordAscii(char *WorkPtr,Word32 Input)
{
	return LongWordToAscii(Input,WorkPtr);	/* Store to the output buffer */
}

/**********************************

	Store a longword into a stream output text file
	as a hex number

**********************************/

char * BURGERCALL StoreALongWordAsciiHex(char *WorkPtr,Word32 Input)
{
	return LongWordToAsciiHex(Input,WorkPtr);	/* Store to the output buffer */
}


/**********************************

	Store a longword into a stream output text file
	as a signed longword

**********************************/

char * BURGERCALL StoreAlongAscii(char *WorkPtr,long Input)
{
	return longToAscii(Input,WorkPtr);	/* Store to the output buffer */
}


/**********************************

	Store an ASCII string into an output stream,
	handle the " as a special case

**********************************/

char * BURGERCALL StoreAParsedString(char *WorkPtr,const char *Input)
{
	Word Letter;
	WorkPtr[0] = '"';		/* Start with a quote */
	++WorkPtr;
	Letter = ((Word8 *)Input)[0];		/* Get the first input char */
	if (Letter) {			/* Valid? */
		do {
			if (Letter=='"') {
				WorkPtr[0] = '"';	/* Make a double quote */
				++WorkPtr;
			}
			if (Letter==9) {		/* Kill tabs from output */
				Letter = ' ';
			}
			WorkPtr[0] = static_cast<char>(Letter);	/* Save the char */
			++WorkPtr;
			++Input;
			Letter = ((Word8 *)Input)[0];		/* Fetch the next char */
		} while (Letter);		/* More? */
	}
	WorkPtr[0] = '"';		/* End with a quote */
	++WorkPtr;
	return WorkPtr;			/* Return the end pointer */
}



/**********************************

	Get an ASCII string from an input stream,
	handle the " as a special case

**********************************/

char * BURGERCALL GetAParsedString(const char *WorkPtr,char *DestPtr,Word Size)
{
	Word Letter;
	Word Index;

	Index = 0;					/* Init dest index */
	Letter = ((Word8 *)WorkPtr)[0];		/* Get the first char */
	if (Letter=='"') {			/* MUST be a quote */
		for (;;) {
			++WorkPtr;			/* Accept the quote */
			Letter = ((Word8 *)WorkPtr)[0];
			if (Letter==9) {	/* Get rid of tabs */
				Letter = ' ';
			}
			if (Letter=='"') {	/* Double quote? */
				++WorkPtr;
				if (((Word8 *)WorkPtr)[0]!='"') {
					break;		/* Nope, exit then */
				}
			}
			if (Index<Size) {	/* Can it fit? */
				DestPtr[Index] = static_cast<char>(Letter);	/* Save resulting ASCII */
				++Index;
			}
		}
	}
	if (Size) {		/* Is there an output buffer? */
		if (Index==Size) {		/* Already full? */
			--Index;			/* Index back */
		}
		DestPtr[Index] = 0;		/* Zero terminate the string */
	}
	return (char *)WorkPtr;		/* Return dest buffer pointer */
}

/**********************************

	Generic parsing proc for reading and writing a Word32

**********************************/

char * BURGERCALL PrefsLongWordProc(char *WorkPtr,PrefsRecord_t *RecordPtr,PrefState_e Pass)
{
	switch (Pass) {
	case PREFWRITE:		/* Write data */
		return StoreALongWordAscii(WorkPtr,((Word32 *)RecordPtr->DataPtr)[0]);
	case PREFDEFAULT:	/* Default */
		((Word32 *)RecordPtr->DataPtr)[0] = (Word32)RecordPtr->Default;	/* Default */
		return WorkPtr;
	default:			/* Read */
		((Word32 *)RecordPtr->DataPtr)[0]= AsciiToLongWord2(WorkPtr,&WorkPtr);
		return WorkPtr;
	}
}


/**********************************

	Generic parsing proc for reading and writing a Word16

**********************************/

char * BURGERCALL PrefsShortProc(char *WorkPtr,PrefsRecord_t *RecordPtr,PrefState_e Pass)
{
	switch (Pass) {
	case PREFWRITE:		/* Write data */
		return StoreALongWordAscii(WorkPtr,((Word16 *)RecordPtr->DataPtr)[0]);
	case PREFDEFAULT:	/* Default */
		((Word16 *)RecordPtr->DataPtr)[0] = (Word16)RecordPtr->Default;	/* Default */
		return WorkPtr;
	default:			/* Read */
		((Word16 *)RecordPtr->DataPtr)[0]= (Word16)AsciiToLongWord2(WorkPtr,&WorkPtr);
		return WorkPtr;
	}
}

/**********************************

	Generic parsing proc for reading and writing a Word32 Array

**********************************/

char * BURGERCALL PrefsLongWordArrayProc(char *WorkPtr,PrefsRecord_t *RecordPtr,PrefState_e Pass)
{
	Word Count;
	Word32 *ArrayPtr;

	Count = RecordPtr->Count;
	if (!Count) {			/* Any elements in the array? */
		return WorkPtr;		/* Exit now! */
	}
	ArrayPtr = (Word32 *)RecordPtr->DataPtr;	/* Get the dest array ptr */
	switch (Pass) {
	case PREFWRITE:		/* Write data */
		do {
			WorkPtr = LongWordToAscii(ArrayPtr[0],WorkPtr);	/* Convert to ASCII */
			++ArrayPtr;
			if (Count!=1) {			/* Not the last one? */
				WorkPtr[0] = ',';
				++WorkPtr;
			}
		} while (--Count);
		return WorkPtr;
	case PREFDEFAULT:	/* Default data */
		FastMemCpy(ArrayPtr,RecordPtr->Default,Count*sizeof(Word32));	/* Copy the array */
		return WorkPtr;
	default:			/* Read data */
		{
			char *OldPtr;
			char *TempPtr;
			OldPtr = WorkPtr;
			do {
				ArrayPtr[0] = AsciiToLongWord2(WorkPtr,&TempPtr);	/* Read one */
				if (TempPtr==WorkPtr) {		/* Nothing? */
					return OldPtr;			/* Abort */
				}
				WorkPtr = TempPtr;			/* Accept last char */
				++ArrayPtr;
				if (Count!=1) {			/* Not the last one? */
					if (((Word8 *)WorkPtr)[0]!=',') {	/* Comma delimiter? */
						return OldPtr;		/* Abort */
					}
					++WorkPtr;			/* Remove the comma */
				}
			} while (--Count);			/* Count down */
			return WorkPtr;		/* Exit now */
		}
	}
}


/**********************************

	Generic parsing proc for reading and writing a Word16 Array

**********************************/

char * BURGERCALL PrefsShortArrayProc(char *WorkPtr,PrefsRecord_t *RecordPtr,PrefState_e Pass)
{
	Word Count;
	Word16 *ArrayPtr;

	Count = RecordPtr->Count;
	if (!Count) {			/* Any elements in the array? */
		return WorkPtr;		/* Exit now! */
	}
	ArrayPtr = (Word16 *)RecordPtr->DataPtr;	/* Get the dest array ptr */
	switch (Pass) {
	case PREFWRITE:		/* Write data */
		do {
			WorkPtr = LongWordToAscii((Word32)ArrayPtr[0],WorkPtr);	/* Convert to ASCII */
			++ArrayPtr;
			if (Count!=1) {			/* Not the last one? */
				WorkPtr[0] = ',';
				++WorkPtr;
			}
		} while (--Count);
		return WorkPtr;
	case PREFDEFAULT:	/* Default data */
		FastMemCpy(ArrayPtr,RecordPtr->Default,Count*sizeof(Word16));	/* Copy the array */
		return WorkPtr;
	default:			/* Read data */
		{
			char *OldPtr;
			char *TempPtr;
			OldPtr = WorkPtr;
			do {
				ArrayPtr[0] = (Word16)AsciiToLongWord2(WorkPtr,&TempPtr);	/* Read one */
				if (TempPtr==WorkPtr) {		/* Nothing? */
					return OldPtr;			/* Abort */
				}
				WorkPtr = TempPtr;			/* Accept last char */
				++ArrayPtr;
				if (Count!=1) {			/* Not the last one? */
					if (((Word8 *)WorkPtr)[0]!=',') {	/* Comma delimiter? */
						return OldPtr;		/* Abort */
					}
					++WorkPtr;			/* Remove the comma */
				}
			} while (--Count);			/* Count down */
			return WorkPtr;		/* Exit now */
		}
	}
}


/**********************************

	Generic parsing proc for reading and writing a String

**********************************/

char * BURGERCALL PrefsStringProc(char *WorkPtr,PrefsRecord_t *RecordPtr,PrefState_e Pass)
{
	switch (Pass) {
	case PREFWRITE:		/* Write data */
		return StoreAParsedString(WorkPtr,(char *)RecordPtr->DataPtr);
	case PREFDEFAULT:	/* Default */
		strcpy((char *)RecordPtr->DataPtr,(char *)RecordPtr->Default);	/* Default */
		return WorkPtr;
	default:
		return GetAParsedString(WorkPtr,(char *)RecordPtr->DataPtr,RecordPtr->Count);
	}
}


/**********************************

	I will parse a Win95 style prefs file and after finding a record,
	I will call a proc to parse out the data.
	Return TRUE if the data was parsed properly, FALSE if I can't do it.

	This code handles Mac, IIgs, MSDOS and UNIX style text file
	images properly.

**********************************/

Word BURGERCALL ScanIniImage(IniIndex_t *IndexPtr,PrefsRecord_t *Record)
{
	Word8 Temp;			/* Ascii Temp */
	Word Size;			/* Length of a string to match */
	const char *Input;	/* Pointer to the input data */
	Word32 InputLength;	/* Length of the input data */

	Input = IndexPtr->ImagePtr;	/* Init the input pointer */
	InputLength = IndexPtr->ImageLength;
	if (!InputLength) {		/* No input data? */
		goto Darn;
	}


	/* First scan for the header token */

	Size = strlen(IndexPtr->Header);	/* Get the length of the file */
	for (;;) {
		if (Size>=InputLength) { 	/* Can't match it! Too small */
			goto Darn;
		}
		if (!memicmp(IndexPtr->Header,Input,Size)) {	/* Match the header? */
			InputLength = InputLength-Size;		/* Remove the header from parsing */
			Input = Input+Size;
			break;		/* I have a match! */
		}
		do {
			if (!InputLength) {	/* No more? */
				goto Darn;
			}
			--InputLength;
			Temp = ((Word8 *)Input)[0];	/* Get a char */
			++Input;
		} while (Temp!=10 && Temp!=13);	/* CR or line feed? */
	}

	/* Now scan for the item token, but stop if I hit a '[' */

	Size = strlen(Record->EntryName);	/* Get size of the token name */
	for (;;) {
		do {			/* Skip to the next CR or LF */
			if (!InputLength) {
				goto Darn;
			}
			--InputLength;
			Temp = ((Word8 *)Input)[0];		/* Find the next line */
			++Input;
		} while (Temp!=10 && Temp!=13);

		if (((Word8 *)Input)[0]=='[') {		/* Another token! */
			goto Darn;
		}
		if (Size>=InputLength) {		/* Can't match it! Too small! */
			goto Darn;
		}
		if (!memicmp(Record->EntryName,Input,Size)) {
			Temp = ((Word8 *)Input)[Size];		/* Last char */
			if (Temp==' ' || Temp=='=' || Temp==9) {	/* Properly ended token? */
				Input = Input+Size;			/* Remove the matched string */
				InputLength = InputLength-Size;
				break;
			}
		}
	}

	/* Remove any whitespace */

	do {		/* Remove whitespace */
		if (!InputLength) {
			goto Darn;
		}
		--InputLength;
		Temp = ((Word8 *)Input)[0];
		++Input;
	} while (Temp==' ' || Temp==9);

	/* Remove the '=' sign */

	if (Temp=='=') {		/* Discard an '=' sign? */
		do {		/* Remove whitespace some more... */
			if (!InputLength) {
				goto Darn;
			}
			--InputLength;
			Temp = ((Word8 *)Input)[0];
			++Input;
		} while (Temp==' ' || Temp==9);
	}

	/* Invoke deep magic and parse the data */

	--Input;		/* Move the pointer back one */
	if (Record->Proc((char *)Input,Record,PREFREAD)==Input) {	/* Convert to longword */
Darn:
		Record->Proc((char *)Input,Record,PREFDEFAULT);
		return FALSE;
	}
	return TRUE;
}

/**********************************

	Parse a longword from a window's style INI file image

**********************************/

Word32 BURGERCALL LongWordFromIniImage(const char *Header,const char *EntryName,const char *Input,Word32 InputLength)
{
	Word8 Temp;
	Word Size;

	if (!InputLength) {		/* No input data? */
		return (Word32)-1;
	}

	/* First scan for the header token */

	Size = strlen(Header);	/* Get the length of the file */
	for (;;) {
		if (Size>=InputLength) { 	/* Can't match it! Too small */
			return (Word32)-1;
		}
		if (!memcmp(Header,Input,Size)) {	/* Match the header? */
			break;		/* I have a match! */
		}
		do {
			Temp = ((Word8 *)Input)[0];	/* Get a char */
			++Input;
			if (!--InputLength) {	/* No more? */
				return (Word32)-1;		/* Not found */
			}
		} while (Temp!=10 && Temp!=13);	/* CR or line feed? */
	}

	/* Now scan for the item token, but die if '[' */

	Size = strlen(EntryName);	/* Get size of the token name */

	for (;;) {
		do {
			Temp = ((Word8 *)Input)[0];		/* Find the next line */
			++Input;
			if (!--InputLength) {
				return (Word32)-1;
			}
		} while (Temp!=10 && Temp!=13);

		if (((Word8 *)Input)[0]=='[') {		/* Another token! */
			return (Word32)-1;
		}
		if (Size>=InputLength) {		/* Can't match it! Too small! */
			return (Word32)-1;
		}
		if (!memcmp(EntryName,Input,Size)) {
			break;
		}
	}

	do {		/* Scan to the = sign */
		Temp = ((Word8 *)Input)[0];
		++Input;
		if (!--InputLength) {
			return (Word32)-1;
		}
	} while (Temp!='=');

	do {		/* Remove whitespace */
		Temp = ((Word8 *)Input)[0];
		++Input;
		if (!--InputLength) {
			return (Word32)-1;
		}
	} while (Temp!=' ' && Temp!=9);

	return AsciiToLongWord((char *)Input);	/* Convert to longword */
}

/**********************************

	Create a prefs file image in memory
	The ASCII text image is in UNIX format!

**********************************/

#define SLOP 100
#define CHUNK 4096

void * BURGERCALL PrefsCreateFileImage(PrefsTemplate_t *MyTemplate,Word32 *LengthPtr)
{
	Word32 Length;		/* Length of the buffer */
	Word Count;				/* Number of entries to print */
	char *FilePtr;			/* Pointer to the dest buffer */
	char *WorkPtr;			/* Work pointer */
	char *EndPtr;			/* Pointer to the end of the buffer - slop */
	PrefsRecord_t *MyEntry;	/* Pointer to a single record */

	FilePtr = (char *)AllocAPointer(CHUNK);
	WorkPtr = FilePtr;		/* Set the work pointer */
	if (!FilePtr) {			/* Couldn't get the default!?!? */
		goto Abort;
	}
	Length = CHUNK;			/* Init the buffer size */
	EndPtr = FilePtr+(CHUNK-SLOP);	/* Get the end pointer */

	WorkPtr = StoreAString(WorkPtr,MyTemplate->Header);	/* Save the header */
	WorkPtr[0] = '\n';		/* All a CR at the end */
	++WorkPtr;

	Count = MyTemplate->Count;		/* Any entries? */
	if (Count) {
		MyEntry = MyTemplate->ArrayPtr;	/* Get the array pointer */
		do {
			if (WorkPtr>=EndPtr) {		/* Out of bounds? */
				Word32 Mark;
				Mark = WorkPtr-FilePtr;	/* Get the offset */
				Length = Length+CHUNK;	/* New buffer size */
				FilePtr = (char *)ResizeAPointer(FilePtr,Length);	/* Grow the pointer */
				if (!FilePtr) {		/* Error? */
					goto Abort;		/* Crap! */
				}
				WorkPtr = FilePtr+Mark;		/* Reset the work pointer via offset */
				EndPtr = FilePtr+(Length-SLOP);		/* New end pointer */
			}
			WorkPtr = StoreAString(WorkPtr,MyEntry->EntryName);
			WorkPtr = StoreAString(WorkPtr," = ");
			WorkPtr = MyEntry->Proc(WorkPtr,MyEntry,PREFWRITE);
			WorkPtr[0] = '\n';		/* Add a LF at the end */
			++WorkPtr;
			++MyEntry;			/* Next entry */
		} while (--Count);		/* More? */
	}
Abort:;
	if (LengthPtr) {		/* Do I need the length returned? */
		Length = 0;			/* Assume bogus */
		if (FilePtr) {
			Length = WorkPtr-FilePtr;	/* Get the true data length */
		}
		*LengthPtr = Length;		/* Return the length (0 or true) */
	}
	return FilePtr;		/* Return the memory pointer */
}

/**********************************

	Create a prefs image in memory and then
	write it to disk.
	Return TRUE if error, FALSE if success.

**********************************/

Word BURGERCALL PrefsWriteFile(PrefsTemplate_t *MyTemplate,const char *FileName,Word AppendFlag)
{
	Word Result;		/* Flag for success or failure */
	FILE *fp;			/* File reference */
	Word32 Length;	/* Length of the prefs file image */
	Word8 *FilePtr;		/* Pointer to the prefs file image */

	Result = TRUE;
	FilePtr = (Word8 *)PrefsCreateFileImage(MyTemplate,&Length);	/* Create the file */
	if (FilePtr) {		/* Error? */
		fp = OpenAFile(FileName,AppendFlag ? "a" : "w");		/* Open the file */
		if (fp) {
			if (fwrite(FilePtr,1,Length,fp)==Length) {	/* Success? */
				Result = FALSE;
			}
			fclose(fp);		/* Close the file */
		}
		DeallocAPointer(FilePtr);		/* Release the memory */
	}
	return Result;		/* Return the answer */
}

/**********************************

	Parse a prefs file image and set all the fields
	to either a default state or the input value

**********************************/

void BURGERCALL PrefsParseFile(PrefsTemplate_t *MyTemplate,const char *FilePtr,Word32 Length)
{
	Word Count;			/* Loop count */
	PrefsRecord_t *MyEntry;		/* Pointer to the current parm */
	IniIndex_t MyIni;	/* Ini file header scan structure */

	Count = MyTemplate->Count;	/* How many to scan? */
	if (Count) {
		MyIni.Header = MyTemplate->Header;	/* Get the header ASCII */
		MyIni.ImagePtr = FilePtr;			/* Pointer to image */
		MyIni.ImageLength = Length;			/* Length of image */
		MyEntry = MyTemplate->ArrayPtr;		/* Init array pointer */
		do {
			ScanIniImage(&MyIni,MyEntry);	/* Parse data */
			++MyEntry;
		} while (--Count);			/* All done? */
	}
}

/**********************************

	Read a prefs file into memory and parse it out

**********************************/

Word BURGERCALL PrefsReadFile(PrefsTemplate_t *MyTemplate,const char *FileName)
{
	Word Result;		/* Did I read in a valid prefs file? */
	FILE *fp;			/* File reference */
	Word32 Length;	/* Length of input file */
	char *FilePtr;		/* Pointer to input file */

	Result = TRUE;		/* Assume error */
	Length = 0;		/* Force defaults */
	FilePtr = 0;		/* Assume no input data */
	fp = OpenAFile(FileName,"r");		/* Open the prefs file */
	if (fp) {			/* Opened ok? */
		Length = fgetfilesize(fp);		/* Get the length of the file */
		if (Length) {
			FilePtr = (char *)AllocAPointerClear(Length);	/* Init the memory */
			if (FilePtr) {
				fread(FilePtr,1,Length,fp);	/* Read it in */
				Result = FALSE;			/* Cool! */
			}
		}
		fclose(fp);		/* Release the file */
	}
	if (Result) {
		Length = 0;		/* Force defaults */
	}
	PrefsParseFile(MyTemplate,FilePtr,Length);		/* Parse it out */
	DeallocAPointer(FilePtr);		/* Release the memory */
	return Result;					/* Exit with possible error */
}



