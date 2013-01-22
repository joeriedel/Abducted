/**********************************

	Burgerlib Palette Manager
	Copyright Bill Heineman
	All rights reserved.
	Written by Bill Heineman

**********************************/

#include "PlPalette.h"

/**********************************

	These are all specific to MS-DOS
	I assume you are using Watcom 11.0 to compile
	the MS-DOS version of Burgerlib

**********************************/

#if defined(__MSDOS__)
#include <conio.h>
#include "MsDos.h"
#include "GrGraphics.h"
#include "ClStdlib.h"

/**********************************

	Set the border color value

**********************************/

void DOSPaletteSetBorderColor(Word Color);

#pragma aux DOSPaletteSetBorderColor = \
	"PUSH EBP" \
	"MOV BH,AL" \
	"MOV EAX,01001H" \
	"INT 010H" \
	"POP EBP" \
	parm [eax] \
	modify [ebx]

void PaletteSetBorderColor(Word Color)
{
	DOSPaletteSetBorderColor(Color);		/* Set the border color */
}

/**********************************

	Return the border color value

**********************************/

Word DOSPaletteGetBorderColor(void);

#pragma aux DOSPaletteGetBorderColor = \
	"PUSH EBP" \
	"MOV EAX,01008H" \
	"INT 010H" \
	"XOR EAX,EAX" \
	"POP EBP" \
	"MOV AL,BH" \
	value [eax] \
	modify [ebx]

Word BURGERCALL PaletteGetBorderColor(void)
{
	return DOSPaletteGetBorderColor();		/* Call VGA driver to get border color */
}

/**********************************

	Using a pointer to a palette, set a specific range of colors
	This routine only works if the video is set
	to 8 bit color mode.

**********************************/

void CallBackPal(Word,Word,void *,Word8 *);

#pragma aux CallBackPal = \
	"PUSH	ES" \
	"PUSH	DS" \
	"XOR	EAX,EAX" \
	"MOV	AX,[ESI+4]" \
	"ADD	ESI,EAX" \
	"MOV	EBX,[PaletteVSync]" \
	"MOV	EAX,04F09H" \
	"OR		EBX,EBX" \
	"JZ		NoSync" \
	"MOV	EBX,128" \
"NoSync:" \
	"MOV	ES,[_x32_zero_base_selector]" \
	"MOV	DS,[_x32_zero_base_selector]" \
	"CALL	ESI" \
	"POP	DS" \
	"POP	ES" \
	parm [edx] [ecx] [edi] [esi] \
	modify [eax ebx ecx edx esi edi]

void BURGERCALL PaletteSetPtr(Word Start,Word Count,const Word8 *PalPtr)
{
	if (!PalPtr || Start>=256) {	/* Bad? */
		return;
	}
	if ((Start+Count)>=257) {		/* Out of range? */
		Count = 256-Start;			/* Set the limit on the count */
	}
	if (!Count /* || !memcmp(&CurrentPalette[Start*3],PalPtr,Count*3) */ ) {		/* No data to transfer? */
		return;
	}

	FastMemCpy(&CurrentPalette[Start*3],PalPtr,Count*3);	/* Update the buffer */
	if (PaletteChangedProc) {
		PaletteChangedProc();
	}
	if (VideoColorDepth!=8) {	/* Palette mode? */
		return;
	}

	/* Update using the VESA call if possible */

	if (BurgerVesaVersion>=2) {			/* Vesa driver 2.0? */
		{
			Word8 *DestPtr;
			Word i;

			DestPtr = (Word8 *)GetRealBufferProtectedPtr();	/* Buffer pointer */
			i = Count;
			if (Burger8BitPalette) {		/* Use as is? */
				do {
					DestPtr[0] = PalPtr[2];	/* Convert to 8 bit data */
					DestPtr[1] = PalPtr[1];	/* Convert to 8 bit data */
					DestPtr[2] = PalPtr[0];	/* Convert to 8 bit data */
					DestPtr[3] = 0;			/* Alpha channel */
					PalPtr=PalPtr+3;
					DestPtr+=4;
				} while (--i);
			} else {
				do {
					DestPtr[0] = PalPtr[2]>>2;	/* Convert to 6 bit data */
					DestPtr[1] = PalPtr[1]>>2;	/* Convert to 6 bit data */
					DestPtr[2] = PalPtr[0]>>2;	/* Convert to 6 bit data */
					DestPtr[3] = 0;			/* Alpha channel */
					PalPtr=PalPtr+3;
					DestPtr+=4;
				} while (--i);
			}
		}
		if (BurgerVideoCallbackBuffer) {		/* Direct call to protected code? */
			CallBackPal(Start,Count,(Word8 *)GetRealBufferProtectedPtr()-(Word32)ZeroBase,BurgerVideoCallbackBuffer);
			return;
		}
	}
		/* Call Vesa via Int 10 */

	{
		Word32 RealPtr;			/* Real mode memory pointer */
		Regs16 MyRegs;

		RealPtr = GetRealBufferPtr();
		MyRegs.ax = 0x4F09;			/* Get/Set palette data */
		if (PaletteVSync) {
			MyRegs.bx = 0x80;		/* VSync */
		} else {
			MyRegs.bx = 0;		/* Set the palette */
		}
		MyRegs.cx = static_cast<Word16>(Count);		/* Register count */
		MyRegs.dx = static_cast<Word16>(Start);		/* Register start */
		MyRegs.di = static_cast<Word16>(RealPtr);
		MyRegs.es = static_cast<Word16>(RealPtr>>16);
		if (Int86x(0x10,&MyRegs,&MyRegs) == 0x004F) {	/* Call the VESA driver */
			return;			/* Exit now */
		}
	}
	PalPtr = PalPtr-(Count*3);		/* Reset the pointer to the beginning */

	/* Ok, I'll do it the old fashioned VGA way */
	/* I'll send it directly to the hardware */

	Count = Count*3;
	outp(0x3C8,Start);			/* Set the start address */
	do {
		outp(0x3C9,((Word8 *)PalPtr)[0]>>2);		/* Convert to 6 bit and write */
		PalPtr = (Word8 *)PalPtr+1;	/* Next source */
	} while (--Count);				/* All done? */
}

#endif

