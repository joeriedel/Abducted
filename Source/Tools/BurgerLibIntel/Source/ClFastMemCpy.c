#include "ClStdLib.h"
#undef FastMemCpy

/**********************************

	Copy some memory quickly, Must have LOW overhead
	This routine has been VTuned. Don't touch it!!!!!
	Why do I do it this way? doubles on ppc 601's will copy fast
	under any circumstances but the 603 and 604 will cause
	a processor exception if the data is not longword aligned and incurs
	a 440 cycle penalty PER instruction. Pathetic.

**********************************/

/* Power PC version */

#if defined(__POWERPC__)

void BURGERCALL FastMemCpy(void *DestPtr,const void *SrcPtr,Word32 Length)
{
	Word WordCount;		/* Count for large chunks */
	double DTemp;		/* Temp double */

	if (((Word32)SrcPtr)&3) {		/* Is the source pointer not long word aligned? */
		goto AlignSource;			/* Align the source */
	}
SourceOk:
	if (!(((Word32)DestPtr)&3)) {		/* Is the dest aligned? */
	/* The pointers are double aligned! */
		WordCount = Length>>5;		/* double copying (32 byte runs) */
		if (WordCount) {			/* Any longwords? */
			do {
				double DTemp2;
				DTemp = ((double *)SrcPtr)[0];
				DTemp2 = ((double *)SrcPtr)[1];
				((double *)DestPtr)[0] = DTemp;
				((double *)DestPtr)[1] = DTemp2;
				DTemp = ((double *)SrcPtr)[2];
				DTemp2 = ((double *)SrcPtr)[3];
				((double *)DestPtr)[2] = DTemp;
				((double *)DestPtr)[3] = DTemp2;
				DestPtr=((Word8 *)DestPtr)+32;
				SrcPtr=((Word8 *)SrcPtr)+32;
			} while (--WordCount);
		}
		if (Length&16) {				/* 16 byte run? */
			double DTemp2;
			DTemp = ((double *)SrcPtr)[0];
			DTemp2 = ((double *)SrcPtr)[1];
			((double *)DestPtr)[0] = DTemp;
			((double *)DestPtr)[1] = DTemp2;
			DestPtr=((Word8 *)DestPtr)+16;
			SrcPtr=((Word8 *)SrcPtr)+16;
		}
		if (Length&8) {				/* 8 byte run? */
			((double *)DestPtr)[0] = ((double*)SrcPtr)[0];
			DestPtr=((Word8 *)DestPtr)+8;
			SrcPtr=((Word8 *)SrcPtr)+8;
		}
		if (Length&4) {				/* 4 byte run? */
			((Word32 *)DestPtr)[0] = ((Word32*)SrcPtr)[0];
			DestPtr=((Word8 *)DestPtr)+4;
			SrcPtr=((Word8 *)SrcPtr)+4;
		}
		if (Length&2) {				/* 2 byte run? */
			((Word16 *)DestPtr)[0] = ((Word16*)SrcPtr)[0];
			DestPtr=((Word8 *)DestPtr)+2;
			SrcPtr=((Word8 *)SrcPtr)+2;
		}
		if (Length&1) {				/* 1 byte left? */
			((Word8 *)DestPtr)[0] = ((Word8 *)SrcPtr)[0];
		}
		return;			/* Done! */
	}
	/* The pointers are NOT longword aligned! */
	WordCount = Length>>4;		/* Longword copying */
	if (WordCount) {			/* Any longwords? */
		do {
			Word32 LTemp,LTemp2;
			LTemp = ((Word32 *)SrcPtr)[0];
			LTemp2 = ((Word32 *)SrcPtr)[1];
			((Word32 *)DestPtr)[0] = LTemp;
			((Word32 *)DestPtr)[1] = LTemp2;
			LTemp = ((Word32 *)SrcPtr)[2];
			LTemp2 = ((Word32 *)SrcPtr)[3];
			((Word32 *)DestPtr)[2] = LTemp;
			((Word32 *)DestPtr)[3] = LTemp2;
			DestPtr=((Word8 *)DestPtr)+16;
			SrcPtr=((Word8 *)SrcPtr)+16;
		} while (--WordCount);
	}
	if (Length&8) {
		Word32 LTemp,LTemp2;
		LTemp = ((Word32 *)SrcPtr)[0];
		LTemp2 = ((Word32 *)SrcPtr)[1];
		((Word32 *)DestPtr)[0] = LTemp;
		((Word32 *)DestPtr)[1] = LTemp2;
		DestPtr=((Word8 *)DestPtr)+8;
		SrcPtr=((Word8 *)SrcPtr)+8;
	}
	if (Length&4) {
		((Word32 *)DestPtr)[0] = ((Word32*)SrcPtr)[0];
		DestPtr=((Word8 *)DestPtr)+4;
		SrcPtr=((Word8 *)SrcPtr)+4;
	}
	if (Length&2) {
		((Word16 *)DestPtr)[0] = ((Word16*)SrcPtr)[0];
		DestPtr=((Word8 *)DestPtr)+2;
		SrcPtr=((Word8 *)SrcPtr)+2;
	}
	if (Length&1) {
		((Word8 *)DestPtr)[0] = ((Word8 *)SrcPtr)[0];
	}
	return;

/* Source is not longword aligned, align it */

AlignSource:
	if (((Word32)SrcPtr)&1 && Length) {
		((Word8 *)DestPtr)[0] = ((Word8 *)SrcPtr)[0];		/* Move a single byte */
		SrcPtr=((Word8 *)SrcPtr)+1;					/* Source is now SHORT aligned! */
		DestPtr=((Word8 *)DestPtr)+1;
		--Length;					/* Remove from the count */
	}
	if (((Word32)SrcPtr)&2 && Length>=2) {
		((Word16 *)DestPtr)[0] = ((Word16 *)SrcPtr)[0];		/* Move a single byte */
		SrcPtr=((Word8 *)SrcPtr)+2;					/* Source is now SHORT aligned! */
		DestPtr=((Word8 *)DestPtr)+2;
		Length-=2;					/* Remove from the count */
	}
	goto SourceOk;
}

