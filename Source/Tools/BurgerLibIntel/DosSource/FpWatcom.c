/**********************************

	Watcom C version

**********************************/

#include "FpFloat.h"

#if defined(__WATCOMC__)

static float Zero = 0;		/* Constants */
static float One = 1.0f;	/* I need a 1.0 constant */
static float Num1024 = 1024.0f;

/**********************************

	Create a square root but I have about a 1%
	margin of error for rounding

**********************************/

/* Note : Watcom prefixs this code with a PUSH EAX */
/* and a MOV EAX,[ESP+8] */

extern float FooSqrtFast(float n);

#pragma aux FooSqrtFast = \
	"PUSH EDX" \
	"MOV EDX,EAX" \
\
	"SHR EAX,23-9-2" \
	"AND EAX,0FFCH" \
\
	"TEST EDX,7FFFFFFFH" \
	"JZ	L1" \
\
	"MOV EAX,BurgerSqrtTable[EAX]" \
	"AND EDX,07F000000H" \
\
	"ADD EAX,EDX" \
	"SHR EAX,1" \
	"MOV [ESP+12],EAX" \
	"FLD DWORD PTR [ESP+12]" \
	"POP EDX" \
	"POP EAX" \
	"RET 4" \
"L1:" \
	"FLD DWORD PTR [Zero]" \
	"POP EDX" \
	parm [eax]

float BURGERCALL SqrtFast(float n)
{
	return FooSqrtFast(n);		/* Call assembly */
}

/**********************************

	Return a floating point number rounded up

**********************************/

extern float FooCeilingFast(void);

#pragma aux FooCeilingFast = \
	"push	eax" \
	"mov	eax,[esp+8]" \
	"fld	dword ptr [esp+8]" \
	"and	eax,07FFFFFFFH" \
	"fist	dword ptr [esp+8]" \
	"cmp	eax,04B000000H" \
	"jae	L1" \
	"fild	dword ptr [esp+8]" \
	"fxch	st(1)" \
	"xor	eax,eax" \
	"fcomp	st(1)" \
	"fnstsw	ax" \
	"and	eax,0x4100" \
	"jnz	L1" \
	"fadd	dword ptr [One]" \
"L1:" \
	"pop	eax" \
	modify exact

float BURGERCALL CeilingFast(float /* n */)
{
	return FooCeilingFast();	/* Above assembly */
}

/**********************************

	Return a floating point number rounded up

**********************************/

extern int FooCeilingFastInt(void);

#pragma aux FooCeilingFastInt = \
	"mov	eax,[esp+4]" \
	"fld	dword ptr [esp+4]" \
	"cmp	eax,0x4F000000" \
	"fist	dword ptr [esp+4]" \
	"mov	eax,0x7FFFFFFF" \
	"jge	L1" \
	"fild	dword ptr [esp+4]" \
	"xor	eax,eax" \
	"fcompp" \
	"fnstsw	ax" \
	"and	eax,0x100" \
	"mov	eax,[esp+4]" \
	"jz		L1" \
	"inc	eax" \
"L1:" \
	value [eax]

int BURGERCALL CeilingFastInt(float /* n */)
{
	return FooCeilingFastInt();	/* Above assembly */
}


/**********************************

	Return a floating point number floored

**********************************/

extern float FooFloorFast(void);

#pragma aux FooFloorFast = \
	"push	eax" \
	"mov	eax,[esp+8]" \
	"fld	dword ptr [esp+8]" \
	"and	eax,07FFFFFFFH" \
	"fist	dword ptr [esp+8]" \
	"cmp	eax,04B000000H" \
	"jae	L1" \
	"fild	dword ptr [esp+8]" \
	"fxch	st(1)" \
	"xor	eax,eax" \
	"fcomp	st(1)" \
	"fnstsw	ax" \
	"and	eax,0x100" \
	"jz		L1" \
	"fsub	dword ptr [One]" \
"L1: pop	eax" \
	modify exact

float BURGERCALL FloorFast(float /* n */)
{
	return FooFloorFast();	/* Above assembly */
}

/**********************************

	Return a floating point number floored

**********************************/

extern int FooFloorFastInt(void);

#pragma aux FooFloorFastInt = \
	"mov	eax,[esp+4]" \
	"fld	dword ptr [esp+4]" \
	"cmp	eax,0x4F000000" \
	"fist	dword ptr [esp+4]" \
	"mov	eax,0x7FFFFFFF" \
	"jge	L1" \
	"fild	dword ptr [esp+4]" \
	"xor	eax,eax" \
	"fcompp" \
	"fnstsw	ax" \
	"and	eax,0x4100" \
	"mov	eax,[esp+4]" \
	"jnz	L1" \
	"cmp	eax,0x80000000" \
	"jz		L1" \
	"dec	eax" \
"L1:" \
	value [eax]

int BURGERCALL FloorFastInt(float /* n */)
{
	return FooFloorFastInt();	/* Above assembly */
}

/**********************************

	Convert a floating point number to an int
	using round to nearest.
	I wanted to inline this but the @(*#$$ Pentium III
	rounds the float 10000000000 to 0x80000000 (A negative int!)

**********************************/

extern int FooFloatToInt(void);
#pragma aux FooFloatToInt = \
	"cmp	dword ptr [esp+4],0x4F000000" \
	"mov	eax,0x7FFFFFFF" \
	"jge	L1" \
	"fld	dword ptr [esp+4]" \
	"fistp	dword ptr [esp+4]" \
	"mov	eax,[esp+4]" \
"L1:" \
	value [eax]

int BURGERCALL FloatToInt(float /* Input */)
{
	return FooFloatToInt();
}

/**********************************

	Given an angle of the range
	of ANGLERANGE (ANGLERANGE==360.0 degrees)
	return the sine.

**********************************/

extern float FooFloatSine(void);
#pragma aux FooFloatSine = \
	"mov eax,[esp+8]" \
	"fld dword ptr [esp+8]" \
	"cmp eax,0x4F000000" \
	"fistp dword ptr [esp+8]" \
	"mov eax,0x7FF" \
	"jge L1" \
	"mov eax,[esp+8]" \
	"and eax,0x7FF" \
