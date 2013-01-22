#include "GrGraphics.h"
#include "LrRect.h"
#include "RzRez.h"
#include "MmMemory.h"
#include "ClStdlib.h"
#include "W9Win95.h"

/**********************************

	Just a table for conversion of
	8 bit graphic to 16 bit
	It MUST be initialized by your
	program before use

**********************************/

Word Table8To16[256];
Word *Table8To16Ptr = Table8To16;
Word8 *VideoPointer;			/* Pointer to current video buffer */
Word VideoWidth;			/* Bytes per scan line */
Word ScreenClipLeft;		/* Clip left */
Word ScreenClipTop;			/* Clip top */
Word ScreenClipRight;		/* Clip right */
Word ScreenClipBottom;		/* Clip bottom */
Word ScreenWidth;			/* Width of the screen */
Word ScreenHeight;			/* Height of the screen */
Word ScreenFlags;			/* Flags for the current mode */
Word VideoRedShift;			/* Bits to shift for Red */
Word VideoGreenShift;		/* Bits to shift for Green */
Word VideoBlueShift;		/* Bits to shift for Blue */
Word VideoRedMask;			/* Bitmask for Red */
Word VideoGreenMask;		/* Bitmask for Green */
Word VideoBlueMask;			/* Bitmask for Blue */
Word VideoRedBits;			/* Number of bits of Red */
Word VideoGreenBits;		/* Number of bits of Green */
Word VideoBlueBits;			/* Number of bits of Blue */
Word VideoColorDepth;		/* Bits per pixel currently at */
Word VideoTrueScreenWidth;	/* Width in pixels of hardware video mode */
Word VideoTrueScreenHeight;	/* Height in pixels of hardware video mode */
Word VideoTrueScreenDepth;	/* Depth in BITS of the video display */
Word VideoPixelDoubleMode;	/* Set to the mode requested 0-3 */
Word VideoFullScreen;	/* TRUE if full screen mode is active */
Word VideoPageLocked;		/* True if the video memory is locked */
Word8 *VideoOffscreen;		/* If offscreen buffer is used, here is the pointer */
Word VideoHardWidth;		/* Bytes per scan line on the hardware */
Word VideoAPIInUse;			/* True if OpenGL is present and active */
Word VideoTextureTypesAllowed;	/* Texture formats I can support */
Word VideoTextureRules;			/* Special rules I have to follow */
Word VideoTextureMinWidth;		/* Minimum texture size */
Word VideoTextureMaxWidth;		/* Maximum texture size */
Word VideoTextureMinHeight;		/* Minimum texture height */
Word VideoTextureMaxHeight;		/* Maximum texture height */
Word VideoVertexMaxCount;		/* Maximum number of vertexs */
Word VideoVertexCount;			/* Number of vertex's processed */
Word VideoUseColorZero;			/* TRUE if Color #0 is used in textures */
DrawALineProc ScreenLine = DrawALine;	/* Draw a line intercept vector */
DrawARectProc ScreenRect = DrawARect;	/* Draw a rect intercept vector */
DrawAPixelProc ScreenPixel = (DrawAPixelProc)SetAPixel;	/* Draw a pixel intercept vector */
DrawARectRemapProc ScreenRectRemap = (DrawARectRemapProc)DrawARectRemap;	/* Remap a rect of pixels */

/* These are machine specific and are NOT for general use! */

#if defined(__MSDOS__)
Word BurgerMaxVideoPage;		/* Maximum number of pages I have */
Word BurgerVideoPage;			/* Current video page */
Word32 BurgerScreenSize;		/* Size in bytes of the offscreen buffer */
Word8 *BurgerBaseVideoPointer;	/* Pointer to VESA base video memory range */
Word8 *BurgerVideoPointers[3];	/* Pointers to the 3 VESA memory pages */
Word8 *BurgerVideoCallbackBuffer;	/* Pointer to the VESA call back code */
Bool BurgerLinearFrameBuffer;	/* TRUE if linear frame buffers exit */
Bool Burger8BitPalette = FALSE;	/* TRUE if VESA supports 8 bit palettes */
Word BurgerVesaVersion = 0;		/* 0 = No VESA, 1 = 1.2, 2 = 2.0 */
#endif

#if defined(__WIN32__)
Bool Win95UseBackBuffer=TRUE;	/* True if the backbuffer is used */
void *Win95LockedMemory;		/* Copy of the video pointer */
void *BurgerHBitMap;			/* HBITMAP for window mode */
Word8 *BurgerHBitMapPtr;			/* Pointer to HBITMAP Memory */
void *BurgerHPalette;			/* HPALETTE for window mode */
#endif

/**********************************

	Get's the current video mode and fills in
	a structure to record it.

**********************************/

void BURGERCALL VideoGetCurrentMode(VideoSaveState_t *Input)
{
	Input->Width = ScreenWidth;
	Input->Height = ScreenHeight;
	Input->Depth = VideoColorDepth;
	Input->Flags = ScreenFlags;
}

/**********************************

	Given a structure that describes the requested
	video mode, restore it

**********************************/

Word BURGERCALL VideoSetMode(const VideoSaveState_t *Input)
{
	if (Input && Input->Width && Input->Height) {	/* Valid? */
		return SetDisplayToSize(Input->Width,Input->Height,Input->Depth,Input->Flags);
	}
	return TRUE;
}

/**********************************

	Set the clip rect

**********************************/

void BURGERCALL SetTheClipBounds(Word Left,Word Top,Word Right,Word Bottom)
{
	Word Temp;
	if (Left>Right) {	/* Sort Left and right */
		Temp = Left;
		Left = Right;
		Right = Temp;
	}

	if (Top>Bottom) {	/* Sort Top and Bottom */
		Temp = Top;
		Top = Bottom;
		Bottom = Temp;
	}

	if (Left<ScreenWidth && Top<ScreenHeight && Left!=Right && Top!=Bottom) {
		if (Right>ScreenWidth) {		/* Clip the rect */
			Right = ScreenWidth;
		}
		if (Bottom>ScreenHeight) {
			Bottom = ScreenHeight;
		}
		ScreenClipLeft = Left;
		ScreenClipTop = Top;
		ScreenClipRight = Right;
		ScreenClipBottom = Bottom;
		return;
	}
	ScreenClipLeft = 0;		/* Invalid bounds rect! */
	ScreenClipTop = 0;
	ScreenClipRight = 0;
	ScreenClipBottom = 0;
}

/**********************************

	Set the current clip rect

**********************************/

void BURGERCALL SetTheClipRect(const LBRect *RectPtr)
{
	ScreenClipLeft = RectPtr->left;
	ScreenClipTop = RectPtr->top;
	ScreenClipRight = RectPtr->right;
	ScreenClipBottom = RectPtr->bottom;
}

/**********************************

	Get the current clip rect

**********************************/

void BURGERCALL GetTheClipRect(LBRect *RectPtr)
{
	RectPtr->left = ScreenClipLeft;
	RectPtr->top = ScreenClipTop;
	RectPtr->right = ScreenClipRight;
	RectPtr->bottom = ScreenClipBottom;
}

/**********************************

	Return a pixel color from the screen, I assume the
	x and y are valid

**********************************/

Word GetAPixel(Word x,Word y)
{
	return VideoPointer[(VideoWidth*y)+x];		/* Get base address */
}

/**********************************

	Return a pixel color from the screen, I assume the
	x and y are valid

**********************************/

Word GetAPixel16(Word x,Word y)
{
	return ((Word16 *)(VideoPointer+(VideoWidth*y)))[x];		/* Get base address */
}

/**********************************

	Set a pixel color on the screen, I assume the
	x and y are valid

**********************************/

void SetAPixel(Word x,Word y,Word Color)
{
	VideoPointer[(VideoWidth*y)+x] = (Word8)Color;		/* Get base address */
}

/**********************************

	Set a pixel color on the screen, I assume the
	x and y are valid

**********************************/

void SetAPixel16(Word x,Word y,Word Color)
{
	((Word16 *)(VideoPointer+(VideoWidth*y)))[x] = (Word16)Color;		/* Get base address */
}

/**********************************

	Set a pixel color on the screen, I assume the
	x and y are valid

**********************************/

void SetAPixelTo16(Word x,Word y,Word Color)
{
	((Word16 *)(VideoPointer+(VideoWidth*y)))[x] = (Word16)Table8To16Ptr[Color];		/* Get base address */
}

/**********************************

	Draw an 8 bit per pixel line
	clipped to the bounds rect
	This routine hurts my brain.

**********************************/

