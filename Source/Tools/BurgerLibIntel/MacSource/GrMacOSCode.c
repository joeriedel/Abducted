/**********************************

	Mac Version of the burgerlib graphics manager

**********************************/

#include "GrGraphics.h"

/**********************************

	Theory of operation...

	All offscreen buffers are GWorlds. I use copybits to update the
	screen quickly since they use 2D hardware assist and up the frame rates
	by quite a bit.

	For PPC versions, I use DrawSprocket. For 68k versions I use the mac system.
	Note: The 68k versions days are numbered.

**********************************/

#if defined(__MAC__)
#include "McMac.h"
#include "MmMemory.h"
#include "ClStdLib.h"
#include "ShStream.h"
#include "InInput.h"
#include "OcOSCursor.h"
#include "TkTick.h"
#include "PlPalette.h"
#include <stdio.h>
#include <drawsprocket.h>
#include <Devices.h>
#include <agl.h>

/* Set to TRUE if you want to call LockPixels on GWorlds */

#define LOCKTHEPIXELS FALSE

/* These are defines for the ATI Trueform technology */

#define ATI_TRUFORM_ENABLE	      	500  /* Enable/Disable TruForm triangle subdivision */
#define ATI_TRUFORM_LEVEL	       	501  /* set TruForm tesselation level */
#define ATI_TRUFORM_POINT_MODE	   	502  /* set TruForm point interpolation mode */
#define ATI_TRUFORM_NORMAL_MODE	 	503  /* set TruForm normal interpolation mode */

#define ATI_FSAA_SAMPLES	     	510  /* set Full Scene Anti-Aliasing (FSAA) samples: 1x, 2x, 4x */

#ifndef GL_PN_TRIANGLES_ATIX
#define GL_PN_TRIANGLES_ATIX 0x6090
#endif
#ifndef GL_PN_TRIANGLES_TESSELATION_LEVEL_ATIX
#define GL_PN_TRIANGLES_TESSELATION_LEVEL_ATIX 0x6094
#endif

#if TARGET_API_MAC_CARBON
#include <CGDirectDisplay.h>
#ifdef __cplusplus
extern "C" {
#endif
EXTERN_API_C( OSStatus ) DSpCanUserSelectContext(DSpContextAttributesPtr inDesiredAttributes,Bool *outUserCanSelectContext);
EXTERN_API_C( OSStatus ) DSpUserSelectContext(DSpContextAttributesPtr   inDesiredAttributes,DisplayIDType inDialogDisplayLocation,DSpEventUPP inEventProc,DSpContextReference *outContext);
EXTERN_API( OSErr ) PBStatusSync(ParmBlkPtr paramBlock);
EXTERN_API( OSErr ) Control(short refNum,short csCode,const void *csParamPtr);
#ifdef __cplusplus
}
#endif
#endif

/**********************************

	These are in assembly on some platforms

**********************************/

#if defined(__POWERPC__)			/* Only with drawsprocket */
typedef struct ContextCache_t {
	DSpContextReference Context;	/* Cached context */
	DSpContextAttributes Attributes;	/* Requested attributes */
	Word Width;						/* Burgerlib width */
	Word Height;					/* Height */
	Word Depth;						/* Depth */
	Word Hertz;						/* Freq */
} ContextCache_t;
#endif

typedef struct GrMacLocals_t {			/* Locals in a single struct */
#if defined(__POWERPC__)				/* DrawSprocket crap */
	void (*KillGLContextProc)(void);	/* Link in OpenGL to shut down the context */
	AGLContext BurgerGLContext;		/* Active OpenGL Context */
	ContextCache_t **ContextCache;	/* Video context cache */
	DSpContextReference MainContext;	/* Master context */

	RGBColor rgbBlack;				/* It's black.. (6 bytes) */
	Word16 ContextCacheCount;		/* Number of valid entries */
#endif

#if TARGET_API_MAC_CARBON			/* Only available with Carbon */
	EventHandlerUPP WinProc;		/* Carbon window proc */
#endif
#if LOCKTHEPIXELS
	PixMapHandle LockedPixMap;		/* PixMap to unlock */
#endif
	GammaTable_t GammaOriginalTable;	/* Original Gamma */
	GammaTable_t GammaCurrentTable;		/* Current gamma */

	Word8 MyWinTitle[256];			/* Window title */
	Word8 PixelDoubleActive;			/* TRUE if pixel doubling is active */
	Word8 OpenGLFlip;				/* TRUE to invoke the OpenGL hack */
	Word8 Hack320x200;				/* If TRUE, then pixel double 320x200 to 640x480 */
	Word8 GammaOk;					/* True if the Gamma is init'd */
} GrMacLocals_t;

static GrMacLocals_t Locals;				/* My local data */

/**********************************

	Carbon requires an update callback proc
	Used in OSX only

**********************************/

#if TARGET_API_MAC_CARBON

#define WIN_EVENT_TYPES_SIZE 2 

/* List of events I handle */

static EventTypeSpec WinEventTypes[WIN_EVENT_TYPES_SIZE] = {
//	{ kEventClassWindow, kEventWindowClose},	//	0
	{ kEventClassWindow, kEventWindowUpdate},
	{ kEventClassWindow, kEventWindowBoundsChanged}
};

static pascal OSStatus WindowProc(EventHandlerCallRef inHandlerCallRef,
	EventRef inEvent,void *inUserData)
{
	switch (GetEventKind(inEvent)) {
//	case kEventWindowClose:
//		break;
	
	case kEventWindowBoundsChanged:
		MouseSetRange(ScreenWidth,ScreenHeight);
//		aglUpdateContext(agl_context);
//		update_window_bounds_info();
		break;
	
	case kEventWindowUpdate:
		{
			GrafPtr oldPort;
			WindowRef theWindow;
			
			GetEventParameter(inEvent,kEventParamDirectObject,typeWindowRef, NULL,
				sizeof(theWindow), NULL,&theWindow);
			GetPort(&oldPort);		/* Save the current port */
			SetPortWindowPort(theWindow);		/* Set my main port */
			BeginUpdate(theWindow);				/* Update */
			EndUpdate(theWindow);				/* End the update event */
			SetPort(oldPort);					/* Restore the port */
		}
		break;
	}
	return noErr;
}

/**********************************

	Create the window for OSX

**********************************/

static INLINECALL Word CreateOSXWindow(const Rect *BoundsRect)
{
	if (CreateNewWindow( kDocumentWindowClass, kWindowCollapseBoxAttribute | kWindowStandardHandlerAttribute,BoundsRect, &VideoWindow )) {
		return TRUE;		/* Oh oh... */
	}
	SetWindowContentColor(VideoWindow, &Locals.rgbBlack);		/* Black background */
	SetWTitle(VideoWindow,Locals.MyWinTitle);
	if (!Locals.WinProc) {
		Locals.WinProc = NewEventHandlerUPP(WindowProc);
	}
	InstallWindowEventHandler(VideoWindow,Locals.WinProc,WIN_EVENT_TYPES_SIZE,
		&WinEventTypes[0],VideoWindow,0);
	
	/* Do the cute "Swish" */
	TransitionWindow(VideoWindow, kWindowZoomTransitionEffect, kWindowShowTransitionAction, NULL );
	
	return FALSE;
}

#endif

/**********************************

	Release the offscreen GWorld

**********************************/

static INLINECALL void ReleaseOldGWorld(void)
{
	if (VideoGWorld) {		/* Do I have an offscreen GWorld? */
		DisposeGWorld(VideoGWorld);	/* Kill it (GL doen't need it) */
		VideoGWorld = 0;
	}
}

/**********************************

	Release the game's window

**********************************/

static INLINECALL void ReleaseOldWindow(void)
{
	if (VideoWindow) {		/* Do I have a window? */
		DisposeWindow(VideoWindow);	/* Kill it */
		VideoWindow = 0;
	}
}

/**********************************

	Release OpenGL (Only available in PowerPC)

**********************************/

#if defined(__POWERPC__)
static INLINECALL void ReleaseGL(void)
{
	if (Locals.KillGLContextProc) {		/* Dispose of GL (Only if GL is linked in) */
		Locals.KillGLContextProc();
	}
}
#else
#define ReleaseGL()			/* Not in 68K */
#endif

/**********************************

	Start DrawSprocket

**********************************/

#if defined(__POWERPC__)
static INLINECALL Word StartDrawSprocket(void)
{
	if (!MacDrawSprocketActive) {	/* Did I test for DrawSprocket? */
		if (DSpStartup) {			/* Is draw sprocket present? */
			if (!DSpStartup()) {	/* Init draw sprocket */
				MacDrawSprocketActive = TRUE;	/* It's ok! */
				DSpSetBlankingColor(&Locals.rgbBlack);
			}
		}
	}
	return MacDrawSprocketActive;		/* Return TRUE if active */
}
#else
#define StartDrawSprocket() 0
#endif

/**********************************

	Stop DrawSprocket

**********************************/

#if defined(__POWERPC__)
static INLINECALL void StopDrawSprocket(void)
{
	if (MacDrawSprocketActive) {	/* Kill off draw sprocket? */
		MacDrawSprocketActive = FALSE;
		DSpShutdown();		/* Bye bye */
	}
}
#else
#define StopDrawSprocket()
#endif

/**********************************

	Return the front buffer

**********************************/

static INLINECALL CGrafPtr GetTheFrontBuffer(void)
{
	CGrafPtr MyPort;
#if defined(__POWERPC__)
	if (MacContext) {					/* Am I using DrawSprocket? */
		CGrafPtr MyPortMem;
		DSpContext_GetFrontBuffer(MacContext,&MyPortMem);
		MyPort = MyPortMem;
	} else
#endif
	{
		MyPort = GetWindowPort(VideoWindow);	/* Draw to the live screen */
	}
	return MyPort;
}

/**********************************

	Return the back buffer

**********************************/

static INLINECALL CGrafPtr GetTheBackBuffer(void)
{
	CGrafPtr MyPort;
#if defined(__POWERPC__)
	if (MacContext) {					/* Am I using DrawSprocket? */
		CGrafPtr MyPortMem;
		DSpContext_GetBackBuffer(MacContext,kDSpBufferKind_Normal,&MyPortMem);
		MyPort = MyPortMem;
	} else
#endif
	{
		MyPort = GetWindowPort(VideoWindow);	/* Draw to the live screen */
	}
	return MyPort;
}

/**********************************

	Determine which monitor the window should appear on
	On exit, VideoDevice points to the chosen device.
	
**********************************/

