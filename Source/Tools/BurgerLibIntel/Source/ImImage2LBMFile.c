#include "ImImage.h"
#include "MmMemory.h"
#include "ClStdLib.h"
#include "PlPalette.h"
#include "FmFile.h"

/**********************************

	All data in an ILBM file is BIG endian

**********************************/

typedef struct MasterHeader_t {	/* Beginning of the file */
	char ID[4];					/* FORM */
	Word32 Length;			/* Size of the file data (Filesize-12) */
	char TypeName[4];			/* ILBM */
} MasterHeader_t;

typedef struct ILBMHeader_t {	/* BMHD record */
	char ID[4];					/* BMHD */
	Word32 Length;			/* Size of the record */
	Word16 Width;			/* Size of the final shape */
	Word16 Height;
	Word32 Dat1;			/* Reserved */
	Word8 Planes;			/* Bits per pixel (8) */
	Word8 Dat2;				/* 0 */
	Word8 Dat3;				/* 1 */
	Word8 Dat4;				/* 0 */
	Word16 Dat5;				/* 0,0 */
	Word8 Dat6;				/* 1 */
	Word8 Dat7;				/* 1 */
	Word16 Width2;			/* Another copy of the size */	
	Word16 Height2;
} ILBMHeader_t;

typedef struct ILBMPalette_t {	/* CMAP record */
	char ID[4];					/* CMAP */
	Word32 Length;			/* Size of the record */
	Word8 Palette[768];			/* RGB Tripletts */
} ILBMPalette_t;

typedef struct ILBMBody_t {		/* BODY record */
	char ID[4];					/* BODY */
	Word32 Length;			/* Size of the record */
	Word8 Data[1];				/* Raw packed data */
} ILBMBody_t;

/**********************************

	Compress RLE bitmap data
	The format is simplicity itself.
	If the byte is 128-255, then I have a run of 257-byte (2-129) with the fill
	byte following.
	If the byte is 0-127 then it is raw data following (+1 to the length)

**********************************/

void BURGERCALL ImageEncodeLBM(FILE *fp,const Word8 *SrcPtr,Word Length)
{
	Word Run;
	Word Temp;
	Word MaxRun;
	
	if (Length) {
		do {
			Temp = SrcPtr[0];	/* Check for repeater */
			if (Length==1) {	/* Only 1 byte remaining? */
				fputc(0,fp);
				fputc(Temp,fp);
				break;			/* Exit now! */
			}
			if (SrcPtr[1] == Temp) {	/* Repeat? */
				Run = 2;			/* Minimum data to pack */
				if (Length>=3) {	/* Is this all the data? */
					if (Length>=128) {	/* Maximum loops to try */
						MaxRun = 128-2;
					} else {
						MaxRun = Length-2;
					}
					do {
						if (SrcPtr[Run]!=Temp) {	/* Find end of repeater */
							break;
						}
						++Run;		/* 1 more to run */
					} while (--MaxRun);
				}

				/* Perform a run length token Run=2-129 */
				fputc(257-Run,fp);	/* 128-255 */
				fputc(Temp,fp);
			} else {
				Run = 2;		/* Raw of 2 */
				if (Length>=3) {		/* More than 2? */
					if (Length>=128) {	/* Get maximum */
						MaxRun = 128-2;
					} else {
						MaxRun = Length-2;
					}
					Temp = SrcPtr[1];	/* Preload the next byte */
					do {		/* Scan for next repeater */
						if (SrcPtr[Run]==Temp) {
							--Run;	/* Remove from the run */
							break;
						}
						Temp = SrcPtr[Run];	/* Get the next byte */
						++Run;		/* +1 to the run */
					} while (--MaxRun);
				}
				/* Perform a raw data transfer */
				fputc(Run-1,fp);
				fwrite(SrcPtr,1,Run,fp);
			}
			SrcPtr+=Run;
			Length-=Run;		/* Remove from data */
		} while (Length);		/* Any data left */
	}
}

/**********************************

	Take a line of 8 bit data and save it off as 8 bit planes.
	Each plane must be encoded seperately. Or bad things happen.

**********************************/

static void BURGERCALL EncodeLBMLine(FILE *fp,const Word8 *SrcPtr,Word Length,Word8 *WorkPtr)
{
	Word i;
	Word LineLength;
	Word Mask;
	
	i = 0;
	LineLength = ((Length+15)>>3)&(~1);	/* Even amount */
	Mask = 0x01;
	do {
		Word8 *DestPtr;
		Word DestMask;
		Word j;
		
		FastMemSet(WorkPtr,0,LineLength);		/* Init the output buffer */
		DestPtr = WorkPtr;		/* Here is where the data will go */
		j = 0;
		DestMask = 0x80;
		do {
			if (SrcPtr[j]&Mask) {
				DestPtr[0] = static_cast<Word8>(DestPtr[0]|DestMask);
			}
			DestMask>>=1;
			if (!DestMask) {
				DestMask = 0x80;
				++DestPtr;
			}
		} while (++j<Length);
		ImageEncodeLBM(fp,WorkPtr,LineLength);		/* Save the line of data */
		Mask<<=1;
	} while (++i<8);		/* All 8 planes done? */
}

/**********************************

	Save an Image_t structure as an ILBM file

**********************************/

