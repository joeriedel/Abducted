#include "SnSound.h"
#include <BREndian.hpp>
#include "ClStdLib.h"
#include "PkPack.h"
#include "MmMemory.h"
#include "PfPrefs.h"
#include "PkVorbisCodec.h"
#include "RsReadStream.h"

#if defined(__LITTLEENDIAN__)
#define SOUNDTYPESHORT SOUNDTYPELSHORT
#else
#define SOUNDTYPESHORT SOUNDTYPEBSHORT
#endif

#define OGGBUFFERSIZE 8192

typedef struct OggState_t {
	ogg_sync_state OggSyncState;		/* sync and verify incoming physical bitstream */
	ogg_stream_state OggStreamState;	/* take physical pages, weld into a logical stream of packets */
	ogg_page OggPage;					/* one Ogg bitstream page.  Vorbis packets are inside */
	ogg_packet OggPacket;				/* one raw packet of data for decode */
	vorbis_info VorbisInfo;				/* struct that stores all the static vorbis bitstream settings */
	vorbis_comment VorbisComment;		/* struct that stores all the bitstream user comments */
	vorbis_dsp_state VorbisDspState;	/* central working state for the packet->PCM decoder */
	vorbis_block VorbisBlock;			/* local working space for packet->PCM decode */
	int convsize;						/* Number of samples to decompress */
	Word State;							/* Decode state */
	int Tempi;
	Word8 LoadBuffer[OGGBUFFERSIZE];		/* Data buffer for loading */
} OggState_t;

typedef struct MaceCodec_t {
	MACEState_t MaceData[2];		/* Decompression states for left/right channels */
	Word LeftOver;					/* TRUE if OldPack is valid */
	Word Stale;						/* Number of uncompressed bytes left */
	Word8 *StalePtr;					/* Pointer to the uncompressed bytes */
	Word8 DecodeBuffer[4096];		/* Buffer to decode to */
	Word8 StaleBuffer[8];			/* Excess decompression buffer */
} MaceCodec_t;

typedef struct ADPCMCodec_t {
	Word LeftOver;					/* TRUE if OldPack is valid */
	Word Stale;						/* Number of uncompressed bytes left */
	Word8 *StalePtr;					/* Pointer to the uncompressed bytes */
	Word8 *DecodeBuffer;				/* Buffer to decode to */
	Word8 *StaleBuffer;				/* Buffer for partial reads */
	Word DecodeBufferSize;			/* Size of the decode buffer */
	Word PackChunkSize;				/* Size of a packed buffer */
	Word SampleCount;				/* Number of samples to decompress */
	Word UnpackChunkSize;			/* Size of decompressed data */
} ADPCMCodec_t;

/**********************************

	Codec to handle raw 8 bit unsigned data

**********************************/

Word32 BURGERCALL DigitalMusicByte(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length)
{
	Word32 ByteResult;
	Word32 MaxLength;
	
	/* Init the codec */

	if (Command==MUSICCODECINIT) {
		ByteResult = SOUNDTYPEBYTE;
	/* How much data to load in */

	} else if (Command==MUSICCODECDECODE) {
		MaxLength = Input->SoundLength-Input->BytesPlayed;
		if (!MaxLength) {
			ByteResult = (Word32)-1;
		} else {
			if (Length>MaxLength) {
				Length = MaxLength;
			}
			ByteResult = Input->ReadProc(Input->CallBackParm,DestPtr,Length);
			if (ByteResult!=(Word32)-1) {		/* Oh oh... */
				Input->BytesPlayed += ByteResult;
			}
		}
	} else {
		ByteResult = 0;
	}
	return ByteResult;
}

/**********************************

	Codec to handle raw 8 bit signed data

**********************************/

Word32 BURGERCALL DigitalMusicChar(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length)
{
	Word32 CharResult;
	Word32 MaxLength;
	
	/* Init the codec */

	if (Command==MUSICCODECINIT) {
		CharResult = SOUNDTYPEBYTE;

	/* How much data to load in */

	} else if (Command==MUSICCODECDECODE) {
		MaxLength = Input->SoundLength-Input->BytesPlayed;
		if (!MaxLength) {
			CharResult = (Word32)-1;
		} else {
			if (Length>MaxLength) {
				Length = MaxLength;
			}
			CharResult = Input->ReadProc(Input->CallBackParm,DestPtr,Length);
			if (CharResult!=(Word32)-1) {		/* Oh oh... */
				Word32 Counter;
				Input->BytesPlayed += CharResult;
				Counter = CharResult>>2;
				if (Counter) {
					do {
						((Word32*)DestPtr)[0] = ((Word32 *)DestPtr)[0] ^ 0x80808080UL;
						DestPtr+=4;
					} while (--Counter);
				}
				Counter = CharResult&3;
				if (Counter) {
					do {
						DestPtr[0] = DestPtr[0] ^ 0x80;
						++DestPtr;
					} while (--Counter);
				}
			}
		}
	} else {
		CharResult = 0;
	}
	return CharResult;
}

/**********************************

	Codec to handle ULaw data

**********************************/

