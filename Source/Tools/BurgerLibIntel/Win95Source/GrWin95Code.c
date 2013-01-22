/**********************************

	Win95/NT version of the Graphics manager
	I require DirectX 3. However I will use DirectX
	5 and 6 if available.

	Last but not least, I do not require DDRAW.DLL to
	be directly linked in. I will manually load in
	the DLL on a demand basis. This way, OpenGL is
	not encumbered on NT systems (NT 4.0 Service Pack #3
	only has DirectX 3, Sigh.)

**********************************/

#include "GrGraphics.h"

#if defined(__WIN32__)		/* Only Win95/NT code here! */

#if defined(__WATCOMC__)
#pragma library ("dxguid.lib");
#endif

#define WIN32_LEAN_AND_MEAN
#define DIRECTDRAW_VERSION 0x700
#define DIRECT3D_VERSION 0x700

/* Include all the Win32 headers! */

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include "../Win95Extern/DirectX/ddraw.h"
#include "../Win95Extern/DirectX/d3d.h"
#include "W9Win95.h"
#include "ClStdLib.h"
#include "MmMemory.h"
#include "PlPalette.h"
#include "ShStream.h"
#include "SsScreenShape.h"
#include "InInput.h"

/* These proc pointers are dynamically loaded in if the DDRAW.DLL */
/* library is present. */

typedef HRESULT (WINAPI *DIRECTDRAWCREATE)(GUID *, LPDIRECTDRAW *, IUnknown *);
typedef HRESULT (WINAPI *DIRECTDRAWENUMERATEA)(LPDDENUMCALLBACKA,LPVOID);
typedef HRESULT (WINAPI *DIRECTDRAWENUMERATEEXA)(LPDDENUMCALLBACKEXA,LPVOID,DWORD);

typedef struct DeviceGuid_t {
	Word DevNum;
	GUID *GuidPtr;
	GUID Guid;
} DeviceGuid_t;

static DDBLTFX BlitTable = {sizeof(DDBLTFX)};		/* For solid blitting */
static HINSTANCE DirectDrawInstance;				/* Reference to DDRAW.DLL */
static DIRECTDRAWCREATE DirectDrawCreatePtr;		/* Pointer to direct draw */
static DIRECTDRAWENUMERATEA DirectDrawEnumerateAPtr;
static DIRECTDRAWENUMERATEEXA DirectDrawEnumerateExAPtr;
static Word OldWindowXYValid = FALSE;				/* Previous window location not set? */
static int OldWindowX,OldWindowY;					/* Previous window location */
static LPDIRECTDRAWGAMMACONTROL lpDDGammaControl;		/* Instance of the gamma handler */
static DDGAMMARAMP DDGammaOld; 						/* Cached gamma settings */
static DDGAMMARAMP DDGammaCurrent;					/* Current gamma settings */
static Word8 GammaDC;								/* TRUE, If I use the WinGDI for Gamma control */
static Word8 GammaInit;								/* TRUE if gamma manager is active */
static Word8 GammaTry;								/* TRUE if gamma manager SHOULD be restarted after a failed SetDisplayToSize() */
IDirect3D *Direct3DPtr;				/* Reference to the direct 3D object */
IDirect3DDevice *Direct3DDevicePtr;	/* Reference to the direct 3D rendering device */
IDirect3DViewport *Direct3DViewPortPtr;	/* Reference to the direct 3d Viewport */
IDirect3DExecuteBuffer *Direct3DExecBufPtr;	/* Reference to the direct 3d execute buffer */
Word8 *Direct3DExecDataBuf;			/* Pointer to the execute data buffer */
Word8 *Direct3DExecInstBuf;			/* Pointer to the execute instruction buffer */
Word8 *Direct3DExecDataPtr;			/* Current free data pointer */
Word8 *Direct3DExecInstPtr;			/* Current free instruction pointer */
Word8 *Direct3DExecInstStartPtr;		/* Pointer to the beginning of the last instruction chunk */
Word8 *Direct3DExecInstEndPtr; 		/* End of the instruction buffer */
Word32 Direct3DExecBuffSize;		/* Size of the execute buffer */

/**********************************

	These are Win95 specific convience routines
	defined in LWWin95.h. They are used to assist in
	the implementation of Burgerlib Video

**********************************/

/**********************************

	Make sure the libaries are loaded in

**********************************/

static void GetDDRAWDLL(void)
{
	if (!DirectDrawInstance) {
		DirectDrawInstance = LoadLibrary("DDRAW.DLL");	/* Get the library */
		if (DirectDrawInstance) {		/* Did it load? */
			DirectDrawCreatePtr = (DIRECTDRAWCREATE)GetProcAddress(DirectDrawInstance,"DirectDrawCreate");
			DirectDrawEnumerateAPtr = (DIRECTDRAWENUMERATEA)GetProcAddress(DirectDrawInstance,"DirectDrawEnumerate");
			DirectDrawEnumerateExAPtr = (DIRECTDRAWENUMERATEEXA)GetProcAddress(DirectDrawInstance,"DirectDrawEnumerateExA");
		}
	}
}

/**********************************

	Call DirectDrawCreate.
	I will first manually load DirectDrawCreate
	and then call it if I find it.
	Note : I do NOT ever release DirectDraw, since
	I use the properties forever...

**********************************/

long BURGERCALL CallDirectDrawCreate(struct _GUID *lpGUID,struct IDirectDraw **lplpDD,void *pUnkOuter)
{
	GetDDRAWDLL();		/* Load it in */
	if (DirectDrawCreatePtr) {			/* Already loaded? */
		return DirectDrawCreatePtr(lpGUID,lplpDD,(IUnknown *)pUnkOuter);	/* Call it */
	}
	return DDERR_NOTINITIALIZED;
}

/**********************************

	Call DirectDrawEnumerateExA
	This is a DirectX 1 or higher call

**********************************/

long BURGERCALL CallDirectDrawEnumerateA(int (__stdcall *lpCallback)(struct _GUID *,char *,char *,void*),void *lpContext)
{
	GetDDRAWDLL();
	if (DirectDrawEnumerateAPtr) {
		return DirectDrawEnumerateAPtr(lpCallback,lpContext);
	}
	return DDERR_NOTINITIALIZED;
}

/**********************************

	Call DirectDrawEnumerateExA
	This is a DirectX 5 or higher call

**********************************/

long BURGERCALL CallDirectDrawEnumerateExA(int (__stdcall *lpCallback)(struct _GUID *,char *,char *,void*,void *),void *lpContext,Word32 Flags)
{
	GetDDRAWDLL();
	if (DirectDrawEnumerateExAPtr) {
		return DirectDrawEnumerateExAPtr((LPDDENUMCALLBACKEXA)lpCallback,lpContext,Flags);
	}
	return DDERR_NOTINITIALIZED;
}

/******************************

	This callback is used to find a specific GUID for
	an enumerated device

******************************/

static int CALLBACK FindDeviceCallback(struct _GUID *pGUID,char * /*pName*/,char * /*pDeviceName */,void *MyData,void * /*Monitor */)
{
	DeviceGuid_t *Ref;

	Ref = (DeviceGuid_t *)MyData;		/* Deref the pointer */
	if (--Ref->DevNum==0) {				/* Found the device yet? */
		if (pGUID) {					/* Specific device? */
			Ref->GuidPtr = &Ref->Guid;		/* Fix the pointer to the struct */
			FastMemCpy(&Ref->Guid,pGUID,sizeof(GUID));	/* Copy the GUID */
		} else {
			Ref->GuidPtr = 0;
		}
		return DDENUMRET_CANCEL;		/* Stop now */
	}
	return DDENUMRET_OK;				/* Keep going */
}

/**********************************

	Scan the device list for the GUID of
	the requested device.
	Since a NULL pointer is valid, I must return both the GUID
	and a pointer to the GUID. Sigh.

**********************************/

Word BURGERCALL Win95VideoGetGuid(struct _GUID **OutPtr,struct _GUID *Output,Word DevNum)
{
	DeviceGuid_t Ref;
	if (!DevNum) {		/* Failsafe! */
		DevNum = 1;
	}
	Ref.DevNum = DevNum;		/* Scan for this device */
	if (CallDirectDrawEnumerateExA(FindDeviceCallback,(void *)&Ref,DDENUM_ATTACHEDSECONDARYDEVICES|
		DDENUM_DETACHEDSECONDARYDEVICES|DDENUM_NONDISPLAYDEVICES)==DD_OK) {
		if (!Ref.DevNum) {		/* Got it? */
			if (Ref.GuidPtr) {
				OutPtr[0] = Output;
				FastMemCpy(Output,Ref.GuidPtr,sizeof(GUID));
			} else {
				FastMemSet(Output,0,sizeof(GUID));
				OutPtr[0] = 0;
			}
			return FALSE;
		}
	}
	return TRUE;
}

/**********************************

	Here is the implementation of Burgerlib
	video for Win95

**********************************/

/**********************************

	Set up the current video page
	for drawing, lock down the pixels
	and set the burgerlib video port

	Set up the Burgerlib graphic variables
	so that I can draw to a Win 95 graphic screen
	I require that SetDisplayToSize() be called to
	set up the environment.

**************************/

void BURGERCALL LockFrontVideoPage(void)
{
	DDSURFACEDESC ddsd;
	HRESULT Foo;
	IDirectDrawSurface *MySurface;

	/* Software only mode (Allocated or HBITMAP) ? */

	if (VideoOffscreen) {
		VideoPointer = VideoOffscreen;
		VideoWidth = VideoHardWidth;
		Win95VideoPointer = 0;		/* Force hardware NOT to work */
		return;
	}

	/* DirectX needs the surface to be locked! */

	if (!VideoPageLocked) {		/* Not locked? */
		Win95UseBackBuffer = FALSE;
	 	MySurface = Win95FrontBuffer;
	 	ddsd.dwSize = sizeof(ddsd);	/* Set the size */
		ddsd.dwFlags = 0;
		Foo = IDirectDrawSurface_Lock(MySurface,0,&ddsd,DDLOCK_WAIT,0);
		if (Foo!=DD_OK) {
			goto Damn;
		}
Cool:
		VideoPageLocked = TRUE;		/* Mark as locked */
		VideoPointer = (Word8 *)ddsd.lpSurface;	/* Set the pointer */
		Win95LockedMemory = VideoPointer;
		Win95WorkBuffer = MySurface;
		Win95VideoPointer = VideoPointer;		/* Allow the unlock to work */
		VideoWidth = ddsd.lPitch;		/* Get the width of each line */
		{		/* I'm directly blitting into a window! */
			Word32 Offset;
			Offset = Win95DestScreenX*((VideoColorDepth+7)>>3);
			VideoPointer = &VideoPointer[(Win95DestScreenY*VideoWidth)+Offset];
			Win95VideoPointer = VideoPointer;
		}
	}
	return;

	/* Oh oh, the surface was lost, I need to restore it */

Damn:;
	if (Foo == DDERR_SURFACELOST) {		/* I can recover from this... */
		Foo = IDirectDrawSurface_Restore(MySurface);	/* Try a restore */
		if (Foo==DD_OK) {
			Foo = IDirectDrawSurface_Lock(MySurface,0,&ddsd,DDLOCK_WAIT,0);
			if (Foo==DD_OK) {
				goto Cool;
			}
		}
	}

/* Drat! Foiled again! */

	VideoPointer = 0;
	NonFatal("Can't lock the buffer bits!\n");	/* Oh oh... */
}

/**********************************

	Set up the current video page
	for drawing, lock down the pixels
	and set the burgerlib video port

	Set up the Burgerlib graphic variables
	so that I can draw to a Win 95 graphic screen
	I require that SetDisplayToSize() be called to
	set up the environment.

**********************************/

