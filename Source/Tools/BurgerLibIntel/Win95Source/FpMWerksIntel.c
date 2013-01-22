/**********************************

	Metrowerks "C" for intel version

**********************************/

#include "FpFloat.h"

#if defined(__MWERKS__) && defined(__INTEL__)

static float Zero = 0;		/* Floating point constants to use */
static float One = 1.0f;	/* Floating point 1.0 */
static float Num1024 = 1024.0f;

/**********************************

	Create a square root but I have about a 1%
	margin of error for rounding

**********************************/

asm float BURGERCALL SqrtFast(float n)
{
	asm {
	mov eax,[esp+4]

	shr	eax,23-9-2
	mov	edx,[esp+4]

	and eax,0xFFC
	and edx,0x7FFFFFFF

	jz	L1

	mov eax,BurgerSqrtTable[eax]
	and edx,0x7F000000

	add eax,edx
	shr eax,1
	mov [esp+4],eax
	fld	dword ptr [esp+4]
	ret
L1:
	fld	dword ptr [Zero]
	ret
	}
}

/**********************************

	Return a floating point number rounded up

**********************************/

asm float BURGERCALL CeilingFast(float n)
{
	asm {
	mov		eax,[esp+4]
	fld		dword ptr [esp+4]
	and		eax,0x7FFFFFFF
	fist	dword ptr [esp+4]
	cmp		eax,0x4B000000
	jae		L0
	fild	dword ptr [esp+4]
	fxch	st(1)
	xor		eax,eax
	fcomp	st(1)
	fnstsw	ax
	and		eax,0x4100
	jz		L1
L0:
	ret
L1:
	fadd	dword ptr [One]
	ret
	}
}

/**********************************

	Return a floating point number rounded up

**********************************/

asm int BURGERCALL CeilingFastInt(float n)
{
	asm {
	mov		eax,[esp+4]
	fld		dword ptr [esp+4]
	cmp		eax,0x4F000000
	fist	dword ptr [esp+4]
	mov		eax,0x7FFFFFFF
	jge		L0
	fild	dword ptr [esp+4]
	xor		eax,eax
	fcompp
	fnstsw	ax
	and		eax,0x100
	mov		eax,[esp+4]
	jnz		L1
L0:
	ret
L1:
	inc		eax
	ret
	}
}

/**********************************

	Return a floating point number floored

**********************************/

asm float BURGERCALL FloorFast(float n)
{
	asm {
	mov		eax,[esp+4]
	fld		dword ptr [esp+4]
	and		eax,0x7FFFFFFF
	fist	dword ptr [esp+4]
	cmp		eax,0x4B000000
	jae		L0
	fild	dword ptr [esp+4]
	fxch	st(1)
	xor		eax,eax
	fcomp	st(1)
	fnstsw	ax
	and		eax,0x100
	jnz		L1
L0:
	ret
L1:
	fsub	dword ptr [One]
	ret
	}
}

/**********************************

	Return a floating point number floored

**********************************/

asm int BURGERCALL FloorFastInt(float n)
{
	asm {
	mov		eax,[esp+4]
	fld		dword ptr [esp+4]
	cmp		eax,0x4F000000
	fist	dword ptr [esp+4]
	mov		eax,0x7FFFFFFF
	jge		L0
	fild	dword ptr [esp+4]
	xor		eax,eax
	fcompp
	fnstsw	ax
	and		eax,0x4100
	mov		eax,[esp+4]
	jz		L1
	ret
L1:
	cmp		eax,0x80000000
	jz		L0
	dec		eax
L0:
	ret
	}
}

/**********************************

	Convert a floating point number to an integer
	but use round to nearest

**********************************/

asm int BURGERCALL FloatToInt(float Input)
{
	asm {
	cmp		dword ptr [esp+4],0x4F000000
	mov		eax,0x7FFFFFFF
	jge		L1
	fld		dword ptr [esp+4]
	fistp	dword ptr [esp+4]
	mov		eax,[esp+4]
L1:
	ret
	}
}

/**********************************

	Given an angle of the range
	of ANGLERANGE (ANGLERANGE==360.0 degrees)
	return the sine.

**********************************/

asm float BURGERCALL FloatSine(float Angle)
{
	asm {
	mov		eax,[esp+4]
	fld		dword ptr [esp+4]
	cmp		eax,0x4F000000
	fistp	dword ptr [esp+4]
	mov		eax,0x7FF
	jge		L1
	mov		eax,[esp+4]
	and		eax,0x7ff
L1:
	fld		dword ptr [FSineTable + eax*4]
	ret
	}
}

/**********************************

	Given an angle of the range
	of ANGLERANGE (ANGLERANGE==360.0 degrees)
	return the cosine.

**********************************/

asm float BURGERCALL FloatCosine(float Angle)
{
	asm {
	mov		eax,[esp+4]
	fld		dword ptr [esp+4]
	cmp		eax,0x4F000000
	fistp	dword ptr [esp+4]
	mov		eax,0x7FF
	jge		L1
	mov		eax,[esp+4]
	and		eax,0x7ff
L1:
	fld		dword ptr [FSineTable+2048 + eax*4]
	ret
	}
}

/**********************************

	Take a floating point cosine and return
	the fixed point angle (Range 0-1024 fixed)

**********************************/