Word32 BURGERCALL DigitalMusicULaw(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length)
{
	Word32 ULawResult;
	Word32 MaxLength;
	Word8 Buffer[4096];
	
	switch (Command) {

	/* Init the codec */
	
	case MUSICCODECINIT:
		ULawResult = SOUNDTYPESHORT;
		break;
	
	/* Dispose of the codec */

/*	case MUSICCODECDESTROY: */
/*	case MUSICCODECRESET: */
	default:
		ULawResult = 0;
		break;

	/* How much data to load in */

	case MUSICCODECDECODE:
		MaxLength = Input->SoundLength-Input->BytesPlayed;
		if (MaxLength<2) {			/* Ignore 0 or 1 */
			ULawResult = (Word32)-1;
		} else {
			ULawResult = 0;
			if (Length>=2) {
				Length &= (~1);				/* Must be an even run */
				if (Length>MaxLength) {
					Length = MaxLength;
				}
				do {
					Word32 Temp;
					Word32 ChunkSize;
					const short *PackTable;
					Word8 *SrcPtr;
				
					ChunkSize = Length>>1;
					if (ChunkSize>sizeof(Buffer)) {
						ChunkSize = sizeof(Buffer);
					}
					Temp = Input->ReadProc(Input->CallBackParm,Buffer,ChunkSize);
					if (!Temp) {
						break;
					}
					if (Temp==(Word32)-1) {		/* Oh oh... */
						Input->BytesPlayed = Input->SoundLength;
						return ULawResult;
					}
						
					ChunkSize = Temp*2;		/* Convert to shorts */
					ULawResult += ChunkSize;	/* Add to output */
					Length -= ChunkSize;	/* Remove remaining */
					PackTable = UnpackULawTable;	/* Local pointer */
					SrcPtr = Buffer;
					do {
						((short *)DestPtr)[0] = PackTable[SrcPtr[0]];
						++SrcPtr;
						DestPtr+=2;
					} while (--Temp);
				} while (Length);
				Input->BytesPlayed += ULawResult;
			}
		}
		break;
	}
	return ULawResult;
}

/**********************************

	Codec to handle ALaw data
	Identical to above, except using ALaw table

**********************************/

Word32 BURGERCALL DigitalMusicALaw(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length)
{
	Word32 Result;
	Word32 MaxLength;
	Word8 Buffer[4096];
	
	switch (Command) {

	/* Init the codec */
	
	case MUSICCODECINIT:
		Result = SOUNDTYPESHORT;
		break;
	
	/* Dispose of the codec */

/*	case MUSICCODECDESTROY: */
/*	case MUSICCODECRESET: */
	default:
		Result = 0;
		break;

	/* How much data to load in */

	case MUSICCODECDECODE:
		MaxLength = Input->SoundLength-Input->BytesPlayed;
		if (MaxLength<2) {			/* Ignore 0 or 1 */
			Result = (Word32)-1;
		} else {
			Result = 0;
			if (Length>=2) {
				Length &= (~1);				/* Must be an even run */
				if (Length>MaxLength) {
					Length = MaxLength;
				}
				do {
					Word32 Temp;
					Word32 ChunkSize;
					const short *PackTable;
					Word8 *SrcPtr;
				
					ChunkSize = Length>>1;
					if (ChunkSize>sizeof(Buffer)) {
						ChunkSize = sizeof(Buffer);
					}
					Temp = Input->ReadProc(Input->CallBackParm,Buffer,ChunkSize);
					if (!Temp) {
						break;
					}
					if (Temp==(Word32)-1) {		/* Oh oh... */
						Input->BytesPlayed = Input->SoundLength;
						return Result;
					}
						
					ChunkSize = Temp*2;		/* Convert to shorts */
					Result += ChunkSize;	/* Add to output */
					Length -= ChunkSize;	/* Remove remaining */
					PackTable = UnpackALawTable;	/* Local pointer */
					SrcPtr = Buffer;
					do {
						((short *)DestPtr)[0] = PackTable[SrcPtr[0]];
						++SrcPtr;
						DestPtr+=2;
					} while (--Temp);
				} while (Length);
				Input->BytesPlayed += Result;
			}
		}
		break;
	}
	return Result;
}

/**********************************

	Codec to handle little endian 16 bit data

**********************************/

Word32 BURGERCALL DigitalMusicLShort(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length)
{
	Word32 Result;
	Word32 MaxLength;
	
	switch (Command) {

	/* Init the codec */
	
	case MUSICCODECINIT:
		Result = SOUNDTYPESHORT;
		break;
	
	/* Dispose of the codec */

/*	case MUSICCODECDESTROY: */
/*	case MUSICCODECRESET: */
	default:
		Result = 0;
		break;

	/* How much data to load in */

	case MUSICCODECDECODE:
		MaxLength = Input->SoundLength-Input->BytesPlayed;
		if (MaxLength<2) {			/* Ignore 0 or 1 */
			Result = (Word32)-1;
		} else {
			Result = 0;
			if (Length>=2) {
				Length &= (~1);				/* Must be an even run */
				if (Length>MaxLength) {
					Length = MaxLength;
				}
				Result = Input->ReadProc(Input->CallBackParm,DestPtr,Length);
				if (Result!=(Word32)-1) {
					Input->BytesPlayed += Result;
#if defined(__BIGENDIAN__)
					{
						Word Temp;
						Temp = Result>>1;
						if (Temp) {
							do {
#if defined(__POWERPC__)
								((Word16 *)DestPtr)[0] = (Word16)__lhbrx((Word16 *)DestPtr,0);
#else
								((Word16 *)DestPtr)[0] = Burger::SwapEndian(((Word16 *)DestPtr)[0]);
#endif
								DestPtr+=2;
							} while (--Temp);
						}
					}
#endif
				}
			}
		}
		break;
	}
	return Result;
}

/**********************************

	Codec to handle big endian 16 bit data

**********************************/

Word32 BURGERCALL DigitalMusicBShort(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length)
{
	Word32 Result;
	Word32 MaxLength;
	
	switch (Command) {

	/* Init the codec */
	
	case MUSICCODECINIT:
		Result = SOUNDTYPESHORT;
		break;
	
	/* Dispose of the codec */

/*	case MUSICCODECDESTROY: */
/*	case MUSICCODECRESET: */
	default:
		Result = 0;
		break;

	/* How much data to load in */

	case MUSICCODECDECODE:
		MaxLength = Input->SoundLength-Input->BytesPlayed;
		if (MaxLength<2) {			/* Ignore 0 or 1 */
			Result = (Word32)-1;
		} else {
			Result = 0;
			if (Length>=2) {
				Length &= (~1);				/* Must be an even run */
				if (Length>MaxLength) {
					Length = MaxLength;
				}
				Result = Input->ReadProc(Input->CallBackParm,DestPtr,Length);
				if (Result!=(Word32)-1) {
					Input->BytesPlayed += Result;
#if defined(__LITTLEENDIAN__)
					{
						Word Temp;
						Temp = Result>>1;
						if (Temp) {
							do {
#if defined(__POWERPC__)
								((Word16 *)DestPtr)[0] = (Word16)__lhbrx((Word16 *)DestPtr,0);
#else
								((Word16 *)DestPtr)[0] = Burger::SwapEndian(((Word16 *)DestPtr)[0]);
#endif
								DestPtr+=2;
							} while (--Temp);
						}
					}
#endif
				}
			}
		}
		break;
	}
	return Result;
}

