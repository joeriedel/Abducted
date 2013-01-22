/**********************************

	Handle Win95 style .INI configuration files

**********************************/

#include "PgPrefFile.h"
#include "ShStream.h"
#include "StString.h"
#include "PfPrefs.h"
#include "MmMemory.h"
#include "ClStdLib.h"
#include "FmFile.h"
#include <string.h>
#include <stdio.h>

/**********************************

	The contents of these structures are PRIVATE!!!

**********************************/

typedef enum {LINEENTRY_ROOT,LINEENTRY_SECTION,LINEENTRY_ENTRY,LINEENTRY_COMMENT} LineEntry_e;

typedef struct LineEntry_t {		/* Generic, can have text, PrefFileEntry_t or PrefFileSection_t */
	LineEntry_e Type; 				/* Type of line present */
	struct LineEntry_t *Next;		/* Next in the list */
	struct LineEntry_t *Prev;		/* Previous entry in the list */
} LineEntry_t;

struct PrefFileEntry_t {
	LineEntry_t Root;				/* LINEENTRY_ENTRY */
	char *ValueText;	   			/* Pointer to text for value */
	char Text[1];					/* Pointer to text */
};

struct PrefFileSection_t {
	LineEntry_t Root;	  			/* LINEENTRY_SECTION */
	char Text[1];						/* Name of the section */
};

typedef struct PrefFileComment_t {
	LineEntry_t Root; 				/* LINEENTRY_COMMENT */
	char Text[1];					/* Raw text */
} PrefFileComment_t;

struct PrefFile_t {
	LineEntry_t Root;				/* LINEENTRY_ROOT */
};

const static char TrueWord[] = "TRUE";
const static char FalseWord[] = "FALSE";

/**********************************

	Insert a comment at the end of a PrefFile_t

**********************************/

static void BURGERCALL LineEntryAddComment(LineEntry_t *Input,const char *Text)
{
	PrefFileComment_t *CommentPtr;
	Word TextLen;

	TextLen = strlen(Text);
	CommentPtr = (PrefFileComment_t *)AllocAPointer(sizeof(PrefFileComment_t)+TextLen);
	if (CommentPtr) {
		FastMemCpy(CommentPtr->Text,Text,TextLen+1);
		CommentPtr->Root.Type = LINEENTRY_COMMENT;
		CommentPtr->Root.Next = Input->Next;
		CommentPtr->Root.Prev = Input;
		CommentPtr->Root.Next->Prev = &CommentPtr->Root;
		CommentPtr->Root.Prev->Next = &CommentPtr->Root;
	}
}

/**********************************

	Create and tear down a PrefFile_t
	structure

**********************************/

/**********************************

	Create a new PrefFile_t structure
	with nothing in it

**********************************/

PrefFile_t * BURGERCALL PrefFileNew(void)
{
	PrefFile_t *Input;
	Input = (PrefFile_t *)AllocAPointer(sizeof(PrefFile_t));
	if (Input) {
		Input->Root.Type = LINEENTRY_ROOT;
		Input->Root.Next = &Input->Root;
		Input->Root.Prev = &Input->Root;
	}
	return Input;
}

/**********************************

	Create a new pref file record and
	initialize it with entries from a text file
	in memory

**********************************/

PrefFile_t * BURGERCALL PrefFileNewFromMemory(const char *Data,Word32 Length)
{
	PrefFile_t *Input;
	Word8 OneLine[1024];			/* Work buffer for a single line of text */

	Input = PrefFileNew();		/* Init the structure */
	if (Input) {
		if (Length) {			/* Any input? */
			do {
				Word32 Len;
				Word Letter;

				Len = Length;	/* Save in memory */
							/* Grab a single line of input */

				ExtractAString2(Data,&Len,EXTRACTSTRDELIMITLF,(char *)OneLine,sizeof(OneLine));
				Length = Length-Len;	/* Remove from the stream */
				Data += Len;

				/* Let's parse the text (I can mangle it) */

				Letter = OneLine[0];
				if (Letter=='[') {		/* Section? */
					char *End;
					End = strchr((char *)OneLine,']');	/* Section end char? */
					if (End) {
						End[0] = 0;				/* End the string here */
						PrefFileAddSection(Input,(char *)(OneLine+1));	/* Make a section */
						continue;
					}
				}

				/* Is this a Foo = Bar entry? */
				/* Skip C++ and ASM comments */

				if (Letter!='=' && Letter!=';' && !(Letter=='/' && OneLine[1]=='/')) {
					char *ParmPtr;

					ParmPtr = strchr((char *)OneLine,'=');	/* Is there an equals in the middle? */
					if (ParmPtr) {
						ParmPtr[0] = 0;		/* Remove the equals */
						++ParmPtr;

						/* Remove whitespace */
						StrStripLeadingAndTrailingWhiteSpace((char *)OneLine);
						StrStripLeadingAndTrailingWhiteSpace(ParmPtr);
						/* Is there a token name? */
						if (OneLine[0]) {
							PrefFileSectionAddEntry((PrefFileSection_t *)Input->Root.Prev,(char *)OneLine,ParmPtr);
						}
						continue;		/* Next line */
					}

				}
				/* It's a comment, leave the line alone */
				LineEntryAddComment(Input->Root.Prev,(char *)OneLine);
			} while (Length);
		}
		return Input;		/* Return the structure */
	}
	return 0;
}

