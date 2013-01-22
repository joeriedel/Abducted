#include "SnSound.h"
#include <BREndian.hpp>
#include "RzRez.h"
#include "ClStdLib.h"
#include "InInput.h"
#include "MmMemory.h"
#include "PkPack.h"
#include <string.h>
#include <stdlib.h>

#if defined(__MSDOS__) && defined(__DOS4G__)
#include <io.h>
#include <fcntl.h>
#elif defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__MAC__)
#include <files.h>
#endif

#ifdef __cplusplus
extern "C" {
static void ANSICALL KillSound(void);
static Word GetPan(PrivSound_t *MySound,void *Input);
static Word GetVolx(PrivSound_t *MySound,void *Input);
static Word SetCallback(PrivSound_t *MySound,void *Input);
static Word GetFrq(PrivSound_t *MySound,void *Input);
}
#endif

SndKillProcPtr BurgerSndKillProcs[3];		/* Shut down procs for all 3 inits */
Word BurgerSamplePriority[MAXVOICECOUNT] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};	/* Sound priority chain */
Word BurgerVoiceCount = MAXVOICECOUNT;
PrivSound_t BurgerSamples[MAXVOICECOUNT+1];
Word BurgerSndExitIn;		/* True if atexit proc is installed */
Word MusicVolume = 255;
Word MasterSoundVolume = 255;
Word SystemState = SfxActive|MusicActive;
Word SfxVolume = 255;		/* Master volume for sound effects */
RezHeader_t *SoundRezHeader = &MasterRezHeader;	/* Sound resource type for internal use */
Word *SoundCookiePtr;		/* Writeback handle for audio channel */
Word PanPosition = 0x8000;	/* Initial pan position of sound to play */
Word SoundLoopFlag;			/* True if you want the sound to loop */
int SoundFrequencyAdjust;	/* Pitch bend */

/**********************************

	Initialize the sound system
	I read the config file and save the data

**********************************/

void BURGERCALL InitSoundPlayer(void)
{
	Word i;
	i = 0;
	do {
		BurgerSamplePriority[i] = i;
	} while (++i<MAXVOICECOUNT);
	
	if (!InitDigitalDriver()) {		/* Init digital audio */
		KillSoundPlayer();		/* I can't start! */
		return;
	}
	BurgerSndKillProcs[DIGISOUNDKILL] = KillSoundPlayer;	/* For system quit */
	BurgerSndExitIn |= DIGISOUNDON;	/* Digital audio is on */
	PanPosition = 0x8000;			/* Init my variables */
	SoundLoopFlag = FALSE;			/* No looping */
	SoundFrequencyAdjust = 0;		/* No adjustment */
	SoundCookiePtr = 0;				/* No cookies */
	SfxVolume = 255;				/* Init initial volume */
	MasterSoundVolume = 255;		/* Maximum volume */
	SystemState |= SfxActive;		/* Enable sound effects */
	EnableSoundShutdownProc();		/* Allow shutdown */
}

/**********************************

	When the sound driver is installed, this routine
	will shut it down.

**********************************/

void BURGERCALL KillSoundPlayer(void)
{
	Word i;

	i = 0;				/* Kill all sound files */
	do {
	 	StopASound(i|SOUND_COOKIE);		/* Bye bye */
	} while (++i<BurgerVoiceCount);

	BurgerSndKillProcs[DIGISOUNDKILL] = 0;
	BurgerSndExitIn &= ~DIGISOUNDON;	/* I am off */
	KillDigitalDriver();		/* Release the driver */
}

/**********************************

	Shut off all the sound drivers
	when the application quits

**********************************/

static Bool ExitProc;		/* Only add the atexit proc once */

static void ANSICALL KillSound(void)
{
	Word i;
	SndKillProcPtr KillProc;

	i = 0;
	do {
		KillProc = BurgerSndKillProcs[i];	/* Is there a proc here? */
		if (KillProc) {
			KillProc();		/* Shut down the services */
		}
	} while (++i<3);
}

void BURGERCALL EnableSoundShutdownProc(void)
{
	if (!ExitProc) {		/* Proc installed? */
		ExitProc = TRUE;	/* Set the flag */
		atexit(KillSound);	/* Install the proc */
	}
}

/**********************************

	Scan an AIFF file in memory and return pointer to a chunk

**********************************/

