/**********************************

	Read in a Macintosh PICT file

**********************************/

#include "ImImage.h"
#include <BREndian.hpp>
#include "MMMemory.h"
#include "ClStdLib.h"
#include "LrRect.h"

#if defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <QDOffscreen.h>
#include <ImageCompression.h>
#endif
#if defined(__MAC__)
#include <QDOffscreen.h>
#include <ImageCompression.h>
#endif
#if defined(__MACOSX__)
#include <stdbool.h>
#include <QD/QDOffscreen.h>
#include <QuickTime/ImageCompression.h>
#endif

#define DirectClass 0
#define MaxTextExtent 1024
#define IndexPacket Word
#define PixelPacket Word

#if 0
typedef struct ImageInfo {
	Word foo;
} ImageInfo;

typedef struct Image {
	Word columns;
	Word rows;
	Word matte;
	Word storage_class;
} Image;

typedef struct ExceptionInfo {
	Word foo;
} ExceptionInfo;
#endif

typedef struct PICTPixmap_t {
	short version;
	short pack_type;
	Word32 pack_size;
	Word32 horizontal_resolution;
	Word32 vertical_resolution;
	short pixel_type;
	short bits_per_pixel;
	short component_count;
	short component_size;
	Word32 plane_bytes;
	Word32 table;
	Word32 reserved;
} PICTPixmap_t;

#define ReadRectangle(rectangle) { \
	rectangle.top=Burger::LoadBig((Word16 *)(InputPtr+0)); \
	rectangle.left=Burger::LoadBig((Word16 *)(InputPtr+2)); \
	rectangle.bottom=Burger::LoadBig((Word16 *)(InputPtr+4)); \
	rectangle.right=Burger::LoadBig((Word16 *)(InputPtr+6)); InputPtr+=8; \
}

static const int codes[] = {
	0,		/* 0x00 NOP */
	0,		/* 0x01 Clip */
	8,		/* 0x02 BkPat*/
	2,		/* 0x03 TxFont */
	2,		/* 0x04 TxFace */
	2,		/* 0x05 TxMode */
	4,		/* 0x06 SpExtra */
	4,		/* 0x07 PnSize */
	2,		/* 0x08 PnMode */
	8,		/* 0x09 PnPat */
	8,		/* 0x0a FillPat */
	4,		/* 0x0b OvSize */
	4,		/* 0x0c Origin */
	2,		/* 0x0d TxSize */ 
	4,		/* 0x0e FgColor */
	4,		/* 0x0f BkColor */
	8,		/* 0x10 TxRatio */
	1,		/* 0x11 Version */
	0,		/* 0x12 BkPixPat */
	0,		/* 0x13 PnPixPat */
	0,		/* 0x14 FillPixPat */
	2,		/* 0x15 PnLocHFrac */
	2,		/* 0x16 ChExtra */
	0,		/* 0x17 reserved */
	0,		/* 0x18 reserved */
	0,		/* 0x19 reserved */
	6,		/* 0x1a RGBFgCol */
	6,		/* 0x1b RGBBkCol */
	0,		/* 0x1c HiliteMode */
	6,		/* 0x1d HiliteColor */
	0,		/* 0x1e DefHilite */
	6,		/* 0x1f OpColor */
	8,		/* 0x20 Line */
	4,		/* 0x21 LineFrom */
	6,		/* 0x22 ShortLine */
	2,		/* 0x23 ShortLineFrom */
	-1,		/* 0x24 reserved */
	-1,		/* 0x25 reserved */
	-1,		/* 0x26 reserved */
	-1,		/* 0x27 reserved */
	0,		/* 0x28 LongText */
	0,		/* 0x29 DHText */
	0,		/* 0x2a DVText */
	0,		/* 0x2b DHDVText */
	-1,		/* 0x2c reserved */
	-1,		/* 0x2d reserved */
	-1,		/* 0x2e reserved */
	-1,		/* 0x2f reserved */
	8,		/* 0x30 frameRect */
	8,		/* 0x31 paintRect */
	8,		/* 0x32 eraseRect */
	8,		/* 0x33 invertRect */
	8,		/* 0x34 fillRect */
	8,		/* 0x35 reserved */
	8,		/* 0x36 reserved */
	8,		/* 0x37 reserved */
	0,		/* 0x38 frameSameRect */
	0,		/* 0x39 paintSameRect */
	0,		/* 0x3a eraseSameRect */
	0,		/* 0x3b invertSameRect */
	0,		/* 0x3c fillSameRect */
	0,		/* 0x3d reserved */
	0,		/* 0x3e reserved */
	0,		/* 0x3f reserved */
	8,		/* 0x40 frameRRect */
	8,		/* 0x41 paintRRect */
	8,		/* 0x42 eraseRRect */
	8,		/* 0x43 invertRRect */
	8,		/* 0x44 fillRRrect */
	8,		/* 0x45 reserved */
	8,		/* 0x46 reserved */
	8,		/* 0x47 reserved */
	0,		/* 0x48 frameSameRRect */
	0,		/* 0x49 paintSameRRect */
	0,		/* 0x4a eraseSameRRect */
	0,		/* 0x4b invertSameRRect */
	0,		/* 0x4c fillSameRRect */
	0,		/* 0x4d reserved */
	0,		/* 0x4e reserved */
	0,		/* 0x4f reserved */
	8,		/* 0x50 frameOval */
	8,		/* 0x51 paintOval */
	8,		/* 0x52 eraseOval */
	8,		/* 0x53 invertOval */
	8,		/* 0x54 fillOval */
	8,		/* 0x55 reserved */
	8,		/* 0x56 reserved */
	8,		/* 0x57 reserved */
	0,		/* 0x58 frameSameOval */
	0,		/* 0x59 paintSameOval */
	0,		/* 0x5a eraseSameOval */
	0,		/* 0x5b invertSameOval */
	0,		/* 0x5c fillSameOval */
	0,		/* 0x5d reserved */
	0,		/* 0x5e reserved */
	0,		/* 0x5f reserved */
	12,		/* 0x60 frameArc */
	12,		/* 0x61 paintArc */
	12,		/* 0x62 eraseArc */
	12,		/* 0x63 invertArc */
	12,		/* 0x64 fillArc */
	12,		/* 0x65 reserved */
	12,		/* 0x66 reserved */
	12,		/* 0x67 reserved */
	4,		/* 0x68 frameSameArc */
	4,		/* 0x69 paintSameArc */
	4,		/* 0x6a eraseSameArc */
	4,		/* 0x6b invertSameArc */
	4,		/* 0x6c fillSameArc */
	4,		/* 0x6d reserved */
	4,		/* 0x6e reserved */
	4,		/* 0x6f reserved */
	0,		/* 0x70 framePoly */
	0,		/* 0x71 paintPoly */
	0,		/* 0x72 erasePoly */
	0,		/* 0x73 invertPoly */
	0,		/* 0x74 fillPoly */
	0,		/* 0x75 reserved */
	0,		/* 0x76 reserved */
	0,		/* 0x77 reserved */
	0,		/* 0x78 frameSamePoly */
	0,		/* 0x79 paintSamePoly */
	0,		/* 0x7a eraseSamePoly */
	0,		/* 0x7b invertSamePoly */
	0,		/* 0x7c fillSamePoly */
	0,		/* 0x7d reserved */
	0,		/* 0x7e reserved */
	0,		/* 0x7f reserved */
	0,		/* 0x80 frameRgn */
	0,		/* 0x81 paintRgn */
	0,		/* 0x82 eraseRgn */
	0,		/* 0x83 invertRgn */
	0,		/* 0x84 fillRgn */
	0,		/* 0x85 reserved */
	0,		/* 0x86 reserved */
	0,		/* 0x87 reserved */
	0,		/* 0x88 frameSameRgn */
	0,		/* 0x89 paintSameRgn */
	0,		/* 0x8a eraseSameRgn */
	0,		/* 0x8b invertSameRgn */
	0,		/* 0x8c fillSameRgn */
	0,		/* 0x8d reserved */
	0,		/* 0x8e reserved */
	0,		/* 0x8f reserved */
	0,		/* 0x90 BitsRect */
	0,		/* 0x91 BitsRgn */
	-1,		/* 0x92 reserved */
	-1,		/* 0x93 reserved */
	-1,		/* 0x94 reserved */
	-1,		/* 0x95 reserved */
	-1,		/* 0x96 reserved */
	-1,		/* 0x97 reserved */
	0,		/* 0x98 PackBitsRect */
	0,		/* 0x99 PackBitsRgn */
	0,		/* 0x9a DirectBitsRect */
	0,		/* 0x9b DirectBitsRgn */
	-1,		/* 0x9c reserved */
	-1,		/* 0x9d reserved */
	-1,		/* 0x9e reserved */
	-1,		/* 0x9f reserved */
	2,		/* 0xa0 ShortComment */
	0		/* 0xa1 LongComment */
};

