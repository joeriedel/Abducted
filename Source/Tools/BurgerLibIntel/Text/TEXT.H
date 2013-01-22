/*********************************

    Text based console driver library
    Copyright 1995, Parson's Engineering
    Use with the WATCOM "C" compiler for the IBM PC

*********************************/

#ifndef __BURGER__
#include <Burger.h>
#endif

#ifndef _STDIO_H_INCLUDED
#include <stdio.h>
#endif

#ifndef __TEXT__
#define __TEXT__
#ifdef __cplusplus
extern "C" {
#endif

#define ZERO "\x00"
#define HOME "\x1e"
#define CR "\r"
#define LF "\n"
#define BELL "\x7"
#define ESC "\x1B"
#define ESCT "\x1BT"
#define ESCY "\x1BY"

typedef enum {TIME12,TIME24,TIME12SEC,TIME24SEC} OutTime_t;
typedef enum {DATEUSA,DATEEUROPE} OutDate_t;
typedef struct {			/* Struct for DispatchKey call */
	Word Key;				/* Key code */
	void (*Code)(void);		/* Pointer to handler */
} DispatchKey_t;
typedef struct {
	void (*Code)(Word);		/* Pointer to previous OutCharVector */
	Byte *TextPtr;			/* Pointer to previous capture string (If any) */
} CaptureText_t;
typedef struct {
	Word *DestWord;			/* Pointer to variable */
	Byte *Strings[1];		/* String pointer list */
} TextListCtrl_t;
typedef enum {		/* Text control types supported */
	ELLowerCase,ELUpperCase,ELCharListEntry,
	ELNumberEntry,ELSingleDigit,ELFloatEntry,ELMultiListEntry
} TextControl_t;
typedef enum {
	IPKeyHit,IPIdle,IPDrawOn,IPDrawOff
} DoInputCB_t;
typedef struct {	/* Struct for text control */
	Word TextX,TextY;		/* X,Y of the text input control */
	Word Width;				/* Width of the control in chars */
	TextControl_t InputType;	/* Type of control */
	void *DestBuffer;		/* Pointer to data buffer */
} EntryList_t;

extern void ConsoleBeep(void);
extern void ConsoleCursorOff(void);
extern void ConsoleCursorOn(void);
extern void ClearEOL(void);
extern void ClearEOP(void);
extern void ClearScreen(void);
extern void SetXY(Word x,Word y);
extern void HomeCursor(void);
extern void TabToSpace(Word NewX);
extern void OutString(Byte *StrPtr);
extern void OutTabbedList(Byte *List,Word MinTab);
extern void OutCenteredList(Byte *List);
extern void OutCenteredAverageList(Byte *List);
extern void (*OutCharVector)(Word Letter);
extern void OutChar(Word Letter);
extern void OutChars(Word Letter,Word Count);
extern void OutBackSpace(void);
extern void OutBackSpaces(Word Count);
extern void OutSpace(void);
extern void OutSpaces(Word Count);
extern void OutCRLF(void);
extern void OutCenter(Byte *StrPtr);
extern void OutHexNibble(Word Value);
extern void OutHexByte(Word Value);
extern void OutHexShort(Word Value);
extern void OutOnOff(Word Value);
extern void OutLongWord(LongWord Value);
extern void OutLongWord2(LongWord Value,Word MinSize,Word LeadChar);
extern void OutMoneyFloat(float Value);
extern void OutMoneyFloat2(float Value,Word MinSize);
extern void OutTime(OutTime_t Flag);
extern void OutDate(OutDate_t Flag);
extern void OutFormatted(Byte *StrPtr);
extern void OutFormatted2(Byte *StrPtr,void **Vars);
extern Word HexAsc(Word Value);
extern Word AscHex(Word Value);
extern void CaptureText(CaptureText_t *OldCapture,Byte *TextStr);
extern void FinishCapture(CaptureText_t *OldCapture);
extern void CaptureFormatted(Byte *TextStr,Byte *FormattedStr);
extern void CaptureFormatted2(Byte *TextStr,Byte *FormattedStr,void **Vars);
extern void SOutLongWord(Byte *TextStr,LongWord Value);
extern void SOutLongWord2(Byte *TextStr,LongWord Value,Word MinSize,Word LeadChar);
extern void SOutMoneyFloat(Byte *TextStr,float Value);
extern void SOutMoneyFloat2(Byte *TextStr,float Value,Word Minsize);
extern Byte *ScanWord(Byte *StrPtr,Word *Result);

#define ILEcho 0x00             /* Allow character echo */
#define ILNoEcho 0x01           /* Hide the password */
#define ILUpperCase 0x00        /* Force upper case input */
#define ILLowerCase 0x02        /* Allow lower case input */
#define ILNumeric 0x04          /* Allow numeric chars only */
#define ILFloat 0x0C            /* Allow floating point numbers */
#define ILInputValid 0x10       /* Input buffer preset */
#define ILCallBack 0x40			/* CallBack proc for input line */
extern Word InputLine(Byte *Buffer,Word Max,Boolean EchoFlag);
extern Word (*InputLineCallBack)(void);
extern Boolean InputNumber(Word *Answer);
extern Boolean InputPrice(float *Answer);
extern Word GetACursorKey(void);
extern Word WaitCursorKey(void);
#define DKNoCursor 0x00			/* No text cursor */
#define DKCursor 0x01			/* Display neat cursor */
#define DKUpperCase 0x02		/* Force upper case */
extern void DispatchKey(Word Flag,DispatchKey_t *Vectors);
extern void DrawOneInputStruct(EntryList_t *EntryPtr);
extern void DrawInputStruct(EntryList_t *EntryPtr,Word Entries);
extern Word DoInput(EntryList_t *EntryPtr);
extern Word InputStruct(EntryList_t *EntryPtr);
extern Word InputStruct2(EntryList_t *EntryPtr,Word (*CallBack)(DoInputCB_t,LongWord));

extern Word TextX;              /* Current console text X coord */
extern Word TextY;              /* Current console text Y coord */
extern Word TextAttrib;         /* Attribute byte for CGA screens */
extern Word TextWidth;          /* Width of video console */
extern Word TextHeight;         /* Height of video console */
extern Word TextInverse;		/* True for inverse text */
extern Short *TextScreen;
extern void FixVideo(void);
extern void InitVideo(void);
extern void FixVideoWidthHeight(void);
extern void Set80x25(void);
extern void Set80x50(void);
extern void PutCharConsole(Word Letter);
extern Word PostKeyPress(Word ASCIIChar);

extern void KillFileWhiteSpace(FILE *fp);
extern void KillFileLine(FILE *fp);
extern Boolean GetAFileToken(FILE *fp,Byte *Dest,Word MaxLen);
extern Word VarToken(Byte *Token,Byte **TokenList);
extern void FetchFileString(FILE *fp,Byte *Dest,Word MaxLen);
extern Word FetchFileWord(FILE *fp);
extern Word FetchFileBinary(FILE *fp);
extern float FetchFileFloat(FILE *fp);
extern void WriteFileString(FILE *fp,Byte *SrcPtr);
extern void WriteFileWord(FILE *fp,Word Val);
extern void WriteFileBinary(FILE *fp,Word Val);
extern void WriteFileFloat(FILE *fp,float Val);
extern void WriteFileStrings(FILE *fp,Byte **Array,Word MaxWidth);

extern void IrqOn(void);
extern void IrqOff(void);
extern void MSDelay(LongWord Value);
extern float RoundCent(float Value);
#ifdef __cplusplus
};
#endif
#endif
