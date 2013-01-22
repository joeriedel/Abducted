/**********************************

	Import an S3M file

**********************************/

#include "SnMadMusic.h"
#include <BREndian.hpp>
#include "MmMemory.h"
#include "ClStdLib.h"

#ifdef __BIGENDIAN__
#define S3MSIG 0x5343524D
#else
#define S3MSIG 0x4D524353
#endif

typedef struct S3MPattern_t {
	Word8 note;		// hi = oct, lo = note
	Word8 intru;
	Word8 vol;
	Word8 SpecialCom;
	Word8 ComArg;
} S3MPattern_t;

typedef struct S3MInstrument_t {
	Word8 instype;
	Word8 insdosname[12];
	Word8 memsegh;
	Word16 memsegl;
	Word32 inslength;
	Word32 insloopbeg;
	Word32 insloopend;
	Word8 insvol;
	Word8 insdsk;
	Word8 inspack;
	Word8 insflags;
	Word32 c2spd;
	Word8 inssig2[4];
	Word16 insgvspos;
	Word16 insint512;
	Word32 insintlastused;
	Word8 insname[28];
	Word8 inssig[4];
} S3MInstrument_t;

typedef struct S3MHeader_t {
	char name[28];
	char sig1;
	char type;
	char sig2[2];
	short ordernum;
	short insnum;
	short patnum;
	short flags;
	short cwtv;
	short ffv;
	Word32 s3msig;		/* "SCRM" */
	Word8 mastervol;
	Word8 initialspeed;
	Word8 initialtempo;
	Word8 mastermul;
	char sig3[12];
	Word8 chanset[32];
	Word8 *orders;
	Word16 *parapins;
	Word16 *parappat;
	S3MInstrument_t *insdata;
} S3MHeader_t;

static void ConvertS3MEffect(Word B0,Word B1, Word8 *CmdPtr, Word8 *ArgPtr)
{
	Word LoB1 = B1&0xF;
	Word HiB1 = B1>>4;
	Word Cmd;
	Word Arg;
	
	switch( B0 + 0x40) {
	default:
		Cmd = 0;
		Arg = 0;
		break;
	// Speed
	case 'A':
		Cmd = speedE;
		Arg = B1;
		break;
	// Tempo
	case 'T':
		Cmd = speedE;
		Arg = B1;
		break;

	case 'B':
		Cmd = fastskipE;
		Arg = B1;
		break;

	case 'C':
		Cmd = skipE;
		Arg = B1;
		break;

	case 'D':
		if (!LoB1 || !HiB1)	{	// Slide volume
			Cmd = slidevolE;
			Arg = B1;
		} else if( HiB1 == 0x0F) {		// Fine Slide volume DOWN
			Cmd = extendedE;
			Arg = LoB1 + (11<<4);
		} else if( LoB1 == 0x0F) {		// Fine Slide volume UP
			Cmd = extendedE;
			Arg = HiB1+(10<<4);
		} else {
			Cmd = 0;
			Arg = 0;
		}
		break;

	case 'E':
		if( HiB1 == 0x0F) {		// FineSlide DOWN
			Cmd = extendedE;
			Arg = LoB1+(2 << 4);		//not supported
		} else if( HiB1 == 0x0E) {	// ExtraFineSlide DOWN
			Cmd = 0;
			Arg = 0;		//not supported
		} else {					// Slide DOWN
			Cmd = upslideE;
			Arg = B1;
		}
		break;

	case 'F':
		if( HiB1 == 0x0F) {		// FineSlide UP
			Cmd = extendedE;
			Arg = LoB1+(1 << 4);		//not supported
		} else if( HiB1 == 0x0E) {	// ExtraFineSlide UP
			Cmd = 0;
			Arg = 0;		//not supported
		} else {					// Slide UP
			Cmd = downslideE;
			Arg = B1;
		}
		break;
	case 'G':
		Cmd = portamentoE;
		Arg = B1;
		break;
	case 'H':
		Cmd = vibratoE;
		Arg = B1;
		break;
	case 'J':
		Cmd = arpeggioE;
		Arg = B1;
		break;
	case 'K':
		Cmd = vibratoslideE;
		Arg = B1;
		break;
	case 'L':
		Cmd = portaslideE;
		Arg = B1;
		break;
	case 'O':
		Cmd = offsetE;
		Arg = B1;
		break;

	case 'S':		// Special Effects
		switch( HiB1) {
		default:
			Cmd = 0;
			Arg = 0;
			break;
		case 2:
			Cmd = extendedE;
			Arg = LoB1+(5 << 4);
			break;	// FineTune
		case 3:
			Cmd = extendedE;
			Arg = LoB1+(4 << 4);
			break;	// Set Vibrato WaveForm
		case 4:
			Cmd = extendedE;
			Arg = LoB1+(7 << 4);
			break;	// Set Tremolo WaveForm
		case 0xB:
			Cmd = extendedE;
			Arg = LoB1+(6 << 4);
			break;	// Loop pattern
		case 0xC:
			Cmd = extendedE;
			Arg = LoB1+(12 << 4);
			break;	// Cut sample
		case 0xD:
			Cmd = extendedE;
			Arg = LoB1+(13 << 4);
			break;	// Delay sample
		case 0xE:
			Cmd = extendedE;
			Arg = LoB1+(14 << 4);
			break;	// Delay pattern
		}
		break;
	}
	CmdPtr[0] = (Word8)Cmd;
	ArgPtr[0] = (Word8)Arg;
}

