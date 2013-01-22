#include "RzRez.h"
#include <BREndian.hpp>
#include "MmMemory.h"
#include "ClStdLib.h"
#include "FmFile.h"
#include "GrGraphics.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define REZFLAGEXTERNAL 1		/* TRUE if external files are allowed */
#define REZFLAGNOCACHE 2		/* TRUE if data caching is disabled */

/* File structure for a burgerlib resource file */

typedef struct RezFileHeader_t {
	char Name[4];		/* BRGR */
	Word32 Count;		/* Number of entries */
	Word32 MemSize;	/* Amount of memory the entries take up */
} RezFileHeader_t;

#define MAXBUFFER 65536UL	/* Size of decompression buffer */

/* Defines for the bits in FileOffset */

#define REZOFFSETFIXED 0x80000000UL			/* True if load in fixed memory */
#define REZOFFSETDECOMPMASK 0x60000000UL	/* Mask for decompressors */
#define REZOFFSETMASK 0x1FFFFFFFUL			/* Big enough for 1 Gig file (GD-ROM or CDRom) */

/* Defines for the bits in NameOffset */

#define NAMEOFFSETMASK 0x003FFFFFUL			/* Filename offset */
#define NAMEOFFSETTESTED 0x00400000UL		/* True if the filename was checked */
#define NAMEOFFSETFILE 0x00800000UL			/* True if a file was found */
#define NAMEOFFSETREFCOUNT 0xFF000000UL		/* Refcount mask */
#define NAMEOFFSETREFSHIFT 24				/* Bits to shift for the refcount */
#define NAMEOFFSETREFADD (1UL<<NAMEOFFSETREFSHIFT)	/* 1 in refcount format */

typedef struct RezEntry_t {
	void **MemHand;			/* Handle to data in memory */
	Word32 FileOffset;	/* Offset into the rez file */
	Word32 Length;		/* Length of the data on disk */
	Word32 NameOffset;	/* Offset to the filename, Flags and Refcount */
} RezEntry_t;				/* Entry in memory */

typedef struct RezGroup_t {
	Word32 RezNum;		/* Base Resource number */
	Word32 Count;			/* Number of entries */
	RezEntry_t Array[1];	/* First entry */
} RezGroup_t;

typedef struct RezName_t {	/* Sort list of filenames */
	Word32 HashOffset;	/* Offset to the filename */
	Word RezNum;			/* Resource number associated with this entry */
} RezName_t;

#ifdef __cplusplus
extern "C" {
static void ANSICALL ResourceExit(void);
static int ANSICALL RezSortNames(const void *One,const void *Two);
}
#endif

static Word8 OnceHit;		/* Set to true to init the shut down code */

/********************************

	Internal routines, these are not intended for public consumption
	
********************************/


/********************************

	Scan for a resource entry in the resource map
	The entry is a pointer to an UNLOCKED handle!
	Therefore, you cannot assume the pointer is valid after
	you make another subroutine call.

********************************/

static RezEntry_t * BURGERCALL ResourceScan(RezHeader_t *Input,Word RezNum)
{
	Word GroupCount;
	RezEntry_t *Result;
	Result = 0;						/* Nothing to return */
	if (Input) {
		GroupCount = Input->Count;		/* Get the group entry count */
		if (GroupCount) {				/* Any resources available? */
			RezGroup_t *MainPtr;
			MainPtr = Input->GroupHandle[0];		/* Get the main pointer (Not locked) */
			do {
				Word Temp;
				Word Count;
				Temp = RezNum-MainPtr->RezNum;		/* Get the offset from base number */
				Count = MainPtr->Count;
				if (Temp<Count) {			/* In this range? */
					Result = &MainPtr->Array[Temp];	/* Return the pointer to the entry */
					break;
				}
				MainPtr = (RezGroup_t *)&MainPtr->Array[Count];	/* Next? */
			} while (--GroupCount);		/* Count down */
		}
	}
	return Result;						/* No good! */
}

/**********************************

	By traversing a valid RezHeader_t, determine
	the number of bytes it would take to store a RezGroup_t
	structure (Used for rebuilding the structure when
	an entry is added or removed)

	Note : Determined to be obsolete.
	
**********************************/

#if 0
static Word32 BURGERCALL ResourceCalcSizeOfDictionary(RezHeader_t *Input)
{
	Word32 Total;		/* Word8 total */
	Word GroupCount;

	Total = 0;						/* Nothing to start with */
	if (Input) {
		GroupCount = Input->Count;		/* Get the entry count */
		if (GroupCount) {				/* Any resources available? */
			RezGroup_t *MainPtr;
			char *AsciiPtr;
			
			MainPtr = Input->GroupHandle[0];		/* Pointer to the data */
			AsciiPtr = (char *)MainPtr;				/* Pointer to the ASCII data base */
			do {
				Word Count;
				RezEntry_t *Entry;
				Count = MainPtr->Count;				/* Count of this group's size */
				
				/* Add in the array for the group */
				
				Total += (Count*sizeof(RezEntry_t))+(sizeof(RezGroup_t)-sizeof(RezEntry_t));
				Entry = &MainPtr->Array[0];			/* Pointer to the first entry */
		
				/* Now check each entry for a name string */
				
				do {
					Word32 Offset2;
					Offset2 = Entry->NameOffset&NAMEOFFSETMASK;
					if (Offset2) {			/* Name? */
						Total += strlen(AsciiPtr+Offset2)+1;	/* Get the string's length */
					}
					++Entry;						/* Next entry */
				} while (--Count);
				MainPtr = (RezGroup_t *)Entry;		/* Next group */
			} while (--GroupCount);					/* Count down */
		}
	}
	return Total;		/* Here is the total */
}
#endif

/**********************************

	Using a resource name list (Sorted so that
	a binary search can be performed), look
	if there is a match in the resource list
	Return TRUE if an entry was found, FALSE if
	there was no match. However, if FALSE is 
	returned, then the entry that will FOLLOW
	the string is returned for new entry insertion

**********************************/

static Word BURGERCALL ResourceFindName(RezHeader_t *Input,const char *RezName,RezName_t **Result)
{
	RezName_t **NameHand;				/* Handle to base list */
	RezName_t *RezNamePtr;				/* Work pointer */
	
	RezNamePtr = 0;						/* Assume error */
	if (Input) {
		NameHand = Input->RezNames;			/* Any names in this resource file? */
		if (NameHand) {						/* Cool... */
			RezName_t *NamePtr;				/* Pointer to the master name list */
			char *TextPtr;					/* Root pointer to text buffer */
			Word MinRec,MaxRec;				/* Bounds for binary search */
			int Test;						/* For string compare test */

			/* If the filename has a prefix "20:FileName.txt" */
			/* Then remove the prefix so it can also match */

			MinRec = ((Word8 *)RezName)[0];
			if (MinRec>='0' && MinRec<('9'+1)) {	/* First char a number? */
				const char *AuxName;				/* Auxilary RezName */
				AuxName = strchr(RezName,':');		/* Find the ':' */
				if (AuxName) {						/* Found it? */
					RezName = AuxName+1;			/* Index past it */
				}
			}

			/* Perform a binary search to find the filename */

			NamePtr = NameHand[0];			/* Deref the handle */
			MinRec = 0;						/* Make the bounds */
			MaxRec = Input->RezNameCount;
			TextPtr = (char *)(Input->GroupHandle[0]);		/* Base pointer to text strings */
			do {
				Word EntryNum;

				EntryNum = ((MaxRec-MinRec)>>1)+MinRec;		/* This is record I will test */
				RezNamePtr = &NamePtr[EntryNum];			/* Check this one */
				Test = stricmp(TextPtr+RezNamePtr->HashOffset,RezName);		/* Test... */
				if (!Test) {								/* Found a match */
					Result[0] = RezNamePtr;					/* Return this record */
					return TRUE;							/* I found a match */
				}
				if (Test>0) {		/* Which way to go? */
					MaxRec = EntryNum;		/* Must look lower */
				} else {
					MinRec = EntryNum+1;	/* Must look higher */
				}
			} while (MinRec<MaxRec);		/* Oh oh... */
			if (Test<0) {
				++RezNamePtr;				/* Must be before this one... */
			}
		}
	}
	Result[0] = RezNamePtr;				/* Return the record I last tested */
	return FALSE;						/* No good! */
}

/**********************************

	Create a dictionary from the data read in
	I will return a RezGroup_t **
	Note : I support the old data type where I had
	unique types in addition to ID's. I applied a hack
	that will remap type "5" to ID+5000. This way
	old games like Killing Time can use the new burgerlib
	without having to update the .REZ files.

**********************************/

static RezGroup_t **RezGroupNew(const Word8 *RezData,const RezFileHeader_t *MyHeader,Word SwapFlag,Word32 StartOffset)
{
	void **RezHandle;		/* Handle to rez dictionary */
	Word32 NewLength;		/* Size of the dictionary */
	Word32 Adjust;		/* Additional data to add */
	RezGroup_t *MainPtr;	/* Pointer to group data */
	RezEntry_t *AuxPtr;		/* Pointer to entry data */
	const Word8 *WorkPtr;			/* Pointer to raw data */
	Word GroupCount;
	Word32 Count;

	GroupCount = MyHeader->Count;		/* Get the entry count */
	if (GroupCount) {

	/* First pass, determine how much memory will my header need */

		WorkPtr = RezData;				/* Init my work pointer */
		Adjust = 0;						/* Get the base memory */
		do {
			Count = ((Word32 *)WorkPtr)[2];	/* Get the count */
			if (SwapFlag) {
				Count = Burger::SwapEndian(Count);		/* Fix endian if needed */
			}
			Adjust += (Count*4)-4;				/* Each entry need 4 more bytes (Minus 4 for the type) */
			WorkPtr = WorkPtr + 12 + (Count*12);	/* Next group */
		} while (--GroupCount);
		NewLength = MyHeader->MemSize + Adjust;		/* Size of true buffer */

		/* Now I have the length, copy the data */

		RezHandle = AllocAHandle2(NewLength,0xFFF1UL<<16);	/* Get the memory */
		if (RezHandle) {			/* Valid? */
			MainPtr = (RezGroup_t *)LockAHandle(RezHandle);	/* Lock it */
			WorkPtr = RezData;
			GroupCount = MyHeader->Count;		/* Reset GroupCount */
			do {
				Word32 TempType;				/* For compatibility with old rez files */

				TempType = ((Word32 *)WorkPtr)[0];		/* Type */
				MainPtr->RezNum = ((Word32 *)WorkPtr)[1];	/* Resource number */
				MainPtr->Count = ((Word32 *)WorkPtr)[2];	/* Number of resources */
				WorkPtr += 12;
				NewLength -= 8;								/* The two longwords */
				if (SwapFlag) {
					MainPtr->RezNum = Burger::SwapEndian(MainPtr->RezNum);
					MainPtr->Count = Burger::SwapEndian(MainPtr->Count);
					TempType = Burger::SwapEndian(TempType);			/* Fix type */
				}

				if (TempType==5) {							/* Patch in sound files */
					MainPtr->RezNum+=5000;					/* Type 5 is now ID+5000 */
				}

				/* Process each resource entry */
				
				Count = MainPtr->Count;
				if (Count) {
					AuxPtr = &MainPtr->Array[0];
					do {
						AuxPtr->FileOffset = ((Word32 *)WorkPtr)[0];
						AuxPtr->Length = ((Word32 *)WorkPtr)[1];
						AuxPtr->NameOffset = ((Word32 *)WorkPtr)[2];
						AuxPtr->MemHand = 0;
						WorkPtr += 12;							/* Next 3 longwords (Rigid) */
						NewLength -= sizeof(RezEntry_t);		/* sizeof an entry */
						if (SwapFlag) {
							AuxPtr->FileOffset = Burger::SwapEndian(AuxPtr->FileOffset);
							AuxPtr->Length = Burger::SwapEndian(AuxPtr->Length);
							AuxPtr->NameOffset = Burger::SwapEndian(AuxPtr->NameOffset);
						}
						if (AuxPtr->NameOffset) {				/* Valid name? */
							AuxPtr->NameOffset+=Adjust;
						}
						AuxPtr->FileOffset += StartOffset;		/* Adjust for difference of file start */
						++AuxPtr;
					} while (--Count);
				}
				MainPtr = (RezGroup_t *)&MainPtr->Array[MainPtr->Count];	/* Next? */
			} while (--GroupCount);							/* Count down */

			if (NewLength) {								/* Any additional data? */
				FastMemCpy(MainPtr,WorkPtr,NewLength);
			}
			UnlockAHandle(RezHandle);						/* Unlock the memory */
			return (RezGroup_t **)RezHandle;
		}
	}
	return 0;			/* Don't make a list... (Bad count or memory error) */
}