/**********************************

	Codec to handle Mace 3:1 compressed data
	2 bytes decompress to 6

**********************************/

Word32 BURGERCALL DigitalMusicMace3(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length)
{
	Word32 Result;
	Word32 MaxLength;
	
	switch (Command) {

	/* Init the codec */
	
	case MUSICCODECINIT:
		Input->CompressStatePtr = AllocAPointerClear(sizeof(MaceCodec_t));
		Result = SOUNDTYPEBYTE;
		break;
	
	case MUSICCODECRESET:
		if (Input->CompressStatePtr) {
			FastMemSet(Input->CompressStatePtr,0,sizeof(MaceCodec_t));
		}
	/* Dispose of the codec */

/*	case MUSICCODECDESTROY: */
	default:
		Result = 0;
		break;

	/* How much data to load in */

	case MUSICCODECDECODE:
		{
			MaceCodec_t *CodecPtr;
			
			/* All done? */
			
			MaxLength = Input->SoundLength-Input->BytesPlayed;
			if (!MaxLength) {
				Result = (Word32)-1;			/* No more from now on! */
			} else {

				/* Data from a previous decode? */

				CodecPtr = (MaceCodec_t *)Input->CompressStatePtr;

				if (Length>MaxLength) {
					Length = MaxLength;		/* Don't go too far */
				}
				
				Result = 0;					/* No bytes decompressed yet */

				MaxLength = CodecPtr->Stale;
				if (MaxLength) {					/*  Check */
					if (MaxLength>=Length) {		/* More than enough? */
						FastMemCpy(DestPtr,CodecPtr->StalePtr,Length);
						CodecPtr->Stale=MaxLength-Length;
						CodecPtr->StalePtr+=Length;	/* Accept the data */
						Input->BytesPlayed+=Length;
						return Length;
					}
					/* Use up the stale buffer */
					
					Result = MaxLength;
					FastMemCpy(DestPtr,CodecPtr->StalePtr,MaxLength);
					DestPtr+=MaxLength;
					Length-=MaxLength;			/* Remove the copies bytes */
					CodecPtr->Stale = 0;		/* Zap the stale buffer */
				}
						
				/* Let's decompress shall we? */
				
				/* Anything requested? */
				
				if (Length) {
					do {
						Word PackNum;
						Word32 InputLength;
						Word Pass1;
						Word LeftOver;
						Word Packets;
						Word8 *DecodePtr;
						
						/* Figure out how much packed data to load */
						
						PackNum = Length/6;		/* How many packets to decompress? */
						Pass1 = PackNum;
						if ((PackNum*6)!=Length) {	/* Overflow? */
							++PackNum;			/* How many packets to load */
						}
						
						/* Now I have the number of bytes I want to read from the file. */
						/* Do I actually have this data to read? */

						InputLength = PackNum*2;		/* Number of bytes I want */
						
						/* Check for buffer overflow */
						
						if (InputLength>=sizeof(CodecPtr->DecodeBuffer)) {
							InputLength = sizeof(CodecPtr->DecodeBuffer);
						}
					
						/* Stale packed data from a previous load? */
						
						LeftOver = CodecPtr->LeftOver;
						InputLength = Input->ReadProc(Input->CallBackParm,&CodecPtr->DecodeBuffer[LeftOver],InputLength-LeftOver);
						if (!InputLength) {
							break;
						}
						if (InputLength==(Word32)-1) {
							Input->BytesPlayed = Input->SoundLength;
							return Result;
						}
						InputLength += LeftOver;		/* Here is the REAL data size */
						CodecPtr->LeftOver = 0;			/* Force acceptance */

						/* At this point, I get wierd */
						/* You see, I must decode up to "Length" bytes but not */
						/* over, but I have to decompress in packets of 6 */
						
						Packets = InputLength>>1;		/* Number of packets I loaded */
						if (Pass1>Packets) {			/* Did I load enough? */
							Pass1 = Packets;
						}
						DecodePtr = CodecPtr->DecodeBuffer;
						if (Pass1) {					/* Shall I decode? */
							MACEExp1to3(DecodePtr,DestPtr,Pass1,&CodecPtr->MaceData[0],&CodecPtr->MaceData[0],1,1);
							PackNum = Pass1*2;

							InputLength -= PackNum;		/* Accept the data */
							DecodePtr += PackNum;
							PackNum = Pass1*6;			/* Number of bytes decoded */
							Length -= PackNum;			/* Remove from length */
							DestPtr += PackNum;			/* Adjust pointer */
							Result += PackNum;			/* Adjust result */
						}
						
						/* Wrap up now? */
						
						if (Length<6 && Length && InputLength>=2) {
							Result += Length;
							MACEExp1to3(DecodePtr,CodecPtr->StaleBuffer,1,&CodecPtr->MaceData[0],&CodecPtr->MaceData[0],1,1);
							InputLength-=2;
							DecodePtr+=2;
							FastMemCpy(DestPtr,CodecPtr->StaleBuffer,Length);
							CodecPtr->StalePtr = CodecPtr->StaleBuffer+Length;
							CodecPtr->Stale = 6-Length;
							Length = 0;
						}
						
						/* Anything left in the decode buffer? */
						
						if (InputLength) {
							CodecPtr->LeftOver = InputLength;
							FastMemCpy(CodecPtr->DecodeBuffer,DecodePtr,InputLength);
						}
					} while (Length);		/* Continue? */
				}
				Input->BytesPlayed+=Result;
			}
		}
		break;
	}
	return Result;
}

