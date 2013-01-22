/*******************************

	Graphics manager

*******************************/

#ifndef __GRGRAPHICS_H__
#define __GRGRAPHICS_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct LBRect;		/* Unix compilers want this, sigh */
struct RezHeader_t;
struct Image_t;

/* In Graphics */

#if defined(__MAC__)
#define BLACK 255
#define WHITE 0
#else
#define BLACK 0
#define WHITE 255
#endif

#define SETDISPLAYWINDOW 0x0				/* Best for debugging */
#define SETDISPLAYFULLSCREEN 0x1			/* True if full screen */
#define SETDISPLAYOFFSCREEN 0x2				/* True if I force system memory video page */
#define SETDISPLAYDOUBLEOK 0x4				/* True if I can allow pixel doubling for 320x200 */
#define SETDISPLAYOPENGL 0x8				/* True if 3D hardware is requested (Check Video3DPreference for API override) */
#define SETDISPLAYD3D 0x10					/* True if 3D hardware is requested (Check Video3DPreference for API override) */
#define SETDISPLAYNOWINDOWTWIDDLE 0x20		/* Don't change any of my window settings for me */

#define VIDEOMODEOPENGL 0x01		/* OpenGL is supported in this mode */
#define VIDEOMODEHARDWARE 0x02		/* DirectX or other native support */
#define VIDEOMODEFAKE 0x04			/* This is a pixel double mode */

#define VIDEOAPISOFTWARE 0
#define VIDEOAPIOPENGL 1
#define VIDEOAPIDIRECT3D 2
#define VIDEOAPIDIRECTDRAW 3

#define VIDEOMODEGETNOFAKE 0x0100	/* Don't allow pixel double modes */

/* Texture formats supported */
#define VIDEOTEXTURETYPE555 0x0001
#define VIDEOTEXTURETYPE565 0x0002
#define VIDEOTEXTURETYPE4444 0x0004
#define VIDEOTEXTURETYPE1555 0x0008
#define VIDEOTEXTURETYPE8PAL 0x0010
#define VIDEOTEXTURETYPE888 0x0020
#define VIDEOTEXTURETYPE8888 0x0040

#define VIDEOTEXTURESQUARE 0x0001		/* 3D Textures MUST be square */
#define VIDEOTEXTUREPOW2 0x0002			/* 3D Textures MUST be a power of 2 in size */
#define VIDEOTEXTURESYSTEMMEMORY 0x0004	/* Textures can exist in system memory (AGP) */

typedef struct LWShape_t {
	Word16 Width;		/* Width of the shape */
	Word16 Height;		/* Height of the shape */
	Word8 Data[1];		/* Raw shape data */
} LWShape_t;

typedef struct LWXShape_t {
	short XOffset;		/* Signed offset for x */
	short YOffset;		/* Signed offset for y */
	LWShape_t Shape;	/* Shape data */
} LWXShape_t;

typedef struct GfxShape_t {
	Word8 Palette[768];	/* Palette for the shape */
	LWXShape_t XShape;	/* XShape for the actual data */
} GfxShape_t;

typedef struct VideoMode_t {
	Word Width;		/* Width of video mode */
	Word Height;	/* Height of video mode */
	Word Depth;		/* Depth of video mode */
	Word Hertz;		/* Video scan rate (0 if not supported) */
	Word Flags;		/* Hardware/OpenGL */
} VideoMode_t;

typedef struct VideoModeArray_t {
	Word Count;				/* Number of modes in the array */
	Word DevNumber;			/* Device number */
	char DeviceName[64];	/* Name of the device */
	VideoMode_t Array[1];	/* Array of modes */
} VideoModeArray_t;

typedef struct VideoDeviceArray_t {
	Word Count;						/* Number of video cards present */
	VideoModeArray_t **Array[1];	/* Modes present (Handles) */
} VideoDeviceArray_t;

typedef struct VideoSaveState_t {
	Word Width;		/* Width of the current screen */
	Word Height;	/* Height of the current screen */
	Word Depth;		/* BIt depth of the current screen */
	Word Flags;		/* Special flags for the current mode */
} VideoSaveState_t;

