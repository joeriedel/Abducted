/**********************************

	This code is ONLY for the DOS version of
	Burgerlib Audio. I use the HMI audio driver for all
	of my sound needs

**********************************/

#include "SnSound.h"

#if defined(__DOS4G__)

#include "TkTick.h"
#include "RzRez.h"
#include "FmFile.h"
#include "MmMemory.h"
#include "PfPrefs.h"
#include "ClStdLib.h"
#include "InInput.h"

#ifdef __cplusplus
extern "C" {
#endif
static Word BURGERCALL IsIt(PrivSound_t *MySound,void *Input);
static Word BURGERCALL SetFrq(PrivSound_t *MySound,void *Input);
static Word BURGERCALL SetPan(PrivSound_t *MySound,void *Input);
static Word BURGERCALL SetVolx(PrivSound_t *MySound,void *Input);
static Word BURGERCALL StopIt(PrivSound_t *MySound,void *Input);
extern void BURGERCALL UninstallTick(void);	/* Remove the ReadTick code */
extern volatile Word32 ReadTickTimeCount;	/* Readtick internal counter */
#ifdef __cplusplus
}
#endif

HANDLE hDIGITimer;		/* handle to digital mixer */
HANDLE hDIGIDriver = (HANDLE)-1;		/* Handle to digital driver */
static _SOS_DIGI_DRIVER sDIGIDriver;	/* digital driver structure */
static char MIDIHeader[] = "[DIGITAL]";
static HANDLE NewTimerHandle;	/* Handle to timer routine */

/**********************************

	Since the music driver installs a new IRQ 8
	routine, I need to replace the ReadTick timer
	with one called by the sound driver or bad things will happen

**********************************/

static void BURGERCALL NewTimer(void)
{
	++ReadTickTimeCount;		/* Just inc the tick value */
}

/**********************************

	Shut down all the drivers,
	each audio driver will install a shut down proc callback
	so I don't have to link in everything, only what you need.

**********************************/

static void KillSoundTimer(void)
{
	Word i;
	void (BURGERCALL *KillProc)(void);

	if (NewTimerHandle) {		/* ReadTick() timer */
		sosTIMERRemoveEvent(NewTimerHandle);	/* Remove the timer */
		NewTimerHandle = 0;		/* Clear it out */
	}

	i = 0;
	do {
		KillProc = BurgerSndKillProcs[i];	/* Is there a proc here? */
		if (KillProc) {
			KillProc();		/* Shut down the services */
		}
	} while (++i<3);
	if (BurgerSndExitIn&TIMERON) {
		sosTIMERUnInitSystem(0);	/* Shut down the timer, (DOS default) */
	}
	BurgerSndExitIn &= ~(TIMERON);			/* Everything is shut down */
}

/**********************************

	Init the HMI timer services. This is required
	before any HMI services can be used.

**********************************/

void BURGERCALL HMIInitSoundTimer(void)
{
	if (!(BurgerSndExitIn&TIMERATEXIT)) {	/* Remove me? */
		BurgerSndExitIn|=TIMERATEXIT;
		atexit(KillSoundTimer);		/* Add the timer */
	}

	if (!(BurgerSndExitIn&TIMERON)) {		/* Did I install my shut down code? */
		ReadTick();		/* Make SURE that the tick handler is installed FIRST! */
		UninstallTick();	/* Remove any installed handler */
		sosTIMERInitSystem(_TIMER_DOS_RATE,_SOS_DEBUG_NORMAL);
		sosTIMERRegisterEvent(TICKSPERSEC,NewTimer,&NewTimerHandle);
		BurgerSndExitIn |= TIMERON;		/* Ack the flag */
	}
}

/**********************************

	Returns true or false is a sound effect is playing

**********************************/

