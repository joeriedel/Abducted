/**********************************

	Win 95 version of streaming audio

**********************************/

#include "SnSound.h"
#if defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN
#define DIRECTSOUND_VERSION 0x700

#include "MmMemory.h"
#include "FmFile.h"
#include "InInput.h"
#include "ClStdLib.h"
#include "RsReadStream.h"
#include "TkTick.h"
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

#define BUFFERSIZESHIFT 12	/* Buffer size as a shift value */
#define BUFFERSIZE (1<<BUFFERSIZESHIFT)		/* Size of the audio buffer */
#define RINGCOUNT 32						/* Number of chunks in the file buffer */
#define FILEBUFFER (BUFFERSIZE*RINGCOUNT)	/* Size of the audio ring buffer */

#define BIGBUFFERSIZE (FILEBUFFER)	/* My work buffer */

typedef struct DigitalMusic_t {
	DigitalMusicReadState_t ReadState;	/* Current audio state */

	/* For the sound channel */
	
	IDirectSoundBuffer *SongSampleHandle;	/* Handle of the sound buffer */
	Word CurrentIndex;			/* Current block being unpacked (Can only be modified by the sound callback) */
	Word EndIndex;				/* Ending block for the ring buffer (Can only be modifed by the data loader) */
	
	/* For the loader */
	
	struct ReadFileStream_t *FileStreamPtr;	/* Instance of the disk loader */
	Word8 *FileBuffer;					/* Main disk buffer */
	Word32 WorkOffset;				/* Pointer to write my unpacked buffer */
	Word State;							/* Current stream state */
	Word FirstTime;						/* TRUE if the sound channel was not started yet */
} DigitalMusic_t;

#define READMODESLEEP 0		/* Buffer is full, no actions pending */
#define READMODESEEK 1		/* Waiting for a seek */
#define READMODEREAD 2		/* Just read in data */
#define READMODEDONE 3		/* No more actions pending */

#define DORMANT 0
#define STARTING 1
#define READING 2

static DigitalMusic_t MyState = {{0}};		/* My local memory */

/**********************************

	Called from the sound manager, this will set up the next sound buffer
	and it MIGHT go dormant if nothing is left

**********************************/

static void DigitalMusicSoundProc(DigitalMusic_t *LocalPtr)
{
	Word Index;
	LPDIRECTSOUNDBUFFER MySample;		/* Direct sound buffer */
	void *lpbuf1;
	void *lpbuf2;
	DWORD dwsize1;
	DWORD dwsize2;

	LocalPtr = &MyState;			/* Load the current work buffer */

	/* If this is the FIRST time I am uploading, then I will */
	/* Prime the pump with about 1/2 second of audio */

	MySample = LocalPtr->SongSampleHandle;
	if (MySample) {
		if (LocalPtr->FirstTime) {
			if (LocalPtr->EndIndex<(RINGCOUNT/2)) {
				return;
			}
			if (IDirectSoundBuffer_Lock(MySample,0,BUFFERSIZE*(RINGCOUNT/2),&lpbuf1,&dwsize1,&lpbuf2,&dwsize2,0)==DS_OK) {
				Word8 *BufferPtr2;
				BufferPtr2 = LocalPtr->FileBuffer;
				FastMemCpy(lpbuf1,BufferPtr2,dwsize1);
			// Second write required?
				if (lpbuf2 && dwsize2) {
					FastMemCpy(lpbuf2,BufferPtr2+dwsize1,dwsize2);
				}
				// Update our buffer offset and unlock sound buffer
				IDirectSoundBuffer_Unlock(MySample,lpbuf1,dwsize1,lpbuf2,dwsize2);
			}
			LocalPtr->CurrentIndex = RINGCOUNT/2;		/* Set the start point */
			IDirectSoundBuffer_SetVolume(MySample,Win95DirectSoundVolumes[MusicVolume]);
			IDirectSoundBuffer_SetFrequency(MySample,BurgerSongFreq);
			IDirectSoundBuffer_Play(MySample,0,0,DSBPLAY_LOOPING);
			LocalPtr->FirstTime = FALSE;		/* Now I am playing... */
		}

		/* Is there any data pending? */
		Index = LocalPtr->CurrentIndex;
		if (Index != LocalPtr->EndIndex) {		/* Is there data available? */
			DWORD dwPlayCursor;		/* Where is the cursor? */
			DWORD dwWriteCursor;		/* Not used */

			if (IDirectSoundBuffer_GetCurrentPosition(MySample,&dwPlayCursor,&dwWriteCursor)==DS_OK) {
				Word LastIndex;
				/* This is the block that I cannot write to */
				LastIndex = dwPlayCursor>>BUFFERSIZESHIFT;
				if (LastIndex!=Index) {
					do {
						Word8 *BufferPtr1;
		
						// Write data to sound buffer. Because the sound buffer is circular, we may have to
						// do two write operations if locked portion of buffer wraps around to start of buffer.

						BufferPtr1 = &LocalPtr->FileBuffer[Index*BUFFERSIZE];
						if (IDirectSoundBuffer_Lock(MySample,Index*BUFFERSIZE,BUFFERSIZE,&lpbuf1,&dwsize1,&lpbuf2,&dwsize2,0)==DS_OK) {
							FastMemCpy(lpbuf1,BufferPtr1,dwsize1);
						// Second write required?
							if (lpbuf2 && dwsize2) {
								FastMemCpy(lpbuf2,BufferPtr1+dwsize1,dwsize2);
							}
							// Update our buffer offset and unlock sound buffer
							IDirectSoundBuffer_Unlock(MySample,lpbuf1,dwsize1,lpbuf2,dwsize2);
						}	
						Index = (Index+1)&(RINGCOUNT-1);		/* Accept the data */
						if (Index==LastIndex) {
							break;
						}
					} while (Index != LocalPtr->EndIndex);
					LocalPtr->CurrentIndex = Index;
				}
			}
		}
	}
}
/**********************************

	This proc actually reads from the data stream
	It is called from the data codec

**********************************/

