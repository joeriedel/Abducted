/**********************************

	Burgerlib Palette Manager
	Copyright Bill Heineman
	All rights reserved.
	Written by Bill Heineman

**********************************/

#include "PlPalette.h"
#include "TkTick.h"
#include "RzRez.h"
#include "ClStdLib.h"
#include "MmMemory.h"
#include "InInput.h"

/**********************************

	Header:
		Set to TRUE for a deferred palette update.

	Synopsis:
		Set to TRUE if the palette was modified but the actual hardware
		update is deferred to vertical blank. If this is set, you must
		assume that CurrentPalette will be read and sent to the hardware
		at a later time.

	Also:
		PaletteSetPtr()

**********************************/

Word BurgerPaletteDirty;		/* Set to TRUE for a deferred palette update */

/**********************************

	Header:
		Copy of the palette currently in the hardware.

	Synopsis:
		This is a copy of the current palette being held by
		the video display hardware. If the system is in a true color
		mode, this will contain the current palette for the simulated
		8 bit context for conversion of 8 bit graphics to a true color
		display.

		This table can only be modified by calls to PaletteSetPtr()
		You must NEVER modify this table directly unless you use it as input
		for a call to PaletteSetPtr().

		Treat this table as Read-only for all other purposes.

		There are 256 colors, each entry is 3 bytes with the order
		being Red, Green and Blue. The values are 0-255 with 0 being off
		and 255 being full intensity

	Also:
		PaletteSetPtr()

**********************************/

Word8 CurrentPalette[768];		/* Current palette being used by hardware */

/**********************************

	Header:
		Table of squares from -255 to 255

	Synopsis:
		Used for some routines to create a square to quickly
		generate a color match. Can be used by other routines
		for their own purposes

	Also:
		PaletteFindColorIndex()

**********************************/

const Word ByteSquareTable[255+256] = {
	65025,64516,64009,63504,63001,62500,62001,61504,	/* -255 to -248 */
	61009,60516,60025,59536,59049,58564,58081,57600,	/* -247 to -240 */
	57121,56644,56169,55696,55225,54756,54289,53824,	/* -239 to -232 */
	53361,52900,52441,51984,51529,51076,50625,50176,	/* -231 to -224 */
	49729,49284,48841,48400,47961,47524,47089,46656,	/* -223 to -216 */
	46225,45796,45369,44944,44521,44100,43681,43264,	/* -215 to -208 */
	42849,42436,42025,41616,41209,40804,40401,40000,	/* -207 to -200 */
	39601,39204,38809,38416,38025,37636,37249,36864,	/* -199 to -192 */
	36481,36100,35721,35344,34969,34596,34225,33856,	/* -191 to -184 */
	33489,33124,32761,32400,32041,31684,31329,30976,	/* -183 to -176 */
	30625,30276,29929,29584,29241,28900,28561,28224,	/* -175 to -168 */
	27889,27556,27225,26896,26569,26244,25921,25600,	/* -167 to -160 */
	25281,24964,24649,24336,24025,23716,23409,23104,	/* -159 to -152 */
	22801,22500,22201,21904,21609,21316,21025,20736,	/* -151 to -144 */
	20449,20164,19881,19600,19321,19044,18769,18496,	/* -143 to -136 */
	18225,17956,17689,17424,17161,16900,16641,16384,	/* -135 to -128 */
	16129,15876,15625,15376,15129,14884,14641,14400,	/* -127 to -120 */
	14161,13924,13689,13456,13225,12996,12769,12544,	/* -119 to -112 */
	12321,12100,11881,11664,11449,11236,11025,10816,	/* -111 to -104 */
	10609,10404,10201,10000,9801,9604,9409,9216,	/* -103 to -96 */
	9025,8836,8649,8464,8281,8100,7921,7744,	/* -95 to -88 */
	7569,7396,7225,7056,6889,6724,6561,6400,	/* -87 to -80 */
	6241,6084,5929,5776,5625,5476,5329,5184,	/* -79 to -72 */
	5041,4900,4761,4624,4489,4356,4225,4096,	/* -71 to -64 */
	3969,3844,3721,3600,3481,3364,3249,3136,	/* -63 to -56 */
	3025,2916,2809,2704,2601,2500,2401,2304,	/* -55 to -48 */
	2209,2116,2025,1936,1849,1764,1681,1600,	/* -47 to -40 */
	1521,1444,1369,1296,1225,1156,1089,1024,	/* -39 to -32 */
	961,900,841,784,729,676,625,576,	/* -31 to -24 */
	529,484,441,400,361,324,289,256,	/* -23 to -16 */
	225,196,169,144,121,100,81,64,	/* -15 to -8 */
	49,36,25,16,9,4,1,0,	/* -7 to 0 */
	1,4,9,16,25,36,49,64,	/* 1 to 8 */
	81,100,121,144,169,196,225,256,	/* 9 to 16 */
	289,324,361,400,441,484,529,576,	/* 17 to 24 */
	625,676,729,784,841,900,961,1024,	/* 25 to 32 */
	1089,1156,1225,1296,1369,1444,1521,1600,	/* 33 to 40 */
	1681,1764,1849,1936,2025,2116,2209,2304,	/* 41 to 48 */
	2401,2500,2601,2704,2809,2916,3025,3136,	/* 49 to 56 */
	3249,3364,3481,3600,3721,3844,3969,4096,	/* 57 to 64 */
	4225,4356,4489,4624,4761,4900,5041,5184,	/* 65 to 72 */
	5329,5476,5625,5776,5929,6084,6241,6400,	/* 73 to 80 */
	6561,6724,6889,7056,7225,7396,7569,7744,	/* 81 to 88 */
	7921,8100,8281,8464,8649,8836,9025,9216,	/* 89 to 96 */
	9409,9604,9801,10000,10201,10404,10609,10816,	/* 97 to 104 */
	11025,11236,11449,11664,11881,12100,12321,12544,	/* 105 to 112 */
	12769,12996,13225,13456,13689,13924,14161,14400,	/* 113 to 120 */
	14641,14884,15129,15376,15625,15876,16129,16384,	/* 121 to 128 */
	16641,16900,17161,17424,17689,17956,18225,18496,	/* 129 to 136 */
	18769,19044,19321,19600,19881,20164,20449,20736,	/* 137 to 144 */
	21025,21316,21609,21904,22201,22500,22801,23104,	/* 145 to 152 */
	23409,23716,24025,24336,24649,24964,25281,25600,	/* 153 to 160 */
	25921,26244,26569,26896,27225,27556,27889,28224,	/* 161 to 168 */
	28561,28900,29241,29584,29929,30276,30625,30976,	/* 169 to 176 */
	31329,31684,32041,32400,32761,33124,33489,33856,	/* 177 to 184 */
	34225,34596,34969,35344,35721,36100,36481,36864,	/* 185 to 192 */
	37249,37636,38025,38416,38809,39204,39601,40000,	/* 193 to 200 */
	40401,40804,41209,41616,42025,42436,42849,43264,	/* 201 to 208 */
	43681,44100,44521,44944,45369,45796,46225,46656,	/* 209 to 216 */
	47089,47524,47961,48400,48841,49284,49729,50176,	/* 217 to 224 */
	50625,51076,51529,51984,52441,52900,53361,53824,	/* 225 to 232 */
	54289,54756,55225,55696,56169,56644,57121,57600,	/* 233 to 240 */
	58081,58564,59049,59536,60025,60516,61009,61504,	/* 241 to 248 */
	62001,62500,63001,63504,64009,64516,65025};			/* 249 to 255 */

