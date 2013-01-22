#include "ImImage.h"
#include <BREndian.hpp>
#include "MmMemory.h"
#include "ClStdLib.h"

/**********************************

	This will take an UNCOMPRESSED 24 bit BMP or a COMPRESSED or
	UNCOMPRESSED 8 bit BMP file.

**********************************/

typedef struct BMPHeader {	/* Header of a BMP file */
	char id[2];		/* Ascii BM */
	Word32 FileSize;	/* Length of the file */
	Word16 Reserved[2];
	Word32 HeaderSize;	/* Offset to the bitmap bits */

	Word32 InfoSize;	/* Size of the info struct */
	Word32 Width;		/* Width of the bit map in pixels */
	Word32 Height;	/* Height of the bit map in pixels */
	Word16 BitPlanes;	/* Number of bit planes (1) */
	Word16 BitDepth;		/* Pixel bit depth (8) */
	Word32 biCompression;	/* Data compression? */
	Word32 biSizeImage;
	Word32 biXPelsPerMeter;
	Word32 biYPelsPerMeter;
	Word32 biClrUsed;		/* Number of colors used */
	Word32 biClrImportant;
} BMPHeader;

#define BMPHeaderSize 54

/**********************************

	Unpack a single line of RLE data for a BMP file using RLE8 format.
	It's a little strange but here goes...

	Grab a byte, if it's not zero then use it as a repeat
		count and grab the next byte as the fill byte.

	If it is zero, then this is an escape code.
		0 : end of line
		1 : end of data
		2 : Delta, get two bytes for a pen move code (Not used)
		3-0xFF absolute run (But Word16 align the source pointer after run)

**********************************/

static INLINECALL const Word8 * BURGERCALL UnpackRLE(Word8 *Dest,const Word8 *Input,Word Length)
{
	Word Token;
	char *BadNews;

	if (Length) {	/* Shall I parse something? */
		Token = Input[0];		/* Get the token */
		if (Token) {			/* Is it a run length? */
RunToken:
			do {
				if (Length<Token) {		/* Too big? */
					goto Abort1;		/* Bad news! */
				}
				Length=Length-Token;	/* Remove the length */
				Word8 Fill = Input[1];	/* Get the fill byte */
				Input=Input+2;		/* Index past the data */
				do {
					Dest[0] = Fill;	/* Fill the buffer */
					++Dest;
				} while (--Token);	/* Run done? */
				Token = Input[0];	/* Next token */
			} while (Token);		/* Still runs? */
		}
		do {
			Token = Input[1];	/* Get the uncompressed run size */
			Input = Input+2;	/* Accept the two bytes */

			if (Token<3) {		/* End of file or line? */
				goto CleanUp;			/* Exit processing */
			}
			if (Token>Length) {		/* Too big? */
				goto AbortLargeRun;
			}
			Length=Length-Token;	/* Remove the length */
			Word Padding = Token&1;			/* 0 or 1 */
			do {
				Dest[0] = Input[0];	/* Copy literal data */
				++Input;
				++Dest;
			} while (--Token);
			Input+=Padding;		/* Add padding */
			Token = Input[0];
		} while (!Token);
		goto RunToken;
CleanUp:		
		if (Token==2) {
			goto AbortJump;
		}
		if (Length) {	/* Check for length */
			goto AbortNotEnough;
		}
		if (Token) {		/* End of data */
			Input-=2;		/* Force this token permanently! */
		}
	}
	return Input;	/* Return this pointer */
	
AbortNotEnough:
	BadNews = "Insufficient data in RLE8 packed array!\n";
	goto Abort;
AbortJump:
	BadNews = "Can't accept a jump RLE opcode!\n";
	goto Abort;
AbortLargeRun:
	BadNews = "Run length is too large!\n";
	goto Abort;
Abort1:
	BadNews = "Run length is too large in RLE8 BMP file!\n";
Abort:
	NonFatal(BadNews);	/* Print the error */
	return 0;
}

/**********************************

	Read in a BMP file and set the variables
	I can read in 24 bit UNCOMPRESSED only.
	8 bit compressed or uncompressed BMP files.
	Return 0 if successful

**********************************/

