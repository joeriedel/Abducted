/**********************************

	This code is ONLY for the MacOS version of
	Burgerlib Audio. I use the Sound Manager 3.0 for all
	of my sound needs

**********************************/

#include "SnSound.h"

#if defined(__MAC__)
#include <BREndian.hpp>
#include "RzRez.h"
#include "ClStdLib.h"
#include "InInput.h"
#include <Sound.h>

#define SNDCHUNKSIZE 2048

typedef struct MySndDoubleBuffer {	/* Needed by MacOS for the double back proc */
	ExtSoundHeader TheHeader;		/* My data */
	Word8 SoundData[SNDCHUNKSIZE-2];	/* Data buffer */
} MySndDoubleBuffer;

static MySndDoubleBuffer MyBuffer[MAXVOICECOUNT];		/* Double buffer arrays */

#ifdef __cplusplus
extern "C" {
static Word IsIt(PrivSound_t *MySound,void *Input);
static Word SetFrq(PrivSound_t *MySound,void *Input);
static Word SetPan(PrivSound_t *MySound,void *Input);
static Word SetVolx(PrivSound_t *MySound,void *Input);
static Word StopIt(PrivSound_t *MySound,void *Input);
static pascal void BurgerCallBackProc(SndChannelPtr Chan,SndCommand *Input);
}
#endif

/**********************************

	The Mac OS version is based on documentation found at...
	devworld.apple.com/ngs/adrpub/dosc/dev/technotes/tn/tn1048.html
	devworld.apple.com/ngs/lpp/adrpub/docs/dev/techsupport/insidemac/Sound/Sound-58.html

**********************************/

static void MacSetSoundVolumes(PrivSound_t *MySound)
{
	Word Vol;
	SndCommand Command;
	Command.cmd = volumeCmd;
	Command.param1 = 0;
	Vol = (MySound->Volume*MasterSoundVolume)/255;
	if (Vol>=128) {		/* Convert range of 0-255 to 0-256 */
		++Vol;
	}
	if (MySound->PanPosition!=0x8000) {		/* I apply the panning here */
		Word Right,Left;
		if (MySound->PanPosition<0x8000) {
			Left = Vol;
			Right = (MySound->PanPosition*Vol)>>15;
		} else {
			Right = Vol;
			Left = ((0xFFFF-MySound->PanPosition)*Vol)>>15;
		}
		Command.param2 = (Right<<16)|Left;
	} else {
		Command.param2 = (Vol<<16)|Vol;
	}
	SndDoImmediate(MySound->SampleHandle,&Command);
}

/**********************************

	Returns true or false is a sound effect is playing

**********************************/

static Word IsIt(PrivSound_t *MySound,void *Input)
{
	if (MySound->SoundPtr) {		/* Is there a pointer? */
		SCStatus Status;
		if (!SndChannelStatus(MySound->SampleHandle,sizeof(Status),&Status)) {
			if (Status.scChannelBusy || Status.scChannelPaused) {
				return TRUE;
			}
		}
	}
	return FALSE;				/* Not active anymore */
}

Word BURGERCALL IsASoundPlaying(Word32 SoundNum)
{
	return SoundIterator(IsIt,0,SoundNum,TRUE);	/* Handle it */
}

/**********************************

	Set the frequency of a sound effect

**********************************/

static Word SetFrq(PrivSound_t *MySound,void *Input)
{
	Word Frq;
	Frq = (Word)Input;
	if (MySound->SampleRate!=Frq) {
		if (MySound->SoundPtr) {
			SndCommand Command;
			Word FailSafe;
			Command.cmd = rateMultiplierCmd;
			Command.param1 = 0;
			FailSafe = MySound->SampleRate;
			if (!FailSafe) {
				FailSafe = 22050;		/* Prevent a divide by zero */
			}
			Command.param2 = (Frq<<16)/FailSafe;		/* Convert to fixed rate offset */
			SndDoImmediate(MySound->SampleHandle,&Command);
		}
		MySound->SampleRate = Frq;	/* Get the pan position value */
	}
	return FALSE;
}

void BURGERCALL SetASoundFrequency(Word32 SoundNum,Word Frequency)
{
	SoundIterator(SetFrq,(void *)Frequency,SoundNum,FALSE);	/* Handle it */
}

/**********************************

	Set the pan position for a sound effect

**********************************/