/**********************************

	Codec to handle Mace 6:1 compressed data
	1 bytes decompress to 6

**********************************/

Word32 BURGERCALL DigitalMusicMace6(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length)
{
	Word32 Result;
	Word32 MaxLength;
	
	switch (Command) {

	/* Init the codec */
	
	case MUSICCODECINIT:
		Input->CompressStatePtr = AllocAPointerClear(sizeof(MaceCodec_t));
		Result = SOUNDTYPEBYTE;
		break;
	
	case MUSICCODECRESET:
		if (Input->CompressStatePtr) {
			FastMemSet(Input->CompressStatePtr,0,sizeof(MaceCodec_t));
		}
	/* Dispose of the codec */

/*	case MUSICCODECDESTROY: */
	default:
		Result = 0;
		break;

	/* How much data to load in */

	case MUSICCODECDECODE:
		{
			MaceCodec_t *CodecPtr;
			
			/* All done? */
			
			MaxLength = Input->SoundLength-Input->BytesPlayed;
			if (!MaxLength) {
				Result = (Word32)-1;			/* No more from now on! */
			} else {

				/* Data from a previous decode? */

				CodecPtr = (MaceCodec_t *)Input->CompressStatePtr;

				if (Length>MaxLength) {
					Length = MaxLength;		/* Don't go too far */
				}
				
				Result = 0;					/* No bytes decompressed yet */

				MaxLength = CodecPtr->Stale;
				if (MaxLength) {					/*  Check */
					if (MaxLength>=Length) {		/* More than enough? */
						FastMemCpy(DestPtr,CodecPtr->StalePtr,Length);
						CodecPtr->Stale=MaxLength-Length;
						CodecPtr->StalePtr+=Length;	/* Accept the data */
						Input->BytesPlayed+=Length;
						return Length;
					}
					/* Use up the stale buffer */
					
					Result = MaxLength;
					FastMemCpy(DestPtr,CodecPtr->StalePtr,MaxLength);
					DestPtr+=MaxLength;
					Length-=MaxLength;			/* Remove the copies bytes */
					CodecPtr->Stale = 0;		/* Zap the stale buffer */
				}
						
				/* Let's decompress shall we? */
				
				/* Anything requested? */
				
				if (Length) {
					do {
						Word PackNum;
						Word32 InputLength;
						Word Pass1;
						Word LeftOver;
						Word Packets;
						Word8 *DecodePtr;
						
						/* Figure out how much packed data to load */
						
						PackNum = Length/6;		/* How many packets to decompress? */
						Pass1 = PackNum;
						if ((PackNum*6)!=Length) {	/* Overflow? */
							++PackNum;			/* How many packets to load */
						}
						
						/* Now I have the number of bytes I want to read from the file. */
						/* Do I actually have this data to read? */

						InputLength = PackNum;			/* Number of bytes I want */
						
						/* Check for buffer overflow */
						
						if (InputLength>=sizeof(CodecPtr->DecodeBuffer)) {
							InputLength = sizeof(CodecPtr->DecodeBuffer);
						}
					
						/* Stale packed data from a previous load? */
						
						LeftOver = CodecPtr->LeftOver;
						InputLength = Input->ReadProc(Input->CallBackParm,&CodecPtr->DecodeBuffer[LeftOver],InputLength-LeftOver);
						if (!InputLength) {
							break;
						}
						if (InputLength==(Word32)-1) {
							Input->BytesPlayed = Input->SoundLength;
							return Result;
						}
						InputLength += LeftOver;		/* Here is the REAL data size */
						CodecPtr->LeftOver = 0;			/* Force acceptance */

						/* At this point, I get wierd */
						/* You see, I must decode up to "Length" bytes but not */
						/* over, but I have to decompress in packets of 6 */
						
						Packets = InputLength;			/* Number of packets I loaded */
						if (Pass1>Packets) {			/* Did I load enough? */
							Pass1 = Packets;
						}
						DecodePtr = CodecPtr->DecodeBuffer;
						if (Pass1) {					/* Shall I decode? */
							MACEExp1to6(DecodePtr,DestPtr,Pass1,&CodecPtr->MaceData[0],&CodecPtr->MaceData[0],1,1);
							PackNum = Pass1;

							InputLength -= PackNum;		/* Accept the data */
							DecodePtr += PackNum;
							PackNum = Pass1*6;			/* Number of bytes decoded */
							Length -= PackNum;			/* Remove from length */
							DestPtr += PackNum;			/* Adjust pointer */
							Result += PackNum;			/* Adjust result */
						}
						
						/* Wrap up now? */
						
						if (Length<6 && Length && InputLength>=1) {
							Result += Length;
							MACEExp1to6(DecodePtr,CodecPtr->StaleBuffer,1,&CodecPtr->MaceData[0],&CodecPtr->MaceData[0],1,1);
							InputLength-=1;
							DecodePtr+=1;
							FastMemCpy(DestPtr,CodecPtr->StaleBuffer,Length);
							CodecPtr->StalePtr = CodecPtr->StaleBuffer+Length;
							CodecPtr->Stale = 6-Length;
							Length = 0;
						}
						
						/* Anything left in the decode buffer? */
						
						if (InputLength) {
							CodecPtr->LeftOver = InputLength;
							FastMemCpy(CodecPtr->DecodeBuffer,DecodePtr,InputLength);
						}
					} while (Length);		/* Continue? */
				}
				Input->BytesPlayed+=Result;
			}
		}
		break;
	}
	return Result;
}

/**********************************

	Codec to handle ADPCM compressed data
	4 bits to a 16 bit short (4:1 compression)
	
**********************************/

