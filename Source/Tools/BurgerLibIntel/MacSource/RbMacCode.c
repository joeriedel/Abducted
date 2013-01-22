#include "RbRedBook.h"

#if defined(__MAC__)
#include <Devices.h>
#include "InInput.h"
#include "ClStdLib.h"
#include "TkTick.h"
#include "FmFile.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
static void RedbookMyCode(void *Input);
static pascal void CallbackProc(ParmBlkPtr Input);
}
#endif

#if TARGET_RT_MAC_CFM

/* In POWERPC and 68K CFM, I can use "C" */

static pascal void CallbackProc(ParmBlkPtr Input)
{
	((Word8 *)Input)[80] = FALSE;
}
#else

/* For classic MacOS 68k, I am passed the pointer in register a0 */

static asm pascal void CallbackProc(register ParmBlkPtr Input)
{
	clr.b	80(a0)
	rts
}
#endif

/**********************************

	Find an empty async command buffer

**********************************/

static ParamBlockRec * BURGERCALL RedbookWaitNotBusy(Redbook_t *Input)
{
	Word i;
	Word8 *ParmPtr;
	for (;;) {
		i = 4;
		ParmPtr = &Input->ParamData[0];
		do {
			if (!ParmPtr[80]) {	/* Available? */
				FastMemSet(ParmPtr,0,sizeof(ParamBlockRec));	/* Blank the structure */
				ParmPtr[80] = TRUE;			/* Mark as busy */
				return (ParamBlockRec *)ParmPtr;				/* Return the pointer */
			}
			ParmPtr+=84;		/* Next one in the chain */
		} while (--i);			/* All scanned? */
		KeyboardKbhit();				/* Poll routines */
	}
}

/**********************************

	Wait for all the async commands to execute

**********************************/

static void BURGERCALL RedbookWait(Redbook_t *Input)
{
	Word i;
	Word StayHere;
	Word8 *ParmPtr;
	do {
		StayHere = FALSE;
		i = 4;
		ParmPtr = &Input->ParamData[0];
		do {
			if (ParmPtr[80]) {	/* Available? */
				StayHere = TRUE;
			}
			ParmPtr+=84;		/* Next one in the chain */
		} while (--i);			/* All scanned? */
		if (StayHere) {
			KeyboardKbhit();				/* Poll routines */
		}
	} while (StayHere);
}

/**********************************

	Initialize a redbook audio cd structure

**********************************/

Word BURGERCALL RedbookInit(Redbook_t *Input)
{
	FastMemSet(Input,0,sizeof(Redbook_t));

	/* MacOS version uses direct calls to the SCSI CD-ROM driver */
#if !TARGET_API_MAC_CARBON
	if (!MacOpenDriver("\p.AppleCD",&Input->OpenDeviceID)) {
		Input->ProcPtr = NewIOCompletionUPP((IOCompletionProcPtr)CallbackProc);		/* Make the callback */
		Input->Active = TRUE;		/* It's ok! */
//		Input->Paused = FALSE;
//		Input->Timer = FALSE;		/* No async timer present */
		RedbookGetTrackStart(Input,1);		/* Force the CD to get the TOC info */
		return FALSE;		/* It's cool! */
	}
#endif
	return TRUE;		/* An error has occured!! */
}

/**********************************

	Shut down and delete the contents of a redbook audio cd structure

**********************************/

void BURGERCALL RedbookDestroy(Redbook_t *Input)
{
	if (Input->Active) {	/* Valid input? */
		RedbookStop(Input);		/* Stop any audio that may be playing */
		RedbookWait(Input);		/* Wait until no longer busy */
		if (Input->ProcPtr) {
			DisposeIOCompletionUPP((IOCompletionUPP)Input->ProcPtr);	/* Get rid of the command */
			Input->ProcPtr = 0;
		}
		Input->Active = FALSE;	/* No longer active */
	}
}

/**********************************

	Mac OS callback proc
	This is a KeyboardKbhit background task that will monitor the
	CD to see if it has shut off, if so, I'll restart it.

**********************************/

