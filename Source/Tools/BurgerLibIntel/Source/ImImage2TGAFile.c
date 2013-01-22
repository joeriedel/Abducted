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

	Take a line of 8 bit pixel data and compress is using
	the RLE format used by TGA files.
	The format itself sucks eggs.
	I send either a 0x80-0xFF to denote 1 to 128 fill bytes
	0 to 0x7F is followed by Count+1 raw bytes

**********************************/

static void CompressTGALine(FILE *fp,const Word8 *DataPtr,Word Length)
{
	Word Fill;		/* Word8 for run fill */
	Word Max;		/* Maximum run length */
	Word Count;		/* Current run length */
	Word8 DataBuf[16];	/* Word8 array for file writing */
	
	if (Length>=3) {	/* Less than 3 remaining? */
		do {
			Max = Length;			/* Maximum number of bytes to check */
			if (Max>128) {
				Max = 128;			/* Maximum run */
			}
			Fill = DataPtr[0];			/* Get the first byte */
			if (DataPtr[1]==Fill) {		/* Try for a run... */
				Count = 2;				/* Assume 2 bytes for fill */
				do {
					if (DataPtr[Count]!=Fill) {		/* Match? */
						break;				/* Get out now! */
					}
				} while (++Count<Max);
				DataBuf[0] = (Word8)Count+127;	/* Save the run length */
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
					Count = Max;
				}
DoIt:
				fputc(Count-1,fp);
				fwrite(DataPtr,1,Count,fp);	/* Save the data */
			}
			Length-=Count;			/* Remove length */
			DataPtr+=Count;			/* Adjust the source pointer */
		} while (Length>=3);		/* More remaining? */
	}

	switch (Length) {		/* Any data remaining? */
	case 2:
		Max = DataPtr[0];	/* Fill byte */
		DataBuf[1] = (Word8)Max;
		if (Max==DataPtr[1]) {
			DataBuf[0] = 0x81;			/* Fill it */
			fwrite(DataBuf,1,2,fp);
			break;
		}
		DataBuf[0] = 1;			/* Save the single byte */
		DataBuf[2] = DataPtr[1];
		fwrite(DataBuf,1,3,fp);
		break;
	case 1:
		DataBuf[0] = 0;				/* Save the single byte */
		DataBuf[1] = DataPtr[0];	/* Send the last byte */
		fwrite(DataBuf,1,2,fp);
	}
}

/**********************************

	Take a line of 8 bit pixel data and compress is using
	the RLE format used by TGA files.
	The format itself sucks eggs.
	I send either a 0x80-0xFF to denote 1 to 128 fill bytes
	0 to 0x7F is followed by Count+1 raw bytes

**********************************/

