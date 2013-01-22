/*******************************

	Resource manager

*******************************/

#ifndef __RZREZ_H__
#define __RZREZ_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RezHeader_t {	/* Master entry to the resource manager */
	void (BURGERCALL *DecompPtrs[3])(Word8 *,Word8 *,Word32,Word32);	/* Decompressors */
	Word32 Count;				/* Number of resource groups */
	Word32 RezNameCount;		/* Number of resource names */
	void *fp;					/* Open file reference */
	struct RezGroup_t **GroupHandle;	/* First entry */
	struct RezName_t **RezNames;		/* Handle to resource names if present */
	Word Flags;					/* Flags on how to handle resources */
} RezHeader_t;					

typedef struct RezNameReturn_t {
	char *RezName;		/* Resource name */
	Word RezNum;		/* Resource number */
} RezNameReturn_t;

/* Resource Handlers */

typedef void (BURGERCALL *ResourceDecompressorProcPtr)(Word8 *,Word8 *,Word32 Length,Word32 PackLength);

extern Bool ResourceJustLoaded;	/* TRUE if ResourceLoadHandle() freshly loaded a handle */
extern RezHeader_t MasterRezHeader;	/* Default resource file */

extern RezHeader_t * BURGERCALL ResourceNew(const char *FileName,Word32 StartOffset);
extern Word BURGERCALL ResourceInit(RezHeader_t *Input,const char *FileName,Word32 StartOffset);
extern void BURGERCALL ResourceDestroy(RezHeader_t *Input);
extern void BURGERCALL ResourceDelete(RezHeader_t *Input);
extern Word BURGERCALL ResourceInitMasterRezHeader(const char *FileName);
extern void BURGERCALL ResourcePurgeCache(RezHeader_t *Input);
extern Word BURGERCALL ResourceExternalFlag(RezHeader_t *Input,Word Flag);
extern Word BURGERCALL ResourceDontCacheFlag(RezHeader_t *Input,Word Flag);
extern Word BURGERCALL ResourceAddName(RezHeader_t *Input,const char *RezName);
extern void BURGERCALL ResourceRemove(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourceRemoveName(RezHeader_t *Input,const char *RezName);
extern Word BURGERCALL ResourceRead(RezHeader_t *Input,Word RezNum,void *DestPtr,Word32 BufSize);
extern void * BURGERCALL ResourceLoad(RezHeader_t *Input,Word RezNum);
extern void * BURGERCALL ResourceLoadByName(RezHeader_t *Input,const char *RezName);
extern void ** BURGERCALL ResourceLoadHandle(RezHeader_t *Input,Word RezNum);
extern void ** BURGERCALL ResourceLoadHandleByName(RezHeader_t *Input,const char *RezName);
extern void BURGERCALL ResourceRelease(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourceReleaseByName(RezHeader_t *Input,const char *RezName);
extern void BURGERCALL ResourceKill(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourceKillByName(RezHeader_t *Input,const char *RezName);
extern void BURGERCALL ResourceDetach(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourceDetachByName(RezHeader_t *Input,const char *RezName);
extern void BURGERCALL ResourcePreload(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourcePreloadByName(RezHeader_t *Input,const char *RezName);
extern Word BURGERCALL ResourceGetRezNum(RezHeader_t *Input,const char *RezName);
extern Word BURGERCALL ResourceGetName(RezHeader_t *Input,Word RezNum,char *Buffer,Word BufferSize);
extern Word BURGERCALL ResourceGetIDFromHandle(RezHeader_t *Input,const void **RezHand,Word *IDFound,char *NameBuffer,Word NameBufferSize);
extern Word BURGERCALL ResourceGetIDFromPointer(RezHeader_t *Input,const void *RezPtr,Word *IDFound,char *NameBuffer,Word NameBufferSize);
extern RezNameReturn_t *BURGERCALL ResourceGetNameArray(RezHeader_t *Input,Word *EntryCountPtr);
extern void BURGERCALL ResourceLogDecompressor(RezHeader_t *Input,Word CompressID,ResourceDecompressorProcPtr Proc);
extern struct LWShape_t * BURGERCALL ResourceLoadShape(RezHeader_t *Input,Word RezNum);
extern struct LWXShape_t * BURGERCALL ResourceLoadXShape(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourcePreloadShape(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourcePreloadXShape(RezHeader_t *Input,Word RezNum);
extern void * BURGERCALL ResourceLoadShapeArray(RezHeader_t *Input,Word RezNum);
extern void * BURGERCALL ResourceLoadXShapeArray(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourcePreloadShapeArray(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourcePreloadXShapeArray(RezHeader_t *Input,Word RezNum);
extern struct LWShape_t ** BURGERCALL ResourceLoadShapeHandle(RezHeader_t *Input,Word RezNum);
extern struct LWXShape_t ** BURGERCALL ResourceLoadXShapeHandle(RezHeader_t *Input,Word RezNum);
extern void ** BURGERCALL ResourceLoadShapeArrayHandle(RezHeader_t *Input,Word RezNum);
extern void ** BURGERCALL ResourceLoadXShapeArrayHandle(RezHeader_t *Input,Word RezNum);
extern struct GfxShape_t *BURGERCALL ResourceLoadGfxShape(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourcePreloadGfxShape(RezHeader_t *Input,Word RezNum);
extern void ** BURGERCALL ResourceLoadGfxShapeHandle(RezHeader_t *Input,Word RezNum);

#ifdef __cplusplus
}
#endif

#endif