static void RedbookMyCode(void *Input)
{
#if !TARGET_API_MAC_CARBON
	Word32 Foo;
	Foo = ReadTick();
	if ((Foo-((Redbook_t *)Input)->TimeMark)>(TICKSPERSEC/2)) {
		Word8 *ParmPtr;
		((Redbook_t *)Input)->TimeMark = Foo;
		
		/* Don't do ANYTHING if the CD drive is busy! */
		Foo = 4;
		ParmPtr = &((Redbook_t *)Input)->ParamData[0];
		do {
			if (ParmPtr[80]) {	/* Available? */
				return;
			}
			ParmPtr+=84;		/* Next one in the chain */
		} while (--Foo);		/* All scanned? */

		if (RedbookGetPlayStatus(static_cast<Redbook_t*>(Input))==REDBOOKMODESTOPPED) {
			ParamBlockRec *StatusPtr;
			StatusPtr = RedbookWaitNotBusy(static_cast<Redbook_t*>(Input));
			Foo = ((Redbook_t *)Input)->FromMark;
			StatusPtr->cntrlParam.ioCRefNum = ((Redbook_t *)Input)->OpenDeviceID;
			StatusPtr->cntrlParam.csCode = 103;		/* Search code the disk */
			StatusPtr->cntrlParam.csParam[0] = 0;		/* Seek track */
			((Word32 *)(&StatusPtr->cntrlParam.csParam[1]))[0] = Foo;
			StatusPtr->cntrlParam.csParam[3] = TRUE;
			StatusPtr->cntrlParam.csParam[4] = 0x9;		/* Proper stereo */
			StatusPtr->cntrlParam.ioCompletion = (IOCompletionUPP)((Redbook_t *)Input)->ProcPtr;
			PBControlAsync(StatusPtr);

			Foo = ((Redbook_t *)Input)->ToMark;		/* Stopping mark? */
			if (Foo) {					/* Yep, mark for stoppage */
				StatusPtr = RedbookWaitNotBusy(static_cast<Redbook_t*>(Input));
				StatusPtr->cntrlParam.ioCRefNum = ((Redbook_t *)Input)->OpenDeviceID;
				StatusPtr->cntrlParam.csCode = 106;		/* Stop at this position */
				StatusPtr->cntrlParam.csParam[0] = 0;		/* Seek track */
				((Word32 *)(&StatusPtr->cntrlParam.csParam[1]))[0] = Foo;
				StatusPtr->cntrlParam.ioCompletion = (IOCompletionUPP)((Redbook_t *)Input)->ProcPtr;
				PBControlAsync(StatusPtr);
			}
		}
	}
#endif
}

/**********************************

	Play a redbook audio track
	I will handle looping via in interrupt routine

**********************************/

void BURGERCALL RedbookPlayRange(Redbook_t *Input,Word Mode,Word StartTrack,Word32 StartPosition,Word EndTrack,Word32 EndPosition)
{
	if (Input->Active) {
		RedbookStop(Input);		/* Make sure I am turned off (Clears timer) */
		if (!StartTrack) {		/* Must start at track 1! */
			StartTrack=1;
		}
		Input->ToMark = 0;		/* Init the mark */
#if !TARGET_API_MAC_CARBON
		{
			ParamBlockRec *StatusPtr;
			Word32 Offset;

/* Due to the fact that Power Macintosh 4400's CD driver doesn't clear the STOP */
/* command after the CD has been forced to a stop (RedbookStop()), I need to */
/* Issue the STOP command BEFORE I issue the start command */
/* This prevents the CD from stopping instantly in playing sequential tracks */

			Offset = RedbookGetTrackStart(Input,1);
			StartPosition = ((RedbookGetTrackStart(Input,StartTrack)+StartPosition-Offset)<<2);

			if (Mode!=REDBOOKPLAY) {		/* Is there an end mark? */
				EndPosition = ((RedbookGetTrackStart(Input,EndTrack)+EndPosition-Offset)<<2);
				Input->ToMark = EndPosition;
			} else {
				EndPosition = (74*60*75)<<2;
			}
			StatusPtr = RedbookWaitNotBusy(Input);
			StatusPtr->cntrlParam.ioCRefNum = Input->OpenDeviceID;
			StatusPtr->cntrlParam.csCode = 106;		/* Stop at this position */
			StatusPtr->cntrlParam.csParam[0] = 0;		/* Seek track */
			StatusPtr->cntrlParam.csParam[1] = (Word16)(EndPosition>>16);
			StatusPtr->cntrlParam.csParam[2] =(Word16)EndPosition;
			StatusPtr->cntrlParam.ioCompletion = (IOCompletionUPP)Input->ProcPtr;
			PBControlAsync(StatusPtr);

			StatusPtr = RedbookWaitNotBusy(Input);
			StatusPtr->cntrlParam.ioCRefNum = Input->OpenDeviceID;
			StatusPtr->cntrlParam.csCode = 103;		/* Search code the disk */
			StatusPtr->cntrlParam.csParam[0] = 0;		/* Seek track */
			StatusPtr->cntrlParam.csParam[1] = (Word16)(StartPosition>>16);
			StatusPtr->cntrlParam.csParam[2] = (Word16)StartPosition;
			StatusPtr->cntrlParam.csParam[3] = TRUE;
			StatusPtr->cntrlParam.csParam[4] = 0x9;		/* Proper stereo */
			StatusPtr->cntrlParam.ioCompletion = (IOCompletionUPP)Input->ProcPtr;
			Input->FromMark = StartPosition;
			PBControlAsync(StatusPtr);

			if (Mode==REDBOOKPLAYLOOP) {
				Input->TimeMark = ReadTick();
				KeyboardAddRoutine(&RedbookMyCode,Input);
				Input->Timer = TRUE;
			}
		}
#endif
		Input->Paused = FALSE;		/* Not paused anymore */
	}
}

