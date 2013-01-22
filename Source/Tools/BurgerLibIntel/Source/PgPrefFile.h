/**********************************

	Library to handle windows style configs cripts

**********************************/

#ifndef __PGPREFFILE_H__
#define __PGPREFFILE_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PrefFile_t PrefFile_t;
typedef struct PrefFileSection_t PrefFileSection_t;
typedef struct PrefFileEntry_t PrefFileEntry_t;

extern PrefFile_t * BURGERCALL PrefFileNew(void);
extern PrefFile_t * BURGERCALL PrefFileNewFromMemory(const char *Data,Word32 Length);
extern PrefFile_t * BURGERCALL PrefFileNewFromFile(const char *FileName);
extern PrefFile_t * BURGERCALL PrefFileNewFromFileAlways(const char *FileName);
extern void BURGERCALL PrefFileDelete(PrefFile_t *Input);
extern Word BURGERCALL PrefFileSaveFile(PrefFile_t *Input,const char *FileName);
extern PrefFileSection_t * BURGERCALL PrefFileFindSection(PrefFile_t *Input,const char *SectionName);
extern PrefFileSection_t * BURGERCALL PrefFileFindSectionAlways(PrefFile_t *Input,const char *SectionName);
extern char *BURGERCALL PrefFileGetList(PrefFile_t *Input);
extern PrefFileEntry_t * BURGERCALL PrefFileSectionFindEntry(PrefFileSection_t *Input,const char *EntryName);
extern void BURGERCALL PrefFileDeleteSection(PrefFile_t *Input,const char *SectionName);
extern void BURGERCALL PrefFileDeletePrefFileSection(PrefFile_t *Input,PrefFileSection_t *SectionPtr);
extern PrefFileSection_t * BURGERCALL PrefFileAddSection(PrefFile_t *Input,const char *SectionName);
extern Word BURGERCALL PrefFileIsEntryPresent(PrefFile_t *Input,const char *SectionName,const char *EntryName);
extern char *BURGERCALL PrefFileSectionGetList(PrefFileSection_t *Input);
extern char *BURGERCALL PrefFileSectionGetRaw(PrefFileSection_t *Input,const char *EntryName);
extern Word BURGERCALL PrefFileSectionGetBoolean(PrefFileSection_t *Input,const char *EntryName,Word Default);
extern Word BURGERCALL PrefFileSectionGetWord(PrefFileSection_t *Input,const char *EntryName,Word Default,Word Min,Word Max);
extern int BURGERCALL PrefFileSectionGetInt(PrefFileSection_t *Input,const char *EntryName,int Default,int Min,int Max);
extern float BURGERCALL PrefFileSectionGetFloat(PrefFileSection_t *Input,const char *EntryName,float Default,float Min,float Max);
extern double BURGERCALL PrefFileSectionGetDouble(PrefFileSection_t *Input,const char *EntryName,double Default,double Min,double Max);
extern void BURGERCALL PrefFileSectionGetString(PrefFileSection_t *Input,const char *EntryName,const char *Default,char *Buffer,Word BufferSize);
extern void BURGERCALL PrefFileSectionGetDualString(PrefFileSection_t *Input,const char *EntryName,const char *Default,char *Buffer,Word BufferSize,const char *Default2,char *Buffer2,Word BufferSize2);
extern void BURGERCALL PrefFileSectionGetMem(PrefFileSection_t *Input,const char *EntryName,const Word8 *Default,Word8 *Buffer,Word BufferSize);
extern void BURGERCALL PrefFileSectionGetWordArray(PrefFileSection_t *Input,const char *EntryName,const Word *Default,Word *Buffer,Word Count);
extern void BURGERCALL PrefFileSectionAddEntry(PrefFileSection_t *Input,const char *EntryName,const char *Default);
extern void BURGERCALL PrefFileSectionPutRaw(PrefFileSection_t *Input,const char *EntryName,const char *RawString);
extern void BURGERCALL PrefFileSectionPutBoolean(PrefFileSection_t *Input,const char *EntryName,Word Data);
extern void BURGERCALL PrefFileSectionPutWord(PrefFileSection_t *Input,const char *EntryName,Word Data);
extern void BURGERCALL PrefFileSectionPutWordHex(PrefFileSection_t *Input,const char *EntryName,Word Data);
extern void BURGERCALL PrefFileSectionPutInt(PrefFileSection_t *Input,const char *EntryName,int Data);
extern void BURGERCALL PrefFileSectionPutFloat(PrefFileSection_t *Input,const char *EntryName,float Data);
extern void BURGERCALL PrefFileSectionPutDouble(PrefFileSection_t *Input,const char *EntryName,double Data);
extern void BURGERCALL PrefFileSectionPutString(PrefFileSection_t *Input,const char *EntryName,const char *Data);
extern void BURGERCALL PrefFileSectionPutDualString(PrefFileSection_t *Input,const char *EntryName,const char *Data,const char *Data2);
extern void BURGERCALL PrefFileSectionPutMem(PrefFileSection_t *Input,const char *EntryName,const Word8 *Data,Word Length);
extern void BURGERCALL PrefFileSectionPutWordArray(PrefFileSection_t *Input,const char *EntryName,const Word *DataPtr,Word Count);

#ifdef __cplusplus
}
#endif

#endif