/**********************************

	Create a new pref file record and
	initialize it with entries from a
	text file on disk

**********************************/

PrefFile_t * BURGERCALL PrefFileNewFromFile(const char *FileName)
{
	Word32 Length;
	void *Data;
	PrefFile_t *Result;
	Data = LoadAFile(FileName,&Length);
	if (Data) {
		Result = PrefFileNewFromMemory(static_cast<char *>(Data),Length);	/* Parse the file */
		DeallocAPointer(Data);				/* Discard the file */
		return Result;
	}
	return 0;
}

/**********************************

	Create a new pref file record and
	initialize it with entries from a
	text file on disk, but if a disk error occurs
	create the structure anyways

**********************************/

PrefFile_t * BURGERCALL PrefFileNewFromFileAlways(const char *FileName)
{
	Word32 Length;
	void *Data;
	PrefFile_t *Result;
	Data = LoadAFile(FileName,&Length);
	if (Data) {
		Result = PrefFileNewFromMemory(static_cast<char *>(Data),Length);	/* Parse the file */
		DeallocAPointer(Data);				/* Discard the file */
		return Result;
	}
	return PrefFileNew();
}

/**********************************

	Dispose of the contents of a PrefFile structure
	This is a quick and dirty way. Note : I do not check
	the linked lists for validity, I just discard the memory
	in one fell swoop

**********************************/

void BURGERCALL PrefFileDelete(PrefFile_t *Input)
{
	LineEntry_t *LinePtr;
	if (Input) {						/* Is the record valid? */
		LinePtr = Input->Root.Next;		/* Get the first entry */
		if (LinePtr!=&Input->Root) {	/* Already back? */
			LineEntry_t *Next;
			do {
				switch (LinePtr->Type) {	/* Type of line entry */
				case LINEENTRY_ENTRY:
					DeallocAPointer(((PrefFileEntry_t *)LinePtr)->ValueText);
					break;
				}
				Next = LinePtr->Next;
				DeallocAPointer(LinePtr);
				LinePtr = Next;
			} while (LinePtr!=&Input->Root);
		}
		DeallocAPointer(Input);			/* Dispose of the root entry */
	}
}

/**********************************

	Routine to save a script file

**********************************/

Word BURGERCALL PrefFileSaveFile(PrefFile_t *Input,const char *FileName)
{
	StreamHandle_t Output;
	Word Result;
	FILE *fp;
	LineEntry_t *LinePtr;

	StreamHandleInitPut(&Output);
	LinePtr = Input->Root.Next;
	if (LinePtr!=&Input->Root) {
		do {
			switch (LinePtr->Type) {
			default:
			case LINEENTRY_COMMENT:
				StreamHandlePutStringNoZero(&Output,((PrefFileComment_t *)LinePtr)->Text);
				break;
			case LINEENTRY_SECTION:
				StreamHandlePutByte(&Output,'[');
				StreamHandlePutStringNoZero(&Output,((PrefFileSection_t *)LinePtr)->Text);
				StreamHandlePutByte(&Output,']');
				break;
			case LINEENTRY_ENTRY:
				StreamHandlePutStringNoZero(&Output,((PrefFileEntry_t *)LinePtr)->Text);
				StreamHandlePutStringNoZero(&Output," = ");
				StreamHandlePutStringNoZero(&Output,((PrefFileEntry_t *)LinePtr)->ValueText);
			}
			StreamHandlePutByte(&Output,'\n');
			LinePtr = LinePtr->Next;
		} while (LinePtr!=&Input->Root);
	}
	StreamHandleEndSave(&Output);
	if (!StreamHandleGetErrorFlag(&Output)) {
		fp = OpenAFile(FileName,"w");		/* Write as a TEXT file */
		if (fp) {
			fwrite(LockAHandle((void **)Output.DataHandle),1,StreamHandleGetSize(&Output),fp);
			fclose(fp);
			Result = FALSE;
		}
	}
	StreamHandleDestroy(&Output);
	return Result;
}

