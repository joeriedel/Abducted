#include "FnFont.h"
#include <BREndian.hpp>
#include "MmMemory.h"
#include "ClStdLib.h"
#include "GrGraphics.h"
#include "PfPrefs.h"
#include "RzRez.h"
#include "PlPalette.h"
#include "LrRect.h"
#include "LkLinkList.h"

typedef struct BurgerlibFont_t {
	Word16 FHeight;		/* Height of the font */
	Word16 FLast;		/* Number of font cells */
	Word16 FFirst;		/* First ASCII char */
	Word8 FontWidths[1];	/* Width table */
} BurgerlibFont_t;

/**********************************

	Initialize a font structure

**********************************/

void BURGERCALL BurgerFontInit(BurgerFont_t *Input,struct RezHeader_t *RezFile,Word RezNum,const Word8 *PalPtr)
{
	FastMemSet(Input,0,sizeof(BurgerFont_t));
	Input->Root.Draw = (FontDrawProc)BurgerFontDrawText;
	Input->Root.GetWidth = (FontWidthProc)BurgerFontWidthText;
	BurgerFontInstallToPalette(Input,RezFile,RezNum,PalPtr);	/* Install the font */
	BurgerFontUseMask(Input);		/* Set up for masked shapes */
}

/**********************************

	Allocate a new font reference struct

**********************************/

BurgerFont_t * BURGERCALL BurgerFontNew(struct RezHeader_t *RezFile,Word RezNum,const Word8 *PalPtr)
{
	BurgerFont_t *MyRef;

	MyRef = (BurgerFont_t *)AllocAPointer(sizeof(BurgerFont_t));
	if (MyRef) {
		BurgerFontInit(MyRef,RezFile,RezNum,PalPtr);
	}
	return MyRef;
}

/**********************************

	Delete a FontRef_t struct

**********************************/

void BURGERCALL BurgerFontDelete(BurgerFont_t *Input)
{
	if (Input) {		/* Failsafe */
		BurgerFontRelease(Input);		/* If a font is allocated, discard it */
		DeallocAPointer(Input);		/* Release the structure pointer */
	}
}

/********************************

	Save the current state of the font manager

********************************/

void BURGERCALL BurgerFontSaveState(BurgerFont_t *Input,BurgerFontState_t *StatePtr)
{
	StatePtr->FontX = Input->Root.FontX;		/* Save the current pen position */
	StatePtr->FontY = Input->Root.FontY;
	StatePtr->FontLoaded = Input->FontLoaded;	/* Save the font number */
	StatePtr->FontRezFile = Input->FontRezFile;	/* Save the resource file */
	StatePtr->FontColorZero = Input->FontColorZero;
	FastMemCpy(&StatePtr->FontOrMask[0],&Input->FontOrMask[0],32);
}

/********************************

	Restore the previous state of the font manager

********************************/

void BURGERCALL BurgerFontRestoreState(BurgerFont_t *Input,const BurgerFontState_t *StatePtr)
{
	if (StatePtr->FontX!=0x7FFF) {
		Input->Root.FontX = StatePtr->FontX;		/* Set the X and Y coords */
		Input->Root.FontY = StatePtr->FontY;
	}
	if (StatePtr->FontLoaded) {		/* Valid font number? */
		BurgerFontInstallToPalette(Input,StatePtr->FontRezFile,StatePtr->FontLoaded,0);	/* Set the new font */
	}
	Input->FontColorZero=StatePtr->FontColorZero;
	FastMemCpy(&Input->FontOrMask[0],&StatePtr->FontOrMask[0],32);
}

/********************************

	Draw a char on the screen
	It can draw to an 8 or 16 bit screen

	You may ask yourself, why in hell do I use goto's like I do,
	I use goto's for the road least traveled by so the CPU pipeline is
	not broken in most cases. This way the code runs faster in the most
	likely code cases.

********************************/