"L1:" \
	"fld dword ptr [FSineTable + eax*4]" \
	value [8087]

float BURGERCALL FloatSine(float /* Angle */)
{
	return FooFloatSine();
}

/**********************************

	Given an angle of the range
	of ANGLERANGE (ANGLERANGE==360.0 degrees)
	return the cosine.

**********************************/

extern float FooFloatCosine(void);
#pragma aux FooFloatCosine = \
	"mov eax,[esp+8]" \
	"fld dword ptr [esp+8]" \
	"cmp eax,0x4F000000" \
	"fistp dword ptr [esp+8]" \
	"mov eax,0x7FF" \
	"jge L1" \
	"mov eax,[esp+8]" \
	"and eax,0x7FF" \
"L1:" \
	"fld dword ptr [FSineTable+2048 +eax*4]" \
	value [8087]

float BURGERCALL FloatCosine(float /* Angle */)
{
	return FooFloatCosine();
}

/**********************************

	Take a floating point cosine and return
	the fixed point angle (Range 0-1024 Fixed32)

**********************************/

extern Fixed32 FooFloatArcCosine(void);
#pragma aux FooFloatArcCosine = \
	"fld	dword ptr[esp+4]" \
	"fmul	dword ptr[Num1024]" \
	"fistp	dword ptr[esp+4]" \
	"mov	eax,[esp+4]" \
	"cmp	eax,0xFFFFFC00" \
	"jle	L4" \
	"cmp	eax,0x00000400" \
	"jge	L3" \
	"mov	eax,FixedArcCosineTable[eax*4+4096]" \
	"ret	4" \
"L3:" \
	"xor	eax,eax" \
	"ret	4" \
"L4:" \
	"mov	eax,0x04000000" \
	value [eax]

Fixed32 BURGERCALL FloatArcCosine(float /* c */)
{
	return FooFloatArcCosine();
}

/**********************************

	Take a fixed point sine and return
	the fixed point angle (Range 1536-2047/0-512 Fixed32)

**********************************/

extern Fixed32 FooFloatArcSine(void);
#pragma aux FooFloatArcSine = \
	"fld	dword ptr[esp+4]" \
	"fmul	dword ptr[Num1024]" \
	"fistp	dword ptr[esp+4]" \
	"mov	eax,[esp+4]" \
	"cmp	eax,0xFFFFFC00" \
	"jle	L4" \
	"cmp	eax,0x00000400" \
	"jge	L3" \
	"mov	eax,FixedArcSineTable[eax*4+4096]" \
	"ret	4" \
"L3:" \
	"mov	eax,0x02000000" \
	"ret	4" \
"L4:" \
	"mov	eax,0x06000000" \
	value [eax]

Fixed32 BURGERCALL FloatArcSine(float /* c */)
{
	return FooFloatArcSine();
}

/**********************************

	Return a dot product of two 3D vectors

**********************************/

#if 0
extern float FooVector3DDot(const Vector3D_t *Input1,const Vector3D_t *Input2);

#pragma aux FooVector3DDot = \
	"fld	dword ptr [eax+0]" \
	"fmul	dword ptr [edx+0]" \
	"fld	dword ptr [eax+4]" \
	"fmul	dword ptr [edx+4]" \
	"fld	dword ptr [eax+8]" \
	"fmul	dword ptr [edx+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st(0)" \
	"faddp	st(1),st(0)" \
	parm [eax] [edx] \
	value [8087]

float BURGERCALL Vector3DDot(const Vector3D_t *Input1,const Vector3D_t *Input2)
{
	return FooVector3DDot(Input1,Input2);
}
#endif

/**********************************

	Return a cross product of two 3D vectors

**********************************/

extern void FooVector3DCross(Vector3D_t *Output,const Vector3D_t *Input1,const Vector3D_t *Input2);

#pragma aux FooVector3DCross = \
	"fld	dword ptr [edx+4]" \
	"fmul	dword ptr [ebx+8]" \
	"fld	dword ptr [edx+8]" \
	"fmul	dword ptr [ebx+0]" \
	"fld	dword ptr [edx+0]" \
	"fmul	dword ptr [ebx+4]" \
	"fld	dword ptr [edx+8]" \
	"fmul	dword ptr [ebx+4]" \
	"fld	dword ptr [edx+0]" \
	"fmul	dword ptr [ebx+8]" \
	"fld	dword ptr [edx+4]" \
	"fmul	dword ptr [ebx+0]" \
	"fxch	st(2)" \
	"fsubp	st(5),st" \
	"fsubp	st(3),st" \
	"fsubp	st(1),st" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+0]" \
	"fstp	dword ptr [eax+4]" \
	"fstp	dword ptr [eax+8]" \
	parm [eax] [edx] [ebx]

void BURGERCALL Vector3DCross(Vector3D_t *Output,const Vector3D_t *Input1,const Vector3D_t *Input2)
{
	FooVector3DCross(Output,Input1,Input2);
}

/**********************************

	Returns the square of the radius of a 3D vector

**********************************/

extern float FooVector3DGetRadiusSqr(const Vector3D_t *Input);

#pragma aux FooVector3DGetRadiusSqr = \
	"fld	dword ptr [eax+0]" \
	"fmul	st,st" \
	"fld	dword ptr [eax+4]" \
	"fmul	st,st" \
	"fld	dword ptr [eax+8]" \
	"fmul	st,st" \
	"fxch	st(1)" \
	"faddp	st(2), st(0)" \
	"faddp	st(1), st(0)" \
	parm [eax]

float BURGERCALL Vector3DGetRadiusSqr(const Vector3D_t *Input)
{
	return FooVector3DGetRadiusSqr(Input);
}

/**********************************

	Normalize a 3D vector
	(Set its distance to 1.0)
	(High precision)

**********************************/

extern void FooVector3DNormalize(Vector3D_t *Input);