/**********************************

	Used to sort between filenames
	
**********************************/

static int ANSICALL RezSortNames(const void *One,const void *Two)
{
	return stricmp((char *)(((RezName_t *)One)->HashOffset),(char *)(((RezName_t *)Two)->HashOffset));		/* For sorting */
}

/**********************************

	Create a hash table for all the filenames
	This way I can use a binary search to quickly
	look up filenames
	
**********************************/

static void ResourceHashNames(RezHeader_t *Input)
{
	Word Total;
	Word GroupCount;
	RezGroup_t *MainPtr;

	/* Kill any lists that were present */
	
	DeallocAHandle((void **)Input->RezNames);
	Input->RezNameCount = 0;
	Input->RezNames = 0;		/* No names are present */

	/* First this I need to do is to see how many entries have filenames */
	
	Total = 0;
	GroupCount = Input->Count;		/* Get the entry count */
	if (GroupCount) {				/* Any resources available? */
		MainPtr = Input->GroupHandle[0];		/* Deref the handle */
		do {
			Word EntryCount2;
			RezEntry_t *WorkPtr1;
			EntryCount2 = MainPtr->Count;
			WorkPtr1 = MainPtr->Array;
			do {
				if (WorkPtr1->NameOffset&NAMEOFFSETMASK) {	/* Name here? */
					++Total;								/* +1 to the total name count */
				}
				++WorkPtr1;
			} while (--EntryCount2);						/* Group done? */
			MainPtr = (RezGroup_t *)WorkPtr1;		/* Next? */
		} while (--GroupCount);						/* Count down */
	}
	
	/* Any names found? */
	
	if (Total) {
	
		/* Allocate the hash table */
		RezName_t **RezNameHand;
		RezNameHand = (RezName_t **)AllocAHandle(sizeof(RezName_t)*Total);
		if (RezNameHand) {
			RezName_t *DestPtr;
			Word32 TextPtr;		/* It's really a pointer to the text */
			
			Input->RezNameCount = Total;		/* Save the main values in the structure */
			Input->RezNames = RezNameHand;
			
			/* Let's fill in the hash table */
			/* Note : I am storing direct pointers to the filename strings */
			/* this is to allow qsort to run without creating a global variable */
			/* containing the base pointer to the strings */
			
			GroupCount = Input->Count;		/* Get the entry count */
			MainPtr = (RezGroup_t *)LockAHandle((void **)Input->GroupHandle);		/* Deref the handle */
			DestPtr = RezNameHand[0];		/* Dest pointer */
			TextPtr = (Word32)MainPtr;		/* Base pointer to text strings */
			do {
				Word Temp2;
				Word RezNum;
				RezEntry_t *WorkPtr2;
				Temp2 = MainPtr->Count;
				WorkPtr2 = MainPtr->Array;
				RezNum = MainPtr->RezNum;
				do {
					if (WorkPtr2->NameOffset&NAMEOFFSETMASK) {	/* Name here? */
						DestPtr->HashOffset = TextPtr+(WorkPtr2->NameOffset&NAMEOFFSETMASK);
						DestPtr->RezNum = RezNum;				/* +1 to the total name count */
						++DestPtr;
					}
					++WorkPtr2;
					++RezNum;
				} while (--Temp2);						/* Group done? */
				MainPtr = (RezGroup_t *)WorkPtr2;		/* Next? */
			} while (--GroupCount);						/* Count down */
				
			/* The hash table MUST be sorted! */
			
			qsort(LockAHandle((void **)RezNameHand),Total,sizeof(RezName_t),RezSortNames);
			
			/* Now, that it's sorted, let's change the pointers back into offsets */
			
			DestPtr = RezNameHand[0];		/* Dest pointer */
			do {
				DestPtr->HashOffset -= TextPtr;		/* Perform the correction */
				++DestPtr;
			} while (--Total);				/* Count down */
			UnlockAHandle((void **)RezNameHand);		/* Release the memory */
			UnlockAHandle((void **)Input->GroupHandle);	/* Release the rest */
			return;
		}
	}
}

/**********************************

	Master resource reference for
	monolithic resource use

**********************************/

RezHeader_t MasterRezHeader;

/**********************************

	After a call to LoadAResource(), this is
	set or cleared to allow the application
	to do post processing to freshly loaded data

**********************************/

Bool ResourceJustLoaded = FALSE;

/**********************************

	Create a new resource reference

**********************************/

RezHeader_t * BURGERCALL ResourceNew(const char *FileName,Word32 StartOffset)
{
	RezHeader_t *Input;
	Input = (RezHeader_t *)AllocAPointer(sizeof(RezHeader_t));	/* Get memory for reference */
	if (Input) {		/* Ok? */
		if (!ResourceInit(Input,FileName,StartOffset)) {	/* Open the file */
			return Input;		/* Everythings cool! */
		}
		DeallocAPointer(Input);	/* Dispose of the structure (Bad init) */
	}
	return 0;		/* Sorry charlie! */
}

/********************************

	Open a resource file for reading

********************************/

Word BURGERCALL ResourceInit(RezHeader_t *Input,const char *FileName,Word32 StartOffset)
{
	RezFileHeader_t MyHeader;	/* Struct for resource file header */
	RezGroup_t **RezHandle;	/* Current resource handle */
	Word SwapFlag;		/* Do I swap endian? */
	FILE *fp;			/* File reference */
	Word8 *RezData;		/* Loaded file header */

	if (Input) {
		FastMemSet(Input,0,sizeof(RezHeader_t));	/* Init the data */
		if (FileName) {
			fp = OpenAFile(FileName,"rb");		/* Can I open the file? */
			if (fp) {
				if (StartOffset) {				/* Even needed? */
					fseek(fp,StartOffset,SEEK_SET);		/* Seek to the beginning */
				}
				fread((char *)&MyHeader,1,12,fp);	/* Read in the header */
				if (!memcmp(MyHeader.Name,"BRGR",4)) {	/* Valid header? */
					SwapFlag = FALSE;			/* Assume native endian */
					if (MyHeader.MemSize >= Burger::SwapEndian(MyHeader.MemSize)) {	/* Swap? */
						MyHeader.MemSize = Burger::SwapEndian(MyHeader.MemSize);
						MyHeader.Count = Burger::SwapEndian(MyHeader.Count);
						SwapFlag = TRUE;		/* Swap everything */
					}

					RezData = (Word8 *)AllocAPointer(MyHeader.MemSize);		/* Allocate memory to load header */
					if (RezData) {
						if (fread((char *)RezData,1,MyHeader.MemSize,fp)==MyHeader.MemSize) {		/* Read in the file header */
							RezHandle = RezGroupNew(RezData,&MyHeader,SwapFlag,StartOffset);
							if (RezHandle) {
								DeallocAPointer(RezData);		/* Dispose of the data */
								Input->fp = (void *)fp;			/* Save the file referance */
								Input->Count = MyHeader.Count;	/* Get the resource count */
								Input->GroupHandle = RezHandle;	/* Get the memory */
								Input->Flags = REZFLAGEXTERNAL;	/* External files are ok */
								ResourceHashNames(Input);		/* Make the initial name hash */
								return FALSE;
							}
						}
						DeallocAPointer(RezData);				/* Release the file header */
					}
				}
				fclose(fp);		/* Close the file (Error) */
			}
		}
	}
	return TRUE;		/* Could not open the file */
}

/**********************************

	Dispose of the contents of a resource file

**********************************/

void BURGERCALL ResourceDestroy(RezHeader_t *Input)
{
 	Word GroupCount;		/* Number of resource groups */

	if (Input) {
		if (Input->fp) {		/* Is there an open file? */
			fclose((FILE *)Input->fp);	/* Close the file */
			Input->fp = 0;		/* Zap the reference */
		}

		/* Dispose of any resources in memory */

		GroupCount = Input->Count;		/* Any valid entries? */
		if (GroupCount) {
			RezGroup_t *MainPtr;	/* Group array pointer */
			MainPtr = (RezGroup_t*)LockAHandle((void **)Input->GroupHandle);	/* Lock it down */
			do {
				Word EntryCount;		/* Number of entries per group */
				RezEntry_t *EntryPtr;	/* Entry pointer */
				EntryPtr = &MainPtr->Array[0];
				EntryCount = MainPtr->Count;
				do {
					DeallocAHandle(EntryPtr->MemHand);		/* Dispose of the memory */
#if _DEBUG
					if ((DebugTraceFlag & DEBUGTRACE_WARNINGS) && (EntryPtr->NameOffset&NAMEOFFSETREFCOUNT)) {
						DebugXMessage("ResourceDestroy() : Resource %u still referenced %u times\n",
							(MainPtr->Count-EntryCount)+MainPtr->RezNum,(EntryPtr->NameOffset>>NAMEOFFSETREFSHIFT));
					}
#endif
					++EntryPtr;		/* Next entry */
				} while (--EntryCount);
				MainPtr = (RezGroup_t *)EntryPtr;
			} while (--GroupCount);
		}
		DeallocAHandle((void **)Input->GroupHandle);		/* Release the resource list */
		DeallocAHandle((void **)Input->RezNames);			/* Release the name list */
		Input->GroupHandle = 0;
		Input->RezNames = 0;
		Input->RezNameCount = 0;
		Input->Count = 0;
	}
}

/**********************************

	Delete a resource file

**********************************/

void BURGERCALL ResourceDelete(RezHeader_t *Input)
{
	if (Input) {
		ResourceDestroy(Input);
		DeallocAPointer(Input);
	}
}

/********************************

	Release all the data allocated by the resource manager.

********************************/

static void ANSICALL ResourceExit(void)
{
	ResourceDestroy(&MasterRezHeader);
}

/********************************

	Initialize the resource manager so I can
	access all my resource chunks

********************************/

Word BURGERCALL ResourceInitMasterRezHeader(const char *FileName)
{
	if (!OnceHit) {			/* Release resources on shutdown */
		OnceHit = TRUE;
		atexit(ResourceExit);		/* Allow closing on exit */
	}
	return ResourceInit(&MasterRezHeader,FileName,0);	/* Open the default resource file */
}

/********************************

	Scan all entries in the resource map and dispose
	of all the entries that loaded in the cache but not
	actually discarded.
	I will only dispose of unlocked purgable handles
	This is useful before a level or map load
	to help prevent memory fragmentation in low memory
	situations. Not really needed to call this routine unless
	memory is a a premium.
	
********************************/

