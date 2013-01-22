/**********************************

	IT file importer

**********************************/

#include "SnMadMusic.h"
#include <BREndian.hpp>
#include "ClStdLib.h"
#include "MmMemory.h"

/* All data is in little endian */

typedef struct ITPatForm {
	Word16 length;
	Word16 row;
	long no;
	char data[4];
} ITPatForm;

typedef struct ITSampForm {
	long ID;
	char DOSName[12];
	Word8 no;
	Word8 GvL;
	Word8 Flag;
	Word8 Vol;
	char SampName[26];
	short Convert;
	long length;			/* Size of the sample size */
	long loopBegin;
	long loopEnd;
	long C5Speed;
	long SusLoopBegin;
	long SusLoopEnd;
	long samplePtr;			/* Offset to the sample */
	Word8 ViS;
	Word8 ViD;
	Word8 ViR;
	Word8 ViT;
} ITSampForm;

typedef struct ITNode {		/* Note : This structure is no good on a mac */
	char y;
	short x;
} ITNode;

typedef struct ITEnv {
	Word8 Flag;
	Word8 Num;
	Word8 LpB;
	Word8 LpE;
	Word8 SLB;
	Word8 SLE;
	ITNode nodes[25];
} ITEnv;

typedef struct ITKeyMap {
	Word8 note;
	Word8 samp;
} ITKeyMap;

typedef struct ITInsForm {		// size = 547
	long ID;
	char DOSName[ 12];
	Word8 no;
	Word8 NNA;
	Word8 DCT;
	Word8 DCA;
	short FadeOut;
	Word8 PPS;
	Word8 PPC;
	Word8 GbV;
	Word8 DfP;
	char no2[ 2];
	short TrkVers;
	Word8 NoS;
	Word8 no3;
	char INSName[ 26];
	char no4[ 6];
	ITKeyMap keyMap[ 120];

	// new structure

	ITEnv volEnv;
	ITEnv panEnv;
	ITEnv pitchEnv;
} ITInsForm;

typedef struct ITOldInsForm	{	// size = 554
	long ID;
	char DOSName[ 12];
	Word8 no;
	Word8 NNA;
	Word8 DCT;
	Word8 DCA;
	short FadeOut;
	Word8 PPS;
	Word8 PPC;
	Word8 GbV;
	Word8 DfP;
	char no2[ 2];
	short TrkVers;
	Word8 NoS;
	Word8 no3;
	char INSName[ 26];
	char no4[ 6];
	ITKeyMap keyMap[ 120];
	// old structure
	ITKeyMap	volEnv[ 100];
} ITOldInsForm;

typedef struct ITHeader_t {		/* IT file header */
	Word32 ID;		/* IMPM */
	char name[26];		/* Song name */
	char no[2];			/* Palette/Row highlight info (For editing) */
	short OrderNum;		/* Number of orders in the song */
	short InsNum;		/* Number of instruments in the song */
	short SmpNum;		/* Number of samples in the song */
	short PatNum;		/* Number of patterns in the song */
	short Cwtv;			/* File version */
	short Cmwt;			/* Tracker version */
	short Flags;		/* Flags */
	short Special;		/* Special flags */
	Word8 globalVol;
	Word8 mixVol;
	Word8 iSpeed;
	Word8 iTempo;
	char panSeparation;
	char null;
	short MsgLgth;		/* Size of internal message */
	long MsgOffset;		/* Where is the message */
	long Reserved;		/* Not used */
	char chanPan[64];	/* Pan data */
	char chanVol[64];	/* Volume data */	
} ITHeader_t;

#ifdef __BIGENDIAN__
#define ITSIG 0x494D504D		/* IMPM */
#define IMPISIG 0x494D5049		/* IMPI */
#define IMPSSIG 0x494D5053		/* IMPS */
#else
#define ITSIG 0x4D504D49
#define IMPISIG 0x49504D49
#define IMPSSIG 0x53504D49
#endif