void BURGERCALL BurgerFontDrawChar(BurgerFont_t *Input,Word Char)
{
	int TempX,TempY;		/* Clipped font x and y */
	Word Width,Height;		/* Clipped width and height of font char */
	Word SkipFirst;			/* True if I should skip the first font pixel */
	Word Mod;				/* Video screen adjust against clipped font width */
	Word ByteWidth;			/* Width of a font cell in bytes */
	Word Mask;
	register Word8 *ScreenPtr;	/* Pointer to screen */
	register Word8 *SrcPtr;		/* Pointer to font data */

	Char-=Input->Root.FontFirst;			/* Adjust to first VALID char */
	if (Char>=Input->Root.FontLast) {	/* Can't draw this character? */
		goto ByeBye;				/* Exit now */
	}
	SrcPtr = ((Word8 **)(Input->FontHandle))[0];		/* Get the font pointer */
	Width = SrcPtr[Char+6];		/* Get the width of the char */

	/* Bounds check the X coord */

	TempX = Input->Root.FontX;			/* Get the X coord */
	Mod = TempX+Width;
	Input->Root.FontX = Mod;				/* Save the new dest X */
	if (TempX>=(int)ScreenClipRight || (int)Mod<=(int)ScreenClipLeft) { 	/* Off the right or left side? */
		goto ByeBye;
	}

	/* Bounds check the Y coord */

	TempY = Input->Root.FontY;			/* Get the Y coord */
	Height = Input->Root.FontHeight;		/* Get the height of the font char */
	SkipFirst = Height+TempY;				/* Get the bottommost Y coord */

	if (TempY>=(int)ScreenClipBottom || (int)SkipFirst<=(int)ScreenClipTop) {	/* Off the top or bottom? */
		goto ByeBye;
	}

	/* Get the pointer to the font data */

	SrcPtr = SrcPtr+Input->FontOffset;
	SrcPtr = ((Word8 *)SrcPtr)+Burger::LoadLittle(&((Word16 *)SrcPtr)[Char]);

	/* Clip the font data */
	/* First clip the Y coord */

	ByteWidth = (Width+1)>>1;	/* Convert to bytes per cel */

	if (TempY<(int)ScreenClipTop) {		/* Clip the top? */
		goto ClipTop;
	}
BackClipTop:

	if (SkipFirst>ScreenClipBottom) {	/* Clip the bottom? */
		goto ClipBottom;
	}
BackClipBottom:

	/* Now, let's clip the X coord */

	SkipFirst = FALSE;		/* I assume I don't need to begin INSIDE the font */
	if (TempX<(int)ScreenClipLeft) {			/* Off the left side? */
		goto ClipLeft;
	}
BackClipLeft:

	if (Mod>ScreenClipRight) {
		goto ClipRight;
	}
BackClipRight:

	/* Let's finally draw the font */
	/* Height = clipped height of font */
	/* Width = clipped pixel width of font */
	/* ByteWidth = Bytes to skip per line of font data */
	/* SrcPtr = Pointer to font */
	/* ScreenPtr = Pointer to screen */
	/* SkipFirst = True if I start drawing on odd pixel in font data */
	/* Mod = Number of pad bytes after each scan line */

	ScreenPtr = (VideoWidth*TempY)+VideoPointer;	/* Make the screen coord */

	if (VideoColorDepth>=9) {		/* Not 8 bit? */
		goto Draw16Bit;				/* Draw using a 16 bit blitter */
	}

	/* This is the 8 bit blitter */

	ScreenPtr += TempX;		/* Make the screen coord */
	Mod = VideoWidth-Width;		/* Create the dest font skip */

	if (SkipFirst) {	/* Will I draw a leading char? */
		--Width;		/* Remove 1 from width pair loop */
		--ByteWidth;	/* There is a ++ to the source pointer for this */
	}
	if (Width&1) {		/* Will I have an ending char? */
		SkipFirst|=2;	/* Set a flag */
		++Mod;			/* +1 to the dest screen address */
	}
	Width>>=1;			/* Convert to pairs */
	ByteWidth-=Width;	/* Remove adder */
	Mask = Input->FontColorZero;		/* Get the mask color */
	do {
		Word XWidth;

		if (SkipFirst&1) {	/* Skip first character? */
			goto SkipF8;		/* Enter in the middle*/
		}
Back8:
		if (Width) {		/* Any center part? */
			XWidth = Width;		/* Save font width in temp */
			do {
				TempY = SrcPtr[0];	/* Get font data */
				TempX = ((Word)TempY)>>4;	/* Split it */
				TempY = TempY&0x0F;
				if (Mask!=(Word)TempX) {	/* Valid? */
					ScreenPtr[0] = Input->FontOrMask[TempX];	/* Store to screen */
				}
				if (Mask!=(Word)TempY) {		/* Ok? */
					ScreenPtr[1] = Input->FontOrMask[TempY];	/* Store the color */
				}
				ScreenPtr+=2;		/* Add the data */
				++SrcPtr;			/* Next source pixel */
			} while (--XWidth);
		}
		if (SkipFirst&2) {	/* Is there a trailing pixel? */
			TempX = SrcPtr[0]>>4;	/* Get font data */
			if ((Word)TempX!=Mask) {	/* Valid? */
				ScreenPtr[0] = Input->FontOrMask[TempX];	/* Store to screen */
			}
		}
		ScreenPtr+=Mod;		/* Adjust the screen pointer */
		SrcPtr += ByteWidth;	/* Adjust the font pointer */
	} while (--Height);
	return;

SkipF8:
	TempX = SrcPtr[0]&0x0F;	/* Get font data */
	if ((Word)TempX!=Mask) {	/* Valid? */
		ScreenPtr[0] = Input->FontOrMask[TempX];	/* Store to screen */
	}
	++ScreenPtr;		/* Next screen byte */
	++SrcPtr;			/* Next source pixel */
	goto Back8;

	/* This routine will draw the font using 16 bit routines */

Draw16Bit:;
	ScreenPtr += TempX<<1;		/* Make the screen coord */
	Mod = VideoWidth-(Width<<1);		/* Create the dest font skip */

	if (SkipFirst) {	/* Will I draw a leading char? */
		--Width;		/* Remove 1 from width pair loop */
		--ByteWidth;	/* There is a ++ to the source pointer for this */
	}
	if (Width&1) {		/* Will I have an ending char? */
		SkipFirst|=2;	/* Set a flag */
		Mod+=2;			/* +1 to the dest screen address */
	}
	Width>>=1;			/* Convert to pairs */
	ByteWidth-=Width;	/* Remove adder */
	Mask = Input->FontColorZero;		/* Get the mask color */
	do {
		Word XWidth;

		if (SkipFirst&1) {	/* Skip first character? */
			goto SkipF16;		/* Enter in the middle*/
		}
Back16:
		if (Width) {		/* Any center part? */
			XWidth = Width;		/* Save font width in temp */
			do {
				TempY = SrcPtr[0];	/* Get font data */
				TempX = ((Word)TempY)>>4;	/* Split it */
				TempY = TempY&0x0F;
				if (Mask!=(Word)TempX) {	/* Valid? */
					((Word16 *)ScreenPtr)[0] = ((Word16 *)Input->FontOrMask)[TempX];	/* Store to screen */
				}
				if (Mask!=(Word)TempY) {		/* Ok? */
					((Word16 *)ScreenPtr)[1] = ((Word16 *)Input->FontOrMask)[TempY];	/* Store the color */
				}
				ScreenPtr+=4;		/* Add the data */
				++SrcPtr;			/* Next source pixel */
			} while (--XWidth);
		}
		if (SkipFirst&2) {	/* Is there a trailing pixel? */
			TempX = SrcPtr[0]>>4;	/* Get font data */
			if ((Word)TempX!=Mask) {	/* Valid? */
				((Word16 *)ScreenPtr)[0] = ((Word16 *)Input->FontOrMask)[TempX];	/* Store to screen */
			}
		}
		ScreenPtr+=Mod;		/* Adjust the screen pointer */
		SrcPtr += ByteWidth;	/* Adjust the font pointer */
	} while (--Height);
	return;

SkipF16:
	TempX = SrcPtr[0]&0x0F;	/* Get font data */
	if ((Word)TempX!=Mask) {	/* Valid? */
		((Word16 *)ScreenPtr)[0] = ((Word16 *)Input->FontOrMask)[TempX];	/* Store to screen */
	}
	ScreenPtr+=2;		/* Next screen byte */
	++SrcPtr;			/* Next source pixel */
	goto Back16;


ByeBye:;			/* Exit */
	return;

	/* Clip the top of the font */
ClipTop:
	Height = SkipFirst-ScreenClipTop;	/* Crop the height (Bottom Y = height) */
	SrcPtr = SrcPtr-((TempY-ScreenClipTop)*(int)ByteWidth);	/* (I'm subtracting a negative number) */
	/* Remove the upper pixels */
	TempY = ScreenClipTop;		/* Zap the top Y coord */
	goto BackClipTop;

		/* Clip the bottom of the font */
ClipBottom:
	Height = ScreenClipBottom-TempY;		/* Crop the height (TempY is already >=0) */
	goto BackClipBottom;

ClipLeft:
	Width = Mod-ScreenClipLeft;		/* Rightmost X is the width */
	TempX -= ScreenClipLeft;
	SkipFirst = TempX&1;	/* Even / Odd */
	SrcPtr = SrcPtr - (TempX>>1)-SkipFirst;	/* Subtracting a negative number */
	TempX = ScreenClipLeft;		/* Reset the dest X */
	goto BackClipLeft;

ClipRight:
	Width = ScreenClipRight-TempX;	/* Clip to the right */
	goto BackClipRight;
}

