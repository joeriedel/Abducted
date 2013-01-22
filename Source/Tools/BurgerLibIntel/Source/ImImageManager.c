#include "ImImage.h"
#include "MmMemory.h"
#include "ClStdLib.h"
#include "PlPalette.h"

const Word RGB2ToRGB3Table[4] = {0x00,0x02,0x05,0x07};
const Word RGB2ToRGB4Table[4] = {0x00,0x05,0x0A,0x0F};
const Word RGB2ToRGB5Table[4] = {0x00,0x0A,0x15,0x1F};
const Word RGB2ToRGB6Table[4] = {0x00,0x15,0x2A,0x3F};
const Word RGB2ToRGB8Table[4] = {0x00,0x55,0xAA,0xFF};
const Word RGB3ToRGB4Table[8] = {0x00,0x02,0x04,0x06,0x09,0x0B,0x0D,0x0F};
const Word RGB3ToRGB5Table[8] = {0x00,0x04,0x09,0x0D,0x12,0x16,0x1B,0x1F};
const Word RGB3ToRGB6Table[8] = {0x00,0x09,0x12,0x1B,0x24,0x2D,0x36,0x3F};
const Word RGB3ToRGB8Table[8] = {0x00,0x24,0x49,0x6D,0x92,0xB6,0xDB,0xFF};
const Word RGB4ToRGB5Table[16] = {
	0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,
	0x11,0x13,0x15,0x17,0x19,0x1B,0x1D,0x1F};
const Word RGB4ToRGB6Table[16] = {
	0x00,0x04,0x08,0x0D,0x11,0x15,0x19,0x1D,
	0x22,0x26,0x2A,0x2E,0x32,0x37,0x3B,0x3F};
const Word RGB4ToRGB8Table[16] = {
	0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
	0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};
const Word RGB5ToRGB6Table[32] = {
	0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,
	0x10,0x12,0x14,0x16,0x18,0x1A,0x1C,0x1E,
	0x21,0x23,0x25,0x27,0x29,0x2B,0x2D,0x2F,
	0x31,0x33,0x35,0x37,0x39,0x3B,0x3D,0x3F};

/**********************************

	Create a new Image_t struct and create default
	buffers

**********************************/

Word BURGERCALL ImageInit(Image_t *Output,Word Width,Word Height,ImageTypes_e Depth)
{
	Word32 Size;

	if (Width && Height) {				/* Valid size? */
		switch (Depth) {				/* Validate the image type */
		case IMAGE332:
		case IMAGE8ALPHA:
		case IMAGE8_PAL_ALPHA_PAL:
		case IMAGE8_PAL_ALPHA:
		case IMAGE8_PAL:
		case IMAGE4444:
		case IMAGE1555:
		case IMAGE555:
		case IMAGE565:
		case IMAGE888:
		case IMAGE8888:

			Output->Width = Width;		/* Init the struct */
			Output->Height = Height;
			Output->DataType = Depth;	/* Set the type */
			Output->RowBytes = Width*((Depth+7)>>3);	/* Bytes per row */
			Size = (Word32)Output->RowBytes*Height;	/* Size for bitmap */
			Output->ImagePtr = (Word8 *)AllocAPointerClear(Size);
			if (Output->ImagePtr) {
				Output->PalettePtr = 0;		/* No palette */
				Output->AlphaPtr = 0;		/* No Alpha channel */

				/* Create the 256 color palette for 8 bit indexed images */

				switch (Depth) {			/* True color images are done now */
				case IMAGE8_PAL:
				case IMAGE8_PAL_ALPHA_PAL:
				case IMAGE8_PAL_ALPHA:
					Output->PalettePtr = (Word8 *)AllocAPointerClear(768);
					if (!Output->PalettePtr) {	/* Palette ok? */
						goto Darn;
					}
				}

				/* Create an 8 bit alpha channel */

				switch (Depth) {
				case IMAGE8_PAL_ALPHA_PAL:
					Size = 256;					/* The alpha channel is 256 bytes because it maps to the palette */
				case IMAGE8ALPHA:
				case IMAGE8_PAL_ALPHA:
					Output->AlphaPtr = (Word8 *)AllocAPointer(Size);	/* Make the channel */
					if (!Output->AlphaPtr) {
						goto Darn;
					}
					FastMemSet(Output->AlphaPtr,0xFF,Size);			/* Set opaque alpha */
				}
				return FALSE;				/* No errors */
Darn:;
				DeallocAPointer(Output->ImagePtr);	/* Release image */
				DeallocAPointer(Output->PalettePtr);
			}
		}
	}
	return TRUE;		/* No good */
}

/**********************************

	Create a new Image_t structure and create default
	buffers

**********************************/

Image_t * BURGERCALL ImageNew(Word Width,Word Height,ImageTypes_e Depth)
{
	Image_t *Output;
	Output = (Image_t *)AllocAPointer(sizeof(Image_t));
	if (Output) {
		if (!ImageInit(Output,Width,Height,Depth)) {		/* Create the structure */
			return Output;				/* No errors */
		}
		DeallocAPointer(Output);
	}
	return 0;		/* Oh oh... */
}

/**********************************

	Make a COPY of the contents
	of another Image_t

**********************************/

Word BURGERCALL ImageInitCopy(Image_t *Output,const Image_t *Input)
{
	const Word8 *Src;
	Word8 *Dest;
	Word Size,Height;
	
	/* Allocate the memory for the new image */
	
	if (!ImageInit(Output,Input->Width,Input->Height,Input->DataType)) {
		
		/* Do I have a palette? */
		
		if (Input->PalettePtr) {

			/* Copy the palette (It's hardcoded to be 768 bytes) */

			FastMemCpy(Output->PalettePtr,Input->PalettePtr,768);		
		}
		
		/* Get the size of a scan line in bytes */
		
		Size = Input->Width*(((Word)Input->DataType+7)>>3);	/* Size of a single scan line */
		if (Size) {
			
			/* Alpha channel? */
			
			if (Input->AlphaPtr) {				
				switch (Input->DataType) {
				case IMAGE8_PAL_ALPHA_PAL:
					FastMemCpy(Output->AlphaPtr,Input->AlphaPtr,256);
					break;
				case IMAGE8ALPHA:
				case IMAGE8_PAL_ALPHA:
					Height = Input->Height;
					Src = Input->AlphaPtr;
					Dest = Output->AlphaPtr;
					do {
						FastMemCpy(Dest,Src,Size);
						Dest+=Output->RowBytes;
						Src+=Input->RowBytes;
					} while (--Height);
				}
			}
			
			/* Bitmap */
			
			if (Input->ImagePtr) {
				Height = Input->Height;
				Src = Input->ImagePtr;
				Dest = Output->ImagePtr;
				do {
					FastMemCpy(Dest,Src,Size);
					Dest+=Output->RowBytes;
					Src+=Input->RowBytes;
				} while (--Height);
			}
		}
		return FALSE;			/* I did it! */
	}
	return TRUE;				/* Error (Probably out of memory) */
}

