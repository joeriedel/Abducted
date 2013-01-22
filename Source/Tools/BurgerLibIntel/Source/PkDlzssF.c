#include "PkPack.h"

/**********************************

	Decompress using LZSS.
	I assume I can decompress in one pass.
	This makes for fast and tight code.

**********************************/

void BURGERCALL DLZSSFast(Word8 *Dest,Word8 *Src,Word32 Length)
{
	Word8 BitBucket;
	Word8 Mask;

	if (Length) {		/* Any data to decompress? */
		Length = (Word32)Dest + Length;	/* Get the last byte address */

		Mask = 0x01;
		BitBucket = *Src++;		/* Grab the initial bit bucket */

		do {
			if (BitBucket&Mask) {			/* Get a bit from the input stream */
				*Dest++ = *Src++;		/* Copy a byte */
			} else {
				Word RunCount;
				Word8 *BackPtr;
				Word8 Temp;
#if defined(__LITTLEENDIAN__)
				RunCount = (Word) ((Word16 *)Src)[0];	/* Fetch 2 bytes */
#else
				RunCount = (Word)Src[0];
				RunCount = RunCount | ((Word)(Src[1])<<8);
#endif
				Src+=2;
				BackPtr = (Word8 *) (0xFFFFF000UL|RunCount);	/* Convert to negative */
				BackPtr = Dest+(Word32)BackPtr;			/* Get source pointer */
				switch ((RunCount>>12)&0xF) {
				case 15:
					Temp = BackPtr[0];
					++BackPtr;
					Dest[0] = Temp;
					++Dest;
				case 14:
					Temp = BackPtr[0];
					++BackPtr;
					Dest[0] = Temp;
					++Dest;
				case 13:
					Temp = BackPtr[0];
					++BackPtr;
					Dest[0] = Temp;
					++Dest;
				case 12:
					Temp = BackPtr[0];
					++BackPtr;
					Dest[0] = Temp;
					++Dest;
				case 11:
					Temp = BackPtr[0];
					++BackPtr;
					Dest[0] = Temp;
					++Dest;
				case 10:
					Temp = BackPtr[0];
					++BackPtr;
					Dest[0] = Temp;
					++Dest;
				case 9:
					Temp = BackPtr[0];
					++BackPtr;
					Dest[0] = Temp;
					++Dest;
				case 8:
					Temp = BackPtr[0];
					++BackPtr;
					Dest[0] = Temp;
					++Dest;
				case 7:
					Temp = BackPtr[0];
					++BackPtr;
					Dest[0] = Temp;
					++Dest;
				case 6:
					Temp = BackPtr[0];
					++BackPtr;
					Dest[0] = Temp;
					++Dest;
				case 5:
					Temp = BackPtr[0];
					++BackPtr;
					Dest[0] = Temp;
					++Dest;
				case 4:
					Temp = BackPtr[0];
					++BackPtr;
					Dest[0] = Temp;
					++Dest;
				case 3:
					Temp = BackPtr[0];
					++BackPtr;
					Dest[0] = Temp;
					++Dest;
				case 2:
					Temp = BackPtr[0];
					++BackPtr;
					Dest[0] = Temp;
					++Dest;
				case 1:
					Temp = BackPtr[0];
					++BackPtr;
					Dest[0] = Temp;
					++Dest;
				case 0:
				default:
					Dest[0] = BackPtr[0];	/* Copy the minimum */
					Dest[1] = BackPtr[1];
					Dest[2] = BackPtr[2];
					Dest = Dest+3;
				}
			}
			Mask <<= 1;			/* Shift out a bit */
			if (!Mask) {		/* No more? */
				BitBucket = *Src++;
				Mask = 0x01;	/* Reset the mask */
			}
		} while ((Word32)Dest<Length);
	}
}