void BURGERCALL DrawALine(int x1,int y1,int x2,int y2,Word Color)
{
	int StepX,StepY;	/* X and Y step (Must be signed!) */
	Word DistX,DistY;	/* Full X and Y motion */
	Word Delta;			/* Stepper */
	Word Count;			/* Loop count */
	Word8 *Screenad;		/* Screen address */

	if (y1>y2) {		/* Sort the points by Y */
		StepX = x1;		/* This causes the line to never */
		x1 = x2;		/* vary and I can optimize assuming */
		x2 = StepX;		/* that y1<=y2 */
		StepY = y1;
		y1 = y2;
		y2 = StepY;
	}

	/* Since the Y's are sorted, check if I am out of Y bounds */

	if (y2<(int)ScreenClipTop || y1>=(int)ScreenClipBottom) {
		return;			/* Exit NOW! */
	}

	/* I can't sort the X, so I create the step either */
	/* to the left or the right */

	DistX = x2-x1;		/* Get the X delta (signed) */
	if ((int)DistX<0) {		/* Step to the left? */
		DistX = 0-DistX;	/* Negate the step to a positive value */
		StepX = -1;			/* Negative X motion */
		if (x1<(int)ScreenClipLeft || x2>=(int)ScreenClipRight) {
			return;		/* It's off the screen! */
		}
	} else {
		if (x2<(int)ScreenClipLeft || x1>=(int)ScreenClipRight) {
			return;		/* It's off the screen! */
		}
		StepX = 1;		/* Positive X */
	}

	/* At this point I will draw SOMETHING */
	/* How much you ask? Good question. */
	/* The bounds rect intersects the line rect, */
	/* I will clip the line to the bounds rect but beware */
	/* that I could clip the ENTIRE line off */

	StepY = VideoWidth;	/* Assume positive Y motion */
	DistY = y2-y1;		/* Get the Y delta (Positive, y1<=y2) */

	Screenad = VideoPointer+((int)VideoWidth*y1)+x1;	/* Init the dest address */

	if (DistX>=DistY) {		/* Step in the X? */
		Word Truncate;		/* Hack to allow clipping on the bottom */
		
		/* If this is a single pixel, the bounds rect already clipped me */
		/* so just draw it and get out */

		if (!DistX) {
			Screenad[0] = (Word8)Color;
			return;
		}

	/* This code assumes I am stepping in the X direction */

		Delta = DistX>>1;	/* Init the delta to 0.5 */
		Count = DistX;		/* This is the pixel count (-1) */

		/* Shall I clip the Xs? */

		Truncate = 0;
		if (StepX>0) {			/* Going right? */
			if (x1<(int)ScreenClipLeft) {
				x1 = ScreenClipLeft-x1;		/* How many lines to chop */
				Delta += x1*DistY;			/* Step the Y delta */
				Screenad += x1;				/* Move the address to the right */
				Count -= x1;				/* Remove the count */
				x1 = Delta/DistX;			/* Get the Y step */
				Delta %= DistX;				/* Fix the delta */
				Screenad += StepY*x1;		/* Move the Y */
				y1 += x1;					/* Set the new Y coord for clipping */
			}
			if (x2>=(int)ScreenClipRight) {	/* Off the right? */
				Truncate = (x2-ScreenClipRight)+1;
				Count -= Truncate;	/* Just truncate */
			}
		} else {			/* I'm going left! */
			if (x1>=(int)ScreenClipRight) {
				x1 = (x1-ScreenClipRight)+1;	/* How many lines to chop */
				Delta += x1*DistY;			/* Step the Y delta */
				Screenad -= x1;				/* Move the address to the left */
				Count -= x1;				/* Remove the count */
				x1 = Delta/DistX;			/* Get the Y step */
				Delta %= DistX;				/* Fix the delta */
				Screenad += StepY*x1;		/* Move the Y */
				y1 += x1;					/* Set the new Y coord for clipping */
			}
			if (x2<(int)ScreenClipLeft) {
				Truncate = ScreenClipLeft-x2;
				Count -= Truncate;	/* Just truncate */
			}
		}


		/* Shall I clip the top? */

		if (y1<(int)ScreenClipTop) {	/* Did it clip? */
			int Foo;
			y1 = ScreenClipTop-y1;	/* Number of pixels to step */
			Foo = (((y1*DistX)-Delta)/DistY)+1;
			Delta += Foo*DistY;			/* Step the Y delta */
			Screenad += Foo*StepX;		/* Move the address (signed) */
			Count -= Foo;				/* Remove the count */
			Foo = Delta/DistX;			/* Get the Y step */
			Delta %= DistX;				/* Fix the delta */
			Screenad += StepY*Foo;		/* Move the X */
		}

		/* Shall I clip the bottom? */

		if (y2>=(int)ScreenClipBottom) {
			int Foo;
			y2 = (y2-ScreenClipBottom)+1;	/* Remove from the bottom */
			Foo = (((y2*DistX)-Delta)/DistY)+1;
			if (Foo>(int)Truncate) {
				Count -= (Foo-Truncate);		/* Truncate */
			}
		}

		/* Draw the X stepping line */

		if ((int)Count>=0) {		/* Anything left? */
			Screenad[0] = (Word8)Color;	/* Draw the initial pixel */
			if (Count) {			/* More? */
				do {
					Delta += DistY;		/* Add the Y step */
					if (Delta>=DistX) {	/* Greater than the X factor */
						Delta -= DistX;	/* Remove the excess */
						Screenad += StepY;
					}
					Screenad += StepX;		/* Inc the x step */
					Screenad[0] = (Word8)Color;
				} while (--Count);		/* All done? */
			}
		}
		return;
	}

	/* This code assumes I am stepping in the Y direction */

	Delta = DistY>>1;		/* Init the delta step */
	Count = DistY;			/* Number of pixels to draw */

	/* Shall I clip the Xs? */

	if (StepX>0) {			/* Going right? */
		if (x1<(int)ScreenClipLeft) {
			int Foo;
			x1 = ScreenClipLeft-x1;	/* Number of pixels to step */
			Foo = (((x1*DistY)-Delta)/DistX)+1;
			y1 += Foo;
			Delta += Foo*DistX;			/* Step the X delta */
			Screenad += Foo*StepY;		/* Move the address down */
			Count -= Foo;				/* Remove the count */
			Foo = Delta/DistY;			/* Get the X step */
			Delta %= DistY;				/* Fix the delta */
			Screenad += StepX*Foo;		/* Move the X (signed) */
		}
		if (x2>=(int)ScreenClipRight) {
			int Foo;
			x2 = (x2-ScreenClipRight)+1;
			Foo = (((x2*DistY)-Delta)/DistX)+1;	/* Get the pixel count */
			Count -= Foo;				/* Truncate */
			y2 -= Foo;					/* Adjust the bottom */
		}
	} else {			/* I'm going left! */
		if (x1>=(int)ScreenClipRight) {
			int Foo;
			x1 = (x1-ScreenClipRight)+1;	/* Number of pixels to step */
			Foo = (((x1*DistY)-Delta)/DistX)+1;
			y1 += Foo;
			Delta += Foo*DistX;			/* Step the X delta */
			Screenad += Foo*StepY;		/* Move the address down */
			Count -= Foo;				/* Remove the count */
			Foo = Delta/DistY;			/* Get the X step */
			Delta %= DistY;				/* Fix the delta */
			Screenad += StepX*Foo;		/* Move the X (signed) */
		}
		if (x2<(int)ScreenClipLeft) {
			int Foo;
			x2 = ScreenClipLeft-x2;
			Foo = (((x2*DistY)-Delta)/DistX)+1;
			Count -= Foo;		/* Truncate */
			y2 -= Foo;			/* Adjust the bottom */
		}

	}


	/* Shall I clip the top? */

	if (y1<(int)ScreenClipTop) {	/* Did it clip? */
		y1 = ScreenClipTop-y1;		/* How many lines to chop */
		Delta += y1*DistX;			/* Step the X delta */
		Screenad += y1*StepY;		/* Move the address down */
		Count -= y1;				/* Remove the count */
		y1 = Delta/DistY;			/* Get the X step */
		Delta %= DistY;				/* Fix the delta */
		Screenad += StepX*y1;		/* Move the X (signed) */
	}

	/* Shall I clip the bottom? */

	if (y2>=(int)ScreenClipBottom) {
		Count -= (y2-ScreenClipBottom)+1;	/* Just truncate */
	}

	if ((int)Count>=0) {		/* Anything left? */
		Screenad[0] = (Word8)Color;	/* Draw the starting pixel */
		if (Count) {			/* More? */
			do {		/* Merge the X coord into the screen pointer */
				Delta += DistX;
				if (Delta>=DistY) {
					Delta -= DistY;	/* Remove the overflow */
					Screenad += StepX;	/* Inc the x step (Directly in screen pointer) */
				}
				Screenad += StepY;		/* Inc the y step */
				Screenad[0] = (Word8)Color;
			} while (--Count);
		}
	}
}

/**********************************

	Draw a 16 bit per pixel line
	clipped to the bounds rect
	This routine hurts my brain.

**********************************/