/********************************

	Draw a text array

********************************/

void BURGERCALL BurgerFontDrawText(BurgerFont_t *Input,const char *TextPtr,Word TextLen)
{
	if (TextLen) {
		do {
			BurgerFontDrawChar(Input,((Word8 *)TextPtr)[0]);		/* Draw the character */
			++TextPtr;					/* Next */
		} while (--TextLen);
	}
}

/********************************

	Return the width of a text array in pixels
	using the installed font

********************************/

Word BURGERCALL BurgerFontWidthText(BurgerFont_t *Input,const char *TextPtr,Word TextLen)
{
	Word Result;
	Word Last;
	Word First;
	Word Val;
	Word8 *WidthTbl;

	Result = 0;		/* Assume no width */
	if (TextLen) {
		WidthTbl = (Word8 *)Input->FontHandle;	/* Init tables */
		if (WidthTbl) {		/* Data? And valid font? */
			WidthTbl = (*(Word8 **)WidthTbl)+6;	/* Index to the table */
			Last = Input->Root.FontLast;
			First = Input->Root.FontFirst;
			do {
				Val = ((Word8 *)TextPtr)[0]-First;	/* Get the char */
				if (Val<Last) {		/* Valid? */
					Result+=WidthTbl[Val];		/* Add to the width */
				}
				++TextPtr;
			} while (--TextLen);
		}
	}
	return Result;
}

