/*******************************

	Burger's Universal library WIN95 version
	This is for Watcom 10.5 and higher...
	Also support for MSVC 4.0

*******************************/

#ifndef __CLSTDLIB_H__
#define __CLSTDLIB_H__

#include <string.h>

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* "C" convenience routines */

#define SWITCH_NORMAL 0x0
#define SWITCH_CALLBACK 0x1
#define SWITCH_WORD 0x2

#define ASCIILEADINGZEROS 0x8000U
#define ASCIINONULL 0x4000U

#define EXTRACTSTRDELIMITLF 0x01
#define EXTRACTSTRNOTRANSLATE 0x02
#define EXTRACTSTRHANDLE 0x04

typedef void (BURGERCALL *SwitchCallBackProc)(char *Input);

typedef struct Switch_t {	/* Used by Switches */
	char *StrPtr;	/* Ascii to check */
	Word32 Value;	/* Preset value to use (Or parm count) */
	void *ValuePtr;	/* Pointer to Word32 or function callback */
	Word Flags;		/* Flags for handler */
} Switch_t;

typedef struct MD2_t {	/* MD2 context */
	Word8 state[16];		/* state */
	Word8 checksum[16];	/* checksum */
	Word8 buffer[16];	/* input buffer */
	Word count;			/* number of bytes, modulo 16 */
} MD2_t;

typedef struct MD4_t {	/* MD4 context. */
	Word32 state[4];	/* Current 128 bit value */
	Word32 countlo;	/* Number of bytes processed (64 bit value) */
	Word32 counthi;
	Word8 buffer[64];	/* input buffer for processing */
} MD4_t;

typedef struct MD5_t {	/* MD5 context. */
	Word32 state[4];	/* Current 128 bit value */
	Word32 countlo;	/* Number of bytes processed (64 bit value) */
	Word32 counthi;
	Word8 buffer[64];	/* input buffer for processing */
} MD5_t;

extern Word8 ReverseBits[256];		/* Table to reverse bits in a byte */
extern Word BURGERCALL Switches(Word argc,char *argv[],const Switch_t *SwitchList);
extern char * BURGERCALL midstr(char *Dest,const char *Source,Word Start,Word Length);
extern char * BURGERCALL stristr(const char *Input, const char *SubStr);
extern void BURGERCALL CStr2PStr(char *DestPtr,const char *SrcPtr);
extern void BURGERCALL PStr2CStr(char *DestPtr,const char *SrcPtr);
extern Word32 BURGERCALL BCDToNum(Word32 Input);
extern Word32 BURGERCALL NumToBCD(Word32 Input);
extern Word32 BURGERCALL PowerOf2(Word32 Input);
extern Word32 BURGERCALL CalcMoreCRC32(const Word8 *buffPtr,Word32 buffSize,Word32 crc);
#define CalcCRC32(buffPtr,buffSize) CalcMoreCRC32(buffPtr,buffSize,0)
extern Word32 BURGERCALL CalcMoreAdler(const Word8 *Buffer,Word32 Length,Word32 crc);
#define CalcAdler(buffPtr,buffSize) CalcMoreAdler(buffPtr,buffSize,1)
extern Word BURGERCALL CalcMoreAdler16(const Word8 *Buffer,Word32 Length,Word CheckSum);
#define CalcAdler16(buffPtr,buffSize) CalcMoreAdler16(buffPtr,buffSize,1)
extern void BURGERCALL MD2Init(MD2_t *Input);
extern void BURGERCALL MD2Update(MD2_t *Input,const Word8 *BufferPtr,Word32 Length);
extern void BURGERCALL MD2Final(Word8 *Output,MD2_t *Input);
extern void BURGERCALL MD2Quick(Word8 *Output,const Word8 *BufferPtr,Word32 Length);
extern void BURGERCALL MD4Init(MD4_t *Input);
extern void BURGERCALL MD4Update(MD4_t *Input,const Word8 *BufferPtr,Word32 Length);
extern void BURGERCALL MD4Final(Word8 *Output,MD4_t *Input);
extern void BURGERCALL MD4Quick(Word8 *Output,const Word8 *BufferPtr,Word32 Length);
extern void BURGERCALL MD5Init(MD5_t *Input);
extern void BURGERCALL MD5Update(MD5_t *Input,const Word8 *BufferPtr,Word32 Length);
extern void BURGERCALL MD5Final(Word8 *Output,MD5_t *Input);
extern void BURGERCALL MD5Quick(Word8 *Output,const Word8 *BufferPtr,Word32 Length);
extern void * BURGERCALL ExtractAString(const char *SrcPtr,Word32 *BufSize,Word Flags);
extern void BURGERCALL ExtractAString2(const char *SrcPtr,Word32 *BufSize,Word Flags,char *DestPtr,Word32 DestSize);
extern void BURGERCALL FastMemCpy(void *DestPtr,const void *SrcPtr,Word32 Length);
#define FastMemCpyAlign(Dest,Src,Length) memcpy(Dest,Src,Length)
extern void BURGERCALL FastMemSet(void *DestPtr,Word Fill,Word32 Length);
extern void BURGERCALL FastMemSet16(void *DestPtr,Word Fill,Word32 Length);
extern Word BURGERCALL FastStrLen(const char *Input);
extern int BURGERCALL FastStrncmp(const char *Input1,const char *Input2,Word MaxLength);
inline void MemZero(Word8 *DestPtr,Word32 Size) { FastMemSet((void *)DestPtr,0,Size); }

