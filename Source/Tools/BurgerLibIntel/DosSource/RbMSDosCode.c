#include "RbRedBook.h"

#if defined(__MSDOS__)
#include "MsDos.h"
#include "ClStdLib.h"
#include "TkTick.h"
#include "InInput.h"
#include "FmFile.h"

typedef struct {			/* MSCDEX command blocks */
	Word8 length;			/* Size of the ioctlo_t */
	Word8 subunit;			/* CD sub unit to access */
	Word8 comcode;			/* Command to issue (12 = WRITE) */
	Word16 status;			/* Error code returned */
	Word8 unused[8];			/* Not used */
	Word8 AddressMode; 		/* 0 for HSG mode */
	Word32 FromMark;		/* Starting sector to play from */
	Word32 ToMark;		/* Ending sector to play to */
} Play_t;

typedef struct {			/* MSCDEX command blocks */
	Word8 length;			/* Size of the ioctlo_t */
	Word8 subunit;			/* CD sub unit to access */
	Word8 comcode;			/* Command to issue (12 = WRITE) */
	Word16 status;			/* Error code returned */
	Word8 unused[8];			/* Not used */
	Word8 media;				/* Media descriptor byte (0) */
	Word32 address;		/* Pointer to sub record (Real memory) */
	Word16 bytes;			/* Number of bytes in the sub record (1 for eject) */
	Word16 sector;			/* Starting MS-DOS sector number */
	Word32 volid;			/* Returned CD volume ID if changed disk */
} ioctlo_t;

typedef struct {
	Word8 Command;			/* (4) for GetVolume */
	Word8 Input0;			/* 0-3 for output channel 0 */
	Word8 Volume0;			/* 0-255 for channel 0 volume */
	Word8 Input1;			/* 0-3 for output channel 1 */
	Word8 Volume1;			/* 0-255 for channel 1 volume */
	Word8 Input2;			/* 0-3 for output channel 2 */
	Word8 Volume2;			/* 0-255 for channel 2 volume */
	Word8 Input3;			/* 0-3 for output channel 3 */
	Word8 Volume3;			/* 0-255 for channel 3 volume */
} AudioInfo_t;

typedef struct {
	Word8 Command;		/* (10) for AUDIODISKINFO */
	Word8 LowestTrack;	/* Lowest track number */
	Word8 HighestTrack;	/* Highest track number */
	Word8 Frame;			/* Starting point for track */
	Word8 Second;		/* In HSG format */
	Word8 Minute;
	Word8 Unused;
} DiskInfo_t;

typedef struct {
	Word8 Command;			/* (10) for AUDIODISKINFO */
	Word8 LowestTrack;		/* Lowest track number */
	Word8 HighestTrack;		/* Highest track number */
	Word32 TrackLeadout;	/* Starting point for leadout */
} DiskInfo2_t;

typedef struct {
	Word8 Command;		/* (11) for TRACKINFO */
	Word8 Track;			/* Track number to get info from */
	Word8 Frame;			/* Starting point for track */
	Word8 Second;		/* In HSG format */
	Word8 Minute;
	Word8 Unused;
	Word8 Control;		/* Track control type */
} TrackInfo_t;

typedef struct {
	Word8 Command;		/* (12) for QINFO */
	Word8 Control;		/* Track control type */
	Word8 Track;			/* Track number to get info from */
	Word8 Index;
	Word8 RMinute;
	Word8 RSecond;		/* In HSG format */
	Word8 RFrame;		/* Running time on the track */
	Word8 Unused;
	Word8 Minute;
	Word8 Second;		/* In HSG format */
	Word8 Frame;			/* Running time of the disk */
} QInfo_t;

typedef struct {
	Word8 Command;			/* (0) for EJECT */
} EjectControl_t;

typedef struct {
	Word8 Command;			/* (1) for LOCK/UNLOCK door */
	Word8 LockFunction;		/* 0 for UNLOCK, 1 for LOCK */
} UnlockControl_t;

