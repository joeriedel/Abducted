/**********************************

	Header for MAD music player
	
**********************************/

#ifndef __MADPLAYER_H__
#define __MADPLAYER_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __FMFILE_H__
#include "FmFile.h"
#endif

#ifndef __SNSOUND_H__
#include "SNSound.h"
#endif

#if defined(__MAC__)
#ifndef __SOUND__
#include <sound.h>
#endif
#endif

#define DEFAULT_VOLFADE		300L
#define MAXINSTRU			255L
#define MAXPOINTER			999L
#define MAXTRACK			256L

typedef struct Cmd_t {			// COMMAND
	Word8 ins;					// Instrument no 0x00: no ins cmd
	Word8 note;					// Note, see table 0xFF : no note cmd
	Word8 cmd;					// Effect cmd
	Word8 arg;					// Effect argument
	Word8 vol;					// Volume 0xFF : no volume cmd
	Word8 Filler;					// Pad to a short! 
} Cmd_t;

typedef struct PatHeader_t {	// HEADER
	long size;					// Length of pattern: standard = 64
	long compMode;				// Compression mode, none = 'NONE'
	char name[32];
	long patBytes;				// Pattern Size in Bytes
	long unused2;
} PatHeader_t;

typedef struct PatData_t {		// DATA STRUCTURE : HEADER + COMMANDS Pattern = 64 notes to play
	PatHeader_t header;
	Cmd_t Cmds[1];
} PatData_t;

typedef struct sData_t {	// SAMPLE
	long size;				// Sample length
	long loopBeg;			// LoopStart
	long loopSize;			// LoopLength
	Word16 c2spd;			// c2spd
	Word8 vol;				// Base volume
	Word8 loopType;
	Word8 amp;				// 8 or 16 bits
	char relNote;
	Word8 stereo;			// Stereo
	Word8 Filler1;
	char name[32];			// Sample name
	char *data;				// Used only in memory, not in files
} sData_t;

enum {
	eClassicLoop	= 0,
	ePingPongLoop	= 1
};

typedef struct EnvRec {		// Volume Enveloppe
	short pos;				// pos
	short val;				// val
} EnvRec;

typedef struct InstrData {		// INSTRUMENT
	char name[32];			// instrument name
	Word8 type;				// Instrument type = 0
	Word8 no;				// Instrument number
	
	short firstSample;		// First sample ID in sample list
	short numSamples;		// Number of samples in instrument
	
	/**/
	
	Word8 what[96];			// Sample number for all notes
	EnvRec volEnv[ 12];		// Points for volume envelope
	EnvRec pannEnv[ 12];	// Points for panning envelope
	
	Word8 volSize;			// Number of volume points
	Word8 pannSize;			// Number of panning points
	
	Word8 volSus;			// Volume sustain point
	Word8 volBeg;			// Volume loop start point
	Word8 volEnd;			// Volume loop end point
	
	Word8 pannSus;			// Panning sustain point
	Word8 pannBeg;			// Panning loop start point
	Word8 pannEnd;			// Panning loop end point
	
	Word8 volType;			// Volume type: bit 0: On; 1: Sustain; 2: Loop
	Word8 pannType;			// Panning type: bit 0: On; 1: Sustain; 2: Loop
	
	Word16 volFade;	// Volume fadeout
	
	Word8 vibDepth;
	Word8 vibRate;
} InstrData;

enum {
	EFON		= 1,
	EFSUSTAIN	= 2,
	EFLOOP		= 4
};

typedef struct MADSpec {			/* Loaded in */
	long MAD;						// Mad Identification
	char name[ 32];					// Music's name
	char infos[ 253];				// Informations & Author name of the music
	Word8 generalPitch;				// General Pitch
	Word8 generalSpeed;				// General Speed
	Word8 generalVol;				// Software general volume
	Word8 numPat;					// Patterns number
	Word8 numChn;					// Channels number
	Word8 numPointers;				// Partition length
	Word8 numInstru;					// Instruments number
	Word8 numSamples;				// Samples number
	Word8 oPointers[ MAXPOINTER];	// Partition : Patterns ID List
	short speed;					// Default speed
	short tempo;					// Default tempo
	Word8 chanPan[ MAXTRACK];		// Channel settings, from 0 to 256
	Word8 chanVol[ MAXTRACK];		// Channel Volume, from 0 to 64
} MADSpec;

