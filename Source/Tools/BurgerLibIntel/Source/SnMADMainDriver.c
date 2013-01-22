/**********************************

	Master code for the MAD music player
	
**********************************/

#include "SnSound.h"
#include "FmFile.h"
#include "MmMemory.h"
#include "ClStdLib.h"
#include "RzRez.h"
#include "SnMadMusic.h"
#include <string.h>
#include <stdlib.h>

#if defined(__MAC__)
#include <Gestalt.h>
#include <Resources.h>
#include <ToolUtils.h>
#include <TextUtils.h>
#include <Sound.h>
#include <SoundInput.h>
#include <SoundComponents.h>
#else
#define rate22khz 0x56EE8BA3L
#endif

#define rate5khz	0x15BB9B5CUL
#define rate16khz	0x4132DDF2UL
#define rate48khz	0xBB800000UL

enum {
	MADFileType = 1,
	MADPtrType = 3
};

static Word ModMusicLastSongNum;	/* Last Mod song playing */
static Word ModMusicPaused;	/* Is the music paused? */
static MADDriverRec *ModMusicDriver;	/* Pointer to the currently loaded driver */
static MADMusic *ModMusicRec;		/* Pointer to the currently loaded song */
static MADLibrary_t ModLibrary;		/* Pointer to the library reference */

/**********************************

	Simple tick based routines
	
**********************************/

/**********************************

	Add to the left and right channels with
	a contant sample
	
**********************************/

void BURGERCALL MADTickLoopFill8(Channel *curVoice,long *LeftBufferPtr,long *RightBufferPtr,long size,int left,int right)
{
	curVoice->prevPtr = 0;			/* Zap the previous pointer */
	if (size>=0) {
		++size;
		do {
			long a,b;
			a = LeftBufferPtr[0];	/* Get the sample pair */
			b = RightBufferPtr[0];
			a = a+left;				/* Add the constants */
			b = b+right;
			LeftBufferPtr[0] = a;	/* Store the pair */
			RightBufferPtr[0] = b;
			LeftBufferPtr+=2;		/* Inc the pointers */
			RightBufferPtr+=2;
		} while (--size);			/* Done? */
	}
}

/**********************************

	Add the lastWord volume scaled to the volume to both
	the left and right channels
	
**********************************/

static void MADTickLoop8(long size, Channel *curVoice, long *ASCBuffer1, long *ASCBuffer2)
{
	long TICKREMOVESIZE;
	
	TICKREMOVESIZE = curVoice->TICKREMOVESIZE;
	
	if (TICKREMOVESIZE && size) {			/* Shall I remove something? */
		Word8 RemoverWorking;				/* Local register */
		long LastWord;						/* Last constant */
		long CurrentLevel;					/* Current volume level */
		long Count;							/* Temp counter */
		
		RemoverWorking = curVoice->RemoverWorking;
		LastWord = curVoice->curLastWordL;
		CurrentLevel = curVoice->curLevelL;
		Count = size;
		if (curVoice->LevelDirectionL) {
			do {
				if (CurrentLevel > 0) {
					--CurrentLevel;
				} else {
					RemoverWorking = FALSE;		/* Stop this */
				}
				ASCBuffer1[0] = ASCBuffer1[0]+((LastWord * CurrentLevel) / TICKREMOVESIZE);
				ASCBuffer1 += 2;
			} while (--Count);
		} else {
			do {
				long Temp;
				if (CurrentLevel < TICKREMOVESIZE) {
					++CurrentLevel;
				} else {
					RemoverWorking = FALSE;		/* Stop this */
				}
				Temp = (LastWord * CurrentLevel) / TICKREMOVESIZE;
				Temp -= LastWord;
				ASCBuffer1[0] = ASCBuffer1[0]+Temp;
				ASCBuffer1 += 2;
			} while (--Count);
		}				
		curVoice->curLevelL = CurrentLevel;	/* Save the new level */

		LastWord = curVoice->curLastWordR;
		CurrentLevel = curVoice->curLevelR;
		if (curVoice->LevelDirectionR) {
			do {			
				if (CurrentLevel > 0) {
					--CurrentLevel;
				} else {
					RemoverWorking = FALSE;		/* Stop this */
				}
				ASCBuffer2[0] = ASCBuffer2[0]+((LastWord * CurrentLevel) / TICKREMOVESIZE);
				ASCBuffer2 += 2;
			} while (--size);
		} else {
			do {
				long Temp;			
				if (CurrentLevel < TICKREMOVESIZE) {
					++CurrentLevel;
				} else {
					RemoverWorking = FALSE;		/* Stop this */
				}
				Temp = (LastWord * CurrentLevel) / TICKREMOVESIZE;
				Temp -= LastWord;
				ASCBuffer2[0] = ASCBuffer2[0]+Temp;
				ASCBuffer2 += 2;
			} while (--size);
		}
		curVoice->curLevelR = CurrentLevel;		/* Save the new level */
		curVoice->RemoverWorking = RemoverWorking;	/* Save the remover flag */
	}
}

/**********************************

	Add the lastWord volume scaled to the volume to both
	the left and right channels
	
**********************************/

void BURGERCALL MADTickRemoverStart8(Channel *curVoice,long *ASCBuffer1,long *ASCBuffer2,MADDriverRec *intDriver)
{
	long curDoVol0;			/* Requested left volume */
	long curDoVol1;			/* Requested right volume */
	
	curDoVol0 = DoVolPanning256(0,curVoice, intDriver);		/* Get the volume */
	curDoVol1 = DoVolPanning256(1,curVoice, intDriver);

	if (curVoice->prevPtr != curVoice->begPtr ||
		(curVoice->curPtr >= curVoice->maxPtr && curVoice->loopSize == 0) ||
		curVoice->prevVol0 != curDoVol0 ||			/* Any differance? */
		curVoice->prevVol1 != curDoVol1) {
		Word DoIT = FALSE;			/* No override */
		
		curVoice->LevelDirectionL = TRUE;
		curVoice->LevelDirectionR = TRUE;
		
		// Right Channel
		if (curVoice->prevVol0 != curDoVol0 && curVoice->prevPtr == curVoice->begPtr) {
			long diff;
			
			diff = curVoice->prevVol0 - curDoVol0;
			if (diff > 0) {
				if (curVoice->prevVol0) {		/* Don't divide by zero */
					curVoice->lastWordR -= (curVoice->lastWordR * curDoVol0) / curVoice->prevVol0;
				}
				curVoice->prevVol0	= curDoVol0;
				DoIT = TRUE;
			} else if( diff < 0) {
				if( curVoice->prevVol0) {		/* Don't divide by zero */
					curVoice->lastWordR -= (curVoice->lastWordR * curDoVol0) / curVoice->prevVol0;
				}
				curVoice->lastWordR = -curVoice->lastWordR;
				curVoice->prevVol0 = curDoVol0;
				curVoice->LevelDirectionR = FALSE;
				DoIT = TRUE;
			}
		} else if (curVoice->prevPtr == curVoice->begPtr) {
			curVoice->lastWordR = 0;
		}
		
		// Left Channel
		if( curVoice->prevVol1 != curDoVol1 && curVoice->prevPtr == curVoice->begPtr) {
			long diff;
			
			diff = curVoice->prevVol1 - curDoVol1;
			if (diff > 0) {
				if( curVoice->prevVol1) {		/* Don't divide by zero */
					curVoice->lastWordL -= (curVoice->lastWordL * curDoVol1) / curVoice->prevVol1;
				}
				curVoice->prevVol1 = curDoVol1;
				DoIT = TRUE;
			} else if (diff < 0) {
				if (curVoice->prevVol1) {		/* Don't divide by zero */
					curVoice->lastWordL -= (curVoice->lastWordL * curDoVol1) / curVoice->prevVol1;
				}
				curVoice->lastWordL = -curVoice->lastWordL;
				curVoice->prevVol1 = curDoVol1;
				curVoice->LevelDirectionL = FALSE;
				DoIT = TRUE;
			}
		} else if (curVoice->prevPtr == curVoice->begPtr) {
			curVoice->lastWordL = 0;
		}
		
		if (curVoice->lastWordL || curVoice->lastWordR || DoIT) {
			long TICKREMOVESIZE;
			TICKREMOVESIZE = ((intDriver->VSYNC/intDriver->finespeed)*80L)/intDriver->VExt;
			curVoice->TICKREMOVESIZE = TICKREMOVESIZE;
			if (curVoice->LevelDirectionR) {
				curVoice->curLevelR = TICKREMOVESIZE;
			} else {
				curVoice->curLevelR = 0;
			}
			if (curVoice->LevelDirectionL) {
				curVoice->curLevelL = TICKREMOVESIZE;
			} else {
				curVoice->curLevelL = 0;
			}
			curVoice->curLastWordR = curVoice->lastWordR;
			curVoice->curLastWordL = curVoice->lastWordL;
			curVoice->lastWordR = 0;
			curVoice->lastWordL = 0;	
			curVoice->RemoverWorking = TRUE;		/* Call MADTickLoop8() */
		}
		curVoice->prevPtr = curVoice->begPtr;
		curVoice->prevVol0 = curDoVol0;
		curVoice->prevVol1 = curDoVol1;
	}
	
	if (curVoice->RemoverWorking) {		/* Shall I remove ticks? */
		MADTickLoop8(intDriver->ASCBUFFER,curVoice, ASCBuffer1, ASCBuffer2);	/* Perform the removal */
	}
}
/**********************************

	Create a default vibrato table
	
**********************************/

static const short vibrato_table[ 64] = {
	0,50,100,149,196,241,284,325,362,396,426,452,473,490,502,510,512,
	510,502,490,473,452,426,396,362,325,284,241,196,149,100,50,0,-49,
	-99,-148,-195,-240,-283,-324,-361,-395,-425,-451,-472,-489,-501,
	-509,-511,-509,-501,-489,-472,-451,-425,-395,-361,-324,-283,-240,
	-195,-148,-99,-49
};

static INLINECALL void MADCreateVibrato( MADDriverRec *MDriver)
{
	Word i;
	i = 0;
	do {
		MDriver->vibrato_table[i] = vibrato_table[i];
	} while (++i<64);
}

/**********************************

	Dispose of the volume table(s)
	
**********************************/

static INLINECALL void MADDisposeVolumeTable( MADDriverRec *intDriver)
{
	MADKillOverShoot( intDriver);
}

/**********************************

	Create the volume table(s)
	
**********************************/

static INLINECALL int MADCreateVolumeTable( MADDriverRec *intDriver)
{
	/* Create the micro delay */
	intDriver->MDelay = (intDriver->DriverSettings.MicroDelaySize * ( intDriver->DriverSettings.outPutRate >> 16)) / 1000;
	return MADCreateOverShoot( intDriver);	
}

/**********************************

	Change tracks
	
**********************************/

static INLINECALL void MADChangeTracks( MADDriverRec *MDriver,int newVal)
{
	Word8 play = MDriver->MADPlay;
	Word8 reading = MDriver->Reading;
	
	MADStopDriver( MDriver);
	MDriver->DriverSettings.numChn = newVal;
	MADDisposeVolumeTable( MDriver);
	MADCreateVolumeTable( MDriver);
	
	if( play) {
		MADStartDriver( MDriver);
	}
	if (reading) {
		MADPlayMusic( MDriver);
	}
}

/**********************************

	Change tracks if needed
	
**********************************/

static INLINECALL void UpdateTracksNumber( MADDriverRec *MDriver)
{
	if (MDriver->curMusic) {
		MADSpec *HeaderPtr;
		HeaderPtr = MDriver->curMusic->header;
		if (HeaderPtr) {
			if (HeaderPtr->numChn != MDriver->DriverSettings.numChn) {
				MADChangeTracks(MDriver,HeaderPtr->numChn);
			}
		}
	}
}

/**********************************

	Get rid of the reverb tables
	
**********************************/

static INLINECALL void MADDisposeReverb( MADDriverRec *intDriver)
{
	if (intDriver->DriverSettings.Reverb) {
		DeallocAPointer(intDriver->ReverbPtr);
	}
	intDriver->ReverbPtr = 0;
}

/**********************************

	Create the reverb table
	
**********************************/

static int MADCreateReverb( MADDriverRec *intDriver)
{
	if (intDriver->DriverSettings.Reverb) {
		intDriver->RDelay = (intDriver->DriverSettings.ReverbSize * ( intDriver->DriverSettings.outPutRate >> 16L)) / 1000L;
		
		switch (intDriver->DriverSettings.outPutBits) {
		case 8:
			intDriver->ReverbPtr = static_cast<char *>(AllocAPointer(intDriver->RDelay * 2L));
			if( intDriver->ReverbPtr == 0L) {
				return MADNeedMemory;
			}
			FastMemSet(intDriver->ReverbPtr,0x80,intDriver->RDelay*2L);
			break;
			
		case 16:
			intDriver->ReverbPtr = static_cast<char *>(AllocAPointerClear( intDriver->RDelay * 4L));
			if( intDriver->ReverbPtr == 0L) {
				return MADNeedMemory;
			}
			break;
		}
	}
	return 0;
}

/**********************************

	Dispose of the driver's main bufer
	
**********************************/

static INLINECALL void MADDisposeDriverBuffer( MADDriverRec *intDriver)
{
	DeallocAPointer( intDriver->IntDataPtr);
	intDriver->IntDataPtr = 0;
}

/**********************************

	Create the driver's main bufer
	
**********************************/

static INLINECALL int MADCreateDriverBuffer( MADDriverRec *intDriver)
{
	long BufSize;
	
	BufSize = intDriver->ASCBUFFER*2L;
	if (intDriver->DriverSettings.outPutBits==16) {
		BufSize = BufSize*2L;
	}
	intDriver->IntDataPtr = static_cast<char *>(AllocAPointer( BufSize));
	if (intDriver->IntDataPtr) {
		intDriver->BufSize = BufSize;
		return 0;
	}
	return MADNeedMemory;
}

/**********************************

	This code is usually executed during interrupts

**********************************/

static const long mytabx[12] = {
	1712L*16L,1616L*16L,1524L*16L,1440L*16L,1356L*16L,1280L*16L,
	1208L*16L,1140L*16L,1076L*16L,1016L*16L,960L*16L,907L*16L
};

/**********************************

	Handle echo and reverb effects
	
**********************************/

/**********************************

	Allocate and initialize the overshoot tables.
	I assume that the tables are bogus or non-existant
	
**********************************/

