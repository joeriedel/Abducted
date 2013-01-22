/**********************************

	Generic pixel blitter

**********************************/

#include "SsScreenShape.h"
#include <BREndian.hpp>
#include "LrRect.h"
#include "MmMemory.h"
#include "PlPalette.h"
#include "RzRez.h"
#include "FmFile.h"
#include "ImImage.h"
#include "StString.h"
#include "ClStdLib.h"

#if defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN
#define GLUT_BUILDING_LIB
#define DIRECTDRAW_VERSION 0x700
#define DIRECT3D_VERSION 0x700
#include <windows.h>
#include "../Win95Extern/DirectX/d3d.h"
#endif

#if defined(__WIN32__) || defined(__MAC__)
#include "FpFloat.h"
#include <BRGlut.h>
#include <BRGL.h>
#define OPENGLPRESENT
#endif

#if defined(__WIN32__)
#include "W9Win95.h"
#endif

#ifdef __cplusplus
extern "C" {
static void BURGERCALL SoftSolidRect(int x,int y,Word Width,Word Height,Word32 Color);
static void BURGERCALL SoftInit(void);
}
#endif

static Word ScreenShapeBytesPerPixel;		/* Bytes per pixel */

/**********************************

	Direct blitter for drawing a scaled shape

**********************************/

static void BURGERCALL DrawShapeLowLevelScaled16(Word x,Word y,Word ScaleWidth,Word ScaleHeight,Word ImageWidth,
	Fixed32 HStep,Fixed32 HDelta,Fixed32 WStep,Fixed32 WDelta,void *ShapePtr)
{
	Word16 *DestPtr;
	Word DestSkip;

	if (ScaleWidth && ScaleHeight) {			/* Good to go? */
		DestPtr = (Word16 *)(&VideoPointer[(VideoWidth*y)]+x*2);		/* Pointer to the screen */
		DestSkip = VideoWidth - ScaleWidth*2;					/* Skip per line */
		do {
			Word TempWidth;
			Word16 *Source;
			Fixed32 WDeltaTemp;

			Source = (Word16 *)(((Word8 *)ShapePtr)+((HDelta>>16)*ImageWidth));	/* Init the Y index */
			TempWidth = ScaleWidth;
			WDeltaTemp = WDelta;			/* Init the X offset */
			do {
				DestPtr[0] = Source[(WDeltaTemp>>16)];		/* Get a source pixel */
				++DestPtr;				/* Next screen pixel */
				WDeltaTemp+=WStep;		/* Step the X */
			} while (--TempWidth);		/* All done? */
			HDelta += HStep;			/* Step the Y */
			DestPtr = (Word16 *)((Word8 *)DestPtr + DestSkip);	/* Next line down */
		} while (--ScaleHeight);
	}
}

/**********************************

	Direct blitter for drawing a scaled shape

**********************************/

static void BURGERCALL DrawShapeLowLevelScaled(Word x,Word y,Word ScaleWidth,Word ScaleHeight,Word ImageWidth,
	Fixed32 HStep,Fixed32 HDelta,Fixed32 WStep,Fixed32 WDelta,void *ShapePtr)
{
	Word8 *DestPtr;
	Word DestSkip;

	if (ScaleWidth && ScaleHeight) {			/* Good to go? */
		DestPtr = &VideoPointer[(VideoWidth*y)]+x;		/* Pointer to the screen */
		DestSkip = VideoWidth - ScaleWidth;					/* Skip per line */
		do {
			Word TempWidth;
			Word8 *Source;
			Fixed32 WDeltaTemp;

			Source = ((Word8 *)ShapePtr)+((HDelta>>16)*ImageWidth);	/* Init the Y index */
			TempWidth = ScaleWidth;
			WDeltaTemp = WDelta;			/* Init the X offset */
			do {
				DestPtr[0] = Source[(WDeltaTemp>>16)];		/* Get a source pixel */
				++DestPtr;				/* Next screen pixel */
				WDeltaTemp+=WStep;		/* Step the X */
			} while (--TempWidth);		/* All done? */
			HDelta += HStep;			/* Step the Y */
			DestPtr = DestPtr + DestSkip;	/* Next line down */
		} while (--ScaleHeight);
	}
}

/**********************************

	Direct blitter for drawing a scaled shape

**********************************/

static void BURGERCALL DrawMShapeLowLevelScaled16(Word x,Word y,Word ScaleWidth,Word ScaleHeight,Word ImageWidth,
	Fixed32 HStep,Fixed32 HDelta,Fixed32 WStep,Fixed32 WDelta,void *ShapePtr)
{
	Word16 *DestPtr;
	Word DestSkip;

	if (ScaleWidth && ScaleHeight) {			/* Good to go? */
		DestPtr = (Word16 *)(&VideoPointer[(VideoWidth*y)]+x*2);		/* Pointer to the screen */
		DestSkip = VideoWidth - ScaleWidth*2;					/* Skip per line */
		do {
			Word TempWidth;
			Word16 *Source;
			Fixed32 WDeltaTemp;

			Source = (Word16 *)(((Word8 *)ShapePtr)+((HDelta>>16)*ImageWidth));	/* Init the Y index */
			TempWidth = ScaleWidth;
			WDeltaTemp = WDelta;			/* Init the X offset */
			do {
				Word Temp;
				Temp = Source[(WDeltaTemp>>16)];		/* Get a source pixel */
				if (Temp) {
					DestPtr[0] = static_cast<Word16>(Temp);
				}
				++DestPtr;				/* Next screen pixel */
				WDeltaTemp+=WStep;		/* Step the X */
			} while (--TempWidth);		/* All done? */
			HDelta += HStep;			/* Step the Y */
			DestPtr = (Word16 *)((Word8 *)DestPtr + DestSkip);	/* Next line down */
		} while (--ScaleHeight);
	}
}

/**********************************

	Direct blitter for drawing a scaled shape

**********************************/

static void BURGERCALL DrawMShapeLowLevelScaled(Word x,Word y,Word ScaleWidth,Word ScaleHeight,Word ImageWidth,
	Fixed32 HStep,Fixed32 HDelta,Fixed32 WStep,Fixed32 WDelta,void *ShapePtr)
{
	Word8 *DestPtr;
	Word DestSkip;

	if (ScaleWidth && ScaleHeight) {			/* Good to go? */
		DestPtr = &VideoPointer[(VideoWidth*y)]+x;		/* Pointer to the screen */
		DestSkip = VideoWidth - ScaleWidth;					/* Skip per line */
		do {
			Word TempWidth;
			Word8 *Source;
			Fixed32 WDeltaTemp;

			Source = ((Word8 *)ShapePtr)+((HDelta>>16)*ImageWidth);	/* Init the Y index */
			TempWidth = ScaleWidth;
			WDeltaTemp = WDelta;			/* Init the X offset */
			do {
				Word Temp;
				Temp = Source[(WDeltaTemp>>16)];		/* Get a source pixel */
				if (Temp) {
					DestPtr[0] = static_cast<Word8>(Temp);
				}
				++DestPtr;				/* Next screen pixel */
				WDeltaTemp+=WStep;		/* Step the X */
			} while (--TempWidth);		/* All done? */
			HDelta += HStep;			/* Step the Y */
			DestPtr = DestPtr + DestSkip;	/* Next line down */
		} while (--ScaleHeight);
	}
}

/**********************************

	Clipped scaled blitter

**********************************/

static void BURGERCALL DrawShapeLowLevelScaledClipped16(const LBRect *DestRect,Word ImageWidth,Word ImageHeight,Word Skip,void *ShapePtr)
{
	Word ScaleWidth,ScaleHeight;
	Fixed32 WStep,WDelta;
	Fixed32 HStep,HDelta;
	int x,y;

	ScaleWidth = (Word)LBRectWidth(DestRect);
	ScaleHeight = (Word)LBRectHeight(DestRect);
	if (ScaleWidth && ScaleHeight) {
		WStep = (Fixed32)((ImageWidth<<16) / ScaleWidth);
		HStep = (Fixed32)((ImageHeight<<16) / ScaleHeight);
		HDelta = 0;
		WDelta = 0;
		x = DestRect->left;
		y = DestRect->top;

		if (x<0) {
			x = 0-x;
			ScaleWidth = ScaleWidth-x;
			if ((int)ScaleWidth<=0) {
				return;
			}
			WDelta = WStep*x;
			x = 0;
		}
		if (y<0) {
			y = 0-y;
			ScaleHeight = ScaleHeight-y;
			if ((int)ScaleHeight<=0) {
				return;
			}
			HDelta = HStep*y;
			y = 0;
		}
		if (x>=(int)ScreenWidth) {	/* NCT all 640 480 are now ScreenWidth ScreenHeight */
			return;
		}
		if (y>=(int)ScreenHeight) {
			return;
		}
		if ((x+ScaleWidth)>ScreenWidth) {
			ScaleWidth = ScreenWidth-x;
		}
		if ((y+ScaleHeight)>ScreenHeight) {
			ScaleHeight = ScreenHeight-y;
		}
		DrawShapeLowLevelScaled16(x,y,ScaleWidth,ScaleHeight,(ImageWidth*2)+Skip,
			HStep,HDelta,WStep,WDelta,ShapePtr);
	}
}

/**********************************

	Clipped scaled blitter

**********************************/

static void BURGERCALL DrawMShapeLowLevelScaledClipped16(const LBRect *DestRect,Word ImageWidth,Word ImageHeight,Word Skip,void *ShapePtr)
{
	Word ScaleWidth,ScaleHeight;
	Fixed32 WStep,WDelta;
	Fixed32 HStep,HDelta;
	int x,y;

	ScaleWidth = (Word)LBRectWidth(DestRect);
	ScaleHeight = (Word)LBRectHeight(DestRect);
	if (ScaleWidth && ScaleHeight) {
		WStep = (Fixed32)((ImageWidth<<16) / ScaleWidth);
		HStep = (Fixed32)((ImageHeight<<16) / ScaleHeight);
		HDelta = 0;
		WDelta = 0;
		x = DestRect->left;
		y = DestRect->top;

		if (x<0) {
			x = 0-x;
			ScaleWidth = ScaleWidth-x;
			if ((int)ScaleWidth<=0) {
				return;
			}
			WDelta = WStep*x;
			x = 0;
		}
		if (y<0) {
			y = 0-y;
			ScaleHeight = ScaleHeight-y;
			if ((int)ScaleHeight<=0) {
				return;
			}
			HDelta = HStep*y;
			y = 0;
		}
		if (x>=(int)ScreenWidth) {		/* NCT all 640 480 are now ScreenWidth ScreenHeight */
			return;
		}
		if (y>=(int)ScreenHeight) {
			return;
		}
		if ((x+ScaleWidth)>ScreenWidth) {
			ScaleWidth = ScreenWidth-x;
		}
		if ((y+ScaleHeight)>ScreenHeight) {
			ScaleHeight = ScreenHeight-y;
		}
		DrawMShapeLowLevelScaled16(x,y,ScaleWidth,ScaleHeight,(ImageWidth*2)+Skip,
			HStep,HDelta,WStep,WDelta,ShapePtr);
	}
}

/**********************************

	Clipped scaled blitter

**********************************/