#pragma aux FooVector3DNormalize = \
	"fld dword ptr [eax]" \
	"fmul st,st" \
	"fld dword ptr [eax+4]" \
	"fmul st,st" \
	"fld dword ptr [eax+8]" \
	"fmul st,st" \
	"fxch st(1)" \
	"faddp st(2),st(0)" \
	"push edx" \
	"push eax" \
	"faddp st(1),st(0)" \
	"fsqrt" \
	"fld One" \
	"fxch st(1)" \
	"fstp dword ptr [esp]" \
	"mov edx,[esp]" \
	"test edx,0x7FFFFFFF" \
	"jz NoGood" \
	"fdiv dword ptr [esp]" \
	"pop edx" \
	"pop edx" \
	"fld dword ptr [eax+0]" \
	"fld dword ptr [eax+4]" \
	"fld dword ptr [eax+8]" \
	"fxch st(2)" \
	"fmul st(0),st(3)" \
	"fxch st(1)" \
	"fmul st(0),st(3)" \
	"fxch st(2)" \
	"fmulp st(3),st(0)" \
	"fstp dword ptr [eax+0]" \
	"fstp dword ptr [eax+4]" \
	"fstp dword ptr [eax+8]" \
	"ret" \
"NoGood:" \
	"pop eax" \
	"pop edx" \
	"fstp st(0)" \
	parm [eax]

void BURGERCALL Vector3DNormalize(Vector3D_t *Input)
{
	FooVector3DNormalize(Input);
}

/**********************************

	Normalize a 3D vector
	(Set its distance to 1.0)
	(High precision)

**********************************/

extern void FooVector3DNormalize2(Vector3D_t *Output,const Vector3D_t *Input);

#pragma aux FooVector3DNormalize2 = \
	"fld dword ptr [edx]" \
	"fmul st,st" \
	"fld dword ptr [edx+4]" \
	"fmul st,st" \
	"fld dword ptr [edx+8]" \
	"fmul st,st" \
	"fxch st(1)" \
	"faddp st(2),st(0)" \
	"push edx" \
	"push eax" \
	"faddp st(1),st(0)" \
	"fsqrt" \
	"fld One" \
	"fxch st(1)" \
	"fstp dword ptr [esp]" \
	"mov edx,[esp]" \
	"test edx,0x7FFFFFFF" \
	"jz NoGood" \
	"fdiv dword ptr [esp]" \
	"pop edx" \
	"pop edx" \
	"fld dword ptr [edx+0]" \
	"fld dword ptr [edx+4]" \
	"fld dword ptr [edx+8]" \
	"fxch st(2)" \
	"fmul st(0),st(3)" \
	"fxch st(1)" \
	"fmul st(0),st(3)" \
	"fxch st(2)" \
	"fmulp st(3),st(0)" \
	"fstp dword ptr [eax+0]" \
	"fstp dword ptr [eax+4]" \
	"fstp dword ptr [eax+8]" \
	"ret" \
"NoGood:" \
	"pop eax" \
	"pop edx" \
	"fstp st(0)" \
	parm [eax] [edx]

void BURGERCALL Vector3DNormalize2(Vector3D_t *Output,const Vector3D_t *Input)
{
	FooVector3DNormalize2(Output,Input);
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
	the yaw (Y), pitch (X), and roll (Z)
	edx = Y
	ebx = X
	ebp = Z

**********************************/

extern void FooMatrix3DSet(Matrix3D_t *Output);

#pragma aux FooMatrix3DSet = \
	"fld	dword ptr [esp+4]" \
	"fld	dword ptr [esp+8]" \
	"fld	dword ptr [esp+12]" \
	"push	ebx" \
	"push	edx" \
	"push	ebp" \
	"fistp	dword ptr [esp+16]" \
	"fistp	dword ptr [esp+20]" \
	"mov	ebp,[esp+16]" \
	"mov	ebx,[esp+20]" \
	"fistp	dword ptr [esp+24]" \
	"mov	edx,[esp+24]" \
	"and	ebp,0x7FF" \
	"and	ebx,0x7FF" \
	"and	edx,0x7FF" \
	"fld	dword ptr [FSineTable + ebp*4]" \
	"fmul	dword ptr [FSineTable + ebx*4]" \
	"fld	dword ptr [FSineTable + 2048 + ebp*4]" \
	"fmul	dword ptr [FSineTable + ebx*4]" \
	"fld	dword ptr [FSineTable + 2048 + ebp*4]" \
	"fmul	dword ptr [FSineTable + 2048 + edx*4]" \
	"fld	dword ptr [FSineTable + ebp*4]" \
	"fmul	dword ptr [FSineTable + 2048 + edx*4]" \
	"fld	dword ptr [FSineTable + 2048 + ebx*4]" \
	"fmul	dword ptr [FSineTable + edx*4]" \
	"fld	dword ptr [FSineTable + edx*4]" \
	"fmul	st,st(5)" \
	"fld	dword ptr [FSineTable + edx*4]" \
	"fmul	st,st(5)" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+8]" \
	"faddp	st(3),st" \
	"fsubp	st(1),st" \
	"fld	dword ptr [FSineTable + ebp*4]" \
	"fmul	dword ptr [FSineTable + 2048 + ebx*4]" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+0]" \
	"fstp	dword ptr [eax+4]" \
	"fchs" \
	"fld	dword ptr [FSineTable + 2048 + ebp*4]" \
	"fmul	dword ptr [FSineTable + 2048 + ebx*4]" \
	"fxch	st(1)" \
	"fstp	dword ptr [eax+12]" \
	"fld	dword ptr [FSineTable + 2048 + ebp*4]" \
	"fmul	dword ptr [FSineTable + edx*4]" \
	"fxch	st(1)" \
	"fstp	dword ptr [eax+16]" \
	"fld	dword ptr [FSineTable + ebp*4]" \
	"fmul	dword ptr [FSineTable + edx*4]" \
	"fld	dword ptr [FSineTable + 2048 + ebx*4]" \
	"fmul	dword ptr [FSineTable + 2048 + edx*4]" \
	"fld	dword ptr [FSineTable + 2048 + edx*4]" \
	"fmulp	st(5),st" \
	"fld	dword ptr [FSineTable + 2048 + edx*4]" \
	"fmulp	st(4),st" \
	"fstp	dword ptr [eax+32]" \
	"fchs" \
	"mov	ebx,[FSineTable+ebx*4]" \
	"fsubrp	st(2),st" \
	"fsubp	st(2),st" \
	"mov	[eax+20],ebx" \
	"fstp	dword ptr [eax+28]" \
	"fstp	dword ptr [eax+24]" \
	"pop	ebp" \
	"pop	edx" \
	"pop	ebx" \
	parm [eax] modify exact