typedef struct {
	Word8 Command;			/* (5) for CLOSETRAY */
} CloseTrayControl_t;

typedef struct {
	Word8 Command;		/* (12) for QINFO */
	Word32 Status;	/* CD Status flags */
} StatusInfo_t;

/**********************************

	Initialize a redbook audio cd structure

**********************************/

Word BURGERCALL RedbookInit(Redbook_t *Input)
{
	FastMemSet(Input,0,sizeof(Redbook_t));

	/* DOS version calls MSCDEX */
	{
		Regs16 MyRegs;
		MyRegs.ax = 0x1500;	/* Get installed state (MSCDEX = 0x15) */
		MyRegs.bx = 0;		/* Init bx in case the call fails */
		Int86x(0x2F,&MyRegs,&MyRegs);	/* Call a multiplexed interrupt */
		if (MyRegs.bx) {		/* Are there any drives? */
			Input->OpenDeviceID = MyRegs.cx;	/* Save the base drive */
			Input->Active = TRUE;		/* I am active! */
//			Input->Paused = FALSE;
//			Input->Timer = FALSE;
			return FALSE;
		}
	}
	return TRUE;		/* An error has occured!! */
}

/**********************************

	Shut down and delete the contents of a redbook audio cd structure

**********************************/

void BURGERCALL RedbookDestroy(Redbook_t *Input)
{
	if (Input->Active) {	/* Valid input? */
		RedbookStop(Input);		/* Stop any audio that may be playing */
		Input->Active = FALSE;	/* No longer active */
	}
}

/**********************************

	DOS callback proc
	This is a KeyboardKbhit background task that will monitor the
	CD to see if it has shut off, if so, I'll restart it.

**********************************/

static void RedbookMyCode(void *Input)
{
	Word32 Foo;
	Foo = ReadTick();		/* Has enough time passed? */
	if ((Foo-((Redbook_t *)Input)->TimeMark)>(TICKSPERSEC/2)) {
		((Redbook_t *)Input)->TimeMark = Foo;
		if (RedbookGetPlayStatus((Redbook_t *)Input)==REDBOOKMODESTOPPED) {
			Regs16 MyRegs;
			Word32 RealTray;
			Play_t *PlayPtr;

		 	PlayPtr = (Play_t *)GetRealBufferProtectedPtr();
			RealTray = GetRealBufferPtr();		/* Real mode pointer */
			FastMemSet(PlayPtr,0,sizeof(Play_t));
			PlayPtr->length = sizeof(Play_t);	/* Save ioctlo_t length */
			PlayPtr->comcode = 132;		/* PLAY command */
			PlayPtr->AddressMode = 0;		/* HSG addressing */
			PlayPtr->FromMark = ((Redbook_t *)Input)->FromMark-150;
			PlayPtr->ToMark = ((Redbook_t *)Input)->ToMark-((Redbook_t *)Input)->FromMark;
			MyRegs.ax = 0x1510;		/* Send Command via multiplexed interrupt */
			MyRegs.cx = static_cast<Word16>(((Redbook_t *)Input)->OpenDeviceID);	/* CD unit number */
			MyRegs.bx = static_cast<Word16>(RealTray);		/* Real pointer */
			MyRegs.es = static_cast<Word16>(RealTray>>16);
			Int86x(0x2F,&MyRegs,&MyRegs);		/* Issue the call */
		}
	}
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
		{
			Regs16 MyRegs;
			Word32 RealTray;
			Play_t *PlayPtr;
			Word Temp;
			Word32 EndBlock;

			Temp = RedbookGetTrackCount(Input);	/* Number of tracks on the CD */
			EndBlock = RedbookGetTrackStart(Input,Temp) +	/* Get the FINAL block on the CD */
				RedbookGetTrackLength(Input,Temp);

			Input->FromMark = RedbookGetTrackStart(Input,StartTrack)+StartPosition;
			if (Mode==REDBOOKPLAY || (EndTrack>Temp)) {
				Input->ToMark = EndBlock;
			} else {
				Input->ToMark = RedbookGetTrackStart(Input,EndTrack)+EndPosition;
			}

		 	PlayPtr = (Play_t *)GetRealBufferProtectedPtr();
			RealTray = GetRealBufferPtr();		/* Real mode pointer */
			FastMemSet(PlayPtr,0,sizeof(Play_t));
			PlayPtr->length = sizeof(Play_t);	/* Save ioctlo_t length */
			PlayPtr->comcode = 132;		/* PLAY command */
			PlayPtr->AddressMode = 0;		/* HSG addressing */
			PlayPtr->FromMark = Input->FromMark-150;
			PlayPtr->ToMark = Input->ToMark-Input->FromMark;
			MyRegs.ax = 0x1510;		/* Send Command via multiplexed interrupt */
			MyRegs.cx = static_cast<Word16>(Input->OpenDeviceID);	/* CD unit number */
			MyRegs.bx = static_cast<Word16>(RealTray);		/* Real pointer */
			MyRegs.es = static_cast<Word16>(RealTray>>16);
			Int86x(0x2F,&MyRegs,&MyRegs);		/* Issue the call */
			if (Mode==REDBOOKPLAYLOOP) {
				Input->TimeMark = ReadTick();	/* Get the current timer */
				KeyboardAddRoutine(&RedbookMyCode,Input);
				Input->Timer = TRUE;		/* Timer is active */
			}
		}
		Input->Paused = FALSE;		/* Not paused anymore */
	}
}

