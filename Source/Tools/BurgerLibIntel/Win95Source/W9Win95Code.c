#include <BRTypes.h>

#if defined(__WIN32__)
#include "W9Win95.h"
#include "MmMemory.h"
#include "ImImage.h"
#include "GrGraphics.h"
#include "ClStdLib.h"
#include "LrRect.h"
#include "SsScreenShape.h"
#include "FmFile.h"
#define WIN32_LEAN_AND_MEAN
#define DIRECTDRAW_VERSION 0x700
#define DIRECT3D_VERSION 0x700
#include <windows.h>
#include <shlobj.h>
#include "../Win95Extern/DirectX/d3d.h"

/**********************************

	Windows 95 specific mouse variables

**********************************/

Word Win95MouseButton;		/* Mouse button contents */
Word Win95MouseX;		/* Virtual mouse X */
Word Win95MouseY;		/* Virtual mouse Y */
int Win95MouseWheel;	/* Virtual mouse Z */
int Win95MouseXDelta;	/* Mouse motion */
int Win95MouseYDelta;
int Win95LastMouseX;	/* Previous mouse position for delta motion */
int Win95LastMouseY;
int Win95DestScreenX;	/* X coord to draw screen to */
int Win95DestScreenY;	/* Y coord to draw screen to */
void *Win95MainWindow;	/* Main window to perform all operations on */
struct IDirectDrawSurface *Win95FrontBuffer;	/* Currently displayed screen */
struct IDirectDrawSurface *Win95BackBuffer;	/* My work buffer */
struct IDirectDrawSurface *Win95WorkBuffer;	/* Target buffer */
struct IDirectDrawSurface *Direct3DZBufferPtr;		/* ZBuffer for 3D */
struct IDirectDrawPalette *Win95WindowPalettePtr;	/* Pointer to game palette */
struct IDirectDraw *DirectDrawPtr;				/* Pointer to direct draw buffer */
struct IDirectDraw2 *DirectDraw2Ptr;			/* Pointer to the direct draw 2 instance */
struct IDirectDraw4 *DirectDraw4Ptr;			/* Pointer to the direct draw 4 instance */
struct IDirectDrawClipper *Win95WindowClipperPtr;	/* Clipper for primary surface */
Word Direct3DZBufferBitDepth;					/* Bits per pixel for D3D ZBuffer */
Word Win95ApplsActive = TRUE;	/* True if the application is active */
void *Win95Instance;	/* Current instance */
Word8 *Win95VideoPointer;	/* Current video pointer */

/**********************************

	Table used for Win95 Direct sound
	conversion

**********************************/

short Win95DirectSoundVolumes[256] = {
	-10000, -8000, -7000, -6415, -6000, -5678, -5415, -5192,
	 -5000, -4830, -4678, -4540, -4415, -4299, -4192, -4093,
	 -4000, -3912, -3830, -3752, -3678, -3607, -3540, -3476,
	 -3415, -3356, -3299, -3245, -3192, -3142, -3093, -3045,
	 -3000, -2955, -2912, -2870, -2830, -2790, -2752, -2714,
	 -2678, -2642, -2607, -2573, -2540, -2508, -2476, -2445,
	 -2415, -2385, -2356, -2327, -2299, -2272, -2245, -2218,
	 -2192, -2167, -2142, -2117, -2093, -2069, -2045, -2022,
	 -2000, -1977, -1955, -1933, -1912, -1891, -1870, -1850,
	 -1830, -1810, -1790, -1771, -1752, -1733, -1714, -1696,
	 -1678, -1660, -1642, -1624, -1607, -1590, -1573, -1557,
	 -1540, -1524, -1508, -1492, -1476, -1460, -1445, -1430,
	 -1415, -1400, -1385, -1370, -1356, -1341, -1327, -1313,
	 -1299, -1285, -1272, -1258, -1245, -1231, -1218, -1205,
	 -1192, -1179, -1167, -1154, -1142, -1129, -1117, -1105,
	 -1093, -1081, -1069, -1057, -1045, -1034, -1022, -1011,
	 -1000,  -988,  -977,  -966,  -955,  -944,  -933,  -923,
	  -912,  -901,  -891,  -881,  -870,  -860,  -850,  -840,
	  -830,  -820,  -810,  -800,  -790,  -780,  -771,  -761,
	  -752,  -742,  -733,  -723,  -714,  -705,  -696,  -687,
	  -678,  -669,  -660,  -651,  -642,  -633,  -624,  -616,
	  -607,  -599,  -590,  -582,  -573,  -565,  -557,  -548,
	  -540,  -532,  -524,  -516,  -508,  -500,  -492,  -484,
	  -476,  -468,  -460,  -453,  -445,  -437,  -430,  -422,
	  -415,  -407,  -400,  -392,  -385,  -377,  -370,  -363,
	  -356,  -348,  -341,  -334,  -327,  -320,  -313,  -306,
	  -299,  -292,  -285,  -278,  -272,  -265,  -258,  -251,
	  -245,  -238,  -231,  -225,  -218,  -212,  -205,  -199,
	  -192,  -186,  -179,  -173,  -167,  -160,  -154,  -148,
	  -142,  -135,  -129,  -123,  -117,  -111,  -105,   -99,
	   -93,   -87,   -81,   -75,   -69,   -63,   -57,   -51,
	   -45,   -39,   -34,   -28,   -22,   -17,   -11,     0
};

