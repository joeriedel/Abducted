/**********************************

	Prefs manager

**********************************/

#ifndef __PFPREFS_H__
#define __PFPREFS_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct PrefsRecord_t;
typedef enum {PREFREAD,PREFWRITE,PREFDEFAULT} PrefState_e;
typedef char * (BURGERCALL *PrefsRecordProc_t)(char *,struct PrefsRecord_t *,PrefState_e);	/* Callback proc for readers */

typedef struct IniIndex_t {	/* Win95 style prefs file image header struct */
	const char *Header;		/* Header to scan for image data */
	const char *ImagePtr;	/* Pointer to the image in memory */
	Word32 ImageLength;	/* Length of the image in memory */
} IniIndex_t;

typedef struct PrefsRecord_t {	/* A single search record */
	char *EntryName;	/* Ascii of entry */
	PrefsRecordProc_t Proc;	/* Callback to process the data */
	void *DataPtr;		/* Pointer to data to store */
	void *Default;		/* Default data pointer (Or possible value) */
	Word Count;			/* Size in elements of buffer */
} PrefsRecord_t;

typedef struct PrefsTemplate_t {	/* Prefs file description record */
	char *Header;		/* Ascii for the header */
	Word Count;			/* Number of entries to scan */
	PrefsRecord_t *ArrayPtr;	/* Array of "Count" entries */
} PrefsTemplate_t;

extern char NibbleToAscii[16];		/* 0-F in upper case */
extern char NibbleToAsciiLC[16];	/* 0-f in lower case */
#define LongWordToAscii(Input,AsciiPtr) LongWordToAscii2(Input,AsciiPtr,0)
extern char * BURGERCALL LongWordToAscii2(Word32 Input,char *AsciiPtr,Word Printing);
#define longToAscii(Input,AsciiPtr) longToAscii2(Input,AsciiPtr,0)
extern char * BURGERCALL longToAscii2(long Input,char *AsciiPtr,Word Printing);
#define AsciiToLongWord(AsciiPtr) AsciiToLongWord2(AsciiPtr,0)
extern Word32 BURGERCALL AsciiToLongWord2(const char *AsciiPtr,char **DestPtr);
#define LongWordToAsciiHex(Input,AsciiPtr) LongWordToAsciiHex2(Input,AsciiPtr,0)
extern char * BURGERCALL LongWordToAsciiHex2(Word32 Input,char *AsciiPtr,Word Printing);
extern char * BURGERCALL ParseBeyondEOL(const char *Input);
extern char * BURGERCALL ParseBeyondWhiteSpace(const char *Input);
extern char * BURGERCALL StoreAString(char *WorkPtr,const char *Input);
extern char * BURGERCALL StoreALongWordAscii(char *WorkPtr,Word32 Input);
extern char * BURGERCALL StoreALongWordAsciiHex(char *WorkPtr,Word32 Input);
extern char * BURGERCALL StoreAlongAscii(char *WorkPtr,long Input);
extern char * BURGERCALL StoreAParsedString(char *WorkPtr,const char *Input);
extern char * BURGERCALL GetAParsedString(const char *WorkPtr,char *DestPtr,Word Size);
#define PrefsWordProc PrefsLongWordProc
#define PrefsWordArrayProc PrefsLongWordArrayProc
extern char * BURGERCALL PrefsLongWordProc(char *WorkPtr,PrefsRecord_t *RecordPtr,PrefState_e Pass);
extern char * BURGERCALL PrefsShortProc(char *WorkPtr,PrefsRecord_t *RecordPtr,PrefState_e Pass);
extern char * BURGERCALL PrefsLongWordArrayProc(char *WorkPtr,PrefsRecord_t *RecordPtr,PrefState_e Pass);
extern char * BURGERCALL PrefsShortArrayProc(char *WorkPtr,PrefsRecord_t *RecordPtr,PrefState_e Pass);
extern char * BURGERCALL PrefsStringProc(char *WorkPtr,PrefsRecord_t *RecordPtr,PrefState_e Pass);
extern Word BURGERCALL ScanIniImage(IniIndex_t *IndexPtr,PrefsRecord_t *Record);
extern Word32 BURGERCALL LongWordFromIniImage(const char *Header,const char *EntryName,const char *Input,Word32 InputLength);
extern void * BURGERCALL PrefsCreateFileImage(PrefsTemplate_t *MyTemplate,Word32 *LengthPtr);
extern Word BURGERCALL PrefsWriteFile(PrefsTemplate_t *MyTemplate,const char *FileName,Word AppendFlag);
extern void BURGERCALL PrefsParseFile(PrefsTemplate_t *MyTemplate,const char *FilePtr,Word32 Length);
extern Word BURGERCALL PrefsReadFile(PrefsTemplate_t *MyTemplate,const char *FileName);

#ifdef __cplusplus
}
#endif

#endif