void BURGERCALL LockVideoPage(void)
{
	DDSURFACEDESC ddsd;
	HRESULT Foo;
	IDirectDrawSurface *MySurface;

	if (!VideoPageLocked) {

		/* Software only mode (Allocated or HBITMAP) ? */

		if (VideoOffscreen) {
			VideoPointer = VideoOffscreen;
			VideoWidth = VideoHardWidth;
			Win95VideoPointer = 0;		/* Force hardware NOT to work */
			return;
		}

		if (Win95UseBackBuffer) {		/* Back or front? */
			MySurface = Win95BackBuffer;
		} else {
			MySurface = Win95FrontBuffer;
		}

		while (IDirectDrawSurface_GetBltStatus(MySurface,DDGBS_ISBLTDONE) == DDERR_WASSTILLDRAWING) {}	/* Is blitting done? */

		ddsd.dwSize = sizeof(ddsd);	/* Set the size */
		ddsd.dwFlags = 0;
		Foo = IDirectDrawSurface_Lock(MySurface,0,&ddsd,DDLOCK_WAIT,0);
		if (Foo!=DD_OK) {
			goto Damn;
		}
Cool:
		VideoPageLocked = TRUE;		/* Mark as locked */
		VideoPointer = (Word8 *)ddsd.lpSurface;	/* Set the pointer */
		Win95LockedMemory = VideoPointer;
		Win95WorkBuffer = MySurface;
		Win95VideoPointer = VideoPointer;
		VideoWidth = ddsd.lPitch;		/* Get the width of each line */
		if (Win95UseBackBuffer) {
			return;
		}
		{		/* I'm directly blitting into a window! */
			Word32 Offset;
			Offset = Win95DestScreenX*((VideoColorDepth+7)>>3);
			VideoPointer = &VideoPointer[(Win95DestScreenY*VideoWidth)+Offset];
			Win95VideoPointer = VideoPointer;
		}
	}
	return;

	/* Oh oh, the surface was lost, I need to restore it */

Damn:;
	if (Foo == DDERR_SURFACELOST) {		/* I can recover from this... */
		Foo = IDirectDrawSurface_Restore(MySurface);	/* Try a restore */
		if (Foo==DD_OK) {
			Foo = IDirectDrawSurface_Lock(MySurface,0,&ddsd,DDLOCK_WAIT,0);
			if (Foo==DD_OK) {
				goto Cool;
			}

	/* In full screen mode, I need to restore the FRONT buffer only */

		} else if (Foo==DDERR_IMPLICITLYCREATED) {
			Foo = IDirectDrawSurface_Restore(Win95FrontBuffer);	/* Try a restore */
			if (Foo==DD_OK) {
				Foo = IDirectDrawSurface_Lock(MySurface,0,&ddsd,DDLOCK_WAIT,0);
				if (Foo==DD_OK) {
					goto Cool;
				}
			}
		}
	}

/* Drat! Foiled again! */

	VideoPointer = 0;
	NonFatal("Can't lock the buffer bits!\n");	/* Oh oh... */
}

/**********************************

	Release the use of the video memory

**********************************/

void BURGERCALL UnlockVideoPage(void)
{
	if (VideoPageLocked) {
		if (Win95VideoPointer) {
			IDirectDrawSurface_Unlock(Win95WorkBuffer,Win95LockedMemory);
			Win95VideoPointer = 0;
		}
		VideoPageLocked = FALSE;		/* It's released */
	}
}

/**********************************

	Take the previous screen and display it to the current
	video display hardware and now draw directly
	to the video display hardware

	Update the screen and use the front buffer
	WARNING!!!!!
	BoundsChecker for some reason will cause a GDI exception
	erron in GDI32.DLL if you are in window mode. This is a
	BoundsChecker problem and not a code problem!

**********************************/

void BURGERCALL UpdateAndNoPageFlip(void)
{
	UnlockVideoPage();		/* Force an unlock */

	if (Win95UseBackBuffer) {		/* Was I drawing to the hidden page? */

		/* Full screen? Simplicity itself */
		if (VideoFullScreen) {
			if (VideoOffscreen) {		/* Offscreen buffer? */
				Word8 *Save;
				Word ByteWidth;
				Save = VideoOffscreen;		/* Save the offscreen pointer */
				VideoOffscreen = 0;		/* Enable hardware */
				LockVideoPage();			/* Lock video */
				if (VideoPointer) {			/* Did I get the lock (Failsafe for mode switch)? */
					ByteWidth = (VideoColorDepth+7)>>3;
					if (VideoColorDepth==15 && VideoTrueScreenDepth==16) {
						VideoOffscreen = Save;
						Video555To565();		/* Do the conversion */
						VideoOffscreen = 0;
					} else {
						DrawShapeLowLevel(0,0,VideoTrueScreenWidth*ByteWidth,VideoTrueScreenHeight,0,Save);
					}
					UnlockVideoPage();			/* Unlock video */
				}
				VideoOffscreen = Save;		/* Restore mode */
			}
			IDirectDrawSurface_Flip(Win95FrontBuffer,0,DDFLIP_WAIT);		/* Perform a page flip */
		} else {

		/* I have to draw in a window. I am burning in hell */
			HDC MyDC;
			HDC WinDC;

			WinDC = GetDC((HWND)Win95MainWindow);		/* Where will I draw it? */
			MyDC = CreateCompatibleDC(WinDC);	/* Temp DC */
			if (BurgerHPalette) {
				SelectPalette(WinDC,(HPALETTE)BurgerHPalette,FALSE);	/* Set the hardware palette */
			}
			RealizePalette(WinDC);			/* Make sure the palette is set */
			SelectObject(MyDC,BurgerHBitMap);	/* Select my temp bitmap */
			BitBlt(WinDC,0,0,ScreenWidth,ScreenHeight,MyDC,0,0,SRCCOPY);
			DeleteDC(MyDC);				/* Get rid of the source context */
			ReleaseDC((HWND)Win95MainWindow,WinDC);	/* Release my window */
		}
		Win95UseBackBuffer = FALSE;		/* Use front buffer */
		Win95WorkBuffer = Win95FrontBuffer;
		Win95VideoPointer = VideoPointer;
	}
}

/**********************************

	Take the previous screen and display it to the current
	video display hardware and now draw to a hidden
	page for page flipping

**********************************/

void BURGERCALL UpdateAndPageFlip(void)
{
	UpdateAndNoPageFlip();			/* No page flipping! */
	Win95UseBackBuffer = TRUE;		/* I'll draw to the back screen */
	Win95WorkBuffer = Win95BackBuffer;	/* Use the back buffer */
	Win95VideoPointer = VideoPointer;	/* Allow hardware */
}

/**********************************

	Release all resources allocated by using
	direct draw to update the video screen

**********************************/

void BURGERCALL ReleaseVideo(void)
{
	Word8 GammaTemp;			/* Keep the try across calls */
	
	UnlockVideoPage();		/* Make SURE the video page is unlocked! */
	GammaTemp = GammaTry;
	VideoOSGammaDestroy();	/* Release any Gamma controls if someone forgot to release it */
	GammaTry = GammaTemp;
	if (Win95WindowClipperPtr) {	/* Is there a clipper? */
		IDirectDrawClipper_Release(Win95WindowClipperPtr);
		Win95WindowClipperPtr = 0;
	}
	if (Win95BackBuffer) {		/* Is there a back buffer? */
		IDirectDrawSurface_Release(Win95BackBuffer);
		Win95BackBuffer = 0;
	}
	if (Win95WindowPalettePtr) {		/* Palette allocated? */
		IDirectDrawPalette_Release(Win95WindowPalettePtr);
		Win95WindowPalettePtr = 0;
	}
	if (Win95FrontBuffer) {		/* Front buffer allocated? */
		IDirectDrawSurface_Release(Win95FrontBuffer);
		Win95FrontBuffer = 0;
	}
	if (DirectDraw4Ptr) {
		IDirectDraw4_Release(DirectDraw4Ptr);
		DirectDraw4Ptr = 0;
	}
	if (DirectDraw2Ptr) {
		IDirectDraw2_Release(DirectDraw2Ptr);
		DirectDraw2Ptr = 0;
	}
	if (DirectDrawPtr) {		/* Direct draw struct allocated? */
		IDirectDraw_Release(DirectDrawPtr);		/* Release direct draw data */
		DirectDrawPtr = 0;
	}
	if (VideoOffscreen && VideoOffscreen!=BurgerHBitMapPtr) {
		DeallocAPointer(VideoOffscreen);
		VideoOffscreen = 0;
	}
	if (BurgerHBitMap) {		/* Was a GDI bitmap allocated? */
		DeleteObject(BurgerHBitMap);
		BurgerHBitMap = 0;
		BurgerHBitMapPtr = 0;
	}
	if (BurgerHPalette) {		/* Was a GDI palette allocated? */
		DeleteObject(BurgerHPalette);
		BurgerHPalette = 0;
	}
	VideoOffscreen = 0;	/* No bitmap present */
	ScreenWidth = 0;		/* Force default */
	ScreenHeight = 0;
	if (DirectDrawInstance) {
		FreeLibrary(DirectDrawInstance);
		DirectDrawInstance = 0;
	}
}

/**********************************

	Initialize a video mode and prepare for
	use of Burgerlib

	I will set up these variables if this call is successful

	BurgerMaxVideoPage = 1-3 pages
	VideoOffscreen = Pointer to offscreen buffer (If mode enabled)
	BurgerScreenSize = Size in bytes of the offscreen buffer
	VideoColorDepth = Depth of the current color map (8,15,16)
	VideoTrueScreenWidth = Width of the screen in PIXELS
	VideoTrueScreenHeight = Height of the screen in PIXELS
	VideoHardWidth = Width of a scan line in BYTES

******************************/

/******************************

	This form of evil will set up the Burgerlib video
	display for Windows 95 using DirectX.

	Note : I have to do a lot of crap to handle direct 3D
	Note : You may ask for a 16 or 15 bit context but DirectX will
		force me to use a 15 or 16 bit mode instead. Read VideoColorDepth
		to get the proper bit type 5:6:5 or 5:5:5

******************************/