/********************************

	Set a color for a font

********************************/

void BURGERCALL BurgerFontSetColor(BurgerFont_t *Input,Word ColorNum,Word Color)
{
	if (VideoColorDepth<9) {		/* 8 bit color mode? */
		Input->FontOrMask[ColorNum] = static_cast<Word8>(Color);	/* Set the color */
		return;
	}
	reinterpret_cast<Word16 *>(Input->FontOrMask)[ColorNum] = static_cast<Word16>(Color);	/* Set as 16 bit */
}

/********************************

	Make color #0 visible

********************************/

void BURGERCALL BurgerFontUseZero(BurgerFont_t *Input)
{
	Input->FontColorZero = (Word)-1;	/* Set to an invalid number */
}

/********************************

	Make color #0 transparent

********************************/

void BURGERCALL BurgerFontUseMask(BurgerFont_t *Input)
{
	Input->FontColorZero = 0;
}

/********************************

	Install a font

********************************/

void BURGERCALL BurgerFontInstallToPalette(BurgerFont_t *Input,RezHeader_t *RezFile,Word FontNum,const Word8 *PalPtr)
{
	BurgerlibFont_t *TempPtr;		/* Pointer to font image */
	void **TempHandle;			/* Handle to font image */
	Word8 *FontPointer;			/* Pointer to font strike */

	if (Input->FontLoaded!=FontNum) {		/* Already in memory? */
		BurgerFontRelease(Input);		/* Release the previous font */
		if (FontNum) {				/* Valid font #? */
			TempHandle = ResourceLoadHandle(RezFile,FontNum);	/* Load it in */
			if (TempHandle) {		/* Did it load? */
				Input->FontHandle = TempHandle;		/* Save the handle */
				TempPtr = (BurgerlibFont_t *)(*TempHandle);
				Input->Root.FontHeight = Burger::LoadLittle(&TempPtr->FHeight);
				Input->Root.FontLast = Burger::LoadLittle(&TempPtr->FLast);
				Input->Root.FontFirst = Burger::LoadLittle(&TempPtr->FFirst);
				Input->FontOffset = Input->Root.FontLast+6;
				Input->FontLoaded = FontNum;		/* Set the new font */
				Input->FontRezFile = RezFile;
				if (PalPtr) {		/* Normal mode? */
					Word Flag;
					FontPointer = ((Word8 *)TempPtr)+Input->FontOffset;
					Flag = Burger::LoadLittle(&((Word16 *)FontPointer)[Input->Root.FontLast]);
					BurgerFontSetColorRGBListToPalette(Input,((Word8 *)FontPointer+Flag),PalPtr);
				}
			}
		}
	}
}