/**********************************

	Header:
		Table to convert 5 bit color to 8 bit color

	Synopsis:
		Scale 0-31 into 0-255 in a linear fashion.
   		The formula is Result = ((i*0xFFFF)/31)>>8

	Also:
		PaletteConvertRGB15ToRGB24()

**********************************/

const Word8 RGB5ToRGB8Table[32] = {
	0x00,0x08,0x10,0x18,0x21,0x29,0x31,0x39,
	0x42,0x4A,0x52,0x5A,0x63,0x6B,0x73,0x7B,
	0x84,0x8C,0x94,0x9C,0xA5,0xAD,0xB5,0xBD,
	0xC6,0xCE,0xD6,0xDE,0xE7,0xEF,0xF7,0xFF};

/**********************************

	Header:
		Table to convert 6 bit color to 8 bit color

	Synopsis:
		Scale 0-63 into 0-255 in a linear fashion.
   		The formula is Result = ((i*0xFFFF)/63)>>8

	Also:
		PaletteConvertRGB16ToRGB24()

**********************************/

const Word8 RGB6ToRGB8Table[64] = {
	0x00,0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,
	0x20,0x24,0x28,0x2C,0x30,0x34,0x38,0x3C,
	0x41,0x45,0x49,0x4D,0x51,0x55,0x59,0x5D,
	0x61,0x65,0x69,0x6D,0x71,0x75,0x79,0x7D,
	0x82,0x86,0x8A,0x8E,0x92,0x96,0x9A,0x9E,
	0xA2,0xA6,0xAA,0xAE,0xB2,0xB6,0xBA,0xBE,
	0xC3,0xC7,0xCB,0xCF,0xD3,0xD7,0xDB,0xDF,
	0xE3,0xE7,0xEB,0xEF,0xF3,0xF7,0xFB,0xFF};

/**********************************

	Header:
		Speed of palette fade in ticks

	Synopsis:
		PaletteFadeToPtr() uses this variable as input
		to determine the speed at which the fade will
		occur. The value is the number of ticks to wait
		for each of the 16 interations.

		Currently, the value is still correct, but the palette
		fade operates at the screen refresh rate for a more
		smoother fade. The time quantum is still correct.

	Also:
		PaletteFadeToPtr()

**********************************/

Word FadeSpeed = TICKSPERSEC/15;	/* 15 frames per second */

/**********************************

	Header:
		Flag for syncing palette changes to VBlank

	Synopsis:
		PaletteSetPtr() uses this variable as to alert it
		whether to stop the program until VBlank and then
		update the hardware palette or either change the palette
		immediately and return control as soon as possible.

		Defaults to FALSE for no syncing to VBlank. However
		PaletteFadeTo() will change this to TRUE during a palette
		fade and restore it when done. This allows a more pleasing
		looking palette fade.

	Also:
		PaletteFadeToPtr(), PaletteSetPtr()

**********************************/

Word PaletteVSync;			/* Do I wait for video sync on palette update? */