Word BURGERCALL SetDisplayToSize(Word Width,Word Height,Word Depth,Word Flags)
{
	DDSURFACEDESC ddsd;
	Word Refresh;
	Word Fake15;
	Word GammaOn;

	GammaOn = GammaInit;
	if (GammaOn) {
		GammaTry = TRUE;		/* Keep trying across several calls */
	}
		
	Fake15 = FALSE;			/* I won't invoke the fake 5:5:5->5:6:5 mode */
	Flags &= (~SETDISPLAYDOUBLEOK);		/* Windows actually supports these modes */
	Refresh = Depth>>16;
	Depth = Depth&0xFFFF;

	if (Depth==24) {
		Depth = 32;		/* Full 32 bits */
	}
	if (Depth==15) {
		if (Flags&SETDISPLAYOFFSCREEN) {
			Fake15 = TRUE;		/* Ok, I may want it invoke the fake 5:5:5 mode */
		}
		Depth = 16;		/* Win95 can only accept a 16 bit mode as input */
	}

	if (!VideoFullScreen) {		/* Was the screen a window before? */
		RECT rc;
		GetWindowRect((HWND)Win95MainWindow,&rc);	/* Save off the window position */
		OldWindowX = rc.left;
		OldWindowY = rc.top;
		OldWindowXYValid = TRUE;		/* The position is valid! */
	}

	ReleaseVideo();		/* Release any direct draw data from last time */

	ScreenWidth = Width;	/* Save the screen size in Burgerlib globals */
	ScreenHeight = Height;
	ScreenFlags = Flags;
	VideoFullScreen = Flags&SETDISPLAYFULLSCREEN;	/* Save in global (Must be done here!) */
	ScreenClipTop = 0;
	ScreenClipLeft = 0;
	ScreenClipRight = Width;
	ScreenClipBottom = Height;
	VideoTrueScreenWidth = Width;
	VideoTrueScreenHeight = Height;

/* If I want a window display buffer, size the window for the new size */

	if (!(Flags&SETDISPLAYNOWINDOWTWIDDLE)) { /* can we play with the window styles? */
		if (!(Flags&SETDISPLAYFULLSCREEN)) {		/* Window mode */
			Word32 dwStyle;	/* Temp window style bits */
			RECT rc; 			/* New window rect */
			RECT rcWork;		/* Temp rect */

			dwStyle = GetWindowStyle((HWND)Win95MainWindow);	/* Get the style of the window */
			dwStyle &= ~WS_POPUP;		/* Can't be a pop-up window */
			dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX;
			SetWindowLong((HWND)Win95MainWindow,GWL_STYLE,dwStyle);	/* Set the style */

			rc.top = rc.left = 0;	/* Init the rect of the window's display area */
			rc.right = Width;
			rc.bottom = Height;

			/* Calcluate the rect of the window after the borders are added... */

			AdjustWindowRectEx(&rc,dwStyle,GetMenu((HWND)Win95MainWindow)!= 0,GetWindowExStyle((HWND)Win95MainWindow));

			/* Resize the window to the new rect */

			SetWindowPos((HWND)Win95MainWindow,HWND_NOTOPMOST,0,0,rc.right-rc.left,rc.bottom-rc.top,SWP_NOMOVE | /* SWP_NOZORDER | */ SWP_NOACTIVATE);

			/* Get the rect of the information bar */

			SystemParametersInfo(SPI_GETWORKAREA,0,&rcWork,0);

			/* Get the x,y position of the window */

			if (OldWindowXYValid) {
				rc.left = OldWindowX;		/* Restore the window position */
				rc.top = OldWindowY;
			} else {
				GetWindowRect((HWND)Win95MainWindow,&rc);	/* Use the old position */
			}

			/* Make sure the window is on screen */

			if (rc.left < rcWork.left) {
				rc.left = rcWork.left;		/* Anchor the window to the screen bar */
			}
			if (rc.top < rcWork.top) {
				rc.top = rcWork.top;
			}
			SetWindowPos((HWND)Win95MainWindow,0,rc.left,rc.top,0,0,SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		} else {
			Word32 dwStyle;	/* Temp window style bits */
			dwStyle = GetWindowStyle((HWND)Win95MainWindow);	/* Get the style of the window */
			dwStyle |= WS_POPUP;		/* Can't be a pop-up window */
			dwStyle &= ~(WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX);
			SetWindowLong((HWND)Win95MainWindow,GWL_STYLE,dwStyle);	/* Set the style */
		}
	}
	
	/* Shall I select a device by reference */

	{
		Word DevNum;
		DevNum = Flags>>16;		/* Get the device number */
		if (DevNum) {			/* Valid? */
			GUID GuidBuf;
			GUID* GuidPtr;
			if (!Win95VideoGetGuid(&GuidPtr,&GuidBuf,DevNum)) {
				if (CallDirectDrawCreate(GuidPtr,&DirectDrawPtr,0)==DD_OK) {
					goto GotInstance;
				}
				NonFatal("Couldn't create the direct draw instance for device #%d\n",DevNum);
				return TRUE;
			}
			NonFatal("Device number #%d is too high\n",DevNum);
			return TRUE;
		}
	}

	/* Allocate a direct draw buffer */

	if (CallDirectDrawCreate(0,&DirectDrawPtr,0) != DD_OK) {
		NonFatal("Couldn't create the direct draw buffer\n");	/* Die!! */
		return TRUE;
	}

GotInstance:;

	/* Create a DirectDraw 2 and 4 reference for the lib's use */
	/* If neither is present, then zero out the pointer to make SURE I don't */
	/* have a dangling pointer anywhere! */

	if (IDirectDraw_QueryInterface(DirectDrawPtr,IID_IDirectDraw2,(void **)&DirectDraw2Ptr)==DD_OK) {	/* DirectX 3? */
		if (IDirectDraw_QueryInterface(DirectDrawPtr,IID_IDirectDraw4,(void **)&DirectDraw4Ptr)!=DD_OK) {	/* DirectX 6? */
			DirectDraw4Ptr = 0;
		}
	} else {
		DirectDraw2Ptr = 0;
	}

		/* Set the cooperative level for the video display */

	{
		Word32 TempFlags;
		TempFlags = (Flags&SETDISPLAYFULLSCREEN) ?
			DDSCL_ALLOWMODEX|DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN :
			DDSCL_NORMAL;		/* Normal window mode */
		if (IDirectDraw_SetCooperativeLevel(DirectDrawPtr,
			(HWND)Win95MainWindow,TempFlags) != DD_OK) {
			NonFatal("Couldn't set direct draw cooperative level\n");
			return TRUE;
		}
	}
	FastMemSet(&ddsd,0,sizeof(ddsd));	/* Init the direct draw surface struct */
	ddsd.dwSize = sizeof(ddsd);		/* Save the size */

	/* If full screen, set the video display to the requested mode */

	if (Flags&SETDISPLAYFULLSCREEN) {
		if (DirectDraw4Ptr) {
			if (IDirectDraw4_SetDisplayMode(DirectDraw4Ptr,
				Width,Height,Depth,Refresh,0) != DD_OK) {
				ReleaseVideo();
				NonFatal("Couldn't set display mode %dx%dx%d at %d hertz\n",Width,Height,Depth,Refresh);
				return TRUE;
			}
		} else {
			if (IDirectDraw_SetDisplayMode(DirectDrawPtr,
				Width,Height,Depth) != DD_OK) {
				ReleaseVideo();
				NonFatal("Couldn't set display mode %dx%dx%d\n",Width,Height,Depth);
				return TRUE;
			}
		}
		ddsd.dwBackBufferCount = 2;		/* Try 3 pages of memory */
		ddsd.dwFlags = DDSD_CAPS|DDSD_BACKBUFFERCOUNT;	/* Get back buffer */
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|	/* Surface */
							DDSCAPS_FLIP | 	/* Allow page flipping */
							DDSCAPS_COMPLEX;	/* Complex mapping */
	} else {
		ddsd.dwFlags = DDSD_CAPS;		/* Simple way */
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	}

	if (Flags&SETDISPLAYD3D) {
		ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;
	}

	/* Get the direct draw primary display page */

	if (IDirectDraw_CreateSurface(DirectDrawPtr,&ddsd,&Win95FrontBuffer,0) != DD_OK) {
		ReleaseVideo();
		NonFatal("Couldn't create the frontbuffer surface\n");
		return TRUE;
	}

	/* Full screen mode is easy, no clipping needed */

	if (Flags&SETDISPLAYFULLSCREEN) {
		DDSCAPS ddscaps;
		FastMemSet(&ddscaps,0,sizeof(ddscaps));		/* Clear the caps struct */
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;	/* Back buffer flag */
		if (Flags&SETDISPLAYD3D) {
			ddscaps.dwCaps |= DDSCAPS_3DDEVICE;
		}
		if (IDirectDrawSurface_GetAttachedSurface(Win95FrontBuffer,
			&ddscaps,&Win95BackBuffer) != DD_OK) {
			ReleaseVideo();
			NonFatal("Couldn't get attached surface on the front buffer\n");
			return TRUE;
		}
	} else {
		FastMemSet(&ddsd,0,sizeof(ddsd));		/* Init surface struct */
		ddsd.dwSize = sizeof(ddsd);		/* Set the size */
		ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;	/* Clip me */
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		if (Flags&SETDISPLAYD3D) {
			ddsd.ddsCaps.dwCaps |=DDSCAPS_3DDEVICE;
		}
		ddsd.dwWidth = Width;		/* Set width and height */
		ddsd.dwHeight = Height;

		if (IDirectDraw_CreateSurface(DirectDrawPtr,
			&ddsd,&Win95BackBuffer,0) != DD_OK) {
			ReleaseVideo();
			NonFatal("Couldn't create the backbuffer for window\n");
			return TRUE;
		}

		if (IDirectDraw_CreateClipper(DirectDrawPtr,
			0,&Win95WindowClipperPtr,0) != DD_OK) {
			ReleaseVideo();
			NonFatal("Couldn't create clipper\n");
			return TRUE;
		}

		if (IDirectDrawClipper_SetHWnd(Win95WindowClipperPtr,
			0,(HWND)Win95MainWindow) != DD_OK) {
			ReleaseVideo();
			NonFatal("Couldn't set clipper hwnd\n");
			return TRUE;
		}
		if (IDirectDrawSurface_SetClipper(Win95FrontBuffer,
			Win95WindowClipperPtr) != DD_OK) {
			ReleaseVideo();
			NonFatal("Couldn't set clipper for frontbuffer\n");
			return TRUE;
		}
		{
			Word j;
			HDC WinDC;
			struct {
				BITMAPINFOHEADER bmiHeader;
				RGBQUAD bmiColors[256];
			} Foo;
			RGBQUAD *FooPtr;

			j = 0;
			FooPtr = Foo.bmiColors;
			do {
				FooPtr->rgbRed = CurrentPalette[j];
				FooPtr->rgbGreen = CurrentPalette[j+1];
				FooPtr->rgbBlue = CurrentPalette[j+2];
				FooPtr->rgbReserved = 0;
				++FooPtr;
				j = j+3;
			} while (j<768);

			VideoHardWidth = (Depth/8)*Width;
			Foo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			Foo.bmiHeader.biWidth = Width;
			Foo.bmiHeader.biHeight = -(int)Height;
			Foo.bmiHeader.biPlanes = 1;
			Foo.bmiHeader.biBitCount = static_cast<Word16>(Depth);
			Foo.bmiHeader.biCompression = 0;
			Foo.bmiHeader.biSizeImage = Height*VideoHardWidth;
			Foo.bmiHeader.biXPelsPerMeter = 0;
			Foo.bmiHeader.biYPelsPerMeter = 0;
			Foo.bmiHeader.biClrUsed = 0;
			Foo.bmiHeader.biClrImportant = 0;

			WinDC = GetDC((HWND)Win95MainWindow);		/* Get my device */
			BurgerHBitMap = CreateDIBSection(WinDC,(LPBITMAPINFO)&Foo,DIB_RGB_COLORS,
				(void **)&BurgerHBitMapPtr,0,0);
			VideoOffscreen = BurgerHBitMapPtr;
			if (Depth==16) {
				HDC wdc;
				Word32 RGBVal;
				wdc = CreateCompatibleDC(WinDC);
				SelectObject(wdc,BurgerHBitMap);
				((Word16 *)VideoOffscreen)[0] = 0x7FFF;
				RGBVal = GetPixel(wdc,0,0);
				DeleteDC(wdc);
				if ((RGBVal&0xFFFFFF)==0xFFFFFF) {
					Depth = 15;
				}
			}
			ReleaseDC((HWND)Win95MainWindow,WinDC);
		}
	}

	/* Prepare a color palette struct for the palette manager */

	if (Depth<=8) {
		struct {
			WORD Ver;
			WORD Count;
			PALETTEENTRY pe[256];
		} MyPal;
		PALETTEENTRY *peptr;
		Word8 *PalPtr;
		Word i;

		i = 0;
		peptr = MyPal.pe;		/* Init memory for palette */
		PalPtr = CurrentPalette;	/* Init with my internal palette */
		CurrentPalette[0] = 0;		/* Fix the first and last colors */
		CurrentPalette[1] = 0;
		CurrentPalette[2] = 0;
		CurrentPalette[765] = 255;
		CurrentPalette[766] = 255;
		CurrentPalette[767] = 255;
		do {
			peptr->peRed = PalPtr[0];		/* Copy over the palette */
			peptr->peGreen = PalPtr[1];
			peptr->peBlue = PalPtr[2];
			peptr->peFlags = 0;
			++peptr;		/* Next entry */
			PalPtr+=3;
		} while (++i<256);		/* All colors done? */
		/* Create a direct draw palette struct */

		if (IDirectDraw_CreatePalette(DirectDrawPtr,
			DDPCAPS_8BIT,MyPal.pe,&Win95WindowPalettePtr,0) != DD_OK) {
			ReleaseVideo();
			NonFatal("Couldn't create the palette\n");
			return TRUE;
		}
		/* Attach the palette to the video display */

		if (IDirectDrawSurface_SetPalette(Win95FrontBuffer,
			Win95WindowPalettePtr)==DDERR_SURFACELOST) {
			IDirectDrawSurface_Restore(Win95FrontBuffer);
			IDirectDrawSurface_SetPalette(Win95FrontBuffer,
				Win95WindowPalettePtr);
		}

		if (!(Flags&SETDISPLAYFULLSCREEN)) {		/* Is this a window? */
			MyPal.Ver = 0x300;
			MyPal.Count = 256;
			BurgerHPalette = CreatePalette((LOGPALETTE *)&MyPal);
		}
	}


	/* Test the memory to see if I can lock it down */

	FastMemSet(&ddsd,0,sizeof(ddsd));		/* Blank the surface struct */
	ddsd.dwSize = sizeof(ddsd);
	{
		int Foo;
		Foo = IDirectDrawSurface_Lock(Win95BackBuffer,0,&ddsd,DDLOCK_WAIT,0);
		if (Foo==DDERR_SURFACELOST) {
			IDirectDrawSurface_Restore(Win95BackBuffer);
			Foo = IDirectDrawSurface_Lock(Win95BackBuffer,0,&ddsd,DDLOCK_WAIT,0);
		}
		if (Foo != DD_OK) {
			ReleaseVideo();
			NonFatal("I could not lock the backbuffer!\n");
			return TRUE;
		}
	}
	IDirectDrawSurface_Unlock(Win95BackBuffer,0);

	/* Is this an offscreen buffer? */

	if (!VideoOffscreen && Flags&SETDISPLAYOFFSCREEN) {
		VideoHardWidth = ((Depth+7)>>3)*Width;
		VideoOffscreen = (Word8 *)AllocAPointer(VideoHardWidth*Height);
	}
	/* If I got a 16 bit mode, test if it's really 16 bit or 15 bit */

	if (Depth==16 && !BurgerHBitMap) {
		HDC hdc;			/* Temp hardware driver context */
		DDSURFACEDESC ddsd;
		struct IDirectDrawSurface *FooPtr;
		FooPtr = Win95BackBuffer;
		if (IDirectDrawSurface_GetDC(FooPtr,&hdc) == DD_OK) {
			goto GotTheDC;
		}
		FooPtr = Win95FrontBuffer;
		if (IDirectDrawSurface_GetDC(FooPtr,&hdc) == DD_OK) {
GotTheDC:
			SetPixel(hdc,0,0,RGB(0,255,0));
			IDirectDrawSurface_ReleaseDC(FooPtr,hdc);
			ddsd.dwSize = sizeof(ddsd);
			if (IDirectDrawSurface_Lock(FooPtr, NULL, &ddsd, DDLOCK_WAIT, NULL) == DD_OK) {
				if ((((Word16 *)ddsd.lpSurface)[0] & 0x07E0) != 0x7E0) {
					Depth = 15;		/* RGB 5.5.5 */
				}
				IDirectDrawSurface_Unlock(FooPtr,NULL);
			}
			IDirectDrawSurface_ReleaseDC(FooPtr,hdc);
		}
	}
	/* Tell the mouse manager about the screen */

	MouseSetRange(ScreenWidth,ScreenHeight);
	VideoColorDepth = Depth;
	VideoTrueScreenDepth = Depth;

	/* These variables only matter in true color modes */

	if (Depth==16 && !Fake15) {
		VideoRedShift = 11;			/* I am going to draw in a 16 bit context */
		VideoRedMask = 0x1F<<11;
		VideoGreenBits = 6;
		VideoGreenMask = 0x3f<<5;
	} else {
		if (Depth==16 && Fake15) {	/* I am a FAKE 5:5:5 mode */
			VideoColorDepth = 15;
		}
		VideoRedShift = 10;
		VideoRedMask = 0x1F<<10;
		VideoGreenBits = 5;
		VideoGreenMask = 0x1F<<5;
	}
	VideoBlueShift = 0;
	VideoBlueMask = 0x1f;
	VideoBlueBits = 5;
	VideoGreenShift = 5;
	VideoRedBits = 5;

	LockVideoPage();			/* Shake up the video page */
	UnlockVideoPage();			/* Release the video page */
	if (GammaOn || GammaTry) {
		VideoOSGammaInit();		/* Restore Gamma control */
	}
	GammaTry = FALSE;			/* I succeeded! */
	return FALSE;				/* I am cool! */
}

/**********************************

	If a window is present, set the text to a specific string

**********************************/

void BURGERCALL VideoSetWindowString(const char *Title)
{
	if (Win95MainWindow) {				/* Is the window present? */
		SetWindowText((HWND)Win95MainWindow,Title);	/* Set the new title text */
	}
}

/**********************************

	Return a handle to an array of video display modes
	Note : This code is long and convoluted, just
	like DirectX!!!

**********************************/

/**********************************

	This is called for each unique display mode.
	I have a pointer to the current 3D device
	attached to this surface so I can determine
	which modes have hardware acceleration and which ones
	don't.

	Note : I call StreamHandlePutMem() to prevent running out of memory.
	Since Intel machines already use little endian, I don't worry about
	the little endianess of StreamHandlePutMem.
	This data is intended for local	use, not global use

**********************************/

typedef struct ModeCallBack_t {
	StreamHandle_t *StreamPtr;		/* Pointer to the output stream */
	D3DDEVICEDESC *Device3D;		/* Pointer to the 3D device (If any!) */
} ModeCallBack_t;

static long __stdcall ModeCallBack(DDSURFACEDESC *SurfacePtr,void *MyStream)
{
	ModeCallBack_t *FooPtr;		/* Work pointer */
	VideoMode_t MyEntry;	/* Video mode found */
	Word Flags;

	FooPtr = (ModeCallBack_t *)MyStream;	/* Typedef the pointer */
	MyEntry.Width = SurfacePtr->dwWidth;	/* Save the pixel width */
	MyEntry.Height = SurfacePtr->dwHeight;
	MyEntry.Depth = SurfacePtr->ddpfPixelFormat.dwRGBBitCount;	/* Depth */
	MyEntry.Hertz = SurfacePtr->dwRefreshRate;					/* Monitor refresh rate */

	Flags = 0;
	if (MyEntry.Depth>8) {					/* True color modes are OpenGL */
		Flags |= VIDEOMODEOPENGL;
	}

	if (MyEntry.Depth<=8) {
		if (FooPtr->Device3D->dwDeviceRenderBitDepth & DDBD_8) {		/* Can I render to 8 bit mode? */
			Flags |= VIDEOMODEHARDWARE;
		}
	} else if (MyEntry.Depth<=16) {
		if (FooPtr->Device3D->dwDeviceRenderBitDepth & DDBD_16) {		/* Can I render in 16 bit mode? */
			Flags |= VIDEOMODEHARDWARE;
		}
	} else if (MyEntry.Depth<=24) {
		if (FooPtr->Device3D->dwDeviceRenderBitDepth & DDBD_24) {		/* Can I render in 24 bit mode? */
			Flags |= VIDEOMODEHARDWARE;
		}
	} else if (MyEntry.Depth<=32) {
		if (FooPtr->Device3D->dwDeviceRenderBitDepth & DDBD_32) {		/* Can I render in 32 bit mode? */
			Flags |= VIDEOMODEHARDWARE;
		}
	}
	MyEntry.Flags = Flags;				/* Save the flags */
	StreamHandlePutMem(FooPtr->StreamPtr,&MyEntry,sizeof(MyEntry));

	return DDENUMRET_OK;	/* Get another one! */
}

/**********************************

	Given a IDirectDraw instance, determine whether
	or not hardware exists for each and every
	display mode available. I will then return a VideoModeArray_t
	handle with the list of supported modes.

	I must be given the device's name. This cannot be gleaned
	from the IDirectDraw interface. Only a IDirectDraw4 interface
	can give me this.

	The D3DDEVICEDESC comes from the D3D instance that comes
	from the IDirectDraw instance

	Note : I support DirectX 6 and lower!.

**********************************/

static VideoModeArray_t **GetModeArray(IDirectDraw *MyDD,Word DevNum,char *DevName,D3DDEVICEDESC *DevPtr)
{
	StreamHandle_t MyStream;
	Word Output;
	VideoModeArray_t **Result;
	ModeCallBack_t Ref;

	StreamHandleInitPut(&MyStream);		/* Init the output stream */
	StreamHandlePutMem(&MyStream,&Output,sizeof(Output));	/* Save space for the count */
	StreamHandlePutMem(&MyStream,&DevNum,sizeof(DevNum));
	StreamHandlePutMem(&MyStream,DevName,64);

	/* Is it DirectX 6 or higher? */

	Ref.StreamPtr = &MyStream;
	Ref.Device3D = DevPtr;
	if (IDirectDraw_EnumDisplayModes(MyDD,DDEDM_REFRESHRATES,0,(void *)&Ref,ModeCallBack)!=DD_OK) {
		if (IDirectDraw_EnumDisplayModes(MyDD,0,0,(void *)&Ref,ModeCallBack) != DD_OK) {	/* Oh oh... */
			goto Darn;		/* Oh, forget it! */
		}
	}

	StreamHandleEndSave(&MyStream);				/* Wrap up the save */
	if (StreamHandleGetErrorFlag(&MyStream)) {	/* Was there an error in saving? */
		goto Darn;			/* I surrender */
	}

	Result = (VideoModeArray_t **)StreamHandleDetachHandle(&MyStream);		/* Get the handle */
	Output = (GetAHandleSize((void **)Result)-(sizeof(Word)+64))/sizeof(VideoMode_t);	/* How large is it */
	Result[0]->Count = Output;		/* Save the length */
	StreamHandleDestroy(&MyStream);	/* Dispose of the structure */
	if (Output) {					/* Any data collected? */
		return Result;				/* Return the handle */
	}
	DeallocAHandle((void **)Result);	/* Kill the handle */
	return 0;			/* I surrender */
Darn:
	StreamHandleDestroy(&MyStream);		/* I surrender */
	return 0;	/* Bogus!! */
}

/**********************************

	This callback is used to find the D3D
	3D rendering hardware attached to a surface.

	I make the assumption that you only have a single
	device attached to the surface.

	I do not want to even THINK about supporting multiple
	rendering devices on a single surface. This makes
	my brain hurt.

**********************************/

static long __stdcall D3DDevEnum(GUID * /* pGUID */,char * /*pDeviceDescription*/,
	char * /*pDeviceName */,D3DDEVICEDESC *HWDescPtr,D3DDEVICEDESC * /*HELDescPtr*/,
	void *Context)
{
	if (!HWDescPtr->dcmColorModel) {		/* I only want HARDWARE! */
		return D3DENUMRET_OK;				/* Try again */
	}
	FastMemCpy(Context,HWDescPtr,sizeof(D3DDEVICEDESC));	/* Get the device */
	return D3DENUMRET_CANCEL;				/* Stop NOW! */
}

/**********************************

	This callback is used to find all of
	the video cards. There could be more than one
	video card/monitor hooked up to any PC, so as
	a result, I need to check all devices and return
	the info on any one of them.

**********************************/

typedef struct AllDev_t {
	StreamHandle_t *StreamPtr;		/* Pointer to output stream */
	Word DevNum;
	Word OnlyOne;
} AllDev_t;

static int __stdcall EnumCallBack2(GUID *pGUID,char *pDescription,char * /*pName*/,void *Context,HMONITOR /*Monitor*/)
{
	IDirectDraw *pDD;
	IDirectDraw4 *pDD4;
	DDCAPS DriverCaps;
	DDCAPS HardwareCaps;
	Word Hardware3D;
	char *DeviceName;
	char DeviceNameBuf[64];
	D3DDEVICEDESC D3DDeviceDesc;
	VideoModeArray_t **TempResult;
	AllDev_t *FooPtr;

	FooPtr = (AllDev_t *)Context;
	FooPtr->DevNum++;

	/* Am I searching for a specific device or all of them? */

	if (FooPtr->OnlyOne) {		/* Specific device */
		if (FooPtr->OnlyOne!=FooPtr->DevNum) {
			return DDENUMRET_OK;
		}
	}

	/* I want this device, can I get the info? */

	if (CallDirectDrawCreate(pGUID,&pDD,0)==DD_OK) {

		/* Determine if 3D hardware is present for this device */

		FastMemSet(&DriverCaps,0,sizeof(DriverCaps));
		FastMemSet(&HardwareCaps,0,sizeof(HardwareCaps));
		DriverCaps.dwSize = sizeof(DriverCaps);
		HardwareCaps.dwSize = sizeof(HardwareCaps);
		if (IDirectDraw_GetCaps(pDD,&DriverCaps,&HardwareCaps)!=DD_OK) {
			FastMemSet(&DriverCaps,0,sizeof(DriverCaps));		/* Zark on error */
			FastMemSet(&HardwareCaps,0,sizeof(HardwareCaps));
		}
		Hardware3D = DriverCaps.dwCaps & DDCAPS_3D;		/* Allow 3D? */

		/* Get the name of the device */

		if (IDirectDraw_QueryInterface(pDD,IID_IDirectDraw4,(void **)&pDD4) == DD_OK) {
			DDDEVICEIDENTIFIER DeviceInfo;
			if (IDirectDraw4_GetDeviceIdentifier(pDD4,&DeviceInfo,0)!=DD_OK) {
				DeviceName = pDescription;
			} else {
				strncpy(DeviceNameBuf,DeviceInfo.szDescription,64);
				DeviceNameBuf[63] = 0;
				DeviceName = DeviceNameBuf;
			}
			IDirectDraw4_Release(pDD4);
		} else {
			DeviceName = pDescription;
		}

		/* Get the type of 3D hardware I have */

		if (Hardware3D) {			/* Do I bother? */
			IDirect3D *pD3D;
			if (IDirectDraw_QueryInterface(pDD,IID_IDirect3D,(void **)&pD3D)==DD_OK) {
				IDirect3D_EnumDevices(pD3D,D3DDevEnum,(void *)&D3DDeviceDesc);
				if (!D3DDeviceDesc.dwSize) {
					Hardware3D = 0;			/* I don't have hardware */
				}
				IDirect3D_Release(pD3D);
			}
		}

		/* Now get the modes present */

		TempResult = GetModeArray(pDD,FooPtr->DevNum,DeviceName,&D3DDeviceDesc);
		if (TempResult) {
			if (FooPtr->OnlyOne) {
				FooPtr->StreamPtr = (StreamHandle_t *)TempResult;
				return DDENUMRET_CANCEL;
			}
			StreamHandlePutMem(FooPtr->StreamPtr,&TempResult,sizeof(TempResult));
		}
	}
	return DDENUMRET_OK;
}

/**********************************

	Check for direct draw already started and
	perform the check.

**********************************/

VideoModeArray_t **BURGERCALL VideoModeArrayNew(Word HardwareOnly)
{
	AllDev_t Ref;
	Word Output;

	Output = HardwareOnly&0xFF;
	if (!Output) {
		Output = 1;
	}
	Ref.StreamPtr = 0;
	Ref.OnlyOne = Output;
	Ref.DevNum = 0;

	if (CallDirectDrawEnumerateExA((int (__stdcall*)(struct _GUID*, char*, char*, void*, void*))EnumCallBack2,(void *)&Ref,DDENUM_ATTACHEDSECONDARYDEVICES|
		DDENUM_DETACHEDSECONDARYDEVICES|DDENUM_NONDISPLAYDEVICES)==DD_OK) {
		return (VideoModeArray_t **)Ref.StreamPtr;
	}
	return 0;
}

/**********************************

	Create the device array

**********************************/

VideoDeviceArray_t **BURGERCALL VideoDeviceArrayNew(Word /* HardwareOnly */)
{
	AllDev_t Ref;
	StreamHandle_t MyStream;
	Ref.StreamPtr = &MyStream;
	Ref.OnlyOne = 0;
	Ref.DevNum = 0;
	StreamHandleInitPut(&MyStream);
	StreamHandlePutLong(&MyStream,0);
	if (CallDirectDrawEnumerateExA((int(__stdcall*)(struct _GUID*, char*, char*, void*, void*))EnumCallBack2,(void *)&Ref,DDENUM_ATTACHEDSECONDARYDEVICES|
		DDENUM_DETACHEDSECONDARYDEVICES|DDENUM_NONDISPLAYDEVICES)==DD_OK) {
		VideoDeviceArray_t **Result;
		Word32 Output;

		StreamHandleEndSave(&MyStream);
		Result = (VideoDeviceArray_t **)StreamHandleDetachHandle(&MyStream);
		StreamHandleDestroy(&MyStream);
		Output = (GetAHandleSize((void **)Result)-(sizeof(Word)))/sizeof(VideoModeArray_t **);	/* How large is it */
		Result[0]->Count = Output;		/* Save the length */
		if (Output) {					/* Any data collected? */
			return Result;				/* Return the handle */
		}
		DeallocAHandle((void **)Result);	/* Kill the handle */
		return 0;			/* I surrender */
	}
	StreamHandleDestroy(&MyStream);
	return 0;
}

/**********************************

	This is a Windows 95 ONLY function

	By invoking DEEP magic, I will divine the version
	of DirectX that is present. Based on sample code
	provided by Microsoft Inc.

	Returned values.
	0		No DirectX installed
	0x100	DirectX version 1 installed
	0x200	DirectX 2 installed
	0x300	DirectX 3 installed
	0x500	At least DirectX 5 installed.
	0x501	At least DirectX 5a installed.


**********************************/

/**********************************

	Subroutine.
	Returns TRUE if DirectInput is present

**********************************/

static Word BURGERCALL IsDirectInputPresent(void)
{
	HINSTANCE DIHinst;		/* DLL instance */
	Word Result;

	Result = FALSE;
	DIHinst = LoadLibrary("DINPUT.DLL");	/* Load the DLL */
	if (DIHinst) {				/* Good! */
		if (GetProcAddress(DIHinst,"DirectInputCreateA")) {
			Result = TRUE;		/* The code is good! */
		}
		FreeLibrary(DIHinst);		/* Get rid of the library */
	}
	return Result;		/* TRUE if ok! */
}

/**********************************

	Perform the deep magic

**********************************/

Word BURGERCALL GetDirectXVersion(void)
{
	OSVERSIONINFO osVer;	/* Used to get the OS I'm running under */

	Word Ver;
	LPDIRECTDRAW pDDraw;	/* DirectDraw1 class */
	LPDIRECTDRAW2 pDDraw2;	/* DirectDraw2 class */
	LPDIRECTDRAWSURFACE pSurf;	/* DirectDrawSurface1 class */
	LPDIRECTDRAWSURFACE3 pSurf3;	/* DirectDrawSurface3 class */
	DDSURFACEDESC desc;		/* Test surface proc */

	Ver = 0;		/* I assume version 0! */

	/* First get the windows platform */
	/* I con only run under WindowsNT and Windows 95 */

	osVer.dwOSVersionInfoSize = sizeof(osVer);
	if (GetVersionEx(&osVer)) {		/* Get the version */

	/* NT is easy... NT 4.0 is DX2, 4.0 SP3 is DX3, 5.0 is DX5 */
	/* and no DX on earlier versions. */

		if (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT) {
			if (osVer.dwMajorVersion >= 4) {

			/* NT4 up to Service Pack 2 is DX2 */
			/* SP3 and up is DX3 */

				if (osVer.dwMajorVersion == 4) {

				/* There is no way to determine the service pack number */
				/* I cheat, check for the presence of DirectInput */
				/* If present, then Service Pack 3 is installed */

					if (!IsDirectInputPresent()) {
						return 0x200;
					}
					return 0x300; //DX3 on NT4 SP3 or higher
				}
				return 0x501;	//DX5a on NT5
			}
		} else {

		/* I must be running Windows 95 */

		/* Windows 98? */

			if ( (osVer.dwBuildNumber & 0xffff) > 1353) { //Check for higher than developer release
				return 0x501; //DX5a on Memphis or higher
			}


		/* At this point in time, we have NO clue what is installed */
		/* we may even be Windows 3.1 (Eck!) */

	/* See if we can create the DirectDraw object. */

			if (CallDirectDrawCreate(NULL, &pDDraw, NULL)>=0) {	/* Got an object? */
				Ver = 0x100;		/* DX 1! */

	/* Now is a DirectDraw2 object present? */

				if (IDirectDraw_QueryInterface(pDDraw,(IID_IDirectDraw2), (LPVOID *)&pDDraw2)>=0) {
					IDirectDraw2_Release(pDDraw2);
					Ver = 0x200;		/* DX 2! */

	/* DirectInput is in DX3 */

					if (IsDirectInputPresent()) {
#if 0
						Ver = 0x300;	/* DX 3! */
#endif
						Ver = 0;		/* We are assuming a future failure */

	/* Now the biggie, try DX5 */
	/* Look for a IDirectDrawSurface3 */

						FastMemSet(&desc,0,sizeof(desc));
						desc.dwSize = sizeof(desc);
						desc.dwFlags = DDSD_CAPS;
						desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	/* Make sure it's installed properly */

						if (IDirectDraw_SetCooperativeLevel(pDDraw,NULL,DDSCL_NORMAL)>=0) {
							if (IDirectDraw_CreateSurface(pDDraw,&desc, &pSurf, NULL)>=0) {

	/* Let's try to get a DirectDrawSurface3 */
								Ver = 0x300;
								if (IDirectDrawSurface_QueryInterface(pSurf,(IID_IDirectDrawSurface3),(LPVOID*)&pSurf3)>=0) {
									IDirectDrawSurface3_Release(pSurf);
									Ver = 0x500;	/* DX 5 */
								}
							}
						}
					}
				}
				IDirectDraw_Release(pDDraw);
			}
		}
	}
	return Ver;
}

/**********************************

	Draw a rectangle in a solid color

**********************************/

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

	/* Win95 has hardware support! */

		if (VideoPointer==Win95VideoPointer) {
			RECT MyRect;
			if (!Win95UseBackBuffer) {	/* Windowed? */
				x += Win95DestScreenX;	/* Adjust for window position */
				y += Win95DestScreenY;
			}
			MyRect.left = x;			/* Set the rect */
			MyRect.right = x+Width;
			MyRect.top = y;
			MyRect.bottom = y+Height;
			BlitTable.dwFillColor = Color;	/* Color to fill with */
			if (VideoPageLocked) {		/* Video locked? */
				UnlockVideoPage();
				IDirectDrawSurface_Blt(Win95WorkBuffer,&MyRect,0,0,DDBLT_COLORFILL | DDBLT_ASYNC,&BlitTable);
				LockVideoPage();
				return;
			}
			IDirectDrawSurface_Blt(Win95WorkBuffer,&MyRect,0,0,DDBLT_COLORFILL | DDBLT_ASYNC,&BlitTable);
			return;
		}

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

/**********************************

	Draw a rectangle in a solid color

**********************************/

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

	if (VideoPointer==Win95VideoPointer) {
		RECT MyRect;
		if (!Win95UseBackBuffer) {	/* Windowed? */
			x += Win95DestScreenX;	/* Adjust for window position */
			y += Win95DestScreenY;
		}
		MyRect.left = x;			/* Set the rect */
		MyRect.right = x+Width;
		MyRect.top = y;
		MyRect.bottom = y+Height;
		BlitTable.dwFillColor = Color;	/* Color to fill with */
		if (VideoPageLocked) {		/* Video locked? */
			UnlockVideoPage();
			IDirectDrawSurface_Blt(Win95WorkBuffer,&MyRect,0,0,DDBLT_COLORFILL | DDBLT_ASYNC,&BlitTable);
			LockVideoPage();
			return;
		}
		IDirectDrawSurface_Blt(Win95WorkBuffer,&MyRect,0,0,DDBLT_COLORFILL | DDBLT_ASYNC,&BlitTable);
		return;
	}

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

/**********************************

	Draw a rectangle in a solid color

**********************************/

void BURGERCALL DrawARectDirectX(int x,int y,Word Width,Word Height,Word Color)
{
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

	/* Win95 has hardware support! */

		RECT MyRect;
		if (!Win95UseBackBuffer) {	/* Windowed? */
			x += Win95DestScreenX;	/* Adjust for window position */
			y += Win95DestScreenY;
		}
		MyRect.left = x;			/* Set the rect */
		MyRect.right = x+Width;
		MyRect.top = y;
		MyRect.bottom = y+Height;
		BlitTable.dwFillColor = Color;	/* Color to fill with */
		IDirectDrawSurface_Blt(Win95WorkBuffer,&MyRect,0,0,DDBLT_COLORFILL | DDBLT_ASYNC,&BlitTable);
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

void BURGERCALL DrawARectDirectXTo16(int x,int y,Word Width,Word Height,Word Color)
{
	DrawARectDirectX(x,y,Width,Height,Table8To16Ptr[Color]);
}

/**********************************

	This is used by D3DDeviceEnumTextures()

	Enumerate the texture surface types and
	log all the ones I support, this is how I know in advance
	what texture formats I can use

**********************************/

static HRESULT __stdcall TextureCheckerProc(DDSURFACEDESC *Input,void * /*d*/)
{
	Word Found;

	Found = 0;
	if ((Input->dwFlags & DDSD_PIXELFORMAT) && (Input->ddsCaps.dwCaps & DDSCAPS_TEXTURE)) {		/* Must have a pixel format */
		if (Input->ddpfPixelFormat.dwFlags & DDPF_RGB) {		/* RGB format valid? */
			/* 16 bit format? */

			if (Input->ddpfPixelFormat.dwRGBBitCount==16) {
				if (Input->ddpfPixelFormat.dwBBitMask==0x1F) {		/* 555, 565 or 1555? */
					if (Input->ddpfPixelFormat.dwGBitMask==(0x1F<<5)) {
						if (Input->ddpfPixelFormat.dwRBitMask==(0x1F<<10)) {
							if (Input->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) {
								if (Input->ddpfPixelFormat.dwRGBAlphaBitMask==0x8000) {
									Found = VIDEOTEXTURETYPE1555;	/* 1:5:5:5 RGB */
								}
							} else {
								Found = VIDEOTEXTURETYPE555;		/* 5:5:5 RGB */
							}
						}
					} else if (Input->ddpfPixelFormat.dwGBitMask==(0x3F<<5)) {
						if (Input->ddpfPixelFormat.dwRBitMask==(0x1F<<11)) {
							Found = VIDEOTEXTURETYPE565;		/* 5:6:5 RGB */
						}
					}
				} else if (Input->ddpfPixelFormat.dwBBitMask==0xF) {
					if (Input->ddpfPixelFormat.dwGBitMask==(0xF<<4)) {
						if (Input->ddpfPixelFormat.dwRBitMask==(0xF<<8)) {
							if (Input->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) {
								if (Input->ddpfPixelFormat.dwRGBAlphaBitMask==(0xF<<12)) {
									Found = VIDEOTEXTURETYPE4444;
								}
							}
						}
					}
				}
			} else if (Input->ddpfPixelFormat.dwRGBBitCount==8) {
				if (Input->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) {
					Found = VIDEOTEXTURETYPE8PAL;
				}
			}
		}
	}
	VideoTextureTypesAllowed |= Found;
	return DDENUMRET_OK;
}

/**********************************

	Given a Direct3D Device,
	find out what kind of texture formats
	the device supports

**********************************/

void BURGERCALL D3DGetTextureInfo(void)
{
	D3DDEVICEDESC HWDesc;			/* Description of the hardware */
	D3DDEVICEDESC HELDesc;			/* Description of the emulator */

	VideoTextureTypesAllowed = 0;	/* Assume no texture formats available */
	VideoTextureRules = 0;			/* Assume no texture rules */

	if (Direct3DDevicePtr) {		/* Failsafe */
		/* What texture formats are allowed by this device? */

		IDirect3DDevice_EnumTextureFormats(Direct3DDevicePtr,TextureCheckerProc,0);		/* Get the texture formats allowed */

	/* What rules or limits do the textures have to adhere to? */

		FastMemSet(&HWDesc,0,sizeof(HWDesc));
		FastMemSet(&HELDesc,0,sizeof(HELDesc));
		HWDesc.dwSize = sizeof(HWDesc);
		HELDesc.dwSize = sizeof(HELDesc);
		if (IDirect3DDevice_GetCaps(Direct3DDevicePtr,&HWDesc,&HELDesc)==DD_OK) {		/* Should ALWAYS be good! */
			if (HWDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2) {
				VideoTextureRules |= VIDEOTEXTUREPOW2;			/* Must be a power of 2 in size */
			}
			if (HWDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY) {
				VideoTextureRules |= VIDEOTEXTURESQUARE;		/* Must be a square */
			}
			if (HWDesc.dwDevCaps & D3DDEVCAPS_TEXTURESYSTEMMEMORY) {
				VideoTextureRules |= VIDEOTEXTURESYSTEMMEMORY;	/* YES! Textures can be in system memory!!! */
			}
			VideoTextureMinWidth = HWDesc.dwMinTextureWidth;	/* Texture boundaries */
			VideoTextureMaxWidth = HWDesc.dwMaxTextureWidth;
			VideoTextureMinHeight = HWDesc.dwMinTextureHeight;
			VideoTextureMaxHeight = HWDesc.dwMaxTextureHeight;
			VideoVertexMaxCount = HWDesc.dwMaxVertexCount;		/* Maximum number of vertexs allowed */
			if (!VideoTextureMaxWidth) {				/* Some sanity checks */
				VideoTextureMaxWidth = (Word)-1;
	 		}
			if (!VideoTextureMaxHeight) {
				VideoTextureMaxHeight = (Word)-1;
			}
			if (!VideoVertexMaxCount || VideoVertexMaxCount>8192) {		/* Arbitrary limit on the vertexs */
				VideoVertexMaxCount = 8192;					/* I'll max out at 8192 */
			}
		}
	}
}

/**********************************

	Set a standard viewport for the direct3D device

**********************************/

void BURGERCALL D3DSetStandardViewport(void)
{
	D3DVIEWPORT MyPort;
 	/* Now I've got the texture rules, I need to create a standard viewport */

	if (Direct3DPtr) {			/* Failsafe */
	 	if (IDirect3D_CreateViewport(Direct3DPtr,&Direct3DViewPortPtr,0)==D3D_OK) {	/* Should always be ok */
 			IDirect3DDevice_AddViewport(Direct3DDevicePtr,Direct3DViewPortPtr);		/* Add it to my device */
			FastMemSet(&MyPort,0,sizeof(MyPort));			/* Set my viewport to a default */
			MyPort.dwSize = sizeof(MyPort);
			MyPort.dwX = 0;
			MyPort.dwY = 0;
			MyPort.dwWidth = ScreenWidth;					/* Screen size */
			MyPort.dwHeight = ScreenHeight;
			MyPort.dvScaleX = (float)(ScreenWidth>>1);		/* Set default to half screen size */
			MyPort.dvScaleY = (float)(ScreenHeight>>1);
			MyPort.dvMaxX = 1.0f;
			MyPort.dvMaxY = 1.0f;
			IDirect3DViewport_SetViewport(Direct3DViewPortPtr,&MyPort);
		}
	}
}

/**********************************

	Create a zbuffer if I can support it

**********************************/

void BURGERCALL D3DCreateZBuffer(void)
{
    DDSURFACEDESC ddsd;
	D3DDEVICEDESC D3DDeviceDesc;		/* What kind of device is this? */

	if (Direct3DZBufferPtr) {		/* I'm paranoid */
		IDirectDrawSurface_Release(Direct3DZBufferPtr);
		Direct3DZBufferPtr = 0;
	}

	if (Direct3DPtr) {
		FastMemSet(&D3DDeviceDesc,0,sizeof(D3DDeviceDesc));
		IDirect3D_EnumDevices(Direct3DPtr,D3DDevEnum,(void *)&D3DDeviceDesc);
		if (D3DDeviceDesc.dwSize) {
			// If we do not have z buffering support
			// on this driver, give up now
			FastMemSet(&ddsd,0,sizeof(DDSURFACEDESC));
		    ddsd.dwSize = sizeof(DDSURFACEDESC);
			ddsd.dwFlags = (DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_ZBUFFERBITDEPTH);
			ddsd.dwHeight = ScreenHeight;
			ddsd.dwWidth = ScreenWidth;
			ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;

			// If we are on a hardware driver, then the z buffer
			// MUST be in video memory.  Otherwise, it MUST be
			// in system memory.  I think.

			ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;

		    // Get the Z buffer bit depth from this driver's
			// D3D device description and add it to the description
			// of the surface we want to create

			if (D3DDeviceDesc.dwDeviceZBufferBitDepth&DDBD_32) {
				ddsd.dwZBufferBitDepth = 32;
			} else if (D3DDeviceDesc.dwDeviceZBufferBitDepth&DDBD_24) {
				ddsd.dwZBufferBitDepth = 24;
			} else if (D3DDeviceDesc.dwDeviceZBufferBitDepth&DDBD_16) {
				ddsd.dwZBufferBitDepth = 16;
			} else {
				return;
			}

	    	// Now we must actually make the z buffer

			if (IDirectDraw_CreateSurface(DirectDrawPtr,&ddsd,&Direct3DZBufferPtr, NULL)==DD_OK) {
    			if (IDirectDrawSurface_AddAttachedSurface(Win95BackBuffer,Direct3DZBufferPtr)==DD_OK) {
					Direct3DZBufferBitDepth = ddsd.dwZBufferBitDepth;	/* Save global */
					return;
				}
				IDirectDrawSurface_Release(Direct3DZBufferPtr);
				Direct3DZBufferPtr = 0;
			}
		}
	}
}

/**********************************

	Initialize the Direct3d execute buffer

**********************************/

void BURGERCALL D3DInitExecuteBuffer(void)
{
	D3DEXECUTEBUFFERDESC MyDesc;

	/* Now let's create the execute buffer for the D3D opcodes */

	if (Direct3DDevicePtr) {
		FastMemSet(&MyDesc,0,sizeof(MyDesc));
		Direct3DExecBuffSize = ((sizeof(D3DINSTRUCTION)+sizeof(D3DTRIANGLE)) * MAXD3DINSTRUCTIONS) + (sizeof(D3DTLVERTEX)*VideoVertexMaxCount);
		MyDesc.dwSize = sizeof(MyDesc);
		MyDesc.dwFlags = D3DDEB_BUFSIZE|D3DDEB_CAPS;		/* I have caps and the buffer size */
		MyDesc.dwBufferSize = Direct3DExecBuffSize;			/* Set the buffer size */
		MyDesc.dwCaps = D3DDEBCAPS_SYSTEMMEMORY;			/* I want system memory */
		if (IDirect3DDevice_CreateExecuteBuffer(Direct3DDevicePtr,&MyDesc,&Direct3DExecBufPtr,0)==D3D_OK) {	/* Did I get it? */
			D3DEXECUTEDATA MyData;

			FastMemSet(&MyDesc,0,sizeof(MyDesc));			/* Reset the description */
			MyDesc.dwSize = sizeof(MyDesc);					/* Reset the size */
			if (IDirect3DExecuteBuffer_Lock(Direct3DExecBufPtr,&MyDesc)==D3D_OK) {
				Word8 *WorkPtr;
				Word8 *WorkStartPtr;

				WorkStartPtr = (Word8 *)MyDesc.lpData+(sizeof(D3DTLVERTEX)*VideoVertexMaxCount);	/* Pointer to the instructions */
				WorkPtr = WorkStartPtr;

				VideoVertexCount = 0;
				STORE_OP_RENDERSTATE_CONST(1,WorkPtr);				/* Turn off culling */
				STORE_DATA_STATE(D3DRENDERSTATE_CULLMODE,D3DCULL_NONE,WorkPtr);

				STORE_OP_RENDERSTATE_CONST(1,WorkPtr);				/* Normal alpha blending */
				STORE_DATA_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_SRCALPHA,WorkPtr);

				STORE_OP_RENDERSTATE_CONST(1,WorkPtr);
				STORE_DATA_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCALPHA,WorkPtr);

				STORE_OP_RENDERSTATE_CONST(1,WorkPtr);				/* Turn it off for now */
				STORE_DATA_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE,WorkPtr);

				STORE_OP_RENDERSTATE_CONST(1,WorkPtr);				/* Enable modulation */
				STORE_DATA_STATE(D3DRENDERSTATE_TEXTUREMAPBLEND,D3DTBLEND_MODULATE,WorkPtr);
				if (Direct3DZBufferPtr) {
					STORE_OP_RENDERSTATE_CONST(1,WorkPtr);
					STORE_DATA_STATE(D3DRENDERSTATE_ZENABLE,TRUE,WorkPtr);
					STORE_OP_RENDERSTATE_CONST(1,WorkPtr);
					STORE_DATA_STATE(D3DRENDERSTATE_ZWRITEENABLE,TRUE,WorkPtr);
					STORE_OP_RENDERSTATE_CONST(1,WorkPtr);
					STORE_DATA_STATE(D3DRENDERSTATE_ZFUNC,D3DCMP_LESSEQUAL,WorkPtr);
				}

				STORE_OP_EXIT(WorkPtr);								/* End the stream here */

				IDirect3DExecuteBuffer_Unlock(Direct3DExecBufPtr);	/* Release the memory */

				FastMemSet(&MyData,0,sizeof(MyData));			/* Prepare to execute */
				MyData.dwSize = sizeof(MyData);
				MyData.dwVertexCount = 0;
				MyData.dwInstructionOffset = WorkStartPtr - (Word8 *)MyDesc.lpData;		/* Offset to instructions */
				MyData.dwInstructionLength = WorkPtr - WorkStartPtr;		/* Length of instructions */
				IDirect3DExecuteBuffer_SetExecuteData(Direct3DExecBufPtr,&MyData);			/* Set the data */
				IDirect3DDevice_BeginScene(Direct3DDevicePtr);
				IDirect3DDevice_Execute(Direct3DDevicePtr,Direct3DExecBufPtr,Direct3DViewPortPtr,D3DEXECUTE_UNCLIPPED);
				IDirect3DDevice_EndScene(Direct3DDevicePtr);
			}
		}
	}
}