void BURGERCALL DrawALine16(int x1,int y1,int x2,int y2,Word Color)
{
	int StepX,StepY;	/* X and Y step (Must be signed!) */
	Word DistX,DistY;	/* Full X and Y motion */
	Word Delta;			/* Stepper */
	Word Count;			/* Loop count */
	Word8 *Screenad;		/* Screen address */

	if (y1>y2) {		/* Sort the points by Y */
		StepX = x1;		/* This causes the line to never */
		x1 = x2;		/* vary and I can optimize assuming */
		x2 = StepX;		/* that y1<=y2 */
		StepY = y1;
		y1 = y2;
		y2 = StepY;
	}

	/* Since the Y's are sorted, check if I am out of Y bounds */

	if (y2<(int)ScreenClipTop || y1>=(int)ScreenClipBottom) {
		return;			/* Exit NOW! */
	}

	/* I can't sort the X, so I create the step either */
	/* to the left or the right */

	DistX = x2-x1;		/* Get the X delta (signed) */
	if ((int)DistX<0) {		/* Step to the left? */
		DistX = 0-DistX;	/* Negate the step to a positive value */
		StepX = -2;			/* Negative X motion */
		if (x1<(int)ScreenClipLeft || x2>=(int)ScreenClipRight) {
			return;		/* It's off the screen! */
		}
	} else {
		if (x2<(int)ScreenClipLeft || x1>=(int)ScreenClipRight) {
			return;		/* It's off the screen! */
		}
		StepX = 2;		/* Positive X */
	}

	/* At this point I will draw SOMETHING */
	/* How much you ask? Good question. */
	/* The bounds rect intersects the line rect, */
	/* I will clip the line to the bounds rect but beware */
	/* that I could clip the ENTIRE line off */

	StepY = VideoWidth;	/* Assume positive Y motion */
	DistY = y2-y1;		/* Get the Y delta (Positive, y1<=y2) */

	Screenad = VideoPointer+((int)VideoWidth*y1)+(x1<<1);	/* Init the dest address */

	if (DistX>=DistY) {		/* Step in the X? */
		Word Truncate;
		
		/* If this is a single pixel, the bounds rect already clipped me */
		/* so just draw it and get out */

		if (!DistX) {
			((Word16 *)Screenad)[0] = (Word16)Color;
			return;
		}

	/* This code assumes I am stepping in the X direction */

		Delta = DistX>>1;	/* Init the delta to 0.5 */
		Count = DistX;		/* This is the pixel count (-1) */

		/* Shall I clip the Xs? */
		Truncate = 0;
		if (StepX>0) {			/* Going right? */
			if (x1<(int)ScreenClipLeft) {
				x1 = ScreenClipLeft-x1;		/* How many lines to chop */
				Delta += x1*DistY;			/* Step the Y delta */
				Screenad += x1<<1;			/* Move the address to the right */
				Count -= x1;				/* Remove the count */
				x1 = Delta/DistX;			/* Get the Y step */
				Delta %= DistX;				/* Fix the delta */
				Screenad += StepY*x1;		/* Move the Y */
				y1 += x1;					/* Set the new Y coord for clipping */
			}
			if (x2>=(int)ScreenClipRight) {	/* Off the right? */
				Truncate = (x2-ScreenClipRight)+1;
				Count -= Truncate;	/* Just truncate */
			}
		} else {			/* I'm going left! */
			if (x1>=(int)ScreenClipRight) {
				x1 = (x1-ScreenClipRight)+1;	/* How many lines to chop */
				Delta += x1*DistY;			/* Step the Y delta */
				Screenad -= x1<<1;			/* Move the address to the left */
				Count -= x1;				/* Remove the count */
				x1 = Delta/DistX;			/* Get the Y step */
				Delta %= DistX;				/* Fix the delta */
				Screenad += StepY*x1;		/* Move the Y */
				y1 += x1;					/* Set the new Y coord for clipping */
			}
			if (x2<(int)ScreenClipLeft) {
				Truncate = ScreenClipLeft-x2;
				Count -= Truncate;	/* Just truncate */
			}
		}


		/* Shall I clip the top? */

		if (y1<(int)ScreenClipTop) {	/* Did it clip? */
			int Foo;
			y1 = ScreenClipTop-y1;	/* Number of pixels to step */
			Foo = (((y1*DistX)-Delta)/DistY)+1;
			Delta += Foo*DistY;			/* Step the Y delta */
			Screenad += Foo*StepX;		/* Move the address (signed) */
			Count -= Foo;				/* Remove the count */
			Foo = Delta/DistX;			/* Get the Y step */
			Delta %= DistX;				/* Fix the delta */
			Screenad += StepY*Foo;		/* Move the X */
		}

		/* Shall I clip the bottom? */

		if (y2>=(int)ScreenClipBottom) {
			int Foo;
			y2 = (y2-ScreenClipBottom)+1;	/* Remove from the bottom */
			Foo = (((y2*DistX)-Delta)/DistY)+1;
			if (Foo>(int)Truncate) {
				Count -= (Foo-Truncate);		/* Truncate */
			}
		}

		/* Draw the X stepping line */

		if ((int)Count>=0) {		/* Anything left? */
			((Word16 *)Screenad)[0] = (Word16)Color;	/* Draw the initial pixel */
			if (Count) {			/* More? */
				do {
					Delta += DistY;		/* Add the Y step */
					if (Delta>=DistX) {	/* Greater than the X factor */
						Delta -= DistX;	/* Remove the excess */
						Screenad += StepY;
					}
					Screenad += StepX;		/* Inc the x step */
					((Word16 *)Screenad)[0] = (Word16)Color;
				} while (--Count);		/* All done? */
			}
		}
		return;
	}

	/* This code assumes I am stepping in the Y direction */

	Delta = DistY>>1;		/* Init the delta step */
	Count = DistY;			/* Number of pixels to draw */

	/* Shall I clip the Xs? */

	if (StepX>0) {			/* Going right? */
		if (x1<(int)ScreenClipLeft) {
			int Foo;
			x1 = ScreenClipLeft-x1;	/* Number of pixels to step */
			Foo = (((x1*DistY)-Delta)/DistX)+1;
			y1 += Foo;
			Delta += Foo*DistX;			/* Step the X delta */
			Screenad += Foo*StepY;		/* Move the address down */
			Count -= Foo;				/* Remove the count */
			Foo = Delta/DistY;			/* Get the X step */
			Delta %= DistY;				/* Fix the delta */
			Screenad += StepX*Foo;		/* Move the X (signed) */
		}
		if (x2>=(int)ScreenClipRight) {
			int Foo;
			x2 = (x2-ScreenClipRight)+1;
			Foo = (((x2*DistY)-Delta)/DistX)+1;	/* Get the pixel count */
			Count -= Foo;				/* Truncate */
			y2 -= Foo;					/* Adjust the bottom */
		}
	} else {			/* I'm going left! */
		if (x1>=(int)ScreenClipRight) {
			int Foo;
			x1 = (x1-ScreenClipRight)+1;	/* Number of pixels to step */
			Foo = (((x1*DistY)-Delta)/DistX)+1;
			y1 += Foo;
			Delta += Foo*DistX;			/* Step the X delta */
			Screenad += Foo*StepY;		/* Move the address down */
			Count -= Foo;				/* Remove the count */
			Foo = Delta/DistY;			/* Get the X step */
			Delta %= DistY;				/* Fix the delta */
			Screenad += StepX*Foo;		/* Move the X (signed) */
		}
		if (x2<(int)ScreenClipLeft) {
			int Foo;
			x2 = ScreenClipLeft-x2;
			Foo = (((x2*DistY)-Delta)/DistX)+1;
			Count -= Foo;		/* Truncate */
			y2 -= Foo;			/* Adjust the bottom */
		}

	}


	/* Shall I clip the top? */

	if (y1<(int)ScreenClipTop) {	/* Did it clip? */
		y1 = ScreenClipTop-y1;		/* How many lines to chop */
		Delta += y1*DistX;			/* Step the X delta */
		Screenad += y1*StepY;		/* Move the address down */
		Count -= y1;				/* Remove the count */
		y1 = Delta/DistY;			/* Get the X step */
		Delta %= DistY;				/* Fix the delta */
		Screenad += StepX*y1;		/* Move the X (signed) */
	}

	/* Shall I clip the bottom? */

	if (y2>=(int)ScreenClipBottom) {
		Count -= (y2-ScreenClipBottom)+1;	/* Just truncate */
	}

	if ((int)Count>=0) {		/* Anything left? */
		((Word16 *)Screenad)[0] = (Word16)Color;	/* Draw the starting pixel */
		if (Count) {			/* More? */
			do {		/* Merge the X coord into the screen pointer */
				Delta += DistX;
				if (Delta>=DistY) {
					Delta -= DistY;	/* Remove the overflow */
					Screenad += StepX;	/* Inc the x step (Directly in screen pointer) */
				}
				Screenad += StepY;		/* Inc the y step */
				((Word16 *)Screenad)[0] = (Word16)Color;
			} while (--Count);
		}
	}
}

/**********************************

	Draw a 16 bit per pixel line
	using an 8 bit index for the color

**********************************/

void BURGERCALL DrawALineTo16(int x1,int y1,int x2,int y2,Word Color)
{
	DrawALine16(x1,y1,x2,y2,Table8To16Ptr[Color]);
}

/**********************************

	Remap a rect of pixels

**********************************/

void BURGERCALL DrawARectRemap(int x,int y,Word Width,Word Height,const Word8 *RemapPtr)
{
	Word8 *Screenad;

	/* Software renderer */
	if (x>=(int)ScreenClipRight || y>=(int)ScreenClipBottom) {	/* Off the right or the bottom? */
		goto Exit;
	}
	Width+=x;			/* Convert to a rect */
	Height+=y;
	if ((int)Width<=(int)ScreenClipLeft || (int)Height<=(int)ScreenClipTop) {		/* Off the left or top? */
		goto Exit;
	}
	if (x<(int)ScreenClipLeft) {			/* Should I clip the X? */
		goto ClipX;
	}
	if (y<(int)ScreenClipTop) {			/* Should I clip the Y? */
		goto ClipY;
	}
YClipOk:
	if (Width>ScreenClipRight) {		/* Is it too wide? */
		goto ClipWidth;
	}
WidthOk:
	if (Height>ScreenClipBottom) {
		goto ClipHeight;
	}
HeightOk:
	Width-=x;		/* Restore a true width and height */
	Height-=y;
	if (Width && Height) {
		Screenad = &VideoPointer[(VideoWidth*y)+x];	/* Get the screen pointer */
		x = VideoWidth-Width;
		do {
			y = Width;
			do {
				Screenad[0] = RemapPtr[Screenad[0]];
				++Screenad;
			} while (--y);
			Screenad+=x;					/* Next line down */
		} while (--Height);						/* All done? */
	}
Exit:;
	return;

ClipX:
	x = ScreenClipLeft;			/* Clip the x */
	if (y>=(int)ScreenClipTop) {		/* Clip the y? */
		goto YClipOk;	/* Nope, it's ok */
	}
ClipY:
	y = ScreenClipTop;			/* Clip the y */
	if (Width<=ScreenClipRight) {	/* Too wide? */
		goto WidthOk;		/* Nope, accept it */
	}
ClipWidth:
	Width = ScreenClipRight;	/* Maximize the width */
	if (Height<=ScreenClipBottom) {	/* Too high? */
		goto HeightOk;		/* Nope, accept it */
	}
ClipHeight:
	Height = ScreenClipBottom;	/* Maximize the height */
	goto HeightOk;
}

/**********************************

	Remap a rect of pixels

**********************************/

void BURGERCALL DrawARectRemap16(int x,int y,Word Width,Word Height,const Word16 *RemapPtr)
{
	Word8 *Screenad;

	/* Software renderer */
	if (x>=(int)ScreenClipRight || y>=(int)ScreenClipBottom) {	/* Off the right or the bottom? */
		goto Exit;
	}
	Width+=x;			/* Convert to a rect */
	Height+=y;
	if ((int)Width<=(int)ScreenClipLeft || (int)Height<=(int)ScreenClipTop) {		/* Off the left or top? */
		goto Exit;
	}
	if (x<(int)ScreenClipLeft) {			/* Should I clip the X? */
		goto ClipX;
	}
	if (y<(int)ScreenClipTop) {			/* Should I clip the Y? */
		goto ClipY;
	}
YClipOk:
	if (Width>ScreenClipRight) {		/* Is it too wide? */
		goto ClipWidth;
	}
WidthOk:
	if (Height>ScreenClipBottom) {
		goto ClipHeight;
	}
HeightOk:
	Width-=x;		/* Restore a true width and height */
	Height-=y;
	if (Width && Height) {
		Screenad = &VideoPointer[(VideoWidth*y)+(x<<1)];	/* Get the screen pointer */
		x = VideoWidth-(Width<<1);
		do {
			y = Width;
			do {
				((Word16 *)Screenad)[0] = RemapPtr[((Word16 *)Screenad)[0]];
				Screenad+=2;
			} while (--y);
			Screenad+=x;					/* Next line down */
		} while (--Height);						/* All done? */
	}
Exit:;
	return;

ClipX:
	x = ScreenClipLeft;			/* Clip the x */
	if (y>=(int)ScreenClipTop) {		/* Clip the y? */
		goto YClipOk;	/* Nope, it's ok */
	}
ClipY:
	y = ScreenClipTop;			/* Clip the y */
	if (Width<=ScreenClipRight) {	/* Too wide? */
		goto WidthOk;		/* Nope, accept it */
	}
ClipWidth:
	Width = ScreenClipRight;	/* Maximize the width */
	if (Height<=ScreenClipBottom) {	/* Too high? */
		goto HeightOk;		/* Nope, accept it */
	}
ClipHeight:
	Height = ScreenClipBottom;	/* Maximize the height */
	goto HeightOk;
}

/**********************************

	Draw a rectangle in a solid color

**********************************/

void BURGERCALL DrawARectTo16(int x,int y,Word Width,Word Height,Word Color)
{
	DrawARect16(x,y,Width,Height,Table8To16Ptr[Color]);
}

/**********************************

	Draw a rectangle in a solid color

**********************************/

