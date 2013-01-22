/**********************************

	File manager

**********************************/

#ifndef __FMFILE_H__
#define __FMFILE_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __TKTICK_H__
#include "TKTick.h"
#endif

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* File manager */

#define FULLPATHSIZE 2048	/* Maximum pathname size for Burgerlib */
#define PREFIXMAX 35		/* Maximum prefixs */
#define PREFIXBOOT 32		/* * prefix */
#define PREFIXPREFS 33		/* @ prefix */
#define PREFIXSYSTEM 34		/* $ prefix */

typedef struct DirectorySearch_t {
	Word32 FileSize;	/* Size of the file */
	TimeDate_t Create;	/* Creation time */
	TimeDate_t Modify;	/* Modification time */
	Bool Dir;		/* True if this is a directory */
	Bool System;		/* True if this is a system file */
	Bool Hidden;		/* True if this file is hidden */
	Bool Locked;		/* True if this file is read only */
	char Name[256];		/* Filename */
#if defined(__MAC__)
	Word Index;			/* Directory index */
	short VRefNum;		/* Volume ID */
	short Padding1;
	long DirID;			/* Directory to scan */
	Word32 FileType;	/* File's type */
	Word32 AuxType;	/* File's creator code */
#elif defined(__MACOSX__)
	void *Enumerator;	/* Filename enumerator */
	char *PathPrefix;	/* Pathname buffer */
	char *PathEnd;		/* Pathname ending */
#elif defined(__MSDOS__)
	Word HandleOk;		/* Handle is valid */
	short FileHandle;	/* Handle to the open directory */
	char MyFindT[44];	/* Dos FindT structure */
#elif defined(__WIN32__)
	void *FindHandle;	/* Win95 file handle */
	Word HandleOk;		/* Handle is valid */
	char MyFindT[320];	/* Win95 FindT structure */
#endif
} DirectorySearch_t;