/**********************************

	Stop redbook audio

**********************************/

void BURGERCALL RedbookStop(Redbook_t *Input)
{
	if (Input->Active) {	/* Valid input? */
		Regs16 MyRegs;
		Word32 RealTray;
		ioctlo_t *IoPtr;

		if (Input->Timer) {
			KeyboardRemoveRoutine(&RedbookMyCode,Input);
			Input->Timer = FALSE;
		}

		IoPtr = (ioctlo_t *)GetRealBufferProtectedPtr();
		RealTray = GetRealBufferPtr();		/* Real mode pointer */

			/* Now I eject the CD */

		FastMemSet(IoPtr,0,sizeof(ioctlo_t));
		IoPtr->length = sizeof(ioctlo_t);	/* Save ioctlo_t length */
		IoPtr->comcode = 133;		/* STOP command */
//		IoPtr->address = 0;
//		IoPtr->bytes = 0;
		MyRegs.ax = 0x1510;		/* Send Command via multiplexed interrupt */
		MyRegs.cx = static_cast<Word16>(Input->OpenDeviceID);	/* CD unit number */
		MyRegs.bx = static_cast<Word16>(RealTray);		/* Real pointer */
		MyRegs.es = static_cast<Word16>(RealTray>>16);
		Int86x(0x2F,&MyRegs,&MyRegs);		/* Issue the call */
		Input->Paused = FALSE;		/* Not paused anymore! */
	}
}

/**********************************

	Pause redbook audio

**********************************/

void BURGERCALL RedbookPause(Redbook_t *Input)
{
	if (Input->Active && !Input->Paused) {	/* Valid input? */
		if (RedbookGetPlayStatus(Input)==REDBOOKMODEPLAYING) {
			Bool Temp;
			Temp = Input->Timer;
			Input->Timer = FALSE;		/* Don't remove the timer if present */
			RedbookStop(Input);		/* Stop the CD */
			Input->Timer = Temp;	/* Restore the timer flag */
			Input->Paused=TRUE;		/* But mark as paused! */
		}
	}
}


/**********************************

	Resume paused redbook audio

**********************************/