int BURGERCALL MADCreateOverShoot(MADDriverRec *intDriver)
{
	switch (intDriver->DriverSettings.outPutBits) {
	case 16:
		{
			long *LongPtr;
			LongPtr = (long*) AllocAPointerClear(((long)intDriver->ASCBUFFER*8L) + intDriver->MDelay*(2L*8L));
			if (LongPtr) {
				intDriver->DASCBuffer = LongPtr;
				return 0;
			}
		}
		break;
		
	case 8:
		{
			short *ShortPtr;
			ShortPtr = (short*) AllocAPointerClear( ( (long) intDriver->ASCBUFFER * 4L) + intDriver->MDelay*(2L*4L));
			if (ShortPtr) {
				char *WorkPtr;

				intDriver->DASCBuffer8 = ShortPtr;
				WorkPtr = (char *) AllocAPointer(256*32);		/* Get the overshoot pointer */
				if (WorkPtr) {			/* Valid pointer? */
					Word i;
					Word8 x;
					intDriver->OverShoot = WorkPtr+((256*16)+128);		/* Save the pointer */
					i = (256*16)/4;			/* Init the base with zeros */
					do {
						((Word32 *)WorkPtr)[0] = 0;	/* Use longword for speed */
						WorkPtr+=4;
					} while (--i);
					i = 256;
					x = 0;
					do {
						WorkPtr[0] = x;		/* Fill with 0-255 */
						++WorkPtr;
						++x;
					} while (--i);
					i = (256*15)/4;
					do {
						((Word32 *)WorkPtr)[0] = 0xFFFFFFFF;	/* Fill the rest with 255 */
						WorkPtr+=4;			/* Use longwords for speed */
					} while (--i);
					return 0;
				}
				DeallocAPointer(ShortPtr);
				intDriver->DASCBuffer8 = 0;
			}
		}
		break;
	}
	return MADNeedMemory;
}

/**********************************

	Dispose of the overshoot tables.
	Burgerlib's memory routines test for NULL pointers
	
**********************************/

void MADKillOverShoot(MADDriverRec *intDriver)
{
	switch( intDriver->DriverSettings.outPutBits) {
	case 16:
		DeallocAPointer(intDriver->DASCBuffer);
		intDriver->DASCBuffer = 0;
		break;

	case 8:
		if (intDriver->OverShoot) {		/* I have to test since the pointer is off by */
			DeallocAPointer(intDriver->OverShoot-((256*16)+128));	/* (256*16)+128 and DeallocAPointer() needs */
			intDriver->OverShoot = 0;		/* The root pointer */
		}			
		DeallocAPointer(intDriver->DASCBuffer8);
		intDriver->DASCBuffer8 = 0;
		break;
	}
}

/**********************************

	Assuming 16 bit samples, add an echo delay
	
**********************************/

static void Sampler16AddDelay( Channel *curVoice, long	*ASCBuffer, MADDriverRec *intDriver)
{
	long chnVol;		/* Right volume */
	long chnVol2;		/* Left volume */
	long *ASCBuffer1;
	long *ASCBuffer2;
	
	chnVol2	= DoVolPanning256(0,curVoice, intDriver);
	chnVol = DoVolPanning256(1,curVoice, intDriver);
	
	{
		long Temp1;
		Temp1 = intDriver->MDelay*2L;
		ASCBuffer1 = ASCBuffer;
		ASCBuffer2 = ASCBuffer + 1;
		if (!(curVoice->ID&1)) {	
			ASCBuffer2 = ASCBuffer2 + Temp1;
		} else {
			ASCBuffer1 = ASCBuffer1 + Temp1;
		}
	}
	if (intDriver->DriverSettings.TickRemover) {
		MADTickRemoverStart8(curVoice,ASCBuffer1,ASCBuffer2,intDriver);
	}
	
	if (curVoice->curPtr < curVoice->maxPtr || curVoice->loopSize) {
		long i;
		char tByte;
		Bool killSample;
		Word32 aDD;			/* Step value */
		long aCC;			/* Delta value */
		long off;
		char *SndBuffer = curVoice->curPtr;
		long preOff = curVoice->preOff;
		char preVal = curVoice->preVal;
		char preVal2 = curVoice->preVal2;

		tByte = 0;
		killSample = FALSE;
		i = intDriver->ASCBUFFER;
		aCC = curVoice->lAC;
		aDD = ((Word32)AMIGA_CLOCKFREQ2 << BYTEDIV) / ( curVoice->period * (intDriver->DriverSettings.outPutRate>>16));

		if (curVoice->pingpong == TRUE && curVoice->loopType == ePingPongLoop) {
			aDD = 0-aDD;	// PINGPONG
		}
		
		if (i) {
			long RightWeight;
			long LeftWeight;
			
			do {
				RightWeight = aCC & ((1 << BYTEDIV) - 1);
				LeftWeight = (1 << BYTEDIV) - RightWeight;
				off = (long) (aCC>>BYTEDIV);
				
				if (preOff != off) {
					if (curVoice->loopType == ePingPongLoop && curVoice->loopSize > 0) {	// PINGPONG
						preOff = off;
						if ((&SndBuffer[off+1] >= curVoice->maxPtr && !curVoice->pingpong) ||
							((&SndBuffer[off+1] <= (curVoice->begPtr + curVoice->loopBeg)) && curVoice->pingpong)) {
							curVoice->pingpong = !curVoice->pingpong;
							aCC = aCC-aDD;
							aDD = 0-aDD;
							RightWeight = aCC & ((1 << BYTEDIV) - 1);
							LeftWeight = (1 << BYTEDIV) - RightWeight;
							off = (long) (aCC>>BYTEDIV);
						}
						preVal = SndBuffer[off];
					} else {
						preVal = preVal2;
						preOff = off;
						if (&SndBuffer[off + 1] >= curVoice->maxPtr) {
							if (curVoice->loopSize > 0) {
								aCC = aCC & ((1 << BYTEDIV) - 1);	
								RightWeight = aCC & ((1 << BYTEDIV) - 1);
								LeftWeight = (1 << BYTEDIV) - RightWeight;
								off = (long) (aCC>>BYTEDIV);
								preOff = off;

								SndBuffer = curVoice->begPtr + curVoice->loopBeg;
								SndBuffer--;
							} else {	// If TICK remove
								MADTickLoopFill8( curVoice, ASCBuffer1, ASCBuffer2, i, (tByte * chnVol), (tByte * chnVol2));
								killSample = TRUE;
								break;
							}
						}
						preVal2 = SndBuffer[off + 1];
					}
				}
				
				tByte = (Word8)(((LeftWeight*preVal) + (RightWeight * SndBuffer[off + 1])) >> BYTEDIV);
				aCC += aDD;
				ASCBuffer1[0] += (tByte * chnVol);
				ASCBuffer1 += 2;
				ASCBuffer2[0] += (tByte * chnVol2);
				ASCBuffer2 += 2;
			} while (--i);
		}
		if (killSample) {
			curVoice->curPtr = curVoice->maxPtr;
		} else {
			if( (aCC>>BYTEDIV) == preOff) {
				curVoice->preOff = 0;
			} else {
				curVoice->preOff = 0xFFFFFFFF;
			}
			curVoice->preVal = preVal;
			curVoice->preVal2 = SndBuffer[off + 1];
			curVoice->curPtr = &SndBuffer[aCC>>BYTEDIV];
		}
		curVoice->lAC = aCC & ((1 << BYTEDIV) - 1);
		curVoice->lastWordL = (tByte * chnVol);
		curVoice->lastWordR = (tByte * chnVol2);
	}
}

/**********************************

	Assuming 16 bit samples, add an echo stereo delay
	
**********************************/

static void Sampler16AddDelayStereo(Channel *curVoice,long *ASCBuffer, MADDriverRec *intDriver)
{
	long chnVol;		/* Right volume */
	long chnVol2;		/* Left volume */
	long *ASCBuffer1;
	long *ASCBuffer2;
	
	chnVol2	= DoVolPanning256(0,curVoice, intDriver);
	chnVol = DoVolPanning256(1,curVoice, intDriver);

	{
		long Temp1;
		Temp1 = intDriver->MDelay*2L;
		ASCBuffer1 = ASCBuffer;
		ASCBuffer2 = ASCBuffer + 1;
		if (!(curVoice->ID&1)) {	
			ASCBuffer2 = ASCBuffer2 + Temp1;
		} else {
			ASCBuffer1 = ASCBuffer1 + Temp1;
		}
	}
	if (intDriver->DriverSettings.TickRemover) {
		MADTickRemoverStart8(curVoice,ASCBuffer1,ASCBuffer2,intDriver);
	}

	if (curVoice->curPtr < curVoice->maxPtr || curVoice->loopSize) {
		long i;
		char tByteR;
		char tByteL;
		Bool killSample;
		long aDD;			/* Step value */
		long aCC;			/* Delta value */
		long off;
		long preOff;
  		char preVal = curVoice->preVal;
  		char preValR = curVoice->preValR;
  		char preVal2 = curVoice->preVal2;
  		char preVal2R = curVoice->preVal2R;
		char * SndBuffer = curVoice->curPtr;

		tByteR = 0;
		tByteL = 0;
		killSample = FALSE;
		i = intDriver->ASCBUFFER;
		aCC = curVoice->lAC;
		aDD = ((Word32)AMIGA_CLOCKFREQ2 << BYTEDIV) / ( curVoice->period * (intDriver->DriverSettings.outPutRate>>16));

		if (curVoice->pingpong == TRUE && curVoice->loopType == ePingPongLoop) {
			aDD = -aDD;	// PINGPONG
		}
		preOff = curVoice->preOff;
			
		if (i) {
			long RightWeight;
			long LeftWeight;
			
			do {
				RightWeight = aCC & ((1 << BYTEDIV) - 1);
				LeftWeight = (1 << BYTEDIV) - RightWeight;
				off = (long) 2*(aCC>>BYTEDIV);
				
				if( preOff != off) {
					if( curVoice->loopType == ePingPongLoop && curVoice->loopSize > 0) {		// PINGPONG
						preOff = off;
						if ((&SndBuffer[off+3] >= curVoice->maxPtr && !curVoice->pingpong) ||
							(&SndBuffer[off+2] <= (curVoice->begPtr + curVoice->loopBeg) && curVoice->pingpong)) {
							curVoice->pingpong = !curVoice->pingpong;
							aCC -= aDD;
							aDD = -aDD;
							RightWeight = aCC & ((1 << BYTEDIV) - 1);
							LeftWeight = (1 << BYTEDIV) - RightWeight;
							off = (long) 2*(aCC>>BYTEDIV);
						}
						preVal = SndBuffer[off];
						preValR = SndBuffer[off+1];
					} else {
						preVal = preVal2;
						preValR = preVal2R;
						preOff = off;
						
						if (&SndBuffer[off + 3] >= curVoice->maxPtr) {
							if( curVoice->loopSize > 0) {
							  aCC = aCC & ((1 << BYTEDIV) - 1);	
							  RightWeight = aCC & ((1 << BYTEDIV) - 1);
							  LeftWeight = (1 << BYTEDIV) - RightWeight;
							  off = (long) 2*(aCC>>BYTEDIV);
							  preOff = off;
							  
							  SndBuffer = (curVoice->begPtr + curVoice->loopBeg) - 2;
							} else {	// If TICK remove
								MADTickLoopFill8( curVoice, ASCBuffer1, ASCBuffer2, i, (tByteL * chnVol), (tByteR * chnVol2));
								killSample = TRUE;
								break;
							}
						}
						preVal2 = SndBuffer[off+2];
						preVal2R = SndBuffer[off+3];
					}
				}
				
				tByteL = (Word8)((LeftWeight * preVal + RightWeight * SndBuffer[off+2])>>BYTEDIV);
				ASCBuffer1[0] += (tByteL * chnVol);
				ASCBuffer1 += 2;
				tByteR = (Word8)((LeftWeight * preValR + RightWeight * SndBuffer[off+3])>>BYTEDIV);
				ASCBuffer2[0] += (tByteR * chnVol2);
				ASCBuffer2 += 2;
				aCC += aDD;
			} while (--i);
		}
		if (killSample) {
			curVoice->curPtr = curVoice->maxPtr;
		} else {
			if (2*(aCC>>BYTEDIV) == preOff) {
				curVoice->preOff = 0;
			} else {
				curVoice->preOff = 0xFFFFFFFF;
			}
			curVoice->preVal = preVal;
			curVoice->preValR = preValR;
			curVoice->preVal2 = SndBuffer[off + 2];
			curVoice->preVal2R = SndBuffer[off + 3];
			curVoice->curPtr = &SndBuffer[2*(aCC>>BYTEDIV)];
		}
		curVoice->lAC = aCC & ((1 << BYTEDIV) - 1);
		curVoice->lastWordL = (tByteL * chnVol);
		curVoice->lastWordR = (tByteR * chnVol2);
	}
}

/**********************************

	Assuming 16 bit samples, add in an echo delay
	
**********************************/

static void Sampler16Addin16Delay( Channel *curVoice, long	*ASCBuffer, MADDriverRec *intDriver)
{
	long chnVol;
	long chnVol2;
	long *ASCBuffer1;
	long *ASCBuffer2;
	long i;
	long tShort;
	Bool killSample;
	long aDD;
	long aCC;
	long off;
	
	chnVol2 = DoVolPanning256( 0, curVoice, intDriver);
	chnVol = DoVolPanning256( 1, curVoice, intDriver);
	
	{
		long Temp1;
		Temp1 = intDriver->MDelay*2L;
		ASCBuffer1 = ASCBuffer;
		ASCBuffer2 = ASCBuffer + 1;
		if (!(curVoice->ID&1)) {	
			ASCBuffer2 = ASCBuffer2 + Temp1;
		} else {
			ASCBuffer1 = ASCBuffer1 + Temp1;
		}
	}
	if (intDriver->DriverSettings.TickRemover) {
		MADTickRemoverStart8(curVoice,ASCBuffer1,ASCBuffer2,intDriver);
	}
	
	if( curVoice->curPtr < curVoice->maxPtr || curVoice->loopSize) {
		short *SndBuffer = (short*)	curVoice->curPtr;
		long RightWeight;
		long LeftWeight;
		long preOff = curVoice->preOff;
		short spreVal = curVoice->spreVal;
		short spreVal2 = curVoice->spreVal2;
		
		aCC = curVoice->lAC;
		aDD = ((Word32)AMIGA_CLOCKFREQ2 << BYTEDIV) / ( curVoice->period * (intDriver->DriverSettings.outPutRate>>16));
		if (curVoice->pingpong == TRUE && curVoice->loopType == ePingPongLoop) {
			aDD = -aDD;	// PINGPONG
		}
		i = intDriver->ASCBUFFER;
		tShort = 0;
		killSample = FALSE;
	
		if (i) {
			do {
				RightWeight = aCC & ((1 << BYTEDIV) - 1);
				LeftWeight = (1 << BYTEDIV) - RightWeight;
				off = (aCC>>BYTEDIV);
				
				if( preOff != off) {
					if( curVoice->loopType == ePingPongLoop && curVoice->loopSize > 0) {	// PINGPONG
						preOff = off;
						if ((&SndBuffer[off+1] >= (short*) curVoice->maxPtr && !curVoice->pingpong) ||
							(&SndBuffer[off+1] <= (short*) (curVoice->begPtr + curVoice->loopBeg) && curVoice->pingpong)) {
							curVoice->pingpong = !curVoice->pingpong;
							aCC -= aDD;
							aDD = -aDD;
							RightWeight = aCC & ((1 << BYTEDIV) - 1);
							LeftWeight = (1 << BYTEDIV) - RightWeight;
							off = (long) (aCC>>BYTEDIV);
						}
						spreVal = SndBuffer[off];
					} else {
						spreVal = spreVal2;
						preOff = off;
						
						if (&SndBuffer[off+1] >= (short*) curVoice->maxPtr) {
							if( curVoice->loopSize > 0) {
							  aCC = aCC & ((1 << BYTEDIV) - 1);	
							  RightWeight = aCC & ((1 << BYTEDIV) - 1);
							  LeftWeight = (1 << BYTEDIV) - RightWeight;
							  off = (long) (aCC>>BYTEDIV);
							  preOff = off;
							  
							  SndBuffer = (short*) (curVoice->begPtr + curVoice->loopBeg);
							  SndBuffer--;
							} else {	// If TICK remove
								MADTickLoopFill8( curVoice, ASCBuffer1, ASCBuffer2, i, (tShort * chnVol) >> 8, (tShort * chnVol2) >> 8);
								killSample = TRUE;
								break;
							}
						}
						spreVal2 = SndBuffer[off+1];
					}
				}
				
				tShort = (LeftWeight*spreVal + RightWeight * SndBuffer[off+1]) >> BYTEDIV;
				aCC += aDD;
				ASCBuffer1[0] += (tShort * chnVol) >> 8;
				ASCBuffer1 += 2;
				ASCBuffer2[0] += (tShort * chnVol2) >> 8;
				ASCBuffer2 += 2;
			} while (--i);
		}
		
		if( killSample) {
			curVoice->curPtr = curVoice->maxPtr;
		} else {
			if( (aCC>>BYTEDIV) == preOff) {
				curVoice->preOff = 0;
			} else {
				curVoice->preOff = 0xFFFFFFFF;	
			}
			curVoice->spreVal = spreVal;
			curVoice->spreVal2 = SndBuffer[off + 1];
			curVoice->curPtr = (char *) (&SndBuffer[aCC>>BYTEDIV]);
		}
		curVoice->lAC = aCC & ((1 << BYTEDIV) - 1);
		curVoice->lastWordL = (tShort * chnVol) >> 8;
		curVoice->lastWordR = (tShort * chnVol2) >> 8;
	}
}