/**********************************

	Convert an IT effect to a MADI effect

**********************************/

static void ConvertITEffect(Word B0,Word B1,Word8 *CmdPtr, Word8 *ArgPtr)
{
	Word LoB1 = B1&0xF;
	Word HiB1 = B1>>4;
	Word Cmd;
	Word Arg;
	
	switch (B0) {
	default:
		Cmd = 0;
		Arg = 0;
		break;
	// Speed
	case 'A'-0x40:
		Cmd = speedE;
		Arg = B1;
		break;
	// Tempo
	case 'T'-0x40:
		Cmd = speedE;
		Arg = B1;
		break;

	case 'B'-0x40:
		Cmd = fastskipE;
		Arg = B1;
		break;

	case 'C'-0x40:
		Cmd = skipE;
		Arg = B1;
		break;

	case 'D'-0x40:
		if (LoB1 == 0 || HiB1 == 0) {	// Slide volume
			Cmd = slidevolE;
			Arg = B1;
		} else if( HiB1 == 0x0F) {		// Fine Slide volume DOWN
			Cmd = extendedE;
			Arg = (LoB1+(11 << 4));
		} else if( LoB1 == 0x0F) {		// Fine Slide volume UP
			Cmd = extendedE;
			Arg = (HiB1+(10 << 4));
		} else {
			Cmd = 0;
			Arg = 0;
		}
		break;

	case 'E'-0x40:
		if( HiB1 == 0x0F) {		// FineSlide DOWN
			Cmd = extendedE;
			Arg = (LoB1+(2 << 4));		//not supported
		} else if( HiB1 == 0x0E) {	// ExtraFineSlide DOWN
			Cmd = 0;
			Arg = 0;		//not supported
		} else {					// Slide DOWN
			Cmd = upslideE;
			Arg = B1;
		}
		break;

	case 'F'-0x40:
		if( HiB1 == 0x0F) {		// FineSlide UP
			Cmd = extendedE;
			Arg = (LoB1+(1 << 4));		//not supported
		} else if( HiB1 == 0x0E) {	// ExtraFineSlide UP
			Cmd = 0;
			Arg = 0;		//not supported
		} else {					// Slide UP
			Cmd = downslideE;
			Arg = B1;
		}
		break;

	case 'G'-0x40:
		Cmd = portamentoE;
		Arg = B1;
		break;
	case 'H'-0x40:
		Cmd = vibratoE;
		Arg = B1;
		break;
	case 'J'-0x40:
		Cmd = arpeggioE;
		Arg = B1;
		break;
	case 'K'-0x40:
		Cmd = vibratoslideE;
		Arg = B1;
		break;
	case 'L'-0x40:
		Cmd = portaslideE;
		Arg = B1;
		break;
	case 'O'-0x40:
		Cmd = offsetE;	
		Arg = B1;
		break;

	case 'S'-0x40:		// Special Effects
		switch (HiB1) {
		default:
			Cmd = 0;
			Arg = 0;
			break;
		case 2:
			Cmd = extendedE;
			Arg = (LoB1+(5 << 4));
			break;	// FineTune
		case 3:
			Cmd = extendedE;
			Arg = (LoB1+(4 << 4));
			break;	// Set Vibrato WaveForm
		case 4:
			Cmd = extendedE;
			Arg = (LoB1+(7 << 4));
			break;	// Set Tremolo WaveForm
		case 0xB:
			Cmd = extendedE;
			Arg = (LoB1+(6 << 4));
			break;	// Loop pattern
		case 0xC:
			Cmd = extendedE;
			Arg = (LoB1+(12 << 4));
			break;	// Cut sample
		case 0xD:
			Cmd = extendedE;
			Arg = (LoB1+(13 << 4));
			break;	// Delay sample
		case 0xE:
			Cmd = extendedE;
			Arg = (LoB1+(14 << 4));
			break;	// Delay pattern
		}
		break;
	}
	CmdPtr[0] = (Word8)Cmd;
	ArgPtr[0] = (Word8)Arg;
}

