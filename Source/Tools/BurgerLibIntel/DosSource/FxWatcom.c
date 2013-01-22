/***************************

	Watcom "C" version

***************************/

#include "FxFixed.h"

#if defined(__WATCOMC__)

/******************************

	Mul two pairs of 32 bit numbers and return the upper 32 bits

******************************/

extern long BURGERCALL FooIntDblMulAdd(long Mul1,long Mul2,long Mul3,long Mul4);

#pragma aux FooIntDblMulAdd = \
	"imul	edx" \
	"push	edx" \
	"push	eax" \
	"mov	eax,ebx" \
	"imul	ecx" \
	"pop	ebx" \
	"pop	ecx" \
	"add	eax,ebx" \
	"adc	edx,ecx" \
	"mov	eax,edx" \
	parm [eax] [edx] [ebx] [ecx] \
	value [eax]

long BURGERCALL IntDblMulAdd(long Mul1,long Mul2,long Mul3,long Mul4)
{
	return FooIntDblMulAdd(Mul1,Mul2,Mul3,Mul4);
}

/**********************************

	Multiply two 16.16 fixed point numbers

**********************************/

extern Fixed32 FooMul(Fixed32 a,Fixed32 b);

#pragma aux FooMul = \
	"imul	edx" \
	"cmp	edx,08000H" \
	"jge	Maximum" \
	"cmp	edx,0FFFF8000H" \
	"jl		Minimum" \
	"shrd	eax,edx,16" \
	"ret" \
"Maximum:" \
	"mov	eax,07FFFFFFFH" \
	"ret" \
"Minimum:" \
	"mov	eax,080000000H" \
	parm [eax] [edx] \
	value [eax]

Fixed32 BURGERCALL IMFixMul(Fixed32 a,Fixed32 b)
{
	return FooMul(a,b);
}


/***************************

	Divide two fixed point numbers and return a fixed point result

***************************/

extern Fixed32 FooDiv(Fixed32 a,Fixed32 b);

#pragma aux FooDiv = \
	"push	ebx" \
	"push	ebp" \
	"push	ecx" \
	"mov	ebp,eax" \
	"xor	ebp,edx" \
	"mov	ecx,eax" \
	"sar	ecx,31" \
	"mov	ebx,edx" \
	"sar	ebx,31" \
	"xor	eax,ecx" \
	"xor	edx,ebx" \
	"sub	eax,ecx" \
	"sub	edx,ebx" \
	"mov	ecx,eax" \
	"shr	ecx,15" \
	"mov	ebx,edx" \
	"cmp	ecx,edx" \
	"jae	Overflow" \
	"mov	edx,eax" \
	"shl	eax,16" \
	"shr	edx,16" \
	"div	ebx" \
	"cmp	ebp,0" \
	"js		NegIt" \
	"pop	ecx" \
	"pop	ebp" \
	"pop	ebx" \
	"ret" \
"NegIt:" \
	"pop	ecx" \
	"pop	ebp" \
	"pop	ebx" \
	"neg	eax" \
	"ret" \
"Overflow:" \
	"cmp	ebp,0" \
	"js		Minimum" \
"Maximum:" \
	"pop	ecx" \
	"pop	ebp" \
	"pop	ebx" \
	"mov	eax,07FFFFFFFH" \
	"ret" \
"Minimum:" \
	"pop	ecx" \
	"pop	ebp" \
	"pop	ebx" \
	"mov	eax,080000000H" \
	parm [eax] [edx] \
	value [eax]

Fixed32 BURGERCALL IMFixDiv(Fixed32 Numerator,Fixed32 Denominator)
{
	return FooDiv(Numerator,Denominator);
}


/***************************

	Return the reciprocal of a fixed point number

***************************/

extern Fixed32 FooRecip(Fixed32 Input);

#pragma aux FooRecip = \
	"cmp	eax,-1" \
	"je		Min" \
	"cmp	eax,2" \
	"jb		Zero" \
	"push	edx" \
	"push	ecx" \
	"mov	ecx,eax" \
	"xor	eax,eax" \
	"mov	edx,1" \
	"idiv	ecx" \
	"pop	ecx" \
	"pop	edx" \
	"ret" \
"Zero:" \
	"mov	eax,0x7FFFFFFF" \
	"ret" \
"Min:" \
	"mov	eax,0x80000000" \
	parm [eax] \
	value [eax]

Fixed32 BURGERCALL IMFixReciprocal(Fixed32 Input)
{
	return FooRecip(Input);
}

/**********************************

	Multiply two 2.30 fixed point numbers

**********************************/

extern Fixed32 FooFMul(Frac32 a,Frac32 b);

