#include "ClStdLib.h"

/**********************************

	This routine will fill memory faster than fast!!!

**********************************/

/**********************************

	Power macintosh version. Optimized for 603/604 processors

**********************************/

/* This is to fool the metroworks compiler into aligning the data */
/* on double boundaries */

#if defined(__POWERPC__)			/* Power PC microprocessors only */

void BURGERCALL FastMemSet16(void *DestPtr,Word Fill,Word32 Length)
{
	Word WordCount;		/* Count for large chunks */
	double DTemp;		/* Temp double */
	double MemDouble;

	Fill &= 0xFFFF;		/* Failsafe */
	Fill = (Fill<<16)+Fill;			/* Convert the byte to a Word32 */
	((Word32 *)&MemDouble)[0] = Fill;
	((Word32 *)&MemDouble)[1] = Fill;
	DTemp = MemDouble;				/* Get the fill value as a double */
	DestPtr=((Word8 *)DestPtr)-8;		/* Allow the use of stfu */
	if (((Word32)DestPtr)&7) {		/* Is the source pointer not long word aligned? */
		goto AlignDest;			/* Align the destination */
	}
	/* The pointers are double aligned! */
	WordCount = Length>>5;		/* double copying (32 byte runs) */
	if (WordCount) {			/* Any longwords? */
SourceOk:
		do {
			((double *)DestPtr)[1] = DTemp;		/* Fill in 32 bytes of data */
			((double *)DestPtr)[2] = DTemp;
			((double *)DestPtr)[3] = DTemp;
			((double *)DestPtr)[4] = DTemp;
			DestPtr=((Word8 *)DestPtr)+32;
		} while (--WordCount);
	}
	if (Length&16) {	/* 16 bytes remaining? */
		goto Do16;
	}
	if (Length&8) {		/* 8 bytes remaining? */
		goto Do8;
	}
	if (Length&4) {		/* 4 bytes remaining? */
		goto Do4;
	}
	if (Length&2) {		/* 2 bytes remaining? */
		goto Do2;
	}
	if (Length&1) {	/* 1 byte left? */
		goto Do1;
	}
	return;

Do16:
	((double *)DestPtr)[1] = DTemp;		/* Fill 16 bytes */
	((double *)DestPtr)[2] = DTemp;
	DestPtr=((Word8 *)DestPtr)+16;
	if (Length&8) {
		goto Do8;
	}
	if (Length&4) {		/* 4 bytes remaining? */
		goto Do4;
	}
	if (Length&2) {		/* 2 bytes remaining? */
		goto Do2;
	}
	if (Length&1) {	/* 1 byte left? */
		goto Do1;
	}
	return;

Do8:
	((double *)DestPtr)[1] = DTemp;
	DestPtr=((Word8 *)DestPtr)+8;
	if (Length&4) {
		goto Do4;
	}
	if (Length&2) {		/* 2 bytes remaining? */
		goto Do2;
	}
	if (Length&1) {	/* 1 byte left? */
		goto Do1;
	}
	return;

Do4:
	((Word32 *)DestPtr)[2] = Fill;
	DestPtr=((Word8 *)DestPtr)+4;
	if (Length&2) {
		goto Do2;
	}
	if (Length&1) {	/* 1 byte left? */
		goto Do1;
	}
	return;

Do2:
	((Word16 *)DestPtr)[4] = Fill;
	DestPtr=((Word8 *)DestPtr)+2;
	if (Length&1) {
		goto Do1;
	}
	return;

Do1:
	((Word8 *)DestPtr)[8] = Fill;	/* Store the final byte */
	return;


/* Dest is not longword aligned, align it */

AlignDest:
	if (Length<64) {
		goto Quick;
	}
	WordCount = 8-(((Word32)DestPtr)&7);
	Length-=WordCount;
	WordCount>>=1;			/* Copy shorts only */
	do {
		((Word16 *)DestPtr)[4] = Fill;		/* Move a single byte */
		DestPtr=((Word8 *)DestPtr)+2;	/* Dest is now SHORT aligned */
	} while (--WordCount);
	WordCount = Length>>5;		/* double copying (32 byte runs) */
	goto SourceOk;


Quick:
	WordCount = Length>>3;
	if (WordCount) {
		do {
			((Word32 *)DestPtr)[2] = Fill;
			((Word32 *)DestPtr)[3] = Fill;
			DestPtr=((Word8 *)DestPtr)+8;		/* Dest is now LONG aligned! */
		} while (--WordCount);
	}
	if (Length&4) {		/* 4 bytes remaining? */
		goto Do4;
	}
	if (Length&2) {		/* 2 bytes remaining? */
		goto Do2;
	}
	if (Length&1) {		/* 1 byte left? */
		goto Do1;
	}
	return;
}

/**********************************

	Pentium processors
	use an assembly function

**********************************/

//#elif defined(__WATCOMC__) || (defined(__WIN32__) && !defined(__MWERKS__))

/**********************************

	Generic code

**********************************/

#else

#include <string.h>
void BURGERCALL FastMemSet16(void *DestPtr,Word Fill,Word32 Length)
{
	memset(DestPtr,Fill,Length);
}

#endif
