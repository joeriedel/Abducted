/**********************************

	MS-DOS Specific code for Burgerlib graphics
	Note : I assume that I am using the Watcom 11.0
	compiler for MS-DOS products

**********************************/

#include "GrGraphics.h"

#if defined(__MSDOS__)
#include "MsDos.h"
#include "ClStdLib.h"
#include "MmMemory.h"

typedef struct {		/* Vesa 2.0 info structure */
	/* 1.0 data */
	Word16 ModeAttributes;
	Word8 WinAAttributes;
	Word8 WinBAttributes;
	Word16 WinGranularity;
	Word16 WinSize;
	Word16 WinASegment;
	Word16 WinBSegment;
	void (*WinFuncPtr)(void);
	Word16 BytesPerScanLine;
	/* 1.2 data */
	Word16 XResolution;
	Word16 YResolution;
	Word8 XCharSize;
	Word8 YCharSize;
	Word8 NumberOfPlanes;
	Word8 BitsPerPixel;
	Word8 NumberOfBanks;
	Word8 MemoryModel;
	Word8 BankSize;
	Word8 NumberOfImagePages;
	Word8 Reserved;
	/* Direct color */
	Word8 RedMaskSize;
	Word8 RedFieldPosition;
	Word8 GreenMaskSize;
	Word8 GreenFieldPosition;
	Word8 BlueMaskSize;
	Word8 BlueFieldPosition;
	Word8 RsvdMaskSize;
	Word8 RsvdFieldPosition;
	Word8 DirectColorModeInfo;
	/* 2.0 data */
	Word8 *PhysBasePtr;
	Word32 OffScreenMemOffset;
	Word16 OffScreenMemSize;
	Word8 Padding[206];
} VesaInfo_t;

typedef struct {
	Word32 VbeSignature;	/* 'VESA' */
	Word16 VbeVersion;		/* Vesa version code */
	Word16 OemStringOffset;	/* Real mode pointer to driver name */
	Word16 OemStringSegment;
	Word16 Capabilities;		/* Capabilities flags */
	Word16 Capabilities2;
	Word16 VideoListOffset;	/* Pointer to mode list */
	Word16 VideoListSegment;
	Word16 TotalMemory;		/* Total memory available */
} VbeInfoBlock_t;

/**********************************

	Set 80x25 text mode

**********************************/

extern void FooMCGAOff(void);

#pragma aux FooMCGAOff = \
	"PUSH EBP" \
	"MOV EAX,3" \
	"INT 010H" \
	"POP EBP" \
	modify [EAX]

void BURGERCALL MCGAOff(void)
{
	FooMCGAOff();
}

/**********************************

	Turn on MCGA (320*200*256 color mode)
	This is an MS-DOS only (and obsolete) function

**********************************/

extern void FooMCGAOn(void);

#pragma aux FooMCGAOn = \
	"PUSH EBP" \
	"MOV EAX,013H" \
	"INT 010H" \
	"POP EBP" \
	modify [EAX]

void BURGERCALL MCGAOn(void)
{
	FooMCGAOn();		/* Call the bios */
	VideoWidth = 320;	/* Set my variables */
	ScreenWidth = 320;
	ScreenClipRight = 320;
	ScreenHeight = 200;
	ScreenClipBottom = 200;
	VideoPointer = &ZeroBase[0xA0000];
	ScreenClipLeft = 0;
	ScreenClipTop = 0;
}

/**********************************

	Set up the current video page
	for drawing, lock down the pixels
	and set the burgerlib video port

**********************************/

void BURGERCALL LockFrontVideoPage(void)
{
	if (VideoOffscreen) {		/* Am I using an offscreen bitmap? */
		VideoPointer = VideoOffscreen;		/* Use this */
	} else {
		VideoPointer = BurgerVideoPointers[BurgerVideoPage];	/* Hardware? */
	}
	VideoWidth = VideoHardWidth;	/* Set the video width */
}

/**********************************

	Set up the current video page
	for drawing, lock down the pixels
	and set the burgerlib video port

**********************************/