typedef struct GammaTable_t {
	float Red[256];		/* Red gamma 0.0 = min, 1.0 = max */
	float Green[256];	/* Green gamma */
	float Blue[256];	/* Blue gamma */
} GammaTable_t;

typedef void (BURGERCALL *DrawARectProc)(int x,int y,Word Width,Word Height,Word Color);
typedef void (BURGERCALL *DrawALineProc)(int x1,int y1,int x2,int y2,Word Color);
typedef void (BURGERCALL *DrawAPixelProc)(int x,int y,Word Color);
typedef void (BURGERCALL *DrawARectRemapProc)(int x,int y,Word Width,Word Height,const void *RemapPtr);
typedef Word (BURGERCALL *VideoModePurgeProc)(VideoMode_t *);

extern Word Table8To16[256];		/* 8 bit to 16 bit conversion table */
extern Word *Table8To16Ptr;			/* Pointer to the draw table to use */
extern Word8 *VideoPointer;			/* Pointer to current video buffer */
extern Word VideoWidth;				/* Bytes per scan line */
extern Word ScreenClipLeft;			/* Clip left */
extern Word ScreenClipTop;			/* Clip top */
extern Word ScreenClipRight;		/* Clip right */
extern Word ScreenClipBottom;		/* Clip bottom */
extern Word ScreenWidth;			/* Clip width in pixels */
extern Word ScreenHeight;			/* Clip height in pixels */
extern Word ScreenFlags;			/* Current flags used for this mode */
extern Word VideoRedShift;			/* Bits to shift for Red */
extern Word VideoGreenShift;		/* Bits to shift for Green */
extern Word VideoBlueShift;			/* Bits to shift for Blue */
extern Word VideoRedMask;			/* Bitmask for Red */
extern Word VideoGreenMask;			/* Bitmask for Green */
extern Word VideoBlueMask;			/* Bitmask for Blue */
extern Word VideoRedBits;			/* Number of bits of Red */
extern Word VideoGreenBits;			/* Number of bits of Green */
extern Word VideoBlueBits;			/* Number of bits of Blue */
extern Word VideoColorDepth;		/* TRUE if palette doesn't exist */
extern Word VideoTrueScreenWidth;	/* Width in PIXELS of the video display */
extern Word VideoTrueScreenHeight;	/* Width in PIXELS of the video display */
extern Word VideoTrueScreenDepth;	/* Depth in BITS of the video display */
extern Word VideoPixelDoubleMode;	/* Set to the mode requested 0-3 */
extern Word VideoFullScreen; 		/* TRUE if full screen mode is active */
extern Word VideoPageLocked;		/* True if the video memory is locked */
extern Word8 *VideoOffscreen;		/* Pointer to offscreen buffer if used */
extern Word VideoHardWidth;			/* Width in BYTES of a video scan line */
extern Word VideoAPIInUse;			/* True if OpenGL is present and active */
extern Word VideoTextureTypesAllowed;	/* Texture formats I can support */
extern Word VideoTextureRules;			/* Special rules I have to follow */
extern Word VideoTextureMinWidth;		/* Minimum texture size */
extern Word VideoTextureMaxWidth;		/* Maximum texture size */
extern Word VideoTextureMinHeight;		/* Minimum texture height */
extern Word VideoTextureMaxHeight;		/* Maximum texture height */
extern Word VideoVertexMaxCount;		/* Maximum number of vertexs */
extern Word VideoVertexCount;			/* Number of vertex's processed */
extern Word VideoUseColorZero;			/* TRUE if Color #0 is used in textures */
extern DrawARectProc ScreenRect;	/* Draw a rect intercept vector */
extern DrawALineProc ScreenLine;	/* Draw a line intercept vector */
extern DrawAPixelProc ScreenPixel;	/* Draw a pixel intercept vector */
extern DrawARectRemapProc ScreenRectRemap;	/* Remap a rect of pixels */