/**********************************

	Release a font back to the resource manager

**********************************/

void BURGERCALL BurgerFontRelease(BurgerFont_t *Input)
{
	Word RezNum;

	RezNum = Input->FontLoaded;
	if (RezNum) {		/* Was a font loaded? */
		ResourceRelease(Input->FontRezFile,RezNum);	/* Release the resource */
		Input->FontLoaded = 0;		/* Kill the variables */
		Input->FontRezFile = 0;
		Input->FontHandle = 0;
		Input->Root.FontLast = 0;
	}
}

/**********************************

	Using a RGB color list, set all the colors to a mounted font.
	I assume that CurrentPalette is the palette I will mount to.

**********************************/

typedef struct {		/* RGB triplett struct */
	Word8 Red;			/* Red value */
	Word8 Green;			/* Green value */
	Word8 Blue;			/* Blue value */
} Triplett;

typedef struct {
	Word8 Count;			/* Number of entries */
	Triplett Data[1];	/* Actual data to set */
} RGBColorList;

void BURGERCALL BurgerFontSetColorRGBListToPalette(BurgerFont_t *Input,const void *RGBList,const Word8 *PalPtr)
{
	Word Count;
	Count = ((RGBColorList *)RGBList)->Count;	/* Get the number of colors */
	if (Count) {		/* No colors?!?!? */
		Word i;
		if (Count>=17) {
			Count = 16;		/* Failsafe, never use more than 16 */
		}
		RGBList = ((Word8 *)RGBList)+1;	/* Point to the tripletts */
		i = 0;		/* Init color index */
		if (VideoColorDepth<9) {
			PalPtr=PalPtr+3;		/* Never remap to use color 0 */
			do {
				BurgerFontSetColor(Input,i,PaletteFindColorIndex(PalPtr,
					((Triplett *)RGBList)->Red,
					((Triplett *)RGBList)->Green,
					((Triplett *)RGBList)->Blue,254)+1);
				RGBList = ((Word8 *)RGBList)+3;		/* Some compilers pad!! */
			} while (++i<Count);	/* Next index */
			return;
		}
		do {
			BurgerFontSetColor(Input,i,PaletteConvertRGB24ToDepth((Word8 *)RGBList,VideoColorDepth));
			RGBList = ((Word8 *)RGBList)+3;		/* Some compilers pad!! */
		} while (++i<Count);
	}
}