static Word32 BURGERCALL DigitalMusicReadProc(void *Input,Word8 *DestPtr,Word32 Count)
{
	DigitalMusic_t *LocalPtr;
	Word8 *WorkPtr;
	Word32 OutSize;
	
	LocalPtr = (DigitalMusic_t*)Input;
	
	/* Is there data to accept? */
	
	WorkPtr = ReadFileStreamGetData(LocalPtr->FileStreamPtr,&OutSize,Count);
	if (WorkPtr) {
		FastMemCpy(DestPtr,WorkPtr,OutSize);
		return OutSize;		/* Return the data actually read */
	}
	
	/* Am I still live? */
	
	if (ReadFileStreamActive(LocalPtr->FileStreamPtr)) {
		return 0;		/* Ok, I guess I'm waiting for data then */
	}
	
	/* Since I'm done, let's make one more attempt (Fixes a race condition) */
	
	WorkPtr = ReadFileStreamGetData(LocalPtr->FileStreamPtr,&OutSize,Count);
	if (WorkPtr) {
		FastMemCpy(DestPtr,WorkPtr,OutSize);
		return OutSize;
	}
	
	/* Data is done */

	return (Word)-1;
}
/**********************************

	This code is a foreground task to fill all audio buffers with OGG/Vorbis
	compressed data. You see, Ogg/Vorbis can allocate memory at runtime and
	as a result, I need foreground CPU time to call the memory manager

**********************************/

