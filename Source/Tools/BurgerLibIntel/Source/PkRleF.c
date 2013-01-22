#include "PkPack.h"

/**********************************

	Unpack data using RLE compression
	Token >=0x80 = 0x101-Token Repeat (Min 2, Max 129)
	Token < 0x80 = Token+1 Data[Token+1] (Min 1, Max 128)

**********************************/

void BURGERCALL DRLEFast(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length)
{
	Word Run;
	Word8 Temp;
	Word Run2;

	if (Length) {
		Length = (Word32)DestPtr + Length;
		do {
			Run = SrcPtr[0];		/* Get the run token */
			if (Run>=128) {		/* Run length? */

				Run = 257-Run;	/* Count the run (2-129) */
				Temp = SrcPtr[1];	/* Filler */
				Run2 = Run>>2;		/* Unrolled loop */
				if (Run2) {			/* Greater than 3 */
					Word32 TempL;
					TempL = Temp;
					TempL = (TempL<<8)|TempL;
					TempL = (TempL<<16)|TempL;
					do {
						((Word32 *)DestPtr)[0] = TempL;	/* Fill in the run */
						DestPtr+=4;
					} while (--Run2);
				}
				Run&=3;		/* Remainder */
				if (Run) {
					do {
						DestPtr[0] = Temp;		/* Fill in the rest */
						++DestPtr;
					} while (--Run);
				}
				SrcPtr=SrcPtr+2;	/* More input? */
			} else {
				++Run;		/* +1 to the count */
				++SrcPtr;	/* Index to the true source data */
				Run2 = Run>>2;
				if (Run2) {
					do {
						((Word32 *)DestPtr)[0] = ((Word32 *)SrcPtr)[0];	/* memcpy */
						DestPtr+=4;
						SrcPtr+=4;
					} while (--Run2);
				}
				Run&=3;
				if (Run) {
					do {
						Temp = SrcPtr[0];
						++SrcPtr;
						DestPtr[0] = Temp;
						++DestPtr;
					} while (--Run);
				}
			}
		} while ((Word32)DestPtr<Length);	/* More? */
	}
}