/**********************************

	Assuming 16 bit samples, add in an echo stereo delay
	
**********************************/

static void Sampler16Addin16DelayStereo( Channel *curVoice, long	*ASCBuffer, MADDriverRec *intDriver)
{
	long chnVol;
	long chnVol2;
	long *ASCBuffer1;
	long *ASCBuffer2;
	
	chnVol2 = DoVolPanning256( 0, curVoice, intDriver);
	chnVol = DoVolPanning256( 1, curVoice, intDriver);
	
	{
		long Temp1;
		Temp1 = intDriver->MDelay*2L;
		ASCBuffer1 = ASCBuffer;
		ASCBuffer2 = ASCBuffer + 1;
		if (!(curVoice->ID&1)) {	
			ASCBuffer2 = ASCBuffer2 + Temp1;
		} else {
			ASCBuffer1 = ASCBuffer1 + Temp1;
		}
	}
	if (intDriver->DriverSettings.TickRemover) {
		MADTickRemoverStart8(curVoice,ASCBuffer1,ASCBuffer2,intDriver);
	}
	

	if (curVoice->curPtr < curVoice->maxPtr || curVoice->loopSize) {
		long i = intDriver->ASCBUFFER;
		long off;
		long tShortL = 0;
		long tShortR = 0;
		Bool killSample = FALSE;
		long aDD;
		long aCC = curVoice->lAC;
		short *SndBuffer = (short*)	curVoice->curPtr;
		long RightWeight;
		long LeftWeight;
		long preOff = curVoice->preOff;
 	 	short spreVal = curVoice->spreVal;
 	 	short spreValR = curVoice->spreValR;
 	 	short spreVal2 = curVoice->spreVal2;
 	 	short spreVal2R = curVoice->spreVal2R;
		
		aDD = ((Word32)AMIGA_CLOCKFREQ2 << BYTEDIV) / ( curVoice->period * (intDriver->DriverSettings.outPutRate>>16));
		if( curVoice->pingpong == TRUE && curVoice->loopType == ePingPongLoop) {
			aDD = -aDD;	// PINGPONG
		}
		if (i) {
			do {
				RightWeight = aCC & ((1 << BYTEDIV) - 1);
				LeftWeight = (1 << BYTEDIV) - RightWeight;
				off = 2*(aCC>>BYTEDIV);
				
				if( preOff != off) {
					if( curVoice->loopType == ePingPongLoop && curVoice->loopSize > 0) {		// PINGPONG
						preOff = off;
						if( (&SndBuffer[off+3] >= (short*) curVoice->maxPtr && !curVoice->pingpong) ||
							(&SndBuffer[off+2] <= (short*) (curVoice->begPtr + curVoice->loopBeg) && curVoice->pingpong)) {
							curVoice->pingpong = !curVoice->pingpong;
							aCC -= aDD;
							aDD = -aDD;
							RightWeight = aCC & ((1 << BYTEDIV) - 1);
							LeftWeight = (1 << BYTEDIV) - RightWeight;
							off = (long) 2*(aCC>>BYTEDIV);
						}
						spreVal = SndBuffer[off];
						spreValR = SndBuffer[off+1];
					} else {
						spreVal = spreVal2;
						spreValR = spreVal2R;
						preOff = off;
						
						if( &SndBuffer[off + 3] >= (short*) curVoice->maxPtr) {
							if( curVoice->loopSize > 0) {
								aCC = aCC & ((1 << BYTEDIV) - 1);	
								RightWeight = aCC & ((1 << BYTEDIV) - 1);
								LeftWeight = (1 << BYTEDIV) - RightWeight;
								off = (long) 2*(aCC>>BYTEDIV);
								preOff = off;
							  
								SndBuffer = (short*) (curVoice->begPtr + curVoice->loopBeg);
								SndBuffer-=2;
							} else {	// If TICK remove
								MADTickLoopFill8( curVoice, ASCBuffer1, ASCBuffer2, i, (tShortL * chnVol) >> 8, (tShortR * chnVol2) >> 8);
								killSample = TRUE;
								break;
							}
						}
						spreVal2 = SndBuffer[off + 2];
						spreVal2R = SndBuffer[off + 3];
					}
				}
				
				tShortL = (	LeftWeight * spreVal + RightWeight * SndBuffer[off+2]) >> BYTEDIV;
				ASCBuffer1[0] += (tShortL * chnVol) >> 8;
				ASCBuffer1 += 2;
				tShortR = (	LeftWeight * spreValR + RightWeight * SndBuffer[off+3]) >> BYTEDIV;
				ASCBuffer2[0] += (tShortR * chnVol2) >> 8;
				ASCBuffer2 += 2;
				aCC += aDD;
			} while (--i);
		}
		
		if( killSample) {
			curVoice->curPtr = curVoice->maxPtr;
		} else {
			if( 2*(aCC>>BYTEDIV) == preOff) {
				curVoice->preOff = 0;
			} else {
				curVoice->preOff = 0xFFFFFFFF;
			}
			curVoice->spreVal = spreVal;
			curVoice->spreValR = spreValR;
			curVoice->spreVal2 = SndBuffer[off+2];
			curVoice->spreVal2R = SndBuffer[off+3];
			curVoice->curPtr = (char *) (SndBuffer + 2*(aCC>>BYTEDIV));
		}
		curVoice->lAC = aCC & ((1 << BYTEDIV) - 1);
		aCC -= aDD;
		curVoice->lastWordL = (tShortL * chnVol) >> 8;
		curVoice->lastWordR = (tShortR * chnVol2) >> 8;
	}
}

/**********************************

	Assuming 16 bit samples, add in an echo delay (Master routine)
	
**********************************/

void BURGERCALL MADPlay16StereoDelay( MADDriverRec *intDriver)
{
	long i;

	i = intDriver->DriverSettings.numChn;
	if (i) {
		Channel *curVoice;
		curVoice = intDriver->chan;
		do {
			long *ASCBuffer;
			ASCBuffer = intDriver->DASCBuffer;
			if (curVoice->amp == 16) {
				if (curVoice->stereo) {
					Sampler16Addin16DelayStereo(curVoice, ASCBuffer, intDriver);
				} else {
					Sampler16Addin16Delay(curVoice, ASCBuffer, intDriver);
				}
			} else if (curVoice->amp == 8) {
				if (curVoice->stereo) {
					Sampler16AddDelayStereo(curVoice, ASCBuffer, intDriver);
				} else {
					Sampler16AddDelay(curVoice, ASCBuffer, intDriver);
				}
			}
			++curVoice;
		} while (--i);
	}
	
	{
		long *ttt;
		short *ASCBuffer;
		ttt = intDriver->DASCBuffer;
		ASCBuffer = (short*)intDriver->IntDataPtr;
		
		i = intDriver->ASCBUFFER*2;
		if (i) {
			do {
				long TVal;
				TVal = ttt[0];
				ttt[0] = 0;
				++ttt;
				if (TVal > 0x7FFF) {
					TVal = 0x7FFF;
				} else if (TVal < -0x7FFF) {
					TVal = -0x7FFF;
				}
				ASCBuffer[0] = (short)TVal;
				++ASCBuffer;
			} while (--i);
		}
	}
}

/**********************************

	Assuming 8 bit samples, add an 8 bit echo delay
	
**********************************/

static void Sampler8in8AddDelay( Channel *curVoice, short	*ASCBuffer, MADDriverRec *intDriver)
{
	char tByte;
	long i;
	long chnVol, chnVol2, off;
	short *ASCBuffer1, *ASCBuffer2;
	Bool killSample;
	long aDD;
	long aCC;
	long RightWeight;
	long LeftWeight;
	char *SndBuffer;
	long preOff;
	char preVal;
	char preVal2;
	
	if( curVoice->curPtr < curVoice->maxPtr || curVoice->loopSize) {
		chnVol2	= DoVolPanning256( 0, curVoice, intDriver);
		chnVol	= DoVolPanning256( 1, curVoice, intDriver);

		i = intDriver->ASCBUFFER;
		killSample = FALSE;
		aCC = curVoice->lAC;
		aDD = ((Word32)AMIGA_CLOCKFREQ2 << BYTEDIV) / ( curVoice->period * (intDriver->DriverSettings.outPutRate>>16));
		if (curVoice->pingpong == TRUE && curVoice->loopType == ePingPongLoop) {
			aDD = -aDD;	// PINGPONG
		}
		
		{
			long Temp1;
			Temp1 = intDriver->MDelay*2L;
			ASCBuffer1 = ASCBuffer;
			ASCBuffer2 = ASCBuffer + 1;
			if (!(curVoice->ID&1)) {	
				ASCBuffer2 = ASCBuffer2 + Temp1;
			} else {
				ASCBuffer1 = ASCBuffer1 + Temp1;
			}
		}
		
		SndBuffer = curVoice->curPtr;
		preOff = curVoice->preOff;
	  	preVal = curVoice->preVal;
	  	preVal2 = curVoice->preVal2;
		if (i) {	
			do {
				RightWeight = aCC & ((1 << BYTEDIV) - 1);
				LeftWeight = (1 << BYTEDIV) - RightWeight;
				off = aCC>>BYTEDIV;
				
				if( preOff != off) {
					if( curVoice->loopType == ePingPongLoop && curVoice->loopSize > 0) {	// PINGPONG
						preOff = off;
						if ((&SndBuffer[off+1] >= curVoice->maxPtr && !curVoice->pingpong) ||
							(&SndBuffer[off+1] <= curVoice->begPtr + curVoice->loopBeg && curVoice->pingpong)) {
							curVoice->pingpong = !curVoice->pingpong;
							aCC -= aDD;
							aDD = -aDD;
							RightWeight = aCC & ((1 << BYTEDIV) - 1);
							LeftWeight = (1 << BYTEDIV) - RightWeight;
							off = (long) (aCC>>BYTEDIV);
						}
						preVal = SndBuffer[off];
					} else {
						preVal = preVal2;
						preOff = off;
						
						if (&SndBuffer[off + 1] >= curVoice->maxPtr) {
							if( curVoice->loopSize > 0) {
								aCC = aCC & ((1 << BYTEDIV) - 1);	
								RightWeight = aCC & ((1 << BYTEDIV) - 1);
								LeftWeight = (1 << BYTEDIV) - RightWeight;
								off = (long) (aCC>>BYTEDIV);
								preOff = off;
							  
								SndBuffer = (curVoice->begPtr + curVoice->loopBeg)-1;
							} else {	// If TICK remove
								killSample = TRUE;
								break;
							}
						}
						preVal2 = SndBuffer[off+1];
					}
				}
				
				tByte = (Word8)((LeftWeight * preVal + RightWeight * SndBuffer[off + 1]) >> BYTEDIV);
				aCC += aDD;
				
				ASCBuffer1[0] += (short)((chnVol * tByte) >> 8);
				ASCBuffer1 += 2;
				ASCBuffer2[0] += (short)((chnVol2 * tByte) >> 8);
				ASCBuffer2 += 2;
			} while (--i);
		}
			
			
		if( killSample) {
			curVoice->curPtr = curVoice->maxPtr;
		} else {
			if( (aCC>>BYTEDIV) == preOff) {
				curVoice->preOff = 0;
			} else {
				curVoice->preOff = 0xFFFFFFFF;	//(long) (aCC>>BYTEDIV);
			}
			curVoice->preVal = preVal;
			curVoice->preVal2 = SndBuffer[off + 1];
			curVoice->curPtr = &SndBuffer[aCC>>BYTEDIV];
		}
		curVoice->lAC = aCC & ((1 << BYTEDIV) - 1);
	}
}

/**********************************

	Assuming 8 bit samples, add a 16 bit echo delay
	
**********************************/