Word32 BURGERCALL DigitalMusicADPCM(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length)
{
	Word32 Result;
	Word32 MaxLength;
	ADPCMCodec_t *CodecPtr;
	
	switch (Command) {

	/* Init the codec */
	
	case MUSICCODECINIT:
		{
			RawSound_t *RawPtr;
			Word Offset1,DecodeBufferSize;
			
			RawPtr = (RawSound_t *)DestPtr;
			Offset1 = (RawPtr->Extra2*2);
			DecodeBufferSize = RawPtr->Extra3;
			if (RawPtr->DataType&SOUNDTYPESTEREO) {
				Offset1 *= 2;
				DecodeBufferSize *= 2;
			}
			if (DecodeBufferSize<8192) {
				DecodeBufferSize = 8192;		/* Let's be generous! */
			}
			
			CodecPtr = static_cast<ADPCMCodec_t *>(AllocAPointerClear(sizeof(ADPCMCodec_t)+(Offset1+DecodeBufferSize)));
			if (CodecPtr) {
				CodecPtr->DecodeBufferSize = DecodeBufferSize;
				Input->CompressStatePtr = CodecPtr;
				CodecPtr->PackChunkSize = RawPtr->Extra3;	/* Size of compressed packets */
				CodecPtr->SampleCount = RawPtr->Extra2;		/* Samples par chunk */
				CodecPtr->UnpackChunkSize = RawPtr->Extra2*2;
				if (RawPtr->DataType&SOUNDTYPESTEREO) {
					CodecPtr->UnpackChunkSize *=2;
				}
				CodecPtr->StaleBuffer = ((Word8 *)CodecPtr)+sizeof(ADPCMCodec_t);
				CodecPtr->DecodeBuffer = &CodecPtr->StaleBuffer[Offset1];
			}
		}
		Result = SOUNDTYPESHORT;
		break;
	
	case MUSICCODECRESET:
		CodecPtr = (ADPCMCodec_t *)Input->CompressStatePtr;
		if (CodecPtr) {
			CodecPtr->LeftOver = 0;
			CodecPtr->Stale = 0;
			CodecPtr->StalePtr = 0;
		}
	/* Dispose of the codec */

/*	case MUSICCODECDESTROY: */
	default:
		Result = 0;
		break;

	/* How much data to load in */

	case MUSICCODECDECODE:
		{			
			/* All done? */
			
			MaxLength = Input->SoundLength-Input->BytesPlayed;
			if (!MaxLength) {
				Result = (Word32)-1;			/* No more from now on! */
			} else {

				/* Data from a previous decode? */

				CodecPtr = (ADPCMCodec_t *)Input->CompressStatePtr;

				if (Length>MaxLength) {
					Length = MaxLength;		/* Don't go too far */
				}
				
				Result = 0;					/* No bytes decompressed yet */

				MaxLength = CodecPtr->Stale;
				if (MaxLength) {					/*  Check */
					if (MaxLength>=Length) {		/* More than enough? */
						FastMemCpy(DestPtr,CodecPtr->StalePtr,Length);
						CodecPtr->Stale=MaxLength-Length;
						CodecPtr->StalePtr+=Length;	/* Accept the data */
						Input->BytesPlayed+=Length;
						return Length;
					}
					/* Use up the stale buffer */
					
					Result = MaxLength;
					FastMemCpy(DestPtr,CodecPtr->StalePtr,MaxLength);
					DestPtr+=MaxLength;
					Length-=MaxLength;			/* Remove the copies bytes */
					CodecPtr->Stale = 0;		/* Zap the stale buffer */
				}
						
				/* Let's decompress shall we? */
				
				/* Anything requested? */
				
				if (Length) {
					do {
						ADPCMUnpackState_t Pack;
						Word PackNum;
						Word32 InputLength;
						Word Pass1;
						Word LeftOver;
						Word Packets;
						Word8 *DecodePtr;
						
						Pass1 = 1;
						if (Input->DataType&SOUNDTYPESTEREO) {	/* Stereo? */
							Pass1 = 2;
						}
						Pack.Channels = Pass1;		/* 1 or 2 channels */
						Pack.BlockSize = CodecPtr->PackChunkSize;
						Pack.SamplesPerBlock = CodecPtr->SampleCount;

						/* Figure out how much packed data to load */
						
						PackNum = Length/CodecPtr->UnpackChunkSize;		/* How many packets to decompress? */
						Pass1 = PackNum;
						if ((PackNum*CodecPtr->UnpackChunkSize)!=Length) {	/* Overflow? */
							++PackNum;			/* How many packets to load */
						}
						
						/* Now I have the number of bytes I want to read from the file. */
						/* Do I actually have this data to read? */

						InputLength = PackNum*CodecPtr->PackChunkSize;			/* Number of bytes I want */
						
						/* Check for buffer overflow */
						
						if (InputLength>= CodecPtr->DecodeBufferSize) {
							InputLength = CodecPtr->DecodeBufferSize;
						}
					
						/* Stale packed data from a previous load? */
						
						LeftOver = CodecPtr->LeftOver;
						InputLength = Input->ReadProc(Input->CallBackParm,&CodecPtr->DecodeBuffer[LeftOver],InputLength-LeftOver);
						if (!InputLength) {
							break;
						}
						if (InputLength==(Word32)-1) {
							Input->BytesPlayed = Input->SoundLength;
							return Result;
						}
						InputLength += LeftOver;		/* Here is the REAL data size */
						CodecPtr->LeftOver = 0;			/* Force acceptance */

						/* At this point, I get wierd */
						/* You see, I must decode up to "Length" bytes but not */
						/* over, but I have to decompress in packets of 6 */
						
						Packets = InputLength/CodecPtr->PackChunkSize;		/* Number of packets I loaded */
						if (Pass1>Packets) {			/* Did I load enough? */
							Pass1 = Packets;
						}
						DecodePtr = CodecPtr->DecodeBuffer;
						if (Pass1) {
							Word i;
							Pack.SrcPtr = DecodePtr;
							Pack.SrcLength = CodecPtr->PackChunkSize*Pass1;
							Pack.OutputPtr = (short *)DestPtr;
							InputLength -= Pack.SrcLength;
							i = Pass1;
							do {
								Word Temp2;
								Temp2 = ADPCMDecodeBlock(&Pack);
								Result+=Temp2;
	 							Length -= Temp2;
								DestPtr+=Temp2;
							} while (--i);
							DecodePtr = Pack.SrcPtr;
						}
						
						/* Wrap up now? */
						
						if (Length<CodecPtr->UnpackChunkSize && Length && 
							(InputLength>=CodecPtr->PackChunkSize)) {
							Word Chunk;
							
							Chunk = CodecPtr->PackChunkSize;
							Pack.SrcPtr = DecodePtr;
							Pack.SrcLength = Chunk;
							Pack.OutputPtr = (short *)(CodecPtr->StaleBuffer);
							ADPCMDecodeBlock(&Pack);

							Result += Length;
							InputLength-=Chunk;
							DecodePtr+=Chunk;
							FastMemCpy(DestPtr,CodecPtr->StaleBuffer,Length);
							CodecPtr->StalePtr = CodecPtr->StaleBuffer+Length;
							CodecPtr->Stale = CodecPtr->UnpackChunkSize-Length;
							Length = 0;
						}
						
						/* Anything left in the decode buffer? */
						
						if (InputLength) {
							CodecPtr->LeftOver = InputLength;
							FastMemCpy(CodecPtr->DecodeBuffer,DecodePtr,InputLength);
						}
					} while (Length);		/* Continue? */
				}
				Input->BytesPlayed+=Result;
			}
		}
		break;
	}
	return Result;
}