static void BURGERCALL DrawShapeLowLevelScaledClipped(const LBRect *DestRect,Word ImageWidth,Word ImageHeight,Word Skip,void *ShapePtr)
{
	Word ScaleWidth,ScaleHeight;
	Fixed32 WStep,WDelta;
	Fixed32 HStep,HDelta;
	int x,y;

	ScaleWidth = (Word)LBRectWidth(DestRect);
	ScaleHeight = (Word)LBRectHeight(DestRect);
	if (ScaleWidth && ScaleHeight) {
		WStep = (Fixed32)((ImageWidth<<16) / ScaleWidth);
		HStep = (Fixed32)((ImageHeight<<16) / ScaleHeight);
		HDelta = 0;
		WDelta = 0;
		x = DestRect->left;
		y = DestRect->top;

		if (x<0) {
			x = 0-x;
			ScaleWidth = ScaleWidth-x;
			if ((int)ScaleWidth<=0) {
				return;
			}
			WDelta = WStep*x;
			x = 0;
		}
		if (y<0) {
			y = 0-y;
			ScaleHeight = ScaleHeight-y;
			if ((int)ScaleHeight<=0) {
				return;
			}
			HDelta = HStep*y;
			y = 0;
		}
		if (x>=(int)ScreenWidth) {				/* NCT all 640 480 are now ScreenWidth ScreenHeight */
			return;
		}
		if (y>=(int)ScreenHeight) {
			return;
		}
		if ((x+ScaleWidth)>ScreenWidth) {
			ScaleWidth = ScreenWidth-x;
		}
		if ((y+ScaleHeight)>ScreenHeight) {
			ScaleHeight = ScreenHeight-y;
		}
		DrawShapeLowLevelScaled(x,y,ScaleWidth,ScaleHeight,ImageWidth+Skip,
			HStep,HDelta,WStep,WDelta,ShapePtr);
	}
}

/**********************************

	Clipped scaled blitter

**********************************/

static void BURGERCALL DrawMShapeLowLevelScaledClipped(const LBRect *DestRect,Word ImageWidth,Word ImageHeight,Word Skip,void *ShapePtr)
{
	Word ScaleWidth,ScaleHeight;
	Fixed32 WStep,WDelta;
	Fixed32 HStep,HDelta;
	int x,y;

	ScaleWidth = (Word)LBRectWidth(DestRect);
	ScaleHeight = (Word)LBRectHeight(DestRect);
	if (ScaleWidth && ScaleHeight) {
		WStep = (Fixed32)((ImageWidth<<16) / ScaleWidth);
		HStep = (Fixed32)((ImageHeight<<16) / ScaleHeight);
		HDelta = 0;
		WDelta = 0;
		x = DestRect->left;
		y = DestRect->top;

		if (x<0) {
			x = 0-x;
			ScaleWidth = ScaleWidth-x;
			if ((int)ScaleWidth<=0) {
				return;
			}
			WDelta = WStep*x;
			x = 0;
		}
		if (y<0) {
			y = 0-y;
			ScaleHeight = ScaleHeight-y;
			if ((int)ScaleHeight<=0) {
				return;
			}
			HDelta = HStep*y;
			y = 0;
		}
		if (x>=(int)ScreenWidth) {				/* NCT all 640 480 are now ScreenWidth ScreenHeight */
			return;
		}
		if (y>=(int)ScreenHeight) {
			return;
		}
		if ((x+ScaleWidth)>ScreenWidth) {
			ScaleWidth = ScreenWidth-x;
		}
		if ((y+ScaleHeight)>ScreenHeight) {
			ScaleHeight = ScreenHeight-y;
		}
		DrawMShapeLowLevelScaled(x,y,ScaleWidth,ScaleHeight,ImageWidth+Skip,
			HStep,HDelta,WStep,WDelta,ShapePtr);
	}
}

/**********************************

	This is the Generic API for drawing to the screen

**********************************/

Word ScreenAPI;			/* Which API am I using (Direct3D, OpenGL?) */
TranslucencyMode_e ScreenTranslucencyMode;		/* Current 3D translucency mode */
FilteringMode_e ScreenFilteringMode;			/* Current texture filtering mode */
ShadingMode_e ScreenShadingMode;				/* Current shading mode */
DepthTestMode_e ScreenDepthTestMode;			/* Type of ZBuffer test */
Word ScreenDepthWriteMode;						/* Write to the ZBuffer? */
Word ScreenPerspectiveMode;						/* Perspective correct mode active? */
Word ScreenWireFrameMode;						/* Are polygons wireframed? */
Word ScreenBlendMode;							/* Last alpha mode */
Word ScreenUsing2DCoords;						/* Whether 2D drawing is currently on */
Word32 ScreenCurrentTexture;					/* Last texture found */
ScreenShapeInitProcPtr ScreenInit;								/* Init the 3D context */
ScreenShapeSolidRectProcPtr ScreenSolidRect;		/* Draw a solid rect */
Word8 *ScreenITable;

/**********************************

	This will allow switching from the API's

**********************************/

#if defined(__WIN32__)
static void BURGERCALL ScreenInitDirect3D(void)
{
	D3DInit(0);
}
#endif

static void BURGERCALL SoftSolidRect(int x,int y,Word Width,Word Height,Word32 Color)
{
	Word NewColor;
	switch (VideoColorDepth) {
	case 15:
		DrawARect16(x,y,Width,Height,((Color>>9)&0x7C00)+((Color>>6)&0x3E0)+((Color>>3)&0x1F));
		break;
	case 16:
		DrawARect16(x,y,Width,Height,((Color>>8)&0xF800)+((Color>>5)&0x7E0)+((Color>>3)&0x1F));
		break;
	case 8:
		NewColor = ((Color>>9)&0x7C00)+((Color>>6)&0x3E0)+((Color>>3)&0x1F);
		DrawARect(x,y,Width,Height,ScreenITable[NewColor]);
	}
}

static void BURGERCALL SoftInit(void)
{
	if (VideoColorDepth==8) {
		Word i;
		DeallocAPointer(ScreenITable);
		ScreenITable = (Word8 *)AllocAPointer(32768);
		i = 0;
		do {
			ScreenITable[i] = static_cast<Word8>(PaletteFindColorIndex(&CurrentPalette[3],RGB5ToRGB8Table[i>>10],RGB5ToRGB8Table[(i>>5)&0x1F],RGB5ToRGB8Table[i&0x1F],254)+1);
		} while (++i<32768);
	}
}

void BURGERCALL ScreenInitAPI(Word APIType)
{

	/* I am inserting some sanity checks for foreign API's */
#if !defined(__WIN32__) && !defined(__MAC__)
	APIType = SCREENSHAPEMODESOFTWARE;
#elif defined(__MAC__)
	if (APIType==SCREENSHAPEMODEDIRECT3D || APIType==SCREENSHAPEMODEDIRECTDRAW) {
		APIType = SCREENSHAPEMODEOPENGL;
	}
#endif

	if (APIType != ScreenAPI) {
		switch (ScreenAPI) {
#ifdef OPENGLPRESENT
			case SCREENSHAPEMODEOPENGL: {
					OpenGLRelease();
				} break;
#endif
#if defined(__WIN32__)
			case SCREENSHAPEMODEDIRECT3D: {
					Direct3DDestroy();
				} break;
#endif
			case SCREENSHAPEMODESOFTWARE: {
					/* we don't have to do anything when
					   switching from SW to something else */
				} break;
		}
	}

	ScreenAPI = APIType;			/* Save the selected API */
	switch (APIType) {
#ifdef OPENGLPRESENT
	case SCREENSHAPEMODEOPENGL:		/* It's OpenGL! */
		ScreenSolidRect = OpenGLSolidRect;
		ScreenInit = OpenGLInit;
		break;
#endif

#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D:	/* Let's use Direct3D! */
		ScreenSolidRect = Direct3DSolidRect;
		ScreenInit = ScreenInitDirect3D;
		break;
#endif
	case SCREENSHAPEMODESOFTWARE:
		ScreenSolidRect = SoftSolidRect;
		ScreenInit = SoftInit;
		break;
	}
}

/**********************************

	Set the video page to a specific size using the
	chosen video API

**********************************/

Word BURGERCALL ScreenSetDisplayToSize(Word Width,Word Height,Word Depth,Word Flags)
{
#if defined(OPENGLPRESENT) || defined(__WIN32__)
	Word Result;
	switch(ScreenAPI) {
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		Result = OpenGLSetDisplayToSize(Width,Height,Depth&0xFFFF,Flags&0xFFFF);
		break;
#endif
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D:
		Result = SetDisplayToSize(Width,Height,Depth,Flags|SETDISPLAYD3D);
		break;
#endif
	default:
		Result = SetDisplayToSize(Width,Height,Depth,Flags);
		break;
	}
	return Result;
#else
	return SetDisplayToSize(Width,Height,Depth,Flags);
#endif
}


/**********************************

	Let's begin the scene's rendering

**********************************/

void BURGERCALL ScreenBeginScene(void)
{
	switch (ScreenAPI) {
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D:
		Direct3DBeginScene();			/* Begin the scene */
		Direct3DLockExecuteBuffer();	/* Lock down the command buffer */
		break;
	case SCREENSHAPEMODEOPENGL:
		{
			extern HDC OpenGLDeviceContext;
			extern HGLRC OpenGLContext;
			//wglMakeCurrent(OpenGLDeviceContext,OpenGLContext);
		}
		break;
#endif
	case SCREENSHAPEMODESOFTWARE:
		LockVideoPage();				/* Prepare the drawing surface */
		break;
	}
	ScreenTranslucencyMode=TRANSLUCENCYMODE_BAD;	/* Current 3D translucency mode */
	ScreenFilteringMode=FILTERINGMODE_BAD;			/* Current texture filtering mode */
	ScreenShadingMode=SHADINGMODE_BAD;				/* Current shading mode */
	ScreenDepthTestMode=DEPTHTESTMODE_BAD;			/* Type of ZBuffer test */
	ScreenDepthWriteMode=666;						/* Write to the ZBuffer? */
	ScreenPerspectiveMode=666;						/* Perspective correct mode active? */
	ScreenBlendMode=666;							/* Last alpha mode */
	ScreenCurrentTexture=0xd5aa96;					/* Last texture found */
}

/**********************************

	End the drawing of the scene.
	I will then page flip to show the results.

**********************************/

void BURGERCALL ScreenEndScene(void)
{
	switch (ScreenAPI) {
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D:
		Direct3DUnlockExecuteBuffer();
		Direct3DEndScene();
#endif
	case SCREENSHAPEMODESOFTWARE:
		UpdateAndPageFlip();			/* Simple for software mode */
		break;

#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		glFinish();						/* Wrap up openGL commands */
		OpenGLSwapBuffers();			/* Page flip */
		break;
#endif
	}
}

/**********************************

	Set the 3D API into a translucent mode

**********************************/