extern void ** PrefixHandles[PREFIXMAX];
extern Word BURGERCALL GetFileModTime(const char *FileName,struct TimeDate_t *Output);
extern Word BURGERCALL GetFileModTimeNative(const char *FileName,struct TimeDate_t *Output);
extern Word BURGERCALL GetFileCreateTime(const char *FileName,struct TimeDate_t *Output);
extern Word BURGERCALL GetFileCreateTimeNative(const char *FileName,struct TimeDate_t *Output);
extern Word BURGERCALL DoesFileExist(const char *FileName);
extern Word BURGERCALL DoesFileExistNative(const char *FileName);
extern int BURGERCALL CompareTimeDates(const struct TimeDate_t *First,const struct TimeDate_t *Second);
extern Word BURGERCALL CreateDirectoryPath(const char *FileName);
extern Word BURGERCALL CreateDirectoryPath2(const char *FileName);
extern Word BURGERCALL CreateDirectoryPathNative(const char *FileName);
extern Word BURGERCALL OpenADirectory(DirectorySearch_t *Input,const char *Name);
extern Word BURGERCALL GetADirectoryEntry(DirectorySearch_t *Input);
extern Word BURGERCALL GetADirectoryEntryExtension(DirectorySearch_t *Input,const char *ExtPtr);
extern void BURGERCALL CloseADirectory(DirectorySearch_t *Input);
extern FILE * BURGERCALL OpenAFile(const char *FileName,const char *type);
extern Word BURGERCALL CopyAFile(const char *DestFile,const char *SrcFile);
extern Word BURGERCALL CopyAFileNative(const char *DestFile,const char *SrcFile);
extern Word BURGERCALL CopyAFileFP(FILE *Destfp,FILE *Srcfp);
extern Word BURGERCALL DeleteAFile(const char *FileName);
extern Word BURGERCALL DeleteAFileNative(const char *FileName);
extern Word BURGERCALL RenameAFile(const char *NewName,const char *OldName);
extern Word BURGERCALL RenameAFileNative(const char *NewName,const char *OldName);
extern char * BURGERCALL ExpandAPath(const char *FileName);
extern char * BURGERCALL ExpandAPathNative(const char *FileName);
extern void BURGERCALL ExpandAPathToBuffer(char *BufferPtr,const char *FileName);
extern void BURGERCALL ExpandAPathToBufferNative(char *BufferPtr,const char *FileName);
extern char * BURGERCALL ConvertNativePathToPath(const char *FileName);
extern char * BURGERCALL GetAPrefix(Word PrefixNum);
extern Word BURGERCALL SetAPrefix(Word PrefixNum,const char *PrefixName);
extern void BURGERCALL SetDefaultPrefixs(void);
extern void BURGERCALL PopAPrefix(Word PrefixNum);
extern char * BURGERCALL GetAVolumeName(Word DriveNum);
extern Word BURGERCALL FindAVolumeByName(const char *VolumeName);
extern Word BURGERCALL ChangeADirectory(const char *DirName);
extern Word32 BURGERCALL fgetfilesize(FILE *fp);
extern void BURGERCALL fwritelong(Word32 Val,FILE *fp);
extern void BURGERCALL fwritelongrev(Word32 Val,FILE *fp);
extern void BURGERCALL fwriteshort(Word16 Val,FILE *fp);
extern void BURGERCALL fwriteshortrev(Word16 Val,FILE *fp);
extern void BURGERCALL fwritestr(const char *ValPtr,FILE *fp);
extern Word32 BURGERCALL fgetlong(FILE *fp);
extern Word32 BURGERCALL fgetlongrev(FILE *fp);
extern short BURGERCALL fgetshort(FILE *fp);
extern short BURGERCALL fgetshortrev(FILE *fp);
extern Word BURGERCALL fgetstr(char *Input,Word Length,FILE *fp);
extern Word BURGERCALL SaveAFile(const char *FileName,const void *DataPtr,Word32 Length);
extern Word BURGERCALL SaveAFileNative(const char *FileName,const void *DataPtr,Word32 Length);
extern Word BURGERCALL SaveAFileFP(FILE *Filefp,const void *DataPtr,Word32 Length);
extern Word BURGERCALL SaveATextFile(const char *FileName,const void *DataPtr,Word32 Length);
extern Word BURGERCALL SaveATextFileNative(const char *FileName,const void *DataPtr,Word32 Length);
extern void * BURGERCALL LoadAFile(const char *FileName,Word32 *Length);
extern void * BURGERCALL LoadAFileNative(const char *FileName,Word32 *Length);
extern void * BURGERCALL LoadAFileFP(FILE *Filefp,Word32 *Length);
extern void ** BURGERCALL LoadAFileHandle(const char *FileName);
extern void ** BURGERCALL LoadAFileHandleNative(const char *FileName);
extern void ** BURGERCALL LoadAFileHandleFP(FILE *Filefp);
extern Word32 BURGERCALL GetAnAuxType(const char *FileName);
extern Word32 BURGERCALL GetAnAuxTypeNative(const char *FileName);
extern Word32 BURGERCALL GetAFileType(const char *FileName);
extern Word32 BURGERCALL GetAFileTypeNative(const char *FileName);
extern void BURGERCALL SetAnAuxType(const char *FileName,Word32 AuxType);
extern void BURGERCALL SetAnAuxTypeNative(const char *FileName,Word32 AuxType);
extern void BURGERCALL SetAFileType(const char *FileName,Word32 FileType);
extern void BURGERCALL SetAFileTypeNative(const char *FileName,Word32 FileType);
extern void BURGERCALL FileSetFileAndAuxType(const char *FileName,Word32 FileType,Word32 AuxType);
extern void BURGERCALL FileSetFileAndAuxTypeNative(const char *FileName,Word32 FileType,Word32 AuxType);
extern Word BURGERCALL AreLongFilenamesAllowed(void);

#if defined(__LITTLEENDIAN__)
#define fwritelongb(x,y) fwritelongrev(x,y)
#define fwritelongl(x,y) fwritelong(x,y)
#define fwriteshortb(x,y) fwriteshortrev(x,y)
#define fwriteshortl(x,y) fwriteshort(x,y)
#define fgetlongb(x) fgetlongrev(x)
#define fgetlongl(x) fgetlong(x)
#define fgetshortb(x) fgetshortrev(x)
#define fgetshortl(x) fgetshort(x)
#else
#define fwritelongb(x,y) fwritelong(x,y)
#define fwritelongl(x,y) fwritelongrev(x,y)
#define fwriteshortb(x,y) fwriteshort(x,y)
#define fwriteshortl(x,y) fwriteshortrev(x,y)
#define fgetlongb(x) fgetlong(x)
#define fgetlongl(x) fgetlongrev(x)
#define fgetshortb(x) fgetshort(x)
#define fgetshortl(x) fgetshortrev(x)
#endif

#ifdef __cplusplus
}
#endif


#endif