#pragma aux FooFMul = \
	"imul	edx" \
	"cmp	edx,020000000H" \
	"jge	Maximum" \
	"cmp	edx,0E0000000H" \
	"jl		Minimum" \
	"shrd	eax,edx,30" \
	"ret" \
"Maximum:" \
	"mov	eax,07FFFFFFFH" \
	"ret" \
"Minimum:" \
	"mov	eax,080000000H" \
	parm [eax] [edx] \
	value [eax]

Fixed32 BURGERCALL IMFracMul(Frac32 a,Frac32 b)
{
	return FooFMul(a,b);
}


/***************************

	Divide two fixed point numbers and return a fixed point result

***************************/

extern Fixed32 FooFDiv(Frac32 a,Frac32 b);

#pragma aux FooFDiv = \
	"push	ebx" \
	"push	ebp" \
	"push	ecx" \
	"mov	ebp,eax" \
	"xor	ebp,edx" \
	"mov	ecx,eax" \
	"sar	ecx,31" \
	"mov	ebx,edx" \
	"sar	ebx,31" \
	"xor	eax,ecx" \
	"xor	edx,ebx" \
	"sub	eax,ecx" \
	"sub	edx,ebx" \
	"mov	ecx,eax" \
	"shr	ecx,1" \
	"mov	ebx,edx" \
	"cmp	ecx,edx" \
	"jae	Overflow" \
	"mov	edx,eax" \
	"shl	eax,30" \
	"shr	edx,2" \
	"div	ebx" \
	"cmp	ebp,0" \
	"js		NegIt" \
	"pop	ecx" \
	"pop	ebp" \
	"pop	ebx" \
	"ret" \
"NegIt:" \
	"pop	ecx" \
	"pop	ebp" \
	"pop	ebx" \
	"neg	eax" \
	"ret" \
"Overflow:" \
	"cmp	ebp,0" \
	"js		Minimum" \
"Maximum:" \
	"pop	ecx" \
	"pop	ebp" \
	"pop	ebx" \
	"mov	eax,07FFFFFFFH" \
	"ret" \
"Minimum:" \
	"pop	ecx" \
	"pop	ebp" \
	"pop	ebx" \
	"mov	eax,080000000H" \
	parm [eax] [edx] \
	value [eax]

Fixed32 BURGERCALL IMFracDiv(Frac32 Numerator,Frac32 Denominator)
{
	return FooFDiv(Numerator,Denominator);
}

/**********************************

	Calculate the square root of a 32 bit unsigned
	value. The maximum value is 46341 for the square root
	of 0x7FFFFFFF. This routine is 100% accurate.

**********************************/

extern Word FooIntSqrt(Word32 A);

#if 0
#pragma aux FooIntSqrt = \
"push eax" \
"fild DWORD PTR [esp]" \
"fsqrt" \
"fistp DWORD PTR [esp]" \
"pop eax" \
parm[eax] value[eax] modify exact [eax]
#else
#pragma aux FooIntSqrt = \
	"push		ecx" \
	"mov		ecx,eax" \
	"push		edx" \
	"xor		eax,eax" \
	"cmp		ecx,040000000H" \
	"jb		L1" \
	"mov		eax,040000000H" \
	"sub		ecx,eax" \
"L1:" \
	"lea		edx,[010000000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L2" \
	"add		eax,010000000H" \
	"sub		ecx,edx" \
"L2:" \
	"lea		edx,[04000000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L3" \
	"add		eax,04000000H" \
	"sub		ecx,edx" \
"L3:" \
	"lea		edx,[01000000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L4" \
	"add		eax,01000000H" \
	"sub		ecx,edx" \
"L4:" \
	"lea		edx,[0400000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L5" \
	"add		eax,0400000H" \
	"sub		ecx,edx" \
"L5:" \
	"lea		edx,[0100000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L6" \
	"add		eax,0100000H" \
	"sub		ecx,edx" \
"L6:" \
	"lea		edx,[040000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L7" \
	"add		eax,040000H" \
	"sub		ecx,edx" \
"L7:" \
	"lea		edx,[010000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L8" \
	"add		eax,010000H" \
	"sub		ecx,edx" \
"L8:" \
	"lea		edx,[04000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L9" \
	"add		eax,04000H" \
	"sub		ecx,edx" \
"L9:" \
	"lea		edx,[01000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L10" \
	"add		eax,01000H" \
	"sub		ecx,edx" \
"L10:" \
	"lea		edx,[0400H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L11" \
	"add		eax,0400H" \
	"sub		ecx,edx" \
"L11:" \
	"lea		edx,[0100H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L12" \
	"add		eax,0100H" \
	"sub		ecx,edx" \
"L12:" \
	"lea		edx,[040H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L13" \
	"add		eax,040H" \
	"sub		ecx,edx" \