static INLINECALL void FindWindowDevice(int Width,int Height,Word Depth)
{
	GDHandle WorkDevice;
	
	if (!VideoDevice) {		/* Has a video device been chosen? */
		VideoDevice = GetMainDevice();		/* Failsafe */
	}

	WorkDevice = GetDeviceList();
	if (WorkDevice) {
		do {
			int NewWidth;
			int OldWidth;
			
			if (TestDeviceAttribute(WorkDevice,screenDevice) && 
				TestDeviceAttribute(WorkDevice,screenActive) &&
				(WorkDevice != VideoDevice)) {
				GDPtr VidPtr;
				GDPtr NewVidPtr;
				VidPtr = VideoDevice[0];
				NewVidPtr = WorkDevice[0];
				NewWidth = NewVidPtr->gdRect.right - NewVidPtr->gdRect.left;
				OldWidth = VidPtr->gdRect.right - VidPtr->gdRect.left;
				
				if ((OldWidth <= Width) &&		/* Is the previous port too small */
					(NewWidth > OldWidth)) {	/* Is this wide enough? */

					VideoDevice = WorkDevice;		/* Accept it! */
					
				} else if (NewWidth > Width) {
				
					NewWidth = NewVidPtr->gdRect.bottom - NewVidPtr->gdRect.top;
					OldWidth = VidPtr->gdRect.bottom - VidPtr->gdRect.top;

					if (OldWidth <= Height &&
						NewWidth > OldWidth) {
						VideoDevice = WorkDevice;
					} else if (NewWidth > Height) {
					
						if (NewVidPtr->gdPMap[0]->pixelSize == Depth && VidPtr->gdPMap[0]->pixelSize != Depth) {
							VideoDevice = WorkDevice;
						} else {
	
//							if (NewVidPtr->gdPMap[0]->pixelSize == Depth) {
#if _DEBUG
								if (WorkDevice != GetMainDevice() && VideoDevice == GetMainDevice()) {
									VideoDevice = WorkDevice;
								}
#endif	
//							}
						}
					}
				}
			}
			WorkDevice = GetNextDevice(WorkDevice);
		} while (WorkDevice);
	}
}

/**********************************

	Make the bounds rect for the new window on the chosen display
	
**********************************/

static INLINECALL void SetNewWinBoundsRect(Rect *Output,Word Width,Word Height,Word Flags)
{
	Word WinWidth,WinHeight;
	int oldcenterx,oldcentery;	/* Center of the window */
	int left,right,top,bottom;
#if TARGET_API_MAC_CARBON
	Rect	availableBounds;
#else
	GDPtr VPtr;
#endif
	
	WinWidth = Width;
	WinHeight = Height;
	if (Flags&SETDISPLAYDOUBLEOK) {
		WinHeight *= 2;
		WinWidth *= 2;
	}

#if !TARGET_API_MAC_CARBON
	VPtr = VideoDevice[0];		/* Get the selected device */

	left = VPtr->gdRect.left + 8 + (WinWidth>>1);
	right = VPtr->gdRect.right - (8 + (WinWidth>>1));
	bottom = VPtr->gdRect.bottom - (8 + (WinHeight>>1));
	top = VPtr->gdRect.top + 24 + (WinHeight>>1);
	
	if (VideoDevice == GetMainDevice()) {
		top += GetMBarHeight();
	}
#else
	GetAvailableWindowPositioningBounds(VideoDevice, &availableBounds);
	
	left = availableBounds.left + 8 + (WinWidth >> 1);
	right = availableBounds.right - (8 + (WinWidth >> 1));
	bottom = availableBounds.bottom - (8 + (WinHeight >> 1));
	top = availableBounds.top + 24 + (WinHeight >> 1);
#endif

	oldcenterx = (left + right)/2;
	oldcentery = (top + bottom)/2;

	if (oldcenterx > right) {
		oldcenterx = right;
	}
	if (oldcenterx < left) {
		oldcenterx = left;
	}
	if (oldcentery > bottom) {
		oldcentery = bottom;
	}
	if (oldcentery < top) {
		oldcentery = top;
	}
	
	top = oldcentery - (WinHeight>>1);
	left = oldcenterx - (WinWidth>>1);
	Output->top = top;
	Output->left = left;
	Output->bottom = top + WinHeight;
	Output->right = left + WinWidth;
}

#if defined(__POWERPC__)

/**********************************

	Dispose of the context cache

**********************************/

static void BURGERCALL ReleaseOldContext(void)
{
	if (MacContext) {		/* Shut down draw sprocket */
		DSpContext_SetState(MacContext,kDSpContextState_Inactive);	/* Inactive context */
		MacContext = 0;		/* It's gone! */
	}
	DeallocAHandle((void **)Locals.ContextCache);	/* Dispose of the list */
	Locals.ContextCacheCount = 0;					/* Kill the list */
	Locals.ContextCache = 0;						/* Kill the memory */

	if (Locals.MainContext) {						/* Is there a master context */
		DSpContext_Release(Locals.MainContext);		/* Release the master context */
		Locals.MainContext = 0;
	}
	MacContext = 0;
	
	/* Please not, normally I would leave DrawSprocket active, */
	/* but since the moron who wrote it for Apple has no clue */
	/* about cleaning up his code, I need to force a shutdown */
	/* and then a DrawSprocket restart to ensure that DrawSprocket */
	/* doesn't go psycho on me. */
	
	/* Bug fixed : Error kDSpContextNotFoundErr was being returned from */
	/* DSpFindBestContext() when no contexts were enabled */
	
	StopDrawSprocket();		/* Kill off draw sprocket */
}

/**********************************

	Given a context, I get the video display it's attached to.
	Then I will grab every context and make an array to scan for
	in the future. This way, I can perform a video switch

**********************************/

static INLINECALL int CreateContextCache(DSpContextReference Found,DSpContextAttributes *Desired)
{
	int Result;
	Word i;
	Word Count;							/* Number of contexts to find */
	DisplayIDType MyID;					/* Current display ID */
	ContextCache_t *WorkPtr;			/* Pointer to context array */
	DSpContextReference MyRef;			/* Register based reference */
	DSpContextReference MyRefMem;		/* Memory based reference for query procs */

	DSpContext_GetDisplayID(Found,&MyID);		/* Get my display ID */
	
	/* Count the number of contexts present for the currently connected monitor */
	
	Count = 0;									/* Init the count */	
	if (!DSpGetFirstContext(MyID,&MyRefMem)) {			/* Get the first context */
		MyRef = MyRefMem;
		if (MyRef) {								/* Valid reference */
			do {
				++Count;
				if (DSpGetNextContext(MyRef,&MyRefMem)) {	/* Next context */
					break;			/* Error?? */
				}
				MyRef = MyRefMem;
			} while (MyRef);		/* All done? */
		}
	}
	Locals.ContextCacheCount = Count;			/* Save the new count */
	
	/* Now, I have the count, allocate a buffer and enumerate the contexts */
	/* Note : I will reduce the context count by removing all */
	/* video contexts that are less than 8 bits per pixel */
	/* Burgerlib does not support <8 bits per pixel */
	
	if (Count) {
		Locals.ContextCache = (ContextCache_t**)AllocAHandleClear(Count*sizeof(ContextCache_t));
		if (Locals.ContextCache) {
			i = 0;										/* Init the count */	
			if (!DSpGetFirstContext(MyID,&MyRefMem)) {		/* Get the first context */
				MyRef = MyRefMem;
				if (MyRef) {								/* Valid reference */
					WorkPtr = (ContextCache_t *)LockAHandle((void **)Locals.ContextCache);
					do {
						if (!DSpContext_GetAttributes(MyRef,&WorkPtr->Attributes)) {
							if (WorkPtr->Attributes.displayBestDepth>=8) {				/* Must be 8,16 or 32 bits */
								WorkPtr->Depth = WorkPtr->Attributes.displayBestDepth;	/* Depth */
								WorkPtr->Width = WorkPtr->Attributes.displayWidth;		/* Save the pixel width */
								WorkPtr->Height = WorkPtr->Attributes.displayHeight;
								WorkPtr->Hertz = WorkPtr->Attributes.frequency>>16;
								WorkPtr->Context = MyRef;
								WorkPtr->Attributes.pageCount = 1;
								WorkPtr->Attributes.contextOptions = kDSpContextOption_DontSyncVBL;	// no page flipping and no VBL sync needed
								++WorkPtr;
								if (++i>=Count) {		/* Failsafe */
									break;
								}
							}
						}
						if (DSpGetNextContext(MyRef,&MyRefMem)) {	/* Next context */
							break;			/* Error?? */
						}
						MyRef = MyRefMem;
					} while (MyRef);		/* All done? */
				}
				UnlockAHandle((void **)Locals.ContextCache);		/* Save the new count */
			}
			Locals.ContextCacheCount = i;						/* Save the new count */
		}
	}
	
	/* Now, let's reserve my context */
	
	Desired->contextOptions |= kDSpContextOption_DontSyncVBL;
	Result = DSpContext_Reserve(Found,Desired);
	if (!Result) {
		Locals.MainContext = Found;					/* Master context */
		i = Locals.ContextCacheCount;
		if (i && DSpContext_Queue) {				/* Allow ONLY it 1.7.3 or later! */
			WorkPtr = (ContextCache_t *)LockAHandle((void **)Locals.ContextCache);
			do {
				if (WorkPtr->Context!=Found) {		/* Don't reserve the context again! */
					DSpContext_Queue(Found,WorkPtr->Context,&WorkPtr->Attributes);
				}
				++WorkPtr;
			} while (--i);
			UnlockAHandle((void **)Locals.ContextCache);
		}
	}
	return Result;		/* Success? */
}
#endif

/**********************************

	Create a Mac offscreen buffer
	and init the burgerlib globals

**********************************/