/**********************************

	This windows 98 specific routine will
	initialize the Direct3D device that is attached
	to the current direct draw surface and query what kind
	of textures can I upload. Other flags will also be
	set to allow Burgerlib to directly access any
	Direct3D features that it supports as long as
	the features are in hardware.

**********************************/

void BURGERCALL D3DInit(const struct _GUID *Input)
{
	VideoTextureTypesAllowed = 0;	/* Assume no texture formats available */
	VideoTextureRules = 0;			/* Assume no texture rules */

	/* First, do I have Direct3D? */

	if (IDirectDraw_QueryInterface(DirectDrawPtr,IID_IDirect3D,(void **)&Direct3DPtr)==DD_OK) {
		D3DCreateZBuffer();		/* Get the z buffer if possible */

		/* Now, Do I have a device override? */
		if (!Input) {					/* Use the default? */
			Input = &IID_IDirect3DHALDevice;	/* Hardware only */
		}

		/* Get the device that is attached to the screen memory */

		if (IDirectDrawSurface_QueryInterface(Win95BackBuffer,Input[0],(void **)&Direct3DDevicePtr)==DD_OK) {
			D3DGetTextureInfo();			/* Get the texture formats */
			D3DSetStandardViewport();		/* Init the viewport */
			D3DInitExecuteBuffer();			/* Init the execute buffer */
		}
	}
}