void * BURGERCALL FindIffChunk(const void *Input,Word32 Name,Word32 Length)
{
	Word32 Skip;
	char AsciiName[5];
	if (Length>=(12+8)) {
		Skip = 12;		/* Initial skip */
#if defined(__LITTLEENDIAN__)
		Name = Burger::SwapEndian(Name);		/* Convert to Motorola endian */
#endif
		do {
			Length-=Skip;
			Input = ((Word8 *)Input)+Skip;
			if (((Word32*)Input)[0] == Name) {	/* Match? */
				goto Gotit;
			}
			Skip = Burger::LoadBig(((Word32 *)Input)[1]);
			Skip = (Skip+9)&(~1);	/* Align to short */
		} while (Skip<Length);
	}
	((Word32 *)(&AsciiName[0]))[0] = Name;
	AsciiName[4] = 0;
	NonFatal("IFF Chunk %s was not found\n",AsciiName);
	return 0;
Gotit:
	return (void *)Input;
}

/**********************************

	This routine will convert an Extended floating
	point data type (Big endian) found in AIFF audio
	files into a double for the native machine.

**********************************/

double BURGERCALL ConvertAiffExtended(const void *Input)
{
#if defined(__LITTLEENDIAN__)	/* Intel version */

	/* Return IEEE double in little endian format */

	double Result;
	Word8 *Output;
	Word Sign,Exponent;

	Output = (Word8 *)&Result;
	Exponent = (((Word8 *)Input)[0]<<8)|((Word8 *)Input)[1];
	Sign = Exponent&0x8000;
	Exponent = (Exponent-(0x0400C-0x040C))&0x7FF;
	Output[7] = (Word8)(Exponent>>4);
	Output[6] = (Word8)(Exponent<<4);
	if (Sign) {
		Output[7]|=0x80;
	}
	Sign = 2;
	do {
		Exponent = ((Word8 *)Input)[Sign];
		if (Sign==2) {
			Exponent &=0x7f;
		}
		Output[8-Sign] |= (Word8)(Exponent>>3);
		if (Sign!=8) {
			Output[7-Sign] = (Word8)(Exponent<<5);
		}
	} while (++Sign<8);
	return Result;

#else	/* Power PC machines */

	/* Return IEEE double in big endian format */

	double Result;
	Word8 *Output;
	Word Sign,Exponent;

	Output = (Word8 *)&Result;
	Exponent = (((Word8 *)Input)[0]<<8)|((Word8 *)Input)[1];
	Sign = Exponent&0x8000;
	Exponent = (Exponent-(0x0400C-0x040C))&0x7FF;
	Output[0] = (Word8)(Exponent>>4);
	Output[1] = (Word8)(Exponent<<4);
	if (Sign) {
		Output[0]|=0x80;
	}
	Sign = 2;
	do {
		Exponent = ((Word8 *)Input)[Sign];
		if (Sign==2) {
			Exponent &=0x7f;
		}
		Output[Sign-1] |= (Word8)(Exponent>>3);
		if (Sign!=8) {
			Output[Sign] = (Word8)(Exponent<<5);
		}
	} while (++Sign<8);
	return Result;
#endif
}

/**********************************

	Return the current sound effects volume

**********************************/

Word BURGERCALL GetSfxVolume(void)
{
	return MasterSoundVolume;
}

/**********************************

	Return the number of active sounds playing

**********************************/

Word BURGERCALL GetNumSoundsPlaying(void)
{
	Word i;
	PrivSound_t *MySound;
	Word Count;

	Count = 0;		/* Init the count */
	i = BurgerVoiceCount;	/* Number of voices to check */
	if (i) {
		MySound = BurgerSamples;
		do {
			if (MySound->SoundPtr) {	/* Active? */
				++Count;		/* +1 to the count */
			}
			++MySound;		/* Next one in the chain */
		} while (--i);
	}
	return Count;		/* Return the active count */
}

/**********************************

	Get the number of the maximum number of sounds
	to be played at the same time

**********************************/

Word BURGERCALL GetMaxSounds(void)
{
	return BurgerVoiceCount;	/* Return the internal value */
}

/**********************************

	Return the pan position of a sound effect

**********************************/

static Word BURGERCALL GetPan(PrivSound_t *MySound,void * /* Input */)
{
	return MySound->PanPosition;	/* Get the pan position value */
}