/**********************************

	Create a new Image_t structure and initialize it with a COPY of the contents
	of another Image_t

**********************************/

Image_t * BURGERCALL ImageNewCopy(const Image_t *Input)
{
	Image_t *Output;
	Output = (Image_t *)AllocAPointer(sizeof(Image_t));
	if (Output) {
		if (!ImageInitCopy(Output,Input)) {		/* Create the structure */
			return Output;				/* No errors */
		}
		DeallocAPointer(Output);
	}
	return 0;		/* Oh oh... */
}

/**********************************

	Delete an allocated Image_t struct

**********************************/

void BURGERCALL ImageDelete(Image_t *Input)
{
	if (Input) {		/* Valid input */
		ImageDestroy(Input);		/* Destroy the input */
		DeallocAPointer(Input);		/* Delete the input */
	}
}

/**********************************

	Dispose of the contents of an image structure

**********************************/

void BURGERCALL ImageDestroy(Image_t *ImagePtr)
{
	if (ImagePtr) {
		DeallocAPointer(ImagePtr->ImagePtr);	/* Dispose of the memory */
		DeallocAPointer(ImagePtr->PalettePtr);	/* Is there a palette? */
		DeallocAPointer(ImagePtr->AlphaPtr);	/* Is there an alpha channel? */
		FastMemSet(ImagePtr,0,sizeof(Image_t));		/* Clear out the buffer */
	}
}

/**********************************

	Flip an image upside down

**********************************/

void BURGERCALL ImageVerticalFlip(Image_t *ImagePtr)
{
	Word Width;
	Word Height;
	Word Pixels;
	Word8 *BufferPtr;
	Word8 *BufferPtr2;

	Width = ImagePtr->Width;		/* Cache all the flags */
	Height = ImagePtr->Height;
	Pixels = (ImagePtr->DataType+7)>>3;	/* Round the bits to bytes */
	BufferPtr = ImagePtr->ImagePtr;	/* Image pointer */
	if (!Pixels || !Width || Height<2 || !BufferPtr) {	/* Invalid? */
		/* Single pixel height can't be flipped */
		return;
	}

	Width = Width*Pixels;		/* Scan line in BYTES */
	BufferPtr2 = &BufferPtr[(Height-1)*(Word32)ImagePtr->RowBytes];	/* Bottommost line */
	Height = Height>>1;		/* Divide height by 2 */
	do {
		Word Count;
		Count = 0;		/* Init byte index */
		do {
			Word8 Temp;
			Temp = BufferPtr[Count];	/* Swap the two pixels */
			BufferPtr[Count] = BufferPtr2[Count];
			BufferPtr2[Count] = Temp;
		} while (++Count<Width);		/* All done? */
		BufferPtr = BufferPtr+ImagePtr->RowBytes;		/* Next line down */
		BufferPtr2 = BufferPtr2-ImagePtr->RowBytes;		/* Next line up */
	} while (--Height);		/* All done? */

	if (ImagePtr->AlphaPtr) {
		switch (ImagePtr->DataType) {
		case IMAGE8_PAL_ALPHA_PAL:
		case IMAGE8_PAL_ALPHA:
			Width = ImagePtr->Width;		/* Scan line in BYTES */
			Height = ImagePtr->Height;
			BufferPtr = ImagePtr->AlphaPtr;
			BufferPtr2 = &BufferPtr[(Height-1)*(Word32)Width];	/* Bottommost line */
			Height = Height>>1;		/* Divide height by 2 */
			do {
				Word Count;
				Count = 0;		/* Init byte index */
				do {
					Word8 Temp;
					Temp = BufferPtr[Count];	/* Swap the two pixels */
					BufferPtr[Count] = BufferPtr2[Count];
					BufferPtr2[Count] = Temp;
				} while (++Count<Width);		/* All done? */
				BufferPtr = BufferPtr+Width;		/* Next line down */
				BufferPtr2 = BufferPtr2-Width;		/* Next line up */
			} while (--Height);		/* All done? */
		}
	}
}

/**********************************

	Flip an image horizontally

**********************************/

void BURGERCALL ImageHorizontalFlip(Image_t *ImagePtr)
{
	Word Width;		/* Width of a line in bytes */
	Word Height;	/* Height of the shape */
	Word Pixels;	/* Bytes per pixel */
	Word Left,Right;		/* Swap offsets */
	Word8 *BufferPtr;	/* Pointer to the data */

	Width = ImagePtr->Width;		/* Cache all the flags */
	Height = ImagePtr->Height;
	Pixels = (ImagePtr->DataType+7)>>3;	/* Round the bits to bytes */
	BufferPtr = ImagePtr->ImagePtr;	/* Image pointer */
	if (!Pixels || Width<2 || !Height || !BufferPtr) {	/* Invalid? */
		/* Single pixel width can't be flipped */
		return;
	}
	Width = Width*Pixels;	/* Offset to the end of the line */
	switch (Pixels) {		/* Use a different for each pixel width */

	/* Swap an image in bytes */

	default:				/* Bytes */
		do {
			Left = 0;		/* Init byte index */
			Right = Width-1;
			do {
				Word8 Temp;
				Temp = BufferPtr[Left];	/* Swap the left and right pixels */
				BufferPtr[Left] = BufferPtr[Right];
				BufferPtr[Right] = Temp;
				++Left;		/* Move towards the middle */
				--Right;
			} while (Left<Right);		/* All done? */
			BufferPtr = BufferPtr+ImagePtr->RowBytes;		/* Next line down */
		} while (--Height);		/* All done? */
		break;

	/* Swap an image in shorts */

	case 2:
		do {
			Left = 0;		/* Init byte index */
			Right = Width-2;
			do {
				Word16 Temp;
				Temp = ((Word16 *)(BufferPtr+Left))[0];	/* Swap the left and right pixels */
				((Word16 *)(BufferPtr+Left))[0] = ((Word16 *)(BufferPtr+Right))[0];
				((Word16 *)(BufferPtr+Right))[0] = Temp;
				Left=Left+2;		/* Move towards the middle */
				Right=Right-2;
			} while (Left<Right);		/* All done? */
			BufferPtr = BufferPtr+ImagePtr->RowBytes;		/* Next line down */
		} while (--Height);		/* All done? */
		break;

	/* Swap an image in 3 byte tripletts */

	case 3:
		do {
			Left = 0;		/* Init byte index */
			Right = Width-3;
			do {
				Word8 Temp;
				Temp = BufferPtr[Left];	/* Swap the left and right pixels */
				BufferPtr[Left] = BufferPtr[Right];
				BufferPtr[Right] = Temp;
				Temp = BufferPtr[Left+1];	/* Swap the left and right pixels */
				BufferPtr[Left+1] = BufferPtr[Right+1];
				BufferPtr[Right+1] = Temp;
				Temp = BufferPtr[Left+2];	/* Swap the left and right pixels */
				BufferPtr[Left+2] = BufferPtr[Right+2];
				BufferPtr[Right+2] = Temp;
				Left=Left+3;		/* Move towards the middle */
				Right=Right-3;
			} while (Left<Right);		/* All done? */
			BufferPtr = BufferPtr+ImagePtr->RowBytes;		/* Next line down */
		} while (--Height);		/* All done? */
		break;

	/* Swap an image in longs */

	case 4:
		do {
			Left = 0;		/* Init byte index */
			Right = Width-4;
			do {
				Word32 Temp;
				Temp = ((Word32 *)(BufferPtr+Left))[0];	/* Swap the left and right pixels */
				((Word32 *)(BufferPtr+Left))[0] = ((Word32 *)(BufferPtr+Right))[0];
				((Word32 *)(BufferPtr+Right))[0] = Temp;
				Left=Left+4;		/* Move towards the middle */
				Right=Right-4;
			} while (Left<Right);		/* All done? */
			BufferPtr = BufferPtr+ImagePtr->RowBytes;		/* Next line down */
		} while (--Height);		/* All done? */
		break;
	}

	/* Horizontally flip the alpha channel */

	if (ImagePtr->AlphaPtr) {
		switch (ImagePtr->DataType) {
		case IMAGE8_PAL_ALPHA_PAL:
		case IMAGE8_PAL_ALPHA:
			Width = ImagePtr->Width;		/* Scan line in BYTES */
			Height = ImagePtr->Height;
			BufferPtr = ImagePtr->AlphaPtr;
			do {
				Left = 0;		/* Init byte index */
				Right = Width-1;
				do {
					Word8 Temp;
					Temp = BufferPtr[Left];	/* Swap the left and right pixels */
					BufferPtr[Left] = BufferPtr[Right];
					BufferPtr[Right] = Temp;
					++Left;		/* Move towards the middle */
					--Right;
				} while (Left<Right);		/* All done? */
				BufferPtr = BufferPtr+Width;		/* Next line down */
			} while (--Height);		/* All done? */
		}
	}
}