extern Word BurgerMaxVideoPage;		/* Maximum number of video pages */
extern Word BurgerVideoPage;		/* Currently using this page */
extern Word BurgerVesaVersion;		/* 0 = No VESA, 1 = 1.2, 2 = 2.0 */
extern Bool Burger8BitPalette;	/* TRUE if 8 bit palettes are supported */
extern Word32 BurgerScreenSize;	/* Size in BYTES of the offscreen bitmap */
extern Word8 *BurgerBaseVideoPointer;	/* Pointer to base video memory range */
extern Word8 *BurgerVideoPointers[3];	/* Pointers to each video page */
extern Word8 *BurgerVideoCallbackBuffer;	/* Pointer to VESA callback code */
extern Bool BurgerLinearFrameBuffer;	/* TRUE if hardware is linear */

#define SetNewVideoWidth(x) VideoWidth=x
extern void BURGERCALL ReleaseVideo(void);
extern Word BURGERCALL SetDisplayToSize(Word Width,Word Height,Word Depth,Word Flags);
extern void BURGERCALL UpdateAndPageFlip(void);
extern void BURGERCALL UpdateAndNoPageFlip(void);
extern void BURGERCALL LockVideoPage(void);
extern void BURGERCALL LockFrontVideoPage(void);
extern void BURGERCALL VideoSetWindowString(const char *Title);
extern VideoModeArray_t **BURGERCALL VideoModeArrayNew(Word Flags);
#define VideoModeArrayDelete(x) DeallocAHandle((void **)x)
extern VideoDeviceArray_t **BURGERCALL VideoDeviceArrayNew(Word Flags);
extern void BURGERCALL VideoDeviceArrayDelete(VideoDeviceArray_t **Input);
extern VideoModeArray_t ** BURGERCALL VideoModeArrayPurge(VideoModeArray_t **Input,VideoModePurgeProc Proc);
extern VideoDeviceArray_t ** BURGERCALL VideoDeviceArrayPurge(VideoDeviceArray_t **Input,VideoModePurgeProc Proc);
extern void BURGERCALL VideoGetCurrentMode(VideoSaveState_t *Input);
extern Word BURGERCALL VideoSetMode(const VideoSaveState_t *Input);
extern void BURGERCALL UnlockVideoPage(void);
extern void BURGERCALL Video555To565(void);

extern void BURGERCALL VideoOSGammaInit(void);
extern void BURGERCALL VideoOSGammaDestroy(void);
extern void BURGERCALL VideoOSGammaAdjust(Fixed32 Intensity);
extern void BURGERCALL VideoOSGammaSet(const GammaTable_t *TablePtr);
extern void BURGERCALL VideoOSGammaGet(GammaTable_t *TablePtr);

#define GetShapeWidth(ShapePtr) (((LWShape_t *)ShapePtr)->Width)
#define GetShapeHeight(ShapePtr) (((LWShape_t *)ShapePtr)->Height)
#define GetXShapeXOffset(ShapePtr) (((LWXShape_t *)ShapePtr)->XOffset)
#define GetXShapeYOffset(ShapePtr) (((LWXShape_t *)ShapePtr)->YOffset)
#define GetXShapeWidth(ShapePtr) (((LWXShape_t *)ShapePtr)->Shape.Width)
#define GetXShapeHeight(ShapePtr) (((LWXShape_t *)ShapePtr)->Shape.Height)
#define GetShapeIndexPtr(ShapeArrayPtr,Index) (&((Word8 *)ShapeArrayPtr)[((Word32 *)ShapeArrayPtr)[Index]])

extern void BURGERCALL SetTheClipBounds(Word Left,Word Top,Word Right,Word Bottom);
extern void BURGERCALL SetTheClipRect(const struct LBRect *RectPtr);
extern void BURGERCALL GetTheClipRect(struct LBRect *RectPtr);
extern Word BURGERCALL GetAPixel(Word x,Word y);
extern Word BURGERCALL GetAPixel16(Word x,Word y);
extern void BURGERCALL SetAPixel(Word x,Word y,Word Color);
extern void BURGERCALL SetAPixel16(Word x,Word y,Word Color);
extern void BURGERCALL SetAPixelTo16(Word x,Word y,Word Color);
extern void BURGERCALL DrawARect(int x,int y,Word Width,Word Height,Word Color);
extern void BURGERCALL DrawARect16(int x,int y,Word Width,Word Height,Word Color);
extern void BURGERCALL DrawARectTo16(int x,int y,Word Width,Word Height,Word Color);
extern void BURGERCALL DrawALine(int x1,int y1,int x2,int y2,Word Color);
extern void BURGERCALL DrawALine16(int x1,int y1,int x2,int y2,Word Color);
extern void BURGERCALL DrawALineTo16(int x1,int y1,int x2,int y2,Word Color);
extern void BURGERCALL DrawARectRemap(int x,int y,Word Width,Word Height,const Word8 *RemapPtr);
extern void BURGERCALL DrawARectRemap16(int x,int y,Word Width,Word Height,const Word16 *RemapPtr);