/**********************************

	Traverse the linked list for a section
	that has the requested name

**********************************/

PrefFileSection_t * BURGERCALL PrefFileFindSection(PrefFile_t *Input,const char *SectionName)
{
	LineEntry_t *LinePtr;

	LinePtr = Input->Root.Next;
	if (LinePtr!=&Input->Root) {
		do {
			if (LinePtr->Type==LINEENTRY_SECTION) {
				if (!stricmp(((PrefFileSection_t *)LinePtr)->Text,SectionName)) {
					return (PrefFileSection_t *)LinePtr;
				}
			}
			LinePtr = LinePtr->Next;
		} while (LinePtr!=&Input->Root);
	}
	return 0;
}

/**********************************

	Traverse the linked list for a section
	that has the requested name, if the name doesn't exist
	create it

**********************************/

PrefFileSection_t * BURGERCALL PrefFileFindSectionAlways(PrefFile_t *Input,const char *SectionName)
{
	PrefFileSection_t *SectionPtr;

	SectionPtr = PrefFileFindSection(Input,SectionName);
	if (!SectionPtr) {
		SectionPtr = PrefFileAddSection(Input,SectionName);
	}
	return SectionPtr;
}

/**********************************

	Make a list of all the sections that are
	present and return the full list.

**********************************/

char *BURGERCALL PrefFileGetList(PrefFile_t *Input)
{
	LineEntry_t *LinePtr;
	Word Size;
	char *Output;
	
	Size = 1;						/* 1 need at minimum, 1 byte */

	LinePtr = Input->Root.Next;
	if (LinePtr!=&Input->Root) {	/* Any data present? */
		do {
			if (LinePtr->Type==LINEENTRY_SECTION) {			/* A section name? */
				Word Len;
				Len = strlen(((PrefFileSection_t *)LinePtr)->Text)+1;	/* Get the length of the string */
				Size+=Len;									/* Add to the running total */
			}
			LinePtr = LinePtr->Next;						/* Next entry */
		} while (LinePtr!=&Input->Root);					/* Back to the beginning? */
	}

	Output = (char *)AllocAPointer(Size);					/* Now get the output buffer */
	if (Output) {
		char *WorkPtr;
		WorkPtr = Output;				/* Save the work pointer */
		LinePtr = Input->Root.Next;
		if (LinePtr!=&Input->Root) {	/* Any data present? */
			do {
				if (LinePtr->Type==LINEENTRY_SECTION) {			/* A section name? */
					Word Len2;
					Len2 = strlen(((PrefFileSection_t *)LinePtr)->Text)+1;			/* Get the string's length */
					FastMemCpy(WorkPtr,((PrefFileSection_t *)LinePtr)->Text,Len2);	/* Copy it */
					WorkPtr+=Len2;								/* Move the work pointer */
				}
				LinePtr = LinePtr->Next;
			} while (LinePtr!=&Input->Root);					/* Next */
		}
		WorkPtr[0] = 0;		/* End the string here */
	}
	return Output;			/* Return the string */
}

/**********************************

	Given a section name and an entry
	name, return the actual entry found

**********************************/

PrefFileEntry_t * BURGERCALL PrefFileSectionFindEntry(PrefFileSection_t *Input,const char *EntryName)
{
	LineEntry_t *LinePtr;
	LinePtr = Input->Root.Next;
	if (LinePtr->Type>LINEENTRY_SECTION) {
		do {
			if (LinePtr->Type==LINEENTRY_ENTRY) {
				if (!stricmp(((PrefFileEntry_t *)LinePtr)->Text,EntryName)) {
					return (PrefFileEntry_t *)LinePtr;
				}
			}
			LinePtr = LinePtr->Next;
		} while (LinePtr->Type>LINEENTRY_SECTION);
	}
	return 0;
}


/**********************************

	Delete a single entry

**********************************/