asm Fixed32 BURGERCALL FloatArcCosine(float c)
{
	asm {
	fld		dword ptr[esp+4]
	fmul	dword ptr[Num1024]
	fistp	dword ptr[esp+4]
	mov		eax,[esp+4]
	cmp		eax,0xFFFFFC00
	jle		L4
	cmp		eax,0x00000400
	jge		L3
	mov		eax,dword ptr [FixedArcCosineTable+4096+eax*4]
	ret
L3:
	xor		eax,eax
	ret
L4:
	mov		eax,0x04000000
	ret
	}
}

/**********************************

	Take a fixed point sine and return
	the fixed point angle (Range 1536-2047/0-512 fixed)

**********************************/

asm Fixed32 BURGERCALL FloatArcSine(float c)
{
	asm {
	fld		dword ptr[esp+4]
	fmul	dword ptr[Num1024]
	fistp	dword ptr[esp+4]
	mov		eax,[esp+4]
	cmp		eax,0xFFFFFC00
	jle		L4
	cmp		eax,0x00000400
	jge		L3
	mov		eax,dword ptr [FixedArcSineTable+4096+eax*4]
	ret
L3:
	mov		eax,0x02000000
	ret
L4:
	mov		eax,0x06000000
	ret
	}
}

/**********************************

	Return a dot product of two 3D vectors

**********************************/

#if 0
asm float BURGERCALL Vector3DDot(const Vector3D_t *Input1,const Vector3D_t *Input2)
{
	asm {
	mov		eax,[esp+4]
	mov		edx,[esp+8]
	fld		dword ptr [eax+0]
	fmul	dword ptr [edx+0]
	fld		dword ptr [eax+4]
	fmul	dword ptr [edx+4]
	fld		dword ptr [eax+8]
	fmul	dword ptr [edx+8]
	fxch	st(1)
	faddp	st(2),st(0)
	faddp	st(1),st(0)
	ret
	}
}
#endif

/**********************************

	Return a cross product of two 3D vectors

**********************************/

asm void BURGERCALL Vector3DCross(Vector3D_t *Output,const Vector3D_t *Input1,const Vector3D_t *Input2)
{
	asm {
	mov		ecx,[esp+4]
	mov		edx,[esp+8]
	mov		eax,[esp+12]
	fld		dword ptr [edx+4]
	fmul	dword ptr [eax+8]
	fld		dword ptr [edx+8]
	fmul	dword ptr [eax+0]
	fld		dword ptr [edx+0]
	fmul	dword ptr [eax+4]
	fld		dword ptr [edx+8]
	fmul	dword ptr [eax+4]
	fld		dword ptr [edx+0]
	fmul	dword ptr [eax+8]
	fld		dword ptr [edx+4]
	fmul	dword ptr [eax+0]
	fxch	st(2)
	fsubp	st(5),st
	fsubp	st(3),st
	fsubp	st(1),st
	fxch	st(2)
	fstp	dword ptr [ecx+0]
	fstp	dword ptr [ecx+4]
	fstp	dword ptr [ecx+8]
	ret
	}
}

/**********************************

	Returns the square of the radius of a 3D vector

**********************************/

asm float BURGERCALL Vector3DGetRadiusSqr(const Vector3D_t *Input)
{
	asm {
	mov		eax,[esp+4]		;Get the pointer
	fld		dword ptr [eax+0]
	fmul	st,st
	fld		dword ptr [eax+4]
	fmul	st,st
	fld		dword ptr [eax+8]
	fmul	st,st
	fxch	st(1)
	faddp	st(2), st(0)
	faddp	st(1), st(0)	;The only stall
	ret
	}
}

/**********************************

	Normalize a 3D vector
	(Set its distance to 1.0)
	(High precision)

**********************************/

asm void BURGERCALL Vector3DNormalize(Vector3D_t *Input)
{
	asm {
	mov ecx,[esp+4]
	fld dword ptr [ecx]
	fmul st,st
	fld dword ptr [ecx+4]
	fmul st,st
	fld dword ptr [ecx+8]
	fmul st,st
	fxch st(1)
	faddp st(2),st(0)
	faddp st(1),st(0)
	fsqrt
	fld dword ptr [One]
	fxch st(1)
	fstp dword ptr [esp+4]
	mov eax,[esp+4]
	test eax,0x7FFFFFFF
	jz NoGood
	fdiv dword ptr [esp+4]
	fld dword ptr [ecx+0]
	fld dword ptr [ecx+4]
	fld dword ptr [ecx+8]
	fxch st(2)
	fmul st(0),st(3)
	fxch st(1)
	fmul st(0),st(3)
	fxch st(2)
	fmulp st(3),st(0)
	fstp dword ptr [ecx+0]
	fstp dword ptr [ecx+4]
	fstp dword ptr [ecx+8]
	ret
NoGood:
	fstp st(0)
	ret
	}
}

/**********************************

	Normalize a 3D vector
	(Set its distance to 1.0)
	(High precision)

**********************************/