/**********************************

	Remove colors 0 and 255 from an 8 bit image

**********************************/

void BURGERCALL ImageRemove0And255(Image_t *ImagePtr)
{
	Word8 *PalPtr;		/* Pointer to the palette to replace */
	Word8 Lookup[256];	/* Remap table */
	Word i;

	PalPtr = ImagePtr->PalettePtr;	/* Get the palette pointer */
	if (PalPtr) {
		switch (ImagePtr->DataType) {
		case IMAGE8_PAL:
		case IMAGE8_PAL_ALPHA_PAL:
		case IMAGE8_PAL_ALPHA:
			i = 1;		/* Init the lookup table for colors 1-254 */
			do {
				Lookup[i] = static_cast<Word8>(i);
			} while (++i<255);

			/* Remap 0 and 255 into colors 1-254 */
			Lookup[0] = static_cast<Word8>(PaletteFindColorIndex(PalPtr+3,PalPtr[0],PalPtr[1],PalPtr[2],254)+1);
			Lookup[255] = static_cast<Word8>(PaletteFindColorIndex(PalPtr+3,PalPtr[765],PalPtr[766],PalPtr[767],254)+1);
			ImageRemapIndexed(ImagePtr,Lookup);		/* Remap the image */
		}
	}
}

/**********************************

	Remap an 8 bit indexed image from one palette
	to another.

**********************************/

void BURGERCALL ImageRepaletteIndexed(Image_t *ImagePtr,const Word8 *PalettePtr)
{
	Word8 *PalPtr;		/* Pointer to the palette to replace */
	Word8 Lookup[256];	/* Remap table */

	PalPtr = ImagePtr->PalettePtr;	/* Get the palette pointer */
	if (PalPtr) {
		switch (ImagePtr->DataType) {
		case IMAGE8_PAL:
		case IMAGE8_PAL_ALPHA_PAL:
		case IMAGE8_PAL_ALPHA:
			PaletteMakeRemapLookup(Lookup,PalettePtr,PalPtr);	/* Create lookup table */
			ImageRemapIndexed(ImagePtr,Lookup);		/* Remap the image */
			FastMemCpy(PalPtr,PalettePtr,768);			/* Copy the palette */
		}
	}
}

/**********************************

	Remap an 8 bit image to another color
	set by using a precalculated table

**********************************/

void BURGERCALL ImageRemapIndexed(Image_t *ImagePtr,const Word8 *RemapPtr)
{
	Word8 *Buffer;
	Word Width;
	Word Height;

	Buffer = ImagePtr->ImagePtr;
	Width = ImagePtr->Width;
	Height = ImagePtr->Height;

	if (Buffer && Width && Height) {
		switch (ImagePtr->DataType) {
		case IMAGE8_PAL:
		case IMAGE8_PAL_ALPHA_PAL:
		case IMAGE8_PAL_ALPHA:
			do {
				Word8 *Buffer2;
				Buffer2 = Buffer;
				do {
					Buffer2[0] = RemapPtr[Buffer2[0]];	/* Remap a pixel */
					++Buffer2;				/* Next byte */
				} while (--Width);		/* Count down */
				Width = ImagePtr->Width;
				Buffer = Buffer+ImagePtr->RowBytes;
			} while (--Height);
		}
	}
}

/**********************************

	Given either a 15, 16, 24 or 32 bit Image_t struct,
	Swap the RGB values from BGR to RGB (I.E. only swap the
	Red and the Blue but leave Green and Alpha alone)

**********************************/

