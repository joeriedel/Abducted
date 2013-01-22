#include "ImImage.h"
#include <BREndian.hpp>
#include "MmMemory.h"
#include "ClStdLib.h"

/**********************************

	Parse out an IBM PCX file

**********************************/

typedef struct {
	Word8 Manufacturer;		/* Must be 10 for PC Paintbrush */
	Word8 Version;			/* 0 = 2.5, 2 = 2.8, 3 = 2.8 with color 5 = 3.0 */
	Word8 Encoding;			/* 1 = PCX run length encoding */
	Word8 BitsPerPixel;		/* Bits per pixel */
	Word16 MinX,MinY,MaxX,MaxY;	/* Bounding rect (Inclusive) */
	Word16 XDPI,YDPI;		/* DPI (Usually 72) */
	Word8 EgaPalette[48];	/* Standard EGA Palette (Ignore) */
	Word8 Zero1;				/* Reserved */
	Word8 BitPlanes;			/* Number of bit planes (1) */
	Word16 BytesPerLine;		/* Bytes per line */
	Word8 Padding[60];		/* This struct must be 128 bytes in size */
} PCXHeader_t;

#define PCXHeaderSize 128	/* Prevent errors with structure padding */

/**********************************

	Decompress PCX bitmap data
	The format is simplicity itself.
	If the input byte is less than 0xC0, then output the byte.
	Otherwise and with 0x3F and use it as a count and fill with
	the next byte.
	Opcode 0xC0 does nothing.
	Note : When I am asked to decode an ODD number of pixels, I have a hack
	to fix a bug in the data overrun buffer. Debabelizer can write out bad
	PCX files when the width is odd.

**********************************/

static const Word8 * BURGERCALL DecompressPCX(Word8 *DestBuffer,const Word8 *InputPtr,Word Length)
{
	Word Temp;
	if (Length) {
		do {
			Temp = InputPtr[0];		/* Fetch byte from input stream */
			++InputPtr;
			if (Temp<0xC0) {		/* Unpacked data? */
				DestBuffer[0] = static_cast<Word8>(Temp);	/* Save byte */
				++DestBuffer;
				--Length;			/* Count down the length */
			} else {
				Temp = Temp&0x3F;	/* Mask count */
				if (Temp) {			/* Was it not 0xC0? */
					Word8 Fill;
					if (Temp>Length) {	/* Overrun buffer? */
						goto Abort;
					}

					Fill = InputPtr[0];		/* Get fill byte */
					++InputPtr;
					Length = Length-Temp;	/* Adjust length */
					do {
						DestBuffer[0] = Fill;	/* Memory fill */
						++DestBuffer;
					} while (--Temp);		/* Count down */
				}
			}
		} while (Length);		/* More? */
	}
	return InputPtr;		/* Return input stream pointer */
Abort:
	NonFatal("PCX decompressor overrun\n");
	return 0;
}

/**********************************

	24 bit images are a strip of red, then green then blue
	Fix this...

**********************************/

static void PCX24Fixup(Word8 *DestPtr,Word8 *SrcPtr,Word Length)
{
	Word i;
	i = Length;
	do {
		DestPtr[0] = SrcPtr[0];
		DestPtr[1] = SrcPtr[Length];
		DestPtr[2] = SrcPtr[Length*2];
		DestPtr+=3;
		++SrcPtr;
	} while (--i);
}

/**********************************

	Convert an PC-Paintbrush PCX file into
	an Image_t structure

**********************************/

