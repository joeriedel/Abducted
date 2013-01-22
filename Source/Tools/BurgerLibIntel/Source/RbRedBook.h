/*******************************

	Burgerlib's CD Audio manager

*******************************/

#ifndef __RBREDBOOK_H__
#define __RBREDBOOK_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* CD Audio Manager */

#define REDBOOKMODEBUSY 0
#define REDBOOKMODESTOPPED 1
#define REDBOOKMODEPLAYING 2
#define REDBOOKMODEPAUSED 3
#define REDBOOKMODEOPENED 4

#define REDBOOKPLAY 0
#define REDBOOKPLAYONCE 1
#define REDBOOKPLAYLOOP 2

#define FRAMESPERSEC 75

#if defined(__WIN32__)	/* Windows 95 version */
typedef struct {			/* Members are private!! */
	Word32 FromMark;		/* Starting mark for loop */
	Word32 ToMark;		/* Destination mark used for resume */
	Word32 OpenDeviceID;	/* Win95 MMSystem device */
	void *ThreadID;			/* ID for the thread processing the CD */
	Word MixerDevice;		/* Device for audio control */
	Word MixerFlags;		/* Volume enable flag for mixer */
	Word LoopingTimerID;	/* Timer proc for looping a CD file */
	Bool Paused;			/* True if paused (Public) */
	Bool Active;			/* True if properly initialized (Public) */
	Bool Timer;			/* True if loop thread is spawned */
	Bool Mixer;			/* True if volume control is present */
} Redbook_t;

#elif defined(__MAC__)	/* MacOS version */
typedef struct {			/* Members are private!! */
	Word32 FromMark;		/* Starting mark for loop */
	Word32 ToMark;		/* Destination mark used for resume */
	Word32 TimeMark;		/* Timer for thread */
	void *ProcPtr;			/* IOCompletionUPP for async calls */
	Word8 ParamData[84*4];	/* MacOS ParamBlockRec (And a pad byte) */
	short OpenDeviceID;		/* MacOS device reference */
	Bool Paused;			/* True if paused (Public) */
	Bool Active;			/* True if properly initialized (Public) */
	Bool Timer;			/* True if loop thread is spawned */
} Redbook_t;
#else					/* DOS version */
typedef struct {			/* Members are private!! */
	Word32 FromMark;		/* Starting mark for loop */
	Word32 ToMark;		/* Destination mark used for resume */
	Word32 TimeMark;		/* Timer for thread */
	Word OpenDeviceID;		/* Dos device number */
	Bool Paused;			/* True if paused (Public) */
	Bool Active;			/* True if properly initialized (Public) */
	Bool Timer;			/* True if loop thread is spawned */
} Redbook_t;

#endif

extern Redbook_t * BURGERCALL RedbookNew(void);
extern Word BURGERCALL RedbookInit(Redbook_t *Input);
extern void BURGERCALL RedbookDelete(Redbook_t *Input);
extern void BURGERCALL RedbookDestroy(Redbook_t *Input);
extern void BURGERCALL RedbookPlay(Redbook_t *Input,Word Mode,Word Track);
extern void BURGERCALL RedbookPlayRange(Redbook_t *Input,Word Mode,Word StartTrack,Word32 StartPosition,Word EndTrack,Word32 EndPosition);
extern Word BURGERCALL RedbookGetCurrentTrack(Redbook_t *Input);
extern Word BURGERCALL RedbookGetTrackCount(Redbook_t *Input);
extern Word32 BURGERCALL RedbookGetTrackLength(Redbook_t *Input,Word Track);
extern Word32 BURGERCALL RedbookGetTrackStart(Redbook_t *Input,Word TrackNum);
extern void BURGERCALL RedbookStop(Redbook_t *Input);
extern void BURGERCALL RedbookGetVolume(Redbook_t *Input,Word *Left,Word *Right);
extern void BURGERCALL RedbookSetVolume(Redbook_t *Input,Word Left,Word Right);
extern Word BURGERCALL RedbookGetPlayStatus(Redbook_t *Input);
extern void BURGERCALL RedbookPause(Redbook_t *Input);
extern void BURGERCALL RedbookResume(Redbook_t *Input);
extern Word32 BURGERCALL RedbookGetPosition(Redbook_t *Input);
extern void BURGERCALL RedbookOpenDrawer(Redbook_t *Input);
extern void BURGERCALL RedbookCloseDrawer(Redbook_t *Input);
extern Word32 BURGERCALL RedbookMakeTMSF(Word Track,Word32 Position);
extern Word32 BURGERCALL RedbookMakePosition(Word Minutes,Word Seconds,Word Frames);
extern void BURGERCALL RedbookSetLoopStart(Redbook_t *Input,Word Track,Word32 Position);
extern void BURGERCALL RedbookSetLoopEnd(Redbook_t *Input,Word Track,Word32 Position);
extern Word BURGERCALL RedbookLogCDByName(Redbook_t *Input,const char *VolumeName);

#ifdef __cplusplus
}
#endif


#endif