/**********************************

	Stop redbook audio

**********************************/

void BURGERCALL RedbookStop(Redbook_t *Input)
{
	if (Input->Active) {	/* Valid input? */
		if (Input->Timer) {
			KeyboardRemoveRoutine(&RedbookMyCode,Input);
			Input->Timer = FALSE;
		}
#if !TARGET_API_MAC_CARBON
		{
			ParamBlockRec *StatusPtr;
			StatusPtr = RedbookWaitNotBusy(Input);
			StatusPtr->cntrlParam.ioCRefNum = Input->OpenDeviceID;
			StatusPtr->cntrlParam.csCode = 106;		/* Stop the disk */
			StatusPtr->cntrlParam.ioCompletion = (IOCompletionUPP)Input->ProcPtr;
			PBControlAsync(StatusPtr);
		}
#endif
		Input->Paused = FALSE;		/* Not paused anymore! */
	}
}

/**********************************

	Pause redbook audio

**********************************/

void BURGERCALL RedbookPause(Redbook_t *Input)
{
	if (Input->Active && !Input->Paused) {	/* Valid input? */
#if !TARGET_API_MAC_CARBON
		ParamBlockRec *StatusPtr;
		StatusPtr = RedbookWaitNotBusy(Input);
		StatusPtr->cntrlParam.ioCRefNum = Input->OpenDeviceID;
		StatusPtr->cntrlParam.csCode = 105;		/* AudioPause command */
		StatusPtr->cntrlParam.csParam[1]=1;		/* Begin a pause */
		StatusPtr->cntrlParam.ioCompletion = (IOCompletionUPP)Input->ProcPtr;
		PBControlAsync(StatusPtr);		/* Issue the pause */
#endif
		Input->Paused = TRUE;
	}
}

/**********************************

	Resume paused redbook audio

**********************************/

void BURGERCALL RedbookResume(Redbook_t *Input)
{
	if (Input->Active && Input->Paused) {	/* Valid input? */
#if !TARGET_API_MAC_CARBON
		ParamBlockRec *StatusPtr;
		StatusPtr = RedbookWaitNotBusy(Input);
		StatusPtr->cntrlParam.ioCRefNum = Input->OpenDeviceID;
		StatusPtr->cntrlParam.csCode = 105;		/* AudioPause */
		StatusPtr->cntrlParam.csParam[1]=0;		/* End pause */
		StatusPtr->cntrlParam.ioCompletion = (IOCompletionUPP)Input->ProcPtr;
		PBControlAsync(StatusPtr);		/* Get the status */
#endif
		Input->Paused = FALSE;
	}
}

/**********************************

	Set the Audio CD's volume.
	Valid input is 0-255 (0=quiet, 255 = maximum)

**********************************/

