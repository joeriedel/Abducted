#include "ImImage.h"
#include <BREndian.hpp>
#include "MmMemory.h"
#include "ClStdLib.h"
#include <string.h>

/**********************************

	Decode a GIF87a or a GIF89a file

**********************************/

typedef struct GIFHeader_t {
	char GIFName[6];	/* GIF87a in ASCII form */
	Word16 Width;		/* Width of the image in pixels */
	Word16 Height;		/* Height of the image in pixels */
	Word8 Descriptor;	/* Bits per pixel descriptor */
	Word8 BackColor;		/* Background color */
	Word8 Null;			/* Not used */
} GIFHeader_t;

typedef struct GIFDecoder_t {
	const Word8 *LZWInputPtr;	/* Pointer to packed data */
	Word BitBucket;			/* Current bit bucket */
	Word BitCount;			/* Current valid bit count in bucket */
	Word ChunkCount;		/* Number of bytes in stream */
	Word LZWCodeSize;		/* Starting data size */
	Word8 *LZWTable[4096];	/* Dictionary pointers */
	Word8 LZWArray[256]; 	/* 0-255 for default data */
	Word LZWSize[4096]; 	/* Size of each entry */
} GIFDecoder_t;

/**********************************

	I don't use GIF extension blocks.
	This code will skip over them.

**********************************/

static const Word8 * BURGERCALL DiscardExtension(const Word8 *InputPtr)
{
	Word Count;
	Count = 2;		/* Index past the '!' and opcode */
	do {
		InputPtr = InputPtr+Count;	/* Move the pointer */
		Count = *InputPtr++;	/* Get the data count */
	} while (Count);	/* Still more data? */
	return InputPtr;	/* Return pointer to following data */
}

/**********************************

	These variables are used by the LZW decompressor

**********************************/

static Word MaskTable[] = {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80,0x100,0x200,0x400,0x800,0x1FFF};

/**********************************

	Get an LZW code token

**********************************/

static Word BURGERCALL GetLZWCode(GIFDecoder_t *Input,Word Count)
{
	Word Code;		/* LZW code fetched from input stream */
	Word Mask;		/* Or mask */
	Word Temp;		/* Temp value */

	Mask = 1;		/* Start at the bottom */
	Code = 0;		/* Init returned value */
	do {
		Temp = Input->BitCount;		/* Get the valid bit count */
		if (!Temp) {			/* Empty? */
			const Word8 *WorkPtr;

			WorkPtr = Input->LZWInputPtr;	/* Cache pointer */
			Temp = Input->ChunkCount;	/* Any more fresh data? */
			if (!Temp) {	/* No more! */
				Temp = WorkPtr[0];	/* Get the packet count */
				if (!Temp) {		/* If zero then keep returning zeros */
					return FALSE;	/* Exit now */
				}
				++WorkPtr;	/* Inc input pointer */
			}
			--Temp;			/* I accept a byte */
			Input->ChunkCount = Temp;		/* Save result */
			Input->BitBucket = WorkPtr[0];	/* Get next byte */
			++WorkPtr;
			Input->LZWInputPtr = WorkPtr;	/* Store the new pointer */
			Temp = 8;		/* 8 valid bits! */
		}
		--Temp;			/* Accept a bit */
		Input->BitCount=Temp;	/* Save new count */
		Temp = Input->BitBucket;	/* Get the bit bucket */
		Input->BitBucket = Temp>>1;	/* Shift out a bit */
		if (Temp&1) {		/* Return true or false */
			Code |= Mask;	/* Blend... */
		}
		Mask <<= 1;		/* Shift up the mask */
	} while (--Count);	/* More? */
	return Code;		/* Return the code */
}

/**********************************

	Init the LZW permenant tokens

**********************************/

static void BURGERCALL InitLZWTokens(GIFDecoder_t *Input)
{
	Word i;
	Word8 *DestPtr;
	i = 0;
	DestPtr = Input->LZWArray;
	Input->BitCount = 0;
	Input->ChunkCount = 0;
	do {
		DestPtr[0] = static_cast<Word8>(i);		/* 0-255 */
		Input->LZWSize[i] = 1;			/* All are 1 byte in size */
		Input->LZWTable[i] = DestPtr;	/* Init pointers to LZWArray table */
		++DestPtr;
	} while (++i<256);
}

