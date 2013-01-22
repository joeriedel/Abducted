/**********************************

	Generic pixel blitter

**********************************/

#ifndef __SSSCREENSHAPE_H__
#define __SSSCREENSHAPE_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __GRGRAPHICS_H__
#include "GrGraphics.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct LBPoint;

enum {
	SCREENSHAPEMODESOFTWARE,
	SCREENSHAPEMODEOPENGL,
	SCREENSHAPEMODEDIRECT3D,
	SCREENSHAPEMODEDIRECTDRAW
};

#define SCREENSHAPEFLAGMODEMASK 0x0F
#define SCREENSHAPEFLAGMASK 0x010
#define SCREENSHAPEFLAGCOMPRESSED 0x020
#define SCREENSHAPEFLAGPURGEABLE 0x040
#define SCREENSHAPEFLAGMASKDISABLE 0x8000

typedef enum {
	TRANSLUCENCYMODE_OFF,
	TRANSLUCENCYMODE_NORMAL,
	TRANSLUCENCYMODE_INVCOLOR,
	TRANSLUCENCYMODE_COLOR,
	TRANSLUCENCYMODE_GLOWING,
	TRANSLUCENCYMODE_DARKENINGCOLOR,
	TRANSLUCENCYMODE_JUSTSETZ,
	TRANSLUCENCYMODE_BAD=0x70000000
} TranslucencyMode_e;

typedef enum {
	FILTERINGMODE_OFF,
	FILTERINGMODE_BILINEAR,
	FILTERINGMODE_BAD=0x70000000
} FilteringMode_e;

typedef enum {
	SHADINGMODE_FLAT,
	SHADINGMODE_GOURAUD,
	SHADINGMODE_PHONG,
	SHADINGMODE_BAD=0x70000000
} ShadingMode_e;

typedef enum {
	DEPTHTESTMODE_NEVER,
	DEPTHTESTMODE_LESS,
	DEPTHTESTMODE_EQUAL,
	DEPTHTESTMODE_LESSEQUAL,
	DEPTHTESTMODE_GREATER,
	DEPTHTESTMODE_NOTEQUAL,
	DEPTHTESTMODE_GREATEREQUAL,
	DEPTHTESTMODE_ALWAYS,
	DEPTHTESTMODE_BAD=0x70000000
} DepthTestMode_e;

struct ScreenShape_t;
typedef Word (BURGERCALL * ScreenShapeActionProc)(struct ScreenShape_t *);
typedef Word (BURGERCALL * ScreenShapeDrawProc)(struct ScreenShape_t* screen_shape, const struct LBRect* dest_rect );

typedef struct ScreenShape_t {	/* Root data class for shapes */
	void *Data1;			/* Reference to data (Usually a handle) */
	void *Data2;			/* Reference to data (Usually a texture ref) */
	void *Data3;			/* Extra reference */
	ScreenShapeActionProc ActionProc;	/* How to load this data? */
	ScreenShapeDrawProc DrawProc;	/* This can be NULL, in which case, blib automatically handles it based on screen mode */
	Word Flags;				/* Is color zero present? */
	Word Width,Height;		/* Size of the shape in pixels */
	int XOffset,YOffset;	/* X/Y adjust for drawing shape */
} ScreenShape_t;

typedef struct ScreenShapeRez_t {	/* Class for loading a shape from a rez file */
	ScreenShape_t MyShape;			/* Root class */
	struct RezHeader_t *RezFile;	/* Master resource reference */
	Word RezNum;					/* Resource number for the shape group to get info from */
	Word RezPal;					/* Palette for resource */
} ScreenShapeRez_t;

typedef struct ScreenShapeGfx_t {	/* Class for loading a Gfx shape from a rez file */
	ScreenShape_t MyShape;			/* Root class */
	struct RezHeader_t *RezFile;	/* Master resource reference */
	Word RezNum;					/* Resource number for the shape */
} ScreenShapeGfx_t;

typedef struct ScreenShapeGfxFile_t {	/* Class for loading a Gfx shape from a file */
	ScreenShape_t MyShape;			/* Root class */
	char **FileName;				/* Handle to filename */
} ScreenShapeGfxFile_t;

typedef struct ScreenShapePtr_t {	/* Class for loading a shape from a pointer */
	ScreenShape_t MyShape;			/* Root class */
	struct Image_t *ImagePtr;		/* Pointer to the shape */
} ScreenShapePtr_t;

typedef struct ScreenShapeRezGroup_t {	/* Class for loading from a group */
	ScreenShape_t MyShape;			/* Root class */
	struct RezHeader_t *RezFile;	/* Master resource reference */
	Word RezNum;					/* Resource number for the shape group to get info from */
	Word RezPal;					/* Palette for resource */
	Word Which;						/* Which shape in the group */
} ScreenShapeRezGroup_t;

