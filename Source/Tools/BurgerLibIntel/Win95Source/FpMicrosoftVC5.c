/**********************************

	Microsoft "C" version

**********************************/

#include "FpFloat.h"

#if defined(__WIN32__) && !defined(__MWERKS__)

#pragma warning(disable:4137)	/* Kill the no return value warning */
#pragma warning(disable:4035)	/* Kill the no return int value warning */

static float Zero = 0;
static float One = 1.0f;
static float Num1024 = 1024.0f;

/**********************************

	Create a square root but I have about a 1%
	margin of error for rounding

**********************************/

__declspec(naked) float BURGERCALL SqrtFast(float n)
{
	_asm {
	mov eax,[esp+4]

	shr	eax,23-9-2
	mov	edx,[esp+4]

	and eax,0FFCH
	and edx,7FFFFFFFH

	jz	L1

	mov eax,BurgerSqrtTable[eax]
	and edx,07F000000H

	add eax,edx
	shr eax,1
	mov [esp+4],eax
	fld	dword ptr [esp+4]
	ret	4
L1:
	fld	dword ptr [Zero]
	ret	4
	}
}

/**********************************

	Return a floating point number rounded up

**********************************/

__declspec(naked) float BURGERCALL CeilingFast(float n)
{
	_asm {
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
	ret		4
L1:
	fadd	dword ptr [One]
	ret		4
	}
}

/**********************************

	Return a floating point number rounded up

**********************************/

__declspec(naked) int BURGERCALL CeilingFastInt(float n)
{
	_asm {
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
	ret		4
L1:
	inc		eax
	ret		4
	}
}

/**********************************

	Return a floating point number floored

**********************************/

__declspec(naked) float BURGERCALL FloorFast(float n)
{
	_asm {
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
	ret		4
L1:
	fsub	dword ptr [One]
	ret		4
	}
}

/**********************************

	Return a floating point number floored

**********************************/

__declspec(naked) int BURGERCALL FloorFastInt(float n)
{
	_asm {
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
	ret		4
L1:
	cmp		eax,0x80000000
	jz		L0
	dec		eax
L0:
	ret		4
	}
}

/**********************************

	Convert a floating point number to an integer
	but use round to nearest

**********************************/

__declspec(naked) int BURGERCALL FloatToInt(float Input)
{
	_asm {
	cmp		dword ptr [esp+4],0x4F000000
	mov		eax,0x7FFFFFFF
	jge		L1
	fld		dword ptr [esp+4]
	fistp	dword ptr [esp+4]
	mov		eax,[esp+4]
L1:
	ret		4
	}
}

/**********************************

	Given an angle of the range
	of ANGLERANGE (ANGLERANGE==360.0 degrees)
	return the sine.

**********************************/

__declspec(naked) float BURGERCALL FloatSine(float Angle)
{
	_asm {
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
	ret		4
	}
}

/**********************************

	Given an angle of the range
	of ANGLERANGE (ANGLERANGE==360.0 degrees)
	return the cosine.

**********************************/

__declspec(naked) float BURGERCALL FloatCosine(float Angle)
{
	_asm {
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
	ret		4
	}
}

/**********************************

	Take a floating point cosine and return
	the fixed point angle (Range 0-1024 fixed)

**********************************/

__declspec(naked) Fixed32 BURGERCALL FloatArcCosine(float c)
{
	_asm {
	fld		dword ptr[esp+4]
	fmul	dword ptr[Num1024]
	fistp	dword ptr[esp+4]
	mov		eax,[esp+4]
	cmp		eax,0xFFFFFC00
	jle		L4
	cmp		eax,0x00000400
	jge		L3
	mov		eax,dword ptr [FixedArcCosineTable+4096+eax*4]
	ret		4
L3:
	xor		eax,eax
	ret		4
L4:
	mov		eax,0x04000000
	ret		4
	}
}

/**********************************

	Take a fixed point sine and return
	the fixed point angle (Range 1536-2047/0-512 fixed)

**********************************/

__declspec(naked) Fixed32 BURGERCALL FloatArcSine(float c)
{
	_asm {
	fld		dword ptr[esp+4]
	fmul	dword ptr[Num1024]
	fistp	dword ptr[esp+4]
	mov		eax,[esp+4]
	cmp		eax,0xFFFFFC00
	jle		L4
	cmp		eax,0x00000400
	jge		L3
	mov		eax,dword ptr [FixedArcSineTable+4096+eax*4]
	ret		4
L3:
	mov		eax,0x02000000
	ret		4
L4:
	mov		eax,0x06000000
	ret		4
	}
}

/**********************************

	Return a dot product of two 3D vectors

**********************************/