/**********************************

	Release the direct 3D context
	I check each and every one for validity
	since I have no idea what state the system was
	in, I have to be paranoid

**********************************/

void BURGERCALL Direct3DDestroy(void)
{
	if (Direct3DExecBufPtr) {
		IDirect3DExecuteBuffer_Release(Direct3DExecBufPtr);
		Direct3DExecBufPtr = 0;
	}
	if (Direct3DViewPortPtr) {
		IDirect3DViewport_Release(Direct3DViewPortPtr);
		Direct3DViewPortPtr = 0;
	}
	if (Direct3DDevicePtr) {
		IDirect3DDevice_Release(Direct3DDevicePtr);
		Direct3DDevicePtr = 0;
	}
	if (Direct3DZBufferPtr) {
		IDirectDrawSurface_Release(Direct3DZBufferPtr);
		Direct3DZBufferPtr = 0;
	}
	if (Direct3DPtr) {
		IDirect3D_Release(Direct3DPtr);
		Direct3DPtr = 0;
	}
	Direct3DExecDataBuf = 0;		/* Zark these variables to an invalid state */
	Direct3DExecInstBuf = 0;
	Direct3DExecBuffSize = 0;
}

/**********************************

	Lock down the Direct3D execute buffer.
	I need to do this before I can queue up Direct3D
	graphics instructions.

**********************************/

