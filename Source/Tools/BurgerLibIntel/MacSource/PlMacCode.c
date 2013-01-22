/**********************************

	Burgerlib Palette Manager
	Copyright Bill Heineman
	All rights reserved.
	Written by Bill Heineman

**********************************/

#include "PlPalette.h"

/**********************************

	Mac OS version

	Note : Due to quirks of MacOS, you CANNOT use color #0,
	it MUST always be white.
	So if you wish to do any palette tricks of any kind make sure you know
	this flaw in advance.

**********************************/

#if defined(__MAC__)
#include "McMac.h"
#include "GrGraphics.h"
#include "ClStdlib.h"
#include <drawsprocket.h>

/**********************************

	Using a pointer to a palette, set a specific range of colors
	This routine only works if the video is set
	to 8 bit color mode.

**********************************/

void PaletteSetPtr(Word Start,Word Count,const Word8 *PalPtr)
{
	CTabHandle ColorHandle;		/* Handle to the main palette */
	Word Temp;					/* Temp */
	Word EndColor;

	if (!PalPtr || Start>=256) {	/* Bad? */
		return;
	}
	if ((Start+Count)>=257) {		/* Out of range? */
		Count = 256-Start;			/* Set the limit on the count */
	}

	EndColor = 256;

#if TARGET_RT_MAC_CFM && !_DEBUG
	if (!MacContext) {		/* Is DrawSprocket active? */
#endif
		if (!Start) {	/* Start at color #0? */
			--Count;	/* Reduce the count */
			if (!Count) {
				return;
			}
			++Start;	/* Start here instead */
			PalPtr = PalPtr+3;	/* Skip the first triplet */
		}
		EndColor = 255;
#if TARGET_RT_MAC_CFM && !_DEBUG
	}
#endif

	if (Start>=EndColor) {		/* Invalid start? */
		return;
	}
	Temp = Start+Count;		/* Final color number */
	if (Temp>EndColor) {			/* Could it write to color #255? */
		Count = EndColor-Start;	/* Prevent a tragedy!! */
	}
	if (Count /* && memcmp(CurrentPalette+(Start*3),PalPtr,Count*3) */) {
		FastMemCpy(CurrentPalette+(Start*3),PalPtr,Count*3);	/* Update the full palette */
		if (PaletteChangedProc) {
			PaletteChangedProc();
		}

	/* Here is where the fun begins. On the mac, if full screen */
	/* I will update drawsprocket to set the hardware palette */

		if (VideoColorDepth==8) {		/* Do I even use a palette? */

			/* Drawsprocket palette support */

	#if defined(__POWERPC__)
			Word TheSeed;				/* This is the palette seed, I need this for a hack */
			if (MacContext) {		/* Is DrawSprocket active? */
				ColorSpec *Colors;
				ColorSpec *Colors2;
				ColorTable *ColorPtr;
				CGrafPtr MyPort;

				DSpContext_GetBackBuffer(MacContext,kDSpBufferKind_Normal,&MyPort);
				ColorPtr = GetPortPixMap(MyPort)[0]->pmTable[0];
				Colors = ColorPtr->ctTable;
				Colors2 = &Colors[Start];
				Temp = Count;
				do {
					Word Temp2;
					Temp2 = PalPtr[0];
					Colors2->rgb.red = (Temp2<<8)+Temp2;		/* Set the new colors */
					Temp2 = PalPtr[1];
					Colors2->rgb.green = (Temp2<<8)+Temp2;
					Temp2 = PalPtr[2];
					Colors2->rgb.blue = (Temp2<<8)+Temp2;
					PalPtr = PalPtr+3;		/* Next triplett */
					++Colors2;							/* Next referance */
				} while (--Temp);
				BurgerPaletteDirty = TRUE;			/* I will update the palette next video update */
				DSpContext_SetCLUTEntries(MacContext,Colors,Start,Count-1); /* Set the colors */
				TheSeed = ColorPtr->ctSeed;			/* I must aquire the color table seed here!! */
				PalPtr = PalPtr-(Count*3);			/* Reset the pointer */
			}
	#endif

			/* Normal MacOS way */
			if (VideoGWorld) {			/* Is it ok? */
#if !TARGET_API_MAC_CARBON
				ColorHandle = VideoGWorld->portPixMap[0]->pmTable;		/* Get handle to local palette */
#else
				ColorHandle = GetGWorldPixMap(VideoGWorld)[0]->pmTable;		/* Get handle to local palette */
#endif
				if (ColorHandle) {			/* Valid palette? */
					ColorTable *ColorPtr;	/* Pointer to color palette */
					ColorSpec *Colors;		/* Pointer to color array */

					ColorPtr = ColorHandle[0];		/* Lock it down */

					Colors = &ColorPtr->ctTable[Start];		/* Get pointer to color table */
					Temp = Count;
					do {			/* Fill in all the color entries */
						Word Temp2;
						Temp2 = PalPtr[0];
						Colors->rgb.red = (Temp2<<8)+Temp2;
						Temp2 = PalPtr[1];
						Colors->rgb.green = (Temp2<<8)+Temp2;
						Temp2 = PalPtr[2];		/* Blue component */
//						if (!Temp2) {
//							Temp2 = 1;			/* Don't use black (#255 is exclusivly for black!) */
//						}
						Colors->rgb.blue = (Temp2<<8)+Temp2;
						PalPtr = PalPtr+3;		/* Next triplett */
						++Colors;				/* Next referance */
					} while (--Temp);			/* All done? */

					/* The palette seed MUST be the same for DrawSprocket */
					/* or hardware assist is slowed down as a palette compare must be */
					/* performed to check for hardware optimization */
					/* This gains about 5% speed by getting rid of the compare in MacOS */
					/* calls to CopyBits() */

	#if defined(__POWERPC__)
					if (MacContext) {
						ColorPtr->ctSeed = TheSeed;		/* Save the DrawSprocket palette seed */
					} else
	#endif
					{
						SetGDevice(VideoDevice);			/* Set to the work device */
						SetEntries(Start,Count-1,&ColorPtr->ctTable[Start]);	/* Set the color entries */
					}
				}
			}
		}
	}
}

#endif