void BURGERCALL Matrix3DSet(Matrix3D_t *Output,float /* yaw */,float /* pitch */,float /* roll */)
{
	FooMatrix3DSet(Output);
}

/**********************************

	Create a rotation matrix with angle for
	yaw (X)

**********************************/

extern void FooMatrix3DSetYaw(Matrix3D_t *Output,float yaw);

#pragma aux FooMatrix3DSetYaw = \
	"fistp	dword ptr [esp+12]" \
	"mov	edx,[esp+12]" \
	"xor	ecx,ecx" \
	"and	edx,0x7FF" \
	"mov	[eax+4],ecx" \
	"mov	[eax+12],ecx" \
	"mov	[eax+20],ecx" \
	"fld	dword ptr [FSineTable + edx*4]" \
	"mov	[eax+28],ecx" \
	"mov	ecx,[FSineTable + edx*4]" \
	"fchs" \
	"mov	[eax+8],ecx" \
	"mov	ecx,0x3F800000" \
	"mov	edx,[FSineTable+2048 + edx*4]" \
	"mov	[eax+16],ecx" \
	"fstp	dword ptr [eax+24]" \
	"mov	dword ptr [eax+0],edx" \
	"mov	dword ptr [eax+32],edx" \
	parm [eax] [8087] \
	modify exact [ecx edx]

void BURGERCALL Matrix3DSetYaw(Matrix3D_t *Output,float yaw)
{
	FooMatrix3DSetYaw(Output,yaw);
}

/**********************************

	Create a rotation matrix with angle for
	pitch (Y)

**********************************/

extern void FooMatrix3DSetPitch(Matrix3D_t *Output,float pitch);

#pragma aux FooMatrix3DSetPitch = \
	"fistp	dword ptr [esp+12]" \
	"mov	edx,[esp+12]" \
	"xor	ecx,ecx" \
	"and	edx,0x7FF" \
	"mov	[eax+4],ecx" \
	"mov	[eax+8],ecx" \
	"mov	[eax+12],ecx" \
	"fld	dword ptr [FSineTable + edx*4]" \
	"mov	[eax+24],ecx" \
	"mov	ecx,[FSineTable + edx*4]" \
	"fchs" \
	"mov	[eax+20],ecx" \
	"mov	ecx,0x3F800000" \
	"mov	edx,[FSineTable+2048 + edx*4]" \
	"mov	[eax+0],ecx" \
	"fstp	dword ptr [eax+28]" \
	"mov	dword ptr [eax+16],edx" \
	"mov	dword ptr [eax+32],edx" \
	parm [eax] [8087] \
	modify exact [ecx edx]

void BURGERCALL Matrix3DSetPitch(Matrix3D_t *Output,float pitch)
{
	FooMatrix3DSetPitch(Output,pitch);
}

/**********************************

	Create a rotation matrix with angle for
	roll (Z)

**********************************/

extern void FooMatrix3DSetRoll(Matrix3D_t *Output,float roll);

#pragma aux FooMatrix3DSetRoll = \
	"fistp	dword ptr [esp+12]" \
	"mov	edx,[esp+12]" \
	"xor	ecx,ecx" \
	"and	edx,0x7FF" \
	"mov	[eax+8],ecx" \
	"mov	[eax+20],ecx" \
	"mov	[eax+24],ecx" \
	"fld	dword ptr [FSineTable + edx*4]" \
	"mov	[eax+28],ecx" \
	"mov	ecx,[FSineTable + edx*4]" \
	"fchs" \
	"mov	[eax+4],ecx" \
	"mov	ecx,0x3F800000" \
	"mov	edx,[FSineTable+2048 + edx*4]" \
	"mov	[eax+32],ecx" \
	"fstp	dword ptr [eax+12]" \
	"mov	dword ptr [eax+0],edx" \
	"mov	dword ptr [eax+16],edx" \
	parm [eax] [8087] \
	modify exact [ecx edx]

void BURGERCALL Matrix3DSetRoll(Matrix3D_t *Output,float roll)
{
	FooMatrix3DSetRoll(Output,roll);
}

/**********************************

	Create a rotation matrix with angles for
	the yaw (Y), pitch (X), and roll (Z)
	The angles are ints from 0-2047 (Rounded to 2048)

	x.x = cy*cz + sx*sy*sz x.y = cy*sz - sx*sy*cz  x.z = cx*sy
	y.x = -cx*sz           y.y = cx*cz             y.z = sx
	z.x = -sy*cz+sx*cy*sz  z.y = sy*-sz + sx*cy*cz z.z = cx*cy

**********************************/

extern void FooMatrix3DSetInt(Matrix3D_t *Output,Word yaw,Word pitch,Word roll);