Word BURGERCALL MacMakeOffscreenGWorld(Word Width,Word Height,Word Depth,Word Flags)
{
	Rect TempRect;

	/* Set the burgerlib variables */

	MacUseBackBuffer = TRUE;		/* I have an offscreen buffer */
	ScreenWidth = Width;
	ScreenHeight = Height;			/* Get the size of the video screen */
	ScreenFlags = Flags;
	VideoFullScreen = Flags&SETDISPLAYFULLSCREEN;
	ScreenClipTop = 0;
	ScreenClipLeft = 0;
	ScreenClipRight = Width;
	ScreenClipBottom = Height;
	if (Depth==16) {		/* Is this a 16 bit context? */
		--Depth;			/* It's a Burgerlib 15 bit context */
	}
	VideoColorDepth = Depth;		/* 32, 15 or 8 */
	VideoTrueScreenDepth = Depth;	/* 32, 15 or 8 */

	TempRect.top = 0;
	TempRect.left = 0;
	TempRect.right = Width;		/* Set the bounds */
	TempRect.bottom = Height;
	if (Depth==15) {			/* Burgerlib wants 8,15 or 32 */
		Depth = 16;				/* Mac wants 16 */
	}
	
	/* Create my GWorld */
	/* Note OpenGL and MacOS for a window don't need it */
	if (!(Flags&SETDISPLAYOPENGL)) {
		Word i,Line;
		Word8 *DestPtr;
		if (NewGWorld(&VideoGWorld,Depth,&TempRect,0,0,0)) {
			VideoGWorld = 0;		/* Make sure I am shut down */
			return TRUE;			/* Error! */
		}
		LockVideoPage();				/* Init the video buffer */
		i = Height;
		DestPtr = VideoPointer;
		Line = Width*((Depth+7)>>3);
		do {
			FastMemSet(DestPtr,0,Line);
			DestPtr += VideoWidth;
		} while (--i);
		UnlockVideoPage();
	}
	return FALSE;				/* I am cool */
}

/**********************************

	Set up the current video page that is being displayed
	for drawing, lock down the pixels
	and set the burgerlib video port

**********************************/

void BURGERCALL LockFrontVideoPage(void)
{
	PixMapHandle MyPixMap;

	if (!VideoPageLocked) {
		if (VideoFullScreen) {
			CGrafPtr MyPort;
			MacUseBackBuffer = FALSE;		/* Force use of the front buffer */
			MyPort = GetTheFrontBuffer();

			SetGWorld(MyPort,VideoDevice);					/* Set the grafport */
			MyPixMap = GetGWorldPixMap(MyPort);				/* Get the GWorld pixmap */
#if LOCKTHEPIXELS
			Locals.LockedPixMap = MyPixMap;
			LockPixels(MyPixMap);
#endif
			VideoPointer = (Word8 *) GetPixBaseAddr(MyPixMap);		/* Get pointer to screen mem */
			VideoWidth = (Word) GetPixRowBytes(MyPixMap);	/* Get width of screen mem */
			VideoPageLocked = TRUE;
		} else {
			MacUseBackBuffer = TRUE;		/* Force use of the back buffer */
			LockVideoPage();
		}
	}
}

/**********************************

	Set up the current video page to the back buffer
	for drawing, lock down the pixels
	and set the burgerlib video port

**********************************/

void BURGERCALL LockVideoPage(void)
{
	PixMapHandle MyPixMap;
	CGrafPtr MyPort;
	
	if (!VideoPageLocked) {
		/* Since there is no override, now get the front or back buffer... */

		/* Try the back buffer first (Most likely) */

		if (MacUseBackBuffer) {						/* Lock the back buffer? */
			if (VideoGWorld) {						/* Use a offscreen GWorld? */
				MyPort = VideoGWorld;	/* Get pointer to offscreen mem */
			} else {
				MyPort = GetTheBackBuffer();
			}
		} else {

		/* Lock the front buffer? */
			MyPort = GetTheFrontBuffer();
		}

		/* MyPixMap has the Mac PixMap to the grafporf I want to play with */
			
		SetGWorld(MyPort,VideoDevice);					/* Set the grafport */
		MyPixMap = GetGWorldPixMap(MyPort);				/* Get the GWorld pixmap */
#if LOCKTHEPIXELS
		Locals.LockedPixMap = MyPixMap;
		LockPixels(MyPixMap);
#endif
		VideoPointer = (Word8 *) GetPixBaseAddr(MyPixMap);	/* Get pointer to screen mem */
		VideoWidth = (Word) GetPixRowBytes(MyPixMap);	/* Get width of screen mem */
		VideoPageLocked = TRUE;
	}
}

/**********************************

	Release the use of the video memory

**********************************/

void BURGERCALL UnlockVideoPage(void)		/* Not needed on the mac */
{
	if (VideoPageLocked) {
#if LOCKTHEPIXELS
		if (Locals.LockedPixMap) {
			UnlockPixels(Locals.LockedPixMap);
			Locals.LockedPixMap = 0;
		}
#endif
		VideoPageLocked = FALSE;
	}
}

/**********************************

	Take the previous screen and display it to the current
	video display hardware and now draw directly
	to the video display hardware

**********************************/

void BURGERCALL UpdateAndNoPageFlip(void)
{	
	Word BackBufferFlag;
	
	UnlockVideoPage();
	
	BackBufferFlag = MacUseBackBuffer;		/* Get the flag's current state */
	
	if (VideoFullScreen) {					/* Can I use the front buffer? */
		MacUseBackBuffer = FALSE;			/* From now on, use the front buffer */
	} else {
		MacUseBackBuffer = TRUE;
	}
	
	if (BackBufferFlag) {				/* Was a back buffer used? */
	
		/* Am I using a software override buffer? */
		/* I.E. Am I pixel doubling? */

		if (Locals.PixelDoubleActive && VideoFullScreen) {
			PixMapHandle MyPixMap;
			Word8 *Dest,*Src;
			Word HardWidth;
			Word SrcWidth;

		/* Destination grafport */
		
#if defined(__POWERPC__)
			if (MacContext) {
				CGrafPtr DestPortMem;
				CGrafPtr DestPort;
				DSpContext_GetFrontBuffer(MacContext,&DestPortMem);
				DestPort = DestPortMem;
				MyPixMap = GetGWorldPixMap(DestPort);
			} else
#endif
			{
				MyPixMap = VideoDevice[0]->gdPMap;		/* Get pointer to screen mem */
			}
			Dest = (Word8 *) GetPixBaseAddr(MyPixMap);	/* Get pointer to screen mem */
			HardWidth = (Word) GetPixRowBytes(MyPixMap);	/* Get width of screen mem */

			MyPixMap = GetPortPixMap(VideoGWorld);	/* Get pointer to offscreen mem */
			Src = (Word8 *) GetPixBaseAddr(MyPixMap);	/* Get pointer to screen mem */
			SrcWidth = (Word) GetPixRowBytes(MyPixMap);	/* Get width of screen mem */

			/* Here, I just blast the pixels as fast as I can */

			if (!VideoPixelDoubleMode) {
				if (VideoColorDepth==8) {
					VideoPixelDouble(Src,Dest,SrcWidth,HardWidth,ScreenWidth,ScreenHeight);
				} else {
					VideoPixelDouble16(Src,Dest,SrcWidth,HardWidth,ScreenWidth,ScreenHeight);
				}
			} else {
				if (VideoColorDepth==8) {
					VideoPixelDoubleChecker(Src,Dest,SrcWidth,HardWidth,ScreenWidth,ScreenHeight);
				} else {
					VideoPixelDoubleChecker16(Src,Dest,SrcWidth,HardWidth,ScreenWidth,ScreenHeight);
				}
			}

		} else if (VideoGWorld) {

		/* Ok, I am using an offscreen GWorld as the source buffer */
		/* As a result, I will use CopyBits for 2D hardware blitting (Very fast!) */	
		
			Rect SrcRect;
			Rect DestRect;
			CGrafPtr DestPortPtr;

			DestPortPtr = GetTheFrontBuffer();
			GetPortBounds(DestPortPtr,&DestRect);
			SrcRect.top = 0;
			SrcRect.left = 0;
			SrcRect.bottom = (short)ScreenHeight;
			SrcRect.right = (short)ScreenWidth;
#if !TARGET_API_MAC_CARBON
			SetGWorld(VideoGWorld,VideoDevice);			/* CopyBits in classic needs to be set to the offscreen buffer */
			CopyBits((BitMap *)&VideoGWorld->portPixMap,(BitMap *)&DestPortPtr->portPixMap,
				&SrcRect, &DestRect,srcCopy,DestPortPtr->visRgn);
#else

/* This fixed a stupid ATI bug in MacOS X 10.0.4. It looks like it's not needed anymore */

#if 1
			if (MacOSGetOSVersion()<0x1000) {
				SetGWorld(VideoGWorld,VideoDevice);			/* CopyBits in classic needs to be set to the offscreen buffer */
			} else {
				SetGWorld(DestPortPtr,VideoDevice);			/* CopyBits in classic needs to be set to the offscreen buffer */
			}
#else
			/* Lane! This DOESN'T WORK! Under OSX running in window'd mode */
			/* The copybits function will always draw the rect in the upper left */
			/* hand corner. Leave the #if 1 alone */
			
			SetGWorld(VideoGWorld,VideoDevice);			/* CopyBits in classic needs to be set to the offscreen buffer */
#endif

			/* Carbon needs a lot more CRAP!! */
			{
				const BitMap *SourcePix;
				const BitMap *DestPix;
				SourcePix = GetPortBitMapForCopyBits(VideoGWorld);
				DestPix = GetPortBitMapForCopyBits(DestPortPtr);
				CopyBits(SourcePix,DestPix,&SrcRect,&DestRect,srcCopy,0);
				
			/* This hack is only here because MacOS X won't */
			/* update a palette properly. Sigh. */
		
				if (BurgerPaletteDirty) {
					Word8 TempPalette[768];					/* Palette copy */
					FastMemCpy(TempPalette,CurrentPalette,768);	/* Copy to temp */
					FastMemSet(CurrentPalette,66,768);		/* Stupid palette */
					PaletteSetPtr(0,256,TempPalette);		/* Set the palette AGAIN! */
					BurgerPaletteDirty = FALSE;				/* Kill the flag */
				}

			/* OSX uses port caching, make sure it is updated */
			
				if (QDIsPortBuffered(DestPortPtr)) {
					QDFlushPortBuffer(DestPortPtr,NULL);
				}
			}
#endif
			/* This is MANDATORY to allow the window title etc to draw properly in */
			/* NEVER REMOVE the SetGWorld() call */
		
			SetGWorld(DestPortPtr,VideoDevice);
		}
	}
}

/**********************************

	Take the previous screen and display it to the current
	video display hardware and now draw to a hidden
	page for page flipping

**********************************/

void BURGERCALL UpdateAndPageFlip(void)
{
	UpdateAndNoPageFlip();		/* Display the current screen */
	MacUseBackBuffer = TRUE;	/* Use the back buffer from now on */
}

/**********************************

	Release everything about the graphics system
	and go back to the ORIGINAL video mode

**********************************/