static const Word8 MacStandardPalette[768] = {
	0xff,0xff,0xff
};

/**********************************

	Read a pix map header from a pict file stream

**********************************/

static const Word8 *ReadPixmap(PICTPixmap_t *Output,const Word8 *InputPtr)
{
	Output->version=Burger::LoadBig((Word16 *)(InputPtr+0));
	Output->pack_type=Burger::LoadBig((Word16 *)(InputPtr+2));
	Output->pack_size=Burger::LoadBig((Word32 *)(InputPtr+4));
	Output->horizontal_resolution=Burger::LoadBig((Word32 *)(InputPtr+8));
	Output->vertical_resolution=Burger::LoadBig((Word32 *)(InputPtr+12));
	Output->pixel_type=Burger::LoadBig((Word16 *)(InputPtr+16));
	Output->bits_per_pixel=Burger::LoadBig((Word16 *)(InputPtr+18));
	Output->component_count=Burger::LoadBig((Word16 *)(InputPtr+20));
	Output->component_size=Burger::LoadBig((Word16 *)(InputPtr+22));
	Output->plane_bytes=Burger::LoadBig((Word32 *)(InputPtr+24));
	Output->table=Burger::LoadBig((Word32 *)(InputPtr+28));
	Output->reserved=Burger::LoadBig((Word32 *)(InputPtr+32));
	InputPtr+=36;
	return InputPtr;
}

/**********************************

	Convert 1,2,4,8,16 and 32 bits per pixel data

**********************************/

static const Word8 *ExpandBuffer(Word8 *TempBuffer,const Word8 *SrcBuf,
	Word32 *BytesPerLine,Word Depth)
{
	Word32 Count;

	Count = BytesPerLine[0];
	if (Count) {
		switch (Depth) {
		case 8:
		case 16:
		case 24:
		case 32:
			return SrcBuf;
		case 4:
			{
				Word8 *Dest4;
				BytesPerLine[0] = Count*2;
				Dest4 = TempBuffer;
				do {
					Word Temp4;
					Temp4 = SrcBuf[0];
					++SrcBuf;
				
					Dest4[0] = static_cast<Word8>(Temp4 >> 4);
					Dest4[1] = static_cast<Word8>(Temp4 & 15);
					Dest4+=2;
				} while (--Count);
			}
			break;
	
		case 2:
			{
				Word8 *Dest2;
				BytesPerLine[0] = Count*4;
				Dest2 = TempBuffer;
				do {
					Word Temp2;
					Temp2 = SrcBuf[0];
					++SrcBuf;
					
					Dest2[0] = static_cast<Word8>((Temp2 >> 6)&3);
					Dest2[1] = static_cast<Word8>((Temp2 >> 4)&3);
					Dest2[2] = static_cast<Word8>((Temp2 >> 2)&3);
					Dest2[3] = static_cast<Word8>(Temp2 & 3);
					Dest2+=4;
				} while (--Count);
			}
			break;
		case 1:
			{
				Word8 *Dest1;
				BytesPerLine[0] = Count*8;
				Dest1 = TempBuffer;
				do {
					Word Temp1;
					Temp1 = SrcBuf[0];
					++SrcBuf;
					
					Dest1[0] = static_cast<Word8>((Temp1 >> 7)&1);
					Dest1[1] = static_cast<Word8>((Temp1 >> 6)&1);
					Dest1[2] = static_cast<Word8>((Temp1 >> 5)&1);
					Dest1[3] = static_cast<Word8>((Temp1 >> 4)&1);
					Dest1[4] = static_cast<Word8>((Temp1 >> 3)&1);
					Dest1[5] = static_cast<Word8>((Temp1 >> 2)&1);
					Dest1[6] = static_cast<Word8>((Temp1 >> 1)&1);
					Dest1[7] = static_cast<Word8>(Temp1 & 1);
					Dest1+=8;
				} while (--Count);
			}
		default:			/* You are kind of screwed */
			break;				
		}
	}
	return TempBuffer;
}

/**********************************

	Decompress the image data and return the pointer to the data

**********************************/

