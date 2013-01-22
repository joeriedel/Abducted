/**********************************

	Compression manager

**********************************/

#ifndef __PKPACK_H__
#define __PKPACK_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PackState_t {	/* State of a decompression buffer */
    Word8 *PackPtr;				/* Packed data pointer */
    Word32 PackLen;			/* Number of packed bytes remaining */
    Word8 *OutPtr;				/* Output data pointer */
    Word32 OutLen;			/* Number of bytes in the output buffer */
    void *Internal;				/* Pointer to algorithm specific code */
} PackState_t;

typedef struct MACEState_t {	/* State of MACE compression/decompression */
	long Sample1;		/* Last running samples */
	long Sample2;		/* Second temp sample */
	long PrevVal;		/* Mask with 0x8000 for + or - direction */
	long TableIndex;	/* Which slope table */
	long LastAmplitude;	/* Slope * PrevVal */
	long LastSlope;		/* Last Slope value */
} MACEState_t;

typedef struct ADPCMUnpackState_t {	/* State of ADPCM decompression */
	Word8 *SrcPtr;			/* Pointer to the source data */
	Word32 SrcLength;		/* Pointer to the size of the source buffer */
	Word Channels;			/* Number of channels to decode (1 or 2) */
	Word BlockSize;			/* Size of each compressed block */
	Word SamplesPerBlock;	/* Number of samples to decompress per block */
	short *OutputPtr;		/* Output buffer */
} ADPCMUnpackState_t;

extern const short UnpackULawTable[256];
extern const short UnpackALawTable[256];

extern void BURGERCALL DLZSS(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length,Word32 PackedLen);
extern void BURGERCALL DLZSSFast(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length);
extern void ** BURGERCALL EncodeLZSS(Word8 *InputBuffer,Word32 Length);
extern void BURGERCALL DHuffman(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length,Word32 PackedLen);
extern void BURGERCALL DHuffmanFast(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length);
extern void ** BURGERCALL EncodeHuffman(Word8 *InputBuffer,Word32 Length);
extern void BURGERCALL DLZH(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length,Word32 PackedLen);
extern void BURGERCALL DLZHFast(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length);
extern void ** BURGERCALL EncodeLZH(Word8 *InputBuffer,Word32 Length);
extern void BURGERCALL DRLE(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length,Word32 PackedLen);
extern void BURGERCALL DRLEFast(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length);
extern void ** BURGERCALL EncodeRLE(Word8 *InputBuffer,Word32 Length);
extern Word BURGERCALL DInflateInit(PackState_t *Input);
extern Word BURGERCALL DInflateMore(PackState_t *Input);
extern void BURGERCALL DInflateDestroy(PackState_t *Input);
extern void BURGERCALL DInflate(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length,Word32 PackedLen);
extern void BURGERCALL DInflateFast(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length);
extern void ** BURGERCALL EncodeInflate(Word8 *InputBuffer,Word32 Length);
extern void BURGERCALL MACEExp1to3(const Word8 *InBufPtr,Word8 *OutBufPtr,Word32 Count,MACEState_t *InStatePtr,MACEState_t *OutStatePtr,Word NumChannels,Word WhichChannel);
extern void BURGERCALL MACEExp1to6(const Word8 *InBufPtr,Word8 *OutBufPtr,Word32 Count,MACEState_t *InStatePtr,MACEState_t *OutStatePtr,Word NumChannels,Word WhichChannel);
extern Word BURGERCALL ADPCMDecodeBlock(ADPCMUnpackState_t *StatePtr);

#ifdef __cplusplus
}
#endif

#endif