#if !defined(__WIN32__)
void BURGERCALL DrawARect(int x,int y,Word Width,Word Height,Word Color)
{
	Word8 *Screenad;

	/* Software renderer */
	if (x>=(int)ScreenClipRight || y>=(int)ScreenClipBottom) {	/* Off the right or the bottom? */
		goto Exit;
	}
	Width+=x;			/* Convert to a rect */
	Height+=y;
	if ((int)Width<=(int)ScreenClipLeft || (int)Height<=(int)ScreenClipTop) {		/* Off the left or top? */
		goto Exit;
	}
	if (x<(int)ScreenClipLeft) {			/* Should I clip the X? */
		goto ClipX;
	}
	if (y<(int)ScreenClipTop) {			/* Should I clip the Y? */
		goto ClipY;
	}
YClipOk:
	if (Width>ScreenClipRight) {		/* Is it too wide? */
		goto ClipWidth;
	}
WidthOk:
	if (Height>ScreenClipBottom) {
		goto ClipHeight;
	}
HeightOk:
	Width-=x;		/* Restore a true width and height */
	Height-=y;
	if (Width && Height) {
		Screenad = &VideoPointer[(VideoWidth*y)+x];	/* Get the screen pointer */
		x = VideoWidth;
		if (Width==(Word)x) {		/* Can I do this in one shot? */
			goto FillAll;	/* Call memfill just one time */
		}
		do {
			FastMemSet(Screenad,Color,Width);		/* Fill memory */
			Screenad+=x;					/* Next line down */
		} while (--Height);						/* All done? */
	}
Exit:;
	return;
FillAll:
	FastMemSet(Screenad,Color,Width*Height);	/* Fill the video buffer */
	return;

ClipX:
	x = ScreenClipLeft;			/* Clip the x */
	if (y>=(int)ScreenClipTop) {		/* Clip the y? */
		goto YClipOk;	/* Nope, it's ok */
	}
ClipY:
	y = ScreenClipTop;			/* Clip the y */
	if (Width<=ScreenClipRight) {	/* Too wide? */
		goto WidthOk;		/* Nope, accept it */
	}
ClipWidth:
	Width = ScreenClipRight;	/* Maximize the width */
	if (Height<=ScreenClipBottom) {	/* Too high? */
		goto HeightOk;		/* Nope, accept it */
	}
ClipHeight:
	Height = ScreenClipBottom;	/* Maximize the height */
	goto HeightOk;
}
#endif

/**********************************

	Draw a rectangle in a solid color

**********************************/

#if !defined(__WIN32__)

void BURGERCALL DrawARect16(int x,int y,Word Width,Word Height,Word Color)
{
	Word8 *Screenad;

	if (x>=(int)ScreenClipRight || y>=(int)ScreenClipBottom) {	/* Off the right or the bottom? */
		goto Exit;
	}
	Width+=x;			/* Convert to a rect */
	Height+=y;
	if ((int)Width<=(int)ScreenClipLeft || (int)Height<=(int)ScreenClipTop) {		/* Off the left or top? */
		goto Exit;
	}
	if (x<(int)ScreenClipLeft) {			/* Should I clip the X? */
		goto ClipX;
	}
	if (y<(int)ScreenClipTop) {			/* Should I clip the Y? */
		goto ClipY;
	}
YClipOk:
	if (Width>ScreenClipRight) {		/* Is it too wide? */
		goto ClipWidth;
	}
WidthOk:
	if (Height>ScreenClipBottom) {
		goto ClipHeight;
	}
HeightOk:
	Width-=x;		/* Restore a true width and height */
	Height-=y;
	x<<=1;			/* Word16 offset */
	Width<<=1;		/* Now I convert pixels to shorts */

	Screenad = &VideoPointer[(VideoWidth*y)+x];	/* Get the screen pointer */
	x = VideoWidth;
	if (Width==(Word)x) {		/* Can I do this in one shot? */
		goto FillAll;	/* Call memfill just one time */
	}
	do {
		FastMemSet16(Screenad,Color,Width);		/* Fill memory */
		Screenad+=x;					/* Next line down */
	} while (--Height);					/* All done? */
Exit:;
	return;
FillAll:
	FastMemSet16(Screenad,Color,Width*Height);	/* Fill the video buffer */
	return;

ClipX:
	x = ScreenClipLeft;			/* Clip the x */
	if (y>=(int)ScreenClipTop) {		/* Clip the y? */
		goto YClipOk;	/* Nope, it's ok */
	}
ClipY:
	y = ScreenClipTop;			/* Clip the y */
	if (Width<=ScreenClipRight) {	/* Too wide? */
		goto WidthOk;		/* Nope, accept it */
	}
ClipWidth:
	Width = ScreenClipRight;	/* Maximize the width */
	if (Height<=ScreenClipBottom) {	/* Too high? */
		goto HeightOk;		/* Nope, accept it */
	}
ClipHeight:
	Height = ScreenClipBottom;	/* Maximize the height */
	goto HeightOk;
}
#endif

/**********************************

	Draw a masked shape to the screen

**********************************/

//#if !defined(__INTEL__) || defined(__MWERKS__) || defined(__BEOS__)

void BURGERCALL DrawMShapeLowLevel(Word x,Word y,Word Width,Word Height,Word Skip,void *ShapePtr)
{
	register Word8 *Screenad;		/* Pointer to screen memory */
	register Word i;				/* Temp */
	Word32 Offset;		/* VideoWidth-Width */
	register Word8 a,b;	/* Force the compiler to use BYTE addressing */
	Word LongWidth;

	if (!Width || !Height) {
		return;
	}
	Screenad = &VideoPointer[(VideoWidth*y)+x];		/* Get base address */
	Offset = VideoWidth-Width;		/* Precalc the offset */
	LongWidth = Width>>2;
	if (!LongWidth) {
		goto BytesOnly;
	}
	Width = Width&3;
	do {
		i = LongWidth;		/* Reset the width count */
		do {
			a = ((Word8 *)ShapePtr)[0];
			b = ((Word8 *)ShapePtr)[1];
			if (a) {
				Screenad[0] = a;
			}
			if (b) {
				Screenad[1] = b;
			}
			a = ((Word8 *)ShapePtr)[2];
			b = ((Word8 *)ShapePtr)[3];
			if (a) {
				Screenad[2] = a;
			}
			if (b) {
				Screenad[3] = b;
			}
			Screenad = Screenad+4;
			ShapePtr = ((Word8 *)ShapePtr)+4;
		} while (--i);		/* All done? */
		if (Width) {
			i = Width;
			do {
				a = ((Word8 *)ShapePtr)[0];
				if (a) {
					Screenad[0] = a;
				}
				Screenad = Screenad+1;
				ShapePtr = ((Word8 *)ShapePtr)+1;
			} while (--i);		/* All done? */
		}
		Screenad+=Offset;	/* Adjust the dest pointer */
		ShapePtr = ((Word8 *)ShapePtr)+Skip;
	} while (--Height);		/* Totally done? */
	return;

	/* 1-3 bytes wide */
BytesOnly:
	do {
		i = Width;		/* Reset the width count */
		do {
			a = ((Word8 *)ShapePtr)[0];
			if (a) {
				Screenad[0] = a;
			}
			Screenad = Screenad+1;
			ShapePtr = ((Word8 *)ShapePtr)+1;
		} while (--i);		/* All done? */
		Screenad+=Offset;	/* Adjust the dest pointer */
		ShapePtr = ((Word8 *)ShapePtr)+Skip;
	} while (--Height);		/* Totally done? */
	return;
}
//#endif

/**********************************

	Draw a masked shape to the screen quickly!

**********************************/

void BURGERCALL DrawMShapeLowLevel16(Word x,Word y,Word Width,Word Height,Word Skip,void *ShapePtr)
{
	Word16 *Screenad;		/* Pointer to screen memory */
	Word i;				/* Temp */
	Word32 Offset;		/* VideoWidth-Width */

	if (Height && Width) {	/* Invalid shape? */
		Screenad = (Word16 *)(&VideoPointer[(VideoWidth*y)+x*2]);		/* Get base address */
		Offset = VideoWidth-(Width*2);		/* Precalc the offset */
		do {
			i = Width;
			do {
				Word Temp;
				Temp = ((Word16 *)ShapePtr)[0];
				if (Temp) {
					Screenad[0] = (Word16)Temp;	/* 8 bit to 16 bit conversion table */
				}
				++Screenad;
				ShapePtr = ((Word8 *)ShapePtr)+2;
			} while (--i);
			Screenad = (Word16 *)(&((Word8 *)Screenad)[Offset]);
			ShapePtr = ((Word8 *)ShapePtr)+Skip;
		} while (--Height);
	}
}

/**********************************

	Clip the shape to the visible screen.
	Draw only a masked shape.

**********************************/

void BURGERCALL DrawMShapeLowLevelClippedTo16(int x,int y,Word Width,Word Height,Word Skip2,void *ShapePtr)
{
	Word Skip;
	if (x<(int)ScreenClipRight && y<(int)ScreenClipBottom) {	/* Clip off right or bottom? */
		if (y<(int)ScreenClipTop) {		/* Clip the top */
			Skip = ScreenClipTop-y;
			Height = Height-Skip;		/* Remove from the shape */
			if ((int)Height<=0) {		/* Clipped off the top? */
				return;
			}
			ShapePtr = ((Word8 *)ShapePtr)+((Width+Skip2)*Skip);	/* Adjust the pointer */
			y = ScreenClipTop;		/* New top Y */
		}

		if (((Word)y+Height)>ScreenClipBottom) {
			Height = ScreenClipBottom-y;		/* Clip the bottom */
		}

		Skip = 0;		/* Assume I don't skip any bytes */

		if (x<(int)ScreenClipLeft) {
			Skip = ScreenClipLeft-x;
			Width = Width-Skip;
			if ((int)Width<=0) {		/* Clipped off the left? */
				return;
			}
			ShapePtr = ((Word8 *)ShapePtr)+Skip;	/* Adjust the source shape */
			x = ScreenClipLeft;		/* New left X */
		}

		if (((Word)x+Width)>ScreenClipRight) {	/* Clip the right? */
			Word Temp;
			Temp = ScreenClipRight-x;		/* Get the NEW width amount */
			Skip = (Width-Temp)+Skip;	/* Add to the line skip */
			Width = Temp;		/* Save new width */
		}
		DrawMShapeLowLevelTo16(x,y,Width,Height,Skip+Skip2,ShapePtr);
	}
}

/**********************************

	Clip the shape to the visible screen.
	Draw only a masked shape.

**********************************/