static void BURGERCALL PrefFileDeleteLine(LineEntry_t *LinePtr)
{
	LineEntry_t *Next;
	LineEntry_t *Prev;

	/* First, discard the contents */

	switch (LinePtr->Type) {	/* Type of line entry */
	case LINEENTRY_ENTRY:
		DeallocAPointer(((PrefFileEntry_t *)LinePtr)->ValueText);
		break;
	}

	/* Now, remove from the linked list */

	Next = LinePtr->Next;		/* Is the list empty? */
	Prev = LinePtr->Prev;
	Next->Prev = Prev;
	Prev->Next = Next;
	DeallocAPointer(LinePtr);
}

/**********************************

	Delete all the entries that belong to a specific section

**********************************/

void BURGERCALL PrefFileDeleteSection(PrefFile_t *Input,const char *SectionName)
{
	PrefFileSection_t *SectionPtr;
	SectionPtr = PrefFileFindSection(Input,SectionName);
	if (SectionPtr) {
		PrefFileDeletePrefFileSection(Input,SectionPtr);
	}
}

/**********************************

	Delete all the entries that belong to a specific section

**********************************/

void BURGERCALL PrefFileDeletePrefFileSection(PrefFile_t * /* Input */,PrefFileSection_t *SectionPtr)
{
	LineEntry_t *Next;
	LineEntry_t *WorkPtr;

	WorkPtr = &SectionPtr->Root;		/* Pointer to the first entry */
	do {
		Next = WorkPtr->Next;			/* Follow the linked list */
		PrefFileDeleteLine(WorkPtr);	/* Kill the line */
		WorkPtr = Next;					/* Next link */
	} while (WorkPtr->Type>LINEENTRY_SECTION);
}

/**********************************

	Create a new section but don't insert data

**********************************/

PrefFileSection_t *BURGERCALL PrefFileAddSection(PrefFile_t *Input,const char *SectionName)
{
	PrefFileSection_t *SectionPtr;
	Word TextLen;

	TextLen = strlen(SectionName);
	SectionPtr = (PrefFileSection_t *)AllocAPointer(sizeof(PrefFileSection_t)+TextLen);
	if (SectionPtr) {
		FastMemCpy(SectionPtr->Text,SectionName,TextLen+1);
		SectionPtr->Root.Type = LINEENTRY_SECTION;
		SectionPtr->Root.Next = &Input->Root;
		SectionPtr->Root.Prev = Input->Root.Prev;
		SectionPtr->Root.Next->Prev = &SectionPtr->Root;
		SectionPtr->Root.Prev->Next = &SectionPtr->Root;
	}
	return SectionPtr;
}

/**********************************

	Quick routine to determine if a
	section/entry is present

**********************************/

Word BURGERCALL PrefFileIsEntryPresent(PrefFile_t *Input,const char *SectionName,const char *EntryName)
{
	PrefFileSection_t *SectionPtr;
	SectionPtr = PrefFileFindSection(Input,SectionName);
	if (SectionPtr) {
		if (PrefFileSectionFindEntry(SectionPtr,EntryName)) {
			return TRUE;
		}
	}
	return FALSE;
}

/**********************************

	Given a sub-section, find all the names of the entries
	present

**********************************/

char *BURGERCALL PrefFileSectionGetList(PrefFileSection_t *Input)
{
	LineEntry_t *LinePtr;
	char *Output;
	Word Size;
	
	Size = 1;							/* At least 1 byte */
	LinePtr = Input->Root.Next;
	if (LinePtr->Type>LINEENTRY_SECTION) {
		do {
			if (LinePtr->Type==LINEENTRY_ENTRY) {		/* Valid entry */
				Word Len;
				Len = strlen(((PrefFileEntry_t *)LinePtr)->Text)+1;	/* Running total */
				Size+=Len;
			}
			LinePtr = LinePtr->Next;
		} while (LinePtr->Type>LINEENTRY_SECTION);		/* Next section? */
	}

	Output = (char *)AllocAPointer(Size);				/* Allocate the buffer */
	if (Output) {
		char *WorkPtr;
		WorkPtr = Output;
		LinePtr = Input->Root.Next;
		if (LinePtr->Type>LINEENTRY_SECTION) {
			do {
				if (LinePtr->Type==LINEENTRY_ENTRY) {	/* Entry? */
					Word Len;
					Len = strlen(((PrefFileEntry_t *)LinePtr)->Text)+1;
					FastMemCpy(WorkPtr,((PrefFileEntry_t *)LinePtr)->Text,Len);	/* Copy the string */
					WorkPtr+=Len;						/* Next entry */
				}
				LinePtr = LinePtr->Next;
			} while (LinePtr->Type>LINEENTRY_SECTION);
		}
		WorkPtr[0] = 0;					/* Terminator */
	}
	return Output;						/* Return the buffer */
}

