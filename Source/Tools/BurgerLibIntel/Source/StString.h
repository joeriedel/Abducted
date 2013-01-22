/**********************************

	Burgerlib String Manager
	Copyright Bill Heineman
	All rights reserved.
	Written by Bill Heineman

**********************************/

#ifndef __STSTRING_H__
#define __STSTRING_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
struct LinkedList_t;

typedef struct LWString_t {
	char *DataPtr;			/* Pointer to actual string */
	Word StrLength;			/* Length of the string */
	Word BufferLength;		/* Length of the string buffer */
	char Bogus;				/* Bogus string for no memory (Failsafe) */
	char Pad1,Pad2,Pad3;	/* Make sure I am long word aligned */
} LWString_t;

/* String Handlers */

extern void BURGERCALL StrStripLeadingSpaces(char* Input);
extern void BURGERCALL StrStripLeadingWhiteSpace(char* Input);
extern void BURGERCALL StrStripTrailingSpaces(char* Input);
extern void BURGERCALL StrStripTrailingWhiteSpace(char* Input);
extern void BURGERCALL StrStripLeadingAndTrailingSpaces(char* Input);
extern void BURGERCALL StrStripLeadingAndTrailingWhiteSpace(char* Input);
extern void BURGERCALL StrStripAllBut(char* Input,const char* ListPtr);
extern void BURGERCALL StrStripAll(char* Input,const char* ListPtr);
extern void BURGERCALL StrStripTrailing(char* Input, const char* ListPtr);
extern void BURGERCALL StrStripLeading(char* Input, const char* ListPtr);
extern char * BURGERCALL StrParseToDelimiter(const char *Input);
extern void BURGERCALL StrParse(struct LinkedList_t *ListPtr,const char *Input);
extern void BURGERCALL StrGetComputerName(char* Output,Word OutputSize);
extern void BURGERCALL StrGetUserName(char* Output,Word OutputSize);
extern char* BURGERCALL StrGetFileExtension(const char *Input);
extern void BURGERCALL StrSetFileExtension(char* Input,const char* NewExtension);
extern char* BURGERCALL StrCopy(const char *Input);
extern char* BURGERCALL StrCopyPad(const char *Input,Word Padding);
extern void ** BURGERCALL StrCopyHandle(const char *Input);
extern void ** BURGERCALL StrCopyPadHandle(const char *Input,Word Padding);
extern char* BURGERCALL DebugStrCopy(const char *Input,const char *File,Word Line);
extern char* BURGERCALL DebugStrCopyPad(const char *Input,Word Padding,const char *File,Word Line);
extern void ** BURGERCALL DebugStrCopyHandle(const char *Input,const char *File,Word Line);
extern void ** BURGERCALL DebugStrCopyPadHandle(const char *Input,Word Padding,const char *File,Word Line);

#if _DEBUG
#define StrCopy(x) DebugStrCopy(x,__FILE__,__LINE__)
#define StrCopyPad(x,y) DebugStrCopyPad(x,y,__FILE__,__LINE__)
#define StrCopyHandle(x) DebugStrCopyHandle(x,__FILE__,__LINE__)
#define StrCopyPadHandle(x,y) DebugStrCopyPadHandle(x,y,__FILE__,__LINE__)
#endif

#ifdef __cplusplus
}
#endif

#endif