#else		/* 680x0 and other risc chip versions */

void BURGERCALL FastMemCpy(void *DestPtr,const void *SrcPtr,Word32 Length)
{
	Word WordCount;		/* Count for large chunks */

	if (((Word32)SrcPtr)&3) {		/* Is the source pointer not long word aligned? */
		goto AlignSource;			/* Align the source */
	}
SourceOk:
	WordCount = Length>>4;		/* Longword copying */
	if (WordCount) {			/* Any longwords? */
		do {
			Word32 LTemp,LTemp2;
			LTemp = ((Word32 *)SrcPtr)[0];
			LTemp2 = ((Word32 *)SrcPtr)[1];
			((Word32 *)DestPtr)[0] = LTemp;
			((Word32 *)DestPtr)[1] = LTemp2;
			LTemp = ((Word32 *)SrcPtr)[2];
			LTemp2 = ((Word32 *)SrcPtr)[3];
			((Word32 *)DestPtr)[2] = LTemp;
			((Word32 *)DestPtr)[3] = LTemp2;
			DestPtr=((Word8 *)DestPtr)+16;
			SrcPtr=((Word8 *)SrcPtr)+16;
		} while (--WordCount);
	}
	if (Length&8) {
		Word32 LTemp,LTemp2;
		LTemp = ((Word32 *)SrcPtr)[0];
		LTemp2 = ((Word32 *)SrcPtr)[1];
		((Word32 *)DestPtr)[0] = LTemp;
		((Word32 *)DestPtr)[1] = LTemp2;
		DestPtr=((Word8 *)DestPtr)+8;
		SrcPtr=((Word8 *)SrcPtr)+8;
	}
	if (Length&4) {
		((Word32 *)DestPtr)[0] = ((Word32*)SrcPtr)[0];
		DestPtr=((Word8 *)DestPtr)+4;
		SrcPtr=((Word8 *)SrcPtr)+4;
	}
	if (Length&2) {
		((Word16 *)DestPtr)[0] = ((Word16*)SrcPtr)[0];
		DestPtr=((Word8 *)DestPtr)+2;
		SrcPtr=((Word8 *)SrcPtr)+2;
	}
	if (Length&1) {
		((Word8 *)DestPtr)[0] = ((Word8 *)SrcPtr)[0];
	}
	return;

/* Source is not longword aligned, align it */

AlignSource:
	if (((Word32)SrcPtr)&1 && Length) {
		((Word8 *)DestPtr)[0] = ((Word8 *)SrcPtr)[0];		/* Move a single byte */
		SrcPtr=((Word8 *)SrcPtr)+1;					/* Source is now SHORT aligned! */
		DestPtr=((Word8 *)DestPtr)+1;
		--Length;					/* Remove from the count */
	}
	if (((Word32)SrcPtr)&2 && Length>=2) {
		((Word16 *)DestPtr)[0] = ((Word16 *)SrcPtr)[0];		/* Move a single byte */
		SrcPtr=((Word8 *)SrcPtr)+2;					/* Source is now SHORT aligned! */
		DestPtr=((Word8 *)DestPtr)+2;
		Length-=2;					/* Remove from the count */
	}
	goto SourceOk;
}

#endif
