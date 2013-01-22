/**********************************

	Burgerlib Palette Manager
	All rights reserved.
	Written by Bill Heineman

**********************************/

#ifndef __PLPALETTE_H__
#define __PLPALETTE_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct RezHeader_t;

/* Private */

extern Word BurgerPaletteDirty;			/* Set to TRUE for a deferred palette update */

/* Public */

typedef struct HSL_t {
	float Hue;			/* Color hue 0 to 1 */
	float Saturation;	/* Color saturation 0 to 1 */
	float Luminance;	/* Color luminance 0 to 1 */
} HSL_t;

typedef struct RGB_t {
	float Red;			/* Red intensity 0 to 1 */
	float Green;		/* Green intensity 0 to 1 */
	float Blue;			/* Blue intensity 0 to 1 */
} RGB_t;

typedef void (BURGERCALL *PaletteChangedProcPtr)(void);
typedef void (BURGERCALL *PaletteFadeCallBackProcPtr)(Word Pass);

extern Word8 CurrentPalette[256*3];		/* Current palette in the hardware (Read ONLY) */
extern const Word ByteSquareTable[255+256];	/* Table of -255 to 255 squared */
extern const Word8 RGB5ToRGB8Table[32];	/* Table to convert 5 bit color to 8 bit color */
extern const Word8 RGB6ToRGB8Table[64];	/* Table to convert 6 bit color to 8 bit color */
extern Word FadeSpeed;			/* Delay in Ticks for a palette change */
extern Word PaletteVSync;		/* Set to TRUE if the palette MUST be changed only during VSync */
extern PaletteFadeCallBackProcPtr PaletteFadeCallBack;
extern PaletteChangedProcPtr PaletteChangedProc;	/* Called whenever the palette was changed */

extern void BURGERCALL PaletteConvertRGB15ToRGB24(Word8 *RGBOut,Word RGBIn);
extern void BURGERCALL PaletteConvertRGB16ToRGB24(Word8 *RGBOut,Word RGBIn);
extern Word BURGERCALL PaletteConvertRGB24ToRGB15(const Word8 *RGBIn);
extern Word BURGERCALL PaletteConvertRGB24ToRGB16(const Word8 *RGBIn);
extern Word BURGERCALL PaletteConvertRGB24ToDepth(const Word8 *RGBIn,Word Depth);
extern Word BURGERCALL PaletteConvertRGBToDepth(Word Red,Word Green,Word Blue,Word Depth);
extern Word BURGERCALL PaletteConvertPackedRGBToDepth(Word32 Color,Word Depth);
extern void BURGERCALL PaletteMake16BitLookup(Word *Output,const Word8 *Input,Word Depth);
extern void BURGERCALL PaletteMake16BitLookupRez(Word *Output,struct RezHeader_t *Input,Word RezNum,Word Depth);
extern void BURGERCALL PaletteMakeRemapLookup(Word8 *Output,const Word8 *DestPal,const Word8 *SourcePal);
extern void BURGERCALL PaletteMakeRemapLookupMasked(Word8 *Output,const Word8 *DestPal,const Word8 *SourcePal);
extern void BURGERCALL PaletteMakeColorMasks(Word8 *Output,Word MaskColor);
extern void BURGERCALL PaletteMakeFadeLookup(Word8 *Output,const Word8 *Input,Word r,Word g,Word b);
extern Word BURGERCALL PaletteFindColorIndex(const Word8 *PalPtr,Word Red,Word Green,Word Blue,Word Count);
extern void BURGERCALL PaletteBlack(void);
extern void BURGERCALL PaletteWhite(void);
extern void BURGERCALL PaletteFadeToBlack(void);
extern void BURGERCALL PaletteFadeToWhite(void);
extern void BURGERCALL PaletteFadeTo(struct RezHeader_t *Input,Word ResID);
extern void BURGERCALL PaletteFadeToPtr(const Word8 *PalettePtr);
extern void BURGERCALL PaletteFadeToHandle(void **PaletteHandle);
extern void BURGERCALL PaletteSet(struct RezHeader_t *Input,Word PalNum);
extern void BURGERCALL PaletteSetHandle(Word Start,Word Count,void **PaletteHandle);
extern void BURGERCALL PaletteSetPtr(Word Start,Word Count,const Word8 *PalettePtr);
extern Word BURGERCALL PaletteGetBorderColor(void);
extern void BURGERCALL PaletteSetBorderColor(Word Color);
extern void BURGERCALL PaletteRGB2HSL(HSL_t *Output,const RGB_t *Input);
extern void BURGERCALL PaletteHSL2RGB(RGB_t *Output,const HSL_t *Input);
extern void BURGERCALL PaletteHSLTween(HSL_t *Output,const HSL_t *HSLPtr1,const HSL_t *HSLPtr2,float Factor,Word Dir);
extern void BURGERCALL PaletteRGBTween(RGB_t *Output,const RGB_t *RGBPtr1,const RGB_t *RGBPtr2,float Factor,Word Dir);

#ifdef __cplusplus
}
#endif

#endif