/**********************************

	Initialize the base window for Windows 95

**********************************/

Word BURGERCALL InitWin95Window(const char *Title,void *Instance,long (__stdcall *Proc)(struct HWND__*,Word,Word,long))
{
	WNDCLASS wc;
	void *MyWindow;

	Win95Instance = Instance;	/* Save the current instance */
	wc.style = CS_DBLCLKS;		/* Accept double clicks */
	wc.lpfnWndProc = Proc;	/* My window callback */
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = (HINSTANCE)Instance;
	wc.hIcon = 0;
	wc.hCursor = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = "WinGameClass";

	if (!RegisterClass(&wc)) {
		NonFatal("Can't register my game class");
		return TRUE;
	}

	MyWindow = CreateWindowEx(
		WS_EX_APPWINDOW,	/* Force top level to the task bar when minimized */
		"WinGameClass",		/* Pointer to registered class name */
		Title,				/* Window title string */
		WS_VISIBLE |	/* Window is visible initially */
		WS_POPUP,		/* Make it a pop-up window */
		0,			/* X coord */
		0,			/* Y coord */
		GetSystemMetrics(SM_CXSCREEN),	/* Width */
		GetSystemMetrics(SM_CYSCREEN),	/* Height */
		0,		/* Window parent */
		0,		/* Window menu */
		(HINSTANCE)Instance,	/* Task number */
		0);			/* Local parameter */

	if (!MyWindow) {
		NonFatal("Can't allocate my window");
		return TRUE;
	}
	Win95MainWindow = MyWindow;
	UpdateWindow((HWND)MyWindow);		/* Blank out the window to black */
	SetFocus((HWND)MyWindow);			/* All input is directed to me */
	return FALSE;			/* I'm OK! */
}

/**********************************

	Adds a directory to the Win95 start folder.
	Returns TRUE if an error occurs

**********************************/

Word BURGERCALL Win95AddGroupToProgramMenu(const char *GroupName)
{
	LPITEMIDLIST pidlStartMenu;		/* Item list for the start menu */
	char szPath[MAX_PATH];

// get the pidl for the start menu
// this will be used to intialize the folder browser

	if (SHGetSpecialFolderLocation((HWND)Win95MainWindow,CSIDL_PROGRAMS,&pidlStartMenu)==NOERROR) {
		if (SHGetPathFromIDList(pidlStartMenu,szPath)) {
			Win95AppendFilenameToDirectory(szPath, GroupName);
			if (!CreateDirectoryPathNative(szPath)) {		/* Create the directory */
/* notify the shell that you made a change */
				SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH,szPath, 0);
			}
			return FALSE;	/* I did it! */
		}
	}
	return TRUE;
}

static HANDLE InstanceLock;		/* Handle the global named file */
static Word Once;				/* atexit init routine */

/**********************************

	Release proc on shutdown

**********************************/

static void ANSICALL UnlockMe(void)
{
	if (InstanceLock) {
		CloseHandle(InstanceLock);
		InstanceLock = 0;			/* Only do it once */
	}
}

/**********************************

	Here's the good stuff. I create a file but it
	stays in memory. Now if I was to execute again,
	I'll see that the file exists and that will
	tell me to go away.

**********************************/

