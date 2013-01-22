/**********************************

	MADI file importer

**********************************/

#include "SnMadMusic.h"
#include <BREndian.hpp>
#include "ClStdLib.h"
#include "MmMemory.h"

#define INFOSSIZE 242

typedef struct MADSpecFile {
	long MAD;						// Mad Identification
	char name[ 32];					// Music's name
	char infos[ INFOSSIZE];			// Informations & Author name of the music
	long EPitch;					// New Pitch
	long ESpeed;					// New Speed
	Word8 XMLinear;					// Linear picth table?
	Word8 MODMode;					// Limit pitch to MOD pitch table
	Word8 showCopyright;				// Show infos at startup? true or false
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
	Word8 chanPan[MAXTRACK];			// Channel settings, from 0 to 256
	Word8 chanVol[MAXTRACK];			// Channel Volume, from 0 to 64
} MADSpecFile;

typedef struct sDataFile_t {	// SAMPLE
	long size;				// Sample length
	long loopBeg;			// LoopStart
	long loopSize;			// LoopLength
	Word8 vol;				// Base volume
	Word8 Filler1;
	unsigned short c2spd;	// c2spd
	Word8 loopType;
	Word8 amp;				// 8 or 16 bits
	char relNote;
	char name[ 32];			// Sample name
	Word8 stereo;			// Stereo
	char *data;				// Used only in memory, not in files
} sDataFile_t;

enum {
	ins 	= 1,
	note	= 2,
	cmd		= 4,
	argu	= 8,
	vol		= 16
};

/**********************************

	Decompress MAD1 file data

**********************************/

static PatData_t* DecompressPartitionMAD1( MADMusic *MDriver, PatData_t* myPat)
{
	PatData_t* finalPtr;
	Word8 *srcPtr;
	Cmd_t *myCmd;
	int maxCmd;
	Word set;

	finalPtr = ( PatData_t*)AllocAPointer(sizeof(PatHeader_t) + myPat->header.size * MDriver->header->numChn * sizeof( Cmd_t));
	if (finalPtr) {
		FastMemCpy(finalPtr,myPat,sizeof( PatHeader_t));

		srcPtr = (Word8*) myPat->Cmds;
		myCmd = finalPtr->Cmds;
		maxCmd = finalPtr->header.size * MDriver->header->numChn;

		/*** Decompression Routine ***/

		if (maxCmd) {
			do {
				myCmd->cmd 	= 0;
				myCmd->note = 0xFF;
				myCmd->arg = 0;
				myCmd->ins = 0;
				myCmd->vol = 0xFF;
				set = srcPtr[0];
				++srcPtr;
				if (set & ins)	{
					myCmd->ins = srcPtr[0];
					++srcPtr;
				}
				if (set & note)	{
					myCmd->note = srcPtr[0];
					++srcPtr;
				}
				if (set & cmd)	{
					myCmd->cmd = srcPtr[0];
					++srcPtr;
				}
				if (set & argu)	{
					myCmd->arg = srcPtr[0];
					++srcPtr;
				}
				if (set & vol)	{
					myCmd->vol = srcPtr[0];
					++srcPtr;
				}
				++myCmd;
			} while (--maxCmd);
		}
	}
	return finalPtr;			/* Could be NULL */
}

/**********************************

	Read in a file and store the records

**********************************/