/**********************************

	Header:
		Callback during a palette fade

	Synopsis:
		PaletteFadeToPtr() uses this variable as a callback during
		a palette fade. You can use this to call sound routines
		to fade in or out music at the same rate the palette is fading.

		This input value Pass is 0 for the first palette write
		and incremented up to 16 for the final pass. Only 0 and
		16 are guaranteed to be called. For machines that have
		slow palette updates or slow machines in general, palette
		fades may have to skip a frame or to to make sure
		the fading is constant.

	Also:
		PaletteFadeToPtr()

**********************************/

PaletteFadeCallBackProcPtr PaletteFadeCallBack;

/**********************************

	Header:
		Callback to alert of a palette change

	Synopsis:
		PaletteSetPtr() can be called at any time by many routines.
		You may wish to have some routine that is alerted whenever the
		system palette is modified. This call is made whenever PaletteSetPtr()
		determines that the call actually modifies the hardware palette.

	Also:
		PaletteSetPtr()

**********************************/

PaletteChangedProcPtr PaletteChangedProc;	/* Callback for palette change */


/**********************************

	Color conversion functions

**********************************/

/**********************************

	Header:
		Convert 15 bit color to 24 bit color

	Synopsis:
		Given a 15 bit RGB pixel value, convert it to a
		24 bit RGB triplett

	Input:
		RGBOut = Pointer to the output buffer.
		RGBIn = 15 bit RGB value in 5:5:5 format.

	Also:
		PaletteConvertRGB16ToRGB24(), RGB5ToRGB8Table

**********************************/

void BURGERCALL PaletteConvertRGB15ToRGB24(Word8 *RGBOut,Word RGBIn)
{
	RGBOut[0] = RGB5ToRGB8Table[(RGBIn>>10)&0x1F];	/* Red */
	RGBOut[1] = RGB5ToRGB8Table[(RGBIn>>5)&0x1F];	/* Green */
	RGBOut[2] = RGB5ToRGB8Table[RGBIn&0x1F];		/* Blue */
}

/**********************************

	Header:
		Convert 16 bit color to 24 bit color

	Synopsis:
		Given a 16 bit RGB pixel value, convert it to a
		24 bit RGB triplett

	Input:
		RGBOut = Pointer to the output buffer.
		RGBIn = 16 bit RGB value in 5:6:5 format.

	Also:
		PaletteConvertRGB15ToRGB24(), RGB5ToRGB8Table, RGB6ToRGB8Table

**********************************/

void BURGERCALL PaletteConvertRGB16ToRGB24(Word8 *RGBOut,Word RGBIn)
{
	RGBOut[0] = RGB5ToRGB8Table[(RGBIn>>11)&0x1F];	/* Red */
	RGBOut[1] = RGB6ToRGB8Table[(RGBIn>>5)&0x3F];	/* Green */
	RGBOut[2] = RGB5ToRGB8Table[RGBIn&0x1F];		/* Blue */
}


/**********************************

	Header:
		Convert 24 bit color to 15 bit color

	Synopsis:
		Given a 24 bit RGB triplett, convert it to a
		15 bit RGB pixel value

	Input:
		RGBIn = Pointer to a 24 bit RGB triplett

	Returns:
		24 bit color value as 5:5:5 color

	Also:
		PaletteConvertRGB15ToRGB24(), PaletteConvertRGB24ToRGB16()

**********************************/

Word BURGERCALL PaletteConvertRGB24ToRGB15(const Word8 *RGBIn)
{
	Word Color;
	Color = (RGBIn[0]<<7)&0x7C00;	/* Red */
	Color |= (RGBIn[1]<<2)&0x03E0;	/* Green */
	Color |= (Word)RGBIn[2]>>3;		/* Blue */
	return Color;
}

/**********************************

	Header:
		Convert 24 bit color to 16 bit color

	Synopsis:
		Given a 24 bit RGB triplett, convert it to a
		16 bit RGB pixel value

	Input:
		RGBIn = Pointer to a 24 bit RGB triplett

	Returns:
		24 bit color value as 5:6:5 color

	Also:
		PaletteConvertRGB16ToRGB24(), PaletteConvertRGB24ToRGB15()

**********************************/

Word BURGERCALL PaletteConvertRGB24ToRGB16(const Word8 *RGBIn)
{
	Word Color;
	Color = (RGBIn[0]<<8)&0xF800;	/* Red */
	Color |= (RGBIn[1]<<3)&0x07E0;	/* Green */
	Color |= (Word)RGBIn[2]>>3;		/* Blue */
	return Color;
}

/**********************************

	Header:
		Convert 24 bit color to current depth

	Synopsis:
		Given a 24 bit RGB triplett, convert it to a
		pixel value of the current depth.

		Note: I can only return an 8, 15 or 16 bit quantity.
		Any other value will force a return of 0.

	Input:
		RGBIn = Pointer to a 24 bit RGB triplett
		Depth = 8, 15 or 16

	Returns:
		24 bit color value as 15, 16 or 8 bit color

	Also:
		PaletteConvertRGB24ToRGB16(), PaletteConvertRGB24ToRGB15(),
		PaletteFindColorIndex(), PaletteConvertRGBToDepth()

**********************************/