Word BURGERCALL Win95Allow1Instance(const char *LockName)
{
	InstanceLock=CreateFileMapping((HANDLE)0xFFFFFFFF,0,PAGE_READONLY,0,32,LockName);

	if (InstanceLock && (GetLastError()!=ERROR_ALREADY_EXISTS)) {	/* Create and not already present? */
		if (!Once) {			/* Did I create the atexit proc? */
			atexit(UnlockMe);	/* For shutdown */
			Once = TRUE;
		}
		return FALSE;			/* No error */
	}
	return TRUE;		/* Error! Either the file couldn't be made or I exist */
}

/**********************************

	Append a filename to the end of
	an MS-DOS/Win95 compatible path

	I will add a DOUBLE null at the end of
	the string since "SHFile" operations need this
	in case of wide strings

**********************************/

void BURGERCALL Win95AppendFilenameToDirectory(char *Directory,const char *Filename)
{
	Word Temp;
	Word Temp2;

	Temp = strlen(Directory);
	if (Temp) {		/* Is there a length? */
		Temp2 = Directory[Temp-1];
		if (Temp2 != '\\' && Temp2 != '/') {
			Directory[Temp] = '\\';
			++Temp;
		}
	}
	Directory+=Temp;		/* Point to the buffer */
	Temp2 = strlen(Filename)+1;	/* Length of the filename string */
	Directory[Temp2] = 0;		/* Add another zero 1 byte beyond the end */
	FastMemCpy(Directory,Filename,Temp2);	/* Copy the name */
}

/**********************************

	Dispose of a Direct3D Texture

**********************************/

void BURGERCALL Direct3DTextureDestroy(Direct3DTexture_t *Input)
{
	if (Input->Surface) {
		IDirectDrawSurface_Release(Input->Surface);
		Input->Surface = 0;
	}
	if (Input->TexturePtr) {
		IDirect3DTexture_Release(Input->TexturePtr);
		Input->TexturePtr = 0;
	}
	Input->TextureHandle = 0;
}

/**********************************

	Dispose of a Direct3D Texture

**********************************/

void BURGERCALL Direct3DTextureDelete(Direct3DTexture_t *Input)
{
	if (Input) {
		Direct3DTextureDestroy(Input);
		DeallocAPointer(Input);
	}
}

/**********************************

	Create a direct 3D texture
	and return the freshly allocated pointer

**********************************/

Direct3DTexture_t * BURGERCALL Direct3DTextureNewImage(const Image_t *ImagePtr)
{
	Direct3DTexture_t *Output;

	Output = (Direct3DTexture_t *)AllocAPointer(sizeof(Direct3DTexture_t));
	if (Output) {
		if (!Direct3DTextureInitImage(Output,ImagePtr)) {		/* Init the structure */
			return Output;
		}
		DeallocAPointer(Output);
	}
	return 0;			/* Oh crap! */
}

/**********************************

	Create a direct 3D texture

**********************************/