Word BURGERCALL Image2LBMFile(Image_t *ImagePtr,const char *FileName)
{
	FILE *fp;
	Word i;
	Word32 Mark;
	Word8 *Buffer;
	Word8 *TempBuffer;
		
	if (ImageValidateToSave(ImagePtr)) {	/* Is this a valid Image_t struct? */
		return TRUE;						/* Nope, error */
	}	
	fp = OpenAFile(FileName,"wb");	/* Open the output file */
	if (!fp) {			/* Oh oh... */
		return TRUE;		/* I couldn't open the file! */
	}
	
	fwrite("FORMXXXXILBMBMHD",1,16,fp);	/* ILBM header */
	fwritelongb(20,fp);		/* Size of the BMHD struct */
	fwriteshortb((Word16)ImagePtr->Width,fp);
	fwriteshortb((Word16)ImagePtr->Height,fp);
	fwritelong(0,fp);
	i = 24;
	if (ImagePtr->DataType==IMAGE8_PAL) {
		i = 8;
	}
	fputc(i,fp);
	fputc(0,fp);
	fputc(1,fp);
	fputc(0,fp);
	fwriteshort(0,fp);
	fwriteshort(0x0101,fp);
	fwriteshortb((Word16)ImagePtr->Width,fp);
	fwriteshortb((Word16)ImagePtr->Height,fp);
	
	/* Header is written, now for the data */
	
	i = ImagePtr->Height;
	Buffer = ImagePtr->ImagePtr;
	
	/* 8 bits per pixel is easy! */
	
	if (ImagePtr->DataType==IMAGE8_PAL) {
		fwrite("CMAP",1,4,fp);
		fwritelongb(768,fp);
		fwrite(ImagePtr->PalettePtr,1,768,fp);
	}

	Mark = ftell(fp)+4;				/* Here is where I'll save the data size */
	fwrite("BODYXXXX",1,8,fp);		/* Send the data for the body */

	if (ImagePtr->DataType==IMAGE8_PAL) {
		TempBuffer = (Word8 *)AllocAPointer(ImagePtr->Width+256);		/* Big enough for padding */
		if (!TempBuffer) {
			goto OhShit;
		}
		do {
			EncodeLBMLine(fp,Buffer,ImagePtr->Width,TempBuffer);	/* Save off the data */
			Buffer += ImagePtr->RowBytes;				/* Next line to encode */
		} while (--i);
	} else {
	/* All the others make me throw up. */
		Word8 *WorkPtr;
		Word8 *Scratch;
		Word Width;
		TempBuffer = (Word8 *)AllocAPointer((ImagePtr->Width+64)*4);		/* Big enough for padding */
		if (!TempBuffer) {
			goto OhShit;
		}
		Scratch = TempBuffer + ((ImagePtr->Width+64)*3);		/* Use half the buffer for scratch */
		Width = ImagePtr->Width;
		do {
			Word j;
			Word8 *Buffer2;
			WorkPtr = TempBuffer;
			j = Width;
			Buffer2 = Buffer;
			do {
				if (ImagePtr->DataType==IMAGE888) {
					WorkPtr[0] = Buffer2[0];
					WorkPtr[Width] = Buffer2[1];
					WorkPtr[Width*2] = Buffer2[2];
					Buffer2+=3;
				} else if (ImagePtr->DataType==IMAGE8888) {
					WorkPtr[0] = Buffer2[0];
					WorkPtr[Width] = Buffer2[1];
					WorkPtr[Width*2] = Buffer2[2];
					Buffer2+=4;
				} else {
					Word8 Triplett[3];		/* Buffer for RGB triplett */
					if (ImagePtr->DataType==IMAGE555) {
						PaletteConvertRGB15ToRGB24(Triplett,((Word16 *)Buffer2)[0]);
					} else {
						PaletteConvertRGB16ToRGB24(Triplett,((Word16 *)Buffer2)[0]);
					}
					WorkPtr[0] = Triplett[0];
					WorkPtr[Width] = Triplett[1];
					WorkPtr[Width*2] = Triplett[2];
					Buffer2+=2;
				}
				++WorkPtr;
			} while (--j);
			
			EncodeLBMLine(fp,TempBuffer,ImagePtr->Width,Scratch);	/* Save off the data */
			EncodeLBMLine(fp,TempBuffer+Width,ImagePtr->Width,Scratch);	/* Save off the data */
			EncodeLBMLine(fp,TempBuffer+Width*2,ImagePtr->Width,Scratch);	/* Save off the data */
			Buffer = Buffer+ImagePtr->RowBytes;
		} while (--i);
	}
	DeallocAPointer(TempBuffer);
	
	/* Now wrap up... */
	
	Buffer = (Word8 *)ftell(fp);
	fseek(fp,4,SEEK_SET);
	fwritelongb((Word32)Buffer-8,fp);	/* Save the size of the file */
	fseek(fp,Mark,SEEK_SET);
	fwritelongb((Word32)Buffer-Mark-4,fp);	/* Save the size of the encoded data */
	fclose(fp);				/* Close the file */
	
#if defined(__MAC__)
	SetAFileType(FileName,'ILBM');		/* Set the Mac filetype */
	SetAnAuxType(FileName,'BABL');
#endif

	return FALSE;			/* I saved it! */
OhShit:
	fclose(fp);		/* Close the file */
	return TRUE;	/* Damn it! */

}