Word BURGERCALL PaletteConvertRGB24ToDepth(const Word8 *RGBIn,Word Depth)
{
	if (Depth==8) {
		return PaletteFindColorIndex(CurrentPalette+3,RGBIn[0],RGBIn[1],RGBIn[2],254)+1;
	}
	if (Depth==15) {
		Depth = ((Word)RGBIn[0]<<7)&0x7C00;		/* Red */
		Depth += ((Word)RGBIn[1]<<2)&0x03E0;	/* Green */
		Depth += ((Word)RGBIn[2])>>3;		/* Blue */
		return Depth;
	}
	if (Depth==16) {
		Depth = ((Word)RGBIn[0]<<8)&0xF800;
		Depth += ((Word)RGBIn[1]<<3)&0x7E0;
		Depth += ((Word)RGBIn[2])>>3;
		return Depth;
	}
	return ((RGBIn[0]<<16)+(RGBIn[1]<<8)+RGBIn[2]);
}

/**********************************

	Header:
		Convert 24 bit color to current depth

	Synopsis:
		Given a red, green and blue value, convert it to a
		pixel value of the current depth.

		Note: I can only return an 8, 15 or 16 bit quantity.
		Any other value will force a return of 0.

	Input:
		Red = Value of red (0-255)
		Green = Value of green (0-255)
		Blue = Value of blue (0-255)
		Depth = 8, 15 or 16

	Returns:
		24 bit color value as 15, 16 or 8 bit color

	Also:
		PaletteConvertRGB24ToRGB16(), PaletteConvertRGB24ToRGB15(),
		PaletteFindColorIndex(), PaletteConvertRGB24ToDepth()

**********************************/

Word BURGERCALL PaletteConvertRGBToDepth(Word Red,Word Green,Word Blue,Word Depth)
{
	if (Depth==8) {
		return PaletteFindColorIndex(CurrentPalette+3,Red,Green,Blue,254)+1;
	}
	if (Depth==15) {
		Depth = (Red<<7)&0x7C00;		/* Red */
		Depth += (Green<<2)&0x03E0;	/* Green */
		Depth += Blue>>3;		/* Blue */
		return Depth;
	}
	if (Depth==16) {
		Depth = (Red<<8)&0xF800;
		Depth += (Green<<3)&0x7E0;
		Depth += Blue>>3;
		return Depth;
	}
	return ((Red<<16)+(Green<<8)+Blue);
}

/**********************************

	Header:
		Convert 24 bit packed color to current depth

	Synopsis:
		Given a red/green/blue Word32 value, convert it to a
		pixel value of the current depth.

		Note: I can only return an 8, 15 or 16 bit quantity.
		Any other value will force a return of 0.

	Input:
		Color = Value of red ((0-255)<<16) + Green ((0-255)<<8) + Blue (0-255)
		Depth = 8, 15 or 16

	Returns:
		24 bit color value as 15, 16 or 8 bit color

	Also:
		PaletteConvertRGB24ToRGB16(), PaletteConvertRGB24ToRGB15(),
		PaletteFindColorIndex(), PaletteConvertRGB24ToDepth()

**********************************/

Word BURGERCALL PaletteConvertPackedRGBToDepth(Word32 Color,Word Depth)
{
	Word Red,Green,Blue;
	Red = (Color>>16)&0xFF;		/* Extract Red/Green/Blue */
	Green = (Color>>8)&0xFF;
	Blue = Color&0xFF;
	
	if (Depth==8) {
		return PaletteFindColorIndex(CurrentPalette+3,Red,Green,Blue,254)+1;
	}
	if (Depth==15) {
		Depth = (Red<<7)&0x7C00;		/* Red */
		Depth += (Green<<2)&0x03E0;	/* Green */
		Depth += Blue>>3;		/* Blue */
		return Depth;
	}
	if (Depth==16) {
		Depth = (Red<<8)&0xF800;
		Depth += (Green<<3)&0x7E0;
		Depth += Blue>>3;
		return Depth;
	}
	return Color;
}


/**********************************

	Header:
		Convert an 8 bit palette to a true color array

	Synopsis:
		Given an 8 bit palette (256 RGB values)
		create a 16 bit color lookup table for drawing to a screen
		using this palette

	Input:
		Output = Pointer to an array of 256 Words
		Input = Pointer to an array of 256 3 byte RGB tripletts
		Depth = 15 or 16

	Also:
		PaletteMake16BitLookupRez(), PaletteConvertRGB24ToDepth()

**********************************/

void BURGERCALL PaletteMake16BitLookup(Word *Output,const Word8 *Input,Word Depth)
{
	Word i;
	i = 256;
	do {
		Output[0] = PaletteConvertRGB24ToDepth(Input,Depth);	/* Convert RGB to 16 bit */
		++Output;	/* Next output pointer */
		Input+=3;	/* Next triplett */
	} while (--i);	/* All done? */
}

/**********************************

	Header:
		Convert an 8 bit palette to a true color array

	Synopsis:
		Given an 8 bit palette (256 RGB values)
		create a 16 bit color lookup table for drawing to a screen
		using this palette. The palette is read in from a resource
		file. The resource is released on exit.

	Input:
		Output = Pointer to an array of 256 Words
		Input = Pointer to the reference of the resource file
		RezNum = Which resource ID to load
		Depth = 15 or 16

	Also:
		PaletteMake16BitLookup(), PaletteConvertRGB24ToDepth()

**********************************/