asm void BURGERCALL Vector3DNormalize2(Vector3D_t *Output,const Vector3D_t *Input)
{
	asm {
	mov ecx,[esp+4]
	mov edx,[esp+8]
	fld dword ptr [edx]
	fmul st,st
	fld dword ptr [edx+4]
	fmul st,st
	fld dword ptr [edx+8]
	fmul st,st
	fxch st(1)
	faddp st(2),st(0)
	faddp st(1),st(0)
	fsqrt
	fld dword ptr [One]
	fxch st(1)
	fstp dword ptr [esp+4]
	mov eax,[esp+4]
	test eax,0x7FFFFFFF
	jz NoGood
	fdiv dword ptr [esp+4]
	fld dword ptr [edx+0]
	fld dword ptr [edx+4]
	fld dword ptr [edx+8]
	fxch st(2)
	fmul st(0),st(3)
	fxch st(1)
	fmul st(0),st(3)
	fxch st(2)
	fmulp st(3),st(0)
	fstp dword ptr [ecx+0]
	fstp dword ptr [ecx+4]
	fstp dword ptr [ecx+8]
	ret
NoGood:
	fstp st(0)
	ret
	}
}


/**********************************

	Assembly for Matrix code

**********************************/

#define XX (0)
#define XY 4
#define XZ 8
#define YX 12
#define YY 16
#define YZ 20
#define ZX 24
#define ZY 28
#define ZZ 32

/**********************************

	Create a rotation matrix with angles for
	the yaw (X), pitch (Y), and roll (Z)

**********************************/

asm void BURGERCALL Matrix3DSet(Matrix3D_t *Output,float yaw,float pitch,float roll)
{
	asm {
	fld		dword ptr [esp+8]
	fld		dword ptr [esp+12]
	fld		dword ptr [esp+16]
	mov		ecx,[esp+4]
	push	ebp
	push	ebx
	fistp	dword ptr [esp+12]
	fistp	dword ptr [esp+16]
	mov		ebp,[esp+12]
	mov		ebx,[esp+16]
	fistp	dword ptr [esp+20]
	mov		edx,[esp+20]
	and		ebp,0x7FF
	and		ebx,0x7FF
	and		edx,0x7FF

	fld		dword ptr [FSineTable + ebp*4]	;sroll * spitch
	fmul	dword ptr [FSineTable + ebx*4]
	fld		dword ptr [FSineTable + 2048 + ebp*4]	;croll* spitch
	fmul	dword ptr [FSineTable + ebx*4]

	fld		dword ptr [FSineTable + 2048 + ebp*4]	;croll * cyaw
	fmul	dword ptr [FSineTable + 2048 + edx*4]
	fld		dword ptr [FSineTable + ebp*4]	;sroll * cyaw
	fmul	dword ptr [FSineTable + 2048 + edx*4]
	fld		dword ptr [FSineTable + 2048 + ebx*4]	;cpitch * syaw
	fmul	dword ptr [FSineTable + edx*4]

	fld		dword ptr [FSineTable + edx*4]	;(sroll * spitch) * syaw
	fmul	st,st(5)
	fld		dword ptr [FSineTable + edx*4]	;(croll * spitch) * syaw
	fmul	st,st(5)
	fxch	st(2)
	fstp	dword ptr [ecx+XZ]
	faddp	st(3),st
	fsubp	st(1),st

	fld		dword ptr [FSineTable + ebp*4]		;sroll * cpitch
	fmul	dword ptr [FSineTable + 2048 + ebx*4]
	fxch	st(2)
	fstp	dword ptr [ecx+XX]
	fstp	dword ptr [ecx+XY]
	fchs
	fld		dword ptr [FSineTable + 2048 + ebp*4]	;croll*cpitch
	fmul	dword ptr [FSineTable + 2048 + ebx*4]
	fxch	st(1)
	fstp	dword ptr [ecx+YX]

	fld		dword ptr [FSineTable + 2048 + ebp*4]	;croll * syaw
	fmul	dword ptr [FSineTable + edx*4]
	fxch	st(1)
	fstp	dword ptr [ecx+YY]
	fld		dword ptr [FSineTable + ebp*4]	;sroll*syaw
	fmul	dword ptr [FSineTable + edx*4]
	fld		dword ptr [FSineTable + 2048 + ebx*4]	;cpitch * cyaw
	fmul	dword ptr [FSineTable + 2048 + edx*4]

	fld		dword ptr [FSineTable + 2048 + edx*4]
	fmulp	st(5),st
	fld		dword ptr [FSineTable + 2048 + edx*4]
	fmulp	st(4),st
	fstp	dword ptr [ecx+ZZ]
	fchs
	mov		ebx,[FSineTable+ebx*4]
	fsubrp	st(2),st
	fsubp	st(2),st
	mov		[ecx+YZ],ebx
	fstp	dword ptr [ecx+ZY]
	fstp	dword ptr [ecx+ZX]
	pop		ebx
	pop		ebp
	ret
	}
}

/**********************************

	Create a rotation matrix with angle for
	yaw (X)

**********************************/