void BURGERCALL DrawMShapeLowLevelClipped(int x,int y,Word Width,Word Height,Word Skip2,void *ShapePtr)
{
	Word Skip;
	if (x<(int)ScreenClipRight && y<(int)ScreenClipBottom) {	/* Clip off right or bottom? */
		if (y<(int)ScreenClipTop) {		/* Clip the top */
			Skip = ScreenClipTop-y;
			Height = Height-Skip;		/* Remove from the shape */
			if ((int)Height<=0) {		/* Clipped off the top? */
				return;
			}
			ShapePtr = ((Word8 *)ShapePtr)+((Width+Skip2)*Skip);	/* Adjust the pointer */
			y = ScreenClipTop;		/* New top Y */
		}

		if (((Word)y+Height)>ScreenClipBottom) {
			Height = ScreenClipBottom-y;		/* Clip the bottom */
		}

		Skip = 0;		/* Assume I don't skip any bytes */

		if (x<(int)ScreenClipLeft) {
			Skip = ScreenClipLeft-x;
			Width = Width-Skip;
			if ((int)Width<=0) {		/* Clipped off the left? */
				return;
			}
			ShapePtr = ((Word8 *)ShapePtr)+Skip;	/* Adjust the source shape */
			x = ScreenClipLeft;		/* New left X */
		}

		if (((Word)x+Width)>ScreenClipRight) {	/* Clip the right? */
			Word Temp;
			Temp = ScreenClipRight-x;		/* Get the NEW width amount */
			Skip = (Width-Temp)+Skip;	/* Add to the line skip */
			Width = Temp;		/* Save new width */
		}
		DrawMShapeLowLevel(x,y,Width,Height,Skip+Skip2,ShapePtr);
	}
}

/**********************************

	Crop the region of pixels to draw with the visible screen.
	Draw only the area that is visible.

**********************************/

void BURGERCALL DrawMShapeLowLevelClipped16(int x,int y,Word Width,Word Height,Word Skip2,void *ShapePtr)
{
	Word Skip;
	if (x<(int)ScreenClipRight && y<(int)ScreenClipBottom) {	/* Clip off right or bottom? */
		if (y<(int)ScreenClipTop) {		/* Clip the top */
			Skip = ScreenClipTop-y;
			Height = Height-Skip;		/* Remove from the shape */
			if ((int)Height<=0) {		/* Clipped off the top? */
				return;
			}
			ShapePtr = ((Word8 *)ShapePtr)+(((Width<<1)+Skip2)*Skip);	/* Adjust the pointer */
			y = ScreenClipTop;		/* New top Y */
		}

		if (((Word)y+Height)>ScreenClipBottom) {
			Height = ScreenClipBottom-y;		/* Clip the bottom */
		}

		Skip = 0;		/* Assume I don't skip any bytes */

		if (x<(int)ScreenClipLeft) {
			Skip = ScreenClipLeft-x;
			Width = Width-Skip;
			if ((int)Width<=0) {		/* Clipped off the left? */
				return;
			}
			Skip <<= 1;	/* Skip this many each line for left clipping */
			ShapePtr = ((Word8 *)ShapePtr)+Skip;	/* Adjust the source shape */
			x = ScreenClipLeft;		/* New left X */
		}

		if (((Word)x+Width)>ScreenClipRight) {	/* Clip the right? */
			Word Temp;
			Temp = ScreenClipRight-x;		/* Get the NEW width amount */
			Skip = ((Width-Temp)<<1)+Skip;	/* Add to the line skip */
			Width = Temp;		/* Save new width */
		}
		DrawMShapeLowLevel16(x,y,Width,Height,Skip+Skip2,ShapePtr);
	}
}

/**********************************

	Draw a masked shape to the screen quickly!

**********************************/

void BURGERCALL DrawMShapeLowLevelTo16(Word x,Word y,Word Width,Word Height,Word Skip,void *ShapePtr)
{
	Word16 *Screenad;		/* Pointer to screen memory */
	Word i;				/* Temp */
	Word32 Offset;		/* VideoWidth-Width */

	if (Height && Width) {	/* Invalid shape? */
		Word *Table;
		Screenad = (Word16 *)(&VideoPointer[(VideoWidth*y)+x*2]);		/* Get base address */
		Offset = VideoWidth-(Width*2);		/* Precalc the offset */
		Table = Table8To16Ptr;
		do {
			i = Width;
			do {
				Word Temp;
				Temp = ((Word8 *)ShapePtr)[0];
				if (Temp) {
					Screenad[0] = (Word16)Table[Temp];	/* 8 bit to 16 bit conversion table */
				}
				++Screenad;
				ShapePtr = ((Word8 *)ShapePtr)+1;
			} while (--i);
			Screenad = (Word16 *)(&((Word8 *)Screenad)[Offset]);
			ShapePtr = ((Word8 *)ShapePtr)+Skip;
		} while (--Height);
	}
}

/**********************************

	Draw a shape using a resource number
	and center it on the screen.

**********************************/

void BURGERCALL DrawRezCenterMShape(RezHeader_t *Input,Word RezNum)
{
	LWShape_t *ShapePtr;
	Word x,y;

	ShapePtr = ResourceLoadShape(Input,RezNum);		/* Load the resource */
	if (ShapePtr) {
		x = (ScreenWidth-GetShapeWidth(ShapePtr))/2;	/* Center X */
		y = (ScreenHeight-GetShapeHeight(ShapePtr))/2;	/* Center Y */
		DrawMShape(x,y,ShapePtr);		/* Draw it */
		ResourceRelease(Input,RezNum);		/* Release it */
	}
}

/**********************************

	Draw a shape using a resource number
	and center it on the screen.

**********************************/

void BURGERCALL DrawRezCenterShape(RezHeader_t *Input,Word RezNum)
{
	LWShape_t *ShapePtr;
	Word x,y;

	ShapePtr = ResourceLoadShape(Input,RezNum);		/* Load the resource */
	if (ShapePtr) {
		x = (ScreenWidth-GetShapeWidth(ShapePtr))/2;	/* Center X */
		y = (ScreenHeight-GetShapeHeight(ShapePtr))/2;	/* Center Y */
		DrawShape(x,y,ShapePtr);		/* Draw it */
		ResourceRelease(Input,RezNum);		/* Release it */
	}
}

/**********************************

	Draw a shape using a resource number

**********************************/

void BURGERCALL DrawRezMShape(Word x,Word y,RezHeader_t *Input,Word RezNum)
{
	void *ShapePtr;
	ShapePtr = ResourceLoadShape(Input,RezNum);
	if (ShapePtr) {
		DrawMShape(x,y,ShapePtr);	/* Load and draw a shape */
		ResourceRelease(Input,RezNum);		/* Release the resource */
	}
}

/**********************************

	Draw a shape using a resource number

**********************************/

void BURGERCALL DrawRezShape(Word x,Word y,RezHeader_t *Input,Word RezNum)
{
	LWShape_t *ShapePtr;

	ShapePtr = ResourceLoadShape(Input,RezNum);
	if (ShapePtr) {
		DrawShape(x,y,ShapePtr);	/* Load and draw a shape */
		ResourceRelease(Input,RezNum);		/* Release the resource */
	}
}

/**********************************

	Draw a shape to the screen quickly!

**********************************/

//#if !defined(__INTEL__)	|| defined(__MWERKS__) || defined(__BEOS__)	/* Assembly for Intel */

void BURGERCALL DrawShapeLowLevel(Word x,Word y,Word Width,Word Height,Word Skip,void *ShapePtr)
{
	Word8 *Screenad;		/* Pointer to screen memory */
	Word i;				/* Temp */
	Word32 Offset;		/* VideoWidth-Width */
	Word LongWidth;

	if (!Height || !Width) {	/* Invalid shape? */
		return;
	}
	Screenad = &VideoPointer[(VideoWidth*y)+x];		/* Get base address */
	Offset = VideoWidth-Width;		/* Precalc the offset */
	LongWidth = Width>>2;		/* Count in longs */
	if (!LongWidth) {
		goto Byte1to3;
	}
	Width &= 3;		/* Get count of byte fragments to copy */
	if (Width) {
		goto LongAndByte;
	}

	/* I only copy 8 byte data! */

	do {
		i = LongWidth;
		do {
			Word32 Temp;
			Temp = ((Word32 *)ShapePtr)[0];
			((Word32 *)Screenad)[0] = Temp;
			Screenad+=4;
			ShapePtr = ((Word8 *)ShapePtr)+4;
		} while (--i);
		Screenad += Offset;
		ShapePtr = ((Word8 *)ShapePtr)+Skip;
	} while (--Height);
	return;

	/* Copy longwords and bytes */

LongAndByte:;
	do {
		i = LongWidth;
		do {
			Word32 Temp;
			Temp = ((Word32 *)ShapePtr)[0];
			((Word32 *)Screenad)[0] = Temp;
			Screenad+=4;
			ShapePtr = ((Word8 *)ShapePtr)+4;
		} while (--i);
		i = Width;
		do {
			((Word8 *)Screenad)[0] = ((Word8 *)ShapePtr)[0];
			Screenad+=1;
			ShapePtr = ((Word8 *)ShapePtr)+1;
		} while (--i);
		Screenad += Offset;
		ShapePtr = ((Word8 *)ShapePtr)+Skip;
	} while (--Height);
	return;

	/* Blit 1,2 or 3 byte wide shapes */

Byte1to3:
	Skip+=Width;		/* Convert skip to true shape width */
	Offset += Width;	/* Offset = VideoWidth */
	switch (Width) {	/* I can only have 3 cases, 1 byte wide, 2 or 3 */
	default:
		do {
			Screenad[0] = ((Word8 *)ShapePtr)[0];	/* Copy a single byte */
			Screenad += Offset;
			ShapePtr = ((Word8 *)ShapePtr)+Skip;
		} while (--Height);		/* All done? */
		return;
	case 2:
		do {
			((Word16 *)Screenad)[0] = ((Word16 *)ShapePtr)[0];	/* Copy 2 bytes */
			Screenad += Offset;
			ShapePtr = ((Word8 *)ShapePtr)+Skip;
		} while (--Height);
		return;
	case 3:		/* 3 */
		do {
			((Word16 *)Screenad)[0] = ((Word16 *)ShapePtr)[0];	/* Copy 3 bytes */
			((Word8 *)Screenad)[2] = ((Word8 *)ShapePtr)[2];
			Screenad += Offset;
			ShapePtr = ((Word8 *)ShapePtr)+Skip;
		} while (--Height);
	}
}

//#endif

/**********************************

	Clip the shape to the visible screen.
	Draw only a shape.

**********************************/

void BURGERCALL DrawShapeLowLevelClippedTo16(int x,int y,Word Width,Word Height,Word Skip2,void *ShapePtr)
{
	Word Skip;
	if (x<(int)ScreenClipRight && y<(int)ScreenClipBottom) {	/* Clip off right or bottom? */
		if (y<(int)ScreenClipTop) {		/* Clip the top */
			Skip = ScreenClipTop-y;
			Height = Height-Skip;		/* Remove from the shape */
			if ((int)Height<=0) {		/* Clipped off the top? */
				return;
			}
			ShapePtr = ((Word8 *)ShapePtr)+((Width+Skip2)*Skip);	/* Adjust the pointer */
			y = ScreenClipTop;		/* New top Y */
		}

		if (((Word)y+Height)>ScreenClipBottom) {
			Height = ScreenClipBottom-y;		/* Clip the bottom */
		}

		Skip = 0;		/* Assume I don't skip any bytes */

		if (x<(int)ScreenClipLeft) {
			Skip = ScreenClipLeft-x;
			Width = Width-Skip;
			if ((int)Width<=0) {		/* Clipped off the left? */
				return;
			}
			ShapePtr = ((Word8 *)ShapePtr)+Skip;	/* Adjust the source shape */
			x = ScreenClipLeft;		/* New left X */
		}

		if (((Word)x+Width)>ScreenClipRight) {	/* Clip the right? */
			Word Temp;
			Temp = ScreenClipRight-x;		/* Get the NEW width amount */
			Skip = (Width-Temp)+Skip;	/* Add to the line skip */
			Width = Temp;		/* Save new width */
		}
		DrawShapeLowLevelTo16(x,y,Width,Height,Skip+Skip2,ShapePtr);
	}
}