void BURGERCALL ReleaseVideo(void)
{
	ReleaseGL();		/* Release OpenGL */

	/* If drawsprocket is present then discard everything */

#if defined(__POWERPC__)

	/* This code will fix a bug in DrawSprocket for Classic */
	/* If the port is set to an offscreen buffer, it may leave the */
	/* system pointing to the SECOND video page */
	/* The following 4 lines MUST STAY to prevent the bug from triggering */
	
	if (MacContext) {					/* Shut down draw sprocket */
		CGrafPtr MyPort;
		DSpContext_GetFrontBuffer(MacContext,&MyPort);	/* Get the front grafport */
		SetGWorld(MyPort,VideoDevice);					/* Set the grafport */
	}
	
	ReleaseOldContext();	/* Release all the video modes */
	StopDrawSprocket();		/* Kill off draw sprocket */
#endif
	ReleaseOldGWorld();		/* Kill the old GWorld */
	ReleaseOldWindow();		/* Kill it */
	ScreenWidth = 0;		/* Force default */
	ScreenHeight = 0;
	
#if !TARGET_API_MAC_CARBON
	{
		CGrafPtr MyPort;
		GetCWMgrPort(&MyPort);	/* Make sure I am set to a valid port */
		SetGWorld(MyPort,0);	/* Set it so that I won't go bad */
	}
#else
	if (Locals.WinProc) {
		DisposeEventHandlerUPP(Locals.WinProc);
		Locals.WinProc = 0;
	}
#endif

}

/**********************************

	Initialize a video mode and prepare for
	use of Burgerlib
	Note : I do not implement page flipping since every
	benchmark on the Mac I've done sucks so bad
	that it is a serious waste of time to write the code
	to truly support page flipping.

	Maybe in the future, MacOSX will have page flipping that works, but then that
	will be a new version of Burgerlib and this code won't need to be updated.

**********************************/

Word BURGERCALL SetDisplayToSize(Word Width,Word Height,Word Depth,Word Flags)
{
	Word Hertz;
#if defined(__POWERPC__)
	int ErrCode;
#endif	
	/* Simple depth overrides */

	UnlockVideoPage();		/* Clear out old events */
	
	/* Only allow doubling on 320 wide contexts */	
	if (Width!=320) {
		Flags &= (~SETDISPLAYDOUBLEOK);
	}
	Hertz = Depth>>16;			/* Isolate the refresh rate */
	Depth = Depth&0xFFFF;		/* Get the bit depth */
	
	/* MacOS only uses 8,16,32 bits per pixel */
	
	if (Depth==24) {
		Depth = 32;		/* Full 32 bits */
	}
	if (Depth==15) {
		Depth = 16;		/* Full 16 bits (Although it ends up as 15 bits) */
	}

	/* Handle bit size variables */
	
	if (Depth == 8) {
		VideoRedBits = VideoGreenBits = VideoBlueBits = 0;
		VideoRedShift = 0;
		VideoGreenShift = 0;
		VideoBlueShift = 0;
		VideoRedMask = 0;
		VideoGreenMask = 0;
		VideoBlueMask = 0;
	} else if (Depth == 16) {
		VideoRedBits = VideoGreenBits = VideoBlueBits = 5;
		VideoRedShift = 10;
		VideoGreenShift = 5;
		VideoBlueShift = 0;
		VideoRedMask = 0x1F<<10;
		VideoGreenMask = 0x1F<<5;
		VideoBlueMask = 0x1F;
	} else if (Depth == 32) {
		VideoRedBits = VideoGreenBits = VideoBlueBits = 8;
		VideoRedShift = 16;
		VideoGreenShift = 8;
		VideoBlueShift = 0;
		VideoRedMask = 0xFF<<16;
		VideoGreenMask = 0xFF<<8;
		VideoBlueMask = 0xFF;
	} if (Depth!=8 && Depth!=16 && Depth!=32) {
		NonFatal("Requested pixel depth is not supported\n");
		return TRUE;
	}

	/* At this point Depth equals 8,16 or 32 only */

	/* Now, am I already in the mode requested? */

	if (ScreenWidth==Width && ScreenHeight==Height && ScreenFlags == Flags) {
		if ((Depth == VideoColorDepth) ||
			(Depth == 16 && VideoColorDepth == 15)) {
			if (VideoGWorld) {
				return FALSE;		/* You are in THE SAME MODE!!! */
			}
		}
	}

	/* Ok, now destroy any previous data */
	/* I don't call ReleaseVideo() since I want to see if I really */
	/* need to kill the video mode. If not, I'll weasel out somehow */
	/* This way I can limit the number of video mode switches I have to do */

	ReleaseOldGWorld();		/* Kill the old GWorld */
	ReleaseGL();			/* Release OpenGL */
	ReleaseOldWindow();		/* Kill it */
	Locals.PixelDoubleActive = FALSE;		/* Failsafe */

	/* Make sure I have a valid port before shutting down */
	
#if !TARGET_API_MAC_CARBON
#if defined(__POWERPC__)
	if (MacContext) {				/* Am I using drawsprocket? */
		CGrafPtr MyPort;
		DSpContext_GetFrontBuffer(MacContext,&MyPort);	/* Get the front grafport */
		SetGWorld(MyPort,VideoDevice);					/* Set the grafport */
	} else 
#endif
	{
		CGrafPtr MyTempPtr;
		GetCWMgrPort(&MyTempPtr);	/* Make sure I am set to a valid port */
		SetGWorld(MyTempPtr,0);		/* Set it so that I won't go bad */
	}
#endif

	/* Handle draw sprocket support for Power Macintoshes */

#if defined(__POWERPC__)

	/* First thing I do is make sure that DrawSprocket is even present and initialized */

	if (Flags&SETDISPLAYFULLSCREEN) {

		/* If I use a default freqency, try 60 hertz */
		/* Fixes an issue with MacOS X */
		
		if (!Hertz) {
			Hertz = 60;
		}
		
		/* See if there is an alternate context present */
		
		if (MacContext) {
			Word i;
			i = Locals.ContextCacheCount;
			if (i) {
				DSpContextReference CloseContext;	/* Cached context for mismatched freq */
				ContextCache_t *WorkPtr;
				Word TestWidth,TestHeight;
				
				WorkPtr = (ContextCache_t *)LockAHandle((void **)Locals.ContextCache);
				CloseContext = 0;			/* No close match */
				if (Flags&SETDISPLAYDOUBLEOK) {
					TestWidth = Width*2;			/* Video width requested */
					TestHeight = Height*2;
					Locals.PixelDoubleActive = TRUE;
				} else {
					TestWidth = Width;			/* Video width requested */
					TestHeight = Height;
					Locals.PixelDoubleActive = FALSE;
				}
				
				do {
					if ((WorkPtr->Width == TestWidth) && 	/* Match? */
						(WorkPtr->Height == TestHeight) &&
						(WorkPtr->Depth == Depth)) {
						if (!CloseContext) {			/* First one hit? */
							CloseContext = WorkPtr->Context;		/* Save the close context */
						}
						if (WorkPtr->Hertz == Hertz) {		/* Try frequency match */
							if ((MacContext == WorkPtr->Context) ||		/* Already selected? */
								!DSpContext_Switch(MacContext,WorkPtr->Context)) {
								MacContext = WorkPtr->Context;		/* Ok, let's switch */
								UnlockAHandle((void **)Locals.ContextCache);
								goto StateSwitched;					/* Tell burgerlib about it */
							}
						}
					}
					++WorkPtr;
				} while (--i);
				UnlockAHandle((void **)Locals.ContextCache);	/* Release the handle */
				if (CloseContext) {							/* One last attempt */
					if ((MacContext == CloseContext) ||
						!DSpContext_Switch(MacContext,CloseContext)) {		/* Try me! */
						MacContext = CloseContext;			/* Ok, let's switch */
						goto StateSwitched;					/* Tell burgerlib about it */
					}
				}
			}
		}
		/* Obviously, I can't just switch. Sigh. */
		
		ReleaseOldContext();		/* Dispose of the cached list if any */

		/* Is DrawSprocket is ok, then try to set the mode via draw sprocket */

		if (StartDrawSprocket()) {		/* Is draw sprocket active? */
			Word Temp;
			DSpContextAttributes Desired;	/* Context I'd like to see */
			DSpContextReference Found;	/* Context I am scanning for */

			/* Now ask DrawSprocket for a context that closely matches what I want */

			FastMemSet(&Desired,0,sizeof(Desired));	/* Clear out the buffer */
			if (Flags&SETDISPLAYDOUBLEOK) {
				Desired.displayWidth = Width*2;			/* Video width requested */
				Desired.displayHeight = Height*2;
				Locals.PixelDoubleActive = TRUE;
			} else {
				Desired.displayWidth = Width;			/* Video width requested */
				Desired.displayHeight = Height;
				Locals.PixelDoubleActive = FALSE;
			}
			Desired.colorNeeds = kDSpColorNeeds_Require;	/* I require color! */
			if (Depth==8) {
				Temp = kDSpDepthMask_8;		/* I want an 8 bit context */
			} else if (Depth==16) {
				Temp = kDSpDepthMask_16;	/* I want a 16 bit context */
			} else {
				Temp = kDSpDepthMask_32;	/* I want a 32 bit context */
			}
			Desired.backBufferDepthMask = Temp;	/* Set the back buffer mask */
			Desired.displayDepthMask = Temp;
			Desired.backBufferBestDepth = Depth;	/* Depth I want */
			Desired.displayBestDepth = Depth;
			Desired.contextOptions = /* kDSpContextOption_PageFlip | kDSpContextOption_DontSyncVBL */ 0;	/* No page flipping */
			Desired.pageCount = 1;		/* Was 3 */
			Desired.frequency = Hertz << 16;	/* Request frequency passed in */
			Found = 0;					/* Didn't find anything yet */

			if (MacOSGetOSVersion()<0x1000) {
				Bool Answer;
				Answer = 0;
				if (!DSpCanUserSelectContext(&Desired,&Answer)) {		/* Multiple monitors? */
					if (Answer) {
						Word State;
						Word Answer;
						Word CursorState;
						State = InputSetState(FALSE);
						CursorState = OSCursorIsVisible();
						OSCursorShow();
						Answer = DSpUserSelectContext(&Desired,0,0,&Found);
						if (!CursorState) {
							OSCursorHide();
						}
						InputSetState(State);
						if (!Answer) {	/* Select a context */
							goto FoundIt;
						}
					}
				}

				/* Have DrawSpocket make the choice */
				
				if (DSpFindBestContext(&Desired,&Found)) {	/* See If I have this video mode */
					Desired.frequency = 0;
					if (DSpFindBestContext(&Desired,&Found)) {	/* See If I have this video mode */
						ReleaseOldContext();
						return TRUE;			/* Don't use draw sprocket */
					}
				}
			} else {
				/* Have DrawSpocket make the choice */
				DisplayIDType DisplayID;
				DMGetDisplayIDByGDevice(GetMainDevice(), &DisplayID, true);
				
				if (DSpFindBestContextOnDisplayID(&Desired,&Found, DisplayID)) {	/* See If I have this video mode */
					ReleaseOldContext();
					return TRUE;			/* Don't use draw sprocket */
				}
			}

FoundIt:;
			/* I got a context I think will work, let's reserve it */

			if (CreateContextCache(Found,&Desired)) {
				return TRUE;
			}

			/* It's MINE!!! Enable it. */
			
			ErrCode = DSpContext_SetState(Found,kDSpContextState_Active);
			if (!ErrCode || (ErrCode == kDSpConfirmSwitchWarning)) {	/* Activate the context */
				MacContext = Found;
StateSwitched:;
				{
					/* Get the video device for the full screen context */
#if 1
					CGrafPtr MyPort;
					DSpContext_GetFrontBuffer(MacContext,&MyPort);
					VideoDevice = GetGWorldDevice(MyPort);
#else

			/* It appears that in OSX, this code will return a NULL GDevice */
					DisplayIDType displayID;
					if (!DSpContext_GetDisplayID(MacContext, &displayID)) {			/* Get the device # */
						DMGetGDeviceByDisplayID(displayID,&VideoDevice,FALSE);		/* Grab it */
					}
#endif
					VideoTrueScreenWidth = Width;
					VideoTrueScreenHeight = Height;
					if (!MacMakeOffscreenGWorld(Width,Height,Depth,Flags)) {
						MouseSetRange(Width,Height);
						UpdateAndPageFlip();	/* Force an update */
						return FALSE;			/* Let's go!! */
					}
				}
			}
			MacContext = 0;		/* Context is invalid somehow */
			ScreenWidth = 0;		/* I am a loozer! */
			ScreenHeight = 0;
			ReleaseOldContext();
			return TRUE;
		}
	}
	ReleaseOldContext();
#endif
	/* If DrawSprocket is either broken or disabled (Window mode) */
	/* Generate a context here using an offscreen GWorld */

	if (Locals.PixelDoubleActive) {		/* I could not do a pixel double mode */
		Locals.PixelDoubleActive = FALSE;
		return TRUE;
	}
	Flags &= (~SETDISPLAYFULLSCREEN);		/* Make sure my flags are OK */
	
	/* Ok, which monitor shall I put this window on? */
	
	FindWindowDevice(Width,Height,Depth);
	
	/* Create the main window in the center of the screen */
	/* VideoDevice is initialized at this point */
	
	{
		Rect r;
		SetNewWinBoundsRect(&r,Width,Height,Flags);

#if !TARGET_API_MAC_CARBON
		VideoWindow = NewCWindow(0,&r,Locals.MyWinTitle,FALSE, noGrowDocProc,kFirstWindowOfGroup, FALSE, 0);
		if (!VideoWindow) {		/* No background window? */
			return TRUE;		/* You are screwed */
		}
		{
			AuxWinHandle awh;
			if (GetAuxWin(VideoWindow, &awh )) {
				CTabHandle theColorTable;
				OSErr err;

				theColorTable = awh[0]->awCTable;
				err = HandToHand( (Handle*)&theColorTable );
				if (!err) {
					ColorSpec *Local;
					Local = &theColorTable[0]->ctTable[wContentColor];
					Local->rgb.red = 0;
					Local->rgb.green = 0;
					Local->rgb.blue = 0;
					CTabChanged( theColorTable );

					// the color table will be disposed by the window manager when the window is disposed
					SetWinColor(VideoWindow, (WCTabHandle)theColorTable );
				}
			}
		}
#else
		if (MacOSGetOSVersion()<0x1000) {
			VideoWindow = NewCWindow(nil,&r,Locals.MyWinTitle,FALSE, noGrowDocProc, kFirstWindowOfGroup, FALSE, 0);
		} else {
			if (CreateOSXWindow(&r)) {
				return TRUE;		/* Ok, you are screwed */
			}
		}
#endif
		SetPortWindowPort(VideoWindow);
		MacShowWindow(VideoWindow);
	}

	/* Create my offscreen buffer */

	if (MacMakeOffscreenGWorld(Width,Height,Depth,Flags)) {
		ReleaseOldWindow();		/* Kill any window */
		return TRUE;
	}

	/* Now show my game window */

#if !TARGET_API_MAC_CARBON
	VideoTrueScreenWidth = qd.screenBits.bounds.right-qd.screenBits.bounds.left;
	VideoTrueScreenHeight = qd.screenBits.bounds.bottom-qd.screenBits.bounds.top;
#else
	{
		if (VideoDevice) {
			Rect *Foo44;
			Foo44 = &VideoDevice[0]->gdRect;
			VideoTrueScreenWidth = Foo44->right-Foo44->left;
			VideoTrueScreenHeight = Foo44->bottom-Foo44->top;
		} else {
			BitMap MyBits;
			GetQDGlobalsScreenBits(&MyBits);
			VideoTrueScreenWidth = MyBits.bounds.right-MyBits.bounds.left;
			VideoTrueScreenHeight = MyBits.bounds.bottom-MyBits.bounds.top;
		}
	}
#endif
	MouseSetRange(Width,Height);
	return FALSE;					/* I'm cool... */
}