static void Sampler8in16AddDelay( Channel *curVoice, short	*ASCBuffer, MADDriverRec *intDriver)
{
	char tByte;
	long chnVol, chnVol2;
	long i;
	long off;
	short *ASCBuffer1, *ASCBuffer2;
	Bool killSample;
	long aDD;
	long aCC;
	char *SndBuffer;
	long RightWeight;
	long LeftWeight;
	long preOff;
	char preVal;
	char preVal2;
	
	if (curVoice->curPtr < curVoice->maxPtr || curVoice->loopSize) {
		chnVol2	= DoVolPanning256(0,curVoice,intDriver);
		chnVol = DoVolPanning256(1,curVoice,intDriver);
		
		{
			long Temp1;
			Temp1 = intDriver->MDelay*2L;
			ASCBuffer1 = ASCBuffer;
			ASCBuffer2 = ASCBuffer + 1;
			if (!(curVoice->ID&1)) {	
				ASCBuffer2 = ASCBuffer2 + Temp1;
			} else {
				ASCBuffer1 = ASCBuffer1 + Temp1;
			}
		}

		aCC = curVoice->lAC;
		aDD = ((Word32)AMIGA_CLOCKFREQ2 << BYTEDIV) / ( curVoice->period * (intDriver->DriverSettings.outPutRate>>16));
		if (curVoice->pingpong == TRUE && curVoice->loopType == ePingPongLoop) {
			aDD = -aDD;	// PINGPONG
		}
		i = intDriver->ASCBUFFER;
		killSample = FALSE;
		SndBuffer = curVoice->curPtr;
		preOff = curVoice->preOff;
		preVal = curVoice->preVal;
		preVal2 = curVoice->preVal2;
		
		#ifdef __LITTLEENDIAN__
		++SndBuffer;				/* Hack for getting the high byte */
		#endif
		if (i) {
			do {
				RightWeight = aCC & ((1 << BYTEDIV) - 1);
				LeftWeight = (1 << BYTEDIV) - RightWeight;
				off = aCC>>BYTEDIV;
				if (preOff != off) {
					if (curVoice->loopType == ePingPongLoop && curVoice->loopSize > 0) {		// PINGPONG
						preOff = off;
						if ((&SndBuffer[2*off +2] >= curVoice->maxPtr && !curVoice->pingpong) ||
							(&SndBuffer[2*off +2] <= (curVoice->begPtr + curVoice->loopBeg) && curVoice->pingpong)) {
							curVoice->pingpong = !curVoice->pingpong;
							aCC -= aDD;
							aDD = -aDD;
							RightWeight = aCC & ((1 << BYTEDIV) - 1);
							LeftWeight = (1 << BYTEDIV) - RightWeight;
							off = (long) (aCC>>BYTEDIV);
						}
						preVal = SndBuffer[2*off];
					} else {
						preVal = preVal2;
						preOff = off;
						if( &SndBuffer[2*off +2] >= curVoice->maxPtr) {
							if( curVoice->loopSize > 0) {
								aCC = aCC & ((1 << BYTEDIV) - 1);	
								RightWeight = aCC & ((1 << BYTEDIV) - 1);
								LeftWeight = (1 << BYTEDIV) - RightWeight;
								off = (long) (aCC>>BYTEDIV);
								preOff = off;
							  
								SndBuffer = (curVoice->begPtr + curVoice->loopBeg)-2;
								#ifdef __LITTLEENDIAN__
								++SndBuffer;				/* Hack for getting the high byte */
								#endif
							} else {		// If TICK remove
								killSample = TRUE;
								break;
							}
						}
						preVal2 = SndBuffer[2*off+2];
					}
				}
				
				tByte = (Word8)((LeftWeight * preVal + RightWeight * SndBuffer[2*off+2]) >> BYTEDIV);
				aCC += aDD;
				
				ASCBuffer1[0] += (short)((chnVol * tByte) >> 8);
				ASCBuffer1 += 2;
				ASCBuffer2[0] += (short)((chnVol2 * tByte) >> 8);
				ASCBuffer2 += 2;
			} while (--i);
		}
		if (killSample) {
			curVoice->curPtr = curVoice->maxPtr;
		} else {
			if( (aCC>>BYTEDIV) == preOff) {
				curVoice->preOff = 0;
			} else {
				curVoice->preOff = 0xFFFFFFFF;	
			}
			curVoice->preVal = preVal;
			curVoice->preVal2 = SndBuffer[2*off + 2];
			#ifdef __LITTLEENDIAN__
			--SndBuffer;				/* Hack for getting the high byte */
			#endif
			curVoice->curPtr = (char *) (&SndBuffer[2*(aCC>>BYTEDIV)]);
		}
		curVoice->lAC = aCC & ((1 << BYTEDIV) - 1);
	}
}

/**********************************

	Assuming 8 bit samples, add a 8 bit echo stereo delay
	
**********************************/

static void Sampler8in8AddDelayStereo(Channel *curVoice,short *ASCBuffer,MADDriverRec *intDriver)
{
	char tByteL, tByteR;
	long i;
	long chnVol, chnVol2;
	long off;
	short *ASCBuffer1, *ASCBuffer2;
	Bool killSample;
	long aDD;
	long aCC;
	char *SndBuffer;
	long RightWeight;
	long LeftWeight;
	long preOff;
  	char preVal;
  	char preValR;
  	char preVal2;
  	char preVal2R;
	
	if( curVoice->curPtr < curVoice->maxPtr || curVoice->loopSize) {
		chnVol2	= DoVolPanning256(0, curVoice, intDriver);
		chnVol	= DoVolPanning256(1, curVoice, intDriver);

		{
			long Temp1;
			Temp1 = intDriver->MDelay*2L;
			ASCBuffer1 = ASCBuffer;
			ASCBuffer2 = ASCBuffer + 1;
			if (!(curVoice->ID&1)) {	
				ASCBuffer2 = ASCBuffer2 + Temp1;
			} else {
				ASCBuffer1 = ASCBuffer1 + Temp1;
			}
		}

		aCC = curVoice->lAC;
		aDD = ((Word32)AMIGA_CLOCKFREQ2 << BYTEDIV) / ( curVoice->period * (intDriver->DriverSettings.outPutRate>>16));
		if( curVoice->pingpong == TRUE && curVoice->loopType == ePingPongLoop) {
			aDD = -aDD;	// PINGPONG
		}
		killSample = FALSE;
		i = intDriver->ASCBUFFER;
				
		SndBuffer = curVoice->curPtr;
		preOff = curVoice->preOff;
		preVal = curVoice->preVal;
		preValR = curVoice->preValR;
		preVal2 = curVoice->preVal2;
		preVal2R = curVoice->preVal2R;

		if (i) {			
			do {
				RightWeight = aCC & ((1 << BYTEDIV) - 1);
				LeftWeight = (1 << BYTEDIV) - RightWeight;
				off = 2*(aCC>>BYTEDIV);
				
				if( preOff != off) {
					if (curVoice->loopType == ePingPongLoop && curVoice->loopSize > 0) {		// PINGPONG
						preOff = off;
						if ((&SndBuffer[off+3] >= curVoice->maxPtr && !curVoice->pingpong) ||
							(&SndBuffer[off+2] <= curVoice->begPtr + curVoice->loopBeg && curVoice->pingpong)) {
							curVoice->pingpong = !curVoice->pingpong;
							aCC -= aDD;
							aDD = -aDD;
							RightWeight = aCC & ((1 << BYTEDIV) - 1);
							LeftWeight = (1 << BYTEDIV) - RightWeight;
							off = (long) 2*(aCC>>BYTEDIV);
						}
						preVal = SndBuffer[off];
						preValR = SndBuffer[off+1];
					} else {
						preVal = preVal2;
						preValR = preVal2R;
						preOff = off;
						
						if( &SndBuffer[off+3] >= curVoice->maxPtr) {
							if( curVoice->loopSize > 0) {
								aCC = aCC & ((1 << BYTEDIV) - 1);	
								RightWeight = aCC & ((1 << BYTEDIV) - 1);
								LeftWeight = (1 << BYTEDIV) - RightWeight;
								off = (long) 2*(aCC>>BYTEDIV);
								preOff = off;
							  
								SndBuffer = (curVoice->begPtr + curVoice->loopBeg)-2;
							} else {	// If TICK remove
								killSample = TRUE;
								break;
							}
						}
						preVal2 = SndBuffer[off+2];
						preVal2R = SndBuffer[off+3];
					}
				}
				
				tByteL = (Word8)(( LeftWeight *	preVal + RightWeight * SndBuffer[off+2]) >> BYTEDIV);
				ASCBuffer1[0] += (short)((chnVol * tByteL) >> 8);
				ASCBuffer1 += 2;
				tByteR = (Word8)(( LeftWeight *	preValR + RightWeight * SndBuffer[off+3]) >> BYTEDIV);
				ASCBuffer2[0] += (short)((chnVol2 * tByteR) >> 8);
				ASCBuffer2 += 2;
				
				aCC += aDD;
			} while (--i);
		}			
		if( killSample) {
			curVoice->curPtr = curVoice->maxPtr;
		} else {
			if( 2*(aCC>>BYTEDIV) == preOff) {
				curVoice->preOff = 0;
			} else {
				curVoice->preOff = 0xFFFFFFFF;
			}
			curVoice->preVal = preVal;
			curVoice->preValR = preValR;
			curVoice->preVal2 = SndBuffer[off+2];
			curVoice->preVal2R = SndBuffer[off+3];
			curVoice->curPtr = &SndBuffer[2*(aCC>>BYTEDIV)];
		}
		curVoice->lAC = aCC & ((1 << BYTEDIV) - 1);
	}
}

/**********************************

	Assuming 8 bit samples, add a 16 bit echo stereo delay
	
**********************************/

static void Sampler8in16AddDelayStereo( Channel *curVoice,short *ASCBuffer, MADDriverRec *intDriver)
{
	char tByteL, tByteR;
	long chnVol, chnVol2;
	long i;
	long off;
	short *ASCBuffer1, *ASCBuffer2;
	Bool killSample;
	long aDD, aCC;
	long RightWeight;
	long LeftWeight;
	char *SndBuffer;
	long preOff;
	char preVal;
	char preValR;
	char preVal2;
	char preVal2R;
	
	if( curVoice->curPtr < curVoice->maxPtr || curVoice->loopSize) {		
		chnVol2	= DoVolPanning256( 0, curVoice, intDriver);
		chnVol	= DoVolPanning256( 1, curVoice, intDriver);
		
		{
			long Temp1;
			Temp1 = intDriver->MDelay*2L;
			ASCBuffer1 = ASCBuffer;
			ASCBuffer2 = ASCBuffer + 1;
			if (!(curVoice->ID&1)) {	
				ASCBuffer2 = ASCBuffer2 + Temp1;
			} else {
				ASCBuffer1 = ASCBuffer1 + Temp1;
			}
		}
		
		aDD = ((Word32)AMIGA_CLOCKFREQ2 << BYTEDIV) / ( curVoice->period * (intDriver->DriverSettings.outPutRate>>16));
		if( curVoice->pingpong == TRUE && curVoice->loopType == ePingPongLoop) {
			aDD = -aDD;	// PINGPONG
		}
		tByteL = 0;
		tByteR = 0;
		i = intDriver->ASCBUFFER;
		killSample = FALSE;
		aCC = curVoice->lAC;

		SndBuffer = curVoice->curPtr;
		preOff = curVoice->preOff;
		preVal = curVoice->preVal;
		preValR = curVoice->preValR;
		preVal2 = curVoice->preVal2;
		preVal2R = curVoice->preVal2R;
			
		#ifdef __LITTLEENDIAN__
		++SndBuffer;
		#endif
		if (i) {	
			do {
				RightWeight = aCC & ((1 << BYTEDIV) - 1);
				LeftWeight = (1 << BYTEDIV) - RightWeight;
				off = 2*(aCC>>BYTEDIV);
				
				if( preOff != off) {
					if( curVoice->loopType == ePingPongLoop && curVoice->loopSize > 0) {		// PINGPONG
						preOff = off;
						if( (&SndBuffer[2*off+6] >= curVoice->maxPtr && !curVoice->pingpong) ||
							(&SndBuffer[2*off+4] <= curVoice->begPtr + curVoice->loopBeg && curVoice->pingpong)) {
							curVoice->pingpong = !curVoice->pingpong;
							aCC -= aDD;
							aDD = -aDD;
							RightWeight = aCC & ((1 << BYTEDIV) - 1);
							LeftWeight = (1 << BYTEDIV) - RightWeight;
							off = (long) 2*(aCC>>BYTEDIV);
						}
						preVal = SndBuffer[2*off];
						preValR = SndBuffer[2*off+2];
					} else {
						preVal = preVal2;
						preValR = preVal2R;
						preOff = off;
						
						if (&SndBuffer[2*off + 6] >= curVoice->maxPtr) {
							if (curVoice->loopSize > 0)  {
								aCC = aCC & ((1 << BYTEDIV) - 1);	
								RightWeight = aCC & ((1 << BYTEDIV) - 1);
								LeftWeight = (1 << BYTEDIV) - RightWeight;
								off = (long) 2*(aCC>>BYTEDIV);
								preOff = off;
								SndBuffer = (curVoice->begPtr + curVoice->loopBeg)-4;
								#ifdef __LITTLEENDIAN__
								++SndBuffer;
								#endif
							} else {	// If TICK remove
								killSample = TRUE;
								break;
							}
						}
						preVal2 = SndBuffer[2*off+4];
						preVal2R = SndBuffer[2*off+6];
					}
				}
				
				tByteL = (Word8)((LeftWeight * preVal + RightWeight * SndBuffer[2*off+4]) >> BYTEDIV);
				*ASCBuffer1 += (short)((chnVol * tByteL) >> 8);
				ASCBuffer1 += 2;
				tByteR = (Word8)((LeftWeight * preValR + RightWeight * SndBuffer[2*off+6]) >> BYTEDIV);
				*ASCBuffer2 += (short)((chnVol2 * tByteR) >> 8);
				ASCBuffer2 += 2;
				aCC += aDD;
			} while (--i);
		}			
		if( killSample) {
			curVoice->curPtr	= curVoice->maxPtr;
		} else {
			if( 2*(aCC>>BYTEDIV) == preOff) {
				curVoice->preOff = 0;
			} else {
				curVoice->preOff = 0xFFFFFFFF;
			}
			curVoice->preVal = preVal;
			curVoice->preValR = preValR;
			curVoice->preVal2 = SndBuffer[2*off + 4];
			curVoice->preVal2R = SndBuffer[2*off + 6];
			#ifdef __LITTLEENDIAN__
			--SndBuffer;
			#endif
			curVoice->curPtr = &SndBuffer[4*(aCC>>BYTEDIV)];
		}
		curVoice->lAC = aCC & ((1 << BYTEDIV) - 1);
	}
}

/**********************************

	Assuming 8 bit samples, add in an echo delay (Master routine)
	
**********************************/

void BURGERCALL MADPlay8StereoDelay( MADDriverRec *intDriver)
{
	long i;

	i = intDriver->DriverSettings.numChn;
	if (i) {
		Channel *curVoice;
		curVoice = intDriver->chan;
		do {
			short *WorkPtr;
			WorkPtr = intDriver->DASCBuffer8;
			if (curVoice->stereo) {
				if (curVoice->amp == 16) {
					Sampler8in16AddDelayStereo( curVoice,WorkPtr,intDriver);
				} else {
					Sampler8in8AddDelayStereo( curVoice,WorkPtr,intDriver);
				}
			} else {
				if( curVoice->amp == 16) {
					Sampler8in16AddDelay( curVoice,WorkPtr,intDriver);
				} else {
					Sampler8in8AddDelay( curVoice,WorkPtr,intDriver);
				}
			}
			++curVoice;
		} while (--i);
	}
	{
		short *ttt;
		char *ASCBuffer;
		ttt = intDriver->DASCBuffer8;
		ASCBuffer = intDriver->IntDataPtr;
		i = intDriver->ASCBUFFER*2;
		if (i) {
			do {
				ASCBuffer[0] = intDriver->OverShoot[ttt[0]];
				ttt[0] = 0;
				++ASCBuffer;
				++ttt;
			} while (--i);
		}
	}
}
/**********************************

	Simple audio effect token parser
	
**********************************/