static Word8 *DecodeImage(const Word8 *PackedPtr,Image_t *image,Word32 bytes_per_line,Word bits_per_pixel)
{
	Word LineWidth;
	Word8 *scanlinebuffer;
	Word j;
	Word i;
	const Word8 *p;
	Word8 *q;
	Word32 length;
	Word8 *pixels;
	Word8 *scanline;
	Word row_bytes;
	Word bytes_per_pixel;
	Word32 number_pixels;
	Word scanline_length;

/* Determine pixel buffer size. */

//	if (bits_per_pixel <= 8) {		/* Paletted image */
		bytes_per_line&=0x7fff;		/* Fix the bytes per line */
//	}
	LineWidth=image->Width;				/* Number of pixels wide the image is */
	bytes_per_pixel=1;				/* Size of each pixel group */
	if (bits_per_pixel == 16) {
		bytes_per_pixel = 2;		/* Will decompress 2 bytes per pixel */
		LineWidth*=2;
	} else if (bits_per_pixel == 32) {
		LineWidth*=4;					/* Increase the buffer size */
	} else if (bits_per_pixel == 24) {
		LineWidth*=3;					/* 3 bytes per pixel */
	}
	if (!bytes_per_line) {
		bytes_per_line=LineWidth;		/* Use my own rowbytes */
	}
	row_bytes=(4*image->Width);

	/* Allocate pixel and scanline buffer. */

	pixels=(Word8 *)AllocAPointerClear(LineWidth*image->Height);
	if (pixels) {
		scanline=(unsigned char *) AllocAPointer(row_bytes*2);
		if (scanline) {
			Word8 *DestPtr;
			Word Height;
			
			scanlinebuffer = scanline+row_bytes;		/* Other buffer */
			Height = image->Height;
			DestPtr = pixels;
			
			/* Uncompress RLE pixels into uncompressed pixel buffer. */

			do {
				if (bytes_per_line < 8) {
					Word32 MemPixelsCount;
					MemPixelsCount=bytes_per_line;		/* Number of compressed bytes */
					p=ExpandBuffer(scanlinebuffer,PackedPtr,&MemPixelsCount,bits_per_pixel);
					FastMemCpy(DestPtr,p,MemPixelsCount);
					PackedPtr+=bytes_per_line;			/* Move the source pointer */
				} else {
					scanline_length=PackedPtr[0];		/* Grab the run length */
					++PackedPtr;
					if (bytes_per_line > /*200*/ 250) {		/* Shall I be parsing shorts? (Note the docs say 250, but reality says 200) */
						scanline_length=(scanline_length<<8)+PackedPtr[0];
						++PackedPtr;
					}
					if (scanline_length > row_bytes) {		/* Too large? */
						NonFatal("Buffer overrun");
						DeallocAPointer(pixels);
						pixels = 0;
						break;
					}
					if (scanline_length) {
						Word Sanity;
						q = DestPtr;
						FastMemCpy(scanline,PackedPtr,scanline_length);
						PackedPtr+=scanline_length;
						j = 0;
						Sanity = LineWidth;
						do {
							length = scanline[j];
							if (!(length & 0x80)) {
								++length;
								number_pixels=length*bytes_per_pixel;
								p=ExpandBuffer(scanlinebuffer,scanline+j+1,&number_pixels,bits_per_pixel);
								if (Sanity<number_pixels) {
									number_pixels = Sanity;
								}
								Sanity -= number_pixels;
								FastMemCpy(q,p,number_pixels);
								q+=number_pixels;
								j+=length*bytes_per_pixel+1;
							} else {
								length=(length^0xff)+2;
								number_pixels=bytes_per_pixel;
								p=ExpandBuffer(scanlinebuffer,scanline+j+1,&number_pixels,bits_per_pixel);
								do {
									if (Sanity<number_pixels) {
										number_pixels = Sanity;
									}
									Sanity -= number_pixels;
									FastMemCpy(q,p,number_pixels);
									q+=number_pixels;
								} while (--length);
								j+=bytes_per_pixel+1;
							}
						} while (j<scanline_length);
					}
				}			
				if (bits_per_pixel==24) {
					const Word8 *Src1;
					Word Width1;
					Word Width2;
				
					FastMemCpy(scanlinebuffer,DestPtr,LineWidth);
					Src1 = scanlinebuffer;
					q = DestPtr;
					Width1 = image->Width;
					if (Width1) {
						Width2 = Width1*2;
						i = Width1;
						do {
							q[0] = Src1[0];
							q[1] = Src1[Width1];
							q[2] = Src1[Width2];
							q+=3;
							++Src1;
						} while (--i);
					}
				} else if (bits_per_pixel==32) {
					const Word8 *Src1;
					Word Width1;
					Word Width2;
				
					FastMemCpy(scanlinebuffer,DestPtr+image->Width,LineWidth-image->Width);
					Src1 = scanlinebuffer;
					q = DestPtr;
					Width1 = image->Width;
					if (Width1) {
						Width2 = Width1*2;
						i = Width1;
						do {
							q[0] = Src1[0];
							q[1] = Src1[Width1];
							q[2] = Src1[Width2];
							q+=3;
							++Src1;
						} while (--i);
					}
					DestPtr-=image->Width;		/* Hack to fake a mul by 3 instead of mul by 4 */
				
				}
#if defined(__LITTLEENDIAN__)
				else if (bits_per_pixel==16) {		/* Endian swap for Intel */
					Word Swap;
					Swap = image->Width;
					if (Swap) {
						q = DestPtr;
						do {
							((Word16 *)q)[0] = Burger::SwapEndian(((Word16 *)q)[0]);
							q+=2;
						} while (--Swap);
					}
				}
#endif
				DestPtr+=LineWidth;
			} while (--Height);
			DeallocAPointer(scanline);
			return pixels;
		}
		DeallocAPointer(scanline);
	}
	return 0;
}

#if 0

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P I C T I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadPICTImage reads an Apple Macintosh QuickDraw/PICT image file
%  and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadPICTImage method is:
%
%      Image *ReadPICTImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadPICTImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/
static Image *ReadPICTImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
	char geometry[MaxTextExtent];
	Image *image;
	IndexPacket index;
	int c,Opcode;
	long PicFlags,j,PictVersion,y;
	LBRect PicSize;
	PICTPixmap_t pixmap;
	register IndexPacket *indexes;
	register long x;
	register PixelPacket *q;
	register long i;
	size_t length;
	unsigned int JPegFound,status;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AllocateImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
//  if (status == FALSE)
  //  ThrowReaderException(FileOpenError,"UnableToOpenFile",image);
  /*
    Read PICT header.
  */
  for (i=0; i < 512; i++)
    (void) ReadBlobByte(image);  /* skip header */
  (void) ReadBlobMSBShort(image);  /* skip picture size */
  ReadRectangle(PicSize);
  while ((c=ReadBlobByte(image)) == 0) {}