#pragma aux FooMatrix3DSetInt = \
	"and	ecx,0x7FF" \
	"and	ebx,0x7FF" \
	"and	edx,0x7FF" \
	"fld	dword ptr [FSineTable + ecx*4]" \
	"fmul	dword ptr [FSineTable + ebx*4]" \
	"fld	dword ptr [FSineTable + 2048 + ecx*4]" \
	"fmul	dword ptr [FSineTable + ebx*4]" \
	"fld	dword ptr [FSineTable + 2048 + ecx*4]" \
	"fmul	dword ptr [FSineTable + 2048 + edx*4]" \
	"fld	dword ptr [FSineTable + ecx*4]" \
	"fmul	dword ptr [FSineTable + 2048 + edx*4]" \
	"fld	dword ptr [FSineTable + 2048 + ebx*4]" \
	"fmul	dword ptr [FSineTable + edx*4]" \
	"fld	dword ptr [FSineTable + edx*4]" \
	"fmul	st,st(5)" \
	"fld	dword ptr [FSineTable + edx*4]" \
	"fmul	st,st(5)" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+8]" \
	"faddp	st(3),st" \
	"fsubp	st(1),st" \
	"fld	dword ptr [FSineTable + ecx*4]" \
	"fmul	dword ptr [FSineTable + 2048 + ebx*4]" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+0]" \
	"fstp	dword ptr [eax+4]" \
	"fchs" \
	"fld	dword ptr [FSineTable + 2048 + ecx*4]" \
	"fmul	dword ptr [FSineTable + 2048 + ebx*4]" \
	"fxch	st(1)" \
	"fstp	dword ptr [eax+12]" \
	"fld	dword ptr [FSineTable + 2048 + ecx*4]" \
	"fmul	dword ptr [FSineTable + edx*4]" \
	"fxch	st(1)" \
	"fstp	dword ptr [eax+16]" \
	"fld	dword ptr [FSineTable + ecx*4]" \
	"fmul	dword ptr [FSineTable + edx*4]" \
	"fld	dword ptr [FSineTable + 2048 + ebx*4]" \
	"fmul	dword ptr [FSineTable + 2048 + edx*4]" \
	"fld	dword ptr [FSineTable + 2048 + edx*4]" \
	"fmulp	st(5),st" \
	"fld	dword ptr [FSineTable + 2048 + edx*4]" \
	"fmulp	st(4),st" \
	"fstp	dword ptr [eax+32]" \
	"fchs" \
	"mov	ebx,[FSineTable+ebx*4]" \
	"fsubrp	st(2),st" \
	"fsubp	st(2),st" \
	"mov	[eax+20],ebx" \
	"fstp	dword ptr [eax+28]" \
	"fstp	dword ptr [eax+24]" \
	parm [eax] [edx] [ebx] [ecx] modify exact [eax ebx]

void BURGERCALL Matrix3DSetInt(Matrix3D_t *Output,Word yaw,Word pitch,Word roll)
{
	FooMatrix3DSetInt(Output,yaw,pitch,roll);
}

/**********************************

	Create a rotation matrix with angles for
	the yaw (Y), pitch (X), and roll (Z)
	The angles are fixed point from 0-2047 (Rounded to 2048)

	x.x = cy*cz + sx*sy*sz x.y = cy*sz - sx*sy*cz  x.z = cx*sy
	y.x = -cx*sz           y.y = cx*cz             y.z = sx
	z.x = -sy*cz+sx*cy*sz  z.y = sy*-sz + sx*cy*cz z.z = cx*cy

**********************************/

extern void FooMatrix3DSetFixed(Matrix3D_t *Output,Fixed32 yaw,Fixed32 pitch,Fixed32 roll);

#pragma aux FooMatrix3DSetFixed = \
	"shr	ecx,16" \
	"shr	ebx,16" \
	"and	ecx,0x7FF" \
	"shr	edx,16" \
	"and	ebx,0x7FF" \
	"and	edx,0x7FF" \
	"fld	dword ptr [FSineTable + ecx*4]" \
	"fmul	dword ptr [FSineTable + ebx*4]" \
	"fld	dword ptr [FSineTable + 2048 + ecx*4]" \
	"fmul	dword ptr [FSineTable + ebx*4]" \
	"fld	dword ptr [FSineTable + 2048 + ecx*4]" \
	"fmul	dword ptr [FSineTable + 2048 + edx*4]" \
	"fld	dword ptr [FSineTable + ecx*4]" \
	"fmul	dword ptr [FSineTable + 2048 + edx*4]" \
	"fld	dword ptr [FSineTable + 2048 + ebx*4]" \
	"fmul	dword ptr [FSineTable + edx*4]" \
	"fld	dword ptr [FSineTable + edx*4]" \
	"fmul	st,st(5)" \
	"fld	dword ptr [FSineTable + edx*4]" \
	"fmul	st,st(5)" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+8]" \
	"faddp	st(3),st" \
	"fsubp	st(1),st" \
	"fld	dword ptr [FSineTable + ecx*4]" \
	"fmul	dword ptr [FSineTable + 2048 + ebx*4]" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+0]" \
	"fstp	dword ptr [eax+4]" \
	"fchs" \
	"fld	dword ptr [FSineTable + 2048 + ecx*4]" \
	"fmul	dword ptr [FSineTable + 2048 + ebx*4]" \
	"fxch	st(1)" \
	"fstp	dword ptr [eax+12]" \
	"fld	dword ptr [FSineTable + 2048 + ecx*4]" \
	"fmul	dword ptr [FSineTable + edx*4]" \
	"fxch	st(1)" \
	"fstp	dword ptr [eax+16]" \
	"fld	dword ptr [FSineTable + ecx*4]" \
	"fmul	dword ptr [FSineTable + edx*4]" \
	"fld	dword ptr [FSineTable + 2048 + ebx*4]" \
	"fmul	dword ptr [FSineTable + 2048 + edx*4]" \
	"fld	dword ptr [FSineTable + 2048 + edx*4]" \
	"fmulp	st(5),st" \
	"fld	dword ptr [FSineTable + 2048 + edx*4]" \
	"fmulp	st(4),st" \
	"fstp	dword ptr [eax+32]" \
	"fchs" \
	"mov	ebx,[FSineTable+ebx*4]" \
	"fsubrp	st(2),st" \
	"fsubp	st(2),st" \
	"mov	[eax+20],ebx" \
	"fstp	dword ptr [eax+28]" \
	"fstp	dword ptr [eax+24]" \
	parm [eax] [edx] [ebx] [ecx] modify exact [eax ebx]

void BURGERCALL Matrix3DSetFixed(Matrix3D_t *Output,Fixed32 yaw,Fixed32 pitch,Fixed32 roll)
{
	FooMatrix3DSetFixed(Output,yaw,pitch,roll);
}

/**********************************

	Same as above but with int	Perform a

**********************************/

/**********************************

	Perform a matrix multiply on the output matrix

**********************************/

extern void FooMatrix3DMul(Matrix3D_t *Output,const Matrix3D_t *Input);