int BURGERCALL ModMusicMADI(const Word8 *MADPtr,Word32 /* Length */,struct MADMusic *MDriver)
{
	int i, x;
	long inOutCount, OffSetToSample;
	PatHeader_t tempPatHeader;
	PatData_t* tempPat;

	if (MADPtr[0] != 'M' ||
		MADPtr[1] != 'A' ||
		MADPtr[2] != 'D' ||
		MADPtr[3] != 'I') {
		return MADFileNotSupportedByThisPlug;
	}
	FastMemSet(MDriver,0,sizeof(MADMusic));

	inOutCount = sizeof( MADSpec);
	MDriver->header = (MADSpec*) AllocAPointerClear( inOutCount);
	if( MDriver->header == 0L) {
		return MADNeedMemory;
	}

	FastMemCpy( MDriver->header, MADPtr, inOutCount);
	OffSetToSample = inOutCount;

	#if defined(__LITTLEENDIAN__)
	MDriver->header->MAD = Burger::SwapEndian(MDriver->header->MAD);
	MDriver->header->speed = Burger::SwapEndian(MDriver->header->speed);
	MDriver->header->tempo = Burger::SwapEndian(MDriver->header->tempo);
	#endif

/**** PARTITION ****/

	for( i = 0; i < MDriver->header->numPat; i++) {
	/** Lecture du header de la partition **/
		inOutCount = sizeof( PatHeader_t);
		FastMemCpy( &tempPatHeader, MADPtr + OffSetToSample, inOutCount);

		#if defined(__LITTLEENDIAN__)
		tempPatHeader.size = Burger::SwapEndian(tempPatHeader.size);
		tempPatHeader.compMode = Burger::SwapEndian(tempPatHeader.compMode);
		tempPatHeader.patBytes = Burger::SwapEndian(tempPatHeader.patBytes);
		tempPatHeader.unused2 = Burger::SwapEndian(tempPatHeader.unused2);
		#endif
		/*************************************************/
		/** Lecture du header + contenu de la partition **/
		/*************************************************/

		switch( tempPatHeader.compMode) {
		case 'MAD1':
			inOutCount = sizeof(PatHeader_t) + tempPatHeader.patBytes;
			break;

		case 'NONE':
		default:
			inOutCount = sizeof(PatHeader_t) + MDriver->header->numChn * tempPatHeader.size * sizeof( Cmd_t);
			break;
		}

		MDriver->partition[ i] = ( PatData_t*) AllocAPointerClear( inOutCount);
		if( MDriver->partition[ i] == 0L) {
			MADMusicDestroy(MDriver);
			return MADNeedMemory;
		}
		FastMemCpy(MDriver->partition[ i], MADPtr + OffSetToSample,  inOutCount);
		OffSetToSample += inOutCount;

		#if defined(__LITTLEENDIAN__)
		MDriver->partition[ i]->header.size = Burger::SwapEndian(MDriver->partition[ i]->header.size);
		MDriver->partition[ i]->header.compMode = Burger::SwapEndian(MDriver->partition[ i]->header.compMode);
		MDriver->partition[ i]->header.patBytes = Burger::SwapEndian(MDriver->partition[ i]->header.patBytes);
		MDriver->partition[ i]->header.unused2 = Burger::SwapEndian(MDriver->partition[ i]->header.unused2);
		#endif
		if( MDriver->partition[ i]->header.compMode == 'MAD1') {
			tempPat = DecompressPartitionMAD1( MDriver, MDriver->partition[ i]);

			if (tempPat == 0L) {
				MADMusicDestroy(MDriver);
				return MADNeedMemory;
			}

			DeallocAPointer(MDriver->partition[ i]);
			MDriver->partition[ i] = tempPat;
		}
	}

/**** INSTRUMENTS ****/
	MDriver->fid = ( InstrData*) AllocAPointerClear( sizeof( InstrData) * (long) MAXINSTRU);
	if( !MDriver->fid) {
		MADMusicDestroy(MDriver);
		return MADNeedMemory;
	}

	inOutCount = sizeof( InstrData) * (long) MDriver->header->numInstru;
	FastMemCpy( MDriver->fid, MADPtr + OffSetToSample, inOutCount);
	OffSetToSample += inOutCount;

	for( i = MDriver->header->numInstru-1; i >= 0 ; i--) {
		InstrData	*curIns = &MDriver->fid[ i];

		#if defined(__LITTLEENDIAN__)
		curIns->numSamples = Burger::SwapEndian(curIns->numSamples);
		curIns->firstSample = Burger::SwapEndian(curIns->firstSample);
		curIns->volFade = Burger::SwapEndian(curIns->volFade);

		for( x = 0; x < 12; x++) {
			curIns->volEnv[ x].pos = Burger::SwapEndian(curIns->volEnv[ x].pos);
			curIns->volEnv[ x].val = Burger::SwapEndian(curIns->volEnv[ x].val);
		}

		for( x = 0; x < 12; x++) {
			curIns->pannEnv[ x].pos = Burger::SwapEndian(curIns->pannEnv[ x].pos);
			curIns->pannEnv[ x].val = Burger::SwapEndian(curIns->pannEnv[ x].val);
		}
		#endif

		if( i != curIns->no) {
			MDriver->fid[ curIns->no] = *curIns;
			MADResetInstrument( curIns);
		}
	}
	MDriver->header->numInstru = MAXINSTRU;

/**** SAMPLES ****/
	MDriver->sample = ( sData_t**) AllocAPointerClear( sizeof( sData_t*) * (long) MAXINSTRU * (long) MAXSAMPLE);
	if( !MDriver->sample) {
		MADMusicDestroy(MDriver);
		return MADNeedMemory;
	}

	for( i = 0; i < MAXINSTRU ; i++) {
		for( x = 0; x < MDriver->fid[ i].numSamples ; x++) {
			sData_t	 *curData;

		// ** Read Sample header **

			curData = MDriver->sample[ i*MAXSAMPLE + x] = (sData_t*) AllocAPointer(sizeof( sData_t));
			if( curData == 0L) {
				MADMusicDestroy(MDriver);
				return MADNeedMemory;
			}

			{
				sDataFile_t *SrcPtr;
				SrcPtr = (sDataFile_t *)(MADPtr+OffSetToSample);
				curData->size = Burger::LoadBig(SrcPtr->size);
				curData->loopBeg = Burger::LoadBig(SrcPtr->loopBeg);
				curData->loopSize = Burger::LoadBig(SrcPtr->loopSize);
				curData->c2spd = Burger::LoadBig(SrcPtr->c2spd);
				curData->vol = SrcPtr->vol;
				curData->loopType = SrcPtr->loopType;
				curData->amp = SrcPtr->amp;
				curData->relNote = SrcPtr->relNote;
				curData->stereo = SrcPtr->stereo;
				FastMemCpy(curData->name,SrcPtr->name,32);;
				curData->data = 0;
			}
			OffSetToSample += sizeof(sDataFile_t);

		// ** Read Sample DATA

			curData->data = static_cast<char *>(AllocAPointer(curData->size));
			if( curData->data == 0L) {
				MADMusicDestroy(MDriver);
				return MADNeedMemory;
			}

			inOutCount = curData->size;
			FastMemCpy(curData->data,  MADPtr + OffSetToSample, inOutCount);
			OffSetToSample += inOutCount;

			#if defined(__LITTLEENDIAN__)
			if (curData->amp == 16) {
				long 	ll;
				short	*shortPtr = (short*) curData->data;

				for( ll = 0; ll < curData->size/2; ll++) {
					shortPtr[ll] = Burger::SwapEndian(shortPtr[ll]);
				}
			}
			#endif
		}
	}

	for( i = 0; i < MAXINSTRU; i++) {
		MDriver->fid[ i].firstSample = (short)(i * MAXSAMPLE);
	}
	MDriver->header->MAD = 'MADI';
	return( 0);
}