//  if (c != 0x11)
//    ThrowReaderException(CorruptImageError,"NotAPICTImageFile",image);
  PictVersion=ReadBlobByte(image);
  if (PictVersion == 2)
    {
      c=ReadBlobByte(image);
//      if (c != 0xff)
//        ThrowReaderException(CorruptImageError,"NotAPICTImageFile",image);
    }
  else {
//    if (PictVersion != 1)
//      ThrowReaderException(CorruptImageError,"NotAPICTImageFile",image);
	}
  /*
    Create black canvas.
  */
  PicFlags=0;
  image->columns=PicSize.right-PicSize.left;
  image->rows=PicSize.bottom-PicSize.top;
  /*
    Interpret PICT opcodes.
  */
  Opcode=0;
  JPegFound=FALSE;
  
  
  
  for ( ; ; )
  {
    if (image_info->ping && (image_info->subrange != 0))
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    if ((PictVersion == 1) || ((TellBlob(image) % 2) != 0))
      Opcode=ReadBlobByte(image);
    if (PictVersion == 2)
      Opcode=ReadBlobMSBShort(image);
    if (Opcode > 0xa1)
      {
      }
    else
      {
         switch (Opcode)
        {
          case 0x01:
          {
            /*
              Clipping rectangle.
            */
            length=ReadBlobMSBShort(image);
            if (length != 0x000a)
              {
                for (i=0; i < (long) (length-2); i++)
                  (void) ReadBlobByte(image);
                break;
              }
            ReadRectangle(PicSize);
            if ((PicSize.left & 0x8000) || (PicSize.top & 0x8000))
              break;
            image->columns=PicSize.right-PicSize.left;
            image->rows=PicSize.bottom-PicSize.top;
            SetImage(image,OpaqueOpacity);
            break;
          }
          case 0x12:
          case 0x13:
          case 0x14:
          {
            long
              pattern;

            unsigned long
              height,
              width;

            /*
              Skip pattern definition.
            */
            pattern=ReadBlobMSBShort(image);
            for (i=0; i < 8; i++)
              (void) ReadBlobByte(image);
            if (pattern == 2)
              {
                for (i=0; i < 5; i++)
                  (void) ReadBlobByte(image);
                break;
              }
            if (pattern != 1)
              ThrowReaderException(CorruptImageError,"UnknownPatternType",
                image);
            length=ReadBlobMSBShort(image);
            ReadRectangle(PicSize);
            InputPtr = ReadPixmap(&pixmap,InputPtr);
            (void) ReadBlobMSBLong(image);
            PicFlags=ReadBlobMSBShort(image);
            length=ReadBlobMSBShort(image);
            for (i=0; i <= (long) length; i++)
              (void) ReadBlobMSBLong(image);
            width=PicSize.bottom-PicSize.top;
            height=PicSize.right-PicSize.left;
            image->depth=pixmap.bits_per_pixel <= 8 ? 8 : QuantumDepth;
            if (pixmap.bits_per_pixel < 8)
              image->depth=8;
            if (pixmap.bits_per_pixel <= 8)
              length&=0x7fff;
            if (pixmap.bits_per_pixel == 16)
              width<<=1;
            if (length == 0)
              length=width;
            if (length < 8)
              {
                for (i=0; i < (long) (length*height); i++)
                  (void) ReadBlobByte(image);
              }
            else
              for (j=0; j < (int) height; j++)
                if (length > 250)
                  for (j=0; j < ReadBlobMSBShort(image); j++)
                    (void) ReadBlobByte(image);
                else
                  for (j=0; j < ReadBlobByte(image); j++)
                    (void) ReadBlobByte(image);
            break;
          }
          case 0x1b:
          {
            /*
              Initialize image background color.
            */
            image->background_color.red=(Quantum)
              ScaleShortToQuantum(ReadBlobMSBShort(image));
            image->background_color.green=(Quantum)
              ScaleShortToQuantum(ReadBlobMSBShort(image));
            image->background_color.blue=(Quantum)
              ScaleShortToQuantum(ReadBlobMSBShort(image));
            break;
          }
          case 0x70:
          case 0x71:
          case 0x72:
          case 0x73:
          case 0x74:
          case 0x75:
          case 0x76:
          case 0x77:
          {
            /*
              Skip polygon or region.
            */
            length=ReadBlobMSBShort(image);
            for (i=0; i < (long) (length-2); i++)
              (void) ReadBlobByte(image);
            break;
          }
          case 0x90:
          case 0x91:
          case 0x98:
          case 0x99:
          case 0x9a:
          case 0x9b:
          {
			long bytes_per_line;
			LBRect source,destination;
			register unsigned char *p;
			size_t j;
			unsigned char *pixels;
			Image *tile_image;

            /*
              Pixmap clipped by a rectangle.
            */
            bytes_per_line=0;
            if ((Opcode != 0x9a) && (Opcode != 0x9b))
              bytes_per_line=ReadBlobMSBShort(image);
            else
              {
                (void) ReadBlobMSBShort(image);
                (void) ReadBlobMSBShort(image);
                (void) ReadBlobMSBShort(image);
              }
            ReadRectangle(PicSize);
            /*
              Initialize tile image.
            */
            tile_image=image;
            if ((PicSize.left != 0) || (PicSize.top != 0) ||
                (PicSize.right != (long) image->columns) ||
                (PicSize.bottom != (long) image->rows) || JPegFound)
              tile_image=CloneImage(image,PicSize.right-PicSize.left,
                PicSize.bottom-PicSize.top,True,exception);
            if (tile_image == (Image *) NULL)
              return((Image *) NULL);
            if ((Opcode == 0x9a) || (Opcode == 0x9b) || (bytes_per_line & 0x8000))
              {
                InputPtr = ReadPixmap(&pixmap,InputPtr);
                tile_image->matte=pixmap.component_count == 4;
              }
            if ((Opcode != 0x9a) && (Opcode != 0x9b))
              {
                /*
                  Initialize colormap.
                */
                tile_image->colors=2;
                if (bytes_per_line & 0x8000)
                  {
                    (void) ReadBlobMSBLong(image);
                    PicFlags=ReadBlobMSBShort(image);
                    tile_image->colors=ReadBlobMSBShort(image)+1;
                  }
                if (!AllocateImageColormap(tile_image,tile_image->colors))
                  {
                    DestroyImage(tile_image);
                    ThrowReaderException(ResourceLimitError,
                      "MemoryAllocationFailed",image)
                  }
                if (bytes_per_line & 0x8000)
                  {
                    for (i=0; i < (long) tile_image->colors; i++)
                    {
                      j=ReadBlobMSBShort(image) % tile_image->colors;
                      if (PicFlags & 0x8000)
                        j=i;
                      tile_image->colormap[j].red=(Quantum)
                        ScaleShortToQuantum(ReadBlobMSBShort(image));
                      tile_image->colormap[j].green=(Quantum)
                        ScaleShortToQuantum(ReadBlobMSBShort(image));
                      tile_image->colormap[j].blue=(Quantum)
                        ScaleShortToQuantum(ReadBlobMSBShort(image));
                    }
                  }
                else
                  {
                    for (i=0; i < (long) tile_image->colors; i++)
                    {
                      tile_image->colormap[i].red=(Quantum) (MaxRGB-
                        tile_image->colormap[i].red);
                      tile_image->colormap[i].green=(Quantum) (MaxRGB-
                        tile_image->colormap[i].green);
                      tile_image->colormap[i].blue=(Quantum) (MaxRGB-
                        tile_image->colormap[i].blue);
                    }
                  }
              }
            ReadRectangle(source);
            ReadRectangle(destination);
            (void) ReadBlobMSBShort(image);
            if ((Opcode == 0x91) || (Opcode == 0x99) || (Opcode == 0x9b))
              {
                /*
                  Skip region.
                */
                length=ReadBlobMSBShort(image);
                for (i=0; i <= (long) (length-2); i++)
                  (void) ReadBlobByte(image);
              }
            if ((Opcode != 0x9a) && (Opcode != 0x9b) &&
                (bytes_per_line & 0x8000) == 0)
              pixels=DecodeImage(image_info,image,tile_image,bytes_per_line,1);
            else
              pixels=DecodeImage(image_info,image,tile_image,bytes_per_line,pixmap.bits_per_pixel);
            if (pixels == (unsigned char *) NULL)
              {
                DestroyImage(tile_image);
                ThrowReaderException(ResourceLimitError,
                  "MemoryAllocationFailed",image)
              }
            /*
              Convert PICT tile image to pixel packets.
            */
            p=pixels;
            for (y=0; y < (long) tile_image->rows; y++)
            {
              q=SetImagePixels(tile_image,0,y,tile_image->columns,1);
              if (q == (PixelPacket *) NULL)
                break;
              indexes=GetIndexes(tile_image);
              for (x=0; x < (long) tile_image->columns; x++)
              {
                if (tile_image->storage_class == PseudoClass)
                  {
                    index=ConstrainColormapIndex(image,*p);
                    indexes[x]=index;
                    q->red=tile_image->colormap[index].red;
                    q->green=tile_image->colormap[index].green;
                    q->blue=tile_image->colormap[index].blue;
                  }
                else
                  {
                    if (pixmap.bits_per_pixel == 16)
                      {
                        i=(*p++);
                        j=(*p);
                        q->red=ScaleCharToQuantum((i & 0x7c) << 1);
                        q->green=ScaleCharToQuantum(((i & 0x03) << 6) |
                          ((j & 0xe0) >> 2));
                        q->blue=ScaleCharToQuantum((j & 0x1f) << 3);
                      }
                    else
                      if (!tile_image->matte)
                        {
                          q->red=ScaleCharToQuantum(*p);
                          q->green=
                            ScaleCharToQuantum(*(p+tile_image->columns));
                          q->blue=ScaleCharToQuantum(*(p+2*tile_image->columns));
                        }
                      else
                        {
                          q->opacity=(Quantum) (MaxRGB-ScaleCharToQuantum(*p));
                          q->red=ScaleCharToQuantum(*(p+tile_image->columns));
                          q->green=(Quantum)
                            ScaleCharToQuantum(*(p+2*tile_image->columns));
                          q->blue=
                           ScaleCharToQuantum(*(p+3*tile_image->columns));
                        }
                  }
                p++;
                q++;
              }
              if (!SyncImagePixels(tile_image))
                break;
              if ((tile_image->storage_class == DirectClass) &&
                  (pixmap.bits_per_pixel != 16))
                p+=(pixmap.component_count-1)*tile_image->columns;
              if (destination.bottom == (long) image->rows)
                if (QuantumTick(y,tile_image->rows))
                  if (!MagickMonitor(LoadImageText,y,tile_image->rows,&image->exception))
                    break;
            }
            (void) LiberateMemory((void **) &pixels);
            if (tile_image != image)
              {
                if (!JPegFound)
                  if ((Opcode == 0x9a) || (Opcode == 0x9b) ||
                      (bytes_per_line & 0x8000))
                    (void) CompositeImage(image,CopyCompositeOp,tile_image,
                      destination.left,destination.top);
                DestroyImage(tile_image);
              }
            if (destination.bottom != (long) image->rows)
              if (!MagickMonitor(LoadImageText,destination.bottom,image->rows,&image->exception))
                break;
            break;
          }
          case 0xa1:
          {
            unsigned char
              *info;

            unsigned long
              type;

            /*
              Comment.
            */
            type=ReadBlobMSBShort(image);
            length=ReadBlobMSBShort(image);
            if (length == 0)
              break;
            (void) ReadBlobMSBLong(image);
            length-=4;
            if (length == 0)
              break;
            info=(unsigned char *) AcquireMemory(length);
            if (info == (unsigned char *) NULL)
              break;
            (void) ReadBlob(image,length,info);
            switch (type)
            {
              case 0xe0:
              {
                image->color_profile.info=(unsigned char *)
                  AcquireMemory(length);
                if (image->color_profile.info == (unsigned char *) NULL)
                  ThrowReaderException(ResourceLimitError,
                    "MemoryAllocationFailed",image);
                image->color_profile.length=length;
                (void) memcpy(image->color_profile.info,info,length);
                break;
              }
              case 0x1f2:
              {
                image->iptc_profile.info=(unsigned char *)
                  AcquireMemory(length);
                if (image->iptc_profile.info == (unsigned char *) NULL)
                  ThrowReaderException(ResourceLimitError,
                    "MemoryAllocationFailed",image);
                image->iptc_profile.length=length;
                (void) memcpy(image->iptc_profile.info,info,length);
                break;
              }
              default:
                break;
            }
            LiberateMemory((void **) &info);
            break;
          }
          default:
          {
            /*
              Skip to next op code.
            */
            if (codes[Opcode].length == -1)
              (void) ReadBlobMSBShort(image);
            else
              for (i=0; i < (long) codes[Opcode].length; i++)
                (void) ReadBlobByte(image);
          }
        }
      }
    if (Opcode == 0xc00)
      {
        /*
          Skip header.
        */
        for (i=0; i < 24; i++)
          (void) ReadBlobByte(image);
        continue;
      }
    if (((Opcode >= 0xb0) && (Opcode <= 0xcf)) ||
        ((Opcode >= 0x8000) && (Opcode <= 0x80ff)))
      continue;
    if (Opcode == 0x8200)
      {
        FILE
          *file;

        Image
          *tile_image;

        ImageInfo
          *clone_info;

        /*
          Embedded JPEG.
        */
        JPegFound=TRUE;
        clone_info=CloneImageInfo(image_info);
        clone_info->blob=(void *) NULL;
        clone_info->length=0;
        TemporaryFilename(clone_info->filename);
        file=fopen(clone_info->filename,"wb");
        if (file == (FILE *) NULL)
          ThrowReaderException(FileOpenError,"UnableToWriteFile",image);
        length=ReadBlobMSBLong(image);
        for (i=0; i < 6; i++)
          (void) ReadBlobMSBLong(image);
        ReadRectangle(PicSize);
        for (i=0; i < 122; i++)
          (void) ReadBlobByte(image);
        for (i=0; i < (long) (length-154); i++)
        {
          c=ReadBlobByte(image);
          (void) fputc(c,file);
        }
        (void) fclose(file);
        tile_image=ReadImage(clone_info,exception);
        DestroyImageInfo(clone_info);
        (void) remove(clone_info->filename);
        if (tile_image == (Image *) NULL)
          continue;
        FormatString(geometry,"%lux%lu",Max(image->columns,tile_image->columns),
          Max(image->rows,tile_image->rows));
        (void) TransformImage(&image,(char *) NULL,geometry);
        (void) CompositeImage(image,CopyCompositeOp,tile_image,PicSize.left,
          PicSize.right);
        DestroyImage(tile_image);
        continue;
      }
    if ((Opcode == 0xff) || (Opcode == 0xffff))
      break;
    if (((Opcode >= 0xd0) && (Opcode <= 0xfe)) ||
        ((Opcode >= 0x8100) && (Opcode <= 0xffff)))
      {
        /*
          Skip reserved.
        */
        length=ReadBlobMSBShort(image);
        for (i=0; i < (long) length; i++)
          (void) ReadBlobByte(image);
        continue;
      }
    if ((Opcode >= 0x100) && (Opcode <= 0x7fff))
      {
        /*
          Skip reserved.
        */
        length=(Opcode >> 7) & 0xff;
        for (i=0; i < (long) length; i++)
          (void) ReadBlobByte(image);
        continue;
      }
  }
  if (EOFBlob(image))
    ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile",image);
  CloseBlob(image);
  return(image);
}
#endif

