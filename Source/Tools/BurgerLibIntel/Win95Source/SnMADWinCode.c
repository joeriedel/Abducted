/**********************************

	Win95 specific MOD file code

**********************************/

#include "SnSound.h"
#if defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN
#define DIRECTSOUND_VERSION 0x700
#include "MmMemory.h"
#include "SnMadMusic.h"
#include "McMac.h"
#include "FmFile.h"
#include "ClStdLib.h"
#include "StString.h"
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

static UINT gwID;				/* ID for the audio thread */
static char *currentBuf;		/* Temp work buffer for audio before uploading */
static Word WhichBuffer;		/* Which buffer am I uploading to (0 or 1) */
static Word32 WIN95BUFFERSIZE;	/* Size of the temp work buffer */

/**********************************

	Save data in small chunks for DirectSound support
	This is called by my audio thread

**********************************/

static Word DirectSave(char *myPtr,MADDriverRec *intDriver)
{
	char *ptrCopy;

	if (intDriver && intDriver->Reading) {		/* Am I reading data right now? */
		/*** Copy values ***/
		ptrCopy = intDriver->IntDataPtr;

		/*** Install New Values ***/

		intDriver->IntDataPtr = myPtr;
		NoteAnalyse( intDriver);
		intDriver->IntDataPtr = ptrCopy;
		if (!intDriver->curMusic || (intDriver->PL < intDriver->curMusic->header->numPointers)) {
			return TRUE;		/* The data is good */
		}
	}
	return FALSE;		/* Bad data! */
}

/**********************************

	Upload data into the directsoundbuffer record
	Remember, NEVER calculate data and upload at the same time. You must
	keep your audio buffer locks to a minimum of time, which means
	create data, then blast as fast as you can with a memcpy

**********************************/