static void CompressTGALine555(FILE *fp,const Word8 *DataPtr,Word Length)
{
	Word Fill;		/* Word8 for run fill */
	Word Max;		/* Maximum run length */
	Word Count;		/* Current run length */
	Word8 DataBuf[16];	/* Word8 array for file writing */
	
	if (Length>=3) {	/* Less than 3 remaining? */
		do {
			Max = Length;			/* Maximum number of bytes to check */
			if (Max>128) {
				Max = 128;			/* Maximum run */
			}
			Fill = ((Word16 *)DataPtr)[0];			/* Get the first byte */
			if (((Word16 *)DataPtr)[1]==Fill) {		/* Try for a run... */
				Count = 2;				/* Assume 2 bytes for fill */
				do {
					if (((Word16 *)DataPtr)[Count]!=Fill) {		/* Match? */
						break;				/* Get out now! */
					}
				} while (++Count<Max);
				DataBuf[0] = (Word8)Count+127;	/* Save the run length */
				DataBuf[1] = (Word8)Fill;	/* Fill byte */
				DataBuf[2] = (Word8)(Fill>>8);
				fwrite(DataBuf,1,3,fp);
			} else {					/* This is for raw data */
				Count = 3;				/* Minimum of 3 */
				Max -= 2;
				if (Count<Max) {
					do {
						if ((((Word16 *)DataPtr)[Count]==((Word16 *)DataPtr)[Count+1]) &&
							(((Word16 *)DataPtr)[Count+1]==((Word16 *)DataPtr)[Count+2])) {
							goto DoIt;
						}
					} while (++Count<Max);
					Count = Max;
				}
DoIt:
				fputc(Count-1,fp);
				Max = 0;
				do {
					fwriteshortl(((Word16 *)DataPtr)[Max],fp);	/* Save the data */
				} while (++Max<Count);
			}
			Length-=Count;			/* Remove length */
			DataPtr+=Count*2;		/* Adjust the source pointer */
		} while (Length>=3);		/* More remaining? */
	}

	switch (Length) {		/* Any data remaining? */
	case 2:
		Max = ((Word16 *)DataPtr)[0];	/* Fill byte */
		DataBuf[1] = (Word8)Max;
		DataBuf[2] = (Word8)(Max>>8);
		if (Max==((Word16 *)DataPtr)[1]) {
			DataBuf[0] = 0x81;			/* Fill it */
			fwrite(DataBuf,1,3,fp);
			break;
		}
		DataBuf[0] = 1;			/* Save the single byte */
		Max = ((Word16 *)DataPtr)[1];
		DataBuf[3] = (Word8)Max;
		DataBuf[4] = (Word8)(Max>>8);
		fwrite(DataBuf,1,5,fp);
		break;
	case 1:
		Max = ((Word16 *)DataPtr)[0];
		DataBuf[0] = 0;				/* Save the single byte */
		DataBuf[1] = (Word8)Max;	/* Send the last byte */
		DataBuf[2] = (Word8)(Max>>8);
		fwrite(DataBuf,1,3,fp);
	}
}

/**********************************

	Take a line of 8 bit pixel data and compress is using
	the RLE format used by TGA files.
	The format itself sucks eggs.
	I send either a 0x80-0xFF to denote 1 to 128 fill bytes
	0 to 0x7F is followed by Count+1 raw bytes

**********************************/

static void CompressTGALine565(FILE *fp,const Word8 *DataPtr,Word Length)
{
	Word Fill;		/* Word8 for run fill */
	Word Max;		/* Maximum run length */
	Word Count;		/* Current run length */
	Word8 DataBuf[16];	/* Word8 array for file writing */
	
	if (Length>=3) {	/* Less than 3 remaining? */
		do {
			Max = Length;			/* Maximum number of bytes to check */
			if (Max>128) {
				Max = 128;			/* Maximum run */
			}
			Fill = ((Word16 *)DataPtr)[0];			/* Get the first byte */
			if (!((((Word16 *)DataPtr)[1]^Fill)&0xFFDF)) {		/* Try for a run... */
				Count = 2;				/* Assume 2 bytes for fill */
				do {
					if ((((Word16 *)DataPtr)[Count]^Fill)&0xFFDF) {		/* Match? */
						break;				/* Get out now! */
					}
				} while (++Count<Max);
				DataBuf[0] = (Word8)Count+127;	/* Save the run length */
				Fill = (Fill&0x1F)|((Fill>>1)&0x7FE0);
				DataBuf[1] = (Word8)Fill;	/* Fill byte */
				DataBuf[2] = (Word8)(Fill>>8);
				fwrite(DataBuf,1,3,fp);
			} else {					/* This is for raw data */
				Count = 3;				/* Minimum of 3 */
				Max -= 2;
				if (Count<Max) {
					do {
						if (!((((Word16 *)DataPtr)[Count]^((Word16 *)DataPtr)[Count+1])&0xFFDF) &&
							!((((Word16 *)DataPtr)[Count+1]^((Word16 *)DataPtr)[Count+2])&0xFFDF)) {
							goto DoIt;
						}
					} while (++Count<Max);
					Count = Max;
				}
DoIt:
				fputc(Count-1,fp);
				Max = 0;
				do {
					Fill = ((Word16 *)DataPtr)[Max];
					Fill = (Fill&0x1F)|((Fill>>1)&0x7FE0);
					fwriteshortl((Word16)Fill,fp);	/* Save the data */
				} while (++Max<Count);
			}
			Length-=Count;			/* Remove length */
			DataPtr+=Count*2;		/* Adjust the source pointer */
		} while (Length>=3);		/* More remaining? */
	}

	switch (Length) {		/* Any data remaining? */
	case 2:
		Max = ((Word16 *)DataPtr)[0];	/* Fill byte */
		Fill = ((Word16 *)DataPtr)[1];
		Max = (Max&0x1F)|((Max>>1)&0x7FE0);
		Fill = (Fill&0x1F)|((Fill>>1)&0x7FE0);
		DataBuf[1] = (Word8)Max;
		DataBuf[2] = (Word8)(Max>>8);
		if (Max==Fill) {
			DataBuf[0] = 0x81;			/* Fill it */
			fwrite(DataBuf,1,3,fp);
			break;
		}
		DataBuf[0] = 1;			/* Save the single byte */
		DataBuf[3] = (Word8)Fill;
		DataBuf[4] = (Word8)(Fill>>8);
		fwrite(DataBuf,1,5,fp);
		break;
	case 1:
		Max = ((Word16 *)DataPtr)[0];
		Max = (Max&0x1F)|((Max>>1)&0x7FE0);
		DataBuf[0] = 0;				/* Save the single byte */
		DataBuf[1] = (Word8)Max;	/* Send the last byte */
		DataBuf[2] = (Word8)(Max>>8);
		fwrite(DataBuf,1,3,fp);
	}
}

