#include "FxFixed.h"

/**********************************

	Metrowerks for Intel

**********************************/

#if defined(__MWERKS__) && defined(__WIN32__)

/******************************

	Mul two pairs of 32 bit numbers and return the upper 32 bits

******************************/

asm long BURGERCALL IntDblMulAdd(long Mul1,long Mul2,long Mul3,long Mul4)
{
	asm {
	mov		eax,[esp+4]
	mov		edx,[esp+8]
	imul	edx
	push	ebx
	mov		ebx,eax
	mov		ecx,edx
	mov		eax,[esp+12+4]
	mov		edx,[esp+16+4]
	imul	edx
	add		eax,ebx
	pop		ebx
	adc		edx,ecx
	mov		eax,edx
	ret
	}
}

/******************************

	Mul two 32 bit numbers and return the upper 32 bits

******************************/

asm long BURGERCALL IntMulHigh32(long Mul1,long Mul2)
{
	asm {
	mov		eax,[esp+4]
	mov		edx,[esp+8]
	imul	edx
	mov		eax,edx
	ret
	}
}

/**********************************

	Calculate the square root of a 32 bit unsigned
	value. The maximum value is 46341 for the square root
	of 0x7FFFFFFF. This routine is 100% accurate.

**********************************/

#if 0
asm Word BURGERCALL IMIntSqrt(Word32 Input)
{
	asm {
	fild	DWORD PTR [esp+4]
	fsqrt
	fistp	DWORD PTR [esp+4]
	mov		eax,[esp+4]
	ret
	}
}
#else
asm Word BURGERCALL IMIntSqrt(Word32 Input)
{
	asm {
	mov		ecx,[esp+4]
	xor		eax,eax
	cmp		ecx,040000000H
	jb		L1
	mov		eax,040000000H
	sub		ecx,eax
L1:
	lea		edx,[010000000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L2
	add		eax,010000000H
	sub		ecx,edx
L2:
	lea		edx,[04000000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L3
	add		eax,04000000H
	sub		ecx,edx
L3:
	lea		edx,[01000000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L4
	add		eax,01000000H
	sub		ecx,edx
L4:
	lea		edx,[0400000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L5
	add		eax,0400000H
	sub		ecx,edx
L5:
	lea		edx,[0100000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L6
	add		eax,0100000H
	sub		ecx,edx
L6:
	lea		edx,[040000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L7
	add		eax,040000H
	sub		ecx,edx
L7:
	lea		edx,[010000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L8
	add		eax,010000H
	sub		ecx,edx
L8:
	lea		edx,[04000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L9
	add		eax,04000H
	sub		ecx,edx
L9:
	lea		edx,[01000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L10
	add		eax,01000H
	sub		ecx,edx
L10:
	lea		edx,[0400H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L11
	add		eax,0400H
	sub		ecx,edx
L11:
	lea		edx,[0100H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L12
	add		eax,0100H
	sub		ecx,edx
L12:
	lea		edx,[040H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L13
	add		eax,040H
	sub		ecx,edx
L13:
	lea		edx,[010H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L14
	add		eax,010H
	sub		ecx,edx
L14:
	lea		edx,[04H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L15
	add		eax,04H
	sub		ecx,edx
L15:
	lea		edx,[01H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L16
	inc		eax
	sub		ecx,edx
L16:
	cmp		ecx,eax
	jbe		L17
	inc		eax
L17:
	ret
	}
}
#endif

/**********************************

	Calculate the square root of a 32 bit unsigned
	value. The input and output are 16.16 fixed
	point. This routine is 100% accurate.

**********************************/

asm Word32 BURGERCALL IMFixSqrt(Word32 Input)
{
	asm {
	mov		ecx,[esp+4]
	xor		eax,eax
	cmp		ecx,040000000H
	jb		L1
	mov		eax,040000000H
	sub		ecx,eax
L1:
	lea		edx,[010000000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L2
	add		eax,010000000H
	sub		ecx,edx
L2:
	lea		edx,[04000000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L3
	add		eax,04000000H
	sub		ecx,edx
L3:
	lea		edx,[01000000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L4
	add		eax,01000000H
	sub		ecx,edx
L4:
	lea		edx,[0400000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L5
	add		eax,0400000H
	sub		ecx,edx
L5:
	lea		edx,[0100000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L6
	add		eax,0100000H
	sub		ecx,edx
L6:
	lea		edx,[040000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L7
	add		eax,040000H
	sub		ecx,edx
L7:
	lea		edx,[010000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L8
	add		eax,010000H
	sub		ecx,edx
L8:
	lea		edx,[04000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L9
	add		eax,04000H
	sub		ecx,edx
L9:
	lea		edx,[01000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L10
	add		eax,01000H
	sub		ecx,edx
L10:
	lea		edx,[0400H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L11
	add		eax,0400H
	sub		ecx,edx
L11:
	lea		edx,[0100H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L12
	add		eax,0100H
	sub		ecx,edx
L12:
	lea		edx,[040H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L13
	add		eax,040H
	sub		ecx,edx
L13:
	lea		edx,[010H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L14
	add		eax,010H
	sub		ecx,edx
L14:
	lea		edx,[04H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L15
	add		eax,04H
	sub		ecx,edx
L15:
	lea		edx,[01H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L16
	inc		eax
	sub		ecx,edx
L16:
	shl		eax,16
	shl		ecx,16
	lea		edx,[04000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L17
	add		eax,04000H
	sub		ecx,edx
L17:
	lea		edx,[01000H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L18
	add		eax,01000H
	sub		ecx,edx
L18:
	lea		edx,[0400H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L19
	add		eax,0400H
	sub		ecx,edx
L19:
	lea		edx,[0100H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L20
	add		eax,0100H
	sub		ecx,edx
L20:
	lea		edx,[040H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L21
	add		eax,040H
	sub		ecx,edx
L21:
	lea		edx,[010H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L22
	add		eax,010H
	sub		ecx,edx
L22:
	lea		edx,[04H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L23
	add		eax,04H
	sub		ecx,edx
L23:
	lea		edx,[01H+eax]
	shr		eax,1
	cmp		ecx,edx
	jb		L24
	inc		eax
	sub		ecx,edx
L24:
	cmp		ecx,eax
	jbe		L25
	inc		eax
L25:
	ret
	}
}

/**********************************

	Multiply two 32 bit values and divide
	the 64 bit result by a thrid value. This way I
	get a high accuracy ratio of Value*(Numerator/Denominator)

**********************************/

asm long BURGERCALL IMIntMulRatioFast(long Mul1,long Mul2,long Div)
{
	asm {
	mov		eax,[esp+4]
	mov		edx,[esp+8]
	mov		ecx,[esp+12]
	imul	edx
	idiv	ecx
	ret
	}
}

/**********************************

	Multiply two 16.16 fixed point numbers

**********************************/

asm Fixed32 BURGERCALL IMFixMul(Fixed32 Val1,Fixed32 Val2)
{
	asm {
	mov 	eax,[esp+4]
	mov		edx,[esp+8]
	imul	edx
	cmp		edx,0x8000
	jge		Maximum
	cmp		edx,0xFFFF8000
	jl		Minimum
	shrd	eax,edx,16
	ret
Maximum:
	mov		eax,0x7FFFFFFF
	ret
Minimum:
	mov		eax,0x80000000
	ret
	}
}

/**********************************

	Multiply two 16.16 fixed point numbers
	with no error checking

**********************************/

asm Fixed32 BURGERCALL IMFixMulFast(Fixed32 Val1,Fixed32 Val2)
{
	asm {
	mov 	eax,[esp+4]
	mov		edx,[esp+8]
	imul	edx
	shrd	eax,edx,16
	ret
	}
}

/***************************

	Divide two fixed point numbers and return a fixed point result

***************************/

asm Fixed32 BURGERCALL IMFixDiv(Fixed32 Numerator,Fixed32 Denominator)
{
	asm {
	mov 	ecx,[esp+4]
	mov		edx,[esp+8]
	push	ebx
	push	ebp
	mov		eax,ecx
	mov		ebp,eax
	xor		ebp,edx
	mov		ecx,eax
	sar		ecx,31
	mov		ebx,edx
	sar		ebx,31
	xor		eax,ecx
	xor		edx,ebx
	sub		eax,ecx
	sub		edx,ebx
	mov		ecx,eax
	shr		ecx,15
	mov		ebx,edx
	cmp		ecx,edx
	jae		Overflow
	mov		edx,eax
	shl		eax,16
	shr		edx,16
	div		ebx
	cmp		ebp,0
	js		NegIt
	pop		ebp
	pop		ebx
	ret
NegIt:
	pop		ebp
	pop		ebx
	neg		eax
	ret
Overflow:
	cmp		ebp,0
	js		Minimum
Maximum:
	pop		ebp
	pop		ebx
	mov		eax,0x7FFFFFFF
	ret
Minimum:
	pop		ebp
	pop		ebx
	mov		eax,0x80000000
	ret
	}
}

/***************************

	Divide two fixed point numbers and return a fixed point result
	no error checking

***************************/

asm Fixed32 BURGERCALL IMFixDivFast(Fixed32 Numerator,Fixed32 Denominator)
{
	asm {
	mov 	eax,[esp+4]
	shl		eax,16
	mov		edx,[esp+4]
	sar		edx,16
	mov		ecx,[esp+8]
	idiv	ecx
	ret
	}
}

/***************************

	Return the reciprocal of a fixed point number

***************************/

asm Fixed32 BURGERCALL IMFixReciprocal(Fixed32 Input)
{
	asm {
	mov 	ecx,[esp+4]
	xor		eax,eax
	cmp		ecx,-1
	jz		Minimum
	cmp		ecx,2
	jb		Zero
	mov		edx,1
	idiv	ecx
	ret
Zero:
	mov		eax,0x7FFFFFFF
	ret
Minimum:
	mov		eax,0x80000000
	ret
	}
}

/**********************************

	Multiply two 2.30 fixed point numbers

**********************************/

asm Frac32 BURGERCALL IMFracMul(Frac32 Val1,Frac32 Val2)
{
	asm {
	mov 	eax,[esp+4]
	mov		edx,[esp+8]
	imul	edx
	cmp		edx,0x20000000
	jge		Maximum
	cmp		edx,0xE0000000
	jl		Minimum
	shrd	eax,edx,30
	ret
Maximum:
	mov		eax,0x7FFFFFFF
	ret
Minimum:
	mov		eax,0x80000000
	ret
	}
}

/***************************

	Divide two 2:30 fixed point numbers and return a fixed point result

***************************/

asm Frac32 BURGERCALL IMFracDiv(Frac32 Numerator,Frac32 Denominator)
{
	asm {
	mov 	ecx,[esp+4]
	mov		edx,[esp+8]
	push	ebx
	push	ebp
	mov		eax,ecx
	mov		ebp,eax
	xor		ebp,edx
	mov		ecx,eax
	sar		ecx,31
	mov		ebx,edx
	sar		ebx,31
	xor		eax,ecx
	xor		edx,ebx
	sub		eax,ecx
	sub		edx,ebx
	mov		ecx,eax
	shr		ecx,1
	mov		ebx,edx
	cmp		ecx,edx
	jae		Overflow
	mov		edx,eax
	shl		eax,30
	shr		edx,2
	div		ebx
	cmp		ebp,0
	js		NegIt
	pop		ebp
	pop		ebx
	ret
NegIt:
	pop		ebp
	pop		ebx
	neg		eax
	ret
Overflow:
	cmp		ebp,0
	js		Minimum
Maximum:
	pop		ebp
	pop		ebx
	mov		eax,0x7FFFFFFF
	ret
Minimum:
	pop		ebp
	pop		ebx
	mov		eax,0x80000000
	ret
	}
}

#endif