#pragma aux FooMatrix3DMul = \
	"sub	esp,40" \
	"mov	[esp+36],ebx" \
	"mov	ebx,[eax+0]" \
	"mov	[esp+0],ebx" \
	"mov	ebx,[eax+4]" \
	"mov	[esp+4],ebx" \
	"mov	ebx,[eax+8]" \
	"mov	[esp+8],ebx" \
	"mov	ebx,[eax+12]" \
	"mov	[esp+12],ebx" \
	"mov	ebx,[eax+16]" \
	"mov	[esp+16],ebx" \
	"mov	ebx,[eax+20]" \
	"mov	[esp+20],ebx" \
	"mov	ebx,[eax+24]" \
	"mov	[esp+24],ebx" \
	"mov	ebx,[eax+28]" \
	"mov	[esp+28],ebx" \
	"mov	ebx,[eax+32]" \
	"mov	[esp+32],ebx" \
	"mov	ebx,edx" \
	"mov	edx,esp" \
	"call	Matrix3DMul2" \
	"mov	eax,[esp+40]" \
	"mov	ebx,[esp+36]" \
	"add	esp,44" \
	"jmp	eax" \
	parm [eax] [edx]

void BURGERCALL Matrix3DMul(Matrix3D_t *Output,const Matrix3D_t *Input)
{
	FooMatrix3DMul(Output,Input);
}

/**********************************

	Perform a matrix multiply
	Floating point will rot your brain on the Pentium

**********************************/

extern void FooMatrix3DMul2(Matrix3D_t *Output,const Matrix3D_t *Input1,const Matrix3D_t *Input2);

#pragma aux FooMatrix3DMul2 = \
	"fld	dword ptr [edx+0]" \
	"fmul	dword ptr [ebx+0]" \
	"fld	dword ptr [edx+12]" \
	"fmul	dword ptr [ebx+4]" \
	"fld	dword ptr [edx+24]" \
	"fmul	dword ptr [ebx+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [edx+4]" \
	"fmul	dword ptr [ebx+0]" \
	"fxch	st(2)" \
	"faddp	st(1),st" \
	"fld	dword ptr [edx+16]" \
	"fmul	dword ptr [ebx+4]" \
	"fld	dword ptr [edx+28]" \
	"fmul	dword ptr [ebx+8]" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+0]" \
	"faddp	st(2),st" \
	"fld	dword ptr [edx+8]" \
	"fmul	dword ptr [ebx+0]" \
	"fxch	st(2)" \
	"faddp	st(1),st" \
	"fld	dword ptr [edx+20]" \
	"fmul	dword ptr [ebx+4]" \
	"fld	dword ptr [edx+32]" \
	"fmul	dword ptr [ebx+8]" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+4]" \
	"faddp	st(2),st" \
	"fld	dword ptr [edx+0]" \
	"fmul	dword ptr [ebx+12]" \
	"fxch	st(2)" \
	"faddp	st(1),st" \
	"fld	dword ptr [edx+12]" \
	"fmul	dword ptr [ebx+16]" \
	"fld	dword ptr [edx+24]" \
	"fmul	dword ptr [ebx+20]" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+8]" \
	"faddp	st(2),st" \
	"fld	dword ptr [edx+4]" \
	"fmul	dword ptr [ebx+12]" \
	"fxch	st(2)" \
	"faddp	st(1),st" \
	"fld	dword ptr [edx+16]" \
	"fmul	dword ptr [ebx+16]" \
	"fld	dword ptr [edx+28]" \
	"fmul	dword ptr [ebx+20]" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+12]" \
	"faddp	st(2),st" \
	"fld	dword ptr [edx+8]" \
	"fmul	dword ptr [ebx+12]" \
	"fxch	st(2)" \
	"faddp	st(1),st" \
	"fld	dword ptr [edx+20]" \
	"fmul	dword ptr [ebx+16]" \
	"fld	dword ptr [edx+32]" \
	"fmul	dword ptr [ebx+20]" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+16]" \
	"faddp	st(2),st" \
	"fld	dword ptr [edx+0]" \
	"fmul	dword ptr [ebx+24]" \
	"fxch	st(2)" \
	"faddp	st(1),st" \
	"fld	dword ptr [edx+12]" \
	"fmul	dword ptr [ebx+28]" \
	"fld	dword ptr [edx+24]" \
	"fmul	dword ptr [ebx+32]" \
	"fxch	st(1)" \
	"faddp	st(3),st" \
	"fld	dword ptr [edx+4]" \
	"fmul	dword ptr [ebx+24]" \
	"fxch	st(3)" \
	"faddp	st(1),st" \
	"fld	dword ptr [edx+16]" \
	"fmul	dword ptr [ebx+28]" \
	"fld	dword ptr [edx+28]" \
	"fmul	dword ptr [ebx+32]" \
	"fxch	st(1)" \
	"faddp	st(4),st" \
	"fld	dword ptr [edx+8]" \
	"fmul	dword ptr [ebx+24]" \
	"fxch	st(4)" \
	"faddp	st(1),st" \
	"fxch	st(3)" \
	"fld	dword ptr [edx+20]" \
	"fmul	dword ptr [ebx+28]" \
	"fld	dword ptr [edx+32]" \
	"fmul	dword ptr [ebx+32]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+24]" \
	"faddp	st(1),st" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+28]" \
	"fstp	dword ptr [eax+20]" \
	"fstp	dword ptr [eax+32]" \
	parm [eax] [edx] [ebx]

void BURGERCALL Matrix3DMul2(Matrix3D_t *Output,const Matrix3D_t *Input1,const Matrix3D_t *Input2)
{
	FooMatrix3DMul2(Output,Input1,Input2);
}

/**********************************

	Multiply a vector by a matrix

**********************************/

extern void FooMatrix3DTransformVector3D(Vector3D_t *Output,const Matrix3D_t *Input);

#pragma aux FooMatrix3DTransformVector3D = \
	"fld	dword ptr [edx+0]" \
	"fmul	dword ptr [eax+0]" \
	"fld	dword ptr [edx+4]" \
	"fmul	dword ptr [eax+4]" \
	"fld	dword ptr [edx+8]" \
	"fmul	dword ptr [eax+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [edx+12]" \
	"fmul	dword ptr [eax+0]" \
	"fld	dword ptr [edx+16]" \
	"fmul	dword ptr [eax+4]" \
	"fld	dword ptr [edx+20]" \
	"fmul	dword ptr [eax+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [edx+24]" \
	"fmul	dword ptr [eax+0]" \
	"fld	dword ptr [edx+28]" \
	"fmul	dword ptr [eax+4]" \
	"fld	dword ptr [edx+32]" \
	"fmul	dword ptr [eax+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fxch	st(5)" \
	"faddp	st(4),st" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"faddp	st(3),st" \
	"fxch	st(1)" \
	"fstp	dword ptr [eax+0]" \
	"fstp	dword ptr [eax+4]" \
	"fstp	dword ptr [eax+8]" \
	parm [eax] [edx]