/**********************************

	These routines will extract information from
	pref entries

**********************************/

char *BURGERCALL PrefFileSectionGetRaw(PrefFileSection_t *Input,const char *EntryName)
{
	PrefFileEntry_t *EntryPtr;

	EntryPtr = PrefFileSectionFindEntry(Input,EntryName);
	if (EntryPtr) {
		return EntryPtr->ValueText;
	}
	return 0;
}

/**********************************

	Return a BOOLEAN value (TRUE or FALSE)

**********************************/

Word BURGERCALL PrefFileSectionGetBoolean(PrefFileSection_t *Input,const char *EntryName,Word Default)
{
	char *Text;
	char *Dest;
	Word32 Val;

	Text = PrefFileSectionGetRaw(Input,EntryName);
	if (Text) {
		if (!stricmp(TrueWord,Text)) {
			return TRUE;
		}
		if (!stricmp(FalseWord,Text)) {
			return FALSE;
		}
		Val = AsciiToLongWord2(Text,&Dest);
		if (Dest!=Text) {
			if (Val) {
				return TRUE;
			}
			return FALSE;
		}
	}
	return Default;
}

/**********************************

	Return an unsigned integer value

**********************************/

Word BURGERCALL PrefFileSectionGetWord(PrefFileSection_t *Input,const char *EntryName,Word Default,Word Min,Word Max)
{
	char *Text;
	char *Dest;
	Word32 Val;

	Text = PrefFileSectionGetRaw(Input,EntryName);
	if (Text) {
		Val = AsciiToLongWord2(Text,&Dest);
		if (Dest!=Text) {
			if (Min && Max) {
				if (Val<Min) {
					return Min;
				}
				if (Val>Max) {
					return Max;
				}
			}
			return Val;
		}
	}
	return Default;
}

/**********************************

	Return an integer value

**********************************/

int BURGERCALL PrefFileSectionGetInt(PrefFileSection_t *Input,const char *EntryName,int Default,int Min,int Max)
{
	char *Text;
	char *Dest;
	long Val;

	Text = PrefFileSectionGetRaw(Input,EntryName);
	if (Text) {
		Val = AsciiToLongWord2(Text,&Dest);
		if (Dest!=Text) {
			if (Min && Max) {
				if (Val<Min) {
					return Min;
				}
				if (Val>Max) {
					return Max;
				}
			}
			return Val;
		}
	}
	return Default;
}

/**********************************

	Return a floating point value

**********************************/

float BURGERCALL PrefFileSectionGetFloat(PrefFileSection_t *Input,const char *EntryName,float Default,float Min,float Max)
{
	char *Text;
	float Val;

	Text = PrefFileSectionGetRaw(Input,EntryName);
	if (Text) {
		if (sscanf(Text,"%f",&Val)==1) {
			if (Min && Max) {
				if (Val<Min) {
					return Min;
				}
				if (Val>Max) {
					return Max;
				}
			}
			return Val;
		}
	}
	return Default;
}

/**********************************

	Return a double floating point value

**********************************/

double BURGERCALL PrefFileSectionGetDouble(PrefFileSection_t *Input,const char *EntryName,double Default,double Min,double Max)
{
	char *Text;
	double Val;

	Text = PrefFileSectionGetRaw(Input,EntryName);
	if (Text) {
		if (sscanf(Text,"%lf",&Val)==1) {
			if (Min && Max) {
				if (Val<Min) {
					return Min;
				}
				if (Val>Max) {
					return Max;
				}
			}
			return Val;
		}
	}
	return Default;
}

/**********************************

	Get an ASCII string

**********************************/

