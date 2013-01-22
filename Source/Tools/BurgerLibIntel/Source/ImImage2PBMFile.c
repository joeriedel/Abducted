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
	char TypeName[4];			/* PBM  */
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

	Save an Image_t structure as an ILBM file

**********************************/

static const Word8 PBMHeader1[20] = {
	'F','O','R','M',
	'X','X','X','X',
	'P','B','M',' ',
	'B','M','H','D',
	0,0,0,20			/* Size of the BMHD struct */
};

static const Word8 PBMHeader2[12] = {
	0,0,0,0,
	8,0,1,0,
	0,0,1,1
};

static const Word8 PBMHeader3[8] = {
	'C','M','A','P',
	0,0,3,0			/* Size of the palette */
};

Word BURGERCALL Image2PBMFile(Image_t *ImagePtr,const char *FileName)
{
	FILE *fp;
	Word i;
	Word32 Mark;
		
	if (ImageValidateToSave(ImagePtr)) {	/* Is this a valid Image_t struct? */
		return TRUE;						/* Nope, error */
	}
	if (ImagePtr->DataType<IMAGE8_PAL_ALPHA_PAL || ImagePtr->DataType>IMAGE8_PAL) {
		NonFatal("Only can process 8 bit images\n");
		return TRUE;
	}
	fp = OpenAFile(FileName,"wb");	/* Open the output file */
	if (!fp) {			/* Oh oh... */
		return TRUE;		/* I couldn't open the file! */
	}
	
	fwrite(PBMHeader1,1,20,fp);	/* ILBM header */
	fwriteshortb((Word16)ImagePtr->Width,fp);
	fwriteshortb((Word16)ImagePtr->Height,fp);
	fwrite(PBMHeader2,1,12,fp);	/* Standard data */
	fwriteshortb((Word16)ImagePtr->Width,fp);
	fwriteshortb((Word16)ImagePtr->Height,fp);
	
	/* 8 bits per pixel is easy! */
	
	fwrite(PBMHeader3,1,8,fp);	/* Standard data */
	fwrite(ImagePtr->PalettePtr,1,768,fp);

	Mark = ftell(fp)+4;				/* Here is where I'll save the data size */
	fwrite("BODYXXXX",1,8,fp);		/* Send the data for the body */

	/* Header is written, now for the data */
	
	{
		Word8 *Buffer;
		i = ImagePtr->Height;
		Buffer = ImagePtr->ImagePtr;
		do {
			ImageEncodeLBM(fp,Buffer,ImagePtr->Width);	/* Save off the data */
			Buffer += ImagePtr->RowBytes;				/* Next line to encode */
		} while (--i);
	}
	
	/* Now wrap up... */
	
	{
		Word32 SizeOffset;
		SizeOffset = (Word32)ftell(fp);
		fseek(fp,4,SEEK_SET);
		fwritelongb(SizeOffset-8,fp);	/* Save the size of the file */
		fseek(fp,Mark,SEEK_SET);
		fwritelongb((SizeOffset-Mark)-4,fp);	/* Save the size of the encoded data */
	}
	fclose(fp);				/* Close the file */
	
#if defined(__MAC__)
	SetAFileType(FileName,'ILBM');		/* Set the Mac filetype */
	SetAnAuxType(FileName,'BABL');
#endif

	return FALSE;			/* I saved it! */
}