Word BURGERCALL GetASoundPan(Word32 SoundNum)
{
	return SoundIterator(GetPan,0,SoundNum,TRUE);	/* Handle it */
}

/**********************************

	Get the volume for a sound effect

**********************************/

static Word BURGERCALL GetVolx(PrivSound_t *MySound,void * /* Input */)
{
	return MySound->Volume;	/* Get the sound's volume */
}

Word BURGERCALL GetASoundVolume(Word32 SoundNum)
{
	return SoundIterator(GetVolx,0,SoundNum,TRUE);	/* Handle it */
}

/**********************************

	Returns the frequency of a sample

**********************************/

static Word BURGERCALL GetFrq(PrivSound_t *MySound,void * /* Input */)
{
	return MySound->SampleRate;	/* Get the sample playback rate */
}

Word BURGERCALL GetASoundFrequency(Word32 SoundNum)
{
	return SoundIterator(GetFrq,0,SoundNum,TRUE);	/* Handle it */
}

/**********************************

	Pause all the sounds

**********************************/

void BURGERCALL PauseAllSounds(void)
{
	Word i;
	i = 0;
	do {
		StopASound(i|SOUND_COOKIE);
	} while (++i<BurgerVoiceCount);
}

/**********************************

	Resume all sound effects

**********************************/

void BURGERCALL ResumeAllSounds(void)
{
}


/**********************************

	Set the maximum number of voices

**********************************/

void BURGERCALL SetMaxSounds(Word Max)
{
	Word i;
	if (Max>MAXVOICECOUNT) {
		Max = MAXVOICECOUNT;	/* Don't go too high */
	}
	if (!Max) {
		Max = 1;	/* Minimum of 1 voice */
	}

	i = 0;
	do {
		StopASound(i|SOUND_COOKIE);		/* Shut down all previous voices */
	} while (++i<BurgerVoiceCount);

	BurgerVoiceCount = Max;		/* Set new maximum */
	i = 0;
	do {
		BurgerSamplePriority[i] = i;	/* Init the priority table */
	} while (++i<Max);		/* All done? */
}

/**********************************

	Set the sound effects volume

**********************************/

#if !defined(__MAC__)
void BURGERCALL SetSfxVolume(Word NewVolume)
{
	Word Flag;
	if (NewVolume>=256) {	/* Make sure the volume is in range */
		NewVolume = 255;
	}
	MasterSoundVolume = NewVolume;
	Flag = SystemState&(~SfxActive);	/* Clear the active flag */
	if (NewVolume) {
		Flag |= SfxActive;		/* Set the flag */
	}
	SystemState = Flag;		/* Save the flag */
}
#endif

/**********************************

	Stop all of the sounds

**********************************/

void BURGERCALL StopAllSounds(void)
{
	Word i;
	i = 0;
	do {
		StopASound(i|SOUND_COOKIE);
	} while (++i<BurgerVoiceCount);
}

/**********************************

	Call this proc on sound shutdown

**********************************/

typedef struct NewBack_t {
	void *NewData;
	SndCompleteProcPtr NewProc;
} NewBack_t;

static Word BURGERCALL SetCallback(PrivSound_t *MySound,void *Input)
{
	MySound->Proc = ((NewBack_t *)Input)->NewProc;	/* Get the pan position value */
	MySound->Data = ((NewBack_t *)Input)->NewData;
	return FALSE;
}

void BURGERCALL SoundSetCallback(Word32 SoundCookie,SndCompleteProcPtr Proc,void *Data)
{
	NewBack_t Local;
	Local.NewProc = Proc;
	Local.NewData = Data;
	SoundIterator(SetCallback,&Local,SoundCookie,FALSE);
}

/**********************************

	Using either a sound cookie or
	a resource ID number, call a proc to handle a specific
	action to a sound channel.
	If a resource number is passed, multiple calls could be performed.
	In this case, I have a flag to abort on the first successful match
	such as information gathering.

**********************************/

