/**********************************

	This is 64 bit code for compilers that support
	native 64 bit quantities. As you can see it's
	far simpler than the code needed for 32 bit
	compilers

**********************************/

#include "Ll64Bit.h"

#ifdef LONGWORD64NATIVE

/**********************************

	Multiply two 32 bit quantities and return
	the 64 bit result

**********************************/

#if !defined(__INTEL__)
LongWord64_t BURGERCALL XLongWord64MulLongTo64(long First,long Second)
{
	return ((LongWord64_t)First*(LongWord64_t)Second);
}
#endif

/**********************************

	Compare two 64 bit numbers for equality (0), less than (-1)
	or greater than (1)

**********************************/

int BURGERCALL XLongWord64Compare(LongWord64_t First,LongWord64_t Second)
{
	if (First<Second) {
		return -1;
	}
	if (First==Second) {
		return 0;
	}
	return 1;
}

/**********************************

	Divide a 64 bit number by a 32 bit number.
	Note : The result is assumed to be 32 bits. Use this
	routine with caution since a divide by zero will cause
	an exception

**********************************/

#if !defined(__INTEL__)
LongWord64_t BURGERCALL XLongWord64DivideByLong(LongWord64_t Numerator,long Denominator)
{
	return Numerator/(LongWord64_t)Denominator;
}
#endif

#endif