/**********************************

	Codec to handle Ogg/Vorbis

**********************************/

Word32 BURGERCALL DigitalMusicOgg(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length)
{
	Word32 Result;
	Word32 MaxLength;
	OggState_t *OggPtr;
	
	switch (Command) {

	/* Init the codec */
	
	case MUSICCODECINIT:
		Input->CompressStatePtr = AllocAPointerClear(sizeof(OggState_t));
		Result = SOUNDTYPESHORT;
		break;
	
	/* Reset the codec */
	
	case MUSICCODECRESET:
		OggPtr = (OggState_t *)Input->CompressStatePtr;
		if (OggPtr) {
			ogg_stream_clear(&OggPtr->OggStreamState);
			vorbis_block_clear(&OggPtr->VorbisBlock);
			vorbis_dsp_clear(&OggPtr->VorbisDspState);
			vorbis_comment_clear(&OggPtr->VorbisComment);
			vorbis_info_clear(&OggPtr->VorbisInfo);  /* must be called last */
			ogg_sync_clear(&OggPtr->OggSyncState);
			ogg_sync_init(&OggPtr->OggSyncState); /* Now we can read pages */
			OggPtr->State = 0;
		}
		Result = 0;
		break;
		
	/* Dispose of the codec */

	case MUSICCODECDESTROY:
		OggPtr = (OggState_t *)Input->CompressStatePtr;
		if (OggPtr) {
			ogg_stream_clear(&OggPtr->OggStreamState);
			vorbis_block_clear(&OggPtr->VorbisBlock);
			vorbis_dsp_clear(&OggPtr->VorbisDspState);
			vorbis_comment_clear(&OggPtr->VorbisComment);
			vorbis_info_clear(&OggPtr->VorbisInfo);  /* must be called last */
			ogg_sync_clear(&OggPtr->OggSyncState);
		}
	default:
		Result = 0;
		break;

	/* How much data to load in */

	case MUSICCODECDECODE:
		/* All done? */
		
		OggPtr = (OggState_t *)Input->CompressStatePtr;

		MaxLength = Input->SoundLength-Input->BytesPlayed;
		if (!MaxLength || OggPtr->State==666) {
			Result = (Word32)-1;			/* No more from now on! */
		} else {
			char *buffer;
			int i;
			int eos;
			
			/* Data from a previous decode? */

			if (Length>MaxLength) {
				Length = MaxLength;		/* Don't go too far */
			}
			
			Result = 0;			/* No bytes decompressed yet */

			/* First part of the decode cycle is to get the header info */
			/* for the Ogg/Vorbis stream */
			
			if (OggPtr->State==0) {
				buffer=ogg_sync_buffer(&OggPtr->OggSyncState,OGGBUFFERSIZE);
				i = Input->ReadProc(Input->CallBackParm,(Word8 *)buffer,OGGBUFFERSIZE);
				if (i==0 || i==-1) {
					return i;
				}
				ogg_sync_wrote(&OggPtr->OggSyncState,i);
				OggPtr->State = 1;
			}

			if (OggPtr->State==2) {
				goto State2;
			}
			if (OggPtr->State==1) {
				if (ogg_sync_pageout(&OggPtr->OggSyncState,&OggPtr->OggPage)==1){
					ogg_stream_init(&OggPtr->OggStreamState,ogg_page_serialno(&OggPtr->OggPage));
					vorbis_info_init(&OggPtr->VorbisInfo);
					vorbis_comment_init(&OggPtr->VorbisComment);
					if (ogg_stream_pagein(&OggPtr->OggStreamState,&OggPtr->OggPage)>=0){ 
						if (ogg_stream_packetout(&OggPtr->OggStreamState,&OggPtr->OggPacket)==1){ 
							if (vorbis_synthesis_headerin(&OggPtr->VorbisInfo,&OggPtr->VorbisComment,&OggPtr->OggPacket)>=0){ 
								i=0;
								do {
									do {
										int result=ogg_sync_pageout(&OggPtr->OggSyncState,&OggPtr->OggPage);
										if (!result) {
											break; /* Need more data */
										}
										if (result==1){
											ogg_stream_pagein(&OggPtr->OggStreamState,&OggPtr->OggPage);
											/* we can ignore any errors here as they'll also become apparent
												at packetout */
											do {
												result=ogg_stream_packetout(&OggPtr->OggStreamState,&OggPtr->OggPacket);
												if (!result) {
													break;
												}
												if (result<0) {
													OggPtr->State = 666;
													return (Word32)-1;
												}
												vorbis_synthesis_headerin(&OggPtr->VorbisInfo,&OggPtr->VorbisComment,&OggPtr->OggPacket);
											} while (++i<2);
										}
									} while (i<2);
									OggPtr->Tempi = i;
State2:;
									i = OggPtr->Tempi;
									buffer=ogg_sync_buffer(&OggPtr->OggSyncState,OGGBUFFERSIZE);
									eos=Input->ReadProc(Input->CallBackParm,(Word8 *)buffer,OGGBUFFERSIZE);
									if (eos==-1) {
										OggPtr->State = 666;
										return (Word32)-1;
									}
									if (eos==0 && i<2) {
										OggPtr->State = 2;
										return 0;
									}
									ogg_sync_wrote(&OggPtr->OggSyncState,eos);
								} while (i<2);
								
								OggPtr->convsize=OGGBUFFERSIZE/OggPtr->VorbisInfo.channels;
								vorbis_synthesis_init(&OggPtr->VorbisDspState,&OggPtr->VorbisInfo); /* central decode state */
								vorbis_block_init(&OggPtr->VorbisDspState,&OggPtr->VorbisBlock);     /* local state for most of the decode */
								OggPtr->State = 3;
							}
						}
					} else {
						OggPtr->State = 666;
						return (Word32)-1;
					}
				} else {
					OggPtr->State = 666;
					return (Word32)-1;
				}
			}
			
			if (OggPtr->State>=3) {
				int result;
				result = 0;
				eos = 0;
				if (OggPtr->State==4) {
					goto State4;
				}
				do {
					do {
						result=ogg_sync_pageout(&OggPtr->OggSyncState,&OggPtr->OggPage);
						if (!result) {
							break; /* need more data */
						}

						if (result>=0) { /* missing or corrupt data at this page position */
							ogg_stream_pagein(&OggPtr->OggStreamState,&OggPtr->OggPage); /* can safely ignore errors at
							this point */

							while (1) {
								result=ogg_stream_packetout(&OggPtr->OggStreamState,&OggPtr->OggPacket);
								if (!result) {
									break; /* need more data */
								}
								if (result>=0){ /* missing or corrupt data at this page position */
									/* we have a packet.  Decode it */
									float **pcm;
									int samples;

									if (vorbis_synthesis(&OggPtr->VorbisBlock,&OggPtr->OggPacket)==0) { /* test for success! */
										vorbis_synthesis_blockin(&OggPtr->VorbisDspState,&OggPtr->VorbisBlock);
									}
									
									
									/* 
									**pcm is a multichannel float vector.  In stereo, for
									example, pcm[0] is left, and pcm[1] is right.  samples is
									the size of each channel.  Convert the float values
									(-1.<=range<=1.) to whatever PCM format and write it out */

State4:;
									while((samples=vorbis_synthesis_pcmout(&OggPtr->VorbisDspState,&pcm))>0){
										int j;
										int bout=samples;
										
										if (bout>OggPtr->convsize) {
											bout = OggPtr->convsize;
										}
										
										j = Length/OggPtr->VorbisInfo.channels;
										j >>= 1;
										if (bout>j) {
											bout = j;
										}

										/* convert floats to 16 bit signed ints (host order) and
										interleave */
										
										for(i=0;i<OggPtr->VorbisInfo.channels;i++){
											short *ptr=((short *)DestPtr)+i;
											float *mono=pcm[i];
											for(j=0;j<bout;j++){
												int val=(int)(mono[j]*32767.f);
												/* might as well guard against clipping */
												if(val>32767){
													val=32767;
												}
												if(val<-32768){
													val=-32768;						
												}
												*ptr=val;
												ptr+=OggPtr->VorbisInfo.channels;
											}
										}
										
										/* tell libvorbis how many samples we actually consumed */
										vorbis_synthesis_read(&OggPtr->VorbisDspState,bout);
										bout *= OggPtr->VorbisInfo.channels*2;
										Length -= bout;
										DestPtr += bout;
										Result += bout;
										if (!Length) {
											Input->BytesPlayed+=Result;
											OggPtr->State = 4;
											return Result;
										}
									}
								}
							}
							if (ogg_page_eos(&OggPtr->OggPage)) {
								eos=1;
							}
						}
					} while (!eos);
					
					if (!eos) {
						buffer=ogg_sync_buffer(&OggPtr->OggSyncState,OGGBUFFERSIZE);
						i=Input->ReadProc(Input->CallBackParm,(Word8 *)buffer,OGGBUFFERSIZE);
						if (i==-1 || i==0) {
							eos = 1;
						} else {
							ogg_sync_wrote(&OggPtr->OggSyncState,i);
						}
					}
				} while (!eos);
				OggPtr->State = 0;
			}
			Input->BytesPlayed+=Result;
		}
		break;
	}
	return Result;
}