static void BURGERCALL DigitalMusicForegroundTask(void *Input)
{
	Word i,j;
	DigitalMusic_t *LocalPtr;

	LocalPtr = (DigitalMusic_t *)Input;
	if (LocalPtr->State==STARTING) {
		Word8 *HeaderPtr;
		Word32 OutSize;
		HeaderPtr = ReadFileStreamGetData(LocalPtr->FileStreamPtr,&OutSize,4096);
		if (HeaderPtr && OutSize==4096) {
			RawSound_t Raw;
			WAVEFORMATEX WaveData;
			DSBUFFERDESC bufferStats;
			
			if (!ParseSoundFileImage(&Raw,HeaderPtr,4096)) {
				FastMemSet(&WaveData,0,sizeof(WaveData));
				WaveData.wFormatTag = WAVE_FORMAT_PCM;
				/* Now fill in all the records for the double buffer record */
				i = Raw.DataType;		/* Get the data type */
				j = 1;
				if (i&SOUNDTYPESTEREO) {	/* Stereo data? */
					j = 2;
				}
				WaveData.nChannels = static_cast<Word16>(j);		/* Stereo or mono */
				WaveData.nSamplesPerSec = Raw.SampleRate;
				BurgerSongFreq = Raw.SampleRate;
				
				i = DigitalMusicReadStateInit(&LocalPtr->ReadState,&Raw,HeaderPtr,FILEBUFFER,DigitalMusicReadProc,LocalPtr);
				switch (i&0xFF) {		/* Type of data */
				default:
//				case SOUNDTYPEBYTE:		/* Unsigned byte */
//				case SOUNDTYPECHAR:		/* Signed byte */
					WaveData.wBitsPerSample = 8;
					break;
				case SOUNDTYPELSHORT:	/* Little endian signed short */
				case SOUNDTYPEBSHORT:	/* Big endian signed short */
					WaveData.wBitsPerSample = 16;
				}
				WaveData.nBlockAlign = (WaveData.wBitsPerSample/8)*WaveData.nChannels;
				WaveData.nAvgBytesPerSec = WaveData.nSamplesPerSec*WaveData.nBlockAlign;
				WaveData.cbSize = 0;
				ReadFileStreamStop(LocalPtr->FileStreamPtr);
				ReadFileStreamStart(LocalPtr->FileStreamPtr,LocalPtr->ReadState.FileOffset);

				// Create sound buffer
				FastMemSet(&bufferStats,0,sizeof(DSBUFFERDESC));
				bufferStats.dwSize = sizeof(DSBUFFERDESC);
				bufferStats.dwFlags = DSBCAPS_LOCSOFTWARE|DSBCAPS_CTRLPAN|DSBCAPS_CTRLFREQUENCY|DSBCAPS_CTRLVOLUME|DSBCAPS_STATIC|DSBCAPS_GETCURRENTPOSITION2;
				bufferStats.dwBufferBytes = BIGBUFFERSIZE;
				bufferStats.lpwfxFormat = &WaveData;
				
				if (!MyDirectSoundDevice || IDirectSound_CreateSoundBuffer(MyDirectSoundDevice,&bufferStats,&LocalPtr->SongSampleHandle,NULL)!=DS_OK) {
					LocalPtr->State = READMODEDONE;		/* I surrender */
					return;
				}
				LocalPtr->FirstTime = TRUE;			/* Pending start */
				
				DigitalMusicReset(&LocalPtr->ReadState);		/* Loop next time */
				LocalPtr->State = READING;

				IDirectSoundBuffer_SetVolume(LocalPtr->SongSampleHandle,Win95DirectSoundVolumes[MusicVolume]);
				LocalPtr->WorkOffset = 0;			/* Reset my variables */
				LocalPtr->CurrentIndex = 0;
				LocalPtr->EndIndex = 0;
			}
		}
	}
	
	/* Now, fill the audio buffers */
	
	if (LocalPtr->State==READING) {
		Word Counter;
		Counter = RINGCOUNT/2;		/* Maximum passes for reading (Prevent locks) */
		do {
			Word ChunkSize;
			Word8 *DestPtr;
			Word Result;

			i = LocalPtr->EndIndex;			/* Write mark */
			j = (i+1)&(RINGCOUNT-1);		/* NEXT index */
			
			/* Check if I am full */
			
			if (j==LocalPtr->CurrentIndex) {
				break;				/* The buffer is full! */
			}
			
			/* Ok, let's decode audio data */
			
			ChunkSize = BUFFERSIZE-LocalPtr->WorkOffset;		/* Number of bytes to read (Could be a partial) */
			DestPtr = &LocalPtr->FileBuffer[(i*BUFFERSIZE)+LocalPtr->WorkOffset];

			Result = DigitalMusicDecode(&LocalPtr->ReadState,DestPtr,ChunkSize);	/* Decompress the data */
			
			/* No data for now... */
			
			if (!Result) {
				break;			/* Bug out... */
				
			/* Data is over? */

			} else if (Result==(Word)-1) {
				if (BurgerSongLoops) {			/* Let's loop the data */
					ReadFileStreamStop(LocalPtr->FileStreamPtr);
					ReadFileStreamStart(LocalPtr->FileStreamPtr,LocalPtr->ReadState.FileOffset);
					DigitalMusicReset(&LocalPtr->ReadState);		/* Reset the data reader */
				} else {
					FastMemSet(DestPtr,DigitalMusicGetSilenceVal(LocalPtr->ReadState.DataType),ChunkSize);
					BurgerLastSong = (Word)-1;
					LocalPtr->State = DORMANT;		/* Go to sleep */
					break;
				}
				
			/* Partial chunk */
			
			} else if (Result!=ChunkSize) {
				LocalPtr->WorkOffset += Result;		/* Accept the data and abort */
						
			/* Read in the whole chunk */
			
			} else {
				LocalPtr->EndIndex = j;			/* Accept the packet */
				LocalPtr->WorkOffset = 0;		/* No padding needed */
			}
				
		} while (--Counter);		/* Shall I read another packet? */
	}

	/* Upload audio as needed */

	DigitalMusicSoundProc(LocalPtr);
}