void BURGERCALL ResourcePurgeCache(RezHeader_t *Input)
{
	Word GroupCount;
	if (Input) {
		GroupCount = Input->Count;		/* Get the entry count */	
		if (GroupCount) {		/* Any resources available? */
			RezGroup_t *MainPtr;
			MainPtr = (RezGroup_t *)LockAHandle((void **)Input->GroupHandle);
			do {
				Word Count;
				RezEntry_t *EntryPtr;
				Count = MainPtr->Count;		/* How many elements? */
				EntryPtr = &MainPtr->Array[0];	/* Index to the data */
				if (Count) {
					do {
						void **DataHand;
						DataHand = EntryPtr->MemHand;	/* Is there a handle here? */
						if (DataHand) {
							if (!(EntryPtr->NameOffset&NAMEOFFSETREFCOUNT)) {	/* Not referenced right now? */
								DeallocAHandle(DataHand);		/* Dispose of it */
								EntryPtr->MemHand = 0;			/* Mark as gone */
							}
						}
						++EntryPtr;		/* Next one */
					} while (--Count);
				}
				MainPtr = (RezGroup_t *)&MainPtr->Array[MainPtr->Count];	/* Next? */
			} while (--GroupCount);		/* Count down */
			UnlockAHandle((void **)Input->GroupHandle);
		}
	}
}

/**********************************

	Enable or disable reading external files
	I will return the current state of the flag
	before the modification

**********************************/

Word BURGERCALL ResourceExternalFlag(RezHeader_t *Input,Word Flag)
{
	Word Temp;
	if (Input) {
		Temp = Input->Flags;
		if (!Flag) {
			Flag = Temp&(~REZFLAGEXTERNAL);
		} else {
			Flag = Temp|REZFLAGEXTERNAL;
		}
		Input->Flags = Flag;
		if (Temp&REZFLAGEXTERNAL) {		/* TRUE/FALSE */
			return TRUE;
		}
	}
	return FALSE;
}

/**********************************

	Enable or disable the purging system
	I will return the current state of the flag
	before the modification

**********************************/

Word BURGERCALL ResourceDontCacheFlag(RezHeader_t *Input,Word Flag)
{
	Word Temp;
	if (Input) {
		Temp = Input->Flags;
		if (!Flag) {
			Flag = Temp&(~REZFLAGNOCACHE);
		} else {
			Flag = Temp|REZFLAGNOCACHE;
		}
		Input->Flags = Flag;
		if (Temp&REZFLAGNOCACHE) {		/* Map to TRUE/FALSE */
			return TRUE;
		}
	}
	return FALSE;
}

/********************************

	If the named resource doesn't already exist in the 
	resource dictionary, insert it into the first free
	slot.
	I return the new or previous resource ID number.
	I return -1 if an error of some kind occurs.
	I do not check to see if the file exists. I just add it to 
	the dictionary.

********************************/

Word BURGERCALL ResourceAddName(RezHeader_t *Input,const char *RezName)
{
	Word GroupCount;		/* Temp count of current groups */
	Word NewStrLen;			/* Length of the string to add */
	Word RezNum;			/* Resource number I assign */
	Word32 OldDictionarySize;	/* Size of the old dictionary */
	RezGroup_t *MainPtr;	/* Work pointer */
	RezGroup_t **MainHand;	/* Work handle */
	RezGroup_t *MainPtr2;	/* Dest pointer */
	RezGroup_t *NextPtr;	/* Forward pointer */
	Word NewCount;			/* New group count */
	Word32 NameOffset;	/* Used for the new name hash table */
	Word32 OffsetAdd;		/* Adder for hash table offset */
		
	RezNum = ResourceGetRezNum(Input,RezName);		/* Is the resource name present? */

	/* If it's not present, add it to the dictionary and assign a new number */
	
	if (RezNum==(Word)-1) {
	
		/* Ok, It's not present, this is evil */
		
		/* If the filename has a prefix "20:FileName.txt" */
		/* Then remove the prefix  */

		if (((Word8 *)RezName)[0]>='0' && ((Word8 *)RezName)[0]<('9'+1)) {	/* First char a number? */
			const char *AuxName;			/* Auxilary RezName */
			AuxName = strchr(RezName,':');	/* Find the ':' */
			if (AuxName) {			/* Found it? */
				RezName = AuxName+1;			/* Index past it */
			}
		}

		/* Ok, RezName has the name FileName.txt */
		
		NewStrLen = strlen(RezName)+1;		/* Length of the new string to add */

		/* Is this an empty resource file? */
		
		GroupCount = Input->Count;		/* Get the entry count */
		if (!GroupCount) {				/* Any resources available? */
			RezGroup_t **GHand;
			/* Create a simple single resource group */

			GHand = (RezGroup_t **)AllocAHandle(sizeof(RezGroup_t)+NewStrLen);
			if (GHand) {		/* Memory ok? */
				RezName_t **RezNameHand0;
				Input->Count = 1;			/* I have a handle */
				Input->RezNameCount = 1;
				Input->GroupHandle = GHand;
				MainPtr = GHand[0];
				MainPtr->Count = 1;	/* 1 entry */
				MainPtr->RezNum = 1;	/* Number 1 */
				MainPtr->Array[0].Length = 0;
				MainPtr->Array[0].MemHand = 0;
				MainPtr->Array[0].FileOffset = 0;
				MainPtr->Array[0].NameOffset = sizeof(RezGroup_t);
				FastMemCpy(&MainPtr->Array[1],RezName,NewStrLen);
				RezNameHand0 = (RezName_t **)AllocAHandle(sizeof(RezName_t));
				Input->RezNames = RezNameHand0;
				if (RezNameHand0) {
					RezName_t *WorkPtr3;
					WorkPtr3 = RezNameHand0[0];
					WorkPtr3->RezNum = 1;
					WorkPtr3->HashOffset = sizeof(RezGroup_t);
					return 1;			/* Resource 1 allocated */
				}
				DeallocAHandle((void **)GHand);
				Input->GroupHandle = 0;
				Input->Count = 0;
				Input->RezNameCount = 0;
			}
			return (Word)-1;				/* I'm boned */
		}

		/* This is a resource file with data in it */
		/* What is the first free number */
		
		NewCount = GroupCount;						/* New group count */
		MainPtr = Input->GroupHandle[0];			/* Get the main pointer */
		RezNum = MainPtr->RezNum;					/* Get the base number */
		OffsetAdd = sizeof(RezEntry_t);				/* Number of bytes to add to the dictionary */
		if (RezNum!=1) {							/* #1 is not used? */
			if (RezNum>=3) {						/* Too far ahead for merge? */
				OffsetAdd += sizeof(RezGroup_t)-sizeof(RezEntry_t);
				++NewCount;
			}
			RezNum = 1;								/* I'm using it now */
		} else {
			Word zz;
			zz = MainPtr->Count;
			RezNum = zz+1;							/* Use the number at the end of the list */
			if (NewCount>=2) {						/* Is there a following group? */
				NextPtr = (RezGroup_t *)&MainPtr->Array[zz];
				if ((RezNum+1)==NextPtr->RezNum) {	/* Is this a merge? */
					OffsetAdd -= sizeof(RezGroup_t)-sizeof(RezEntry_t);
					--NewCount;
				}
			}
		}
		
		/* At this point OffsetAdd has the array growth value and NewStrLen */
		/* has the new string length */
		
		OldDictionarySize = GetAHandleSize((void **)Input->GroupHandle);		/* Current dictionary size */
		NameOffset = OldDictionarySize+OffsetAdd;
		MainHand = (RezGroup_t **)AllocAHandle(NameOffset+NewStrLen);
		if (!MainHand) {
			return (Word)-1;		/* You are so boned! */
		}
		
		/* Now I have the new entry. Let's copy the old dictionary into */
		/* the new one and include the one new entry */
		
		/* In this pass, I will copy ONLY the RezGroup_t structures */
		
		GroupCount = Input->Count;				/* Get the group entry count */
		MainPtr = Input->GroupHandle[0];		/* Get the main pointer (Not locked) */
		MainPtr2 = MainHand[0];					/* Destination pointer */
		
		{
			Word Count1;
			Word Len1;
			Count1 = MainPtr->Count;
			Len1 = (sizeof(RezEntry_t)*Count1)+(sizeof(RezGroup_t)-sizeof(RezEntry_t));
			NextPtr = (RezGroup_t *)((Word8 *)MainPtr+Len1);
			if (RezNum==1) {						/* Insertion... */
				MainPtr2->RezNum = 1;
				if (MainPtr->RezNum>=3) {			/* Whole new entry? */
					MainPtr2->Count = 1;
					NextPtr = MainPtr;				/* Don't touch the first entry */
					MainPtr2 = (RezGroup_t *)((Word8 *)MainPtr2+sizeof(RezGroup_t));
				} else {
					MainPtr2->Count = Count1+1;
					Len1 = (sizeof(RezEntry_t)*Count1);
					FastMemCpy(&MainPtr2->Array[1],&MainPtr->Array[0],Len1);
					MainPtr2 = (RezGroup_t *)((Word8 *)MainPtr2+Len1+sizeof(RezGroup_t));
					--GroupCount;
				}
			} else {
			
				MainPtr2->Count = Count1+1;				/* Append */
				MainPtr2->RezNum = MainPtr->RezNum;		/* New root reznum */
				Len1 = (sizeof(RezEntry_t)*Count1);
				FastMemCpy(&MainPtr2->Array[0],&MainPtr->Array[0],Len1);	/* Copy the base data */
				--GroupCount;
				if (GroupCount && NextPtr->RezNum==(RezNum+1)) {		/* Merge? */
					MainPtr2->Count += NextPtr->Count;
					Len1 = (sizeof(RezEntry_t)*NextPtr->Count);
					FastMemCpy(&MainPtr2->Array[Count1+1],&NextPtr->Array[0],Len1);
					NextPtr = (RezGroup_t *)((Word8 *)NextPtr+Len1+(sizeof(RezGroup_t)-sizeof(RezEntry_t)));
					--GroupCount;
				}
				Len1 = (sizeof(RezEntry_t)*MainPtr2->Count)+(sizeof(RezGroup_t)-sizeof(RezEntry_t));
				MainPtr2 = (RezGroup_t *)((Word8 *)MainPtr2+Len1);
			}
			MainPtr = NextPtr;
		}
					
		/* Now, the altered dictionary is copied, now copy the rest */

		GroupCount = OldDictionarySize-((Word8 *)MainPtr-(Word8 *)Input->GroupHandle[0]);	/* Number of bytes for the name array */
		FastMemCpy(MainPtr2,MainPtr,GroupCount);						/* Copy the strings */
		MainPtr2 = (RezGroup_t *)((Word8 *)MainPtr2+GroupCount);			/* End of the buffer */
		FastMemCpy(MainPtr2,RezName,NewStrLen);							/* Copy the NEW string at the end */
		
		/* Traverse the new dictionary and alter all the strings to point to the new */
		/* offsets */
				
		GroupCount = NewCount;			/* Get the NEW group entry count */
		MainPtr = MainHand[0];			/* Get the main pointer */
		do {
			Word TempRezNum;
			Word Count4;					/* Number of entries to worry about */
			RezEntry_t *Entry;				/* Temp entry */
			Count4 = MainPtr->Count;				/* Number of entries in this group */
			Entry = &MainPtr->Array[0];				/* Pointer to the first entry */
			
			/* Now check each entry for a name string */
			TempRezNum = MainPtr->RezNum;
			do {
				Word32 Offset3;
				if (RezNum==TempRezNum) {		/* Create the new entry */
					Entry->MemHand = 0;
					Entry->FileOffset = 0;
					Entry->Length = 0;
					Entry->NameOffset = NameOffset;						
				} else {
					Offset3 = Entry->NameOffset&NAMEOFFSETMASK;
					if (Offset3) {			/* Is there a name? */
						Entry->NameOffset += OffsetAdd;
					}
				}
				++TempRezNum;
				++Entry;						/* Next entry */
			} while (--Count4);
			MainPtr = (RezGroup_t *)Entry;		/* Next group */
		} while (--GroupCount);					/* Count down */
		
		/* I am done!!! */
			
		DeallocAHandle((void **)Input->GroupHandle);		/* Release the old dictionary */
		Input->GroupHandle = MainHand;						/* Set the new dictionary */
		Input->Count = NewCount;
		
		/* Now, let's create the hash for the names! */

		{
			RezName_t **RHand;
			RezName_t *NamePtr;
			RHand = Input->RezNames;
			if (!RHand) {						/* No names existed before? */
				RHand = (RezName_t **)AllocAHandle(sizeof(RezName_t));
				if (RHand) {
					RezName_t *WorkPtr3;
					Input->RezNames = RHand;
					Input->RezNameCount = 1;
					WorkPtr3 = RHand[0];
					WorkPtr3->RezNum = RezNum;
					WorkPtr3->HashOffset = NameOffset;
					return RezNum;
				}
				Input->RezNameCount = 0;
				return RezNum;
			}

			/* I will be inserting into an existing dictionary */
			/* First, adjust the current dictionary to the new string offsets */
		
			GroupCount = Input->RezNameCount;
			NamePtr = RHand[0];
			do {
				NamePtr->HashOffset += OffsetAdd;
				++NamePtr;
			} while (--GroupCount);
			
			NewStrLen = Input->RezNameCount+1;		/* New name count */
			RHand = (RezName_t **)ResizeAHandle((void **)RHand,NewStrLen*sizeof(RezName_t));
			Input->RezNames = RHand;
			if (RHand) {
				RezName_t *OutPtr;
				RezName_t *OutPtrx;
				ResourceFindName(Input,RezName,&OutPtrx);
				OutPtr = OutPtrx;
				memmove(OutPtr+1,OutPtr,((NewStrLen-1)*sizeof(RezName_t))-((Word8 *)OutPtr-(Word8 *)RHand[0]));
				OutPtr->RezNum = RezNum;
				OutPtr->HashOffset = NameOffset;
				Input->RezNameCount = NewStrLen;
			} else {
				Input->RezNameCount = 0;
			}
		}
	}
	return RezNum;		/* No good! */
}