void BURGERCALL ImageSwapBGRToRGB(Image_t *ImagePtr)
{
	Word Height;
	Word8 *Buffer;
	Word8 *Buffer2;
	Word PixelCount;

	Buffer2 = ImagePtr->ImagePtr;
	if (Buffer2) {	/* Invalid input? */
		Height = ImagePtr->Height;
		PixelCount = ImagePtr->Width;
		if (Height && PixelCount) {
			do {
				Buffer = Buffer2;
				switch (ImagePtr->DataType) {
				default:
					return;

				case IMAGE332:
					do {
						Word Temp;

						Temp = Buffer[0];
						Buffer[0] = (Word8)((Temp&0x1C) + (RGB2ToRGB3Table[Temp&0x3]<<6) + ((Temp>>6)&0x3));
						++Buffer;
					} while (--PixelCount);
					break;

				case IMAGE1555:
					do {
						Word Temp;

						Temp = ((Word16 *)Buffer)[0];		/* Get the 16 bit data */
						((Word16 *)Buffer)[0] = (Word16)((Temp&0x83E0)+((Temp<<10)&0x7C00)+((Temp>>10)&0x1F));
						Buffer=Buffer+2;				/* Next pixel */
					} while (--PixelCount);
					break;

				case IMAGE4444:
					do {
						Word Temp;

						Temp = ((Word16 *)Buffer)[0];		/* Get the 16 bit data */
						((Word16 *)Buffer)[0] = (Word16)((Temp&0xF0F0)+((Temp<<8)&0x0F00)+((Temp>>8)&0x0F));
						Buffer=Buffer+2;				/* Next pixel */
					} while (--PixelCount);
					break;

				case IMAGE555:			/* Handle 5:5:5 RGB */
					do {
						Word Temp;

						Temp = ((Word16 *)Buffer)[0];		/* Get the 16 bit data */
						((Word16 *)Buffer)[0] = (Word16)((Temp&0x3E0)+((Temp<<10)&0x7C00)+((Temp>>10)&0x1F));
						Buffer=Buffer+2;				/* Next pixel */
					} while (--PixelCount);
					break;

				case IMAGE565:			/* Handle 5:6:5 RGB */
					do {
						Word Temp;

						Temp = ((Word16 *)Buffer)[0];		/* Get the 16 bit data */
						((Word16 *)Buffer)[0] = (Word16)((Temp&0x7E0)+((Temp<<11)&0xF800)+((Temp>>11)&0x1F));
						Buffer=Buffer+2;				/* Next pixel */
					} while (--PixelCount);
					break;

				case IMAGE888:			/* Handle 8:8:8 RGB */
					do {
						Word8 Temp;
						Temp = Buffer[0];		/* Save off the Red value */
						Buffer[0] = Buffer[2];	/* Store the current Blue in Red */
						Buffer[2] = Temp;		/* Save Red in the blue value */
						Buffer = Buffer+3;
					} while (--PixelCount);		/* All done? */
					break;

				case IMAGE8888:			/* Handle 8:8:8:8 ARGB */
					do {
						Word8 Temp;
#if defined(__LITTLEENDIAN__)
						Temp = Buffer[0];		/* Save off the Red value */
						Buffer[0] = Buffer[2];	/* Store the current Blue in Red */
						Buffer[2] = Temp;		/* Save Red in the blue value */
#else
						Temp = Buffer[1];		/* Save off the Red value */
						Buffer[1] = Buffer[3];	/* Store the current Blue in Red */
						Buffer[3] = Temp;		/* Save Red in the blue value */
#endif
						Buffer = Buffer+4;
					} while (--PixelCount);		/* All done? */
				}
				Buffer2 = Buffer2+ImagePtr->RowBytes;
				PixelCount = ImagePtr->Width;
			} while (--Height);
		}
	}
}

/**********************************

	Before saving an Image_t structure as a picture file,
	I will first check it for validity

**********************************/

Word BURGERCALL ImageValidateToSave(Image_t *ImagePtr)
{
	Word i;
	char *BadNews;
	if (!ImagePtr->Width) {
		goto AbortWidth0;
	}
	if (!ImagePtr->Height) {
		goto AbortHeight0;
	}
	if (!ImagePtr->ImagePtr) {
		goto AbortNoImage;
	}
	i = ImagePtr->DataType;
	if (i!=IMAGE8_PAL && i!=IMAGE555 && i!=IMAGE565 && i!=IMAGE888 && i!=IMAGE8888) {	/* Must be 8,16 or 24 bits per pixel */
		NonFatal("Image is %u, not 8, 15, 16, 24 or 32 bits deep!\n",i);
		return TRUE;
	}
	if (i==IMAGE8_PAL && !ImagePtr->PalettePtr) {
		goto AbortNoPalette;
	}
	return FALSE;

AbortWidth0:
	BadNews = "Can't save a zero width image!\n";
	goto Abort;
AbortHeight0:
	BadNews = "Can't save a zero height image!\n";
	goto Abort;
AbortNoImage:
	BadNews = "No pixel data in image structure!\n";
	goto Abort;
AbortNoPalette:
	BadNews = "Palette required for 8 bit images\n";
Abort:
	NonFatal(BadNews);
	return TRUE;
}

/**********************************

	The following routines allow conversion
	of one Image_t format to another
	They are quick and dirty hacks and are
	not meant to win a prize.

**********************************/

/**********************************

	Convert an Image_t into 332 RGB format
	I assume that the Output Image_t is already an IMAGE332
	format shape and all entries are valid.
	I will scale the texture to fit the new texture

**********************************/

Word BURGERCALL ImageStore332(Image_t *Output,const Image_t *Input)
{
	Word32 StepX,StepY;
	Word32 DeltaX,DeltaY;
	Word8 *BufferPtr;
	Word8 *SrcPtr;
	Word8 *DestPtr;
	Word Width,Height;

	if (Output && (Output->DataType==IMAGE332) && Output->Width && Output->Height &&
		Output->ImagePtr && Input->Width && Input->Height) {
		StepX = ((Word32)Input->Width<<16)/Output->Width;
		StepY = ((Word32)Input->Height<<16)/Output->Height;
		BufferPtr = Output->ImagePtr;
		Height = Output->Height;
		DeltaY = 0;
		switch (Input->DataType) {
		case IMAGE332:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					DestPtr[0] = SrcPtr[DeltaX>>16];
					DeltaX+=StepX;
					++DestPtr;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;

		case IMAGE8_PAL_ALPHA_PAL:
		case IMAGE8_PAL_ALPHA:
		case IMAGE8_PAL:
			{
				Word8 *PalPtr;
				PalPtr = Input->PalettePtr;
				if (PalPtr) {
					do {
						DeltaX = 0;
						DestPtr = BufferPtr;
						SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
						Width = Output->Width;
						do {
							Word Temp;
							Temp = (Word)(SrcPtr[DeltaX>>16])*3;
							DestPtr[0] = (PalPtr[Temp]&0xE0)+((PalPtr[Temp+1]>>3)&0x1C)+(PalPtr[Temp+2]>>6);
							DeltaX+=StepX;
							++DestPtr;
						} while (--Width);
						DeltaY += StepY;
						BufferPtr += Output->RowBytes;
					} while (--Height);
					return FALSE;
				}
			}
			break;
		case IMAGE4444:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word Temp;
					Temp = ((Word16*)(SrcPtr))[DeltaX>>16];
					DestPtr[0] = static_cast<Word8>(((Temp>>4)&0xE0) + ((Temp>>3)&0x1C) + ((Temp>>2)&0x3));
					DeltaX+=StepX;
					++DestPtr;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
		case IMAGE1555:
		case IMAGE555:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word Temp;
					Temp = ((Word16*)(SrcPtr))[DeltaX>>16];
					DestPtr[0] = static_cast<Word8>(((Temp>>7)&0xE0) + ((Temp>>5)&0x1C) + ((Temp>>3)&0x3));
					DeltaX+=StepX;
					++DestPtr;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
		case IMAGE565:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word Temp;
					Temp = ((Word16*)(SrcPtr))[DeltaX>>16];
					DestPtr[0] = static_cast<Word8>(((Temp>>8)&0xE0) + ((Temp>>6)&0x1C) + ((Temp>>3)&0x3));
					DeltaX+=StepX;
					++DestPtr;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
		case IMAGE888:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word8 *TempPtr;
					TempPtr = &SrcPtr[(DeltaX>>16)*3];
					DestPtr[0] = (TempPtr[0]&0xE0) + ((TempPtr[1]>>3)&0x1C) + (TempPtr[2]>>6);
					DeltaX+=StepX;
					++DestPtr;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
		case IMAGE8888:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word32 Temp;
					Temp = ((Word32*)(SrcPtr))[DeltaX>>16];
					DestPtr[0] = (Word8)(((Temp>>16)&0xE0) + ((Temp>>11)&0x1C) + ((Temp>>6)&0x3));
					DeltaX+=StepX;
					++DestPtr;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
		}
	}
	return TRUE;
}
/**********************************

	Convert an Image_t into 8 Bit indexed format
	I assume that the Output Image_t is already an IMAGE8_PAL
	format shape and all entries are valid.
	I will scale the texture to fit the new texture

**********************************/