"L13:" \
	"lea		edx,[010H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L14" \
	"add		eax,010H" \
	"sub		ecx,edx" \
"L14:" \
	"lea		edx,[04H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L15" \
	"add		eax,04H" \
	"sub		ecx,edx" \
"L15:" \
	"lea		edx,[01H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L16" \
	"inc		eax" \
	"sub		ecx,edx" \
"L16:" \
	"cmp		ecx,eax" \
	"jbe		L17" \
	"inc		eax" \
"L17:" \
	"pop		edx" \
	"pop		ecx" \
	parm [eax] value [eax] modify exact [eax]
#endif

Word BURGERCALL IMIntSqrt(Word32 Input)
{
	return FooIntSqrt(Input);
}

/**********************************

	Calculate the square root of a 32 bit unsigned
	value. The input and output are 16.16 fixed
	point. This routine is 100% accurate.

**********************************/

extern Word32 FooFixSqrt(Word32 A);

#pragma aux FooFixSqrt = \
	"push		ecx" \
	"mov		ecx,eax" \
	"push		edx" \
	"xor		eax,eax" \
	"cmp		ecx,040000000H" \
	"jb		L1" \
	"mov		eax,040000000H" \
	"sub		ecx,eax" \
"L1:" \
	"lea		edx,[010000000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L2" \
	"add		eax,010000000H" \
	"sub		ecx,edx" \
"L2:" \
	"lea		edx,[04000000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L3" \
	"add		eax,04000000H" \
	"sub		ecx,edx" \
"L3:" \
	"lea		edx,[01000000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L4" \
	"add		eax,01000000H" \
	"sub		ecx,edx" \
"L4:" \
	"lea		edx,[0400000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L5" \
	"add		eax,0400000H" \
	"sub		ecx,edx" \
"L5:" \
	"lea		edx,[0100000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L6" \
	"add		eax,0100000H" \
	"sub		ecx,edx" \
"L6:" \
	"lea		edx,[040000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L7" \
	"add		eax,040000H" \
	"sub		ecx,edx" \
"L7:" \
	"lea		edx,[010000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L8" \
	"add		eax,010000H" \
	"sub		ecx,edx" \
"L8:" \
	"lea		edx,[04000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L9" \
	"add		eax,04000H" \
	"sub		ecx,edx" \
"L9:" \
	"lea		edx,[01000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L10" \
	"add		eax,01000H" \
	"sub		ecx,edx" \
"L10:" \
	"lea		edx,[0400H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L11" \
	"add		eax,0400H" \
	"sub		ecx,edx" \
"L11:" \
	"lea		edx,[0100H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L12" \
	"add		eax,0100H" \
	"sub		ecx,edx" \
"L12:" \
	"lea		edx,[040H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L13" \
	"add		eax,040H" \
	"sub		ecx,edx" \
"L13:" \
	"lea		edx,[010H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L14" \
	"add		eax,010H" \
	"sub		ecx,edx" \
"L14:" \
	"lea		edx,[04H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L15" \
	"add		eax,04H" \
	"sub		ecx,edx" \
"L15:" \
	"lea		edx,[01H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L16" \
	"inc		eax" \
	"sub		ecx,edx" \
"L16:" \
	"shl		eax,16" \
	"shl		ecx,16" \
	"lea		edx,[04000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L17" \
	"add		eax,04000H" \
	"sub		ecx,edx" \
"L17:" \
	"lea		edx,[01000H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L18" \
	"add		eax,01000H" \
	"sub		ecx,edx" \
"L18:" \
	"lea		edx,[0400H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L19" \
	"add		eax,0400H" \
	"sub		ecx,edx" \
"L19:" \
	"lea		edx,[0100H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L20" \
	"add		eax,0100H" \
	"sub		ecx,edx" \
"L20:" \
	"lea		edx,[040H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L21" \
	"add		eax,040H" \
	"sub		ecx,edx" \
"L21:" \
	"lea		edx,[010H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L22" \
	"add		eax,010H" \
	"sub		ecx,edx" \
"L22:" \
	"lea		edx,[04H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L23" \
	"add		eax,04H" \
	"sub		ecx,edx" \
"L23:" \
	"lea		edx,[01H+eax]" \
	"shr		eax,1" \
	"cmp		ecx,edx" \
	"jb		L24" \
	"inc		eax" \
	"sub		ecx,edx" \
"L24:" \
	"cmp		ecx,eax" \
	"jbe		L25" \
	"inc		eax" \
"L25:" \
	"pop		edx" \
	"pop		ecx" \
	parm [eax] value [eax] modify exact [eax]

Word32 BURGERCALL IMFixSqrt(Word32 Input)
{
	return FooFixSqrt(Input);
}

#endif