/**********************************

	If a window is present, set the text to a specific string

**********************************/

void BURGERCALL VideoSetWindowString(const char *Title)
{
	CStr2PStr((char *)Locals.MyWinTitle,Title);
	if (VideoWindow) {
		SetPortWindowPort(VideoWindow);
		SetWTitle(VideoWindow,Locals.MyWinTitle);
	}
}

/**********************************

	Return a handle to an array of video display modes
	Note : I can add other modes that I can fake in software

**********************************/

static const Word FakeModes[] = {
	320,200, 320,240, 400,300, 512,384, 640,400, 640,480, 0
};

/**********************************

	Given a GDHandle, return all the display modes

**********************************/

static VideoModeArray_t ** BURGERCALL GetModes(GDHandle MyDevice,Word HardwareOnly,Word DevNum)
{
#if TARGET_RT_MAC_CFM			/* Only available with DrawSprocket */
	VideoModeArray_t **Result;
	StreamHandle_t MyStream;
	DisplayIDType MyDevID;
	DSpContextReference MyRef;
	VideoMode_t MyEntry;	/* Video mode found */
	Word Output;
	char DeviceName[64];

	StreamHandleInitPut(&MyStream);
	StreamHandlePutLong(&MyStream,0);
	StreamHandlePutMem(&MyStream,&DevNum,sizeof(DevNum));
	sprintf(DeviceName,"Display device #%d",DevNum);
	StreamHandlePutMem(&MyStream,DeviceName,64);
	Output = FALSE;				/* 16 bit mode was not found */

	if (!DMGetDisplayIDByGDevice(MyDevice,&MyDevID,TRUE)) {	/* Get the device ID */
		DSpGetFirstContext(MyDevID,&MyRef);			/* Get the first context */
		if (MyRef) {				/* Valid reference */
			do {
				DSpContextAttributes MyAttr;		/* Space for the attributes */
				FastMemSet(&MyAttr,0,sizeof(MyAttr));
				if (!DSpContext_GetAttributes(MyRef,&MyAttr)) {
					Word Flags;
					MyEntry.Width = MyAttr.displayWidth;		/* Save the pixel width */
					MyEntry.Height = MyAttr.displayHeight;
					MyEntry.Depth = MyAttr.displayBestDepth;	/* Depth */
					MyEntry.Hertz = MyAttr.frequency>>16;
					Flags = 0;
					if (MyEntry.Depth==8) {
						Output |= 1;
					} else if (MyEntry.Depth==16) {
						Output |= 2;
#if !defined(__POWERPC__)
						Flags = VIDEOMODEHARDWARE;
#else
						Flags = VIDEOMODEOPENGL|VIDEOMODEHARDWARE;
#endif
					} else if (MyEntry.Depth==32) {
						Output |= 4;
#if !defined(__POWERPC__)
						Flags = VIDEOMODEHARDWARE;
#else
						Flags = VIDEOMODEOPENGL|VIDEOMODEHARDWARE;
#endif
					}
					MyEntry.Flags = Flags;
					StreamHandlePutMem(&MyStream,&MyEntry,sizeof(MyEntry));
				}
				if (DSpGetNextContext(MyRef,&MyRef)) {	/* Next context */
					break;			/* Error?? */
				}
			} while (MyRef);		/* All done? */
		}
	}

	if (!(HardwareOnly&VIDEOMODEGETNOFAKE)) {
		Word i;
		i = 0;
		do {
			MyEntry.Width = FakeModes[i];
			MyEntry.Height = FakeModes[i+1];
			MyEntry.Hertz = 0;
			MyEntry.Flags = VIDEOMODEFAKE;
			if (Output&1) {
				MyEntry.Depth = 8;
				StreamHandlePutMem(&MyStream,&MyEntry,sizeof(MyEntry));
			}
			if (Output&2) {
				MyEntry.Depth = 16;
				StreamHandlePutMem(&MyStream,&MyEntry,sizeof(MyEntry));
			}
			if (Output&4) {
				MyEntry.Depth = 32;
				StreamHandlePutMem(&MyStream,&MyEntry,sizeof(MyEntry));
			}

			i+=2;
		} while (FakeModes[i]);
	}
	StreamHandleEndSave(&MyStream);
	Result = (VideoModeArray_t **)StreamHandleDetachHandle(&MyStream);
	StreamHandleDestroy(&MyStream);
	if (Result) {
		Word Offset;
		Offset = GetAHandleSize((void **)Result);
		Offset = (Offset-(64+(sizeof(Word)*2)))/sizeof(MyEntry);	/* Any data present? */
		if (Offset) {
			Result[0]->Count = Offset;
			return Result;
		}
		DeallocAHandle((void **)Result);	/* Kill the handle */
	}
#endif
	return 0;
}

/**********************************

	Return all the video modes present in a specific device

**********************************/