Word BURGERCALL ImageParsePCX(Image_t *ImagePtr,const Word8 *InputPtr)
{
	PCXHeader_t *Header;
	Word Temp,Temp2;
	Word32 Length;
	Word8 *DestBuffer;
	char *BadNews;
	Word LineWidth;
	
	FastMemSet(ImagePtr,0,sizeof(Image_t));		/* Init the input struct */

	Header = (PCXHeader_t *)InputPtr;
	InputPtr = InputPtr+PCXHeaderSize;		/* Adjust the input pointer */

	/* Is this a PCX file? */

	if (Header->Manufacturer!=10) {
		goto AbortBadID;
	}
	if (Header->Version!=5) {
		goto AbortBadVersion;
	}
	if (Header->Encoding!=1) {
		goto AbortBadEncode;
	}
	if (Header->BitsPerPixel!=8) {
		goto AbortBadBPP;
	}
	if (Header->BitPlanes!=1 && Header->BitPlanes!=3) {
		goto AbortBadPlanes;
	}
	Temp = Burger::LoadLittle(&Header->MinX);
	Temp2 = Burger::LoadLittle(&Header->MaxX);
	Temp2 = Temp2-Temp;
	++Temp2;
	if ((int)Temp2<=0) {		/* Valid width? */
		goto AbortBadWidth;
	}
	ImagePtr->Width = Temp2;

	Temp = Burger::LoadLittle(&Header->MinY);
	Temp2 = Burger::LoadLittle(&Header->MaxY);
	++Temp2;
	if ((int)Temp2<=0) {		/* Valid height? */
		goto AbortBadHeight;
	}
	ImagePtr->Height = Temp2;

	LineWidth = Burger::LoadLittle(&Header->BytesPerLine);
	if (LineWidth!=((ImagePtr->Width+1)&(~1))) {			/* Rounded up... */
		goto AbortBadWidth2;
	}

	/* Let's parse out the data */

	/* 8 bits per pixel version */
	
	if (Header->BitPlanes==1) {
		ImagePtr->DataType = IMAGE8_PAL;		/* I only process 8 bits per pixel images */
		ImagePtr->RowBytes = ImagePtr->Width;
		Length = (Word32)ImagePtr->Width*ImagePtr->Height;
		DestBuffer = (Word8 *)AllocAPointer(Length+2);		/* Extra for padding */
		if (!DestBuffer) {
			goto OhShit;
		}
		ImagePtr->ImagePtr = DestBuffer;		/* Save pointer */
		Temp = ImagePtr->Height;
		do {
			InputPtr = DecompressPCX(DestBuffer,InputPtr,LineWidth);
			if (!InputPtr) {
				goto OhShit;
			}
			DestBuffer+=ImagePtr->Width;
		} while (--Temp);
		/* Let's parse out the palette! */

		/* Note, the PCX format SAYS I'm supposed to find a 12 as the next */
		/* byte, however, I've found a rare file that has a 0xC1,0xFF in the */
		/* byte stream of pad data. This little hack will be able to skip pad */
		/* data and find the palette */
		
		Temp = 8;				/* Only scan 8 bytes. I assume I'm screwed if it's beyond this */
		do {
			if (InputPtr[0]==12) {		/* Is the current byte a palette entry? */
				break;
			}
			++InputPtr;				/* Skip the pad bytes */
		} while (--Temp);
		if (!Temp) {				/* Did I not find a palette? */
			goto AbortNoPal;		/* I can't find a palette token for 8 bytes */
		}
		
		DestBuffer = (Word8 *)AllocAPointer(768);	/* Get palette memory */
		if (!DestBuffer) {
			goto OhShit;
		}
		ImagePtr->PalettePtr = DestBuffer;	/* Save it */
		FastMemCpy(DestBuffer,InputPtr+1,768);	/* Copy the palette */
		return FALSE;			/* I did it!! */
	}
	
	/* Let's parse out a 24 bits per pixel image */
	
	ImagePtr->DataType = IMAGE888;
	ImagePtr->RowBytes = ImagePtr->Width*3;
	Length = (Word32)ImagePtr->Width*ImagePtr->Height*3;
	DestBuffer = (Word8 *)AllocAPointer(Length);
	if (!DestBuffer) {
		goto OhShit;
	}
	ImagePtr->ImagePtr = DestBuffer;		/* Save pointer */
	Temp = ImagePtr->Height;
	Temp2 = ImagePtr->Width;
	BadNews = (char *)AllocAPointer((Temp2*3)+32);
	if (!BadNews) {
		goto OhShit;
	}
	do {
		InputPtr = DecompressPCX((Word8 *)BadNews,InputPtr,LineWidth);
		if (!InputPtr) {
			break;
		}
		InputPtr = DecompressPCX((Word8 *)BadNews+Temp2,InputPtr,LineWidth);
		if (!InputPtr) {
			break;
		}
		InputPtr = DecompressPCX((Word8 *)BadNews+Temp2*2,InputPtr,LineWidth);
		if (!InputPtr) {
			break;
		}
		PCX24Fixup(DestBuffer,(Word8 *)BadNews,Temp2);
		DestBuffer+=Temp2*3;
	} while (--Temp);
	DeallocAPointer(BadNews);
	if (!InputPtr) {
		goto OhShit;
	}
	return FALSE;
		
AbortBadID:
	BadNews = "Not a PC-Paintbrush PCX file!\n";
	goto Abort;
AbortBadVersion:
	BadNews = "I can only process version 3.0 PCX files!\n";
	goto Abort;
AbortBadEncode:
	BadNews = "I can only process PCX encoded PCX files!\n";
	goto Abort;
AbortBadBPP:
	BadNews = "Not an 8 bit per pixel PCX file!\n";
	goto Abort;
AbortBadPlanes:
	BadNews = "The PCX file must be \"Chunky\" pixel format\n";
	goto Abort;
AbortBadWidth:
	BadNews = "Invalid image width in PCX file\n";
	goto Abort;
AbortBadHeight:
	BadNews = "Invalid image height in PCX file\n";
	goto Abort;
AbortBadWidth2:
	BadNews = "Bytes per line does not match PCX image width!\n";
	goto Abort;
AbortNoPal:
	BadNews = "Palette command byte was not found in PCX file!\n";
Abort:
	NonFatal(BadNews);
OhShit:;
	ImageDestroy(ImagePtr);		/* Discard the data */
	return TRUE;			/* Bogus! */
}