void BURGERCALL ScreenForceTranslucencyMode(TranslucencyMode_e NewMode)
{
	ScreenTranslucencyMode=NewMode;				/* Set the new state */

#if defined(__WIN32__) || defined(OPENGLPRESENT)
	switch (ScreenAPI) {
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D: {				/* Direct 3D version */
		Word8 *WorkPtr;
		Direct3DCheckExecBuffer(32,0);
		WorkPtr = Direct3DExecInstPtr;
		switch(NewMode) {
		case TRANSLUCENCYMODE_OFF:
			if (ScreenBlendMode != FALSE) {
				ScreenBlendMode = FALSE;
				STORE_OP_RENDERSTATE_CONST(1,WorkPtr);
				STORE_DATA_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE, WorkPtr);
			}
			STORE_OP_RENDERSTATE_CONST(2, WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE, WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ZERO, WorkPtr);
			break;
		case TRANSLUCENCYMODE_NORMAL:
			if (ScreenBlendMode != TRUE) {
				ScreenBlendMode = TRUE;
				STORE_OP_RENDERSTATE_CONST(1, WorkPtr);
				STORE_DATA_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE, WorkPtr);
			}
			STORE_OP_RENDERSTATE_CONST(2, WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_SRCALPHA, WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCALPHA, WorkPtr);
			break;
		case TRANSLUCENCYMODE_COLOR:
			if (ScreenBlendMode != TRUE) {
				ScreenBlendMode = TRUE;
				STORE_OP_RENDERSTATE_CONST(1, WorkPtr);
				STORE_DATA_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE, WorkPtr);
			}
			STORE_OP_RENDERSTATE_CONST(2, WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ZERO, WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_SRCCOLOR, WorkPtr);
			break;
		case TRANSLUCENCYMODE_INVCOLOR:
			if (ScreenBlendMode != TRUE) {
				ScreenBlendMode = TRUE;
				STORE_OP_RENDERSTATE_CONST(1, WorkPtr);
				STORE_DATA_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE, WorkPtr);
			}
			STORE_OP_RENDERSTATE_CONST(2, WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ZERO, WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCCOLOR, WorkPtr);
			break;
		case TRANSLUCENCYMODE_GLOWING:
			if (ScreenBlendMode != TRUE) {
				ScreenBlendMode = TRUE;
				STORE_OP_RENDERSTATE_CONST(1, WorkPtr);
				STORE_DATA_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE, WorkPtr);
			}
			STORE_OP_RENDERSTATE_CONST(2, WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_SRCALPHA, WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE, WorkPtr);
			break;
		case TRANSLUCENCYMODE_DARKENINGCOLOR:
			if (ScreenBlendMode != TRUE) {
				ScreenBlendMode = TRUE;
				STORE_OP_RENDERSTATE_CONST(1, WorkPtr);
				STORE_DATA_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE, WorkPtr);
			}
			STORE_OP_RENDERSTATE_CONST(2, WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_INVDESTCOLOR, WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ZERO, WorkPtr);
			break;
		case TRANSLUCENCYMODE_JUSTSETZ:
			if (ScreenBlendMode != TRUE) {
				ScreenBlendMode = TRUE;
				STORE_OP_RENDERSTATE_CONST(1, WorkPtr);
				STORE_DATA_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE, WorkPtr);
			}
			STORE_OP_RENDERSTATE_CONST(2, WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ZERO, WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE, WorkPtr);
			break;
		}
		Direct3DExecInstPtr = WorkPtr;
		}
#endif
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		switch (NewMode) {
		case TRANSLUCENCYMODE_OFF:
			if (ScreenBlendMode != FALSE) {
				ScreenBlendMode = FALSE;
				glDisable(GL_BLEND);
			}
			glBlendFunc(GL_ONE,GL_ZERO);
			break;
		case TRANSLUCENCYMODE_NORMAL:
			if (ScreenBlendMode != TRUE) {
				ScreenBlendMode = TRUE;
				glEnable(GL_BLEND);
			}
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			break;
		case TRANSLUCENCYMODE_COLOR:
			if (ScreenBlendMode != TRUE) {
				ScreenBlendMode = TRUE;
				glEnable(GL_BLEND);
			}
			glBlendFunc(GL_ZERO,GL_SRC_COLOR);
			break;
		case TRANSLUCENCYMODE_INVCOLOR:
			if (ScreenBlendMode != TRUE) {
				ScreenBlendMode = TRUE;
				glEnable(GL_BLEND);
			}
			glBlendFunc(GL_ZERO,GL_ONE_MINUS_SRC_COLOR);
			break;
		case TRANSLUCENCYMODE_GLOWING:
			if (ScreenBlendMode != TRUE) {
				ScreenBlendMode = TRUE;
				glEnable(GL_BLEND);
			}
			glBlendFunc(GL_SRC_ALPHA,GL_ONE);
			break;
		case TRANSLUCENCYMODE_DARKENINGCOLOR:
			if (ScreenBlendMode != TRUE) {
				ScreenBlendMode = TRUE;
				glEnable(GL_BLEND);
			}
			glBlendFunc(GL_ONE_MINUS_DST_COLOR,GL_ZERO);
			break;
		case TRANSLUCENCYMODE_JUSTSETZ:
			if (ScreenBlendMode != TRUE) {
				ScreenBlendMode = TRUE;
				glEnable(GL_BLEND);
			}
			glBlendFunc(GL_ZERO,GL_ONE);
			break;
		}
		break;
#endif
	}
#endif
}

/**********************************

	Enable/Disable filtering

**********************************/

void BURGERCALL ScreenForceFilteringMode(FilteringMode_e NewMode)
{
	ScreenFilteringMode = NewMode;
#if defined(__WIN32__) || defined(OPENGLPRESENT)
	switch (ScreenAPI) {
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D: {			/* Direct3d Present? */
		Word8 *WorkPtr;
		Direct3DCheckExecBuffer(20,0);
		WorkPtr = Direct3DExecInstPtr;
	    STORE_OP_RENDERSTATE_CONST(2, WorkPtr);
		switch(NewMode) {
		case FILTERINGMODE_OFF:
		    STORE_DATA_STATE(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_NEAREST, WorkPtr);
		    STORE_DATA_STATE(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_NEAREST, WorkPtr);
			break;
		case FILTERINGMODE_BILINEAR:
		    STORE_DATA_STATE(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR, WorkPtr);
		    STORE_DATA_STATE(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR, WorkPtr);
			break;
		}
		Direct3DExecInstPtr = WorkPtr;
		}
#endif
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		switch (NewMode) {
		case FILTERINGMODE_OFF:
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			break;
		case FILTERINGMODE_BILINEAR:
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			break;
		}
		break;
#endif
	}
#endif
}

/**********************************

	Enable/Disable wire frame mode

**********************************/

void BURGERCALL ScreenForceWireFrameMode(Word Flag)
{
	ScreenWireFrameMode=Flag;
#if defined(__WIN32__) || defined(OPENGLPRESENT)
	switch (ScreenAPI) {
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D: {		/* Direct 3D present? */
		Word8 *WorkPtr;
		Direct3DCheckExecBuffer(12,0);
		WorkPtr = Direct3DExecInstPtr;
		STORE_OP_RENDERSTATE_CONST(1,WorkPtr);
		if (Flag) {
			STORE_DATA_STATE(D3DRENDERSTATE_FILLMODE,D3DFILL_WIREFRAME, WorkPtr);
		} else {
			STORE_DATA_STATE(D3DRENDERSTATE_FILLMODE,D3DFILL_SOLID,WorkPtr);
		}
		Direct3DExecInstPtr = WorkPtr;
		}
#endif
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		if (Flag) {
			glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		} else {
			glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		}
		break;
#endif
	}
#endif
}

/**********************************

	Enable/Disable texturing

**********************************/

void BURGERCALL ScreenForceTexture(Word32 TexNum)
{
#if defined(__WIN32__) || defined(OPENGLPRESENT)
	switch (ScreenAPI) {
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D:			/* Direct3d Present? */
		{
			Word8 *WorkPtr;
			Direct3DCheckExecBuffer(12,0);
			WorkPtr = Direct3DExecInstPtr;
			STORE_OP_RENDERSTATE_CONST(1,WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_TEXTUREHANDLE,TexNum,WorkPtr);
			Direct3DExecInstPtr = WorkPtr;
		}
#endif
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		if (!TexNum) {
			glDisable(GL_TEXTURE_2D);
		} else {
			if (!ScreenCurrentTexture) {
				glEnable(GL_TEXTURE_2D);		/* Turn on texturing */
			}
			glBindTexture(GL_TEXTURE_2D,TexNum);				/* Lock this texture */
		}
		break;
#endif
	}
#endif
	ScreenCurrentTexture = TexNum;		/* I need the previous var for OpenGL */
}

/**********************************

	Enable/Disable texture perspective corrent mode

**********************************/

void BURGERCALL ScreenForcePerspective(Word Flag)
{
	ScreenPerspectiveMode=Flag;
#if defined(__WIN32__) || defined(OPENGLPRESENT)
	switch (ScreenAPI) {
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D: {		/* Direct 3D present? */
		Word8 *WorkPtr;
		Direct3DCheckExecBuffer(12,0);
		WorkPtr = Direct3DExecInstPtr;
		STORE_OP_RENDERSTATE_CONST(1,WorkPtr);
		if (Flag) {
			STORE_DATA_STATE(D3DRENDERSTATE_TEXTUREPERSPECTIVE,TRUE, WorkPtr);
		} else {
			STORE_DATA_STATE(D3DRENDERSTATE_TEXTUREPERSPECTIVE,FALSE, WorkPtr);
		}
		Direct3DExecInstPtr = WorkPtr;
		}
#endif
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		if (Flag) {
			glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
		} else {
			glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_FASTEST);
		}
		break;
#endif
	}
#endif
}

/**********************************

	Set the type of solid color shading to use

**********************************/

void BURGERCALL ScreenForceShadingMode(ShadingMode_e NewMode)
{
	ScreenShadingMode = NewMode;
#if defined(__WIN32__) || defined(OPENGLPRESENT)
	switch (ScreenAPI) {
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D: {			/* Direct3d Present? */
		Word8 *WorkPtr;
		Direct3DCheckExecBuffer(12,0);
		WorkPtr = Direct3DExecInstPtr;
	    STORE_OP_RENDERSTATE_CONST(1, WorkPtr);
		switch(NewMode) {
		case SHADINGMODE_FLAT:
		    STORE_DATA_STATE(D3DRENDERSTATE_SHADEMODE,D3DSHADE_FLAT, WorkPtr);
			break;
		case SHADINGMODE_GOURAUD:
		    STORE_DATA_STATE(D3DRENDERSTATE_SHADEMODE,D3DSHADE_GOURAUD,WorkPtr);
			break;
		case SHADINGMODE_PHONG:
		    STORE_DATA_STATE(D3DRENDERSTATE_SHADEMODE,D3DSHADE_PHONG, WorkPtr);
		}
		Direct3DExecInstPtr = WorkPtr;
		}
#endif
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		switch (NewMode) {
		case SHADINGMODE_FLAT:
			glShadeModel(GL_FLAT);
			break;
		case SHADINGMODE_GOURAUD:
		case SHADINGMODE_PHONG:
			glShadeModel(GL_SMOOTH);		/* OpenGL only supports Gouraud */
			break;
		}
		break;
#endif
	}
#endif
}

/**********************************

	Enable/Disable writing to the ZBuffer

**********************************/

void BURGERCALL ScreenForceDepthWriteMode(Word Flag)
{
	ScreenDepthWriteMode=Flag;
#if defined(__WIN32__) || defined(OPENGLPRESENT)
	switch (ScreenAPI) {
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D: {		/* Direct 3D present? */
		Word8 *WorkPtr;
		Direct3DCheckExecBuffer(12,0);
		WorkPtr = Direct3DExecInstPtr;
		STORE_OP_RENDERSTATE_CONST(1,WorkPtr);
		if (Flag) {
			STORE_DATA_STATE(D3DRENDERSTATE_ZWRITEENABLE,TRUE, WorkPtr);
		} else {
			STORE_DATA_STATE(D3DRENDERSTATE_ZWRITEENABLE,FALSE,WorkPtr);
		}
		Direct3DExecInstPtr = WorkPtr;
		}
#endif
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		if (Flag) {
			glDepthMask(GL_TRUE);
		} else {
			glDepthMask(GL_FALSE);
		}
		break;
#endif
	}
#endif
}