VideoModeArray_t ** BURGERCALL VideoModeArrayNew(Word HardwareOnly)
{
#if TARGET_RT_MAC_CFM			/* Only available with DrawSprocket */
	GDHandle MyDevice;
	Word Which;

	if (StartDrawSprocket()) {				/* Draw sprocket linked in? */
		Which = HardwareOnly&0xFF;
		if (!Which) {
			Which = 1;
		}
		MyDevice = GetDeviceList();	/* Get the first active device */
		if (MyDevice) {
			Word DevNum;
			DevNum = 1;
			do {
				if (DevNum==Which) {
					return GetModes(MyDevice,HardwareOnly,DevNum);	/* Get the modes available */
				}
				++DevNum;
				MyDevice = GetNextDevice(MyDevice);			/* Next device in the chain */
			} while (MyDevice);								/* More? */
		}
	}
#endif
	return 0;			/* Oh poo! */
}

/**********************************

	Return all the video modes present in all devices

**********************************/

VideoDeviceArray_t ** BURGERCALL VideoDeviceArrayNew(Word HardwareOnly)
{
#if TARGET_RT_MAC_CFM			/* Only available with DrawSprocket */
	GDHandle MyDevice;
	Word DevNum;
	StreamHandle_t MyStream;
	VideoDeviceArray_t **Result;

	if (StartDrawSprocket()) {				/* Draw sprocket linked in? */
		StreamHandleInitPut(&MyStream);
		StreamHandlePutLong(&MyStream,0);
		MyDevice = GetDeviceList();	/* Get the first active device */
		if (MyDevice) {
			DevNum = 0;
			do {
				VideoModeArray_t **TempResult;
				TempResult = GetModes(MyDevice,HardwareOnly,DevNum+1);	/* Get the modes available */
				if (TempResult) {
					StreamHandlePutLongMoto(&MyStream,(Word32)TempResult);
					++DevNum;
				}
				MyDevice = GetNextDevice(MyDevice);			/* Next device in the chain */
			} while (MyDevice);								/* More? */
		}
		StreamHandleEndSave(&MyStream);
		Result = (VideoDeviceArray_t **)StreamHandleDetachHandle(&MyStream);
		StreamHandleDestroy(&MyStream);
		if (Result) {
			Result[0]->Count = DevNum;
			if (DevNum) {
				return Result;
			}
			DeallocAHandle((void **)Result);
		}
	}
#endif
	return 0;
}

#if defined(__POWERPC__)

/**********************************

	Make sure the OpenGL context is burgerlib's

**********************************/

void BURGERCALL OpenGLSetCurrent(void)
{
	if (Locals.BurgerGLContext) {			/* Failsafe */
		aglSetCurrentContext(Locals.BurgerGLContext);				/* Make this my context */
	}
}

/**********************************

	Perform a page flip in OpenGL

**********************************/

void BURGERCALL OpenGLSwapBuffers(void)
{
	if (Locals.BurgerGLContext) {
		Locals.OpenGLFlip++;
		aglSwapBuffers(Locals.BurgerGLContext);
	}
}

/**********************************

	Get a pixel format

**********************************/

static AGLPixelFormat BURGERCALL OpenGLGetFormat(const GLint *attrib)
{
	AGLPixelFormat BurgerPixelFormat;
	if (VideoDevice) {		/* Do I already have a device picked? */
		BurgerPixelFormat = aglChoosePixelFormat(&VideoDevice,1, attrib);
		if (!BurgerPixelFormat) {			/* Didn't get it? */
			BurgerPixelFormat = aglChoosePixelFormat(0, 0, attrib);	/* Try generic */
		}
	} else {
		BurgerPixelFormat = aglChoosePixelFormat(0, 0, attrib);
	}
	return BurgerPixelFormat;			/* Return the format selected */
}

/**********************************

	Initialize the OpenGL context and set the video display
	to a specific size

**********************************/

static GLint attrib[] = { AGL_RGBA, AGL_ACCELERATED,AGL_DOUBLEBUFFER,AGL_NO_RECOVERY,
	AGL_RED_SIZE,8,AGL_GREEN_SIZE,8,AGL_BLUE_SIZE,8,AGL_ALPHA_SIZE,0,
	AGL_STENCIL_SIZE,0,AGL_DEPTH_SIZE, 16,AGL_NONE };

Word BURGERCALL OpenGLSetDisplayToSize(Word Width,Word Height,Word Depth,Word Flags)
{
	AGLPixelFormat BurgerPixelFormat;
	AGLDrawable GLWindow;
	Word Temp;
	
	if (!aglChoosePixelFormat) {		/* Is OpenGL even linked in? */
		ReleaseVideo();		/* Make SURE I released the video context! */
		return TRUE;
	}

	Locals.KillGLContextProc = OpenGLRelease;
	
	/* If window mode, create a window */

	if (SetDisplayToSize(Width,Height,Depth,Flags|SETDISPLAYOPENGL)) {
		return TRUE;		/* Oh oh... */
	}

	if (VideoColorDepth==32) {
		Temp = 8;
	} else {
		Temp = 5;
	}
	attrib[5] = Temp;
	attrib[7] = Temp;
	attrib[9] = Temp;
//	attrib[15] = 16;	// Depth buffer
	
	ReleaseOldGWorld();			/* Kill the old GWorld (GL has no need for it) */
	LockFrontVideoPage();		/* Fix the current port */
	UnlockVideoPage();

	/* See if the window was created */

	if (VideoWindow) {									/* I am a window already */
		MacShowWindow(VideoWindow);		/* Display the window */
		GLWindow = GetWindowPort(VideoWindow);
	/* Full screen contexts need a window anyways or OpenGL will barf! */
	
	} else {
		Rect rectWin;
		RGBColor rgbSave;
		int Work;
		GDPtr VideoPtr;
		
		// create a new window in our context 
		// note: OpenGL is expecting a window so it can enumerate the devices it spans, 
		// center window in our context's gdevice

		SetGDevice(VideoDevice);			/* Must be mounted onto my device to begin with!!! */
		
		/* Now this really sucks. */
		/* MacOS 9 and before, needs a window for OpenGL to enumerate from */
		/* Yet MacOS 10 will have this window cover up everything. */
		/* Yet another damn code fork */
		
		if (MacOSGetOSVersion()<0x1000) {
			VideoPtr = VideoDevice[0];
			Work = (VideoPtr->gdRect.top + (VideoPtr->gdRect.bottom - VideoPtr->gdRect.top) / 2);
			rectWin.top = (short) (Work-(ScreenHeight / 2));
			Work = (VideoPtr->gdRect.left + (VideoPtr->gdRect.right - VideoPtr->gdRect.left) / 2);
			rectWin.left = (short) (Work-(ScreenWidth / 2));
			rectWin.right = (short) (rectWin.left + ScreenWidth);
			rectWin.bottom = (short) (rectWin.top + ScreenHeight);
			
			/* Now, I've got the port rect, make a phony window */
			/* The GDevice MUST be set to the proper display */
			
			VideoWindow = NewCWindow(NULL, &rectWin, Locals.MyWinTitle, 0, plainDBox, kFirstWindowOfGroup, 0, 0);
			// paint back ground black before fade in to avoid white background flash
			
			MacShowWindow(VideoWindow);
			SetPortWindowPort(VideoWindow);
			GetForeColor(&rgbSave);
			RGBForeColor(&Locals.rgbBlack);
			GetWindowPortBounds(VideoWindow,&rectWin);
			PaintRect(&rectWin);
			RGBForeColor(&rgbSave);		// ensure color is reset for proper blitting
			GLWindow = GetWindowPort(VideoWindow);
		} else {
			CGrafPtr pPort;
			// use DSp's front buffer on Mac OS X
			DSpContext_GetFrontBuffer(MacContext, &pPort);

		// there is a problem in Mac OS X GM CoreGraphics that may not size the port pixmap correctly
		// this will check the vertical sizes and offset if required to fix the problem
		// this will not center ports that are smaller then a particular resolution
			{
				long deltaV, deltaH;
				Rect portBounds;
				PixMapHandle hPix = GetPortPixMap(pPort);
				Rect pixBounds = hPix[0]->bounds;
				GetPortBounds(pPort,&portBounds);
				deltaV = (portBounds.bottom - portBounds.top) - (pixBounds.bottom - pixBounds.top) +
				         (portBounds.bottom - portBounds.top - ScreenHeight) / 2;
				deltaH = -(portBounds.right - portBounds.left - ScreenWidth) / 2;
				if (deltaV || deltaH) {
					GrafPtr pPortSave;
					GetPort(&pPortSave);
					SetPort((GrafPtr)pPort);
					// set origin to account for CG offset and if requested drawable smaller than screen rez
					SetOrigin(deltaH, deltaV);
					SetPort(pPortSave);
				}
			}
			GLWindow = pPort;
		}
	}
	
/* Never unload a code module */
	
	aglConfigure(AGL_RETAIN_RENDERERS, GL_TRUE);

/* Choose pixel format */

	BurgerPixelFormat = OpenGLGetFormat(attrib);			/* Get the pixel format */
	if (!BurgerPixelFormat) {
		attrib[1] = AGL_NONE;
		BurgerPixelFormat = OpenGLGetFormat(attrib);		/* Let's try with software */
		attrib[1] = AGL_ACCELERATED;
	}

	if (BurgerPixelFormat) {
	/* Create an AGL context */

		Locals.BurgerGLContext = aglCreateContext(BurgerPixelFormat,0);
		if (Locals.BurgerGLContext) {				/* Did the context get created? */

			/* Attach the context to the window */
			if (aglSetDrawable(Locals.BurgerGLContext,GLWindow)) {		/* Set the grafport */
				/* Make this context current */
				if (aglSetCurrentContext(Locals.BurgerGLContext)) {				/* Make this my context */
					aglDestroyPixelFormat(BurgerPixelFormat);	/* Dispose of it... */
					VideoAPIInUse = VIDEOAPIOPENGL;		/* I am open for business! */
					MouseSetRange(Width,Height);
					Locals.OpenGLFlip = 0;
					return FALSE;			/* I'm OK! */
				}
			}
		}
		aglDestroyPixelFormat(BurgerPixelFormat);	/* Dispose of it... */
	}
	ReleaseVideo();		/* Make SURE I released the video context! */
	return TRUE;		/* Oh oh... */
}

/**********************************

	Release OpenGL is initialized

**********************************/

void BURGERCALL OpenGLRelease(void)
{
	/* Make the context not current, set it's drawable to NULL, and destroy it */

	if (Locals.BurgerGLContext) {			/* Was a context created? */
		if (Locals.OpenGLFlip&1) {			/* Flip to the main page */
			OpenGLSwapBuffers();
		}
		aglSetCurrentContext(0);	/* Set the context */
		aglSetDrawable(Locals.BurgerGLContext,0);
		aglDestroyContext(Locals.BurgerGLContext);
		Locals.BurgerGLContext = 0;
	}
	VideoAPIInUse = VIDEOAPISOFTWARE;		/* Don't call OpenGL anymore! */
}