/**********************************

	Decode a Macintosh big endian pict resource
	This is a difficult file to parse
	
**********************************/

Word BURGERCALL ImageParsePict(Image_t *ImagePtr,const Word8 *InputPtr)
{
//	Word8 MyRemapTable[256];			/* Color remap table */
//	Word RowBytes;					/* Rowbytes in source pixel data */
//	Word8 *DestPtr;					/* Output */
	Word Opcode;					/* PICT File opcode */
	Word PictVersion;
	Word PicFlags;
	Word JPegFound;
	Word EvenOdd;
	LBRect PicSize;
//	Word BitsPerPixel;				/* 4 or 8 is ok */
//	BitmapHeader_t *MaskBitmapPtr;	/* Mask header */
//	BitmapHeader_t *IconBitmapPtr;	/* Icon header */
//	const Word8 *MaskBitMap;			/* Pointer to the mask image */
//	const Word8 *IconBitMap;			/* Pointer to the 1 bit per pixel image */
	PICTPixmap_t pixmap;
	
	FastMemSet(ImagePtr,0,sizeof(Image_t));				/* Clear out result */

	EvenOdd = (Word)InputPtr&1;		/* Is the data already aligned? FALSE if so, TRUE if odd alignment */
	
	/* Parse out the main header */
	{
		Word Foo1;
		ImagePtr->Width = Burger::LoadBig((Word16 *)(InputPtr+8))-Burger::LoadBig((Word16 *)(InputPtr+4));
		ImagePtr->Height = Burger::LoadBig((Word16 *)(InputPtr+6))-Burger::LoadBig((Word16 *)(InputPtr+2));
		InputPtr+=10;
		
		/* Scan for 0x00, 0x11 pattern */
		
		do {
			Foo1 = InputPtr[0];
			++InputPtr;
		} while (!Foo1);
		if (Foo1!=0x011) {
			NonFatal("0x11 not found in PICT image. Probably bad data.\n");
			goto OhShit;
		}
		
		/* I support vesion 0x02, 0x0FF or 0x01 only */

		PictVersion = InputPtr[0];
		if (PictVersion==2) {
			if (InputPtr[1]!=0xFF) {
				NonFatal("0x02,0xFF not found in header. Bad version?\n");
				goto OhShit;
			}
			++InputPtr;
		} else if (PictVersion!=1) {
			NonFatal("Bad version\n");
			goto OhShit;
		}
		++InputPtr;
	}
	PicFlags=0;
	JPegFound=FALSE;
	Opcode = 0;

	for ( ; ; ) {
		if ((PictVersion == 1) || (((Word)InputPtr & 1)!=EvenOdd)) {
			Opcode=InputPtr[0];
			++InputPtr;
		}
		if (PictVersion == 2) {
			Opcode=Burger::LoadBig((Word16 *)InputPtr);
			InputPtr+=2;
		}
		if (Opcode<=0xA1) {
			switch (Opcode) {

			/* Clipping rectangle. */

			case 0x01:
				{
					Word RectLen;
					RectLen=Burger::LoadBig(((Word16 *)InputPtr));
					if (RectLen == 10) {			/* I don't support regions */
						if (!(InputPtr[2] & 0x80) && !(InputPtr[4] & 0x80)) {
							Word NewWidth,NewHeight;
							NewWidth = Burger::LoadBig((Word16 *)(InputPtr+8))-Burger::LoadBig((Word16 *)(InputPtr+4));
							NewHeight = Burger::LoadBig((Word16 *)(InputPtr+6))-Burger::LoadBig((Word16 *)(InputPtr+2));
							ImagePtr->Width = NewWidth;
							ImagePtr->Height = NewHeight;
						}
					}
					InputPtr+=RectLen;
				}
				break;

			case 0x12:
			case 0x13:
			case 0x14:
				NonFatal("Found unsupported pattern opcode 0x%02X\n",Opcode);
				goto OhShit;
//				break;
			
			/* Background color */
			
			case 0x1b:
//				Burger::LoadBig((Word16 *)(InputPtr+0));	/* Red */
//				Burger::LoadBig((Word16 *)(InputPtr+2));	/* Green */
//				Burger::LoadBig((Word16 *)(InputPtr+4));	/* Blue */
				InputPtr += 6;
				break;
	
			/* Polygons */
			
			case 0x70:
			case 0x71:
			case 0x72:
			case 0x73:
			case 0x74:
			case 0x75:
			case 0x76:
			case 0x77:
				InputPtr+=Burger::LoadBig((Word16 *)(InputPtr));
				break;

			/* Pixmap clipped by a rectangle. */

			case 0x90:
			case 0x91:
			case 0x98:
			case 0x99:
			case 0x9a:
			case 0x9b:
				{
					long bytes_per_line;
					LBRect source,destination;
//					unsigned char *p;
//					size_t j;
					unsigned char *pixels;
					Image_t *tile_image;

					bytes_per_line=0;
					if ((Opcode != 0x9a) && (Opcode != 0x9b)) {
						bytes_per_line=Burger::LoadBig((Word16 *)InputPtr);
						InputPtr += 2;
					} else {
						InputPtr += 4;
						bytes_per_line = Burger::LoadBig((Word16 *)InputPtr);	//	top bit = color QD
						InputPtr += 2;
					}
					PicSize.top = Burger::LoadBig((short *)(InputPtr+0));
					PicSize.left = Burger::LoadBig((short *)(InputPtr+2));
					PicSize.bottom = Burger::LoadBig((short *)(InputPtr+4));
					PicSize.right = Burger::LoadBig((short *)(InputPtr+6));
					InputPtr+=8;
					
					/* Initialize tile image. */

					tile_image=ImagePtr;
					if ((PicSize.left) || (PicSize.top) ||
						(PicSize.right != (long) ImagePtr->Width) ||
						(PicSize.bottom != (long) ImagePtr->Height) || JPegFound) {
//						tile_image=CloneImage(image,PicSize.right-PicSize.left,PicSize.bottom-PicSize.top,True,exception);
					}
					if (!tile_image) {
						goto OhShit;
					}
					if ((Opcode == 0x9a) || (Opcode == 0x9b) || (bytes_per_line & 0x8000)) {
						InputPtr = ReadPixmap(&pixmap,InputPtr);
						if (pixmap.bits_per_pixel==32) {
							if (pixmap.component_count != 4) {
								pixmap.bits_per_pixel = 24;
							}
						}
					}
					if ((Opcode != 0x9a) && (Opcode != 0x9b)) {
						Word ColorCount;
						if (!ImagePtr->PalettePtr) {
							ImagePtr->PalettePtr = (Word8 *)AllocAPointerClear(768);
							FastMemCpy(ImagePtr->PalettePtr,MacStandardPalette,768);
						}
						ColorCount = 2;			/* Assume just two colors */
						if (bytes_per_line & 0x8000) {
							Word kk;
							PicFlags=Burger::LoadBig((Word16 *)(InputPtr+4));
							ColorCount=Burger::LoadBig((Word16 *)(InputPtr+6))+1;
							InputPtr+=8;
							kk = 0;
							do {
								Word8 *PalPtrC2;
								Word CIndex;
								CIndex = Burger::LoadBig((Word16 *)InputPtr);
								if (PicFlags & 0x8000) {
									CIndex = kk;
								}
								if (CIndex<256) {
									PalPtrC2 = &ImagePtr->PalettePtr[CIndex*3];
									PalPtrC2[0] = InputPtr[2];
									PalPtrC2[1] = InputPtr[4];
									PalPtrC2[2] = InputPtr[6];
								}
								InputPtr+=8;
							} while (++kk<ColorCount);
						}
					}
					source.top = Burger::LoadBig((short *)(InputPtr+0));
					source.left = Burger::LoadBig((short *)(InputPtr+2));
					source.bottom = Burger::LoadBig((short *)(InputPtr+4));
					source.right = Burger::LoadBig((short *)(InputPtr+6));
					destination.top = Burger::LoadBig((short *)(InputPtr+8));
					destination.left = Burger::LoadBig((short *)(InputPtr+10));
					destination.bottom = Burger::LoadBig((short *)(InputPtr+12));
					destination.right = Burger::LoadBig((short *)(InputPtr+14));
					InputPtr+=18;
					if ((Opcode == 0x91) || (Opcode == 0x99) || (Opcode == 0x9b)) {
						/* Skip region. */
						Word RegLen;
						RegLen=Burger::LoadBig((short *)(InputPtr+0));
						InputPtr += RegLen;
					}
					if ((Opcode != 0x9a) && (Opcode != 0x9b) && !(bytes_per_line & 0x8000)) {
						pixmap.bits_per_pixel = 1;
					}
					pixels=DecodeImage(InputPtr,tile_image,bytes_per_line,pixmap.bits_per_pixel);
					ImagePtr->ImagePtr = pixels;
					if (pixmap.bits_per_pixel<=8) {
						ImagePtr->RowBytes = ImagePtr->Width;
						ImagePtr->DataType = IMAGE8_PAL;
						if (!ImagePtr->PalettePtr) {
							ImagePtr->PalettePtr = (Word8 *)AllocAPointer(768);
							FastMemCpy(ImagePtr->PalettePtr,MacStandardPalette,768);
						}
					} else if (pixmap.bits_per_pixel==16) {
						ImagePtr->RowBytes = ImagePtr->Width*2;
						ImagePtr->DataType = IMAGE555;
					} else {
						ImagePtr->RowBytes = ImagePtr->Width*3;
						ImagePtr->DataType = IMAGE888;
					}
					return 0;

#if 0
					if (!pixels) {
						DestroyImage(tile_image);
						ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed",image)
					}

					/* Convert PICT tile image to pixel packets. */
					p=pixels;
					for (y=0; y < (long) tile_image->rows; y++) {
						q=SetImagePixels(tile_image,0,y,tile_image->columns,1);
						if (q == (PixelPacket *) NULL)
							break;
						indexes=GetIndexes(tile_image);
						for (x=0; x < (long) tile_image->columns; x++) {
							if (tile_image->storage_class == PseudoClass) {
								index=ConstrainColormapIndex(image,*p);
								indexes[x]=index;
								q->red=tile_image->colormap[index].red;
								q->green=tile_image->colormap[index].green;
								q->blue=tile_image->colormap[index].blue;
							} else {
								if (pixmap.bits_per_pixel == 16) {
									i=(*p++);
									j=(*p);
									q->red=ScaleCharToQuantum((i & 0x7c) << 1);
									q->green=ScaleCharToQuantum(((i & 0x03) << 6) |
										((j & 0xe0) >> 2));
									q->blue=ScaleCharToQuantum((j & 0x1f) << 3);
								} else if (!tile_image->matte) {		/* 24 bits per pixel? */
									q->red=ScaleCharToQuantum(*p);
									q->green=ScaleCharToQuantum(*(p+tile_image->columns));
									q->blue=ScaleCharToQuantum(*(p+2*tile_image->columns));
								} else {
									q->opacity=(Quantum) (MaxRGB-ScaleCharToQuantum(*p));
									q->red=ScaleCharToQuantum(*(p+tile_image->columns));
									q->green=(Quantum)ScaleCharToQuantum(*(p+2*tile_image->columns));
									q->blue=ScaleCharToQuantum(*(p+3*tile_image->columns));
								}
							}
							p++;
							q++;
						}
//						if (!SyncImagePixels(tile_image))
//							break;
//						if ((tile_image->storage_class == DirectClass) && (pixmap.bits_per_pixel != 16))
//							p+=(pixmap.component_count-1)*tile_image->columns;
//						if (destination.bottom == (long) image->rows)
//							if (QuantumTick(y,tile_image->rows))
//								if (!MagickMonitor(LoadImageText,y,tile_image->rows,&image->exception))
//									break;
//					}
#endif
//					(void) LiberateMemory((void **) &pixels);
//					if (tile_image != image) {
//						if (!JPegFound)
//							if ((Opcode == 0x9a) || (Opcode == 0x9b) || (bytes_per_line & 0x8000))
//								(void) CompositeImage(image,CopyCompositeOp,tile_image,
//						destination.left,destination.top);
//						DestroyImage(tile_image);
//					}
//					if (destination.bottom != (long) image->rows)
//						if (!MagickMonitor(LoadImageText,destination.bottom,image->rows,&image->exception))
//							break;
//					break;
				}

			/* Comment field */
							
			case 0xa1:
				{
					Word32 type;
					Word CommentLen;
					
					type=Burger::LoadBig((Word16 *)(InputPtr+0));
					CommentLen=Burger::LoadBig((Word16 *)(InputPtr+2));
					InputPtr += 4;
//					if (CommentLen) {
						/* Parse comment if I REALLY care */
						InputPtr += CommentLen;
//					}
				}
				break;
				
			default:
				/* Skip to next op code. */

				if (codes[Opcode] == -1) {
					InputPtr+=2;
				} else {
					InputPtr += codes[Opcode];
				}
//				NonFatal("Found unsupported opcode 0x%02X\n",Opcode);
//				goto OhShit;
				break;
			}
		}
		
		/* Is this the header? */

		if (Opcode == 0xc00) {
			InputPtr += 24;
			continue;
		}
		
		if (Opcode == 0x28)
		{
			InputPtr += 4;
			InputPtr += InputPtr[0] + 1;
			continue;
		}
		
		/* Known empty opcodes */
		
		if (((Opcode >= 0xB0) && (Opcode <= 0xCF)) ||
			((Opcode >= 0x8000) && (Opcode <= 0x80FF))) {
			continue;
		}
		
		if (Opcode == 0x8200) {
#if defined(__MAC__) || defined(__WIN32__) || defined(__MACOSX__)
			Word32				FileSize;
			const Word8 *			QTImageData;
			Word32				MatteSize, MaskSize;
			LBRect					PicSize;
			
			FileSize = Burger::LoadBig((Word32 *)InputPtr);
			InputPtr += 4;
			
			QTImageData = InputPtr;
			
			QTImageData += 2;	//	Version
			QTImageData += sizeof(MatrixRecord);	//	Matrix
			MatteSize = Burger::LoadBig((Word32 *)QTImageData);
			QTImageData += 4;	//	Matte Size
			QTImageData += sizeof(Rect);	//	Matte bounds
			QTImageData += 2;	//	Flags
			PicSize.top = Burger::LoadBig((Word16 *)QTImageData);	QTImageData += 2;
			PicSize.left = Burger::LoadBig((Word16 *)QTImageData);	QTImageData += 2;
			PicSize.bottom = Burger::LoadBig((Word16 *)QTImageData);QTImageData += 2;
			PicSize.right = Burger::LoadBig((Word16 *)QTImageData);	QTImageData += 2;
			QTImageData += 2;	//	Unused short
			QTImageData += 2;	//	Quality
			MaskSize = Burger::LoadBig((Word32 *)QTImageData);
			QTImageData += 4;	//	Mask Size
			
			QTImageData += MatteSize + MaskSize;
			
			if (Burger::LoadBig((Word32 *)QTImageData) == sizeof(ImageDescription))
			{
				ImageDescriptionHandle	desc;
				GWorldPtr				gworld;
				PixMapHandle			pmh;
				Word8 *					descPtr;
				OSErr					err;
				Rect					imageRect;
			
				desc = (ImageDescriptionHandle)NewHandle(sizeof(ImageDescription));

				//	Set this up...
				HLock((Handle)desc);
				descPtr = (Word8 *)*desc;
				*(ImageDescriptionPtr)descPtr = *(ImageDescriptionPtr)QTImageData;
				*(Word32 *)descPtr = Burger::LoadBig((Word32 *)descPtr);descPtr += 4;	//	idSize
				*(Word32 *)descPtr = Burger::LoadBig((Word32 *)descPtr);descPtr += 4;	//	cType
				*(Word32 *)descPtr = Burger::LoadBig((Word32 *)descPtr);descPtr += 4;	//	rsvd1
				*(Word16 *)descPtr = Burger::LoadBig((Word16 *)descPtr);		descPtr += 2;	//	rsvd2
				*(Word16 *)descPtr = Burger::LoadBig((Word16 *)descPtr);		descPtr += 2;	//	dataRefIndex
				*(Word16 *)descPtr = Burger::LoadBig((Word16 *)descPtr);		descPtr += 2;	//	version
				*(Word16 *)descPtr = Burger::LoadBig((Word16 *)descPtr);		descPtr += 2;	//	revisionLevel
				*(Word32 *)descPtr = Burger::LoadBig((Word32 *)descPtr);descPtr += 4;	//	vendor
				*(Word32 *)descPtr = Burger::LoadBig((Word32 *)descPtr);descPtr += 4;	//	temporalQuality
				*(Word32 *)descPtr = Burger::LoadBig((Word32 *)descPtr);descPtr += 4;	//	spatialQuality
				*(Word16 *)descPtr = Burger::LoadBig((Word16 *)descPtr);		descPtr += 2;	//	width
				*(Word16 *)descPtr = Burger::LoadBig((Word16 *)descPtr);		descPtr += 2;	//	height
				*(Word32 *)descPtr = Burger::LoadBig((Word32 *)descPtr);descPtr += 4;	//	hRes
				*(Word32 *)descPtr = Burger::LoadBig((Word32 *)descPtr);descPtr += 4;	//	vRes
				*(Word32 *)descPtr = Burger::LoadBig((Word32 *)descPtr);descPtr += 4;	//	dataSize
				*(Word16 *)descPtr = Burger::LoadBig((Word16 *)descPtr);		descPtr += 2;	//	frameCount
				descPtr += 32;	//	Name
				*(Word16 *)descPtr = Burger::LoadBig((Word16 *)descPtr);		descPtr += 2;	//	depth
				*(Word16 *)descPtr = Burger::LoadBig((Word16 *)descPtr);		descPtr += 2;	//	clutID
				
				QTImageData += (**desc).idSize;
				
				gworld = nil;
				err = NewImageGWorld(&gworld, desc, /*keepLocal*/ noNewDevice);
				
				if (err == noErr)
				{
					GWorldPtr	oldWorld;
					GDHandle	oldDevice;
					
					pmh = GetGWorldPixMap(gworld);
					LockPixels(pmh);
					
					MacSetRect(&imageRect, 0, 0, (**desc).width, (**desc).height);
					
					GetGWorld(&oldWorld, &oldDevice);
					SetGWorld(gworld, GetGWorldDevice(gworld));
					err = DecompressImage((Ptr)QTImageData,
										 desc,
										 pmh,
										 &imageRect,
										 &imageRect,
										 srcCopy,
										 nil);
					
					SetGWorld(oldWorld, oldDevice);
				}
							
				if (err == noErr)
				{
					Word32	NumBytes;
					
					ImagePtr->RowBytes = (**pmh).rowBytes & 0x3FFF;
					
					NumBytes = ImagePtr->RowBytes * ImagePtr->Height;
					
					ImagePtr->ImagePtr = static_cast<Word8 *>(AllocAPointer(NumBytes));
					FastMemCpy(ImagePtr->ImagePtr, GetPixBaseAddr(pmh), NumBytes);
					
					switch ((**pmh).pixelSize)
					{
						case 8:		ImagePtr->DataType = IMAGE8_PAL;	break;
						case 16:	ImagePtr->DataType = IMAGE555;		break;
						case 24:	ImagePtr->DataType = IMAGE888;		break;
						case 32:	ImagePtr->DataType = IMAGE8888;		break;
						default:	NonFatal("Unsuppored PICT bit depth %d.\n", (**pmh).pixelSize);
					}
					
					if ((**pmh).pixelSize == 16)
					{
						int height = ImagePtr->Height;
						int skip = ImagePtr->RowBytes - (ImagePtr->Width * 2);
						Word16 *	pixels = (Word16 *)ImagePtr->ImagePtr;
						
						do
						{
							int width = ImagePtr->Width;
							do
							{
								*pixels = Burger::LoadBig(pixels);
								++pixels;
							} while (--width);
							
							pixels = (Word16 *)(((Word8 *)pixels) + skip);
						} while (--height);
					}
					
					if (ImagePtr->DataType == IMAGE8_PAL) {
						if (!ImagePtr->PalettePtr) {
							ImagePtr->PalettePtr = (Word8 *)AllocAPointer(768);
							FastMemCpy(ImagePtr->PalettePtr,MacStandardPalette,768);
						}
					}
				}
				else
					NonFatal("Unable to decompress QT PICT.\n");
				
				if (gworld)
					DisposeGWorld(gworld);
				
				if (desc)
					DisposeHandle((Handle)desc);
				
				return 0;
			}
			
			InputPtr+=FileSize;
			
			continue;
#elif 1
			Word32 FileSize;
			JPegFound=TRUE;
			FileSize = Burger::LoadBig((Word32 *)InputPtr);
//			SaveAFile("Foo.qtif",InputPtr+4,FileSize);
//			InputPtr+=158;
			InputPtr+=FileSize+4;
//			NonFatal("Imbedded jpeg not supported yet.\n");
//			goto OhShit;
		
			continue;
#else
			FILE *file;
			Image *tile_image;
			ImageInfo *clone_info;

			/* Embedded JPEG. */

			JPegFound=TRUE;
			clone_info=CloneImageInfo(image_info);
			clone_info->blob=(void *) NULL;
			clone_info->length=0;
			TemporaryFilename(clone_info->filename);
			file=fopen(clone_info->filename,"wb");
			if (file == (FILE *) NULL)
			ThrowReaderException(FileOpenError,"UnableToWriteFile",image);
			
			length=ReadBlobMSBLong(image);
			for (i=0; i < 6; i++)
			(void) ReadBlobMSBLong(image);
			ReadRectangle(PicSize);
			for (i=0; i < 122; i++)
			(void) ReadBlobByte(image);
			for (i=0; i < (long) (length-154); i++) {
				c=ReadBlobByte(image);
				(void) fputc(c,file);
			}
			(void) fclose(file);
			tile_image=ReadImage(clone_info,exception);
			DestroyImageInfo(clone_info);
			(void) remove(clone_info->filename);
			if (tile_image == (Image *) NULL)
				continue;
			FormatString(geometry,"%lux%lu",Max(image->columns,tile_image->columns),Max(image->rows,tile_image->rows));
			(void) TransformImage(&image,(char *) NULL,geometry);
			(void) CompositeImage(image,CopyCompositeOp,tile_image,PicSize.left,PicSize.right);
			DestroyImage(tile_image);
			continue;
//			NonFatal("Imbedded jpeg not supported yet.\n");
//			goto OhShit;
#endif
		}
		if (Opcode == 0x8201)
		{
			Word32 FileSize;
			FileSize = Burger::LoadBig((Word32 *)InputPtr);
			InputPtr+=FileSize+4;
			continue;
		}
		
		if ((Opcode == 0xFF) || (Opcode == 0xFFFF)) {
			break;			/* Abort code? */
		}
		
		if (((Opcode >= 0xD0) && (Opcode <= 0xFE)) ||
			((Opcode >= 0x8100) && (Opcode <= 0xFFFF))) {
			Word Len12;
			
			/* Skip reserved. */
			Len12=Burger::LoadBig(((Word16 *)InputPtr));
			InputPtr+=Len12;
			continue;
		}

		if ((Opcode >= 0x100) && (Opcode <= 0x7fff)) {
			/* Skip reserved. */
			Word Len11;
			Len11=(Opcode >> 7) & 0xff;
			InputPtr+=Len11;
			continue;
		}
	}
#if 0
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
		if (BitsPerPixel!=4 && BitsPerPixel!=8) {
			NonFatal("Cicn's must be 4 or 8 bits per pixel!\n");
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
	
	DestPtr = AllocAPointer((Word32)ImagePtr->Width*(Word32)ImagePtr->Height);
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
					MyRemapTable[Index] = Index;
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
		} else {
			/* 4 bits per pixel */
			
			Word TempWidth;
			do {
				const Word8 *WorkPtr;
				Word i;
				Word Temp;

				TempWidth = ImagePtr->Width;
				WorkPtr = SrcPtr;
				i = 0;
				do {
					Temp = WorkPtr[0];
					if (!(i&1)) {
						Temp = (Temp>>4);
					} else {
						Temp = Temp&0xF;
						++WorkPtr;
					}
					DestPtr[0] = Temp;
					++DestPtr;
				} while (++i<TempWidth);
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
				MyRemapTable[0] = xx;			/* Force a remap to here */
				PalPtrxx = &PalPtr[xx*3];
				PalPtrxx[0] = PalPtr[0];		/* Copy the color */
				PalPtrxx[1] = PalPtr[1];
				PalPtrxx[2] = PalPtr[2];
				break;
			}
		} while (++xx<255);
		
		if (xx>=256) {			/* Ok, all colors are used */
			MyRemapTable[0] = PaletteFindColorIndex(PalPtr+3,PalPtr[0],PalPtr[1],PalPtr[2],254)+1;
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
		
		RowBytes = Burger::LoadBig(&MaskBitmapPtr->RowBytes);		/* Mask row bytes */
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
#endif
OhShit:;
	ImageDestroy(ImagePtr);		/* Free the memory */
	return TRUE;				/* Decode or memory error */
}
