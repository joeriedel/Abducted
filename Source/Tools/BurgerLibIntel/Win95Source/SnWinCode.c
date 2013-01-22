/**********************************

	This code is ONLY for the Win95 version of
	Burgerlib Audio. I use the DirectSound for all
	of my sound needs

**********************************/

#include "SnSound.h"

#if defined(__WIN32__)
#include <BREndian.hpp>
#include "RzRez.h"
#include "ClStdLib.h"
#include "W9Win95.h"
#include "InInput.h"
#define WIN32_LEAN_AND_MEAN
#define DIRECTDRAW_VERSION 0x700
#define DIRECTSOUND_VERSION 0x700
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <ddraw.h>

typedef HRESULT (WINAPI *DIRECTSOUNDCREATE)(GUID *, LPDIRECTSOUND *, IUnknown *);

static HINSTANCE DirectSoundInstance;			/* Reference to DSOUND.DLL */
static DIRECTSOUNDCREATE DirectSoundProcPtr;	/* Pointer to direct draw */

struct IDirectSound *MyDirectSoundDevice;
struct IDirectSoundBuffer *MyDirectSoundBuffer;

/**********************************

	Returns true or false is a sound effect is playing

**********************************/

static Word BURGERCALL IsIt(PrivSound_t *MySound,void * /*Input*/)
{
	if (MySound->SoundPtr) {		/* Is there a pointer? */
		DWORD Status;
		if (!IDirectSoundBuffer_GetStatus(MySound->SampleHandle,&Status)) {
			if (Status & (DSBSTATUS_PLAYING | DSBSTATUS_LOOPING)) {
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

static Word BURGERCALL SetFrq(PrivSound_t *MySound,void *Input)
{
	Word Frq;
	Frq = (Word)Input;
	if (MySound->SampleRate!=Frq) {
		MySound->SampleRate = Frq;	/* Get the pan position value */

		if (MySound->SoundPtr) {
			IDirectSoundBuffer_SetFrequency(MySound->SampleHandle,Frq);
		}
	}
	return FALSE;
}

void BURGERCALL SetASoundFrequency(Word32 SoundNum,Word Frequency)
{
	SoundIterator(SetFrq,(void*)Frequency,SoundNum,FALSE);	/* Handle it */
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

		if (MySound->SoundPtr) {
			int WinPan;
			if (Pan<0x8000) {
				WinPan = Win95DirectSoundVolumes[Pan>>7];
			} else {
				WinPan = -Win95DirectSoundVolumes[(0xFFFF-Pan)>>7];
			}
			IDirectSoundBuffer_SetPan(MySound->SampleHandle,WinPan);
		}
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
	Vol = (Word32)Input;
	if (MySound->Volume!=Vol) {
		MySound->Volume = Vol;	/* Get the pan position value */
		Vol *= MasterSoundVolume;
		Vol /= 255;		/* Set the new scaled volume */

		if (MySound->SoundPtr) {
			IDirectSoundBuffer_SetVolume(MySound->SampleHandle,Win95DirectSoundVolumes[Vol]);
		}
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

static Word BURGERCALL StopIt(PrivSound_t *MySound,void * /*Input*/)
{
	if (MySound->SoundPtr) {		/* Is the channel used? */
		Word RezNum;
		Word *CookiePtr;

		IDirectSoundBuffer_Stop(MySound->SampleHandle);	/* Make sure it's stopped */
		IDirectSoundBuffer_Release(MySound->SampleHandle);	/* Release the sound */
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

	Upload sound data and convert if needed

**********************************/

static void CopySoundData(Word8 *DestPtr,const Word8 *SrcPtr,Word32 Length,Word Type)
{
	if (Length) {
		Type = Type&0xFF;
		switch (Type) {
		case SOUNDTYPEBYTE:
		case SOUNDTYPELSHORT:
			FastMemCpy(DestPtr,SrcPtr,Length);
			break;
		case SOUNDTYPECHAR:
			do {
				DestPtr[0] = SrcPtr[0]^0x80;
				++SrcPtr;
				++DestPtr;
			} while (--Length);
			break;
		case SOUNDTYPEBSHORT:
			Length>>=1;
			if (Length) {
				do {
					((Word16 *)DestPtr)[0] = Burger::SwapEndian(((Word16 *)SrcPtr)[0]);
					SrcPtr+=2;
					DestPtr+=2;
				} while (--Length);
			}
		}
	}
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

		/* Here is the MACHINE specific code */
		/* Your mileage may vary */

/**********************************

	Direct Sound for Windows 95

**********************************/

		{
			DSBUFFERDESC bufferStats;

			FastMemSet(&bufferStats,0,sizeof(bufferStats));

			bufferStats.dwSize = sizeof (bufferStats);
			bufferStats.dwFlags = DSBCAPS_LOCSOFTWARE|DSBCAPS_CTRLPAN|DSBCAPS_CTRLFREQUENCY|DSBCAPS_CTRLVOLUME|DSBCAPS_STATIC|DSBCAPS_GETCURRENTPOSITION2;
			bufferStats.dwBufferBytes = MySound->SoundLength;
//			bufferStats.dwReserved = 0;
			bufferStats.lpwfxFormat = (WAVEFORMATEX *)&MySound->SampleRecord;

			MySound->SampleRecord.wFormatTag = WAVE_FORMAT_PCM;
			if (MySound->DataType&SOUNDTYPESTEREO) {		/* Stereo data? */
				j = 2;
			} else {
				j = 1;
			}
			MySound->SampleRecord.nChannels = static_cast<Word16>(j);
			MySound->SampleRecord.nSamplesPerSec = MySound->SampleRate+SoundFrequencyAdjust;

			i = MySound->DataType&0xFF;		/* Get the data type */
			switch (i) {
			default:
//			case SOUNDTYPEBYTE:
//			case SOUNDTYPECHAR:
				MySound->SampleRecord.wBitsPerSample = 8;
				break;
			case SOUNDTYPELSHORT:
			case SOUNDTYPEBSHORT:
				MySound->SampleRecord.wBitsPerSample = 16;
			}

			MySound->SampleRecord.nBlockAlign = (MySound->SampleRecord.wBitsPerSample/8)*MySound->SampleRecord.nChannels;
			MySound->SampleRecord.nAvgBytesPerSec = MySound->SampleRecord.nSamplesPerSec*
				MySound->SampleRecord.nBlockAlign;
//			MySound->SampleRecord.cbSize = 0;

			if (!IDirectSound_CreateSoundBuffer(MyDirectSoundDevice,&bufferStats,&MySound->SampleHandle,NULL)) {
				Word8 *FooPtr1,*FooPtr2;
				DWORD FooLength1,FooLength2;

				if (!IDirectSoundBuffer_Lock(MySound->SampleHandle,0,
					MySound->SoundLength,
					(void **)&FooPtr1,&FooLength1,
					(void **)&FooPtr2,&FooLength2,
					0)) {
					int WinPan;

					CopySoundData(FooPtr1,MySound->SoundPtr,FooLength1,MySound->DataType);

					if (FooPtr2 && FooLength2) {
						CopySoundData(FooPtr2,MySound->SoundPtr+FooLength1,FooLength2,MySound->DataType);
					}
					IDirectSoundBuffer_Unlock(MySound->SampleHandle,FooPtr1,
					FooLength1,FooPtr2,FooLength2);
					IDirectSoundBuffer_SetVolume(MySound->SampleHandle,Win95DirectSoundVolumes[(MySound->Volume*MasterSoundVolume)/255]);
					if (MySound->PanPosition<0x8000) {
						WinPan = Win95DirectSoundVolumes[MySound->PanPosition>>7];
					} else {
						WinPan = -Win95DirectSoundVolumes[(0xFFFF-MySound->PanPosition)>>7];
					}
					IDirectSoundBuffer_SetPan(MySound->SampleHandle,WinPan);
					i = 0;
					if (MySound->LoopEnd) {
						i = DSBPLAY_LOOPING;
					}
					IDirectSoundBuffer_Play(MySound->SampleHandle,0,0,i);
					goto GoodSnd;
				}
				IDirectSoundBuffer_Release(MySound->SampleHandle);
			}
			MySound->SoundPtr = 0;		/* Error!! */
		}
GoodSnd:;

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

void BURGERCALL DigitalSoundCheckOff(void * /*Input*/)
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

	Call DirectSoundCreate.
	I will first manually load DirectSoundCreate
	and then call it if I find it.
	Note : I do NOT ever release DirectSound, since
	I use the properties forever...

**********************************/

static HRESULT BURGERCALL CallDirectSoundCreate(GUID FAR *lpGUID, LPDIRECTSOUND FAR *lplpDS, IUnknown FAR *pUnkOuter)
{
	if (DirectSoundProcPtr) {			/* Already loaded? */
		return DirectSoundProcPtr(lpGUID,lplpDS,pUnkOuter);	/* Call it */
	}

	/* Let's load it in! */

	if (!DirectSoundInstance) {
		DirectSoundInstance = LoadLibrary("DSOUND.DLL");	/* Get the library */
	}
	if (DirectSoundInstance) {		/* Did it load? */
		DirectSoundProcPtr = (DIRECTSOUNDCREATE)GetProcAddress(DirectSoundInstance,"DirectSoundCreate");
		if (DirectSoundProcPtr) {		/* Did I find the proc? */
			return DirectSoundProcPtr(lpGUID,lplpDS,pUnkOuter);	/* Call it */
		}
	}
	return DDERR_NOTINITIALIZED;
}

/**********************************

	Generic routine to start up the digital sound driver
	(Direct Sound version)

**********************************/

Word BURGERCALL InitDigitalDriver(void)
{
	DSBUFFERDESC bufferStats;
	WAVEFORMATEX bufferFormat;
	DWORD bytesWritten;
	const char *ErrorText;
	HWND MyWindow;

	if (BurgerSndExitIn & (DIGIMUSICON|DIGISOUNDON)) {		/* Already on? */
		return TRUE;		/* Don't start again */
	}

	// Open the sound device by creating an DirectSound object

	if (!CallDirectSoundCreate(NULL,&MyDirectSoundDevice, NULL)) {
	
		/* In case I don't have a game window, I'll take the frontmost active window */
		/* If one doesn't exist, then you are so screwed!!! */
		
		MyWindow = (HWND)Win95MainWindow;
		if (!MyWindow) {
			MyWindow = GetActiveWindow();
		}

		// Set lowest level of cooperation
		if (!IDirectSound_SetCooperativeLevel(MyDirectSoundDevice,MyWindow,DSSCL_PRIORITY)) {
			goto Cool;
		}
		if (!IDirectSound_SetCooperativeLevel(MyDirectSoundDevice,MyWindow,DSSCL_NORMAL)) {
Cool:;
			// Create a primary buffer so that we can change its sample rate attribute

			FastMemSet(&bufferStats,0,sizeof(bufferStats));
			bufferStats.dwSize = sizeof (bufferStats);
//			bufferStats.dwBufferBytes = 0;
//			bufferStats.lpwfxFormat = NULL;
			bufferStats.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;

			// Allocate the composite buffer

			if (!IDirectSound_CreateSoundBuffer(MyDirectSoundDevice,&bufferStats,&MyDirectSoundBuffer,NULL)) {
				goto Great1;
			}

			// Let's try again but no sound volume control

			FastMemSet(&bufferStats,0,sizeof(bufferStats));
			bufferStats.dwSize = sizeof(bufferStats);
//			bufferStats.dwBufferBytes = 0;
//			bufferStats.lpwfxFormat = NULL;
			bufferStats.dwFlags = DSBCAPS_PRIMARYBUFFER;
			if (!IDirectSound_CreateSoundBuffer(MyDirectSoundDevice,&bufferStats,&MyDirectSoundBuffer,NULL)) {
Great1:;
			// Get current format
				FastMemSet(&bufferFormat,0,sizeof(bufferFormat));
				if (!IDirectSoundBuffer_GetFormat(MyDirectSoundBuffer, &bufferFormat, sizeof (bufferFormat), &bytesWritten) ) {
					bufferFormat.wFormatTag = WAVE_FORMAT_PCM;
					bufferFormat.nChannels = 2;
					bufferFormat.nSamplesPerSec = 22050;
					bufferFormat.wBitsPerSample = 16;
					bufferFormat.nBlockAlign = (16 / 8) * 2;
					bufferFormat.nAvgBytesPerSec = bufferFormat.nSamplesPerSec * bufferFormat.nBlockAlign;

					// Set the new format, but don't die if was unable to take

					IDirectSoundBuffer_SetFormat (MyDirectSoundBuffer,&bufferFormat);
					KeyboardAddRoutine(DigitalSoundCheckOff,0);
					return TRUE;
				}
				ErrorText = "Could not GetFormat for the sound buffer";
			} else {
				ErrorText = "Could not create a primary sound buffer";
			}
		} else {
			ErrorText = "Direct sound could not set the priority";
		}
		IDirectSound_Release(MyDirectSoundDevice);
		MyDirectSoundDevice = 0;
	} else {
		ErrorText = "Direct sound could not be started";
	}
	DebugMessage("%s\n",ErrorText);
	{
		char OhGreat[256];
		strcpy(OhGreat,ErrorText);
		strcat(OhGreat,", sound is disabled");
		OkAlertMessage("Direct sound error",OhGreat);
		strcpy(ErrorMsg,ErrorText);
	}
	return FALSE;
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
	if (MyDirectSoundDevice) {	/* Was it initialized? */
		IDirectSound_Release(MyDirectSoundDevice);	/* Bye bye */
		MyDirectSoundDevice = 0;
	}
	if (DirectSoundInstance) {
		FreeLibrary(DirectSoundInstance);
		DirectSoundInstance = 0;
	}
}

#endif