Word BURGERCALL SoundIterator(SndIteratorProcPtr Proc,void *Input,Word32 Num,Word OneHit)
{
	PrivSound_t *MySound;
	Word RetVal;
	Word i;

	RetVal = FALSE;				/* Assume it fails */
	MySound = BurgerSamples;	/* Init the pointer */
	if (Num&SOUND_COOKIE) {		/* Is it a cookie? */
		Num = Num & (~SOUND_COOKIE);	/* Remove the mask */
		if (Num<BurgerVoiceCount) {		/* Valid cookie? */
			RetVal = Proc(&MySound[Num],Input);	/* Handle it */
		}
	} else {
		i = BurgerVoiceCount;		/* How many active voices? */
		if (i) {
			do {
				if (MySound->SampleResource == Num) {	/* Match? */
					RetVal = Proc(MySound,Input);	/* Call the proc */
					if (OneHit) {		/* Abort now? */
						break;
					}
				}
				++MySound;		/* Next structure */
			} while (--i);		/* More to go? */
		}
	}
	return RetVal;		/* Return the result */
}

/**********************************

	I am passed a pointer to a sound file.
	I will determine if it is a WAV, VOC or AIF file

**********************************/

/* .WAV file header structure */

typedef struct WavHeader_t {
	Word8 szRIFF[4];		/* ASCII RIFF */
	Word32 dwFormatLength;	/* Length of file contents */
	Word8 szWAVE[4];		/* ASCII WAVE */
	Word8 szFMT[4];		/* ASCII fmt_ */
	Word32 dwWaveFormatLength;	/* Size of fmt_ struct (16) */
	Word16 wFormatTag;	/* Compression type */
	Word16 wChannels;	/* Number of sound channels (1,2) */
	Word32 dwSamplesPerSec;	/* Sample rate */
	Word32 dwAvgBytesPerSec;	/* Bytes per second */
	Word16 wBlockAlign;		/* Data alignment (2) */
	Word16 wBitsPerSample;	/* Bits per sample (8,16) */
	Word16 wExtSize;			/* Extra data for compressed formats */
	Word16 wSamplesPerBlock;		/* Number of samples in each block */
	Word16 wNumCoefs;		/* Number of coefs in tables */
} WavHeader_t;

typedef struct WavData_t {
	Word8 szDATA[4];			/* ASCII data */
	Word32 dwDataLength;	/* Size of the data */
} WavData_t;

#define HEADERSIZE 0x2C

#if defined(__BIGENDIAN__)
#define RIFFASCII 0x52494646UL
#define AIFFASCII 0x41494646UL
#define AIFCASCII 0x41494643UL
#define WAVEASCII 0x57415645UL
#define FORMASCII 0x464F524DUL
#define DATAASCII 0x64617461UL
#define MAC6ASCII 0x4D414336UL
#define MAC3ASCII 0x4D414333UL
#define OGGASCII 0x4F676753UL
#else
#define RIFFASCII 0x46464952UL
#define AIFFASCII 0x46464941UL
#define AIFCASCII 0x43464941UL
#define WAVEASCII 0x45564157UL
#define FORMASCII 0x4D524F46UL
#define DATAASCII 0x61746164UL
#define MAC6ASCII 0x3643414DUL
#define MAC3ASCII 0x3343414DUL
#define OGGASCII 0x5367674FUL
#endif

/* Data for AIFF files (Big endian) */

typedef struct {		/* 12 byte header for ALL IFF files */
	char Form[4];		/* FORM */
	Word32 Length;	/* Length of the file contents */
	char Aiff[4];		/* AIFF */
} AIFFHeader_t;

typedef struct {		/* COMM chunk */
	char Comm[4];		/* COMM */
	Word32 Length;	/* Length of the struct */
	Word16 NumChannels;	/* Number of channels (1 or 2) */
	Word32 NumSampleFrames;	/* Number of SAMPLES, not bytes */
	Word16 SampleSize;	/* 8 or 16 bit */
	Word8 SampleRate[10];	/* Extended floating point */
	Word32 CompType;
} AIFFCommon_t;

typedef struct {		/* SSND chunk */
	char Ssnd[4];		/* SSND */
	Word32 Length;	/* Length of the data */
	Word32 Offset;	/* Offset to the start */
	Word32 BlockSize;	/* Size of each unit */
	Word8 Data[1];		/* Data */
} AIFFSsnd_t;

/* Data for VOC files (Little endian) */

typedef struct {		/* Voc file header */
	char Name[20];		/* "Creative Voice File\x1A" */
	Word16 Offset;		/* Offset to the header */
	Word16 Version;		/* Version number */
	Word16 Checksum;		/* Version +0x1234 */
} VOCHeader_t;