asm void BURGERCALL Matrix3DSetYaw(Matrix3D_t *Output,float yaw)
{
	asm {
	mov		ecx,[esp+4]
	fld		dword ptr [esp+8]
	fistp	dword ptr [esp+8]
	mov		edx,[esp+8]
	xor		eax,eax
	and		edx,0x7FF
	mov		[ecx+XY],eax
	mov		[ecx+YX],eax
	mov		[ecx+YZ],eax
	fld		dword ptr [FSineTable + edx*4]
	mov		[ecx+ZY],eax
	mov		eax,[FSineTable + edx*4]
	fchs
	mov		[ecx+XZ],eax
	mov		eax,0x3F800000
	mov		edx,[FSineTable+2048 + edx*4]
	mov		[ecx+YY],eax
	fstp	dword ptr [ecx+ZX]
	mov		dword ptr [ecx+XX],edx
	mov		dword ptr [ecx+ZZ],edx
	ret
	}
}

/**********************************

	Create a rotation matrix with angle for
	pitch (Y)

**********************************/

asm void BURGERCALL Matrix3DSetPitch(Matrix3D_t *Output,float pitch)
{
	asm {
	mov		ecx,[esp+4]
	fld		dword ptr [esp+8]
	fistp	dword ptr [esp+8]
	mov		edx,[esp+8]
	xor		eax,eax
	and		edx,0x7FF
	mov		[ecx+XY],eax
	mov		[ecx+XZ],eax
	mov		[ecx+YX],eax
	fld		dword ptr [FSineTable + edx*4]
	mov		[ecx+ZX],eax
	mov		eax,[FSineTable + edx*4]
	fchs
	mov		[ecx+YZ],eax
	mov		eax,0x3F800000
	mov		edx,[FSineTable+2048 + edx*4]
	mov		[ecx+XX],eax
	fstp	dword ptr [ecx+ZY]
	mov		dword ptr [ecx+YY],edx
	mov		dword ptr [ecx+ZZ],edx
	ret
	}
}

/**********************************

	Create a rotation matrix with angle for
	roll (Z)

**********************************/

asm void BURGERCALL Matrix3DSetRoll(Matrix3D_t *Output,float roll)
{
	asm {
	mov		ecx,[esp+4]
	fld		dword ptr [esp+8]
	fistp	dword ptr [esp+8]
	mov		edx,[esp+8]
	xor		eax,eax
	and		edx,0x7FF
	mov		[ecx+XZ],eax
	mov		[ecx+YZ],eax
	mov		[ecx+ZX],eax
	fld		dword ptr [FSineTable + edx*4]
	mov		[ecx+ZY],eax
	mov		eax,[FSineTable + edx*4]
	fchs
	mov		[ecx+XY],eax
	mov		eax,0x3F800000
	mov		edx,[FSineTable+2048 + edx*4]
	mov		[ecx+ZZ],eax
	fstp	dword ptr [ecx+YX]
	mov		dword ptr [ecx+XX],edx
	mov		dword ptr [ecx+YY],edx
	ret
	}
}

/**********************************

	Create a rotation matrix with angles for
	the yaw (Y), pitch (X), and roll (Z)
	The angles are ints from 0-2047 (Rounded to 2048)

	x.x = cy*cz + sx*sy*sz x.y = cy*sz - sx*sy*cz  x.z = cx*sy
	y.x = -cx*sz           y.y = cx*cz             y.z = sx
	z.x = -sy*cz+sx*cy*sz  z.y = sy*-sz + sx*cy*cz z.z = cx*cy

**********************************/

asm void BURGERCALL Matrix3DSetInt(Matrix3D_t *Output,Word yaw,Word pitch,Word roll)
{
	asm {
	mov		ecx,[esp+4]
	mov		edx,[esp+8]
	push	ebx
	push	ebp
	mov		ebx,[esp+20]
	mov		ebp,[esp+24]
	and		edx,0x7FF
	and		ebx,0x7FF
	and		ebp,0x7FF

	fld		dword ptr [FSineTable + ebp*4]	;sroll * spitch
	fmul	dword ptr [FSineTable + ebx*4]
	fld		dword ptr [FSineTable + 2048 + ebp*4]	;croll* spitch
	fmul	dword ptr [FSineTable + ebx*4]

	fld		dword ptr [FSineTable + 2048 + ebp*4]	;croll * cyaw
	fmul	dword ptr [FSineTable + 2048 + edx*4]
	fld		dword ptr [FSineTable + ebp*4]	;sroll * cyaw
	fmul	dword ptr [FSineTable + 2048 + edx*4]
	fld		dword ptr [FSineTable + 2048 + ebx*4]	;cpitch * syaw
	fmul	dword ptr [FSineTable + edx*4]

	fld		dword ptr [FSineTable + edx*4]	;(sroll * spitch) * syaw
	fmul	st,st(5)
	fld		dword ptr [FSineTable + edx*4]	;(croll * spitch) * syaw
	fmul	st,st(5)
	fxch	st(2)
	fstp	dword ptr [ecx+XZ]
	faddp	st(3),st
	fsubp	st(1),st

	fld		dword ptr [FSineTable + ebp*4]		;sroll * cpitch
	fmul	dword ptr [FSineTable + 2048 + ebx*4]
	fxch	st(2)
	fstp	dword ptr [ecx+XX]
	fstp	dword ptr [ecx+XY]
	fchs
	fld		dword ptr [FSineTable + 2048 + ebp*4]	;croll*cpitch
	fmul	dword ptr [FSineTable + 2048 + ebx*4]
	fxch	st(1)
	fstp	dword ptr [ecx+YX]

	fld		dword ptr [FSineTable + 2048 + ebp*4]	;croll * syaw
	fmul	dword ptr [FSineTable + edx*4]
	fxch	st(1)
	fstp	dword ptr [ecx+YY]
	fld		dword ptr [FSineTable + ebp*4]	;sroll*syaw
	fmul	dword ptr [FSineTable + edx*4]
	fld		dword ptr [FSineTable + 2048 + ebx*4]	;cpitch * cyaw
	fmul	dword ptr [FSineTable + 2048 + edx*4]

	fld		dword ptr [FSineTable + 2048 + edx*4]
	fmulp	st(5),st
	fld		dword ptr [FSineTable + 2048 + edx*4]
	fmulp	st(4),st
	fstp	dword ptr [ecx+ZZ]
	fchs
	mov		ebx,[FSineTable+ebx*4]
	fsubrp	st(2),st
	fsubp	st(2),st
	mov		[ecx+YZ],ebx
	fstp	dword ptr [ecx+ZY]
	fstp	dword ptr [ecx+ZX]
	pop		ebp
	pop		ebx
	ret
	}
}