void BURGERCALL RedbookSetVolume(Redbook_t *Input,Word Left,Word Right)
{
	if (Input->Active) {	/* Just check for the drive */
#if !TARGET_API_MAC_CARBON
		ParamBlockRec *StatusPtr;
		StatusPtr = RedbookWaitNotBusy(Input);
		StatusPtr->cntrlParam.ioCRefNum = Input->OpenDeviceID;
		StatusPtr->cntrlParam.csCode = 109;		/* Set Audio volume*/
		Right = (Left<<8)|Right;
		StatusPtr->cntrlParam.csParam[0] = Right;
		StatusPtr->cntrlParam.ioCompletion = (IOCompletionUPP)Input->ProcPtr;
		PBControlAsync(StatusPtr);		/* Issue the command */
#endif
	}
}

/**********************************

	Set a new loop start point

**********************************/

void BURGERCALL RedbookSetLoopStart(Redbook_t *Input,Word Track,Word32 Position)
{
	if (Input->Active) {		/* Valid structure */
		Input->FromMark = RedbookGetTrackStart(Input,Track)+Position;
	}
}


/**********************************

	Set a new loop end point

**********************************/

void BURGERCALL RedbookSetLoopEnd(Redbook_t *Input,Word Track,Word32 Position)
{
	if (Input->Active) {		/* Valid structure */
		Word32 Mark;
		Mark = RedbookGetTrackStart(Input,Track);		/* Get the beginning of the track */
		if (Mark) {
			Input->ToMark = Mark+Position;		/* Save the mark */
			return;
		}
		Mark = RedbookGetTrackStart(Input,Track-1);	/* Get the FINAL track */
		if (Mark) {
			Input->ToMark = RedbookGetTrackLength(Input,Track-1)+Mark;	/* Ignore position */
		}
	}
}

/**********************************

	Open the CD drawer

**********************************/

void BURGERCALL RedbookOpenDrawer(Redbook_t *Input)
{
	if (Input->Active) {	/* Valid input? */
		RedbookStop(Input);		/* Stop any audio output */
	}
}

/**********************************

	Close the CD drawer

**********************************/

void BURGERCALL RedbookCloseDrawer(Redbook_t *Input)
{
	if (Input->Active) {	/* Valid input? */
	}
}


/**********************************

	Return the current volume
	for the CD drive

**********************************/

void BURGERCALL RedbookGetVolume(Redbook_t *Input,Word *LeftPtr,Word *RightPtr)
{
	Word Left,Right;
	Left = 0;		/* Init the volume */
	Right = 0;
	
#if !TARGET_API_MAC_CARBON
	if (Input->Active) {	/* Just check for the drive */
		ParamBlockRec Status;
		FastMemSet(&Status,0,sizeof(Status));
		Status.cntrlParam.ioCRefNum = Input->OpenDeviceID;
		Status.cntrlParam.csCode = 112;		/* Read Audio volume*/
		if (!PBControlSync(&Status)) {		/* Did it succeed? */
			Left = (Status.cntrlParam.csParam[0]>>8)&0xFF;
			Right = Status.cntrlParam.csParam[0]&0xFF;
		}
	}
#endif
	if (LeftPtr) {			/* Return the volume */
		LeftPtr[0] = Left;
	}
	if (RightPtr) {
		RightPtr[0] = Right;
	}
}

/**********************************

	Return the starting frame of a track in
	absolute frames

**********************************/

