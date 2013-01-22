#include "PkPack.h"

/**********************************

	Decompress using LZSS
	I will save my state if I run out of packed data before decompression
	is complete

**********************************/

static Word OldState;		/* Last state of decompression */
static Word OldBitBucket;	/* Previous bit bucket */
static Word OldRun;			/* Previous 16 bit token (Half) */
static Word32 OldLength;	/* Previous ending pointer */
static Word8 *OldDest;		/* Previous dest pointer */

void BURGERCALL DLZSS(Word8 *Dest,Word8 *Src,Word32 Length,Word32 PackedLen)
{
	Word BitBucket;
	Word RunCount;

	if (!PackedLen) {		/* Decompress from old data? */
		return;		/* Exit */
	}

	PackedLen = (Word32)Src+PackedLen;	/* Fake the end pointer */

	if (!Length) {			/* Continue? */
		Length = OldLength;	/* Restore remaining length */
		Dest = OldDest;		/* Restore remaining destination */
		BitBucket = OldBitBucket;	/* Restore the bit bucket */
		RunCount = OldState;
		if (RunCount==1) {
			goto Entry1;	/* Single byte */
		}
		if (RunCount==2) {
			goto Entry2;	/* Get bit bucket */
		}
		if (RunCount==3) {	/* Get half token */
			RunCount = OldRun;
			goto Entry3;
		}
		goto Entry4;		/* Parse 16 bit token */
	}

	/* Normal decompression entry */

	Length = (Word32)Dest + Length;	/* Get the last byte address */
	OldLength = Length;		/* Save the final address */

	BitBucket = (Word) (*Src++)|0x100;		/* Grab the initial bit bucket */

	do {
		if (BitBucket&1) {			/* Get a bit from the input stream */
			if (Src==(Word8 *)PackedLen) {		/* No data remaining? */
				goto Save1;			/* Save state */
			}
Entry1:
			*Dest++ = *Src++;		/* Copy a byte */
		} else {
			if (Src==(Word8 *)PackedLen) {
				goto Save4;
			}
Entry4:
			RunCount = (Word) (*Src++);
			if (Src==(Word8 *)PackedLen) {
				goto Save3;
			}
Entry3:
			RunCount = RunCount | ((Word) (*Src++)<<8);
			{
				Word8 *BackPtr;
				Word8 Temp;
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
		}
		BitBucket>>=1;			/* Shift out a bit */
		if (BitBucket==1) {		/* No more? */
			if (Src==(Word8 *)PackedLen) {	/* No more data? */
				goto Save2;
			}
Entry2:
			BitBucket = (Word)(*Src++) | 0x100;
		}
	} while ((Word32)Dest<Length);
	OldState = 0;			/* Decompression was normal */
	return;

/* Stopped getting a single packed byte */

Save1:
	RunCount = 1;		/* Fetch 1 single unpacked byte */
	goto Exit;

Save2:
	RunCount = 2;		/* Fetching a bit token */
	goto Exit;

Save3:
	OldRun = RunCount;	/* Save the other half */
	RunCount = 3;		/* I only need one byte! */
	goto Exit;

Save4:
	RunCount = 4;		/* Fetch a 16 bit token */
Exit:;
	OldState = RunCount;
	OldBitBucket = BitBucket;		/* Save the bit bucket */
	OldDest = Dest;		/* Save the pointer */
}