Word BURGERCALL ImageStore8Pal(Image_t *Output,const Image_t *Input)
{
	Word32 StepX,StepY;
	Word32 DeltaX,DeltaY;
	Word8 *BufferPtr;
	Word8 *SrcPtr;
	Word8 *DestPtr;
	Word Width,Height;

	if (Output && (Output->DataType==IMAGE8_PAL) && Output->Width && Output->Height &&
		Output->ImagePtr && Input->Width && Input->Height) {
		StepX = ((Word32)Input->Width<<16)/Output->Width;
		StepY = ((Word32)Input->Height<<16)/Output->Height;
		BufferPtr = Output->ImagePtr;
		Height = Output->Height;
		DeltaY = 0;
		switch (Input->DataType) {
		case IMAGE8_PAL_ALPHA_PAL:
		case IMAGE8_PAL_ALPHA:
		case IMAGE8_PAL:
			{
				Word8 *PalPtr;
				PalPtr = Input->PalettePtr;
				if (PalPtr) {
					FastMemCpy(Output->PalettePtr,PalPtr,768);
					do {
						DeltaX = 0;
						DestPtr = BufferPtr;
						SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
						Width = Output->Width;
						do {
							DestPtr[0] = SrcPtr[DeltaX>>16];
							DeltaX+=StepX;
							++DestPtr;
						} while (--Width);
						DeltaY += StepY;
						BufferPtr += Output->RowBytes;
					} while (--Height);
					return FALSE;
				}
			}
			break;
		case IMAGE4444:
		case IMAGE332:
		case IMAGE1555:
		case IMAGE555:
		case IMAGE565:
		case IMAGE888:
		case IMAGE8888:
			break;
		}
	}
	return TRUE;
}

/**********************************

	Convert an Image_t into 4444 ARGB format
	I assume that the Output Image_t is already an IMAGE4444
	format shape and all entries are valid.
	I will scale the texture to fit the new texture

**********************************/

Word BURGERCALL ImageStore4444(Image_t *Output,const Image_t *Input)
{
	Word32 StepX,StepY;
	Word32 DeltaX,DeltaY;
	Word8 *BufferPtr;
	Word8 *SrcPtr;
	Word8 *DestPtr;
	Word Width,Height;

	if (Output && (Output->DataType==IMAGE4444) && Output->Width && Output->Height &&
		Output->ImagePtr && Input->Width && Input->Height) {
		StepX = ((Word32)Input->Width<<16)/Output->Width;
		StepY = ((Word32)Input->Height<<16)/Output->Height;
		BufferPtr = Output->ImagePtr;
		Height = Output->Height;
		DeltaY = 0;
		switch (Input->DataType) {
		case IMAGE332:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word Temp;
					Temp = SrcPtr[DeltaX>>16];
					((Word16 *)DestPtr)[0] = static_cast<Word16>(0xF000 + (RGB3ToRGB4Table[Temp>>5]<<8) + (RGB3ToRGB4Table[(Temp>>2)&0x7]<<4) + RGB2ToRGB4Table[Temp&3]);
					DeltaX+=StepX;
					DestPtr+=2;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;

		case IMAGE4444:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					((Word16 *)DestPtr)[0] = ((Word16 *)SrcPtr)[DeltaX>>16];
					DeltaX+=StepX;
					DestPtr+=2;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;

		case IMAGE8ALPHA:
		case IMAGE8_PAL_ALPHA_PAL:
		case IMAGE8_PAL_ALPHA:
		case IMAGE8_PAL:
		case IMAGE1555:
		case IMAGE555:
		case IMAGE565:
		case IMAGE888:
		case IMAGE8888:
			break;
		}
	}
	return TRUE;
}

/**********************************

	Convert an Image_t into 555 RGB format
	I assume that the Output Image_t is already an IMAGE555
	format shape and all entries are valid.
	I will scale the texture to fit the new texture

**********************************/

Word BURGERCALL ImageStore555(Image_t *Output,const Image_t *Input)
{
	Word32 StepX,StepY;
	Word32 DeltaX,DeltaY;
	Word8 *BufferPtr;
	Word8 *SrcPtr;
	Word8 *DestPtr;
	Word Width,Height;

	if (Output && (Output->DataType==IMAGE555) && Output->Width && Output->Height &&
		Output->ImagePtr && Input->Width && Input->Height) {
		StepX = ((Word32)Input->Width<<16)/Output->Width;
		StepY = ((Word32)Input->Height<<16)/Output->Height;
		BufferPtr = Output->ImagePtr;
		Height = Output->Height;
		DeltaY = 0;
		switch (Input->DataType) {
		case IMAGE8_PAL_ALPHA_PAL:
		case IMAGE8_PAL_ALPHA:
		case IMAGE8_PAL:
			{
				Word8 *PalPtr;
				PalPtr = Input->PalettePtr;
				if (PalPtr) {
					do {
						DeltaX = 0;
						DestPtr = BufferPtr;
						SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
						Width = Output->Width;
						do {
							Word Temp;
							Temp = (Word)(SrcPtr[DeltaX>>16])*3;
							((Word16 *)DestPtr)[0] = static_cast<Word16>((((Word)PalPtr[Temp]<<7)&0x7C00)+(((Word)PalPtr[Temp+1]<<2)&0x3E0)+(PalPtr[Temp+2]>>3));
							DeltaX+=StepX;
							DestPtr+=2;
						} while (--Width);
						DeltaY += StepY;
						BufferPtr += Output->RowBytes;
					} while (--Height);
					return FALSE;
				}
			}
			break;
		case IMAGE332:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word Temp;
					Temp = SrcPtr[DeltaX>>16];
					((Word16 *)DestPtr)[0] = static_cast<Word16>((RGB3ToRGB5Table[((Temp>>5)&0x7)]<<10) + (RGB3ToRGB5Table[((Temp>>2)&0x7)]<<5) + RGB2ToRGB5Table[(Temp&0x3)]);
					DeltaX+=StepX;
					DestPtr+=2;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;

		case IMAGE4444:
		case IMAGE8ALPHA:
			break;
			
		case IMAGE1555:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word Temp;
					Temp = ((Word16 *)SrcPtr)[DeltaX>>16];
					((Word16 *)DestPtr)[0] = static_cast<Word16>(Temp&0x7FFF);
					DeltaX+=StepX;
					DestPtr+=2;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;

		case IMAGE555:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word16 Temp;
					Temp = ((Word16 *)SrcPtr)[DeltaX>>16];
					((Word16 *)DestPtr)[0] = Temp;
					DeltaX+=StepX;
					DestPtr+=2;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;

		case IMAGE565:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word Temp;
					Temp = ((Word16 *)SrcPtr)[DeltaX>>16];
					((Word16 *)DestPtr)[0] = static_cast<Word16>(((Temp>>1)&0x7FE0) + (Temp&0x1F));
					DeltaX+=StepX;
					DestPtr+=2;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
		case IMAGE888:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word8 *Temp;
					Temp = &SrcPtr[(DeltaX>>16)*3];
					((Word16 *)DestPtr)[0] = ((Temp[0]&0xF8)<<(10-3)) + ((Temp[1]&0xF8)<<(5-3)) + (Temp[2]>>3);
					DeltaX+=StepX;
					DestPtr+=2;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
			
		case IMAGE8888:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word8 *Temp;
					Temp = &SrcPtr[(DeltaX>>16)*4];
					((Word16 *)DestPtr)[0] = ((Temp[0]&0xF8)<<(10-3)) + ((Temp[1]&0xF8)<<(5-3)) + (Temp[2]>>3);
					DeltaX+=StepX;
					DestPtr+=2;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
		}
	}
	return TRUE;
}