/**********************************

	Shut down the audio services

**********************************/

static void BURGERCALL DigitalMusicStop(void)
{
	DigitalMusic_t *LocalPtr;

	LocalPtr = &MyState;
	
	/* Kill the foreground task */
	
	KeyboardRemoveRoutine(DigitalMusicForegroundTask,LocalPtr);

	/* Dispose of the audio channel */
	
	if (LocalPtr->SongSampleHandle) {
		IDirectSoundBuffer_Stop(LocalPtr->SongSampleHandle);
		IDirectSoundBuffer_Release(LocalPtr->SongSampleHandle);
		LocalPtr->SongSampleHandle = 0;
	}

	/* Release the audio file stream */
	
	ReadFileStreamDelete(LocalPtr->FileStreamPtr);
	LocalPtr->FileStreamPtr = 0;

	/* Release the audio codec */
	
	DigitalMusicReadStateDestroy(&LocalPtr->ReadState);

	/* Release the audio buffer */
	
	if (LocalPtr->FileBuffer) {
		DeallocAPointer(LocalPtr->FileBuffer);					/* Release the memory */
		LocalPtr->FileBuffer = 0;
	}	
	BurgerLastSong = 0;					/* No song is playing */
}


/**********************************

	Play a streaming audio file from WinOS

**********************************/

void BURGERCALL DigitalMusicPlay(Word Num)
{
	if (Num!=BurgerLastSong) {		/* Should I do anything? */
		DigitalMusic_t *LocalPtr;
		LocalPtr = &MyState;
		DigitalMusicStop();			/* Stop the previous song (If any) */
		if (!MyDirectSoundDevice || !(SystemState & MusicActive)) {		/* Don't do anything if music isn't allowed */
			Num = 0;
		}
		BurgerLastSong = Num;
		if (Num) {
			LocalPtr->FileStreamPtr = ReadFileStreamNew(DigitalMusicNameCallback(Num),32,4096);
			if (LocalPtr->FileStreamPtr) {				
				Word8 *MyBufferPtr;
				ReadFileStreamStart(LocalPtr->FileStreamPtr,0);
				MyBufferPtr = (Word8 *)AllocAPointerClear(BIGBUFFERSIZE);
				if (MyBufferPtr) {
					LocalPtr->FileBuffer = MyBufferPtr;			/* Pointer to buffer to work with */
					BurgerSongLoops = SoundLoopFlag;
					KeyboardAddRoutine(DigitalMusicForegroundTask,LocalPtr);
					LocalPtr->State = STARTING;
					SoundLoopFlag = FALSE;		/* Reset on exit */
					return;
				}						
				ReadFileStreamDelete(LocalPtr->FileStreamPtr);
				LocalPtr->FileStreamPtr = 0;
			}
		}
	}
	SoundLoopFlag = FALSE;		/* Reset on exit */
}

/**********************************

	Alter the frequency of a digital song

**********************************/

void BURGERCALL DigitalMusicSetFrequency(Word Frequency)
{
	DigitalMusic_t *LocalPtr;
	LocalPtr = &MyState;
	BurgerSongFreq = Frequency;
	if (LocalPtr->SongSampleHandle) {
		IDirectSoundBuffer_SetFrequency(LocalPtr->SongSampleHandle,Frequency);
	}
}

/**********************************

	Set the volume of a song

**********************************/

void BURGERCALL DigitalMusicSetVolume(Word NewVolume)
{
	Word Temp;
	DigitalMusic_t *LocalPtr;
	LocalPtr = &MyState;
	if (NewVolume>=256) {		/* Failsafe */
		NewVolume = 255;		/* Maximum volume */
	}
	MusicVolume = NewVolume;
	Temp = SystemState&~(MusicActive);
	if (NewVolume) {
		Temp |= MusicActive;	/* Set the active flag */
	}
	SystemState = Temp;
	if (LocalPtr->SongSampleHandle) {
		IDirectSoundBuffer_SetVolume(LocalPtr->SongSampleHandle,Win95DirectSoundVolumes[NewVolume]);
	}
}

#endif
