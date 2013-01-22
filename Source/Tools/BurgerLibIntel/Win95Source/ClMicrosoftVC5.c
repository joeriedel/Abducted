#include "ClStdLib.h"

/**********************************

	Microsquish "C"

**********************************/

#if !defined(__MWERKS__) && defined(__WIN32__)
#pragma warning(disable:4035)

/**********************************

	Take an arbitrary value and round it up to
	the nearest power of 2
	If the input is 0x40000001 to 0xFFFFFFFF, I return 0x80000000
	Zero will return zero

**********************************/

__declspec(naked) Word32 BURGERCALL PowerOf2(Word32 Input)
{
	_asm {
	cmp ecx,0x40000000		/* Too large a number? */
	ja Maximum				/* Return the maximum */
	cmp ecx,2				/* Too small a number? */
	jbe Minimum				/* Return as is */
	dec ecx					/* Move down a bit */
	bsr	ecx,ecx				/* Find the highest set bit (0-30) */
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

	Swap the endian of a long

**********************************/

#if 0
__declspec(naked) long BURGERCALL Burger::Swap(long Val)
{
	_asm {
	mov		eax,ecx
	bswap	eax
	ret
	}
}

/**********************************

	Swap the endian of a Word32

**********************************/

__declspec(naked) Word32 BURGERCALL Burger::Swap(Word32 Val)
{
	_asm {
	mov		eax,ecx
	bswap	eax
	ret
	}
}
/**********************************

	Swap the endian of a short

**********************************/

__declspec(naked) short BURGERCALL Burger::Swap(short Val)
{
	_asm {
	mov	al,ch
	mov ah,cl
	ret
	}
}

/**********************************

	Swap the endian of a short

**********************************/

__declspec(naked) Word16 BURGERCALL Burger::Swap(Word16 Val)
{
	_asm {
	mov	al,ch
	mov ah,cl
	ret
	}
}
#endif

/**********************************

	Perform the ANSI function strlen()
	This is in optimized Pentium assembly
	The algorithm is explained in the generic "C" version

**********************************/

__declspec(naked) Word BURGERCALL FastStrLen(const char *Input)
{
	_asm {
	mov eax,ecx
	push ebx
	mov ebx,ecx
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

__declspec(naked) IntelFP_e BURGERCALL IntelSetFPPrecision(IntelFP_e Input)
{
	_asm {
	push eax
	shl	ecx,8		/* Move to the Pentium bits area */
	fnstcw	[esp]	/* Get the current status word */
	mov eax,dword ptr [esp]		/* Get the previous value */
	mov edx,dword ptr [esp]
	and eax,0x0300	/* Mask for return value */
	and edx,0xFCFF	/* Mask off unused bits */
	shr eax,8		/* Convert to enum */
	or	edx,ecx 	/* Blend in the bits */
	mov dword ptr [esp],edx		/* Store in memory */
	fldcw [esp]		/* Save the new status register */
	pop	ecx
	ret
	}
}

#endif