/**********************************

	How shall the depth test work?

**********************************/

void BURGERCALL ScreenForceDepthTestMode(DepthTestMode_e NewMode)
{
	ScreenDepthTestMode = NewMode;
#if defined(__WIN32__) || defined(OPENGLPRESENT)
	switch (ScreenAPI) {
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D: {			/* Direct3d Present? */
		Word8 *WorkPtr;
		Direct3DCheckExecBuffer(12,0);
		WorkPtr = Direct3DExecInstPtr;
	    STORE_OP_RENDERSTATE_CONST(1, WorkPtr);
		switch(NewMode) {
		case DEPTHTESTMODE_NEVER:
		    STORE_DATA_STATE(D3DRENDERSTATE_ZFUNC,D3DCMP_NEVER, WorkPtr);
			break;
		case DEPTHTESTMODE_LESS:
		    STORE_DATA_STATE(D3DRENDERSTATE_ZFUNC,D3DCMP_LESS,WorkPtr);
			break;
		case DEPTHTESTMODE_EQUAL:
		    STORE_DATA_STATE(D3DRENDERSTATE_ZFUNC,D3DCMP_EQUAL, WorkPtr);
			break;
		case DEPTHTESTMODE_LESSEQUAL:
		    STORE_DATA_STATE(D3DRENDERSTATE_ZFUNC,D3DCMP_LESSEQUAL,WorkPtr);
			break;
		case DEPTHTESTMODE_GREATER:
		    STORE_DATA_STATE(D3DRENDERSTATE_ZFUNC,D3DCMP_GREATER,WorkPtr);
			break;
		case DEPTHTESTMODE_NOTEQUAL:
		    STORE_DATA_STATE(D3DRENDERSTATE_ZFUNC,D3DCMP_NOTEQUAL,WorkPtr);
			break;
		case DEPTHTESTMODE_GREATEREQUAL:
		    STORE_DATA_STATE(D3DRENDERSTATE_ZFUNC,D3DCMP_GREATEREQUAL,WorkPtr);
			break;
		case DEPTHTESTMODE_ALWAYS:
		    STORE_DATA_STATE(D3DRENDERSTATE_ZFUNC,D3DCMP_ALWAYS,WorkPtr);
			break;
		}
		Direct3DExecInstPtr = WorkPtr;
		}
#endif
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		switch (NewMode) {
		case DEPTHTESTMODE_NEVER:
		    glDepthFunc(GL_NEVER);
			break;
		case DEPTHTESTMODE_LESS:
		    glDepthFunc(GL_LESS);
			break;
		case DEPTHTESTMODE_EQUAL:
		    glDepthFunc(GL_EQUAL);
			break;
		case DEPTHTESTMODE_LESSEQUAL:
		    glDepthFunc(GL_LEQUAL);
			break;
		case DEPTHTESTMODE_GREATER:
		    glDepthFunc(GL_GREATER);
			break;
		case DEPTHTESTMODE_NOTEQUAL:
		    glDepthFunc(GL_NOTEQUAL);
			break;
		case DEPTHTESTMODE_GREATEREQUAL:
		    glDepthFunc(GL_GEQUAL);
			break;
		case DEPTHTESTMODE_ALWAYS:
		    glDepthFunc(GL_ALWAYS);
			break;
		}
		break;
#endif
	}
#endif
}

void BURGERCALL ScreenForceUse2DCoords( Word use2d )
{
#if defined(OPENGLPRESENT)
	switch (ScreenAPI) {
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		if (use2d && !ScreenUsing2DCoords) {
			glMatrixMode( GL_PROJECTION );
			glPushMatrix();
			glLoadIdentity();
			glOrtho( 0, ScreenWidth, ScreenHeight, 0, -1, 1 );
			glMatrixMode( GL_MODELVIEW );
			glPushMatrix();
			glLoadIdentity();
		} else if (!use2d && ScreenUsing2DCoords) {
			glMatrixMode( GL_PROJECTION );
			glPopMatrix();
			glMatrixMode( GL_MODELVIEW );
			glPopMatrix();
		}
		break;
#endif
	}
#endif
	ScreenUsing2DCoords = use2d;
}




/**********************************

	Initialize the generic pixel blitter

**********************************/

void BURGERCALL ScreenShapeInit(ScreenShape_t *Input,ScreenShapeActionProc ActionProc)
{
	FastMemSet(Input,0,sizeof(ScreenShape_t));		/* Init the base data */
	Input->ActionProc = ActionProc;					/* Init the loader */
	Input->Flags = SCREENSHAPEFLAGPURGEABLE|ScreenAPI;		/* Default flags */
	ScreenShapeBytesPerPixel = (VideoColorDepth+7)>>3;
}

/**********************************

	Create a new pixel blitter

**********************************/

ScreenShape_t *BURGERCALL ScreenShapeNew(ScreenShapeActionProc ActionProc)
{
	ScreenShape_t *Input;
	Input = (ScreenShape_t *)AllocAPointer(sizeof(ScreenShape_t));		/* Create a new screen shape */
	if (Input) {		/* Got the memory? */
		ScreenShapeInit(Input,ActionProc);		/* Initialize it */
	}
	return Input;
}

/**********************************

	Dispose of a ScreenShape_t structure

**********************************/

void BURGERCALL ScreenShapeDestroy(ScreenShape_t *Input)
{
	switch (Input->Flags & SCREENSHAPEFLAGMODEMASK) {
	case SCREENSHAPEMODESOFTWARE:
		DeallocAHandle((void **)Input->Data1);
		break;
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D:
		Direct3DTextureDestroy((Direct3DTexture_t *)&Input->Data1);
		break;
#endif
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		if (Input->Data2) {
			glDeleteTextures(1,(GLuint *)&Input->Data1);
			Input->Data2 = 0;
		}
		break;
#endif
	}
	FastMemSet(Input,0,sizeof(ScreenShape_t));	/* Zark it */
}

/**********************************

	Dispose of a ScreenShape_t structure

**********************************/

void BURGERCALL ScreenShapeDelete(ScreenShape_t *Input)
{
	if (Input) {		/* Valid pointer? */
		ScreenShapeDestroy(Input);	/* Dispose of the contents */
		DeallocAPointer(Input);		/* Dispose of the memory */
	}
}

/**********************************

	Dispose of the memory in a ScreenShape_t structure

**********************************/

void BURGERCALL ScreenShapePurge(ScreenShape_t *Input)
{
	switch (Input->Flags & SCREENSHAPEFLAGMODEMASK) {
	case SCREENSHAPEMODESOFTWARE:
		DeallocAHandle((void **)Input->Data1);
		break;
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D:
		Direct3DTextureDestroy((Direct3DTexture_t *)&Input->Data1);
		break;
#endif
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		if (Input->Data2) {
			glDeleteTextures(1,(GLuint *)&Input->Data1);
			Input->Data2 = 0;
		}
		break;
#endif
	}
	Input->Data1 = 0;
	Input->Data2 = 0;
	Input->Data3 = 0;
	Input->Flags |= SCREENSHAPEFLAGPURGEABLE;
}

/**********************************

	Draw the shape

**********************************/

void BURGERCALL ScreenShapeDraw(ScreenShape_t *Input,int x,int y)
{
	if (ScreenShapeLoad(Input)) {
		return;
	}
	x += Input->XOffset;
	y += Input->YOffset;
	
	if (Input->DrawProc)  { /* user defined draw function */
		LBRect rect;
		rect.left = x;
		rect.top = y;
		rect.right = x+Input->Width;
		rect.bottom = y+Input->Height;
		Input->DrawProc(Input,&rect);		/* Call the custom draw routine */
		return;
	}
	
	switch (Input->Flags&SCREENSHAPEFLAGMODEMASK) {
	case SCREENSHAPEMODESOFTWARE:
		{
			void **ShapePtr;
			ShapePtr = (void **)Input->Data1;
			LockVideoPage();			/* Lock down the page */
			if (!(Input->Flags&SCREENSHAPEFLAGMASK)) {
				if (VideoColorDepth==8) {
					DrawShapeLowLevelClipped(x,y,Input->Width,Input->Height,0,ShapePtr[0]);
				} else {
					DrawShapeLowLevelClipped16(x,y,Input->Width,Input->Height,0,ShapePtr[0]);
				}
			} else {
				if (VideoColorDepth==8) {
					DrawMShapeLowLevelClipped(x,y,Input->Width,Input->Height,0,ShapePtr[0]);
				} else {
					DrawMShapeLowLevelClipped16(x,y,Input->Width,Input->Height,0,ShapePtr[0]);
				}
			}
			if (Input->Flags&SCREENSHAPEFLAGPURGEABLE) {
				SetHandlePurgeFlag(ShapePtr,TRUE);
			}
		}
		break;
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D:
		Direct3DTextureDraw2D((Direct3DTexture_t *)&Input->Data1,x,y,Input->Width,Input->Height);
		break;
#endif
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		OpenGLTextureDraw2D((Word)Input->Data1,x,y,Input->Width,Input->Height);
		break;
#endif
	}
}

/**********************************

	Draw the shape scaled

**********************************/

void BURGERCALL ScreenShapeDrawScaled(ScreenShape_t *Input,const LBRect *DestRect)
{
	if (ScreenShapeLoad(Input)) {
		return;
	}
	
	if (Input->DrawProc) {		/* user defined draw function */
		Input->DrawProc(Input,DestRect);
		return;
	}
	
	switch (Input->Flags & SCREENSHAPEFLAGMODEMASK) {
	case SCREENSHAPEMODESOFTWARE:
		{
			void **ShapePtr;
			ShapePtr = (void **)Input->Data1;
			LockVideoPage();			/* Lock down the page */
			if (!(Input->Flags&SCREENSHAPEFLAGMASK)) {
				if (VideoColorDepth==8) {
					DrawShapeLowLevelScaledClipped(DestRect,Input->Width,Input->Height,0,ShapePtr[0]);
				} else {
					DrawShapeLowLevelScaledClipped16(DestRect,Input->Width,Input->Height,0,ShapePtr[0]);
				}
			} else {
				if (VideoColorDepth==8) {
					DrawMShapeLowLevelScaledClipped(DestRect,Input->Width,Input->Height,0,ShapePtr[0]);
				} else {
					DrawMShapeLowLevelScaledClipped16(DestRect,Input->Width,Input->Height,0,ShapePtr[0]);
				}
			}
			if (Input->Flags&SCREENSHAPEFLAGPURGEABLE) {
				SetHandlePurgeFlag(ShapePtr,TRUE);
			}
		}
		break;
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D:
		Direct3DTextureDraw2D((Direct3DTexture_t *)&Input->Data1,DestRect->left,DestRect->top,DestRect->right-DestRect->left,DestRect->bottom-DestRect->top);
		break;
#endif
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		OpenGLTextureDraw2D((Word)Input->Data1,DestRect->left,DestRect->top,DestRect->right-DestRect->left,DestRect->bottom-DestRect->top);
		break;
#endif
	}
}


