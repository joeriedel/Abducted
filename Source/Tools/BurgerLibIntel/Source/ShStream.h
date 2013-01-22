/*******************************

	Streaming manager

*******************************/

#ifndef __SHSTREAM_H__
#define __SHSTREAM_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct Vector2D_t;
struct Vector3D_t;
struct Matrix3D_t;
struct Euler_t;
struct LWRect_t;
struct LWPoint_t;

/* Memory streaming routines */

typedef struct StreamHandle_t {
	Word8 **DataHandle;	/* Handle of the buffer for streaming */
	Word32 Mark;		/* Current file mark */
	Word32 BufferSize;	/* Maximum memory in buffer */
	Word32 EOFMark;	/* Size of the valid data */
	Word ErrorFlag;		/* Set to TRUE if an error occured */
} StreamHandle_t;

extern StreamHandle_t * BURGERCALL StreamHandleNewPut(void);
extern StreamHandle_t * BURGERCALL StreamHandleNewGet(void **GetHandle);
extern StreamHandle_t * BURGERCALL StreamHandleNewGetPtr(void *GetPtr,Word32 Size);
extern StreamHandle_t * BURGERCALL StreamHandleNewGetFile(const char *FileName);
extern void BURGERCALL StreamHandleInitPut(StreamHandle_t *Input);
extern void BURGERCALL StreamHandleInitGet(StreamHandle_t *Input,void **GetHandle);
extern void BURGERCALL StreamHandleInitGetPtr(StreamHandle_t *Input,void *GetPtr,Word32 Size);
extern Word BURGERCALL StreamHandleInitGetFile(StreamHandle_t *Input,const char *FileName);
extern void BURGERCALL StreamHandleDestroy(StreamHandle_t *Input);
extern void BURGERCALL StreamHandleDelete(StreamHandle_t *Input);
extern void ** BURGERCALL StreamHandleDetachHandle(StreamHandle_t *Input);
extern void BURGERCALL StreamHandleEndSave(StreamHandle_t *Input);
extern Word BURGERCALL StreamHandleSaveFile(StreamHandle_t *Input,const char *FileName);
extern Word BURGERCALL StreamHandleSaveTextFile(StreamHandle_t *Input,const char *FileName);
extern Word BURGERCALL StreamHandleGetByte(StreamHandle_t *Input);
extern Word BURGERCALL StreamHandleGetShort(StreamHandle_t *Input);
extern Word32 BURGERCALL StreamHandleGetLong(StreamHandle_t *Input);
extern float BURGERCALL StreamHandleGetFloat(StreamHandle_t *Input);
extern double BURGERCALL StreamHandleGetDouble(StreamHandle_t *Input);
extern void BURGERCALL StreamHandleGetVector2D(StreamHandle_t *Input,struct Vector2D_t *Output);
extern void BURGERCALL StreamHandleGetVector3D(StreamHandle_t *Input,struct Vector3D_t *Output);
extern void BURGERCALL StreamHandleGetMatrix3D(StreamHandle_t *Input,struct Matrix3D_t *Output);
extern void BURGERCALL StreamHandleGetEuler(StreamHandle_t *Input,struct Euler_t *Output);
extern void BURGERCALL StreamHandleGetLWRect(StreamHandle_t *Input,struct LWRect_t *Output);
extern void BURGERCALL StreamHandleGetLWPoint(StreamHandle_t *Input,struct LWPoint_t *Output);
extern Word BURGERCALL StreamHandleGetShortMoto(StreamHandle_t *Input);
extern Word32 BURGERCALL StreamHandleGetLongMoto(StreamHandle_t *Input);
extern float BURGERCALL StreamHandleGetFloatMoto(StreamHandle_t *Input);
extern double BURGERCALL StreamHandleGetDoubleMoto(StreamHandle_t *Input);
extern void BURGERCALL StreamHandleGetVector2DMoto(StreamHandle_t *Input,struct Vector2D_t *Output);
extern void BURGERCALL StreamHandleGetVector3DMoto(StreamHandle_t *Input,struct Vector3D_t *Output);
extern void BURGERCALL StreamHandleGetMatrix3DMoto(StreamHandle_t *Input,struct Matrix3D_t *Output);
extern void BURGERCALL StreamHandleGetEulerMoto(StreamHandle_t *Input,struct Euler_t *Output);
extern void BURGERCALL StreamHandleGetMem(StreamHandle_t *Input,void *DestPtr,Word32 Length);
extern void * BURGERCALL StreamHandleGetString(StreamHandle_t *Input,Word Flags);
extern void BURGERCALL StreamHandleGetString2(StreamHandle_t *Input,Word Flags,char *DestBuffer,Word32 MaxLength);
extern void BURGERCALL StreamHandlePutByte(StreamHandle_t *Input,Word Val);
extern void BURGERCALL StreamHandlePutShort(StreamHandle_t *Input,Word Val);
extern void BURGERCALL StreamHandlePutLong(StreamHandle_t *Input,Word32 Val);
extern void BURGERCALL StreamHandlePutFloat(StreamHandle_t *Input,float Val);
extern void BURGERCALL StreamHandlePutDouble(StreamHandle_t *Input,double Val);
extern void BURGERCALL StreamHandlePutVector2D(StreamHandle_t *Input,const struct Vector2D_t *Val);
extern void BURGERCALL StreamHandlePutVector3D(StreamHandle_t *Input,const struct Vector3D_t *Val);
extern void BURGERCALL StreamHandlePutMatrix3D(StreamHandle_t *Input,const struct Matrix3D_t *Val);
extern void BURGERCALL StreamHandlePutEuler(StreamHandle_t *Input,const struct Euler_t *Val);
extern void BURGERCALL StreamHandlePutShortMoto(StreamHandle_t *Input,Word Val);
extern void BURGERCALL StreamHandlePutLongMoto(StreamHandle_t *Input,Word32 Val);
extern void BURGERCALL StreamHandlePutFloatMoto(StreamHandle_t *Input,float Val);
extern void BURGERCALL StreamHandlePutDoubleMoto(StreamHandle_t *Input,double Val);
extern void BURGERCALL StreamHandlePutVector2DMoto(StreamHandle_t *Input,const struct Vector2D_t *Val);
extern void BURGERCALL StreamHandlePutVector3DMoto(StreamHandle_t *Input,const struct Vector3D_t *Val);
extern void BURGERCALL StreamHandlePutMatrix3DMoto(StreamHandle_t *Input,const struct Matrix3D_t *Val);
extern void BURGERCALL StreamHandlePutEulerMoto(StreamHandle_t *Input,const struct Euler_t *Val);
extern void BURGERCALL StreamHandlePutMem(StreamHandle_t *Input,const void *SrcPtr,Word32 Length);
extern void BURGERCALL StreamHandlePutString(StreamHandle_t *Input,const void *SrcPtr);
extern void BURGERCALL StreamHandlePutStringNoZero(StreamHandle_t *Input,const void *SrcPtr);
#define StreamHandleGetMark(x) (x)->Mark
extern void BURGERCALL StreamHandleSetMark(StreamHandle_t *Input,Word32 NewMark);
#define StreamHandleGetSize(x) (x)->EOFMark
extern void BURGERCALL StreamHandleSetSize(StreamHandle_t *Input,Word32 Size);
#define StreamHandleGetErrorFlag(x) (x)->ErrorFlag
#define StreamHandleClearErrorFlag(x) (x)->ErrorFlag=FALSE
#define StreamHandleSetErrorFlag(x) (x)->ErrorFlag=TRUE
extern void BURGERCALL StreamHandleSkip(StreamHandle_t *Input,long Offset);
extern void BURGERCALL StreamHandleSkipString(StreamHandle_t *Input);
extern void * BURGERCALL StreamHandleLock(StreamHandle_t *Input);
extern void * BURGERCALL StreamHandleLockExpand(StreamHandle_t *Input,Word32 Size);
extern void BURGERCALL StreamHandleUnlock(StreamHandle_t *Input,const void *EndPtr);
extern void BURGERCALL StreamHandleExpand(StreamHandle_t *Input,Word32 Size);

#ifdef __cplusplus
}
#endif


#endif