/********************************

	Remove a resource from the rez file
	I look for the entry and if found, I remove it from
	the resource dictionary.
	Note : I do not alter the resource file itself in any
	way, I just remove the ability to access this resource

********************************/

void BURGERCALL ResourceRemove(RezHeader_t *Input,Word RezNum)
{
	Word32 NewTotal;			/* New dictionary size */
	RezEntry_t *Entry;			/* Current entry */
	Word GroupCount;			/* Group count */
	RezGroup_t *MainPtr;		/* Main group offset */
	RezGroup_t *MainPtr2;		/* Destination */
	RezGroup_t **MainHand;		/* New rez handle */
	Word NewCount;				/* New group count */
	Word EntryIndex;			/* Index from base group */
	Word32 OffsetDelete;		/* String offset of the string to purge */
	Word32 OffsetBefore;			/* Offset adjust BEFORE the delimiter */
	Word32 OffsetAfter;			/* Offset adjust AFTER the delimiter */
	Word DelStrLen;				/* Length of the string removed */
	
	/* It there a resource by this number? */
	
	Entry = ResourceScan(Input,RezNum);
	if (!Entry) {
		return;						/* Get out now... */
	}
	
	/* Make sure the data is gone */
	
	{
		void **MemHand;
		MemHand = Entry->MemHand;
		if (MemHand) {		/* Is there a handle? */
			Word32 Offset;
			Entry->MemHand = 0;		/* Mark as GONE */
			Offset = Entry->NameOffset;
			Entry->NameOffset = Offset&NAMEOFFSETMASK;	/* No references */
#if _DEBUG
			if ((DebugTraceFlag & DEBUGTRACE_WARNINGS) && (Offset&NAMEOFFSETREFCOUNT)>=(NAMEOFFSETREFADD*2)) {
				DebugXMessage("ResourceRemove() : Removing resource %u that is referenced %lu times\n",RezNum,Offset>>NAMEOFFSETREFSHIFT);
			}
#endif
			DeallocAHandle(MemHand);
		}
	}
	
	/* Just wipe out the dictionary? */
	
	GroupCount = Input->Count;		/* Get the entry count */
	MainPtr = Input->GroupHandle[0];		/* Base pointer */
	if (GroupCount==1 && MainPtr->Count==1) {	/* It there only one entry? */
		DeallocAHandle((void **)Input->GroupHandle);
		DeallocAHandle((void **)Input->RezNames);
		Input->GroupHandle = 0;
		Input->RezNames = 0;
		Input->RezNameCount = 0;
		Input->Count = 0;						/* Kill the contents */
		return;
	}

	/* Now, create a new resource directory without the entry in question */
	/* Note : If I am deleting an entry in the middle of a group, I need */
	/* to create two groups */
	
	/* Calculate the size of the new dictionary */

	OffsetAfter = GetAHandleSize((void **)Input->GroupHandle);
	NewTotal = OffsetAfter-sizeof(RezEntry_t);
	
	Entry = 0;									/* Shut up the compiler */
	MainPtr = Input->GroupHandle[0];			/* Get the main pointer (Not locked) */
	do {
		Word Count2;
		EntryIndex = RezNum-MainPtr->RezNum;	/* Get the offset from base number */
		Count2 = MainPtr->Count;
		if (EntryIndex<Count2) {				/* In this range? */
			Entry = &MainPtr->Array[EntryIndex];	/* Return the pointer to the entry */
			break;
		}
		MainPtr = (RezGroup_t *)&MainPtr->Array[Count2];	/* Next? */
	} while (--GroupCount);		/* Count down */

	/* MainPtr points to the group that I will be affecting */
	/* at this point in the code */
	
	/* Is there a filename attached? Remove it */
	
	DelStrLen = 0;
	OffsetDelete = Entry->NameOffset&NAMEOFFSETMASK;		/* Is there a string? */
	if (OffsetDelete) {
		DelStrLen = strlen((char *)(Input->GroupHandle[0])+OffsetDelete)+1;	/* Remove the string length */
		NewTotal -= DelStrLen;
	}
	
	/* Now, adjust due to changes in the dictionary */
	
	/* Am I splitting a RezGroup_t? */
	
	NewCount = Input->Count;
	if (EntryIndex && ((EntryIndex+1)!=MainPtr->Count)) {	/* First or last entry? */
		++NewCount;
		NewTotal += (sizeof(RezGroup_t)-sizeof(RezEntry_t));	/* Add in the new entry */
	}
	
	/* Am I removing a RezGroup_t? */
	
	if (!EntryIndex && MainPtr->Count==1) {		/* Only 1 entry? */
		--NewCount;
		NewTotal -= (sizeof(RezGroup_t)-sizeof(RezEntry_t));
	}
	
	/* Now, I have the size of the new dictionary */
	
#if 0
	MainHand = (RezGroup_t **)AllocAHandle(NewTotal);		/* Allocate the dictionary */
	if (!MainHand) {			/* I am totally screwed! */
		return;					/* I'm out of here!! */
	}
#else
	MainHand = Input->GroupHandle;
#endif	
	
	/* Now I have the new entry. Let's copy the old dictionary into */
	/* the new one except the one entry I am going to purge */
	
	/* In this pass, I will copy ONLY the RezGroup_t structures */
	
	GroupCount = Input->Count;				/* Get the group entry count */
	MainPtr = Input->GroupHandle[0];		/* Get the main pointer (Not locked) */
	MainPtr2 = MainHand[0];					/* Destination pointer */
	do {
		Word Count3;						/* Number of entries to worry about */
		Word Len3;							/* Number of bytes to transfer */
		
		EntryIndex = RezNum-MainPtr->RezNum;		/* Get the offset from base number */
		Count3 = MainPtr->Count;					/* Number of entries in this group */
		
		if (EntryIndex>=Count3) {					/* Out of range? Copy the group as is */
			Len3 = (sizeof(RezEntry_t)*Count3)+(sizeof(RezGroup_t)-sizeof(RezEntry_t));
			FastMemCpy(MainPtr2,MainPtr,Len3);		/* Copy this group as is */
			MainPtr2 = (RezGroup_t *)((Word8 *)MainPtr2+Len3);	/* Adjust my pointer */

		} else if (EntryIndex || Count3!=1) {		/* Don't discard? (If both are false, then I eject this group) */

			/* Ok, the resource I want to discard is here, but do I truncate or do I split? */
				
			if (!EntryIndex) {							/* Remove the first? */
				MainPtr2->Count = Count3-1;
				MainPtr2->RezNum = MainPtr->RezNum+1;
				Len3 = (sizeof(RezEntry_t)*Count3)+(sizeof(RezGroup_t)-sizeof(RezEntry_t)*2);
				Entry = &MainPtr->Array[1];
				
			} else if ((EntryIndex+1)==Count3) {		/* Remove the last? */
				MainPtr2->Count = Count3-1;
				MainPtr2->RezNum = MainPtr->RezNum;
				Len3 = (sizeof(RezEntry_t)*Count3)+(sizeof(RezGroup_t)-sizeof(RezEntry_t)*2);
				Entry = &MainPtr->Array[0];
	
			} else {									/* Split me */
				MainPtr2->Count = EntryIndex;
				MainPtr2->RezNum = MainPtr->RezNum;
				Len3 = (sizeof(RezEntry_t)*EntryIndex)+(sizeof(RezGroup_t)-sizeof(RezEntry_t));
				FastMemCpy(&MainPtr2->Array[0],&MainPtr->Array[0],Len3);
				MainPtr2 = (RezGroup_t *)((Word8 *)MainPtr2+Len3);	/* Adjust my pointer */

				MainPtr2->Count = (Count3-EntryIndex)-1;
				MainPtr2->RezNum = MainPtr->RezNum+EntryIndex+1;
				Len3 = (sizeof(RezEntry_t)*MainPtr2->Count)+(sizeof(RezGroup_t)-sizeof(RezEntry_t));
				Entry = &MainPtr->Array[EntryIndex+1];
			}
			FastMemCpy(&MainPtr2->Array[0],Entry,Len3);
			MainPtr2 = (RezGroup_t *)((Word8 *)MainPtr2+Len3);	/* Adjust my pointer */
		}
		MainPtr = (RezGroup_t *)&MainPtr->Array[Count3];	/* Next? */
	} while (--GroupCount);		/* Count down */
	
	OffsetBefore = ((Word8 *)MainPtr)-(Word8 *)(Input->GroupHandle[0]);		/* Offset to the strings */

	EntryIndex = OffsetBefore-(((Word8 *)MainPtr2)-(Word8 *)(MainHand[0]));	/* Number to subtract */
	
	if (DelStrLen) {
		Word32 Temp44;
		Temp44 = OffsetDelete-OffsetBefore;
		FastMemCpy(MainPtr2,MainPtr,Temp44);
		MainPtr2 = (RezGroup_t *)((Word8 *)MainPtr2+Temp44);
		FastMemCpy(MainPtr2,(Word8 *)MainPtr+Temp44+DelStrLen,(OffsetAfter-OffsetDelete)-DelStrLen);
	} else {
		FastMemCpy(MainPtr2,MainPtr,OffsetAfter-OffsetBefore);
	}
	
	/* Now, the dictionary is copied, now copy the filename strings */
	/* Note: I am using the NEW dictionary to look at but the ORIGINAL dictionary to get the strings */
	/* this way I don't have to screw around with memory moving pointers... */
	
	{
		char *AsciiPtr;					/* Base pointer for the strings */
		Word Count4;					/* Number of entries to worry about */

		OffsetBefore = EntryIndex;
		OffsetAfter = EntryIndex+DelStrLen;	/* Remove the dead string */
		GroupCount = NewCount;			/* Get the NEW group entry count */
		MainPtr = MainHand[0];			/* Get the main pointer */
		AsciiPtr = (char *)(Input->GroupHandle[0]);		/* Base pointer for strings */
		do {
			Count4 = MainPtr->Count;				/* Number of entries in this group */
			Entry = &MainPtr->Array[0];				/* Pointer to the first entry */
		
			/* Now check each entry for a name string */
				
			do {
				Word32 Offset3;
				Offset3 = Entry->NameOffset&NAMEOFFSETMASK;
				if (Offset3) {			/* Is there a name? */
					if (Offset3<OffsetDelete) {
						Offset3=OffsetBefore;
					} else {
						Offset3=OffsetAfter;
					}
					Entry->NameOffset = Entry->NameOffset-Offset3;
				}
				++Entry;						/* Next entry */
			} while (--Count4);
			MainPtr = (RezGroup_t *)Entry;		/* Next group */
		} while (--GroupCount);					/* Count down */
	}
	
	/* I am done!!! */

#if 0
	DeallocAHandle((void **)Input->GroupHandle);		/* Release the old dictionary */
	Input->GroupHandle = MainHand;						/* Set the new dictionary */
#else
	Input->GroupHandle = (RezGroup_t **)ResizeAHandle((void **)Input->GroupHandle,NewTotal);
#endif
	Input->Count = NewCount;
	
	/* Now, let's fix the name hash! */
	
	GroupCount = Input->RezNameCount;
	if (GroupCount) {						/* Get the number of entries to process */
		RezName_t *RNamePtr;
		RNamePtr = Input->RezNames[0];
		DelStrLen = FALSE;
		do {
			Word32 Offset4;
			Offset4 = RNamePtr->HashOffset;
			if (Offset4==OffsetDelete) {	/* Remove the entry */
				FastMemCpy(RNamePtr,RNamePtr+1,(GroupCount-1)*sizeof(RezName_t));
				DelStrLen = TRUE;
			} else {
				if (Offset4<OffsetDelete) {
					Offset4 -= OffsetBefore;
				} else {
					Offset4 -= OffsetAfter;
				}
				RNamePtr->HashOffset = Offset4;		/* Store the result */
				++RNamePtr; 
			}
		} while (--GroupCount);

		if (DelStrLen) {							/* Do I need to resize? */
			GroupCount = Input->RezNameCount-1;
			Input->RezNameCount = GroupCount;
			Input->RezNames = (RezName_t **)ResizeAHandle((void **)Input->RezNames,GroupCount*sizeof(RezName_t));
		}
	}
}


