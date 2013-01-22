#include "RbRedBook.h"
#include "MmMemory.h"

/**********************************

	Create a new redbook audio structure

**********************************/

Redbook_t * BURGERCALL RedbookNew(void)
{
	Redbook_t *Input;
	Input = (Redbook_t *)AllocAPointer(sizeof(Redbook_t));	/* Get the structure memory */
	if (Input) {
		RedbookInit(Input);		/* Initialize the structure */
	}
	return Input;		/* Return the pointer or nothing */
}

/**********************************

	Dispose of a redbook audio structure

**********************************/

void BURGERCALL RedbookDelete(Redbook_t *Input)
{
	if (Input) {
		RedbookDestroy(Input);		/* Discard the contents */
		DeallocAPointer(Input);		/* Dispose of the pointer */
	}
}

/**********************************

	Convert Minutes, seconds and frames into a generic
	position

**********************************/

Word32 BURGERCALL RedbookMakePosition(Word Minutes,Word Seconds,Word Frames)
{
	Word32 Val;
	Val = Minutes*60;		/* Minutes mul by 60 */
	Val = (Val+Seconds)*FRAMESPERSEC;	/* Seconds to frames */
	return Val+Frames;		/* Return final result */
}

/**********************************

	Convert a position into a TMSF field for the CD rom
	Note : The data is returned as a longword

**********************************/

Word32 BURGERCALL RedbookMakeTMSF(Word Track,Word32 Position)
{
	union {
		Word8 c[4];		/* Big endian array */
		Word32 l;		/* Returned long word */
	} Result;
	Word Min,Sec;

	Min = Position/(FRAMESPERSEC*60);		/* Extract minutes */
	Position -= Min*(FRAMESPERSEC*60);
	Sec = Position/FRAMESPERSEC;		/* Extract seconds */
	Position -= Sec*FRAMESPERSEC;		/* Frame is the remainder */

	Result.c[0] = (Word8)Track;		/* Track entry */
	Result.c[1] = (Word8)Min;		/* Minute */
	Result.c[2] = (Word8)Sec;		/* Second */
	Result.c[3] = (Word8)Position;	/* Frame */

	return Result.l;	/* Return the long word */
}

/**********************************

	Play a simple redbook audio track
	Call a lower level routine to do the dirty work

**********************************/

void BURGERCALL RedbookPlay(Redbook_t *Input,Word Mode,Word Track)
{
	RedbookPlayRange(Input,Mode,Track,0,Track+1,0);
}


#if !defined(__MSDOS__) && !defined(__MAC__) && !defined(__WIN32__)

#include "ClStdLib.h"

/**********************************

	Initialize a redbook audio cd structure

**********************************/

Word BURGERCALL RedbookInit(Redbook_t *Input)
{
	FastMemSet(Input,0,sizeof(Redbook_t));
	return TRUE;		/* An error has occured!! */
}

/**********************************

	Shut down and delete the contents of a redbook audio cd structure

**********************************/

void BURGERCALL RedbookDestroy(Redbook_t *Input)
{
}

/**********************************

	Play a redbook audio track
	I will handle looping via in interrupt routine

**********************************/

void BURGERCALL RedbookPlayRange(Redbook_t *Input,Word Mode,Word StartTrack,Word32 StartPosition,Word EndTrack,Word32 EndPosition)
{
}

/**********************************

	Stop redbook audio

**********************************/

void BURGERCALL RedbookStop(Redbook_t *Input)
{
}

/**********************************

	Pause redbook audio

**********************************/

void BURGERCALL RedbookPause(Redbook_t *Input)
{
}

/**********************************

	Resume paused redbook audio

**********************************/

void BURGERCALL RedbookResume(Redbook_t *Input)
{
}

/**********************************

	Set the Audio CD's volume.
	Valid input is 0-255 (0=quiet, 255 = maximum)

**********************************/

void BURGERCALL RedbookSetVolume(Redbook_t *Input,Word Left,Word Right)
{
}

/**********************************

	Set a new loop start point

**********************************/

void BURGERCALL RedbookSetLoopStart(Redbook_t *Input,Word Track,Word32 Position)
{
}

/**********************************

	Set a new loop end point

**********************************/

void BURGERCALL RedbookSetLoopEnd(Redbook_t *Input,Word Track,Word32 Position)
{
}

/**********************************

	Open the CD drawer

**********************************/

void BURGERCALL RedbookOpenDrawer(Redbook_t *Input)
{
}

/**********************************

	Close the CD drawer

**********************************/

void BURGERCALL RedbookCloseDrawer(Redbook_t *Input)
{
}

/**********************************

	Return the current volume
	for the CD drive

**********************************/

void BURGERCALL RedbookGetVolume(Redbook_t *Input,Word *LeftPtr,Word *RightPtr)
{
	if (LeftPtr) {			/* Return the volume */
		LeftPtr[0] = 0;
	}
	if (RightPtr) {
		RightPtr[0] = 0;
	}
}

/**********************************

	Return the starting frame of a track in
	absolute frames

**********************************/

Word32 BURGERCALL RedbookGetTrackStart(Redbook_t *Input,Word TrackNum)
{
	return 0;		/* Track not found or error */
}

/**********************************

	Return the length of a track in minutes, second and frame

**********************************/

Word32 BURGERCALL RedbookGetTrackLength(Redbook_t *Input,Word TrackNum)
{
	return 0;		/* Track not found or error */
}

/**********************************

	Return the number of tracks contained in a CD

**********************************/

Word BURGERCALL RedbookGetTrackCount(Redbook_t *Input)
{
	return 0;		/* No tracks found (Error) */
}

/**********************************

	Return the current position of the CD in CD sectors

**********************************/

Word32 BURGERCALL RedbookGetPosition(Redbook_t *Input)
{
	return 0;		/* Track not found or error */
}

/**********************************

	Return the currently playing track

**********************************/

Word BURGERCALL RedbookGetCurrentTrack(Redbook_t *Input)
{
	return 0;		/* No tracks found (Error) */
}

/**********************************

	Return the current status of the CD audio driver

**********************************/

Word BURGERCALL RedbookGetPlayStatus(Redbook_t *Input)
{
	return REDBOOKMODESTOPPED;		/* No tracks found (Error) */
}

/**********************************

	Scan all the devices for a device with a CD
	with the proper name.

**********************************/

Word BURGERCALL RedbookLogCDByName(Redbook_t *Input,const char *VolumeName)
{
	return TRUE;		/* An error occured! */
}


#endif