#define NUMBER_NOTES 96
#define NOFINETUNE 8363
#define MIN_VOLUME 0
#define MAX_VOLUME 64
#define MAX_PANNING 64
#define MAXSAMPLE 64L
#define MAXPATTERN 200
#define MAXPATTERNSIZE 900
#define MAX_ARP 3
#define MAXPLUG 40

enum {
	MADNeedMemory = -1,
	MADReadingErr = -2,
	MADIncompatibleFile = -3,
	MADLibraryNotInitialized = -4,
	MADParametersErr = -5,
	MADUnknowErr = -6,
	MADSoundManagerErr = -7,
	MADOrderNotImplemented = -8,
	MADFileNotSupportedByThisPlug = -9,
	MADCannotFindPlug = -10,
	MADMusicHasNoDriver = -11,
	MADDriverHasNoMusic	= -12
};

typedef struct Channel {
	long ID;					// Channel ID - 0 to MAXTRACK

	char *begPtr;				// Sample Data Ptr - Beginning of data
	char *maxPtr;				// Sample Data Ptr - End of data
	char *curPtr;				// Sample Data Ptr - Current position
	long sizePtr;			// Sample Size in bytes

	long		amp;				// Sample amplitude: 8 or 16 bits

	long		loopBeg;			// Loop Beginning
	long		loopSize;			// Loop Size

	long		ins;				// Current Instrument ID
	long		insOld;				// Previous Instrument ID played on this channel
	long		samp;				// Current Sample ID

	long		fineTune;			// Finetune

	long		note;				// Note
	long		noteOld;			// Previous note played on this channel

	long 		period;				// Current period
	long		periodOld;			// Previous period played on this channel

	long		vol;				// Channel vol (0 to 64)
	long		pann;				// Channel pan (0 to 64)

	long 		cmd;				// Command
	Word8		arg;				// Argument of command
	Word8		volcmd;				// Volume Command
	short Filler2;
	
	long 		arp[ MAX_ARP];		// Used for arpeggio command
	long 		arpindex;			// Used for arpeggio command

	long		viboffset;			// Used for vibrato command
	long 		vibdepth;			// Used for vibrato command
	long 		vibrate;			// Used for vibrato command

	long 		slide;				// Used for slideUp and slideDown command

	long 		pitchgoal;			// Used for portamento command
	long 		pitchrate;			// Used for portamento command

	long 		volumerate;			// Used for slideVolume command

	long		oldArg[ 16];

	short a;
	short b;
	short p;
	short aa;
	short bb;
	short pp;
	
	long volEnv;
	long volFade;
	long pannEnv;
	long lAC;

	char *prevPtr;
	long lastWordL, curLastWordL;
	long lastWordR, curLastWordR;
	long curLevelL, curLevelR;

	Word8 LevelDirectionL, LevelDirectionR, RemoverWorking;
	Word8 KeyOn;

	long prevVol0;
	long prevVol1;

	/**/

	short GPat, GReader;

	/**/

	Word8 GEffect;
	Word8 stereo;
	Word8 loopType;
	Word8 pingpong;

	long preOff;
	char preVal, preVal2;
	char preValR, preVal2R;

	short spreVal, spreVal2;
	short spreValR, spreVal2R;
	long TICKREMOVESIZE;
} Channel;

typedef struct MADMusic {
	MADSpec *header;								// Music Header - See 'MAD.h'
	PatData_t *partition[ MAXPATTERN];				// Patterns
	InstrData *fid;									// Instruments
	sData_t **sample;								// Samples
	Bool musicUnderModification;					// Tell the driver to NOT access music data
	Word8 Padding[3];
} MADMusic;