/********************************

	Remove a resource from the rez file
	by name

********************************/

void BURGERCALL ResourceRemoveName(RezHeader_t *Input,const char *RezName)
{
	Word RezNum;
	RezNum = ResourceGetRezNum(Input,RezName);	/* Get the index number */
	if (RezNum != (Word)-1) {
		ResourceRemove(Input,RezNum);			/* Remove this entry */
	}
}

/********************************

	Load in a resource
	into a static buffer

********************************/

Word BURGERCALL ResourceRead(RezHeader_t *Input,Word RezNum,void *DestPtr,Word32 BufSize)
{
	void **RezHand;		/* Resource handle */
	Word Result;		/* Return code */
	Word32 Length;	/* True size of the resource */
	
	Result = TRUE;
	RezHand = ResourceLoadHandle(Input,RezNum);		/* Load it in */
	if (RezHand) {				/* Ok? */
		Length = GetAHandleSize(RezHand);	/* How much memory? */
		if (BufSize>=Length) {			/* Is the output buffer big enough? */
			BufSize = Length;			/* Use the smaller */
			Result = FALSE;				/* It's ok */
		}
		FastMemCpy(DestPtr,RezHand[0],BufSize);	/* Copy the data */
		ResourceRelease(Input,RezNum);	/* Release the data */
	}
	return Result;		/* Return FALSE if OK */
}

/********************************

	Load in a resource

********************************/

void * BURGERCALL ResourceLoad(RezHeader_t *Input,Word RezNum)
{
	return LockAHandle(ResourceLoadHandle(Input,RezNum));	/* Get the handle */
}

/********************************

	Load in a resource

********************************/

void * BURGERCALL ResourceLoadByName(RezHeader_t *Input,const char *RezName)
{
	return LockAHandle(ResourceLoadHandleByName(Input,RezName));	/* Get the handle */
}

/********************************

	Load in a resource and return the handle.
	The returned handle is NOT locked...

********************************/

void ** BURGERCALL ResourceLoadHandle(RezHeader_t *Input,Word RezNum)
{
	RezEntry_t *Entry;		/* Pointer to entry */
	void **BufferHand;		/* Handle to main memory */
	Word32 Offset;		/* File offset */
	Word32 FileNameOffset;	/* Filename offset */
	Word Flags;				/* Memory flags */
	FILE *fp;				/* File reference */
	Word32 DataLength;	/* Input length */

	ResourceJustLoaded = FALSE;		/* Assume cached or failed */
	
	Entry = ResourceScan(Input,RezNum);		/* Find the resource */
	if (!Entry) {		/* Find the entry */
		return 0;		/* The resource does not exist! */
	}

	/* Copy the structure contents since Entry is pointing to */
	/* an unlocked handle */

	BufferHand = Entry->MemHand;			/* Get the current handle */
	Offset = Entry->FileOffset;				/* Preload the offset */
	DataLength = Entry->Length;				/* Preload the length */
	FileNameOffset = Entry->NameOffset;		/* Filename offset */
	Entry->NameOffset=FileNameOffset+NAMEOFFSETREFADD;	/* Increase the reference count */

	if (BufferHand) {		/* Valid handle? */
		if (BufferHand[0]) {		/* Handle not purged? */
			SetHandlePurgeFlag(BufferHand,FALSE);	/* Can't purge */
			return BufferHand;	/* Return the handle */
		}
		Entry->MemHand = 0;			/* Mark as gone! */
		DeallocAHandle(BufferHand);	/* Release the memory */
		BufferHand = 0;				/* Set to zero for future error checking */
	}
	Flags = RezNum<<16;			/* ID the handle as the resource manager */
	if (Offset&REZOFFSETFIXED) {		/* Where to load? */
		Flags |= HANDLEFIXED;		/* Place in fixed memory */
	}

	/* See if I can load in the file from disk */

	if ((FileNameOffset&NAMEOFFSETMASK) && (Input->Flags&REZFLAGEXTERNAL)) {	/* Should I load in the true filename */
		char *FileName;
		
		/* Did I already try to load as a file? */
		
		if (FileNameOffset&NAMEOFFSETTESTED) {
			if (!(FileNameOffset&NAMEOFFSETFILE)) {
				goto NoLoad;
			}
		}
		FileName = (char *)(LockAHandle((void **)Input->GroupHandle))+(FileNameOffset&NAMEOFFSETMASK);
		fp = OpenAFile(FileName,"rb");
		if (fp) {
			Word32 NewLength;

			NewLength = fgetfilesize(fp);	/* Get the NEW length */
			if (NewLength) {
				BufferHand = AllocAHandle2(NewLength,Flags);	/* Get memory */
				if (BufferHand) {		/* Got the memory? */
					if (fread((char *)LockAHandle(BufferHand),1,NewLength,fp)!=NewLength) {
						DeallocAHandle(BufferHand);		/* Discard the memory */
						BufferHand = 0;			/* Can't load it in! */
					}
				}
			}
			fclose(fp);		/* Close the file */
		}
		UnlockAHandle((void **)Input->GroupHandle);	/* Release the filename */
		Entry = ResourceScan(Input,RezNum);		/* Oh crud! */
		Entry->NameOffset |= (NAMEOFFSETTESTED|NAMEOFFSETFILE);	/* Mark as tested */
		if (BufferHand) {
			goto WrapUp;		/* I got it now! */
		}
		Entry->NameOffset &= (~NAMEOFFSETFILE);		/* Not found */
	}

NoLoad:;

	/* Let's load it in from the .REZ file */
	
	fp = (FILE *)Input->fp;		/* Get the file referance */
	if (!fp || !Offset) {		/* No resource file found? */
		goto BadExit;
	}

	fseek(fp,Offset&REZOFFSETMASK,SEEK_SET); /* Seek into the file */
	if (Offset&REZOFFSETDECOMPMASK) {		/* Is this compressed? */
		void (BURGERCALL *DecompPtr)(Word8 *,Word8 *,Word32,Word32);
		Word32 PackedHeader,BufferSize,PackedSize;
		Word8 *PackedPtr;

		DecompPtr = Input->DecompPtrs[((Offset>>29)&3)-1];	/* Get the compressor */
		if (!DecompPtr) {		/* Is there a compressor logged? */
			goto BadExit;
		}

		PackedHeader = fgetlongl(fp);		/* Get the length */
		BufferHand = AllocAHandle2(PackedHeader,Flags);	/* Get dest buffer */
		if (!BufferHand) {	/* No memory for uncompressed data? */
			goto BadExit;
		}

		PackedSize = DataLength-4;	/* Size of packed data */
		BufferSize = (PackedSize<MAXBUFFER) ? PackedSize : MAXBUFFER;
		PackedPtr = (Word8 *)AllocAPointer(BufferSize);	/* Get Buffer */
		if (!PackedPtr) {		/* No compressed data buffer? */
			goto BadExit2;
		}
		do {		/* Loop for decompression */
			Word32 ChunkSize;		/* Size of disk chunk to read */

			ChunkSize = (BufferSize<PackedSize) ? BufferSize : PackedSize;
			if (fread((char *)PackedPtr,1,ChunkSize,fp)!=ChunkSize) {
				goto BadExit2;
			}
			DecompPtr((Word8 *)LockAHandle(BufferHand),PackedPtr,PackedHeader,ChunkSize);
			PackedHeader = 0;	/* All remaining passes will use cached state */
			PackedSize -= ChunkSize;
		} while (PackedSize);
		DecompPtr(0,0,0,0);			/* Force a shutdown */
		DeallocAPointer(PackedPtr);	/* Release the temp buffer */
		goto WrapUp;		/* Unlock handle and exit */
	}

	/* Uncompressed data */

	BufferHand = AllocAHandle2(DataLength,Flags); /* Get the memory */
	if (BufferHand) {		/* Memory ok? */
		if (fread((char *)LockAHandle(BufferHand),1,DataLength,fp)!=DataLength) {	/* Read it in */
			goto BadExit2;
		}
WrapUp:
		UnlockAHandle(BufferHand);
		ResourceJustLoaded = TRUE;		/* Data is new */
		Entry = ResourceScan(Input,RezNum);		/* Reset the pointer */
		Entry->MemHand = BufferHand;		/* Save the handle */
#if _DEBUG
		if (DebugTraceFlag&DEBUGTRACE_REZLOAD) {	/* Should I print it? */
			DebugXMessage("Loaded resource %u",RezNum);
			if ((FileNameOffset&NAMEOFFSETMASK)) {
				DebugXMessage(", file %s",(char *)(LockAHandle((void **)Input->GroupHandle))+(FileNameOffset&NAMEOFFSETMASK));
				UnlockAHandle((void **)Input->GroupHandle);
			}
			DebugXString("\n");
		}
#endif
		return BufferHand;		/* Return the handle */
	}
BadExit2:
	DeallocAHandle(BufferHand);				/* Dispose of my memory */
BadExit:
	Entry = ResourceScan(Input,RezNum);		/* Oh crud! */
	Entry->NameOffset &= (~NAMEOFFSETREFCOUNT);	/* Kill the ref count */
	Entry->MemHand = 0;
	return 0;			/* No data, No error needed */
}

/********************************

	Load in a resource and return the handle.
	The returned handle is NOT locked...

********************************/

void ** BURGERCALL ResourceLoadHandleByName(RezHeader_t *Input,const char *RezName)
{
	Word RezNum;

	RezNum = ResourceGetRezNum(Input,RezName);	/* Get the index number */
	if (RezNum == (Word)-1) {
		RezNum = ResourceAddName(Input,RezName);	/* Try to add it */
		if (RezNum==(Word)-1) {		/* No good? */
			return 0;			/* Bad news */
		}
	}
	return ResourceLoadHandle(Input,RezNum);		/* Load the file */
}

/********************************

	Release a resource by marking it purgeable
	but don't destroy it.

********************************/