void BURGERCALL Matrix3DTransformVector3D(Vector3D_t *Output,const Matrix3D_t *Input)
{
	FooMatrix3DTransformVector3D(Output,Input);
}

/**********************************

	Multiply a vector by a matrix

**********************************/

extern void FooMatrix3DTransformVector3D2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Input2);

#pragma aux FooMatrix3DTransformVector3D2 = \
	"fld	dword ptr [edx+0]" \
	"fmul	dword ptr [ebx+0]" \
	"fld	dword ptr [edx+4]" \
	"fmul	dword ptr [ebx+4]" \
	"fld	dword ptr [edx+8]" \
	"fmul	dword ptr [ebx+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [edx+12]" \
	"fmul	dword ptr [ebx+0]" \
	"fld	dword ptr [edx+16]" \
	"fmul	dword ptr [ebx+4]" \
	"fld	dword ptr [edx+20]" \
	"fmul	dword ptr [ebx+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [edx+24]" \
	"fmul	dword ptr [ebx+0]" \
	"fld	dword ptr [edx+28]" \
	"fmul	dword ptr [ebx+4]" \
	"fld	dword ptr [edx+32]" \
	"fmul	dword ptr [ebx+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fxch	st(5)" \
	"faddp	st(4),st" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"faddp	st(3),st" \
	"fxch	st(1)" \
	"fstp	dword ptr [eax+0]" \
	"fstp	dword ptr [eax+4]" \
	"fstp	dword ptr [eax+8]" \
	parm [eax] [edx] [ebx]

void BURGERCALL Matrix3DTransformVector3D2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Input2)
{
	FooMatrix3DTransformVector3D2(Output,Input,Input2);
}

/**********************************

	Transform a vector and then add a point

**********************************/

extern void FooMatrix3DTransformVector3DAdd(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add);

#pragma aux FooMatrix3DTransformVector3DAdd = \
	"fld	dword ptr [eax+0]" \
	"fmul	dword ptr [edx+0]" \
	"fld	dword ptr [eax+4]" \
	"fmul	dword ptr [edx+4]" \
	"fld	dword ptr [eax+8]" \
	"fmul	dword ptr [edx+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [eax+0]" \
	"fmul	dword ptr [edx+12]" \
	"fld	dword ptr [eax+4]" \
	"fmul	dword ptr [edx+16]" \
	"fld	dword ptr [eax+8]" \
	"fmul	dword ptr [edx+20]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [eax+0]" \
	"fmul	dword ptr [edx+24]" \
	"fld	dword ptr [eax+4]" \
	"fmul	dword ptr [edx+28]" \
	"fld	dword ptr [eax+8]" \
	"fmul	dword ptr [edx+32]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fxch	st(4)" \
	"faddp	st(5),st" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"faddp	st(2),st" \
	"fxch	st(2)" \
	"fadd	dword ptr [ebx+0]" \
	"fxch	st(2)" \
	"fadd	dword ptr [ebx+4]" \
	"fxch	st(1)" \
	"fadd	dword ptr [ebx+8]" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+0]" \
	"fstp	dword ptr [eax+4]" \
	"fstp	dword ptr [eax+8]" \
	parm [EAX] [EDX] [EBX]

void BURGERCALL Matrix3DTransformVector3DAdd(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add)
{
	FooMatrix3DTransformVector3DAdd(Output,Input,Add);
}

/**********************************

	Transform a vector and then add a point

**********************************/

extern void FooMatrix3DTransformVector3DAdd2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add,const Vector3D_t *InputV);

#pragma aux FooMatrix3DTransformVector3DAdd2 = \
	"fld	dword ptr [ecx+0]" \
	"fmul	dword ptr [edx+0]" \
	"fld	dword ptr [ecx+4]" \
	"fmul	dword ptr [edx+4]" \
	"fld	dword ptr [ecx+8]" \
	"fmul	dword ptr [edx+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [ecx+0]" \
	"fmul	dword ptr [edx+12]" \
	"fld	dword ptr [ecx+4]" \
	"fmul	dword ptr [edx+16]" \
	"fld	dword ptr [ecx+8]" \
	"fmul	dword ptr [edx+20]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [ecx+0]" \
	"fmul	dword ptr [edx+24]" \
	"fld	dword ptr [ecx+4]" \
	"fmul	dword ptr [edx+28]" \
	"fld	dword ptr [ecx+8]" \
	"fmul	dword ptr [edx+32]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fxch	st(4)" \
	"faddp	st(5),st" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"faddp	st(2),st" \
	"fxch	st(2)" \
	"fadd	dword ptr [ebx+0]" \
	"fxch	st(2)" \
	"fadd	dword ptr [ebx+4]" \
	"fxch	st(1)" \
	"fadd	dword ptr [ebx+8]" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+0]" \
	"fstp	dword ptr [eax+4]" \
	"fstp	dword ptr [eax+8]" \
	parm [EAX] [EDX] [EBX] [ECX]

void BURGERCALL Matrix3DTransformVector3DAdd2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add,const Vector3D_t *InputV)
{
	FooMatrix3DTransformVector3DAdd2(Output,Input,Add,InputV);
}

/**********************************

	Multiply a vector by a matrix

**********************************/

extern void FooMatrix3DITransformVector3D(Vector3D_t *Output,const Matrix3D_t *Input);

