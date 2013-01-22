/*******************************

	Sound Manager

*******************************/

#ifndef __SNSOUND_H__
#define __SNSOUND_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

/* Sound manager private data */

#if defined(__MSDOS__) && defined(__DOS4G__)		/* PC-DOS uses HMI for sound */
#include <sosm.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct RawSound_t;
struct MADMusic;

/* Needed for streaming audio */

typedef struct FooWAVEFORMATEX {
    Word16 wFormatTag;			/* format type */
    Word16 nChannels;			/* number of channels (i.e. mono, stereo...) */
    Word32 nSamplesPerSec;	/* sample rate */
    Word32 nAvgBytesPerSec;	/* for buffer estimation */
    Word16 nBlockAlign;		/* block size of data */
    Word16 wBitsPerSample;	/* number of bits per sample of mono data */
    Word16 cbSize;			/* the count in bytes of the size of extra information (after cbSize) */
} FooWAVEFORMATEX;

/* Public defines */

#define MAXVOICECOUNT 32	/* Number of active voices */

/* Private structures */

typedef struct PrivSound_t {	/* Internal sound struct */
	Word8 *SoundPtr;			/* Pointer to the data (NULL if not used) */
	Word32 SoundLength;	/* Length of the data in bytes */
	Word32 LoopStart;		/* Sample to start from */
	Word32 LoopEnd;		/* Sample to end the loop (0 for no looping) */
	void (BURGERCALL *Proc)(void *);	/* Completion routine */
	void *Data;				/* Data to pass for completion routine */
	struct RezHeader_t *SampleRezHeader;	/* Resource type */
	Word *CookiePtr;		/* Pointer to the cookie reference */
	Word SampleResource;	/* Resource ID number */
	Word DataType;			/* Type of data found */
	Word SampleRate;		/* Samples per second to play */
	Word PanPosition;		/* Stereo position of the sound */
	Word Volume;			/* Volume for the sound sample */
	Word Dirty;				/* Sample rate, Pan or volume is different */
#if defined(__MSDOS__) && defined(__DOS4G__)
	HANDLE SampleHandle;	/* HMI Sample handle */
	_SOS_SAMPLE SampleRecord;	/* HMI Sample record */
#elif defined(__WIN32__)
	struct IDirectSoundBuffer *SampleHandle;	/* Direct sound buffer */
	FooWAVEFORMATEX SampleRecord;	/* Describe the buffer */
#elif defined(__MAC__)
	struct SndChannel *SampleHandle;	/* Sound manager channel */
	Word32 WorkOffset;		/* Current data marker */
	void *CallBackUPP;			/* UPP pointer */
#endif
} PrivSound_t;

#define MIDIKILL 0
#define DIGIMUSICKILL 1
#define DIGISOUNDKILL 2

typedef void (BURGERCALL *SndKillProcPtr)(void);
extern SndKillProcPtr BurgerSndKillProcs[3];	/* Shut down procs */
extern Word BurgerSamplePriority[MAXVOICECOUNT];	/* Sound priority chain */
extern PrivSound_t BurgerSamples[MAXVOICECOUNT+1];
extern Word BurgerVoiceCount;	/* Maximum number of sound voices */
extern Word MusicVolume;		/* Music volume */
extern Word MasterSoundVolume;	/* Master sound volume */

extern Word BurgerLastSong;		/* Song number currently playing */
extern Word BurgerSongFreq;		/* Samples per second for current song */
extern Word BurgerSongLoops;	/* True if the song loops */

#define DIGIMUSICON 1		/* Flags for BurgerSndExitIn */
#define DIGISOUNDON 2
#define MIDIMUSICON 4

extern Word BurgerSndExitIn;		/* True if atexit proc is installed */
extern Word32 BurgerMidiSongPtr;	/* Handle or pointer to current MIDI song data */

/* Private internal routines */

typedef Word (BURGERCALL *SndIteratorProcPtr)(PrivSound_t *,void *);
extern Word BURGERCALL SoundIterator(SndIteratorProcPtr Proc,void *Data,Word32 Num,Word OneHit);
extern void BURGERCALL KillDigitalDriver(void);
extern Word BURGERCALL InitDigitalDriver(void);
extern void BURGERCALL EnableSoundShutdownProc(void);
extern void BURGERCALL DigitalSoundCheckOff(void *Input);
extern Word BURGERCALL SoundSilenceByte(const PrivSound_t *Input);

#if defined(__MSDOS__) && defined(__DOS4G__)
extern HANDLE hDIGITimer;		/* handle to digital mixer */
#define TIMERON 8		/* Extra flags for BurgerSndExitIn */
#define TIMERATEXIT 16
extern void BURGERCALL HMIInitSoundTimer(void);
extern HANDLE hDIGIDriver;		/* handle to digital driver */