extern void BURGERCALL ScreenClear(Word Color);
extern void BURGERCALL ScreenBox(int x,int y,Word Width,Word Height,Word Color);
extern void BURGERCALL ScreenBox2(int x,int y,Word Width,Word Height,Word Color1,Word Color2);
extern void BURGERCALL ScreenThickBox(int x,int y,Word Width,Word Height,Word Color);
extern void BURGERCALL ScreenThickBox2(int x,int y,Word Width,Word Height,Word Color1,Word Color2);
extern void BURGERCALL ScreenBoxRemap(int x,int y,Word Width,Word Height,const void *RemapPtr);
extern void BURGERCALL ScreenBoxDropShadow(int x,int y,Word Width,Word Height,Word Color1,Word Color2);
extern void BURGERCALL ScreenRectDropShadow(int x,int y,Word Width,Word Height,Word Color1,Word Color2,Word Color3);

extern void BURGERCALL DrawShapeLowLevel(Word x,Word y,Word Width,Word Height,Word Skip,void *ShapePtr);
extern void BURGERCALL DrawShapeLowLevelClipped(int x,int y,Word Width,Word Height,Word Skip,void *ShapePtr);
#define DrawShape(x,y,p) DrawShapeLowLevel(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,((LWShape_t *)p)->Data)
#define DrawShapeClipped(x,y,p) DrawShapeLowLevelClipped(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawRawShape(x,y,w,h,p) DrawShapeLowLevel(x,y,w,h,0,p)
#define DrawRawShapeClipped(x,y,w,h,p) DrawShapeLowLevelClipped(x,y,w,h,0,p)
#define DrawXShape(x,y,p) DrawShapeLowLevel(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
#define DrawXShapeClipped(x,y,p) DrawShapeLowLevelClipped(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
extern void BURGERCALL DrawRezShape(Word x,Word y,struct RezHeader_t *Input,Word RezNum);
extern void BURGERCALL DrawRezCenterShape(struct RezHeader_t *Input,Word RezNum);

extern void BURGERCALL DrawMShapeLowLevel(Word x,Word y,Word Width,Word Height,Word Skip,void *ShapePtr);
extern void BURGERCALL DrawMShapeLowLevelClipped(int x,int y,Word Width,Word Height,Word Skip,void *ShapePtr);
#define DrawMShape(x,y,p) DrawMShapeLowLevel(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,((LWShape_t *)p)->Data)
#define DrawMShapeClipped(x,y,p) DrawMShapeLowLevelClipped(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawRawMShape(x,y,w,h,p) DrawMShapeLowLevel(x,y,w,h,0,p)
#define DrawRawMShapeClipped(x,y,w,h,p) DrawMShapeLowLevelClipped(x,y,w,h,0,p)
#define DrawXMShape(x,y,p) DrawMShapeLowLevel(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
#define DrawXMShapeClipped(x,y,p) DrawMShapeLowLevelClipped(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
extern void BURGERCALL DrawRezMShape(Word x,Word y,struct RezHeader_t *Input,Word RezNum);
extern void BURGERCALL DrawRezCenterMShape(struct RezHeader_t *Input,Word RezNum);