Word32 BURGERCALL RedbookGetTrackStart(Redbook_t *Input,Word TrackNum)
{
#if !TARGET_API_MAC_CARBON
	if (Input->Active) {
		ParamBlockRec Status;
		Word32 StartPos;		/* Marks for the length of a track */
		Word8 Buffer[4];			/* Temp buffer for the track data */

		FastMemSet(&Status,0,sizeof(Status));
		((Word32 *)Buffer)[0] = 0;		/* Zap the buffer */

		StartPos = (Word32)&Buffer[0];		/* Get the pointer to a longword */
		Status.cntrlParam.ioCRefNum = Input->OpenDeviceID;
		Status.cntrlParam.csCode = 100;		/* ReadTOC */
		Status.cntrlParam.csParam[0] = 3;		/* Get the starting address of two tracks */
		((Word32 *)(&Status.cntrlParam.csParam[1]))[0] = StartPos;
		Status.cntrlParam.csParam[3] = 4;		/* Buffer is 4 bytes in size */
		Status.cntrlParam.csParam[4] = NumToBCD(TrackNum)<<8; /* Get the track requested */
		if (!PBControlSync(&Status)) {		/* Read it */
			StartPos = RedbookMakePosition(BCDToNum(Buffer[1]),BCDToNum(Buffer[2]),BCDToNum(Buffer[3]));
			if (StartPos) {		/* Proper value? */
				return StartPos;
			}
		}
		FastMemSet(&Status,0,sizeof(Status));	/* Get the leadout (For the last track ending) */
		Status.cntrlParam.ioCRefNum = Input->OpenDeviceID;
		Status.cntrlParam.csCode = 100;		/* ReadTOC */
		Status.cntrlParam.csParam[0] = 2;		/* Lead out area */
		if (!PBControlSync(&Status)) {		/* Get lead out starting address */
			return RedbookMakePosition(BCDToNum((Status.cntrlParam.csParam[0]>>8)&0xFF),BCDToNum(Status.cntrlParam.csParam[0]&0xFF),BCDToNum((Status.cntrlParam.csParam[1]>>8)&0xFF));
		}
	}
#endif
	return 0;		/* Track not found or error */
}

/**********************************

	Return the length of a track in minutes, second and frame

**********************************/

Word32 BURGERCALL RedbookGetTrackLength(Redbook_t *Input,Word TrackNum)
{
#if !TARGET_API_MAC_CARBON
	if (Input->Active) {
		ParamBlockRec Status;
		Word32 StartPos,EndPos;		/* Marks for the length of a track */
		Word8 Buffer[8];					/* Temp buffer for the track data */
		
		((Word32 *)Buffer)[0] = 0;	/* Zap the buffer */
		((Word32 *)Buffer)[1] = 0;
		FastMemSet(&Status,0,sizeof(Status));
		StartPos = (Word32)&Buffer[0];		/* Get the pointer to a longword */
		Status.cntrlParam.ioCRefNum = Input->OpenDeviceID;
		Status.cntrlParam.csCode = 100;		/* ReadTOC */
		Status.cntrlParam.csParam[0] = 3;		/* Get the starting address of two tracks */
		((Word32 *)(&Status.cntrlParam.csParam[1]))[0] = StartPos;
		Status.cntrlParam.csParam[3] = 8;		/* Buffer is 8 bytes in size */
		Status.cntrlParam.csParam[4] = NumToBCD(TrackNum)<<8; /* Get the track requested */
		if (!PBControlSync(&Status)) {		/* Read it */
			StartPos = RedbookMakePosition(BCDToNum(Buffer[1]),BCDToNum(Buffer[2]),BCDToNum(Buffer[3]));
			EndPos = RedbookMakePosition(BCDToNum(Buffer[5]),BCDToNum(Buffer[6]),BCDToNum(Buffer[7]));
			if (!EndPos) {			/* Last track requested? */
				FastMemSet(&Status,0,sizeof(Status));	/* Get the leadout (For the last track ending) */
				Status.cntrlParam.ioCRefNum = Input->OpenDeviceID;
				Status.cntrlParam.csCode = 100;		/* ReadTOC */
				Status.cntrlParam.csParam[0] = 2;		/* Lead out area */
				EndPos = StartPos;						/* Assume failure */
				if (!PBControlSync(&Status)) {		/* Get lead out starting address */
					EndPos = RedbookMakePosition(BCDToNum((Status.cntrlParam.csParam[0]>>8)&0xFF),BCDToNum(Status.cntrlParam.csParam[0]&0xFF),BCDToNum((Status.cntrlParam.csParam[1]>>8)&0xFF));
				}
			}
			return EndPos-StartPos;		/* Return the length in frames */
		}
	}
#endif
	return 0;		/* Track not found or error */
}

/**********************************

	Return the number of tracks contained in a CD

**********************************/

