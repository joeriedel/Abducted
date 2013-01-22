#include "ImImage.h"
#include <BREndian.hpp>
#include "MmMemory.h"
#include "ClStdLib.h"
#include <string.h>

/**********************************

	Decode a ppat resource
	Note: All data is stored BIG ENDIAN

**********************************/

typedef struct PPatHeader_t {	/* Max pixel pattern */
	short Version;				/* == 1 */
	Word32 PixMapOffset;		/* Offset to PixMap info */
	Word32 PixelDataOffset;	/* Offset to Pixels */
	long ExpandedPixelImage;	/* Filler */
	short PatternValidFlag;		/* Filler */
	long ExpandedPattern;		/* Filler */
	Word8 Pattern[8];			/* Old style pattern */
} PPatHeader_t;

typedef struct PixMapHeader_t {
	long BaseAddress;			/* Filler */
	Word16 RowBytes;				/* &0xE000 == 0x8000; &0x1FFF == rowbytes */
	short Top;					/* Bounds rect */
	short Left;
	short Bottom;
	short Right;
	short Version;				/* == 0 */
	short PackingFormat;		/* == 0 */
	long SizeOfData;			/* Filler */
	Fixed32 HResolution;			/* == 0x00480000 ; 72 fixed */
	Fixed32 VResolution;			/* == 0x00480000 ; 72 fixed */
	short PixelStorageFormat;	/* == 0; Chunky */
	short BitsPerPixel;			/* 8 = palette */
	short ComponentsPerPixel;	/* == 1 */
	short BitsPerField;			/* == 8 */
	long Filler1;
	Word32 ColorTableOffset;
	long Filler2;
} PixMapHeader_t;

typedef struct ColorTableHeader_t {
	Word32 ctSeed;			/* Quicktime random number seed */
	short ctFlags;				/* Color flags */
	short ctSize;				/* Number of entries that follow (Can be zero) */
} ColorTableHeader_t;

typedef struct ColorEntry_t {
	short Index;				/* Color entry number 0-255 */
	Word16 Red;					/* Red component, 0-65535 */
	Word16 Green;
	Word16 Blue;
} ColorEntry_t;

/**********************************

	Decode a Macintosh big endian ppat resource
	This is a VERY simple file format, hard to auto detect.
	
**********************************/

Word BURGERCALL ImageParsePPat(Image_t *ImagePtr,const Word8 *InputPtr)
{
	Word32 PixMapOffset;			/* Offset to pixmap data */
	Word32 PixelDataOffset;		/* Offset to pixel data */
	Word32 ColorTableOffset;		/* Offset to color data */
	Word RowBytes;			/* Rowbytes in source pixel data */
	Word8 *DestPtr;			/* Output */
	
	FastMemSet(ImagePtr,0,sizeof(Image_t));				/* Clear out result */
	
	{
		PPatHeader_t *HeaderPtr;
		short Version;

		HeaderPtr = (PPatHeader_t *)InputPtr;			/* Get the pointer to the header */
		Version = Burger::LoadBig(&HeaderPtr->Version);
		PixMapOffset = Burger::LoadBig(&HeaderPtr->PixMapOffset);
		PixelDataOffset = Burger::LoadBig(&HeaderPtr->PixelDataOffset);
		
		if (Version != 1 || PixMapOffset < sizeof(PPatHeader_t) ||
			PixelDataOffset < (sizeof(PPatHeader_t) + sizeof(PixMapHeader_t))) {
			/* Header ok? */
			NonFatal("Invalid ppat header\n");
			goto OhShit;
		}
	}
	
	{
		PixMapHeader_t *PixMapHeaderPtr;
		PixMapHeaderPtr = (PixMapHeader_t *)(InputPtr + PixMapOffset);
	
		ImagePtr->Width = Burger::LoadBig(&PixMapHeaderPtr->Right) - Burger::LoadBig(&PixMapHeaderPtr->Left);	/* Get the size */
		ImagePtr->Height = Burger::LoadBig(&PixMapHeaderPtr->Bottom) - Burger::LoadBig(&PixMapHeaderPtr->Top);
		ColorTableOffset = Burger::LoadBig(&PixMapHeaderPtr->ColorTableOffset);
		RowBytes = Burger::LoadBig(&PixMapHeaderPtr->RowBytes);
	
		if (PixMapHeaderPtr->Version != 0 || PixMapHeaderPtr->PackingFormat != 0 || (RowBytes & 0xE000) != 0x8000) {
			/* Header ok? */
			NonFatal("Invalid / unsupported pixmapheader header\n");
			goto OhShit;
		}
	
		if (Burger::LoadBig(&PixMapHeaderPtr->BitsPerPixel)!=8) {
			NonFatal("ppats must be 8 bits per pixel!\n");
			goto OhShit;
		}
	}
	
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
	
	if (!ColorTableOffset) {		/* Is there a palette? */
		NonFatal("No global palette in ppat file\n");
		goto OhShit;
	}
	
	/* Process the pixel data */
	
	{
		Word TempHeight;
		const Word8 *SrcPtr;

		RowBytes &= 0x1FFF;
		SrcPtr = InputPtr + PixelDataOffset;
		TempHeight = ImagePtr->Height;
		do {
			FastMemCpy(DestPtr,SrcPtr,ImagePtr->Width);
			DestPtr += ImagePtr->Width;	/* Next line down */
			SrcPtr += RowBytes;			/* Next line down */
		} while (--TempHeight);
	}
	
	/* Process the color data */
	
	{
		ColorTableHeader_t *ColorTableHeaderPtr;
		ColorEntry_t *ColorEntryPtr;
		Word Count;
		
		ColorTableHeaderPtr = (ColorTableHeader_t *)(InputPtr + ColorTableOffset);
		Count = Burger::LoadBig(&ColorTableHeaderPtr->ctSize)+1;	/* Number of entries */
		DestPtr = (Word8 *)AllocAPointerClear(768);			/* Allocate the palette */
		if (!DestPtr) {
			goto OhShit;
		}
		ImagePtr->PalettePtr = DestPtr;		/* Save the palette */
	
		{						/* Are there any entries? */
			Word Index;
			
			ColorEntryPtr = (ColorEntry_t *)(ColorTableHeaderPtr+1); 
			do {
				Word8 *FooPtr;
				Index = Burger::LoadBig(&ColorEntryPtr->Index);
				if (Index<256) {				/* Because I am paranoid */
					FooPtr = &DestPtr[Index*3];
					FooPtr[0] = (Word8)(Burger::LoadBig(&ColorEntryPtr->Red) >> 8);
					FooPtr[1] = (Word8)(Burger::LoadBig(&ColorEntryPtr->Green) >> 8);
					FooPtr[2] = (Word8)(Burger::LoadBig(&ColorEntryPtr->Blue) >> 8);
				}
				++ColorEntryPtr;
			} while (--Count);
		}
	}
	return FALSE;				/* I am ok! */
	
OhShit:;
	ImageDestroy(ImagePtr);		/* Free the memory */
	return TRUE;				/* Decode or memory error */
}
