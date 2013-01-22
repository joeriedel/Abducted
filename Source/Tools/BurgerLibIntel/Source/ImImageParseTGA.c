#include "ImImage.h"
#include <BREndian.hpp>
#include "MmMemory.h"
#include "ClStdLib.h"
#include <stdio.h>

/**********************************

	This routine will take a TGA file image in memory and
	convert it into an Image_t structure.
	It will handle both uncompressed and compressed 8,15,24 and 32
	bit TGA images.

	Info was based on David McDuffee's "Targa.format" FAQ I found
	while surfing the 'net.

**********************************/

/**********************************

	This is the data format for a TGA structure
	I use Bytes for the ColorMap to prevent
	the Mac compilers from improperly padding this structure
	and cause havoc in the process

**********************************/

typedef struct TGAHeader {		/* This is the header for a TGA file */
	Word8 ImageIdent;	/* Image identification field size in bytes */
	Word8 ColorMapType;	/* 1 for a color image */
	Word8 ImageType;		/* Data type present */
	Word8 ColorMapOriginLow;	/* Index of the first color map entry */
	Word8 ColorMapOriginHigh;
	Word8 ColorMapLengthLow;	/* Count of color map entries */
	Word8 ColorMapLengthHigh;
	Word8 ColorMapEntrySize;	/* Size (In bits) for each color entry */
	Word16 XOrigin;		/* Lower left x origin of image */
	Word16 YOrigin;		/* Lower left y origin of image */
	Word16 Width;		/* Width of the image in pixels */
	Word16 Height;		/* Height of the image in pixels */
	Word8 BitDepth;		/* Bits per pixels for the image */
	Word8 Descriptor;	/* Image descriptor bits */
} TGAHeader;

#define TGAHeaderSize 18	/* Make sure that struct padding isn't happening */

/**********************************

	Decompress a TGA file using RLE encoding
	I can only decompress BYTES.

**********************************/

static void BURGERCALL UnpackTGAByte(Word8 *DestBuffer,const Word8 *InputPtr,Word32 Pixels)
{
	Word Count;
	do {
		Count = InputPtr[0];	/* Get the counter */
		if (Count&0x80) {		/* Packed? */
			Word8 Temp;
			Count = Count-0x7F;	/* Remove the high bit */
			Pixels = Pixels-Count;
			Temp = InputPtr[1];	/* Get the repeater */
			InputPtr+=2;
			do {
				DestBuffer[0] = Temp;	/* Fill memory */
				++DestBuffer;
			} while (--Count);
		} else {
			++InputPtr;
			++Count;			/* +1 to the count */
			Pixels = Pixels-Count;	/* Adjust amount */
			do {
				DestBuffer[0] = InputPtr[0];	/* Memcpy */
				++InputPtr;
				++DestBuffer;
			} while (--Count);		/* All done? */
		}
	} while ((long)Pixels>0);	/* Is there still more? */
}

/**********************************

	Decompress a TGA file using RLE encoding
	I can only decompress SHORTS and swap endian if needed

**********************************/

static void BURGERCALL UnpackTGAShort(Word8 *DestBuffer,const Word8 *InputPtr,Word32 Pixels)
{
	Word Count;
	do {
		Count = InputPtr[0];	/* Get the counter */
		if (Count&0x80) {		/* Packed? */
			Word16 Temp;
			Count = Count-0x7F;	/* Remove the high bit and add 1 */
			Pixels = Pixels-Count;
			Temp = Burger::LoadLittle(((Word16 *)(InputPtr+1))[0]);	/* Get the repeater */
			InputPtr = InputPtr+3;
			do {
				((Word16 *)DestBuffer)[0] = Temp;	/* Fill memory */
				DestBuffer = DestBuffer+2;
			} while (--Count);
		} else {
			++InputPtr;
			++Count;			/* +1 to the count */
			Pixels = Pixels-Count;	/* Adjust amount */
			do {
				((Word16 *)DestBuffer)[0] = Burger::LoadLittle(((Word16 *)InputPtr)[0]);	/* Memcpy */
				InputPtr = InputPtr+2;
				DestBuffer = DestBuffer+2;
			} while (--Count);		/* All done? */
		}
	} while ((long)Pixels>0);	/* Is there still more? */
}

/**********************************

	Decompress a TGA file using RLE encoding
	I can only decompress 3 BYTES.

**********************************/

