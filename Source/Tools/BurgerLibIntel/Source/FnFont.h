/*******************************

	Font Manager

*******************************/

#ifndef __FNFONT_H__
#define __FNFONT_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct FontRef_t;
struct LinkedList_t;
struct LBRect;

typedef void (BURGERCALL *FontDrawProc)(struct FontRef_t *,const char *,Word);
typedef Word (BURGERCALL *FontWidthProc)(struct FontRef_t *,const char *,Word);

typedef struct FontRef_t {		/* This is the generic class for fonts */
	int FontX;			/* X coord to draw the font */
	int FontY;			/* Y coord to draw the font */
	Word FontHeight;	/* Height of the font in pixels */
	Word FontFirst;		/* First allowable font to draw */
	Word FontLast;		/* Last char I can draw */
	FontDrawProc Draw;	/* Draw text */
	FontWidthProc GetWidth;	/* Get the width of the text */
} FontRef_t;

typedef struct BurgerFontState_t {
	Word8 FontOrMask[32];	/* Or mask (Could be shorts) */
	struct RezHeader_t *FontRezFile;	/* Resource file */
	int FontX;			/* X coord */
	int FontY;			/* Y coord */
	Word FontLoaded;	/* Resource */
	Word FontColorZero;	/* And mask */
} BurgerFontState_t;

typedef struct BurgerFont_t {	/* State of a font */
	FontRef_t Root;		/* Root of the font */
	Word8 FontOrMask[32];	/* Color of font (Could be shorts) */
	struct RezHeader_t *FontRezFile;	/* Resource file */
	void **FontHandle;	/* Handle to the active font */
	Word FontOffset;	/* Offset to the pixel array */
	Word FontLoaded;	/* Resource ID of the last font loaded */
	Word FontColorZero;	/* Mask for font */
} BurgerFont_t;

typedef struct FontWidthEntry_t {	/* Describe each line */
	Word32 Offset;				/* Offset to the text string */
	Word Length;					/* Length of the string entry */
} FontWidthEntry_t;

typedef struct FontWidthLists_t {	/* Hash for formatted text */
	FontRef_t *FontPtr;				/* Font to use */
	Word Count;						/* Number of valid entries */
	FontWidthEntry_t Entries[1];	/* Array of entries */
} FontWidthLists_t;

extern void BURGERCALL BurgerFontInit(BurgerFont_t *Input,struct RezHeader_t *RezFile,Word RezNum,const Word8 *PalPtr);
extern BurgerFont_t * BURGERCALL BurgerFontNew(struct RezHeader_t *RezFile,Word RezNum,const Word8 *PalPtr);
#define BurgerFontDestroy(x) BurgerFontRelease(x)
extern void BURGERCALL BurgerFontDelete(BurgerFont_t *Input);
extern void BURGERCALL BurgerFontSaveState(BurgerFont_t *RefPtr,BurgerFontState_t *StatePtr);
extern void BURGERCALL BurgerFontRestoreState(BurgerFont_t *RefPtr,const BurgerFontState_t *StatePtr);
extern void BURGERCALL BurgerFontDrawChar(BurgerFont_t *RefPtr,Word Letter);
extern void BURGERCALL BurgerFontDrawText(BurgerFont_t *RefPtr,const char *TextPtr,Word TextLen);
extern Word BURGERCALL BurgerFontWidthText(BurgerFont_t *RefPtr,const char *TextPtr,Word TextLen);
extern void BURGERCALL BurgerFontSetColor(BurgerFont_t *RefPtr,Word ColorNum,Word Color);
extern void BURGERCALL BurgerFontUseZero(BurgerFont_t *RefPtr);
extern void BURGERCALL BurgerFontUseMask(BurgerFont_t *RefPtr);
extern void BURGERCALL BurgerFontInstallToPalette(BurgerFont_t *RefPtr,struct RezHeader_t *RezFile,Word FontNum,const Word8 *PalPtr);
extern void BURGERCALL BurgerFontRelease(BurgerFont_t *RefPtr);
extern void BURGERCALL BurgerFontSetColorRGBListToPalette(BurgerFont_t *RefPtr,const void *RGBList,const Word8 *PalPtr);
extern void BURGERCALL BurgerFontSetToPalette(BurgerFont_t *RefPtr,const Word8 *PalPtr);

extern void BURGERCALL FontSetXY(FontRef_t *RefPtr,int x,int y);
extern Word BURGERCALL FontWidthChar(FontRef_t *RefPtr,Word Letter);
extern Word BURGERCALL FontWidthLong(FontRef_t *Input,long Val);
extern Word BURGERCALL FontWidthLongWord(FontRef_t *RefPtr,Word32 Val);
extern Word BURGERCALL FontWidthString(FontRef_t *RefPtr,const char *TextPtr);
extern Word BURGERCALL FontWidthListWidest(FontRef_t *FontPtr,struct LinkedList_t *ListPtr);
extern void BURGERCALL FontDrawChar(FontRef_t *RefPtr,Word Letter);
extern void BURGERCALL FontDrawLong(FontRef_t *Input,long Val);
extern void BURGERCALL FontDrawLongWord(FontRef_t *RefPtr,Word32 Val);
extern void BURGERCALL FontDrawString(FontRef_t *RefPtr,const char *TextPtr);
extern void BURGERCALL FontDrawStringCenterX(FontRef_t *RefPtr,int x,int y,const char *TextPtr);
extern void BURGERCALL FontDrawStringAtXY(FontRef_t *RefPtr,int x,int y,const char *TextPtr);
extern Word BURGERCALL FontCharsForPixelWidth(FontRef_t *RefPtr,const char *TextPtr,Word Width);

extern FontWidthLists_t * BURGERCALL FontWidthListNew(FontRef_t *FontRef,const char *TextPtr,Word PixelWidth);
#define FontWidthListDelete(x) DeallocAPointer(x)
extern void BURGERCALL FontWidthListDraw(FontWidthLists_t *Input,const struct LBRect *BoundsPtr,Word YTop,const char *TextPtr);

#ifdef __cplusplus
}
#endif

#endif