/**********************************

	Draw the shape scaled from a sub rect

**********************************/

void BURGERCALL ScreenShapeDrawScaledSubRect(ScreenShape_t *Input,const LBRect *DestRect,const LBRect *SrcRect)
{
	if (ScreenShapeLoad(Input)) {
		return;
	}
	switch (Input->Flags & SCREENSHAPEFLAGMODEMASK) {
	case SCREENSHAPEMODESOFTWARE:
		{
			void **ShapeHand;
			Word8 *ShapePtr;
			Word RowBytes;
			Word ByteOne;
			Word Width,Height;
			Word Skip;

			ShapeHand = (void **)Input->Data1;
			LockVideoPage();							/* Lock the video buffer */
			ByteOne = (VideoColorDepth+7)>>3;
			RowBytes = Input->Width*ByteOne;
			ShapePtr = ((Word8 *)ShapeHand[0])+(SrcRect->top*RowBytes)+(SrcRect->left*ByteOne);
			Width = SrcRect->right-SrcRect->left;
			Height = SrcRect->bottom-SrcRect->top;
			Skip = RowBytes-(Width*ByteOne);
			if (!(Input->Flags&SCREENSHAPEFLAGMASK)) {
				if (VideoColorDepth==8) {
					DrawShapeLowLevelScaledClipped(DestRect,Width,Height,Skip,ShapePtr);
				} else {
					DrawShapeLowLevelScaledClipped16(DestRect,Width,Height,Skip,ShapePtr);
				}
			} else {
				if (VideoColorDepth==8) {
					DrawMShapeLowLevelScaledClipped(DestRect,Width,Height,Skip,ShapePtr);
				} else {
					DrawMShapeLowLevelScaledClipped16(DestRect,Width,Height,Skip,ShapePtr);
				}

			}
			if (Input->Flags&SCREENSHAPEFLAGPURGEABLE) {
				SetHandlePurgeFlag(ShapeHand,TRUE);
			}
		}
		break;
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D:
		{
			float UV[4];
			float w,h;
			Word i;
			i = Input->Width;
			if (i<FLOATRECIPTABLESIZE) {
				w = FloatRecipTable[i];
			} else {
				w = 1.0f/(float)i;
			}
			i = Input->Height;
			if (i<FLOATRECIPTABLESIZE) {
				h = FloatRecipTable[i];
			} else {
				h = 1.0f/(float)i;
			}
			UV[0] = (float)SrcRect->left*w;
			UV[2] = (float)SrcRect->right*w;
			UV[1] = (float)SrcRect->top*h;
			UV[3] = (float)SrcRect->bottom*h;
			Direct3DTextureDraw2DSubRect((Direct3DTexture_t *)&Input->Data1,DestRect->left,DestRect->top,DestRect->right-DestRect->left,DestRect->bottom-DestRect->top,UV);
		}
		break;
#endif
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		{
			float UV[4];
			float w,h;
			Word i;
			i = Input->Width;
			if (i<FLOATRECIPTABLESIZE) {
				w = FloatRecipTable[i];
			} else {
				w = 1.0f/(float)i;
			}
			i = Input->Height;
			if (i<FLOATRECIPTABLESIZE) {
				h = FloatRecipTable[i];
			} else {
				h = 1.0f/(float)i;
			}
			UV[0] = (float)SrcRect->left*w;
			UV[2] = (float)SrcRect->right*w;
			UV[1] = (float)SrcRect->top*h;
			UV[3] = (float)SrcRect->bottom*h;
			OpenGLTextureDraw2DSubRect((Word)Input->Data1,DestRect->left,DestRect->top,DestRect->right-DestRect->left,DestRect->bottom-DestRect->top,UV);
		}
		break;
#endif
	}
}

/**********************************

	Allow direct access to the texture

**********************************/

void BURGERCALL ScreenShapeLock(ScreenShape_t *Input,Image_t *Output)
{
	void **ShapePtr;
	if (!ScreenShapeLoad(Input)) {
		switch (Input->Flags & SCREENSHAPEFLAGMODEMASK) {
		case SCREENSHAPEMODESOFTWARE:
			ShapePtr = (void **)Input->Data1;	/* Get the handle */
			Output->ImagePtr = (Word8 *)LockAHandle(ShapePtr);
			Output->Width = Input->Width;
			Output->Height = Input->Height;
			Output->RowBytes = Input->Width*ScreenShapeBytesPerPixel;
			Output->PalettePtr = CurrentPalette;
			Output->AlphaPtr = 0;
			Output->DataType = (ImageTypes_e)VideoColorDepth;
			return;
		}
	}
	FastMemSet(Output,0,sizeof(Image_t));	/* Ohoh, I did a bad thing! */
}

/**********************************

	Release the direct access

**********************************/

void BURGERCALL ScreenShapeUnlock(ScreenShape_t *Input)
{
	void **ShapePtr;
	switch (Input->Flags & SCREENSHAPEFLAGMODEMASK) {
	case SCREENSHAPEMODESOFTWARE:
		ShapePtr = (void **)Input->Data1;
		if (ShapePtr) {
			UnlockAHandle(ShapePtr);
			if (Input->Flags&SCREENSHAPEFLAGPURGEABLE) {
				SetHandlePurgeFlag(ShapePtr,TRUE);
			}
		}
		break;
	}
}

/**********************************

	Load the data for a shape

**********************************/

Word BURGERCALL ScreenShapeLoad(ScreenShape_t *Input)
{
	if (!Input->ActionProc) {	/* user disable screen-shape loading */
		return FALSE;
	}
	switch (Input->Flags & SCREENSHAPEFLAGMODEMASK) {
	case SCREENSHAPEMODESOFTWARE:
		if (Input->Data1 && ((void **)Input->Data1)[0]) {
			return FALSE;
		}
		if (!Input->ActionProc(Input)) {
			return FALSE;
		}
		break;
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D:
#endif
	case SCREENSHAPEMODEOPENGL:
		if (Input->Data2) {
			return FALSE;
		}
		if (!Input->ActionProc(Input)) {
			return FALSE;
		}
		break;
	}
	return TRUE;
}

/**********************************



**********************************/

void BURGERCALL ScreenShapeDisallowPurge(ScreenShape_t *Input)
{
	switch (Input->Flags & SCREENSHAPEFLAGMODEMASK) {
	case SCREENSHAPEMODESOFTWARE:
		if (Input->Data1 && ((void **)Input->Data1)[0]) {
			SetHandlePurgeFlag((void **)Input->Data1,FALSE);
		}
	}
	Input->Flags &= ~SCREENSHAPEFLAGPURGEABLE;
}

/**********************************

	Load the data for a shape

**********************************/

void BURGERCALL ScreenShapeAllowPurge(ScreenShape_t *Input)
{
	switch (Input->Flags & SCREENSHAPEFLAGMODEMASK) {
	case SCREENSHAPEMODESOFTWARE:
		if (Input->Data1 && ((void **)Input->Data1)[0]) {
			SetHandlePurgeFlag((void **)Input->Data1,TRUE);
		}
	}
	Input->Flags |= SCREENSHAPEFLAGPURGEABLE;
}

/**********************************

	Return the bounding rect for a shape

**********************************/

void BURGERCALL ScreenShapeGetBounds(ScreenShape_t *Input,LBRect *Bounds)
{
	if (ScreenShapeLoad(Input)) {
		Bounds->top = 0;
		Bounds->left = 0;
		Bounds->bottom = 0;
		Bounds->right = 0;
		return;
	}
	Bounds->left = Input->XOffset;
	Bounds->top = Input->YOffset;
	Bounds->right = Input->XOffset+Input->Width;
	Bounds->bottom = Input->YOffset+Input->Height;
}

/**********************************

	Return the size of a shape

**********************************/

void BURGERCALL ScreenShapeGetSize(ScreenShape_t *Input,LBPoint *Size)
{
	if (ScreenShapeLoad(Input)) {
		Size->x = 0;
		Size->y = 0;
		return;
	}
	Size->x = Input->Width;
	Size->y = Input->Height;
}

/**********************************

	Get a pixel color

**********************************/

Word BURGERCALL ScreenShapeGetPixel(ScreenShape_t *Input,int x,int y)
{
	void **ShapePtr;
	Word Result;

	if (ScreenShapeLoad(Input)) {
		return 0;
	}
	Result = 0;
	switch (Input->Flags & SCREENSHAPEFLAGMODEMASK) {
	case SCREENSHAPEMODESOFTWARE:
		ShapePtr = (void **)Input->Data1;
		if ((Word)x<Input->Width && (Word)y<Input->Height) {
			x = y*Input->Width+x;
			if (VideoColorDepth==8) {
				Result = ((Word8 **)ShapePtr)[0][x];
			} else {
				Result = ((Word16 **)ShapePtr)[0][x];
			}
		}
		if (Input->Flags&SCREENSHAPEFLAGPURGEABLE) {
			SetHandlePurgeFlag(ShapePtr,TRUE);
		}
	}
	return Result;
}

/**********************************

	Given three shapes, I will draw a variable length bar using a top shape,
	a bottom shape and a fill pattern.
	The array of shapes will be Pattern (0), Top (1) and Bottom (2)

**********************************/

int BURGERCALL ScreenShapeVPatternBar(ScreenShape_t **ArrayPtr,int x,int TopY,int BottomY)
{
	int Height;				/* Number of pixels to draw */
	int Center;				/* Size of the pattern to draw in pixels */
	int Result;				/* Center Y coord of the pattern if any */
	LBPoint PicSizeT;		/* Size of the top shape */
	LBPoint PicSizeB;		/* Size of the bottom shape */

	Result = -1;										/* Assume no center point */
	Height = BottomY-TopY;								/* Get the height of the section */
	if (Height>0) {										/* Anything to draw? */
		LBRect ClipRect;

		ScreenShapeGetSize(ArrayPtr[1],&PicSizeT);		/* Top art size */
		ScreenShapeGetSize(ArrayPtr[2],&PicSizeB);		/* Bottom art size */

		Center = (Height-PicSizeT.y)-PicSizeB.y;		/* Get the center art size if any */
		if (Center>=0) {								/* Center is present (It means only the center is clipped) */
		
			/* Draw the top part */
			
			ScreenShapeDraw(ArrayPtr[1],x,TopY);		/* Draw the top */
			ScreenShapeDraw(ArrayPtr[2],x,BottomY-PicSizeB.y);		/* Draw the bottom */

			/* I have to draw the center */

			if (Center) {	
				TopY = TopY+PicSizeT.y;				/* Top of the center part */
				Result = (Center>>1)+TopY;			/* Center of the center (Redundant isn't it?) */
				GetTheClipRect(&ClipRect);			/* Clipping is involved */
				SetTheClipBounds(ClipRect.left,ClipRect.top,ClipRect.right,TopY+Center);		/* Clip the bottom */
				do {
					ScreenShapeDraw(ArrayPtr[0],x,TopY);		/* Draw the fill pattern */
					Height = ArrayPtr[0]->Height;									/* Height of the pattern */
					TopY += Height;							/* Move down */
					Center -= Height;						/* Remove remaining pixels */
				} while (Center>0);							/* Any left? */
				SetTheClipRect(&ClipRect);					/* Restore the clip rect */
			}
			
			/* I'm done */
			
		} else {
		
			/* This is tricky, there is no center, therefore I */
			/* need to clip the top and bottom pieces */
			/* Since the top and bottom are NOT guaranteed to be the same */
			/* height, I have to check the three cases, clip top only, clip bottom only, */
			/* clip both evenly */
			
			GetTheClipRect(&ClipRect);
			Center = Height>>1;			/* Split the bounds in half */
			
			if (Center>=PicSizeT.y) {	/* Top is not clipped */
				
				/* Bottom clipped only */
				
				ScreenShapeDraw(ArrayPtr[1],x,TopY);			/* Draw the top only */
				SetTheClipBounds(ClipRect.left,TopY+PicSizeT.y,ClipRect.right,ClipRect.bottom);
				ScreenShapeDraw(ArrayPtr[2],x,BottomY-PicSizeB.y);
				
			} else if ((Height-Center)>=PicSizeB.y) {		/* May be Center+1 for odd Height */
				
				/* Top clipped only */
				
				ScreenShapeDraw(ArrayPtr[2],x,BottomY-PicSizeB.y);
				SetTheClipBounds(ClipRect.left,ClipRect.top,ClipRect.right,BottomY-PicSizeB.y);
				ScreenShapeDraw(ArrayPtr[1],x,TopY);
				
			} else {

				/* Both are clipped evenly */

				SetTheClipBounds(ClipRect.left,ClipRect.top,ClipRect.right,TopY+Center);
				ScreenShapeDraw(ArrayPtr[1],x,TopY);
				SetTheClipBounds(ClipRect.left,TopY+Center,ClipRect.right,ClipRect.bottom);
				ScreenShapeDraw(ArrayPtr[2],x,BottomY-PicSizeB.y);
			}
			SetTheClipRect(&ClipRect);					/* Restore the clip rect */
		}
	}
	return Result;			/* Return -1 if no center, otherwise the center Y of the pattern */
}