/**********************************

	Convert an Image_t into 565 RGB format
	I assume that the Output Image_t is already an IMAGE565
	format shape and all entries are valid.
	I will scale the texture to fit the new texture

**********************************/

Word BURGERCALL ImageStore565(Image_t *Output,const Image_t *Input)
{
	Word32 StepX,StepY;
	Word32 DeltaX,DeltaY;
	Word8 *BufferPtr;
	Word8 *SrcPtr;
	Word8 *DestPtr;
	Word Width,Height;

	if (Output && (Output->DataType==IMAGE565) && Output->Width && Output->Height &&
		Output->ImagePtr && Input->Width && Input->Height) {
		StepX = ((Word32)Input->Width<<16)/Output->Width;
		StepY = ((Word32)Input->Height<<16)/Output->Height;
		BufferPtr = Output->ImagePtr;
		Height = Output->Height;
		DeltaY = 0;
		switch (Input->DataType) {
		case IMAGE8_PAL_ALPHA_PAL:
		case IMAGE8_PAL_ALPHA:
		case IMAGE8_PAL:
			{
				Word8 *PalPtr;
				PalPtr = Input->PalettePtr;
				if (PalPtr) {
					do {
						DeltaX = 0;
						DestPtr = BufferPtr;
						SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
						Width = Output->Width;
						do {
							Word Temp;
							Temp = (Word)(SrcPtr[DeltaX>>16])*3;
							((Word16 *)DestPtr)[0] = static_cast<Word16>((((Word)PalPtr[Temp]<<8)&0xF800)+(((Word)PalPtr[Temp+1]<<3)&0x7E0)+(PalPtr[Temp+2]>>3));
							DeltaX+=StepX;
							DestPtr+=2;
						} while (--Width);
						DeltaY += StepY;
						BufferPtr += Output->RowBytes;
					} while (--Height);
					return FALSE;
				}
			}
			break;

		case IMAGE332:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word Temp;
					Temp = SrcPtr[DeltaX>>16];
					((Word16 *)DestPtr)[0] = static_cast<Word16>((RGB3ToRGB5Table[((Temp>>5)&0x7)]<<11) + (RGB3ToRGB6Table[((Temp>>2)&0x7)]<<5) + RGB2ToRGB5Table[(Temp&0x3)]);
					DeltaX+=StepX;
					DestPtr+=2;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;

		case IMAGE4444:
		case IMAGE8ALPHA:
			break;
			
		case IMAGE1555:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word Temp;
					Temp = ((Word16 *)SrcPtr)[DeltaX>>16];
					((Word16 *)DestPtr)[0] = static_cast<Word16>(((Temp<<1)&0xFFC0) + (Temp&0x3F));
					DeltaX+=StepX;
					DestPtr+=2;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;

		case IMAGE555:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word Temp;
					Temp = ((Word16 *)SrcPtr)[DeltaX>>16];
					((Word16 *)DestPtr)[0] = static_cast<Word16>(((Temp<<1)&0xFFC0) + (Temp&0x3F));
					DeltaX+=StepX;
					DestPtr+=2;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;

		case IMAGE565:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word16 Temp;
					Temp = reinterpret_cast<Word16 *>(SrcPtr)[DeltaX>>16];
					reinterpret_cast<Word16 *>(DestPtr)[0] = Temp;
					DeltaX+=StepX;
					DestPtr+=2;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
		case IMAGE888:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word8 *Temp;
					Temp = &SrcPtr[(DeltaX>>16)*3];
					((Word16 *)DestPtr)[0] = ((Temp[0]&0xF8)<<(11-3)) + ((Temp[1]&0xFC)<<(5-2)) + (Temp[2]>>3);
					DeltaX+=StepX;
					DestPtr+=2;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
			
		case IMAGE8888:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word8 *Temp;
					Temp = &SrcPtr[(DeltaX>>16)*4];
					((Word16 *)DestPtr)[0] = ((Temp[0]&0xF8)<<(11-3)) + ((Temp[1]&0xFC)<<(5-2)) + (Temp[2]>>3);
					DeltaX+=StepX;
					DestPtr+=2;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
		}
	}
	return TRUE;
}

/**********************************

	Convert an Image_t into 1555 RGB format
	I assume that the Output Image_t is already an IMAGE1555
	format shape and all entries are valid.
	I will scale the texture to fit the new texture

**********************************/

Word BURGERCALL ImageStore1555(Image_t *Output,const Image_t *Input)
{
	Word32 StepX,StepY;
	Word32 DeltaX,DeltaY;
	Word8 *BufferPtr;
	Word8 *SrcPtr;
	Word8 *DestPtr;
	Word Width,Height;

	if (Output && (Output->DataType==IMAGE1555) && Output->Width && Output->Height &&
		Output->ImagePtr && Input->Width && Input->Height) {
		StepX = ((Word32)Input->Width<<16)/Output->Width;
		StepY = ((Word32)Input->Height<<16)/Output->Height;
		BufferPtr = Output->ImagePtr;
		Height = Output->Height;
		DeltaY = 0;
		switch (Input->DataType) {
		case IMAGE332:
		case IMAGE4444:
		case IMAGE8ALPHA:
		case IMAGE8_PAL_ALPHA_PAL:
		case IMAGE8_PAL_ALPHA:
		case IMAGE8_PAL:
			break;
		case IMAGE1555:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word16 Temp;
					Temp = reinterpret_cast<Word16 *>(SrcPtr)[DeltaX>>16];
					reinterpret_cast<Word16 *>(DestPtr)[0] = Temp;
					DeltaX+=StepX;
					DestPtr+=2;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
		case IMAGE555:
		case IMAGE565:
		case IMAGE888:
		case IMAGE8888:
			break;
		}
	}
	return TRUE;
}

/**********************************

	Convert an Image_t into 888 RGB format
	I assume that the Output Image_t is already an IMAGE888
	format shape and all entries are valid.
	I will scale the texture to fit the new texture

**********************************/