typedef struct {
	Word8 Type;			/* Data type (Must be 9) */
	Word8 LengthLo;		/* 3 bytes for the length */
	Word16 LengthHi;		/* upper 2 bytes for length */
	Word32 SampleRate;	/* Samples per second */
	Word8 SampleSize;	/* 8 or 16 bits */
	Word8 Channels;		/* 1 or 2 channels */
	Word8 Four;			/* A four */
	Word8 Filler[5];		/* Padding */
	Word8 Data[1];		/* Data */
} VOCData_t;

Word BURGERCALL ParseSoundFileImage(RawSound_t *Output,const void *Input,Word32 Length)
{
	Word Result;

	FastMemSet(Output,0,sizeof(RawSound_t));	/* Clear out the struct */
	Result = TRUE;			/* Assume error */

/* Is this a windows .WAV file? */

	if (Length >= HEADERSIZE &&
		((Word32 *)Input)[0] == RIFFASCII &&
		((Word32 *)Input)[2] == WAVEASCII) {
		WavData_t *DataPtr;
		Word TempType;
		Word Channels;
		// setup a pointer to the wave header
		// set size of the sample and pointer to the sample
		DataPtr = (WavData_t *)(((Word8 *)Input)+Burger::LoadLittle(&((WavHeader_t *)Input)->dwWaveFormatLength)+20);
		if (((Word32 *)DataPtr)[0] != DATAASCII) {
			DataPtr = (WavData_t*)(Burger::LoadLittle(&DataPtr->dwDataLength)+(Word8 *)DataPtr+8);
		}
		Output->SoundLength = Burger::LoadLittle(&DataPtr->dwDataLength);
		Output->SoundPtr = ((Word8 *)DataPtr)+8;
		Output->SampleRate = Burger::LoadLittle(&((WavHeader_t *)Input)->dwSamplesPerSec);
		Channels = Burger::LoadLittle(&((WavHeader_t*)Input)->wChannels);
		TempType = Burger::LoadLittle(&((WavHeader_t *)Input)->wFormatTag);
		switch (TempType) {
		default:
		case 1:			/* PCM data */
			if (Burger::LoadLittle(&((WavHeader_t *)Input)->wBitsPerSample) == 8) {	/* 8 bit data? */
				Output->DataType = SOUNDTYPEBYTE;
			} else {
				Output->DataType = SOUNDTYPELSHORT;		/* 16 bit samples */
			}
			Result = FALSE;		/* It's ok */
			break;
		case 2:			/* ADPCM data */
			if (Burger::LoadLittle(&((WavHeader_t *)Input)->wBitsPerSample) == 4) {	/* 4 bit data? */
				Output->DataType = SOUNDTYPEADPCM;
				Output->Extra1 = Output->SoundLength;	/* Compressed data length */
				Output->Extra2 = Burger::LoadLittle(&((WavHeader_t *)Input)->wSamplesPerBlock);
				Output->Extra3 = Burger::LoadLittle(&((WavHeader_t *)Input)->wBlockAlign);
				Output->SoundLength = (((Output->Extra1 / Output->Extra3) * Output->Extra2) * Channels);
				/* Next, for any partial blocks, substract overhead from it and it
					   will leave # of samples to read. */
				Output->SoundLength += ((Output->Extra1 - ((Output->Extra1/Output->Extra3)
					*Output->Extra3)) - (6 * Channels)) * Channels;
				Output->SoundLength *= 2;
				Result = FALSE;
			}
			break;
		case 6:			/* ALaw data */
			if (Burger::LoadLittle(&((WavHeader_t *)Input)->wBitsPerSample) == 8) {	/* 8 bit data? */
				Output->DataType = SOUNDTYPEALAW;
				Output->Extra1 = Output->SoundLength;	/* Compressed data length */
				Output->SoundLength *= 2;
				Result = FALSE;
			}
			break;
		case 7:			/* uLaw data */
			if (Burger::LoadLittle(&((WavHeader_t *)Input)->wBitsPerSample) == 8) {	/* 8 bit data? */
				Output->DataType = SOUNDTYPEULAW;
				Output->Extra1 = Output->SoundLength;	/* Compressed data length */
				Output->SoundLength *= 2;
				Result = FALSE;
			}
			break;
		}

		if (Channels==2) {
			Output->DataType|=SOUNDTYPESTEREO;
		}

/* Is this an AIFF file? */

	} else if ((Length>=12) &&
		(((Word32 *)Input)[0] == FORMASCII) &&
		((((Word32 *)Input)[2] == AIFFASCII) ||
		(((Word32 *)Input)[2] == AIFCASCII))) {
		AIFFCommon_t *CommPtr;
		AIFFSsnd_t *SsndPtr;

		CommPtr = static_cast<AIFFCommon_t *>(FindIffChunk(Input,'COMM',Length));	/* Get the COMM record */
		if (CommPtr) {
			SsndPtr = static_cast<AIFFSsnd_t *>(FindIffChunk(Input,'SSND',Length));	/* Get the sound data */
			if (SsndPtr) {
				Output->SoundPtr = &SsndPtr->Data[0];
				Output->SoundLength = Burger::LoadBig(SsndPtr->Length)-8;
				Output->SampleRate = (long)ConvertAiffExtended(&CommPtr->SampleRate[0]);
				if (Burger::LoadBig(CommPtr->SampleSize) == 8) {	/* 8 bit data? */
					Output->DataType = SOUNDTYPECHAR;
				} else {
					Output->DataType = SOUNDTYPEBSHORT;		/* 16 bit samples */
				}
				if (((Word32 *)Input)[2] == AIFCASCII) {
					if (CommPtr->CompType==MAC6ASCII) {
						Output->DataType = SOUNDTYPEMACE6;
						Output->Extra1 = Output->SoundLength;
						Output->SoundLength *= 6;
					} else if (CommPtr->CompType==MAC3ASCII) {
						Output->DataType = SOUNDTYPEMACE3;
						Output->Extra1 = Output->SoundLength;
						Output->SoundLength *= 3;
					} else {
						return Result;
					}
				}
				if (Burger::LoadBig(CommPtr->NumChannels) == 2) {
					Output->DataType|=SOUNDTYPESTEREO;
					Output->DataType &= 0xFFFF;
				}
				Result = FALSE;
			}
		}

/* Is this a type 9 VOC file? */

	} else if (Length>=(sizeof(VOCData_t)+sizeof(VOCHeader_t)) &&
		!memcmp(&((VOCHeader_t*)Input)->Name[0],"Creative Voice File\x1A",20) &&
		Burger::LoadLittle(&((VOCHeader_t *)Input)->Version) == (Burger::LoadLittle(&((VOCHeader_t *)Input)->Checksum))+0x1234) {
		VOCData_t *DataPtr;
		DataPtr = (VOCData_t *)((Word8 *)Input+Burger::LoadLittle(&((VOCHeader_t*)Input)->Offset));
		if (DataPtr->Type==9) {			/* I only support type 9 */
			Output->SoundPtr = &DataPtr->Data[0];	/* Pointer to the wav data */
			Output->SoundLength = (((Word32)Burger::LoadLittle(&DataPtr->LengthHi)<<8)+DataPtr->LengthLo)-12;
			Output->SampleRate = Burger::LoadLittle(&DataPtr->SampleRate);
			if (DataPtr->SampleSize==8) {
				Output->DataType = SOUNDTYPECHAR;
			} else {
				Output->DataType = SOUNDTYPELSHORT;
			}
			if (DataPtr->Channels==2) {
				Output->DataType|=SOUNDTYPESTEREO;
			}
			Result = FALSE;
		}
		
/* Is this an OGG/Vorbis file? */
	} else if (Length>=0x2C &&
		((Word32 *)Input)[0] == OGGASCII) {
		Output->DataType = SOUNDTYPEOGG;
		Output->SoundPtr = (Word8 *)Input;
		Output->SoundLength = (Word32)-1;		/* Can't tell unless I scan the whole thing */
		Output->SampleRate = Burger::LoadLittle((Word32 *)(&((Word8 *)Input)[0x28]));
		if (((Word8 *)Input)[0x27]==2) {
			Output->DataType|=SOUNDTYPESTEREO;
		}
		Result = FALSE;
	}
	
	if (!Result) {		/* Is this valid? */
		float FTemp;
		Word32 Samples;
		
		Samples = Output->SoundLength;		/* Number of bytes in the data */
		if (Output->DataType&SOUNDTYPESTEREO) {
			Samples >>= 1;					/* Cut in half for stereo */
		}
		
		switch (Output->DataType&0xFF) {	/* Type of data */
		case SOUNDTYPELSHORT:
		case SOUNDTYPEBSHORT:
			Samples >>= 1;					/* Cut in half for 16 bit samples */
		default:
			break;
		}
		if (Samples) {						/* Any samples survive? */
			Output->SampleCount = Samples;
			FTemp = (float)Output->SampleRate;		/* Get the rate */
			if (FTemp) {							/* Prevent divide by zero */
				Output->TimeInSeconds = (float)Samples/FTemp;		/* Simple conversion to length of time */
			}
		}
	}
	return Result;
}