/**********************************

	Given three shapes, I will draw a variable length bar using a left shape,
	a right shape and a fill pattern.
	The array of shapes will be Pattern (0), Top (1) and Bottom (2)

**********************************/

int BURGERCALL ScreenShapeHPatternBar(ScreenShape_t **ArrayPtr,int y,int LeftX,int RightX)
{
	int Width;				/* Number of pixels to draw */
	int Center;				/* Size of the pattern to draw in pixels */
	int Result;				/* Center Y coord of the pattern if any */
	LBPoint PicSizeT;		/* Size of the top shape */
	LBPoint PicSizeB;		/* Size of the bottom shape */

	Result = -1;										/* Assume no center point */
	Width = RightX-LeftX;								/* Get the height of the section */
	if (Width>0) {										/* Anything to draw? */
		LBRect ClipRect;

		ScreenShapeGetSize(ArrayPtr[1],&PicSizeT);		/* Top art size */
		ScreenShapeGetSize(ArrayPtr[2],&PicSizeB);		/* Bottom art size */

		Center = (Width-PicSizeT.x)-PicSizeB.x;			/* Get the center art size if any */
		if (Center>=0) {								/* Center is present (It means only the center is clipped) */
		
			/* Draw the top part */
			
			ScreenShapeDraw(ArrayPtr[1],LeftX,y);		/* Draw the top */
			ScreenShapeDraw(ArrayPtr[2],RightX-PicSizeB.x,y);		/* Draw the bottom */

			/* I have to draw the center */

			if (Center) {	
				LeftX = LeftX+PicSizeT.x;				/* Top of the center part */
				Result = (Center>>1)+LeftX;				/* Center of the center (Redundant isn't it?) */
				GetTheClipRect(&ClipRect);				/* Clipping is involved */
				SetTheClipBounds(ClipRect.left,ClipRect.top,LeftX+Center,ClipRect.bottom);		/* Clip the bottom */
				do {
					ScreenShapeDraw(ArrayPtr[0],LeftX,y);	/* Draw the fill pattern */
					Width = ArrayPtr[0]->Width;				/* Height of the pattern */
					LeftX += Width;							/* Move down */
					Center -= Width;						/* Remove remaining pixels */
				} while (Center>0);							/* Any left? */
				SetTheClipRect(&ClipRect);					/* Restore the clip rect */
			}
			
			/* I'm done */
			
		} else {
		
			/* This is tricky, there is no center, therefore I */
			/* need to clip the top and bottom pieces */
			/* Since the left and right are NOT guaranteed to be the same */
			/* height, I have to check the three cases, clip left only, clip right only, */
			/* clip both evenly */
			
			GetTheClipRect(&ClipRect);
			Center = Width>>1;			/* Split the bounds in half */
			
			if (Center>=PicSizeT.x) {	/* Top is not clipped */
				
				/* Bottom clipped only */
				
				ScreenShapeDraw(ArrayPtr[1],LeftX,y);			/* Draw the left only */
				SetTheClipBounds(LeftX+PicSizeT.x,ClipRect.top,ClipRect.right,ClipRect.bottom);
				ScreenShapeDraw(ArrayPtr[2],RightX-PicSizeB.x,y);
				
			} else if ((Width-Center)>=PicSizeB.x) {		/* May be Center+1 for odd Height */
				
				/* Top clipped only */
				
				ScreenShapeDraw(ArrayPtr[2],RightX-PicSizeB.x,y);
				SetTheClipBounds(ClipRect.left,ClipRect.top,RightX-PicSizeB.x,ClipRect.bottom);
				ScreenShapeDraw(ArrayPtr[1],LeftX,y);
				
			} else {

				/* Both are clipped evenly */

				SetTheClipBounds(ClipRect.left,ClipRect.top,LeftX+Center,ClipRect.bottom);
				ScreenShapeDraw(ArrayPtr[1],LeftX,y);
				SetTheClipBounds(LeftX+Center,ClipRect.top,ClipRect.right,ClipRect.bottom);
				ScreenShapeDraw(ArrayPtr[2],RightX-PicSizeB.x,y);
			}
			SetTheClipRect(&ClipRect);					/* Restore the clip rect */
		}
	}
	return Result;			/* Return -1 if no center, otherwise the center Y of the pattern */
}

/**********************************

	This function takes an 8 bit shape and
	palette and converts it to the native format
	Set's the DataHandle entry and the MaskPresent flag
	if successful

**********************************/

Word BURGERCALL ScreenShapeConvertFromImage(ScreenShape_t *Input,const Image_t *ImagePtr)
{
	const Word8 *ShapePtr;
	const Word8 *PalPtr;
	Word Width,Height;	/* Size of the shape */
	void **Result;		/* Temp handle */
	Word32 i;			/* Temp */
	Word32 Total;		/* Total number of pixels to convert */
	Word8 *DestPtr;		/* Destination to write to */
	Word MaskFlag;		/* TRUE if color zero is present */
	Word Table[256];	/* Color remap table */

	Width = ImagePtr->Width;		/* Get the shape's width */
	Height = ImagePtr->Height;		/* and height */
	Input->Width = Width;
	Input->Height = Height;

	switch (Input->Flags & SCREENSHAPEFLAGMODEMASK) {
#if defined(__WIN32__)
	case SCREENSHAPEMODEDIRECT3D:
		return Direct3DTextureInitImage((Direct3DTexture_t *)&Input->Data1,ImagePtr);
#endif
#if defined(OPENGLPRESENT)
	case SCREENSHAPEMODEOPENGL:
		Width = OpenGLLoadTexture((Word *)&Input->Data1,ImagePtr);
		if (!Width) {
			Input->Data2 = (void *)-1;
		}
		return Width;
#endif
	case SCREENSHAPEMODESOFTWARE:
		PalPtr = ImagePtr->PalettePtr;
		ShapePtr = ImagePtr->ImagePtr;
		Total = Width*(Word32)Height;			/* Total number of bytes */
		if (Total) {							/* Valid shape? */
			i = Total * ((VideoColorDepth+7)>>3);	//	8, 15, 32
			
			Result = AllocAHandle(i);		/* Get the destination memory + Width, Height, XOff, YOff */
			if (Result) {
				Input->Data1 = (void *)Result;	/* Save the handle */

				/* How shall it be remapped? */

				MaskFlag = 0;			/* No color #0 found yet */
				if (ImagePtr->DataType==IMAGE8_PAL || ImagePtr->DataType==IMAGE8_PAL_ALPHA_PAL) {
					if (VideoColorDepth==8) {	/* Simple repalette? */
						Table[0] = 0;			/* Don't change color #0 */
						i = 1;
						do {
							PalPtr += 3;
							Table[i] = PaletteFindColorIndex(&CurrentPalette[3],PalPtr[0],PalPtr[1],PalPtr[2],254)+1;
						} while (++i<256);		/* Done? */
						DestPtr = (Word8 *)(Result[0]);
						do {
							Word Temp;
							Temp = Table[ShapePtr[0]];
							DestPtr[0] = (Word8)Temp;
							if (!Temp) {			/* Masked? */
								MaskFlag = SCREENSHAPEFLAGMASK;	/* It has holes in it */
							}
							++ShapePtr;		/* Next source */
							++DestPtr;		/* Next dest */
						} while (--Total);
					} else {
						PaletteMake16BitLookup(Table,PalPtr,VideoColorDepth);	/* Convert to 16 bit */
						if (!VideoUseColorZero) {
							Table[0] = 0;		/* Index zero is still zero */
							i = 1;
							do {
								if (!Table[i]) {	/* Black? */
									Table[i] = 1;	/* Convert to ultra dark blue */
								}
							} while (++i<256);
						}
						DestPtr = (Word8 *)(Result[0]);
						do {
							Word Temp;
							Temp = Table[ShapePtr[0]];
							((Word16 *)DestPtr)[0] = (Word16)Temp;
							if (!Temp) {
								MaskFlag = SCREENSHAPEFLAGMASK;
							}
							++ShapePtr;
							DestPtr += 2;
						} while (--Total);
					}
					Input->Flags = MaskFlag|(Input->Flags&(~SCREENSHAPEFLAGMASK));
					return FALSE;
				} else {
					Image_t DestImage;
					DestImage.DataType = (ImageTypes_e)VideoColorDepth;
					DestImage.RowBytes = Width*((VideoColorDepth+7)>>3);
					DestImage.Width = ImagePtr->Width;
					DestImage.Height = ImagePtr->Height;
					DestImage.AlphaPtr = 0;
					DestImage.PalettePtr = CurrentPalette;
					DestImage.ImagePtr = static_cast<Word8 *>(Result[0]);
					ImageStore(&DestImage,ImagePtr);			/* Perform the conversion */

					/* Special code to convert RGBA to ARGB */
					
					if (DestImage.DataType == IMAGE8888) {
						Word x,y,x1;
						x = DestImage.Width;
						y = DestImage.Height;
						if (x && y) {
							Word32 *ptr;
							Word Skip;
							Skip = DestImage.RowBytes-(x*4);
							ptr = (Word32 *)DestImage.ImagePtr;
							do {
								x1 = x;
								do {
									Word32 color;
									color = ptr[0];
#if defined(__BIGENDIAN__)
									color = (color>>8) | (color<<24);	/* >>8 RGBA -> 0RGB, <<24 RGBA -> A000 */
#else
									color = (color<<8) | (color>>24);	/* <<8 ABGR -> BGR0, >>24 ABGR -> 000A */
#endif
									ptr[0] = color;			/* ARGB or BGRA */
									++ptr;
								} while (--x1);				/* Width done */
								ptr = (Word32 *)((Word8 *)ptr+Skip);	/* Next line */
							} while (--y);					/* Height done? */
						}
					}
					return FALSE;			/* I did it!! */
				}
			}
		}
		break;
	}
	return TRUE;
}