#define DrawShapeLowLevel16(x,y,w,h,s,p) DrawShapeLowLevel(x*2,y,w*2,h,s,p)
extern void BURGERCALL DrawShapeLowLevelClipped16(int x,int y,Word Width,Word Height,Word Skip,void *ShapePtr);
#define DrawShape16(x,y,p) DrawShapeLowLevel16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawShapeClipped16(x,y,p) DrawShapeLowLevelClipped16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawRawShape16(x,y,w,h,p) DrawShapeLowLevel16(x,y,w,h,0,p)
#define DrawRawShapeClipped16(x,y,w,h,p) DrawShapeLowLevelClipped16(x,y,w,h,0,p)
#define DrawXShape16(x,y,p) DrawShapeLowLevel16(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
#define DrawXShapeClipped16(x,y,p) DrawShapeLowLevelClipped16(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
extern void BURGERCALL DrawRezShape16(Word x,Word y,struct RezHeader_t *Input,Word RezNum);
extern void BURGERCALL DrawRezCenterShape16(struct RezHeader_t *Input,Word RezNum);

extern void BURGERCALL DrawMShapeLowLevel16(Word x,Word y,Word Width,Word Height,Word Skip,void *ShapePtr);
extern void BURGERCALL DrawMShapeLowLevelClipped16(int x,int y,Word Width,Word Height,Word Skip,void *ShapePtr);
#define DrawMShape16(x,y,p) DrawMShapeLowLevel16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawMShapeClipped16(x,y,p) DrawMShapeLowLevelClipped16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawRawMShape16(x,y,w,h,p) DrawMShapeLowLevel16(x,y,w,h,0,p)
#define DrawRawMShapeClipped16(x,y,w,h,p) DrawMShapeLowLevelClipped16(x,y,w,h,0,p)
#define DrawXMShape16(x,y,p) DrawMShapeLowLevel(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
#define DrawXMShapeClipped16(x,y,p) DrawMShapeLowLevelClipped16(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
extern void BURGERCALL DrawRezMShape16(Word x,Word y,struct RezHeader_t *Input,Word RezNum);
extern void BURGERCALL DrawRezCenterMShape16(struct RezHeader_t *Input,Word RezNum);

extern void BURGERCALL DrawShapeLowLevelTo16(Word x,Word y,Word Width,Word Height,Word Skip,void *ShapePtr);
extern void BURGERCALL DrawShapeLowLevelClippedTo16(int x,int y,Word Width,Word Height,Word Skip,void *ShapePtr);
#define DrawShapeTo16(x,y,p) DrawShapeLowLevelTo16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawShapeClippedTo16(x,y,p) DrawShapeLowLevelClippedTo16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawRawShapeTo16(x,y,w,h,p) DrawShapeLowLevelTo16(x,y,w,h,0,p)
#define DrawRawShapeClippedTo16(x,y,w,h,p) DrawShapeLowLevelClippedTo16(x,y,w,h,0,p)
#define DrawXShapeTo16(x,y,p) DrawShapeLowLevelTo16(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
#define DrawXShapeClippedTo16(x,y,p) DrawShapeLowLevelClippedTo16(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)

extern void BURGERCALL DrawMShapeLowLevelTo16(Word x,Word y,Word Width,Word Height,Word Skip,void *ShapePtr);
extern void BURGERCALL DrawMShapeLowLevelClippedTo16(int x,int y,Word Width,Word Height,Word Skip,void *ShapePtr);
#define DrawMShapeTo16(x,y,p) DrawMShapeLowLevelTo16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawMShapeClippedTo16(x,y,p) DrawMShapeLowLevelClippedTo16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawRawMShapeTo16(x,y,w,h,p) DrawMShapeLowLevelTo16(x,y,w,h,0,p)
#define DrawRawMShapeClippedTo16(x,y,w,h,p) DrawMShapeLowLevelClippedTo16(x,y,w,h,0,p)
#define DrawXMShapeTo16(x,y,p) DrawMShapeLowLevelTo16(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
#define DrawXMShapeClippedTo16(x,y,p) DrawMShapeLowLevelClippedTo16(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)