/**********************************

	Play a sound effect

**********************************/

Word BURGERCALL PlayASound(Word32 SoundNum)
{
	Word Cookie;
	Word i;
	RawSound_t TheSound;
	void *SoundPtr;
	void **SoundHandle;
	Word32 SoundSize;

	Cookie = (Word)-1;					/* Assume no voice was allocated */
	if (SoundNum&SOUND_EXCLUSIVE) {		/* Shall I shut off all previous incarnations */
		SoundNum &= (~SOUND_EXCLUSIVE);
		StopASound(SoundNum);			/* Stop the sound */
	}

	if ((BurgerSndExitIn&DIGISOUNDON) && (SystemState&SfxActive) &&
		BurgerVoiceCount && SoundNum) {

		i = SetErrBombFlag(FALSE);	/* Don't bomb, just don't play sound */
		SoundHandle = ResourceLoadHandle(SoundRezHeader,SoundNum);	/* Try to load it */
		SetErrBombFlag(i);		/* Restore flag */

		if (SoundHandle) {		/* Didn't load? */
			SoundSize = GetAHandleSize(SoundHandle);	/* Get size of memory */
			SoundPtr = LockAHandle(SoundHandle);		/* Lock it down */
			if (!ParseSoundFileImage(&TheSound,SoundPtr,SoundSize)) {
				Cookie = PlayARawSound(&TheSound);
				if (Cookie!=(Word)-1) {
					PrivSound_t *MySound;
					MySound = &BurgerSamples[Cookie&0xFF];
					MySound->SampleResource = SoundNum;
					MySound->SampleRezHeader = SoundRezHeader;
				}
			}
			if (Cookie==(Word)-1) {			/* Error? */
				ResourceRelease(SoundRezHeader,SoundNum);		/* Oh oh... */
			}
		}
	}
	return Cookie;
}

