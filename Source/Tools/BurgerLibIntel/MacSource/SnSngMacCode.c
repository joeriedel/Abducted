/**********************************

	MacOS version of the streaming audio manager.
	This is a full async function. It was hell to write
	since MacOS does not have true multi-tasking.
	This routine has deferred tasks that call you upon completion
	but they don't block in the multi-threaded sense. As a result,
	I have had to resort to drastic measures to get this to
	work properly

**********************************/

#include "SnSound.h"

#if defined(__MAC__)
#include "McMac.h"
#include "InInput.h"
#include "FmFile.h"
#include "ClStdLib.h"
#include "MmMemory.h"
#include "RsReadStream.h"
#include <Sound.h>
#include <Devices.h>

#define BUFFERSIZESHIFT 12	/* Buffer size as a shift value */
#define BUFFERSIZE (1<<BUFFERSIZESHIFT)		/* Size of the audio buffer */
#define RINGCOUNT 32						/* Number of chunks in the file buffer */
#define FILEBUFFER (BUFFERSIZE*RINGCOUNT)	/* Size of the audio ring buffer */

#define BIGBUFFERSIZE (FILEBUFFER)	/* My work buffer */

typedef struct DigitalMusic_t {
	DigitalMusicReadState_t ReadState;	/* Current audio state */

	/* For the sound channel */
	
	SndChannel *SongSampleHandle;		/* Sound manager channel */
	SndCallBackUPP CallBackUPP;			/* UPP pointer for sound manager callbacks */
	volatile Word CurrentIndex;			/* Current block being unpacked (Can only be modified by the sound callback) */
	volatile Word EndIndex;				/* Ending block for the ring buffer (Can only be modifed by the data loader) */
	volatile Word SoundDormant;			/* TRUE if the sound player is asleep */
	Word Frames;						/* Number of audio frames in a block */
	ExtSoundHeader SndHeader;			/* Header to pass to MacOS */
	short Padding;						/* The SndHeader above is not long aligned */
	
	/* For the loader */
	
	struct ReadFileStream_t *FileStreamPtr;	/* Instance of the disk loader */
	Word8 *FileBuffer;					/* Main disk buffer */
	Word32 WorkOffset;				/* Pointer to write my unpacked buffer */
	Word State;							/* Current stream state */
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

static pascal void DigitalMusicSoundProc(SndChannel *Chan,SndCommand *Input)
{
	DigitalMusic_t *LocalPtr;		/* Work pointer */
	SndCommand mySndCmd;			/* To pass a sound command */
	Word Index;
	
	#if !TARGET_RT_MAC_CFM
	long OldA5;
	OldA5 = SetA5(Chan->userInfo);	/* Get the A5 register */
	#endif

	LocalPtr = &MyState;			/* Load the current work buffer */

	/* Is there any data pending? */

	Index = LocalPtr->CurrentIndex;
	if (Index != LocalPtr->EndIndex) {		/* Is there data available? */
		/* Pass this buffer to the sound manager */
	
		LocalPtr->SndHeader.samplePtr = (char *)(&LocalPtr->FileBuffer[Index*BUFFERSIZE]);
//		LocalPtr->SndHeader.numChannels = 2;		/* Preset */
		LocalPtr->SndHeader.sampleRate = BurgerSongFreq<<16;
		LocalPtr->SndHeader.loopStart = 0;
		LocalPtr->SndHeader.loopEnd = 0;
		LocalPtr->SndHeader.encode = extSH;			/* Extended sound header */
		LocalPtr->SndHeader.baseFrequency = 0;
		LocalPtr->SndHeader.numFrames = LocalPtr->Frames;	/* Number of samples */
		LocalPtr->SndHeader.markerChunk = 0;
		LocalPtr->SndHeader.instrumentChunks = 0;
		LocalPtr->SndHeader.AESRecording = 0;
//		LocalPtr->SndHeader.sampleSize = 16;		/* Preset */
	
		LocalPtr->CurrentIndex = (Index+1)&(RINGCOUNT-1);		/* Accept the data */

		/* Make sure the scaler is OFF! */
	
		mySndCmd.cmd = rateMultiplierCmd;
		mySndCmd.param1 = 0;
		mySndCmd.param2 = (1<<16);		/* Convert to fixed rate offset */
		SndDoCommand(Chan,&mySndCmd,TRUE);

		/* Send the command to play the next buffer */
	
		mySndCmd.cmd = bufferCmd;
		mySndCmd.param1 = 0;
		mySndCmd.param2 = (int)&LocalPtr->SndHeader;
		SndDoCommand(Chan,&mySndCmd,TRUE);
	
	/* Shall I call myself after this buffer is complete? */

		mySndCmd.cmd = callBackCmd;		/* Queue a callback */
		mySndCmd.param1 = 0;
		mySndCmd.param2 = 0;
		SndDoCommand(Chan,&mySndCmd,TRUE);
		LocalPtr->SoundDormant = FALSE;
	} else {
		LocalPtr->SoundDormant = TRUE;
	}

	#if !TARGET_RT_MAC_CFM
	SetA5(OldA5);			/* 680x0 cleanup */
	#endif
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

static void DigitalMusicForegroundTask(void *Input)
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
			SndCommand Command;
			
			if (!ParseSoundFileImage(&Raw,HeaderPtr,4096)) {					
				/* Now fill in all the records for the double buffer record */
				i = Raw.DataType;		/* Get the data type */
				j = 1;
				if (i&SOUNDTYPESTEREO) {	/* Stereo data? */
					j = 2;
				}
				LocalPtr->SndHeader.numChannels = j;		/* Stereo or mono */
				BurgerSongFreq = Raw.SampleRate;
				
				i = DigitalMusicReadStateInit(&LocalPtr->ReadState,&Raw,HeaderPtr,FILEBUFFER,DigitalMusicReadProc,LocalPtr);
//				i = SOUNDTYPEBSHORT;
				switch (i&0xFF) {		/* Type of data */
				default:
	//			case SOUNDTYPEBYTE:		/* Unsigned byte */
	//			case SOUNDTYPECHAR:		/* Signed byte */
					LocalPtr->SndHeader.sampleSize = 8;
					LocalPtr->Frames = BUFFERSIZE;
					break;
				case SOUNDTYPELSHORT:	/* Little endian signed short */
				case SOUNDTYPEBSHORT:	/* Big endian signed short */
					LocalPtr->SndHeader.sampleSize = 16;
					LocalPtr->Frames = BUFFERSIZE/2;
				}
				if (LocalPtr->SndHeader.numChannels==2) {
					LocalPtr->Frames >>= 1;
				}
				ReadFileStreamStop(LocalPtr->FileStreamPtr);
				ReadFileStreamStart(LocalPtr->FileStreamPtr,LocalPtr->ReadState.FileOffset);
				DigitalMusicReset(&LocalPtr->ReadState);		/* Loop next time */
				LocalPtr->State = READING;

				Command.cmd = volumeCmd;
				i = MusicVolume;
				if (i>=128) {			/* Convert range of 0-255 to 0-256 */
					++i;
				}
				Command.param2 = (i<<16)|i;
				SndDoImmediate(LocalPtr->SongSampleHandle,&Command);
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
			
	if (LocalPtr->SoundDormant && LocalPtr->CurrentIndex!=LocalPtr->EndIndex) {
		SndCommand Bogus;
		DigitalMusicSoundProc(LocalPtr->SongSampleHandle,&Bogus);
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
	
	if (LocalPtr->SongSampleHandle) {	/* Active? */
		SndCommand Command;
		Command.cmd = quietCmd;		/* Shut down the output */
		Command.param1 = 0;
		Command.param2 = 0;
		SndDoImmediate(LocalPtr->SongSampleHandle,&Command);	/* Send the command */
		Command.cmd = flushCmd;
		Command.param1 = 0;
		Command.param2 = 0;
		SndDoImmediate(LocalPtr->SongSampleHandle,&Command);
		SndDisposeChannel(LocalPtr->SongSampleHandle,TRUE);	/* Kill the sound channel */
		LocalPtr->SongSampleHandle = 0;
	}
	LocalPtr->SoundDormant = TRUE;		/* Audio is off */

	/* Release the audio callback UPP */
	
	if (LocalPtr->CallBackUPP) {
		DisposeSndCallBackUPP(LocalPtr->CallBackUPP);
		LocalPtr->CallBackUPP = 0;
	}

	/* Release the audio file stream */
	
	ReadFileStreamDelete(LocalPtr->FileStreamPtr);
	LocalPtr->FileStreamPtr = 0;

	/* Release the audio codec */
	
	DigitalMusicReadStateDestroy(&LocalPtr->ReadState);

	/* Release the audio buffer */
	
	if (LocalPtr->FileBuffer) {
		UnholdMemory(LocalPtr->FileBuffer,BIGBUFFERSIZE);		/* Release virtual memory */
		DeallocAPointer(LocalPtr->FileBuffer);					/* Release the memory */
		LocalPtr->FileBuffer = 0;
	}
	
	BurgerLastSong = 0;					/* No song is playing */
}

/**********************************

	Play a streaming audio file from MacOS

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
				ReadFileStreamStart(LocalPtr->FileStreamPtr,0);
				LocalPtr->CallBackUPP = NewSndCallBackUPP(&DigitalMusicSoundProc);	/* Make it */
				if (LocalPtr->CallBackUPP) {
					if (!SndNewChannel(&LocalPtr->SongSampleHandle,sampledSynth,0,LocalPtr->CallBackUPP)) {
						Word8 *MyBufferPtr;
					
						#if !TARGET_RT_MAC_CFM
						{
							long MyA5;
							MyA5 = SetCurrentA5();
							LocalPtr->SongSampleHandle->userInfo = MyA5;
						}
						#endif
					
						MyBufferPtr = (Word8 *)AllocAPointerClear(BIGBUFFERSIZE);
						if (MyBufferPtr) {
							HoldMemory(MyBufferPtr,BIGBUFFERSIZE);		/* Tell VM not to swap out this memory */
							LocalPtr->FileBuffer = MyBufferPtr;			/* Pointer to buffer to work with */
							BurgerSongLoops = SoundLoopFlag;
							KeyboardAddRoutine(DigitalMusicForegroundTask,LocalPtr);
							LocalPtr->State = STARTING;
							SoundLoopFlag = FALSE;		/* Reset on exit */
							return;
						}						
						SndDisposeChannel(LocalPtr->SongSampleHandle,TRUE);	/* Kill the sound channel */
						LocalPtr->SongSampleHandle = 0;
					}
					DisposeSndCallBackUPP(LocalPtr->CallBackUPP);
					LocalPtr->CallBackUPP = 0;			
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
}

/**********************************

	Set the volume of a song

**********************************/

void BURGERCALL DigitalMusicSetVolume(Word NewVolume)
{
	Word Temp;
	DigitalMusic_t *LocalPtr;
	LocalPtr = &MyState;
	if (NewVolume>=256) {
		NewVolume = 255;
	}
	MusicVolume = NewVolume;
	Temp = SystemState&~(MusicActive);
	if (NewVolume) {
		Temp |= MusicActive;
	}
	SystemState = Temp;
	
	NewVolume *= MusicVolume;
	NewVolume /= 255;		/* Set the new scaled volume */

	if (LocalPtr->SongSampleHandle) {
		SndCommand Command;
		Command.cmd = volumeCmd;
		if (NewVolume>=128) {		/* Convert range of 0-255 to 0-256 */
			++NewVolume;
		}
		Command.param2 = (NewVolume<<16)|NewVolume;
		SndDoImmediate(LocalPtr->SongSampleHandle,&Command);
	}
}

#endif