/**********************************

	Take a line of 8 bit pixel data and compress is using
	the RLE format used by TGA files.
	The format itself sucks eggs.
	I send either a 0x80-0xFF to denote 1 to 128 fill bytes
	0 to 0x7F is followed by Count+1 raw bytes

**********************************/

#ifdef __BIGENDIAN__
#define MASK24 0xFFFFFF00
#else
#define MASK24 0xFFFFFF
#endif

static void CompressTGALine888(FILE *fp,const Word8 *DataPtr,Word Length)
{
	Word Fill;		/* Word8 for run fill */
	Word Max;		/* Maximum run length */
	Word Count;		/* Current run length */
	Word8 DataBuf[16];	/* Word8 array for file writing */
	
	if (Length>=3) {	/* Less than 3 remaining? */
		do {
			Max = Length;			/* Maximum number of bytes to check */
			if (Max>128) {
				Max = 128;			/* Maximum run */
			}
			Fill = ((Word32 *)DataPtr)[0];			/* Get the first byte */
			if (!((((Word32 *)&(DataPtr[3]))[0]^Fill)&MASK24)) {		/* Try for a run... */
				Count = 2;				/* Assume 2 bytes for fill */
				do {
					if ((((Word32 *)(&DataPtr[Count*3]))[0]^Fill)&MASK24) {		/* Match? */
						break;				/* Get out now! */
					}
				} while (++Count<Max);
				DataBuf[0] = (Word8)Count+127;	/* Save the run length */
				DataBuf[1] = DataPtr[2];	/* Fill byte */
				DataBuf[2] = DataPtr[1];
				DataBuf[3] = DataPtr[0];
				fwrite(DataBuf,1,4,fp);
			} else {					/* This is for raw data */
				Count = 3;				/* Minimum of 3 */
				Max -= 2;
				if (Count<Max) {
					do {
						if (!((((Word32 *)(&DataPtr[Count*3]))[0]^((Word32 *)(&DataPtr[(Count+1)*3]))[0])&MASK24) &&
							!((((Word32 *)(&DataPtr[(Count+1)*3]))[0]^((Word32 *)(&DataPtr[(Count+2)*3]))[0])&MASK24)) {
							goto DoIt;
						}
					} while (++Count<Max);
					Count = Max;
				}
DoIt:
				fputc(Count-1,fp);
				Max = 0;
				do {
					Fill = Max*3;
					fputc(DataPtr[Fill+2],fp);
					fputc(DataPtr[Fill+1],fp);
					fputc(DataPtr[Fill],fp);
				} while (++Max<Count);
			}
			Length-=Count;			/* Remove length */
			DataPtr+=Count*3;		/* Adjust the source pointer */
		} while (Length>=3);		/* More remaining? */
	}

	switch (Length) {		/* Any data remaining? */
	case 2:
		DataBuf[1] = DataPtr[2];
		DataBuf[2] = DataPtr[1];
		DataBuf[3] = DataPtr[0];
		if (DataPtr[0]==DataPtr[3] && DataPtr[1]==DataPtr[4] && DataPtr[2] == DataPtr[5]) {
			DataBuf[0] = 0x81;			/* Fill it */
			fwrite(DataBuf,1,4,fp);
			break;
		}
		DataBuf[0] = 1;			/* Save the single byte */
		DataBuf[4] = DataPtr[5];
		DataBuf[5] = DataPtr[4];
		DataBuf[6] = DataPtr[3];
		fwrite(DataBuf,1,7,fp);
		break;
	case 1:
		DataBuf[0] = 0;				/* Save the single byte */
		DataBuf[1] = DataPtr[2];	/* Send the last byte */
		DataBuf[2] = DataPtr[1];
		DataBuf[3] = DataPtr[0];
		fwrite(DataBuf,1,4,fp);
	}
}