/**********************************

	Set a new GL context (For manual creation)

**********************************/

AGLContext BURGERCALL OpenGLSetContext(AGLContext Context)
{
	AGLContext OldContext;
	OldContext = Locals.BurgerGLContext;
	Locals.BurgerGLContext = Context;
	return OldContext;
}

/**********************************

	Get my current GL contet

**********************************/

AGLContext BURGERCALL OpenGLGetContext(void)
{
	return Locals.BurgerGLContext;
}

typedef void (*ATITruFormProc)(GLenum pname, GLint param);

#if TARGET_API_MAC_CARBON && 0

/**********************************

	Get the pointer to the GL extension for ATI cards

**********************************/

static ATITruFormProc OpenGLATIProc;		/* Pointer to the OSX ATI GL extension */
static Word8 OpenGLATITested;				/* TRUE if tested */

static ATITruFormProc BURGERCALL MacOSXGetATIProc(void)
{
	ATITruFormProc MyProc;
	CFBundleRef openglBundle;
	FSRefParam fileRefParam;
	FSRef fileRef;
	CFURLRef url;
	
	MyProc = OpenGLATIProc;			/* Did I already find it? */

	if (!OpenGLATITested) {			/* Was it tested? */
		OpenGLATITested = TRUE;
		FastMemSet(&fileRefParam,0,sizeof(fileRefParam));
		FastMemSet(&fileRef,0,sizeof(fileRef));
		
		fileRefParam.ioNamePtr = "\pOpenGL.framework";
		fileRefParam.newRef = &fileRef;

		/* Locate the OS's Frameworks folder */
		if (!FindFolder( kSystemDomain, kFrameworksFolderType, FALSE, &fileRefParam.ioVRefNum, &fileRefParam.ioDirID)) {

			/* Make an FSRef for the OpenGL framework within */
			
			if (!PBMakeFSRefSync(&fileRefParam)) {

				/* Create a url for the OpenGL Framework from the FSRef */

				url = CFURLCreateFromFSRef( kCFAllocatorDefault, &fileRef );
				if (url) {

					/* Load the OpenGL framework bundle */
					openglBundle = CFBundleCreate(kCFAllocatorDefault, url);
					CFRelease(url);		/* Get rid of the URL */
					
					if (openglBundle) {
					
						/* Get the proc pointer */
					
						MyProc = (ATITruFormProc)CFBundleGetFunctionPointerForName(openglBundle,CFSTR("glPNTrianglesiATIX"));
						OpenGLATIProc = MyProc;			/* Set the proc pointer */
					}
				}
			}
		}
	}
	return MyProc;		/* Return the proc pointer */
}

#endif

/**********************************

	Enable or disable TrueForm technology

**********************************/

static const float Truformamb[4] = { 1, 1, 1, 1 };

void BURGERCALL OpenGLATISetTruform(Word Setting)
{	
	long Flag;
	AGLContext MyContext;
	MyContext = Locals.BurgerGLContext;

	if (MyContext) {		/* Is the context valid? */
	
		if (Setting>=8) {
			Setting = 7;		/* Maximum */
		}
		
#if TARGET_API_MAC_CARBON && 0		/* Not needed anymore */
		if (MacOSGetOSVersion()>=0x1000) {
			if (Setting) {
				ATITruFormProc MyProc;
				MyProc = MacOSXGetATIProc();
				glEnable(GL_PN_TRIANGLES_ATIX);
				glEnable(GL_LIGHTING);
				glLightModelfv(GL_LIGHT_MODEL_AMBIENT,Truformamb); 
				glEnable(GL_COLOR_MATERIAL);
				if (MyProc) {
					MyProc(GL_PN_TRIANGLES_TESSELATION_LEVEL_ATIX,Setting);
				}
			} else {
				glDisable(GL_PN_TRIANGLES_ATIX);
				glDisable(GL_LIGHTING);
			}
		} else
#endif
		{

		/* Let's do it the MacOS 9 way */
		
			aglGetError();				/* Flush previous errors */
			if (Setting) {
				Flag = TRUE;			/* Enable */
				aglSetInteger(MyContext, ATI_TRUFORM_ENABLE,&Flag);
				aglGetError();			/* Clear error generated */
				glEnable(GL_LIGHTING);
				glLightModelfv(GL_LIGHT_MODEL_AMBIENT,Truformamb); 
				glEnable(GL_COLOR_MATERIAL);
				Flag = Setting;
				aglSetInteger(MyContext,ATI_TRUFORM_LEVEL,&Flag);
			} else {
				Flag = FALSE;			/* Disable */
				aglSetInteger(MyContext,ATI_TRUFORM_ENABLE,&Flag);
			}
			aglGetError();				/* Flush any error codes that might be generated */
		}
	}
}

/**********************************

	Enable or disable Full Screen Anti-Aliasing technology

**********************************/

void BURGERCALL OpenGLATISetFSAA(Word Setting)
{	
	long Flag;
	AGLContext MyContext;
	MyContext = Locals.BurgerGLContext;

	if (MyContext) {		/* Is the context valid? */

		/* Only 1,2 or 4 are allowed */
			
		if (Setting<1) {
			Setting = 1;		/* Disable the feature */
		}
		if (Setting>=3) {		/* Allow 2x or 4x */
			Setting = 4;
		}
		
		/* Let's do it the MacOS 9 way */

		Flag = Setting;
		aglGetError();		/* Flush previous errors */
		aglSetInteger(MyContext, ATI_FSAA_SAMPLES,&Flag);		/* Enable/Disable the feature */
		aglGetError();		/* Flush any error codes that might be generated */
	}
}

#endif

/**********************************

	Read the Gamma table from the currently
	logged video device.
	MacOS Classic ONLY

**********************************/

static Word MacOSGetGammaTableClassic(GrMacLocals_t *LocalPtr)
{
	VDGammaRecord DeviceGammaRec;
	ParamBlockRec cParam;
	GammaTblPtr GammaTablePtr;
	GDHandle MyDevHand;
	
	MyDevHand = VideoDevice;
	if (!MyDevHand) {
		MyDevHand = GetMainDevice();		/* Get the default device as a failsafe */
	}
	
	if (MyDevHand) {						/* Do I have a device? */
		FastMemSet(&cParam,0,sizeof(cParam));			/* Erase the input structure */
		cParam.cntrlParam.ioCRefNum = MyDevHand[0]->gdRefNum;		/* Get the device number */
		cParam.cntrlParam.csCode = cscGetGamma;				/* Get Gamma commnd to device */

		((VDGammaRecord **)(&cParam.cntrlParam.csParam[0]))[0] = &DeviceGammaRec;	/* record for gamma */
		if (!PBStatusSync(&cParam)) {		/* get gamma table */

			/* I got the gamma table */
			/* Now, let's parse it and get the data from it */
			
			GammaTablePtr = (GammaTblPtr)DeviceGammaRec.csGTable;	// pull table out of record
			if (GammaTablePtr) {
				Word DataWidth;
				
				/* Sanity checks */
				
				DataWidth = GammaTablePtr->gDataWidth;		/* Get the bit size */
				
				if (DataWidth && DataWidth<=8) {			/* Acceptable bit size 1-8 is acceptable */
					Word EntryCount;
					EntryCount = GammaTablePtr->gDataCnt;	/* Number of entries */
					if (EntryCount && EntryCount<=256) {	/* Sane count? 1-256 is ok */
						Word Channels;
						Channels = GammaTablePtr->gChanCnt;
						if (Channels==1 || Channels==3) {	/* Mono or RGB are the only acceptable ones */
							GammaTable_t *DestPtr;
							Word i;
							const Word8 *SrcPtr;
							float Factor;
							SrcPtr = (Word8 *)(&GammaTablePtr->gFormulaData[GammaTablePtr->gFormulaSize]);		/* Data pointer */

							/* Get the RGB byte to float conversion */
							Factor = 1.0f/(float)((1<<DataWidth)-1);		/* Scalar */
							DestPtr = &LocalPtr->GammaOriginalTable;
							FastMemSet(DestPtr,0,sizeof(LocalPtr->GammaOriginalTable));		/* Init the output */
							
							/* Convert a monochrome Gamma ramp to RGB */

							i = 0;
							if (Channels==1) {
								do {
									float FTemp;
									FTemp = ((float)(SrcPtr[0]))*Factor;		/* Convert mono to RGB (Greyscale) */
									DestPtr->Red[i] = FTemp;
									DestPtr->Green[i] = FTemp;
									DestPtr->Blue[i] = FTemp;
									++SrcPtr;
								} while (++i<EntryCount);
							} else {

							/* Convert a RGB ramp to float RGB */
							
								const Word8 *SrcPtr2;
								const Word8 *SrcPtr3;
								
								/* RGB arrays are immediately following */
								
								SrcPtr2 = SrcPtr+EntryCount;		/* Index to the next table */
								SrcPtr3 = SrcPtr2+EntryCount;
								do {
									DestPtr->Red[i]= ((float)(SrcPtr[0]))*Factor;
									DestPtr->Green[i]= ((float)(SrcPtr2[0]))*Factor;
									DestPtr->Blue[i]= ((float)(SrcPtr3[0]))*Factor;
									++SrcPtr;
									++SrcPtr2;
									++SrcPtr3;
								} while (++i<EntryCount);
							}
							FastMemCpy(&LocalPtr->GammaCurrentTable,&LocalPtr->GammaOriginalTable,sizeof(LocalPtr->GammaOriginalTable));
							return FALSE;
						}
					}
				}
			}
		}
	}
	return TRUE;		/* Error! */
}

/**********************************

	Set the Gamma table for the currently
	logged video device with a BurgerLib gamma table
	MacOS Classic ONLY

**********************************/