/**********************************

	Perform a matrix multiply on the output matrix

**********************************/

asm void BURGERCALL Matrix3DMul(Matrix3D_t *Output,const Matrix3D_t *Input)
{
	asm {
	mov		ecx,[esp+4]
	mov		edx,[esp+8]
	sub		esp,48
	mov		eax,[ecx+0]
	mov		[esp+12],eax
	mov		eax,[ecx+4]
	mov		[esp+16],eax
	mov		eax,[ecx+8]
	mov		[esp+20],eax
	mov		eax,[ecx+12]
	mov		[esp+24],eax
	mov		eax,[ecx+16]
	mov		[esp+28],eax
	mov		eax,[ecx+20]
	mov		[esp+32],eax
	mov		eax,[ecx+24]
	mov		[esp+36],eax
	mov		eax,[ecx+28]
	mov		[esp+40],eax
	mov		eax,[ecx+32]
	mov		[esp+44],eax
	mov		[esp+8],edx
	lea		edx,[esp+12]
	mov		[esp+0],ecx
	mov		[esp+4],edx
	call	Matrix3DMul2
	mov		eax,[esp+48]
	add		esp,52
	jmp		eax
	}
}

/**********************************

	Perform a matrix multiply
	Floating point will rot your brain on the Pentium

**********************************/

#define Output ECX
#define Input1 EDX
#define Input2 EAX