void BURGERCALL PaletteMake16BitLookupRez(Word *Output,RezHeader_t *Input,Word RezNum,Word Depth)
{
	Word8 *RezPal;

	RezPal = (Word8 *)ResourceLoad(Input,RezNum);	/* Load the resource */
	if (RezPal) {			/* Ok? */
		PaletteMake16BitLookup(Output,RezPal,Depth);	/* Create the table */
		ResourceRelease(Input,RezNum);		/* Release the resource */
	}
}

/**********************************

	Header:
		Create an 8 bit remap table

	Synopsis:
		Create a pixel data conversion table by taking
   		the a source palette and trying to find the best match
		in a destination palette.
		The returned data will allow you to quickly color map a 256 color
		image onto any palette.

	Input:
		Output = Pointer to an array of 256 bytes
		DestPtr = Pointer to a palette of 256 colors (768 bytes) to map to
		SourcePal = Pointer to the palette of the shape (768 bytes) to map from

	Also:
		PaletteFindColorIndex(), PaletteMakeRemapLookupMasked()

**********************************/

void BURGERCALL PaletteMakeRemapLookup(Word8 *Output,const Word8 *DestPal,const Word8 *SourcePal)
{
	Word i;
	DestPal+=3;
	i = 256;
	do {
		Output[0] = static_cast<Word8>(PaletteFindColorIndex(DestPal,SourcePal[0],SourcePal[1],SourcePal[2],254)+1);
		++Output;
		SourcePal += 3;
	} while (--i);		/* All 256 colors done? */
}

/**********************************

	Header:
		Create an 8 bit remap table

	Synopsis:
		Create a pixel data conversion table by taking
   		the a source palette and trying to find the best match
		in a destination palette.
		The returned data will allow you to quickly color map a 256 color
		image onto any palette.

		Note: I will not alter color #0 and I will not map to either color #0
		of color #255. Color #255 will be remapped to an equivalent color.

	Input:
		Output = Pointer to an array of 256 bytes
		DestPtr = Pointer to a palette of 256 colors (768 bytes) to map to
		SourcePal = Pointer to the palette of the shape (768 bytes) to map from

	Also:
		PaletteFindColorIndex(), PaletteMakeRemapLookup()

**********************************/

void BURGERCALL PaletteMakeRemapLookupMasked(Word8 *Output,const Word8 *DestPal,const Word8 *SourcePal)
{
	Word i;
	DestPal+=3;
	SourcePal+=3;		/* Skip the first color */
	i = 255;
	Output[0] = 0;
	++Output;
	do {
		Output[0] = static_cast<Word8>(PaletteFindColorIndex(DestPal,SourcePal[0],SourcePal[1],SourcePal[2],254)+1);
		++Output;
		SourcePal += 3;
	} while (--i);		/* All 256 colors done? */
}

/**********************************

	Header:
		Create an 8 bit mask table

	Synopsis:
		Create a pixel mask table to make the MaskColor index
		transparent. This table will have the first 256 bytes containing
		the value to 'OR' with. The second 256 bytes will be the mask value.

		What effectively occurs is that the first 256 bytes are 0-255 in
		that order and the last 256 bytes are 0. The mask color will have a
		mask of 255 and a color of zero. This is to create a table
		to draw masked shapes without having to incur branch penalties
		on processors that have problems with mispredicted branches.

	Input:
		Output = Pointer to an array of 512 bytes
		MaskColor = Color index to mask as invisible (0-255) or (-1) for not used

	Also:
		PaletteMakeRemapLookup()

**********************************/

void BURGERCALL PaletteMakeColorMasks(Word8 *Output,Word MaskColor)
{
	Word8 *MaskPtr;
	Word i;
	i = 0;
	MaskPtr = Output+256;
	do {
		Output[i] = static_cast<Word8>(i);		/* Save the or mask */
		MaskPtr[i] = 0;		/* Save the and mask */
	} while (++i<256);
	MaskPtr[MaskColor] = 0xFF;	/* Set the and mask for invisible */
	Output[MaskColor] = 0;		/* Set the or mask for invisible */
}

/**********************************

	Header:
		Create a remap table to brighten or darken a shape

	Synopsis:
		If you wish to remap an 8 bit image by altering the brightness
		of one or more colors, this routine will create a remap table
		based on the new brightness.
		Give the palette for the shape and pass the red, green and blue
   		scaling adjustments in percentage points.
		100 is treated as 1.0, 50 is .5 and 200 is 2.0 intensity

	Input:
		Output = Pointer to an array of 256 bytes for remap table
		Input = Pointer to an 8 bit palette (768 bytes)
		r = Red percentage adjust (0-1000)
		g = Green percentage adjust (0-1000)
		b = Blue percentage adjust (0-1000)

	Also:
		PaletteMakeRemapLookup()

**********************************/

#define RANGEBITS 6		/* 6 bits of fraction */
#define PERCENT 100

