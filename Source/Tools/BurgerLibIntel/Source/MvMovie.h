/**********************************

	Flic file manager

**********************************/

#ifndef __MVMOVIE_H__
#define __MVMOVIE_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __SHSTREAM_H__
#include "ShStream.h"
#endif

#ifndef __IMIMAGE_H__
#include "IMImage.h"
#endif

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Flic player */

#define MOVIEDISKBASED 0
#define MOVIERAMBASED 1

typedef struct FlicMovie_t {
	FILE *fp;				/* Input file stream */
	Word8 *Buffer;			/* Input file buffer */
	Word32 BufferSize;	/* Size of the input buffer */
	StreamHandle_t MyInput;	/* Ram based input buffer */
	Word32 MovieSpeed;	/* Speed to play a movie */
	Word32 TickStart;		/* Tick the movie began */
	Word32 TickStop;		/* Tick the movie was paused at */
	Word32 FirstFrameMark;	/* Offset to the first movie frame */
	Word32 LoopFrameMark;	/* Offset to the loop frame */
	Word32 FileOffset;	/* Offset into the file for seeking */
	Image_t FrameImage;		/* Current image of the movie */
	Word CurrentFrameNumber;	/* Current frame being shown */
	Word MaxFrameNumber;	/* Frames in the movie */
	Bool Active;			/* True if this movie is in progress */
	Bool Paused;			/* True if the movie is paused */
	Bool Completed;		/* True if the movie has completed */
	Bool AllowFrameSkipping;	/* True if I can skip frames */
	Bool Looping;		/* True if it loops */
} FlicMovie_t;

extern FlicMovie_t * BURGERCALL FlicMovieNew(const char *FileName,Word Flags,Word32 FileOffset);
extern Word BURGERCALL FlicMovieInit(FlicMovie_t *Input,const char *FileName,Word Flags,Word32 FileOffset);
extern void BURGERCALL FlicMovieDelete(FlicMovie_t *Input);
extern void BURGERCALL FlicMovieDestroy(FlicMovie_t *Input);
#define FlicMovieIsPlaying(Input) ((Input)->Active)
#define FlicMovieGetWidth(Input) ((Input)->FrameImage.Width)
#define FlicMovieGetHeight(Input) ((Input)->FrameImage.Height)
extern Word BURGERCALL FlicMovieDataRead(FlicMovie_t *Input,void *Data,Word32 Length);
extern void BURGERCALL FlicMovieReset(FlicMovie_t *Input);
extern Word BURGERCALL FlicMovieStart(FlicMovie_t *Input);
extern void BURGERCALL FlicMovieStop(FlicMovie_t *Input);
extern void BURGERCALL FlicMoviePause(FlicMovie_t *Input);
extern void BURGERCALL FlicMovieResume(FlicMovie_t *Input);
extern void BURGERCALL FlicMovieSetLoopFlag(FlicMovie_t *Input,Word LoopFlag);
extern void BURGERCALL FlicMovieSetSpeed(FlicMovie_t *Input,Word FramesPerSecond);
extern void BURGERCALL FlicMovieSetToFrame(FlicMovie_t *Input,Word FrameNumber);
extern Image_t * BURGERCALL FlicMovieGetImage(FlicMovie_t *Input);
extern Image_t * BURGERCALL FlicMovieUpdate(FlicMovie_t *Input);

/* DPaint Anim player */

typedef struct DPaintAnimMovie_t {
	FILE *fp;				/* Input file stream */
	Word8 *Buffer;			/* Input file buffer */
	Word8 *DictionaryBuffer;	/* Dictionary buffer */
	StreamHandle_t MyInput;	/* Ram based input buffer */
	Word32 MovieSpeed;	/* Speed to play a movie */
	Word32 TickStart;		/* Tick the movie began */
	Word32 TickStop;		/* Tick the movie was paused at */
	Word32 FileOffset;	/* Offset into the file for seeking */
	Word DictionarySize;	/* Size of the dictionary in records */
	Image_t FrameImage;		/* Current image of the movie */
	Word CurrentFrameNumber;	/* Current frame being shown */
	Word MaxFrameNumber;	/* Frames in the movie */
	Bool Active;			/* True if this movie is in progress */
	Bool Paused;			/* True if the movie is paused */
	Bool Completed;		/* True if the movie has completed */
	Bool AllowFrameSkipping;	/* True if I can skip frames */
	Bool Looping;		/* True if it loops */
} DPaintAnimMovie_t;

extern DPaintAnimMovie_t * BURGERCALL DPaintAnimMovieNew(const char *FileName,Word Flags,Word32 FileOffset);
extern Word BURGERCALL DPaintAnimMovieInit(DPaintAnimMovie_t *Input,const char *FileName,Word Flags,Word32 FileOffset);
extern void BURGERCALL DPaintAnimMovieDelete(DPaintAnimMovie_t *Input);
extern void BURGERCALL DPaintAnimMovieDestroy(DPaintAnimMovie_t *Input);
#define DPaintAnimMovieIsPlaying(Input) ((Input)->Active)
#define DPaintAnimMovieGetWidth(Input) ((Input)->FrameImage.Width)
#define DPaintAnimMovieGetHeight(Input) ((Input)->FrameImage.Height)
extern Word BURGERCALL DPaintAnimMovieDataRead(DPaintAnimMovie_t *Input,void *Data,Word32 Length);
extern void BURGERCALL DPaintAnimMovieReset(DPaintAnimMovie_t *Input);
extern Word BURGERCALL DPaintAnimMovieStart(DPaintAnimMovie_t *Input);
extern void BURGERCALL DPaintAnimMovieStop(DPaintAnimMovie_t *Input);
extern void BURGERCALL DPaintAnimMoviePause(DPaintAnimMovie_t *Input);
extern void BURGERCALL DPaintAnimMovieResume(DPaintAnimMovie_t *Input);
extern void BURGERCALL DPaintAnimMovieSetLoopFlag(DPaintAnimMovie_t *Input,Word LoopFlag);
extern void BURGERCALL DPaintAnimMovieSetSpeed(DPaintAnimMovie_t *Input,Word FramesPerSecond);
extern Image_t * BURGERCALL DPaintAnimMovieGetImage(DPaintAnimMovie_t *Input);
extern Image_t * BURGERCALL DPaintAnimMovieUpdate(DPaintAnimMovie_t *Input);

#ifdef __cplusplus
}
#endif

#endif

