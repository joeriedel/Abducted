/**********************************

	Burgerlib Palette Manager
	Copyright 1995-1999 Bill Heineman
	All rights reserved.
	Written by Bill Heineman

**********************************/

#include "PlPalette.h"

/**********************************

	This is the Windows 95/98/NT version

**********************************/

#if defined(__WIN32__)

/**********************************

	Using a pointer to a palette, set a specific range of colors
	This routine only works if the video is set
	to 8 bit color mode.

**********************************/

#define WIN32_LEAN_AND_MEAN
#define DIRECTDRAW_VERSION 0x700
#include <windows.h>
#include <ddraw.h>
#include "W9Win95.h"
#include "GrGraphics.h"
#include "ClStdlib.h"

#define FIRSTENTRY 1
#define LASTENTRY 255

void BURGERCALL PaletteSetPtr(Word Start,Word Count,const Word8 *PalPtr)
{
	Word Temp;

	if (!PalPtr || Start>=256) {	/* Bad? */
		return;
	}
	if ((Start+Count)>=257) {		/* Out of range? */
		Count = 256-Start;			/* Set the limit on the count */
	}
	if (!Count) {		/* No data to transfer? */
		return;
	}

	if (Start<FIRSTENTRY) {		/* Start at color #0? */
		Temp = FIRSTENTRY-Start;	/* How many to skip */
		if (Count<=Temp) {
			return;
		}
		Count -= Temp;
		Start=FIRSTENTRY;		/* Start here instead */
		PalPtr = PalPtr+(Temp*3);		/* Skip the first triplet */
	}
	if (Start>=LASTENTRY) {		/* Invalid start? */
		return;
	}
	Temp = Start+Count;
	if (Temp>LASTENTRY) {		/* Could it write to color #255? */
		Count = LASTENTRY-Start;	/* Prevent a tragedy! */
	}
	if (Count /* && memcmp(CurrentPalette+(Start*3),PalPtr,Count*3) */ ) {
		FastMemCpy(CurrentPalette+(Start*3),PalPtr,Count*3);	/* Update the full palette */
		if (PaletteChangedProc) {
			PaletteChangedProc();
		}
		if (Win95WindowPalettePtr || BurgerHPalette) {		/* Is direct draw active?? */
			PALETTEENTRY WinPal[256];
			PALETTEENTRY *WPalPtr;
			Temp = Count;
			WPalPtr = WinPal;
			do {
				WPalPtr->peRed = PalPtr[0];
				WPalPtr->peGreen = PalPtr[1];
				WPalPtr->peBlue = PalPtr[2];
				WPalPtr->peFlags = 0;
				PalPtr = PalPtr+3;
				++WPalPtr;
			} while (--Temp);
			Temp = VideoPageLocked;		/* Save the lock state */
			if (Temp) {
				UnlockVideoPage();		/* Release video if locked */
			}

		/* Now set Direct Draw's palette */

			if (Win95WindowPalettePtr) {
				if (IDirectDrawPalette_SetEntries((LPDIRECTDRAWPALETTE)Win95WindowPalettePtr,0,Start,Count,WinPal)==DDERR_SURFACELOST) {
					IDirectDrawSurface_Restore(Win95FrontBuffer);
					IDirectDrawPalette_SetEntries((LPDIRECTDRAWPALETTE)Win95WindowPalettePtr,0,Start,Count,WinPal);
				}
			}

		/* If GDI is present, set it too. */

			if (BurgerHPalette) {
				HDC FooDC;
				SetPaletteEntries((HPALETTE)BurgerHPalette,Start,Count,WinPal);
				FooDC = GetDC((HWND)Win95MainWindow);
				if (FooDC) {
					if (BurgerHPalette) {
						SelectPalette(FooDC,(HPALETTE)BurgerHPalette,FALSE);		/* Select this palette */
					}
					RealizePalette(FooDC);		/* Map the palette */

					if (BurgerHBitMap) {		/* Is there a bitmap? */
						HDC MyDC;
						Word Temp;

						MyDC = CreateCompatibleDC(FooDC);	/* Get a bitmap */
						if (MyDC) {
							Temp = Count;		/* Convert PALETTEENTRIES to RGBQUADS */
							WPalPtr = WinPal;
							do {
								Word8 Temp2;
								Temp2 = WPalPtr->peRed;
								WPalPtr->peRed = WPalPtr->peBlue;
								WPalPtr->peBlue = Temp2;
								++WPalPtr;
							} while (--Temp);
							SelectObject(MyDC,BurgerHBitMap);		/* Select the bitmap */
							SetDIBColorTable(MyDC,Start,Count,(RGBQUAD *)WinPal);	/* Set the palette */
							DeleteDC(MyDC);			/* bye bye! */
						}
					}
					ReleaseDC((HWND)Win95MainWindow,FooDC);
				}
				if (Win95MainWindow) {
					InvalidateRect((HWND)Win95MainWindow,0,FALSE);
				}
			}
			if (Temp) {
				LockVideoPage();		/* Unlock video buffer */
			}
		}
	}
}
#endif