#elif defined(__WIN32__)	/* Global data for Direct sound support */
extern struct IDirectSound *MyDirectSoundDevice;
extern struct IDirectSoundBuffer *MyDirectSoundBuffer;
extern short Win95DirectSoundVolumes[256];	/* Table for volume translation */
#endif

/* These are public */

/* Sound handlers */

#define SOUND_EXCLUSIVE 0x80000000UL
#define SOUND_COOKIE 0x40000000UL

#define SOUNDTYPEBYTE 0		/* Unsigned 8 bit data */
#define SOUNDTYPECHAR 1		/* Signed 8 bit data */
#define SOUNDTYPELSHORT 2	/* Little endian short */
#define SOUNDTYPEBSHORT 3	/* Big endian short */
#define SOUNDTYPEADPCM 4	/* MS ADPCM compression */
#define SOUNDTYPEDVIPCM 5	/* Intel DVI ADPCM compression */
#define SOUNDTYPEMP3 6		/* MP3 Audio */
#define SOUNDTYPEULAW 7		/* MuLaw */
#define SOUNDTYPEALAW 8		/* ALaw */
#define SOUNDTYPEMACE3 9	/* Mace 3:1 */
#define SOUNDTYPEMACE6 10	/* Mace 6:1 */
#define SOUNDTYPEOGG 11		/* OGG/Vorbis Audio */
#define SOUNDTYPESTEREO 0x8000	/* Stereo data */
#define SOUNDTYPEDOUBLEBUFFER 0x4000	/* Double buffered */

/* SystemState defines */

#define SfxActive 1
#define MusicActive 2
#define PauseActive 4

typedef void (BURGERCALL *SndCompleteProcPtr)(void *);
typedef char *(BURGERCALL *MakeSongProc)(Word);

typedef struct RawSound_t {
	Word8 *SoundPtr;			/* Pointer to the data */
	Word32 SoundLength;	/* Length of the data in bytes */
	Word32 LoopStart;		/* Sample to start from */
	Word32 LoopEnd;		/* Sample to end the loop (0 for no looping) */
	SndCompleteProcPtr Proc;	/* Completion routine */
	void *Data;				/* Data to pass for completion routine */
	Word DataType;			/* Type of data found */
	Word SampleRate;		/* Samples per second to play */
	Word8 *CompPtr;			/* Used by compression */
	Word Extra1;			/* Used by compression */
	Word Extra2;			/* Used by compression */
	Word Extra3;			/* Used by compression */
	Word32 SampleCount;	/* Output from ParseSoundFileImage */
	float TimeInSeconds;	/* Output from ParseSoundFileImage */
} RawSound_t;

#define MUSICCODECINIT 0
#define MUSICCODECDESTROY 1
#define MUSICCODECDECODE 2
#define MUSICCODECRESET 3

typedef Word32 (BURGERCALL *DecodeCallbackProc)(void *,Word8 *,Word32);
typedef Word32 (BURGERCALL *DecodeCodecProc)(struct DigitalMusicReadState_t *,Word,Word8 *,Word32);

typedef struct DigitalMusicReadState_t {	/* Private state structure */
	DecodeCallbackProc ReadProc;	/* Read data proc */
	void *CallBackParm;				/* Parm for the read callback */
	DecodeCodecProc CodecProc;		/* Decompressor */
	void *CompressStatePtr;			/* Extra data needed by codec */
	Word32 FileOffset;			/* Offset to where in the file data starts at */
	Word32 SoundLength;			/* Size of data to play (Decompressed) */
	Word32 BytesPlayed;			/* Number of bytes processed */
	Word DataType;					/* Type of input data (Codec index) */
} DigitalMusicReadState_t;

typedef int (BURGERCALL *MADImportPtr)(const Word8 *DataPtr,Word32 Length,struct MADMusic *MadFile);

extern Word SystemState;	/* Global game state flags */
extern Word SfxVolume;		/* Master volume of game sound effects */
extern Word *SoundCookiePtr;	/* Writeback handle for audio channel */
extern Word PanPosition;	/* Initial pan position of sound to play */
extern Word SoundLoopFlag;	/* True if you want the sound to loop */
extern int SoundFrequencyAdjust;	/* Pitch bend */
extern struct RezHeader_t *SoundRezHeader;	/* Resource file for sound loading */
extern MakeSongProc DigitalMusicNameCallback;