/**********************************

	Reset the LZW token tree

**********************************/

static void BURGERCALL ResetLZWTokens(GIFDecoder_t *Input)
{
	Word i;
	i = 256;		/* Generated tokens */
	do {
		Input->LZWSize[i] = 0;		/* Nothing here! */
		Input->LZWTable[i] = 0;	/* No pointers */
	} while (++i<4096);		/* All of them */
}

/**********************************

	Decompress the image

**********************************/

static Word BURGERCALL UnpackGIFLZW(Word8 *DestBuffer,const Word8 *InputPtr,Word32 Length)
{
	Word Code;		/* Current token */
	Word OldCode;	/* Previous token */
	Word NewCode;	/* Newest token being generated */
	Word CodeSize;	/* Size of tokens in bits */
	Word CodeMask;	/* NewCode number to match to increase token bit size */
	Word Count;		/* Temp index counter */
	Word8 *OldMark;	/* Pointer from previous loop */
	Word8 *PrevPtr;	/* Current dest pointer */
	GIFDecoder_t *Decoder;	/* Temp data pointer */

	Decoder = static_cast<GIFDecoder_t *>(AllocAPointerClear(sizeof(*Decoder)));
	if (Decoder) {
		Decoder->LZWCodeSize = InputPtr[0];		/* Number of bits for code size */
		++InputPtr;
		Decoder->LZWInputPtr = InputPtr;
		OldCode = (Word)-1;
		CodeSize = Decoder->LZWCodeSize+1;
		NewCode = 256+2;
		InitLZWTokens(Decoder);
		do {
			Code = GetLZWCode(Decoder,CodeSize);	/* Get LZW token */
			if (Code == 257) {		/* End token? */
				break;
			}
			if (Code==256) {		/* Reset token? */
				ResetLZWTokens(Decoder);	/* Reset the dictionary */
				NewCode = 256+2;	/* Reset next new token */
				CodeSize = Decoder->LZWCodeSize+1;	/* 9 bits again */
				CodeMask = MaskTable[CodeSize];	/* Reset the mask */
				OldCode = (Word)-1;
				OldMark = DestBuffer;
				continue;
			}
			PrevPtr = DestBuffer;
			InputPtr = Decoder->LZWTable[Code];		/* Get the string pointer */
			if (InputPtr) {					/* Valid pointer? */
				Count = Decoder->LZWSize[Code];		/* Size of the string */
				Length -= Count;			/* Remove from failsafe */
				do {
					*DestBuffer++ = *InputPtr++;	/* Copy the string */
				} while (--Count);
				if (OldCode==(Word)-1) {	/* First pass? */
					goto FirstTime;			/* Then don't make a new record */
				}
			} else {
				Count = Decoder->LZWSize[OldCode]+1;	/* Get the previous match */
				Length -= Count;			/* Remove from length */
				InputPtr = OldMark;			/* Set the pointer */
				do {
					*DestBuffer++ = *InputPtr++;	/* Copy data */
				} while (--Count);
			}
			Decoder->LZWSize[NewCode] = Decoder->LZWSize[OldCode]+1;	/* Set the new length */
			Decoder->LZWTable[NewCode] = OldMark;		/* Data pointer */
			++NewCode;		/* Next new code */

			if (NewCode==CodeMask) {	/* Do I need another bit next pass? */
				++CodeSize;				/* Increase the bit size */
				CodeMask = MaskTable[CodeSize];	/* Use table */
			}
FirstTime:;
			OldCode = Code;			/* Set the new previous index */
			OldMark = PrevPtr;		/* Pointer to this work data */
		} while (Length);

		/* Exit */
		DeallocAPointer(Decoder);	/* Release the temp memory */
	 	if (!Length) {		/* Check if properly decompressed */
			return FALSE;		/* Good! */
		}
		NonFatal("Error in GIF decompression\n");
	}
	return TRUE;	/* Bad */
}