void BURGERCALL RedbookResume(Redbook_t *Input)
{
	if (Input->Active && Input->Paused) {	/* Valid input? */
		Regs16 MyRegs;
		Word32 RealTray;
		ioctlo_t *IoPtr;

		IoPtr = (ioctlo_t *)GetRealBufferProtectedPtr();
		RealTray = GetRealBufferPtr();		/* Real mode pointer */

			/* Now I eject the CD */

		FastMemSet(IoPtr,0,sizeof(ioctlo_t));
		IoPtr->length = sizeof(ioctlo_t);	/* Save ioctlo_t length */
		IoPtr->comcode = 136;		/* RESUME command */
//		IoPtr->address = 0;
//		IoPtr->bytes = 0;
		MyRegs.ax = 0x1510;		/* Send Command via multiplexed interrupt */
		MyRegs.cx = static_cast<Word16>(Input->OpenDeviceID);	/* CD unit number */
		MyRegs.bx = static_cast<Word16>(RealTray);		/* Real pointer */
		MyRegs.es = static_cast<Word16>(RealTray>>16);
		Int86x(0x2F,&MyRegs,&MyRegs);		/* Issue the call */
		Input->Paused = FALSE;
	}
}

/**********************************

	Set the Audio CD's volume.
	Valid input is 0-255 (0=quiet, 255 = maximum)

**********************************/

void BURGERCALL RedbookSetVolume(Redbook_t *Input,Word Left,Word Right)
{
	if (Input->Active) {
		Regs16 MyRegs;
		Word32 RealTray;
		ioctlo_t *IoPtr;
		AudioInfo_t *AudioPtr;

		IoPtr = (ioctlo_t *)GetRealBufferProtectedPtr();
		RealTray = GetRealBufferPtr();		/* Real mode pointer */

		FastMemSet(IoPtr,0,sizeof(ioctlo_t));
		IoPtr->length = sizeof(ioctlo_t);	/* Save ioctlo_t length */
		IoPtr->comcode = 12;		/* WRITE command */
		IoPtr->address = RealTray+sizeof(ioctlo_t);
		IoPtr->bytes = sizeof(AudioInfo_t);
		AudioPtr = (AudioInfo_t *)(IoPtr+1);
		AudioPtr->Command = 0x03;		/* Command code (GETAUDIOINFO) */
		AudioPtr->Volume0 = (Word8)Left;
		AudioPtr->Volume2 = (Word8)Left;
		AudioPtr->Volume1 = (Word8)Right;
		AudioPtr->Volume3 = (Word8)Right;
		AudioPtr->Input0 = 0;
		AudioPtr->Input1 = 1;
		AudioPtr->Input2 = 2;
		AudioPtr->Input3 = 3;
		MyRegs.ax = 0x1510;		/* Send Command via multiplexed interrupt */
		MyRegs.cx = static_cast<Word16>(Input->OpenDeviceID);	/* CD unit number */
		MyRegs.bx = static_cast<Word16>(RealTray);		/* Real pointer */
		MyRegs.es = static_cast<Word16>(RealTray>>16);
		Int86x(0x2F,&MyRegs,&MyRegs);		/* Issue the call */
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
		{
			Regs16 MyRegs;
			Word32 RealTray;
			ioctlo_t *IoPtr;

			IoPtr = (ioctlo_t *)GetRealBufferProtectedPtr();
			RealTray = GetRealBufferPtr();		/* Real mode pointer */

			/* First I need to unlock the CD door */

			FastMemSet(IoPtr,0,sizeof(ioctlo_t)+sizeof(UnlockControl_t));
			IoPtr->length = sizeof(ioctlo_t);	/* Save ioctlo_t length */
			IoPtr->comcode = 12;		/* WRITE command */
			IoPtr->address = RealTray+sizeof(ioctlo_t);
			IoPtr->bytes = sizeof(UnlockControl_t);
			{
				UnlockControl_t *UnlockPtr;
				UnlockPtr = (UnlockControl_t *)(IoPtr+1);
				UnlockPtr->Command = 0x01;		/* Command code (EJECT Disk) */
//				UnlockPtr->LockFunction = 0;	/* Unlock the door */
			}
			MyRegs.ax = 0x1510;		/* Send Command via multiplexed interrupt */
			MyRegs.cx = static_cast<Word16>(Input->OpenDeviceID);	/* CD unit number */
			MyRegs.bx = static_cast<Word16>(RealTray);		/* Real pointer */
			MyRegs.es = static_cast<Word16>(RealTray>>16);
			Int86x(0x2F,&MyRegs,&MyRegs);		/* Issue the call */

			/* Now I eject the CD */

			FastMemSet(IoPtr,0,sizeof(ioctlo_t)+sizeof(EjectControl_t));
			IoPtr->length = sizeof(ioctlo_t);	/* Save ioctlo_t length */
			IoPtr->comcode = 12;		/* WRITE command */
			IoPtr->address = RealTray+sizeof(ioctlo_t);
			IoPtr->bytes = sizeof(EjectControl_t);
//			{
//				EjectControl_t *EjectPtr;
//				EjectPtr = (EjectControl_t *)(IoPtr+1);
//				EjectPtr->Command = 0x00;		/* Command code (EJECT Disk) */
//			}
			MyRegs.ax = 0x1510;		/* Send Command via multiplexed interrupt */
			MyRegs.cx = static_cast<Word16>(Input->OpenDeviceID);	/* CD unit number */
			MyRegs.bx = static_cast<Word16>(RealTray);		/* Real pointer */
			MyRegs.es = static_cast<Word16>(RealTray>>16);
			Int86x(0x2F,&MyRegs,&MyRegs);		/* Issue the call */
		}
	}
}