extern void BURGERCALL EraseShape(Word x,Word y,void *ShapePtr);
extern void BURGERCALL EraseMBShape(Word x,Word y,void *ShapePtr,void *BackPtr);
extern Word BURGERCALL TestMShape(Word x,Word y,void *ShapePtr);
extern Word BURGERCALL TestMBShape(Word x,Word y,void *ShapePtr,void *BackPtr);
extern void BURGERCALL VideoPixelDouble16(const Word8 *SourcePtr,Word8 *DestPtr,Word SourceRowBytes,Word DestRowBytes,Word Width,Word Height);
extern void BURGERCALL VideoPixelDoubleChecker16(const Word8 *SourcePtr,Word8 *DestPtr,Word SourceRowBytes,Word DestRowBytes,Word Width,Word Height);
extern void BURGERCALL VideoPixelDouble(const Word8 *SourcePtr,Word8 *DestPtr,Word SourceRowBytes,Word DestRowBytes,Word Width,Word Height);
extern void BURGERCALL VideoPixelDoubleChecker(const Word8 *SourcePtr,Word8 *DestPtr,Word SourceRowBytes,Word DestRowBytes,Word Width,Word Height);

extern Word BURGERCALL OpenGLSetDisplayToSize(Word Width,Word Height,Word Depth,Word Flags);
extern void BURGERCALL OpenGLMakeCurrent(void);
extern void BURGERCALL OpenGLRelease(void);
extern void BURGERCALL OpenGLSwapBuffers(void);
extern void BURGERCALL OpenGLInit(void);
extern void BURGERCALL OpenGLSolidRect(int x,int y,Word Width,Word Height,Word32 Color);
extern void BURGERCALL OpenGLLine(int x,int y,int x2,int y2,Word32 Color);
extern void BURGERCALL OpenGLTextureDraw2D(Word TexNum,int x,int y,Word Width,Word Height);
extern void BURGERCALL OpenGLTextureDraw2DSubRect(Word TexNum,int x,int y,Word Width,Word Height,const float *UVPtr);
extern Word BURGERCALL OpenGLLoadTexture(Word *TextureNum,const struct Image_t *ImagePtr);
extern void BURGERCALL OpenGLSetCurrent(void);
extern void BURGERCALL OpenGLATISetTruform(Word Setting);
extern void BURGERCALL OpenGLATISetFSAA(Word Setting);

#if defined(__MAC__) || defined(__MACOSX__)
extern struct __AGLContextRec * BURGERCALL OpenGLSetContext(struct __AGLContextRec *Context);
extern struct __AGLContextRec * BURGERCALL OpenGLGetContext(void);
#endif

#if defined(__WIN32__)

extern Word BURGERCALL WinSetDisplayToSize(Word Width,Word Height,Word Depth);
extern void BURGERCALL DrawARectDirectX(int x,int y,Word Width,Word Height,Word Color);
#define DrawARectDirectX16 DrawARectDirectX
extern void BURGERCALL DrawARectDirectXTo16(int x,int y,Word Width,Word Height,Word Color);

/* These macros help store data into the output stream for D3D operations */
/* I assume little endian in the operations since this belongs to a PC exclusively */

#define STORE_OP_CONST(a,b,c,p) ((Word32 *)p)[0] = (((Word32)a) + (((Word32)b)<<8) + (((Word32)c)<<16)); p =p+4;
#define STORE_OP(a,b,c,p) ((D3DINSTRUCTION*)p)->bOpcode = (Word8)a; ((D3DINSTRUCTION*)p)->bSize = (Word8)b; ((D3DINSTRUCTION*)p)->wCount = (Word16)c; p = p+4;