static void BURGERCALL UnpackTGAByte3(Word8 *DestBuffer,const Word8 *InputPtr,Word32 Pixels)
{
	Word Count;
	do {
		Count = *InputPtr++;	/* Get the counter */
		if (Count&0x80) {		/* Packed? */
			Word16 Temp;
			Word8 Temp2;
			Count = Count-0x7F;	/* Remove the high bit */
			Pixels = Pixels-Count;
			Temp = Burger::SwapEndian(((Word16 *)(InputPtr+1))[0]);	/* Get G,R */
			Temp2 = InputPtr[0];		/* Blue */
			InputPtr = InputPtr+3;
			do {
				((Word16 *)DestBuffer)[0] = Temp;	/* R and G */
				DestBuffer[2] = Temp2;		/* Blue */
				DestBuffer = DestBuffer+3;
			} while (--Count);
		} else {
			++Count;			/* +1 to the count */
			Pixels = Pixels-Count;	/* Adjust amount */
			do {
				DestBuffer[0] = InputPtr[2];	/* Red */
				DestBuffer[1] = InputPtr[1];	/* Green */
				DestBuffer[2] = InputPtr[0];	/* Blue */
				DestBuffer = DestBuffer+3;		/* Next index */
				InputPtr = InputPtr+3;
			} while (--Count);		/* All done? */
		}
	} while ((long)Pixels>0);	/* Is there still more? */
}

/**********************************

	Decompress a TGA file using RLE encoding
	I can only decompress LongWords and swap endian if needed

**********************************/

static void BURGERCALL UnpackTGALong(Word8 *DestBuffer,const Word8 *InputPtr,Word32 Pixels)
{
	Word Count;
	do {
		Count = *InputPtr++;	/* Get the counter */
		if (Count&0x80) {		/* Packed? */
			Word32 Temp;
			Count = Count-0x7F;	/* Remove the high bit and add 1 */
			Pixels = Pixels-Count;
			((Word8 *)&Temp)[0] = InputPtr[2];	/* Red */
			((Word8 *)&Temp)[1] = InputPtr[1];	/* Green */
			((Word8 *)&Temp)[2] = InputPtr[0];	/* Blue */
			((Word8 *)&Temp)[3] = InputPtr[3];	/* Alpha */
			InputPtr = InputPtr+4;
			do {
				((Word32 *)DestBuffer)[0] = Temp;	/* Fill memory */
				DestBuffer = DestBuffer+4;
			} while (--Count);
		} else {
			++Count;				/* +1 to the count */
			Pixels = Pixels-Count;	/* Adjust amount */
			do {
				DestBuffer[0] = InputPtr[2];	/* Red */
				DestBuffer[1] = InputPtr[1];	/* Green */
				DestBuffer[2] = InputPtr[0];	/* Blue */
				DestBuffer[3] = InputPtr[3];	/* Alpha */
				InputPtr = InputPtr+4;
				DestBuffer = DestBuffer+4;
			} while (--Count);		/* All done? */
		}
	} while ((long)Pixels>0);	/* Is there still more? */
}

/**********************************

	Read in a TGA file and set the variables
	PicHeight,PicWidth,PicBuffer,Palette
	return 0 if successful

**********************************/