void BURGERCALL PrefFileSectionGetString(PrefFileSection_t *Input,const char *EntryName,const char *Default,char *Buffer,Word BufferSize)
{
	char *Text;
	Word Len;

	Text = PrefFileSectionGetRaw(Input,EntryName);
	if (Text) {
		if (GetAParsedString(Text,Buffer,BufferSize)!=Text) {		/* Is this a "foo" string? */
			return;			/* It was delimited by quotes */
		}
		if (((Word8 *)Text)[0]) {	/* Does the string contain text? */
			Default = Text;		/* Use the string as is */
		}
	}

	if (BufferSize) {		/* Is there a buffer? */
		if (!Default) {
			Len = 0;
		} else {
			Len = strlen(Default);	/* Size of the string */
			if (Len>=BufferSize) {		/* Too big? */
				Len = BufferSize-1;		/* Truncate */
			}
		 	FastMemCpy(Buffer,Default,Len);		/* Copy it */
		}
		Buffer[Len] = 0;		/* Force ending byte */
	}
}

/**********************************

	Get dual ASCII strings

**********************************/

void BURGERCALL PrefFileSectionGetDualString(PrefFileSection_t *Input,const char *EntryName,const char *Default,char *Buffer,Word BufferSize,const char *Default2,char *Buffer2,Word BufferSize2)
{
	char *Text;
	Word Len;
	Word FirstOk;

	Text = PrefFileSectionGetRaw(Input,EntryName);
	FirstOk = FALSE;
	if (Text) {
		char *Temp;
		Temp = GetAParsedString(Text,Buffer,BufferSize);
		if (Temp!=Text) {
			FirstOk = TRUE;
			Temp = ParseBeyondWhiteSpace(Temp);
			if (GetAParsedString(Temp,Buffer2,BufferSize2)!=Temp) {
				return;
			}
		}
	}

	if (!FirstOk && BufferSize) {		/* Is there a buffer? */
		if (!Default) {
			Len = 0;
		} else {
			Len = strlen(Default);	/* Size of the string */
			if (Len>=BufferSize) {		/* Too big? */
				Len = BufferSize-1;		/* Truncate */
			}
		 	FastMemCpy(Buffer,Default,Len);		/* Copy it */
		}
		Buffer[Len] = 0;		/* Force ending byte */
	}

	if (BufferSize2) {		/* Is there a buffer? */
		if (!Default) {
			Len = 0;
		} else {
			Len = strlen(Default2);	/* Size of the string */
			if (Len>=BufferSize2) {		/* Too big? */
				Len = BufferSize2-1;		/* Truncate */
			}
		 	FastMemCpy(Buffer,Default2,Len);		/* Copy it */
		}
		Buffer2[Len] = 0;		/* Force ending byte */
	}
}

/**********************************

	Get an ASCII string

**********************************/

void BURGERCALL PrefFileSectionGetMem(PrefFileSection_t *Input,const char *EntryName,const Word8 *Default,Word8 *Buffer,Word BufferSize)
{
	char *Text;
	Word Len;

	if (BufferSize) {
		Text = PrefFileSectionGetRaw(Input,EntryName);
		if (Text) {
			char *RetVal;
			Word8 *WorkPtr;
			Len = BufferSize;
			WorkPtr = Buffer;
			do {
				Text = ParseBeyondWhiteSpace(Text);		/* Get rid of spaces */
				WorkPtr[0] = (Word8)AsciiToLongWord2(Text,&RetVal);
				if (Text==RetVal) {
					goto Abort;
				}
				Text = RetVal;
				if (Text[0]==',') {
					++Text;		/* Skip commas */ 
				}
				++WorkPtr;
			} while (--Len);
			return;
		}
Abort:
		if (Default) {			/* Is there a default buffer */
			FastMemCpy(Buffer,Default,BufferSize);
		}
	}
}

/**********************************

	Get an array of words

**********************************/

void BURGERCALL PrefFileSectionGetWordArray(PrefFileSection_t *Input,const char *EntryName,const Word *Default,Word *Buffer,Word BufferSize)
{
	char *Text;
	Word Len;

	if (BufferSize) {
		Text = PrefFileSectionGetRaw(Input,EntryName);
		if (Text) {
			char *RetVal;
			Word *WorkPtr;
			Len = BufferSize;
			WorkPtr = Buffer;
			do {
				Text = ParseBeyondWhiteSpace(Text);		/* Get rid of spaces */
				WorkPtr[0] = (Word)AsciiToLongWord2(Text,&RetVal);
				if (Text==RetVal) {
					goto Abort;
				}
				Text = RetVal;
				if (Text[0]==',') {
					++Text;		/* Skip commas */ 
				}
				++WorkPtr;
			} while (--Len);
			return;
		}
Abort:
		if (Default) {			/* Is there a default buffer */
			FastMemCpy(Buffer,Default,sizeof(Word)*BufferSize);
		}
	}
}

