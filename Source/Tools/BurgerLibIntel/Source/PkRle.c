#include "PkPack.h"
#include "MmMemory.h"

/**********************************

	Compress data using RLE encoding
	(This version uses the format from ILBM files)

**********************************/

void ** BURGERCALL EncodeRLE(Word8 *InputBuffer,Word32 Length)
{
	Word8 Temp;
	Word Run;
	Word MaxRun;
	Word8 *OutputBuffer;
	void **OutputHandle;

	if (!Length) {		/* Any input data? */
		return 0;
	}
	OutputHandle = AllocAHandle(Length+(Length>>3)+10);
	if (!OutputHandle) {		/* Can't get memory! */
		return 0;		/* Abort anyway! */
	}
	OutputBuffer = (Word8 *)LockAHandle(OutputHandle);	/* Lock it down */
	do {
		if (Length==1) {	/* Only 1 byte remaining? */
			Run = 1;		/* Perform the single pack byte */
			goto NoPack;
		}
		Temp = InputBuffer[0];		/* Check for repeater */
		if (InputBuffer[1] == Temp) {	/* Repeat? */
			Run = 2;			/* Minimum data to pack */
			if (Length>=3) {	/* Is this all the data? */
				if (Length>=129) {	/* Maximum loops to try */
					MaxRun = 129-2;
				} else {
					MaxRun = Length-2;
				}
				do {
					if (InputBuffer[Run]!=Temp) {	/* Find end of repeater */
						break;
					}
					++Run;		/* 1 more to run */
				} while (--MaxRun);
			}

			/* Perform a run length token Run=2-129 */
			Length-=Run;		/* Remove packed data */
			OutputBuffer[0] = (Word8)(257-Run);	/* 128-255 */
			OutputBuffer[1] = Temp;
			OutputBuffer+=2;	/* +2 to output */
			InputBuffer+=Run;	/* Move input pointer */
		} else {
			Run = 2;		/* Raw of 2 */
			if (Length>=3) {		/* More than 2? */
				if (Length>=128) {	/* Get maximum */
					MaxRun = 128-2;
				} else {
					MaxRun = Length-2;
				}
				Temp = InputBuffer[1];	/* Preload the next byte */
				do {		/* Scan for next repeater */
					if (InputBuffer[Run]==Temp) {
						--Run;	/* Remove from the run */
						break;
					}
					Temp = InputBuffer[Run];	/* Get the next byte */
					++Run;		/* +1 to the run */
				} while (--MaxRun);
			}
			/* Perform a raw data transfer */
NoPack:
			Length-=Run;		/* Remove from data */
			*OutputBuffer++ = (Word8)(Run-1);	/* Run 1-128 */
			do {
				*OutputBuffer++ = *InputBuffer++;
			} while (--Run);
		}
	} while (Length);		/* Any data left */

	Length = OutputBuffer-(Word8 *)(*OutputHandle);	/* Resize the output */
	UnlockAHandle(OutputHandle);
	return ResizeAHandle(OutputHandle,Length);
}

