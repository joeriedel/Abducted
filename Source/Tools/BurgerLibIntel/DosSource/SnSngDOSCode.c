/**********************************

	Digital streaming music

**********************************/

#include "SnSound.h"

#if defined(__DOS4G__)
#include "MmMemory.h"
#include "FmFile.h"
#include "InInput.h"
#include "ClStdLib.h"
#include "PkPack.h"
#include "RsReadStream.h"
#include <string.h>
#include <io.h>
#include <fcntl.h>

/**********************************

	Digital music driver

**********************************/

#define BUFFERSIZESHIFT 12		/* Buffer size as a shift value */
#define BUFFERSIZE (1<<BUFFERSIZESHIFT)		/* Size of the audio buffer */
#define RINGCOUNT 32 						/* Number of chunks in the file buffer */
#define FILEBUFFER (BUFFERSIZE*RINGCOUNT)	/* Size of the audio ring buffer */

#define BIGBUFFERSIZE (FILEBUFFER)		/* My work buffer */

typedef struct DigitalMusic_t {
	DigitalMusicReadState_t ReadState;	/* Current audio state */

	/* For the sound channel */

	HANDLE SongSampleHandle;		/* HMI Sample handle */
	_SOS_SAMPLE SongSampleRecord;	/* HMI Sample record */
	volatile Word CurrentIndex;		/* Current block being unpacked */
	volatile Word EndIndex;			/* Ending block for the ring buffer */

	/* For the loader */

	struct ReadFileStream_t *FileStreamPtr;	/* Instance of the disk loader */
	Word8 *FileBuffer;				/* Main disk buffer */
	Word32 WorkOffset;			/* Pointer to write my unpacked buffer */
	Word State;						/* Current stream state */
} DigitalMusic_t;

#define READMODESLEEP 0		/* Buffer is full, no actions pending */
#define READMODESEEK 1		/* Waiting for a seek */
#define READMODEREAD 2		/* Just read in data */
#define READMODEDONE 3		/* No more actions pending */

#define DORMANT 0
#define STARTING 1
#define READING 2

static DigitalMusic_t MyState = {{0}};

/**********************************

	Call back for seamless audio
	I am the only routine allowed to write to "FirstAudioBuffer"

**********************************/

static void __cdecl SampleCallback(PSOSSAMPLE sSample)
{
	Word i;
	sSample->pSample = (char *)&MyState.FileBuffer[MyState.CurrentIndex*BUFFERSIZE];
	sSample->wLength = BUFFERSIZE;		/* Save size of the buffer */
	i = MyState.CurrentIndex;	/* Go to the next buffer */
	if (i!=MyState.EndIndex) {		/* At the end? */
		++i;			/* Don't go to the next buffer */
		i&=(RINGCOUNT-1);
		MyState.CurrentIndex = i;
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

	Refill all the audio buffers that are empty
	An IRQ routine will change "FirstAudioBuffer"
	I am the only routine allowed to write to "LastAudioBuffer"

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

			if (!ParseSoundFileImage(&Raw,HeaderPtr,4096)) {
				FastMemSet(&LocalPtr->SongSampleRecord,0,sizeof(_SOS_SAMPLE));
				LocalPtr->SongSampleRecord.wRate = Raw.SampleRate;
				LocalPtr->SongSampleRecord.wPanPosition = PanPosition;

				/* Now fill in all the records for the double buffer record */
				i = Raw.DataType;		/* Get the data type */
				j = 1;
				if (i&SOUNDTYPESTEREO) {	/* Stereo data? */
					j = 2;
				}
				LocalPtr->SongSampleRecord.wChannels = j;		/* Stereo or mono */
				BurgerSongFreq = Raw.SampleRate;

				i = DigitalMusicReadStateInit(&LocalPtr->ReadState,&Raw,HeaderPtr,FILEBUFFER,DigitalMusicReadProc,LocalPtr);
				switch (i&0xFF) {		/* Type of data */
				default:
	//			case SOUNDTYPEBYTE:		/* Unsigned byte */
	//			case SOUNDTYPECHAR:		/* Signed byte */
					LocalPtr->SongSampleRecord.wBitsPerSample = 8;
					LocalPtr->SongSampleRecord.wFormat = _PCM_UNSIGNED;
					break;
				case SOUNDTYPELSHORT:	/* Little endian signed short */
				case SOUNDTYPEBSHORT:	/* Big endian signed short */
					LocalPtr->SongSampleRecord.wBitsPerSample = 16;
//					LocalPtr->SongSampleRecord.wFormat = 0;
				}
				ReadFileStreamStop(LocalPtr->FileStreamPtr);
				ReadFileStreamStart(LocalPtr->FileStreamPtr,LocalPtr->ReadState.FileOffset);
				DigitalMusicReset(&LocalPtr->ReadState);		/* Loop next time */
				LocalPtr->State = READING;

				LocalPtr->SongSampleRecord.pSample = (char *)LocalPtr->FileBuffer;
				LocalPtr->SongSampleRecord.wLength = BUFFERSIZE;
				LocalPtr->SongSampleRecord.wVolume = MK_VOLUME((MusicVolume<<7),(MusicVolume<<7));
				LocalPtr->SongSampleRecord.wPriority = 0;
				LocalPtr->SongSampleRecord.pfnSampleProcessed = SampleCallback;

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

	/* Restart audio if somehow is not active */

	if (LocalPtr->SongSampleHandle==-1 && LocalPtr->CurrentIndex!=LocalPtr->EndIndex) {
		LocalPtr->SongSampleHandle = sosDIGIStartSample(hDIGIDriver,&LocalPtr->SongSampleRecord);	/* Play the sound */
	}
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

	if (LocalPtr->SongSampleHandle!=-1) {	/* Active? */
		sosDIGIStopSample(hDIGIDriver,LocalPtr->SongSampleHandle);		/* Kill the sound channel */
		LocalPtr->SongSampleHandle = -1;
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

	Play a digital sound file

**********************************/

void BURGERCALL DigitalMusicPlay(Word Num)
{
	if (Num!=BurgerLastSong) {		/* Should I do anything? */
		DigitalMusic_t *LocalPtr;
		LocalPtr = &MyState;
		DigitalMusicStop();			/* Stop the previous song (If any) */
		if (!(SystemState & MusicActive)) {		/* Don't do anything if music isn't allowed */
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
					LocalPtr->SongSampleHandle = -1;
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
	BurgerSongFreq = Frequency;
	if (BurgerLastSong!=-1) {
		sosDIGISetSampleRate(hDIGIDriver,MyState.SongSampleHandle,Frequency);
	}
}

/**********************************

	Set the volume of a song

**********************************/

void BURGERCALL DigitalMusicSetVolume(Word NewVolume)
{
	Word Temp;
	if (NewVolume>=256) {
		NewVolume = 255;
	}
	MusicVolume = NewVolume;
	Temp = SystemState&~(MusicActive);
	if (NewVolume) {
		Temp |= MusicActive;
	}
	SystemState = Temp;
	if (BurgerLastSong!=-1) {
		if (NewVolume>=254) {
			NewVolume = 0x7FFF7FFF;
		} else {
			NewVolume = NewVolume<<7;			/* Convert to HMI format */
			NewVolume |= (NewVolume<<16);
		}
		MyState.SongSampleRecord.wVolume = NewVolume;
		sosDIGISetSampleVolume(hDIGIDriver,MyState.SongSampleHandle,NewVolume);
	}
}

#endif