asm void BURGERCALL Matrix3DMul2(Matrix3D_t *Output,const Matrix3D_t *Input1,const Matrix3D_t *Input2)
{
	asm {

	mov	Output,[esp+4]
	mov	Input1,[esp+8]
	mov	Input2,[ESP+12]

;
; First I do the Muls for XX
;

	fld	dword ptr [Input1+XX]	;XX
	fmul	dword ptr [Input2+XX]
	fld	dword ptr [Input1+YX]	;XX XX
	fmul	dword ptr [Input2+XY]
	fld	dword ptr [Input1+ZX]	;XX XX XX
	fmul	dword ptr [Input2+XZ]
	fxch	st(1)
	faddp	st(2),st		;XX XX

;
; As I perform the Muls for XY
; I interleave the adds for XX
; to prevent any lame matrix multiply stalls
;

	fld	dword ptr [Input1+XY]	;XY XX XX
	fmul	dword ptr [Input2+XX]
	fxch	st(2)			;XX XX XY
	faddp	st(1),st		;Second add
	fld	dword ptr [Input1+YY]
	fmul	dword ptr [Input2+XY]
	fld	dword ptr [Input1+ZY]	;XY XY XX XY
	fmul	dword ptr [Input2+XZ]
	fxch	st(2)
	fstp	dword ptr [Output+XX]	;Store result
	faddp	st(2),st		;XY XY

;
; As you see above, I have a pattern,
; Follow the pattern to it's logical conclusion
;

; XZ

	fld	dword ptr [Input1+XZ]	;XZ XY XY
	fmul	dword ptr [Input2+XX]
	fxch	st(2)			;XY XY XZ
	faddp	st(1),st			;Second add
	fld	dword ptr [Input1+YZ]	;XZ XY XZ
	fmul	dword ptr [Input2+XY]
	fld	dword ptr [Input1+ZZ]	;XZ XZ XY XZ
	fmul	dword ptr [Input2+XZ]
	fxch	st(2)			;XY XZ XZ XZ
	fstp	dword ptr [Output+XY]	;Store result
	faddp	st(2),st		;XZ XZ

;YX

	fld	dword ptr [Input1+XX]	;YX XZ XZ
	fmul	dword ptr [Input2+YX]
	fxch	st(2)			;XZ XZ YX
	faddp	st(1),st		;Second add
	fld	dword ptr [Input1+YX]
	fmul	dword ptr [Input2+YY]
	fld	dword ptr [Input1+ZX]	;YX YX XZ YX
	fmul	dword ptr [Input2+YZ]
	fxch	st(2)			;XZ YX YX YX
	fstp	dword ptr [Output+XZ]	;Store result
	faddp	st(2),st		;YX YX

;YY

	fld	dword ptr [Input1+XY]	;YY YX YX
	fmul	dword ptr [Input2+YX]
	fxch	st(2)			;YX YX YY
	faddp	st(1),st		;Second add
	fld	dword ptr [Input1+YY]
	fmul	dword ptr [Input2+YY]
	fld	dword ptr [Input1+ZY]	;YY YY YX YY
	fmul	dword ptr [Input2+YZ]
	fxch	st(2)
	fstp	dword ptr [Output+YX]	;Store result
	faddp	st(2),st		;YY YY

;YZ

	fld	dword ptr [Input1+XZ]	;YZ YY YY
	fmul	dword ptr [Input2+YX]
	fxch	st(2)			;YY YY YZ
	faddp	st(1),st		;Second add
	fld	dword ptr [Input1+YZ]
	fmul	dword ptr [Input2+YY]
	fld	dword ptr [Input1+ZZ]	;YZ YZ YY YZ
	fmul	dword ptr [Input2+YZ]
	fxch	st(2)			;YY YZ YZ YZ
	fstp	dword ptr [Output+YY]	;Store result
	faddp	st(2),st		;YZ YZ

;
; Now I don't save, I keep them on the stack
; to eat latency cycles at the end of the routine
;

;ZX

	fld	dword ptr [Input1+XX]	;ZX YZ YZ
	fmul	dword ptr [Input2+ZX]
	fxch	st(2)			;YZ YZ ZX
	faddp	st(1),st		;Second add
	fld	dword ptr [Input1+YX]
	fmul	dword ptr [Input2+ZY]
	fld	dword ptr [Input1+ZX]	;ZX ZX YZ ZX
	fmul	dword ptr [Input2+ZZ]
	fxch	st(1)
	faddp	st(3),st		;ZX YZ ZX

;ZY

	fld	dword ptr [Input1+XY]	;ZY ZX YZ ZX
	fmul	dword ptr [Input2+ZX]
	fxch	st(3)			;ZX ZX YZ ZY
	faddp	st(1),st		;Second add
	fld	dword ptr [Input1+YY]
	fmul	dword ptr [Input2+ZY]
	fld	dword ptr [Input1+ZY]	;ZY ZY ZX YZ ZY
	fmul	dword ptr [Input2+ZZ]
	fxch	st(1)
	faddp	st(4),st		;ZY ZX YZ ZY

;ZZ

	fld	dword ptr [Input1+XZ]	;ZZ ZY ZX YZ ZY
	fmul	dword ptr [Input2+ZX]
	fxch	st(4)			;ZY ZY ZX YZ ZZ
	faddp	st(1),st		;Second add
	fxch	st(3)			;ZZ ZX YZ ZY
	fld	dword ptr [Input1+YZ]	;ZZ ZZ ZX YZ ZY
	fmul	dword ptr [Input2+ZY]
	fld	dword ptr [Input1+ZZ]	;ZZ ZZ ZZ ZX YZ ZY
	fmul	dword ptr [Input2+ZZ]
	fxch	st(1)
	faddp	st(2),st		;ZZ ZZ ZX YZ ZY
	fxch	st(2)			;ZX ZZ ZZ YZ ZY


; Since I can't interleave anymore I have to
; eat the 2 cycle + 2 cycle stall

	fstp	dword ptr [Output+ZX]	;Store result
	faddp	st(1),st		;ZZ YZ ZY
	fxch	st(2)			;ZY YZ ZZ
	fstp	dword ptr [Output+ZY]	;Store result
	fstp	dword ptr [Output+YZ]	;Store result
	fstp	dword ptr [Output+ZZ]	;Store result
	ret
	}
}

#undef Output
#undef Input1
#undef Input2

/**********************************

	Multiply a vector by a matrix

**********************************/

asm void BURGERCALL Matrix3DTransformVector3D(Vector3D_t *Output,const Matrix3D_t *Input)
{
	asm {
	mov		ecx,[esp+4]
	mov		edx,[esp+8]
	fld		dword ptr [edx+XX]
	fmul	dword ptr [ecx+0]
	fld		dword ptr [edx+XY]
	fmul	dword ptr [ecx+4]
	fld		dword ptr [edx+XZ]
	fmul	dword ptr [ecx+8]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [edx+YX]
	fmul	dword ptr [ecx+0]
	fld		dword ptr [edx+YY]
	fmul	dword ptr [ecx+4]
	fld		dword ptr [edx+YZ]
	fmul	dword ptr [ecx+8]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [edx+ZX]
	fmul	dword ptr [ecx+0]
	fld		dword ptr [edx+ZY]
	fmul	dword ptr [ecx+4]
	fld		dword ptr [edx+ZZ]
	fmul	dword ptr [ecx+8]
	fxch	st(1)
	faddp	st(2),st
	fxch	st(5)
	faddp	st(4),st
	fxch	st(1)
	faddp	st(2),st
	faddp	st(3),st
	fxch	st(1)
	fstp	dword ptr [ecx+0]
	fstp	dword ptr [ecx+4]
	fstp	dword ptr [ecx+8]
	ret
	}
}

/**********************************

	Multiply a vector by a matrix

**********************************/