/**********************************

	Array of audio codecs

**********************************/

static DecodeCodecProc Codecs[] = {
	DigitalMusicByte,	/* BYTE */
	DigitalMusicChar,	/* CHAR */
	DigitalMusicLShort,	/* LSHORT */
	DigitalMusicBShort,	/* BSHORT */
	DigitalMusicADPCM,	/* ADPCM */
	DigitalMusicByte,	/* DVIPCM */
	DigitalMusicByte,	/* MP3 */
	DigitalMusicULaw,	/* ULAW */
	DigitalMusicALaw,	/* ALAW */
	DigitalMusicMace3,	/* MACE3 */
	DigitalMusicMace6,	/* MACE6 */
	DigitalMusicOgg		/* OGG */
};

/**********************************

	Convert a song number into a filename
	This is used by the music driver code and
	may be replaced by a user routine

**********************************/

#define OFFSET 12
static char SongName[] = "9:Music:Song00.Wav";

static char * BURGERCALL MakeSongFileName(Word SongNum)
{
	LongWordToAscii2(SongNum,&SongName[OFFSET],ASCIILEADINGZEROS|ASCIINONULL|2);
	return SongName;		/* Return the string pointer */
}

Word BurgerLastSong;		/* Song number currently playing */
Word BurgerSongFreq;		/* Frequency to play the music at */
Word BurgerSongLoops;		/* True if the song loops */
MakeSongProc DigitalMusicNameCallback = MakeSongFileName;