/**********************************

	Decode a Compuserve GIF file

**********************************/

Word BURGERCALL ImageParseGIF(Image_t *ImagePtr,const Word8 *InputPtr)
{
	GIFHeader_t Header;
	Word Count;
	Word32 Length;
	Word8 *DestBuffer;
	Word Gif89;

	FastMemSet(ImagePtr,0,sizeof(Image_t));		/* Clear out result */
	FastMemCpy(&Header,InputPtr,13);		/* Get the GIF header */
	InputPtr = InputPtr+13;				/* Index past the header */

	Gif89 = FALSE;
	if (memcmp(Header.GIFName,"GIF87a",6)) {	/* Header ok? */
		if (memcmp(Header.GIFName,"GIF89a",6)) {
			NonFatal("Invalid GIF 87/89 ASCII header\n");
			goto OhShit;
		}
		Gif89 = TRUE;
	}
	ImagePtr->Width = Burger::LoadLittle(&Header.Width);	/* Get the size */
	ImagePtr->Height = Burger::LoadLittle(&Header.Height);
	if ((Header.Descriptor&7)!=7) {
		NonFatal("GIF files must be 8 bits per pixel!\n");
		goto OhShit;
	}
	ImagePtr->DataType = IMAGE8_PAL;
	ImagePtr->RowBytes = ImagePtr->Width;
	if (!(Header.Descriptor&0x80)) {		/* Is there a palette? */
		NonFatal("No global palette in GIF file\n");
		goto OhShit;
	}
	Count = Header.Descriptor>>4;		/* Isolate the upper 4 bits */
	Count = Count&7;		/* Get the bits per pixel */
	Count = 7;
	Count = 2<<Count;		/* Convert to a color count */
	DestBuffer = (Word8 *)AllocAPointer(768);
	if (!DestBuffer) {
		goto OhShit;
	}
	ImagePtr->PalettePtr = DestBuffer;		/* Save the palette */
	FastMemSet(DestBuffer,0,768);				/* Blank it out */
	Count = Count*3;
	FastMemCpy(DestBuffer,InputPtr,Count);		/* Get the entries */
	InputPtr = InputPtr+Count;				/* Accept the entries */

	while (InputPtr[0]=='!') {		/* Is this an extension block? */
		InputPtr = DiscardExtension(InputPtr);	/* Discard it */
	}

	if (InputPtr[0]!=',') {			/* Data block? */
		NonFatal("Unknown data block found instead of bitmap\n");
		goto OhShit;
	}

	/* Now I have a bit map, let's decompress it using a version of LZW */

	++InputPtr;
	if (((Word16 *)InputPtr)[0] || ((Word16 *)InputPtr)[1]) {
		NonFatal("Origin of GIF image is not zero\n");
		goto OhShit;
	}

	if (Burger::LoadLittle(&reinterpret_cast<const Word16 *>(InputPtr)[2]) != ImagePtr->Width ||
		Burger::LoadLittle(&reinterpret_cast<const Word16 *>(InputPtr)[3]) != ImagePtr->Height) {
		NonFatal("Image parsed is not the same size as total image!\n");
		goto OhShit;
	}
	if (InputPtr[8]) {
		NonFatal("Non-supported sub-descriptor in GIF\n");
		goto OhShit;
	}
	InputPtr = InputPtr+9;		/* Index to LZW data */

	Length = (Word32)ImagePtr->Width*ImagePtr->Height;
	DestBuffer = (Word8 *)AllocAPointer(Length);	/* Space for data */
	if (!DestBuffer) {
		goto OhShit;
	}
	ImagePtr->ImagePtr = DestBuffer;		/* Save the buffer */
	if (!UnpackGIFLZW(DestBuffer,InputPtr,Length)) {	/* Decompress it */
		return FALSE;		/* I got it! */
	}

OhShit:;
	ImageDestroy(ImagePtr);		/* Free the memory */
	return TRUE;
}