static Word WriteDataToBuffer(LPDIRECTSOUNDBUFFER lpDsb,Word32 dwOffset,Word8 *lpbSoundData,DWORD dwSoundBytes)
{
	void *lpvPtr1;			/* First buffer */
	DWORD dwBytes1;		/* Size */
	void *lpvPtr2;			/* Second buffer */
	DWORD dwBytes2;		/* Size of second buffer */
	HRESULT hr;				/* Error code if any */

	// Obtain write pointer.
	hr = IDirectSoundBuffer_Lock(lpDsb, dwOffset, dwSoundBytes, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);

	// If we got DSERR_BUFFERLOST, restore and retry lock.
	if (DSERR_BUFFERLOST == hr) {
		IDirectSoundBuffer_Restore(lpDsb);			/* Create the buffer again */
		hr = IDirectSoundBuffer_Lock(lpDsb, dwOffset, dwSoundBytes, &lpvPtr1,	/* Try to lock again */
			&dwBytes1, &lpvPtr2, &dwBytes2, 0);
	}
	if (DS_OK == hr) {
		// Write to pointers.
		FastMemCpy(lpvPtr1,lpbSoundData,dwBytes1);
		if (lpvPtr2) {			/* Is there a second buffer */
			FastMemCpy(lpvPtr2,lpbSoundData+dwBytes1, dwBytes2);
		}
		// Release the data back to DirectSound.
		hr = IDirectSoundBuffer_Unlock(lpDsb, lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
		if (DS_OK == hr) {
			return TRUE;		/* Yeah! */
		}
	}
	return FALSE;		/* I am screwed */
}

/**********************************

	Create my output sound buffer

**********************************/

static Word LoadSamp(LPDIRECTSOUNDBUFFER *lplpDsb,
	Word8 * /*samp*/,Word32 length,Word /*flags*/, MADDriverRec *WinMADDriver)
{
    DSBUFFERDESC dsbdesc;
    HRESULT hr;
    WAVEFORMATEX pcmwf;

    // Set up wave format structure.
    FastMemSet(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
    pcmwf.wFormatTag = WAVE_FORMAT_PCM;
    pcmwf.nChannels = 2;
    pcmwf.nSamplesPerSec = WinMADDriver->DriverSettings.outPutRate >> 16L;
    pcmwf.wBitsPerSample = WinMADDriver->DriverSettings.outPutBits;
    pcmwf.nBlockAlign = pcmwf.nChannels * (pcmwf.wBitsPerSample/8);
    pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;

	// Set up DSBUFFERDESC structure.
	FastMemSet(&dsbdesc, 0, sizeof(DSBUFFERDESC)); // Zero it out.
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);

	dsbdesc.dwFlags = DSBCAPS_CTRLPAN|DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLFREQUENCY | DSBCAPS_GLOBALFOCUS;
	dsbdesc.dwBufferBytes = length;
	dsbdesc.dwReserved = 0;
	dsbdesc.lpwfxFormat = &pcmwf;
	hr = IDirectSound_CreateSoundBuffer(MyDirectSoundDevice, &dsbdesc, lplpDsb, NULL);
	if(hr != DS_OK) {
		*lplpDsb=0;			/* Loser! */
		return FALSE;
	}
	return TRUE;			/* Success! */
}

/**********************************

	This is my main thread is is called every 40ms to upload data
	as needed to the sound buffer

**********************************/

static volatile int timersema=0;		/* Must be atomic! */

static void CALLBACK TimeProc(Word /*IDEvent*/,Word /*uReserved*/,DWORD dwUser,
	DWORD /*dwReserved1*/,DWORD /*dwReserved2*/)
{
	DWORD pos;
	DWORD posp;

	/* use semaphore to prevent entering the mixing routines twice.. do we need this ? */

	if(++timersema==1) {
		MADDriverRec *WinMADDriver = (MADDriverRec*) dwUser;

		IDirectSoundBuffer_GetCurrentPosition( WinMADDriver->lpSwSamp, &pos, &posp);

		if (WhichBuffer && pos >= (WIN95BUFFERSIZE/2)) {		/* Time to upload? */
			WhichBuffer = 0;

			if( !DirectSave( currentBuf, WinMADDriver)) {
				switch( WinMADDriver->DriverSettings.outPutBits) {
				case 8:
					FastMemSet(currentBuf,0x80,WIN95BUFFERSIZE/2);
					break;
				case 16:
					FastMemSet(currentBuf,0,WIN95BUFFERSIZE/2);
					break;
				}
			}
			WriteDataToBuffer( WinMADDriver->lpSwSamp, 0, (Word8*) currentBuf, WIN95BUFFERSIZE/2);
		} else if (!WhichBuffer && (pos < (WIN95BUFFERSIZE/2))) {
			WhichBuffer = 1;

			if (!DirectSave( currentBuf + WIN95BUFFERSIZE/2, WinMADDriver)) {
				switch( WinMADDriver->DriverSettings.outPutBits) {
				case 8:
					FastMemSet(currentBuf+(WIN95BUFFERSIZE/2),0x80,WIN95BUFFERSIZE/2);
					break;

				case 16:
					FastMemSet(currentBuf+(WIN95BUFFERSIZE/2),0,WIN95BUFFERSIZE/2);
					break;
				}
			}
			WriteDataToBuffer( WinMADDriver->lpSwSamp, WIN95BUFFERSIZE/2, (Word8*) (currentBuf + WIN95BUFFERSIZE/2), WIN95BUFFERSIZE/2);
		}
	}
	timersema--;
}

/**********************************

	Start the sound system
	(I create a thread that monitors the main
	buffer and plays music async)

**********************************/

int BURGERCALL MADSndOpen( MADDriverRec *WinMADDriver)
{
	WhichBuffer = 0;		/* Upload to buffer #0 */

	if (MyDirectSoundDevice) {
		WIN95BUFFERSIZE = WinMADDriver->BufSize*2;		/* Double buffer */
		currentBuf = static_cast<char *>(AllocAPointerClear(WIN95BUFFERSIZE));
		if (currentBuf) {
			WinMADDriver->lpSwSamp = 0;
			if (LoadSamp(&WinMADDriver->lpSwSamp, 0L, WIN95BUFFERSIZE, DSBCAPS_LOCSOFTWARE, WinMADDriver)) {
				if (WinMADDriver->lpSwSamp) {
					IDirectSoundBuffer_Play(WinMADDriver->lpSwSamp, 0, 0, DSBPLAY_LOOPING);

					timeBeginPeriod(20);      /* set the minimum resolution */

					/*  Set up the callback event.  The callback function
					 *  MUST be in a FIXED CODE DLL!!! -> not in Win95  */

					gwID = timeSetEvent(40,40,TimeProc,(DWORD) WinMADDriver,TIME_PERIODIC);
					if (gwID) {
						return FALSE;		/* I am good to go! */
					}
				}
			}
			DeallocAPointer(currentBuf);
			currentBuf = 0;
		}
	}
	return -1;			/* You are screwed */
}

/**********************************

	Shut down audio hardware

**********************************/

void BURGERCALL MADSndClose(MADDriverRec *WinMADDriver)
{
	/* stop the timer */
	if (gwID) {
		timeKillEvent( gwID);				/* Release the timer thread */
		timeEndPeriod( 20);					/* Dispose of the event routine */
		gwID = 0;
	}

	if (WinMADDriver->lpSwSamp) {			/* Was a DirectSound buffer allocated? */
		IDirectSoundBuffer_Stop(WinMADDriver->lpSwSamp);		/* Stop audio output */
		IDirectSoundBuffer_Release( WinMADDriver->lpSwSamp);	/* Kill the buffer */
		WinMADDriver->lpSwSamp = 0L;		/* Gone! */
	}
	DeallocAPointer(currentBuf);
	currentBuf = 0;
}

/**********************************

	Start audio

**********************************/

void BURGERCALL MADStartDriver( MADDriverRec *MDriver)
{
	MDriver->MADPlay = TRUE;
	MDriver->Reading = FALSE;

	MADCleanDriver( MDriver);

	MADCheckSpeed( MDriver->curMusic, MDriver);
}

/**********************************

	Stop audio

**********************************/

void BURGERCALL MADStopDriver( MADDriverRec *MDriver)
{
	MDriver->MADPlay = FALSE;
	MADCleanDriver( MDriver);
}


/**********************************

	Get the driver defaults based on the system you are in

**********************************/

void BURGERCALL MADGetBestDriver( MADDriverSettings *Init)
{
	Init->numChn = 4;
	Init->repeatMusic = TRUE;
	Init->surround = FALSE;
	Init->Reverb = FALSE;
	Init->TickRemover = FALSE;
	Init->MicroDelaySize = 25;
	Init->ReverbSize = 100;
	Init->ReverbStrength = 20;
	Init->outPutBits = 16;
	Init->outPutRate = 44100U<<16;
	Init->driverMode = DirectSound95NT;
}

#endif
