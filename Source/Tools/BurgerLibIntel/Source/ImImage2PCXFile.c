#include "ImImage.h"
#include "MmMemory.h"
#include "ClStdLib.h"
#include "PlPalette.h"
#include "FmFile.h"

/**********************************

	Parse out an IBM PCX file

**********************************/

typedef struct PCXHeader_t {
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

	Compress PCX bitmap data
	The format is simplicity itself.
	If the input byte is less than 0xC0, then output the byte.
	Otherwise and with 0x3F and use it as a count and fill with
	the next byte.
	Opcode 0xC0 does nothing.

**********************************/

void BURGERCALL ImageEncodePCX(FILE *fp,const Word8 *SrcPtr,Word Length)
{
	Word Temp;
	Word Pad;
	if (Length) {
		Pad = Length&1;
		do {
			Temp = SrcPtr[0];
			if (Length<2) {		/* Only 1 byte left? */
				if (Temp>=0xC0) {		/* Must run length? */
					fputc(0xC1,fp);		/* Phony run of 1 */
				}
				fputc(Temp,fp);			/* Send data */
				break;					/* Get out! */
			}
			if (Temp<0xC0 && Temp!=SrcPtr[1]) {			/* Must run length? */
				fputc(Temp,fp);			/* Save the raw byte */
				--Length;			/* Remove the length */
				++SrcPtr;			/* Accept the byte */
			} else {				/* Repeater */
				Word Count;
				Word Max;
				Count = 1;
				Max = Length;
				if (Max>63) {
					Max = 63;
				}
				if (Count<Max) {
					do {
						if (Temp!=SrcPtr[Count]) {
							break;
						}
					} while (++Count<Max);
				}
				fputc(Count+0xC0,fp);
				fputc(Temp,fp);
				Length-=Count;
				SrcPtr+=Count;
			}
		} while (Length);		/* More? */
		if (Pad) {				/* Do I add a pad byte? */
			fputc(0,fp);
		}
	}
}

/**********************************

	Write an Image_t structure out as a PCX file

**********************************/

static const Word8 PCXHeader1[8] = {
	10,		/* 10 for PC Paintbrush */
	5,		/* Version 3.0 */
	1,		/* PCX compression */
	8,		/* 8 bits per pixel */
	0,0,0,0	/* Two shorts for MinX and MinY */
};

static const Word8 PCXHeader2[4] = {
	72,0,
	72,0
};

Word BURGERCALL Image2PCXFile(Image_t *ImagePtr,const char *FileName)
{
	FILE *fp;
	Word i;
	Word8 *Buffer;
	Word8 Bogus[256];
	
	if (ImageValidateToSave(ImagePtr)) {	/* Is this a valid Image_t struct? */
		return TRUE;		/* Nope, error */
	}
	fp = OpenAFile(FileName,"wb");	/* Open the output file */
	if (!fp) {			/* Oh oh... */
		return TRUE;		/* I couldn't open the file! */
	}
	
	FastMemSet(Bogus,0,sizeof(Bogus));
	fwrite(PCXHeader1,1,8,fp);
	fwriteshortl((Word16)(ImagePtr->Width-1),fp);	/* Width */
	fwriteshortl((Word16)(ImagePtr->Height-1),fp);	/* Height */
	fwrite(PCXHeader2,1,4,fp);	/* DPI */
	
	i = 3;		/* 3 bit planes */
	if (ImagePtr->DataType>=IMAGE8_PAL_ALPHA_PAL && ImagePtr->DataType<=IMAGE8_PAL) {
		i = 1;	/* A single bit plane */
	}
	Bogus[49] = (Word8)i;
	fwrite(Bogus,1,50,fp);		/* 48 bytes for EGA palette + 1 for Zero1 */
	i = ((ImagePtr->Width+1)&(~1));	/* Make sure it's even! */
	Bogus[50] = (Word8)i;
	Bogus[51] = (Word8)(i>>8);
	fwrite(Bogus,1,60+52,fp);
	
	/* Header is written, now for the data */
	
	i = ImagePtr->Height;
	Buffer = ImagePtr->ImagePtr;
	
	/* 8 bits per pixel is easy! */
	
	if (ImagePtr->DataType>=IMAGE8_PAL_ALPHA_PAL && ImagePtr->DataType<=IMAGE8_PAL) {
		do {
			ImageEncodePCX(fp,Buffer,ImagePtr->Width);	/* Save off the data */
			Buffer += ImagePtr->RowBytes;				/* Next line to encode */
		} while (--i);
		/* Finally, now write the palette! */
		
		fputc(12,fp);		/* Palette ID byte */
		fwrite(ImagePtr->PalettePtr,1,768,fp);	/* Easy as cake! */
	} else {
	
	/* All the others make me throw up. */
		Word8 *TempBuffer;
		Word8 *WorkPtr;
		Word Width;
		TempBuffer = (Word8 *)AllocAPointer(ImagePtr->Width*3);
		if (!TempBuffer) {
			goto OhShit;
		}
		Width = ImagePtr->Width;
		do {
			Word j;
			Word8 *Buffer2;
			Buffer2 = Buffer;
			WorkPtr = TempBuffer;
			j = Width;
			if (ImagePtr->DataType==IMAGE888) {
				do {
					WorkPtr[0] = Buffer2[0];
					WorkPtr[Width] = Buffer2[1];
					WorkPtr[Width*2] = Buffer2[2];
					Buffer2+=3;
					++WorkPtr;
				} while (--j);

			} else if (ImagePtr->DataType==IMAGE8888) {
				do {
					WorkPtr[0] = Buffer2[0];
					WorkPtr[Width] = Buffer2[1];
					WorkPtr[Width*2] = Buffer2[2];
					Buffer2+=4;
					++WorkPtr;
				} while (--j);

			} else if ((ImagePtr->DataType==IMAGE555) || (ImagePtr->DataType==IMAGE1555)) {
				do {
					Word8 Triplett555[3];		/* Buffer for RGB triplett */
					PaletteConvertRGB15ToRGB24(Triplett555,((Word16 *)Buffer2)[0]);
					WorkPtr[0] = Triplett555[0];
					WorkPtr[Width] = Triplett555[1];
					WorkPtr[Width*2] = Triplett555[2];
					Buffer2+=2;
					++WorkPtr;
				} while (--j);
			} else if (ImagePtr->DataType==IMAGE565) {
				do {
					Word8 Triplett565[3];		/* Buffer for RGB triplett */
					PaletteConvertRGB16ToRGB24(Triplett565,((Word16 *)Buffer2)[0]);
					WorkPtr[0] = Triplett565[0];
					WorkPtr[Width] = Triplett565[1];
					WorkPtr[Width*2] = Triplett565[2];
					Buffer2+=2;
					++WorkPtr;
				} while (--j);
			}
			ImageEncodePCX(fp,TempBuffer,ImagePtr->Width);	/* Save off the data */
			ImageEncodePCX(fp,TempBuffer+Width,ImagePtr->Width);	/* Save off the data */
			ImageEncodePCX(fp,TempBuffer+Width*2,ImagePtr->Width);	/* Save off the data */
			Buffer = Buffer+ImagePtr->RowBytes;
		} while (--i);
		DeallocAPointer(TempBuffer);
	}
	
	/* Now wrap up... */
	
	fclose(fp);				/* Close the file */
	
#if defined(__MAC__)
	SetAFileType(FileName,'PCX ');		/* Set the Mac filetype */
	SetAnAuxType(FileName,'BABL');
#endif

	return FALSE;			/* I saved it! */
OhShit:
	fclose(fp);		/* Close the file */
	return TRUE;	/* Damn it! */

}