/**********************************

	This procedure handles all of the specific code for a ScreenShapeGfx_t
	structure

**********************************/

static Word BURGERCALL ScreenShapeGfxHandler(ScreenShapeGfx_t *Input)
{
	Word8 *PalPtr;
	Word8 *ShapePtr;
	Word8 *AlphaPtr;
	Word Result;
	Result = TRUE;
	if (Input->RezFile) {
		PalPtr = (Word8 *)ResourceLoad(Input->RezFile,Input->RezNum);
		if (PalPtr) {
			Word Size;
			Image_t TempImage;
			ShapePtr = PalPtr+768;
#ifdef __BIGENDIAN__
			if (ResourceJustLoaded) {
				((Word16 *)ShapePtr)[0] = Burger::SwapEndian(((Word16 *)ShapePtr)[0]);
				((Word16 *)ShapePtr)[1] = Burger::SwapEndian(((Word16 *)ShapePtr)[1]);
				((Word16 *)ShapePtr)[2] = Burger::SwapEndian(((Word16 *)ShapePtr)[2]);
				((Word16 *)ShapePtr)[3] = Burger::SwapEndian(((Word16 *)ShapePtr)[3]);
			}
#endif
			TempImage.Width = GetXShapeWidth(ShapePtr);
			TempImage.Height = GetXShapeHeight(ShapePtr);
			Input->MyShape.XOffset = GetXShapeXOffset(ShapePtr);
			Input->MyShape.YOffset = GetXShapeYOffset(ShapePtr);
			TempImage.DataType = IMAGE8_PAL;
			TempImage.RowBytes = TempImage.Width;
			ShapePtr += 8;
			TempImage.ImagePtr = ShapePtr;
			TempImage.PalettePtr = PalPtr;
		
			AlphaPtr = 0;			/* Assume no alpha channel */
			
			/* Check if an alpha channel is even needed */
			
			if (!(Input->MyShape.Flags&SCREENSHAPEFLAGMASKDISABLE)) {
				Size = TempImage.Width*TempImage.Height;
				if (Size) {
					PalPtr = ShapePtr;
					Result = Size;
					do {
						if (!PalPtr[0]) {			/* Empty pixel... */
							Input->MyShape.Flags |= SCREENSHAPEFLAGMASK;
							break;
						}
						++PalPtr;
					} while (--Result);
					if (Input->MyShape.Flags & SCREENSHAPEFLAGMASK) {
						Result = Size;
						AlphaPtr = (Word8 *)AllocAPointer(Size);			/* Make the alpha channel */
						if (AlphaPtr) {
							TempImage.DataType = IMAGE8_PAL_ALPHA_PAL;	/* Change the data type */
							TempImage.AlphaPtr = AlphaPtr;
							PalPtr = AlphaPtr;
							do {
								Word Temp;
								Temp = 0;
								if (ShapePtr[0]) {			/* Valid pixel? */
									Temp = 0xFF;
								}
								PalPtr[0] = (Word8)Temp;		/* Save the channel */
								++ShapePtr;
								++PalPtr;
							} while (--Result);
						}
					}
				}
			}
			Result = ScreenShapeConvertFromImage(&Input->MyShape,&TempImage);
			DeallocAPointer(AlphaPtr);			/* Kill the alpha channel */
			ResourceRelease(Input->RezFile,Input->RezNum);
		}
	}
	return Result;
}

/**********************************

	Initialize a ScreenShapeGfx_t structure

**********************************/

void BURGERCALL ScreenShapeGfxInit(ScreenShapeGfx_t *Input,struct RezHeader_t *RezFile,Word RezNum)
{
	Input->RezFile = RezFile;
	Input->RezNum = RezNum;
	ScreenShapeInit(&Input->MyShape,(ScreenShapeActionProc)ScreenShapeGfxHandler);
}

/**********************************

	Create a new ScreenShapeGfx_t structure

**********************************/

ScreenShapeGfx_t *BURGERCALL ScreenShapeGfxNew(struct RezHeader_t *RezFile,Word RezNum)
{
	ScreenShapeGfx_t *Input;
	Input = (ScreenShapeGfx_t*)AllocAPointer(sizeof(ScreenShapeGfx_t));	/* Create the pointer */
	if (Input) {
		ScreenShapeGfxInit(Input,RezFile,RezNum);	/* Initialize it */
	}
	return Input;
}

/**********************************

	Reinitialize a ScreenShapeGfx structure

**********************************/

void BURGERCALL ScreenShapeGfxReinit(ScreenShapeGfx_t *Input,struct RezHeader_t *RezFile,Word RezNum)
{
	if (Input->RezFile != RezFile || Input->RezNum!=RezNum || !Input->MyShape.ActionProc) {	/* Different? */
		ScreenShapeDestroy(&Input->MyShape);					/* Discard any data */
		ScreenShapeGfxInit(Input,RezFile,RezNum);				/* Reset it */
	}
}

/**********************************

	This procedure handles all of the specific code for a ScreenShapeBmpFile_t
	structure

**********************************/

static Word BURGERCALL ScreenShapeGfxFileHandler(ScreenShapeGfxFile_t *Input)
{
	Word8 *PalPtr;
	Word8 *ShapePtr;
	Word8 *AlphaPtr;
	Word8 *DataPtr;
	Word Result;
	Result = TRUE;
	if (Input->FileName) {
		DataPtr = (Word8 *)LoadAFile(static_cast<char *>(LockAHandle((void **)Input->FileName)),0);
		UnlockAHandle((void **)Input->FileName);
		if (DataPtr) {
			Word Size;
			Image_t TempImage;
			PalPtr = DataPtr;
			ShapePtr = DataPtr+768;
#ifdef __BIGENDIAN__
			((Word16 *)ShapePtr)[0] = Burger::SwapEndian(((Word16 *)ShapePtr)[0]);
			((Word16 *)ShapePtr)[1] = Burger::SwapEndian(((Word16 *)ShapePtr)[1]);
			((Word16 *)ShapePtr)[2] = Burger::SwapEndian(((Word16 *)ShapePtr)[2]);
			((Word16 *)ShapePtr)[3] = Burger::SwapEndian(((Word16 *)ShapePtr)[3]);
#endif
			TempImage.Width = GetXShapeWidth(ShapePtr);
			TempImage.Height = GetXShapeHeight(ShapePtr);
			Input->MyShape.XOffset = GetXShapeXOffset(ShapePtr);
			Input->MyShape.YOffset = GetXShapeYOffset(ShapePtr);
			TempImage.DataType = IMAGE8_PAL;
			TempImage.RowBytes = TempImage.Width;
			ShapePtr += 8;
			TempImage.ImagePtr = ShapePtr;
			TempImage.PalettePtr = PalPtr;
		
			AlphaPtr = 0;			/* Assume no alpha channel */
			
			/* Check if an alpha channel is even needed */
			
			if (!(Input->MyShape.Flags&SCREENSHAPEFLAGMASKDISABLE)) {
				Size = TempImage.Width*TempImage.Height;
				if (Size) {
					PalPtr = ShapePtr;
					Result = Size;
					do {
						if (!PalPtr[0]) {			/* Empty pixel... */
							Input->MyShape.Flags |= SCREENSHAPEFLAGMASK;
							break;
						}
						++PalPtr;
					} while (--Result);
					if (Input->MyShape.Flags & SCREENSHAPEFLAGMASK) {
						Result = Size;
						AlphaPtr = (Word8 *)AllocAPointer(Size);			/* Make the alpha channel */
						if (AlphaPtr) {
							TempImage.DataType = IMAGE8_PAL_ALPHA_PAL;	/* Change the data type */
							TempImage.AlphaPtr = AlphaPtr;
							PalPtr = AlphaPtr;
							do {
								Word Temp;
								Temp = 0;
								if (ShapePtr[0]) {			/* Valid pixel? */
									Temp = 0xFF;
								}
								PalPtr[0] = (Word8)Temp;		/* Save the channel */
								++ShapePtr;
								++PalPtr;
							} while (--Result);
						}
					}
				}
			}
			Result = ScreenShapeConvertFromImage(&Input->MyShape,&TempImage);
			DeallocAPointer(AlphaPtr);			/* Kill the alpha channel */
			DeallocAPointer(DataPtr);
		}
	}
	return Result;
}

/**********************************

	Initialize a ScreenShapeGfxFile_t structure

**********************************/

void BURGERCALL ScreenShapeGfxFileInit(ScreenShapeGfxFile_t *Input,const char *FileName)
{
	Input->FileName = (char **)StrCopyHandle(FileName);
	ScreenShapeInit(&Input->MyShape,(ScreenShapeActionProc)ScreenShapeGfxFileHandler);
}

/**********************************

	Create a new ScreenShapeGfxFile_t structure

**********************************/

ScreenShapeGfxFile_t *BURGERCALL ScreenShapeGfxFileNew(const char *FileName)
{
	ScreenShapeGfxFile_t *Input;
	Input = (ScreenShapeGfxFile_t*)AllocAPointer(sizeof(ScreenShapeGfxFile_t));	/* Create the pointer */
	if (Input) {
		ScreenShapeGfxFileInit(Input,FileName);	/* Initialize it */
	}
	return Input;
}



/**********************************

	This procedure handles all of the specific code for a ScreenShapePtr_t
	structure

**********************************/

static Word BURGERCALL ScreenShapePtrHandler(ScreenShapePtr_t *Input)
{
	return ScreenShapeConvertFromImage(&Input->MyShape,Input->ImagePtr);
}

/**********************************

	Initialize a ScreenShapeRezGroup_t structure

**********************************/

void BURGERCALL ScreenShapePtrInit(ScreenShapePtr_t *Input,struct Image_t *ImagePtr)
{
	Input->ImagePtr = ImagePtr;
	ScreenShapeInit(&Input->MyShape,(ScreenShapeActionProc)ScreenShapePtrHandler);
}

/**********************************

	Create a new ScreenShapePtr_t structure

**********************************/

ScreenShapePtr_t *BURGERCALL ScreenShapePtrNew(struct Image_t *ImagePtr)
{
	ScreenShapePtr_t *Input;
	Input = (ScreenShapePtr_t*)AllocAPointer(sizeof(ScreenShapePtr_t));	/* Create the pointer */
	if (Input) {
		ScreenShapePtrInit(Input,ImagePtr);	/* Initialize it */
	}
	return Input;
}


/**********************************

	This procedure handles all of the specific code for a ScreenShapeRezGroup_t
	structure

**********************************/

static Word BURGERCALL ScreenShapeRezGroupHandler(ScreenShapeRezGroup_t *Input)
{
	Word8 *PalPtr;
	Word8 *ShapePtr;
	Word Result;
	Result = TRUE;
	if (Input->RezFile) {
		PalPtr = (Word8 *)ResourceLoad(Input->RezFile,Input->RezPal);
		if (PalPtr) {
			ShapePtr = (Word8 *)ResourceLoadXShapeArray(Input->RezFile,Input->RezNum);
			if (ShapePtr) {
				Image_t TempImage;
				ShapePtr = GetShapeIndexPtr(ShapePtr,Input->Which);
				TempImage.Width = GetXShapeWidth(ShapePtr);
				TempImage.Height = GetXShapeHeight(ShapePtr);
				Input->MyShape.XOffset = GetXShapeXOffset(ShapePtr);
				Input->MyShape.YOffset = GetXShapeYOffset(ShapePtr);
				TempImage.DataType = IMAGE8_PAL;
				TempImage.RowBytes = TempImage.Width;
				TempImage.ImagePtr = ShapePtr+8;
				TempImage.PalettePtr = PalPtr;
				Result = ScreenShapeConvertFromImage(&Input->MyShape,&TempImage);
				ResourceRelease(Input->RezFile,Input->RezNum);
			}
			ResourceRelease(Input->RezFile,Input->RezPal);
		}
	}
	return Result;		/* Assume error! */
}