enum {
	SoundManagerDriver=1,			// MAC ONLY You should use only SoundManagerDriver for full compatibility !
	BeOSSoundDriver,				// BE ONLY when using with BeOS compatible systems ! - NOT FOR MAC
	DirectSound95NT,				// WINDOWS 95/NT ONLY when using with PC compatible systems ! - NOT FOR MAC
	NoHardwareDriver				// NO HARDWARE CONNECTION, will not produce any sound
};

typedef struct MADDriverSettings {
	long numChn;				// Active tracks from 2 to 32, automatically setup when a new music is loaded
	short outPutBits;			// 8 or 16 Bits
	short driverMode;			// SoundManagerDriver, BeOSSoundDriver, DirectSound95NT
	Word32 outPutRate;		// Fixed32 number, by example : rate44khz, rate22050hz, rate22khz, rate11khz, rate11025hz
	long MicroDelaySize;		// Micro delay duration (in ms, max 1 sec = 1000 ms, min = 0 ms)
	Word8 repeatMusic;			// If music finished, repeat it or stop.
	Word8 surround;				// Surround effect active? true/false
	Word8 Reverb;				// Reverb effect active? true/false
	Word8 TickRemover;			// Remove volume/sample/loop ticks.
	long ReverbSize;			// Reverb delay duration (in ms, min = 25 ms, max 1 sec = 1000 ms)
	long ReverbStrength;		// Reverb strength in % (0 <-> 70)
} MADDriverSettings;

typedef struct MADLibrary_t {
	Word TotalPlug;					// no of Plugs in pointer ThePlug
	MADImportPtr ThePlug[MAXPLUG];		// Pointers on plugs code & infos
} MADLibrary_t;

typedef struct MADDriverRec {	
	MADDriverSettings DriverSettings;		// Driver SetUp -- READ ONLY --
	MADMusic *curMusic;						// Current music played by this driver, it can be 0L !!!
	MADLibrary_t *lib;
	Word8 Reading;						// Reading indicator
	Word8 JumpToNextPattern;				// Shall I jump to the next pattern?
	Word8 MADPlay;
	Word8 Filler1;
	short PartitionReader;					// Current position in pattern (0...999)
	short Pat;								// Current ID Pattern, see 'Patterns list'
	short PL;								// Current position in partition, see 'Partition list'
	short speed;							// Current speed, see speed Effect
	short finespeed;						// Current finespeed, see speed Effect
	short VExt;								// External music speed, see 'Adaptators' window. 80 = normal
	short FreqExt;							// External music pitch, see 'Adaptators' window. 80 = normal
	short smallcounter;
	long VolGlobal;							// Global SOFTWARE volume (This is NOT Mac hardware volume!) from 0 to 64
	Bool Active[ MAXTRACK];				// Channel Active?
	
	long MIN_PITCH, MAX_PITCH;
	char *IntDataPtr;
	long ASCBUFFER;
	long BufSize;
	long VSYNC, BufCounter, BytesToGenerate;
	short vibrato_table[ 64];
	char * OverShoot;
	long *DASCBuffer;
	short *DASCBuffer8;
	long MDelay;
	long RDelay;
	char *ReverbPtr;

	#if defined(__WIN32__)
	struct IDirectSoundBuffer*lpSwSamp;		// ONLY available if you are using Win95 DirectSound driver
	#endif	
	
	#if defined(__MAC__)
	Word8 *SndBuffer;					/* Audio buffer */
	struct SndChannel *MusicChannelPP;	/* The SndChannelPtr to apply SndDoCommand, etc. */
	ExtSoundHeader SndHeader;			/* Header to pass to MacOS */
	SndCallBackUPP CallBackUPP;			/* UPP pointer for sound manager callbacks */
	#endif
	Channel chan[MAXTRACK];				// Current driver channels (BIG!)
} MADDriverRec;