extern void BURGERCALL ModMusicInit(void);
extern void BURGERCALL ModMusicShutdown(void);
extern void BURGERCALL ModMusicImporter(MADImportPtr ImportPtr);
extern Word BURGERCALL ModMusicPlay(Word SongNum);
extern Word BURGERCALL ModMusicPlayByFilename(const char *FileName);
extern Word BURGERCALL ModMusicPlayByPtr(const Word8 *DataPtr,Word32 Length);
extern void BURGERCALL ModMusicStop(void);
extern void BURGERCALL ModMusicPause(void);
extern void BURGERCALL ModMusicResume(void);
extern void BURGERCALL ModMusicReset(void);
extern Word BURGERCALL ModMusicGetVolume(void);
extern void BURGERCALL ModMusicSetVolume(Word NewVolume);
extern int BURGERCALL ModMusicS3M(const Word8 *DataPtr,Word32 Length,struct MADMusic *MadFile);
extern int BURGERCALL ModMusicMADI(const Word8 *DataPtr,Word32 Length,struct MADMusic *MadFile);
extern int BURGERCALL ModMusicIT(const Word8 *DataPtr,Word32 Length,struct MADMusic *MadFile);
extern int BURGERCALL ModMusicXM(const Word8 *DataPtr,Word32 Length,struct MADMusic *MadFile);

extern Word32 BURGERCALL DigitalMusicByte(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicChar(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicULaw(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicALaw(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicLShort(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicBShort(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicMace3(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicMace6(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicADPCM(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicOgg(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word BURGERCALL DigitalMusicGetSilenceVal(Word Type);
extern Word BURGERCALL DigitalMusicDecode(DigitalMusicReadState_t *Input,Word8 *DestBuffer,Word32 Length);
extern Word BURGERCALL DigitalMusicReadStateInit(DigitalMusicReadState_t *Output,struct RawSound_t *Input,Word8 *ImagePtr,Word32 MaxSize,DecodeCallbackProc Proc,void *Parm);
extern void BURGERCALL DigitalMusicReadStateDestroy(DigitalMusicReadState_t *Input);
extern void BURGERCALL DigitalMusicReset(DigitalMusicReadState_t *Input);
extern void BURGERCALL DigitalMusicInit(void);
extern void BURGERCALL DigitalMusicShutdown(void);
extern Word BURGERCALL DigitalMusicIsPlaying(void);
extern Word BURGERCALL DigitalMusicGetFrequency(void);
extern Word BURGERCALL DigitalMusicGetVolume(void);
extern void BURGERCALL DigitalMusicSetFilenameProc(MakeSongProc Proc);
extern void BURGERCALL DigitalMusicSetFrequency(Word Freq);
extern void BURGERCALL DigitalMusicSetVolume(Word Volume);
extern void BURGERCALL DigitalMusicPlay(Word SongNum);
extern void BURGERCALL DigitalMusicPause(void);
extern void BURGERCALL DigitalMusicResume(void);

extern void BURGERCALL InitSoundPlayer(void);
extern void BURGERCALL KillSoundPlayer(void);
extern void BURGERCALL StopASound(Word32 SoundCookie);
extern Word BURGERCALL PlayASound(Word32 SoundNum);
extern Word BURGERCALL PlayARawSound(RawSound_t *Input);
extern Word BURGERCALL ParseSoundFileImage(RawSound_t *Output,const void *Input,Word32 Length);
extern double BURGERCALL ConvertAiffExtended(const void *Input);
extern void * BURGERCALL FindIffChunk(const void *Input,Word32 Name,Word32 Length);
extern void BURGERCALL PauseAllSounds(void);
extern void BURGERCALL ResumeAllSounds(void);
extern void BURGERCALL StopAllSounds(void);
extern void BURGERCALL SetMaxSounds(Word Max);
extern Word BURGERCALL GetMaxSounds(void);
extern Word BURGERCALL GetNumSoundsPlaying(void);
extern Word BURGERCALL GetSfxVolume(void);
extern void BURGERCALL SetSfxVolume(Word NewVolume);
extern Word BURGERCALL IsASoundPlaying(Word32 SoundCookie);
extern Word BURGERCALL GetASoundFrequency(Word32 SoundCookie);
extern void BURGERCALL SetASoundFrequency(Word32 SoundCookie,Word Frequency);
extern Word BURGERCALL GetASoundVolume(Word32 SoundCookie);
extern void BURGERCALL SetASoundVolume(Word32 SoundCookie,Word Volume);
extern Word BURGERCALL GetASoundPan(Word32 SoundCookie);
extern void BURGERCALL SetASoundPan(Word32 SoundCookie,Word Pan);
extern void BURGERCALL SoundSetCallback(Word32 SoundCookie,SndCompleteProcPtr Proc,void *Data);

#ifdef __cplusplus
}
#endif

#endif