/**********************************

	The following functions will set records
	in an entry list

**********************************/

/**********************************

	Create a new entry in this section.
	I do NOT check if the entry name is already present

**********************************/

void BURGERCALL PrefFileSectionAddEntry(PrefFileSection_t *Input,const char *EntryName,const char *Default)
{
	LineEntry_t *LinePtr;
	PrefFileEntry_t *EntryPtr;
	Word TextLen;

	TextLen = strlen(EntryName);
	EntryPtr = (PrefFileEntry_t *)AllocAPointer(sizeof(PrefFileEntry_t)+TextLen);
	if (EntryPtr) {
		FastMemCpy(EntryPtr->Text,EntryName,TextLen+1);
		EntryPtr->ValueText = StrCopy(Default);
		EntryPtr->Root.Type = LINEENTRY_ENTRY;

		LinePtr = Input->Root.Next;
		if (LinePtr->Type>LINEENTRY_SECTION) {
			do {
				LinePtr = LinePtr->Next;
			} while (LinePtr->Type>LINEENTRY_SECTION);
		}
		EntryPtr->Root.Next = LinePtr;
		EntryPtr->Root.Prev = LinePtr->Prev;
		EntryPtr->Root.Next->Prev = &EntryPtr->Root;
		EntryPtr->Root.Prev->Next = &EntryPtr->Root;
	}
}

/**********************************

	Find an entry and replace the text.
	If an entry wasn't found, then create it.

**********************************/

void BURGERCALL PrefFileSectionPutRaw(PrefFileSection_t *Input,const char *EntryName,const char *RawString)
{
	PrefFileEntry_t *EntryPtr;

	EntryPtr = PrefFileSectionFindEntry(Input,EntryName);
	if (EntryPtr) {
		DeallocAPointer(EntryPtr->ValueText);		/* Discard the old data */
		EntryPtr->ValueText = StrCopy(RawString);	/* Insert new data */
		return;
	}
	PrefFileSectionAddEntry(Input,EntryName,RawString);
}

/**********************************

	Store a Bool value

**********************************/

void BURGERCALL PrefFileSectionPutBoolean(PrefFileSection_t *Input,const char *EntryName,Word Data)
{
	if (Data) {
		PrefFileSectionPutRaw(Input,EntryName,TrueWord);
		return;
	}
	PrefFileSectionPutRaw(Input,EntryName,FalseWord);
}

/**********************************

	Store an unsigned value

**********************************/

void BURGERCALL PrefFileSectionPutWord(PrefFileSection_t *Input,const char *EntryName,Word Data)
{
	char Text[16];
	LongWordToAscii(Data,Text);
	PrefFileSectionPutRaw(Input,EntryName,Text);
}

/**********************************

	Store a value as hex

**********************************/

void BURGERCALL PrefFileSectionPutWordHex(PrefFileSection_t *Input,const char *EntryName,Word Data)
{
	char Text[16];
	Text[0] = '0';
	Text[1] = 'x';
	LongWordToAsciiHex(Data,Text+2);
	PrefFileSectionPutRaw(Input,EntryName,Text);
}

/**********************************

	Store a value as hex

**********************************/

void BURGERCALL PrefFileSectionPutInt(PrefFileSection_t *Input,const char *EntryName,int Data)
{
	char Text[16];
	longToAscii(Data,Text);
	PrefFileSectionPutRaw(Input,EntryName,Text);
}

/**********************************

	Store a float

**********************************/

void BURGERCALL PrefFileSectionPutFloat(PrefFileSection_t *Input,const char *EntryName,float Data)
{
	char Text[64];

	sprintf(Text,"%g",Data);
	PrefFileSectionPutRaw(Input,EntryName,Text);
}

/**********************************

	Store a double

**********************************/

void BURGERCALL PrefFileSectionPutDouble(PrefFileSection_t *Input,const char *EntryName,double Data)
{
	char Text[64];

	sprintf(Text,"%g",Data);
	PrefFileSectionPutRaw(Input,EntryName,Text);
}

/**********************************

	Store a single string

**********************************/

