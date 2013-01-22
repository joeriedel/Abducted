#include "RbRedBook.h"

#if defined(__WIN32__)
#include "ClStdLib.h"
#include "FmFile.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

/**********************************

	Initialize a redbook audio cd structure

**********************************/

Word BURGERCALL RedbookInit(Redbook_t *Input)
{
	FastMemSet(Input,0,sizeof(Redbook_t));

	/* Windows 95 version uses the multimedia manager to access the CD-Audio tracks */

//	Input->Mixer = FALSE;
	{
		MCI_OPEN_PARMS OpenMe;
		OpenMe.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_CD_AUDIO;	/* CD audio driver */
		if (!mciSendCommand(0,MCI_OPEN,MCI_WAIT|MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID,
			(DWORD)&OpenMe)) {	/* Did it open? */
			Word DevCount,i;

			Input->OpenDeviceID = OpenMe.wDeviceID;		/* I have the device ID */
			Input->Active = TRUE;	/* I have an active CD device */
//			Input->Paused = FALSE;
//			Input->Timer = FALSE;
			{
				MCI_SET_PARMS SetMe;
				SetMe.dwTimeFormat = MCI_FORMAT_TMSF;	/* Force the time format */
				mciSendCommand(Input->OpenDeviceID,MCI_SET,MCI_WAIT|MCI_SET_TIME_FORMAT,(DWORD)&SetMe);
			}

			/* For Win95, I need to scan for the aux audio mixer */
			/* So I can set and get the volume */

			DevCount=auxGetNumDevs();		/* How many devices? */
			if (DevCount) {					/* Any found? */
				AUXCAPS MyCaps;
				i = 0;
				do {			/* Find the mixer for the volume control */
					if (!auxGetDevCaps(i,&MyCaps,sizeof(MyCaps))) {
						if (MyCaps.wTechnology&AUXCAPS_CDAUDIO) {	/* CD Mixer? */
							Input->Mixer = TRUE;		/* I have a mixer! */
							Input->MixerFlags = MyCaps.dwSupport;	/* Save flags */
							Input->MixerDevice = i;		/* Save the device ID */
							break;
						}
					}
				} while (++i<DevCount);		/* Scan all */
			}
			return FALSE;		/* It's cool!! */
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

		/* The windows 95 version needs to release the device driver */
		{
			MCI_GENERIC_PARMS CloseMe;
			mciSendCommand(Input->OpenDeviceID,MCI_CLOSE,MCI_WAIT,(DWORD)&CloseMe);
		}
		Input->Mixer = FALSE;	/* No mixer active */
		Input->Active = FALSE;	/* No longer active */
	}
}

/**********************************

	This is the Windows 95 callback proc

**********************************/

static void CALLBACK LoopProc(Word uID,Word uMsg,DWORD dwUser,DWORD dw1,DWORD dw2)
{
	Word Status;
	Redbook_t *Input;
	Input = (Redbook_t *)dwUser;		/* Get pointer to my data */
	if (Input->Active && !Input->Paused) {	/* Must be valid */
		Status = RedbookGetPlayStatus(Input);
		if (Status==REDBOOKMODEOPENED) {
			RedbookStop(Input);
			return;
		}
		if (Status==REDBOOKMODESTOPPED) {
			MCI_PLAY_PARMS PlayMe;
			FastMemSet(&PlayMe,0,sizeof(PlayMe));
			PlayMe.dwFrom = Input->FromMark;	/* Restart the music */
			PlayMe.dwTo = Input->ToMark;
			if (mciSendCommand(Input->OpenDeviceID,MCI_PLAY,MCI_FROM|MCI_TO,(DWORD)&PlayMe)) {
			/* The error occured, just send a raw play command */
				mciSendCommand(Input->OpenDeviceID,MCI_PLAY,MCI_FROM,(DWORD)&PlayMe);
			}
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
		Input->ToMark = 0;		/* Init the mark */
		{
			MCI_SET_PARMS SetMe;
			FastMemSet(&SetMe,0,sizeof(SetMe));
			SetMe.dwAudio = MCI_SET_AUDIO_ALL;		/* Enable audio output */
			mciSendCommand(Input->OpenDeviceID,MCI_SET,MCI_SET_AUDIO|MCI_SET_ON,(DWORD)&SetMe);
		}
		{
			MCI_PLAY_PARMS PlayMe;
			Word Flags;
			FastMemSet(&PlayMe,0,sizeof(PlayMe));	/* Blank it out */
			PlayMe.dwFrom = RedbookMakeTMSF(StartTrack,StartPosition);
			Input->FromMark = PlayMe.dwFrom;
			Flags = MCI_FROM;
			if (Mode!=REDBOOKPLAY) {		/* Play forever? */
				PlayMe.dwTo = RedbookMakeTMSF(EndTrack,EndPosition);
				Flags = MCI_FROM|MCI_TO;	/* Add an end mark */
				Input->ToMark = PlayMe.dwTo;	/* Save the mark */
			}
		/* An error COULD occur if I wish to play to the end of */
		/* the CD on some device drivers. */
			if (mciSendCommand(Input->OpenDeviceID,MCI_PLAY,Flags,(DWORD)&PlayMe)) {
				/* The error occured, just send a raw play command */
				mciSendCommand(Input->OpenDeviceID,MCI_PLAY,MCI_FROM,(DWORD)&PlayMe);
			}

			/* Spawn an timer thread at 2 times a second */

			if (Mode==REDBOOKPLAYLOOP) {
				Input->LoopingTimerID = timeSetEvent(1000/2,1000/60,LoopProc,(DWORD)Input,TIME_PERIODIC);
				Input->Timer = TRUE;
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
		MCI_GENERIC_PARMS StopMe;
		if (Input->Timer) {		/* Is there a background task? */
			timeKillEvent(Input->LoopingTimerID);	/* Kill the thread */
			Input->Timer = FALSE;
		}
		mciSendCommand(Input->OpenDeviceID,MCI_STOP,0,(DWORD)&StopMe);
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
			MCI_GENERIC_PARMS PauseMe;
			mciSendCommand(Input->OpenDeviceID,MCI_PAUSE,0,(DWORD)&PauseMe);
			Input->Paused = TRUE;
		}
	}
}

/**********************************

	Resume paused redbook audio

**********************************/

void BURGERCALL RedbookResume(Redbook_t *Input)
{
	if (Input->Active && Input->Paused) {	/* Valid input? */
		{
			MCI_SET_PARMS SetMe;
			SetMe.dwAudio = MCI_SET_AUDIO_ALL;		/* Enable audio output */
			mciSendCommand(Input->OpenDeviceID,MCI_SET,MCI_SET_AUDIO|MCI_SET_ON,(DWORD)&SetMe);
		}
		{
			MCI_PLAY_PARMS PlayMe;
			Word Flags;
			Flags = 0;
			if (Input->ToMark) {		/* Is there a destination? */
				Flags = MCI_TO;			/* Play to mark */
				PlayMe.dwTo = Input->ToMark;
			}
			mciSendCommand(Input->OpenDeviceID,MCI_PLAY,Flags,(DWORD)&PlayMe);
		}
		Input->Paused = FALSE;
	}
}

/**********************************

	Set the Audio CD's volume.
	Valid input is 0-255 (0=quiet, 255 = maximum)

**********************************/

void BURGERCALL RedbookSetVolume(Redbook_t *Input,Word Left,Word Right)
{
	if (Input->Mixer) {		/* Can I adjust the volume? */
		Word32 Volume;
		Left=(Left<<8)|Left;	/* Convert from Burgerlib to Win95 */
		Right=(Right<<8)|Right;
		if (!(Input->MixerFlags&AUXCAPS_LRVOLUME)) {	/* Can't control? */
			Left = (Right+Left)>>1;		/* Average the volumes for mono */
			Right = Left;
		}
		Volume = (Right<<16)|Left;		/* Create the volume */
		auxSetVolume(Input->MixerDevice,Volume);	/* Set it */
	}
}

/**********************************

	Set a new loop start point

**********************************/

void BURGERCALL RedbookSetLoopStart(Redbook_t *Input,Word Track,Word32 Position)
{
	if (Input->Active) {		/* Valid structure */
		Input->FromMark = RedbookMakeTMSF(Track,Position);
	}
}

/**********************************

	Set a new loop end point

**********************************/

void BURGERCALL RedbookSetLoopEnd(Redbook_t *Input,Word Track,Word32 Position)
{
	if (Input->Active) {		/* Valid structure */
		Input->ToMark = RedbookMakeTMSF(Track,Position);
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
			MCI_SET_PARMS SetMe;
			mciSendCommand(Input->OpenDeviceID,MCI_SET,MCI_SET_DOOR_OPEN,(DWORD)&SetMe);
		}
	}
}


/**********************************

	Close the CD drawer

**********************************/

void BURGERCALL RedbookCloseDrawer(Redbook_t *Input)
{
	if (Input->Active) {	/* Valid input? */
		MCI_SET_PARMS SetMe;
		mciSendCommand(Input->OpenDeviceID,MCI_SET,MCI_SET_DOOR_CLOSED,(DWORD)&SetMe);
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

	if (Input->Mixer) {		/* Is there a volume mixer? */
		if (Input->MixerFlags&(AUXCAPS_LRVOLUME|AUXCAPS_VOLUME)) {
			unsigned long Volume;
			if (!auxGetVolume(Input->MixerDevice,&Volume)) {	/* Get volume */
				Left = (Volume>>8)&0xFF;		/* Convert to Burgerlib */
				if (Input->MixerFlags&AUXCAPS_LRVOLUME) {	/* Left/Right */
					Right = Volume>>24;
				} else {
					Right = Left;		/* Fake stereo from mono */
				}
			}
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
		Word Fail;
		MCI_STATUS_PARMS StatusMe;
		MCI_SET_PARMS SetMe;

		SetMe.dwTimeFormat = MCI_FORMAT_MSF;	/* Absolute Min,Sec Frame */
		mciSendCommand(Input->OpenDeviceID,MCI_SET,MCI_WAIT|MCI_SET_TIME_FORMAT,(DWORD)&SetMe);

		StatusMe.dwItem = MCI_STATUS_POSITION;	/* Win95 status command */
		StatusMe.dwTrack = TrackNum;			/* Track to check */
		Fail = mciSendCommand(Input->OpenDeviceID,MCI_STATUS,MCI_STATUS_ITEM|MCI_TRACK|MCI_WAIT,(DWORD)&StatusMe);

		SetMe.dwTimeFormat = MCI_FORMAT_TMSF;	/* Set it back! */
		mciSendCommand(Input->OpenDeviceID,MCI_SET,MCI_WAIT|MCI_SET_TIME_FORMAT,(DWORD)&SetMe);

		if (!Fail) {
			return RedbookMakePosition(MCI_MSF_MINUTE(StatusMe.dwReturn),
				MCI_MSF_SECOND(StatusMe.dwReturn),
				MCI_MSF_FRAME(StatusMe.dwReturn));
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
		MCI_STATUS_PARMS StatusMe;
		StatusMe.dwItem = MCI_STATUS_LENGTH;	/* Win95 status command */
		StatusMe.dwTrack = TrackNum;			/* Track to check */
		if (!mciSendCommand(Input->OpenDeviceID,MCI_STATUS,MCI_STATUS_ITEM|MCI_TRACK|MCI_WAIT,(DWORD)&StatusMe)) {
			return RedbookMakePosition(MCI_MSF_MINUTE(StatusMe.dwReturn),
				MCI_MSF_SECOND(StatusMe.dwReturn),
				MCI_MSF_FRAME(StatusMe.dwReturn));
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
		MCI_STATUS_PARMS StatusMe;
		StatusMe.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;		/* Win95 status command */
		if (!mciSendCommand(Input->OpenDeviceID,MCI_STATUS,MCI_STATUS_ITEM|MCI_WAIT,(DWORD)&StatusMe)) {
			return (Word)StatusMe.dwReturn;		/* Got the data */
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
		MCI_STATUS_PARMS StatusMe;
		StatusMe.dwItem = MCI_STATUS_POSITION;		/* Win95 status command */
		if (!mciSendCommand(Input->OpenDeviceID,MCI_STATUS,MCI_STATUS_ITEM|MCI_WAIT,(DWORD)&StatusMe)) {
			return RedbookMakePosition(MCI_TMSF_MINUTE(StatusMe.dwReturn),
				MCI_TMSF_SECOND(StatusMe.dwReturn),
				MCI_TMSF_FRAME(StatusMe.dwReturn));
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
		MCI_STATUS_PARMS StatusMe;
		StatusMe.dwItem = MCI_STATUS_CURRENT_TRACK;		/* Win95 status command */
		if (!mciSendCommand(Input->OpenDeviceID,MCI_STATUS,MCI_STATUS_ITEM|MCI_WAIT,(DWORD)&StatusMe)) {
			return (Word)StatusMe.dwReturn;		/* Got the track number */
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
		MCI_STATUS_PARMS StatusMe;
		StatusMe.dwItem = MCI_STATUS_MODE;		/* Win95 status command */
		if (!mciSendCommand(Input->OpenDeviceID,MCI_STATUS,MCI_STATUS_ITEM|MCI_WAIT,(DWORD)&StatusMe)) {
			switch (StatusMe.dwReturn) {
			case MCI_MODE_STOP:
				if (!Input->Paused) {		/* It's really stopped */
					break;
				}
			case MCI_MODE_PAUSE:
				return REDBOOKMODEPAUSED;	/* It's paused */
			case MCI_MODE_PLAY:
				return REDBOOKMODEPLAYING;	/* Actively playing */
			case MCI_MODE_NOT_READY:
			case MCI_MODE_SEEK:
			case MCI_MODE_RECORD:
				return REDBOOKMODEBUSY;		/* Busy or command pending */
			case MCI_MODE_OPEN:
				return REDBOOKMODEOPENED;	/* Drawer is open */
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
		if (VolumeNumber!=(Word)-1) {		/* Valid? */
		}
	}
	return TRUE;		/* An error occured! */
}



#endif