Word BURGERCALL ImageStore888(Image_t *Output,const Image_t *Input)
{
	Word32 StepX,StepY;
	Word32 DeltaX,DeltaY;
	Word8 *BufferPtr;
	Word8 *SrcPtr;
	Word8 *DestPtr;
	Word Width,Height;

	if (Output && (Output->DataType==IMAGE888) && Output->Width && Output->Height &&
		Output->ImagePtr && Input->Width && Input->Height) {
		StepX = ((Word32)Input->Width<<16)/Output->Width;
		StepY = ((Word32)Input->Height<<16)/Output->Height;
		BufferPtr = Output->ImagePtr;
		Height = Output->Height;
		DeltaY = 0;
		switch (Input->DataType) {
		case IMAGE8_PAL_ALPHA_PAL:
		case IMAGE8_PAL_ALPHA:
		case IMAGE8_PAL:
			{
				Word8 *PalPtr;
				PalPtr = Input->PalettePtr;
				if (PalPtr) {
					do {
						DeltaX = 0;
						DestPtr = BufferPtr;
						SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
						Width = Output->Width;
						do {
							Word Temp;
							Temp = (Word)(SrcPtr[DeltaX>>16])*3;
							DestPtr[0] = PalPtr[Temp];
							DestPtr[1] = PalPtr[Temp+1];
							DestPtr[2] = PalPtr[Temp+2];
							DeltaX+=StepX;
							DestPtr+=3;
						} while (--Width);
						DeltaY += StepY;
						BufferPtr += Output->RowBytes;
					} while (--Height);
					return FALSE;
				}
			}
			break;
		case IMAGE332:
		case IMAGE4444:
		case IMAGE8ALPHA:
			break;

		case IMAGE1555:
		case IMAGE555:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word Temp;
					Temp = ((Word16 *)SrcPtr)[DeltaX>>16];
					DestPtr[0] = RGB5ToRGB8Table[(Temp>>10)&0x1F];
					DestPtr[1] = RGB5ToRGB8Table[(Temp>>5)&0x1F];
					DestPtr[2] = RGB5ToRGB8Table[Temp&0x1F];
					DeltaX+=StepX;
					DestPtr+=3;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;

		case IMAGE565:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word Temp;
					Temp = ((Word16 *)SrcPtr)[DeltaX>>16];
					DestPtr[0] = RGB5ToRGB8Table[(Temp>>11)&0x1F];
					DestPtr[1] = RGB6ToRGB8Table[(Temp>>5)&0x3F];
					DestPtr[2] = RGB5ToRGB8Table[Temp&0x1F];
					DeltaX+=StepX;
					DestPtr+=3;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;

		case IMAGE888:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word8 *PalPtr;
					PalPtr = &SrcPtr[(DeltaX>>16)*3];
					DestPtr[0] = PalPtr[0];
					DestPtr[1] = PalPtr[1];
					DestPtr[2] = PalPtr[2];
					DeltaX+=StepX;
					DestPtr+=3;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
			
		case IMAGE8888:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word8 *PalPtr;
					PalPtr = &SrcPtr[(DeltaX>>16)*4];
					DestPtr[0] = PalPtr[0];
					DestPtr[1] = PalPtr[1];
					DestPtr[2] = PalPtr[2];
					DeltaX+=StepX;
					DestPtr+=3;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
		}
	}
	return TRUE;
}

/**********************************

	Convert an Image_t into 8888 RGB format
	I assume that the Output Image_t is already an IMAGE8888
	format shape and all entries are valid.
	I will scale the texture to fit the new texture

**********************************/