Word BURGERCALL Direct3DTextureInitImage(Direct3DTexture_t *Output,const Image_t *ImagePtr)
{
	DDSURFACEDESC ddsd;
	IDirectDrawSurface *MySurfPtr;
	IDirectDrawSurface *TempSurfPtr;
	Word WantType;
	Word Result;
	Word TexWidth,TexHeight;

	TexWidth = ImagePtr->Width;			/* Get the proposed texture size */
	TexHeight = ImagePtr->Height;

	if (VideoTextureRules&VIDEOTEXTUREPOW2) {		/* Do I require a power of 2 size? */
		TexWidth = PowerOf2(TexWidth);
		TexHeight = PowerOf2(TexHeight);
	}
	if (VideoTextureRules&VIDEOTEXTURESQUARE) {
		if (TexWidth!=TexHeight) {
			if (TexWidth>TexHeight) {
				TexHeight = TexWidth;
			} else {
				TexWidth = TexHeight;
			}
		}
	}
	if (TexWidth>VideoTextureMaxWidth) {			/* Is there a texture size maximum */
		TexWidth = VideoTextureMaxWidth;
	}
	if (TexHeight>VideoTextureMaxHeight) {
		TexHeight = VideoTextureMaxHeight;
	}
	if (TexWidth<VideoTextureMinWidth) {
		TexWidth = VideoTextureMinWidth;
	}
	if (TexHeight<VideoTextureMinHeight) {
		TexHeight = VideoTextureMinHeight;
	}

	/* First thing's first, try to match up the texture type to the proper texture format */
	/* that I can use for the current mode */

	Result = TRUE;			/* I assume I will fail */
	FastMemSet(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT);
	ddsd.ddsCaps.dwCaps = (DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY );
	ddsd.dwWidth = TexWidth;
	ddsd.dwHeight = TexHeight;
	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

	if (ImagePtr->DataType==IMAGE8_PAL && (VideoTextureTypesAllowed&VIDEOTEXTURETYPE8PAL)) {
		ddsd.ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8|DDPF_RGB;
		ddsd.ddpfPixelFormat.dwRGBBitCount = 8;
		WantType = IMAGE8_PAL;				/* I'll save it as an 8 bit image */
	} else {
		ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;		/* My target is true color */
		ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
		ddsd.ddpfPixelFormat.dwBBitMask = 0x1F;
		if (VideoTextureTypesAllowed&VIDEOTEXTURETYPE555) {
			ddsd.ddpfPixelFormat.dwRBitMask = 0x1F<<10;
			ddsd.ddpfPixelFormat.dwGBitMask = 0x1F<<5;
			WantType = IMAGE555;
		} else if (VideoTextureTypesAllowed&VIDEOTEXTURETYPE565) {
			ddsd.ddpfPixelFormat.dwRBitMask = 0x1F<<11;
			ddsd.ddpfPixelFormat.dwGBitMask = 0x3f<<5;
			WantType = IMAGE565;
		} else {
			Output->TexturePtr = 0;
			Output->Surface = 0;
			Output->TextureHandle = 0;
			return TRUE;		/* You are screwed! */
		}
	}

	/* Ok, now I know what format I'll ask Direct3D to create the surface, let's create it */

	if (IDirectDraw_CreateSurface(DirectDrawPtr,&ddsd,&TempSurfPtr,0)==DD_OK) {
		DDSURFACEDESC ddsdback;
		IDirect3DTexture *TempTexPtr;

		/* Now for the fun part */
		/* DirectDraw can only handle surfaces that are the SAME format as the live video screen */
		/* So I have to create a TEMP surface, load the data into the temp and then have the */
		/* Direct3DDeviceLoad() command actually transfer the pixels to a surface I can use */
		/* This is so bas-ackwards that I am still fuming at how much time I wasted trying to figure */
		/* this stuff out. */

		TempSurfPtr->QueryInterface(IID_IDirect3DTexture,(void **)&TempTexPtr);		/* Get the D3D texture type */
		FastMemSet(&ddsd,0,sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		IDirectDrawSurface_GetSurfaceDesc(TempSurfPtr,&ddsd);						/* Get the description from the surface */
		ddsd.dwFlags = (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT);
		ddsd.ddsCaps.dwCaps = (DDSCAPS_TEXTURE | DDSCAPS_ALLOCONLOAD);					/* ALLOCONLOAD is magic DO NOT REMOVE!! */
		IDirectDraw_CreateSurface(DirectDrawPtr,&ddsd,&MySurfPtr,0);						/* Create the surface for the final product */

		/* Is this an 8 bit image for the target? */

		if (WantType==IMAGE8_PAL) {
			Word Index;
			Word8 *SrcPtr;
			PALETTEENTRY *DestPtr;
			PALETTEENTRY TempPal[256];		/* 256 colors for the surface */
			IDirectDrawPalette *FooPal;		/* Temp palette */

			Index = 256;
			SrcPtr = ImagePtr->PalettePtr;
			DestPtr = TempPal;
			do {
				DestPtr->peRed = SrcPtr[0];
				DestPtr->peGreen = SrcPtr[1];
				DestPtr->peBlue = SrcPtr[2];
				DestPtr->peFlags = 0;
				SrcPtr+=3;
				DestPtr++;
			} while (--Index);
			if (IDirectDraw_CreatePalette(DirectDrawPtr,DDPCAPS_8BIT|DDPCAPS_ALLOW256,TempPal,&FooPal,0)==DD_OK) {
				IDirectDrawSurface_SetPalette(TempSurfPtr,FooPal);		/* Set the palette for my input */
				IDirectDrawSurface_SetPalette(MySurfPtr,FooPal);		/* Set the palette for my output */
			}
		}

		/* Now, let's place the pixels in the temp surface */

		FastMemSet(&ddsdback,0,sizeof(ddsdback));
		ddsdback.dwSize = sizeof(ddsdback);
		if (IDirectDrawSurface_Lock(TempSurfPtr,0,&ddsdback,DDLOCK_WAIT,0)==DD_OK) {		/* Lock it */
			Image_t DestImage;
			IDirect3DTexture *DestTexPtr;
			Word8 FakePal[768];				/* Just a waste */

			FastMemSet(&DestImage,0,sizeof(DestImage));		/* Describe the destination surface */
			DestImage.Width = TexWidth;				/* Size of the texture */
			DestImage.Height = TexHeight;
			DestImage.DataType = (ImageTypes_e)WantType;
			DestImage.ImagePtr = (Word8 *)ddsdback.lpSurface;
			DestImage.RowBytes = ddsdback.lPitch;
			if (WantType==IMAGE8_PAL) {
				DestImage.PalettePtr = FakePal;				/* Phony palette */
			}
			ImageStore(&DestImage,ImagePtr);				/* Transfer the pixels (It can scale) */
			IDirectDrawSurface_Unlock(TempSurfPtr,ddsdback.lpSurface);	/* Release the surface */

		/* Now, I have the pixels */

			MySurfPtr->QueryInterface(IID_IDirect3DTexture,(void **)&DestTexPtr);
			if (IDirect3DTexture_Load(DestTexPtr,TempTexPtr)==DD_OK) {		/* Actually make the FINAL texture for Direct3D */
				if (IDirect3DTexture_GetHandle(DestTexPtr,Direct3DDevicePtr,&Output->TextureHandle)==DD_OK) {		/* Get the reference */
					Output->Surface = MySurfPtr;
					Output->TexturePtr = DestTexPtr;
					IDirect3DTexture_Release(TempTexPtr);			/* Release the temp texture */
					IDirectDrawSurface_Release(TempSurfPtr);		/* Release the temp surface */
					return FALSE;
				}
			}
			IDirect3DTexture_Release(DestTexPtr);		/* Release the texture reference */
		}
		IDirect3DTexture_Release(TempTexPtr);			/* Release the temp texture */
		IDirectDrawSurface_Release(TempSurfPtr);		/* Release the temp surface */
		IDirectDrawSurface_Release(MySurfPtr);		/* Get rid of the final since I failed! */
	}
	Output->TexturePtr = 0;
	Output->Surface = 0;
	Output->TextureHandle = 0;
	return TRUE;			/* Oh crud! */
}

/**********************************

	Draw a Direct3D Texture as a 2D Image

**********************************/

void Direct3DTextureDraw2D(Direct3DTexture_t *Input,int x,int y,Word Width,Word Height)
{
	Direct3DCheckExecBuffer(32,4);
	{
		D3DTLVERTEX *vertexPtr;

		vertexPtr = &((D3DTLVERTEX *)Direct3DExecDataBuf)[VideoVertexCount];
		vertexPtr[0].sx = (float)x;
		vertexPtr[0].sy = (float)y;
		vertexPtr[0].sz = 0;
		vertexPtr[0].tu = 0;
		vertexPtr[0].tv = 0;
		vertexPtr[0].rhw = 1.0f;
		vertexPtr[0].color = 0xffffffff;
		vertexPtr[0].specular = 0xFF000000;

		vertexPtr[1].sx = (float)(x+Width);
		vertexPtr[1].sy = vertexPtr[0].sy;
		vertexPtr[1].sz = 0;
		vertexPtr[1].tu = 1;
		vertexPtr[1].tv = 0;
		vertexPtr[1].rhw = 1.0f;
		vertexPtr[1].color = 0xffffffff;
		vertexPtr[1].specular = 0xFF000000;

		vertexPtr[2].sx = vertexPtr[1].sx;
		vertexPtr[2].sy = (float)(y+Height);
		vertexPtr[2].sz = 0;
		vertexPtr[2].tu = 1;
		vertexPtr[2].tv = 1;
		vertexPtr[2].rhw = 1.0f;
		vertexPtr[2].color = 0xffffffff;
		vertexPtr[2].specular = 0xFF000000;

		vertexPtr[3].sx = vertexPtr[0].sx;
		vertexPtr[3].sy = vertexPtr[2].sy;
		vertexPtr[3].sz = 0;
		vertexPtr[3].tu = 0;
		vertexPtr[3].tv = 1;
		vertexPtr[3].rhw = 1.0f;
		vertexPtr[3].color = 0xffffffff;
		vertexPtr[3].specular = 0xFF000000;
	}
	{
		Word8 *WorkPtr;
		Word i;
		WorkPtr = Direct3DExecInstPtr;
		if (ScreenCurrentTexture!=Input->TextureHandle) {
			ScreenCurrentTexture = Input->TextureHandle;
			STORE_OP_RENDERSTATE_CONST(1,WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_TEXTUREHANDLE,Input->TextureHandle,WorkPtr);
		}
		i = VideoVertexCount;
		STORE_OP_TRIANGLE_CONST(2,WorkPtr);
		STORE_DATA_TRIANGLE(i,i+1,i+3,WorkPtr);
		STORE_DATA_TRIANGLE(i+1,i+2,i+3,WorkPtr);
		Direct3DExecInstPtr = WorkPtr;
		VideoVertexCount = i+4;
	}
}

/**********************************

	Draw a Direct3D Texture as a 2D Image

**********************************/

void Direct3DTextureDraw2DSubRect(Direct3DTexture_t *Input,int x,int y,Word Width,Word Height,const float *UVPtr)
{
	Direct3DCheckExecBuffer(32,4);
	{
		D3DTLVERTEX *vertexPtr;

		vertexPtr = &((D3DTLVERTEX *)Direct3DExecDataBuf)[VideoVertexCount];
		vertexPtr[0].sx = (float)x;
		vertexPtr[0].sy = (float)y;
		vertexPtr[0].sz = 0;
		vertexPtr[0].tu = UVPtr[0];
		vertexPtr[0].tv = UVPtr[1];
		vertexPtr[0].rhw = 1.0f;
		vertexPtr[0].color = 0xffffffff;
		vertexPtr[0].specular = 0xFF000000;

		vertexPtr[1].sx = (float)(x+Width);
		vertexPtr[1].sy = vertexPtr[0].sy;
		vertexPtr[1].sz = 0;
		vertexPtr[1].tu = UVPtr[2];
		vertexPtr[1].tv = UVPtr[1];
		vertexPtr[1].rhw = 1.0f;
		vertexPtr[1].color = 0xffffffff;
		vertexPtr[1].specular = 0xFF000000;

		vertexPtr[2].sx = vertexPtr[1].sx;
		vertexPtr[2].sy = (float)(y+Height);
		vertexPtr[2].sz = 0;
		vertexPtr[2].tu = UVPtr[2];
		vertexPtr[2].tv = UVPtr[3];
		vertexPtr[2].rhw = 1.0f;
		vertexPtr[2].color = 0xffffffff;
		vertexPtr[2].specular = 0xFF000000;

		vertexPtr[3].sx = vertexPtr[0].sx;
		vertexPtr[3].sy = vertexPtr[2].sy;
		vertexPtr[3].sz = 0;
		vertexPtr[3].tu = UVPtr[0];
		vertexPtr[3].tv = UVPtr[3];
		vertexPtr[3].rhw = 1.0f;
		vertexPtr[3].color = 0xffffffff;
		vertexPtr[3].specular = 0xFF000000;
	}
	{
		Word8 *WorkPtr;
		Word i;
		WorkPtr = Direct3DExecInstPtr;
		if (ScreenCurrentTexture!=Input->TextureHandle) {
			ScreenCurrentTexture = Input->TextureHandle;
			STORE_OP_RENDERSTATE_CONST(1,WorkPtr);
			STORE_DATA_STATE(D3DRENDERSTATE_TEXTUREHANDLE,Input->TextureHandle,WorkPtr);
		}
		i = VideoVertexCount;
		STORE_OP_TRIANGLE_CONST(2,WorkPtr);
		STORE_DATA_TRIANGLE(i,i+1,i+3,WorkPtr);
		STORE_DATA_TRIANGLE(i+1,i+2,i+3,WorkPtr);
		Direct3DExecInstPtr = WorkPtr;
		VideoVertexCount = i+4;
	}
}

#endif