/**********************************

	Set the font to use a different palette

**********************************/

void BURGERCALL BurgerFontSetToPalette(BurgerFont_t *RefPtr,const Word8 *PalPtr)
{
	Word8 *TempPtr;
	if (RefPtr->FontHandle && PalPtr) {	/* Is there a handle? */
		TempPtr = (Word8 *)(RefPtr->FontHandle[0]) + RefPtr->FontOffset;
		TempPtr = TempPtr+Burger::LoadLittle(&((Word16 *)TempPtr)[RefPtr->Root.FontLast]);
		BurgerFontSetColorRGBListToPalette(RefPtr,TempPtr,PalPtr);
	}
}



/********************************

	Generic font drawing routines used by any font system

********************************/

/********************************

	Set the pen X,Y for the font manager

********************************/

void BURGERCALL FontSetXY(FontRef_t *Input,int x,int y)
{
	Input->FontX = x;	/* Just set the X and Y coords */
	Input->FontY = y;
}

/********************************

	Draw a longword in the current font

********************************/

Word BURGERCALL FontWidthChar(FontRef_t *Input,Word Letter)
{
	char Ascii;

	Ascii = (char)Letter;
	return Input->GetWidth(Input,&Ascii,1);
}

/********************************

	Draw a longword in the current font

********************************/

Word BURGERCALL FontWidthLong(FontRef_t *Input,long Val)
{
	char Ascii[16];

	longToAscii(Val,Ascii);
	return Input->GetWidth(Input,Ascii,strlen(Ascii));
}

/********************************

	Draw a longword in the current font

********************************/

Word BURGERCALL FontWidthLongWord(FontRef_t *Input,Word32 Val)
{
	char Ascii[16];

	LongWordToAscii(Val,Ascii);
	return Input->GetWidth(Input,Ascii,strlen(Ascii));
}

/********************************

	Get a string's width in pixels
	
********************************/

Word BURGERCALL FontWidthString(FontRef_t *Input,const char *TextPtr)
{
	if (TextPtr) {
		return Input->GetWidth(Input,TextPtr,strlen(TextPtr));		/* Get the text width */
	}
	return 0;
}

/**********************************

	Scan a linked list of strings and get the width
	of the string as it would be in the chosen font.
	Return the width of the largest string.

**********************************/

Word BURGERCALL FontWidthListWidest(FontRef_t *FontPtr,LinkedList_t *ListPtr)
{
	Word Widest;				/* Return value */
	Word Width;					/* Temp width */
	
	Widest = 0;					/* Assume no width */
	if (ListPtr && FontPtr) {	/* Valid font and list? */
		LinkedListEntry_t *EntryPtr;
		EntryPtr = LinkedListGetFirst(ListPtr);		/* Get the first string */
		if (EntryPtr) {								/* Valid list */
			do {
				const char *TextPtr;				/* String pointer */
				TextPtr = (const char *)EntryPtr->Data;		/* Get the string pointer */
				Width = FontPtr->GetWidth(FontPtr,TextPtr,strlen(TextPtr));	/* Get the string's width */
				if (Width>Widest) {						/* Wider than before? */
					Widest = Width;						/* Use this value now */
				}
				EntryPtr = EntryPtr->Next;				/* Follow the linked list */
			} while (EntryPtr);							/* More? */
		}
	}
	return Widest;									/* Return the widest entry in pixels */
}

/********************************

	Draw a single character

********************************/

void BURGERCALL FontDrawChar(FontRef_t *Input,Word Letter)
{
	char Ascii;
	Ascii = (char)Letter;
	Input->Draw(Input,&Ascii,1);
}

/********************************

	Draw a longword in the current font

********************************/