typedef struct ScreenShapeGifFile_t {	/* Class for loading from a gif file */
	ScreenShape_t MyShape;			/* Root class */
	char **FileName;				/* Handle to filename */
} ScreenShapeGifFile_t;

typedef struct ScreenShapeGif_t {	/* Class for loading from a gif file */
	ScreenShape_t MyShape;			/* Root class */
	struct RezHeader_t *RezFile;	/* Master resource reference */
	Word RezNum;					/* Resource number for the shape */
} ScreenShapeGif_t;

typedef struct ScreenShapeBmpFile_t {	/* Class for loading from a bmp file */
	ScreenShape_t MyShape;			/* Root class */
	char **FileName;				/* Handle to filename */
} ScreenShapeBmpFile_t;

typedef struct ScreenShapeBmp_t {	/* Class for loading from a group */
	ScreenShape_t MyShape;			/* Root class */
	struct RezHeader_t *RezFile;	/* Master resource reference */
	Word RezNum;					/* Resource number for the shape */
} ScreenShapeBmp_t;

typedef void (BURGERCALL *ScreenShapeInitProcPtr)(void);
typedef void (BURGERCALL *ScreenShapeSolidRectProcPtr)(int,int,Word,Word,Word32);

extern Word ScreenAPI;			/* Which API am I using (Direct3D, OpenGL?) */
extern TranslucencyMode_e ScreenTranslucencyMode;		/* Current 3D translucency mode */
extern FilteringMode_e ScreenFilteringMode;				/* Current texture filtering mode */
extern ShadingMode_e ScreenShadingMode;					/* Current shading mode */
extern DepthTestMode_e ScreenDepthTestMode;				/* Type of ZBuffer test */
extern Word ScreenDepthWriteMode;						/* Write to the ZBuffer? */
extern Word ScreenPerspectiveMode;						/* Perspective correct mode active? */
extern Word ScreenWireFrameMode;						/* Are polygons wireframed? */
extern Word ScreenBlendMode;							/* Last alpha mode */
extern Word ScreenUsing2DCoords;						/* Whether 2D drawing is currently on */
extern Word32 ScreenCurrentTexture;					/* Last texture found */
extern ScreenShapeInitProcPtr ScreenInit;									/* Init the 3D context */
extern ScreenShapeSolidRectProcPtr ScreenSolidRect;		/* Draw a solid rect */

extern void BURGERCALL ScreenInitAPI(Word APIType);
extern Word BURGERCALL ScreenSetDisplayToSize(Word Width,Word Height,Word Depth,Word Flags);
extern void BURGERCALL ScreenBeginScene(void);
extern void BURGERCALL ScreenEndScene(void);
#define ScreenSetTranslucencyMode(x) if (ScreenTranslucencyMode!=(x)) { ScreenForceTranslucencyMode(x); }
extern void BURGERCALL ScreenForceTranslucencyMode(TranslucencyMode_e NewMode);
#define ScreenSetFilteringMode(x) if (ScreenFilteringMode!=(x)) { ScreenForceFilteringMode(x); }
extern void BURGERCALL ScreenForceFilteringMode(FilteringMode_e NewMode);
#define ScreenSetWireFrameMode(x) if (ScreenWireFrameMode!=(x)) { ScreenForceWireFrameMode(x); }
extern void BURGERCALL ScreenForceWireFrameMode(Word Flag);
#define ScreenSetTexture(x) if (ScreenCurrentTexture!=(x)) { ScreenForceTexture(x); }
extern void BURGERCALL ScreenForceTexture(Word32 TexNum);
#define ScreenSetPerspective(x) if (ScreenPerspectiveMode!=(x)) { ScreenForcePerspective(x); }
extern void BURGERCALL ScreenForcePerspective(Word Flag);
#define ScreenSetShadingMode(x) if (ScreenShadingMode!=(x)) { ScreenForceShadingMode(x); }
extern void BURGERCALL ScreenForceShadingMode(ShadingMode_e NewMode);
#define ScreenSetDepthWriteMode(x) if (ScreenDepthWriteMode!=(x)) { ScreenForceDepthWriteMode(x); }
extern void BURGERCALL ScreenForceDepthWriteMode(Word Flag);
#define ScreenSetDepthTestMode(x) if (ScreenDepthTestMode!=(x)) { ScreenForceDepthTestMode(x); }
extern void BURGERCALL ScreenForceDepthTestMode(DepthTestMode_e NewMode);
#define ScreenUse2DCoords(x) if (ScreenUsing2DCoords!=(x)) { ScreenForceUse2DCoords(x); }
extern void BURGERCALL ScreenForceUse2DCoords( Word use2d );