void BURGERCALL Direct3DLockExecuteBuffer(void)
{
    D3DEXECUTEBUFFERDESC MyDesc;

	if (!Direct3DExecDataBuf) {
		FastMemSet(&MyDesc,0,sizeof(MyDesc));
		MyDesc.dwSize = sizeof(MyDesc);

		if (IDirect3DExecuteBuffer_Lock(Direct3DExecBufPtr,&MyDesc) == D3D_OK) {
		    Direct3DExecDataBuf = (Word8 *)MyDesc.lpData;
			Direct3DExecInstBuf = Direct3DExecDataBuf + (sizeof(D3DTLVERTEX) * VideoVertexMaxCount);
			Direct3DExecInstStartPtr = Direct3DExecInstBuf;
			Direct3DExecDataPtr = Direct3DExecDataBuf;
		    Direct3DExecInstPtr = Direct3DExecInstStartPtr + sizeof(D3DINSTRUCTION) + sizeof(D3DPROCESSVERTICES);	/* Save room for the Instruction and the vertices operation */
			Direct3DExecInstEndPtr = Direct3DExecDataBuf + Direct3DExecBuffSize-4;

			VideoVertexCount = 0;						/* No vertexs stored yet */
			ScreenShadingMode = SHADINGMODE_BAD;		/* Init shading mode */
			ScreenPerspectiveMode = 0xD5AA96;			/* These numbers are BOGUS!!! */
			ScreenBlendMode = 0xD5AA96;
			ScreenCurrentTexture = 0xD5AA96;
		}
	}
}

