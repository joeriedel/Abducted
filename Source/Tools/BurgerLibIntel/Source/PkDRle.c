#include "PkPack.h"

/**********************************

	Unpack data using RLE compression
	Token >=0x80 = 0x101-Token Repeat (Min 2, Max 129)
	Token < 0x80 = Token+1 Data[Token+1] (Min 1, Max 128)

**********************************/

static Word OldState;		/* Last state of decompression */
static Word OldRun;			/* Previous run length */
static Word32 OldLength;	/* Previous ending pointer */
static Word8 *OldDest;		/* Previous dest pointer */

void BURGERCALL DRLE(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length,Word32 PackedLen)
{
	Word Run;
	Word8 Temp;

	if (!PackedLen) {		/* Decompress from old data? */
		return;		/* Exit */
	}

	PackedLen = (Word32)SrcPtr+PackedLen;	/* Fake the end pointer */

	if (!Length) {			/* Continue? */
		Length = OldLength;	/* Restore remaining length */
		DestPtr = OldDest;		/* Restore remaining destination */
		Run = OldState;
		if (Run==0) {		/* Get token? */
			goto Entry1;	/* Begin data */
		}
		if (Run==1) {		/* Get repeat byte */
			Run = OldRun;
			goto Entry2;	/* Get repeat byte */
		}
		Run = OldRun;		/* Raw data */
		goto Entry3;		/* Parse 16 bit token */
	}

	Length = (Word32)DestPtr + Length;	/* Final address */
	OldLength = Length;		/* Save final address */
	do {
		if (SrcPtr==(Word8 *)PackedLen) {	/* No more tokens? */
			goto Save1;
		}
Entry1:
		Run = SrcPtr[0];		/* Get the run token */
		++SrcPtr;
		if (Run>=128) {		/* Run length? */
			Run = 257-Run;	/* Count the run (2-129) */
			if (SrcPtr==(Word8 *)PackedLen) {	/* Out of data? */
				goto Save2;
			}
Entry2:
			Temp = SrcPtr[0];	/* Filler */
			do {
				DestPtr[0] = Temp;		/* Fill in the rest */
				++DestPtr;
			} while (--Run);
			++SrcPtr;	/* More input? */
		} else {
			++Run;		/* +1 to the count */
Entry3:
			if ((SrcPtr+Run)>(Word8 *)PackedLen) {	/* Will it overflow? */
				OldRun=Run;					/* Save the FULL run */
				Run = (Word8 *)PackedLen-SrcPtr;	/* How many bytes can I transfer? */
				if (Run) {				/* Any? */
					OldRun-=Run;		/* Remove from total */
					do {
						Temp = SrcPtr[0];	/* Transfer raw data */
						++SrcPtr;
						DestPtr[0] = Temp;
						++DestPtr;
					} while (--Run);
				}
				Run = 2;
				goto Exit;
			}
			do {
				Temp = SrcPtr[0];		/* Transfer data */
				++SrcPtr;
				DestPtr[0] = Temp;
				++DestPtr;
			} while (--Run);
		}
	} while ((Word32)DestPtr<Length);	/* More? */
	OldState = 0;	/* Decompression was normal */
	return;
Save2:
	OldRun = Run;
	Run = 1;
	goto Exit;
Save1:
	Run = 0;
Exit:
	OldState = Run;		/* Save the state of the decompressor */
	OldDest = DestPtr;	/* Save the dest pointer */
}