#define STORE_OP_RENDERSTATE_CONST(a,p) ((Word32 *)p)[0] = ((Word32)D3DOP_STATERENDER + (((Word32)sizeof(D3DSTATE))<<8) + (((Word32)a)<<16)); p = p+4;
#define STORE_OP_RENDERSTATE(a,p) ((Word16 *)p)[0] = (((Word16)D3DOP_STATERENDER) + ((Word16)sizeof(D3DSTATE)<<8)); ((Word16 *)p)[1] = (Word16)a; p = p+4;
#define STORE_OP_PROCESSVERTICES_CONST(a,p) ((Word32 *)p)[0] = ((Word32)D3DOP_PROCESSVERTICES + (((Word32)sizeof(D3DPROCESSVERTICES))<<8) + (((Word32)a)<<16)); p = p+4;
#define STORE_OP_PROCESSVERTICES(a,p) ((Word16 *)p)[0] = (((Word16)D3DOP_PROCESSVERTICES) + ((Word16)sizeof(D3DPROCESSVERTICES)<<8)); ((Word16 *)p)[1] = (Word16)a; p = p+4;
#define STORE_OP_TRIANGLE_CONST(a,p) ((Word32 *)p)[0] = ((Word32)D3DOP_TRIANGLE + (((Word32)sizeof(D3DTRIANGLE))<<8) + (((Word32)a)<<16)); p = p+4;
#define STORE_OP_TRIANGLE(a,p) ((Word16 *)p)[0] = (((Word16)D3DOP_TRIANGLE) + ((Word16)sizeof(D3DTRIANGLE)<<8)); ((Word16 *)p)[1] = (Word16)a; p = p+4;
#define STORE_OP_EXIT(p) ((Word32 *)p)[0] = (Word32)D3DOP_EXIT; p = ((Word8 *)p+4);

#define STORE_DATA_STATE(a,b,p) ((D3DSTATE *)p)->drstRenderStateType = (D3DRENDERSTATETYPE)a; ((D3DSTATE *)p)->dwArg[0] = b; p = p+8;
#define STORE_DATA_PROCESSVERTICES(flgs,strt,cnt,p) ((D3DPROCESSVERTICES *)p)->dwFlags = flgs; ((D3DPROCESSVERTICES *)p)->wStart = (Word16)strt; ((D3DPROCESSVERTICES *)p)->wDest = (Word16)strt; ((D3DPROCESSVERTICES *)p)->dwCount = cnt; ((D3DPROCESSVERTICES *)p)->dwReserved = 0; p = p+16;
#define STORE_DATA_TRIANGLE(a,b,c,p) ((D3DTRIANGLE *)p)->v1 = (Word16)(a); ((D3DTRIANGLE *)p)->v2 = (Word16)(b); ((D3DTRIANGLE *)p)->v3 = (Word16)(c); ((D3DTRIANGLE *)p)->wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE; p = p + 8;

#define MAXD3DINSTRUCTIONS 1024		/* Maximum number of D3D instructions to queue */

extern struct IDirect3D *Direct3DPtr;				/* Reference to the direct 3D object */
extern struct IDirect3DDevice *Direct3DDevicePtr;	/* Reference to the direct 3D rendering device */
extern struct IDirect3DViewport *Direct3DViewPortPtr;	/* Reference to the direct 3d Viewport */
extern struct IDirect3DExecuteBuffer *Direct3DExecBufPtr;	/* Reference to the direct 3d execute buffer */
extern Word8 *Direct3DExecDataBuf;			/* Pointer to the execute data buffer */
extern Word8 *Direct3DExecInstBuf;			/* Pointer to the execute instruction buffer */
extern Word8 *Direct3DExecDataPtr;			/* Current free data pointer */
extern Word8 *Direct3DExecInstPtr;			/* Current free instruction pointer */
extern Word8 *Direct3DExecInstStartPtr;		/* Pointer to the beginning of the last instruction chunk */
extern Word32 Direct3DExecBuffSize;		/* Size of the execute buffer */

extern void BURGERCALL D3DGetTextureInfo(void);
extern void BURGERCALL D3DSetStandardViewport(void);
extern void BURGERCALL D3DCreateZBuffer(void);
extern void BURGERCALL D3DInitExecuteBuffer(void);
extern void BURGERCALL D3DInit(const struct _GUID *Input);
extern void BURGERCALL Direct3DDestroy(void);
extern void BURGERCALL Direct3DLockExecuteBuffer(void);
extern void BURGERCALL Direct3DUnlockExecuteBuffer(void);
extern void BURGERCALL Direct3DCheckExecBuffer(Word InstCount,Word VertexCount);
extern void BURGERCALL Direct3DBeginScene(void);
extern void BURGERCALL Direct3DEndScene(void);
extern void BURGERCALL Direct3DSolidRect(int x,int y,Word Width,Word Height,Word32 Color);
extern void BURGERCALL Direct3DLine(int x,int y,int x2,int y2,Word32 Color);

#endif

#ifdef __cplusplus
}
#endif

#endif