/**********************************

	Generic sound effect routines

**********************************/


#if !(defined(__MSDOS__) && defined(__DOS4G__)) && !defined(__MAC__) && !defined(__WIN32__)

/**********************************

	Returns true or false is a sound effect is playing

**********************************/

Word BURGERCALL IsASoundPlaying(Word32 /* SoundNum */)
{
	return FALSE;
}

/**********************************

	Set the frequency of a sound effect

**********************************/

void BURGERCALL SetASoundFrequency(Word32 /* SoundNum */,Word /* Frequency */)
{
}

/**********************************

	Set the pan position for a sound effect

**********************************/

void BURGERCALL SetASoundPan(Word32 /* SoundNum */,Word /* Pan */)
{
}

/**********************************

	Set the volume for a sound effect

**********************************/

void BURGERCALL SetASoundVolume(Word32 /* SoundNum */,Word /* Volume */)
{
}

/**********************************

	Stop a sound effect in progress if needed

**********************************/

void BURGERCALL StopASound(Word32 /* SoundNum */)
{
}

/**********************************

	Play a sound effect using a structure describing
	how to play a sound effect.
	PanPosition and SoundLoopFlag are used as overrides

	I will return the Cookie number

**********************************/

Word BURGERCALL PlayARawSound(RawSound_t * /* Input */)
{
	if (SoundCookiePtr) {
		SoundCookiePtr[0] = (Word)-1;	/* Save the channel allocated */
		SoundCookiePtr = 0;				/* Reset the cookie pointer */
	}
	return (Word)-1;
}

/**********************************

	Generic routine to start up the digital sound driver

**********************************/

Word BURGERCALL InitDigitalDriver(void)
{
	return TRUE;
}

/**********************************

	Generic routine to shut down the digital sound driver

**********************************/

void BURGERCALL KillDigitalDriver(void)
{
}

#endif