/**********************************

	Lock down the Direct3D execute buffer.
	I need to do this before I can queue up Direct3D
	graphics instructions.

**********************************/

void BURGERCALL Direct3DUnlockExecuteBuffer(void)
{
	D3DEXECUTEDATA MyData;

	if (Direct3DExecDataBuf) {
		Word8 *WorkPtr;
		WorkPtr = Direct3DExecInstStartPtr;			/* This is the buffer where the vertex array exists */
		STORE_OP_PROCESSVERTICES_CONST(1,WorkPtr);	/* Save the opcode */
		STORE_DATA_PROCESSVERTICES(D3DPROCESSVERTICES_COPY,0,VideoVertexCount,WorkPtr);
		WorkPtr = Direct3DExecInstPtr;
		STORE_OP_EXIT(WorkPtr);

		FastMemSet(&MyData,0,sizeof(MyData));
		MyData.dwSize = sizeof(MyData);
		MyData.dwVertexCount = VideoVertexCount;
		MyData.dwInstructionOffset = Direct3DExecInstBuf - Direct3DExecDataBuf;
		MyData.dwInstructionLength = WorkPtr - Direct3DExecInstBuf;

		IDirect3DExecuteBuffer_Unlock(Direct3DExecBufPtr);		/* Release the buffer */
		IDirect3DExecuteBuffer_SetExecuteData(Direct3DExecBufPtr,&MyData);		/* set the data buffer */
		IDirect3DDevice_Execute(Direct3DDevicePtr,Direct3DExecBufPtr,Direct3DViewPortPtr,D3DEXECUTE_UNCLIPPED);
		Direct3DExecDataBuf = 0;			/* Mark as unused */
	}
}

/**********************************

	This is a bottleneck routine.
	I check if I will overflow the D3D exec buffer. If so
	then I will flush it and reset it.

**********************************/

void BURGERCALL Direct3DCheckExecBuffer(Word InstCount,Word VertexCount)
{
	if (((VertexCount+VideoVertexCount)>VideoVertexMaxCount) ||	/* Vertex wrap? */
		(&Direct3DExecInstPtr[InstCount]>Direct3DExecInstEndPtr)) {
		Direct3DUnlockExecuteBuffer();
		Direct3DLockExecuteBuffer();
	}
}

/**********************************

	Begin the 3D scene

**********************************/

void BURGERCALL Direct3DBeginScene(void)
{
	IDirect3DDevice_BeginScene(Direct3DDevicePtr);
}

/**********************************

	End the 3D scene

**********************************/

void BURGERCALL Direct3DEndScene(void)
{
	IDirect3DDevice_EndScene(Direct3DDevicePtr);
}

/**********************************

	Draw a solid rect in Direct3D mode

**********************************/

void BURGERCALL Direct3DSolidRect(int x,int y,Word Width,Word Height,Word32 Color)
{
	Word8 *WorkPtr;
	D3DTLVERTEX *VertPtr;
	Word i;

	Direct3DCheckExecBuffer(32,4);
	WorkPtr = Direct3DExecInstPtr;
	if (ScreenCurrentTexture) {
		ScreenCurrentTexture = 0;
		STORE_OP_RENDERSTATE_CONST(1,WorkPtr);
		STORE_DATA_STATE(D3DRENDERSTATE_TEXTUREHANDLE,0,WorkPtr);
	}
	VertPtr = &((D3DTLVERTEX *)Direct3DExecDataBuf)[VideoVertexCount];
	VertPtr[0].sx = (float)x;
	VertPtr[0].sy = (float)y;
	VertPtr[0].sz = 0;
	VertPtr[0].color = Color;
	VertPtr[0].rhw = 0;
	VertPtr[0].specular = 0;
	VertPtr[0].tu = 0;
	VertPtr[0].tv = 0;
	VertPtr[1].sx = (float)(x+Width);
	VertPtr[1].sy = (float)y;
	VertPtr[1].sz = 0;
	VertPtr[1].color = Color;
	VertPtr[1].rhw = 0;
	VertPtr[1].specular = 0;
	VertPtr[1].tu = 0;
	VertPtr[1].tv = 0;
	VertPtr[2].sx = (float)(x+Width);
	VertPtr[2].sy = (float)(y+Height);
	VertPtr[2].sz = 0;
	VertPtr[2].color = Color;
	VertPtr[2].rhw = 0;
	VertPtr[2].specular = 0;
	VertPtr[2].tu = 0;
	VertPtr[2].tv = 0;
	VertPtr[3].sx = (float)x;
	VertPtr[3].sy = (float)(y+Height);
	VertPtr[3].sz = 0;
	VertPtr[3].color = Color;
	VertPtr[3].rhw = 0;
	VertPtr[3].specular = 0;
	VertPtr[3].tu = 0;
	VertPtr[3].tv = 0;
	STORE_OP_TRIANGLE(2,WorkPtr);
	i = VideoVertexCount;
	STORE_DATA_TRIANGLE(i+0,i+1,i+2,WorkPtr);
	STORE_DATA_TRIANGLE(i+0,i+2,i+3,WorkPtr);
	VideoVertexCount=i+4;
	Direct3DExecInstPtr = WorkPtr;
}

/**********************************

	OpenGL code

**********************************/

HGLRC OpenGLContext;			/* Current OpenGL context */
HDC OpenGLDeviceContext;		/* Window to attach the GL context to */

/**********************************

	Create my GL context for the current display mode

**********************************/

#if 0
static PIXELFORMATDESCRIPTOR xxxpfd = {
	sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
	1,								// version number
	PFD_DRAW_TO_WINDOW				// support window
	| PFD_SUPPORT_OPENGL			// support OpenGL
	| PFD_DOUBLEBUFFER,				// double buffered
	PFD_TYPE_RGBA,					// RGBA type
	24,								// 16-bit color depth
	0, 0, 0, 0, 0, 0,				// color bits ignored
	0,								// no alpha buffer
	0,								// shift bit ignored
	0,								// no accumulation buffer
	0, 0, 0, 0,						// accum bits ignored
	32,								// 16-bit z-buffer
	0,								// no stencil buffer
	0,								// no auxiliary buffer
	PFD_MAIN_PLANE,					// main layer
	0,								// reserved
	0, 0, 0							// layer masks ignored
};
#endif

static HGLRC WinSetupGL(Word /* Depth */)
{
	PIXELFORMATDESCRIPTOR pfd;
    int  pixelFormat;
    HGLRC rv;

	FastMemSet(&pfd,0,sizeof(pfd));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW	// support window
		| PFD_SUPPORT_OPENGL		// support OpenGL
		| PFD_DOUBLEBUFFER ,		// double buffered
	pfd.iPixelType = PFD_TYPE_RGBA;		// True color
	pfd.cColorBits = 32;				// True color
	pfd.cDepthBits = 32;				// Deep buffer
	pfd.cStencilBits = 8;
	pfd.cAlphaBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	pixelFormat = ChoosePixelFormat(OpenGLDeviceContext, &pfd);		/* Get the pixel format */
	if ( pixelFormat ) {
        if ( SetPixelFormat(OpenGLDeviceContext, pixelFormat, &pfd) ) {	/* Select this context */
            rv = wglCreateContext( OpenGLDeviceContext );				/* Create the context */
            if (rv) {
                if (wglMakeCurrent( OpenGLDeviceContext, rv ) ) {		/* Make GL use this context */
                    return rv;
				}
				wglDeleteContext(rv );			/* Oh crap, clean up */
            }
		}
	}
	
	pfd.cAlphaBits = 0;
	pfd.cColorBits = 24;

	pixelFormat = ChoosePixelFormat(OpenGLDeviceContext, &pfd);		/* Get the pixel format */
	if ( pixelFormat ) {
        if (SetPixelFormat(OpenGLDeviceContext, pixelFormat, &pfd)) {	/* Select this context */
            rv = wglCreateContext( OpenGLDeviceContext );				/* Create the context */
            if (rv) {
                if (wglMakeCurrent( OpenGLDeviceContext, rv ) ) {		/* Make GL use this context */
                    return rv;
				}
				wglDeleteContext(rv );			/* Oh crap, clean up */
            }
		}
	}
    return 0;			/* Error */
}

/**********************************

    Sets the pertinent burger vars for a given screen resolution
    
**********************************/  
static void BURGERCALL SetupBurgerVarsForVideoMode( Word Width, Word Height, Word Depth, Word FullScreen )
{
	ScreenWidth = Width;	/* Save the screen size in Burgerlib globals */
	ScreenHeight = Height;
	ScreenClipTop = 0;
	ScreenClipLeft = 0;
	ScreenClipRight = Width;
	ScreenClipBottom = Height;
	VideoTrueScreenWidth = Width;
	VideoTrueScreenHeight = Height;
	VideoFullScreen = (FullScreen) ? SETDISPLAYFULLSCREEN : SETDISPLAYWINDOW;	/* Save in global (Must be done here!) */
	VideoColorDepth = Depth;
	VideoTrueScreenDepth = Depth;
	if (Depth==16) {
		VideoRedShift = 11;
		VideoRedMask = 0x1F<<11;
		VideoGreenBits = 6;
		VideoGreenMask = 0x3f<<5;
	} else {
		VideoRedShift = 10;
		VideoRedMask = 0x1F<<10;
		VideoGreenBits = 5;
		VideoGreenMask = 0x1F<<5;
	}
	VideoBlueShift = 0;
	VideoBlueMask = 0x1f;
	VideoBlueBits = 5;
	VideoGreenShift = 5;
	VideoRedBits = 5;
}