asm void BURGERCALL Matrix3DTransformVector3D2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Input2)
{
	asm {
	mov		ecx,[esp+4]
	mov		edx,[esp+8]
	mov		eax,[esp+12]
	fld		dword ptr [edx+XX]
	fmul	dword ptr [eax+0]
	fld		dword ptr [edx+XY]
	fmul	dword ptr [eax+4]
	fld		dword ptr [edx+XZ]
	fmul	dword ptr [eax+8]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [edx+YX]
	fmul	dword ptr [eax+0]
	fld		dword ptr [edx+YY]
	fmul	dword ptr [eax+4]
	fld		dword ptr [edx+YZ]
	fmul	dword ptr [eax+8]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [edx+ZX]
	fmul	dword ptr [eax+0]
	fld		dword ptr [edx+ZY]
	fmul	dword ptr [eax+4]
	fld		dword ptr [edx+ZZ]
	fmul	dword ptr [eax+8]
	fxch	st(1)
	faddp	st(2),st
	fxch	st(5)
	faddp	st(4),st
	fxch	st(1)
	faddp	st(2),st
	faddp	st(3),st
	fxch	st(1)
	fstp	dword ptr [ecx+0]
	fstp	dword ptr [ecx+4]
	fstp	dword ptr [ecx+8]
	ret
	}
}

/**********************************

	Transform a vector and then add a point

**********************************/

asm void BURGERCALL Matrix3DTransformVector3DAdd(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add)
{
	asm {
	mov		ecx,[esp+4]
	mov		edx,[esp+8]
	mov		eax,[esp+12]
	fld		dword ptr [ecx+0]
	fmul	dword ptr [edx+XX]
	fld		dword ptr [ecx+4]
	fmul	dword ptr [edx+XY]
	fld		dword ptr [ecx+8]
	fmul	dword ptr [edx+XZ]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [ecx+0]
	fmul	dword ptr [edx+YX]
	fld		dword ptr [ecx+4]
	fmul	dword ptr [edx+YY]
	fld		dword ptr [ecx+8]
	fmul	dword ptr [edx+YZ]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [ecx+0]
	fmul	dword ptr [edx+ZX]
	fld		dword ptr [ecx+4]
	fmul	dword ptr [edx+ZY]
	fld		dword ptr [ecx+8]
	fmul	dword ptr [edx+ZZ]
	fxch	st(1)
	faddp	st(2),st
	fxch	st(4)
	faddp	st(5),st
	fxch	st(1)
	faddp	st(2),st
	faddp	st(2),st
	fxch	st(2)
	fadd	dword ptr [eax+0]
	fxch	st(2)
	fadd	dword ptr [eax+4]
	fxch	st(1)
	fadd	dword ptr [eax+8]
	fxch	st(2)
	fstp	dword ptr [ecx+0]
	fstp	dword ptr [ecx+4]
	fstp	dword ptr [ecx+8]
	ret
	}
}

/**********************************

	Transform a vector and then add a point

**********************************/

asm void BURGERCALL Matrix3DTransformVector3DAdd2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add,const Vector3D_t *InputV)
{
	asm {
	mov		ecx,[esp+4]
	mov		edx,[esp+8]
	mov		eax,[esp+12]
	mov		[esp+4],ebx
	mov		ebx,[esp+16]
	fld		dword ptr [edx+XX]
	fmul	dword ptr [ebx+0]
	fld		dword ptr [ebx+4]
	fmul	dword ptr [edx+XY]
	fld		dword ptr [ebx+8]
	fmul	dword ptr [edx+XZ]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [ebx+0]
	fmul	dword ptr [edx+YX]
	fld		dword ptr [ebx+4]
	fmul	dword ptr [edx+YY]
	fld		dword ptr [ebx+8]
	fmul	dword ptr [edx+YZ]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [ebx+0]
	fmul	dword ptr [edx+ZX]
	fld		dword ptr [ebx+4]
	fmul	dword ptr [edx+ZY]
	fld		dword ptr [ebx+8]
	fmul	dword ptr [edx+ZZ]
	fxch	st(1)
	faddp	st(2),st
	fxch	st(4)
	faddp	st(5),st
	fxch	st(1)
	faddp	st(2),st
	faddp	st(2),st
	fxch	st(2)
	fadd	dword ptr [eax+0]
	fxch	st(2)
	fadd	dword ptr [eax+4]
	fxch	st(1)
	fadd	dword ptr [eax+8]
	fxch	st(2)
	mov		ebx,[esp+4]
	fstp	dword ptr [ecx+0]
	fstp	dword ptr [ecx+4]
	fstp	dword ptr [ecx+8]
	ret
	}
}

/**********************************

	Multiply a vector by a matrix

**********************************/

asm void BURGERCALL Matrix3DITransformVector3D(Vector3D_t *Output,const Matrix3D_t *Input)
{
	asm {
	mov		ecx,[esp+4]
	mov		edx,[esp+8]
	fld		dword ptr [edx+XX]
	fmul	dword ptr [ecx+0]
	fld		dword ptr [edx+YX]
	fmul	dword ptr [ecx+4]
	fld		dword ptr [edx+ZX]
	fmul	dword ptr [ecx+8]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [edx+XY]
	fmul	dword ptr [ecx+0]
	fld		dword ptr [edx+YY]
	fmul	dword ptr [ecx+4]
	fld		dword ptr [edx+ZY]
	fmul	dword ptr [ecx+8]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [edx+XZ]
	fmul	dword ptr [ecx+0]
	fld		dword ptr [edx+YZ]
	fmul	dword ptr [ecx+4]
	fld		dword ptr [edx+ZZ]
	fmul	dword ptr [ecx+8]
	fxch	st(1)
	faddp	st(2),st
	fxch	st(5)
	faddp	st(4),st
	fxch	st(1)
	faddp	st(2),st
	faddp	st(3),st
	fxch	st(1)
	fstp	dword ptr [ecx+0]
	fstp	dword ptr [ecx+4]
	fstp	dword ptr [ecx+8]
	ret
	}
}