/**********************************

	Close the CD drawer

**********************************/

void BURGERCALL RedbookCloseDrawer(Redbook_t *Input)
{
	if (Input->Active) {	/* Valid input? */
		Regs16 MyRegs;
		Word32 RealTray;
		ioctlo_t *IoPtr;

	 	IoPtr = (ioctlo_t *)GetRealBufferProtectedPtr();
		RealTray = GetRealBufferPtr();		/* Real mode pointer */

		/* Now I eject the CD */

		FastMemSet(IoPtr,0,sizeof(ioctlo_t));
		IoPtr->length = sizeof(ioctlo_t);	/* Save ioctlo_t length */
		IoPtr->comcode = 12;		/* WRITE command */
		IoPtr->address = RealTray+sizeof(ioctlo_t);
		IoPtr->bytes = sizeof(CloseTrayControl_t);
		{
			CloseTrayControl_t *ClosePtr;
			ClosePtr = (CloseTrayControl_t *)(IoPtr+1);
			ClosePtr->Command = 0x05;		/* Command code (CLOSETRAY) */
		}
		MyRegs.ax = 0x1510;		/* Send Command via multiplexed interrupt */
		MyRegs.cx = static_cast<Word16>(Input->OpenDeviceID);	/* CD unit number */
		MyRegs.bx = static_cast<Word16>(RealTray);		/* Real pointer */
		MyRegs.es = static_cast<Word16>(RealTray>>16);
		Int86x(0x2F,&MyRegs,&MyRegs);		/* Issue the call */
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
	if (Input->Active) {
		Regs16 MyRegs;
		Word32 RealTray;
		ioctlo_t *IoPtr;
		AudioInfo_t *AudioPtr;

		IoPtr = (ioctlo_t *)GetRealBufferProtectedPtr();
		RealTray = GetRealBufferPtr();		/* Real mode pointer */

		FastMemSet(IoPtr,0,sizeof(ioctlo_t)+sizeof(AudioInfo_t));
		IoPtr->length = sizeof(ioctlo_t);	/* Save ioctlo_t length */
		IoPtr->comcode = 3;		/* WRITE command */
		IoPtr->address = RealTray+sizeof(ioctlo_t);
		IoPtr->bytes = sizeof(AudioInfo_t);
		AudioPtr = (AudioInfo_t *)(IoPtr+1);
		AudioPtr->Command = 0x04;		/* Command code (GETAUDIOINFO) */
		MyRegs.ax = 0x1510;		/* Send Command via multiplexed interrupt */
		MyRegs.cx = static_cast<Word16>(Input->OpenDeviceID);	/* CD unit number */
		MyRegs.bx = static_cast<Word16>(RealTray);		/* Real pointer */
		MyRegs.es = static_cast<Word16>(RealTray>>16);
		Int86x(0x2F,&MyRegs,&MyRegs);		/* Issue the call */
		if (!(IoPtr->status&0x8000)) {		/* Error? */
			Left = AudioPtr->Volume0;		/* Save the volumes */
			Right = AudioPtr->Volume1;
		}
	}

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
	if (Input->Active) {
		Regs16 MyRegs;
		Word32 RealTray;
		ioctlo_t *IoPtr;
		TrackInfo_t *InfoPtr;

		IoPtr = (ioctlo_t *)GetRealBufferProtectedPtr();
		RealTray = GetRealBufferPtr();		/* Real mode pointer */

		FastMemSet(IoPtr,0,sizeof(ioctlo_t)+sizeof(TrackInfo_t));
		IoPtr->length = sizeof(ioctlo_t);	/* Save ioctlo_t length */
		IoPtr->comcode = 3;				/* READ command */
		IoPtr->address = RealTray+sizeof(ioctlo_t);
		IoPtr->bytes = sizeof(TrackInfo_t);
		InfoPtr = (TrackInfo_t *)(IoPtr+1);
		InfoPtr->Command = 0x0B;		/* Command code (GETTRACKINFO) */
		InfoPtr->Track = static_cast<Word8>(TrackNum);
		MyRegs.ax = 0x1510;				/* Send Command via multiplexed interrupt */
		MyRegs.cx = static_cast<Word16>(Input->OpenDeviceID);	/* CD unit number */
		MyRegs.bx = static_cast<Word16>(RealTray);		/* Real pointer */
		MyRegs.es = static_cast<Word16>(RealTray>>16);
		Int86x(0x2F,&MyRegs,&MyRegs);	/* Issue the call */
		if (!(IoPtr->status&0x8000)) {	/* Error? */
			return RedbookMakePosition(InfoPtr->Minute,InfoPtr->Second,InfoPtr->Frame);		/* Get the place */
		}
	}
	return 0;		/* Track not found or error */
}

/**********************************

	Return the length of a track in minutes, second and frame

**********************************/

Word32 BURGERCALL RedbookGetTrackLength(Redbook_t *Input,Word TrackNum)
{
	if (Input->Active) {
		Regs16 MyRegs;
		Word32 RealTray;
		ioctlo_t *IoPtr;
		DiskInfo_t *InfoPtr;
		Word32 StartPos,EndPos;

		StartPos = RedbookGetTrackStart(Input,TrackNum);
		EndPos = RedbookGetTrackStart(Input,TrackNum+1);
		if (EndPos) {		/* Is the ending mark valid? */
			return EndPos-StartPos;		/* The difference is the length */
		}
		IoPtr = (ioctlo_t *)GetRealBufferProtectedPtr();
		RealTray = GetRealBufferPtr();	/* Real mode pointer */

		FastMemSet(IoPtr,0,sizeof(ioctlo_t)+sizeof(DiskInfo_t));
		IoPtr->length = sizeof(ioctlo_t);		/* Save ioctlo_t length */
		IoPtr->comcode = 3;				/* READ command */
		IoPtr->address = RealTray+sizeof(ioctlo_t);
		IoPtr->bytes = sizeof(DiskInfo_t);
		InfoPtr = (DiskInfo_t *)(IoPtr+1);
		InfoPtr->Command = 0x0A;		/* Command code (GETTRACKINFO) */
		MyRegs.ax = 0x1510;				/* Send Command via multiplexed interrupt */
		MyRegs.cx = static_cast<Word16>(Input->OpenDeviceID);	/* CD unit number */
		MyRegs.bx = static_cast<Word16>(RealTray);		/* Real pointer */
		MyRegs.es = static_cast<Word16>(RealTray>>16);
		Int86x(0x2F,&MyRegs,&MyRegs);		/* Issue the call */
		if (!(IoPtr->status&0x8000)) {		/* Error? */
			return RedbookMakePosition(InfoPtr->Minute,
				InfoPtr->Second,InfoPtr->Frame)-StartPos;	/* Get the place */
		}
	}
	return 0;		/* Track not found or error */
}


/**********************************

	Return the number of tracks contained in a CD

**********************************/

Word BURGERCALL RedbookGetTrackCount(Redbook_t *Input)
{
	if (Input->Active) {
		Regs16 MyRegs;
		Word32 RealTray;
		ioctlo_t *IoPtr;
		DiskInfo2_t *InfoPtr;

		IoPtr = (ioctlo_t *)GetRealBufferProtectedPtr();
		RealTray = GetRealBufferPtr();		/* Real mode pointer */

		FastMemSet(IoPtr,0,sizeof(ioctlo_t)+sizeof(DiskInfo2_t));
		IoPtr->length = sizeof(ioctlo_t);	/* Save ioctlo_t length */
		IoPtr->comcode = 3;		/* READ command */
		IoPtr->address = RealTray+sizeof(ioctlo_t);
		IoPtr->bytes = sizeof(DiskInfo2_t);
		InfoPtr = (DiskInfo2_t *)(IoPtr+1);
		InfoPtr->Command = 0x0A;		/* Command code (GETAUDIOINFO) */
		MyRegs.ax = 0x1510;		/* Send Command via multiplexed interrupt */
		MyRegs.cx = static_cast<Word16>(Input->OpenDeviceID);	/* CD unit number */
		MyRegs.bx = static_cast<Word16>(RealTray);		/* Real pointer */
		MyRegs.es = static_cast<Word16>(RealTray>>16);
		Int86x(0x2F,&MyRegs,&MyRegs);		/* Issue the call */
		if (!(IoPtr->status&0x8000)) {		/* Error? */
			return InfoPtr->HighestTrack;		/* Save the volumes */
		}
	}
	return 0;		/* No tracks found (Error) */
}

/**********************************

	Return the current position of the CD in CD sectors

**********************************/

Word32 BURGERCALL RedbookGetPosition(Redbook_t *Input)
{
	if (Input->Active) {
		Regs16 MyRegs;
		Word32 RealTray;
		ioctlo_t *IoPtr;
		QInfo_t *InfoPtr;

		IoPtr = (ioctlo_t *)GetRealBufferProtectedPtr();
		RealTray = GetRealBufferPtr();		/* Real mode pointer */

		FastMemSet(IoPtr,0,sizeof(ioctlo_t)+sizeof(QInfo_t));
		IoPtr->length = sizeof(ioctlo_t);	/* Save ioctlo_t length */
		IoPtr->comcode = 3;		/* READ command */
		IoPtr->address = RealTray+sizeof(ioctlo_t);
		IoPtr->bytes = sizeof(QInfo_t);
		InfoPtr = (QInfo_t *)(IoPtr+1);
		InfoPtr->Command = 0x0C;		/* Command code (GETQINFO) */
		InfoPtr->Control = 1;			/* Get HSG data */
		MyRegs.ax = 0x1510;		/* Send Command via multiplexed interrupt */
		MyRegs.cx = static_cast<Word16>(Input->OpenDeviceID);	/* CD unit number */
		MyRegs.bx = static_cast<Word16>(RealTray);		/* Real pointer */
		MyRegs.es = static_cast<Word16>(RealTray>>16);
		Int86x(0x2F,&MyRegs,&MyRegs);		/* Issue the call */
		if (!(IoPtr->status&0x8000)) {		/* Error? */
			return RedbookMakePosition(InfoPtr->Minute,
				InfoPtr->Second,InfoPtr->Frame);	/* Convert the track from BCD */
		}
	}
	return 0;		/* Track not found or error */
}

/**********************************

	Return the currently playing track

**********************************/

Word BURGERCALL RedbookGetCurrentTrack(Redbook_t *Input)
{
	if (Input->Active) {
		Regs16 MyRegs;
		Word32 RealTray;
		ioctlo_t *IoPtr;
		QInfo_t *InfoPtr;

		IoPtr = (ioctlo_t *)GetRealBufferProtectedPtr();
		RealTray = GetRealBufferPtr();		/* Real mode pointer */

		FastMemSet(IoPtr,0,sizeof(ioctlo_t)+sizeof(QInfo_t));
		IoPtr->length = sizeof(ioctlo_t);	/* Save ioctlo_t length */
		IoPtr->comcode = 3;		/* READ command */
		IoPtr->address = RealTray+sizeof(ioctlo_t);
		IoPtr->bytes = sizeof(QInfo_t);
		InfoPtr = (QInfo_t *)(IoPtr+1);
		InfoPtr->Command = 0x0C;		/* Command code (GETQINFO) */
		MyRegs.ax = 0x1510;		/* Send Command via multiplexed interrupt */
		MyRegs.cx = static_cast<Word16>(Input->OpenDeviceID);	/* CD unit number */
		MyRegs.bx = static_cast<Word16>(RealTray);		/* Real pointer */
		MyRegs.es = static_cast<Word16>(RealTray>>16);
		Int86x(0x2F,&MyRegs,&MyRegs);		/* Issue the call */
		if (!(IoPtr->status&0x8000)) {		/* Error? */
			return BCDToNum(InfoPtr->Track);	/* Convert the track from BCD */
		}
	}
	return 0;		/* No tracks found (Error) */
}

/**********************************

	Return the current status of the CD audio driver

**********************************/

Word BURGERCALL RedbookGetPlayStatus(Redbook_t *Input)
{
	if (Input->Active) {		/* Is it active? */
		Regs16 MyRegs;
		Word32 RealTray;
		ioctlo_t *IoPtr;
		StatusInfo_t *InfoPtr;

		IoPtr = (ioctlo_t *)GetRealBufferProtectedPtr();
		RealTray = GetRealBufferPtr();		/* Real mode pointer */

		FastMemSet(IoPtr,0,sizeof(ioctlo_t)+sizeof(StatusInfo_t));
		IoPtr->length = sizeof(ioctlo_t);	/* Save ioctlo_t length */
		IoPtr->comcode = 3;		/* READ command */
		IoPtr->address = RealTray+sizeof(ioctlo_t);
		IoPtr->bytes = sizeof(StatusInfo_t);
		InfoPtr = (StatusInfo_t *)(IoPtr+1);
		InfoPtr->Command = 0x06;		/* Command code (GETSTATUS) */
		MyRegs.ax = 0x1510;		/* Send Command via multiplexed interrupt */
		MyRegs.cx = static_cast<Word16>(Input->OpenDeviceID);	/* CD unit number */
		MyRegs.bx = static_cast<Word16>(RealTray);		/* Real pointer */
		MyRegs.es = static_cast<Word16>(RealTray>>16);
		Int86x(0x2F,&MyRegs,&MyRegs);		/* Issue the call */
		if (!(IoPtr->status&0x8000)) {		/* Error? */
			if (InfoPtr->Status&1) {		/* Door opened? */
				return REDBOOKMODEOPENED;
			}
			if (IoPtr->status&0x200) {		/* Is the drive busy? */
				return REDBOOKMODEPLAYING;
			}
			if (Input->Paused) {			/* Paused or stopped? */
				return REDBOOKMODEPAUSED;
			}
		}
	}
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
		}
	}
	return TRUE;		/* An error occured! */
}

#endif