#define LOW(para) ((para) & 15)
#define HI(para) ((para) >> 4)

/**********************************

	Get the volume slide token
	
**********************************/

static INLINECALL void MADParseSlideVol(Channel *ch,Word Arg)
{
	int Temp;
	Temp = Arg&0x0F;		/* Get the low 4 bits */
	if (Temp) {
		ch->volumerate = -Temp;		/* Negate the volume value */
	} else {
		ch->volumerate = (Arg>>4);	/* Use the upper 4 bits */
	}
}

/**********************************

	Execute a music command
	
**********************************/

void BURGERCALL MADDoEffect(Channel *ch,int call, MADDriverRec *intDriver)
{
	long offset;
	switch( ch->cmd) {
	default:				/* I surrender */
		ch->cmd = 0;
		ch->arg = 0;
		return;

	case arpeggioE:				// OK
		if (ch->arg && ch->arp[0]) {
			offset = ch->arpindex;
			if (++offset >= MAX_ARP) {		/* Adjust the index */
				offset = 0;					/* Back to the beginning */
			}
			ch->arpindex = offset;
			ch->period = ch->arp[offset];
		}
		break;

	case skipE:					// OK
		if (call == (intDriver->speed - 1)) {
			Word Temp2;
			MADSpec *HeaderPtr;

			if (intDriver->JumpToNextPattern) {
				HeaderPtr = intDriver->curMusic->header;
				offset = intDriver->PL;		/* Get the partition index */
				if (intDriver->PartitionReader) {
					++offset;
					intDriver->PL = (short)offset;
					intDriver->Pat = HeaderPtr->oPointers[offset];	/* Next pattern */
				}

				Temp2 = ch->arg;
				intDriver->PartitionReader = static_cast<short>(((Temp2>>4) * 10) + (Temp2&0x0f));		/* Convert from BCD */

				if (offset >= HeaderPtr->numPointers) {
					intDriver->PL = 0;
					intDriver->Pat = HeaderPtr->oPointers[0];
					MADCleanDriver(intDriver);
					if (!intDriver->DriverSettings.repeatMusic) {
						intDriver->Reading = FALSE;
					}
				}
			} else {
				intDriver->PartitionReader = 0;
			}
			ch->cmd = 0;
			ch->arg = 0;
		}
		break;

	case fastskipE:						// OK
		if (call == (intDriver->speed - 1)) {

			if (intDriver->JumpToNextPattern) {
				MADSpec *HeaderPtr;
				if (intDriver->PL > ch->arg) {		// Evite les boucles
					if (!intDriver->DriverSettings.repeatMusic) {
						intDriver->Reading = FALSE;
					}
				}

				HeaderPtr = intDriver->curMusic->header;
				offset = ch->arg;
				intDriver->PL = (short)offset;
				intDriver->Pat = HeaderPtr->oPointers[offset];

				if (offset >= HeaderPtr->numPointers) {
					intDriver->PL = 0;
					intDriver->Pat = HeaderPtr->oPointers[0];

					MADCleanDriver( intDriver);
					if( !intDriver->DriverSettings.repeatMusic) {
						intDriver->Reading = FALSE;
					}
				}
			}
			intDriver->PartitionReader = 0;
			ch->cmd = 0;
			ch->arg = 0;
		}
		break;

	case downslideE:						// OK
		if( ch->period > intDriver->MIN_PITCH) {
			ch->period -= ch->slide*4;
		}
		break;

	case upslideE:							// OK
		if( ch->period < intDriver->MAX_PITCH) {
			ch->period += ch->slide*4;
		}
		break;

	case vibratoE:
		offset = (ch->viboffset+ch->vibrate)&0x3F;
		ch->viboffset = offset;

		offset = ((long) intDriver->vibrato_table[offset] * (long) ch->vibdepth) / 512L;

		ch->period = ch->periodOld + offset*4;
		break;

	case slidevolE:						// OK
		offset = ch->vol + ch->volumerate;
		if( offset < MIN_VOLUME) {
			offset = MIN_VOLUME;
		} else if( offset > MAX_VOLUME) {
			offset = MAX_VOLUME;
		}
		ch->vol = offset;
		break;

	case portamentoE:
		{
			long goal;
			goal = ch->pitchgoal;
			offset = ch->period;
			if (offset != goal) {
				if (offset < goal) {
					offset += ch->pitchrate*4;
					if (offset > goal) {
						ch->cmd = 0;
						ch->arg = 0;
						offset = goal;
					}
					ch->period = offset;
				} else if (offset > goal) {
					offset -= ch->pitchrate*4;
					if( offset < goal) {
						ch->cmd = 0;
						ch->arg = 0;
						offset = goal;
					}
					ch->period = offset;
				}
			}
		}
		break;

	case portaslideE:
		ch->cmd = portamentoE;			/* Fake commands */
		MADDoEffect(ch,call,intDriver);
		ch->cmd = slidevolE;
		MADDoEffect(ch,call,intDriver);
		ch->cmd = portaslideE;			/* Restore the token */
		break;

	case vibratoslideE:
		ch->cmd = vibratoE;				/* Fake commands */
		MADDoEffect( ch, call, intDriver);
		ch->cmd = slidevolE;
		MADDoEffect( ch, call, intDriver);
		ch->cmd = vibratoslideE;		/* Restore the token */
		break;

	case extendedE:
		if ((ch->arg&0xF0)==(12<<4)) {
			if (call>= (ch->arg&0x0F)) {
				ch->vol = 0;
			}
		}
		break;
	}

	if (call == intDriver->speed - 1) {		/* At the end? */
		ch->arg = 0;			/* Zap the command */
		ch->cmd = 0;
	}
}

/**********************************

	Init a command byte
	
**********************************/

void BURGERCALL MADSetUpEffect( Channel *ch, MADDriverRec *intDriver)
{
	int cmd,arg;
	int temp;

	arg = ch->arg;
	cmd = ch->cmd;		/* Get the command token */
	if (!arg) {		/* Any argument? */
		switch(cmd) {
		default:
			arg = ch->oldArg[cmd];
			ch->arg = arg;
		case arpeggioE:
		case nothingE:
		case fastskipE:
		case volumeE:
		case panningE:
		case skipE:
		case extendedE:
		case speedE:
			break;
		}
	} else {
		ch->oldArg[cmd] = arg;		/* Save the previous argument */
	}
	
	switch (cmd) {
	case upslideE:							// OK
	case downslideE:						// OK
		if (arg) {
			ch->slide = arg;
		}
		break;

	case vibratoE:							// OK
		temp = arg>>4;
		if (temp) {
			ch->vibrate = temp;
		}
		temp = arg&0x0F;
		if (temp) {
			ch->vibdepth = temp;
		}
		ch->periodOld = ch->period;
		break;

	case arpeggioE:						// OK
		if (!arg) {
			ch->arp[ 0] = 0;
		} else {
			if (ch->note != 0xFF) {
				int note;
				note = ch->note + (arg>>4);
				if (note < NUMBER_NOTES) {
					ch->arp[1] = GetOldPeriod( note, NOFINETUNE);
				}
				note = ch->note + (arg&0x0F);
				if (note < NUMBER_NOTES) {
					ch->arp[2] = GetOldPeriod( note, NOFINETUNE);
				}
				ch->arpindex = 0;
				ch->arp[0] = ch->period;
			}
		}
		break;

	case slidevolE:						// OK
		MADParseSlideVol(ch,arg);
		break;

	case extendedE:
		switch((arg>>4)) {
//		case 0:		// Turn On/Off filter
//			break;

		case 1:		// Fineslide up
			temp = arg&0x0F;
			ch->period -= temp*4;
			break;

		case 2:		// Fineslide down
			temp = arg&0x0F;
			ch->period += temp*4;
//			break;

		case 3:		// Set glissando on/off
//			break;

		case 4:		// Set vibrato waveform
//			break;

		case 5:		// Set finetune value
//			ch->fineTune	= finetune[arg&0x0F];
//			ch->period	= GetOldPeriod(ch->Amiga, ch->fineTune);
//			break;

		case 6:		// Loop pattern
//			break;

		case 7:		// Set tremolo waveform
//			break;

		case 8:		// Unused
//			break;

		case 9:
			break;

		case 10:	// Fine volume slide up
			temp = ch->vol;
			temp += (arg&0x0F);

			if( temp < MIN_VOLUME) {
				temp = MIN_VOLUME;
			} else if( temp > MAX_VOLUME) {
				temp = MAX_VOLUME;
			}
			ch->vol = temp;
			break;

		case 11:	// Fine volume slide down
			temp = ch->vol;
			temp -= (arg&0x0F);

			if (temp < MIN_VOLUME) {
				temp = MIN_VOLUME;
			} else if (temp > MAX_VOLUME) {
				temp = MAX_VOLUME;
			}
			ch->vol = temp;
			break;

//		case 12:	// Cut sample
//			break;

//		case 13:	// Delay sample
//			break;

//		case 14:	// Delay pattern
//			break;

//		case 15:	// Invert loop
//			break;
		}
		break;

	case portamentoE:				// OK
		ch->pitchrate = arg;

		if (ch->note != 0xFF) {
			ch->pitchgoal = (short)GetOldPeriod( ch->note, ch->fineTune);
		} else if (!arg) {
			ch->pitchgoal = ch->period;
		}
		break;

	case portaslideE:				// OK
		if (ch->note != 0xFF) {
			ch->pitchgoal = (short)GetOldPeriod( ch->note, ch->fineTune);
		} else if( ch->pitchgoal == 0) {
			ch->pitchgoal = ch->period;
		}
		MADParseSlideVol(ch,arg);
		break;

	case vibratoslideE:
		ch->periodOld = ch->period;
		MADParseSlideVol(ch,arg);
		break;

	case speedE:
		if (arg < 32) {		/** Setting de la speed + reset de la finespeed **/
			if (arg) {
				intDriver->speed = arg;
			}
		} else {		/** Setting de finespeed **/
			intDriver->finespeed = arg;
		}
//		break;

	case skipE:
//		break;

	case fastskipE:
		break;

	case offsetE:
		ch->curPtr = ch->begPtr+(arg*256);
		break;

	case panningE:
		arg = ( (long) arg * (long)  MAX_PANNING) / (long) 0xFF;

		if (arg < 0) {
			arg = 0;
		} else if (arg > MAX_PANNING) {
			arg = MAX_PANNING;
		}
		ch->pann = arg;
		break;

	case volumeE:
		if (arg < MIN_VOLUME) {
			arg = MIN_VOLUME;
		} else if( arg > MAX_VOLUME) {
			arg = MAX_VOLUME;
		}
		ch->vol = arg;
		break;
	}
}

/**********************************

	Execute a volume command
	
**********************************/

void BURGERCALL MADDoVolCmd( Channel *ch,int call)
{
	int vol;
	int volLO;
	
	vol = ch->volcmd;
	volLO = vol & 0xf;
	switch (vol>>4) {
	case 0x6:					// volslide down
		vol = ch->vol-volLO;
			
		if (vol < MIN_VOLUME) {
			vol = MIN_VOLUME;
		} else if (vol > MAX_VOLUME) {
			vol = MAX_VOLUME;
		}
		ch->vol = vol;
		break;

	case 0x7:					// volslide up
		vol = ch->vol + volLO;
		
		if(vol < MIN_VOLUME) {
			vol = MIN_VOLUME;
		} else if (vol > MAX_VOLUME) {
			vol = MAX_VOLUME;
		}
		ch->vol = vol;
		break;

	// volume-row fine volume slide is compatible with protracker
	//   EBx and EAx effects i.e. a zero nibble means DO NOT SLIDE, as
	//  opposed to 'take the last sliding value'.
	//

	case 0x8:						// finevol down
		if (call == 1) {
			vol = ch->vol - volLO;
	
			if (vol < MIN_VOLUME) {
				vol = MIN_VOLUME;
			} else if (vol > MAX_VOLUME) {
				vol = MAX_VOLUME;
			}
			ch->vol = vol;
		}
		break;

	case 0x9:                       // finevol up
		if (call == 1) {
			vol = ch->vol + volLO;
	
			if (vol < MIN_VOLUME) {
				vol = MIN_VOLUME;
			} else if (vol > MAX_VOLUME) {
				vol = MAX_VOLUME;
			}
			ch->vol = vol;
		}
		break;

//	case 0xa:                       // set vibrato speed
//		break;

//	case 0xb:                       // vibrato
//		break;

//	case 0xc:                       // set panning
//		break;

//	case 0xd:                       // panning slide left
//		// only slide when data nibble not zero:
//		break;

//	case 0xe:                       // panning slide right
//		// only slide when data nibble not zero:
//		break;

//	case 0xf:                       // tone porta
//		break;
	}
}

/**********************************

	Return the volume for the channel for a voice
	(Take into account the pan value)
	Value returned is 0-256

**********************************/

long BURGERCALL DoVolPanning256(Word whichChannel, Channel *ch, MADDriverRec *intDriver)
{
	// Compute Volume !
	Word32 pannValue;
	Word32 temp;

	if (intDriver->Active[ch->ID]) {

		if (intDriver->curMusic) {
			temp = ( (Word32) ch->vol * (Word32) ch->volEnv * (Word32) ch->volFade) / (16L*32767L);
			temp = (temp * (Word32) intDriver->curMusic->header->chanVol[ ch->ID]) / MAX_VOLUME;
		} else {
			temp = 256;
		}

		// Compute Panning

		if( whichChannel != 3) {
			pannValue = ch->pannEnv;
			if( whichChannel == 1) {
				pannValue = MAX_PANNING - pannValue;
			}
			temp = (temp * pannValue) / MAX_PANNING;
		}

		// Vol Global

		temp = (temp * intDriver->VolGlobal) / (MAX_VOLUME + 20);

		if( temp > 256) {
			temp = 256;
		}
		return temp;
	}
	return 0;		/* Ah fudge!! */
}

/**********************************

	Reset a driver

**********************************/