void BURGERCALL PaletteMakeFadeLookup(Word8 *Output,const Word8 *Input,Word r,Word g,Word b)
{
	Word8 MyPal[768];	/* Local palette */
	Word8 *TempPtr;		/* Temp palette pointer */
	Word i;		/* Temps */

	TempPtr = &MyPal[0];	/* Create the new table using a temp palette */
	r = r<<RANGEBITS;		/* Scale up for fraction */
	g = g<<RANGEBITS;
	b = b<<RANGEBITS;
	i = PERCENT;		/* 1.0 in percentages */
	r = r/i;			/* Create a fixed point number */
	g = g/i;
	b = b/i;
	i = 0;				/* All 256 colors */
	do {
		Word Temp;
		Temp = (Word)Input[0];
		Temp = Temp * r;		/* Perform the RGB scale */
		Temp = Temp >> RANGEBITS;	/* Isolate the integer */
		if (Temp>=256) {			/* Too high? */
			Temp = 255;
		}
		TempPtr[0] = static_cast<Word8>(Temp);

		Temp = (Word)Input[1];
		Temp = Temp * g;
		Temp = Temp >> RANGEBITS;	/* Perform the RGB scale */
		if (Temp>=256) {
			Temp = 255;
		}
		TempPtr[1] = static_cast<Word8>(Temp);

		Temp = (Word)Input[2];
		Temp = Temp * b;
		Temp = Temp >> RANGEBITS;	/* Perform the RGB scale */
		if (Temp>=256) {
			Temp = 255;
		}
		TempPtr[2] = static_cast<Word8>(Temp);
		Input=Input+3;
		TempPtr=TempPtr+3;
	} while (++i<256);
	Input = Input-768;			/* Restore the input pointer */
	PaletteMakeRemapLookup(Output,Input,MyPal);		/* Perform the actual work */
}

/**********************************

	Header:
		Lookup a color in an 8 bit palette

	Synopsis:
		Given an 8 bit red, green and blue value,
		return the pixel value that is the most closest match.

	Input:
		PalPtr = Pointer to an 8 bit palette to map to (768 bytes)
		Red = Red value (0-255)
		Green = Green value (0-255)
		Blue = Blue value (0-255)
		Count = Number of colors in the palette

	Returns:
		Color index that is the closest match. (0-(Count-1))

	Also:
		PaletteConvertRGBToDepth()

**********************************/

Word BURGERCALL PaletteFindColorIndex(const Word8 *PalPtr,Word Red,Word Green,Word Blue,Word Count)
{
	Word i;
	Word32 ClosestDist;	/* Closest distance found */
	Word ClosestIndex;		/* Color index for closest distance */

	ClosestIndex = 0;		/* Index found */
	if (Count) {
		ClosestDist = (Word32)-1;	/* Use a bogus number */
		Red=Red+255;			/* Adjust so that the negative index is positive */
		Green=Green+255;
		Blue=Blue+255;
		i = 0;
		do {
			Word32 NewDelta;
			Word Temp;

			Temp = PalPtr[0];		/* Get the difference */
			Temp = Red-Temp;
			NewDelta = ByteSquareTable[Temp];	/* Get the square */
			Temp = PalPtr[1];
			Temp = Green-Temp;
			Temp = ByteSquareTable[Temp];
			NewDelta = NewDelta+Temp;
			Temp = PalPtr[2];
			Temp = Blue-Temp;
			Temp = ByteSquareTable[Temp];
			NewDelta = NewDelta+Temp;
			if (NewDelta < ClosestDist) {		/* Closer? */
				ClosestIndex = i;		/* This color is the closest match */
				if (!NewDelta) {		/* Perfect match? */
					break;
				}
				ClosestDist = NewDelta;	/* Save the distance */
			}
			PalPtr = PalPtr+3;		/* Next index */
		} while (++i<Count);		/* All colors checked? */
	}
	return ClosestIndex;	/* Return the lookup */
}


/**********************************

	These functions actually change the color palette
	that the hardware uses

**********************************/

/**********************************

	Header:
		Set the hardware palette to black

	Synopsis:
		All color entries in the hardware palette are
		set to black.

	Also:
		PaletteSetPtr(), PaletteWhite(), PaletteFadeToBlack()

**********************************/

void BURGERCALL PaletteBlack(void)
{
	Word8 TempPalette[sizeof(CurrentPalette)];
	FastMemSet(TempPalette,0,sizeof(TempPalette));
	PaletteSetPtr(0,256,TempPalette);
}

/**********************************

	Header:
		Set the hardware palette to white

	Synopsis:
		All color entries in the hardware palette are
		set to white

	Also:
		PaletteSetPtr(), PaletteBlack(), PaletteFadeToWhite()

**********************************/

void BURGERCALL PaletteWhite(void)
{
	Word8 TempPalette[sizeof(CurrentPalette)];
	FastMemSet(TempPalette,255,sizeof(TempPalette));
	PaletteSetPtr(0,256,TempPalette);
}


/**********************************

	Header:
		Fade the hardware palette to black

	Synopsis:
		All color entries in the hardware palette are
		set to black slowly over time.

	Also:
		PaletteFadeToPtr(), PaletteBlack(), PaletteFadeToWhite()

**********************************/

void BURGERCALL PaletteFadeToBlack(void)
{
	Word8 TempPalette[sizeof(CurrentPalette)];
	FastMemSet(&TempPalette[0],0,sizeof(TempPalette));
	PaletteFadeToPtr(TempPalette);
}

/**********************************

	Header:
		Fade the hardware palette to white

	Synopsis:
		All color entries in the hardware palette are
		set to white slowly over time.

	Also:
		PaletteFadeToPtr(), PaletteWhite(), PaletteFadeToBlack()

**********************************/

void BURGERCALL PaletteFadeToWhite(void)
{
	Word8 TempPalette[sizeof(CurrentPalette)];
	FastMemSet(&TempPalette[0],255,sizeof(TempPalette));
	PaletteFadeToPtr(TempPalette);
}