/**********************************

	Set the Win95 box to a full screen mode 
	but DO NOT USE DirectX!!!
	
	OpenGL hates that!

**********************************/

Word BURGERCALL WinSetDisplayToSize(Word Width,Word Height,Word Depth)
{
	// Go to full screen
	DEVMODE dmode, bestmode;
//	LONG changeResult;
	int modenum;
//	Word32 highest_freq = 0;
		
	modenum = 0;

	bestmode.dmDisplayFrequency = 0;
	
	for (;;)
	{
		FastMemSet(&dmode,0,sizeof(dmode));		/* Clear it out */
		dmode.dmSize = sizeof(dmode);			/* Let windows know the size I want */
		
		if( EnumDisplaySettings(NULL, modenum, &dmode ) == FALSE ) break;
		
		if ( dmode.dmBitsPerPel == Depth &&		/* Match the size? */
			dmode.dmPelsWidth == Width &&       /* Look for the highest frequency */
			dmode.dmPelsHeight == Height &&
			(dmode.dmDisplayFrequency > bestmode.dmDisplayFrequency) ) 
		{
			dmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
			if( ChangeDisplaySettings( &dmode, CDS_TEST ) == DISP_CHANGE_SUCCESSFUL )
			{
				/* this is the best mode so far */
				bestmode = dmode;
			}
		}
		modenum++;
	}

	if ( bestmode.dmDisplayFrequency <= 0 ) {	/* Didn't find it? */
		ChangeDisplaySettings (NULL, 0);		/* Switch video mode to default */
		return TRUE;
	}

	/* We found a valid mode, so try to REALLY set it. */
	if( ChangeDisplaySettings( &bestmode, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
	{
		ChangeDisplaySettings( NULL, 0 );
		return TRUE; /* BAD */
	}
		
	ChangeDisplaySettings (&dmode, CDS_FULLSCREEN);
	SetupBurgerVarsForVideoMode( Width, Height, Depth, TRUE );
		
	return FALSE;
}

/**********************************

	Make sure the OpenGL context is burgerlib's

**********************************/

void BURGERCALL OpenGLSetCurrent(void)
{
	if (OpenGLDeviceContext) {		/* Failsafe */
		wglMakeCurrent(OpenGLDeviceContext,OpenGLContext);
	}
}

/**********************************

	Page flip OpenGL style

**********************************/

void BURGERCALL OpenGLSwapBuffers(void)
{
	if (OpenGLDeviceContext) {		/* Failsafe */
		SwapBuffers( OpenGLDeviceContext );
	}
}

/**********************************

	Release the OpenGL display mode

**********************************/

void BURGERCALL OpenGLRelease(void)
{
	if (OpenGLContext) {				/* Did I start one up? */
		if (OpenGLDeviceContext) {		/* Do I have a device? */
			ReleaseDC((HWND)Win95MainWindow,OpenGLDeviceContext );	/* Release the device */
			OpenGLDeviceContext = 0;
		}
		wglMakeCurrent(0,0);			/* Make sure the system has a default context */
		wglDeleteContext(OpenGLContext);	/* Bye bye */
		OpenGLContext = 0;
		ReleaseVideo();					/* Any last cleanup */
		ChangeDisplaySettings(NULL, 0);	/* Restore the display mode */
	}
}

/**********************************

	Set a fullscreen or windows OpenGL context

**********************************/

Word BURGERCALL OpenGLSetDisplayToSize(Word Width,Word Height,Word Depth,Word Flags)
{
	Word WorkDepth;
	Word WorkFlags;
	Word GammaOn;
	Word NewVidWidth, NewVidHeight;
	
	WorkDepth = Depth;
	WorkFlags = Flags;
	GammaOn = GammaInit;		/* Save if Gamma was already saved */
	ReleaseVideo();				/* Release any previous video mode */
	
	NewVidWidth = Width;
	NewVidHeight = Height;
	
	//
	// For some reason, EVEN if the display is windowed, we must make a call
	// to set the display resolution, BEFORE initializing OpenGL. Otherwise
	// the windowed device will be Microsoft GDI renderer.
	//
	// In the event that we're doing windowed mode, we grab the current video
	// settings, and do a temporary ChangeDisplaySettings() via WinSetDisplayToSize().
	// Then we override some burger vars to ensure the drawing fits withing the 
	// current Width/Height of the window which is passed into this routine by
	// the caller.
	//
	for (;;) {
		
		if( (WorkFlags&1)==0 ) // Find Current Video Mode
		{
			DEVMODEA dmode;
			dmode.dmSize = sizeof(dmode);
			EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &dmode );
			Width = dmode.dmPelsWidth;
			Height = dmode.dmPelsHeight;
		}
		
		if (!WinSetDisplayToSize(Width,Height,WorkDepth)) {	
		
			//
			// If we are not in fullscreen, override burger lib vars.
			//
			if( (WorkFlags&1) == 0 )
			{
				SetupBurgerVarsForVideoMode( NewVidWidth, NewVidHeight, WorkDepth, FALSE );
			}
			
			OpenGLDeviceContext = GetDC((HWND)Win95MainWindow);
			if (OpenGLDeviceContext) {
				OpenGLContext = WinSetupGL(WorkDepth);
				if (OpenGLContext) {
					if (GammaOn) {
						VideoOSGammaInit();		/* Restore Gamma control */
					}
					return FALSE;
				}
				ReleaseDC((HWND)Win95MainWindow,OpenGLDeviceContext);
				OpenGLDeviceContext = 0;
			}
		}
		ReleaseVideo();
		ChangeDisplaySettings(0,0);
		if (WorkDepth==Depth && Depth>16) {
			WorkDepth = 16;
			continue;
		}
		WorkDepth = Depth;
		if (WorkFlags&1) {
			WorkFlags &= (~1);
			continue;
		}
		break;
	}
	return TRUE;
}

/**********************************

	Enable or disable TrueForm technology

**********************************/

void BURGERCALL OpenGLATISetTruform(Word /* Setting */)
{
}

/**********************************

	Enable or disable Full Screen Anti-Aliasing technology

**********************************/

void BURGERCALL OpenGLATISetFSAA(Word /* Setting */)
{
}

/**********************************

	Init the gamma manager

**********************************/

void BURGERCALL VideoOSGammaInit(void)
{

	/* I have two paths depending on DIrectX's state */
	
	if (!GammaInit) {
	
		/* Use DirectX Gamma if DirectX allow it */
		
		if (VideoFullScreen && !OpenGLContext && Win95FrontBuffer) {
			/* Query for the DirectDraw Gamma control attached to the front buffer */
		
			if (IDirectDraw_QueryInterface(Win95FrontBuffer,IID_IDirectDrawGammaControl,(void **)&lpDDGammaControl)==S_OK) {

			/* DirectX gamma fading is active */
				IDirectDrawGammaControl_GetGammaRamp(lpDDGammaControl,0,&DDGammaOld);
				FastMemCpy(&DDGammaCurrent,&DDGammaOld,sizeof(DDGammaOld));
				GammaInit = TRUE;
				return;
			}
		}
		
		/* Ok, I'm not using DirectDraw, let use the DIB code */
		
		if (Win95MainWindow) {		/* Must have a window */
			HDC WinDC;

			WinDC = GetDC((HWND)Win95MainWindow);		/* Where will I draw it? */
			if (WinDC) {
				GetDeviceGammaRamp(WinDC,&DDGammaOld);
				FastMemCpy(&DDGammaCurrent,&DDGammaOld,sizeof(DDGammaOld));
				GammaDC = TRUE;		/* Using Device driver */
			}
			GammaInit = TRUE;
		}
	}
}

/**********************************

	Shut down the gamma manager

**********************************/

void BURGERCALL VideoOSGammaDestroy(void)
{
	GammaTry = FALSE;		/* Don't restart */
	if (GammaInit) {		/* Was it started? */
	
		/* Shut down DirectX */
		
		if (lpDDGammaControl) {
			IDirectDrawGammaControl_SetGammaRamp(lpDDGammaControl,0,&DDGammaOld);
			IDirectDrawGammaControl_Release(lpDDGammaControl);
			lpDDGammaControl = 0;
		}
		
		/* Release the windows driver */
		
		if (GammaDC) {
			HDC WinDC;

			WinDC = GetDC((HWND)Win95MainWindow);		/* Where will I draw it? */
			if (WinDC) {
				SetDeviceGammaRamp(WinDC,&DDGammaOld);
			}
			GammaDC = FALSE;
		}
		GammaInit = FALSE;
	}
}

/**********************************

	Darken or brighten the gamma using 1.0 as the base intensity

**********************************/

void BURGERCALL VideoOSGammaAdjust(Fixed32 Intensity)
{
	Word i;
	if (GammaInit) {
		if (Intensity<0) {
			Intensity = 0;
		}
		if (Intensity>=0x80000) {
			Intensity = 0x80000;
		}
		Intensity>>=4;
		i = 0;
		do {
			Word Val;
			Val = (DDGammaOld.red[i]*(Word)Intensity)>>12;
			if (Val>=65536) {
				Val = 65535;
			}
			DDGammaCurrent.red[i] = static_cast<Word16>(Val);
			Val = (DDGammaOld.green[i]*(Word)Intensity)>>12;
			if (Val>=65536) {
				Val = 65535;
			}
			DDGammaCurrent.green[i] = static_cast<Word16>(Val);
			Val = (DDGammaOld.blue[i]*(Word)Intensity)>>12;
			if (Val>=65536) {
				Val = 65535;
			}
			DDGammaCurrent.blue[i] = static_cast<Word16>(Val);
		} while (++i<256);
		if (lpDDGammaControl) {
			IDirectDrawGammaControl_SetGammaRamp(lpDDGammaControl,0, &DDGammaCurrent);
		}
		if (GammaDC) {
			HDC WinDC;

			WinDC = GetDC((HWND)Win95MainWindow);		/* Where will I draw it? */
			if (WinDC) {
				SetDeviceGammaRamp(WinDC,&DDGammaCurrent);
			}
		}
	}
}

/**********************************

	Set the gamma to a specific color ramp

**********************************/

void BURGERCALL VideoOSGammaSet(const GammaTable_t *TablePtr)
{
	Word i;
	if (GammaInit && TablePtr) {
		i = 0;
		do {
			int Val;
			Val = (int)(TablePtr->Red[i]*65535.0f);
			if (Val<0) {
				Val = 0;
			}
			if (Val>=65536) {
				Val = 65535;
			}
			DDGammaCurrent.red[i] = (WORD)Val;
			Val = (int)(TablePtr->Green[i]*65535.0f);
			if (Val<0) {
				Val = 0;
			}
			if (Val>=65536) {
				Val = 65535;
			}
			DDGammaCurrent.green[i] = (WORD)Val;
			Val = (int)(TablePtr->Blue[i]*65535.0f);
			if (Val<0) {
				Val = 0;
			}
			if (Val>=65536) {
				Val = 65535;
			}
			DDGammaCurrent.blue[i] = (WORD)Val;
		} while (++i<256);
		if (lpDDGammaControl) {
			IDirectDrawGammaControl_SetGammaRamp(lpDDGammaControl,0, &DDGammaCurrent);
		}
		if (GammaDC) {
			HDC WinDC;

			WinDC = GetDC((HWND)Win95MainWindow);		/* Where will I draw it? */
			if (WinDC) {
				SetDeviceGammaRamp(WinDC,&DDGammaCurrent);
			}
		}
	}
}

/**********************************

	Get the current gamma table

**********************************/

void BURGERCALL VideoOSGammaGet(GammaTable_t *TablePtr)
{
	Word i;
	if (TablePtr) {
		if (GammaInit) {
			i = 0;
			do {
				TablePtr->Red[i] = (DDGammaCurrent.red[i]*(1.0f/65535.0f));
				TablePtr->Green[i] = (DDGammaCurrent.green[i]*(1.0f/65535.0f));
				TablePtr->Blue[i] = (DDGammaCurrent.blue[i]*(1.0f/65535.0f));
			} while (++i<256);
		} else {
			FastMemSet(TablePtr,0,sizeof(GammaTable_t));
		}
	}
}

#endif