static Word SetPan(PrivSound_t *MySound,void *Input)
{
	Word Pan;
	Pan = (Word)Input;
	if (MySound->PanPosition!=Pan) {
		MySound->PanPosition = Pan;	/* Get the pan position value */
		MySound->Dirty = TRUE;
	}
	return FALSE;
}

void BURGERCALL SetASoundPan(Word32 SoundNum,Word Pan)
{
	SoundIterator(SetPan,(void *)Pan,SoundNum,FALSE);	/* Handle it */
}

/**********************************

	Set the volume for a sound effect

**********************************/

static Word SetVolx(PrivSound_t *MySound,void *Input)
{
	Word32 Vol;
	Vol = (Word32)Input;
	if (MySound->Volume!=Vol) {
		MySound->Volume = Vol;	/* Get the pan position value */
		MySound->Dirty = TRUE;
	}
	return FALSE;
}

void BURGERCALL SetASoundVolume(Word32 SoundNum,Word Volume)
{
	if (Volume>=256) {	/* Make sure the volume is in range */
		Volume = 255;
	}
	SoundIterator(SetVolx,(void *)Volume,SoundNum,FALSE);	/* Handle it */
}

/**********************************

	Set the sound effects volume

**********************************/

void BURGERCALL SetSfxVolume(Word NewVolume)
{
	Word i;
	if (NewVolume>=256) {	/* Make sure the volume is in range */
		NewVolume = 255;
	}
	i = SystemState&(~SfxActive);	/* Clear the active flag */
	if (NewVolume) {
		i |= SfxActive;		/* Set the flag */
	}
	SystemState = i;		/* Save the flag */
	if (MasterSoundVolume!=NewVolume) {
		PrivSound_t *MySound;		
		MasterSoundVolume = NewVolume;
		i = MAXVOICECOUNT;
		MySound = BurgerSamples;
		do {
			MySound->Dirty = TRUE;		/* Make sure I alter the sound volumes */
			++MySound;
		} while (--i);
	}
}

/**********************************

	Low level routine to shut down a sound
	using my internal audio reference
	Also check if no one else is using the source resource,
	I will then release the memory

**********************************/

static Word BURGERCALL StopIt(PrivSound_t *MySound,void *Input)
{
	if (MySound->SoundPtr) {		/* Is the channel used? */
		Word RezNum;
		Word *CookiePtr;

		SndCommand Command;
		Command.cmd = quietCmd;		/* Shut down the output */
		Command.param1 = 0;
		Command.param2 = 0;
		SndDoImmediate(MySound->SampleHandle,&Command);	/* Send the command */
		Command.cmd = flushCmd;
		Command.param1 = 0;
		Command.param2 = 0;
		SndDoImmediate(MySound->SampleHandle,&Command);
		if (MySound->Proc) {		/* Call the completion pointer */
			MySound->Proc(MySound->Data);
		}
		MySound->SoundPtr = 0;			/* Clear the channel */

		RezNum = MySound->SampleResource;	/* Get the resource number */
		if (RezNum!=(Word)-1) {		/* Special sound number */
			ResourceRelease(MySound->SampleRezHeader,RezNum);	/* Release the sound resource */
		}
		CookiePtr = MySound->CookiePtr;	/* Is there a callback? */
		if (CookiePtr) {
			CookiePtr[0] = (Word)-1;	/* Mark as not used */
		}
	}
	return FALSE;
}

/**********************************

	Stop a sound effect in progress if needed

**********************************/

void BURGERCALL StopASound(Word32 SoundNum)
{
	SoundIterator(StopIt,0,SoundNum,FALSE);
}

/**********************************

	Play a sound effect using a structure describing
	how to play a sound effect.
	PanPosition and SoundLoopFlag are used as overrides

	I will return the Cookie number

**********************************/


/**********************************

	Convert and transfer the data
	I return the number of FRAMES converted!

**********************************/

