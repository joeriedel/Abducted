#include "ImImage.h"
#include <BREndian.hpp>
#include "MmMemory.h"
#include "ClStdLib.h"
#include "PlPalette.h"

/**********************************

	Decode a CICN resource
	Note: All data is stored BIG ENDIAN

**********************************/

typedef struct CICNHeader_t {	/* Max pixel pattern */
	long BaseAddress;			/* Filler */
	Word16 RowBytes;				/* &0xE000 == 0x8000; &0x1FFF == rowbytes */
	short Top;					/* Bounds rect */
	short Left;
	short Bottom;
	short Right;
	short Version;				/* == 0 */
	short PackingFormat;		/* == 0 */
	long SizeOfData;			/* Filler */
	Fixed32 HResolution;			/* == 0x00480000 ; 72 Fixed32 */
	Fixed32 VResolution;			/* == 0x00480000 ; 72 Fixed32 */
	short PixelStorageFormat;	/* == 0; Chunky */
	short BitsPerPixel;			/* 4 or 8 = palette */
	short ComponentsPerPixel;	/* == 1 */
	short BitsPerField;			/* == 8 */
	long Filler1;				/* Offset to the next plane */
	Word32 ColorTableOffset;	/* Offset to the color table */
	long Filler2;				/* Reserved */
	long Filler3;
} CICNHeader_t;

typedef struct BitmapHeader_t {
	Word16 RowBytes;				/* &0xE000 == 0x8000; &0x1FFF == rowbytes */
	short Top;					/* Bounds rect */
	short Left;
	short Bottom;
	short Right;
	short Version;				/* == 0 */
	short PackingFormat;		/* == 0 */
} BitmapHeader_t;

typedef struct ColorTableHeader_t {
	Word32 ctSeed;			/* Quicktime random number seed */
	Word16 ctFlags;				/* Color flags */
	Word16 ctSize;				/* Number of entries that follow (Can be zero) */
} ColorTableHeader_t;

typedef struct ColorEntry_t {
	Word16 Index;				/* Color entry number 0-255 */
	Word8 Red[2];				/* Red component, 0-65535 */
	Word8 Green[2];
	Word8 Blue[2];
} ColorEntry_t;

/**********************************

	Decode a Macintosh big endian ppat resource
	This is a difficult file to parse
	
**********************************/