#if defined(__MWERKS__) || defined(__BEOS__)
extern int BURGERCALL memicmp(const char *StrPtr1,const char *StrPtr2,Word Length);
#if !defined(_MSL_NEEDS_EXTRAS)
extern char * BURGERCALL strupr(char *Source);
extern char * BURGERCALL strlwr(char *Source);
extern char * BURGERCALL strdup(const char *source);
extern int BURGERCALL stricmp(const char *StrPtr1,const char *StrPtr2);
extern int BURGERCALL strnicmp(const char *StrPtr1,const char *StrPtr2,Word32 Len);
#endif
#endif

/* The point of DebugString is that it goes away in release code */

extern void BURGERCALL DebugXString(const char *String);
extern void BURGERCALL DebugXshort(short i);
extern void BURGERCALL DebugXShort(Word16 i);
extern void BURGERCALL DebugXlong(long i);
extern void BURGERCALL DebugXLongWord(Word32 i);
extern void BURGERCALL DebugXDouble(double i);
extern void BURGERCALL DebugXPointer(const void *i);
extern void ANSICALL DebugXMessage(const char *String,...);
#define DebugXWord(x) DebugXLongWord(x)

#if _DEBUG			/* Allow in debug code */
#define DebugString(x) DebugXString(x)
#define Debugshort(x) DebugXshort(x)
#define DebugShort(x) DebugXShort(x)
#define Debuglong(x) DebugXlong(x)
#define DebugLongWord(x) DebugXLongWord(x)
#define DebugDouble(x) DebugXDouble(x)
#define DebugPointer(x) DebugXPointer(x)
#define DebugWord(x) DebugXLongWord((Word32)x)
#define DebugMessage DebugXMessage
#else
#define DebugString(x)		/* Remove from release code */
#define Debugshort(x)
#define DebugShort(x)
#define Debuglong(x)
#define DebugLongWord(x)
#define DebugDouble(x)
#define DebugPointer(x)
#define DebugMessage 1 ? (void)0 : DebugXMessage
#define DebugWord(x)
#endif

#define DEBUGTRACE_MEMORYLEAKS 1 /* Test and display memory leaks */
#define DEBUGTRACE_REZLOAD 2	/* Print the name of a resource file being loaded */
#define DEBUGTRACE_FILELOAD 4	/* Print the name of a file being loaded */
#define DEBUGTRACE_WARNINGS 8	/* Print possible errors */
#define DEBUGTRACE_NETWORK 0x10	/* Network commands */
#define DEBUGTRACE_THEWORKS 0x1F	/* GIMME everything! */

typedef Word (BURGERCALL *SystemProcessCallBackProc)(const char *Input);

extern Word DebugTraceFlag;	/* Set these flag for debug spew */
extern Word BombFlag;		/* If true then bomb on ANY error */
extern Word IAmExiting;		/* TRUE if in a shutdown state */
extern char ErrorMsg[512];		/* Last message printed */
extern void ANSICALL Fatal(const char *FatalMsg,...);
extern void ANSICALL NonFatal(const char *ErrorMsg,...);
extern Word BURGERCALL SetErrBombFlag(Word Flag);
extern void BURGERCALL Halt(void);
extern void BURGERCALL SaveJunk(const void *Data,Word32 Length);
extern void BURGERCALL OkAlertMessage(const char *Title,const char *Message);
extern Word BURGERCALL OkCancelAlertMessage(const char *Title,const char *Message);
extern Word BURGERCALL SystemProcessFilenames(SystemProcessCallBackProc Proc);
extern Word BURGERCALL GetQuickTimeVersion(void);
extern Word BURGERCALL LaunchURL(const char *URLPtr);
extern Word BURGERCALL BurgerlibVersion(void);

typedef struct LibRef_t LibRef_t;
extern LibRef_t * BURGERCALL LibRefInit(const char *LibName);
extern void BURGERCALL LibRefDelete(LibRef_t *LibRef);
extern void * BURGERCALL LibRefGetProc(LibRef_t *LibRef,const char *ProcName);
extern void *BURGERCALL LibRefGetFunctionInLib(const char *LibName,const char *ProcName);

extern void BURGERCALL PrintHexDigit(Word Val);
extern void BURGERCALL PrintHexByte(Word Val);
extern void BURGERCALL PrintHexShort(Word Val);
extern void BURGERCALL PrintHexLongWord(Word32 Val);

#if defined(__WATCOMC__)
#pragma aux Halt = "INT 3" modify exact
#endif

#if defined(__INTEL__)
#define FastMemCpy(x,y,z) memcpy(x,y,z)
#endif

#undef strlen			/* For some compilers, this is intrinsic */
#define strlen(x) FastStrLen(x)

#ifdef __POWERPC__
#define strncmp(x,y,z) FastStrncmp(x,y,z)
#endif

/* Intel convience routines */

#if defined(__INTEL__)

typedef enum {
	IntelFP24=0,
	IntelFP56=2,
	IntelFP64=3
} IntelFP_e;

extern IntelFP_e BURGERCALL IntelSetFPPrecision(IntelFP_e Input);

#endif

#ifdef __cplusplus
}
#endif

#endif
