/**********************************

	Load data from disk async

**********************************/

#ifndef __RSREADSTREAM_H__
#define __RSREADSTREAM_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

struct ReadFileStream_t;

#ifdef __cplusplus
extern "C" {
#endif

extern struct ReadFileStream_t *BURGERCALL ReadFileStreamNew(const char *FileName,Word Count,Word ChunkSize);
extern void BURGERCALL ReadFileStreamDelete(struct ReadFileStream_t *Input);
extern Word BURGERCALL ReadFileStreamActive(struct ReadFileStream_t *Input);
extern Word BURGERCALL ReadFileStreamPending(struct ReadFileStream_t *Input);
extern void BURGERCALL ReadFileStreamStop(struct ReadFileStream_t *Input);
extern void BURGERCALL ReadFileStreamStart(struct ReadFileStream_t *Input,Word32 Offset);
extern Word32 BURGERCALL ReadFileStreamAvailBytes(struct ReadFileStream_t *Input);
extern Word8 * BURGERCALL ReadFileStreamGetData(struct ReadFileStream_t *Input,Word32 *ReadSizeOut,Word32 ReadSize);
extern void BURGERCALL ReadFileStreamAcceptData(struct ReadFileStream_t *Input);

#ifdef __cplusplus
}
#endif

#endif
