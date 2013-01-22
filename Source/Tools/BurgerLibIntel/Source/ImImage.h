/**********************************

	Image manager
	For handling image formats in software

**********************************/

#ifndef __IMIMAGE_H__
#define __IMIMAGE_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __FMFILE_H__
#include "FmFile.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Image routines */

typedef enum {IMAGE332=4,IMAGE8ALPHA=5,IMAGE8_PAL_ALPHA_PAL=6,IMAGE8_PAL_ALPHA=7,IMAGE8_PAL=8,
	IMAGE4444=13,IMAGE1555=14,IMAGE555=15,IMAGE565=16,IMAGE888=24,IMAGE8888=32} ImageTypes_e;

typedef struct Image_t {	/* Used by image routines */
	Word8 *ImagePtr;		/* Pointer to pixel array (AllocAPointer()) */
	Word8 *PalettePtr;	/* Pointer to RGB tripletts, only for 8 BPP images */
	Word8 *AlphaPtr;		/* Alpha channel if any */
	Word Width;			/* Width of image in pixels */
	Word Height;		/* Height of image in pixels */
	Word RowBytes;		/* Number of bytes per scan line */
	ImageTypes_e DataType;		/* 8,16,24 Bits per pixel */
} Image_t;

typedef enum {PSD_FLAG_RGB,PSD_FLAG_ALPHA,PSD_FLAG_MASK,PSD_FLAG_RGBA} PSDFlag_e;

typedef struct PSDImageLayer_t {
	char* Name;						/* Name of the image (If any) */
	int Top, Left, Bottom, Right;	/* Image bounds rect */
	int MaskTop, MaskLeft, MaskBottom, MaskRight;	/* Mask rect */
	int Width, Height;			/* Size of the image */
	int MaskWidth, MaskHeight;	/* Size of the mask */
	Word8* RGB;					/* RGB data (24 bit) */
	Word8* Alpha;				/* Alpha data */
	Word8* Mask;					/* Mask data */
} PSDImageLayer_t;

typedef struct PSDImage_t {
	Word Width;					/* Width in pixels */
	Word Height;				/* Height in pixels */
	Word NumLayers;				/* Number of layers present */
	PSDImageLayer_t* Layers;	/* Array of layers */
} PSDImage_t;

extern const Word RGB2ToRGB3Table[4];
extern const Word RGB2ToRGB4Table[4];
extern const Word RGB2ToRGB5Table[4];
extern const Word RGB2ToRGB6Table[4];
extern const Word RGB2ToRGB8Table[4];
extern const Word RGB3ToRGB4Table[8];
extern const Word RGB3ToRGB5Table[8];
extern const Word RGB3ToRGB6Table[8];
extern const Word RGB3ToRGB8Table[8];
extern const Word RGB4ToRGB5Table[16];
extern const Word RGB4ToRGB6Table[16];
extern const Word RGB4ToRGB8Table[16];
extern const Word RGB5ToRGB6Table[32];

extern Word BURGERCALL ImageInit(Image_t *Output,Word Width,Word Height,ImageTypes_e Depth);
extern Image_t * BURGERCALL ImageNew(Word Width,Word Height,ImageTypes_e Depth);
extern Word BURGERCALL ImageInitCopy(Image_t *Output,const Image_t *Input);
extern Image_t * BURGERCALL ImageNewCopy(const Image_t *Input);
extern void BURGERCALL ImageDelete(Image_t *ImagePtr);
extern void BURGERCALL ImageDestroy(Image_t *ImagePtr);
extern Word BURGERCALL ImageParseAny(Image_t *Output,const Word8 *InputPtr,Word32 InputLength);
extern Word BURGERCALL ImageParseBMP(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParseCicn(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParseGIF(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParseIIGS(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParseJPG(Image_t *Output,const Word8 *InputPtr,Word32 InputLength);
extern Word BURGERCALL ImageParseLBM(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParsePCX(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParsePPat(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParsePict(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParsePSD(Image_t *Output,const Word8 *InputPtr,Word Layer);
extern Word BURGERCALL ImageParseTGA(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParseTIFF(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageExtractFromPSDImage(Image_t *Output,PSDImageLayer_t* Layer,PSDFlag_e flags);
extern Word BURGERCALL Image2BMPFile(Image_t *ImagePtr,const char *FileName,Word NoCompress);
extern Word BURGERCALL Image2GIFFile(Image_t *ImagePtr,const char *FileName);
extern Word BURGERCALL Image2IIGSFile(Image_t *ImagePtr,const char *FileName);
extern Word BURGERCALL Image2JPGFile(Image_t *ImagePtr,const char *FileName,Word CompressionFactor);
extern Word BURGERCALL Image2LBMFile(Image_t *ImagePtr,const char *FileName);
extern Word BURGERCALL Image2PBMFile(Image_t *ImagePtr,const char *FileName);
extern Word BURGERCALL Image2PCXFile(Image_t *ImagePtr,const char *FileName);
extern Word BURGERCALL Image2PSDFile(Image_t *ImagePtr,const char *FileName);
extern Word BURGERCALL Image2TGAFile(Image_t *ImagePtr,const char *FileName,Word NoCompress);
extern Word BURGERCALL Image2TIFFFile(Image_t *ImagePtr,const char *FileName);
extern Word BURGERCALL ImageStore(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore4444(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore555(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore565(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore1555(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore888(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore8888(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore332(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore8Pal(Image_t *Output,const Image_t *Input);
extern void BURGERCALL ImageColorKey8888(Image_t* SrcImagePtr,Word r,Word g,Word b,Word a);
extern Word BURGERCALL ImageSubImage(Image_t* Output,Word x,Word y,const Image_t* Input);
extern void BURGERCALL ImageVerticalFlip(Image_t *ImagePtr);
extern void BURGERCALL ImageHorizontalFlip(Image_t *ImagePtr);
extern void BURGERCALL ImageRemove0And255(Image_t *ImagePtr);
extern void BURGERCALL ImageRepaletteIndexed(Image_t *ImagePtr,const Word8 *PalettePtr);
extern void BURGERCALL ImageRemapIndexed(Image_t *ImagePtr,const Word8 *RemapPtr);
extern void BURGERCALL ImageSwapBGRToRGB(Image_t *ImagePtr);
extern Word BURGERCALL ImageValidateToSave(Image_t *ImagePtr);
extern void BURGERCALL ImageEncodeLBM(FILE *fp,const Word8 *SrcPtr,Word Length);
extern void BURGERCALL ImageEncodePCX(FILE *fp,const Word8 *SrcPtr,Word Length);
extern Word BURGERCALL PSDImageParse(PSDImage_t* Image, const Word8* InStream);
extern void BURGERCALL PSDImageDestroy(PSDImage_t* Input);

#ifdef __cplusplus
}
#endif

#endif




