static int ConvertS3M2Mad(const Word8 *theS3M, long /* size */, MADMusic *theMAD)
{
	long i, x, z, channel, Row;
	long starting;
	Word8 *MaxPtr;
	Word8 *theInstrument[ MAXINSTRU];
	Word8 tempChar;
	Word8 *theS3MCopy;
	short Note, Octave, maxTrack;

	/**** Variables pour le MAD ****/
	Cmd_t *aCmd;

	/**** Variables pour le S3M ****/

	S3MHeader_t s3minfo;
	/********************************/

	i = 0;
	do {
		theInstrument[i] = 0L;
	} while (++i<MAXINSTRU);

	/**** Header principal *****/
	theS3MCopy = (Word8*) theS3M;

	FastMemCpy(&s3minfo, theS3MCopy, 96);
	theS3MCopy += 96;

	#if defined(__BIGENDIAN__)
	s3minfo.ordernum = Burger::SwapEndian(s3minfo.ordernum);
	s3minfo.insnum = Burger::SwapEndian(s3minfo.insnum);
	s3minfo.patnum = Burger::SwapEndian(s3minfo.patnum);
	s3minfo.flags = Burger::SwapEndian(s3minfo.flags);
	s3minfo.cwtv = Burger::SwapEndian(s3minfo.cwtv);
	s3minfo.ffv = Burger::SwapEndian(s3minfo.ffv);
	#endif

	/**** Order Num *****/
	s3minfo.orders = (unsigned char *) AllocAPointer( s3minfo.ordernum);
	if( s3minfo.orders == 0L) {
		return MADNeedMemory;
	}
	FastMemCpy(s3minfo.orders, theS3MCopy,s3minfo.ordernum);
	theS3MCopy += s3minfo.ordernum;

	/**** Ins Num *****/
	s3minfo.parapins = (unsigned short *) AllocAPointer( s3minfo.insnum * 2L);
	if( s3minfo.parapins == 0L) {
		return MADNeedMemory;
	}
	FastMemCpy( s3minfo.parapins, theS3MCopy, s3minfo.insnum * 2L);
	theS3MCopy += s3minfo.insnum * 2L;

#if defined(__BIGENDIAN__)
	for( i = 0; i < s3minfo.insnum; i++) {
		s3minfo.parapins[ i] = Burger::SwapEndian(s3minfo.parapins[i]);
	}
#endif

	/**** Pat Num *****/
	s3minfo.parappat = (unsigned short *) AllocAPointer( s3minfo.patnum * 2L);
	if( s3minfo.parappat == 0L) return MADNeedMemory;
	FastMemCpy( s3minfo.parappat,theS3MCopy,  s3minfo.patnum * 2L);
	theS3MCopy += s3minfo.patnum * 2L;
#if defined(__BIGENDIAN__)
	for( i = 0; i < s3minfo.patnum; i++) {
		s3minfo.parappat[ i] = Burger::SwapEndian(s3minfo.parappat[i]);
	}
#endif
	/**** Ins Data ****/
	if( s3minfo.insnum > MAXINSTRU) s3minfo.insnum = MAXINSTRU;
	s3minfo.insdata = (S3MInstrument_t *) AllocAPointer( sizeof(S3MInstrument_t) * s3minfo.insnum);
	if( s3minfo.insdata == 0L) {
		return MADNeedMemory;
	}
	
	for (i = 0; i < s3minfo.insnum; i++) {
		theInstrument[ i] = 0L;

		theS3MCopy = (Word8*) theS3M;
		theS3MCopy += s3minfo.parapins[i]*16L;

		FastMemCpy(&s3minfo.insdata[i], theS3MCopy,  sizeof(S3MInstrument_t));

#if defined(__BIGENDIAN__)
		s3minfo.insdata[i].memsegl = Burger::SwapEndian(s3minfo.insdata[i].memsegl);
		s3minfo.insdata[i].inslength = Burger::SwapEndian(s3minfo.insdata[i].inslength);
#endif
		if( s3minfo.insdata[i].insflags&1) {
#if defined(__BIGENDIAN__)
			s3minfo.insdata[i].insloopbeg = Burger::SwapEndian(s3minfo.insdata[i].insloopbeg);
			s3minfo.insdata[i].insloopend = Burger::SwapEndian(s3minfo.insdata[i].insloopend);
#endif
		} else {
			s3minfo.insdata[i].insloopbeg = 0;
			s3minfo.insdata[i].insloopend = 0;
		}

#if defined(__BIGENDIAN__)
		s3minfo.insdata[i].c2spd = Burger::SwapEndian(s3minfo.insdata[i].c2spd);
		s3minfo.insdata[i].insgvspos = Burger::SwapEndian(s3minfo.insdata[i].insgvspos);
		s3minfo.insdata[i].insint512 = Burger::SwapEndian(s3minfo.insdata[i].insint512);
		s3minfo.insdata[i].insintlastused = Burger::SwapEndian(s3minfo.insdata[i].insintlastused);
#endif

		if (s3minfo.insdata[i].instype == 1 && s3minfo.insdata[i].inspack == 0 &&
			s3minfo.insdata[i].inssig[ 0] == 'S' &&
			s3minfo.insdata[i].inssig[ 1] == 'C' &&
			s3minfo.insdata[i].inssig[ 2] == 'R' &&
			s3minfo.insdata[i].inssig[ 3] == 'S') {
			long tempL;

			theS3MCopy = (Word8*) theS3M;

			tempL = (((long)s3minfo.insdata[i].memsegh)<<16|s3minfo.insdata[i].memsegl)<<4;

			theS3MCopy += tempL;

			theInstrument[ i] = (Word8 *)theS3MCopy;
		} else {
			theInstrument[ i] = 0L;
		}
	}

	/******** Le S3M a ŽtŽ lu et analysŽ ***********/
	/******** Copie des informations dans le MAD ***/

	theMAD->header = (MADSpec*) AllocAPointerClear( sizeof( MADSpec));
	if( theMAD->header == 0L) {
		return MADNeedMemory;
	}
	theMAD->header->MAD = 'MADI';
	i = 0;
	do {
		theMAD->header->name[i] = 0;
	} while (++i<32);
	i = 0;
	do {
		theMAD->header->name[i] = s3minfo.name[i];
	} while (++i<28);

	strcpy( theMAD->header->infos,"Burgerlib Conversion");

	theMAD->header->numPat = (Word8)s3minfo.patnum;
	theMAD->header->numPointers	= (Word8)s3minfo.ordernum;
	theMAD->header->speed = s3minfo.initialspeed;
	theMAD->header->tempo = s3minfo.initialtempo;

	i = 0;
	do {
		theMAD->header->oPointers[ i] = 0;
	} while (++i<128);
	
	for(i=0; i<s3minfo.ordernum; i++) {
		theMAD->header->oPointers[ i] = s3minfo.orders[i];
		if (theMAD->header->oPointers[ i] >= s3minfo.patnum) {
			theMAD->header->oPointers[ i] = 0;
		}
	}

	x = 1;
	i = 0;
	do {
		if( x > 0) {
			theMAD->header->chanPan[ i] = MAX_PANNING/4;
		} else {
			theMAD->header->chanPan[ i] = MAX_PANNING - MAX_PANNING/4;
		}
		x--;
		if( x == -2) {
			x = 2;
		}
		theMAD->header->chanVol[ i] = MAX_VOLUME;
	} while (++i < MAXTRACK);
	
	theMAD->header->generalVol		= 64;
	theMAD->header->generalSpeed	= 80;
	theMAD->header->generalPitch	= 80;

	/************************/
	/***** INSTRUMENTS  *****/
	/************************/

	theMAD->fid = ( InstrData*) AllocAPointerClear( sizeof( InstrData) * (long) MAXINSTRU);
	if( !theMAD->fid) {
		return MADNeedMemory;
	}
	theMAD->sample = ( sData_t**) AllocAPointerClear( sizeof( sData_t*) * (long) MAXINSTRU * (long) MAXSAMPLE);
	if( !theMAD->sample) {
		return MADNeedMemory;
	}
	i = 0;
	do {
		x = 0;
		do {
			theMAD->sample[ i*MAXSAMPLE + x] = 0L;
		} while (++x < MAXSAMPLE);
		theMAD->fid[i].numSamples	= 0;
	} while (++i < MAXINSTRU);

	for(i=0; i<s3minfo.insnum; i++) {
		InstrData		*curIns = &theMAD->fid[ i];

		curIns->type	= 0;

		x = 0;
		do {
			theMAD->fid[i].name[x] = s3minfo.insdata[i].insname[x];
		} while (++x<28);

		if( theInstrument[ i] != 0L) {
			sData_t	*curData;

			curIns->numSamples = 1;
			curIns->volFade = DEFAULT_VOLFADE;

			curData = theMAD->sample[ i*MAXSAMPLE + 0] = (sData_t*) AllocAPointerClear( sizeof( sData_t));
			if( curData == 0L) {
				return MADNeedMemory;
			}
			curData->size		= s3minfo.insdata[i].inslength;
			curData->loopBeg 	= s3minfo.insdata[i].insloopbeg;
			curData->loopSize 	= s3minfo.insdata[i].insloopend - s3minfo.insdata[i].insloopbeg;
			curData->vol		= s3minfo.insdata[i].insvol;
			curData->c2spd		= (Word16)s3minfo.insdata[i].c2spd;
			curData->loopType	= 0;
			curData->amp		= 8;
			if (s3minfo.insdata[i].insflags&4) {
				curData->amp		= 16;
			}
			curData->relNote	= 0;

			if( curData->amp == 16) {
				curData->size *= 2;
				curData->loopBeg *= 2;
				curData->loopSize *= 2;
			}

			curData->data = (char *)AllocAPointer( curData->size);
			if( curData->data == 0L) {
				return MADNeedMemory;
			}
			if( curData->data != 0L) {
				FastMemCpy( curData->data,  theInstrument [i],curData->size);

				switch( curData->amp) {
				case 16:
					{
						short *b16 = (short*) curData->data;
						long temp;

						for( temp = 0; temp < curData->size/2; temp++) {
#if defined(__BIGENDIAN__)
							b16[temp] = Burger::SwapEndian(b16[temp]);
#endif
							if( s3minfo.ffv != 1) {
								*(b16 + temp) -= (short)0x8000;
							}
						}
					}
					break;

				case 8:
					if( s3minfo.ffv != 1) {
						long temp;

						for( temp = 0; temp < curData->size; temp++) {
							*(curData->data + temp) -= (char)0x80;
						}
					}
					break;
				}
			}
		} else {
			curIns->numSamples = 0;
		}
	}

	i = 0;
	do {
		theMAD->fid[ i].firstSample = (short)(i * MAXSAMPLE);
	} while (++i < MAXINSTRU);
	
	/********************/

	/*********************/
	/*           Check MaxTrack         */
	/*********************/

	maxTrack = 0;
	i = 0;
	do {
		if (s3minfo.chanset[ i] < 32) {
			maxTrack++;
		}
	} while (++i<32);
	maxTrack++;
	maxTrack /= 2;
	maxTrack *= 2;

	/********************/
	/***** TEMPORAIRE ******/
	/********************/

	theMAD->header->numChn = (Word8)maxTrack;

	starting = 0;

	i = 0;
	do {
		theMAD->partition[ i] = 0L;
	} while (++i < MAXPATTERN);
	
	for( i = 0; i < theMAD->header->numPat ; i++) {
		theMAD->partition[ i] = (PatData_t*) AllocAPointerClear( sizeof( PatHeader_t) + theMAD->header->numChn * 64L * sizeof( Cmd_t));
		if( theMAD->partition[ i] == 0L) {
			return MADNeedMemory;
		}
		theMAD->partition[ i]->header.size = 64L;
		theMAD->partition[ i]->header.compMode = 'NONE';

		x = 0;
		do {
			theMAD->partition[ i]->header.name[ x] = 0;
		} while (++x<20);
		theMAD->partition[ i]->header.patBytes = 0L;
		theMAD->partition[ i]->header.unused2 = 0L;

		MaxPtr = (Word8 *) theMAD->partition[ i];
		MaxPtr += sizeof( PatHeader_t) + theMAD->header->numChn * 64L * sizeof( Cmd_t);

		Row = 0;
		do {
			for(z = 0; z < theMAD->header->numChn; z++) {
				aCmd = GetMADCommand( Row, z, theMAD->partition[ i]);
				aCmd->note = 0xFF;
				aCmd->ins = 0;
				aCmd->cmd = 0;
				aCmd->arg = 0;
				aCmd->vol = 0xFF;
			}
		} while (++Row<64);

		if( s3minfo.parappat[ i] > 0) {
			theS3MCopy = (Word8*) theS3M;
			theS3MCopy += ( (long) s3minfo.parappat[i] )*16L;
			theS3MCopy+=2;

			Row = 0;
			do {
				/*
					BYTE:flag, 	0		=	end of row
								&31		=	channel
								&32		=	follows;  BYTE:note, BYTE:instrument
								&64		=	follows;  BYTE:volume
								&128	=	follows; BYTE:command, BYTE:info
				*/

				tempChar = *theS3MCopy;
				theS3MCopy++;

				if (tempChar == 0) {
					Row++;
				} else {
					// Channel

					channel = tempChar&0x1F;
					if( channel >= 0 && channel < theMAD->header->numChn) {
						aCmd = GetMADCommand( Row, channel, theMAD->partition[ i]);
					} else {
						aCmd = 0L;
					}
					// PERIOD

					if (tempChar & 32) {
						if (aCmd) {
							Word Note2;
							Note2 = theS3MCopy[ 0];
							Octave = static_cast<short>((Note2 & 0xF0) >> 4);
							Note = static_cast<short>(Note2 & 0x0F);
							Note2 = Octave*12 + Note;
							if (Note2 >= NUMBER_NOTES) {
								Note2 = 0xFF;
							}
							aCmd->note = (Word8)Note2;
							aCmd->ins = theS3MCopy[ 1];
						}
						theS3MCopy += 2L;
					}

					// VOLUME

					if (tempChar & 64) {
						if (aCmd) {
							Word Vol;
							Vol = theS3MCopy[ 0];
							if( Vol > 64) {
								Vol = 64;
							}
							aCmd->vol = (Word8)(Vol+0x10);
						}
						theS3MCopy += 1L;
					} else {
						if (aCmd) {
							aCmd->vol = 255;
						}
					}
						// PARAMETER
					if (tempChar & 128) {
						if (aCmd) {
							if (theS3MCopy[ 0] != 255) {
								ConvertS3MEffect( theS3MCopy[ 0], theS3MCopy[ 1], &aCmd->cmd, &aCmd->arg);
							}
						}
						theS3MCopy += 2L;
					}
				}
			} while (Row<64);
		}
	}
	DeallocAPointer(s3minfo.orders);
	DeallocAPointer(s3minfo.parapins);
	DeallocAPointer(s3minfo.parappat);
	DeallocAPointer(s3minfo.insdata);
	return 0;
}

/**********************************

	Import an S3M file

**********************************/

int BURGERCALL ModMusicS3M(const Word8 *AlienFile,Word32 sndSize,MADMusic *MadFile)
{
	int Result;
	Result = MADFileNotSupportedByThisPlug;		/* Not supported */
	if (((S3MHeader_t*)AlienFile)->s3msig == S3MSIG) {
		Result = ConvertS3M2Mad(AlienFile,sndSize,MadFile);
	}
	return Result;		
}