/**********************************

	Take a line of 8 bit pixel data and compress is using
	the RLE format used by TGA files.
	The format itself sucks eggs.
	I send either a 0x80-0xFF to denote 1 to 128 fill bytes
	0 to 0x7F is followed by Count+1 raw bytes

**********************************/

static void CompressTGALine8888(FILE *fp,const Word8 *DataPtr,Word Length)
{
	Word Fill;		/* Word8 for run fill */
	Word Max;		/* Maximum run length */
	Word Count;		/* Current run length */
	Word8 DataBuf[16];	/* Word8 array for file writing */
	
	if (Length>=3) {	/* Less than 3 remaining? */
		do {
			Max = Length;			/* Maximum number of bytes to check */
			if (Max>128) {
				Max = 128;			/* Maximum run */
			}
			Fill = ((Word32 *)DataPtr)[0];			/* Get the first byte */
			if (((Word32 *)DataPtr)[1]==Fill) {		/* Try for a run... */
				Count = 2;				/* Assume 2 bytes for fill */
				do {
					if (((Word32 *)DataPtr)[Count]!=Fill) {		/* Match? */
						break;				/* Get out now! */
					}
				} while (++Count<Max);
				DataBuf[0] = (Word8)Count+127;	/* Save the run length */
				DataBuf[1] = DataPtr[2];	/* Fill byte */
				DataBuf[2] = DataPtr[1];
				DataBuf[3] = DataPtr[0];
				DataBuf[4] = DataPtr[3];
				fwrite(DataBuf,1,5,fp);
			} else {					/* This is for raw data */
				Count = 3;				/* Minimum of 3 */
				Max -= 2;
				if (Count<Max) {
					do {
						if ((((Word32 *)DataPtr)[Count]==((Word32 *)DataPtr)[Count+1]) &&
							(((Word32 *)DataPtr)[Count+1]==((Word32 *)DataPtr)[Count+2])) {
							goto DoIt;
						}
					} while (++Count<Max);
					Count = Max;
				}
DoIt:
				fputc(Count-1,fp);
				Max = 0;
				do {
					Fill = Max*4;
					fputc(DataPtr[Fill+2],fp);
					fputc(DataPtr[Fill+1],fp);
					fputc(DataPtr[Fill],fp);
					fputc(DataPtr[Fill+3],fp);
				} while (++Max<Count);
			}
			Length-=Count;			/* Remove length */
			DataPtr+=Count*4;		/* Adjust the source pointer */
		} while (Length>=3);		/* More remaining? */
	}

	switch (Length) {		/* Any data remaining? */
	case 2:
		DataBuf[1] = DataPtr[2];
		DataBuf[2] = DataPtr[1];
		DataBuf[3] = DataPtr[0];
		DataBuf[4] = DataPtr[3];
		if (((Word32 *)DataPtr)[0]==((Word32 *)DataPtr)[1]) {
			DataBuf[0] = 0x81;			/* Fill it */
			fwrite(DataBuf,1,5,fp);
			break;
		}
		DataBuf[0] = 1;			/* Save the single byte */
		DataBuf[5] = DataPtr[6];
		DataBuf[6] = DataPtr[5];
		DataBuf[7] = DataPtr[4];
		DataBuf[8] = DataPtr[7];
		fwrite(DataBuf,1,9,fp);
		break;
	case 1:
		DataBuf[0] = 0;				/* Save the single byte */
		DataBuf[1] = DataPtr[2];	/* Send the last byte */
		DataBuf[2] = DataPtr[1];
		DataBuf[3] = DataPtr[0];
		DataBuf[4] = DataPtr[3];
		fwrite(DataBuf,1,5,fp);
	}
}