/**********************************

	Crop the region of pixels to draw with the visible screen.
	Draw only the area that is visible.

**********************************/

void BURGERCALL DrawShapeLowLevelClipped(int x,int y,Word Width,Word Height,Word Skip2,void *ShapePtr)
{
	Word Skip;
	if (x<(int)ScreenClipRight && y<(int)ScreenClipBottom) {	/* Clip off right or bottom? */
		if (y<(int)ScreenClipTop) {		/* Clip the top */
			Skip = ScreenClipTop-y;
			Height = Height-Skip;		/* Remove from the shape */
			if ((int)Height<=0) {		/* Clipped off the top? */
				return;
			}
			ShapePtr = ((Word8 *)ShapePtr)+((Width+Skip2)*Skip);	/* Adjust the pointer */
			y = ScreenClipTop;		/* New top Y */
		}

		if (((Word)y+Height)>ScreenClipBottom) {
			Height = ScreenClipBottom-y;		/* Clip the bottom */
		}

		Skip = 0;		/* Assume I don't skip any bytes */

		if (x<(int)ScreenClipLeft) {
			Skip = ScreenClipLeft-x;
			Width = Width-Skip;
			if ((int)Width<=0) {		/* Clipped off the left? */
				return;
			}
			ShapePtr = ((Word8 *)ShapePtr)+Skip;	/* Adjust the source shape */
			x = ScreenClipLeft;		/* New left X */
		}

		if (((Word)x+Width)>ScreenClipRight) {	/* Clip the right? */
			Word Temp;
			Temp = ScreenClipRight-x;		/* Get the NEW width amount */
			Skip = (Width-Temp)+Skip;	/* Add to the line skip */
			Width = Temp;		/* Save new width */
		}
		DrawShapeLowLevel(x,y,Width,Height,Skip+Skip2,ShapePtr);
	}
}

/**********************************

	Crop the region of pixels to draw with the visible screen.
	Draw only the area that is visible.

**********************************/

void BURGERCALL DrawShapeLowLevelClipped16(int x,int y,Word Width,Word Height,Word Skip2,void *ShapePtr)
{
	Word Skip;
	if (x<(int)ScreenClipRight && y<(int)ScreenClipBottom) {	/* Clip off right or bottom? */
		if (y<(int)ScreenClipTop) {		/* Clip the top */
			Skip = ScreenClipTop-y;
			Height = Height-Skip;		/* Remove from the shape */
			if ((int)Height<=0) {		/* Clipped off the top? */
				return;
			}
			ShapePtr = ((Word8 *)ShapePtr)+(((Width<<1)+Skip2)*Skip);	/* Adjust the pointer */
			y = ScreenClipTop;		/* New top Y */
		}

		if (((Word)y+Height)>ScreenClipBottom) {
			Height = ScreenClipBottom-y;		/* Clip the bottom */
		}

		Skip = 0;		/* Assume I don't skip any bytes */

		if (x<(int)ScreenClipLeft) {
			Skip = ScreenClipLeft-x;
			Width = Width-Skip;
			if ((int)Width<=0) {		/* Clipped off the left? */
				return;
			}
			Skip <<= 1;
			ShapePtr = ((Word8 *)ShapePtr)+Skip;	/* Adjust the source shape */
			x = ScreenClipLeft;		/* New left X */
		}

		if (((Word)x+Width)>ScreenClipRight) {	/* Clip the right? */
			Word Temp;
			Temp = ScreenClipRight-x;		/* Get the NEW width amount */
			Skip = ((Width-Temp)<<1)+Skip;	/* Add to the line skip */
			Width = Temp;		/* Save new width */
		}
		DrawShapeLowLevel16(x,y,Width,Height,Skip+Skip2,ShapePtr);
	}
}

/**********************************

	Draw a masked shape to the screen quickly!

**********************************/

void BURGERCALL DrawShapeLowLevelTo16(Word x,Word y,Word Width,Word Height,Word Skip,void *ShapePtr)
{
	Word16 *Screenad;		/* Pointer to screen memory */
	Word i;				/* Temp */
	Word32 Offset;		/* VideoWidth-Width */

	if (Height && Width) {	/* Invalid shape? */
		Word *Table;
		Screenad = (Word16 *)(&VideoPointer[(VideoWidth*y)+x*2]);		/* Get base address */
		Offset = VideoWidth-(Width*2);		/* Precalc the offset */
		Table = Table8To16Ptr;
		do {
			i = Width;
			do {
				Screenad[0] = (Word16)Table[((Word8 *)ShapePtr)[0]];	/* 8 bit to 16 bit conversion table */
				++Screenad;
				ShapePtr = ((Word8 *)ShapePtr)+1;
			} while (--i);
			Screenad = (Word16 *)(&((Word8 *)Screenad)[Offset]);
			ShapePtr = ((Word8 *)ShapePtr)+Skip;
		} while (--Height);
	}
}

/**********************************

	Clear the screen

**********************************/

void BURGERCALL ScreenClear(Word Color)
{
	ScreenRect(0,0,ScreenWidth,ScreenHeight,Color);
}

/**********************************

	Draw a frame around a rect

**********************************/

void BURGERCALL ScreenBox(int x,int y,Word Width,Word Height,Word Color)
{
	if (Width<3 || Height<3) {		/* Solid? */
		ScreenRect(x,y,Width,Height,Color);	/* It's not hollow */
	} else {
		ScreenRect(x,y,Width,1,Color);		/* Draw the top line */
		--Height;
		ScreenRect(x,y+Height,Width,1,Color);	/* Bottom line */
		--Height;
		++y;
		ScreenRect(x,y,1,Height,Color);		/* Left edge */
		ScreenRect(x+Width-1,y,1,Height,Color);	/* Right edge */
	}
}

/**********************************

	Draw a two color box for a psuedo 3D look

**********************************/

void BURGERCALL ScreenBox2(int x,int y,Word Width,Word Height,Word Color1,Word Color2)
{
	if (Width && Height) {		/* Do I bother? */
		ScreenRect(x,y,Width,1,Color1);		/* Draw the top line */
		if (Height>=2) {					/* Taller than 1 pixel? */
			--Height;
			ScreenRect(x,y+Height,Width,1,Color2);	/* Bottom line */
			if (Height>=2) {				/* Anything in the middle? */
				--Height;
				++y;
				ScreenRect(x,y,1,Height,Color1);		/* Left edge */
				ScreenRect(x+Width-1,y,1,Height,Color2);	/* Right edge */
			}
		}
	}
}

/**********************************

	Draw a box outline that is 2 pixels thick

**********************************/

void BURGERCALL ScreenThickBox(int x,int y,Word Width,Word Height,Word Color)
{
	if (Width<5 || Height<5) {		/* Solid box anyways? */
		ScreenRect(x,y,Width,Height,Color);
	} else {
		ScreenRect(x,y,Width,2,Color);		/* Draw the top line */
		Height-=2;
		ScreenRect(x,y+Height,Width,2,Color);	/* Bottom line */
		Height-=2;
		y+=2;
		ScreenRect(x,y,2,Height,Color);		/* Left edge */
		ScreenRect(x+Width-2,y,2,Height,Color);	/* Right edge */
	}
}

/**********************************

	Draw a 2 color box that is 2 pixels thick

**********************************/

void BURGERCALL ScreenThickBox2(int x,int y,Word Width,Word Height,Word Color1,Word Color2)
{
	ScreenBox2(x,y,Width,Height,Color1,Color2);	/* Draw the initial box */
	if (Width>=2 && Height>=2) {		/* Do I bother with the inner box? */
		ScreenBox2(x+1,y+1,Width-2,Height-2,Color1,Color2);
	}
}

/**********************************

	Draw a frame around a rect
	with a remap table

**********************************/

void BURGERCALL ScreenBoxRemap(int x,int y,Word Width,Word Height,const void *RemapPtr)
{
	if (Width && Height) {		/* Do I bother? */
		ScreenRectRemap(x,y,Width,1,RemapPtr);		/* Draw the top line */
		if (Height>=2) {					/* Taller than 1 pixel? */
			--Height;
			ScreenRectRemap(x,y+Height,Width,1,RemapPtr);	/* Bottom line */
			if (Height>=2) {				/* Anything in the middle? */
				--Height;
				++y;
				ScreenRectRemap(x,y,1,Height,RemapPtr);		/* Left edge */
				ScreenRectRemap(x+Width-1,y,1,Height,RemapPtr);	/* Right edge */
			}
		}
	}
}

/**********************************

	Draw an empty box and draw an offset "Drop shadow"
	Used for cute menus
	
**********************************/

void BURGERCALL ScreenBoxDropShadow(int x,int y,Word Width,Word Height,Word Color1,Word Color2)
{
	ScreenBox(x,y,Width,Height,Color1);				/* Draw the base box */
	if (Width>=3) {									/* Worth my time? */
		ScreenRect(x+2,y+Height,Width-1,1,Color2);	/* Draw the shadow on the bottom */
	}
	if (Height>=3) {								/* Worth my time? */
		ScreenRect(x+Width,y+2,1,Height-1,Color2);	/* Draw the shadow on the right */
	}
}

/**********************************

	Draw an empty box and draw an offset "Drop shadow"
	Used for cute menus and fill the background with a solid color
	
**********************************/

void BURGERCALL ScreenRectDropShadow(int x,int y,Word Width,Word Height,Word Color1,Word Color2,Word Color3)
{
	ScreenBox(x,y,Width,Height,Color1);				/* Draw the base box */
	if (Width>=3) {									/* Worth my time? */
		ScreenRect(x+2,y+Height,Width-1,1,Color2);	/* Draw the shadow on the bottom */
	}
	if (Height>=3) {								/* Worth my time? */
		ScreenRect(x+Width,y+2,1,Height-1,Color2);	/* Draw the shadow on the right */
	}
	if (Width>=3 && Height>=3) {					/* Anything in the center? */
		ScreenRect(x+1,y+1,Width-2,Height-2,Color3);
	}
}