#if 0
__declspec(naked) float BURGERCALL Vector3DDot(const Vector3D_t *Input1,const Vector3D_t *Input2)
{
	_asm {
	fld		dword ptr [ecx+0]
	fmul	dword ptr [edx+0]
	fld		dword ptr [ecx+4]
	fmul	dword ptr [edx+4]
	fld		dword ptr [ecx+8]
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

__declspec(naked) void BURGERCALL Vector3DCross(Vector3D_t *Output,const Vector3D_t *Input1,const Vector3D_t *Input2)
{
	_asm {
	mov		eax,[esp+4]
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
	mov		eax,[esp+0]
	add		esp,8
	fsubp	st(1),st
	fxch	st(2)
	fstp	dword ptr [ecx+0]
	fstp	dword ptr [ecx+4]
	fstp	dword ptr [ecx+8]
	jmp		eax
	}
}

/**********************************

	Returns the square of the radius of a 3D vector

**********************************/

__declspec(naked) float BURGERCALL Vector3DGetRadiusSqr(const Vector3D_t *Input)
{
	_asm {
	fld		dword ptr [ecx+0]
	fmul	st,st
	fld		dword ptr [ecx+4]
	fmul	st,st
	fld		dword ptr [ecx+8]
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

__declspec(naked) void BURGERCALL Vector3DNormalize(Vector3D_t *Input)
{
	_asm {
	fld dword ptr [ecx]
	fmul st,st
	fld dword ptr [ecx+4]
	fmul st,st
	fld dword ptr [ecx+8]
	fmul st,st
	fxch st(1)
	faddp st(2),st(0)
	push eax
	faddp st(1),st(0)
	fsqrt
	fld dword ptr [One]
	fxch st(1)
	fstp dword ptr [esp]
	mov eax,[esp]
	test eax,0x7FFFFFFF
	jz NoGood
	fdiv dword ptr [esp]
	pop	eax
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
	pop eax
	fstp st(0)
	ret
	}
}

/**********************************

	Normalize a 3D vector
	(Set its distance to 1.0)
	(High precision)

**********************************/

__declspec(naked) void BURGERCALL Vector3DNormalize2(Vector3D_t *Output,const Vector3D_t *Input)
{
	_asm {
	fld dword ptr [edx]
	fmul st,st
	fld dword ptr [edx+4]
	fmul st,st
	fld dword ptr [edx+8]
	fmul st,st
	fxch st(1)
	faddp st(2),st(0)
	push eax
	faddp st(1),st(0)
	fsqrt
	fld dword ptr [One]
	fxch st(1)
	fstp dword ptr [esp]
	mov eax,[esp]
	test eax,0x7FFFFFFF
	jz NoGood
	fdiv dword ptr [esp]
	pop	eax
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
	pop eax
	fstp st(0)
	ret
	}
}

/**********************************

	Assembly for Matrix code

**********************************/

#define XX 0
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

__declspec(naked) void BURGERCALL Matrix3DSet(Matrix3D_t *Output,float yaw,float pitch,float roll)
{
	_asm {
	fld		dword ptr [esp+4]
	fld		dword ptr [esp+8]
	fld		dword ptr [esp+12]
	push	ebx
	push	ebp
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
	pop		ebp
	pop		ebx
	ret		12
	}
}

/**********************************

	Create a rotation matrix with angle for
	yaw (X)

**********************************/

__declspec(naked) void BURGERCALL Matrix3DSetYaw(Matrix3D_t *Output,float yaw)
{
	_asm {
	fld		dword ptr [esp+4]
	fistp	dword ptr [esp+4]
	mov		edx,[esp+4]
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
	ret		4
	}
}

/**********************************

	Create a rotation matrix with angle for
	pitch (Y)

**********************************/

__declspec(naked) void BURGERCALL Matrix3DSetPitch(Matrix3D_t *Output,float pitch)
{
	_asm {
	fld		dword ptr [esp+4]
	fistp	dword ptr [esp+4]
	mov		edx,[esp+4]
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
	ret		4
	}
}

/**********************************

	Create a rotation matrix with angle for
	roll (Z)

**********************************/

__declspec(naked) void BURGERCALL Matrix3DSetRoll(Matrix3D_t *Output,float roll)
{
	_asm {
	fld		dword ptr [esp+4]
	fistp	dword ptr [esp+4]
	mov		edx,[esp+4]
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
	ret		4
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

__declspec(naked) void BURGERCALL Matrix3DSetInt(Matrix3D_t *Output,Word yaw,Word pitch,Word roll)
{
	_asm {
	push	ebx
	push	ebp
	mov		ebx,[esp+12]
	mov		ebp,[esp+16]
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
	ret		8
	}
}

/**********************************

	Create a rotation matrix with angles for
	the yaw (Y), pitch (X), and roll (Z)
	The angles are fixed point from 0-2047 (Rounded to 2048)

	x.x = cy*cz + sx*sy*sz x.y = cy*sz - sx*sy*cz  x.z = cx*sy
	y.x = -cx*sz           y.y = cx*cz             y.z = sx
	z.x = -sy*cz+sx*cy*sz  z.y = sy*-sz + sx*cy*cz z.z = cx*cy

**********************************/

__declspec(naked) void BURGERCALL Matrix3DSetFixed(Matrix3D_t *Output,Fixed32 yaw,Fixed32 pitch,Fixed32 roll)
{
	_asm {
	push	ebx
	push	ebp
	shr		edx,16
	mov		ebx,[esp+12]
	shr		ebx,16
	mov		ebp,[esp+16]
	shr		ebp,16
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
	ret		8
	}
}

/**********************************

	Perform a matrix multiply on the output matrix

**********************************/

__declspec(naked) void BURGERCALL Matrix3DMul(Matrix3D_t *Output,const Matrix3D_t *Input)
{
	_asm {
	sub		esp,40
	mov		eax,[ecx+0]
	mov		[esp+4],eax
	mov		eax,[ecx+4]
	mov		[esp+8],eax
	mov		eax,[ecx+8]
	mov		[esp+12],eax
	mov		eax,[ecx+12]
	mov		[esp+16],eax
	mov		eax,[ecx+16]
	mov		[esp+20],eax
	mov		eax,[ecx+20]
	mov		[esp+24],eax
	mov		eax,[ecx+24]
	mov		[esp+28],eax
	mov		eax,[ecx+28]
	mov		[esp+32],eax
	mov		eax,[ecx+32]
	mov		[esp+36],eax
	mov		[esp+0],edx
	lea		edx,[esp+4]
	call	Matrix3DMul2
	mov		eax,[esp+36]
	add		esp,40
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

__declspec(naked) void BURGERCALL Matrix3DMul2(Matrix3D_t *Output,const Matrix3D_t *Input1,const Matrix3D_t *Input2)
{
	_asm {

	mov	Input2,[esp+4]

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
	ret		4
	}
}

#undef Output
#undef Input1
#undef Input2

/**********************************

	Multiply a vector by a matrix

**********************************/

__declspec(naked) void BURGERCALL Matrix3DTransformVector3D(Vector3D_t *Output,const Matrix3D_t *Input)
{
	_asm {
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

__declspec(naked) void BURGERCALL Matrix3DTransformVector3D2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Input2)
{
	_asm {
	mov		eax,[esp+4]
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
	ret		4
	}
}

/**********************************

	Transform a vector and then add a point

**********************************/

__declspec(naked) void BURGERCALL Matrix3DTransformVector3DAdd(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add)
{
	_asm {
	mov		eax,[esp+4]
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
	ret		4
	}
}

/**********************************

	Transform a vector and then add a point

**********************************/

__declspec(naked) void BURGERCALL Matrix3DTransformVector3DAdd2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add,const Vector3D_t *InputV)
{
	_asm {
	mov		eax,[esp+8]
	fld		dword ptr [edx+XX]
	fmul	dword ptr [eax+0]
	fld		dword ptr [eax+4]
	fmul	dword ptr [edx+XY]
	fld		dword ptr [eax+8]
	fmul	dword ptr [edx+XZ]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [eax+0]
	fmul	dword ptr [edx+YX]
	fld		dword ptr [eax+4]
	fmul	dword ptr [edx+YY]
	fld		dword ptr [eax+8]
	fmul	dword ptr [edx+YZ]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [eax+0]
	fmul	dword ptr [edx+ZX]
	fld		dword ptr [eax+4]
	fmul	dword ptr [edx+ZY]
	fld		dword ptr [eax+8]
	fmul	dword ptr [edx+ZZ]
	fxch	st(1)
	faddp	st(2),st
	fxch	st(4)
	faddp	st(5),st
	fxch	st(1)
	faddp	st(2),st
	mov		eax,[esp+4]
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
	ret		8
	}
}

/**********************************

	Multiply a vector by a matrix

**********************************/

__declspec(naked) void BURGERCALL Matrix3DITransformVector3D(Vector3D_t *Output,const Matrix3D_t *Input)
{
	_asm {
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

__declspec(naked) void BURGERCALL Matrix3DITransformVector3D2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Input2)
{
	_asm {
	mov		eax,[esp+4]
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
	ret		4
	}
}

/**********************************

	Transform a vector and then add a point

**********************************/

__declspec(naked) void BURGERCALL Matrix3DITransformVector3DAdd(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add)
{
	_asm {
	mov		eax,[esp+4]
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
	ret		4
	}
}

/**********************************

	Transform a vector and then add a point

**********************************/

__declspec(naked) void BURGERCALL Matrix3DITransformVector3DAdd2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add,const Vector3D_t *InputV)
{
	_asm {
	mov		eax,[esp+8]
	fld		dword ptr [edx+XX]
	fmul	dword ptr [eax+0]
	fld		dword ptr [eax+4]
	fmul	dword ptr [edx+YX]
	fld		dword ptr [eax+8]
	fmul	dword ptr [edx+ZX]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [eax+0]
	fmul	dword ptr [edx+XY]
	fld		dword ptr [eax+4]
	fmul	dword ptr [edx+YY]
	fld		dword ptr [eax+8]
	fmul	dword ptr [edx+ZY]
	fxch	st(1)
	faddp	st(2),st
	fld		dword ptr [eax+0]
	fmul	dword ptr [edx+XZ]
	fld		dword ptr [eax+4]
	fmul	dword ptr [edx+YZ]
	fld		dword ptr [eax+8]
	fmul	dword ptr [edx+ZZ]
	fxch	st(1)
	faddp	st(2),st
	fxch	st(4)
	faddp	st(5),st
	fxch	st(1)
	faddp	st(2),st
	mov		eax,[esp+4]
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
	ret		8
	}
}
#endif