/**********************************

	Write an Image_t structure out as a BMP file

**********************************/

Word BURGERCALL Image2TGAFile(Image_t *ImagePtr,const char *FileName,Word Compress)
{
	FILE *fp;				/* Open file record */
	Word8 *Buffer;			/* Pointer to saved memory */
	Word i;					/* Temp */
	Word32 Length;		/* 32 bit image offset */
	Word32 MemData;		/* 4 bytes of data */
	Word Depth;

	if (ImageValidateToSave(ImagePtr)) {	/* Is this a valid Image_t struct? */
		return TRUE;		/* Nope, error */
	}
	fp = OpenAFile(FileName,"wb");	/* Open the output file */
	if (!fp) {			/* Oh oh... */
		return TRUE;		/* I couldn't open the file! */
	}

	/* For true color images */
	
	fputc(0,fp);		/* Image identification field size in bytes */

	Depth = ImagePtr->DataType;
	if (Depth==15) {
		Depth = 16;
	}

	if (Depth==8) {
		fputc(1,fp);		/* 1 for a color image */
		if (Compress) {
			fputc(9,fp);
		} else {
			fputc(1,fp);		/* Data type present */
		}
		fwriteshort(0,fp);	/* Color map origin */
		fwriteshortl(256,fp);	/* Color map size */
		fputc(24,fp);		 	/* Size (In bits) for each color entry */
	} else {
		fputc(0,fp);		/* 1 for a color image */
		if (Compress) {
			fputc(10,fp);
		} else {
			fputc(2,fp);		/* Data type present */
		}
		fwriteshort(0,fp);	/* Color map origin */
		fwriteshort(0,fp);	/* Color map size */
		fputc(0,fp);		 	/* Size (In bits) for each color entry */
	}
	fwriteshort(0,fp);		/* Lower left x origin of image */
	fwriteshort(0,fp);		/* Lower left y origin of image */
	fwriteshortl((Word16)ImagePtr->Width,fp);		/* Width of the image in pixels */
	fwriteshortl((Word16)ImagePtr->Height,fp);		/* Height of the image in pixels */
	fputc(Depth,fp);	/* Bits per pixels for the image */
	fputc(0,fp);		/* Image descriptor bits */

	/* Now, here's where we send out the data */

	/* Code for the 8 bits per pixel version */

	MemData = 0;		/* Array of zeros at least 3 long */
	if (ImagePtr->DataType==IMAGE8_PAL) {		/* 8 bit image */
		Word8 Palette[768];				/* Palette to save to disk */
		Word8 *OutP;
		Buffer = ImagePtr->PalettePtr;	/* First save the palette */
		OutP = Palette;
		i = 256;
		do {
			OutP[0] = Buffer[2];		/* Blue */
			OutP[1] = Buffer[1];		/* Green */
			OutP[2] = Buffer[0];		/* Red */
			OutP+=3;
			Buffer=Buffer+3;		/* Next palette entry */
		} while (--i);				/* All done? */
		fwrite(Palette,1,768,fp);	/* Save to disk */
		i = ImagePtr->Height;		/* Height of image */
		Buffer = ImagePtr->ImagePtr + ((i-1)*(Word32)ImagePtr->RowBytes);	/* Bottom line */
		if (Compress) {
			do {
				CompressTGALine(fp,Buffer,ImagePtr->Width);	/* Compress a line */
				Buffer = Buffer-ImagePtr->RowBytes;		/* Next line up */
			} while (--i);		/* All done? */
		} else {
			do {
				fwrite(Buffer,1,ImagePtr->Width,fp);	/* Save a line to disk */
				Buffer = Buffer-ImagePtr->RowBytes;		/* Next line up */
			} while (--i);		/* All done? */
		}

	/* 15,16, 24 and 32 bits per pixel image will be stored as 24 bits */

	} else {
		i = ImagePtr->Height;

		/* 24 bit images */

		if (ImagePtr->DataType==IMAGE888) {	/* Send off the bytes */
			Length = (Word32)ImagePtr->RowBytes;	/* Size in bytes of each line */
			Buffer = ImagePtr->ImagePtr + ((i-1)*Length);	/* Bottommost line */
			Length = Length<<1;			/* 1 line size times 2 */
			if (Compress) {
				do {
					CompressTGALine888(fp,Buffer,ImagePtr->Width);	/* Compress a line */
					Buffer = Buffer-ImagePtr->RowBytes;		/* Next line up */
				} while (--i);		/* All done? */
			} else {
				do {
					Word j;
					j = ImagePtr->Width;		/* Get count */
					do {
						fputc(Buffer[2],fp);	/* Save out the scan line */
						fputc(Buffer[1],fp);
						fputc(Buffer[0],fp);
						Buffer=Buffer+3;		/* Next triplett */
					} while (--j);			/* Next pixel */
					Buffer = Buffer-Length;		/* Next line up (Sub line length * 2) */
				} while (--i);		/* All done? */
			}
		/* 32 bit images */

		} else if (ImagePtr->DataType==IMAGE8888) {
			Length = (Word32)ImagePtr->Width*4;	/* Size in bytes of each line */
			Buffer = ImagePtr->ImagePtr + ((i-1)*Length);	/* Bottommost line */
			Length = Length<<1;		/* 1 line size times 2 */
			if (Compress) {
				do {
					CompressTGALine8888(fp,Buffer,ImagePtr->Width);	/* Compress a line */
					Buffer = Buffer-ImagePtr->RowBytes;		/* Next line up */
				} while (--i);		/* All done? */
			} else {
				do {
					Word j;
					j = ImagePtr->Width;		/* Get count */
					do {
						fputc(Buffer[2],fp);	/* Save out the scan line */
						fputc(Buffer[1],fp);
						fputc(Buffer[0],fp);
						fputc(Buffer[3],fp);
						Buffer=Buffer+4;		/* Next triplett */
					} while (--j);			/* Next pixel */
					Buffer = Buffer-Length;		/* Next line up (Sub line length * 2) */
				} while (--i);		/* All done? */
			}
		} else {

	/* This routine saves 16 bit data as 24 bit data */

			Length = (Word32)ImagePtr->Width*2;	/* Length of line in bytes */
			Buffer = ImagePtr->ImagePtr + ((i-1)*Length);	/* Bottommost line */
			Length = Length<<1;		/* 1 line size times 2 */

			if (ImagePtr->DataType==IMAGE555) {
				if (Compress) {
					do {
						CompressTGALine555(fp,Buffer,ImagePtr->Width);	/* Compress a line */
						Buffer = Buffer-ImagePtr->RowBytes;		/* Next line up */
					} while (--i);		/* All done? */
				} else {
					do {
						Word j;
						j = ImagePtr->Width;		/* Get count */
						do {
							fwriteshortl(((Word16 *)Buffer)[0],fp);
							Buffer=Buffer+2;
						} while (--j);
						Buffer = Buffer-Length;		/* Next line up */
					} while (--i);
				}
			} else {
				if (Compress) {
					do {
						CompressTGALine565(fp,Buffer,ImagePtr->Width);	/* Compress a line */
						Buffer = Buffer-ImagePtr->RowBytes;		/* Next line up */
					} while (--i);		/* All done? */
				} else {
					do {
						Word j;
						j = ImagePtr->Width;		/* Get count */
						do {
							Word16 Pixel;
							Pixel = ((Word16 *)Buffer)[0];
							Pixel = (Pixel&0x1F)|((Pixel>>1)&0x7FE0);
							fwriteshortl(Pixel,fp);
							Buffer=Buffer+2;
						} while (--j);
						Buffer = Buffer-Length;		/* Next line up */
					} while (--i);
				}
			}
		}
	}

	/* Now wrap up... */

	fclose(fp);				/* Close the file */

#if defined(__MAC__)
	SetAFileType(FileName,'TPIC');		/* Set the Mac filetype */
	SetAnAuxType(FileName,'ogle');
#endif

	return FALSE;			/* I saved it! */
}