void BURGERCALL ResourceRelease(RezHeader_t *Input,Word RezNum)
{
	RezEntry_t *Entry;
	Entry = ResourceScan(Input,RezNum);	/* Scan for the resource */
	if (Entry) {					/* Is it here? */
		void **MemHand;
		Word32 Offset;
		Offset = Entry->NameOffset;
#if _DEBUG
		if ((DebugTraceFlag & DEBUGTRACE_WARNINGS) && !(Offset&NAMEOFFSETREFCOUNT)) {		/* Should not be zero */
			DebugXMessage("ResourceRelease() : RefCount is zero for resource %u!\n",RezNum);
			Entry = ResourceScan(Input,RezNum);
		}
#endif
		if (Offset>=NAMEOFFSETREFADD) {			/* Not referenced? */
			Offset-=NAMEOFFSETREFADD;			/* Release a reference */
			Entry->NameOffset = Offset;
			if (!(Offset&NAMEOFFSETREFCOUNT)) {		/* No longer referenced? */
				MemHand = Entry->MemHand;
				if (MemHand) {			/* Is there a handle? */
					UnlockAHandle(MemHand);		/* Unlock it */
					if (Input->Flags&REZFLAGNOCACHE) {
						Entry->MemHand = 0;
						DeallocAHandle(MemHand);
					} else {
						SetHandlePurgeFlag(MemHand,TRUE);	/* Mark as purgable */
					}
				}
			}
		}
	}
}

/********************************

	Release a resource by marking it purgeable
	but don't destroy it by name.

********************************/

void BURGERCALL ResourceReleaseByName(RezHeader_t *Input,const char *RezName)
{
	RezEntry_t *Entry;
	Word RezNum;
	RezNum = ResourceGetRezNum(Input,RezName);	/* Scan for the resource */
	if (RezNum!=(Word)-1) {
		Entry = ResourceScan(Input,RezNum);
		if (Entry) {					/* Is it here? */
			void **MemHand;
			Word32 Offset;
			Offset = Entry->NameOffset;
#if _DEBUG
			if ((DebugTraceFlag & DEBUGTRACE_WARNINGS) && !(Offset&NAMEOFFSETREFCOUNT)) {		/* Should not be zero */
				DebugXMessage("ResourceReleaseByName() : RefCount is zero for resource %s!\n",RezName);
				Entry = ResourceScan(Input,RezNum);
			}
#endif
			if (Offset>=NAMEOFFSETREFADD) {			/* Not referenced? */
				Offset-=NAMEOFFSETREFADD;			/* Release a reference */
				Entry->NameOffset = Offset;
				if (!(Offset&NAMEOFFSETREFCOUNT)) {		/* No longer referenced? */
					MemHand = Entry->MemHand;
					if (MemHand) {			/* Is there a handle? */
						UnlockAHandle(MemHand);		/* Unlock it */
						if (Input->Flags&REZFLAGNOCACHE) {
							Entry->MemHand = 0;
							DeallocAHandle(MemHand);
						} else {
							SetHandlePurgeFlag(MemHand,TRUE);	/* Mark as purgable */
						}
					}
				}
			}
		}
	}
}

/********************************

	Destroy the data associated with a resource

********************************/

void BURGERCALL ResourceKill(RezHeader_t *Input,Word RezNum)
{
	RezEntry_t *Entry;
	Entry = ResourceScan(Input,RezNum);	/* Scan for the resource */
	if (Entry) {
		void **MemHand;
		MemHand = Entry->MemHand;
		if (MemHand) {		/* Is there a handle? */
			Word32 Offset;
			Entry->MemHand = 0;		/* Mark as GONE */
			Offset = Entry->NameOffset;
			Entry->NameOffset = Offset&NAMEOFFSETMASK;	/* No references */
#if _DEBUG
			if ((DebugTraceFlag & DEBUGTRACE_WARNINGS) && (Offset&NAMEOFFSETREFCOUNT)>=(NAMEOFFSETREFADD*2)) {
				DebugXMessage("ResourceKill() : Killing resource %u that is referenced %lu times\n",RezNum,Offset>>NAMEOFFSETREFSHIFT);
			}
#endif
			DeallocAHandle(MemHand);
		}
	}
}

/********************************

	Destroy the data associated with a resource

********************************/

void BURGERCALL ResourceKillByName(RezHeader_t *Input,const char *RezName)
{
	RezEntry_t *Entry;
	Word RezNum;
	RezNum = ResourceGetRezNum(Input,RezName);
	if (RezNum!=(Word)-1) {
		Entry = ResourceScan(Input,RezNum);	/* Scan for the resource */
		if (Entry) {
			void **MemHand;
			MemHand = Entry->MemHand;
			if (MemHand) {		/* Is there a handle? */
				Word32 Offset;
				Entry->MemHand = 0;		/* Mark as GONE */
				Offset = Entry->NameOffset;
				Entry->NameOffset = Offset & NAMEOFFSETMASK;	/* No references */
#if _DEBUG
				if ((DebugTraceFlag & DEBUGTRACE_WARNINGS) && (Offset&NAMEOFFSETREFCOUNT)>=(NAMEOFFSETREFADD*2)) {
					DebugXMessage("ResourceKillByName() : Killing resource %s that is referenced %lu times\n",RezName,Offset>>NAMEOFFSETREFSHIFT);
				}
#endif
				DeallocAHandle(MemHand);
			}
		}
	}
}

/********************************

	Destroy the data associated with a resource

********************************/

void BURGERCALL ResourceDetach(RezHeader_t *Input,Word RezNum)
{
	RezEntry_t *Entry;
	Entry = ResourceScan(Input,RezNum);
	if (Entry) {		/* Scan for the resource */
		Word32 Offset;
		Entry->MemHand = 0;		/* Mark as GONE */
		Offset = Entry->NameOffset;
		Entry->NameOffset = Offset & (~NAMEOFFSETREFCOUNT);	/* No references */
#if _DEBUG
		if ((DebugTraceFlag & DEBUGTRACE_WARNINGS) && (Offset&NAMEOFFSETREFCOUNT)!=NAMEOFFSETREFADD) {	/* 1 time is ok */
			DebugXMessage("ResourceDetach() : Detaching resource %u that is referenced %lu times\n",RezNum,Offset>>NAMEOFFSETREFSHIFT);
		}
#endif
	}
}

/********************************

	Destroy the data associated with a resource

********************************/

void BURGERCALL ResourceDetachByName(RezHeader_t *Input,const char *RezName)
{
	RezEntry_t *Entry;
	Word RezNum;
	RezNum = ResourceGetRezNum(Input,RezName);
	if (RezNum!=(Word)-1) {
		Entry = ResourceScan(Input,RezNum);
		if (Entry) {		/* Scan for the resource */
			Word32 Offset;
			Entry->MemHand = 0;		/* Mark as GONE */
			Offset = Entry->NameOffset;
			Entry->NameOffset = Offset & (~NAMEOFFSETREFCOUNT);	/* No references */
#if _DEBUG
			if ((DebugTraceFlag & DEBUGTRACE_WARNINGS) && (Offset&NAMEOFFSETREFCOUNT)!=NAMEOFFSETREFADD) {	/* 1 time is ok */
				DebugXMessage("ResourceDetachByName() : Detaching resource %s that is referenced %lu times\n",RezName,Offset>>NAMEOFFSETREFSHIFT);
			}
#endif
		}
	}
}

/********************************

	Preload in a resource

********************************/

void BURGERCALL ResourcePreload(RezHeader_t *Input,Word RezNum)
{
	if (ResourceLoadHandle(Input,RezNum)) {	/* Get the handle */
		ResourceRelease(Input,RezNum);		/* Release the resource */
	}
}

/********************************

	Load in a resource

********************************/

void BURGERCALL ResourcePreloadByName(RezHeader_t *Input,const char *RezName)
{
	Word RezNum;
	RezNum = ResourceGetRezNum(Input,RezName);	/* Get the index number */
	if (RezNum == (Word)-1) {
		RezNum = ResourceAddName(Input,RezName);	/* Try to add it */
		if (RezNum==(Word)-1) {		/* No good? */
			return;					/* Bad news! */
		}
	}
	if (ResourceLoadHandle(Input,RezNum)) {	/* Get the handle */
		ResourceRelease(Input,RezNum);		/* Release it */
	}
}

/********************************

	This is a bottleneck.
	Scan the entries for the filename and
	return the resource entry number

********************************/

Word BURGERCALL ResourceGetRezNum(RezHeader_t *Input,const char *RezName)
{
	RezName_t *NamePtr;				/* Pointer to the master name list */
	
	if (ResourceFindName(Input,RezName,&NamePtr)) {
		return NamePtr->RezNum;
	}
	return (Word)-1;		/* No good! */
}

/********************************

	Scan for a resource entry in the resource map

********************************/

Word BURGERCALL ResourceGetName(RezHeader_t *Input,Word RezNum,char *Buffer,Word BufferSize)
{
	Word GroupCount;
	if (Input && Buffer && BufferSize) {			/* Valid output buffer? */
		GroupCount = Input->Count;		/* Get the entry count */
		if (GroupCount) {				/* Any resources available? */
			RezGroup_t *MainPtr;
			char *TextPtr;
			MainPtr = Input->GroupHandle[0];		/* Get the root group pointer */
			TextPtr = (char *)MainPtr;				/* This is the base offset to the names */
			do {
				Word Temp;
				Temp = RezNum-MainPtr->RezNum;		/* Get the offset from base number */
				if (Temp<MainPtr->Count) {			/* In this group? */
					RezEntry_t *EntryPtr;
					EntryPtr = &MainPtr->Array[Temp];	/* I got the entry */
					if (EntryPtr->NameOffset&NAMEOFFSETMASK) {			/* Do I have a name? */
						TextPtr = TextPtr+(EntryPtr->NameOffset&NAMEOFFSETMASK);		/* Pointer to the name */
						Temp = strlen(TextPtr);			/* How many bytes to copy? */
						if (Temp>=BufferSize) {			/* Will it overwrite the buffer? */
							Temp = BufferSize-1;		/* Clip the string */
						}
						Buffer[Temp] = 0;				/* Zero terminate it */
						FastMemCpy(Buffer,TextPtr,Temp);	/* Copy it */
						return FALSE;					/* I found it! */
					}
					break;
				}
				MainPtr = (RezGroup_t *)&MainPtr->Array[MainPtr->Count];	/* Next? */
			} while (--GroupCount);		/* Count down */
		}
		Buffer[0] = 0;		/* There is no name */
	}
	return TRUE;			/* No good! */
}

/********************************

	Given a handle, scan the resource map to see if it is under
	Resource manager control. If so, return the resource ID number
	and the resource name. Return error code if not present (Right now
	it's TRUE), otherwise return zero for no error if I found it.

********************************/