/**********************************

	Header:
		Fade the hardware palette to a palette

	Synopsis:
		All color entries in the hardware palette are
		set to the new palette slowly over time.

	Input:
		Input = Reference to the resource file that has the
		palette to fade to.
		PaletteNum = The resource number of the palette

	Also:
		PaletteFadeToPtr(), PaletteFadeToWhite(), PaletteFadeToBlack()

**********************************/

void BURGERCALL PaletteFadeTo(RezHeader_t *Input,Word PaletteNum)
{
	void *Temp;
	Temp = ResourceLoad(Input,PaletteNum);	/* Load in the resource file */
	if (Temp) {			/* Ok? */
		PaletteFadeToPtr((Word8 *)Temp);
		ResourceRelease(Input,PaletteNum);	/* Release the resource data */
	}
}

/**********************************

	Header:
		Fade the hardware palette to a palette

	Synopsis:
		All color entries in the hardware palette are
		set to the new palette slowly over time.

		The handle is returned unlocked. Nothing is performed if the
		handle is invalid or purged.

	Input:
		PaletteHandle = Handle to the palette to fade to.

	Also:
		PaletteFadeToPtr(), PaletteFadeToWhite(), PaletteFadeToBlack()

**********************************/

void BURGERCALL PaletteFadeToHandle(void **PaletteHandle)
{
	void *PalPtr;
	PalPtr = LockAHandle(PaletteHandle);	/* Lock it down */
	if (PalPtr) {						/* Uh oh!! */
		PaletteFadeToPtr((Word8 *)PalPtr);		/* Lock the handle and fade */
		UnlockAHandle(PaletteHandle);	/* Release the memory */
	}
}

/**********************************

	Header:
		Fade the hardware palette to a palette

	Synopsis:
		Given an arbitrary palette, and a palette speed variable (FadeSpeed)
		Fade from the currently shown palette to the new arbitrary palette
		using smooth steps. CurrentPalette contains the current palette, you
		can't store you new palette there since it will cause to routine to think
		the fade is already completed.

		If the requested palette is the same as CurrentPalette, this routine
		does nothing.

		Every time the hardware palette is written to, the function PaletteFadeCallBack()
		is called if the user has set it. On exit, PaletteFadeCallBack() is set to
		zero so it must be reset every time a palette fade routine is called if
		you desire the callback. PaletteFadeCallBack() is passed a 0 on startup
		and the value increments until it reaches 16. That is when the palette
		is finished. Only parameter 0 and 16 are guaranteed to be called. All
		other value will only be passed once or skipped due to low machine
		speed.

	Input:
		PalettePtr = Pointer to the palette to fade to.

	Platform:
		Running under Win95 or BeOS, the thread that calls this routine will
		be put to sleep during time delays between VBL palette updates. KeyboardKbhit()
		will only be called no more than the VBL rate. All other platforms will
		wait between VBL's with CPU polling.

	Also:
		PaletteFadeTo(), PaletteFadeToWhite(), PaletteFadeToBlack(), KeyboardKbhit(),
		FadeSpeed, PaletteVSync, CurrentPalette, ReadTick(), PaletteFadeCallBack()

**********************************/

void BURGERCALL PaletteFadeToPtr(const Word8 *PalettePtr)
{
	int DestPalette[768];		/* Must be SIGNED! */

	if (memcmp(PalettePtr,CurrentPalette,768)) {	/* Same palette? */
		Word OldFlag;
		OldFlag = PaletteVSync;		/* Save the palette VSync flag */
		PaletteVSync = TRUE;		/* Since I am fading, I can wait for VSync */

		/* Need to first get the deltas for each color component */
		/* Since a char is -127 to 128 and the differences are -255 to 255 */
		/* I need the difference table to be an array of int's */

		{
			Word8 *TempPtr;
			int *ShortPtr;
			TempPtr = CurrentPalette;		/* Pointer to palette */
			ShortPtr = DestPalette;			/* Pointer to delta */
			do {
				Word a1,a2;
				a1 = TempPtr[0];			/* Get the values */
				a2 = PalettePtr[0];
				ShortPtr[0] = a1-a2;		/* Calc the delta */
				++TempPtr;
				++PalettePtr;
				++ShortPtr;
			} while (TempPtr<&CurrentPalette[768]);
		}
		PalettePtr = PalettePtr-768;		/* Restore the palette pointer */
		{
			Word Total;			/* Total number of ticks to elapse */
			Word32 Mark;		/* Time mark */
			Fixed32 Scale;		/* Palette scale temp 0.0-1.0 */
			Word LastCall;		/* Last callback constant */

			Mark = ReadTick();	/* Get the time base */
			Total = 16*FadeSpeed;	/* Number of ticks to elapse */
			LastCall = 0;		/* Don't call on zero */
			do {
				int *ShortPtr;
				Word8 *TempPtr;

				KeyboardKbhit();					/* Allow events */
				Scale = ReadTick()-Mark;	/* Get elapsed time */
				Scale = Scale*(0x10000/(long)Total);
				if (Scale>0x10000) {		/* Overflow? */
					Scale = 0x10000;
				}
				Scale=0x10000-Scale;		/* Reverse the scale */
				TempPtr = CurrentPalette;	/* Init palette pointers */
				ShortPtr = DestPalette;
				do {
					Fixed32 Foo;

					Foo = ShortPtr[0];	/* Get a delta */
					Foo = Foo*Scale;	/* Scale the delta */
					Foo = Foo>>16;		/* Div by 16 (Signed) */
					TempPtr[0] = (Word8)(PalettePtr[0]+Foo);	/* Add to base */
					++TempPtr;
					++PalettePtr;
					++ShortPtr;
				} while (TempPtr<&CurrentPalette[768]);
				PalettePtr = PalettePtr-768;	/* Restore the pointer */
				if (PaletteFadeCallBack) {		/* Is there a callback? */
					Word Count;
					Count = 16-(Scale>>12);		/* 0-16 */
					if (Count>LastCall) {		/* Is it later? */
						LastCall = Count;
						PaletteFadeCallBack(Count);	/* Call the callback routine */
					}
				}
				PaletteSetPtr(0,256,CurrentPalette);		/* Set the new palette */
			} while (Scale);		/* All done? */
		}
		PaletteVSync = OldFlag;
	}
	PaletteFadeCallBack = 0;	/* Clear out the callback */
}

