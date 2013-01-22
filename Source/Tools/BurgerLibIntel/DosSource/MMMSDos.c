#include "MmMemory.h"

/**********************************

	This code is DOS Only!

**********************************/

#if defined(__MSDOS__)
#include "ClStdLib.h"
#include "MsDos.h"
#include <stdlib.h>

#if defined(__X32__)
#include <X32.h>
#pragma library ("x32.lib");
#endif

/**********************************

	Convert an encoded real mode address (16:16) (Segment:Offset)
	into a protected mode pointer so that I can use the
	memory that the real mode pointer points to.

**********************************/

void * BURGERCALL RealToProtectedPtr(Word32 RealPtr)
{
#if defined(__X32__)
	return &ZeroBase[((RealPtr>>12)&0xFFFFFFF0)+(Word16)RealPtr];
#else
	return (void *)(((RealPtr>>12)&0xFFFFFFF0)+(Word16)RealPtr);
#endif
}

/**********************************

	Allocate an 8K buffer in real memory for use in DOS calls.
	Return the pointer in protected memory.

**********************************/

void * BURGERCALL GetRealBufferProtectedPtr(void)
{
	Word32 TempReal;
	/* Get real pointer and then convert to true pointer */
	TempReal = GetRealBufferPtr();		/* Get the buffer pointer */
	if (TempReal) {		/* Did it allocate? */
		return RealToProtectedPtr(TempReal);	/* Convert to real pointer */
	}
	return 0;		/* Error! */
}

/**********************************

	Allocate an 8K buffer in real memory for use in DOS calls.
	Return the pointer in SEGMENT:OFFSET format

**********************************/

static Word32 RealBufferPtr;	/* Cached pointer to real memory */

static void ReleaseBuff(void)
{
	DeallocRealMemory(RealBufferPtr);		/* Release the memory */
}

Word32 BURGERCALL GetRealBufferPtr(void)
{
	Word32 Foo;			/* Real memory pointer */

	Foo = RealBufferPtr;	/* Is the buffer already allocated? */
	if (Foo) {
		return Foo;			/* Return the buffer pointer */
	}
	Foo = AllocRealMemory(8192);	/* Get some REAL memory */
	if (Foo) {
		RealBufferPtr = Foo;	/* Save in global */
		atexit(ReleaseBuff);	/* Allow release on exit */
	}
	return Foo;				/* Return value */
}

/**********************************

	Allocate a chunk of REAL mode memory.
	Note : I use DOS to allocate memory under
	allocations under the X32 DOS extender.
	It is recommended that you use DOS4GW for applications
	that manipulate large numbers of real mode memory blocks

**********************************/

Word32 BURGERCALL AllocRealMemory(Word32 Size)
{
#if defined(__X32__)
	Size = _x32_real_alloc(Size);		/* Call X32 to allocate memory */
	if (!Size) {			/* Did I get it? */
		NonFatal("Can't allocate real memory\n");
	}
	return Size;
#else
	Regs16 Regs;
	Size = Size+15;			/* Round to the nearest 16 bytes */
	Size = Size>>4;			/* Number of paragraphs to allocate */
	Regs.ax = 0x4800;		/* DOS allocate memory command */
	Regs.bx = (Word16)Size;	/* Number of paragraphs to allocate */
	Int86x(0x21,&Regs,&Regs);		/* Allocate */
	if (Regs.flags&1) {		/* Error?? */
		NonFatal("Can't allocate real memory\n");
		return 0;
	}
	Size = Regs.ax;		/* Get the segment */
	return Size<<16;	/* Return as a real mode pointer */
#endif
}

/**********************************

	Release real mode memory back to DOS
	Note : This routine will NOT work under X32.
	For programs running under X32, you allocate real mode
	memory on startup and never release it. Otherwise
	you must use the DOS4GW dos extender for reliable results.

**********************************/

void BURGERCALL DeallocRealMemory(Word32 RealPtr)
{
#if defined(__X32__)
	_x32_real_free(RealPtr);		/* Call X32 to allocate memory */
#else
	Regs16 Regs;

	RealPtr = RealPtr>>16;	/* Isolate the segment */
	Regs.ax = 0x4900;		/* DOS release memory command */
	Regs.es = (Word16)RealPtr;	/* Get the segment */
	Int86x(0x21,&Regs,&Regs);	/* Release it */
	if (Regs.flags&1) {		/* Always an error under X32 */
		NonFatal("Can't release real memory\n");
	}
#endif
}

#endif