extern void BURGERCALL ScreenShapeInit(ScreenShape_t *Input,ScreenShapeActionProc ActionProc);
extern ScreenShape_t *BURGERCALL ScreenShapeNew(ScreenShapeActionProc ActionProc);
extern void BURGERCALL ScreenShapeDestroy(ScreenShape_t *Input);
extern void BURGERCALL ScreenShapeDelete(ScreenShape_t *Input);
extern void BURGERCALL ScreenShapePurge(ScreenShape_t *Input);
extern void BURGERCALL ScreenShapeDraw(ScreenShape_t *Input,int x,int y);
extern void BURGERCALL ScreenShapeDrawScaled(ScreenShape_t *Input,const struct LBRect *DestRect);
extern void BURGERCALL ScreenShapeDrawScaledSubRect(ScreenShape_t *Input,const struct LBRect *DestRect,const struct LBRect *SrcRect);
extern void BURGERCALL ScreenShapeLock(ScreenShape_t *Input,struct Image_t *Output);
extern void BURGERCALL ScreenShapeUnlock(ScreenShape_t *Input);
extern Word BURGERCALL ScreenShapeLoad(ScreenShape_t *Input);
extern void BURGERCALL ScreenShapeDisallowPurge(ScreenShape_t *Input);
extern void BURGERCALL ScreenShapeAllowPurge(ScreenShape_t *Input);
extern void BURGERCALL ScreenShapeGetBounds(ScreenShape_t *Input,struct LBRect *Bounds);
extern void BURGERCALL ScreenShapeGetSize(ScreenShape_t *Input,struct LBPoint *Size);
extern Word BURGERCALL ScreenShapeGetPixel(ScreenShape_t *Input,int x,int y);
extern int BURGERCALL ScreenShapeVPatternBar(ScreenShape_t **ArrayPtr,int x,int TopY,int BottomY);
extern int BURGERCALL ScreenShapeHPatternBar(ScreenShape_t **ArrayPtr,int y,int LeftX,int RightX);
extern Word BURGERCALL ScreenShapeConvertFromImage(ScreenShape_t *Input,const struct Image_t *ImagePtr);

extern void BURGERCALL ScreenShapeGfxInit(ScreenShapeGfx_t *Input,struct RezHeader_t *RezFile,Word RezNum);
extern ScreenShapeGfx_t *BURGERCALL ScreenShapeGfxNew(struct RezHeader_t *RezFile,Word RezNum);
extern void BURGERCALL ScreenShapeGfxReinit(ScreenShapeGfx_t *Input,struct RezHeader_t *RezFile,Word RezNum);

extern void BURGERCALL ScreenShapeGfxFileInit(ScreenShapeGfxFile_t *Input,const char *FileName);
extern ScreenShapeGfxFile_t *BURGERCALL ScreenShapeGfxFileNew(const char *FileName);

extern void BURGERCALL ScreenShapePtrInit(ScreenShapePtr_t *Input,struct Image_t *ImagePtr);
extern ScreenShapePtr_t *BURGERCALL ScreenShapePtrNew(struct Image_t *ImagePtr);

extern void BURGERCALL ScreenShapeRezGroupInit(ScreenShapeRezGroup_t *Input,struct RezHeader_t *RezFile,Word RezNum,Word RezPal,Word Which);
extern ScreenShapeRezGroup_t *BURGERCALL ScreenShapeRezGroupNew(struct RezHeader_t *RezFile,Word RezNum,Word RezPal,Word Which);

extern void BURGERCALL ScreenShapeGifFileInit(ScreenShapeGifFile_t *Input,const char *FileName);
extern ScreenShapeGifFile_t *BURGERCALL ScreenShapeGifFileNew(const char *FileName);

extern void BURGERCALL ScreenShapeGifInit(ScreenShapeGif_t *Input,struct RezHeader_t *RezFile,Word RezNum);
extern ScreenShapeGif_t *BURGERCALL ScreenShapeGifNew(struct RezHeader_t *RezFile,Word RezNum);
extern void BURGERCALL ScreenShapeGifReinit(ScreenShapeGif_t *Input,struct RezHeader_t *RezFile,Word RezNum);

extern void BURGERCALL ScreenShapeBmpFileInit(ScreenShapeBmpFile_t *Input,const char *FileName);
extern ScreenShapeBmpFile_t *BURGERCALL ScreenShapeBmpFileNew(const char *FileName);

extern void BURGERCALL ScreenShapeBmpInit(ScreenShapeBmp_t *Input,struct RezHeader_t *RezFile,Word RezNum);
extern ScreenShapeBmp_t *BURGERCALL ScreenShapeBmpNew(struct RezHeader_t *RezFile,Word RezNum);
extern void BURGERCALL ScreenShapeBmpReinit(ScreenShapeBmp_t *Input,struct RezHeader_t *RezFile,Word RezNum);

#ifdef __cplusplus
}
#endif

#endif