static Word SndTransfer(Word8 *DestPtr,const Word8 *SrcPtr,Word32 Length,Word Type,Word32 *Offset)
{
	Word32 Frames;
	Word Temp;
	
	Frames = Length>>1;		/* Assume frames == length/2 */
	if (Type&SOUNDTYPESTEREO) {		/* If stereo data, then halve the frame count */
		Frames>>=1;					/* Halve again */
	}
	if (Length) {
		switch (Type&0xFF) {
		case SOUNDTYPEBSHORT:	/* Big endian shorts */
		default:
			Offset[0] += Length;
			FastMemCpy(DestPtr,SrcPtr,Length);	/* No translation needed */
			break;
		case SOUNDTYPEBYTE:		/* Unsigned chars */
			Length>>=1;
			Offset[0] += Length;
			do {
				Temp = (((Word8 *)SrcPtr)[0]^0x80)<<8;
				((short *)DestPtr)[0] = Temp;	/* Convert to signed */
				++SrcPtr;
				DestPtr=DestPtr+2;
			} while (--Length);
			break;
		case SOUNDTYPECHAR:
			Length>>=1;
			Offset[0] += Length;
			do {
				Temp = ((Word8 *)SrcPtr)[0]<<8;
				((short *)DestPtr)[0] = Temp;	/* Convert to signed */
				++SrcPtr;
				DestPtr=DestPtr+2;
			} while (--Length);
			break;
		case SOUNDTYPELSHORT:
			Offset[0] += Length;
			Length>>=1;
			do {
				((Word16 *)DestPtr)[0] = Burger::LoadLittle((Word16 *)SrcPtr);
				SrcPtr=SrcPtr+2;				/* Adjust the pointers */
				DestPtr=DestPtr+2;
			} while (--Length);
		}
	}
	return Frames;					/* Number of sound manager frames */
}

/**********************************

	Refill the data buffers for the MacOS sound manager
	This is called via interrupts!!!!
	Keep it short and to the point.

**********************************/

static pascal void BurgerCallBackProc(SndChannelPtr Chan,SndCommand *Input)
{
	SndCommand mySndCmd;
	Word32 Offset;
	Word32 Length;
	Word Type;
	Word8 *SrcPtr;
	PrivSound_t *MySound;
	MySndDoubleBuffer *WorkPtr;
	Word Cookie;
	Word EndNow;
	
	#if !TARGET_RT_MAC_CFM
	long OldA5;
	OldA5 = SetA5(Chan->userInfo);	/* Get the A5 register */
	#endif

	MySound = (PrivSound_t *)Input->param2;	/* Get the sound record pointer */
	Cookie = Input->param1;
	WorkPtr = &MyBuffer[Cookie];
	Offset = MySound->WorkOffset;			/* Get the starting offset */
	SrcPtr = MySound->SoundPtr+Offset;	/* Get the data pointer */
	Length = MySound->SoundLength-Offset;	/* End of data */
	Type = MySound->DataType&0xFF;
	if ((Type ==SOUNDTYPEBYTE) || (Type==SOUNDTYPECHAR)) {
		Length <<= 1;			/* I'll create twice as much data */
	}
	if (Length>SNDCHUNKSIZE) {	/* Clamp to buffer size */
		Length = SNDCHUNKSIZE;
	}

	WorkPtr->TheHeader.numFrames = SndTransfer((Word8 *)WorkPtr->TheHeader.sampleArea,SrcPtr,Length,MySound->DataType,&MySound->WorkOffset);
	EndNow = FALSE;
	if (MySound->WorkOffset>=MySound->SoundLength) {		/* No more? */
		if (MySound->SoundLength && MySound->LoopEnd) {
			MySound->WorkOffset = 0;
		} else {
			EndNow = TRUE;
		}
	}
	WorkPtr->TheHeader.samplePtr = 0;
//	WorkPtr->TheHeader.numChannels = 2;
	WorkPtr->TheHeader.sampleRate = MySound->SampleRate<<16;
	WorkPtr->TheHeader.loopStart = 0;
	WorkPtr->TheHeader.loopEnd = 0;
	WorkPtr->TheHeader.encode = extSH;
	WorkPtr->TheHeader.baseFrequency = 0;
//	WorkPtr->TheHeader.numFrames = LocalPtr->Frames;
	WorkPtr->TheHeader.markerChunk = NULL;
	WorkPtr->TheHeader.instrumentChunks = NULL;
	WorkPtr->TheHeader.AESRecording = NULL;
//	WorkPtr->TheHeader.sampleSize = 16;
	
	mySndCmd.cmd = rateMultiplierCmd;
	mySndCmd.param1 = 0;
	mySndCmd.param2 = (1<<16);		/* Convert to fixed rate offset */
	SndDoCommand(Chan,&mySndCmd,TRUE);

	mySndCmd.cmd = bufferCmd;
	mySndCmd.param1 = 0;
	mySndCmd.param2 = (int)&WorkPtr->TheHeader;
	SndDoCommand(Chan,&mySndCmd,TRUE);
	
	// and another callback
	if (!EndNow) {	
		mySndCmd.cmd = callBackCmd;
		mySndCmd.param1 = Cookie;
		mySndCmd.param2 = (long)MySound;
		SndDoCommand(Chan,&mySndCmd,TRUE);
	}
	#if !TARGET_RT_MAC_CFM
	SetA5(OldA5);
	#endif
}