/**********************************

	Multiply a vector by a matrix

**********************************/

asm void BURGERCALL Matrix3DITransformVector3D2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Input2)
{
	asm {
	mov		ecx,[esp+4]
	mov		edx,[esp+8]
	mov		eax,[esp+12]
	fld		dword ptr [edx+XX]
	fmul	dword ptr [eax+0]
	fld		dword ptr [edx+YX]
	fmul	dword ptr [eax+4]
	fld		dword ptr [edx+ZX]
	fmul	dword ptr [eax+8]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [edx+XY]
	fmul	dword ptr [eax+0]
	fld		dword ptr [edx+YY]
	fmul	dword ptr [eax+4]
	fld		dword ptr [edx+ZY]
	fmul	dword ptr [eax+8]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [edx+XZ]
	fmul	dword ptr [eax+0]
	fld		dword ptr [edx+YZ]
	fmul	dword ptr [eax+4]
	fld		dword ptr [edx+ZZ]
	fmul	dword ptr [eax+8]
	fxch	st(1)
	faddp	st(2),st
	fxch	st(5)
	faddp	st(4),st
	fxch	st(1)
	faddp	st(2),st
	faddp	st(3),st
	fxch	st(1)
	fstp	dword ptr [ecx+0]
	fstp	dword ptr [ecx+4]
	fstp	dword ptr [ecx+8]
	ret
	}
}

/**********************************

	Transform a vector and then add a point

**********************************/

asm void BURGERCALL Matrix3DITransformVector3DAdd(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add)
{
	asm {
	mov		ecx,[esp+4]
	mov		edx,[esp+8]
	mov		eax,[esp+12]
	fld		dword ptr [ecx+0]
	fmul	dword ptr [edx+XX]
	fld		dword ptr [ecx+4]
	fmul	dword ptr [edx+YX]
	fld		dword ptr [ecx+8]
	fmul	dword ptr [edx+ZX]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [ecx+0]
	fmul	dword ptr [edx+XY]
	fld		dword ptr [ecx+4]
	fmul	dword ptr [edx+YY]
	fld		dword ptr [ecx+8]
	fmul	dword ptr [edx+ZY]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [ecx+0]
	fmul	dword ptr [edx+XZ]
	fld		dword ptr [ecx+4]
	fmul	dword ptr [edx+YZ]
	fld		dword ptr [ecx+8]
	fmul	dword ptr [edx+ZZ]
	fxch	st(1)
	faddp	st(2),st
	fxch	st(4)
	faddp	st(5),st
	fxch	st(1)
	faddp	st(2),st
	faddp	st(2),st
	fxch	st(2)
	fadd	dword ptr [eax+0]
	fxch	st(2)
	fadd	dword ptr [eax+4]
	fxch	st(1)
	fadd	dword ptr [eax+8]
	fxch	st(2)
	fstp	dword ptr [ecx+0]
	fstp	dword ptr [ecx+4]
	fstp	dword ptr [ecx+8]
	ret
	}
}

/**********************************

	Transform a vector and then add a point

**********************************/

asm void BURGERCALL Matrix3DITransformVector3DAdd2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add,const Vector3D_t *InputV)
{
	asm {
	mov		ecx,[esp+4]
	mov		edx,[esp+8]
	mov		eax,[esp+12]
	mov		[esp+4],ebx
	mov		ebx,[esp+16]
	fld		dword ptr [edx+XX]
	fmul	dword ptr [ebx+0]
	fld		dword ptr [ebx+4]
	fmul	dword ptr [edx+YX]
	fld		dword ptr [ebx+8]
	fmul	dword ptr [edx+ZX]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [ebx+0]
	fmul	dword ptr [edx+XY]
	fld		dword ptr [ebx+4]
	fmul	dword ptr [edx+YY]
	fld		dword ptr [ebx+8]
	fmul	dword ptr [edx+ZY]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [ebx+0]
	fmul	dword ptr [edx+XZ]
	fld		dword ptr [ebx+4]
	fmul	dword ptr [edx+YZ]
	fld		dword ptr [ebx+8]
	fmul	dword ptr [edx+ZZ]
	fxch	st(1)
	faddp	st(2),st
	fxch	st(4)
	faddp	st(5),st
	fxch	st(1)
	faddp	st(2),st
	faddp	st(2),st
	fxch	st(2)
	fadd	dword ptr [eax+0]
	fxch	st(2)
	fadd	dword ptr [eax+4]
	fxch	st(1)
	fadd	dword ptr [eax+8]
	fxch	st(2)
	mov		ebx,[esp+4]
	fstp	dword ptr [ecx+0]
	fstp	dword ptr [ecx+4]
	fstp	dword ptr [ecx+8]
	ret
	}
}


#endif