#pragma aux FooMatrix3DITransformVector3D = \
	"fld	dword ptr [edx+0]" \
	"fmul	dword ptr [eax+0]" \
	"fld	dword ptr [edx+12]" \
	"fmul	dword ptr [eax+4]" \
	"fld	dword ptr [edx+24]" \
	"fmul	dword ptr [eax+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [edx+4]" \
	"fmul	dword ptr [eax+0]" \
	"fld	dword ptr [edx+16]" \
	"fmul	dword ptr [eax+4]" \
	"fld	dword ptr [edx+28]" \
	"fmul	dword ptr [eax+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [edx+8]" \
	"fmul	dword ptr [eax+0]" \
	"fld	dword ptr [edx+20]" \
	"fmul	dword ptr [eax+4]" \
	"fld	dword ptr [edx+32]" \
	"fmul	dword ptr [eax+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fxch	st(5)" \
	"faddp	st(4),st" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"faddp	st(3),st" \
	"fxch	st(1)" \
	"fstp	dword ptr [eax+0]" \
	"fstp	dword ptr [eax+4]" \
	"fstp	dword ptr [eax+8]" \
	parm [eax] [edx]

void BURGERCALL Matrix3DITransformVector3D(Vector3D_t *Output,const Matrix3D_t *Input)
{
	FooMatrix3DITransformVector3D(Output,Input);
}

/**********************************

	Multiply a vector by a matrix

**********************************/

extern void FooMatrix3DITransformVector3D2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Input2);

#pragma aux FooMatrix3DITransformVector3D2 = \
	"fld	dword ptr [edx+0]" \
	"fmul	dword ptr [ebx+0]" \
	"fld	dword ptr [edx+12]" \
	"fmul	dword ptr [ebx+4]" \
	"fld	dword ptr [edx+24]" \
	"fmul	dword ptr [ebx+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [edx+4]" \
	"fmul	dword ptr [ebx+0]" \
	"fld	dword ptr [edx+16]" \
	"fmul	dword ptr [ebx+4]" \
	"fld	dword ptr [edx+28]" \
	"fmul	dword ptr [ebx+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [edx+8]" \
	"fmul	dword ptr [ebx+0]" \
	"fld	dword ptr [edx+20]" \
	"fmul	dword ptr [ebx+4]" \
	"fld	dword ptr [edx+32]" \
	"fmul	dword ptr [ebx+8]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fxch	st(5)" \
	"faddp	st(4),st" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"faddp	st(3),st" \
	"fxch	st(1)" \
	"fstp	dword ptr [eax+0]" \
	"fstp	dword ptr [eax+4]" \
	"fstp	dword ptr [eax+8]" \
	parm [eax] [edx] [ebx]

void BURGERCALL Matrix3DITransformVector3D2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Input2)
{
	FooMatrix3DITransformVector3D2(Output,Input,Input2);
}

/**********************************

	Transform a vector and then add a point

**********************************/

extern void FooMatrix3DITransformVector3DAdd(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add);

#pragma aux FooMatrix3DITransformVector3DAdd = \
	"fld	dword ptr [eax+0]" \
	"fmul	dword ptr [edx+0]" \
	"fld	dword ptr [eax+4]" \
	"fmul	dword ptr [edx+12]" \
	"fld	dword ptr [eax+8]" \
	"fmul	dword ptr [edx+24]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [eax+0]" \
	"fmul	dword ptr [edx+4]" \
	"fld	dword ptr [eax+4]" \
	"fmul	dword ptr [edx+16]" \
	"fld	dword ptr [eax+8]" \
	"fmul	dword ptr [edx+28]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [eax+0]" \
	"fmul	dword ptr [edx+8]" \
	"fld	dword ptr [eax+4]" \
	"fmul	dword ptr [edx+20]" \
	"fld	dword ptr [eax+8]" \
	"fmul	dword ptr [edx+32]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fxch	st(4)" \
	"faddp	st(5),st" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"faddp	st(2),st" \
	"fxch	st(2)" \
	"fadd	dword ptr [ebx+0]" \
	"fxch	st(2)" \
	"fadd	dword ptr [ebx+4]" \
	"fxch	st(1)" \
	"fadd	dword ptr [ebx+8]" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+0]" \
	"fstp	dword ptr [eax+4]" \
	"fstp	dword ptr [eax+8]" \
	parm [EAX] [EDX] [EBX]

void BURGERCALL Matrix3DITransformVector3DAdd(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add)
{
	FooMatrix3DITransformVector3DAdd(Output,Input,Add);
}

/**********************************

	Transform a vector and then add a point

**********************************/

extern void FooMatrix3DITransformVector3DAdd2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add,const Vector3D_t *InputV);

#pragma aux FooMatrix3DITransformVector3DAdd2 = \
	"fld	dword ptr [ecx+0]" \
	"fmul	dword ptr [edx+0]" \
	"fld	dword ptr [ecx+4]" \
	"fmul	dword ptr [edx+12]" \
	"fld	dword ptr [ecx+8]" \
	"fmul	dword ptr [edx+24]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [ecx+0]" \
	"fmul	dword ptr [edx+4]" \
	"fld	dword ptr [ecx+4]" \
	"fmul	dword ptr [edx+16]" \
	"fld	dword ptr [ecx+8]" \
	"fmul	dword ptr [edx+28]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fld	dword ptr [ecx+0]" \
	"fmul	dword ptr [edx+8]" \
	"fld	dword ptr [ecx+4]" \
	"fmul	dword ptr [edx+20]" \
	"fld	dword ptr [ecx+8]" \
	"fmul	dword ptr [edx+32]" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"fxch	st(4)" \
	"faddp	st(5),st" \
	"fxch	st(1)" \
	"faddp	st(2),st" \
	"faddp	st(2),st" \
	"fxch	st(2)" \
	"fadd	dword ptr [ebx+0]" \
	"fxch	st(2)" \
	"fadd	dword ptr [ebx+4]" \
	"fxch	st(1)" \
	"fadd	dword ptr [ebx+8]" \
	"fxch	st(2)" \
	"fstp	dword ptr [eax+0]" \
	"fstp	dword ptr [eax+4]" \
	"fstp	dword ptr [eax+8]" \
	parm [EAX] [EDX] [EBX] [ECX]

void BURGERCALL Matrix3DITransformVector3DAdd2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add,const Vector3D_t *InputV)
{
	FooMatrix3DITransformVector3DAdd2(Output,Input,Add,InputV);
}
#endif