/**********************************

	Header:
		Set the hardware palette to a palette

	Synopsis:
		Set the hardware palette to an arbitrary 256 color palette
		loaded from the resource file. The resource is released upon exit.

		If the resource had an error loading, nothing occurs.

	Input:
		PalettePtr = Reference to the resource file.
		PaletteNum = Resource ID of the palette

	Also:
		PaletteSetPtr(), ResourceLoad(), ResourceRelease()

**********************************/

void BURGERCALL PaletteSet(RezHeader_t *Input,Word PaletteNum)
{
	void *TempPtr;
	TempPtr = ResourceLoad(Input,PaletteNum);	/* Load and show */
	if (TempPtr) {
		PaletteSetPtr(0,256,(Word8 *)TempPtr);
		ResourceRelease(Input,PaletteNum);		/* Release it */
	}
}

/**********************************

	Header:
		Set the hardware palette to a palette

	Synopsis:
		Set an arbitrary range of hardware palette entries to an
		arbitrary color palette that is referenced by a handle. If the
		handle is invalid or purged, nothing occurs. Start + Count must not
		exceed 256, if it does the range will be truncated.

		The source palette's first entry will be mapped to the Start entry
		in the hardware palette.

	Input:
		Start = First hardware index to modify (0-255)
		Count = Number of colors to change (0-256)
		PaletteHandle = Handle to the array of "Count" tripletts

	Also:
		PaletteSetPtr(), ResourceLoad(), ResourceRelease()

**********************************/

void BURGERCALL PaletteSetHandle(Word Start,Word Count,void **PaletteHandle)
{
	void *TempPtr;
	TempPtr = LockAHandle(PaletteHandle);	/* Lock down the handle */
	if (TempPtr) {							/* Is it ok? */
		PaletteSetPtr(Start,Count,(Word8 *)TempPtr);	/* Set the palette */
		UnlockAHandle(PaletteHandle);		/* Unlock the handle */
	}
}

/**********************************

	These routines are machine specific

**********************************/

/**********************************

	Header:
		Set the hardware border color

	Synopsis:
		Given a EGA color value, set the PC border color to
		that value. Currently, this routine is only for old programs
		and is no longer needed.

	Input:
		Color = EGA value of the border color.

	Platform:
		This routine exists in MS-DOS. All other platforms have
		a stub that performs no operation. Don't use this routine unless
		you are writing some DOS application and need this feature.

	Also:
		PaletteGetBorderColor()

**********************************/

#if !defined(__MSDOS__)
void PaletteSetBorderColor(Word /* Color */)
{
}

/**********************************

	Header:
		Get the hardware border color

	Synopsis:
		Get the EGA color value from the PC video display hardware.
		Currently, this routine is only for old programs
		and is no longer needed.

	Returns:
		EGA value of the border color (0-15).

	Platform:
		This routine exists in MS-DOS. All other platforms have
		a stub that returns 0. Don't use this routine unless
		you are writing some DOS application and need this feature.

	Also:
		PaletteSetBorderColor()

**********************************/

Word BURGERCALL PaletteGetBorderColor(void)
{
	return 0;
}
#endif

/**********************************

	Header:
		Set the hardware palette

	Synopsis:
		Get the EGA color value from the PC video display hardware.
		Currently, this routine is only for old programs
		and is no longer needed.

	Returns:
		EGA value of the border color (0-15).

	Platform:
		MS-DOS supports VESA 2.0, VESA 1.2, INT 010H and direct to
		hardware (In that order). Win95 uses DirectDraw and GDI.
		MacOS uses DrawSprocket and SetEntries(). All other platforms
		will update CurrentPalette and perform no other action.

	Also:
		PaletteSet(), PaletteFadeToPtr(), PaletteVSync

**********************************/

#if !defined(__MSDOS__) && !defined(__MAC__) && !defined(__WIN32__)
void PaletteSetPtr(Word Start,Word Count,const Word8 *PalPtr)
{
	if (!PalPtr || Start>=256) {	/* Bad? */
		return;
	}
	if ((Start+Count)>=257) {		/* Out of range? */
		Count = 256-Start;			/* Set the limit on the count */
	}
	if (Count) {
		FastMemCpy(CurrentPalette+(Start*3),PalPtr,Count*3);	/* Update the full palette */
		if (PaletteChangedProc) {
			PaletteChangedProc();
		}
	}
}
#endif