/**********************************

	Erase a shape quickly. Erasing entails drawing the
	shape's area in color #0 (Black)

**********************************/

#if !defined(__WATCOMC__)
void EraseShape(Word x,Word y,void *ShapePtr)
{
	register Word8 *Screenad;	/* Pointer to screen memory */
	register Word16 Width;		/* Width of the shape in bytes */
	register Word16 Height;		/* Height of the shape in pixels */
	register Word i;			/* Temp */
	register Word32 Offset;	/* VideoWidth-Width */
	register Word8 Zero;

	Screenad = &VideoPointer[(VideoWidth*y)+x];		/* Get base address */
	Width = ((LWShape_t *)ShapePtr)->Width;	/* Get the shape's dimensions */
	Height = ((LWShape_t *)ShapePtr)->Height;
	if (Width && Height) {		/* Must be valid! */
		Offset = VideoWidth-Width;		/* Precalc the offset */
		Zero = 0;				/* Cache the value to store */
		do {
			i = Width;			/* Reset the width count */
			do {
				*Screenad++ = Zero;	/* Copy a byte */
			} while (--i);		/* All done? */
			Screenad+=Offset;	/* Adjust the dest pointer */
		} while (--Height);		/* Totally done? */
	}
}
#endif

/**********************************

	Erase a masked shape quickly. Erasing entails drawing the
	shape's area using data from virgin background instead
	of the actual shape data

**********************************/

void EraseMBShape(Word x,Word y, void *ShapePtr,void *BackPtr)
{
	register Word8 *Screenad;		/* Pointer to screen memory */
	register Word16 Width;			/* Width of the shape in bytes */
	register Word16 Height;			/* Height of the shape in pixels */
	register Word i;				/* Temp */
	register Word32 Offset;		/* VideoWidth-Width */
	register Word32 Offset2;		/* ScreenWidth-Width */

	Screenad = &VideoPointer[(VideoWidth*y)+x];			/* Get base address */
	BackPtr = &((Word8 *)BackPtr)[(ScreenWidth*y)+x];	/* Address to the background */
	Width = ((LWShape_t *)ShapePtr)->Width;		/* Get the shape's dimensions */
	Height = ((LWShape_t *)ShapePtr)->Height;
	if (Width && Height) {			/* Must be valid! */
		Offset = VideoWidth-Width;		/* Precalc the offset */
		Offset2 = ScreenWidth-Width;
		ShapePtr = &((LWShape_t *)ShapePtr)->Data[0];
		do {
			i = Width;		/* Reset the width count */
			do {
				register Word8 b;		/* Force the compiler to use BYTE addressing */
				b = ((Word8 *)ShapePtr)[0];		/* Fetch screen data */
				if (!b) {
					Screenad[0] = ((Word8 *)BackPtr)[0];		/* Get the mask */
				}
				ShapePtr = (Word8 *)ShapePtr+1;
				++Screenad;
				BackPtr = ((Word8 *)BackPtr)+1;
			} while (--i);			/* All done? */
			Screenad+=Offset;		/* Adjust the dest pointer */
			BackPtr=((Word8 *)BackPtr)+Offset2;
		} while (--Height);			/* Totally done? */
	}
}

/**********************************

	Test for a shape collision

**********************************/

Word TestMShape(Word x,Word y,void *ShapePtr)
{
	Word8 *ScreenPtr;
	Word8 *Screenad;
	Word Width;
	Word Height;
	Word Width2;

	Width = ((LWShape_t *)ShapePtr)->Width;
	Height = ((LWShape_t *)ShapePtr)->Height;
	if (!Width || !Height) {			/* Failsafe */
		return FALSE;
	}
	ShapePtr = &((LWShape_t *)ShapePtr)->Data[0];
	ScreenPtr = (Word8 *) &VideoPointer[(VideoWidth*y)+x];
	do {
		Width2 = Width;
		Screenad = ScreenPtr;
		do {
			if (((Word8 *)ShapePtr)[0]) {
				if (Screenad[0] != ((Word8 *)ShapePtr)[0]) {
					return TRUE;		/* They touched! */
				}
			}
			++Screenad;
			ShapePtr = ((Word8 *)ShapePtr)+1;
		} while (--Width2);
		ScreenPtr+=VideoWidth;
	} while (--Height);
	return FALSE;
}

/**********************************

	Test for a masked shape collision

**********************************/

Word TestMBShape(Word x,Word y,void *ShapePtr,void *BackPtr)
{
	Word8 *ScreenPtr;
	Word8 *Screenad;
	Word8 *Backad;
	Word Width;
	Word Height;
	Word Width2;

	Width = ((LWShape_t *)ShapePtr)->Width;		/* Get the width of the shape */
	Height = ((LWShape_t *)ShapePtr)->Height;		/* Get the height of the shape */
	ShapePtr = &((LWShape_t *)ShapePtr)->Data[0];	/* Point to the screen */
	ScreenPtr = (Word8 *) &VideoPointer[(VideoWidth*y)+x];
	BackPtr = &((Word8 *)BackPtr)[(y*ScreenWidth)+x];		/* Index to the erase buffer */
	do {
		Width2 = Width;			/* Init width count */
		Screenad = ScreenPtr;
		Backad = (Word8 *)BackPtr;
		do {
			if (((Word8 *)ShapePtr)[0]) {
				if (*Screenad != *Backad) {
					return TRUE;
				}
			}
			ShapePtr = (Word8 *)ShapePtr+1;
			++Screenad;
			++Backad;
		} while (--Width2);
		ScreenPtr +=VideoWidth;
		BackPtr = ((Word8 *)BackPtr) + ScreenWidth;
	} while (--Height);
	return FALSE;
}

/**********************************

	Pixel double routine for 16 bit graphics

**********************************/

#if !defined(__POWERPC__) || defined(__MACOSX__)
/* Assembly in PowerPC */
void BURGERCALL VideoPixelDouble16(const Word8 *SourcePtr,Word8 *DestPtr,Word SourceRowBytes,
	Word DestRowBytes,Word Width,Word Height)
{
	Word h;
	Word8 *DestPtr2;

	DestPtr2 = DestPtr+DestRowBytes;				/* Pointer to the line directly below */
	DestRowBytes = (DestRowBytes*2)-(Width*4);	/* Offset to the next line */
	SourceRowBytes = SourceRowBytes-(Width*2);		/* Offset to the next line */
	Width = Width>>1;
	do {
		h = Width;				/* Number of pixels to process */
		do {
			Word32 Temp,Temp2;

			Temp = ((Word32 *)SourcePtr)[0];		/* Fetch 2 pixels */
			Temp2 = Temp&0xFFFF;
			Temp = Temp&0xFFFF0000;
			Temp = Temp|(Temp>>16);
			Temp2 = Temp2|(Temp2<<16);
#if defined(__BIGENDIAN__)
			((Word32 *)DestPtr)[0] = Temp;
			((Word32 *)DestPtr)[1] = Temp2;
			((Word32 *)DestPtr2)[0] = Temp;
			((Word32 *)DestPtr2)[1] = Temp2;
#else
			((Word32 *)DestPtr)[0] = Temp2;
			((Word32 *)DestPtr)[1] = Temp;
			((Word32 *)DestPtr2)[0] = Temp2;
			((Word32 *)DestPtr2)[1] = Temp;
#endif
			SourcePtr+=4;
			DestPtr+=8;
			DestPtr2+=8;
		} while (--h);
		SourcePtr+=SourceRowBytes;
		DestPtr+=DestRowBytes;
		DestPtr2+=DestRowBytes;
	} while (--Height);
}
#endif

/**********************************

	Pixel double routine for 16 bit graphics
	Algorithm #2, checkerboard pattern

**********************************/

#if !defined(__POWERPC__) || defined(__MACOSX__)
void BURGERCALL VideoPixelDoubleChecker16(const Word8 *SourcePtr,Word8 *DestPtr,Word SourceRowBytes,
	Word DestRowBytes,Word Width,Word Height)
{
	Word h;
	Word8 *DestPtr2;

	DestPtr2 = DestPtr+DestRowBytes;				/* Pointer to the line directly below */
	DestRowBytes = (DestRowBytes*2)-(Width*4);	/* Offset to the next line */
	SourceRowBytes = SourceRowBytes-(Width*2);		/* Offset to the next line */
	Width = Width>>1;
	do {
		h = Width;				/* Number of pixels to process */
		do {
			Word32 Temp;

			Temp = ((Word32 *)SourcePtr)[0];		/* Fetch 2 pixels */
#if defined(__BIGENDIAN__)	
			((Word32 *)DestPtr)[0] = Temp&0xFFFF0000;
			((Word32 *)DestPtr)[1] = Temp<<16;
			((Word32 *)DestPtr2)[0] = Temp>>16;
			((Word32 *)DestPtr2)[1] = Temp&0xFFFF;
#else
			((Word32 *)DestPtr)[0] = Temp&0xFFFF;
			((Word32 *)DestPtr)[1] = Temp>>16;
			((Word32 *)DestPtr2)[0] = Temp<<16;
			((Word32 *)DestPtr2)[1] = Temp&0xFFFF0000;
#endif
			SourcePtr+=4;
			DestPtr+=8;
			DestPtr2+=8;
		} while (--h);
		SourcePtr+=SourceRowBytes;
		DestPtr+=DestRowBytes;
		DestPtr2+=DestRowBytes;
	} while (--Height);
}
#endif

/**********************************

	Pixel double routine for 16 bit graphics

**********************************/

/* Assembly in PowerPC */
void BURGERCALL VideoPixelDouble(const Word8 *SourcePtr,Word8 *DestPtr,Word SourceRowBytes,
	Word DestRowBytes,Word Width,Word Height)
{
	Word h;
	Word8 *DestPtr2;

	DestPtr2 = DestPtr+DestRowBytes;				/* Pointer to the line directly below */
	DestRowBytes = (DestRowBytes*2)-(Width*2);		/* Offset to the next line */
	SourceRowBytes = SourceRowBytes-Width;			/* Offset to the next line */
	Width = Width>>2;
	do {
		h = Width;				/* Number of pixels to process */
		do {
			Word32 Temp,Temp2;

			Temp = ((Word32 *)SourcePtr)[0];		/* Fetch 4 pixels */
			Temp2 = ((Temp<<16)&0xFF000000)|((Temp<<8)&0xFF00);
			Temp = (Temp&0xFF000000)|((Temp>>8)&0xFF00);
			Temp = Temp|(Temp>>8);
			Temp2 = Temp2|(Temp2>>8);
#if defined(__BIGENDIAN__)
			((Word32 *)DestPtr)[0] = Temp;
			((Word32 *)DestPtr)[1] = Temp2;
			((Word32 *)DestPtr2)[0] = Temp;
			((Word32 *)DestPtr2)[1] = Temp2;
#else
			((Word32 *)DestPtr)[0] = Temp2;
			((Word32 *)DestPtr)[1] = Temp;
			((Word32 *)DestPtr2)[0] = Temp2;
			((Word32 *)DestPtr2)[1] = Temp;
#endif
			SourcePtr+=4;
			DestPtr+=8;
			DestPtr2+=8;
		} while (--h);
		SourcePtr+=SourceRowBytes;
		DestPtr+=DestRowBytes;
		DestPtr2+=DestRowBytes;
	} while (--Height);
}