static Word MacOSSetGammaTableClassic(GrMacLocals_t *LocalPtr,const GammaTable_t *TablePtr)
{
	VDGammaRecord DeviceGammaRec;
	ParamBlockRec cParam;
	GammaTblPtr GammaTablePtr;
	GammaTblPtr NewGammaTablePtr;
	Word8 TempGammaTable[4096];
	GDHandle MyDevHand;


	MyDevHand = VideoDevice;
	if (!MyDevHand) {
		MyDevHand = GetMainDevice();		/* Get the default device as a failsafe */
	}
	
	if (MyDevHand) {						/* Do I have a device? */
		FastMemSet(&cParam,0,sizeof(cParam));			/* Erase the input structure */
		cParam.cntrlParam.ioCRefNum = MyDevHand[0]->gdRefNum;		/* Get the device number */
		cParam.cntrlParam.csCode = cscGetGamma;				/* Get Gamma commnd to device */

		((VDGammaRecord **)(&cParam.cntrlParam.csParam[0]))[0] = &DeviceGammaRec;	/* record for gamma */
		if (!PBStatusSync(&cParam)) {		/* get gamma table */

			/* I got the gamma table */
			/* Now, let's parse it and get the data from it */
			
			GammaTablePtr = (GammaTblPtr)DeviceGammaRec.csGTable;	// pull table out of record
			if (GammaTablePtr) {
				Word DataWidth;
				
				/* Sanity checks */
				
				DataWidth = GammaTablePtr->gDataWidth;		/* Get the bit size */
				
				if (DataWidth && DataWidth<=8) {			/* Acceptable bit size 1-8 is acceptable */
					Word EntryCount;
					EntryCount = GammaTablePtr->gDataCnt;	/* Number of entries */
					if (EntryCount && EntryCount<=256) {	/* Sane count? 1-256 is ok */
						Word Channels;
						Channels = GammaTablePtr->gChanCnt;
						if (Channels==1 || Channels==3) {	/* Mono or RGB are the only acceptable ones */
							Ptr csPtr;
							Word i;
							Word8 *DestPtr;
							float Factor;

							NewGammaTablePtr = (GammaTblPtr)(&TempGammaTable[0]);
							FastMemCpy(NewGammaTablePtr,GammaTablePtr,(sizeof(GammaTbl)-2)+GammaTablePtr->gFormulaSize);
							
							/* At this point, I have a valid input structure */
							/* I'm going to copy it, make changes and then post the changes */
							/* to the Video driver to adjust the gamma */
							

							DestPtr = (Word8 *)(&NewGammaTablePtr->gFormulaData[NewGammaTablePtr->gFormulaSize]);

							/* Get the RGB byte to float conversion */
							Factor = (float)((1<<DataWidth)-1);

							/* Convert a monochrome Gamma ramp to RGB */

							i = 0;
							if (Channels==1) {
								float Factor2;
								Factor2 = Factor*(1.0f/3.0f);
								do {
									float FTemp;
									FTemp = TablePtr->Red[i]+TablePtr->Green[i]+TablePtr->Blue[i];
									FTemp = FTemp*Factor2;		/* Get the average */
									if (FTemp<0) {				/* Clip the numbers */
										FTemp = 0;
									}
									if (FTemp>Factor) {
										FTemp = Factor;
									}
									DestPtr[i] = (Word8)FTemp;
									++DestPtr;
								} while (++i<EntryCount);
							} else {
							/* Convert a RGB ramp to float RGB */
							
								Word8 *DestPtr2;
								Word8 *DestPtr3;
								DestPtr2 = DestPtr+EntryCount;
								DestPtr3 = DestPtr2+EntryCount;
								do {
									float FTemp2;
									FTemp2 = TablePtr->Red[i]*Factor;		/* Get the value */
									if (FTemp2<0) {				/* Clip the numbers */
										FTemp2 = 0;
									}
									if (FTemp2>Factor) {
										FTemp2 = Factor;
									}
									DestPtr[0] = (Word8)FTemp2;
									
									FTemp2 = TablePtr->Green[i]*Factor;		/* Get the value */
									if (FTemp2<0) {				/* Clip the numbers */
										FTemp2 = 0;
									}
									if (FTemp2>Factor) {
										FTemp2 = Factor;
									}
									DestPtr2[0] = (Word8)FTemp2;
									
									FTemp2 = TablePtr->Blue[i]*Factor;		/* Get the value */
									if (FTemp2<0) {				/* Clip the numbers */
										FTemp2 = 0;
									}
									if (FTemp2>Factor) {
										FTemp2 = Factor;
									}
									DestPtr3[0] = (Word8)FTemp2;
									
									++DestPtr;
									++DestPtr2;
									++DestPtr3;
								} while (++i<EntryCount);
							}

							/* Call the driver and pass in the new gamma table */
							
							DeviceGammaRec.csGTable = (Ptr) NewGammaTablePtr;				// setup restore record
							csPtr = (Ptr) &DeviceGammaRec;
							if (!Control(MyDevHand[0]->gdRefNum,cscSetGamma, (Ptr)&csPtr)) {	// restore gamma
								GDPtr TempDevPtr;
								PixMapPtr PicPtr;
								
								FastMemCpy(&LocalPtr->GammaCurrentTable,TablePtr,sizeof(LocalPtr->GammaCurrentTable));
								TempDevPtr = MyDevHand[0];
								PicPtr = TempDevPtr->gdPMap[0];
								if (PicPtr->pixelSize == 8) {		/* if successful and on an 8 bit device */
									CTabPtr ColorPtr;
									VDSetEntryRecord setEntriesRec;
									
									ColorPtr = PicPtr->pmTable[0];			// do SetEntries to force CLUT update
									setEntriesRec.csTable = &ColorPtr->ctTable[0];
									setEntriesRec.csStart = 0;
									setEntriesRec.csCount = ColorPtr->ctSize;
									csPtr = (Ptr) &setEntriesRec;
									Control(TempDevPtr->gdRefNum, cscSetEntries, (Ptr) &csPtr);	// SetEntries in CLUT
								}
								return FALSE;
							}
						}
					}
				}
			}
		}
	}
	return TRUE;		/* Error! */
}

/**********************************

	Switchable functions for CARBON only

**********************************/

#if TARGET_API_MAC_CARBON
static Word MacOSGetGammaTable(GrMacLocals_t *LocalPtr)
{
	CGTableCount sampleCount;

	if (MacOSGetOSVersion()<0x1000) {
		return MacOSGetGammaTableClassic(LocalPtr);
	}

	/* MacOSX version */
	
	FastMemSet(&LocalPtr->GammaOriginalTable,0,sizeof(LocalPtr->GammaOriginalTable));
	if (!CGGetDisplayTransferByTable(0,256,LocalPtr->GammaOriginalTable.Red, LocalPtr->GammaOriginalTable.Green,LocalPtr->GammaOriginalTable.Blue, &sampleCount)) {
		FastMemCpy(&LocalPtr->GammaCurrentTable,&LocalPtr->GammaOriginalTable,sizeof(LocalPtr->GammaOriginalTable));
		return FALSE;
	}
	return TRUE;
}

static Word MacOSSetGammaTable(GrMacLocals_t *LocalPtr,const GammaTable_t *TablePtr)
{
	if (MacOSGetOSVersion()<0x1000) {
		return MacOSSetGammaTableClassic(LocalPtr,TablePtr);
	}

	/* MacOSX version */
		
	if (!CGSetDisplayTransferByTable(0,256,TablePtr->Red,TablePtr->Green,TablePtr->Blue)) {
		FastMemCpy(&LocalPtr->GammaCurrentTable,TablePtr,sizeof(LocalPtr->GammaCurrentTable));
		return FALSE;
	}
	return TRUE;
}

#else
#define MacOSGetGammaTable MacOSGetGammaTableClassic
#define MacOSSetGammaTable MacOSSetGammaTableClassic
#endif


/**********************************

	Init the gamma manager

**********************************/

void BURGERCALL VideoOSGammaInit(void)
{
	GrMacLocals_t *LocalPtr;

	LocalPtr = &Locals;	
	if (!LocalPtr->GammaOk) {				/* Was it initialized? */
		if (!MacOSGetGammaTable(LocalPtr)) {	/* Load the current gamma */
			LocalPtr->GammaOk = TRUE;			/* I'm initialized */
		}
	}
}

/**********************************

	Shut down the gamma manager

**********************************/

void BURGERCALL VideoOSGammaDestroy(void)
{
	GrMacLocals_t *LocalPtr;

	LocalPtr = &Locals;	
	if (LocalPtr->GammaOk) {			/* Was the gamma manager enabled? */
		VideoOSGammaSet(&LocalPtr->GammaOriginalTable);		/* Restore the gamma to the start value */
		LocalPtr->GammaOk = FALSE;		/* Disable the gamma manager */
	}
}

/**********************************

	Darken or brighten the gamma using (Fixed32) 1.0 as the base intensity

**********************************/

void BURGERCALL VideoOSGammaAdjust(Fixed32 Intensity)
{
	float Factor;
	Word i;
	GrMacLocals_t *LocalPtr;

	LocalPtr = &Locals;	
	if (LocalPtr->GammaOk) {			/* Only do it if Gamma control is enabled */
	
		/* Sanity checks */
		
		if (Intensity<0) {
			Intensity = 0;
		}
		if (Intensity>=0x80000) {
			Intensity = 0x80000;
		}
		Factor = (float)Intensity*(1.0f/65536.0f);
		i = 0;
		do {
			LocalPtr->GammaCurrentTable.Red[i] = LocalPtr->GammaOriginalTable.Red[i]*Factor;
			LocalPtr->GammaCurrentTable.Green[i] =  LocalPtr->GammaOriginalTable.Green[i]*Factor;
			LocalPtr->GammaCurrentTable.Blue[i] =  LocalPtr->GammaOriginalTable.Blue[i]*Factor;
		} while (++i<256);
		VideoOSGammaSet(&LocalPtr->GammaCurrentTable);		/* Set the gamma factor */
	}
}

/**********************************

	Set the gamma based on specific gamma entries

**********************************/

void BURGERCALL VideoOSGammaSet(const GammaTable_t *TablePtr)
{
	GrMacLocals_t *LocalPtr;

	LocalPtr = &Locals;
	if (TablePtr && LocalPtr->GammaOk) {			/* Can I work? */
		if (!MacOSSetGammaTable(LocalPtr,TablePtr)) {	/* Set the gamma */
			FastMemCpy(&LocalPtr->GammaCurrentTable,TablePtr,sizeof(GammaTable_t));	/* Make sure my table is updated */
		}
	}
}

/**********************************

	Return the current gamma table last uploaded to the video card.

**********************************/

void BURGERCALL VideoOSGammaGet(GammaTable_t *TablePtr)
{
	GrMacLocals_t *LocalPtr;

	LocalPtr = &Locals;
	if (TablePtr) {
		if (LocalPtr->GammaOk) {		/* Did I init the Gamma manager? */
			FastMemCpy(TablePtr,&LocalPtr->GammaCurrentTable,sizeof(GammaTable_t));
		} else {
			FastMemSet(TablePtr,0,sizeof(GammaTable_t));		/* Zap the table */
		}
	}
}

#endif