void BURGERCALL MADCleanDriver( MADDriverRec *intDriver)
{
	Word i, x;
	Channel *curVoice;
	i = 0;
	curVoice = intDriver->chan;
	do {
		curVoice->ID = i;

		curVoice->begPtr = 0L;
		curVoice->maxPtr = 0L;
		curVoice->curPtr = 0L;
		curVoice->sizePtr = 0L;

		curVoice->amp = 8;

		curVoice->loopBeg = 0;
		curVoice->loopSize = 0;

		curVoice->ins = 0;
		curVoice->insOld = 0;

		curVoice->fineTune = NOFINETUNE;

		curVoice->note = 0xFF;
		curVoice->noteOld = 0xFF;


		curVoice->period = GetOldPeriod( 40, NOFINETUNE);
		curVoice->periodOld= GetOldPeriod( 40, NOFINETUNE);

		curVoice->vol = 64;
		curVoice->cmd = 0;
		curVoice->arg = 0;

		x = 0;
		do {
			curVoice->arp[ x] = 0;
		} while (++x<MAX_ARP);
		curVoice->arpindex = 0;

		curVoice->viboffset	= 0;
		curVoice->vibdepth = 0;
		curVoice->vibrate = 0;
		curVoice->slide = 0;
		curVoice->pitchgoal = 0;
		curVoice->pitchrate = 0;
		curVoice->volumerate = 0;
		curVoice->volcmd = 0L;

		x = 0;
		do {
			curVoice->oldArg[x] = 0;
		} while (++x<16);

		curVoice->KeyOn = FALSE;
		curVoice->a = 0;
		curVoice->b = 1;
		curVoice->p = 0;
		curVoice->volEnv = 64;
		curVoice->volFade = 32767;
		curVoice->lAC = 0;

		curVoice->lastWordR = 0;
		curVoice->lastWordL = 0;
		curVoice->curLevelL = 0;
		curVoice->curLevelR = 0;
		curVoice->prevPtr = 0;
		curVoice->prevVol0 = 1;
		curVoice->prevVol1 = 1;

		curVoice->loopType = eClassicLoop;
		curVoice->pingpong = FALSE;

		curVoice->preOff = 0xFFFFFFFF;
		curVoice->preVal2 = 0;
		curVoice->preVal2R = 0;

		curVoice->spreVal2 = 0;
		curVoice->spreVal2R	= 0;

		curVoice->preVal = 0;
		curVoice->spreVal = 0;

		curVoice->RemoverWorking = FALSE;
		curVoice->TICKREMOVESIZE = 1;
		++curVoice;
	} while (++i<MAXTRACK);
	intDriver->BufCounter = 0;
	intDriver->BytesToGenerate = 0;
}

/**********************************

	Interpolate the current volume from the position
	in the envelope

**********************************/

static INLINECALL long InterpolateEnv(long p,EnvRec *a,EnvRec *b)
{
	long p1,p2,v1;

	p1 = a->pos;
	v1 = a->val;
	if (p >= p1) {
		p2 = b->pos;
		if( p1 != p2) {
			return v1 + (((p-p1)*((long)b->val-v1)) / (p2-p1));
		}
	}
	return v1;
}

/**********************************

	Handle fadeout

**********************************/

static INLINECALL void ProcessFadeOut( Channel *ch, MADDriverRec *intDriver)
{
	if (intDriver->curMusic) {
		if (!ch->KeyOn) {
			long volTemp;
			volTemp = ch->volFade - intDriver->curMusic->fid[ch->ins].volFade;
			if(volTemp < 0) {
				volTemp = 0;
				ch->loopBeg = volTemp;
				ch->loopSize = volTemp;
			}
			ch->volFade = volTemp;
		}
	}
}

/**********************************

	Handle envelopes

**********************************/

static void ProcessEnvelope( Channel *ch, MADDriverRec *intDriver)
{
	InstrData *curIns;

	ch->volEnv = 64;
	if (ch->ins >= 0) {
		curIns = &intDriver->curMusic->fid[ ch->ins];
		if (curIns->volSize > 0) {

			if( curIns->volType & EFON) {
				//  active? -> copy variables

				Word a,b;
				int p;

				a=ch->a;

				if( curIns->volSize == 1) {		// Just 1 point !
					ch->volEnv = curIns->volEnv[a].val;
					ch->p = curIns->volEnv[a].pos;
				} else {
					b=ch->b;
					p=ch->p;
					ch->volEnv = InterpolateEnv(p,&curIns->volEnv[a], &curIns->volEnv[b]);

					if (!(curIns->volType & EFSUSTAIN) || !ch->KeyOn || a!=curIns->volSus || p!=curIns->volEnv[a].pos) {
						if (++p >= curIns->volEnv[b].pos) {
							a=b;
							++b;
							if (curIns->volType & EFLOOP) {
								if (b > curIns->volEnd) {
									a=curIns->volBeg;
									b=a+1;
									p=curIns->volEnv[a].pos;
								}
							} else {
								if (b >= curIns->volSize) {
									--b;
									--p;
								}
							}
						}
						ch->a=static_cast<short>(a);
						ch->b=static_cast<short>(b);
						ch->p=p;
					}
				}
			}
		}
	}
}

/**********************************

	Handle panning

**********************************/

static void ProcessPanning( Channel *ch, MADDriverRec *intDriver)
{
	InstrData *curIns;

	ch->pannEnv = ch->pann;
	if (ch->ins >= 0) {
		curIns = &intDriver->curMusic->fid[ch->ins];
		if (curIns->pannSize > 0) {

			if (curIns->pannType & EFON) {
				//  active? -> copy variables

				Word aa,bb;
				int pp;

				aa = (Word)ch->aa;

				if (curIns->pannSize == 1) {		// Just 1 point !
					ch->pannEnv = curIns->pannEnv[aa].val;
					ch->pp = curIns->pannEnv[aa].pos;
				} else {
					bb = (Word)ch->bb;
					pp = (Word)ch->pp;
					ch->pannEnv = InterpolateEnv(pp,&curIns->pannEnv[aa],&curIns->pannEnv[bb]);
					if (++pp >= curIns->pannEnv[bb].pos) {
						aa=bb;
						++bb;
						if (curIns->pannType & EFLOOP) {
							if (bb > curIns->pannEnd) {
								aa=curIns->pannBeg;
								bb=aa+1;
								pp=curIns->pannEnv[aa].pos;
							}
						} else {
							if (bb >= curIns->pannSize) {
								--bb;
								--pp;
							}
						}
						ch->aa=(short)aa;
						ch->bb=(short)bb;
					}
					ch->pp=(short)pp;
				}
			}
		}
	}
}

/**********************************

	Return the frequency to play a note

**********************************/

long BURGERCALL GetOldPeriod(int note, long c2spd)
{
	Word32 period, n,o;

	if (note == 0xFF || note == 0xFE || !c2spd) {
		return 4242;			/* Standard delay */
	}

	if (note < 0) {
		note = 0;
	}
	o = note/12;		/* Octave */
	n = note-(o*12);	/* Quick modulo for Note */

	period = (Word32) ((Word32) ( 8363UL * mytabx[n]) >> o ) / (Word32) c2spd;

	if (!period) {		/* Oops? */
		period = 7242;	/* Return default */
	}
	return period;
}

/**********************************

	Process a music note

**********************************/

static void ReadNote( Channel *curVoice,const Cmd_t *theNoteCmd, MADDriverRec *intDriver)
{
	Word8 ins;
	Word8 note;					// Note, see table		0xFF : no note cmd
	Word8 cmd;					// Effect cmd
	Word8 arg;					// Effect argument
	Word8 vol;					// Volume				0xFF : no volume cmd

	ins = theNoteCmd->ins;
	note = theNoteCmd->note;
	cmd = theNoteCmd->cmd;
	arg = theNoteCmd->arg;
	vol = theNoteCmd->vol;

/********************************************/
/*        EXTRA small positionning          */
/********************************************/

	if (cmd == 0x0E && (arg >> 4) == 0x0D) {
		if( intDriver->smallcounter == 0 && !curVoice->GEffect) {
			curVoice->GEffect = TRUE;
			curVoice->GPat = intDriver->Pat;
			curVoice->GReader = intDriver->PartitionReader;
		}

		if (intDriver->smallcounter < (arg & 0x0F)) {
			return;			/* Do it later */
		}
	}
	curVoice->GEffect = FALSE;		// <- Continue - Play note NOW !

/********************************************/
/*        Read command and compute it       */
/********************************************/
	if (ins || (note != 0xFF && note != 0xFE)) {
	/********************************/
	/* PrŽpare les notes manquantes */
	/********************************/

		if (!ins) {
			ins = (Word8)curVoice->insOld;
		} else {
			curVoice->insOld = ins;
		}

	/********************************/

		if (ins && (note != 0xFF && note != 0xFE)) {
			sData_t *curData;
			short shins, samp;

		/**** INSTRUMENT ****/

			shins = ins - 1;
			if (shins >= MAXINSTRU) {
				shins = MAXINSTRU-1;
			}
			samp = intDriver->curMusic->fid[shins].what[note];

			if( samp < intDriver->curMusic->fid[shins].numSamples) {
				curData = intDriver->curMusic->sample[ intDriver->curMusic->fid[shins].firstSample + samp];
				curVoice->ins = shins;
				curVoice->amp = curData->amp;
				curVoice->stereo = curData->stereo;
				curVoice->samp = samp;
				curVoice->loopType= curData->loopType;

			/**** RESET NOTE ****/
				if(cmd != portamentoE && cmd != portaslideE) {
					curVoice->prevPtr = 0L;
					curVoice->maxPtr = curVoice->curPtr = curVoice->begPtr = curData->data;
					curVoice->maxPtr += curData->size;
					curVoice->sizePtr = curData->size;
					curVoice->lAC = 0;
					curVoice->pingpong = FALSE;
					curVoice->preOff = 0xFFFFFFFF;
					curVoice->preVal = 0;
					curVoice->spreVal = 0;
					curVoice->preVal2 = curVoice->curPtr[0];
					if( curVoice->amp == 8) {
						curVoice->preVal2R = curVoice->curPtr[1];
					} else {
						curVoice->preVal2R = curVoice->curPtr[2];
					}
					curVoice->spreVal2 = ((short*)curVoice->curPtr)[0];
					curVoice->spreVal2R	= ((short*)curVoice->curPtr)[1];

					if( curData->loopSize > 2) {
						curVoice->loopBeg = curData->loopBeg;
						curVoice->loopSize = curData->loopSize;
						curVoice->maxPtr = (char *) ((long) curData->data + curData->loopBeg + curData->loopSize);
					} else {
						curVoice->loopBeg = 0;
						curVoice->loopSize = 0;
					}
					curVoice->viboffset = 0;

					if (cmd != volumeE) {
						curVoice->vol = curData->vol;
						if( curVoice->vol > MAX_VOLUME) {
							curVoice->vol = MAX_VOLUME;
						}
						curVoice->volFade = 32767;
					}

					if(cmd != panningE) {
						curVoice->pann = intDriver->curMusic->header->chanPan[ curVoice->ID];
						if( curVoice->pann > MAX_PANNING) {
							curVoice->pann = MAX_PANNING;
						}
					}
					curVoice->pp=0;		/* Start panning */
					curVoice->aa=0;
					curVoice->bb=1;

					curVoice->p=0;		/* Start envelope */
					curVoice->a=0;
					curVoice->b=1;
				}
			}
		} else if (ins && note == 0xFF) {
			if( curVoice->samp < intDriver->curMusic->fid[ curVoice->ins].numSamples) {
				sData_t	*curData;

				curData	= intDriver->curMusic->sample[ intDriver->curMusic->fid[ curVoice->ins].firstSample + curVoice->samp];

				if( curData != 0L) {
					if( cmd != volumeE) {	// intCmd.cmd != slidevolE &&
						curVoice->vol 			= curData->vol;
						if( curVoice->vol > MAX_VOLUME) {
							curVoice->vol = MAX_VOLUME;
						}
						curVoice->volFade = 32767;
					}

					if( cmd != panningE) {
						curVoice->pann = intDriver->curMusic->header->chanPan[ curVoice->ID];
						if( curVoice->pann > MAX_PANNING) {
							curVoice->pann = MAX_PANNING;
						}
					}
				}
			}
		}

		if( note != 0xFF && note != 0xFE) {
		/**** NOTE & PERIOD ****/
			MADMusic *CurMusicPtr;
			sData_t	*curData;
			short	samp;

			CurMusicPtr = intDriver->curMusic;
			samp = CurMusicPtr->fid[ curVoice->ins].what[ note];
			if( samp < CurMusicPtr->fid[ curVoice->ins].numSamples) {
				curData = CurMusicPtr->sample[CurMusicPtr->fid[ curVoice->ins].firstSample + samp];

				curVoice->note = note + curData->relNote;
				curVoice->fineTune = curData->c2spd;
				curVoice->KeyOn = TRUE;

				if( cmd != portamentoE && cmd != portaslideE) {
					curVoice->period = GetOldPeriod( curVoice->note, curVoice->fineTune);
					curVoice->periodOld = curVoice->period = (curVoice->period * (long) intDriver->FreqExt) / 80L;
				}
			}
		}
	} else {
		curVoice->note = 0xFF;
	}

/**************/
/*   VOLUME   */
/**************/
	if( vol != 0xFF) {
		if(vol >= 0x10 && vol <= 0x50) {
			vol = vol - 0x10;			/* 0-0x40 */
			curVoice->vol = vol;
			curVoice->volcmd = 0;
		} else {
			curVoice->volcmd = vol;
		}
	} else {
		curVoice->volcmd = 0;
	}
	curVoice->cmd = cmd;
	curVoice->arg = arg;

	MADSetUpEffect( curVoice, intDriver);

/**************/
/*   KEY OFF  */
/**************/
	if( note == 0xFE) {
		curVoice->KeyOn	= FALSE;
	}
}

/**********************************

	Add in an 8 bit reverb

**********************************/

static INLINECALL void ComputeReverb8(Word8 *orgPtr,Word8 *destPtr,long xx,long strength)
{
	long temp1;

	if (xx) {
		strength = (strength*128)/100L;
		do {
			temp1 = destPtr[0] + ((strength * (orgPtr[0] - 0x80L)) >>7);

			if( temp1 > 0xFF) {
				temp1 = 0xFF;		// overflow ?
			} else if (temp1 < 0) {
				temp1 = 0;
			}
			destPtr[0] = (Word8)temp1;
			++orgPtr;
			++destPtr;
		} while (--xx);
	}
}

/**********************************

	Add in a 16 bit reverb

**********************************/

static INLINECALL void ComputeReverb16(short *orgPtr,short *destPtr,long xx,long strength)
{
	long temp1;

	if (xx) {
		strength = (strength*128)/100L;
		do {
			temp1 = destPtr[0] + ((strength * (long)orgPtr[0]) >>7);

			if (temp1 > 0x7FFFL) {
				temp1 = 0x7FFFL;	// overflow ?
			} else if (temp1 < -0x7FFFL ) {
				temp1 = -0x7FFFL;
			}
			destPtr[0] = (short)temp1;
			++orgPtr;
			++destPtr;
		} while (--xx);
	}
}

/**********************************

	Add in the surround sound effect

**********************************/

static INLINECALL void ApplySurround( MADDriverRec *intDriver)
{
	long i;
	i = intDriver->ASCBUFFER;
	if (i) {
		switch( intDriver->DriverSettings.outPutBits) {
		case 8:
			{
				char *data = (char*) intDriver->IntDataPtr;
				do {
					data[0] = -1-data[0];
					data += 2;
				} while (--i);
			}
			break;

		case 16:
			{
				short *data = (short*) intDriver->IntDataPtr;
				do {
					data[0] = -1-data[0];
					data += 2;
				} while (--i);
			}
			break;
		}
	}
}

/**********************************

	Mix sound channels to the output stream

**********************************/