/**********************************

	Pixel double routine for 16 bit graphics
	Algorithm #2, checkerboard pattern

**********************************/

void BURGERCALL VideoPixelDoubleChecker(const Word8 *SourcePtr,Word8 *DestPtr,Word SourceRowBytes,
	Word DestRowBytes,Word Width,Word Height)
{
	Word h;
	Word8 *DestPtr2;

	DestPtr2 = DestPtr+DestRowBytes;				/* Pointer to the line directly below */
	DestRowBytes = (DestRowBytes*2)-(Width*2);	/* Offset to the next line */
	SourceRowBytes = SourceRowBytes-Width;		/* Offset to the next line */
	Width = Width>>2;
	do {
		h = Width;				/* Number of pixels to process */
		do {
			Word32 Temp;

			Temp = ((Word32 *)SourcePtr)[0];		/* Fetch 2 pixels */
#if defined(__BIGENDIAN__)	
			((Word32 *)DestPtr)[0] = Temp&0xFFFF0000;
			((Word32 *)DestPtr)[1] = Temp<<16;
			((Word32 *)DestPtr2)[0] = Temp>>16;
			((Word32 *)DestPtr2)[1] = Temp&0xFFFF;
#else
			((Word32 *)DestPtr)[0] = Temp&0xFFFF;
			((Word32 *)DestPtr)[1] = Temp>>16;
			((Word32 *)DestPtr2)[0] = Temp<<16;
			((Word32 *)DestPtr2)[1] = Temp&0xFFFF0000;
#endif
			SourcePtr+=4;
			DestPtr+=8;
			DestPtr2+=8;
		} while (--h);
		SourcePtr+=SourceRowBytes;
		DestPtr+=DestRowBytes;
		DestPtr2+=DestRowBytes;
	} while (--Height);
}

#if !defined(__MSDOS__) && !defined(__WIN32__) && !defined(__MAC__) && !defined(__MACOSX__)

#include "MmMemory.h"

/**********************************

	Set up the current video page
	for drawing, lock down the pixels
	and set the burgerlib video port

**********************************/

void BURGERCALL LockFrontVideoPage(void)
{
}

/**********************************

	Set up the current video page
	for drawing, lock down the pixels
	and set the burgerlib video port

**********************************/

void BURGERCALL LockVideoPage(void)
{
}

/**********************************

	Release the use of the video memory

**********************************/

#undef UnlockVideoPage
extern void UnlockVideoPage(void);

void BURGERCALL UnlockVideoPage(void)		/* Not needed */
{
}

/**********************************

	Take the previous screen and display it to the current
	video display hardware and now draw directly
	to the video display hardware

**********************************/

void BURGERCALL UpdateAndNoPageFlip(void)
{
}

/**********************************

	Take the previous screen and display it to the current
	video display hardware and now draw to a hidden
	page for page flipping

**********************************/

void UpdateAndPageFlip(void)
{
}

/**********************************

	Release everything about the graphics system
	and go back to the ORIGINAL video mode

**********************************/

void BURGERCALL ReleaseVideo(void)
{
	DeallocAPointer(VideoOffscreen);	/* Release it */
	VideoOffscreen = 0;	/* Make sure it's gone */
	ScreenWidth = 0;		/* Force default */
	ScreenHeight = 0;
}


/**********************************

	Initialize a video mode and prepare for
	use of Burgerlib

**********************************/

Word BURGERCALL SetDisplayToSize(Word Width,Word Height,Word Depth,Word Flags)
{
	return TRUE;
}

/**********************************

	If a window is present, set the text to a specific string

**********************************/

void BURGERCALL VideoSetWindowString(const char *Title)
{
}

/**********************************

	Return a handle to an array of video display modes
	Pass in the device number to check
	Note : I can add other modes that I can fake in software

**********************************/

VideoModeArray_t ** BURGERCALL VideoModeArrayNew(Word HardwareOnly)
{
	return 0;
}

/**********************************

	Return a list of all the device modes available

**********************************/


VideoDeviceArray_t ** BURGERCALL VideoDeviceArrayNew(Word HardwareOnly)
{
	return 0;
}

#endif

/**********************************

	Dispose of a VideoDeviceArray_t handle

**********************************/

void BURGERCALL VideoDeviceArrayDelete(VideoDeviceArray_t **Input)
{
	if (Input) {		/* Is the handle even valid? */
		Word i;
		i = Input[0]->Count;		/* Get the entry count */
		if (i) {					/* Valid? */
			Word j;
			j = 0;
			do {
				VideoModeArrayDelete(Input[0]->Array[j]);	/* Delete */
			} while (++j<i);
		}
		DeallocAHandle((void **)Input);		/* Dispose of the main handle */
	}
}

/**********************************

	Traverse the available modes and remove any that
	a supplied function deems unworthy

**********************************/

VideoModeArray_t ** BURGERCALL VideoModeArrayPurge(VideoModeArray_t **Input,VideoModePurgeProc Proc)
{
	Word Count,j,i;
	if (Input) {
		Count = Input[0]->Count;
		if (Count) {
			i = 0;
			j = 0;
			do {
				if (!Proc(&Input[0]->Array[i])) {
					Input[0]->Array[j] = Input[0]->Array[i];
					++j;
				}
			} while (++i<Count);
			Input[0]->Count = j;
			if (j) {
				if (j!=Count) {
					return (VideoModeArray_t **)ResizeAHandle((void **)Input,(sizeof(Word)*2)+64+(sizeof(VideoMode_t)*j));
				}
				return Input;
			}
		}
		DeallocAHandle((void **)Input);
	}
	return 0;
}

/**********************************

	Traverse the available modes and remove any that
	a supplied function deems unworthy

**********************************/

VideoDeviceArray_t ** BURGERCALL VideoDeviceArrayPurge(VideoDeviceArray_t **Input,VideoModePurgeProc Proc)
{
	Word i,j,Count;
	if (Input) {		/* Is the input even valid? */
		Count = Input[0]->Count;		/* Number of entries */
		if (Count) {
			i = 0;
			j = 0;
			do {
				VideoModeArray_t **TempHand;
				TempHand = VideoModeArrayPurge(Input[0]->Array[i],Proc);
				if (TempHand) {
					Input[0]->Array[j] = TempHand;
					++j;
				}
			} while (++i<Count);
			Input[0]->Count = j;		/* New and improved count */
			if (j) {					/* Any entries left? */
				if (j!=Count) {			/* Did it change? */
					return (VideoDeviceArray_t **)ResizeAHandle((void **)Input,sizeof(Word)+(sizeof(VideoMode_t **)*j));
				}
				return Input;
			}
		}
		DeallocAHandle((void **)Input);		/* Discard the handle */
	}
	return 0;
}

/**********************************

	Convert from 5:5:5 to 5:6:5 format.
	For accuracies sake, I will shift the 5
	bits of green into 6 bit by using this formula

	G = (G<<1)+(G&1);

	This way, 0x1f becomes 0x3F for full intensity instead
	of 0x3E if I used a simple shift.
	The extra cycle is negligable since the real bottleneck
	is shoving data onto the PCI bus, this is handled by
	combining the writes with a U/V paired write to do
	a 64 bit write using two integer registers on the
	intel platform.

	The routine assumes VideoWidth and VideoPointer are
	pointing to true hardware page and VideoOffscreen points
	to the offscreen buffer

**********************************/

#if !defined(__INTEL__) || defined(__MWERKS__)

void BURGERCALL Video555To565(void)
{
	Word i;
	Word j;
	Word32 *SrcPtr;
	Word32 *DestPtr;

	SrcPtr = (Word32 *)VideoOffscreen;
	DestPtr = (Word32 *)VideoPointer;
	j = ScreenHeight;
	do {
		Word8 *TempPtr;
		i = ScreenWidth/4;
		TempPtr = (Word8*)DestPtr;
		do {
			Word32 a,b,c,d;
			a = SrcPtr[0];
			b = SrcPtr[1];
			c = a+a;
			d = b+b;
			a = a&0x3f003F;
			b = b&0x3f003F;
			c = c&0xFFC0FFC0;
			d = d&0xFFC0FFC0;
			a = a+c;
			b = b+d;
			DestPtr[0] = a;
			DestPtr[1] = b;
			SrcPtr+=2;
			DestPtr+=2;
		} while (--i);
		DestPtr = (Word32 *)(TempPtr+VideoWidth);
	} while (--j);
}
#endif

/**********************************

	OpenGL convience routines

**********************************/

#if !defined(__WIN32__) && !(defined(__MAC__) && defined(__POWERPC__)) && !defined(__MACOSX__)

/**********************************

	Initialize the OpenGL context and set the video display
	to a specific size

**********************************/

Word BURGERCALL OpenGLSetDisplayToSize(Word /* Width */,Word /* Height */,Word /* Depth */,Word /* Flags */)
{
	return TRUE;			/* Always return an error */
}

/**********************************

	Release OpenGL is initialized

**********************************/

void BURGERCALL OpenGLRelease(void)
{
}

/**********************************

	Perform a page flip in OpenGL

**********************************/

void BURGERCALL OpenGLSwapBuffers(void)
{
}

void BURGERCALL OpenGLATISetTruform(Word /* Setting */)
{
}

void BURGERCALL OpenGLATISetFSAA(Word /* Setting */)
{
}

#endif

#if !defined(__WIN32__) && !defined(__MAC__)  && !defined(__MACOSX__)

/**********************************

	Init the gamma manager

**********************************/

void BURGERCALL VideoOSGammaInit(void)
{
}

/**********************************

	Shut down the gamma manager

**********************************/

void BURGERCALL VideoOSGammaDestroy(void)
{
}

/**********************************

	Darken or brighten the gamma using (Fixed32) 1.0 as the base intensity

**********************************/

void BURGERCALL VideoOSGammaAdjust(Fixed32 /* Intensity */)
{
}

/**********************************

	Set the gamma based on specific gamma entries

**********************************/

void BURGERCALL VideoOSGammaSet(const GammaTable_t * /* TablePtr */)
{
}

/**********************************

	Return the current gamma table last uploaded to the video card.

**********************************/

void BURGERCALL VideoOSGammaGet(GammaTable_t *TablePtr)
{
	FastMemSet(TablePtr,0,sizeof(GammaTable_t));
}

#endif