Word BURGERCALL ImageParseBMP(Image_t *ImagePtr,const Word8 *Data)
{
	BMPHeader *Header;	/* Header to TGA file */
	Word8 *Buffer;		/* Pointer to saved memory */
	const Word8 *PalPtr;		/* Source pointer */
	char *BadNews;		/* Error message */
	Word i;
	Word Depth;

	FastMemSet(ImagePtr,0,sizeof(Image_t));	/* Init the return buffer */
	Header = (BMPHeader *)Data;

	if (Header->id[0]!='B' || Header->id[1]!='M') {
		goto AbortNoBM;
	}
	Depth = Burger::LoadLittle(&Header->BitDepth);
	if (Depth != 8 && Depth!=24) {
		NonFatal("Can't process %u bits per pixel, only 8 or 24.\n",Depth);
		goto OhShit;
	}
	if (Depth==24 && Header->biCompression) {		/* No compression on 24 bit */
		goto Abort24Comp;
	}

	ImagePtr->Width = Burger::LoadLittle(&Header->Width);	/* Save the size */
	ImagePtr->Height = Burger::LoadLittle(&Header->Height);
	if (Depth==8) {
		ImagePtr->DataType = IMAGE8_PAL;		/* 8 or 24 */
		ImagePtr->RowBytes = ImagePtr->Width;
	} else {
		ImagePtr->DataType = IMAGE888;			/* 24 bit */
		ImagePtr->RowBytes = ImagePtr->Width*3;
	}
	if (!ImagePtr->Width) {
		goto AbortWidth0;
	}
	if (!ImagePtr->Height) {
		goto AbortHeight0;
	}

	/* If it's an 8 bit image, grab the palette */

	if (Depth==8) {		/* Only indexed images have a palette */
		Buffer = (Word8 *)AllocAPointer(768);	/* Get a palette */
		if (!Buffer) {
			goto OhShit;
		}
		ImagePtr->PalettePtr = Buffer;		/* Save the palette pointer */
		PalPtr = &Data[Burger::LoadLittle(&Header->InfoSize)+14];		/* Index to the palette */
		i = 0;
		do {
			Buffer[0] = PalPtr[2];		/* Read in the palette */
			Buffer[1] = PalPtr[1];
			Buffer[2] = PalPtr[0];
			PalPtr=PalPtr+4;
			Buffer=Buffer+3;
		} while (++i<256);
	}

	/* Get the memory for the pixel map */

	Depth = Depth>>3;		/* 1 or 3 */
	Buffer = (Word8 *)AllocAPointer((Word32)ImagePtr->Width*ImagePtr->Height*Depth);
	if (!Buffer) {
		goto OhShit;
	}
	ImagePtr->ImagePtr = Buffer;		/* Save the buffer pointer */

	PalPtr = &Data[Burger::LoadLittle(&Header->HeaderSize)];	/* File data */
	Buffer = &Buffer[(ImagePtr->Height-1)*((Word32)ImagePtr->Width*Depth)];

	i = ImagePtr->Height;		/* Get the height */
	if (Header->biCompression) {		/* 8 bit compressed */
		Depth = ImagePtr->Width;		/* Get the bytes per line */
		do {
			PalPtr = UnpackRLE(Buffer,PalPtr,Depth);	/* Decompress */
			if (!PalPtr) {			/* Error? */
				goto OhShit;
			}
			Buffer = Buffer-Depth;	/* Next line up */
		} while (--i);			/* All done? */
	} else {
		Word RunWidth;
		if (Depth==1) {			/* 8 bits per pixel? */
			Depth = ImagePtr->Width;		/* Bytes per line */
			RunWidth = (Depth+3)&(~3);		/* Pad to longword */
			do {
				FastMemCpy(Buffer,PalPtr,Depth);	/* Copy the line */
				PalPtr = PalPtr+RunWidth;	/* Next line down */
				Buffer = Buffer-Depth;		/* Next line up */
			} while(--i);
		} else {			/* 24 bits per pixel */
			Depth = ImagePtr->Width*3;	/* Bytes per line */
			RunWidth = (0-Depth)&3;		/* Pad to longword */
			Depth = Depth<<1;			/* Subtract 2 lines instead of one */
			do {
				Word j;
				j = ImagePtr->Width;	/* Pixel count */
				do {
					Buffer[0] = PalPtr[2];	/* Red */
					Buffer[1] = PalPtr[1];	/* Green */
					Buffer[2] = PalPtr[0];	/* Blue */
					Buffer=Buffer+3;		/* Next pixel */
					PalPtr=PalPtr+3;
				} while (--j);				/* Count down */
				PalPtr = PalPtr+RunWidth;	/* Add any padding */
				Buffer = Buffer-Depth;		/* Next line up */
			} while(--i);
		}
	}
	return FALSE;			/* No errors! */
	
	/* Errors! */
AbortWidth0:
	BadNews = "Image width is zero!\n";
	goto Abort;
AbortHeight0:
	BadNews = "Image height is zero!\n";
	goto Abort;
Abort24Comp:
	BadNews = "24 bit image file is compressed.\n";
	goto Abort;
AbortNoBM:
	BadNews = "No 'BM' id string found.\n";
Abort:
	NonFatal(BadNews);
OhShit:;
	ImageDestroy(ImagePtr);		/* Discard any allocated memory */
	return TRUE;		/* Exit with error */
}