/**********************************

	Begin the actual generic code...

**********************************/

Word BURGERCALL PlayARawSound(RawSound_t *Input)
{
	Word Cookie;
	Word i,j;
	PrivSound_t *MySound;

	Cookie = (Word)-1;		/* Assume no voice was allocated */
	if ((BurgerSndExitIn&DIGISOUNDON) && (SystemState&SfxActive) &&
		BurgerVoiceCount && Input->SoundPtr) {

	/* I will now play the sound, first find an empty channel */

		MySound = BurgerSamples;
		Cookie = 0;
		j = 99;

		do {
			if (!MySound->SoundPtr) {		/* Find an empty sound channel */
				goto Begin;			/* Found it! */
			}
		/* Is this the one I'll steal? */

			if (!MySound->LoopEnd && BurgerSamplePriority[Cookie]<j) {
				j = BurgerSamplePriority[Cookie];		/* Save priority */
				i = Cookie;			/* Save the cookie number */
			}
			++MySound;
		} while (++Cookie<BurgerVoiceCount);		/* All done? */

		if (j==99) {		/* No voices available? */
			Cookie = (Word)-1;	/* Bad!! */
			goto EndNow;
		}
		Cookie = i;			/* I'll steal this voice */
		StopASound(Cookie|SOUND_COOKIE);		/* Dispose of the sound */
		MySound = &BurgerSamples[Cookie];

	/* Start the sound */
	/* At this point, I now have a sound channel ready for my use */

Begin:

	/* Fill in my required data */

		MySound->SoundPtr = Input->SoundPtr;
		MySound->SoundLength = Input->SoundLength;
		if (Input->LoopEnd) {
			MySound->LoopStart = Input->LoopStart;
			MySound->LoopEnd = Input->LoopEnd;
		} else {
			MySound->LoopStart = 0;
			if (SoundLoopFlag) {
				MySound->LoopEnd = Input->SoundLength;
			} else {
				MySound->LoopEnd = 0;
			}
		}

		MySound->Proc = Input->Proc;
		MySound->Data = Input->Data;
		MySound->DataType = Input->DataType;
		MySound->SampleRate = Input->SampleRate+SoundFrequencyAdjust;
		MySound->PanPosition = PanPosition;
		MySound->Volume = SfxVolume;
		MySound->Dirty = FALSE;

	/* Fill in the EXTRA data (This may be filled in by other code) */

		MySound->CookiePtr = SoundCookiePtr;
		MySound->SampleResource = (Word)-1;		/* No resource assigned */
		MySound->SampleRezHeader = 0;

		/* Here is the MACHINE specific code */
		/* Your mileage may vary */


		MacSetSoundVolumes(MySound);	/* Set the pan and volume */

		{
			MySndDoubleBuffer *MyBufferPtr;
			SndCommand Command;
			MyBufferPtr = &MyBuffer[Cookie];		/* Pointer to buffer to work with */
			
			/* Now fill in all the records for the double buffer record */
			i = MySound->DataType;		/* Get the data type */
			j = 1;
			if (i&SOUNDTYPESTEREO) {	/* Stereo data? */
				j = 2;
			}
			MyBufferPtr->TheHeader.numChannels = j;		/* Stereo or mono */
//			switch (i&0xFF) {		/* Type of data */
//			default:
//			case SOUNDTYPEBYTE:		/* Unsigned byte */
//			case SOUNDTYPECHAR:		/* Signed byte */
//				TheHeader.dbhSampleSize = 8;
//				break;
//			case SOUNDTYPELSHORT:	/* Little endian signed short */
//			case SOUNDTYPEBSHORT:	/* Big endian signed short */
				j<<=1;				/* Double the number of BYTES per sample */
				MyBufferPtr->TheHeader.sampleSize = 16;
//			}
			MySound->WorkOffset = 0;		/* Start at the beginning */
			Command.param1 = Cookie;
			Command.param2 = (long)MySound;
			BurgerCallBackProc(MySound->SampleHandle,&Command);	/* Fill buffer #1 */
		}

/**********************************

	Now that the sound has started, I go back
	to generic code

**********************************/

	/* Save the new voice priority in the FIFO buffer */

		j = BurgerSamplePriority[Cookie];		/* Get the previous priority */
		i = 0;
		do {
			if (BurgerSamplePriority[i]>=j) {	/* Reshuffle the priority chain */
				--BurgerSamplePriority[i];
			}
		} while (++i<BurgerVoiceCount);
		BurgerSamplePriority[Cookie] = (BurgerVoiceCount-1);	/* Set the highest priority */
	}
EndNow:
	PanPosition = 0x8000;	/* Center pan position */
	SoundLoopFlag = FALSE;	/* Don't loop */
	SfxVolume = 255;		/* Max volume */
	SoundFrequencyAdjust = 0;	/* Don't pitch bend */
	Cookie |= SOUND_COOKIE;			/* Make the cookie a true cookie */
	if (SoundCookiePtr) {
		SoundCookiePtr[0] = Cookie;	/* Save the channel allocated */
		SoundCookiePtr = 0;			/* Reset the cookie pointer */
	}
	return Cookie;			/* Return the channel I started (Or -1 for failure) */
}