/**********************************

	Init the digital music player

**********************************/

void BURGERCALL DigitalMusicInit(void)
{
	if (!InitDigitalDriver()) {		/* Can I start the digital system */
		DigitalMusicShutdown();	/* Death! */
		return;
	}
	BurgerSndKillProcs[DIGIMUSICKILL] = DigitalMusicShutdown;	/* Remove on shutdown */
	BurgerSndExitIn |= DIGIMUSICON;		/* I am active */
	DigitalMusicSetVolume(255);		/* Set maximum volume */
	EnableSoundShutdownProc();
}

/**********************************

	Release the digital song player

**********************************/

void BURGERCALL DigitalMusicShutdown(void)
{
	DigitalMusicPlay(0);		/* Kill the music */
	BurgerSndKillProcs[DIGIMUSICKILL] = 0;	/* Don't call me anymore */
	BurgerSndExitIn &= ~DIGIMUSICON;	/* I am zapped */
	KillDigitalDriver();		/* Release the driver */
}

/**********************************

	Return TRUE if a song is playing
	The value is the song # being played

**********************************/

Word BURGERCALL DigitalMusicIsPlaying(void)
{
	if (!BurgerLastSong || BurgerLastSong==(Word)-1) {
		return FALSE;
	}
	return BurgerLastSong;
}

/**********************************

	Get the current pitch of the music

**********************************/

Word BURGERCALL DigitalMusicGetFrequency(void)
{
	return BurgerSongFreq;	/* Return sample rate */
}

/**********************************

	Get the volume of a song

**********************************/

Word BURGERCALL DigitalMusicGetVolume(void)
{
	return MusicVolume;		/* Just return the global */
}

/**********************************

	Set the callback function

**********************************/

void BURGERCALL DigitalMusicSetFilenameProc(MakeSongProc Proc)
{
	if (!Proc) {
		Proc = MakeSongFileName;
	}
	DigitalMusicNameCallback = Proc;
}

/**********************************

	Returns 8 bits of data representing silence for the Wave file format.

	Since we are dealing only with PCM format, we can fudge a bit and take
	advantage of the fact that for all PCM formats, silence can be represented
	by a single byte, repeated to make up the proper word size. The actual size
	of a word of wave data depends on the format:

	PCM Format		Word Size	Silence Data
	8-bit mono		1 byte		0x80
	8-bit stereo	2 bytes		0x8080
	16-bit mono		2 bytes		0x0000
	16-bit stereo	4 bytes		0x00000000

**********************************/

Word BURGERCALL DigitalMusicGetSilenceVal(Word Type)
{
	Type = Type&0xFF;
	if (Type == SOUNDTYPEBYTE) {
		return 0x80;	/* 8 bit format */
	}
	return 0x00;		/* 16 bit or signed 8 bit*/
}

/**********************************

	Read data into an audio buffer,
	convert to the proper destination format

**********************************/

Word BURGERCALL DigitalMusicDecode(DigitalMusicReadState_t *Input,Word8 *DestBuffer,Word32 Length)
{
	if (Input->CodecProc) {
		return Input->CodecProc(Input,MUSICCODECDECODE,DestBuffer,Length);
	}
	return (Word)-1;		/* End of data */

}

/**********************************

	Init a DigitalMusicReadState structure

**********************************/

Word BURGERCALL DigitalMusicReadStateInit(DigitalMusicReadState_t *Output,RawSound_t *Input,Word8 *ImageStart,Word32 MaxSize,DecodeCallbackProc Proc,void *Parm)
{
	Word Type;

	FastMemSet(Output,0,sizeof(DigitalMusicReadState_t));		/* Reset the struct */
	Output->ReadProc = Proc;									/* Save the read data proc */
	Output->CallBackParm = Parm;
	Output->FileOffset = Input->SoundPtr-ImageStart;			/* File reader offset */
	Output->SoundLength = Input->SoundLength;					/* Default decompressed data size */
/*	Output->BytesPlayed = 0;	*/

	Type = Input->DataType;
	Output->DataType = Type;
	Type = Type&0xFF;
	if (Type>SOUNDTYPEOGG) {
		Type = SOUNDTYPEBYTE;		/* Failsafe! */
	}
	Output->CodecProc = Codecs[Type];	/* Get the decompression codec */
	return Output->CodecProc(Output,MUSICCODECINIT,(Word8 *)Input,MaxSize);
	
}

/**********************************

	Dispose of a DigitalMusicReadState structure

**********************************/

void BURGERCALL DigitalMusicReadStateDestroy(DigitalMusicReadState_t *Input)
{
	if (Input->CodecProc) {
		Input->CodecProc(Input,MUSICCODECDESTROY,0,0);
		Input->CodecProc = 0;		/* Zap the pointer */
	}
	DeallocAPointer(Input->CompressStatePtr);
	Input->CompressStatePtr = 0;
}

/**********************************

	Move the file mark to the beginning

**********************************/

void BURGERCALL DigitalMusicReset(DigitalMusicReadState_t *Input)
{
	Input->BytesPlayed = 0;	/* Reset the played bytes size */
	if (Input->CodecProc) {
		Input->CodecProc(Input,MUSICCODECRESET,0,0);
	}
}


#if !defined(__MAC__) && !defined(__MSDOS__) && !defined(__WIN32__)

#include <stdio.h>

/**********************************

	Begin song playback

**********************************/

void BURGERCALL DigitalMusicPlay(Word FileNum)
{
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
	if (NewVolume>=256) {
		NewVolume = 255;
	}
	MusicVolume = NewVolume;
	Temp = SystemState&~(MusicActive);
	if (NewVolume) {
		Temp |= MusicActive;
	}
	SystemState = Temp;
}

#endif

/**********************************

	Pause digital music

**********************************/

void BURGERCALL DigitalMusicPause(void)
{
}

/**********************************

	Resume digital music

**********************************/

void BURGERCALL DigitalMusicResume(void)
{
}