Word BURGERCALL ImageParseTGA(Image_t *ImagePtr,const Word8 *InputPtr)
{
	TGAHeader Header;		/* Header to TGA file */
	Word Depth;
	
	FastMemSet(ImagePtr,0,sizeof(Image_t));	/* Blank out the input structure */

	FastMemCpy(&Header,InputPtr,TGAHeaderSize);	/* Copy the header to a local */
	InputPtr = InputPtr+TGAHeaderSize;		/* Index past the header */

	/* Check if I even support this format type */

	switch (Header.ImageType) {	/* Can only parse 1,2,9 and 10 */
	default:
		NonFatal("Can't parse image type %d, must be 1,2,9 or 10\n",Header.ImageType);
		goto OhShit;
	case 1:		/* Unpacked indexed */
	case 2:		/* Unpacked true color */
//	case 3:		/* Unpacked gray scale */
	case 9:		/* RLE Packed indexed */
	case 10:;	/* RLE Packed true color */
//	case 11:	/* RLE Packed gray scale */
	}

	/* I don't support some special features! */

	if (Header.XOrigin || Header.YOrigin) {
		NonFatal("Image has a non-zero origin\n");
		goto OhShit;
	}

#if 0								/* It appear's that no one really cares about this... */
	if (Header.Descriptor) {		/* Get the image descriptor */
		NonFatal("Must have zero for image descriptor\n");
		goto OhShit;
	}
#endif
	ImagePtr->Width = Burger::LoadLittle(Header.Width);	/* Convert the endian */
	ImagePtr->Height = Burger::LoadLittle(Header.Height);
	Depth = Header.BitDepth;	/* And bit depth */

	if (!ImagePtr->Width) {
		NonFatal("Can't process a file with a width of zero!\n");
		goto OhShit;
	}

	if (!ImagePtr->Height) {
		NonFatal("Can't process a file with a height of zero!\n");
		goto OhShit;
	}

/**********************************

	On all images, skip past any image identification record

**********************************/

	InputPtr = InputPtr + Header.ImageIdent;

/**********************************

	Is there a palette present?

**********************************/

	if (Header.ColorMapType == 1 || Header.ColorMapType==9) {		/* Palette present! */
		Word8 *PalPtr;		/* Work pointer to palette */
		Word i;				/* Temp */
		Word Count;			/* Number of colors to process */

		PalPtr = (Word8 *)AllocAPointer(768);	/* Get memory for palette */
		if (!PalPtr) {
			goto OhShit;		/* Yeah right!! */
		}
		ImagePtr->PalettePtr = PalPtr;		/* Save in returned structure */
		FastMemSet(PalPtr,0,768);				/* Blank it out */
		Count = Header.ColorMapLengthLow+(Header.ColorMapLengthHigh<<8);	/* Get the ending color */
		if (Count) {		/* Any colors used? */
			i = Header.ColorMapOriginLow+(Header.ColorMapOriginHigh<<8);	/* Get the starting color */
			if ((i+Count)>256) {
				i = (i+Count)-1;		/* Get the final color index */
				NonFatal("Color index %u cannot be > 255\n",i);
				goto OhShit;
			}
			if (i) {		/* Should I skip any colors? */
				i = i*3;
				PalPtr = PalPtr+i;		/* Index past them in the palette */
			}
			if (Header.ColorMapEntrySize == 16) {
				do {
					Word Foo;
					Foo = Burger::LoadLittle(((Word16 *)InputPtr)[0]);
					i = (Foo>>(10-3))&0xF8;	/* Isolate red */
					PalPtr[0] = static_cast<Word8>(i);	/* Red */
					i = (Foo>>(5-3))&0xF8;	/* Isolate green */
					PalPtr[1] = static_cast<Word8>(i);	/* Green */
					i = (Foo<<3)&0xF8;		/* Isolate blue */
					PalPtr[2] = static_cast<Word8>(i);	/* Blue */
					PalPtr = PalPtr+3;
					InputPtr = InputPtr+2;
				} while (--Count);
			} else if (Header.ColorMapEntrySize == 24) {
				do {
					PalPtr[0] = InputPtr[2];	/* Red */
					PalPtr[1] = InputPtr[1];	/* Green */
					PalPtr[2] = InputPtr[0];	/* Blue */
					PalPtr = PalPtr+3;
					InputPtr = InputPtr+3;
				} while (--Count);
			} else if (Header.ColorMapEntrySize == 32) {
				do {
					PalPtr[0] = InputPtr[2];	/* Red */
					PalPtr[1] = InputPtr[1];	/* Green */
					PalPtr[2] = InputPtr[0];	/* Blue */
					PalPtr = PalPtr+3;
					InputPtr = InputPtr+4;		/* Skip extra */
				} while (--Count);
			} else {
				NonFatal("Palette uses %u bits per color entry, only 16,24 and 32 allowed\n",Header.ColorMapEntrySize);
				goto OhShit;
			}
		}
	}

/**********************************

	Now, there are 4 ways this file can be parsed,
	Packed/Unpacked Index color or TrueColor

	Indexed images REQUIRE a palette!

**********************************/

	if (Header.ImageType == 1 || Header.ImageType==9) {		/* Indexed? */
		Word32 Length;		/* Size of the data in bytes */
		Word8 *DestBuffer;		/* Pointer to the buffer */

		if (!ImagePtr->PalettePtr) {
		 	NonFatal("Image file is missing a palette!\n");
			goto OhShit;
		}
		if (!Depth || Depth >= 9) {
			NonFatal("The image has a bit depth of %u, I can only process 1-8 bit indexed images.\n",ImagePtr->DataType);
			goto OhShit;
		}
		ImagePtr->DataType = IMAGE8_PAL;			/* I will return only an 8 bit image */
		ImagePtr->RowBytes = ImagePtr->Width;
		Length = (Word32)ImagePtr->Width*ImagePtr->Height;	/* Get buffer size */

		DestBuffer = (Word8 *)AllocAPointer(Length);
		if (!DestBuffer) {
			goto OhShit;
		}
		ImagePtr->ImagePtr = DestBuffer;	/* Save the pointer */

		if (Header.ImageType == 1) {		/* Unpacked indexed data */
			FastMemCpy(DestBuffer,InputPtr,Length);
		} else {		/* Compressed */
			UnpackTGAByte(DestBuffer,InputPtr,Length);	/* Decompress it */
		}

	} else {			/* True color! */
		Word32 Length;
		Word Pixels;
		Word8 *DestBuffer;

		DestBuffer = ImagePtr->PalettePtr;	/* Dispose of a true color palette */
		if (DestBuffer) {
			DeallocAPointer(DestBuffer);	/* Should not have a palette */
			ImagePtr->PalettePtr = 0;
		}

		if (Depth==16) {
			Depth = 15;
		}
		if (Depth != 15 && Depth != 24 && Depth!=32) {
			NonFatal("The image has a bit depth of %u, I can only process true color images of 15,16,24 or 32 bits.\n",ImagePtr->DataType);
			goto OhShit;
		}
		Pixels = (Depth+7)>>3;		/* Convert to bytes 2 or 3 */
		ImagePtr->RowBytes = Pixels*ImagePtr->Width;
		if (Depth==15) {
			ImagePtr->DataType = IMAGE555;
		} else if (Depth==24) {
			ImagePtr->DataType = IMAGE888;
		} else {
			ImagePtr->DataType = IMAGE8888;
		}
		Length = (Word32)ImagePtr->Width*ImagePtr->Height;

		DestBuffer = (Word8 *)AllocAPointer(Length*Pixels);
		if (!DestBuffer) {
			goto OhShit;
		}
		ImagePtr->ImagePtr = DestBuffer;

		if (Pixels == 2) {		/* 16 bit */
			if (Header.ImageType == 2) {	/* Unpacked? */
				do {
					Word16 Temp;
					Temp = Burger::LoadLittle(((Word16 *)InputPtr)[0])&0x7FFF;
					((Word16 *)DestBuffer)[0] = Temp;		/* Save pixel */
					DestBuffer=DestBuffer+2;
					InputPtr = InputPtr+2;
				} while (--Length);		/* All done? */
			} else {
				UnpackTGAShort(DestBuffer,InputPtr,Length);
			}
		} else if (Pixels == 3) {
			if (Header.ImageType == 2) {		/* Unpacked */
				do {
					DestBuffer[0] = InputPtr[2];	/* Red */
					DestBuffer[1] = InputPtr[1];	/* Green */
					DestBuffer[2] = InputPtr[0];	/* Blue */
					DestBuffer = DestBuffer+3;
					InputPtr = InputPtr + 3;
				} while (--Length);
			} else {
				UnpackTGAByte3(DestBuffer,InputPtr,Length);
			}
		} else {
			if (Header.ImageType == 2) {	/* Unpacked? */
				do {
					DestBuffer[0] = InputPtr[2];	/* Red */
					DestBuffer[1] = InputPtr[1];	/* Green */
					DestBuffer[2] = InputPtr[0];	/* Blue */
					DestBuffer[3] = InputPtr[3];	/* Alpha */
					DestBuffer=DestBuffer+4;
					InputPtr = InputPtr+4;
				} while (--Length);		/* All done? */
			} else {
				UnpackTGALong(DestBuffer,InputPtr,Length);
			}
		}
	}
	ImageVerticalFlip(ImagePtr);		/* Flip the image upside down */
	return FALSE;		/* Exit */

/* Errors! */

OhShit:
	ImageDestroy(ImagePtr);		/* Wipe out the image record */
	return TRUE;				/* Return the error */
}