/**********************************

	This is a foreground polling routine called
	by the KeyboardKbhit proc chain. It will scan the voices
	and dispose of any sounds that have finished playing
	so the memory won't be wasted.

**********************************/

void BURGERCALL DigitalSoundCheckOff(void *Input)
{
	Word i,Max;
	PrivSound_t *MySound;

	Max = BurgerVoiceCount;			/* Any voices active? */
	if (Max) {
		MySound = BurgerSamples;	/* Get the array */
		i = 0;			/* Release any sounds that have ended normally */
		do {
			if (MySound->SoundPtr) {		/* Is the voice active? */
				if (!IsIt(MySound,0)) {		/* In use? */
					StopIt(MySound,0);		/* Release the voice */
				} else if (MySound->Dirty) {
					MySound->Dirty = FALSE;
					MacSetSoundVolumes(MySound);
				}
			}
			++MySound;		/* Next */
		} while (++i<Max);	/* All scanned? */
	}
}

/**********************************

	Generic routine to start up the digital sound driver
	(Direct Sound version)

**********************************/

Word BURGERCALL InitDigitalDriver(void)
{
	PrivSound_t *MySound;
	Word i;

	if (BurgerSndExitIn & (DIGIMUSICON|DIGISOUNDON)) {		/* Already on? */
		return TRUE;		/* Don't start again */
	}
	FastMemSet(BurgerSamples,0,sizeof(BurgerSamples));
	i = MAXVOICECOUNT;
	MySound = BurgerSamples;
	do {
		MySound->CallBackUPP = (void *)NewSndCallBackUPP(&BurgerCallBackProc);	/* Make it */
		if (SndNewChannel(&MySound->SampleHandle,sampledSynth,0,(SndCallBackUPP)MySound->CallBackUPP)) {
			NonFatal("SndNewCannel could not allocate 9 channels!\n");
			KillDigitalDriver();		/* Clear out the voices */
			return FALSE;
		}
		#if !TARGET_RT_MAC_CFM
		MySound->SampleHandle->userInfo = SetCurrentA5();
		#endif
		++MySound;
	} while (--i);
	KeyboardAddRoutine(DigitalSoundCheckOff,0);
	return TRUE;
}

/**********************************

	Generic routine to shut down the digital sound driver

**********************************/

void BURGERCALL KillDigitalDriver(void)
{
	if (BurgerSndExitIn & (DIGIMUSICON|DIGISOUNDON)) {	/* Is the code still on? */
		return;		/* Don't shut down! */
	}

	KeyboardRemoveRoutine(DigitalSoundCheckOff,0);
	{
		PrivSound_t *MySound;
		Word i;
		MySound = BurgerSamples;
		i = MAXVOICECOUNT;
		do {
			if (MySound->SampleHandle) {	/* Is a channel created? */
				SndDisposeChannel(MySound->SampleHandle,TRUE);
				MySound->SampleHandle = 0;
			}
			if (MySound->CallBackUPP) {
				DisposeSndCallBackUPP((SndCallBackUPP)MySound->CallBackUPP);
				MySound->CallBackUPP = 0;
			}
			++MySound;
		} while (--i);
	}
}



#endif

