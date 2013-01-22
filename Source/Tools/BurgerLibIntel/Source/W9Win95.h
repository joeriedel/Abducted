/*******************************

	Burger's Universal library WIN95 version
	This is for Watcom 10.5 and higher...
	Also support for MSVC 4.0

*******************************/

#ifndef __W9WIN95_H__
#define __W9WIN95_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#if defined(__WIN32__)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Direct3DTexture_t {
	unsigned long TextureHandle;			/* The Direct3D texture handle for rendering */
	struct IDirectDrawSurface *Surface;		/* Pointer to the DirectDraw Surface */
	struct IDirect3DTexture *TexturePtr;	/* Pointer to the Direct3D Texture reference */
} Direct3DTexture_t;

/**********************************

	Win 95 specific functions

**********************************/

extern Word TickWakeUpFlag;		/* TRUE if I should get tick events */
extern Word Win95NoProcess;		/* Shut down ProcessSystemEvents */
extern Word Win95MouseButton;	/* Mouse button contents */
extern Word Win95MouseX;		/* Virtual mouse X */
extern Word Win95MouseY;		/* Virtual mouse Y */
extern int Win95MouseWheel;		/* Virtual mouse Z */
extern int Win95MouseXDelta;	/* Mouse motion */
extern int Win95MouseYDelta;
extern int Win95LastMouseX;		/* Previous mouse position for delta motion */
extern int Win95LastMouseY;
extern int Win95DestScreenX;	/* X coord to draw screen to */
extern int Win95DestScreenY;	/* Y coord to draw screen to */
extern void *Win95MainWindow;	/* Main window to perform all operations on */
extern struct IDirectDrawSurface *Win95FrontBuffer;	/* Currently displayed screen */
extern struct IDirectDrawSurface *Win95BackBuffer;	/* My work buffer */
extern struct IDirectDrawSurface *Win95WorkBuffer;	/* Which buffer am I using? */
extern struct IDirectDrawSurface *Direct3DZBufferPtr;		/* ZBuffer for 3D */
extern struct IDirectDrawPalette *Win95WindowPalettePtr;	/* Pointer to game palette */
extern struct IDirectDraw *DirectDrawPtr;			/* Pointer to direct draw instance */
extern struct IDirectDraw2 *DirectDraw2Ptr;			/* Pointer to the direct draw 2 instance */
extern struct IDirectDraw4 *DirectDraw4Ptr;			/* Pointer to the direct draw 4 instance */
extern struct IDirectDrawClipper *Win95WindowClipperPtr;	/* Clipper for primary surface */
extern Word Direct3DZBufferBitDepth;					/* Bits per pixel for D3D ZBuffer */
extern Word Win95ApplsActive;	/* True if the application is active */
extern Word BURGERCALL Win95ProcessMessage(struct tagMSG *MessagePtr);
extern void BURGERCALL ProcessSystemEvents(void);
extern Word8 *Win95VideoPointer;	/* Locked videopointer for hardware blits */
extern void *Win95Instance;		/* HINSTANCE of the current app */
extern Bool Win95UseBackBuffer;	/* True if the backbuffer is used */
extern void *Win95LockedMemory;		/* Copy of the video pointer */
extern void *BurgerHBitMap;			/* HBITMAP for window mode */
extern Word8 *BurgerHBitMapPtr;		/* Pointer to HBITMAP Memory */
extern void *BurgerHPalette;		/* HPALETTE for window mode */
extern short Win95DirectSoundVolumes[256];

extern Word BURGERCALL InitWin95Window(const char *Title,void *Instance,long (__stdcall *Proc)(struct HWND__*,Word,Word,long));
extern Word BURGERCALL GetDirectXVersion(void);
extern void BURGERCALL Win95AppendFilenameToDirectory(char *Directory,const char *Filename);
extern Word BURGERCALL Win95AddGroupToProgramMenu(const char *GroupName);
extern Word BURGERCALL Win95DeleteGroupFromProgramMenu(const char *GroupName);
extern Word BURGERCALL Win95Allow1Instance(const char *LockName);
extern long BURGERCALL CallDirectDrawCreate(struct _GUID *lpGUID,struct IDirectDraw **lplpDD,void *pUnkOuter);
extern long BURGERCALL CallDirectDrawEnumerateA(int (__stdcall *lpCallback)(struct _GUID *,char *,char *,void*),void *lpContext);
extern long BURGERCALL CallDirectDrawEnumerateExA(int (__stdcall *)(struct _GUID *,char*,char*,void*,void *),void *lpContext,Word32 Flags);
extern Word BURGERCALL Win95VideoGetGuid(struct _GUID **OutPtr,struct _GUID *Output,Word DevNum);

extern Word BURGERCALL Direct3DTextureInitImage(Direct3DTexture_t *Input,const struct Image_t *ImagePtr);
extern Direct3DTexture_t * BURGERCALL Direct3DTextureNewImage(const struct Image_t *ImagePtr);
extern void BURGERCALL Direct3DTextureDelete(Direct3DTexture_t *Input);
extern void BURGERCALL Direct3DTextureDestroy(Direct3DTexture_t *Input);
extern void BURGERCALL Direct3DTextureDraw2D(Direct3DTexture_t *Input,int x,int y,Word Width,Word Height);
extern void BURGERCALL Direct3DTextureDraw2DSubRect(Direct3DTexture_t *Input,int x,int y,Word Width,Word Height,const float *UVPtr);

#ifdef __cplusplus
}
#endif
#endif

#endif