/**********************************

	Initialize a ScreenShapeRezGroup_t structure

**********************************/

void BURGERCALL ScreenShapeRezGroupInit(ScreenShapeRezGroup_t *Input,struct RezHeader_t *RezFile,Word RezNum,Word RezPal,Word Which)
{
	Input->RezFile = RezFile;
	Input->RezNum = RezNum;
	Input->RezPal = RezPal;
	Input->Which = Which;
	ScreenShapeInit(&Input->MyShape,(ScreenShapeActionProc)ScreenShapeRezGroupHandler);
}

/**********************************

	Create a new ScreenShapeRezGroup_t structure

**********************************/

ScreenShapeRezGroup_t *BURGERCALL ScreenShapeRezGroupNew(struct RezHeader_t *RezFile,Word RezNum,Word RezPal,Word Which)
{
	ScreenShapeRezGroup_t *Input;
	Input = (ScreenShapeRezGroup_t*)AllocAPointer(sizeof(ScreenShapeRezGroup_t));	/* Create the pointer */
	if (Input) {
		ScreenShapeRezGroupInit(Input,RezFile,RezNum,RezPal,Which);	/* Initialize it */
	}
	return Input;
}

/**********************************

	This procedure handles all of the specific code for a ScreenShapeRezGroup_t
	structure

**********************************/

static Word BURGERCALL ScreenShapeGifFileHandler(ScreenShapeGifFile_t *Input)
{
	void *DataPtr;						/* Pointer to loaded file */
	if (Input->FileName) {
		DataPtr = LoadAFile((char *)LockAHandle((void **)Input->FileName),0);		/* Load in the file */
		UnlockAHandle((void **)Input->FileName);									/* Unlock the filename */
		if (DataPtr) {
			Image_t TempImage;				/* Work image */
			Word Result;					/* Temp error code */

			Result = ImageParseGIF(&TempImage,(Word8 *)DataPtr);		/* Valid GIF file? */
			DeallocAPointer(DataPtr);								/* Discard the file */
			if (!Result) {
				Input->MyShape.XOffset = 0;							/* Store the offsets */
				Input->MyShape.YOffset = 0;
				Result = ScreenShapeConvertFromImage(&Input->MyShape,&TempImage);	/* Convert the image */
				ImageDestroy(&TempImage);							/* Dispose of the shape */
				return Result;				/* Return the error code from ConvertFromImage() */
			}
		}
	}
	return TRUE;			/* Assume error! */
}

/**********************************

	Initialize a ScreenShapeGifFile_t structure

**********************************/

void BURGERCALL ScreenShapeGifFileInit(ScreenShapeGifFile_t *Input,const char *FileName)
{
	Input->FileName = (char **)StrCopyHandle(FileName);
	ScreenShapeInit(&Input->MyShape,(ScreenShapeActionProc)ScreenShapeGifFileHandler);
}

/**********************************

	Create a new ScreenShapeGifFile_t structure

**********************************/

ScreenShapeGifFile_t *BURGERCALL ScreenShapeGifFileNew(const char *FileName)
{
	ScreenShapeGifFile_t *Input;
	Input = (ScreenShapeGifFile_t*)AllocAPointer(sizeof(ScreenShapeGifFile_t));	/* Create the pointer */
	if (Input) {
		ScreenShapeGifFileInit(Input,FileName);	/* Initialize it */
	}
	return Input;
}

/**********************************

	This procedure handles all of the specific code for a ScreenShapeGif_t
	structure

**********************************/

static Word BURGERCALL ScreenShapeGifHandler(ScreenShapeGif_t *Input)
{
	void *DataPtr;						/* Pointer to loaded file */
	if (Input->RezFile) {
		DataPtr = ResourceLoad(Input->RezFile,Input->RezNum);		/* Load in the file */
		if (DataPtr) {
			Image_t TempImage;				/* Work image */
			Word Result;					/* Temp error code */

			Result = ImageParseGIF(&TempImage,(Word8 *)DataPtr);		/* Valid GIF file? */
			ResourceRelease(Input->RezFile,Input->RezNum);
			if (!Result) {
				Input->MyShape.XOffset = 0;							/* Store the offsets */
				Input->MyShape.YOffset = 0;
				Result = ScreenShapeConvertFromImage(&Input->MyShape,&TempImage);	/* Convert the image */
				ImageDestroy(&TempImage);							/* Dispose of the shape */
				return Result;				/* Return the error code from ConvertFromImage() */
			}
		}
	}
	return TRUE;			/* Assume error! */
}

/**********************************

	Initialize a ScreenShapeGif_t structure

**********************************/

void BURGERCALL ScreenShapeGifInit(ScreenShapeGif_t *Input,struct RezHeader_t *RezFile,Word RezNum)
{
	Input->RezFile = RezFile;
	Input->RezNum = RezNum;
	ScreenShapeInit(&Input->MyShape,(ScreenShapeActionProc)ScreenShapeGifHandler);
}

/**********************************

	Create a new ScreenShapeGif_t structure

**********************************/

ScreenShapeGif_t *BURGERCALL ScreenShapeGifNew(struct RezHeader_t *RezFile,Word RezNum)
{
	ScreenShapeGif_t *Input;
	Input = (ScreenShapeGif_t*)AllocAPointer(sizeof(ScreenShapeGif_t));	/* Create the pointer */
	if (Input) {
		ScreenShapeGifInit(Input,RezFile,RezNum);	/* Initialize it */
	}
	return Input;
}

/**********************************

	Reinitialize a ScreenShapeGif_t structure

**********************************/

void BURGERCALL ScreenShapeGifReinit(ScreenShapeGif_t *Input,struct RezHeader_t *RezFile,Word RezNum)
{
	if (Input->RezFile != RezFile || Input->RezNum!=RezNum || !Input->MyShape.ActionProc) {	/* Different? */
		ScreenShapeDestroy(&Input->MyShape);					/* Discard any data */
		ScreenShapeGifInit(Input,RezFile,RezNum);				/* Reset it */
	}
}

/**********************************

	This procedure handles all of the specific code for a ScreenShapeBmpFile_t
	structure

**********************************/

static Word BURGERCALL ScreenShapeBmpFileHandler(ScreenShapeBmpFile_t *Input)
{
	void *DataPtr;						/* Pointer to loaded file */
	if (Input->FileName) {
		DataPtr = LoadAFile((char *)LockAHandle((void **)Input->FileName),0);		/* Load in the file */
		UnlockAHandle((void **)Input->FileName);									/* Unlock the filename */
		if (DataPtr) {
			Image_t TempImage;				/* Work image */
			Word Result;					/* Temp error code */

			Result = ImageParseBMP(&TempImage,(Word8 *)DataPtr);		/* Valid BMP file? */
			DeallocAPointer(DataPtr);								/* Discard the file */
			if (!Result) {
				Input->MyShape.XOffset = 0;							/* Store the offsets */
				Input->MyShape.YOffset = 0;
				Result = ScreenShapeConvertFromImage(&Input->MyShape,&TempImage);	/* Convert the image */
				ImageDestroy(&TempImage);							/* Dispose of the shape */
				return Result;				/* Return the error code from ConvertFromImage() */
			}
		}
	}
	return TRUE;			/* Assume error! */
}

/**********************************

	Initialize a ScreenShapeBmpFile_t structure

**********************************/

void BURGERCALL ScreenShapeBmpFileInit(ScreenShapeBmpFile_t *Input,const char *FileName)
{
	Input->FileName = (char **)StrCopyHandle(FileName);
	ScreenShapeInit(&Input->MyShape,(ScreenShapeActionProc)ScreenShapeBmpFileHandler);
}

/**********************************

	Create a new ScreenShapeBmpFile_t structure

**********************************/

ScreenShapeBmpFile_t *BURGERCALL ScreenShapeBmpFileNew(const char *FileName)
{
	ScreenShapeBmpFile_t *Input;
	Input = (ScreenShapeBmpFile_t*)AllocAPointer(sizeof(ScreenShapeBmpFile_t));	/* Create the pointer */
	if (Input) {
		ScreenShapeBmpFileInit(Input,FileName);	/* Initialize it */
	}
	return Input;
}

/**********************************

	This procedure handles all of the specific code for a ScreenShapeBmp_t
	structure

**********************************/

static Word BURGERCALL ScreenShapeBmpHandler(ScreenShapeBmp_t *Input)
{
	void *DataPtr;						/* Pointer to loaded file */
	if (Input->RezFile) {
		DataPtr = ResourceLoad(Input->RezFile,Input->RezNum);		/* Load in the file */
		if (DataPtr) {
			Image_t TempImage;				/* Work image */
			Word Result;					/* Temp error code */

			Result = ImageParseBMP(&TempImage,(Word8 *)DataPtr);		/* Valid GIF file? */
			ResourceRelease(Input->RezFile,Input->RezNum);
			if (!Result) {
				Input->MyShape.XOffset = 0;							/* Store the offsets */
				Input->MyShape.YOffset = 0;
				Result = ScreenShapeConvertFromImage(&Input->MyShape,&TempImage);	/* Convert the image */
				ImageDestroy(&TempImage);							/* Dispose of the shape */
				return Result;				/* Return the error code from ConvertFromImage() */
			}
		}
	}
	return TRUE;			/* Assume error! */
}

/**********************************

	Initialize a ScreenShapeBmp_t structure

**********************************/

void BURGERCALL ScreenShapeBmpInit(ScreenShapeBmp_t *Input,struct RezHeader_t *RezFile,Word RezNum)
{
	Input->RezFile = RezFile;
	Input->RezNum = RezNum;
	ScreenShapeInit(&Input->MyShape,(ScreenShapeActionProc)ScreenShapeBmpHandler);
}

/**********************************

	Create a new ScreenShapeBmp_t structure

**********************************/

ScreenShapeBmp_t *BURGERCALL ScreenShapeBmpNew(struct RezHeader_t *RezFile,Word RezNum)
{
	ScreenShapeBmp_t *Input;
	Input = (ScreenShapeBmp_t*)AllocAPointer(sizeof(ScreenShapeBmp_t));	/* Create the pointer */
	if (Input) {
		ScreenShapeBmpInit(Input,RezFile,RezNum);	/* Initialize it */
	}
	return Input;
}

/**********************************

	Reinitialize a ScreenShapeBmp_t structure

**********************************/

void BURGERCALL ScreenShapeBmpReinit(ScreenShapeBmp_t *Input,struct RezHeader_t *RezFile,Word RezNum)
{
	if (Input->RezFile != RezFile || Input->RezNum!=RezNum || !Input->MyShape.ActionProc) {	/* Different? */
		ScreenShapeDestroy(&Input->MyShape);					/* Discard any data */
		ScreenShapeBmpInit(Input,RezFile,RezNum);				/* Reset it */
	}
}