void BURGERCALL FontDrawLong(FontRef_t *Input,long Val)
{
	char Ascii[16];

	longToAscii(Val,Ascii);
	Input->Draw(Input,Ascii,strlen(Ascii));
}

/********************************

	Draw a longword in the current font

********************************/

void BURGERCALL FontDrawLongWord(FontRef_t *Input,Word32 Val)
{
	char Ascii[16];

	LongWordToAscii(Val,Ascii);
	Input->Draw(Input,Ascii,strlen(Ascii));
}

/********************************

	Draw a string

********************************/

void BURGERCALL FontDrawString(FontRef_t *Input,const char *TextPtr)
{
	if (TextPtr) {
		Input->Draw(Input,TextPtr,strlen(TextPtr));		/* Draw the text */
	}
}

/********************************

	Draw a centered string

********************************/

void BURGERCALL FontDrawStringCenterX(FontRef_t *Input,int x,int y,const char *TextPtr)
{
	Word Width;
	if (TextPtr) {				/* Failsafe */
		Word Len;
		Len = strlen(TextPtr);	/* Get the string length */
		Width = Input->GetWidth(Input,TextPtr,Len);	/* Get the string width */
		Input->FontX = x - (Width>>1);
		Input->FontY = y;
		Input->Draw(Input,TextPtr,Len);
	}
}

/********************************

	Draw a centered string

********************************/

void BURGERCALL FontDrawStringAtXY(FontRef_t *Input,int x,int y,const char *TextPtr)
{
	Input->FontX = x;		/* Set the location */
	Input->FontY = y;
	if (TextPtr) {				/* Failsafe */
		Input->Draw(Input,TextPtr,strlen(TextPtr));
	}
}

/**********************************

	Return the number of characters
	in a string that would fit in a pixel width.
	
**********************************/

Word BURGERCALL FontCharsForPixelWidth(FontRef_t *RefPtr,const char *TextPtr,Word Width)
{
	Word Result;				/* Return value */
	const char *MarkPtr;
	Word Temp;					/* Number of characters I will TRY to add to the main string */
	const char *Foo;

	Result = 0;					/* Number of characters that are valid */
	MarkPtr = TextPtr;			/* Save in a temp */
	Foo = TextPtr;				/* Get the current mark */

	for (;;) {
		Word PixelWidth;				/* Number of pixels in this test */
		Word Char;						/* Temp */
			
		/* Scan to the next space, CR/LF or the end of the string */
		
		do {
			Char = ((Word8 *)Foo)[0];	/* Get a character */
			++Foo;		/* Next char */
		} while (Char && Char!=' ' && Char != 13 && Char!=10);	/* End here? */
		--Foo;
		Temp = Foo-MarkPtr;			/* Number of characters to test for */

		PixelWidth = RefPtr->GetWidth(RefPtr,TextPtr,Result+Temp);		/* Test the pixel width */
		if (PixelWidth>Width) {			/* Is this too long? */
			if (!Result) {				/* Nothing parsed before */
				Result = Temp;			/* Accept the line as is... (Bad but it will have to do) */
			}
			break;
		}
		Result+=Temp;					/* Accept this length */
		if (Char!=' ') {				/* End of the line */
			break;						/* Return this NOW!! */
		}
		MarkPtr = Foo;					/* My new starting point */
		do {
			++Foo;						/* Skip past the space */
			Char = ((Word8 *)Foo)[0];
		} while (Char==' ');			/* Wait until a non-space */
	}
	return Result;						/* No data to print */
}


/**********************************

	Given a bunch of unformatted text, determine where
	all the line breaks are and then create an array that
	describes when to print each an every line.
	This way, the calculations don't need to be redone every time
	
**********************************/