Word BURGERCALL ImageParseCicn(Image_t *ImagePtr,const Word8 *InputPtr)
{
	Word8 MyRemapTable[256];			/* Color remap table */
	Word RowBytes;					/* Rowbytes in source pixel data */
	Word8 *DestPtr;					/* Output */
	Word BitsPerPixel;				/* 4 or 8 is ok */
	BitmapHeader_t *MaskBitmapPtr;	/* Mask header */
	BitmapHeader_t *IconBitmapPtr;	/* Icon header */
	const Word8 *MaskBitMap;			/* Pointer to the mask image */
	const Word8 *IconBitMap;			/* Pointer to the 1 bit per pixel image */
	
	FastMemSet(ImagePtr,0,sizeof(Image_t));				/* Clear out result */
	
	/* Parse out the main header */
	
	{
		CICNHeader_t *PixMapHeaderPtr;
		PixMapHeaderPtr = (CICNHeader_t *)InputPtr;
	
		ImagePtr->Width = Burger::LoadBig(&PixMapHeaderPtr->Right) - Burger::LoadBig(&PixMapHeaderPtr->Left);	/* Get the size */
		ImagePtr->Height = Burger::LoadBig(&PixMapHeaderPtr->Bottom) - Burger::LoadBig(&PixMapHeaderPtr->Top);
		RowBytes = Burger::LoadBig(&PixMapHeaderPtr->RowBytes);
	
		if (PixMapHeaderPtr->Version != 0 || PixMapHeaderPtr->PackingFormat != 0 || (RowBytes & 0xE000) != 0x8000) {
			/* Header ok? */
			NonFatal("Invalid / unsupported cicn header header\n");
			goto OhShit;
		}
	
		BitsPerPixel = Burger::LoadBig(&PixMapHeaderPtr->BitsPerPixel);
		if (BitsPerPixel != 1 && BitsPerPixel != 2 && BitsPerPixel!=4 && BitsPerPixel!=8) {
			NonFatal("Cicn's must be 2, 4 or 8 bits per pixel!\n");
			goto OhShit;
		}
	}
	InputPtr += sizeof(CICNHeader_t);		/* Accept the header */
	
	/* Let's start setting the data */
	
	ImagePtr->DataType = IMAGE8_PAL;
	ImagePtr->RowBytes = ImagePtr->Width;

	if (!ImagePtr->Width) {
		NonFatal("Width can't be zero.");
		goto OhShit;
	}
	
	if (!ImagePtr->Height) {
		NonFatal("Height can't be zero.");
		goto OhShit;
	}	
	
	DestPtr = static_cast<Word8 *>(AllocAPointer((Word32)ImagePtr->Width*(Word32)ImagePtr->Height));
	if (!DestPtr) {
		goto OhShit;
	}
	ImagePtr->ImagePtr = DestPtr;
	
	/* Find the mask and 1 bit per pixel images */

	MaskBitmapPtr = (BitmapHeader_t *)InputPtr;
	InputPtr += sizeof(BitmapHeader_t);
	IconBitmapPtr = (BitmapHeader_t *)InputPtr;
	InputPtr += sizeof(BitmapHeader_t);
	
	MaskBitMap = InputPtr;
	InputPtr += (Burger::LoadBig(&MaskBitmapPtr->RowBytes)*ImagePtr->Height);
	IconBitMap = InputPtr;
	InputPtr += (Burger::LoadBig(&IconBitmapPtr->RowBytes)*ImagePtr->Height);

	/* Process the color data (Palette data) */
	/* Note: I have to track color #0 since that will become the */
	/* invisible color for the final image */
	
	{
		ColorTableHeader_t *ColorTableHeaderPtr;
		ColorEntry_t *ColorEntryPtr;
		Word Count;
		
		FastMemSet(MyRemapTable,0,sizeof(MyRemapTable));		/* Init the remap table */
		MyRemapTable[0] = 255;									/* Mark as unused */
		
		ColorTableHeaderPtr = (ColorTableHeader_t *)(InputPtr);
		Count = Burger::LoadBig(&ColorTableHeaderPtr->ctSize)+1;	/* Number of entries */
		DestPtr = (Word8 *)AllocAPointerClear(768);			/* Allocate the palette */
		if (!DestPtr) {
			goto OhShit;
		}
		ImagePtr->PalettePtr = DestPtr;		/* Save the palette */
		ColorEntryPtr = (ColorEntry_t *)(ColorTableHeaderPtr+1); 
	
		{						/* Are there any entries? */
			Word Index;
			
			do {
				Word8 *FooPtr;
				Index = Burger::LoadBig(&ColorEntryPtr->Index);
				if (Index<256) {				/* Because I am paranoid */
					MyRemapTable[Index] = static_cast<Word8>(Index);
					FooPtr = &DestPtr[Index*3];
					FooPtr[0] = ColorEntryPtr->Red[0];		/* Use the high 8 bits as big endian data */
					FooPtr[1] = ColorEntryPtr->Green[0];
					FooPtr[2] = ColorEntryPtr->Blue[0];
				}
				++ColorEntryPtr;
			} while (--Count);
		}
		InputPtr = (Word8 *)ColorEntryPtr;			/* Image follows the palette */
	}

	RowBytes &= 0x1FFF;			/* Fix the rowbytes for the image */

	/* Process the pixel data */
	
	{
		Word TempHeight;
		const Word8 *SrcPtr;
		
		SrcPtr = InputPtr;
		TempHeight = ImagePtr->Height;
		DestPtr = ImagePtr->ImagePtr;
		if (BitsPerPixel==8) {
			do {
				FastMemCpy(DestPtr,SrcPtr,ImagePtr->Width);
				DestPtr += ImagePtr->Width;	/* Next line down */
				SrcPtr += RowBytes;			/* Next line down */
			} while (--TempHeight);
		} else if (BitsPerPixel==4) {

			/* 4 bits per pixel */
			
			Word TempWidth;
			do {
				const Word8 *WorkPtr;
				Word i;
				Word Temp;

				TempWidth = ImagePtr->Width;
				WorkPtr = SrcPtr;
				i = TempWidth;
				do {
					Temp = WorkPtr[0];
					++WorkPtr;
					DestPtr[0] = static_cast<Word8>(Temp>>4);
					++DestPtr;
					if (!--TempWidth) {
						break;
					}
					DestPtr[0] = static_cast<Word8>(Temp&0x0F);
					++DestPtr;
				} while (--TempWidth);
				SrcPtr += RowBytes;			/* Next line down */
			} while (--TempHeight);
		} else if (BitsPerPixel==2) {
		
			/* 2 bits per pixel */
			
			Word TempWidth;
			do {
				const Word8 *WorkPtr;
				Word i;
				Word Temp;

				TempWidth = ImagePtr->Width;
				WorkPtr = SrcPtr;
				i = TempWidth;
				do {
					Temp = WorkPtr[0];
					++WorkPtr;
					DestPtr[0] = static_cast<Word8>(Temp>>6);
					++DestPtr;
					if (!--TempWidth) {
						break;
					}
					DestPtr[0] = static_cast<Word8>((Temp>>4)&3);
					++DestPtr;
					if (!--TempWidth) {
						break;
					}
					DestPtr[0] = static_cast<Word8>((Temp>>2)&3);
					++DestPtr;
					if (!--TempWidth) {
						break;
					}
					DestPtr[0] = static_cast<Word8>(Temp&3);
					++DestPtr;
				} while (--TempWidth);
				SrcPtr += RowBytes;			/* Next line down */
			} while (--TempHeight);
		} else if (BitsPerPixel==1) {
		
			/* 2 bits per pixel */
			
			Word TempWidth;
			do {
				const Word8 *WorkPtr;
				Word i;
				Word Temp;

				TempWidth = ImagePtr->Width;
				WorkPtr = SrcPtr;
				i = TempWidth;
				do {
					Temp = WorkPtr[0];
					++WorkPtr;
					DestPtr[0] = static_cast<Word8>((Temp>>7)&1);
					++DestPtr;
					if (!--TempWidth) {
						break;
					}
					DestPtr[0] = static_cast<Word8>((Temp>>6)&1);
					++DestPtr;
					if (!--TempWidth) {
						break;
					}
					DestPtr[0] = static_cast<Word8>((Temp>>5)&1);
					++DestPtr;
					if (!--TempWidth) {
						break;
					}
					DestPtr[0] = static_cast<Word8>((Temp>>4)&1);
					++DestPtr;
					if (!--TempWidth) {
						break;
					}
					DestPtr[0] = static_cast<Word8>((Temp>>3)&1);
					++DestPtr;
					if (!--TempWidth) {
						break;
					}
					DestPtr[0] = static_cast<Word8>((Temp>>2)&1);
					++DestPtr;
					if (!--TempWidth) {
						break;
					}
					DestPtr[0] = static_cast<Word8>((Temp>>1)&1);
					++DestPtr;
					if (!--TempWidth) {
						break;
					}
					DestPtr[0] = static_cast<Word8>(Temp&1);
					++DestPtr;
				} while (--TempWidth);
				SrcPtr += RowBytes;			/* Next line down */
			} while (--TempHeight);
		}		
	}
	
	/* Now, make color #0 the mask */
	
	if (MyRemapTable[0]!=255) {					/* Did it use color #0? */
		Word xx;
		Word8 *PalPtr;
		PalPtr = ImagePtr->PalettePtr;
		xx = 1;			/* Ignore color #0 */
		do {
			if (MyRemapTable[xx]!=xx) {
				Word8 *PalPtrxx;
				MyRemapTable[0] = static_cast<Word8>(xx);			/* Force a remap to here */
				PalPtrxx = &PalPtr[xx*3];
				PalPtrxx[0] = PalPtr[0];		/* Copy the color */
				PalPtrxx[1] = PalPtr[1];
				PalPtrxx[2] = PalPtr[2];
				break;
			}
		} while (++xx<255);
		
		if (xx>=256) {			/* Ok, all colors are used */
			MyRemapTable[0] = static_cast<Word8>(PaletteFindColorIndex(PalPtr+3,PalPtr[0],PalPtr[1],PalPtr[2],254)+1);
		}
		PalPtr[0] = 0;			/* Zap color #0 */
		PalPtr[1] = 0;
		PalPtr[2] = 0;
		ImageRemapIndexed(ImagePtr,MyRemapTable);		/* Force the remap */
	}
	
	/* Zap all the pixels the masks say that I should zap to zero */
	
	{
		Word TempHeight2;
		const Word8 *SrcPtr2;
		Word TempWidth2;
		
		RowBytes = Burger::LoadBig(&MaskBitmapPtr->RowBytes)&0x1FFF;		/* Mask row bytes */
		SrcPtr2 = MaskBitMap;
		TempHeight2 = ImagePtr->Height;
		DestPtr = ImagePtr->ImagePtr;
		do {
			const Word8 *WorkPtr2;
			Word Temp2;
			Word Flags;
			
			TempWidth2 = ImagePtr->Width;
			WorkPtr2 = SrcPtr2;
			Flags = 0x80;			/* Bit mask */
			Temp2 = WorkPtr2[0];
			++WorkPtr2;
			do {
				if (!(Temp2&Flags)) {		/* Invisible? */
					DestPtr[0] = 0;			/* Zap the pixel */
				}
				Flags >>= 1;
				if (!Flags) {				/* Need another bit? */
					Flags = 0x80;			/* Reset the flags */
					Temp2 = WorkPtr2[0];
					++WorkPtr2;
				}
				++DestPtr;
			} while (--TempWidth2);			/* All done? */
			SrcPtr2 += RowBytes;			/* Next line down */
		} while (--TempHeight2);
	}

	/* I am finally done! */

	return FALSE;				/* I am ok! */
OhShit:;
	ImageDestroy(ImagePtr);		/* Free the memory */
	return TRUE;				/* Decode or memory error */
}
