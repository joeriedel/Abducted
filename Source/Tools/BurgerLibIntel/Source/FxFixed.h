/**********************************

	Calls to the integer math toolbox
	This is COMPLETELY machine independent

**********************************/

#ifndef __FXFIXED_H__
#define __FXFIXED_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct Vector3D_t;
struct Matrix3D_t;

/* Limit Values */

#define minLongint 0x80000000	/* Minimum negative signed long integer */
#define minFrac 0x80000000		/* Pinned value for negative Frac overflow */
#define minFixed 0x80000000		/* Pinned value for negative Fixed32 overflow */
#define minInt 0x8000			/* Minimum negative signed integer */
#define maxInt 0x7FFF			/* Maximum positive signed integer */
#define maxUInt 0xFFFFU			/* Maximum positive unsigned integer */
#define maxLongint 0x7FFFFFFFL	/* Maximum positive signed Longint */
#define maxFrac 0x7FFFFFFFL		/* Pinned value for positive Frac overflow */
#define maxFixed 0x7FFFFFFFL	/* Pinned value for positive Fixed32 overflow */
#define maxULong 0xFFFFFFFFUL	/* Maximum unsigned Long */
#define PiFixed 0x0032440L		/* Pi in fixed point */

typedef char Float80Bit[10];	/* 80 bit float (Extended) */

typedef struct FixedVector2D_t {
	Fixed32 x,y;
} FixedVector2D_t;

typedef struct FixedVector3D_t {
	Fixed32 x,y,z;
} FixedVector3D_t;

typedef struct FixedMatrix3D_t {
	FixedVector3D_t x,y,z;
} FixedMatrix3D_t;

typedef struct FixedQuat_t {
	Fixed32 x,y,z,w;
} FixedQuat_t;

typedef struct FixedMatrix4D_t {
	FixedQuat_t x,y,z,w;
} FixedMatrix4D_t;

#define FLOATTOFIXED(x) (Fixed32)((x)*65536.0f)
#define FIXEDTOFLOAT(x) (float)((x)*(1.0f/65536.0f))
#define INTTOFIXED(x) (Fixed32)((x)<<16)
#define FIXEDTOINT(x) FixedToInt(x)

extern Word BURGERCALL IMLoWord(Word32 Input);
extern Word BURGERCALL IMHiWord(Word32 Input);
extern Word32 BURGERCALL IMHexIt(Word Value);
extern Word32 BURGERCALL IMMultiply(Word InputA,Word InputB);
extern Word BURGERCALL IMIntSqrt(Word32 Input);
extern Word32 BURGERCALL IMFixSqrt(Word32 x);
extern long BURGERCALL IMIntMulRatioFast(long Mul1,long Mul2,long Div);
extern long BURGERCALL IntDblMulAdd(long Mul1,long Mul2,long Mul3,long Mul4);
extern long BURGERCALL IntMulHigh32(long Mul1,long Mul2);

extern Fixed32 BURGERCALL IMFixMul(Fixed32 InputA,Fixed32 InputB);
extern Fixed32 BURGERCALL IMFixMulFast(Fixed32 InputA,Fixed32 InputB);
extern Fixed32 BURGERCALL IMFixDiv(Fixed32 Numerator,Fixed32 Denominator);
extern Fixed32 BURGERCALL IMFixDivFast(Fixed32 Numerator,Fixed32 Denominator);
extern int BURGERCALL IMFixRound(Fixed32 Input);
extern Fixed32 BURGERCALL IMLong2Fix(long Input);
extern Fixed32 BURGERCALL IMFixReciprocal(Fixed32 Input);
extern Fixed32 BURGERCALL IMFixATan2(long Input1,long Input2);

extern Frac32 BURGERCALL IMFracCos(Fixed32 Input);
extern Frac32 BURGERCALL IMFracSin(Fixed32 Input);
extern Frac32 BURGERCALL IMFracMul(Frac32 InputA,Frac32 InputB);
extern Frac32 BURGERCALL IMFracDiv(Frac32 Numerator,Frac32 Denominator);

extern Fixed32 BURGERCALL FloatToFixed(float Input);
extern float BURGERCALL FixedToFloat(Fixed32 Input);
extern int BURGERCALL FixedToInt(Fixed32 Input);

