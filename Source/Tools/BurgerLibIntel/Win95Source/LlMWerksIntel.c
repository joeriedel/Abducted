/**********************************

	Metrowerks "C" for intel version

**********************************/

#include "Ll64Bit.h"

#if defined(__MWERKS__) && defined(__INTEL__)

/**********************************

	Multiply two 32 bit quantities and return
	the 64 bit result

**********************************/

asm LongWord64_t BURGERCALL XLongWord64MulLongTo64(long First,long Second)
{
	asm {
	mov	eax,[esp+4]
	mov	edx,[esp+8]
	imul edx
	ret
	}
}

/**********************************

	Divide a 64 bit number by a 32 bit number.
	Note : The result is assumed to be 32 bits. Use this
	routine with caution since a divide by zero will cause
	an exception

**********************************/

asm LongWord64_t BURGERCALL XLongWord64DivideByLong(LongWord64_t Numerator,long Denominator)
{
	asm {
	mov eax,[esp+4]
	mov	edx,[esp+8]
	mov	ecx,[esp+12]
	idiv ecx
	cdq
	ret
	}
}

#endif