/**********************************

	Convert an IT file to a MADI file

**********************************/

static int ConvertIT2Mad(const Word8 * theIT, long /* MODSize */, MADMusic *theMAD)
{
	ITHeader_t *ITHeaderPtr;		/* Pointer to the data header (Note : Little endian) */
	Word OrderNum;					/* Converted data from the ITHeader_t */
	Word InsNum;
	Word SmpNum;
	Word PatNum;
	Word HeaderFlags;

	const Word8 *theITCopy;			/* Work pointer to the input data */
	const Word8 *ITOrdersPtr;		/* Pointer to Orders (Not allocated) */
	SWord32 *ITParaInsPtr;				/* Instrument data (Not allocated/little endian) */
	SWord32 *ITParaSampPtr;			/* Sample data (Not allocated/little endian) */
	SWord32 *ITParaPatPtr;				/* Pattern data (Not allocaed/little endian) */
	ITInsForm **InsDataArrayPtr;	/* Array of pointers to instruments */
	ITSampForm **SampDataArrayPtr;	/* Array of pointers to samples */
	Word8 *theInstrument[256];		/* Mad Instrument */
	
//	ITSampForm *SampDataPtr;
	long i, x, z, channel, Row;
	Word8 *MaxPtr;
	Word8 tempChar;
	int maxTrack;
	Cmd_t *aCmd;
	int Error;

	ITOrdersPtr = 0;
	ITParaInsPtr = 0;
	ITParaSampPtr = 0;
	ITParaPatPtr = 0;
	InsDataArrayPtr = 0;
	SampDataArrayPtr = 0;

	FastMemSet(theInstrument,0,sizeof(theInstrument));

	/* Get the header */
	
	ITHeaderPtr = (ITHeader_t *)theIT;
	theITCopy = theIT+192;

	/**** Order Num *****/
	
	OrderNum = Burger::LoadLittle(&ITHeaderPtr->OrderNum);
	if (OrderNum) {					/* Any data present? */
		ITOrdersPtr = theITCopy;	/* Lock the pointer */
		theITCopy += OrderNum;		/* Accept the data */
	}


	/**** Ins Num *****/
	
	InsNum = Burger::LoadLittle(&ITHeaderPtr->InsNum);
	if (InsNum) {
		ITParaInsPtr = (SWord32 *)theITCopy;
		theITCopy += InsNum * 4UL;
	}

	/**** Samp Num *****/
	SmpNum = Burger::LoadLittle(&ITHeaderPtr->SmpNum);
	if (SmpNum) {
		ITParaSampPtr = (SWord32 *) theITCopy;
		theITCopy += SmpNum * 4UL;
	}

	/**** Pat Num *****/
	PatNum = Burger::LoadLittle(&ITHeaderPtr->PatNum);
	if (PatNum) {
		ITParaPatPtr = (long *)theITCopy;
		theITCopy += PatNum * 4UL;
	}

	/* Instruments */
	
	HeaderFlags = Burger::LoadLittle(&ITHeaderPtr->Flags);
	if ((HeaderFlags & 4) && InsNum) {
		/**** Ins Data ****/
		Word InsTemp1;
		ITInsForm **FormTempPtr1;
		InsDataArrayPtr = (ITInsForm **) AllocAPointer(sizeof(ITInsForm *) * InsNum);
		Error = MADNeedMemory;
		if (InsDataArrayPtr) {
			InsTemp1 = 0;
			FormTempPtr1 = InsDataArrayPtr;
			Error = 0;			/* Assume no error */
			do {
				ITInsForm *WorkPtr1;
				WorkPtr1 = (ITInsForm*)((Word8*) theIT+Burger::LoadLittle(&ITParaInsPtr[InsTemp1]));
				if (WorkPtr1->ID != IMPISIG) {
					Error = MADIncompatibleFile;
					break;
				}
				FormTempPtr1[0] = WorkPtr1;
				++FormTempPtr1;
			} while (++InsTemp1<InsNum);
		}
		if (Error) {
			goto CleanUp;
		}
	}

	/**** Samp Data ****/
	
	if (SmpNum) {
		Word SmpNum2;
		ITSampForm **SampWorkPtr;
		SampDataArrayPtr = (ITSampForm **) AllocAPointer(sizeof(ITSampForm *) * SmpNum);
		if (!SampDataArrayPtr) {
			Error = MADNeedMemory;
			goto CleanUp;
		}
		SmpNum2 = 0;
		SampWorkPtr = SampDataArrayPtr;
		do {
			ITSampForm *SampTmpPtr;
			
			SampTmpPtr = (ITSampForm*)((Word8 *)theIT+Burger::LoadLittle(&ITParaSampPtr[SmpNum2]));
			if (SampTmpPtr->ID != IMPSSIG) {
				Error = MADIncompatibleFile;
				goto CleanUp;
			}
			SampWorkPtr[0] = SampTmpPtr;
			if (Burger::LoadLittle(&SampTmpPtr->length) > 0) {
				theInstrument[SmpNum2] = (Word8 *)theIT+Burger::LoadLittle(&SampTmpPtr->samplePtr);
			}
			++SampWorkPtr;
		} while (++SmpNum2<SmpNum);
	}

	/* Here is where I output my converted sound file */

	/* TODO, Optimize from here down */
	
	theMAD->header = (MADSpec*) AllocAPointerClear( sizeof( MADSpec));
	if( theMAD->header == 0L) return MADNeedMemory;

	theMAD->header->MAD = 'MADI';
	strncpy(theMAD->header->name,ITHeaderPtr->name,28);
	theMAD->header->name[27] = 0;
	
	strcpy( theMAD->header->infos,"Converted");

	theMAD->header->numPat = (Word8)PatNum;
	theMAD->header->numPointers	= (Word8)OrderNum;
	theMAD->header->speed = ITHeaderPtr->iSpeed;
	theMAD->header->tempo = ITHeaderPtr->iTempo;

	FastMemSet(theMAD->header->oPointers,0,sizeof(theMAD->header->oPointers));
	for (i=0; i<(int)OrderNum; i++) {
		Word Px;
		Px = ITOrdersPtr[i];
		if (Px >= PatNum) {
			Px = 0;
		}
		theMAD->header->oPointers[i] = (Word8)Px;
	}

	i = 0;
	do {
		if (i < 64) {
			theMAD->header->chanPan[ i] = ITHeaderPtr->chanPan[i];
			theMAD->header->chanVol[ i] = ITHeaderPtr->chanVol[i];
		} else {
			if (!(i & 1)) {
				theMAD->header->chanPan[ i] = MAX_PANNING/4;
			} else {
				theMAD->header->chanPan[ i] = MAX_PANNING - MAX_PANNING/4;
			}
			theMAD->header->chanVol[ i] = MAX_VOLUME;
		}
	} while (++i<MAXTRACK);
	theMAD->header->generalVol		= 64;
	theMAD->header->generalSpeed	= 80;
	theMAD->header->generalPitch	= 80;


	// ********************
	// ***** INSTRUMENTS *****
	// ********************

	theMAD->fid = ( InstrData*) AllocAPointerClear( sizeof( InstrData) * (long) MAXINSTRU);
	if( !theMAD->fid) return MADNeedMemory;

	theMAD->sample = ( sData_t**) AllocAPointerClear( sizeof( sData_t*) * (long) MAXINSTRU * (long) MAXSAMPLE);
	if( !theMAD->sample) return MADNeedMemory;

	for( i = 0; i < MAXINSTRU; i++) {
		theMAD->fid[ i].firstSample = (short)(i * MAXSAMPLE);
	}
	for(i  = 0 ; i < MAXINSTRU; i++) {
		for( x = 0; x < MAXSAMPLE; x++) {
			theMAD->sample[ i*MAXSAMPLE + x] = 0L;
		}
		theMAD->fid[i].numSamples	= 0;
	}

	if(HeaderFlags & 4) {		// USE INSTRUMENTS
		int minSamp;

		for (i=0; i<(int)InsNum; i++) {
			InstrData *curIns = &theMAD->fid[ i];

			curIns->type	= 0;

			{
				sData_t	*curData;
				int prevSamp;
				int zz;
				int newsamp;
				ITInsForm *InsDataPtr1;
				
				InsDataPtr1 = InsDataArrayPtr[i];
				// Instrument conversion

				curIns->numSamples	= 0;	//ITinfo.insdata[ i].NoS;
				for( x = 0; x < 26; x++) {
					curIns->name[ x] = InsDataPtr1->INSName[ x];
				}
				for( x = 0; x < 96; x++) {
					curIns->what[ x] = 0;
				}
				minSamp = 200;
				for( x = 0; x < 120; x++) {
					newsamp = InsDataPtr1->keyMap[x].samp;
					if (newsamp) {
						if (newsamp>(int)SmpNum) {
							newsamp = SmpNum;
						}
						if (newsamp-1 < minSamp) {
							minSamp = newsamp-1;
						}
					}
				}

				for( x = 0; x < 120; x++) {
					newsamp = InsDataPtr1->keyMap[ x].samp;
					if(newsamp) {
						if (newsamp>(int)SmpNum) {
							newsamp = SmpNum;
						}
						if( InsDataPtr1->keyMap[ x].note < 96) {
							curIns->what[InsDataPtr1->keyMap[x].note] = newsamp-1 - minSamp;
						}
					}
				}

				// Samples conversion

				zz = 0;
				prevSamp = -1;

				for( zz = 0; zz < 120; zz++) {
					if (InsDataPtr1->keyMap[ x].note<96) {
					newsamp = InsDataPtr1->keyMap[zz].samp;
					if (newsamp) {
						--newsamp;
						if (newsamp>=(int)SmpNum) {
							newsamp = SmpNum-1;
						}
						if (prevSamp != newsamp) {
							ITSampForm *SampPtr3;
							prevSamp = newsamp;
							curData = theMAD->sample[ i*MAXSAMPLE + curIns->numSamples] = (sData_t*) AllocAPointerClear( sizeof( sData_t));
							if( curData == 0L) return MADNeedMemory;
							SampPtr3 = SampDataArrayPtr[prevSamp];
							curData->size = Burger::LoadLittle(&SampPtr3->length);
							if(SampPtr3->Flag&16) {
								curData->loopBeg 	= Burger::LoadLittle(&SampPtr3->loopBegin);
								curData->loopSize	= Burger::LoadLittle(&SampPtr3->loopEnd) - curData->loopBeg;
							} else {
								curData->loopBeg = 0;
								curData->loopSize = 0;
							}

							curData->vol = SampPtr3->GvL;
							curData->c2spd = (Word16)Burger::LoadLittle(&SampPtr3->C5Speed);
							curData->loopType	= 0;
							curData->amp			= 8;

							if( SampPtr3->Flag&2) {
								curData->amp		= 16;

								curData->size		*= 2;
								curData->loopBeg	*= 2;
								curData->loopSize *= 2;
							}


							curData->relNote	= -12;
							for( z = 0; z < 26; z++) {
								curData->name[ z] = SampPtr3->SampName[ z];
							}
							curData->data = 0;
							if (curData->size) {
								curData->data = (char *)AllocAPointer(curData->size);
								if( curData->data == 0L) {
									return MADNeedMemory;
								}
							}
							if( curData->data != 0L) {
								Word ConvertFlag3;
								FastMemCpy(  curData->data,theInstrument[ prevSamp], curData->size);
								ConvertFlag3 = Burger::LoadLittle(&SampPtr3->Convert);
								if( !(ConvertFlag3 & 1) && curData->amp == 8) {
									long temp;

									for( temp = 0; temp < curData->size; temp++) {
										*(curData->data + temp) -= (char)0x80;
									}
								}
	
								if( curData->amp == 16) {
									unsigned short 	*tempShort = (unsigned short*) curData->data;
									long temp;
	
									for( temp = 0; temp < curData->size/2; temp++) {
#if defined(__BIGENDIAN__)
										tempShort[ temp] = Burger::SwapEndian(tempShort[temp]);
#endif
										if( !(ConvertFlag3 & 1)) {
											*(tempShort + temp) -= 0x8000;
										}
									}
								}
							}
							curIns->numSamples++;
						}
					}
					}
				}
			}
		}
	} else {										// USE SAMPLES AS INSTRUMENTS
		for(i=0; i<(int)SmpNum; i++) {
			InstrData *curIns = &theMAD->fid[ i];

			curIns->type	= 0;

			if( theInstrument[i]) {
				sData_t	*curData;
				ITSampForm *SampPtr4;

				curIns->numSamples	= 1;
				curIns->volFade			= DEFAULT_VOLFADE;

				curData = theMAD->sample[ i*MAXSAMPLE + 0] = (sData_t*) AllocAPointerClear( sizeof( sData_t));
				if( curData == 0L) return MADNeedMemory;

				SampPtr4 = SampDataArrayPtr[i];
				curData->size		= Burger::LoadLittle(&SampPtr4->length);
				curData->loopBeg 	= Burger::LoadLittle(&SampPtr4->loopBegin);
				curData->loopSize	= Burger::LoadLittle(&SampPtr4->loopEnd) - curData->loopBeg;
				curData->vol		= SampPtr4->GvL;
				curData->c2spd		= (Word16)Burger::LoadLittle(&SampPtr4->C5Speed);
				curData->loopType	= 0;
				curData->amp		= 8;

				if( SampPtr4->Flag&2) {
					curData->amp		= 16;

					curData->size		*= 2;
					curData->loopBeg	*= 2;
					curData->loopSize 	*= 2;
				}

				curData->relNote	= -12;
				for( x = 0; x < 26; x++) {
					curIns->name[x] = SampPtr4->SampName[x];
				}
				curData->data = (char *)AllocAPointer( curData->size);
				if( curData->data == 0L) {
					return MADNeedMemory;
				}
				if( curData->data != 0L) {
					Word ConvertFlag4;
					FastMemCpy( curData->data, theInstrument[i], curData->size);
					ConvertFlag4 = Burger::LoadLittle(&SampPtr4->Convert);
					if( !(ConvertFlag4 & 1) && curData->amp == 8) {
						long temp;

						for( temp = 0; temp < curData->size; temp++) {
							*(curData->data + temp) -= (char)0x80;
						}
					}

					if( curData->amp == 16) {
						unsigned short 	*tempShort = (unsigned short*) curData->data;
						long temp;

						for( temp = 0; temp < curData->size/2; temp++) {
#if defined(__BIGENDIAN__)
							tempShort[ temp] = Burger::SwapEndian(tempShort[temp]);
#endif
							if( !(ConvertFlag4 & 1)) {
								*(tempShort + temp) -= 0x8000;
							}
						}
					}
				}
			} else {
				curIns->numSamples = 0;
			}
		}
	}
	//	*********************
	//	*           Check MaxTrack         *
	//	*********************

	maxTrack = 0;

	for( i = 0; i < theMAD->header->numPat ; i++) {
		ITPatForm *curITPat;
		long PatTemp1;
		PatTemp1 = Burger::LoadLittle(&ITParaPatPtr[i]);
		if(PatTemp1) {
			curITPat = (ITPatForm*) (theIT + PatTemp1);

#if defined(__BIGENDIAN__)
			curITPat->length = Burger::SwapEndian(curITPat->length);
			curITPat->row = Burger::SwapEndian(curITPat->row);
#endif
			if(PatTemp1 > 0) {
				char *curDataPat = curITPat->data;
				char maskvariable = 0;
				char prevmaskvariable[ MAXTRACK];
				Bool NeedChannelToRead = TRUE;
				int xx;

				for( xx = 0; xx < MAXTRACK; xx++) {
					prevmaskvariable[ xx] = 0;
				}
				Row = 0;
				while( Row < curITPat->row) {
					tempChar = *curDataPat;
					curDataPat++;

					if (tempChar == 0) {
						Row++;
					} else {
						if( NeedChannelToRead) {
							// Channel
							channel = (tempChar-1) & 63;

							if( channel > maxTrack) {
								maxTrack = channel;
							}
						}

						if(tempChar & 128) {
							prevmaskvariable[ channel] = maskvariable = *curDataPat;
							curDataPat++;
						} else {
							maskvariable = prevmaskvariable[ channel];
						}
						// NOTE
						if( maskvariable & 1) curDataPat++;
						if( maskvariable & 2) curDataPat++;
						if( maskvariable & 4) curDataPat++;
						if( maskvariable & 8) curDataPat += 2;
					}
				}

				if( curDataPat - curITPat->data !=  curITPat->length) {
					return MADUnknowErr;
				}
			}
		}
	}

	maxTrack = ((maxTrack+2)/2)*2;

	// ********************
	// ***** TEMPORAIRE ******
	// ********************

	theMAD->header->numChn = (Word8)maxTrack;

	for( i = 0; i < MAXPATTERN; i++) {
		theMAD->partition[ i] = 0L;
	}
	for( i = 0; i < theMAD->header->numPat ; i++) {
		ITPatForm *curITPat;
		long PatTemp2;
		PatTemp2 = Burger::LoadLittle(&ITParaPatPtr[i]);
		if(PatTemp2) {
			curITPat = (ITPatForm*) (theIT + PatTemp2);

			theMAD->partition[ i] = (PatData_t*) AllocAPointerClear( sizeof( PatHeader_t) + theMAD->header->numChn * curITPat->row * sizeof( Cmd_t));
			if( theMAD->partition[ i] == 0L) return MADNeedMemory;

			theMAD->partition[ i]->header.size 			= curITPat->row;
			theMAD->partition[ i]->header.compMode 	= 'NONE';

			for( x = 0; x < 20; x++) theMAD->partition[ i]->header.name[ x] = 0;

			MaxPtr = (Word8 *) theMAD->partition[ i];
			MaxPtr += sizeof( PatHeader_t) + theMAD->header->numChn * curITPat->row * sizeof( Cmd_t);

			for( Row = 0; Row < curITPat->row; Row++) {
				for(z = 0; z < theMAD->header->numChn; z++) {
					aCmd = GetMADCommand( Row, z, theMAD->partition[ i]);

					aCmd->note		= 0xFF;
					aCmd->ins			= 0;
					aCmd->cmd			= 0;
					aCmd->arg			= 0;
					aCmd->vol			= 0xFF;
				}
			}
		} else {	// No Data for this pattern - Clear Pattern
			#define DEFSIZE 10L
			theMAD->partition[ i] = (PatData_t*) AllocAPointerClear( sizeof( PatHeader_t) + theMAD->header->numChn * DEFSIZE * sizeof( Cmd_t));
			if( theMAD->partition[ i] == 0L) {
				return MADNeedMemory;
			}
			theMAD->partition[ i]->header.size = DEFSIZE;
			theMAD->partition[ i]->header.compMode 	= 'NONE';

			strcpy( theMAD->partition[ i]->header.name, "Not used pattern");

			for( Row = 0; Row < DEFSIZE; Row++) {
				for(z = 0; z < theMAD->header->numChn; z++) {
					aCmd = GetMADCommand( Row, z, theMAD->partition[ i]);

					aCmd->note = 0xFF;
					aCmd->ins = 0;
					aCmd->cmd = 0;
					aCmd->arg = 0;
					aCmd->vol = 0xFF;
				}
			}
		}

		if (PatTemp2) {
			char *curDataPat = curITPat->data;
			char maskvariable = 0;
			char prevmaskvariable[ MAXTRACK];
			Bool NeedChannelToRead = TRUE;
			short xx;

			char lastnote[ MAXTRACK];
			char lastins[ MAXTRACK];
			char lastvol[ MAXTRACK];
			char lastcmd[ MAXTRACK];
			char lasteff[ MAXTRACK];

			for( xx = 0; xx < MAXTRACK; xx++) {
				prevmaskvariable[ xx] = 0;
			}
			Row = 0;
			while( Row < curITPat->row) {
				tempChar = *curDataPat;
				curDataPat++;

				if( tempChar == 0) {
					Row++;
				} else {
					if( NeedChannelToRead) {
						// Channel
						channel = (tempChar-1) & 63;
						if( channel >= 0 && channel < theMAD->header->numChn) {
							aCmd = GetMADCommand( Row, channel, theMAD->partition[ i]);
						} else {
							return MADUnknowErr;
						}
					}

					if(tempChar & 128) {
						prevmaskvariable[ channel] = maskvariable = *curDataPat;
						curDataPat++;
					} else {
						maskvariable = prevmaskvariable[ channel];
					}
					// NOTE
					if( maskvariable & 1 || maskvariable & 16) {
						char note;

						if( maskvariable & 1) {
							note = *curDataPat++;
						} else {
							note = lastnote[ channel];
						}
						lastnote[ channel] = note;
						if( aCmd != 0L) {
							aCmd->note = note;
							if( aCmd->note == 255) {
								aCmd->note = 0xFE;
							} else if (aCmd->note >= NUMBER_NOTES) {
								aCmd->note = 0xFF;
							}
						}
					}

					// INSTRUMENT
					if( maskvariable & 2 || maskvariable & 32) {
						char ins;

						if( maskvariable & 2) {
							ins = *curDataPat++;
						} else {
							ins = lastins[ channel];
						}
						lastins[ channel] = ins;
						if( aCmd != 0L) {
							aCmd->ins = ins;
						}
					}

					// VOLUME
					if( maskvariable & 4 || maskvariable & 64) {
						char vol;

						if( maskvariable & 4) {
							vol = *curDataPat++;
						} else {
							vol = lastvol[ channel];
						}
						lastvol[ channel] = vol;

						if (aCmd != 0L) {
							aCmd->vol = vol;
							if (aCmd->vol > 64) {
								aCmd->vol = 64;
							}
							aCmd->vol += 0x10;
						}
					} else {
						aCmd->vol = 255;
					}
					// PARAMETER
					if( maskvariable & 8 || maskvariable & 128) {
						Word eff, cmd;

						if( maskvariable & 8) {
							eff = ((Word8 *)curDataPat)[0];
							cmd = ((Word8 *)curDataPat)[1];
							curDataPat += 2;
						} else {
							eff = lasteff[channel];
							cmd = lastcmd[channel];
						}
						lasteff[channel] = static_cast<char>(eff);
						lastcmd[channel] = static_cast<char>(cmd);

						if (aCmd != 0L) {
							ConvertITEffect( eff, cmd, &aCmd->cmd, &aCmd->arg);
						}
					}
				}
			}

			if (curDataPat - curITPat->data !=  curITPat->length) {
				return MADUnknowErr;
			}
		}
	}
	Error = 0;
CleanUp:;
	if (Error) {
		MADMusicDestroy(theMAD);
	}
	DeallocAPointer(InsDataArrayPtr);
	DeallocAPointer(SampDataArrayPtr);
	return Error;
}

/**********************************

	Stub to perform the import

**********************************/

int BURGERCALL ModMusicIT(const Word8 *DataPtr,Word32 sndSize,MADMusic *MadFile)
{
	int Result;
	
	Result = MADFileNotSupportedByThisPlug;
	if (((ITHeader_t *)DataPtr)->ID == ITSIG) {
		Result = ConvertIT2Mad(DataPtr,sndSize, MadFile);
	}
	return Result;
}