extern void BURGERCALL FixedVector3DInit(FixedVector3D_t *Input,Fixed32 x,Fixed32 y,Fixed32 z);
#define FixedVector3DDestroy(x)
#define FixedVector3DSet(a,x,y,z) FixedVector3DInit(a,x,y,z)
extern void BURGERCALL FixedVector3DZero(FixedVector3D_t *Output);
extern void BURGERCALL FixedVector3DFromVector3D(FixedVector3D_t *Output,const struct Vector3D_t *Input);
extern void BURGERCALL IntVector3DFromVector3D(FixedVector3D_t *Output,const struct Vector3D_t *Input);
extern void BURGERCALL FixedVector3DNegate(FixedVector3D_t *Input);
extern void BURGERCALL FixedVector3DNegate2(FixedVector3D_t *Output,const FixedVector3D_t *Input);
extern void BURGERCALL FixedVector3DAdd(FixedVector3D_t *Output,const FixedVector3D_t *Input);
extern void BURGERCALL FixedVector3DAdd3(FixedVector3D_t *Output,const FixedVector3D_t *Input1,const FixedVector3D_t *Input2);
extern void BURGERCALL FixedVector3DSub(FixedVector3D_t *Output,const FixedVector3D_t *Input);
extern void BURGERCALL FixedVector3DSub3(FixedVector3D_t *Output,const FixedVector3D_t *Input1,const FixedVector3D_t *Input2);
extern void BURGERCALL FixedVector3DMul(FixedVector3D_t *Output,Fixed32 Val);
extern void BURGERCALL FixedVector3DMul3(FixedVector3D_t *Output,const FixedVector3D_t *Input,Fixed32 Val);
extern Word BURGERCALL FixedVector3DEqual(const FixedVector3D_t *Input1,const FixedVector3D_t *Input2);
extern Word BURGERCALL FixedVector3DEqualWithinRange(const FixedVector3D_t *Input1,const FixedVector3D_t *Input2,Fixed32 Range);
extern Fixed32 BURGERCALL FixedVector3DGetAxis(const FixedVector3D_t *Input,Word Axis);
extern void BURGERCALL FixedVector3DSetAxis(FixedVector3D_t *Output,Word Axis,Fixed32 Val);
extern Fixed32 BURGERCALL FixedVector3DDot(const FixedVector3D_t *Input1,const FixedVector3D_t *Input2);
extern void BURGERCALL FixedVector3DCross(FixedVector3D_t *Output,const FixedVector3D_t *Input1,const FixedVector3D_t *Input2);
extern Fixed32 BURGERCALL FixedVector3DGetRadiusSqr(const FixedVector3D_t *Input);
extern Fixed32 BURGERCALL FixedVector3DGetRadius(const FixedVector3D_t *Input);
extern Fixed32 BURGERCALL FixedVector3DGetRadiusFast(const FixedVector3D_t *Input);
extern void BURGERCALL FixedVector3DSetRadius(FixedVector3D_t *Input,Fixed32 Len);
extern void BURGERCALL FixedVector3DNormalize(FixedVector3D_t *Input);
extern void BURGERCALL FixedVector3DNormalizeFast(FixedVector3D_t *Input);
extern void BURGERCALL FixedVector3DNormalizeToLen(FixedVector3D_t *Input,Fixed32 Len);

#define FixedMatrix3DInit(x) FixedMatrix3DZero(x)
#define FixedMatrix3DDestroy(x)
extern void BURGERCALL FixedMatrix3DZero(FixedMatrix3D_t *Input);
extern void BURGERCALL FixedMatrix3DIdentity(FixedMatrix3D_t *Input);
extern void BURGERCALL FixedMatrix3DFromMatrix3D(FixedMatrix3D_t *Output,const struct Matrix3D_t *Input);
extern void BURGERCALL FixedMatrix3DSet(FixedMatrix3D_t *Output,Fixed32 yaw,Fixed32 pitch,Fixed32 roll);
extern void BURGERCALL FixedMatrix3DSetYaw(FixedMatrix3D_t *Output,Fixed32 yaw);
extern void BURGERCALL FixedMatrix3DSetPitch(FixedMatrix3D_t *Output,Fixed32 pitch);
extern void BURGERCALL FixedMatrix3DSetRoll(FixedMatrix3D_t *Output,Fixed32 roll);
extern void BURGERCALL FixedMatrix3DTranspose(FixedMatrix3D_t *Input);
extern void BURGERCALL FixedMatrix3DTranspose2(FixedMatrix3D_t *Output,const FixedMatrix3D_t *Input);
extern void BURGERCALL FixedMatrix3DMul(FixedMatrix3D_t *Output,const FixedMatrix3D_t *Input);
extern void BURGERCALL FixedMatrix3DMul2(FixedMatrix3D_t *Output,const FixedMatrix3D_t *Input1,const FixedMatrix3D_t *Input2);
extern void BURGERCALL FixedMatrix3DGetXVector(FixedVector3D_t *Output,const FixedMatrix3D_t *Input);
extern void BURGERCALL FixedMatrix3DGetYVector(FixedVector3D_t *Output,const FixedMatrix3D_t *Input);
extern void BURGERCALL FixedMatrix3DGetZVector(FixedVector3D_t *Output,const FixedMatrix3D_t *Input);
extern void BURGERCALL FixedMatrix3DMulVector(FixedVector3D_t *Output,const FixedMatrix3D_t *Input);
extern void BURGERCALL FixedMatrix3DMulVector2(FixedVector3D_t *Output,const FixedMatrix3D_t *Input,const FixedVector3D_t *Input2);
extern void BURGERCALL FixedMatrix3DMulVectorAddVector(FixedVector3D_t *Output,const FixedMatrix3D_t *Input,const FixedVector3D_t *Add);
extern void BURGERCALL FixedMatrix3DMulVectorAddVector2(FixedVector3D_t *Output,const FixedMatrix3D_t *Input,const FixedVector3D_t *Add,const FixedVector3D_t *InputV);

extern void BURGERCALL FixedQuatIdentity(FixedQuat_t *Input);

#if defined(__WATCOMC__)
#pragma aux IMFixMulFast = "imul edx" "shrd eax,edx,16" parm [eax] [edx] value [eax] modify exact [eax edx]
#pragma aux IMFixDivFast = "mov edx,eax" "shl eax,16" "sar edx,16" "idiv ebx" parm [eax] [ebx] value [eax] modify exact [eax edx]
#pragma aux IMIntMulRatioFast = "imul edx" "idiv ebx" parm [eax] [edx] [ebx] value [eax] modify exact [eax edx]
#pragma aux IntMulHigh32 = "imul edx" parm [eax] [edx] value [edx] modify exact [eax edx]
#elif defined(__68K__)
#pragma parameter __D0 IMIntMulRatioFast(__D0,__D1,__D2)
long IMIntMulRatioFast(long a,long b,long c) = {0x4c01,0xc01,0x4c42,0xc01};	/* muls.l d1,d1:d0 divs.l ds,d1:d0 */
#endif

#ifdef __cplusplus
}
#endif

#endif