FontWidthLists_t * BURGERCALL FontWidthListNew(FontRef_t *FontRef,const char *TextPtr,Word PixelWidth)
{
	Word Height;
	const char *WorkPtr;
	FontWidthLists_t *Output;
	
	/* Do allocate the buffer, I need to figure out how many lines */
	/* will be needed in the final product */
	
	WorkPtr = TextPtr;				/* Start the work pointer */
	Height = 0;						/* No lines parsed yet */
	if (((Word8 *)WorkPtr)[0]) {		/* Any data present? */
		do {
			Word Len2;
			Len2 = FontCharsForPixelWidth(FontRef,WorkPtr,PixelWidth);		/* Get the chars to print */
			WorkPtr+=Len2;		/* Next line */
			if (((Word8 *)WorkPtr)[0]==13) {		/* Skip any CR or LF's */
				++WorkPtr;
			}
			if (((Word8 *)WorkPtr)[0]==10) {		/* LF */
				++WorkPtr;
			}
			WorkPtr = ParseBeyondWhiteSpace(WorkPtr);		/* Skip all the spaces at the beginning of the next line */
			++Height;					/* Add 1 to the line count */
		} while (((Word8 *)WorkPtr)[0]);			/* Still more to print? */
	}
	Output = (FontWidthLists_t *)AllocAPointer((sizeof(FontWidthLists_t)-sizeof(FontWidthEntry_t))+(Height*sizeof(FontWidthEntry_t)));
	if (Output) {
		FontWidthEntry_t *DestPtr;
		Output->Count = Height;
		Output->FontPtr = FontRef;
		if (Height) {
			WorkPtr = TextPtr;
			DestPtr = &Output->Entries[0];
			do {
				Word LineLength;
				LineLength = FontCharsForPixelWidth(FontRef,WorkPtr,PixelWidth);
				DestPtr->Length = LineLength;
				DestPtr->Offset = WorkPtr-TextPtr;
				WorkPtr+=LineLength;
				if (((Word8 *)WorkPtr)[0]==13) {		/* Skip any CR or LF's */
					++WorkPtr;
				}
				if (((Word8 *)WorkPtr)[0]==10) {
					++WorkPtr;
				}
				WorkPtr = ParseBeyondWhiteSpace(WorkPtr);		/* Skip all the spaces */
				++DestPtr;					/* Add 1 to the line count */
			} while (((Word8 *)WorkPtr)[0]);			/* Still more to print? */
		}			
	}
	return Output;
}

/**********************************

	Take a pointer to unformatted text, a pointer to a format
	description, a bounds rect to draw to and a Y offset to draw from
	
**********************************/

void BURGERCALL FontWidthListDraw(FontWidthLists_t *Input,const LBRect *BoundsPtr,Word YTop,const char *TextPtr)
{
	FontWidthEntry_t *DataPtr;
	int x,y;
	LBRect ClipRect;
	Word StartIndex;
	Word FontHeight;
	
	FontHeight = Input->FontPtr->FontHeight;
	
	StartIndex = YTop/FontHeight;		/* Which line shall I start with? */
	if (StartIndex<Input->Count) {		/* Is this line even on the text? */
		GetTheClipRect(&ClipRect);		/* Get the screen clip rect */
		SetTheClipRect(BoundsPtr);		/* Set the bounds rect to draw to */
		DataPtr = &Input->Entries[StartIndex];		/* Get the pointer to the text to draw */
		x = BoundsPtr->left;						/* Starting position */
		y = BoundsPtr->top-(YTop-(StartIndex*FontHeight));	/* Starting Y */
		StartIndex = Input->Count-StartIndex;		/* Number of lines of text to draw */
		do {
			Input->FontPtr->FontX = x;		/* Set the origin */
			Input->FontPtr->FontY = y;			
			Input->FontPtr->Draw(Input->FontPtr,TextPtr+DataPtr->Offset,DataPtr->Length);		/* Draw the line of text */
			y +=FontHeight;									/* Next line down */
			++DataPtr;
			if (!--StartIndex) {					/* No more text? */
				break;
			}
		} while (y<BoundsPtr->bottom);				/* Off the bottom? */
		SetTheClipRect(&ClipRect);					/* Restore the clip rect */
	}
}