void BURGERCALL PrefFileSectionPutString(PrefFileSection_t *Input,const char *EntryName,const char *Data)
{
	char Text[256];		/* Buffer for most cases */
	char *TextPtr;
	Word Len;

	Len = (strlen(Data)*2)+4;		/* Length of buffer needed */
	if (Len<256) {					/* Quick & Dirty? */
		TextPtr = StoreAParsedString(Text,Data);		/* Convert to string */
		TextPtr[0] = 0;									/* Terminate */
		PrefFileSectionPutRaw(Input,EntryName,Text);	/* Place in list */
	} else {
		TextPtr = (char *)AllocAPointer(Len);			/* Allocate a buffer */
		if (TextPtr) {
			char *FooPtr;
			FooPtr = StoreAParsedString(TextPtr,Data);	/* Operate on the buffer */
			FooPtr[0] = 0;
			PrefFileSectionPutRaw(Input,EntryName,TextPtr);
			DeallocAPointer(TextPtr);					/* Discard the buffer */
		}
	}
}

/**********************************

	Store two seperate strings

**********************************/

void BURGERCALL PrefFileSectionPutDualString(PrefFileSection_t *Input,const char *EntryName,const char *Data,const char *Data2)
{
	char Text[256];		/* Buffer for most cases */
	char *TextPtr;
	Word Len;

	Len = ((strlen(Data)+strlen(Data2))*2)+4;		/* Length of buffer needed */

	if (Len<256) {					/* Quick & Dirty? */
		TextPtr = StoreAParsedString(Text,Data);		/* Convert to string */
		TextPtr[0] = ' ';
		TextPtr = StoreAParsedString(TextPtr+1,Data2);
		TextPtr[0] = 0;									/* Terminate */
		PrefFileSectionPutRaw(Input,EntryName,Text);	/* Place in list */
	} else {
		TextPtr = (char *)AllocAPointer(Len);			/* Allocate a buffer */
		if (TextPtr) {
			char *FooPtr;
			FooPtr = StoreAParsedString(TextPtr,Data);	/* Operate on the buffer */
			FooPtr[0] = ' ';
			FooPtr = StoreAParsedString(FooPtr+1,Data2);
			FooPtr[0] = 0;
			PrefFileSectionPutRaw(Input,EntryName,TextPtr);
			DeallocAPointer(TextPtr);					/* Discard the buffer */
		}
	}
}

/**********************************

	Store an array of bytes

**********************************/

void BURGERCALL PrefFileSectionPutMem(PrefFileSection_t *Input,const char *EntryName,const Word8 *Data,Word Length)
{
	char Text[256];		/* Buffer for most cases */
	char *TextPtr;
	char *FooPtr;
	Word Len;

	Len = (Length*5)+1;		/* Length of buffer needed */
	if (Len<256) {					/* Quick & Dirty? */
		TextPtr = Text;
	} else {
		TextPtr = (char *)AllocAPointer(Len);
		if (!TextPtr) {
			return;
		}
	}
	FooPtr = TextPtr;
	if (Length) {
		do {
			FooPtr[0] = '0';
			FooPtr[1] = 'x';
			FooPtr[2] = NibbleToAscii[Data[0]>>4];
			FooPtr[3] = NibbleToAscii[Data[0]&15];
			FooPtr[4] = ' ';
			FooPtr=FooPtr+5;
			++Data;
		} while (--Length);
		--FooPtr;
	}
	FooPtr[0] = 0;
	PrefFileSectionPutRaw(Input,EntryName,TextPtr);
	if (Len>=256) {
		DeallocAPointer(TextPtr);					/* Discard the buffer */
	}
}

/**********************************

	Store an array of Words

**********************************/

void BURGERCALL PrefFileSectionPutWordArray(PrefFileSection_t *Input,const char *EntryName,const Word *Data,Word Count)
{
	char Text[256];		/* Buffer for most cases */
	char *TextPtr;
	char *FooPtr;
	Word Len;

	Len = (Count*12)+1;		/* Length of buffer needed */
	if (Len<256) {			/* Quick & Dirty? */
		TextPtr = Text;
	} else {
		TextPtr = (char *)AllocAPointer(Len);
		if (!TextPtr) {
			return;
		}
	}
	FooPtr = TextPtr;
	if (Count) {
		do {
			FooPtr = LongWordToAscii(Data[0],FooPtr);
			FooPtr[0] = ',';
			++FooPtr;
			++Data;
		} while (--Count);
		--FooPtr;		/* Remove the final comma */
	}
	FooPtr[0] = 0;
	PrefFileSectionPutRaw(Input,EntryName,TextPtr);
	if (Len>=256) {
		DeallocAPointer(TextPtr);					/* Discard the buffer */
	}
}