Word BURGERCALL ResourceGetIDFromHandle(RezHeader_t *Input,const void **RezHand,Word *IDFound,char *NameBuffer,Word NameBufferSize)
{
	Word GroupCount;
	
	if (Input && RezHand) {				/* Can't find a NULL entry */
		GroupCount = Input->Count;		/* Get the entry count */
		if (GroupCount) {				/* Any resources available? */
			RezGroup_t *MainPtr;
			char *AsciiPtr;
			
			MainPtr = Input->GroupHandle[0];		/* Pointer to the data */
			AsciiPtr = (char *)MainPtr;				/* Pointer to the ASCII data base */
			do {
				Word Count;
				RezEntry_t *Entry;
				Count = MainPtr->Count;				/* Count of this group's size */
				
				/* Add in the array for the group */
				
				Entry = &MainPtr->Array[0];			/* Pointer to the first entry */
		
				/* Now check each entry for a name string */
				
				do {
					if (Entry->MemHand == (void **)RezHand) {		/* Is it a match? */
						if (IDFound) {						/* Do I want the ID number? */
							IDFound[0] = (MainPtr->RezNum+MainPtr->Count)-Count;
						}
						if (NameBuffer && NameBufferSize) {		/* Do I want the name? */
							Word32 Offset;
							char *TextPtr;
							
							Offset = Entry->NameOffset&NAMEOFFSETMASK;			/* Do I have a name? */
							if (Offset) {			/* Name present? */
								Word Temp;
								
								TextPtr = AsciiPtr+Offset;		/* Pointer to the name */
								Temp = strlen(TextPtr);			/* How many bytes to copy? */
								if (Temp>=NameBufferSize) {		/* Will it overwrite the buffer? */
									Temp = NameBufferSize-1;	/* Clip the string */
								}
								NameBuffer[Temp] = 0;					/* Zero terminate it */
								FastMemCpy(NameBuffer,TextPtr,Temp);	/* Copy it */
							} else {
								NameBuffer[0] = 0;		/* Blank name */
							}
						}
						return FALSE;				/* I found it */
					}
					++Entry;						/* Next entry */
				} while (--Count);
				MainPtr = (RezGroup_t *)Entry;		/* Next group */
			} while (--GroupCount);					/* Count down */
		}
	}
	return TRUE;		/* I didn't find it. */
}

/********************************

	Given a handle, scan the resource map to see if it is under
	Resource manager control. If so, return the resource ID number
	and the resource name. Return error code if not present (Right now
	it's TRUE), otherwise return zero for no error if I found it.

********************************/

Word BURGERCALL ResourceGetIDFromPointer(RezHeader_t *Input,const void *RezPtr,Word *IDFound,char *NameBuffer,Word NameBufferSize)
{
	Word GroupCount;
	
	if (Input && RezPtr) {				/* Can't find a NULL entry */
		GroupCount = Input->Count;		/* Get the entry count */
		if (GroupCount) {				/* Any resources available? */
			RezGroup_t *MainPtr;
			char *AsciiPtr;
			
			MainPtr = Input->GroupHandle[0];		/* Pointer to the data */
			AsciiPtr = (char *)MainPtr;				/* Pointer to the ASCII data base */
			do {
				Word Count;
				RezEntry_t *Entry;
				Count = MainPtr->Count;				/* Count of this group's size */
				
				/* Add in the array for the group */
				
				Entry = &MainPtr->Array[0];			/* Pointer to the first entry */
		
				/* Now check each entry for a name string */
				
				do {
					if (Entry->MemHand && Entry->MemHand[0] == RezPtr) {		/* Is it a match? */
						if (IDFound) {						/* Do I want the ID number? */
							IDFound[0] = (MainPtr->RezNum+MainPtr->Count)-Count;
						}
						if (NameBuffer && NameBufferSize) {		/* Do I want the name? */
							Word32 Offset;
							char *TextPtr;
							
							Offset = Entry->NameOffset&NAMEOFFSETMASK;			/* Do I have a name? */
							if (Offset) {				/* Name present? */
								Word Temp;
								
								TextPtr = AsciiPtr+Offset;		/* Pointer to the name */
								Temp = strlen(TextPtr);			/* How many bytes to copy? */
								if (Temp>=NameBufferSize) {		/* Will it overwrite the buffer? */
									Temp = NameBufferSize-1;	/* Clip the string */
								}
								NameBuffer[Temp] = 0;					/* Zero terminate it */
								FastMemCpy(NameBuffer,TextPtr,Temp);	/* Copy it */
							} else {
								NameBuffer[0] = 0;		/* Blank name */
							}
						}
						return FALSE;				/* I found it */
					}
					++Entry;						/* Next entry */
				} while (--Count);
				MainPtr = (RezGroup_t *)Entry;		/* Next group */
			} while (--GroupCount);					/* Count down */
		}
	}
	return TRUE;		/* I didn't find it. */
}

/********************************

	Return the array of resource names
	The list of names is returned in SORTED order

********************************/

RezNameReturn_t *BURGERCALL ResourceGetNameArray(RezHeader_t *Input,Word *EntryCountPtr)
{
	RezNameReturn_t *DestPtr;	/* Output pointer */
	
	DestPtr = 0;				/* Assume failure */
	if (EntryCountPtr) {		/* Is there a return pointer? */
		EntryCountPtr[0] = 0;	/* No data present */
		if (Input->RezNames) {	/* Any names in the resource? */
			Word Counter;		/* Temp counter */
			Word Size;			/* Output byte size */
			char *TextPtr;		/* Base text pointer */
			RezName_t *RezNamePtr;	/* Data pointer */
			
			Counter = Input->RezNameCount;					/* Number of entries */
			Size = Counter*(sizeof(RezNameReturn_t)+1);		/* Number of bytes without names */

			/* Scan all the names to get the size of the result buffer */
			
			TextPtr = (char *)(Input->GroupHandle[0]);		/* Base pointer to text strings */
			RezNamePtr = Input->RezNames[0];				/* Init the data pointer */
			do {
				Size += strlen(TextPtr+RezNamePtr->HashOffset);		/* String size */
				++RezNamePtr;
			} while (--Counter);
			
			/* Allocate the return buffer */
			
			DestPtr = (RezNameReturn_t *)AllocAPointer(Size);
			if (DestPtr) {
				char *DestTextPtr;
				RezNameReturn_t *WorkPtr;
				
				/* Now, fill in the structure */
				
				Counter = Input->RezNameCount;					/* Reset the number of entries */
				EntryCountPtr[0] = Counter;						/* Return the entry count */
				DestTextPtr = (char *)(&DestPtr[Counter]);		/* Text buffer AFTER the structures */
				TextPtr = (char *)(Input->GroupHandle[0]);		/* Base pointer to text strings */
				RezNamePtr = Input->RezNames[0];				/* Reset again */
				WorkPtr = DestPtr;								/* Output pointer */
				do {
					Word TempLen;
					WorkPtr->RezName = DestTextPtr;				/* Text pointer */
					WorkPtr->RezNum = RezNamePtr->RezNum;		/* Resource ID number */
					TempLen = strlen(TextPtr+RezNamePtr->HashOffset)+1;	/* Skip to the name empty slot */
					FastMemCpy(DestTextPtr,TextPtr+RezNamePtr->HashOffset,TempLen);		/* Copy the name string */
					DestTextPtr += TempLen;		/* Skip to the name empty slot */
					++RezNamePtr;				/* Next struct */
					++WorkPtr;
				} while (--Counter);			/* Keep going? */
			}
		}
	}
	return DestPtr;		/* Return the data created */
}

/**********************************

	Log a resource decompressor

**********************************/

void BURGERCALL ResourceLogDecompressor(RezHeader_t *Input,Word CompressID,ResourceDecompressorProcPtr Proc)
{
	if (--CompressID<3) {	/* Allowable? */
		Input->DecompPtrs[CompressID] = Proc;	/* Save the function pointer */
	}
#if _DEBUG
	else {
		if (DebugTraceFlag&DEBUGTRACE_WARNINGS) {
			DebugXString("LogResourceDecompressor() : CompressID is not 1-3, it's ");
			DebugXWord(CompressID+1);
			DebugXString("\n");
		}
	}
#endif
}

/**********************************

	Load a Burgerlib shape assuming it's
	little endian. I will swap the endian on
	big endian machines

**********************************/

LWShape_t * BURGERCALL ResourceLoadShape(RezHeader_t *Input,Word RezNum)
{
#if defined(__BIGENDIAN__)
	LWShape_t *Result;
	Result = (LWShape_t *)ResourceLoad(Input,RezNum);
	if (Result && ResourceJustLoaded) {
		Result->Width = Burger::SwapEndian(Result->Width);
		Result->Height = Burger::SwapEndian(Result->Height);
	}
	return Result;
#else
	return (LWShape_t *)ResourceLoad(Input,RezNum);
#endif
}


/**********************************

	Load a Burgerlib shape assuming it's
	little endian. I will swap the endian on
	big endian machines

**********************************/

LWXShape_t * BURGERCALL ResourceLoadXShape(RezHeader_t *Input,Word RezNum)
{
#if defined(__BIGENDIAN__)
	LWXShape_t *Result;
	Result = (LWXShape_t *)ResourceLoad(Input,RezNum);
	if (Result && ResourceJustLoaded) {
		Result->XOffset = Burger::SwapEndian(Result->XOffset);
		Result->YOffset = Burger::SwapEndian(Result->YOffset);
		Result->Shape.Width = Burger::SwapEndian(Result->Shape.Width);
		Result->Shape.Height = Burger::SwapEndian(Result->Shape.Height);
	}
	return Result;
#else
	return (LWXShape_t *)ResourceLoad(Input,RezNum);
#endif
}

/**********************************

	Load a Burgerlib shape assuming it's
	little endian. I will swap the endian on
	big endian machines

**********************************/

void BURGERCALL ResourcePreloadShape(RezHeader_t *Input,Word RezNum)
{
#if defined(__BIGENDIAN__)
	LWShape_t *Result;
	Result = (LWShape_t *)ResourceLoad(Input,RezNum);
	if (Result) {
		if (ResourceJustLoaded) {
			Result->Width = Burger::SwapEndian(Result->Width);
			Result->Height = Burger::SwapEndian(Result->Height);
		}
		ResourceRelease(Input,RezNum);
	}
#else
	ResourcePreload(Input,RezNum);
#endif
}

/**********************************

	Load a Burgerlib shape assuming it's
	little endian. I will swap the endian on
	big endian machines

**********************************/

void BURGERCALL ResourcePreloadXShape(RezHeader_t *Input,Word RezNum)
{
#if defined(__BIGENDIAN__)
	LWXShape_t *Result;
	Result = (LWXShape_t *)ResourceLoad(Input,RezNum);
	if (Result) {
		if (ResourceJustLoaded) {
			Result->XOffset = Burger::SwapEndian(Result->XOffset);
			Result->YOffset = Burger::SwapEndian(Result->YOffset);
			Result->Shape.Width = Burger::SwapEndian(Result->Shape.Width);
			Result->Shape.Height = Burger::SwapEndian(Result->Shape.Height);
		}
		ResourceRelease(Input,RezNum);
	}
#else
	ResourcePreload(Input,RezNum);
#endif
}

/**********************************

	Load a Burgerlib shape array assuming it's
	little endian. I will swap the endian on
	big endian machines

**********************************/

void * BURGERCALL ResourceLoadShapeArray(RezHeader_t *Input,Word RezNum)
{
#if defined(__BIGENDIAN__)
	void *Result;
	Result = ResourceLoad(Input,RezNum);	/* Load in the resource */
	if (Result && ResourceJustLoaded) {		/* Just loaded in? */
		Word32 Count;
		Count = Burger::SwapEndian(((Word32 *)Result)[0]);	/* Get the count */
		if (Count) {		/* Any entries? */
			Word i;
			Count >>=2;	/* Get the TRUE count */
			i = 0;
			do {
				LWShape_t *TempPtr;
				Word32 Offset;
				Offset = Burger::SwapEndian(((Word32 *)Result)[i]);
				((Word32 *)Result)[i] = Offset;	/* Save the NEW offset */
				TempPtr = (LWShape_t *)(&(((Word8 *)Result)[Offset]));
				TempPtr->Width = Burger::SwapEndian(TempPtr->Width);
				TempPtr->Height = Burger::SwapEndian(TempPtr->Height);
			} while (++i<Count);
		}
	}
	return Result;
#else
	return ResourceLoad(Input,RezNum);	/* Just load it */
#endif
}

/**********************************

	Load a Burgerlib shape array assuming it's
	little endian. I will swap the endian on
	big endian machines

**********************************/