Word BURGERCALL RedbookGetTrackCount(Redbook_t *Input)
{
#if !TARGET_API_MAC_CARBON
	if (Input->Active) {
		ParamBlockRec Status;
		FastMemSet(&Status,0,sizeof(Status));
		Status.cntrlParam.ioCRefNum = Input->OpenDeviceID;
		Status.cntrlParam.csCode = 100;
		Status.cntrlParam.csParam[0] = 1;		/* Get first and track count info */
		if (!PBControlSync(&Status)) {
			return BCDToNum(Status.cntrlParam.csParam[0]&0xFF);
		}
	}
#endif
	return 0;		/* No tracks found (Error) */
}

/**********************************

	Return the current position of the CD in CD sectors

**********************************/

Word32 BURGERCALL RedbookGetPosition(Redbook_t *Input)
{
#if !TARGET_API_MAC_CARBON
	if (Input->Active) {
		ParamBlockRec Status;
		FastMemSet(&Status,0,sizeof(Status));
		Status.cntrlParam.ioCRefNum = Input->OpenDeviceID;
		Status.cntrlParam.csCode = 101;		/* Read the Q subcode */
		if (!PBControlSync(&Status)) {		/* Did it succeed? */
			return RedbookMakePosition(BCDToNum(Status.cntrlParam.csParam[1]&0xFF),
				BCDToNum((Status.cntrlParam.csParam[2]>>8)&0xFF),
				BCDToNum(Status.cntrlParam.csParam[2]&0xFF));
		}
	}
#endif
	return 0;		/* Track not found or error */
}

/**********************************

	Return the currently playing track

**********************************/

Word BURGERCALL RedbookGetCurrentTrack(Redbook_t *Input)
{
#if !TARGET_API_MAC_CARBON
	if (Input->Active) {
		ParamBlockRec Status;
		FastMemSet(&Status,0,sizeof(Status));
		Status.cntrlParam.ioCRefNum = Input->OpenDeviceID;
		Status.cntrlParam.csCode = 101;		/* Read Q-Subcode information */
		if (!PBControlSync(&Status)) {
			return BCDToNum(Status.cntrlParam.csParam[0]&0xFF);
		}
	}
#endif
	return 0;		/* No tracks found (Error) */
}

/**********************************

	Return the current status of the CD audio driver

**********************************/

Word BURGERCALL RedbookGetPlayStatus(Redbook_t *Input)
{
#if !TARGET_API_MAC_CARBON
	if (Input->Active) {		/* Is it active? */
		ParamBlockRec Status;
		FastMemSet(&Status,0,sizeof(Status));
		Status.cntrlParam.ioCRefNum = Input->OpenDeviceID;
		Status.cntrlParam.csCode = 107;		/* Stop the disk */
		if (!PBControlSync(&Status)) {		/* Get the status */
			Word Temp;
			Temp = (Status.cntrlParam.csParam[0]>>8)&0xff;		/* Get the status bits */
			switch (Temp) {
			case 0:		/* Play in progress */
			case 3:		/* Muted */
				return REDBOOKMODEPLAYING;
			case 1:		/* Pause? */
				return REDBOOKMODEPAUSED;
//			case 2:		/* Play operation completed */
//			case 4:		/* Error condition */
			case 5:		/* Shut down */
				return REDBOOKMODESTOPPED;
			}
		}

	}
#endif
	return REDBOOKMODESTOPPED;		/* No tracks found (Error) */
}

/**********************************

	Scan all the devices for a device with a CD
	with the proper name.

**********************************/

Word BURGERCALL RedbookLogCDByName(Redbook_t *Input,const char *VolumeName)
{
	Word VolumeNumber;
	if (Input->Active) {		/* Valid structure? */
		VolumeNumber = FindAVolumeByName(VolumeName);		/* Is the CD logged? */
		if (VolumeNumber!=-1) {		/* Valid? */
			Str255 volumeName;
			HParamBlockRec pb;
			pb.volumeParam.ioCompletion = 0;
			pb.volumeParam.ioNamePtr = (StringPtr) volumeName;
			pb.volumeParam.ioVRefNum = -1-VolumeNumber;
			pb.volumeParam.ioVolIndex = 0;
			if (!PBHGetVInfoSync(&pb)) {		/* Get Volume Info */
				Input->OpenDeviceID = pb.volumeParam.ioVDRefNum;
				return FALSE;
			}
		}
	}
	return TRUE;		/* An error occured! */
}


#endif