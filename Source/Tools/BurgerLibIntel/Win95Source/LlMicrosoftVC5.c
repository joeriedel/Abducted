/**********************************

	Microsoft "C" version

**********************************/

#include "Ll64Bit.h"

#if defined(__WIN32__) && !defined(__MWERKS__)

#pragma warning(disable:4035)	/* Kill the no return int value warning */

/**********************************

	Multiply two 32 bit quantities and return
	the 64 bit result

**********************************/

__declspec(naked) LongWord64_t BURGERCALL XLongWord64MulLongTo64(long First,long Second)
{
	_asm {
	mov	eax,edx
	imul ecx
	ret
	}
}

/**********************************

	Divide a 64 bit number by a 32 bit number.
	Note : The result is assumed to be 32 bits. Use this
	routine with caution since a divide by zero will cause
	an exception

**********************************/

__declspec(naked) LongWord64_t BURGERCALL XLongWord64DivideByLong(LongWord64_t Numerator,long Denominator)
{
	_asm {
	mov	eax,ecx
	mov	ecx,[esp+4]
	idiv ecx
	cdq
	ret	4
	}
}

#endif
