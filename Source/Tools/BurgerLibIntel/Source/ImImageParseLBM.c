#include "ImImage.h"
#include <BREndian.hpp>
#include "ClStdLib.h"
#include "MmMemory.h"
#include "SnSound.h"

/**********************************

	All data in an ILBM file is BIG endian

**********************************/

typedef struct MasterHeader_t {	/* Beginning of the file */
	char ID[4];					/* FORM */
	Word32 Length;
	char TypeName[4];			/* ILBM or PBM' ' */
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

	Unpack data using ILBM compression
	I return the pointer to the next compressed run.

**********************************/

static Word8 * BURGERCALL UnpackILBMData(Word8 *DestBuffer,Word8 *InputPtr,Word32 Length)
{
	Word Run;
	if (Length) {
		do {
			Run = InputPtr[0];		/* Get the run token */
			if (Run & 0x80) {		/* Run length? */
				Word32 a;
				Run = 0x101-Run;	/* Count the run */
				if (Length<Run) {	/* Too large? */
					goto Foo;
				}
				Length = Length-Run;	/* Remove from count */
				a = InputPtr[1];
				InputPtr=InputPtr+2;
				a = a+(a<<8);
				a = a+(a<<16);		/* 32 bit value */
				if (Run>=16) {
					do {
						((Word32 *)DestBuffer)[0] = a;
						((Word32 *)DestBuffer)[1] = a;
						((Word32 *)DestBuffer)[2] = a;
						((Word32 *)DestBuffer)[3] = a;
						DestBuffer+=16;
						Run-=16;
					} while (Run>=16);
				}
				if (Run&8) {
					((Word32 *)DestBuffer)[0] = a;
					((Word32 *)DestBuffer)[1] = a;
					DestBuffer+=8;
				}
				if (Run&4) {
					((Word32 *)DestBuffer)[0] = a;
					DestBuffer+=4;
				}
				if (Run&2) {
					((Word16 *)DestBuffer)[0] = (Word16)a;
					DestBuffer+=2;
				}
				if (Run&1) {
					DestBuffer[0] = (Word8)a;
					DestBuffer+=1;
				}
			} else {
				++Run;		/* +1 to the count */
				++InputPtr;	/* Index to the true source data */
				if (Length<Run) {	/* Too large? */
					goto Foo;
				}
				Length = Length-Run;	/* Remove from count */
				if (Run>=16) {
					do {
						Word32 a,b;
						a = ((Word32 *)InputPtr)[0];
						b = ((Word32 *)InputPtr)[1];
						((Word32 *)DestBuffer)[0] = a;
						((Word32 *)DestBuffer)[1] = b;
						a = ((Word32 *)InputPtr)[2];
						b = ((Word32 *)InputPtr)[3];
						((Word32 *)DestBuffer)[2] = a;
						((Word32 *)DestBuffer)[3] = b;
						InputPtr+=16;
						DestBuffer+=16;
						Run-=16;
					} while (Run>=16);
				}
				if (Run&8) {
					Word32 a,b;
					a = ((Word32 *)InputPtr)[0];
					b = ((Word32 *)InputPtr)[1];
					((Word32 *)DestBuffer)[0] = a;
					((Word32 *)DestBuffer)[1] = b;
					InputPtr+=8;
					DestBuffer+=8;
				}
				if (Run&4) {
					Word32 a;
					a = ((Word32 *)InputPtr)[0];
					InputPtr+=4;
					((Word32 *)DestBuffer)[0] = a;
					DestBuffer+=4;
				}
				if (Run&2) {
					((Word16 *)DestBuffer)[0] = ((Word16 *)InputPtr)[0];
					InputPtr+=2;
					DestBuffer+=2;
				}
				if (Run&1) {
					DestBuffer[0] = InputPtr[0];
					InputPtr+=1;
					DestBuffer+=1;
				}
			}
		} while (Length);	/* More? */
	}
	return InputPtr;		/* Return the current data pointer */

Foo:;
	NonFatal("Data overrun in packed ILBM data.\n");
	return 0;
}

/**********************************

	Unpack bit planed data
	I support 8 and 24 bit data

**********************************/

static Word BURGERCALL UnpackILBM(Word8 *TempPtr,Word8 *PackBuffer,Word Width,Word Height,Word Depth)
{
	Word8 *LineBuffer;		/* Pointer to a bit plane buffer */
	Word PlaneStep;		/* Bytes per pixel plane (Padded to Word16) */
	Word Planes;		/* Plane # currently decoding */
	Word i,j;
	Word Mask,Input;
	Word PlaneMask;
	Word RetVal;
	Word PixelSize;

	PixelSize = (Depth+7)>>3;
	LineBuffer = (Word8 *)AllocAPointer((Width+16)*PixelSize);	/* Buffer to deinterleave memory */
	if (!LineBuffer) {			/* Error (How!!) */
		return FALSE;
	}
	PlaneStep = (Width+15)&(~15);
	PlaneStep=PlaneStep>>3;		/* Number of bytes per plane */
	RetVal = FALSE;		/* Assume OK */
	Width = Width*PixelSize;
	do {
		PackBuffer = UnpackILBMData(LineBuffer,PackBuffer,PlaneStep*Depth);
		if (!PackBuffer) {		/* Error? */
			RetVal = TRUE;		/* Set error code */
			break;			/* Exit */
		}
		FastMemSet(TempPtr,0,Width);	/* Clear out the old line */
		Planes = 0; 	/* Start at the first bit plane */
		PlaneMask = 1;		/* Init dest mask */
		do {
			j = Planes>>3;		/* Init dest index */
			i = PlaneStep*Planes;
			Mask = 0;
			do {
				if (!Mask) {		/* Mask needs to be set? */
					Input = LineBuffer[i];	/* Get a source byte */
					++i;
					Mask = 0x80;	/* Reset the mask */
				}
				if (Mask&Input) {	/* Shall I convert it? */
					TempPtr[j] |= (Word8)PlaneMask;	/* Save it */
				}
				Mask>>=1;		/* Adjust the mask */
				j+=PixelSize;
			} while(j<Width);	/* At the end? */
			PlaneMask = PlaneMask<<1;
			if (PlaneMask>=256) {	/* If 24 bit, then I need to loop the dest mask */
				PlaneMask = 1;
			}
		} while (++Planes<Depth);	/* All 8 bit planes done? */
		TempPtr=TempPtr+Width;	/* Next line down please */
	} while (--Height);
	DeallocAPointer(LineBuffer);		/* Release the line buffer */
	return RetVal;			/* Return error code if any */
}

/**********************************

	Load in an IBM LBM file and set the variables
	10/22/98 I added 24 bit color support
	return 0 if successful

**********************************/

#if defined(__BIGENDIAN__)
#define FORMASCII 0x464F524D
#define ILBMASCII 0x494C424D
#define PBMASCII 0x50424D20
#else
#define FORMASCII 0x4D524F46
#define ILBMASCII 0x4D424C49
#define PBMASCII 0x204D4250
#endif

Word BURGERCALL ImageParseLBM(Image_t *ImagePtr,const Word8 *Data)
{
	char *BadNews;
	Word FormType;
	Word Depth;
	Word8 *TempPtr;
	Word32 FileLength;

	FastMemSet(ImagePtr,0,sizeof(Image_t));		/* Init input struct */

	if (((Word32 *)&((MasterHeader_t *)Data)->ID[0])[0] != FORMASCII) {
		goto AbortBadForm;
	}
	FileLength = ((Word32 *)&((MasterHeader_t *)Data)->TypeName[0])[0];
	if (FileLength == ILBMASCII) {		/* Bit planes? */
		FormType = FALSE;
	} else if (FileLength != PBMASCII) {	/* PBM file? */
		goto AbortNotSupported;
	} else {
		FormType = TRUE;
	}

	FileLength = Burger::LoadBig(((MasterHeader_t *)Data)->Length)+8;		/* End of file */

	TempPtr = (Word8 *)FindIffChunk(Data,'BMHD',FileLength); /* Find the common chunk */
	if (!TempPtr) {
		goto OhShit;
	}
	ImagePtr->Width = Burger::LoadBig(((ILBMHeader_t*)TempPtr)->Width);		/* Get the width of the chunk */
	if (!ImagePtr->Width) {
		goto AbortBadWidth;
	}
	ImagePtr->Height = Burger::LoadBig(((ILBMHeader_t*)TempPtr)->Height);
	if (!ImagePtr->Height) {
		goto AbortBadHeight;
	}
	Depth = ((ILBMHeader_t*)TempPtr)->Planes;
	if (!Depth) {
		goto AbortBadDepth;
	}
	if (Depth > 9) {
		if (Depth!=24) {
			goto AbortBadDepth;		/* Not 8 or 24 bit! */
		}
		if (FormType) {
			goto AbortBadPBM;		/* I don't support 24 bit PBM files */
		}
	}

	if (Depth<9) {		/* Get the palette */
		Word32 PalSize;
		TempPtr = (Word8 *)AllocAPointerClear(768);		/* Get buffer for palette */
		if (!TempPtr) {
			goto OhShit;
		}
		ImagePtr->PalettePtr = TempPtr;
		TempPtr = (Word8 *)FindIffChunk(Data,'CMAP',FileLength); /* Read in the palette */
		if (!TempPtr) {
			goto OhShit;
		}
		PalSize = Burger::LoadBig(((ILBMPalette_t *)TempPtr)->Length);
		if (PalSize>768) {
			PalSize = 768;
		}
		FastMemCpy(ImagePtr->PalettePtr,&((ILBMPalette_t *)TempPtr)->Palette[0],PalSize);		/* Copy to my own memory */
	}

		/* Now process the 8 or 24 bit file */

	TempPtr = (Word8 *)AllocAPointer((Word32)ImagePtr->Width*(ImagePtr->Height*((Depth+7)>>3)));
	if (!TempPtr) {
		goto OhShit;
	}
	ImagePtr->ImagePtr = TempPtr;		/* Fill in the rest */
	TempPtr = (Word8 *)FindIffChunk(Data,'BODY',FileLength);	/* Pixel data field */
	if (!TempPtr) {
		goto OhShit;
	}
	TempPtr+=8;
	if (FormType) {		/* Chunky pixels (8 bit PBM only) */
		if (!UnpackILBMData(ImagePtr->ImagePtr,TempPtr,(Word32)ImagePtr->Width*ImagePtr->Height)) {
			goto OhShit;
		}
		ImagePtr->DataType = IMAGE8_PAL;
		ImagePtr->RowBytes = ImagePtr->Width;
		return FALSE;
	}
	if (UnpackILBM(ImagePtr->ImagePtr,TempPtr,ImagePtr->Width,ImagePtr->Height,Depth)) {
		goto OhShit;		/* Ok!! */
	}
	if (Depth==24) {
		ImagePtr->DataType = IMAGE888;
		ImagePtr->RowBytes = ImagePtr->Width*3;
	} else {
		ImagePtr->DataType = IMAGE8_PAL;
		ImagePtr->RowBytes = ImagePtr->Width;
	}
	return FALSE;		/* It's ok! */

AbortBadWidth:;
	BadNews ="Width can't be zero.\n";
	goto Abort;
AbortBadHeight:;
	BadNews ="Height can't be zero\n";
	goto Abort;
AbortBadPBM:;
	BadNews ="Can't process a 24 bit color PBM file\n";
	goto Abort;
AbortBadDepth:;
	BadNews ="Can't process anything but 8 or 24 bit color images\n";
	goto Abort;
AbortNotSupported:;
	BadNews = "Not a supported IFF file\n";
	goto Abort;
AbortBadForm:;
	BadNews = "No FORM record (Not an LBM or PBM File)\n";
Abort:;
	NonFatal(BadNews);
OhShit:;
	ImageDestroy(ImagePtr);		/* Dispose of any allocated memory */
	return TRUE;		/* Error! */
}