void BURGERCALL LockVideoPage(void)
{
	if (VideoOffscreen) {		/* Am I using an offscreen bitmap? */
		VideoPointer = VideoOffscreen;		/* Use this */
	} else {
		VideoPointer = BurgerVideoPointers[BurgerVideoPage];	/* Hardware? */
	}
	VideoWidth = VideoHardWidth;	/* Set the video width */
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

extern void FooUpdateAndNoPageFlip(Word32 Address,Word8 *CallBuff);

#pragma aux FooUpdateAndNoPageFlip = \
	"PUSH ES" \
	"PUSH DS" \
	"XOR EAX,EAX" \
	"MOV AX,[ESI+2]" \
	"ADD ESI,EAX" \
	"MOV EAX,04F07H" \
	"MOV BL,0" \
	"MOV EDX,ECX" \
	"SHR EDX,16" \
	"AND ECX,0FFFFH" \
	"MOV DS,[_x32_zero_base_selector]" \
	"CALL ESI" \
	"POP DS" \
	"POP ES" \
	parm [ecx] [esi] \
	modify [eax ebx ecx edx esi edi]

void BURGERCALL UpdateAndNoPageFlip(void)
{
	Regs16 MyRegs;

	if (VideoOffscreen) {
		if (BurgerLinearFrameBuffer) {
			FastMemCpy(BurgerVideoPointers[BurgerVideoPage],VideoOffscreen,BurgerScreenSize);
		} else {
			UpdateSVGA(VideoOffscreen);
		}
		return;
	}
	if (BurgerVideoCallbackBuffer) {
		FooUpdateAndNoPageFlip((BurgerVideoPage*BurgerScreenSize)>>2,BurgerVideoCallbackBuffer);
	} else {
		MyRegs.ax = 0x4F07;
		MyRegs.bx = 0x0000;
		MyRegs.cx = 0;			/* X coord */
		MyRegs.dx = static_cast<Word16>(BurgerVideoPage*VideoTrueScreenHeight);	/* Y coord */
		Int86x(0x10,&MyRegs,&MyRegs);
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
	if (!VideoOffscreen) {
		++BurgerVideoPage;			/* Use the next video page */
		if (BurgerVideoPage>=BurgerMaxVideoPage) {
			BurgerVideoPage = 0;
		}
	}
}

/**********************************

	Release everything about the graphics system
	and go back to the ORIGINAL video mode

**********************************/

void BURGERCALL ReleaseVideo(void)
{
	DeallocAPointer(VideoOffscreen);	/* Release it */
	VideoOffscreen = 0;	/* Make sure it's gone */

	/* VESA 1.2 stuff */

	DeallocAPointer(BurgerVideoCallbackBuffer);	/* Was there callback code? */
	BurgerVideoCallbackBuffer = 0;	/* Goodbye! */
	ScreenWidth = 0;		/* Force default */
	ScreenHeight = 0;
}

/**********************************

	Given a request list, I will try to set the video
	display hardware into a mode that will show the
	graphics as requested.
	I allow 8,15 and 16 bit color.
	I allow page flipping (Triple buffering maximum)
	and I also allow offscreen drawing for maximum throughput

	If the function fails, the machine will be left in an
	unknown state. Please save the current state of the video
	and restore the state on exit.
	Uses VESA 1.0 through 2.0 for high resolution contexts

	I will set up these variables if this call is successful

	Burger8BitPalette = TRUE if 8 bit palette hardware is detected
		and initialized properly
	BurgerVesaVersion = 0 = No vesa driver, 1 = 1.2, 2 = 2.0 or higher
	BurgerMaxVideoPage = 1-3 pages
	BurgerVideoCallbackBuffer = Pointer to VESA callbacks (If found)
	VideoOffscreen = Pointer to offscreen buffer (If mode enabled)
	BurgerScreenSize = Size in bytes of the offscreen buffer
	VideoColorDepth = Depth of the current color map (8,15,16)
	VideoTrueScreenWidth = Width of the screen in PIXELS
	VideoTrueScreenHeight = Height of the screen in PIXELS
	VideoHardWidth = Width of a scan line in BYTES
	BurgerLinearFrameBuffer = TRUE if linear mode is on

**********************************/

Word BURGERCALL SetDisplayToSize(Word Width,Word Height,Word Depth,Word Flags)
{
	Word i;
	Regs16 MyRegs;		/* Used for all the VESA calls */
	Word Mode;			/* Video display mode to use */
	Word32 RealPtr;	/* Real pointer to VESA buffer */
	Word8 *RealBuffer;	/* Protected pointer to VESA buffer */
	Word32 Offset;
	VbeInfoBlock_t *VbePtr;

	ReleaseVideo();		/* Release any and all vide buffers */

	if (Depth!=8 && Depth!=15 && Depth!=16) {	/* Bad input? */
		NonFatal("Color depth is not supported\n");
		return TRUE;
	}

	RealPtr = GetRealBufferPtr();	/* I have to get a real mode buffer */
	RealBuffer = static_cast<Word8 *>(GetRealBufferProtectedPtr());	/* Protected pointer */

	/* Let me detect a VESA 2.0 video driver */
	/* and get the display list */

	Burger8BitPalette = FALSE;	/* Assume 6 bit palettes */
	BurgerMaxVideoPage = 3;		/* Assume page flipping */
	VideoTrueScreenWidth = Width;
	VideoTrueScreenHeight = Height;
	VideoColorDepth = Depth;
	VideoTrueScreenDepth = Depth;
	BurgerScreenSize = ((Depth+7)>>3)*Width*(Word32)Height;	/* Size of a video buffer */

	VbePtr = (VbeInfoBlock_t *)(RealBuffer+1024);
	FastMemSet(VbePtr,0,512);	/* Init the buffer in advance */
	MyRegs.ax = 0x4F00;			/* Detect a vesa driver */
	MyRegs.di = (Word16)RealPtr+1024;
	MyRegs.es = (Word16)(RealPtr>>16);
	VbePtr->VbeSignature = '2EBV';	/* VBE2 reversed */
	if (Int86x(0x10,&MyRegs,&MyRegs)==0x004F &&
		VbePtr->VbeSignature=='ASEV') {		/* Success? */

			/* I have a VESA driver, now what version is it? */
		Word Version;
		Word16 *ListPtr;

		Version = VbePtr->VbeVersion;
		if (Version<0x102) {		/* I must have at least 1.2 */
			goto NotVesa;
		}
		BurgerVesaVersion = 1;		/* 1.2 driver */
		if (Version>=0x200) {
			BurgerVesaVersion = 2;	/* It's 2.0 */
			/* Do I have 8 bit palettes? (Only in 2.0) */
			if (VbePtr->Capabilities & 1) {
				Burger8BitPalette = TRUE;	/* I use 8 bit colors */
			}
		}

	/* Now I will determine if I have the video mode in question */

		Offset = (((Word32)VbePtr->VideoListSegment)<<4)+VbePtr->VideoListOffset;
		if (!Offset) {		/* Valid pointer? */
			NonFatal("The VESA driver doesn't have a mode list\n");
			return TRUE;
		}
		ListPtr = (Word16 *)(&ZeroBase[Offset]);
		for (;;) {
			if (ListPtr[0]==(Word16)-1) {	/* End of the list? */
				if (Depth==8 && Width==320 && Height==200) {
					Mode = 0x13;		/* Special case this... */
					break;
				}
				NonFatal("Requested video mode is not in the VESA list\n");
				return TRUE;
			}
			Mode = ListPtr[0];
			MyRegs.ax = 0x4F01;		/* Get info */
			MyRegs.cx = static_cast<Word16>(Mode);	/* Get mode */
			MyRegs.di = static_cast<Word16>(RealPtr);
			MyRegs.es = static_cast<Word16>(RealPtr>>16);
			if (Int86x(0x10,&MyRegs,&MyRegs)==0x004F) {
				if ((Width==((VesaInfo_t *)RealBuffer)->XResolution) &&
					(Height==((VesaInfo_t *)RealBuffer)->YResolution) &&
					(Depth==((VesaInfo_t *)RealBuffer)->BitsPerPixel) &&
					(((VesaInfo_t *)RealBuffer)->NumberOfPlanes==1)) {
					break;
				}
			}
			++ListPtr;
		}

	/* Now the moment of truth, let's actually start the video mode */

		if (BurgerVesaVersion>=2) {	/* Try linear frame buffer */
			MyRegs.ax = 0x4F02;		/* Init the video mode */
			MyRegs.bx = static_cast<Word16>(Mode|0x4000);	/* Linear frame buffer */
			if (Int86x(0x10,&MyRegs,&MyRegs)==0x004f) {	/* Do it! */
				BurgerLinearFrameBuffer = TRUE;	/* Frame buffer present! */
				goto VesaSuccess;
			}
		}
		MyRegs.ax = 0x4F02;		/* Init the video mode */
		MyRegs.bx = static_cast<Word16>(Mode);		/* Save the mode # */
		if (Int86x(0x10,&MyRegs,&MyRegs)!=0x004f) {	/* Paged video */
			NonFatal("Can't init VESA video display mode\n");
			return TRUE;
		}
		if (BurgerScreenSize>65536) {	/* Does it fit in 1 bank? */
			Flags |= SETDISPLAYOFFSCREEN;	/* Non-linear */
			BurgerMaxVideoPage = 1;		/* Only 1 page will be used */
		}

VesaSuccess:
		if (Burger8BitPalette) {	/* Can I use an 8 bit palette? */
			MyRegs.ax = 0x4F08;
			MyRegs.bx = 0x0800;		/* 8 bits per pixel */
			if (Int86x(0x10,&MyRegs,&MyRegs)!=0x004F) {
				Burger8BitPalette = FALSE;	/* Assume 6 bits */
			}
		}

		/* Now the video mode is initialized */
		/* Set up the pointers to the memory and the maximum */
		/* number of video frames the memory will allow */

		MyRegs.ax = 0x4F01;		/* Get mode info */
		MyRegs.cx = static_cast<Word16>(Mode);	/* Mode initialized */
		MyRegs.di = (Word16)RealPtr;		/* My real mode memory */
		MyRegs.es = (Word16)(RealPtr>>16);
		if (Int86x(0x10,&MyRegs,&MyRegs)!=0x004F) {		/* Should NEVER fail! */
			NonFatal("Can't get information on VESA video mode\n");
			return TRUE;		/* Shit!! */
		}
		VideoHardWidth = ((VesaInfo_t *)RealBuffer)->BytesPerScanLine;
		if (BurgerLinearFrameBuffer) {		/* Linear? */
			BurgerBaseVideoPointer = static_cast<Word8*>(MapPhysicalAddress(((VesaInfo_t *)RealBuffer)->PhysBasePtr,BurgerScreenSize*BurgerMaxVideoPage));
		} else {
			BurgerBaseVideoPointer = &ZeroBase[((Word32)((VesaInfo_t *)RealBuffer)->WinASegment)<<4];
		}

		/* Since a VESA driver was not found, All I can support */
		/* is 320x200x8 mode */

	} else {
NotVesa:
		if (Width!=320 || Height!=200 || Depth!=8) {
			NonFatal("No VESA driver present, only 320x200 supported\n");
			return TRUE;
		}
		MyRegs.ax = 0x13;		/* MCGA mode */
		Int86x(0x10,&MyRegs,&MyRegs);	/* Enable it */
		if (MyRegs.flags&1) {		/* Error? */
			NonFatal("Can't initialize mode 0x13\n");
			return TRUE;
		}
		BurgerBaseVideoPointer = &ZeroBase[0xA0000];	/* Base pointer */
		BurgerMaxVideoPage = 1;	/* Only 1 video page allowed */
		VideoHardWidth = 320;	/* 320 bytes per line */
		BurgerLinearFrameBuffer = TRUE;
		Flags |= SETDISPLAYOFFSCREEN;		/* Force offscreen buffer */
	}

	/* Should I use an offscreen buffer anyways?? */

	if (Flags&SETDISPLAYOFFSCREEN) {
		VideoOffscreen = (Word8 *)AllocAPointer(BurgerScreenSize);
		if (!VideoOffscreen) {	/* Can't get the offscreen memory! */
			return TRUE;		/* AllocAPointer() already reported the error */
		}
	}

	/* Init pointers to HARDWARE video memory */
	/* Non-Linear frame buffers only have 1 page */

	i = 0;
	do {
		BurgerVideoPointers[i] = BurgerBaseVideoPointer+(BurgerScreenSize*i);
	} while (++i<BurgerMaxVideoPage);

	/* Now, we check if we can get the protected mode code */
	/* Only available in Vesa 2.0 */

	if (BurgerVesaVersion>=2) {
		MyRegs.ax = 0x4F0A;
		MyRegs.bx = 0;		/* Return the table */
		if (Int86x(0x10,&MyRegs,&MyRegs)==0x004F) {		/* Success? */
			Word BuffSize;
			/* Allocate a buffer and copy the code */
			BuffSize = MyRegs.cx;
			if (BuffSize) {	/* Data present? */
				BurgerVideoCallbackBuffer = (Word8 *)AllocAPointer(BuffSize);
				if (BurgerVideoCallbackBuffer) {	/* Got the memory? */
					Word32 Offset;
					Offset = (((Word32)MyRegs.es)<<4)+MyRegs.di;
					FastMemCpy(BurgerVideoCallbackBuffer,&ZeroBase[Offset],BuffSize);
				}
			}
		}
	}

	/* I am finally done!! */
	/* Set user globals and get out! */

	ScreenWidth = Width;		/* Set my globals */
	ScreenHeight = Height;
	ScreenClipTop = 0;
	ScreenClipLeft = 0;
	ScreenClipRight = Width;
	ScreenClipBottom = Height;
	return FALSE;
}

/**********************************

	If a window is present, set the text to a specific string

**********************************/

void BURGERCALL VideoSetWindowString(const char * /* Title */)
{
}

/**********************************

	Return a handle to an array of video display modes

**********************************/

VideoModeArray_t ** BURGERCALL VideoModeArrayNew(Word /* HardwareOnly */)
{
	return 0;		/* Return NOTHING */
}

/**********************************

	Return a handle to an array of video modes

**********************************/

VideoDeviceArray_t ** BURGERCALL VideoDeviceArrayNew(Word /* HardwareOnly */)
{
	return 0;		/* Return NOTHING */
}

#endif