static INLINECALL void GenerateSound(MADDriverRec *intDriver)
{
	switch(intDriver->DriverSettings.outPutBits) {
	case 8:
		MADPlay8StereoDelay(intDriver);
		intDriver->IntDataPtr += intDriver->ASCBUFFER*2L;
		intDriver->DASCBuffer8 += intDriver->ASCBUFFER*2L;
		break;
	case 16:
		MADPlay16StereoDelay( intDriver);
		intDriver->IntDataPtr += intDriver->ASCBUFFER*4L;
		intDriver->DASCBuffer += intDriver->ASCBUFFER*2L;
		break;
	}
}

/**********************************

	Process all the pending notes

**********************************/

void BURGERCALL NoteAnalyse( MADDriverRec *intDriver)
{
	long InterruptBufferSize, i,j;
	long tVSYNC;

	/* If the music is being loaded or edited, don't play anything */
	if (intDriver->curMusic) {
		if (intDriver->curMusic->musicUnderModification) {	// SILENCE
			switch( intDriver->DriverSettings.outPutBits) {
			case 8:
				FastMemSet(intDriver->IntDataPtr,0x80,intDriver->ASCBUFFER);
				break;
			case 16:
				FastMemSet(intDriver->IntDataPtr,0,intDriver->ASCBUFFER*2);
				break;
			}
			return;
		}
	}

	InterruptBufferSize = intDriver->ASCBUFFER;
	if (InterruptBufferSize>0) {
		long *DASCopy;
		short *DASCopy8;
		long ASCBUFFERCopy;
		char * DataPtrCopy;

		DataPtrCopy	= intDriver->IntDataPtr;
		DASCopy	= intDriver->DASCBuffer;
		DASCopy8 = intDriver->DASCBuffer8;
		ASCBUFFERCopy = intDriver->ASCBUFFER;
		do {
			long Temp;
			Word NoteReading;
			
			/********************/
			/* Sound Generating */
			/********************/
			
			Temp = intDriver->BytesToGenerate - intDriver->BufCounter;
			
			if(Temp < 0) {
				Temp = 0;
			}
			if( Temp > InterruptBufferSize) {
				Temp = InterruptBufferSize;
				NoteReading = FALSE;
			} else {
				NoteReading = TRUE;
			}
			intDriver->ASCBUFFER = Temp;
			if (Temp > 0) {
				GenerateSound(intDriver);
				Temp = intDriver->ASCBUFFER;
				intDriver->BufCounter += Temp;
				InterruptBufferSize -= Temp;
			}
			
			/**************************/
			/* Note & Effect Analyser */
			/**************************/
			
			if (!NoteReading) {
				break;		/* Force exit */
			}
				
			if (intDriver->curMusic && intDriver->Reading) {
				j = intDriver->curMusic->header->numChn;
				if (j) {
					Channel *curVoice;
					curVoice = intDriver->chan;
					i = 0;
					do {
						if (curVoice->GEffect) {
							ReadNote(curVoice,GetMADCommand(curVoice->GReader,i,intDriver->curMusic->partition[curVoice->GPat]),intDriver);
						}
						++curVoice;
					} while (++i<j);
				}
			}
			
			/*********/
			
			if( ++intDriver->smallcounter >= intDriver->speed) {
				MADMusic *CurMusicPtr;
				intDriver->smallcounter = 0;			/* Reset the counter */
				CurMusicPtr = intDriver->curMusic;
				if (CurMusicPtr) {
					j = CurMusicPtr->header->numChn;
					if (j) {
						Channel *curVoice;
						curVoice = intDriver->chan;
						i = 0;
						do {
							if (intDriver->Reading) {
								ReadNote(curVoice,GetMADCommand(intDriver->PartitionReader, i, CurMusicPtr->partition[ intDriver->Pat]), intDriver);
							}
						
							ProcessEnvelope(curVoice, intDriver);
							ProcessPanning(curVoice, intDriver);
							ProcessFadeOut(curVoice, intDriver);
							++curVoice;
						} while (++i<j);
					}
					
					if (intDriver->Reading) {
						if (++intDriver->PartitionReader >= CurMusicPtr->partition[intDriver->Pat]->header.size) {
							intDriver->PartitionReader = 0;
							
							if (intDriver->JumpToNextPattern) {
								int PL;
								MADSpec *HeaderPtr;
								
								PL = (int)intDriver->PL;
								HeaderPtr = CurMusicPtr->header;
								++PL;
								intDriver->PL = (short)PL;
								intDriver->Pat = HeaderPtr->oPointers[PL];
								
								if (intDriver->speed == 1 && PL >= HeaderPtr->numPointers) {
									intDriver->PL = 0;
									intDriver->Pat = HeaderPtr->oPointers[0];
									
									MADCleanDriver(intDriver);
									if (!intDriver->DriverSettings.repeatMusic) {
										intDriver->Reading = FALSE;
									}
								}
							}
						}
					}
				}
			} else {
				MADMusic *CurMusicPtr;
				CurMusicPtr = intDriver->curMusic;
				if (CurMusicPtr) {
					i = intDriver->DriverSettings.numChn;
					if (i) {
						Channel *curVoice;
						curVoice = intDriver->chan;
						do {
							MADDoVolCmd(curVoice, intDriver->smallcounter);
							MADDoEffect(curVoice, intDriver->smallcounter, intDriver);
							ProcessEnvelope(curVoice, intDriver);
							ProcessPanning(curVoice, intDriver);
							ProcessFadeOut(curVoice, intDriver);
							++curVoice;
						} while (--i);
					}
					
					if (intDriver->Reading) {
						if (intDriver->smallcounter == intDriver->speed - 1) {
							if( intDriver->PL >= intDriver->curMusic->header->numPointers) {
								intDriver->PL = 0;
								intDriver->Pat = intDriver->curMusic->header->oPointers[0];
								
								MADCleanDriver(intDriver);
								if (!intDriver->DriverSettings.repeatMusic) {
									intDriver->Reading = FALSE;
								}
							}
						}
					}
				}
			}
			
			tVSYNC = ((intDriver->VSYNC/intDriver->finespeed)*80L)/intDriver->VExt;
			intDriver->BytesToGenerate += tVSYNC;
		} while (InterruptBufferSize > 0);
		intDriver->ASCBUFFER = ASCBUFFERCopy;
		intDriver->IntDataPtr = DataPtrCopy;
		intDriver->DASCBuffer = DASCopy;
		intDriver->DASCBuffer8 = DASCopy8;
	}
	
	
	if( intDriver->DriverSettings.MicroDelaySize) {
		switch( intDriver->DriverSettings.outPutBits) {
		case 16:
			FastMemCpy(intDriver->DASCBuffer, intDriver->DASCBuffer + intDriver->ASCBUFFER*2, intDriver->MDelay*8L);
			FastMemSet((intDriver->DASCBuffer + intDriver->MDelay*2L),0, intDriver->ASCBUFFER*8);
			break;
			
		case 8:
			if (intDriver->MDelay &1) {
				FastMemCpy(intDriver->DASCBuffer8, intDriver->DASCBuffer8 + intDriver->ASCBUFFER*2,  1 + intDriver->MDelay*4L);
				FastMemSet((intDriver->DASCBuffer8 + intDriver->MDelay*2L),0, (1 + intDriver->ASCBUFFER)*4);
			} else {
				FastMemCpy(intDriver->DASCBuffer8, intDriver->DASCBuffer8 + intDriver->ASCBUFFER*2,  intDriver->MDelay*4L);
				FastMemSet((intDriver->DASCBuffer8 + intDriver->MDelay*2L),0,intDriver->ASCBUFFER*4);
			}
			break;
		}
	}
	
	if( intDriver->DriverSettings.surround) {
		ApplySurround( intDriver);
	}
	if( intDriver->DriverSettings.Reverb && intDriver->ASCBUFFER < intDriver->RDelay) {
		switch( intDriver->DriverSettings.outPutBits) {
		case 8:
			ComputeReverb8( (Word8*) intDriver->ReverbPtr, (Word8*) intDriver->IntDataPtr, intDriver->ASCBUFFER*2L, intDriver->DriverSettings.ReverbStrength);
			FastMemCpy(intDriver->ReverbPtr,intDriver->ReverbPtr + intDriver->ASCBUFFER*2L, intDriver->RDelay*2L - intDriver->ASCBUFFER*2L);
			FastMemCpy(intDriver->ReverbPtr + intDriver->RDelay*2L - intDriver->ASCBUFFER*2L, intDriver->IntDataPtr, intDriver->ASCBUFFER*2L);
			break;
				
		case 16:
			ComputeReverb16( (short*) intDriver->ReverbPtr, (short*) intDriver->IntDataPtr, intDriver->ASCBUFFER*2L, intDriver->DriverSettings.ReverbStrength);
			FastMemCpy(intDriver->ReverbPtr,intDriver->ReverbPtr + intDriver->ASCBUFFER*4,  (intDriver->RDelay - intDriver->ASCBUFFER)*4);
			FastMemCpy(intDriver->ReverbPtr + intDriver->RDelay*4 - intDriver->ASCBUFFER*4,intDriver->IntDataPtr,  intDriver->ASCBUFFER*4);
			break;
		}
	}
}

/**********************************

	Create the driver record
	
**********************************/

int BURGERCALL MADCreateDriver( MADDriverSettings	*DriverInitParam, MADLibrary_t *lib, MADDriverRec** returnDriver)
{
	int theErr;
	long i;
	MADDriverRec* MDriver;
	
	*returnDriver = 0L;
		
	/*************************/
	/** Paramaters checking **/
	/*************************/
	
	theErr = 0;
	
	DriverInitParam->numChn &= (~1);
	if ((DriverInitParam->numChn < 2) ||
		(DriverInitParam->numChn > MAXTRACK) ||
		(DriverInitParam->outPutBits != 8 && DriverInitParam->outPutBits != 16) ||
		(DriverInitParam->outPutRate < rate5khz) ||
		(DriverInitParam->outPutRate > rate48khz) ||
		(DriverInitParam->MicroDelaySize < 0) ||
		(DriverInitParam->MicroDelaySize > 1000)) {
		theErr = MADParametersErr;
	}
	if (DriverInitParam->driverMode != SoundManagerDriver &&
		DriverInitParam->driverMode != BeOSSoundDriver &&
		DriverInitParam->driverMode != DirectSound95NT &&
		DriverInitParam->driverMode != NoHardwareDriver) {
		theErr = MADParametersErr;
	}
	if (DriverInitParam->Reverb) {
		if ((DriverInitParam->ReverbSize < 25) ||
			(DriverInitParam->ReverbSize > 1000) ||
			(DriverInitParam->ReverbStrength < 0) ||
			(DriverInitParam->ReverbStrength > 70)) {
			theErr = MADParametersErr;
		}
	}
	
	if (theErr) {
		return theErr;
	}
	
	MDriver = (MADDriverRec*) AllocAPointerClear( sizeof( MADDriverRec));
	if (!MDriver) {
		return MADNeedMemory;
	}
	MDriver->lib = lib;
	MDriver->curMusic = 0L;
	MDriver->Reading = FALSE;
	
	MADStopDriver(MDriver);
	MADCreateVibrato(MDriver);
	
	i = 0;
	do {
		MDriver->Active[i] = TRUE;
	} while (++i<MAXTRACK);
	MDriver->DriverSettings	= *DriverInitParam;
	MDriver->Reading = FALSE;
	MDriver->JumpToNextPattern = TRUE;
	MDriver->smallcounter = 128;			// Start immediately
	MDriver->BufCounter	= 0;
	MDriver->BytesToGenerate = 0;
	MDriver->speed = 6;
	MDriver->finespeed = 125;
	MDriver->VExt = 80;
	MDriver->FreqExt = 80;
	MDriver->VolGlobal = 64;
	
	MDriver->MIN_PITCH = GetOldPeriod( NUMBER_NOTES-1, NOFINETUNE);
	MDriver->MAX_PITCH = GetOldPeriod( 0, NOFINETUNE);
	
	MADCleanDriver(MDriver);
	
	/*************************/
	/** 	Driver MODE	    **/
	/*************************/
	
	switch( MDriver->DriverSettings.driverMode) {		
	case SoundManagerDriver:
		MDriver->ASCBUFFER = (370 * (MDriver->DriverSettings.outPutRate>>16L)) / 22254L;
		MDriver->ASCBUFFER &= (~1);
		if (MDriver->ASCBUFFER < 370) {
			MDriver->ASCBUFFER = 370;
		}
		break;
		
	case NoHardwareDriver:
		MDriver->ASCBUFFER = 1024;
		break;
		
	case BeOSSoundDriver:
		MDriver->ASCBUFFER = 1024L;
		if( MDriver->DriverSettings.outPutBits == 8) {
			MDriver->ASCBUFFER *= 2L;
		}
		break;
		
	case DirectSound95NT:
		MDriver->ASCBUFFER = 7500L;
		break;
				
	default:
		return MADParametersErr;
	}
	
	
	theErr = MADCreateDriverBuffer( MDriver);
	if (theErr) {
		return theErr;
	}
	
	/* Create the timing */
	
	MDriver->VSYNC = ((MDriver->DriverSettings.outPutRate>>16) * 125L) / (50L);

	theErr = MADCreateReverb( MDriver);
	if( theErr) {
		return theErr;
	}
	theErr = MADCreateVolumeTable(	MDriver);
	if (theErr) {
		return theErr;
	}
	
	/**********************************************/
	/**    Interruption - Hardware Support       **/
	/**********************************************/

	theErr = MADSndOpen(MDriver);
	if( theErr) {
		return theErr;
	}
	
	*returnDriver = MDriver;
	
	return 0;
}

/**********************************

	Dispose of the driver record
	
**********************************/

void BURGERCALL MADDisposeDriver( MADDriverRec* MDriver)
{
	if (MDriver) {
		if (MDriver->IntDataPtr) {
			MDriver->Reading = FALSE;
			MADCleanDriver( MDriver);
			MADSndClose( MDriver);
		}
		MADDisposeDriverBuffer( MDriver);
		MADDisposeVolumeTable( MDriver);	
		MADDisposeReverb( MDriver);
		DeallocAPointer( (char *) MDriver);
	}
}

/**********************************

	Attach hardware to a song
	
**********************************/

int BURGERCALL MADAttachDriverToMusic( MADDriverRec *driver, MADMusic *music)
{
	if ((!driver) ||
		(!music)) {
		return -1;
	}
	driver->curMusic = music;
	driver->VolGlobal = music->header->generalVol;
	if( driver->VolGlobal <= 0) {
		driver->VolGlobal = 64;
	}
	driver->VExt = music->header->generalSpeed;
	if( driver->VExt <= 0) {
		driver->VExt = 80;
	}
	driver->FreqExt = music->header->generalPitch;
	if( driver->FreqExt <= 0) {
		driver->FreqExt = 80;
	}
	MADReset( driver);
	UpdateTracksNumber( driver);
	return 0;
}

/**********************************

	Call the proper importer
	
**********************************/