enum {
	arpeggioE 		= 0,	//	0x00
	downslideE 		= 1,	//	0x01
	upslideE 		= 2,	//	0x02
	portamentoE 	= 3,	//	0x03
	vibratoE 		= 4,	//	0x04
	portaslideE 	= 5,	//	0x05
	vibratoslideE	= 6,	//	0x06
	nothingE 		= 7,	//	0x07
	panningE		= 8,	//	0x08
	offsetE 		= 9,	//	0x09
	slidevolE 		= 10,	//	0x0A
	fastskipE 		= 11,	//	0x0B
	volumeE 		= 12,	//	0x0C
	skipE 			= 13,	//	0x0D
	extendedE 		= 14,	//	0x0E
	speedE 			= 15	//	0x0F
};

#define AMIGA_CLOCKFREQ2	14317456L		//3575872L -- 14303488L
#define BYTEDIV	8L
#define	EXTRASMALLCOUNTER	5

#ifdef __cplusplus
extern "C" {
#endif

/* OS Specific file */

extern int BURGERCALL MADSndOpen(MADDriverRec *intDriver);
extern void BURGERCALL MADSndClose( MADDriverRec *inMADDriver);
extern void	BURGERCALL MADGetBestDriver( MADDriverSettings *DriverInitParam);		
extern void BURGERCALL MADStartDriver( MADDriverRec *MDriver);
extern void BURGERCALL MADStopDriver( MADDriverRec *MDriver);

/* SnMADMainDriver.c */

extern int BURGERCALL MADCreateDriver( MADDriverSettings *DriverInitParam, MADLibrary_t *MADLib, MADDriverRec** returnDriver);
extern void BURGERCALL MADDisposeDriver( MADDriverRec *MDriver);
extern int BURGERCALL MADAttachDriverToMusic( MADDriverRec *driver, MADMusic *music);
extern int BURGERCALL MADLoadMusicPtr( MADLibrary_t *lib, MADMusic **music,const Word8 * myPtr,Word32 Length);
extern void BURGERCALL MADPurgeTrack( MADDriverRec *intDriver);
extern void BURGERCALL MADResetInstrument( InstrData		*curIns);
extern void BURGERCALL MADKillInstrument( MADMusic*,Word ins);
extern void BURGERCALL MADCheckSpeed( MADMusic *MDriver, MADDriverRec *intDriver);
extern int BURGERCALL MADPlayMusic( MADDriverRec *MDriver);
extern int BURGERCALL MADStopMusic( MADDriverRec *MDriver);
extern void BURGERCALL MADReset( MADDriverRec *MDriver);
extern void BURGERCALL MADMusicDestroy( MADMusic *);
extern Cmd_t* BURGERCALL GetMADCommand(int position,int channel,PatData_t* aPatData);

/* SnMADInterrupt.c */

extern long BURGERCALL DoVolPanning256(Word whichChannel, Channel *ch, MADDriverRec *intDriver);
extern void BURGERCALL MADCleanDriver(MADDriverRec *intDriver);
extern long BURGERCALL GetOldPeriod(int note,long c2spd);
extern void BURGERCALL NoteAnalyse( MADDriverRec *intDriver);

/* SnMADDelayOutput.c */

extern int BURGERCALL MADCreateOverShoot(MADDriverRec *intDriver);
extern void	BURGERCALL MADKillOverShoot(MADDriverRec *intDriver);
extern void BURGERCALL MADPlay16StereoDelay(MADDriverRec *intDriver);
extern void BURGERCALL MADPlay8StereoDelay(MADDriverRec *intDriver);

/* SnMADEffects.c */

extern void BURGERCALL MADDoEffect(Channel *ch,int call, MADDriverRec *intDriver);
extern void BURGERCALL MADSetUpEffect(Channel *ch,MADDriverRec *intDriver);
extern void BURGERCALL MADDoVolCmd(Channel *ch,int call);

/* SnMADTickRemover.c */

extern void BURGERCALL MADTickLoopFill8(Channel *curVoice,long *LeftBufferPtr,long *RightBufferPtr,long size,int left,int right);
extern void BURGERCALL MADTickRemoverStart8(Channel *curVoice,long *ASCBuffer1,long *ASCBuffer2,MADDriverRec *intDriver);

#ifdef __cplusplus
}
#endif
#endif
