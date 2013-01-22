#include "ImImage.h"
#include "ClStdLib.h"
#include "PlPalette.h"
#include "FmFile.h"
#include <stdio.h>

/**********************************

	Write an Image_t structure out as a BMP file
	I can save compressed or uncompressed 8 bit images
	and uncompressed 24 bit images

**********************************/

typedef struct BMPHeader {	/* Header of a BMP file (Little Endian) */
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

/**********************************

	Take a line of 8 bit pixel data and compress is using
	the RLE8 format used by BMP files.
	The format itself sucks eggs.
	I first send a compress token of 0 or a fill byte of 1-255
	If it's 1-255 I then follow the token with the byte I fill with.

	If it's zero I then follow it with zero for EOL, 1 for EO data
	and 3-FF for raw data (Followed by raw data). I don't use token 2 (Jump)

**********************************/

static void CompressBMPLine(FILE *fp,const Word8 *DataPtr,Word Length,Word Line)
{
	Word Fill;		/* Word8 for run fill */
	Word Max;		/* Maximum run length */
	Word Count;		/* Current run length */
	Word8 DataBuf[16];	/* Word8 array for file writing */
	
	if (Length>=3) {	/* Less than 3 remaining? */
		do {
			Max = Length;			/* Maximum number of bytes to check */
			if (Max>255) {
				Max = 255;			/* Maximum run */
			}
			Fill = DataPtr[0];			/* Get the first byte */
			if (DataPtr[1]==Fill) {		/* Try for a run... */
				Count = 2;				/* Assume 2 bytes for fill */
				do {
					if (DataPtr[Count]!=Fill) {		/* Match? */
						break;				/* Get out now! */
					}
				} while (++Count<Max);
				DataBuf[0] = (Word8)Count;	/* Save the run length */
				DataBuf[1] = (Word8)Fill;	/* Fill byte */
				fwrite(DataBuf,1,2,fp);
			} else {					/* This is for raw data */
				Count = 3;				/* Minimum of 3 */
				Max -= 2;
				if (Count<Max) {
					do {
						if ((DataPtr[Count]==DataPtr[Count+1]) &&
							(DataPtr[Count+1]==DataPtr[Count+2])) {
							goto DoIt;
						}
					} while (++Count<Max);
					Count = Max+2;
					if (Count&1 && Count<Length) {	/* Try to not waste data */
						--Count;
					}
				}
DoIt:
				DataBuf[0] = 0;			/* Raw token */
				DataBuf[1] = (Word8)Count;	/* Run count */
				fwrite(DataBuf,1,2,fp);
				fwrite(DataPtr,1,Count,fp);	/* Save the data */
				if (Count&1) {
					fputc(0,fp);		/* Pad to a short */
				}
			}
			Length-=Count;			/* Remove length */
			DataPtr+=Count;			/* Adjust the source pointer */
		} while (Length>=3);		/* More remaining? */
	}

	Count = 0;
	switch (Length) {		/* Any data remaining? */
	case 2:
		Max = DataPtr[0];	/* Fill byte */
		DataBuf[1] = (Word8)Max;
		if (Max==DataPtr[1]) {
			DataBuf[0] = 2;			/* Fill it */
			Count = 2;
			break;
		}
		DataBuf[0] = 1;			/* Save the single byte */
		++DataPtr;
		Count = 2;
	case 1:
		DataBuf[Count] = 1;			/* Save the single byte */
		++Count;
		DataBuf[Count] = DataPtr[0];	/* Send the last byte */
		++Count;
	}
	if (Line!=1) {	/* Line = 1 for EOP */
		Line = 0;	/* EOL */
	}
	DataBuf[Count] = 0;			/* END token */
	++Count;
	DataBuf[Count] = (Word8)Line;	/* EOL or EOP */
	++Count;
	fwrite(DataBuf,1,Count,fp);
}

#define BMPHeaderSize 54

/**********************************

	Write an Image_t structure out as a BMP file

**********************************/

Word BURGERCALL Image2BMPFile(Image_t *ImagePtr,const char *FileName,Word Compress)
{
	Word32 Mark;			/* Size of the header */
	FILE *fp;				/* Open file record */
	Word8 *Buffer;			/* Pointer to saved memory */
	Word Padding;			/* Word8 count for line padding */
	Word i;					/* Temp */
	Word32 Length;		/* 32 bit image offset */
	Word32 MemData;		/* 4 bytes of data */

	if (ImageValidateToSave(ImagePtr)) {	/* Is this a valid Image_t struct? */
		return TRUE;		/* Nope, error */
	}
	fp = OpenAFile(FileName,"wb");	/* Open the output file */
	if (!fp) {			/* Oh oh... */
		return TRUE;		/* I couldn't open the file! */
	}

	fwrite("BM",1,2,fp);	/* Write the first header */
	fwritelong(0,fp);		/* Write the file length (Change later) */
	fwritelong(0,fp);		/* Write the reserved entries */
	Mark = 0x436;
	Padding = 8;
	if (ImagePtr->DataType!=IMAGE8_PAL) {
		Compress = FALSE;	/* No compression is allowed!!! */
		Mark = 0x36;		/* No palette */
		Padding = 24;
	}
	fwritelongl(Mark,fp);	/* Header with palette size */
	fwritelongl(40,fp);		/* Data info size */
	fwritelongl(ImagePtr->Width,fp);	/* Image size */
	fwritelongl(ImagePtr->Height,fp);
	fwriteshortl(1,fp);			/* Bit planes */
	fwriteshortl((Word16)Padding,fp);	/* Bit depth */
	fwritelongl(Compress!=0,fp);		/* Compression enable 0 or 1 */
	fwritelong(0,fp);			/* Output length */
	fwritelongl(0xb13,fp);		/* Pixels per meter */
	fwritelongl(0xb13,fp);
	fwritelong(0,fp);		/* Colors used */
	fwritelong(0,fp);		/* Colors important */

	/* Now, here's where we send out the data */

	/* Code for the 8 bits per pixel version */

	MemData = 0;		/* Array of zeros at least 3 long */
	Padding = ImagePtr->Width;
	if (ImagePtr->DataType==IMAGE8_PAL) {		/* 8 bit image */
		Word8 Palette[1024];				/* Palette to save to disk */
		Word8 *OutP;
		Buffer = ImagePtr->PalettePtr;	/* First save the palette */
		OutP = Palette;
		i = 256;
		do {
			OutP[0] = Buffer[2];		/* Blue */
			OutP[1] = Buffer[1];		/* Green */
			OutP[2] = Buffer[0];		/* Red */
			OutP[3] = 0;				/* Extra */
			OutP+=4;
			Buffer=Buffer+3;		/* Next palette entry */
		} while (--i);				/* All done? */
		fwrite(Palette,1,1024,fp);	/* Save to disk */
		i = ImagePtr->Height;		/* Height of image */
		Buffer = ImagePtr->ImagePtr + ((i-1)*(Word32)ImagePtr->RowBytes);	/* Bottom line */
		if (Compress) {
			do {
				CompressBMPLine(fp,Buffer,ImagePtr->Width,i);	/* Compress a line */
				Buffer = Buffer-ImagePtr->RowBytes;		/* Next line up */
			} while (--i);		/* All done? */
		} else {
			Padding = (0-Padding)&3;	/* 0,3,2,1 */
			do {
				fwrite(Buffer,1,ImagePtr->Width,fp);	/* Save a line to disk */
				if (Padding) {			/* Any padding? */
					fwrite(&MemData,1,Padding,fp);	/* Filler */
				}
				Buffer = Buffer-ImagePtr->RowBytes;		/* Next line up */
			} while (--i);		/* All done? */
		}

	/* 15,16, 24 and 32 bits per pixel image will be stored as 24 bits */

	} else {
		Padding = (0-(Padding*3))&3;	/* Padding after each pixel */
		i = ImagePtr->Height;

		/* 24 bit images */

		if (ImagePtr->DataType==IMAGE888) {	/* Send off the bytes */
			Length = (Word32)ImagePtr->RowBytes;	/* Size in bytes of each line */
			Buffer = ImagePtr->ImagePtr + ((i-1)*Length);	/* Bottommost line */
			Length = Length<<1;			/* 1 line size times 2 */
			do {
				Word j;
				j = ImagePtr->Width;		/* Get count */
				do {
					fputc(Buffer[2],fp);	/* Save out the scan line */
					fputc(Buffer[1],fp);
					fputc(Buffer[0],fp);
					Buffer=Buffer+3;		/* Next triplett */
				} while (--j);			/* Next pixel */
				if (Padding) {		/* Any padding? */
					fwrite(&MemData,1,Padding,fp);	/* Filler */
				}
				Buffer = Buffer-Length;		/* Next line up (Sub line length * 2) */
			} while (--i);		/* All done? */

		/* 32 bit images */

		} else if (ImagePtr->DataType==IMAGE8888) {
			Length = (Word32)ImagePtr->Width*4;	/* Size in bytes of each line */
			Buffer = ImagePtr->ImagePtr + ((i-1)*Length);	/* Bottommost line */
			Length = Length<<1;		/* 1 line size times 2 */
			do {
				Word j;
				j = ImagePtr->Width;		/* Get count */
				do {
					fputc(Buffer[2],fp);	/* Save out the scan line */
					fputc(Buffer[1],fp);
					fputc(Buffer[0],fp);
					Buffer=Buffer+4;		/* Next triplett */
				} while (--j);			/* Next pixel */
				if (Padding) {		/* Any padding? */
					fwrite(&MemData,1,Padding,fp);	/* Filler */
				}
				Buffer = Buffer-Length;		/* Next line up (Sub line length * 2) */
			} while (--i);		/* All done? */
		} else {

	/* This routine saves 16 bit data as 24 bit data */

			Word8 Triplett[3];		/* Buffer for RGB triplett */
			Length = (Word32)ImagePtr->Width*2;	/* Length of line in bytes */
			Buffer = ImagePtr->ImagePtr + ((i-1)*Length);	/* Bottommost line */
			Length = Length<<1;		/* 1 line size times 2 */
			do {
				Word j;
				j = ImagePtr->Width;		/* Get count */
				do {
					if (ImagePtr->DataType==IMAGE565) {
						PaletteConvertRGB16ToRGB24(Triplett,((Word16 *)Buffer)[0]);
					} else {
						PaletteConvertRGB15ToRGB24(Triplett,((Word16 *)Buffer)[0]);
					}
					fputc(Triplett[2],fp);
					fputc(Triplett[1],fp);
					fputc(Triplett[0],fp);
					Buffer=Buffer+2;
				} while (--j);
				if (Padding) {		/* Any padding? */
					fwrite(&MemData,1,Padding,fp);	/* Filler */
				}
				Buffer = Buffer-Length;		/* Next line up */
			} while (--i);
		}
	}

	/* Now wrap up... */

	Length = ftell(fp);	/* What's the length of the file */
	Mark = Length-Mark;	/* Size of the data */
	fseek(fp,2,SEEK_SET);
	fwritelongl(Length,fp);	/* Write the length of the total file */
	fseek(fp,34,SEEK_SET);
	fwritelongl(Mark,fp);	/* Write the length of the packed data */
	fclose(fp);				/* Close the file */

#if defined(__MAC__)
	SetAFileType(FileName,'BMP ');		/* Set the Mac filetype */
	SetAnAuxType(FileName,'ogle');
#endif

	return FALSE;			/* I saved it! */
}