int BURGERCALL MADLoadMusicPtr( MADLibrary_t *lib, MADMusic **Output,const Word8 * myPtr,Word32 Length)
{
	Word i,j;
	int Code;

	Output[0] = 0;			/* Assume nothing returned */
	j = lib->TotalPlug;
	if (j) {
		MADMusic *Input;
		Input = (MADMusic*) AllocAPointerClear(sizeof(MADMusic));		/* Get the record */
		if (Input) {
			i = 0;
			do {
				Code = lib->ThePlug[i](myPtr,Length,Input);	/* Try this importer */
				if (!Code) {
					Output[0] = Input;		/* I got it!! */
					return 0;
				}
				if (Code!=MADFileNotSupportedByThisPlug) {		/* Not compatible? */
					return Code;			/* I accept it, but I have an error */
				}
			} while (++i<j);				/* Try another plug */
		}
	}
	return MADCannotFindPlug;				/* I give up */
}

/**********************************

	Dispose of a track

**********************************/

void MADPurgeTrack( MADDriverRec *intDriver)
{
	int i;
	Channel *curVoice;
	
	i = intDriver->DriverSettings.numChn;
	if (i) {
		curVoice = intDriver->chan;
		do {
			curVoice->prevPtr = 0;
		
			curVoice->curPtr = curVoice->maxPtr;
			curVoice->lAC = 0;
			curVoice->loopBeg = 0;
			curVoice->loopSize = 0;
			curVoice->RemoverWorking = FALSE;
			curVoice->TICKREMOVESIZE = 1;
		} while (--i);
	}
}

/**********************************

	Reset an instrument to defaults

**********************************/

void BURGERCALL MADResetInstrument( InstrData	*curIns)
{
	int i;

	i = 0;
	do {
		curIns->name[ i]	= 0;
	} while (++i<32);
	curIns->type		= 0;
	curIns->numSamples	= 0;
	
	/**/
	
	i = 0;
	do {
		curIns->what[i] = 0;
	} while (++i<96);
	i = 0;
	do {
		curIns->volEnv[i].pos = 0;
		curIns->volEnv[i].val = 0;
		curIns->pannEnv[i].pos = 0;
		curIns->pannEnv[i].val = 0;
	} while (++i<12);
	
	curIns->volSize		= 0;
	curIns->pannSize	= 0;
	
	curIns->volSus		= 0;
	curIns->volBeg		= 0;
	curIns->volEnd		= 0;
	
	curIns->pannSus		= 0;
	curIns->pannBeg		= 0;
	curIns->pannEnd		= 0;
	
	curIns->volType		= 0;
	curIns->pannType	= 0;
	
	curIns->volFade		= DEFAULT_VOLFADE;
	curIns->vibDepth	= 0;
	curIns->vibRate		= 0;
}

/**********************************

	Dispose of an instrument

**********************************/

void BURGERCALL MADKillInstrument( MADMusic *music,Word ins)
{
	int i;
	InstrData		*curIns;
	Bool			IsReading;

	if(music) {
		curIns = &music->fid[ ins];
		
		IsReading = music->musicUnderModification;
		music->musicUnderModification = TRUE;
		
		for( i = 0; i < curIns->numSamples; i++) {
			if( music->sample[ ins * MAXSAMPLE + i] != 0L) {
				if (music->sample[ ins * MAXSAMPLE + i]->data != 0L) {
					DeallocAPointer(music->sample[ ins * MAXSAMPLE + i]->data);
					music->sample[ ins * MAXSAMPLE + i]->data = 0L;
				}
				DeallocAPointer(music->sample[ ins * MAXSAMPLE + i]);
				music->sample[ ins * MAXSAMPLE + i] = 0L;
			}
		}
		MADResetInstrument( curIns);
		music->musicUnderModification = IsReading;
	}
}

/**********************************

	Set a song's default speed

**********************************/

void BURGERCALL MADCheckSpeed( MADMusic *MDriver, MADDriverRec *intDriver)
{
	int i, x, y;
	int curPartitionReader;
	Cmd_t *aCmd;
	Bool CmdSpeed = FALSE, FineFound = FALSE;

	if (!MDriver) return;
	if (MDriver->header == 0L) return;
	if (!intDriver) return;
	
	for( i = intDriver->PL; i >= 0; i--) {
		if( i == intDriver->PL) {
			curPartitionReader = intDriver->PartitionReader;
			if( curPartitionReader >= MDriver->partition[ MDriver->header->oPointers[ i]]->header.size) {
				curPartitionReader--;
			}
		} else {
			curPartitionReader = MDriver->partition[ MDriver->header->oPointers[ i]]->header.size-1;
		}

		for( x = curPartitionReader; x >= 0; x--) {
			for( y = MDriver->header->numChn-1; y >= 0 ; y--) {
				aCmd = GetMADCommand( x, y, MDriver->partition[ MDriver->header->oPointers[ i]]);
				
				if( aCmd->cmd == speedE) {					
					if( aCmd->arg < 32) {
						if( aCmd->arg != 0) {
							if( CmdSpeed == FALSE) {
								intDriver->speed = aCmd->arg;
							}
							CmdSpeed = TRUE;
						}
					} else {
						if( aCmd->arg != 0) {
							if( FineFound == FALSE) {
								intDriver->finespeed = aCmd->arg;
							}
							FineFound = TRUE;
						}
					}
				}
			}
			if( CmdSpeed == TRUE && FineFound == TRUE) {
				return;
			}
		}
	}
		

	if( !CmdSpeed) {
		intDriver->speed = MDriver->header->speed;
	}
	if( !FineFound) {
		intDriver->finespeed = MDriver->header->tempo;
	}
}

/**********************************

	Start a song
	
**********************************/

int BURGERCALL MADPlayMusic( MADDriverRec *MDriver)
{
	if (MDriver) {
		if (MDriver->curMusic) {
			MDriver->Reading = TRUE;
			return 0;
		}
		return MADDriverHasNoMusic;
	}
	return MADParametersErr;
}

/**********************************

	Stop a song
	
**********************************/

int BURGERCALL MADStopMusic( MADDriverRec *MDriver)
{
	if (MDriver) {
		if (MDriver->curMusic) {
			MDriver->Reading = FALSE;
			return 0;			/* I'm ok! */
		}
		return MADDriverHasNoMusic;
	}
	return MADParametersErr;
}

/**********************************

	Reset a song
	
**********************************/

void BURGERCALL MADReset( MADDriverRec *MDriver)
{
	MADCleanDriver( MDriver);
	MDriver->BufCounter = 0;
	MDriver->BytesToGenerate = 0;
	MDriver->smallcounter = 128;
	
	MDriver->PL = 0;
	MDriver->PartitionReader = 0;
	if (MDriver->curMusic) {
		MDriver->Pat = MDriver->curMusic->header->oPointers[0];
		MDriver->speed = MDriver->curMusic->header->speed;
		MDriver->finespeed = MDriver->curMusic->header->tempo;
	}
}

/**********************************

	Delete a song
	
**********************************/

void BURGERCALL MADMusicDestroy(MADMusic *Input)
{
	Word i;
	Input->musicUnderModification = TRUE;		/* IRQ's can't touch me! */

	if (Input->header) {
		i = 0;
		do {
			DeallocAPointer(Input->partition[i]);
		} while (++i<MAXPATTERN);
		i = 0;
		do {
			MADKillInstrument(Input,i);
		} while (++i<MAXINSTRU);
		DeallocAPointer(Input->header);
	}
	DeallocAPointer(Input->fid);
	DeallocAPointer(Input->sample);
	FastMemSet(Input,0,sizeof(MADMusic));
}

/**********************************

	Return a specific MAD command byte
	
**********************************/

Cmd_t* BURGERCALL GetMADCommand(int PosX,int TrackIdX, PatData_t* tempMusicPat)
{
	if (PosX < 0) {
		PosX = 0;
	} else if (PosX >= tempMusicPat->header.size) {
		PosX = tempMusicPat->header.size;
		if (PosX) {
			--PosX;
		}
	}
	return &tempMusicPat->Cmds[(tempMusicPat->header.size * TrackIdX) + PosX];
}

/**********************************

	Init the MOD music driver

**********************************/

void BURGERCALL ModMusicInit(void)
{
	MADDriverSettings WhichDriver;
	if (InitDigitalDriver()) {		/* Can I start the digital system */
		if (!ModMusicDriver) {
			MADGetBestDriver(&WhichDriver);	/* Which driver should I use? */
			ModLibrary.TotalPlug = 0;
			if (!MADCreateDriver(&WhichDriver,&ModLibrary,&ModMusicDriver)) {
				BurgerSndKillProcs[MIDIKILL] = ModMusicShutdown;	/* Set the shut down */
				BurgerSndExitIn |= MIDIMUSICON;		/* I am active */
				EnableSoundShutdownProc();	/* Enable shutdown */
				return;
			}
			ModMusicDriver = 0;
			ModLibrary.TotalPlug = 0;
		}
		ModMusicShutdown();
	}
}

/**********************************

	Kill the music player

**********************************/

void BURGERCALL ModMusicShutdown(void)
{
	ModMusicStop();					/* If there is a song, stop it */

	if (ModMusicDriver) {
		MADDisposeDriver(ModMusicDriver);	/* Dispose music driver */
		ModMusicDriver = 0;		/* Kill the pointer */
	}
	ModLibrary.TotalPlug = 0;		/* Close music library */

	BurgerSndKillProcs[MIDIKILL] = 0;		/* Acknowledge my proc */
	BurgerSndExitIn &= ~MIDIMUSICON;	/* I am zapped */
	KillDigitalDriver();		/* Release the driver */
}

/**********************************

	Log an importer

**********************************/

void BURGERCALL ModMusicImporter(MADImportPtr ImportPtr)
{
	MADLibrary_t *WorkPtr;
	Word i;
	if (ImportPtr && ModMusicDriver) {
		WorkPtr = &ModLibrary;
		i = WorkPtr->TotalPlug;
		if (i<MAXPLUG) {
			WorkPtr->ThePlug[i] = ImportPtr;
			++i;
			WorkPtr->TotalPlug=i;
		}
	}
}

/**********************************

	Play a midi song

**********************************/

Word BURGERCALL ModMusicPlay(Word SongNum)
{
	if (ModMusicDriver) {
		Word OpenErr;
		OpenErr = 0;
		if (ModMusicLastSongNum!=SongNum) {
			ModMusicStop();
			if (SongNum) {
				void **Data;
				Data = ResourceLoadHandle(SoundRezHeader,SongNum);
				OpenErr = TRUE;
				if (Data) {
					Word32 Length;
					Length = GetAHandleSize(Data);
					OpenErr = ModMusicPlayByPtr(static_cast<Word8 *>(LockAHandle(Data)),Length);
					UnlockAHandle(Data);
					ResourceRelease(SoundRezHeader,SongNum);
					if (!OpenErr) {
						ModMusicLastSongNum = SongNum;
					}
				}
			}
		}
		return OpenErr;
	}
	return TRUE;
}

/**********************************

	Play a song from a file

**********************************/

Word BURGERCALL ModMusicPlayByFilename(const char *FileName)
{
	Word Result;
	void *DataPtr;
	Word32 Length;
	
	Result = TRUE;
	if (ModMusicDriver) {
		ModMusicStop();		/* Shut down the previous song */
		DataPtr = LoadAFile(FileName,&Length);
		if (DataPtr) {
			Result = ModMusicPlayByPtr(static_cast<Word8 *>(DataPtr),Length);
			DeallocAPointer(DataPtr);
			if (!Result) {
				ModMusicLastSongNum = (Word)-1;
			}
		}
	}
	return Result;
}

/**********************************

	Play a MOD file image

**********************************/

Word BURGERCALL ModMusicPlayByPtr(const Word8 *DataPtr,Word32 Length)
{
	Word OpenErr;
	
	if (ModMusicDriver) {
		ModMusicStop();
		OpenErr = 0;
		if (DataPtr) {
			OpenErr = MADLoadMusicPtr(&ModLibrary,&ModMusicRec,DataPtr,Length);
			if (!OpenErr) {
				MADAttachDriverToMusic(ModMusicDriver,ModMusicRec);
				MADStartDriver(ModMusicDriver);			// Turn interrupt driver function ON
				MADPlayMusic(ModMusicDriver);			// Read the current partition in memory
			}
		}
		return OpenErr;
	}
	return TRUE;
}

/**********************************

	Stop a midi song

**********************************/

void BURGERCALL ModMusicStop(void)
{
	if (ModMusicLastSongNum) {
		ModMusicLastSongNum = 0;
		MADStopMusic(ModMusicDriver);		// Stop reading current partition
		MADStopDriver(ModMusicDriver);		// Stop driver interrupt function
		MADMusicDestroy(ModMusicRec);		// Dispose the current music
		DeallocAPointer(ModMusicRec);
		ModMusicRec = 0;					// No song present
	}
}

/**********************************

	Pause a playing song

**********************************/

void BURGERCALL ModMusicPause(void)
{
	if (!ModMusicPaused) {
		if (ModMusicLastSongNum) {
			MADStopMusic(ModMusicDriver);		/* Pause the song */
			MADCleanDriver(ModMusicDriver);		/* Stop all instruments */
		}
		ModMusicPaused = TRUE;
	}
}

/**********************************

	Unpause a playing song

**********************************/

void BURGERCALL ModMusicResume(void)
{
	if (ModMusicPaused) {
		if (ModMusicLastSongNum) {
			MADPlayMusic(ModMusicDriver);		/* Resume the song */
		}
		ModMusicPaused = FALSE;
	}
}

/**********************************

	Reset a playing song

**********************************/

void BURGERCALL ModMusicReset(void)
{
	if (ModMusicLastSongNum) {
		MADReset(ModMusicDriver);		/* Reset the song */
	}
}

/**********************************

	Return the current volume of the music

**********************************/

Word BURGERCALL ModMusicGetVolume(void)
{
	return MusicVolume;
}

/**********************************

	Set the volume of a midi song

**********************************/

void BURGERCALL ModMusicSetVolume(Word Volume)
{
	if (Volume>=256) {
		Volume = 255;
	}
	MusicVolume = Volume;
	if (!Volume) {
		SystemState &= ~(MusicActive);
	} else {
		SystemState |= MusicActive;
	}
	if (Volume>=128) {			/* Adjust to force 0 to 64 */
		++Volume;
	}
	if (ModMusicDriver) {		/* Active driver? */
		ModMusicDriver->VolGlobal = Volume>>2;		/* 0 to 64 */
	}
}

#if !defined(__MAC__) && !defined(__MSDOS__) && !defined(__WIN32__)

/**********************************

	Start the sound system
	
**********************************/

int BURGERCALL MADSndOpen( MADDriverRec *inMADDriver)
{
	return -1;		/* Can't open a channel */
}

/**********************************

	Shut down audio hardware
	
**********************************/

void BURGERCALL MADSndClose(MADDriverRec *inMADDriver)
{

}

/**********************************

	Start audio
	
**********************************/

void BURGERCALL MADStartDriver( MADDriverRec *MDriver)
{
}

/**********************************

	Stop audio
	
**********************************/

void BURGERCALL MADStopDriver( MADDriverRec *MDriver)
{
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
	Init->outPutBits = 8;
	Init->outPutRate = 22050<<16;
}


#endif