Word BURGERCALL ImageStore8888(Image_t *Output,const Image_t *Input)
{
	Word32 StepX,StepY;
	Word32 DeltaX,DeltaY;
	Word8 *BufferPtr;
	Word8 *SrcPtr;
	Word8 *DestPtr;
	Word Width,Height;

	if (Output && (Output->DataType==IMAGE8888) && Output->Width && Output->Height &&
		Output->ImagePtr && Input->Width && Input->Height) {
		StepX = ((Word32)Input->Width<<16)/Output->Width;
		StepY = ((Word32)Input->Height<<16)/Output->Height;
		BufferPtr = Output->ImagePtr;
		Height = Output->Height;
		DeltaY = 0;
		switch (Input->DataType) {
		case IMAGE8_PAL_ALPHA_PAL:
			{
				Word8 *PalPtr;
				Word8 *AlphaPtr;
				PalPtr = Input->PalettePtr;
				AlphaPtr = Input->AlphaPtr;
				if (PalPtr && AlphaPtr) {
					do {
						Word8 *Alpha;
						DeltaX = 0;
						DestPtr = BufferPtr;
						SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
						Alpha = AlphaPtr+((DeltaY>>16)*Input->RowBytes);
						Width = Output->Width;
						do {
							Word Temp;
							Temp = (Word)(SrcPtr[DeltaX>>16])*3;
							DestPtr[0] = PalPtr[Temp];
							DestPtr[1] = PalPtr[Temp+1];
							DestPtr[2] = PalPtr[Temp+2];
							DestPtr[3] = Alpha[DeltaX>>16];
							DeltaX+=StepX;
							DestPtr+=4;
						} while (--Width);
						DeltaY += StepY;
						BufferPtr += Output->RowBytes;
					} while (--Height);
					return FALSE;
				}
			}
			break;
		case IMAGE8_PAL_ALPHA:
			{
				Word8 *AlphaPtr;
				AlphaPtr = Input->AlphaPtr;
				if (AlphaPtr) {
					do {
						DeltaX = 0;
						DestPtr = BufferPtr;
						SrcPtr = AlphaPtr+((DeltaY>>16)*Input->RowBytes);
						Width = Output->Width;
						do {
							DestPtr[0] = 0;
							DestPtr[1] = 0;
							DestPtr[2] = 0;
							DestPtr[3] = SrcPtr[DeltaX>>16];
							DeltaX+=StepX;
							DestPtr+=4;
						} while (--Width);
						DeltaY += StepY;
						BufferPtr += Output->RowBytes;
					} while (--Height);
					return FALSE;
				}
			}
			break;
		case IMAGE8_PAL:
			{
				Word8 *PalPtr;
				PalPtr = Input->PalettePtr;
				if (PalPtr) {
					do {
						DeltaX = 0;
						DestPtr = BufferPtr;
						SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
						Width = Output->Width;
						do {
							Word Temp;
							Temp = (Word)(SrcPtr[DeltaX>>16])*3;
							DestPtr[0] = PalPtr[Temp];
							DestPtr[1] = PalPtr[Temp+1];
							DestPtr[2] = PalPtr[Temp+2];
							DestPtr[3] = 0xFF;
							DeltaX+=StepX;
							DestPtr+=4;
						} while (--Width);
						DeltaY += StepY;
						BufferPtr += Output->RowBytes;
					} while (--Height);
					return FALSE;
				}
			}
			break;
		case IMAGE555:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = (Input->ImagePtr+((DeltaY>>16)*Input->RowBytes));
				Width = Output->Width;
				do {
					Word Temp;
					Temp = ((Word16 *)SrcPtr)[DeltaX>>16];
					DestPtr[0] = RGB5ToRGB8Table[(Temp>>10)&0x1F];
					DestPtr[1] = RGB5ToRGB8Table[(Temp>>5)&0x1F];
					DestPtr[2] = RGB5ToRGB8Table[Temp&0x1F];
					DestPtr[3] = 0xFF;
					DeltaX+=StepX;
					DestPtr+=4;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;

		case IMAGE565:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word Temp;
					Temp = ((Word16 *)SrcPtr)[DeltaX>>16];
					DestPtr[0] = RGB5ToRGB8Table[(Temp>>11)&0x1F];
					DestPtr[1] = RGB6ToRGB8Table[(Temp>>5)&0x3F];
					DestPtr[2] = RGB5ToRGB8Table[Temp&0x1F];
					DestPtr[3] = 0xFF;
					DeltaX+=StepX;
					DestPtr+=4;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;

		case IMAGE888:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					Word8 *PalPtr;
					PalPtr = &SrcPtr[(DeltaX>>16)*3];
					DestPtr[0] = PalPtr[0];
					DestPtr[1] = PalPtr[1];
					DestPtr[2] = PalPtr[2];
					DestPtr[3] = 0xFF;
					DeltaX+=StepX;
					DestPtr+=4;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
		
		case IMAGE8888:
			do {
				DeltaX = 0;
				DestPtr = BufferPtr;
				SrcPtr = Input->ImagePtr+((DeltaY>>16)*Input->RowBytes);
				Width = Output->Width;
				do {
					((Word32 *)DestPtr)[0] = ((Word32 *)SrcPtr)[DeltaX>>16];
					DeltaX+=StepX;
					DestPtr+=4;
				} while (--Width);
				DeltaY += StepY;
				BufferPtr += Output->RowBytes;
			} while (--Height);
			return FALSE;
			
		case IMAGE332:
		case IMAGE4444:
		case IMAGE8ALPHA:
		case IMAGE1555:
			break;
		}
	}
	return TRUE;
}

/**********************************
	
	Take an 8888 image and for every pixel that matches r, g, b, set the alpha
	value accordingly.
	This is a quick way to create an alpha override.
	
	Note: I do not alter the alpha on pixels that do not match the RGB constants
	exactly

**********************************/

void BURGERCALL ImageColorKey8888(Image_t* SrcImagePtr,Word r,Word g,Word b,Word a)
{
	Word8* SrcPtr;
	Word32 Color;
	Word Width,Height;
	
	if (SrcImagePtr->DataType == IMAGE8888) {		/* Sanity Check */
		Height = SrcImagePtr->Height;					/* Number of iterations */
		if (Height && SrcImagePtr->Width) {
			SrcPtr = SrcImagePtr->ImagePtr;						 /* Store Ptr */
#if defined(__BIGENDIAN__)
			Color = (r<<24)|(g<<16)|(b<<8);					/* Get the RGB constants */
			do {
				Word32 *WorkPtr;
				WorkPtr = (Word32 *)SrcPtr;				/* Get the work pointer */
				Width = SrcImagePtr->Width;					/* Number of pixels to parse */
				do {
					if ((WorkPtr[0]&0xFFFFFF00)==Color) {	/* Color key match? */
						((Word8 *)WorkPtr)[3] = (Word8)a;		/* Store the new alpha */
					}
					++WorkPtr;
				} while (--Width);							/* The line is done? */
				SrcPtr += SrcImagePtr->RowBytes;			/* Next line */
			} while (--Height);
#else
			Color = (r)|(g<<8)|(b<<16);						/* Get the RGB constants */
			do {
				Word32 *WorkPtr;
				WorkPtr = (Word32 *)SrcPtr;				/* Get the work pointer */
				Width = SrcImagePtr->Width;					/* Number of pixels to parse */
				do {
					if ((WorkPtr[0]&0x00FFFFFF)==Color) {	/* Color key match? */
						((Word8 *)WorkPtr)[3] = (Word8)a;		/* Store the new alpha */
					}
					++WorkPtr;
				} while (--Width);							/* The line is done? */
				SrcPtr += SrcImagePtr->RowBytes;			/* Next line */
			} while (--Height);
#endif
		}
	}
}

/**********************************

	Take two "LIKE" Image_t structures, then clip out
	a rect that is the width and the height from the DstImagePtr
	structure, use the x,y as the origin point and cut it out from
	the source image.
	
	I assume that the dest image_t already has buffers allocated
	for the type that it is and that it is the same as the source image_t

	Return TRUE if an error had occured (Bounds check failure or unlike types)

**********************************/

Word BURGERCALL ImageSubImage(Image_t* Output,Word x,Word y,const Image_t* Input)
{
	Word Height;				/* Number of lines to copy */
	Word ByteSize;				/* Number of bytes per pixel/line */
	Word8 *SrcPtr;				/* Running source pointer */
	Word8 *DestPtr;				/* Running dest pointer */

	if ((Input->DataType == Output->DataType) &&		/* Must be like types */
		((x + Output->Width) <= Input->Width) &&		/* Sanity check */
		((y + Output->Height) <= Input->Height)) {
			
		Height = Output->Height;						/* Get the requested height */
		if (Height) {
			SrcPtr = &Input->ImagePtr[Input->RowBytes*y];		/* Word8 offset */
			ByteSize = ((Word)Input->DataType+7)>>3;	/* Bytes per pixel */
			SrcPtr = &SrcPtr[ByteSize*x];				/* Proper pixel address */
			ByteSize = ByteSize*Output->Width;			/* Pixels to copy */
			DestPtr = Output->ImagePtr;					/* Copy to here */
			do {
				FastMemCpy(DestPtr,SrcPtr,ByteSize);	/* Copy the line */
				DestPtr+=Output->RowBytes;				/* Next line */
				SrcPtr+=Input->RowBytes;				/* Next line */
			} while (--Height);							/* All done? */
		}
		return FALSE;	/* Copied fine! */
	}
	return TRUE;		/* Oh crap... */
}
	
/**********************************

	Generic data converter to ANY type

**********************************/

Word BURGERCALL ImageStore(Image_t *Output,const Image_t *Input)
{
	switch (Output->DataType) {
	case IMAGE332:
		return ImageStore332(Output,Input);		/* Convert to the format */
	case IMAGE8_PAL:
		return ImageStore8Pal(Output,Input);	/* Convert to the format */
	case IMAGE4444:
		return ImageStore4444(Output,Input);	/* Convert to the format */
	case IMAGE555:
		return ImageStore555(Output,Input);		/* Convert to the format */
	case IMAGE565:
		return ImageStore565(Output,Input);		/* Convert to the format */
	case IMAGE1555:
		return ImageStore1555(Output,Input);	/* Convert to the format */
	case IMAGE888:
		return ImageStore888(Output,Input);		/* Convert to the format */
	case IMAGE8888:
		return ImageStore8888(Output,Input);	/* Convert to the format */
	}
	return TRUE;			/* Oh oh, It's not supported yet!! */
}