void * BURGERCALL ResourceLoadXShapeArray(RezHeader_t *Input,Word RezNum)
{
#if defined(__BIGENDIAN__)
	void *Result;
	Result = ResourceLoad(Input,RezNum);	/* Load in the resource */
	if (Result && ResourceJustLoaded) {		/* Just loaded in? */
		Word32 Count;
		Count = Burger::SwapEndian(((Word32 *)Result)[0]);	/* Get the count */
		if (Count) {		/* Any entries? */
			Word i;
			Count >>=2;	/* Get the TRUE count */
			i = 0;
			do {
				LWXShape_t *TempPtr;
				Word32 Offset;
				Offset = Burger::SwapEndian(((Word32 *)Result)[i]);
				((Word32 *)Result)[i] = Offset;	/* Save the NEW offset */
				TempPtr = (LWXShape_t *)(&(((Word8 *)Result)[Offset]));
				TempPtr->XOffset = Burger::SwapEndian(TempPtr->XOffset);
				TempPtr->YOffset = Burger::SwapEndian(TempPtr->YOffset);
				TempPtr->Shape.Width = Burger::SwapEndian(TempPtr->Shape.Width);
				TempPtr->Shape.Height = Burger::SwapEndian(TempPtr->Shape.Height);
			} while (++i<Count);
		}
	}
	return Result;
#else
	return ResourceLoad(Input,RezNum);	/* Just load it */
#endif
}

/**********************************

	Load a Burgerlib shape array assuming it's
	little endian. I will swap the endian on
	big endian machines

**********************************/

void BURGERCALL ResourcePreloadShapeArray(RezHeader_t *Input,Word RezNum)
{
#if defined(__BIGENDIAN__)
	void *Result;
	Result = ResourceLoad(Input,RezNum);	/* Load in the resource */
	if (Result) {
		if (ResourceJustLoaded) {		/* Just loaded in? */
			Word32 Count;
			Count = Burger::SwapEndian(((Word32 *)Result)[0]);	/* Get the count */
			if (Count) {		/* Any entries? */
				Word i;
				Count >>=2;	/* Get the TRUE count */
				i = 0;
				do {
					LWShape_t *TempPtr;
					Word32 Offset;
					Offset = Burger::SwapEndian(((Word32 *)Result)[i]);
					((Word32 *)Result)[i] = Offset;	/* Save the NEW offset */
					TempPtr = (LWShape_t *)(&(((Word8 *)Result)[Offset]));
					TempPtr->Width = Burger::SwapEndian(TempPtr->Width);
					TempPtr->Height = Burger::SwapEndian(TempPtr->Height);
				} while (++i<Count);
			}
		}
		ResourceRelease(Input,RezNum);
	}
#else
	ResourcePreload(Input,RezNum);	/* Just load it */
#endif
}


/**********************************

	Load a Burgerlib shape array assuming it's
	little endian. I will swap the endian on
	big endian machines

**********************************/

void BURGERCALL ResourcePreloadXShapeArray(RezHeader_t *Input,Word RezNum)
{
#if defined(__BIGENDIAN__)
	void *Result;
	Result = ResourceLoad(Input,RezNum);	/* Load in the resource */
	if (Result) {
		if (ResourceJustLoaded) {		/* Just loaded in? */
			Word32 Count;
			Count = Burger::SwapEndian(((Word32 *)Result)[0]);	/* Get the count */
			if (Count) {		/* Any entries? */
				Word i;
				Count >>=2;	/* Get the TRUE count */
				i = 0;
				do {
					LWXShape_t *TempPtr;
					Word32 Offset;
					Offset = Burger::SwapEndian(((Word32 *)Result)[i]);
					((Word32 *)Result)[i] = Offset;	/* Save the NEW offset */
					TempPtr = (LWXShape_t *)(&(((Word8 *)Result)[Offset]));
					TempPtr->XOffset = Burger::SwapEndian(TempPtr->XOffset);
					TempPtr->YOffset = Burger::SwapEndian(TempPtr->YOffset);
					TempPtr->Shape.Width = Burger::SwapEndian(TempPtr->Shape.Width);
					TempPtr->Shape.Height = Burger::SwapEndian(TempPtr->Shape.Height);
				} while (++i<Count);
			}
		}
		ResourceRelease(Input,RezNum);
	}
#else
	ResourceLoad(Input,RezNum);	/* Just load it */
#endif
}

/**********************************

	Load a Burgerlib shape assuming it's
	little endian. I will swap the endian on
	big endian machines

**********************************/

LWShape_t ** BURGERCALL ResourceLoadShapeHandle(RezHeader_t *Input,Word RezNum)
{
#if defined(__BIGENDIAN__)
	LWShape_t **Result;
	Result = (LWShape_t **)ResourceLoadHandle(Input,RezNum);
	if (Result && ResourceJustLoaded) {
		LWShape_t *TempPtr;
		TempPtr = Result[0];
		TempPtr->Width = Burger::SwapEndian(TempPtr->Width);
		TempPtr->Height = Burger::SwapEndian(TempPtr->Height);
	}
	return Result;
#else
	return (LWShape_t **)ResourceLoadHandle(Input,RezNum);
#endif
}


/**********************************

	Load a Burgerlib shape assuming it's
	little endian. I will swap the endian on
	big endian machines

**********************************/

LWXShape_t ** BURGERCALL ResourceLoadXShapeHandle(RezHeader_t *Input,Word RezNum)
{
#if defined(__BIGENDIAN__)
	LWXShape_t **Result;
	Result = (LWXShape_t **)ResourceLoadHandle(Input,RezNum);
	if (Result && ResourceJustLoaded) {
		LWXShape_t *TempPtr;
		TempPtr = Result[0];
		TempPtr->XOffset = Burger::SwapEndian(TempPtr->XOffset);
		TempPtr->YOffset = Burger::SwapEndian(TempPtr->YOffset);
		TempPtr->Shape.Width = Burger::SwapEndian(TempPtr->Shape.Width);
		TempPtr->Shape.Height = Burger::SwapEndian(TempPtr->Shape.Height);
	}
	return Result;
#else
	return (LWXShape_t **)ResourceLoadHandle(Input,RezNum);
#endif
}

/**********************************

	Load a Burgerlib shape array assuming it's
	little endian. I will swap the endian on
	big endian machines

**********************************/

void ** BURGERCALL ResourceLoadShapeArrayHandle(RezHeader_t *Input,Word RezNum)
{
#if defined(__BIGENDIAN__)
	void **Result;
	Result = ResourceLoadHandle(Input,RezNum);	/* Load in the resource */
	if (Result && ResourceJustLoaded) {		/* Just loaded in? */
		Word32 Count;
		void *DataPtr;
		DataPtr = Result[0];
		Count = Burger::SwapEndian(((Word32 *)DataPtr)[0]);	/* Get the count */
		if (Count) {		/* Any entries? */
			Word i;
			Count >>=2;	/* Get the TRUE count */
			i = 0;
			do {
				LWShape_t *TempPtr;
				Word32 Offset;
				Offset = Burger::SwapEndian(((Word32 *)DataPtr)[i]);
				((Word32 *)DataPtr)[i] = Offset;	/* Save the NEW offset */
				TempPtr = (LWShape_t *)(&(((Word8 *)DataPtr)[Offset]));
				TempPtr->Width = Burger::SwapEndian(TempPtr->Width);
				TempPtr->Height = Burger::SwapEndian(TempPtr->Height);
			} while (++i<Count);
		}
	}
	return Result;
#else
	return ResourceLoadHandle(Input,RezNum);	/* Just load it */
#endif
}

/**********************************

	Load a Burgerlib shape array assuming it's
	little endian. I will swap the endian on
	big endian machines

**********************************/

void ** BURGERCALL ResourceLoadXShapeArrayHandle(RezHeader_t *Input,Word RezNum)
{
#if defined(__BIGENDIAN__)
	void **Result;
	Result = ResourceLoadHandle(Input,RezNum);	/* Load in the resource */
	if (Result && ResourceJustLoaded) {		/* Just loaded in? */
		Word32 Count;
		void *DataPtr;
		DataPtr = Result[0];
		Count = Burger::SwapEndian(((Word32 *)DataPtr)[0]);	/* Get the count */
		if (Count) {		/* Any entries? */
			Word i;
			Count >>=2;	/* Get the TRUE count */
			i = 0;
			do {
				LWXShape_t *TempPtr;
				Word32 Offset;
				Offset = Burger::SwapEndian(((Word32 *)DataPtr)[i]);
				((Word32 *)DataPtr)[i] = Offset;	/* Save the NEW offset */
				TempPtr = (LWXShape_t *)(&(((Word8 *)DataPtr)[Offset]));
				TempPtr->XOffset = Burger::SwapEndian(TempPtr->XOffset);
				TempPtr->YOffset = Burger::SwapEndian(TempPtr->YOffset);
				TempPtr->Shape.Width = Burger::SwapEndian(TempPtr->Shape.Width);
				TempPtr->Shape.Height = Burger::SwapEndian(TempPtr->Shape.Height);
			} while (++i<Count);
		}
	}
	return Result;
#else
	return ResourceLoadHandle(Input,RezNum);	/* Just load it */
#endif
}

/**********************************

	Load a Burgerlib shape assuming it's
	little endian. I will swap the endian on
	big endian machines

**********************************/

GfxShape_t * BURGERCALL ResourceLoadGfxShape(RezHeader_t *Input,Word RezNum)
{
#if defined(__BIGENDIAN__)
	GfxShape_t *Result;
	Result = (GfxShape_t *)ResourceLoad(Input,RezNum);
	if (Result && ResourceJustLoaded) {
		Result->XShape.XOffset = Burger::SwapEndian(Result->XShape.XOffset);
		Result->XShape.YOffset = Burger::SwapEndian(Result->XShape.YOffset);
		Result->XShape.Shape.Width = Burger::SwapEndian(Result->XShape.Shape.Width);
		Result->XShape.Shape.Height = Burger::SwapEndian(Result->XShape.Shape.Height);
	}
	return Result;
#else
	return (GfxShape_t *)ResourceLoad(Input,RezNum);
#endif
}

/**********************************

	Load a Burgerlib shape assuming it's
	little endian. I will swap the endian on
	big endian machines

**********************************/

void BURGERCALL ResourcePreloadGfxShape(RezHeader_t *Input,Word RezNum)
{
#if defined(__BIGENDIAN__)
	GfxShape_t *Result;
	Result = (GfxShape_t *)ResourceLoad(Input,RezNum);
	if (Result) {
		if (ResourceJustLoaded) {
			Result->XShape.XOffset = Burger::SwapEndian(Result->XShape.XOffset);
			Result->XShape.YOffset = Burger::SwapEndian(Result->XShape.YOffset);
			Result->XShape.Shape.Width = Burger::SwapEndian(Result->XShape.Shape.Width);
			Result->XShape.Shape.Height = Burger::SwapEndian(Result->XShape.Shape.Height);
		}
		ResourceRelease(Input,RezNum);
	}
#else
	ResourcePreload(Input,RezNum);
#endif
}


/**********************************

	Load a Burgerlib shape assuming it's
	little endian. I will swap the endian on
	big endian machines

**********************************/

void ** BURGERCALL ResourceLoadGfxShapeHandle(RezHeader_t *Input,Word RezNum)
{
#if defined(__BIGENDIAN__)
	GfxShape_t **Result;
	Result = (GfxShape_t **)ResourceLoadHandle(Input,RezNum);
	if (Result && ResourceJustLoaded) {
		GfxShape_t *TempPtr;
		TempPtr = Result[0];
		TempPtr->XShape.XOffset = Burger::SwapEndian(TempPtr->XShape.XOffset);
		TempPtr->XShape.YOffset = Burger::SwapEndian(TempPtr->XShape.YOffset);
		TempPtr->XShape.Shape.Width = Burger::SwapEndian(TempPtr->XShape.Shape.Width);
		TempPtr->XShape.Shape.Height = Burger::SwapEndian(TempPtr->XShape.Shape.Height);
	}
	return (void **)Result;
#else
	return ResourceLoadHandle(Input,RezNum);
#endif
}