static Word BURGERCALL IsIt(PrivSound_t *MySound,void * /* Input */ )
{
	if (MySound->SoundPtr) {		/* Is there a pointer? */
		if (!sosDIGISampleDone(hDIGIDriver,MySound->SampleHandle)) {
			return TRUE;			/* Sound is active */
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

static Word BURGERCALL SetFrq(PrivSound_t *MySound,void *Input)
{
	Word Frq;
	Frq = (Word)Input;
	if (MySound->SampleRate!=Frq) {
		MySound->SampleRate = Frq;	/* Get the pan position value */

		if (MySound->SoundPtr) {	/* Voice active? */
			sosDIGISetSampleRate(hDIGIDriver,MySound->SampleHandle,Frq);
		}
		MySound->SampleRecord.wRate = Frq;
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

static Word BURGERCALL SetPan(PrivSound_t *MySound,void *Input)
{
	Word Pan;
	Pan = (Word)Input;
	if (MySound->PanPosition!=Pan) {
		MySound->PanPosition = Pan;	/* Get the pan position value */

		if (MySound->SoundPtr) {	/* Voice active? */
			sosDIGISetPanLocation(hDIGIDriver,MySound->SampleHandle,Pan);
		}
		MySound->SampleRecord.wPanPosition = Pan;
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

static Word BURGERCALL SetVolx(PrivSound_t *MySound,void *Input)
{
	Word32 Vol;
	Vol = (Word)Input;
	if (MySound->Volume!=Vol) {
		MySound->Volume = Vol;	/* Get the pan position value */
		Vol *= MasterSoundVolume;
		Vol /= 255;		/* Set the new scaled volume */

		if (Vol>=254) {
			Vol = 0x7FFF7FFF;
		} else {
			Vol = Vol<<7;			/* Convert to HMI format */
			Vol |= (Vol<<16);
		}

		if (MySound->SoundPtr) {	/* Voice active? */
			sosDIGISetSampleVolume(hDIGIDriver,MySound->SampleHandle,Vol);
		}
		MySound->SampleRecord.wVolume = Vol;
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

	Low level routine to shut down a sound
	using my internal audio reference
	Also check if no one else is using the source resource,
	I will then release the memory

**********************************/

static Word BURGERCALL StopIt(PrivSound_t *MySound,void * /* Input */)
{
	if (MySound->SoundPtr) {		/* Is the channel used? */
		Word RezNum;
		Word *CookiePtr;

		sosDIGIStopSample(hDIGIDriver,MySound->SampleHandle);	/* HMI stops the sound */
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

	/* Fill in the EXTRA data (This may be filled in by other code) */

		MySound->CookiePtr = SoundCookiePtr;
		MySound->SampleResource = (Word)-1;		/* No resource assigned */
		MySound->SampleRezHeader = 0;

		FastMemSet(&MySound->SampleRecord,0,sizeof(_SOS_SAMPLE));	/* Clear it */
		MySound->SampleRecord.pSample = (char *)MySound->SoundPtr;
		MySound->SampleRecord.wLength = MySound->SoundLength;
		MySound->SampleRecord.wRate = MySound->SampleRate+SoundFrequencyAdjust;
		MySound->SampleRecord.wPanPosition = MySound->PanPosition;
		i = MySound->Volume*MasterSoundVolume;		/* 0x7FFF is the max */
		if (i>=(255*255)) {
			i = 65535;			/* Force 0x7FFF for max volume in HMI */
		}
		i >>= 1;
		MySound->SampleRecord.wVolume = MK_VOLUME(i,i);
		MySound->SampleRecord.wPriority = 100;
		if (MySound->DataType&SOUNDTYPESTEREO) {		/* Stereo data? */
			i = 2;
		} else {
			i = 1;
		}
		MySound->SampleRecord.wChannels = i;
		if (MySound->LoopEnd) {
			MySound->SampleRecord.wLoopLength = MySound->LoopEnd-MySound->LoopStart;
			MySound->SampleRecord.wLoopCount = (Word)-1;
		}
		i = MySound->DataType&0xFF;		/* Get the data type */
		switch (i) {
		default:
//		case SOUNDTYPEBYTE:
			MySound->SampleRecord.wFormat = _PCM_UNSIGNED;
		case SOUNDTYPECHAR:
			MySound->SampleRecord.wBitsPerSample = 8;
			break;
		case SOUNDTYPEBSHORT:
//			MySound->SampleRecord.wFormat = _PCM_UNSIGNED;
		case SOUNDTYPELSHORT:
			MySound->SampleRecord.wBitsPerSample = 16;
		}
		MySound->SampleHandle = sosDIGIStartSample(hDIGIDriver,&MySound->SampleRecord); /* Play the sound */

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

void BURGERCALL DigitalSoundCheckOff(void * /* Input */)
{
	Word i,Max;
	PrivSound_t *MySound;

	Max = BurgerVoiceCount;			/* Any voices active? */
	if (Max) {
		MySound = BurgerSamples;	/* Get the array */
		i = 0;			/* Release any sounds that have ended normally */
		do {
			if (MySound->SoundPtr) {		/* Is the voice active? */
				if (!IsASoundPlaying(i|SOUND_COOKIE)) {		/* In use? */
					StopASound(i|SOUND_COOKIE);		/* Release the voice */
				}
			}
			++MySound;		/* Next */
		} while (++i<Max);	/* All scanned? */
	}
}

/**********************************

	Generic routine to start up the digital sound driver

**********************************/

Word BURGERCALL InitDigitalDriver(void)
{
	Word wDIGIDeviceID;

	if (BurgerSndExitIn & (DIGIMUSICON|DIGISOUNDON)) {		/* Already on? */
		return TRUE;		/* Don't start again */
	}

	HMIInitSoundTimer();	/* Init the timer system */

	{
		Word8 *FileData;
		Word32 FileLength;
		Word OldFlag;
		OldFlag = SetErrBombFlag(FALSE);	/* Don't bomb */
		FileData = (Word8 *)LoadAFile("Sound.cfg",&FileLength);
		SetErrBombFlag(OldFlag);			/* Restore the flag */

		FastMemSet(&sDIGIDriver,0,sizeof(sDIGIDriver));

		if (!FileData) {			/* No config file */
			NonFatal("Can't find the file Sound.cfg\n");
			return FALSE;
		}
		wDIGIDeviceID = LongWordFromIniImage(MIDIHeader,"DeviceID",(char *)FileData,FileLength);
		sDIGIDriver.sHardware.wPort = LongWordFromIniImage(MIDIHeader,"DevicePort",(char *)FileData,FileLength);
		sDIGIDriver.sHardware.wDMA = LongWordFromIniImage(MIDIHeader,"DeviceDMA",(char *)FileData,FileLength);
		sDIGIDriver.sHardware.wIRQ = LongWordFromIniImage(MIDIHeader,"DeviceIRQ",(char *)FileData,FileLength);
		DeallocAPointer(FileData);		/* Release the file information */

		if (wDIGIDeviceID==-1 || !wDIGIDeviceID) {
			return FALSE;
		}
	}

	/* initialize the digital and midi systems */

	sosDIGIInitSystem(0,_SOS_DEBUG_NORMAL);

	/* set up the digital driver */

	sDIGIDriver.wDriverRate = 22050;		/* Sample rate */
	sDIGIDriver.wDMABufferSize = 0x1000;	/* Size of buffer */
	sDIGIDriver.wID = wDIGIDeviceID;
	/* initialize the digital driver if needed */

	if (sosDIGIInitDriver(&sDIGIDriver,&hDIGIDriver)) {
		NonFatal("Can't start the digital driver!\n");
		return FALSE;
	}

	/* register digital timer event (mixer) */

	sosTIMERRegisterEvent(120,sDIGIDriver.pfnMixFunction,&hDIGITimer);

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
	if (hDIGITimer!=-1) {		/* Timer already shut down? */
		sosTIMERRemoveEvent(hDIGITimer);	/* Remove the digital timer */
		hDIGITimer = -1;
	}
	if (hDIGIDriver!=-1) {		/* Digital timer shut down? */
		sosDIGIUnInitDriver(hDIGIDriver,TRUE,TRUE);	/* Release buffer, Driver */
		hDIGIDriver = (HANDLE)-1;
	}
	sosDIGIUnInitSystem();		/* Shut down the digital system */
}

#endif
