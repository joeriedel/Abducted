#include "ClStdLib.h"

/**********************************

	Metrowerks for Intel

**********************************/

#if defined(__MWERKS__) && defined(__INTEL__)
/**********************************

	Take an arbitrary value and round it up to
	the nearest power of 2
	If the input is 0x40000001 to 0xFFFFFFFF, I return 0x80000000
	Zero will return zero

**********************************/

asm Word32 PowerOf2(Word32 Input)
{
	_asm {
	mov eax,[esp+4]			/* Get the parm */
	cmp eax,0x40000000		/* Too large a number? */
	ja Maximum				/* Return the maximum */
	cmp eax,2				/* Too small a number? */
	jbe Minimum				/* Return as is */
	dec eax					/* Move down a bit */
	bsr	ecx,eax				/* Find the highest set bit (0-30) */
	mov eax,2				/* Shift up the nearest power of two */
	shl eax,cl				/* Return the result */
Minimum:
	ret						/* Result in EAX */
Maximum:
	mov eax,0x80000000		/* Maximum value */
	ret
	}
}

/**********************************

	Perform the ANSI function strlen()
	This is in optimized Pentium assembly
	The algorithm is explained in the generic "C" version

**********************************/

asm Word BURGERCALL FastStrLen(const char *Input)
{
	_asm {
	mov eax,[esp+4]
	push ebx
	mov ebx,eax
	and eax,3
	neg eax
	jz LL
	mov edx,0xFFFFFFFF
	lea ecx,[32+eax*8]
	shr edx,cl
	mov ecx,[ebx+eax]
	add eax,4
	or ecx,edx
	mov edx,ecx
	xor ecx,0xFFFFFFFF
	sub edx,0x01010101
	and ecx,0x80808080
	test ecx,edx
	jnz L0
LL:
	mov edx,[ebx+eax]
	mov ecx,[ebx+eax]
	add eax,4
	xor ecx,0xFFFFFFFF
	sub edx,0x01010101
	and ecx,0x80808080
	test ecx,edx
	jz LL
L0:
	add edx,0x01010101
	test edx,0xFF
	jz L4
	test edx,0xFF00
	jz L3
	test edx,0xFF0000
	jz L2
	dec eax
	pop ebx
	ret
L2:
	sub eax,2
	pop ebx
	ret
L3:
	sub eax,3
	pop ebx
	ret
L4:
	sub eax,4
	pop ebx
	ret
	}
}

/**********************************

	Change the floating point precision in
	Intel processors to allow fast divides for
	routines that really need it.
	I return the previous state so you can quickly
	change it back.

**********************************/

asm IntelFP_e BURGERCALL IntelSetFPPrecision(IntelFP_e Input)
{
	_asm {
	mov ecx,[esp+4]
	shl	ecx,8		/* Move to the Pentium bits area */
	fnstcw	[esp+4]	/* Get the current status word */
	mov eax,dword ptr [esp+4]		/* Get the previous value */
	mov edx,dword ptr [esp+4]
	and eax,0x0300	/* Mask for return value */
	and edx,0xFCFF	/* Mask off unused bits */
	shr eax,8		/* Convert to enum */
	or	edx,ecx 	/* Blend in the bits */
	mov dword ptr [esp+4],edx		/* Store in memory */
	fldcw [esp+4]		/* Save the new status register */
	ret
	}
}

#endif

