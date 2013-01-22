/*******************************

	The Burger Bill Universal library
	Main typedefs header (This is usually included
	by everything!!!)

*******************************/

#ifndef __BURGER__
#define __BURGER__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#if defined(__MAC__)
#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif
#endif

#include <stdio.h>
#include <string.h>

#if defined(__INTEL__) && !defined(__BEOS__)
#include <pshpack1.h>		/* Word8 align structures */
#elif defined(__MWERKS__)
#pragma options align=mac68k
#endif

#if defined(__MWERKS__)		/* Allow nameless structs */
#pragma ANSI_strict off
#pragma enumsalwaysint on
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LBPoint {	/* Point coord in 2D space */
	int	x;		/* X coord of point */
	int y;		/* Y coord of point */
} LBPoint;

typedef struct LBRect {	/* Rect coord in 2D space */
	int left;	/* Topleft x of rect */
	int top;	/* Topleft y of rect */
	int right;	/* Bottomright x of rect */
	int bottom;	/* Bottomright y of rect */
} LBRect;

typedef struct LBRectList {	/* Array of rects */
	Word NumRects;	/* Current number of rects in list */
	Word MaxRects;	/* Size of the array */
	LBRect **RectList;	/* Handle to array of rects */
} LBRectList;

#if defined(__MAC__)
typedef struct Point SYSPOINT;		/* Mac OS has a precompiler header! */
typedef struct Rect SYSRECT;
#elif defined(__BEOS__)
typedef struct {			/* BeOS! */
	float x;
	float y;
} SYSPOINT;
typedef struct {
	float left;
	float top;
	float right;
	float bottom;
} SYSRECT;
#else
typedef struct {			/* Dos and Windows! */
	long x;
	long y;
} SYSPOINT;
typedef struct {
	long left;
	long top;
	long right;
	long bottom;
} SYSRECT;
#endif

/* IBM specific data structures */

#define HANDLELOCK 0x80		/* Lock flag */
#define HANDLEFIXED 0x40	/* fixed memory flag */
#define HANDLEMALLOC 0x20	/* Memory was Malloc'd */
#define HANDLEINUSE 0x10	/* True if handle is used */
#define HANDLEPURGEBITS 0x01	/* Allow purge flag */

#define REZ_MEM_ID 0xFFF1	/* Resource manager memory ID */
#define DEBUG_MEM_ID 0xFFF2	/* Memory debugger memory ID */
#define DEAD_MEM_ID 0xFFFE	/* Dead memory space ID */

typedef struct MyHandle {
	void *MemPtr;			/* Pointer to true memory (Must be first!) */
	Word32 Length;		/* Length of memory */
	Word32 Flags;			/* Memory flags or parent used handle */
	struct MyHandle *NextHandle;	/* Next handle in the chain */
	struct MyHandle *PrevHandle;
} MyHandle;

/* 64 bit value handler */

#define LONGWORD64NATIVE
#if defined(__MWERKS__) || defined(__MRC__)
typedef long long LongWord64_t;
#elif defined(__WATCOMC__) || defined(__WIN32__)
typedef __int64 LongWord64_t;
#else
#undef LONGWORD64NATIVE
typedef struct LongWord64_t {
#if defined(__BIGENDIAN__)
	long hi;		/* Upper 32 bits */
	Word32 lo;	/* Lower 32 bits */
#else
	Word32 lo;	/* Lower 32 bits */
	long hi;		/* Upper 32 bits */
#endif
} LongWord64_t;
#endif

/* 64 bit value handler */

#ifdef LONGWORD64NATIVE
#define LongWord64Add(Output,Input) (Output)[0]+=(Input)[0]
#define LongWord64Add3(Output,First,Second) (Output)[0] = (First)[0]+(Second)[0]
#define LongWord64Sub(Output,Input) (Output)[0]-=(Input)[0]
#define LongWord64Sub3(Output,First,Second) (Output)[0] = (First)[0]-(Second)[0]
#define LongWord64Mul(Output,Input) (Output)[0]*=(Input)[0]
#define LongWord64Mul3(Output,First,Second) (Output)[0] = (First)[0]*(Second)[0]
#define LongWord64ToDouble(Input) (double)((Input)[0])
#define LongWord64FromDouble(Output,Input) (Output)[0] = (LongWord64_t)Input
#define LongWord64MulLongTo64(Output,First,Second) (Output)[0] = XLongWord64MulLongTo64(First,Second)
#define LongWord64Negate(Input) (Input)[0] = -(Input)[0]
#define LongWord64FromLong(Output,Input) (Output)[0] = (LongWord64_t)Input
#define LongWord64FromLong2(x,y,z) ((x)[0] = ((LongWord64_t)y)+((LongWord64_t)z<<32))
#define LongWord64ToLong(Input) (long)((Input)[0])
#define LongWord64ToHiLong(Input) (long)((Input)[0]>>32)
#define LongWord64Compare(First,Second) XLongWord64Compare((First)[0],(Second)[0])
#define LongWord64DivideByLong(Numerator,Denominator) (Numerator)[0] = XLongWord64DivideByLong((Numerator)[0],Denominator)
extern LongWord64_t BURGERCALL XLongWord64MulLongTo64(long First,long Second);
extern int BURGERCALL XLongWord64Compare(LongWord64_t First,LongWord64_t Second);
extern LongWord64_t BURGERCALL XLongWord64DivideByLong(LongWord64_t Numerator,long Denominator);
#else
extern void BURGERCALL LongWord64Add(LongWord64_t *Output,const LongWord64_t *Input);
extern void BURGERCALL LongWord64Add3(LongWord64_t *Output,const LongWord64_t *First,const LongWord64_t *Second);
extern void BURGERCALL LongWord64Sub(LongWord64_t *Output,const LongWord64_t *Input);
extern void BURGERCALL LongWord64Sub3(LongWord64_t *Output,const LongWord64_t *First,const LongWord64_t *Second);
extern void BURGERCALL LongWord64Mul(LongWord64_t *Output,const LongWord64_t *Input);
extern void BURGERCALL LongWord64Mul3(LongWord64_t *Output,const LongWord64_t *First,const LongWord64_t *Second);
extern double BURGERCALL LongWord64ToDouble(const LongWord64_t *Input);
extern void BURGERCALL LongWord64FromDouble(LongWord64_t *Output,double Input);
extern void BURGERCALL LongWord64MulLongTo64(LongWord64_t *Output,long First,long Second);
extern void BURGERCALL LongWord64Negate(LongWord64_t *Input);
extern void BURGERCALL LongWord64FromLong(LongWord64_t *Output,long Input);
#define LongWord64FromLong2(x,y,z) ((x).lo=y),((x.hi)=z)
#define LongWord64ToLong(Input) (long)((Input).lo)
#define LongWord64ToHiLong(Input) (long)((Input).hi)
extern int BURGERCALL LongWord64Compare(LongWord64_t *First,LongWord64_t *Second);
#endif

#if defined(__WATCOMC__)
#pragma aux XLongWord64MulLongTo64 = "imul edx" parm [eax] [edx] value [eax edx]
#pragma aux XLongWord64DivideByLong = "idiv ebx" parm [eax] [edx] [ebx] modify exact [eax edx] value [eax edx]
#endif

/* Math functions */

typedef struct Vector2D_t {
	float x,y;
} Vector2D_t;

typedef struct Vector3D_t {
	float x,y,z;
} Vector3D_t;

typedef struct Matrix3D_t {
	Vector3D_t x,y,z;
} Matrix3D_t;

typedef struct Quat_t {
	float x,y,z,w;
} Quat_t;

typedef struct Matrix4D_t {
	Quat_t x,y,z,w;
} Matrix4D_t;

typedef struct FixedEuler_t {
	Fixed32 x,y,z;
} FixedEuler_t;

typedef struct Euler_t {
	int x,y,z;
} Euler_t;

#define	Pi 3.141592653589793238		/* Pretty accurate eh? */
#define ANGLERANGE 2048		   		/* Number of angles in angle table */
#define ANGLEMASK (ANGLERANGE-1)	/* Rounding mask for angles */
#define TANTABLESIZE 2048			/* Size of the Tangent arrays */
#define FLOATRECIPTABLESIZE 1024	/* Size of the float recip table */

/* Math functions */

extern float FAngleArray[TANTABLESIZE+1];
extern Fixed32 FixedArcCosineTable[ANGLERANGE+1];
extern Fixed32 FixedArcSineTable[ANGLERANGE+1];
extern float FSineTable[ANGLERANGE+(ANGLERANGE/4)];
extern float FloatRecipTable[FLOATRECIPTABLESIZE];
extern float BURGERCALL SqrtFast(float Input);
extern float BURGERCALL CeilingFast(float Input);
extern int BURGERCALL CeilingFastInt(float Input);
extern float BURGERCALL FloorFast(float Input);
extern int BURGERCALL FloorFastInt(float Input);
extern int BURGERCALL FloatToInt(float Input);
extern float BURGERCALL FloatSine(float Angle);
extern float BURGERCALL FloatCosine(float Angle);
extern float BURGERCALL FloatSineInt(int Angle);
extern float BURGERCALL FloatCosineInt(int Angle);
extern float BURGERCALL FloatATan2(float x,float y);
extern Fixed32 BURGERCALL FixedArcCosine(Fixed32 c);
extern Fixed32 BURGERCALL FloatArcCosine(float c);
extern Fixed32 BURGERCALL FixedArcSine(Fixed32 s);
extern Fixed32 BURGERCALL FloatArcSine(float s);
extern float BURGERCALL FloatAbs(float Input);

extern void BURGERCALL Vector2DInit(Vector2D_t *Input,float x,float y);
#define Vector2DDestroy(x)
#define Vector2DSet(a,x,y) Vector2DInit(a,x,y)
extern void BURGERCALL Vector2DZero(Vector2D_t *Output);
extern void BURGERCALL Vector2DNegate(Vector2D_t *Input);
extern void BURGERCALL Vector2DNegate2(Vector2D_t *Output,const Vector2D_t *Input);
extern float BURGERCALL Vector2DDot(const Vector2D_t *Input1,const Vector2D_t *Input2);
extern float BURGERCALL Vector2DCross(const Vector2D_t *Input1,const Vector2D_t *Input2);
extern float BURGERCALL Vector2DGetRadiusSqr(const Vector2D_t *Input);
extern float BURGERCALL Vector2DGetRadius(const Vector2D_t *Input);
extern float BURGERCALL Vector2DGetRadiusFast(const Vector2D_t *Input);
extern float BURGERCALL Vector2DGetDistanceSqr(const Vector2D_t *Input1,const Vector2D_t *Input2);
extern float BURGERCALL Vector2DGetDistance(const Vector2D_t *Input1,const Vector2D_t *Input2);
extern float BURGERCALL Vector2DGetDistanceFast(const Vector2D_t *Input1,const Vector2D_t *Input2);
extern void BURGERCALL Vector2DNormalize(Vector2D_t *Input);
extern void BURGERCALL Vector2DNormalize2(Vector2D_t *Output,const Vector2D_t *Input);
extern void BURGERCALL Vector2DNormalize3(Vector2D_t *Output,float x,float y);
extern void BURGERCALL Vector2DNormalizeFast(Vector2D_t *Input);
extern void BURGERCALL Vector2DNormalizeFast2(Vector2D_t *Output,const Vector2D_t *Input);
extern void BURGERCALL Vector2DNormalizeFast3(Vector2D_t *Output,float x,float y);

extern void BURGERCALL Vector3DInit(Vector3D_t *Input,float x,float y,float z);
#define Vector3DSet(a,x,y,z) Vector3DInit(a,x,y,z)
extern void BURGERCALL Vector3DZero(Vector3D_t *Output);
extern void BURGERCALL Vector3DFromIntVector3D(Vector3D_t *Output,const struct FixedVector3D_t *Input);
extern void BURGERCALL Vector3DFromFixedVector3D(Vector3D_t *Output,const struct FixedVector3D_t *Input);
extern void BURGERCALL Vector3DNegate(Vector3D_t *Input);
extern void BURGERCALL Vector3DNegate2(Vector3D_t *Output,const Vector3D_t *Input);
extern void BURGERCALL Vector3DAdd(Vector3D_t *Output,const Vector3D_t *Input);
extern void BURGERCALL Vector3DAdd3(Vector3D_t *Output,const Vector3D_t *Input1,const Vector3D_t *Input2);
extern void BURGERCALL Vector3DSub(Vector3D_t *Output,const Vector3D_t *Input);
extern void BURGERCALL Vector3DSub3(Vector3D_t *Output,const Vector3D_t *Input1,const Vector3D_t *Input2);
extern void BURGERCALL Vector3DMul(Vector3D_t *Output,float Val);
extern void BURGERCALL Vector3DMul3(Vector3D_t *Output,const Vector3D_t *Input,float Val);
extern Word BURGERCALL Vector3DEqual(const Vector3D_t *Input1,const Vector3D_t *Input2);
extern Word BURGERCALL Vector3DEqualWithinRange(const Vector3D_t *Input1,const Vector3D_t *Input2,float Range);
extern float BURGERCALL Vector3DGetAxis(const Vector3D_t *Input,Word Axis);
extern void BURGERCALL Vector3DSetAxis(Vector3D_t *Output,Word Axis,float Val);
extern float BURGERCALL Vector3DDot(const Vector3D_t *Input1,const Vector3D_t *Input2);
extern void BURGERCALL Vector3DCross(Vector3D_t *Output,const Vector3D_t *Input1,const Vector3D_t *Input2);
extern float BURGERCALL Vector3DGetRadiusSqr(const Vector3D_t *Input);
extern float BURGERCALL Vector3DGetRadius(const Vector3D_t *Input);
extern float BURGERCALL Vector3DGetRadiusFast(const Vector3D_t *Input);
extern float BURGERCALL Vector3DGetDistanceSqr(const Vector3D_t *Input1,const Vector3D_t *Input2);
extern float BURGERCALL Vector3DGetDistance(const Vector3D_t *Input1,const Vector3D_t *Input2);
extern float BURGERCALL Vector3DGetDistanceFast(const Vector3D_t *Input1,const Vector3D_t *Input2);
extern void BURGERCALL Vector3DSetRadius(Vector3D_t *Input,float Len);
extern void BURGERCALL Vector3DNormalize(Vector3D_t *Input);
extern void BURGERCALL Vector3DNormalize2(Vector3D_t *Output,const Vector3D_t *Input);
extern void BURGERCALL Vector3DNormalize3(Vector3D_t *Output,float x,float y,float z);
extern void BURGERCALL Vector3DNormalizeFast(Vector3D_t *Input);
extern void BURGERCALL Vector3DNormalizeFast2(Vector3D_t *Output,const Vector3D_t *Input);
extern void BURGERCALL Vector3DNormalizeFast3(Vector3D_t *Output,float x,float y,float z);
extern void BURGERCALL Vector3DNormalizeToLen(Vector3D_t *Input,float Len);
extern void BURGERCALL Vector3DNormalizeToLen2(Vector3D_t *Output,const Vector3D_t *Input,float Len);

#define Matrix3DInit(x) Matrix3DZero(x)
extern void BURGERCALL Matrix3DZero(Matrix3D_t *Input);
extern void BURGERCALL Matrix3DIdentity(Matrix3D_t *Input);
extern void BURGERCALL Matrix3DFromFixedMatrix3D(Matrix3D_t *Output,const struct FixedMatrix3D_t *Input);
extern void BURGERCALL Matrix3DSet(Matrix3D_t *Output,float yaw,float pitch,float roll);
extern void BURGERCALL Matrix3DSetYaw(Matrix3D_t *Output,float yaw);
extern void BURGERCALL Matrix3DSetPitch(Matrix3D_t *Output,float pitch);
extern void BURGERCALL Matrix3DSetRoll(Matrix3D_t *Output,float roll);
extern void BURGERCALL Matrix3DSetInt(Matrix3D_t *Output,Word yaw,Word pitch,Word roll);
extern void BURGERCALL Matrix3DSetYawInt(Matrix3D_t *Output,Word yaw);
extern void BURGERCALL Matrix3DSetPitchInt(Matrix3D_t *Output,Word pitch);
extern void BURGERCALL Matrix3DSetRollInt(Matrix3D_t *Output,Word roll);
extern void BURGERCALL Matrix3DSetFixed(Matrix3D_t *Output,Fixed32 yaw,Fixed32 pitch,Fixed32 roll);
extern void BURGERCALL Matrix3DSetYawFixed(Matrix3D_t *Output,Fixed32 yaw);
extern void BURGERCALL Matrix3DSetPitchFixed(Matrix3D_t *Output,Fixed32 pitch);
extern void BURGERCALL Matrix3DSetRollFixed(Matrix3D_t *Output,Fixed32 roll);
extern void BURGERCALL Matrix3DSetFromEuler(Matrix3D_t *Output,const Euler_t *Input);
extern void BURGERCALL Matrix3DSetFromFixedEuler(Matrix3D_t *Output,const FixedEuler_t *Input);
extern void BURGERCALL Matrix3DSetFromQuat(Matrix3D_t *Output,const Quat_t *Input);
extern void BURGERCALL Matrix3DSetTranslate2D(Matrix3D_t *Input,float xVal,float yVal);
extern void BURGERCALL Matrix3DTranspose(Matrix3D_t *Input);
extern void BURGERCALL Matrix3DTranspose2(Matrix3D_t *Output,const Matrix3D_t *Input);
extern void BURGERCALL Matrix3DMul(Matrix3D_t *Output,const Matrix3D_t *Input);
extern void BURGERCALL Matrix3DMul2(Matrix3D_t *Output,const Matrix3D_t *Input1,const Matrix3D_t *Input2);
extern void BURGERCALL Matrix3DGetXVector(Vector3D_t *Output,const Matrix3D_t *Input);
extern void BURGERCALL Matrix3DGetYVector(Vector3D_t *Output,const Matrix3D_t *Input);
extern void BURGERCALL Matrix3DGetZVector(Vector3D_t *Output,const Matrix3D_t *Input);
extern void BURGERCALL Matrix3DTransformVector3D(Vector3D_t *Output,const Matrix3D_t *Input);
extern void BURGERCALL Matrix3DTransformVector3D2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Input2);
extern void BURGERCALL Matrix3DTransformVector3DAdd(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add);
extern void BURGERCALL Matrix3DTransformVector3DAdd2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add,const Vector3D_t *InputV);
extern void BURGERCALL Matrix3DITransformVector3D(Vector3D_t *Output,const Matrix3D_t *Input);
extern void BURGERCALL Matrix3DITransformVector3D2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Input2);
extern void BURGERCALL Matrix3DITransformVector3DAdd(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add);
extern void BURGERCALL Matrix3DITransformVector3DAdd2(Vector3D_t *Output,const Matrix3D_t *Input,const Vector3D_t *Add,const Vector3D_t *InputV);
extern void BURGERCALL Matrix3DTransform2D(Vector2D_t *Output,const Matrix3D_t *Input);
extern void BURGERCALL Matrix3DTransform2D2(Vector2D_t *Output,const Matrix3D_t *Input,const Vector2D_t *InputV);
extern void BURGERCALL Matrix3DITransform2D(Vector2D_t *Output,const Matrix3D_t *Input);
extern void BURGERCALL Matrix3DITransform2D2(Vector2D_t *Output,const Matrix3D_t *Input,const Vector2D_t *InputV);

extern void BURGERCALL EulerFromMatrix3D(Euler_t *Output,const Matrix3D_t *Input);
extern void BURGERCALL FixedEulerFromMatrix3D(FixedEuler_t *Output,const Matrix3D_t *Input);

extern void BURGERCALL QuatIdentity(Quat_t *Input);
extern void BURGERCALL QuatNormalize(Quat_t *Input);
extern float BURGERCALL QuatDot(const Quat_t *Input1,const Quat_t *Input2);
extern void BURGERCALL QuatMul2(Quat_t *Output,const Quat_t *Input1,const Quat_t *Input2);

extern void BURGERCALL Matrix4DZero(Matrix4D_t *Input);
extern void BURGERCALL Matrix4DIdentity(Matrix4D_t *Input);
extern void BURGERCALL Matrix4DSetTranslate3D(Matrix4D_t *Output,float xVal,float yVal,float zVal);
extern void BURGERCALL Matrix4DSetScale(Matrix4D_t *Output,float xVal,float yVal,float zVal);
extern void BURGERCALL Matrix4DTransformVector3D(Vector3D_t *Output,const Matrix4D_t *Input);
extern void BURGERCALL Matrix4DTransformVector3D2(Vector3D_t *Output,const Matrix4D_t *Input,const Vector3D_t *Input2);
extern void BURGERCALL Matrix4DITransformVector3D(Vector3D_t *Output,const Matrix4D_t *Input);
extern void BURGERCALL Matrix4DITransformVector3D2(Vector3D_t *Output,const Matrix4D_t *Input,const Vector3D_t *Input2);

/* Inlines... */

#if defined(__INTEL__) || defined(__POWERPC__)
#define Vector3DZero(a) {(a)->x=0;(a)->y=0;(a)->z=0;}
#define Vector3DNegate(a) {(a)->x=-(a)->x;(a)->y=-(a)->y;(a)->z=-(a)->z;}
#define Vector3DNegate2(a,b) {(a)->x=-(b)->x;(a)->y=-(b)->y;(a)->z=-(b)->z;}
#define Vector3DAdd(a,b) {(a)->x=(a)->x+(b)->x;(a)->y=(a)->y+(b)->y;(a)->z=(a)->z+(b)->z;}
#define Vector3DAdd3(a,b,c) {(a)->x=(b)->x+(c)->x;(a)->y=(b)->y+(c)->y;(a)->z=(b)->z+(c)->z;}
#define Vector3DSub(a,b) {(a)->x=(a)->x-(b)->x;(a)->y=(a)->y-(b)->y;(a)->z=(a)->z-(b)->z;}
#define Vector3DSub3(a,b,c) {(a)->x=(b)->x-(c)->x;(a)->y=(b)->y-(c)->y;(a)->z=(b)->z-(c)->z;}
#define Vector3DMul(a,b) {(a)->x=(a)->x*(b);(a)->y=(a)->y*(b);(a)->z=(a)->z*(b);}
#define Vector3DMul3(a,b,c) {(a)->x=(b)->x*(c);(a)->y=(b)->y*(c);(a)->z=(b)->z*(c);}
#define Vector3DDot(a,b) (((a)->x*(b)->x) + ((a)->y*(b)->y) + ((a)->z * (b)->z))
#endif

#if defined(__INTEL__)
#define FloatAbs(x) (float)fabs(x)
#elif defined(__POWERPC__)
#define FloatAbs(x) (float)__fabs(x)
#endif

/* Fixed32 point math */

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
#elif defined(__68K__)
#pragma parameter __D0 IMIntMulRatioFast(__D0,__D1,__D2)
long IMIntMulRatioFast(long a,long b,long c) = {0x4c01,0xc01,0x4c42,0xc01};	/* muls.l d1,d1:d0 divs.l ds,d1:d0 */
#endif

/* Time and events */

typedef struct TimeDate_t {	/* Used by FileModTime routines */
	Word32 Year;		/* Year 2002 */
	Word16 Milliseconds;	/* 0-999 */
	Word8 Month;		/* 1-12 */
	Word8 Day;		/* 1-31 */
	Word8 DayOfWeek;	/* 0-6 */
	Word8 Hour;		/* 0-23 */
	Word8 Minute;	/* 0-59 */
	Word8 Second;	/* 0-59 */
} TimeDate_t;

#define TICKSPERSEC 60
extern Word32 LastTick;
extern Word32 BURGERCALL ReadTick(void);
#define ResetLastTick() LastTick=ReadTick()
#define WaitEvent() WaitTicksEvent(0)
#define WaitOneTick() ResetLastTick(),WaitTick()
#define WaitTick() WaitTicks(1)
extern void BURGERCALL WaitTicks(Word TickCount);
extern Word BURGERCALL WaitTicksEvent(Word TickCount);
extern Word32 BURGERCALL ReadTickMicroseconds(void);
extern Word32 BURGERCALL ReadTickMilliseconds(void);
extern void BURGERCALL TimeDateGetCurrentTime(TimeDate_t *Input);
extern Word BURGERCALL TimeDateFromANSITime(TimeDate_t *Output,Word32 Input);
extern void BURGERCALL TimeDateTimeString(char *Output,const TimeDate_t *Input);
extern void BURGERCALL TimeDateTimeStringPM(char *Output,const TimeDate_t *Input);
extern void BURGERCALL TimeDateDateString(char *Output,const TimeDate_t *Input);
extern void BURGERCALL TimeDateDateStringVerbose(char *Output,const TimeDate_t *Input);
extern void BURGERCALL TimeDateStreamHandleRead(TimeDate_t *Output,struct StreamHandle_t *Input);
extern void BURGERCALL TimeDateStreamHandleWrite(const TimeDate_t *Input,struct StreamHandle_t *Output);

/* Thread manager */

typedef Word32 Mutex_t;
typedef struct TimerTask_t TimerTask_t;
typedef Word (BURGERCALL *TimerTaskProc)(void *DataPtr);

extern Word BURGERCALL MutexLock(Mutex_t *Input);
extern void BURGERCALL MutexUnlock(Mutex_t *Input);
extern TimerTask_t * BURGERCALL TimerTaskNew(Word Period,TimerTaskProc Proc,void *ProcData,Word Active);
extern void BURGERCALL TimerTaskDelete(TimerTask_t *Input);
extern Word BURGERCALL TimerTaskGetActive(const TimerTask_t *Input);
extern Word BURGERCALL TimerTaskSetActive(TimerTask_t *Input,Word Flag);
extern Word BURGERCALL TimerTaskGetPeriod(const TimerTask_t *Input);
extern Word BURGERCALL TimerTaskSetPeriod(TimerTask_t *Input,Word Period);

/* In Palette */

typedef struct HSL_t {
	float Hue;			/* Color hue 0 to 1 */
	float Saturation;	/* Color saturation 0 to 1 */
	float Luminance;	/* Color luminance 0 to 1 */
} HSL_t;

typedef struct RGB_t {
	float Red;			/* Red intensity 0 to 1 */
	float Green;		/* Green intensity 0 to 1 */
	float Blue;			/* Blue intensity 0 to 1 */
} RGB_t;

typedef void (BURGERCALL *PaletteChangedProcPtr)(void);
typedef void (BURGERCALL *PaletteFadeCallBackProcPtr)(Word Pass);

extern Word8 CurrentPalette[256*3];		/* Current palette in the hardware (Read ONLY) */
extern const Word ByteSquareTable[255+256];	/* Table of -255 to 255 squared */
extern const Word8 RGB5ToRGB8Table[32];	/* Table to convert 5 bit color to 8 bit color */
extern const Word8 RGB6ToRGB8Table[64];	/* Table to convert 6 bit color to 8 bit color */
extern Word FadeSpeed;			/* Delay in Ticks for a palette change */
extern Word PaletteVSync;		/* Set to TRUE if the palette MUST be changed only during VSync */
extern PaletteFadeCallBackProcPtr PaletteFadeCallBack;
extern PaletteChangedProcPtr PaletteChangedProc;	/* Called whenever the palette was changed */

extern void BURGERCALL PaletteConvertRGB15ToRGB24(Word8 *RGBOut,Word RGBIn);
extern void BURGERCALL PaletteConvertRGB16ToRGB24(Word8 *RGBOut,Word RGBIn);
extern Word BURGERCALL PaletteConvertRGB24ToRGB15(const Word8 *RGBIn);
extern Word BURGERCALL PaletteConvertRGB24ToRGB16(const Word8 *RGBIn);
extern Word BURGERCALL PaletteConvertRGB24ToDepth(const Word8 *RGBIn,Word Depth);
extern Word BURGERCALL PaletteConvertRGBToDepth(Word Red,Word Green,Word Blue,Word Depth);
extern Word BURGERCALL PaletteConvertPackedRGBToDepth(Word32 Color,Word Depth);
extern void BURGERCALL PaletteMake16BitLookup(Word *Output,const Word8 *Input,Word Depth);
extern void BURGERCALL PaletteMake16BitLookupRez(Word *Output,struct RezHeader_t *Input,Word RezNum,Word Depth);
extern void BURGERCALL PaletteMakeRemapLookup(Word8 *Output,const Word8 *DestPal,const Word8 *SourcePal);
extern void BURGERCALL PaletteMakeRemapLookupMasked(Word8 *Output,const Word8 *DestPal,const Word8 *SourcePal);
extern void BURGERCALL PaletteMakeColorMasks(Word8 *Output,Word MaskColor);
extern void BURGERCALL PaletteMakeFadeLookup(Word8 *Output,const Word8 *Input,Word r,Word g,Word b);
extern Word BURGERCALL PaletteFindColorIndex(const Word8 *PalPtr,Word Red,Word Green,Word Blue,Word Count);
extern void BURGERCALL PaletteBlack(void);
extern void BURGERCALL PaletteWhite(void);
extern void BURGERCALL PaletteFadeToBlack(void);
extern void BURGERCALL PaletteFadeToWhite(void);
extern void BURGERCALL PaletteFadeTo(struct RezHeader_t *Input,Word ResID);
extern void BURGERCALL PaletteFadeToPtr(const Word8 *PalettePtr);
extern void BURGERCALL PaletteFadeToHandle(void **PaletteHandle);
extern void BURGERCALL PaletteSet(struct RezHeader_t *Input,Word PalNum);
extern void BURGERCALL PaletteSetHandle(Word Start,Word Count,void **PaletteHandle);
extern void BURGERCALL PaletteSetPtr(Word Start,Word Count,const Word8 *PalettePtr);
extern Word BURGERCALL PaletteGetBorderColor(void);
extern void BURGERCALL PaletteSetBorderColor(Word Color);
extern void BURGERCALL PaletteRGB2HSL(HSL_t *Output,const RGB_t *Input);
extern void BURGERCALL PaletteHSL2RGB(RGB_t *Output,const HSL_t *Input);
extern void BURGERCALL PaletteHSLTween(HSL_t *Output,const HSL_t *HSLPtr1,const HSL_t *HSLPtr2,float Factor,Word Dir);
extern void BURGERCALL PaletteRGBTween(RGB_t *Output,const RGB_t *RGBPtr1,const RGB_t *RGBPtr2,float Factor,Word Dir);

/* Input handlers */

#define PadLeft 0x1UL
#define PadRight 0x2UL
#define PadUp 0x4UL
#define PadDown 0x8UL
#define PadHatLeft 0x10UL
#define PadHatRight 0x20UL
#define PadHatUp 0x40UL
#define PadHatDown 0x80UL
#define PadThrottleUp 0x100UL
#define PadThrottleDown 0x200UL
#define PadTwistLeft 0x400UL
#define PadTwistRight 0x800UL
#define PadButton1 0x1000UL
#define PadButton2 0x2000UL
#define PadButton3 0x4000UL
#define PadButton4 0x8000UL
#define PadButton5 0x10000UL
#define PadButton6 0x20000UL
#define PadButton7 0x40000UL
#define PadButton8 0x80000UL
#define PadButton9 0x100000UL
#define PadButton10 0x200000UL
#define PadButton11 0x400000UL
#define PadButton12 0x800000UL
#define PadButton13 0x1000000UL
#define PadButton14 0x2000000UL
#define PadButton15 0x4000000UL
#define PadButton16 0x8000000UL
#define PadButton17 0x10000000UL
#define PadButton18 0x20000000UL
#define PadButton19 0x40000000UL
#define PadButton20 0x80000000UL

typedef struct JoyAutoRepeat_t {	/* Used by JoyAutoRepeater */
	Word32 JoyBits;	/* Bit field to test for */
	Word InitialTick;	/* Delay for initial joydown */
	Word RepeatTick;	/* Delay for repeater */
	Word32 TimeMark;	/* Internal time mark */
	Word HeldDown;		/* Zero this to init the struct */
} JoyAutoRepeat_t;

typedef void (BURGERCALL *KeyboardCallBack)(void *);
typedef Word (BURGERCALL *KeyboardGetchCallBackPtr)(Word Key);
typedef struct ForceFeedback_t ForceFeedback_t;
typedef struct ForceFeedbackData_t ForceFeedbackData_t;
typedef struct ForceFeedbackEffect_t ForceFeedbackEffect_t;

extern Word BURGERCALL InputSetState(Word ActiveFlag);
extern Word BURGERCALL InputGetState(void);

extern volatile Word8 KeyArray[128];		/* Scan codes of keys pressed */
extern void BURGERCALL KeyboardInit(void);
extern void BURGERCALL KeyboardDestroy(void);
extern KeyboardGetchCallBackPtr KeyboardGetchCallBack;	/* Key stealers */
extern Word BURGERCALL KeyboardGetch(void);
extern Word BURGERCALL KeyboardKbhit(void);
extern void BURGERCALL KeyboardAddRoutine(KeyboardCallBack Proc,void *Data);
extern void BURGERCALL KeyboardRemoveRoutine(KeyboardCallBack Proc,void *Data);
extern void BURGERCALL KeyboardFlush(void);
extern Word BURGERCALL KeyboardGet(void);
extern Word BURGERCALL KeyboardGet2(void);
extern void BURGERCALL KeyboardCallPollingProcs(void);
extern Word BURGERCALL KeyboardGetKeyLC(void);
extern Word BURGERCALL KeyboardGetKeyUC(void);
extern Word BURGERCALL KeyboardAnyPressed(void);
extern Word BURGERCALL KeyboardIsPressed(Word ScanCode);
extern Word BURGERCALL KeyboardHasBeenPressed(Word ScanCode);
extern void BURGERCALL KeyboardClearKey(Word ScanCode);
extern Word BURGERCALL KeyboardHasBeenPressedClear(Word ScanCode);
extern Word BURGERCALL KeyboardStringToScanCode(const char *StringPtr);
extern void BURGERCALL KeyboardScanCodeToString(char *StringPtr,Word StringSize,Word ScanCode);
extern Word BURGERCALL KeyboardWait(void);

extern Word MousePresent;
extern Word MouseClicked;
extern Word BURGERCALL MouseInit(void);
extern void BURGERCALL MouseDestroy(void);
extern Word BURGERCALL MouseReadButtons(void);
extern void BURGERCALL MouseReadAbs(Word *x,Word *y);
extern void BURGERCALL MouseReadDelta(int *x,int *y);
extern int BURGERCALL MouseReadWheel(void);
extern void BURGERCALL MouseSetRange(Word x,Word y);
extern void BURGERCALL MouseSetPosition(Word x,Word y);

#define AXISCOUNT 6
#define MAXJOYNUM 4
enum {AXISMIN,AXISMAX,AXISCENTER,AXISLESS,AXISMORE,AXISENTRIES};
extern Word JoystickPercent[MAXJOYNUM][AXISCOUNT];		/* Cache for percentages */
extern Word JoystickPresent;
extern Word32 JoystickLastButtons[MAXJOYNUM];
extern Word32 JoystickLastButtonsDown[MAXJOYNUM];
extern Word JoystickBoundaries[MAXJOYNUM][AXISENTRIES*AXISCOUNT];
extern Word BURGERCALL JoystickInit(void);
extern void BURGERCALL JoystickDestroy(void);
extern Word32 BURGERCALL JoystickReadButtons(Word Which);
extern void BURGERCALL JoystickReadNow(Word Which);
extern Word BURGERCALL JoystickReadAbs(Word Axis,Word Which);
extern int BURGERCALL JoystickReadDelta(Word Axis,Word Which);
extern Word BURGERCALL JoystickGetAxisCount(Word Which);
extern void BURGERCALL JoystickSetCenter(Word Axis,Word Which);
extern void BURGERCALL JoystickSetMin(Word Axis,Word Which);
extern void BURGERCALL JoystickSetMax(Word Axis,Word Which);
extern void BURGERCALL JoystickSetDigital(Word Axis,Word Percent,Word Which);
extern void BURGERCALL JoystickBoundariesChanged(void);
extern Word BURGERCALL JoyAutoRepeater(JoyAutoRepeat_t *Input,Word32 JoyBits);

extern ForceFeedback_t * BURGERCALL ForceFeedbackNew(void);
extern void BURGERCALL ForceFeedbackDelete(ForceFeedback_t *RefPtr);
extern void BURGERCALL ForceFeedbackReacquire(ForceFeedback_t *RefPtr);
extern ForceFeedbackData_t * BURGERCALL ForceFeedbackDataNew(ForceFeedback_t *RefPtr,const char *FilenamePtr);
extern ForceFeedbackData_t * BURGERCALL ForceFeedbackDataNewRez(ForceFeedback_t *RefPtr,struct RezHeader_t *RezRef,Word RezNum);
extern void BURGERCALL ForceFeedbackDataDelete(ForceFeedbackData_t *FilePtr);
extern ForceFeedbackEffect_t * BURGERCALL ForceFeedbackEffectNew(ForceFeedbackData_t *FilePtr,const char *EffectNamePtr);
extern void BURGERCALL ForceFeedbackEffectDelete(ForceFeedbackEffect_t *effect);
extern Word BURGERCALL ForceFeedbackEffectPlay(ForceFeedbackEffect_t *Input);
extern void BURGERCALL ForceFeedbackEffectStop(ForceFeedbackEffect_t *Input);
extern Word BURGERCALL ForceFeedbackEffectIsPlaying(ForceFeedbackEffect_t *Input);
extern void BURGERCALL ForceFeedbackEffectSetGain(ForceFeedbackEffect_t *Input,long NewGain);
extern void BURGERCALL ForceFeedbackEffectSetDuration(ForceFeedbackEffect_t *Input,Word32 NewDuration);

#if defined(__MAC__)
extern Word KeyModifiers;			/* If a key is read, pass back the keyboard modifiers */
extern Word ScanCode;				/* Scan code of key last read */
extern Word FixMacKey(struct EventRecord *Event);
extern Bool MacSystemTaskFlag;
extern Word (*MacEventIntercept)(struct EventRecord *MyEventPtr);
extern Word DoMacEvent(Word Mask,struct EventRecord *Event);
extern Word BURGERCALL MacInputLockInputSprocket(void);
extern void BURGERCALL MacInputUnlockInputSprocket(void);
#endif

#if defined(__BEOS__)
extern int BURGERCALL BeOSSpawnMain(int (*MainCode)(int,char **),int argc, char **argv);
#endif

/* In Font */

typedef void (BURGERCALL *FontDrawProc)(struct FontRef_t *,const char *,Word);
typedef Word (BURGERCALL *FontWidthProc)(struct FontRef_t *,const char *,Word);

typedef struct FontRef_t {		/* This is the generic class for fonts */
	int FontX;			/* X coord to draw the font */
	int FontY;			/* Y coord to draw the font */
	Word FontHeight;	/* Height of the font in pixels */
	Word FontFirst;		/* First allowable font to draw */
	Word FontLast;		/* Last char I can draw */
	FontDrawProc Draw;	/* Draw text */
	FontWidthProc GetWidth;	/* Get the width of the text */
} FontRef_t;

typedef struct BurgerFontState_t {
	Word8 FontOrMask[32];	/* Or mask (Could be shorts) */
	struct RezHeader_t *FontRezFile;	/* Resource file */
	int FontX;			/* X coord */
	int FontY;			/* Y coord */
	Word FontLoaded;	/* Resource */
	Word FontColorZero;	/* And mask */
} BurgerFontState_t;

typedef struct BurgerFont_t {	/* State of a font */
	FontRef_t Root;		/* Root of the font */
	Word8 FontOrMask[32];	/* Color of font (Could be shorts) */
	struct RezHeader_t *FontRezFile;	/* Resource file */
	void **FontHandle;	/* Handle to the active font */
	Word FontOffset;	/* Offset to the pixel array */
	Word FontLoaded;	/* Resource ID of the last font loaded */
	Word FontColorZero;	/* Mask for font */
} BurgerFont_t;

typedef struct FontWidthEntry_t {	/* Describe each line */
	Word32 Offset;				/* Offset to the text string */
	Word Length;					/* Length of the string entry */
} FontWidthEntry_t;

typedef struct FontWidthLists_t {	/* Hash for formatted text */
	FontRef_t *FontPtr;				/* Font to use */
	Word Count;						/* Number of valid entries */
	FontWidthEntry_t Entries[1];	/* Array of entries */
} FontWidthLists_t;

extern void BURGERCALL BurgerFontInit(BurgerFont_t *Input,struct RezHeader_t *RezFile,Word RezNum,const Word8 *PalPtr);
extern BurgerFont_t * BURGERCALL BurgerFontNew(struct RezHeader_t *RezFile,Word RezNum,const Word8 *PalPtr);
#define BurgerFontDestroy(x) BurgerFontRelease(x)
extern void BURGERCALL BurgerFontDelete(BurgerFont_t *Input);
extern void BURGERCALL BurgerFontSaveState(BurgerFont_t *RefPtr,BurgerFontState_t *StatePtr);
extern void BURGERCALL BurgerFontRestoreState(BurgerFont_t *RefPtr,const BurgerFontState_t *StatePtr);
extern void BURGERCALL BurgerFontDrawChar(BurgerFont_t *RefPtr,Word Letter);
extern void BURGERCALL BurgerFontDrawText(BurgerFont_t *RefPtr,const char *TextPtr,Word TextLen);
extern Word BURGERCALL BurgerFontWidthText(BurgerFont_t *RefPtr,const char *TextPtr,Word TextLen);
extern void BURGERCALL BurgerFontSetColor(BurgerFont_t *RefPtr,Word ColorNum,Word Color);
extern void BURGERCALL BurgerFontUseZero(BurgerFont_t *RefPtr);
extern void BURGERCALL BurgerFontUseMask(BurgerFont_t *RefPtr);
extern void BURGERCALL BurgerFontInstallToPalette(BurgerFont_t *RefPtr,struct RezHeader_t *RezFile,Word FontNum,const Word8 *PalPtr);
extern void BURGERCALL BurgerFontRelease(BurgerFont_t *RefPtr);
extern void BURGERCALL BurgerFontSetColorRGBListToPalette(BurgerFont_t *RefPtr,const void *RGBList,const Word8 *PalPtr);
extern void BURGERCALL BurgerFontSetToPalette(BurgerFont_t *RefPtr,const Word8 *PalPtr);

extern void BURGERCALL FontSetXY(FontRef_t *RefPtr,int x,int y);
extern Word BURGERCALL FontWidthChar(FontRef_t *RefPtr,Word Letter);
extern Word BURGERCALL FontWidthLong(FontRef_t *Input,long Val);
extern Word BURGERCALL FontWidthLongWord(FontRef_t *RefPtr,Word32 Val);
extern Word BURGERCALL FontWidthString(FontRef_t *RefPtr,const char *TextPtr);
extern Word BURGERCALL FontWidthListWidest(FontRef_t *FontPtr,struct LinkedList_t *ListPtr);
extern void BURGERCALL FontDrawChar(FontRef_t *RefPtr,Word Letter);
extern void BURGERCALL FontDrawLong(FontRef_t *Input,long Val);
extern void BURGERCALL FontDrawLongWord(FontRef_t *RefPtr,Word32 Val);
extern void BURGERCALL FontDrawString(FontRef_t *RefPtr,const char *TextPtr);
extern void BURGERCALL FontDrawStringCenterX(FontRef_t *RefPtr,int x,int y,const char *TextPtr);
extern void BURGERCALL FontDrawStringAtXY(FontRef_t *RefPtr,int x,int y,const char *TextPtr);
extern Word BURGERCALL FontCharsForPixelWidth(FontRef_t *RefPtr,const char *TextPtr,Word Width);

extern FontWidthLists_t * BURGERCALL FontWidthListNew(FontRef_t *FontRef,const char *TextPtr,Word PixelWidth);
#define FontWidthListDelete(x) DeallocAPointer(x)
extern void BURGERCALL FontWidthListDraw(FontWidthLists_t *Input,const struct LBRect *BoundsPtr,Word YTop,const char *TextPtr);

/* In Graphics */

struct LBRect;		/* Unix compilers want this, sigh */
#if defined(__MAC__)
#define BLACK 255
#define WHITE 0
#else
#define BLACK 0
#define WHITE 255
#endif

#define SETDISPLAYWINDOW 0x0				/* Best for debugging */
#define SETDISPLAYFULLSCREEN 0x1			/* True if full screen */
#define SETDISPLAYOFFSCREEN 0x2				/* True if I force system memory video page */
#define SETDISPLAYDOUBLEOK 0x4				/* True if I can allow pixel doubling for 320x200 */
#define SETDISPLAYOPENGL 0x8				/* True if 3D hardware is requested (Check Video3DPreference for API override) */
#define SETDISPLAYD3D 0x10					/* True if 3D hardware is requested (Check Video3DPreference for API override) */
#define SETDISPLAYNOWINDOWTWIDDLE 0x20		/* Don't change any of my window settings for me */

#define VIDEOMODEOPENGL 0x01		/* OpenGL is supported in this mode */
#define VIDEOMODEHARDWARE 0x02		/* DirectX or other native support */
#define VIDEOMODEFAKE 0x04			/* This is a pixel double mode */

#define VIDEOAPISOFTWARE 0
#define VIDEOAPIOPENGL 1
#define VIDEOAPIDIRECT3D 2
#define VIDEOAPIDIRECTDRAW 3

#define VIDEOMODEGETNOFAKE 0x0100	/* Don't allow pixel double modes */

/* Texture formats supported */
#define VIDEOTEXTURETYPE555 0x0001
#define VIDEOTEXTURETYPE565 0x0002
#define VIDEOTEXTURETYPE4444 0x0004
#define VIDEOTEXTURETYPE1555 0x0008
#define VIDEOTEXTURETYPE8PAL 0x0010
#define VIDEOTEXTURETYPE888 0x0020
#define VIDEOTEXTURETYPE8888 0x0040

#define VIDEOTEXTURESQUARE 0x0001		/* 3D Textures MUST be square */
#define VIDEOTEXTUREPOW2 0x0002			/* 3D Textures MUST be a power of 2 in size */
#define VIDEOTEXTURESYSTEMMEMORY 0x0004	/* Textures can exist in system memory (AGP) */

typedef struct LWShape_t {
	Word16 Width;		/* Width of the shape */
	Word16 Height;		/* Height of the shape */
	Word8 Data[1];		/* Raw shape data */
} LWShape_t;

typedef struct LWXShape_t {
	short XOffset;		/* Signed offset for x */
	short YOffset;		/* Signed offset for y */
	LWShape_t Shape;	/* Shape data */
} LWXShape_t;

typedef struct GfxShape_t {
	Word8 Palette[768];	/* Palette for the shape */
	LWXShape_t XShape;	/* XShape for the actual data */
} GfxShape_t;

typedef struct VideoMode_t {
	Word Width;		/* Width of video mode */
	Word Height;	/* Height of video mode */
	Word Depth;		/* Depth of video mode */
	Word Hertz;		/* Video scan rate (0 if not supported) */
	Word Flags;		/* Hardware/OpenGL */
} VideoMode_t;

typedef struct VideoModeArray_t {
	Word Count;				/* Number of modes in the array */
	Word DevNumber;			/* Device number */
	char DeviceName[64];	/* Name of the device */
	VideoMode_t Array[1];	/* Array of modes */
} VideoModeArray_t;

typedef struct VideoDeviceArray_t {
	Word Count;						/* Number of video cards present */
	VideoModeArray_t **Array[1];	/* Modes present (Handles) */
} VideoDeviceArray_t;

typedef struct VideoSaveState_t {
	Word Width;		/* Width of the current screen */
	Word Height;	/* Height of the current screen */
	Word Depth;		/* BIt depth of the current screen */
	Word Flags;		/* Special flags for the current mode */
} VideoSaveState_t;

typedef struct GammaTable_t {
	float Red[256];		/* Red gamma 0.0 = min, 1.0 = max */
	float Green[256];	/* Green gamma */
	float Blue[256];	/* Blue gamma */
} GammaTable_t;

typedef void (BURGERCALL *DrawARectProc)(int x,int y,Word Width,Word Height,Word Color);
typedef void (BURGERCALL *DrawALineProc)(int x1,int y1,int x2,int y2,Word Color);
typedef void (BURGERCALL *DrawAPixelProc)(int x,int y,Word Color);
typedef void (BURGERCALL *DrawARectRemapProc)(int x,int y,Word Width,Word Height,const void *RemapPtr);
typedef Word (BURGERCALL *VideoModePurgeProc)(VideoMode_t *);

extern Word Table8To16[256];		/* 8 bit to 16 bit conversion table */
extern Word *Table8To16Ptr;			/* Pointer to the draw table to use */
extern Word8 *VideoPointer;			/* Pointer to current video buffer */
extern Word VideoWidth;				/* Bytes per scan line */
extern Word ScreenClipLeft;			/* Clip left */
extern Word ScreenClipTop;			/* Clip top */
extern Word ScreenClipRight;		/* Clip right */
extern Word ScreenClipBottom;		/* Clip bottom */
extern Word ScreenWidth;			/* Clip width in pixels */
extern Word ScreenHeight;			/* Clip height in pixels */
extern Word ScreenFlags;			/* Current flags used for this mode */
extern Word VideoRedShift;			/* Bits to shift for Red */
extern Word VideoGreenShift;		/* Bits to shift for Green */
extern Word VideoBlueShift;			/* Bits to shift for Blue */
extern Word VideoRedMask;			/* Bitmask for Red */
extern Word VideoGreenMask;			/* Bitmask for Green */
extern Word VideoBlueMask;			/* Bitmask for Blue */
extern Word VideoRedBits;			/* Number of bits of Red */
extern Word VideoGreenBits;			/* Number of bits of Green */
extern Word VideoBlueBits;			/* Number of bits of Blue */
extern Word VideoColorDepth;		/* TRUE if palette doesn't exist */
extern Word VideoTrueScreenWidth;	/* Width in PIXELS of the video display */
extern Word VideoTrueScreenHeight;	/* Width in PIXELS of the video display */
extern Word VideoTrueScreenDepth;	/* Depth in BITS of the video display */
extern Word VideoPixelDoubleMode;	/* Set to the mode requested 0-3 */
extern Word VideoFullScreen; 		/* TRUE if full screen mode is active */
extern Word VideoPageLocked;		/* True if the video memory is locked */
extern Word8 *VideoOffscreen;		/* Pointer to offscreen buffer if used */
extern Word VideoHardWidth;			/* Width in BYTES of a video scan line */
extern Word VideoAPIInUse;			/* True if OpenGL is present and active */
extern Word VideoTextureTypesAllowed;	/* Texture formats I can support */
extern Word VideoTextureRules;			/* Special rules I have to follow */
extern Word VideoTextureMinWidth;		/* Minimum texture size */
extern Word VideoTextureMaxWidth;		/* Maximum texture size */
extern Word VideoTextureMinHeight;		/* Minimum texture height */
extern Word VideoTextureMaxHeight;		/* Maximum texture height */
extern Word VideoVertexMaxCount;		/* Maximum number of vertexs */
extern Word VideoVertexCount;			/* Number of vertex's processed */
extern Word VideoUseColorZero;			/* TRUE if Color #0 is used in textures */
extern DrawARectProc ScreenRect;	/* Draw a rect intercept vector */
extern DrawALineProc ScreenLine;	/* Draw a line intercept vector */
extern DrawAPixelProc ScreenPixel;	/* Draw a pixel intercept vector */
extern DrawARectRemapProc ScreenRectRemap;	/* Remap a rect of pixels */

extern Word BurgerMaxVideoPage;		/* Maximum number of video pages */
extern Word BurgerVideoPage;		/* Currently using this page */
extern Word BurgerVesaVersion;		/* 0 = No VESA, 1 = 1.2, 2 = 2.0 */
extern Bool Burger8BitPalette;	/* TRUE if 8 bit palettes are supported */
extern Word32 BurgerScreenSize;	/* Size in BYTES of the offscreen bitmap */
extern Word8 *BurgerBaseVideoPointer;	/* Pointer to base video memory range */
extern Word8 *BurgerVideoPointers[3];	/* Pointers to each video page */
extern Word8 *BurgerVideoCallbackBuffer;	/* Pointer to VESA callback code */
extern Bool BurgerLinearFrameBuffer;	/* TRUE if hardware is linear */

#define SetNewVideoWidth(x) VideoWidth=x
extern void BURGERCALL ReleaseVideo(void);
extern Word BURGERCALL SetDisplayToSize(Word Width,Word Height,Word Depth,Word Flags);
extern void BURGERCALL UpdateAndPageFlip(void);
extern void BURGERCALL UpdateAndNoPageFlip(void);
extern void BURGERCALL LockVideoPage(void);
extern void BURGERCALL LockFrontVideoPage(void);
extern void BURGERCALL VideoSetWindowString(const char *Title);
extern VideoModeArray_t **BURGERCALL VideoModeArrayNew(Word Flags);
#define VideoModeArrayDelete(x) DeallocAHandle((void **)x)
extern VideoDeviceArray_t **BURGERCALL VideoDeviceArrayNew(Word Flags);
extern void BURGERCALL VideoDeviceArrayDelete(VideoDeviceArray_t **Input);
extern VideoModeArray_t ** BURGERCALL VideoModeArrayPurge(VideoModeArray_t **Input,VideoModePurgeProc Proc);
extern VideoDeviceArray_t ** BURGERCALL VideoDeviceArrayPurge(VideoDeviceArray_t **Input,VideoModePurgeProc Proc);
extern void BURGERCALL VideoGetCurrentMode(VideoSaveState_t *Input);
extern Word BURGERCALL VideoSetMode(const VideoSaveState_t *Input);
extern void BURGERCALL UnlockVideoPage(void);
extern void BURGERCALL Video555To565(void);

extern void BURGERCALL VideoOSGammaInit(void);
extern void BURGERCALL VideoOSGammaDestroy(void);
extern void BURGERCALL VideoOSGammaAdjust(Fixed32 Intensity);
extern void BURGERCALL VideoOSGammaSet(const GammaTable_t *TablePtr);
extern void BURGERCALL VideoOSGammaGet(GammaTable_t *TablePtr);

#define GetShapeWidth(ShapePtr) (((LWShape_t *)ShapePtr)->Width)
#define GetShapeHeight(ShapePtr) (((LWShape_t *)ShapePtr)->Height)
#define GetXShapeXOffset(ShapePtr) (((LWXShape_t *)ShapePtr)->XOffset)
#define GetXShapeYOffset(ShapePtr) (((LWXShape_t *)ShapePtr)->YOffset)
#define GetXShapeWidth(ShapePtr) (((LWXShape_t *)ShapePtr)->Shape.Width)
#define GetXShapeHeight(ShapePtr) (((LWXShape_t *)ShapePtr)->Shape.Height)
#define GetShapeIndexPtr(ShapeArrayPtr,Index) (&((Word8 *)ShapeArrayPtr)[((Word32 *)ShapeArrayPtr)[Index]])

extern void BURGERCALL SetTheClipBounds(Word Left,Word Top,Word Right,Word Bottom);
extern void BURGERCALL SetTheClipRect(const struct LBRect *RectPtr);
extern void BURGERCALL GetTheClipRect(struct LBRect *RectPtr);
extern Word BURGERCALL GetAPixel(Word x,Word y);
extern Word BURGERCALL GetAPixel16(Word x,Word y);
extern void BURGERCALL SetAPixel(Word x,Word y,Word Color);
extern void BURGERCALL SetAPixel16(Word x,Word y,Word Color);
extern void BURGERCALL SetAPixelTo16(Word x,Word y,Word Color);
extern void BURGERCALL DrawARect(int x,int y,Word Width,Word Height,Word Color);
extern void BURGERCALL DrawARect16(int x,int y,Word Width,Word Height,Word Color);
extern void BURGERCALL DrawARectTo16(int x,int y,Word Width,Word Height,Word Color);
extern void BURGERCALL DrawALine(int x1,int y1,int x2,int y2,Word Color);
extern void BURGERCALL DrawALine16(int x1,int y1,int x2,int y2,Word Color);
extern void BURGERCALL DrawALineTo16(int x1,int y1,int x2,int y2,Word Color);
extern void BURGERCALL DrawARectRemap(int x,int y,Word Width,Word Height,const Word8 *RemapPtr);
extern void BURGERCALL DrawARectRemap16(int x,int y,Word Width,Word Height,const Word16 *RemapPtr);

extern void BURGERCALL ScreenClear(Word Color);
extern void BURGERCALL ScreenBox(int x,int y,Word Width,Word Height,Word Color);
extern void BURGERCALL ScreenBox2(int x,int y,Word Width,Word Height,Word Color1,Word Color2);
extern void BURGERCALL ScreenThickBox(int x,int y,Word Width,Word Height,Word Color);
extern void BURGERCALL ScreenThickBox2(int x,int y,Word Width,Word Height,Word Color1,Word Color2);
extern void BURGERCALL ScreenBoxRemap(int x,int y,Word Width,Word Height,const void *RemapPtr);
extern void BURGERCALL ScreenBoxDropShadow(int x,int y,Word Width,Word Height,Word Color1,Word Color2);
extern void BURGERCALL ScreenRectDropShadow(int x,int y,Word Width,Word Height,Word Color1,Word Color2,Word Color3);

extern void BURGERCALL DrawShapeLowLevel(Word x,Word y,Word Width,Word Height,Word Skip,void *ShapePtr);
extern void BURGERCALL DrawShapeLowLevelClipped(int x,int y,Word Width,Word Height,Word Skip,void *ShapePtr);
#define DrawShape(x,y,p) DrawShapeLowLevel(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,((LWShape_t *)p)->Data)
#define DrawShapeClipped(x,y,p) DrawShapeLowLevelClipped(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawRawShape(x,y,w,h,p) DrawShapeLowLevel(x,y,w,h,0,p)
#define DrawRawShapeClipped(x,y,w,h,p) DrawShapeLowLevelClipped(x,y,w,h,0,p)
#define DrawXShape(x,y,p) DrawShapeLowLevel(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
#define DrawXShapeClipped(x,y,p) DrawShapeLowLevelClipped(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
extern void BURGERCALL DrawRezShape(Word x,Word y,struct RezHeader_t *Input,Word RezNum);
extern void BURGERCALL DrawRezCenterShape(struct RezHeader_t *Input,Word RezNum);

extern void BURGERCALL DrawMShapeLowLevel(Word x,Word y,Word Width,Word Height,Word Skip,void *ShapePtr);
extern void BURGERCALL DrawMShapeLowLevelClipped(int x,int y,Word Width,Word Height,Word Skip,void *ShapePtr);
#define DrawMShape(x,y,p) DrawMShapeLowLevel(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,((LWShape_t *)p)->Data)
#define DrawMShapeClipped(x,y,p) DrawMShapeLowLevelClipped(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawRawMShape(x,y,w,h,p) DrawMShapeLowLevel(x,y,w,h,0,p)
#define DrawRawMShapeClipped(x,y,w,h,p) DrawMShapeLowLevelClipped(x,y,w,h,0,p)
#define DrawXMShape(x,y,p) DrawMShapeLowLevel(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
#define DrawXMShapeClipped(x,y,p) DrawMShapeLowLevelClipped(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
extern void BURGERCALL DrawRezMShape(Word x,Word y,struct RezHeader_t *Input,Word RezNum);
extern void BURGERCALL DrawRezCenterMShape(struct RezHeader_t *Input,Word RezNum);

#define DrawShapeLowLevel16(x,y,w,h,s,p) DrawShapeLowLevel(x*2,y,w*2,h,s,p)
extern void BURGERCALL DrawShapeLowLevelClipped16(int x,int y,Word Width,Word Height,Word Skip,void *ShapePtr);
#define DrawShape16(x,y,p) DrawShapeLowLevel16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawShapeClipped16(x,y,p) DrawShapeLowLevelClipped16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawRawShape16(x,y,w,h,p) DrawShapeLowLevel16(x,y,w,h,0,p)
#define DrawRawShapeClipped16(x,y,w,h,p) DrawShapeLowLevelClipped16(x,y,w,h,0,p)
#define DrawXShape16(x,y,p) DrawShapeLowLevel16(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
#define DrawXShapeClipped16(x,y,p) DrawShapeLowLevelClipped16(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
extern void BURGERCALL DrawRezShape16(Word x,Word y,struct RezHeader_t *Input,Word RezNum);
extern void BURGERCALL DrawRezCenterShape16(struct RezHeader_t *Input,Word RezNum);

extern void BURGERCALL DrawMShapeLowLevel16(Word x,Word y,Word Width,Word Height,Word Skip,void *ShapePtr);
extern void BURGERCALL DrawMShapeLowLevelClipped16(int x,int y,Word Width,Word Height,Word Skip,void *ShapePtr);
#define DrawMShape16(x,y,p) DrawMShapeLowLevel16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawMShapeClipped16(x,y,p) DrawMShapeLowLevelClipped16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawRawMShape16(x,y,w,h,p) DrawMShapeLowLevel16(x,y,w,h,0,p)
#define DrawRawMShapeClipped16(x,y,w,h,p) DrawMShapeLowLevelClipped16(x,y,w,h,0,p)
#define DrawXMShape16(x,y,p) DrawMShapeLowLevel(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
#define DrawXMShapeClipped16(x,y,p) DrawMShapeLowLevelClipped16(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
extern void BURGERCALL DrawRezMShape16(Word x,Word y,struct RezHeader_t *Input,Word RezNum);
extern void BURGERCALL DrawRezCenterMShape16(struct RezHeader_t *Input,Word RezNum);

extern void BURGERCALL DrawShapeLowLevelTo16(Word x,Word y,Word Width,Word Height,Word Skip,void *ShapePtr);
extern void BURGERCALL DrawShapeLowLevelClippedTo16(int x,int y,Word Width,Word Height,Word Skip,void *ShapePtr);
#define DrawShapeTo16(x,y,p) DrawShapeLowLevelTo16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawShapeClippedTo16(x,y,p) DrawShapeLowLevelClippedTo16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawRawShapeTo16(x,y,w,h,p) DrawShapeLowLevelTo16(x,y,w,h,0,p)
#define DrawRawShapeClippedTo16(x,y,w,h,p) DrawShapeLowLevelClippedTo16(x,y,w,h,0,p)
#define DrawXShapeTo16(x,y,p) DrawShapeLowLevelTo16(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
#define DrawXShapeClippedTo16(x,y,p) DrawShapeLowLevelClippedTo16(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)

extern void BURGERCALL DrawMShapeLowLevelTo16(Word x,Word y,Word Width,Word Height,Word Skip,void *ShapePtr);
extern void BURGERCALL DrawMShapeLowLevelClippedTo16(int x,int y,Word Width,Word Height,Word Skip,void *ShapePtr);
#define DrawMShapeTo16(x,y,p) DrawMShapeLowLevelTo16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawMShapeClippedTo16(x,y,p) DrawMShapeLowLevelClippedTo16(x,y,((LWShape_t *)p)->Width,((LWShape_t *)p)->Height,0,&((LWShape_t *)p)->Data)
#define DrawRawMShapeTo16(x,y,w,h,p) DrawMShapeLowLevelTo16(x,y,w,h,0,p)
#define DrawRawMShapeClippedTo16(x,y,w,h,p) DrawMShapeLowLevelClippedTo16(x,y,w,h,0,p)
#define DrawXMShapeTo16(x,y,p) DrawMShapeLowLevelTo16(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)
#define DrawXMShapeClippedTo16(x,y,p) DrawMShapeLowLevelClippedTo16(((LWXShape_t*)p)->XOffset+x,((LWXShape_t*)p)->YOffset+y, \
	((LWXShape_t *)p)->Shape.Width,((LWXShape_t *)p)->Shape.Height,0,&((LWXShape_t*)p)->Shape.Data)

extern void BURGERCALL EraseShape(Word x,Word y,void *ShapePtr);
extern void BURGERCALL EraseMBShape(Word x,Word y,void *ShapePtr,void *BackPtr);
extern Word BURGERCALL TestMShape(Word x,Word y,void *ShapePtr);
extern Word BURGERCALL TestMBShape(Word x,Word y,void *ShapePtr,void *BackPtr);
extern void BURGERCALL VideoPixelDouble16(const Word8 *SourcePtr,Word8 *DestPtr,Word SourceRowBytes,Word DestRowBytes,Word Width,Word Height);
extern void BURGERCALL VideoPixelDoubleChecker16(const Word8 *SourcePtr,Word8 *DestPtr,Word SourceRowBytes,Word DestRowBytes,Word Width,Word Height);
extern void BURGERCALL VideoPixelDouble(const Word8 *SourcePtr,Word8 *DestPtr,Word SourceRowBytes,Word DestRowBytes,Word Width,Word Height);
extern void BURGERCALL VideoPixelDoubleChecker(const Word8 *SourcePtr,Word8 *DestPtr,Word SourceRowBytes,Word DestRowBytes,Word Width,Word Height);

extern Word BURGERCALL OpenGLSetDisplayToSize(Word Width,Word Height,Word Depth,Word Flags);
extern void BURGERCALL OpenGLMakeCurrent(void);
extern void BURGERCALL OpenGLRelease(void);
extern void BURGERCALL OpenGLSwapBuffers(void);
extern void BURGERCALL OpenGLInit(void);
extern void BURGERCALL OpenGLSolidRect(int x,int y,Word Width,Word Height,Word32 Color);
extern void BURGERCALL OpenGLLine(int x,int y,int x2,int y2,Word32 Color);
extern void BURGERCALL OpenGLTextureDraw2D(Word TexNum,int x,int y,Word Width,Word Height);
extern void BURGERCALL OpenGLTextureDraw2DSubRect(Word TexNum,int x,int y,Word Width,Word Height,const float *UVPtr);
extern Word BURGERCALL OpenGLLoadTexture(Word *TextureNum,const struct Image_t *ImagePtr);
extern void BURGERCALL OpenGLSetCurrent(void);
extern void BURGERCALL OpenGLATISetTruform(Word Setting);
extern void BURGERCALL OpenGLATISetFSAA(Word Setting);

#if defined(__MAC__)
extern struct __AGLContextRec * BURGERCALL OpenGLSetContext(struct __AGLContextRec *Context);
extern struct __AGLContextRec * BURGERCALL OpenGLGetContext(void);
#endif

#if defined(__WIN32__)

extern Word BURGERCALL WinSetDisplayToSize(Word Width,Word Height,Word Depth);
extern void BURGERCALL DrawARectDirectX(int x,int y,Word Width,Word Height,Word Color);
#define DrawARectDirectX16 DrawARectDirectX
extern void BURGERCALL DrawARectDirectXTo16(int x,int y,Word Width,Word Height,Word Color);

/* These macros help store data into the output stream for D3D operations */
/* I assume little endian in the operations since this belongs to a PC exclusively */

#define STORE_OP_CONST(a,b,c,p) ((Word32 *)p)[0] = (((Word32)a) + (((Word32)b)<<8) + (((Word32)c)<<16)); p =p+4;
#define STORE_OP(a,b,c,p) ((D3DINSTRUCTION*)p)->bOpcode = (Word8)a; ((D3DINSTRUCTION*)p)->bSize = (Word8)b; ((D3DINSTRUCTION*)p)->wCount = (Word16)c; p = p+4;

#define STORE_OP_RENDERSTATE_CONST(a,p) ((Word32 *)p)[0] = ((Word32)D3DOP_STATERENDER + (((Word32)sizeof(D3DSTATE))<<8) + (((Word32)a)<<16)); p = p+4;
#define STORE_OP_RENDERSTATE(a,p) ((Word16 *)p)[0] = (((Word16)D3DOP_STATERENDER) + ((Word16)sizeof(D3DSTATE)<<8)); ((Word16 *)p)[1] = (Word16)a; p = p+4;
#define STORE_OP_PROCESSVERTICES_CONST(a,p) ((Word32 *)p)[0] = ((Word32)D3DOP_PROCESSVERTICES + (((Word32)sizeof(D3DPROCESSVERTICES))<<8) + (((Word32)a)<<16)); p = p+4;
#define STORE_OP_PROCESSVERTICES(a,p) ((Word16 *)p)[0] = (((Word16)D3DOP_PROCESSVERTICES) + ((Word16)sizeof(D3DPROCESSVERTICES)<<8)); ((Word16 *)p)[1] = (Word16)a; p = p+4;
#define STORE_OP_TRIANGLE_CONST(a,p) ((Word32 *)p)[0] = ((Word32)D3DOP_TRIANGLE + (((Word32)sizeof(D3DTRIANGLE))<<8) + (((Word32)a)<<16)); p = p+4;
#define STORE_OP_TRIANGLE(a,p) ((Word16 *)p)[0] = (((Word16)D3DOP_TRIANGLE) + ((Word16)sizeof(D3DTRIANGLE)<<8)); ((Word16 *)p)[1] = (Word16)a; p = p+4;
#define STORE_OP_EXIT(p) ((Word32 *)p)[0] = (Word32)D3DOP_EXIT; p = (void *)((Word8 *)p+4);

#define STORE_DATA_STATE(a,b,p) ((D3DSTATE *)p)->drstRenderStateType = (D3DRENDERSTATETYPE)a; ((D3DSTATE *)p)->dwArg[0] = b; p = p+8;
#define STORE_DATA_PROCESSVERTICES(flgs,strt,cnt,p) ((D3DPROCESSVERTICES *)p)->dwFlags = flgs; ((D3DPROCESSVERTICES *)p)->wStart = (Word16)strt; ((D3DPROCESSVERTICES *)p)->wDest = (Word16)strt; ((D3DPROCESSVERTICES *)p)->dwCount = cnt; ((D3DPROCESSVERTICES *)p)->dwReserved = 0; p = p+16;
#define STORE_DATA_TRIANGLE(a,b,c,p) ((D3DTRIANGLE *)p)->v1 = (Word16)(a); ((D3DTRIANGLE *)p)->v2 = (Word16)(b); ((D3DTRIANGLE *)p)->v3 = (Word16)(c); ((D3DTRIANGLE *)p)->wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE; p = p + 8;

#define MAXD3DINSTRUCTIONS 1024		/* Maximum number of D3D instructions to queue */

extern struct IDirect3D *Direct3DPtr;				/* Reference to the direct 3D object */
extern struct IDirect3DDevice *Direct3DDevicePtr;	/* Reference to the direct 3D rendering device */
extern struct IDirect3DViewport *Direct3DViewPortPtr;	/* Reference to the direct 3d Viewport */
extern struct IDirect3DExecuteBuffer *Direct3DExecBufPtr;	/* Reference to the direct 3d execute buffer */
extern Word8 *Direct3DExecDataBuf;			/* Pointer to the execute data buffer */
extern Word8 *Direct3DExecInstBuf;			/* Pointer to the execute instruction buffer */
extern Word8 *Direct3DExecDataPtr;			/* Current free data pointer */
extern Word8 *Direct3DExecInstPtr;			/* Current free instruction pointer */
extern Word8 *Direct3DExecInstStartPtr;		/* Pointer to the beginning of the last instruction chunk */
extern Word32 Direct3DExecBuffSize;		/* Size of the execute buffer */

extern void BURGERCALL D3DGetTextureInfo(void);
extern void BURGERCALL D3DSetStandardViewport(void);
extern void BURGERCALL D3DCreateZBuffer(void);
extern void BURGERCALL D3DInitExecuteBuffer(void);
extern void BURGERCALL D3DInit(const struct _GUID *Input);
extern void BURGERCALL Direct3DDestroy(void);
extern void BURGERCALL Direct3DLockExecuteBuffer(void);
extern void BURGERCALL Direct3DUnlockExecuteBuffer(void);
extern void BURGERCALL Direct3DCheckExecBuffer(Word InstCount,Word VertexCount);
extern void BURGERCALL Direct3DBeginScene(void);
extern void BURGERCALL Direct3DEndScene(void);
extern void BURGERCALL Direct3DSolidRect(int x,int y,Word Width,Word Height,Word32 Color);
extern void BURGERCALL Direct3DLine(int x,int y,int x2,int y2,Word32 Color);

#endif

/* Screen Shape Manager */

enum {
	SCREENSHAPEMODESOFTWARE,
	SCREENSHAPEMODEOPENGL,
	SCREENSHAPEMODEDIRECT3D,
	SCREENSHAPEMODEDIRECTDRAW
};

#define SCREENSHAPEFLAGMODEMASK 0x0F
#define SCREENSHAPEFLAGMASK 0x010
#define SCREENSHAPEFLAGCOMPRESSED 0x020
#define SCREENSHAPEFLAGPURGEABLE 0x040
#define SCREENSHAPEFLAGMASKDISABLE 0x8000

typedef enum {
	TRANSLUCENCYMODE_OFF,
	TRANSLUCENCYMODE_NORMAL,
	TRANSLUCENCYMODE_INVCOLOR,
	TRANSLUCENCYMODE_COLOR,
	TRANSLUCENCYMODE_GLOWING,
	TRANSLUCENCYMODE_DARKENINGCOLOR,
	TRANSLUCENCYMODE_JUSTSETZ,
	TRANSLUCENCYMODE_BAD=0x70000000
} TranslucencyMode_e;

typedef enum {
	FILTERINGMODE_OFF,
	FILTERINGMODE_BILINEAR,
	FILTERINGMODE_BAD=0x70000000
} FilteringMode_e;

typedef enum {
	SHADINGMODE_FLAT,
	SHADINGMODE_GOURAUD,
	SHADINGMODE_PHONG,
	SHADINGMODE_BAD=0x70000000
} ShadingMode_e;

typedef enum {
	DEPTHTESTMODE_NEVER,
	DEPTHTESTMODE_LESS,
	DEPTHTESTMODE_EQUAL,
	DEPTHTESTMODE_LESSEQUAL,
	DEPTHTESTMODE_GREATER,
	DEPTHTESTMODE_NOTEQUAL,
	DEPTHTESTMODE_GREATEREQUAL,
	DEPTHTESTMODE_ALWAYS,
	DEPTHTESTMODE_BAD=0x70000000
} DepthTestMode_e;

struct ScreenShape_t;
typedef Word (BURGERCALL *ScreenShapeActionProc)(struct ScreenShape_t *);
typedef Word (BURGERCALL *ScreenShapeDrawProc)(struct ScreenShape_t* screen_shape, const struct LBRect* dest_rect);

typedef struct ScreenShape_t {	/* Root data class for shapes */
	void *Data1;			/* Reference to data (Usually a handle) */
	void *Data2;			/* Reference to data (Usually a texture ref) */
	void *Data3;			/* Extra reference */
	ScreenShapeActionProc ActionProc;	/* How to load this data? */
	ScreenShapeDrawProc DrawProc;	/* This can be NULL, in which case, blib automatically handles it based on screen mode */
	Word Flags;				/* Is color zero present? */
	Word Width,Height;		/* Size of the shape in pixels */
	int XOffset,YOffset;	/* X/Y adjust for drawing shape */
} ScreenShape_t;

typedef struct ScreenShapeRez_t {	/* Class for loading a shape from a rez file */
	ScreenShape_t MyShape;			/* Root class */
	struct RezHeader_t *RezFile;	/* Master resource reference */
	Word RezNum;					/* Resource number for the shape group to get info from */
	Word RezPal;					/* Palette for resource */
} ScreenShapeRez_t;

typedef struct ScreenShapeGfx_t {	/* Class for loading a Gfx shape from a rez file */
	ScreenShape_t MyShape;			/* Root class */
	struct RezHeader_t *RezFile;	/* Master resource reference */
	Word RezNum;					/* Resource number for the shape */
} ScreenShapeGfx_t;

typedef struct ScreenShapeGfxFile_t {	/* Class for loading a Gfx shape from a file */
	ScreenShape_t MyShape;			/* Root class */
	char **FileName;				/* Handle to filename */
} ScreenShapeGfxFile_t;

typedef struct ScreenShapePtr_t {	/* Class for loading a shape from a pointer */
	ScreenShape_t MyShape;			/* Root class */
	struct Image_t *ImagePtr;		/* Pointer to the shape */
} ScreenShapePtr_t;

typedef struct ScreenShapeRezGroup_t {	/* Class for loading from a group */
	ScreenShape_t MyShape;			/* Root class */
	struct RezHeader_t *RezFile;	/* Master resource reference */
	Word RezNum;					/* Resource number for the shape group to get info from */
	Word RezPal;					/* Palette for resource */
	Word Which;						/* Which shape in the group */
} ScreenShapeRezGroup_t;

typedef struct ScreenShapeGifFile_t {	/* Class for loading from a gif file */
	ScreenShape_t MyShape;			/* Root class */
	char **FileName;				/* Handle to filename */
} ScreenShapeGifFile_t;

typedef struct ScreenShapeGif_t {	/* Class for loading from a gif file */
	ScreenShape_t MyShape;			/* Root class */
	struct RezHeader_t *RezFile;	/* Master resource reference */
	Word RezNum;					/* Resource number for the shape */
} ScreenShapeGif_t;

typedef struct ScreenShapeBmpFile_t {	/* Class for loading from a bmp file */
	ScreenShape_t MyShape;			/* Root class */
	char **FileName;				/* Handle to filename */
} ScreenShapeBmpFile_t;

typedef struct ScreenShapeBmp_t {	/* Class for loading from a group */
	ScreenShape_t MyShape;			/* Root class */
	struct RezHeader_t *RezFile;	/* Master resource reference */
	Word RezNum;					/* Resource number for the shape */
} ScreenShapeBmp_t;

typedef void (BURGERCALL *ScreenShapeInitProcPtr)(void);
typedef void (BURGERCALL *ScreenShapeSolidRectProcPtr)(int,int,Word,Word,Word32);

extern Word ScreenAPI;			/* Which API am I using (Direct3D, OpenGL?) */
extern TranslucencyMode_e ScreenTranslucencyMode;		/* Current 3D translucency mode */
extern FilteringMode_e ScreenFilteringMode;				/* Current texture filtering mode */
extern ShadingMode_e ScreenShadingMode;					/* Current shading mode */
extern DepthTestMode_e ScreenDepthTestMode;				/* Type of ZBuffer test */
extern Word ScreenDepthWriteMode;						/* Write to the ZBuffer? */
extern Word ScreenPerspectiveMode;						/* Perspective correct mode active? */
extern Word ScreenWireFrameMode;						/* Are polygons wireframed? */
extern Word ScreenBlendMode;							/* Last alpha mode */
extern Word ScreenUsing2DCoords;						/* Whether 2D drawing is currently on */
extern Word32 ScreenCurrentTexture;					/* Last texture found */
extern ScreenShapeInitProcPtr ScreenInit;									/* Init the 3D context */
extern ScreenShapeSolidRectProcPtr ScreenSolidRect;		/* Draw a solid rect */

extern void BURGERCALL ScreenInitAPI(Word APIType);
extern Word BURGERCALL ScreenSetDisplayToSize(Word Width,Word Height,Word Depth,Word Flags);
extern void BURGERCALL ScreenBeginScene(void);
extern void BURGERCALL ScreenEndScene(void);
#define ScreenSetTranslucencyMode(x) if (ScreenTranslucencyMode!=(x)) { ScreenForceTranslucencyMode(x); }
extern void BURGERCALL ScreenForceTranslucencyMode(TranslucencyMode_e NewMode);
#define ScreenSetFilteringMode(x) if (ScreenFilteringMode!=(x)) { ScreenForceFilteringMode(x); }
extern void BURGERCALL ScreenForceFilteringMode(FilteringMode_e NewMode);
#define ScreenSetWireFrameMode(x) if (ScreenWireFrameMode!=(x)) { ScreenForceWireFrameMode(x); }
extern void BURGERCALL ScreenForceWireFrameMode(Word Flag);
#define ScreenSetTexture(x) if (ScreenCurrentTexture!=(x)) { ScreenForceTexture(x); }
extern void BURGERCALL ScreenForceTexture(Word32 TexNum);
#define ScreenSetPerspective(x) if (ScreenPerspectiveMode!=(x)) { ScreenForcePerspective(x); }
extern void BURGERCALL ScreenForcePerspective(Word Flag);
#define ScreenSetShadingMode(x) if (ScreenShadingMode!=(x)) { ScreenForceShadingMode(x); }
extern void BURGERCALL ScreenForceShadingMode(ShadingMode_e NewMode);
#define ScreenSetDepthWriteMode(x) if (ScreenDepthWriteMode!=(x)) { ScreenForceDepthWriteMode(x); }
extern void BURGERCALL ScreenForceDepthWriteMode(Word Flag);
#define ScreenSetDepthTestMode(x) if (ScreenDepthTestMode!=(x)) { ScreenForceDepthTestMode(x); }
extern void BURGERCALL ScreenForceDepthTestMode(DepthTestMode_e NewMode);
#define ScreenUse2DCoords(x) if (ScreenUsing2DCoords!=(x)) { ScreenForceUse2DCoords(x); }
extern void BURGERCALL ScreenForceUse2DCoords( Word use2d );

extern void BURGERCALL ScreenShapeInit(ScreenShape_t *Input,ScreenShapeActionProc ActionProc);
extern ScreenShape_t *BURGERCALL ScreenShapeNew(ScreenShapeActionProc ActionProc);
extern void BURGERCALL ScreenShapeDestroy(ScreenShape_t *Input);
extern void BURGERCALL ScreenShapeDelete(ScreenShape_t *Input);
extern void BURGERCALL ScreenShapePurge(ScreenShape_t *Input);
extern void BURGERCALL ScreenShapeDraw(ScreenShape_t *Input,int x,int y);
extern void BURGERCALL ScreenShapeDrawScaled(ScreenShape_t *Input,const struct LBRect *DestRect);
extern void BURGERCALL ScreenShapeDrawScaledSubRect(ScreenShape_t *Input,const struct LBRect *DestRect,const struct LBRect *SrcRect);
extern void BURGERCALL ScreenShapeLock(ScreenShape_t *Input,struct Image_t *Output);
extern void BURGERCALL ScreenShapeUnlock(ScreenShape_t *Input);
extern Word BURGERCALL ScreenShapeLoad(ScreenShape_t *Input);
extern void BURGERCALL ScreenShapeDisallowPurge(ScreenShape_t *Input);
extern void BURGERCALL ScreenShapeAllowPurge(ScreenShape_t *Input);
extern void BURGERCALL ScreenShapeGetBounds(ScreenShape_t *Input,struct LBRect *Bounds);
extern void BURGERCALL ScreenShapeGetSize(ScreenShape_t *Input,struct LBPoint *Size);
extern Word BURGERCALL ScreenShapeGetPixel(ScreenShape_t *Input,int x,int y);
extern int BURGERCALL ScreenShapeVPatternBar(ScreenShape_t **ArrayPtr,int x,int TopY,int BottomY);
extern int BURGERCALL ScreenShapeHPatternBar(ScreenShape_t **ArrayPtr,int y,int LeftX,int RightX);
extern Word BURGERCALL ScreenShapeConvertFromImage(ScreenShape_t *Input,const struct Image_t *ImagePtr);

extern void BURGERCALL ScreenShapeGfxInit(ScreenShapeGfx_t *Input,struct RezHeader_t *RezFile,Word RezNum);
extern ScreenShapeGfx_t *BURGERCALL ScreenShapeGfxNew(struct RezHeader_t *RezFile,Word RezNum);
extern void BURGERCALL ScreenShapeGfxReinit(ScreenShapeGfx_t *Input,struct RezHeader_t *RezFile,Word RezNum);

extern void BURGERCALL ScreenShapeGfxFileInit(ScreenShapeGfxFile_t *Input,const char *FileName);
extern ScreenShapeGfxFile_t *BURGERCALL ScreenShapeGfxFileNew(const char *FileName);

extern void BURGERCALL ScreenShapePtrInit(ScreenShapePtr_t *Input,struct Image_t *ImagePtr);
extern ScreenShapePtr_t *BURGERCALL ScreenShapePtrNew(struct Image_t *ImagePtr);

extern void BURGERCALL ScreenShapeRezGroupInit(ScreenShapeRezGroup_t *Input,struct RezHeader_t *RezFile,Word RezNum,Word RezPal,Word Which);
extern ScreenShapeRezGroup_t *BURGERCALL ScreenShapeRezGroupNew(struct RezHeader_t *RezFile,Word RezNum,Word RezPal,Word Which);

extern void BURGERCALL ScreenShapeGifFileInit(ScreenShapeGifFile_t *Input,const char *FileName);
extern ScreenShapeGifFile_t *BURGERCALL ScreenShapeGifFileNew(const char *FileName);

extern void BURGERCALL ScreenShapeGifInit(ScreenShapeGif_t *Input,struct RezHeader_t *RezFile,Word RezNum);
extern ScreenShapeGif_t *BURGERCALL ScreenShapeGifNew(struct RezHeader_t *RezFile,Word RezNum);
extern void BURGERCALL ScreenShapeGifReinit(ScreenShapeGif_t *Input,struct RezHeader_t *RezFile,Word RezNum);

extern void BURGERCALL ScreenShapeBmpFileInit(ScreenShapeBmpFile_t *Input,const char *FileName);
extern ScreenShapeBmpFile_t *BURGERCALL ScreenShapeBmpFileNew(const char *FileName);

extern void BURGERCALL ScreenShapeBmpInit(ScreenShapeBmp_t *Input,struct RezHeader_t *RezFile,Word RezNum);
extern ScreenShapeBmp_t *BURGERCALL ScreenShapeBmpNew(struct RezHeader_t *RezFile,Word RezNum);
extern void BURGERCALL ScreenShapeBmpReinit(ScreenShapeBmp_t *Input,struct RezHeader_t *RezFile,Word RezNum);

/* OS Cursor handler */

extern void BURGERCALL OSCursorSet(Word Curnum);
extern void BURGERCALL OSCursorShow(void);
extern void BURGERCALL OSCursorHide(void);
extern void BURGERCALL OSCursorReset(void);
extern Word BURGERCALL OSCursorPresent(void);
extern Word BURGERCALL OSCursorIsVisible(void);
extern Word BURGERCALL OSCursorNumber(void);

/* Rect handlers */

extern void BURGERCALL LBPointFromSYSPOINT(LBPoint *Output,const SYSPOINT *Input);
extern void BURGERCALL LBPointToSYSPOINT(SYSPOINT *Output,const LBPoint *Input);
extern Word BURGERCALL SYSRECTPtInRect(const SYSRECT *InputRect,int x,int y);
extern Word BURGERCALL LBPointRead(LBPoint *Output,FILE *fp);
extern Word BURGERCALL LBPointWrite(const LBPoint *Input,FILE *fp);

extern void BURGERCALL LBRectSetRect(LBRect *Input,int x1,int y1,int x2,int y2);
#define LBRectWidth(Input) ((Input)->right-(Input)->left)
#define LBRectHeight(Input) ((Input)->bottom-(Input)->top)
extern void BURGERCALL LBRectSetRectEmpty(LBRect *Input);
extern void BURGERCALL LBRectSetWidth(LBRect *Input,int Width);
extern void BURGERCALL LBRectSetHeight(LBRect *Input,int Height);
extern Word BURGERCALL LBRectPtInRect(const LBRect *Input,int x,int y);
extern Word BURGERCALL LBRectPointInRect(const LBRect *Input1,const LBPoint *Input2);
extern void BURGERCALL LBRectOffsetRect(LBRect *Input,int h,int v);
extern void BURGERCALL LBRectInsetRect(LBRect *Input,int x,int y);
extern Word BURGERCALL LBRectIsEqual(const LBRect *Input1,const LBRect *Input2);
extern Word BURGERCALL LBRectIntersectRect(LBRect *Input,const LBRect *rect1,const LBRect *rect2);
extern void BURGERCALL LBRectUnionRect(LBRect *Output,const LBRect *Input1,const LBRect *Input2);
extern void BURGERCALL LBRectAddPointToRect(LBRect *Output,const LBPoint *Input);
extern void BURGERCALL LBRectAddXYToRect(LBRect *Input,int x,int y);
extern Word BURGERCALL LBRectIsRectEmpty(const LBRect *Input);
extern Word BURGERCALL LBRectIsInRect(const LBRect *Input1,const LBRect *Input2);
extern void BURGERCALL LBRectClipWithinRect(LBRect *Input,const LBRect *Bounds);
extern void BURGERCALL LBRectMove(LBRect *Input,int x,int y);
extern void BURGERCALL LBRectMoveX(LBRect *Input,int x);
extern void BURGERCALL LBRectMoveY(LBRect *Input,int y);
extern void BURGERCALL LBRectMoveToPoint(LBRect *Input,const LBPoint *Input2);
extern void BURGERCALL LBRectMoveWithinRect(LBRect *Input,const LBRect *Bounds);
extern void BURGERCALL LBRectFix(LBRect *Input);
extern void BURGERCALL LBRectGetCenter(int *x,int *y,const LBRect *Input);
extern int BURGERCALL LBRectGetCenterX(const LBRect *Input);
extern int BURGERCALL LBRectGetCenterY(const LBRect *Input);
extern void BURGERCALL LBRectGetCenterPoint(LBPoint *Output,const LBRect *Input);
extern void BURGERCALL LBRectCenterAroundPoint(LBRect *Output,const LBPoint *Input);
extern void BURGERCALL LBRectCenterAroundXY(LBRect *Output,int x,int y);
extern void BURGERCALL LBRectCenterAroundX(LBRect *Output,int x);
extern void BURGERCALL LBRectCenterAroundY(LBRect *Output,int y);
extern void BURGERCALL LBRectCenterAroundRectCenter(LBRect *Output,const LBRect *Input);
extern void BURGERCALL LBRectCenterAroundRectCenterX(LBRect *Output,const LBRect *Input);
extern void BURGERCALL LBRectCenterAroundRectCenterY(LBRect *Output,const LBRect *Input);
extern void BURGERCALL LBRectMapPoint(LBPoint *Output,const LBRect *SrcBoundsRect,const LBRect *DestBoundsRect,const LBPoint *Input);
extern void BURGERCALL LBRectMapRect(LBRect *Output,const LBRect *SrcBoundsRect,const LBRect *DestBoundsRect,const LBRect *Input);
extern void BURGERCALL LBRectFromSYSRECT(LBRect *Output,const SYSRECT *Input);
extern void BURGERCALL LBRectToSYSRECT(SYSRECT *Output,const LBRect *Input);
extern Word BURGERCALL LBRectRead(LBRect *Output,FILE *fp);
extern Word BURGERCALL LBRectWrite(const LBRect *Input,FILE *fp);
extern void BURGERCALL LBRectReadStream(LBRect *Output,struct StreamHandle_t *fp);
extern void BURGERCALL LBRectWriteStream(const LBRect *Input,struct StreamHandle_t *fp);

extern LBRectList * BURGERCALL LBRectListNew(void);
extern void BURGERCALL LBRectListDelete(LBRectList *Input);
extern void BURGERCALL LBRectListInit(LBRectList *Input);
extern void BURGERCALL LBRectListDestroy(LBRectList *Input);
extern Word BURGERCALL LBRectListRectClip(LBRectList *Input,const LBRect* b,const LBRect* t);
extern void BURGERCALL LBRectListClipOutRect(LBRectList *Input,const LBRect *bound);
extern void BURGERCALL LBRectListClipOutRectList(LBRectList *Input,const LBRectList *list);
extern void BURGERCALL LBRectListAppendRect(LBRectList *Input,const LBRect *rect);
extern void BURGERCALL LBRectListAppendRectList(LBRectList *Input,const LBRectList *list);
extern void BURGERCALL LBRectListCopy(LBRectList *Input,const LBRectList *list);
extern void BURGERCALL LBRectListRead(LBRectList *Output,FILE *fp);
extern void BURGERCALL LBRectListWrite(const LBRectList *Input,FILE *fp);

/* Memory handlers */

#define MMStageCompact 0
#define MMStagePurge 1

typedef void (BURGERCALL *MemPurgeProcPtr)(Word Stage);

extern Word32 MaxMemSize;		/* Maximum memory the program will take (4M) */
extern Word32 MinReserveSize;	/* Minimum memory for OS (64K) */
extern Word MinHandles;			/* Number of handles to create (500) */
extern MemPurgeProcPtr MemPurgeCallBack;	/* Callback before memory purging */
extern void BURGERCALL InitMemory(void);	/* Call this FIRST! */
#define AllocAHandle(MemSize) AllocAHandle2(MemSize,0)
extern void ** BURGERCALL AllocAHandle2(Word32 MemSize,Word Flag);
extern void ** BURGERCALL AllocAHandleClear(Word32 MemSize);
extern void BURGERCALL DeallocAHandle(void **MemHandle);
extern void * BURGERCALL AllocAPointer(Word32 MemSize);
extern void * BURGERCALL AllocAPointerClear(Word32 MemSize);
extern void BURGERCALL DeallocAPointer(void *MemPtr);
extern void ** BURGERCALL ReallocAHandle(void **MemHandle);
extern void ** BURGERCALL FindAHandle(void *MemPtr);
extern void * BURGERCALL LockAHandle(void **MemHandle);
extern void BURGERCALL UnlockAHandle(void **MemHandle);
extern void BURGERCALL CompactHandles(void);
extern void BURGERCALL PurgeAHandle(void **MemHandle);
extern Word BURGERCALL PurgeHandles(Word32 MemSize);
extern Word32 BURGERCALL GetAHandleSize(void **MemHandle);
extern Word32 BURGERCALL GetAPointerSize(void *MemPtr);
extern Word BURGERCALL GetAHandleLockedState(void **MemHandle);
extern void BURGERCALL SetAHandleLockedState(void **MemHandle,Word State);
extern Word32 BURGERCALL GetTotalFreeMem(void);
extern Word32 BURGERCALL GetTotalAllocatedMem(void);
extern void BURGERCALL SetHandlePurgeFlag(void **MemHandle,Word Flag);
extern void BURGERCALL DumpHandles(void);
extern void * BURGERCALL ResizeAPointer(void *Mem,Word32 Size);
extern void ** BURGERCALL ResizeAHandle(void **Mem,Word32 Size);
extern void * BURGERCALL MemoryNewPointerCopy(const void *Mem,Word32 Size);
extern void ** BURGERCALL MemoryNewHandleCopy(void **Mem);
extern void BURGERCALL DebugAddSourceLine(void **MemHandle,const char *Source,Word32 Line,Word IsPointer);
extern Word BURGERCALL DebugRemoveSourceLine(void **MemHandle,const char *Source,Word LineNum);
extern void BURGERCALL DebugGetSourceLineInfo(void **MemHanel,char **Source,Word32 *Line);
extern void ** BURGERCALL DebugAllocAHandle2(Word32 MemSize,Word Flag,const char *Source,Word LineNum);
extern void ** BURGERCALL DebugAllocAHandleClear(Word32 MemSize,const char *Source,Word LineNum);
extern void BURGERCALL DebugDeallocAHandle(void **MemHandle,const char *Source,Word LineNum);
extern void * BURGERCALL DebugAllocAPointer(Word32 MemSize,const char *Source,Word LineNum);
extern void * BURGERCALL DebugAllocAPointerClear(Word32 MemSize,const char *Source,Word LineNum);
extern void BURGERCALL DebugDeallocAPointer(void *MemPtr,const char *Source,Word LineNum);
extern void ** BURGERCALL DebugReallocAHandle(void **MemHandle,const char *Source,Word LineNum);
extern void * BURGERCALL DebugResizeAPointer(void *Mem,Word32 Size,const char *Source,Word LineNum);
extern void ** BURGERCALL DebugResizeAHandle(void **Mem,Word32 Size,const char *Source,Word LineNum);
extern void * BURGERCALL DebugMemoryNewPointerCopy(const void *Mem,Word32 Size,const char *Source,Word LineNum);
extern void ** BURGERCALL DebugMemoryNewHandleCopy(void **Mem,const char *Source,Word LineNum);
extern Word BURGERCALL DebugMemoryIsPointerValid(void *MemPtr);
extern Word BURGERCALL DebugMemoryIsHandleValid(void **MemHandle);

#if _DEBUG
#undef AllocAHandle
#define AllocAHandle(x) DebugAllocAHandle2(x,0,__FILE__,__LINE__)
#define AllocAHandle2(x,y) DebugAllocAHandle2(x,y,__FILE__,__LINE__)
#define AllocAHandleClear(x) DebugAllocAHandleClear(x,__FILE__,__LINE__)
#define DeallocAHandle(x) DebugDeallocAHandle(x,__FILE__,__LINE__)
#define AllocAPointer(x) DebugAllocAPointer(x,__FILE__,__LINE__)
#define AllocAPointerClear(x) DebugAllocAPointerClear(x,__FILE__,__LINE__)
#define DeallocAPointer(x) DebugDeallocAPointer(x,__FILE__,__LINE__)
#define ReallocAHandle(x) DebugReallocAHandle(x,__FILE__,__LINE__)
#define ResizeAPointer(x,y) DebugResizeAPointer(x,y,__FILE__,__LINE__)
#define ResizeAHandle(x,y) DebugResizeAHandle(x,y,__FILE__,__LINE__)
#define MemoryNewPointerCopy(x,y) DebugMemoryNewPointerCopy(x,y,__FILE__,__LINE__)
#define MemoryNewHandleCopy(x) DebugMemoryNewHandleCopy(x,__FILE__,__LINE__)
#define MemoryIsPointerValid(x) DebugMemoryIsPointerValid(x)
#define MemoryIsHandleValid(x) DebugMemoryIsHandleValid(x)
#else
#define MemoryIsPointerValid(x)
#define MemoryIsHandleValid(x)
#endif

#if !defined(__MAC__)
extern MyHandle UsedHand1;		/* First used memory handle */
extern MyHandle UsedHand2;		/* Last used memory handle */
extern MyHandle PurgeHands;		/* Purged handle list */
extern MyHandle FreeMemHands;	/* Free handle list */
extern MyHandle PurgeHandleFiFo;	/* Purged handle linked list */
extern void BURGERCALL ReleaseMemRange(void *MemPtr,Word32 Length,MyHandle *Parent);
extern void BURGERCALL GrabMemRange(void *MemPtr,Word32 Length,MyHandle *Parent,MyHandle *Scan);
extern MyHandle * BURGERCALL GetFromFreeHandleList(void);
extern void BURGERCALL AddToFreeHandleList(MyHandle *Input);
#endif

#if defined(__MSDOS__)
extern void *_x32_zero_base_ptr;
extern Word16 _x32_zero_base_selector;
#define ZeroBase ((Word8 *)_x32_zero_base_ptr)
extern Word32 BURGERCALL AllocRealMemory(Word32 Size);
extern void BURGERCALL DeallocRealMemory(Word32 RealPtr);
extern void * BURGERCALL RealToProtectedPtr(Word32 RealPtr);
extern Word32 BURGERCALL GetRealBufferPtr(void);
extern void * BURGERCALL GetRealBufferProtectedPtr(void);
#endif

typedef enum { MEMPOOL_STATIC,MEMPOOL_DYNAMIC,MEMPOOL_LOOP} MemPoolBehaviour_e;

typedef struct MemPool_t {
	void ***ArrayHandle;		/* Handle to free memory list */
	Word8 *RootMem;				/* Pointer to the allocated memory */
	Word ArraySize;				/* Size of the free memory list */
	Word Count;					/* Number of entries used */
	Word ChunkSize;				/* Size of each entry in bytes */
	MemPoolBehaviour_e Behaviour;	/* Type of MemPool_t */
} MemPool_t;

extern Word BURGERCALL MemPoolInit(MemPool_t *Input,Word InitialSize,Word ChunkSize,MemPoolBehaviour_e Behaviour);
extern MemPool_t * BURGERCALL MemPoolNew(Word InitialSize,Word ChunkSize,MemPoolBehaviour_e Behaviour);
extern void BURGERCALL MemPoolDestroy(MemPool_t *Input);
extern void BURGERCALL MemPoolDelete(MemPool_t *Input);
extern void * BURGERCALL MemPoolAllocate(MemPool_t *Input);
extern void BURGERCALL MemPoolDeallocate(MemPool_t *Input,void *MemPtr);

/* Resource Handlers */

typedef struct RezHeader_t {	/* Master entry to the resource manager */
	void (BURGERCALL *DecompPtrs[3])(Word8 *,Word8 *,Word32,Word32);	/* Decompressors */
	Word32 Count;				/* Number of resource groups */
	Word32 RezNameCount;		/* Number of resource names */
	void *fp;					/* Open file reference */
	struct RezGroup_t **GroupHandle;	/* First entry */
	struct RezName_t **RezNames;		/* Handle to resource names if present */
	Word Flags;					/* Flags on how to handle resources */
} RezHeader_t;

typedef struct RezNameReturn_t {
	char *RezName;		/* Resource name */
	Word RezNum;		/* Resource number */
} RezNameReturn_t;

typedef void (BURGERCALL *ResourceDecompressorProcPtr)(Word8 *,Word8 *,Word32 Length,Word32 PackLength);

extern Bool ResourceJustLoaded;	/* TRUE if ResourceLoadHandle() freshly loaded a handle */
extern RezHeader_t MasterRezHeader;	/* Default resource file */

extern RezHeader_t *ResourceNew(const char *FileName,Word32 StartOffset);
extern Word BURGERCALL ResourceInit(RezHeader_t *Input,const char *FileName,Word32 StartOffset);
extern void BURGERCALL ResourceDestroy(RezHeader_t *Input);
extern void BURGERCALL ResourceDelete(RezHeader_t *Input);
extern Word BURGERCALL ResourceInitMasterRezHeader(const char *FileName);
extern void BURGERCALL ResourcePurgeCache(RezHeader_t *Input);
extern Word BURGERCALL ResourceExternalFlag(RezHeader_t *Input,Word Flag);
extern Word BURGERCALL ResourceDontCacheFlag(RezHeader_t *Input,Word Flag);
extern Word BURGERCALL ResourceAddName(RezHeader_t *Input,const char *RezName);
extern void BURGERCALL ResourceRemove(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourceRemoveName(RezHeader_t *Input,const char *RezName);
extern Word BURGERCALL ResourceRead(RezHeader_t *Input,Word RezNum,void *DestPtr,Word32 BufSize);
extern void * BURGERCALL ResourceLoad(RezHeader_t *Input,Word RezNum);
extern void * BURGERCALL ResourceLoadByName(RezHeader_t *Input,const char *RezName);
extern void ** BURGERCALL ResourceLoadHandle(RezHeader_t *Input,Word RezNum);
extern void ** BURGERCALL ResourceLoadHandleByName(RezHeader_t *Input,const char *RezName);
extern void BURGERCALL ResourceRelease(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourceReleaseByName(RezHeader_t *Input,const char *RezName);
extern void BURGERCALL ResourceKill(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourceKillByName(RezHeader_t *Input,const char *RezName);
extern void BURGERCALL ResourceDetach(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourceDetachByName(RezHeader_t *Input,const char *RezName);
extern void BURGERCALL ResourcePreload(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourcePreloadByName(RezHeader_t *Input,const char *RezName);
extern Word BURGERCALL ResourceGetRezNum(RezHeader_t *Input,const char *RezName);
extern Word BURGERCALL ResourceGetName(RezHeader_t *Input,Word RezNum,char *Buffer,Word BufferSize);
extern Word BURGERCALL ResourceGetIDFromHandle(RezHeader_t *Input,const void **RezHand,Word *IDFound,char *NameBuffer,Word NameBufferSize);
extern Word BURGERCALL ResourceGetIDFromPointer(RezHeader_t *Input,const void *RezPtr,Word *IDFound,char *NameBuffer,Word NameBufferSize);
extern RezNameReturn_t *BURGERCALL ResourceGetNameArray(RezHeader_t *Input,Word *EntryCountPtr);
extern void BURGERCALL ResourceLogDecompressor(RezHeader_t *Input,Word CompressID,ResourceDecompressorProcPtr Proc);
extern struct LWShape_t * BURGERCALL ResourceLoadShape(RezHeader_t *Input,Word RezNum);
extern struct LWXShape_t * BURGERCALL ResourceLoadXShape(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourcePreloadShape(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourcePreloadXShape(RezHeader_t *Input,Word RezNum);
extern void * BURGERCALL ResourceLoadShapeArray(RezHeader_t *Input,Word RezNum);
extern void * BURGERCALL ResourceLoadXShapeArray(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourcePreloadShapeArray(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourcePreloadXShapeArray(RezHeader_t *Input,Word RezNum);
extern struct LWShape_t ** BURGERCALL ResourceLoadShapeHandle(RezHeader_t *Input,Word RezNum);
extern struct LWXShape_t ** BURGERCALL ResourceLoadXShapeHandle(RezHeader_t *Input,Word RezNum);
extern void ** BURGERCALL ResourceLoadShapeArrayHandle(RezHeader_t *Input,Word RezNum);
extern void ** BURGERCALL ResourceLoadXShapeArrayHandle(RezHeader_t *Input,Word RezNum);
extern struct GfxShape_t *BURGERCALL ResourceLoadGfxShape(RezHeader_t *Input,Word RezNum);
extern void BURGERCALL ResourcePreloadGfxShape(RezHeader_t *Input,Word RezNum);
extern void ** BURGERCALL ResourceLoadGfxShapeHandle(RezHeader_t *Input,Word RezNum);


/* ASync data reading */

typedef struct ReadFileStream_t ReadFileStream_t;

extern struct ReadFileStream_t *BURGERCALL ReadFileStreamNew(const char *FileName,Word Count,Word ChunkSize);
extern void BURGERCALL ReadFileStreamDelete(struct ReadFileStream_t *Input);
extern Word BURGERCALL ReadFileStreamActive(struct ReadFileStream_t *Input);
extern Word BURGERCALL ReadFileStreamPending(struct ReadFileStream_t *Input);
extern void BURGERCALL ReadFileStreamStop(struct ReadFileStream_t *Input);
extern void BURGERCALL ReadFileStreamStart(struct ReadFileStream_t *Input,Word32 Offset);
extern Word32 BURGERCALL ReadFileStreamAvailBytes(struct ReadFileStream_t *Input);
extern Word8 * BURGERCALL ReadFileStreamGetData(struct ReadFileStream_t *Input,Word32 *ReadSizeOut,Word32 ReadSize);
extern void BURGERCALL ReadFileStreamAcceptData(struct ReadFileStream_t *Input);

/* Compression routines */

typedef struct PackState_t {	/* State of a decompression buffer */
    Word8 *PackPtr;				/* Packed data pointer */
    Word32 PackLen;			/* Number of packed bytes remaining */
    Word8 *OutPtr;				/* Output data pointer */
    Word32 OutLen;			/* Number of bytes in the output buffer */
    void *Internal;				/* Pointer to algorithm specific code */
} PackState_t;

typedef struct MACEState_t {	/* State of MACE compression/decompression */
	long Sample1;		/* Last running samples */
	long Sample2;		/* Second temp sample */
	long PrevVal;		/* Mask with 0x8000 for + or - direction */
	long TableIndex;	/* Which slope table */
	long LastAmplitude;	/* Slope * PrevVal */
	long LastSlope;		/* Last Slope value */
} MACEState_t;

typedef struct ADPCMUnpackState_t {	/* State of ADPCM decompression */
	Word8 *SrcPtr;			/* Pointer to the source data */
	Word32 SrcLength;		/* Pointer to the size of the source buffer */
	Word Channels;			/* Number of channels to decode (1 or 2) */
	Word BlockSize;			/* Size of each compressed block */
	Word SamplesPerBlock;	/* Number of samples to decompress per block */
	short *OutputPtr;		/* Output buffer */
} ADPCMUnpackState_t;

extern void BURGERCALL DLZSS(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length,Word32 PackedLen);
extern void BURGERCALL DLZSSFast(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length);
extern void ** BURGERCALL EncodeLZSS(Word8 *InputBuffer,Word32 Length);
extern void BURGERCALL DHuffman(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length,Word32 PackedLen);
extern void BURGERCALL DHuffmanFast(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length);
extern void ** BURGERCALL EncodeHuffman(Word8 *InputBuffer,Word32 Length);
extern void BURGERCALL DLZH(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length,Word32 PackedLen);
extern void BURGERCALL DLZHFast(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length);
extern void ** BURGERCALL EncodeLZH(Word8 *InputBuffer,Word32 Length);
extern void BURGERCALL DRLE(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length,Word32 PackedLen);
extern void BURGERCALL DRLEFast(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length);
extern void ** BURGERCALL EncodeRLE(Word8 *InputBuffer,Word32 Length);
extern Word BURGERCALL DInflateInit(PackState_t *Input);
extern Word BURGERCALL DInflateMore(PackState_t *Input);
extern void BURGERCALL DInflateDestroy(PackState_t *Input);
extern void BURGERCALL DInflate(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length,Word32 PackedLen);
extern void BURGERCALL DInflateFast(Word8 *DestPtr,Word8 *SrcPtr,Word32 Length);
extern void ** BURGERCALL EncodeInflate(Word8 *InputBuffer,Word32 Length);
extern void BURGERCALL MACEExp1to3(const Word8 *InBufPtr,Word8 *OutBufPtr,Word32 Count,MACEState_t *InStatePtr,MACEState_t *OutStatePtr,Word NumChannels,Word WhichChannel);
extern void BURGERCALL MACEExp1to6(const Word8 *InBufPtr,Word8 *OutBufPtr,Word32 Count,MACEState_t *InStatePtr,MACEState_t *OutStatePtr,Word NumChannels,Word WhichChannel);
extern Word BURGERCALL ADPCMDecodeBlock(ADPCMUnpackState_t *StatePtr);

/* Ogg libraries */

typedef struct oggpack_buffer {
	Word8 *ptr;			/* Pointer to the current byte scanned */
	Word8 *buffer;		/* Pointer to the work buffer */
	Word32 endbyte;	/* Bytes accepted */
	Word32 storage;	/* Size of the buffer in bytes */
	Word endbit;		/* Work bit (0-7) */
} oggpack_buffer;

typedef struct ogg_page {
	Word8 *header;			/* Pointer to the data header */
	Word8 *body;				/* Pointer to the data */
	Word32 header_len;	/* Size of the header */
	Word32 body_len;		/* Size of the data */
} ogg_page;

typedef struct ogg_stream_state {
	Word8 *body_data;		/* bytes from packet bodies */
	long body_storage;		/* storage elements allocated */
	long body_fill;			/* elements stored; fill mark */
	long body_returned;		/* elements of fill returned */

	int *lacing_vals;      /* The values that will go to the segment table */
	LongWord64_t *granule_vals;	/* granulepos values for headers. Not compact */
								/* this way, but it is simple coupled to the lacing fifo */
	long lacing_storage;
	long lacing_fill;
	long lacing_packet;
	long lacing_returned;

	Word8 header[282];		/* working space for header encode */
	Word8 Padding[2];		/* Long align */
	int header_fill;

	int e_o_s;				/* set when we have buffered the last packet in the logical bitstream */
	int b_o_s;				/* set after we've written the initial page of a logical bitstream */
	long serialno;
	long pageno;
	LongWord64_t packetno;	/* sequence number for decode; the framing
								knows where there's a hole in the data,
								but we need coupling so that the codec
								(which is in a seperate abstraction
								layer) also knows about the gap */
	LongWord64_t granulepos;
} ogg_stream_state;

typedef struct ogg_packet {
	Word8 *packet;
	long bytes;
	long b_o_s;
	long e_o_s;
	LongWord64_t granulepos;
	LongWord64_t packetno;		/* sequence number for decode; the framing
								knows where there's a hole in the data,
								but we need coupling so that the codec
								(which is in a seperate abstraction
								layer) also knows about the gap */
} ogg_packet;

typedef struct ogg_sync_state {
	Word8 *data;
	int storage;
	int fill;
	int returned;
	int unsynced;
	int headerbytes;
	int bodybytes;
} ogg_sync_state;

extern void BURGERCALL oggpack_writeinit(oggpack_buffer *b);
extern void BURGERCALL oggpack_reset(oggpack_buffer *b);
extern void BURGERCALL oggpack_writeclear(oggpack_buffer *b);
extern void BURGERCALL oggpack_readinit(oggpack_buffer *b,Word8 *buf,Word32 bytes);
extern void BURGERCALL oggpack_write(oggpack_buffer *b,Word32 value,Word bits);
extern Word32 BURGERCALL oggpack_look(oggpack_buffer *b,Word bits);
extern Word BURGERCALL oggpack_look1(oggpack_buffer *b);
extern Word32 BURGERCALL oggpack_look_huff(oggpack_buffer *b,Word bits);
extern void BURGERCALL oggpack_adv(oggpack_buffer *b,Word bits);
extern void BURGERCALL oggpack_adv1(oggpack_buffer *b);
extern int BURGERCALL oggpack_adv_huff(oggpack_buffer *b,Word bits);
extern Word32 BURGERCALL oggpack_read(oggpack_buffer *b,Word bits);
extern Word BURGERCALL oggpack_read1(oggpack_buffer *b);
extern Word32 BURGERCALL oggpack_bytes(oggpack_buffer *b);
extern Word32 BURGERCALL oggpack_bits(oggpack_buffer *b);
extern Word8 *BURGERCALL oggpack_get_buffer(oggpack_buffer *b);
extern Word BURGERCALL ogg_page_version(ogg_page *og);
extern Word BURGERCALL ogg_page_continued(ogg_page *og);
extern Word BURGERCALL ogg_page_bos(ogg_page *og);
extern Word BURGERCALL ogg_page_eos(ogg_page *og);
extern LongWord64_t BURGERCALL ogg_page_granulepos(ogg_page *og);
extern Word32 BURGERCALL ogg_page_serialno(ogg_page *og);
extern Word32 BURGERCALL ogg_page_pageno(ogg_page *og);
extern Word BURGERCALL ogg_page_packets(ogg_page *og);
extern int BURGERCALL ogg_stream_init(ogg_stream_state *os,int serialno);
extern void BURGERCALL ogg_stream_clear(ogg_stream_state *os);
extern void BURGERCALL ogg_stream_destroy(ogg_stream_state *os);
extern void BURGERCALL ogg_page_checksum_set(ogg_page *og);
extern int BURGERCALL ogg_stream_packetin(ogg_stream_state *os, ogg_packet *op);
extern int BURGERCALL ogg_stream_flush(ogg_stream_state *os, ogg_page *og);
extern int BURGERCALL ogg_stream_pageout(ogg_stream_state *os, ogg_page *og);
extern int BURGERCALL ogg_stream_eos(ogg_stream_state *os);
extern int BURGERCALL ogg_sync_init(ogg_sync_state *oy);
extern int BURGERCALL ogg_sync_clear(ogg_sync_state *oy);
extern int BURGERCALL ogg_sync_destroy(ogg_sync_state *oy);
extern char *BURGERCALL ogg_sync_buffer(ogg_sync_state *oy, long size);
extern int BURGERCALL ogg_sync_wrote(ogg_sync_state *oy, long bytes);
extern long BURGERCALL ogg_sync_pageseek(ogg_sync_state *oy,ogg_page *og);
extern int BURGERCALL ogg_sync_pageout(ogg_sync_state *oy, ogg_page *og);
extern int BURGERCALL ogg_stream_pagein(ogg_stream_state *os, ogg_page *og);
extern int BURGERCALL ogg_sync_reset(ogg_sync_state *oy);
extern int BURGERCALL ogg_stream_reset(ogg_stream_state *os);
extern int BURGERCALL ogg_stream_packetout(ogg_stream_state *os,ogg_packet *op);
extern int BURGERCALL ogg_stream_packetpeek(ogg_stream_state *os,ogg_packet *op);
extern void BURGERCALL ogg_packet_clear(ogg_packet *op);

/* Vorbis libraries */

#define OV_FALSE      -1  
#define OV_EOF        -2
#define OV_HOLE       -3
#define OV_EREAD      -128
#define OV_EFAULT     -129
#define OV_EIMPL      -130
#define OV_EINVAL     -131
#define OV_ENOTVORBIS -132
#define OV_EBADHEADER -133
#define OV_EVERSION   -134
#define OV_ENOTAUDIO  -135
#define OV_EBADPACKET -136
#define OV_EBADLINK   -137
#define OV_ENOSEEK    -138

typedef struct vorbis_info {
	int version;
	int channels;
	long rate;
	long bitrate_upper;
	long bitrate_nominal;
	long bitrate_lower;
	long bitrate_window;
	void *codec_setup;
} vorbis_info;

typedef struct vorbis_dsp_state {
	int analysisp;
	vorbis_info *vi;
	float **pcm;
	float **pcmret;
	int pcm_storage;
	int pcm_current;
	int pcm_returned;
	int preextrapolate;
	int eofflag;
	long lW;
	long W;
	long nW;
	long centerW;
	LongWord64_t granulepos;
	LongWord64_t sequence;
	LongWord64_t glue_bits;
	LongWord64_t time_bits;
	LongWord64_t floor_bits;
	LongWord64_t res_bits;
	void *backend_state;
} vorbis_dsp_state;

typedef struct vorbis_block {
	/* necessary stream state for linking to the framing abstraction */
	float  **pcm;       /* this is a pointer into local storage */ 
	oggpack_buffer opb;
	long lW;
	long W;
	long nW;
	int pcmend;
	int mode;
	int eofflag;
	LongWord64_t granulepos;
	LongWord64_t sequence;
	vorbis_dsp_state *vd; /* For read-only access of configuration */

	/* local storage to avoid remallocing; it's up to the mapping to
	structure it */
	void *localstore;
	long localtop;
	long localalloc;
	long totaluse;
	struct vorbis_alloc_chain *reap;

	/* bitmetrics for the frame */
	long glue_bits;
	long time_bits;
	long floor_bits;
	long res_bits;
	void *internal;
} vorbis_block;

typedef struct vorbis_alloc_chain {
	void *ptr;
	struct vorbis_alloc_chain *next;
} vorbis_alloc_chain;

typedef struct vorbis_comment{
	char **user_comments;
	int *comment_lengths;
	int comments;
	char *vendor;
} vorbis_comment;

extern int analysis_noisy;
extern int BURGERCALL vorbis_analysis(vorbis_block *vb,ogg_packet *op);
extern void BURGERCALL _analysis_output(char *base,int i,float *v,int n,int bark,int dB);
extern void BURGERCALL vorbis_comment_init(vorbis_comment *vc);
extern void BURGERCALL vorbis_comment_add(vorbis_comment *vc, char *comment); 
extern void BURGERCALL vorbis_comment_add_tag(vorbis_comment *vc, char *tag, char *contents);
extern char *BURGERCALL vorbis_comment_query(vorbis_comment *vc, char *tag, int count);
extern int BURGERCALL vorbis_comment_query_count(vorbis_comment *vc, char *tag);
extern void BURGERCALL vorbis_comment_clear(vorbis_comment *vc);
extern int BURGERCALL vorbis_info_blocksize(vorbis_info *vi,int zo);
extern void BURGERCALL vorbis_info_init(vorbis_info *vi);
extern void BURGERCALL vorbis_info_clear(vorbis_info *vi);
extern int BURGERCALL vorbis_synthesis_headerin(vorbis_info *vi,vorbis_comment *vc,ogg_packet *op);
extern int BURGERCALL vorbis_commentheader_out(vorbis_comment *vc, ogg_packet *op);
extern int BURGERCALL vorbis_analysis_headerout(vorbis_dsp_state *v,vorbis_comment *vc,
	ogg_packet *op,ogg_packet *op_comm,ogg_packet *op_code);
extern int BURGERCALL vorbis_synthesis(vorbis_block *vb,ogg_packet *op);
extern long BURGERCALL vorbis_packet_blocksize(vorbis_info *vi,ogg_packet *op);
extern int BURGERCALL vorbis_block_init(vorbis_dsp_state *v, vorbis_block *vb);
extern void *BURGERCALL _vorbis_block_alloc(vorbis_block *vb,long bytes);
extern void BURGERCALL _vorbis_block_ripcord(vorbis_block *vb);
extern int BURGERCALL vorbis_block_clear(vorbis_block *vb);
extern int BURGERCALL vorbis_analysis_init(vorbis_dsp_state *v,vorbis_info *vi);
extern void BURGERCALL vorbis_dsp_clear(vorbis_dsp_state *v);
extern float **BURGERCALL vorbis_analysis_buffer(vorbis_dsp_state *v,int vals);
extern int BURGERCALL vorbis_analysis_wrote(vorbis_dsp_state *v,int vals);
extern int BURGERCALL vorbis_analysis_blockout(vorbis_dsp_state *v,vorbis_block *vb);
extern int BURGERCALL vorbis_synthesis_init(vorbis_dsp_state *v,vorbis_info *vi);
extern int BURGERCALL vorbis_synthesis_blockin(vorbis_dsp_state *v,vorbis_block *vb);
extern int BURGERCALL vorbis_synthesis_pcmout(vorbis_dsp_state *v,float ***pcm);
extern int BURGERCALL vorbis_synthesis_read(vorbis_dsp_state *v,int samples);

/* Image routines */

typedef enum {IMAGE332=4,IMAGE8ALPHA=5,IMAGE8_PAL_ALPHA_PAL=6,IMAGE8_PAL_ALPHA=7,IMAGE8_PAL=8,
	IMAGE4444=13,IMAGE1555=14,IMAGE555=15,IMAGE565=16,IMAGE888=24,IMAGE8888=32} ImageTypes_e;

typedef struct Image_t {	/* Used by image routines */
	Word8 *ImagePtr;		/* Pointer to pixel array (AllocAPointer()) */
	Word8 *PalettePtr;	/* Pointer to RGB tripletts, only for 8 BPP images */
	Word8 *AlphaPtr;		/* Alpha channel if any */
	Word Width;			/* Width of image in pixels */
	Word Height;		/* Height of image in pixels */
	Word RowBytes;		/* Number of bytes per scan line */
	ImageTypes_e DataType;		/* 8,16,24 Bits per pixel */
} Image_t;

typedef enum {PSD_FLAG_RGB,PSD_FLAG_ALPHA,PSD_FLAG_MASK,PSD_FLAG_RGBA} PSDFlag_e;

typedef struct PSDImageLayer_t {
	char* Name;						/* Name of the image (If any) */
	int Top, Left, Bottom, Right;	/* Image bounds rect */
	int MaskTop, MaskLeft, MaskBottom, MaskRight;	/* Mask rect */
	int Width, Height;			/* Size of the image */
	int MaskWidth, MaskHeight;	/* Size of the mask */
	Word8* RGB;					/* RGB data (24 bit) */
	Word8* Alpha;				/* Alpha data */
	Word8* Mask;					/* Mask data */
} PSDImageLayer_t;

typedef struct PSDImage_t {
	Word Width;					/* Width in pixels */
	Word Height;				/* Height in pixels */
	Word NumLayers;				/* Number of layers present */
	PSDImageLayer_t* Layers;	/* Array of layers */
} PSDImage_t;

extern Word BURGERCALL ImageInit(Image_t *Output,Word Width,Word Height,ImageTypes_e Depth);
extern Image_t * BURGERCALL ImageNew(Word Width,Word Height,ImageTypes_e Depth);
extern Word BURGERCALL ImageInitCopy(Image_t *Output,const Image_t *Input);
extern Image_t * BURGERCALL ImageNewCopy(const Image_t *Input);
extern void BURGERCALL ImageDelete(Image_t *ImagePtr);
extern void BURGERCALL ImageDestroy(Image_t *ImagePtr);
extern Word BURGERCALL ImageParseAny(Image_t *Output,const Word8 *InputPtr,Word32 InputLength);
extern Word BURGERCALL ImageParseBMP(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParseCicn(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParseGIF(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParseIIGS(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParseJPG(Image_t *Output,const Word8 *InputPtr,Word32 InputLength);
extern Word BURGERCALL ImageParseLBM(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParsePCX(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParsePPat(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParsePict(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParsePSD(Image_t *Output,const Word8 *InputPtr,Word Layer);
extern Word BURGERCALL ImageParseTGA(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageParseTIFF(Image_t *Output,const Word8 *InputPtr);
extern Word BURGERCALL ImageExtractFromPSDImage(Image_t *Output,PSDImageLayer_t* Layer,PSDFlag_e flags);
extern Word BURGERCALL Image2BMPFile(Image_t *ImagePtr,const char *FileName,Word NoCompress);
extern Word BURGERCALL Image2GIFFile(Image_t *ImagePtr,const char *FileName);
extern Word BURGERCALL Image2IIGSFile(Image_t *ImagePtr,const char *FileName);
extern Word BURGERCALL Image2JPGFile(Image_t *ImagePtr,const char *FileName,Word CompressionFactor);
extern Word BURGERCALL Image2LBMFile(Image_t *ImagePtr,const char *FileName);
extern Word BURGERCALL Image2PBMFile(Image_t *ImagePtr,const char *FileName);
extern Word BURGERCALL Image2PCXFile(Image_t *ImagePtr,const char *FileName);
extern Word BURGERCALL Image2PSDFile(Image_t *ImagePtr,const char *FileName);
extern Word BURGERCALL Image2TGAFile(Image_t *ImagePtr,const char *FileName,Word NoCompress);
extern Word BURGERCALL Image2TIFFFile(Image_t *ImagePtr,const char *FileName);
extern Word BURGERCALL ImageStore(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore4444(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore555(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore565(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore1555(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore888(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore8888(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore332(Image_t *Output,const Image_t *Input);
extern Word BURGERCALL ImageStore8Pal(Image_t *Output,const Image_t *Input);
extern void BURGERCALL ImageColorKey8888(Image_t* SrcImagePtr,Word r,Word g,Word b,Word a);
extern Word BURGERCALL ImageSubImage(Image_t* Output,Word x,Word y,const Image_t* Input);
extern void BURGERCALL ImageVerticalFlip(Image_t *ImagePtr);
extern void BURGERCALL ImageHorizontalFlip(Image_t *ImagePtr);
extern void BURGERCALL ImageRemove0And255(Image_t *ImagePtr);
extern void BURGERCALL ImageRepaletteIndexed(Image_t *ImagePtr,const Word8 *PalettePtr);
extern void BURGERCALL ImageRemapIndexed(Image_t *ImagePtr,const Word8 *RemapPtr);
extern void BURGERCALL ImageSwapBGRToRGB(Image_t *ImagePtr);
extern Word BURGERCALL ImageValidateToSave(Image_t *ImagePtr);
extern void BURGERCALL ImageEncodeLBM(FILE *fp,const Word8 *SrcPtr,Word Length);
extern void BURGERCALL ImageEncodePCX(FILE *fp,const Word8 *SrcPtr,Word Length);
extern Word BURGERCALL PSDImageParse(PSDImage_t* Image, const Word8* InStream);
extern void BURGERCALL PSDImageDestroy(PSDImage_t* Input);

/* Random number generator */

typedef struct Random_t {
	Word Seed;		/* Random number seed */
	Word Index1;	/* First lookup index */
	Word Index2;	/* Second lookup index */
	Word RndArray[17];	/* Array of seed values (Polynomial) */
} Random_t;

extern Random_t RndMain;		/* Main random number instance */
extern Random_t RndAux;			/* Aux random number instance */
extern Random_t * BURGERCALL RndNew(Word NewSeed);
#define RndDelete(Input) DeallocAPointer(Input)
extern void BURGERCALL RndRandomize(Random_t *Input);
extern void BURGERCALL RndHardwareRandomize(Random_t *Input);
extern Word BURGERCALL RndGetRandom(Random_t *Input,Word Range);
extern void BURGERCALL RndSetRandomSeed(Random_t *Input,Word Seed);
extern int BURGERCALL RndGetRandomSigned(Random_t *Input,Word Range);
#define Randomize() RndRandomize(&RndMain)
#define HardwareRandomize() RndHardwareRandomize(&RndMain)
#define GetRandom(Range) RndGetRandom(&RndMain,Range)
#define SetRandomSeed(Seed) RndSetRandomSeed(&RndMain,Seed)
#define GetRandomSigned(Range) RndGetRandomSigned(&RndMain,Range)
#define AuxRandomize() RndRandomize(&RndAux)
#define AuxHardwareRandomize() RndHardwareRandomize(&RndAux)
#define AuxGetRandom(Range) RndGetRandom(&RndAux,Range)
#define AuxSetRandomSeed(Seed) RndSetRandomSeed(&RndAux,Seed)
#define AuxGetRandomSigned(Range) RndGetRandomSigned(&RndAux,Range)

/* File manager */

#define FULLPATHSIZE 2048	/* Maximum pathname size for Burgerlib */
#define PREFIXMAX 35		/* Maximum prefixs */
#define PREFIXBOOT 32		/* * prefix */
#define PREFIXPREFS 33		/* @ prefix */
#define PREFIXSYSTEM 34		/* $ prefix */

typedef struct DirectorySearch_t {
	Word32 FileSize;	/* Size of the file */
	TimeDate_t Create;	/* Creation time */
	TimeDate_t Modify;	/* Modification time */
	Bool Dir;		/* True if this is a directory */
	Bool System;		/* True if this is a system file */
	Bool Hidden;		/* True if this file is hidden */
	Bool Locked;		/* True if this file is read only */
	char Name[256];		/* Filename */
#if defined(__MAC__)
	Word Index;			/* Directory index */
	short VRefNum;		/* Volume ID */
	short Padding1;
	long DirID;			/* Directory to scan */
	Word32 FileType;	/* File's type */
	Word32 AuxType;	/* File's creator code */
#elif defined(__MACOSX__)
	void *Enumerator;	/* Filename enumerator */
	char *PathPrefix;	/* Pathname buffer */
	char *PathEnd;		/* Pathname ending */
#elif defined(__MSDOS__)
	Word HandleOk;		/* Handle is valid */
	short FileHandle;	/* Handle to the open directory */
	char MyFindT[44];	/* Dos FindT structure */
#elif defined(__WIN32__)
	void *FindHandle;	/* Win95 file handle */
	Word HandleOk;		/* Handle is valid */
	char MyFindT[320];	/* Win95 FindT structure */
#endif
} DirectorySearch_t;

extern void ** PrefixHandles[PREFIXMAX];
extern Word BURGERCALL GetFileModTime(const char *FileName,struct TimeDate_t *Output);
extern Word BURGERCALL GetFileModTimeNative(const char *FileName,struct TimeDate_t *Output);
extern Word BURGERCALL GetFileCreateTime(const char *FileName,struct TimeDate_t *Output);
extern Word BURGERCALL GetFileCreateTimeNative(const char *FileName,struct TimeDate_t *Output);
extern Word BURGERCALL DoesFileExist(const char *FileName);
extern Word BURGERCALL DoesFileExistNative(const char *FileName);
extern int BURGERCALL CompareTimeDates(const struct TimeDate_t *First,const struct TimeDate_t *Second);
extern Word BURGERCALL CreateDirectoryPath(const char *FileName);
extern Word BURGERCALL CreateDirectoryPath2(const char *FileName);
extern Word BURGERCALL CreateDirectoryPathNative(const char *FileName);
extern Word BURGERCALL OpenADirectory(DirectorySearch_t *Input,const char *Name);
extern Word BURGERCALL GetADirectoryEntry(DirectorySearch_t *Input);
extern Word BURGERCALL GetADirectoryEntryExtension(DirectorySearch_t *Input,const char *ExtPtr);
extern void BURGERCALL CloseADirectory(DirectorySearch_t *Input);
extern FILE * BURGERCALL OpenAFile(const char *FileName,const char *type);
extern Word BURGERCALL CopyAFile(const char *DestFile,const char *SrcFile);
extern Word BURGERCALL CopyAFileNative(const char *DestFile,const char *SrcFile);
extern Word BURGERCALL CopyAFileFP(FILE *Destfp,FILE *Srcfp);
extern Word BURGERCALL DeleteAFile(const char *FileName);
extern Word BURGERCALL DeleteAFileNative(const char *FileName);
extern Word BURGERCALL RenameAFile(const char *NewName,const char *OldName);
extern Word BURGERCALL RenameAFileNative(const char *NewName,const char *OldName);
extern char * BURGERCALL ExpandAPath(const char *FileName);
extern char * BURGERCALL ExpandAPathNative(const char *FileName);
extern void BURGERCALL ExpandAPathToBuffer(char *BufferPtr,const char *FileName);
extern void BURGERCALL ExpandAPathToBufferNative(char *BufferPtr,const char *FileName);
extern char * BURGERCALL ConvertNativePathToPath(const char *FileName);
extern char * BURGERCALL GetAPrefix(Word PrefixNum);
extern Word BURGERCALL SetAPrefix(Word PrefixNum,const char *PrefixName);
extern void BURGERCALL SetDefaultPrefixs(void);
extern void BURGERCALL PopAPrefix(Word PrefixNum);
extern char * BURGERCALL GetAVolumeName(Word DriveNum);
extern Word BURGERCALL FindAVolumeByName(const char *VolumeName);
extern Word BURGERCALL ChangeADirectory(const char *DirName);
extern Word32 BURGERCALL fgetfilesize(FILE *fp);
extern void BURGERCALL fwritelong(Word32 Val,FILE *fp);
extern void BURGERCALL fwritelongrev(Word32 Val,FILE *fp);
extern void BURGERCALL fwriteshort(Word16 Val,FILE *fp);
extern void BURGERCALL fwriteshortrev(Word16 Val,FILE *fp);
extern void BURGERCALL fwritestr(const char *ValPtr,FILE *fp);
extern Word32 BURGERCALL fgetlong(FILE *fp);
extern Word32 BURGERCALL fgetlongrev(FILE *fp);
extern short BURGERCALL fgetshort(FILE *fp);
extern short BURGERCALL fgetshortrev(FILE *fp);
extern Word BURGERCALL fgetstr(char *Input,Word Length,FILE *fp);
extern Word BURGERCALL SaveAFile(const char *FileName,const void *DataPtr,Word32 Length);
extern Word BURGERCALL SaveAFileNative(const char *FileName,const void *DataPtr,Word32 Length);
extern Word BURGERCALL SaveAFileFP(FILE *Filefp,const void *DataPtr,Word32 Length);
extern Word BURGERCALL SaveATextFile(const char *FileName,const void *DataPtr,Word32 Length);
extern Word BURGERCALL SaveATextFileNative(const char *FileName,const void *DataPtr,Word32 Length);
extern void * BURGERCALL LoadAFile(const char *FileName,Word32 *Length);
extern void * BURGERCALL LoadAFileNative(const char *FileName,Word32 *Length);
extern void * BURGERCALL LoadAFileFP(FILE *Filefp,Word32 *Length);
extern void ** BURGERCALL LoadAFileHandle(const char *FileName);
extern void ** BURGERCALL LoadAFileHandleNative(const char *FileName);
extern void ** BURGERCALL LoadAFileHandleFP(FILE *Filefp);
extern Word32 BURGERCALL GetAnAuxType(const char *FileName);
extern Word32 BURGERCALL GetAnAuxTypeNative(const char *FileName);
extern Word32 BURGERCALL GetAFileType(const char *FileName);
extern Word32 BURGERCALL GetAFileTypeNative(const char *FileName);
extern void BURGERCALL SetAnAuxType(const char *FileName,Word32 AuxType);
extern void BURGERCALL SetAnAuxTypeNative(const char *FileName,Word32 AuxType);
extern void BURGERCALL SetAFileType(const char *FileName,Word32 FileType);
extern void BURGERCALL SetAFileTypeNative(const char *FileName,Word32 FileType);
extern void BURGERCALL FileSetFileAndAuxType(const char *FileName,Word32 FileType,Word32 AuxType);
extern void BURGERCALL FileSetFileAndAuxTypeNative(const char *FileName,Word32 FileType,Word32 AuxType);
extern Word BURGERCALL AreLongFilenamesAllowed(void);

#if defined(__LITTLEENDIAN__)
#define fwritelongb(x,y) fwritelongrev(x,y)
#define fwritelongl(x,y) fwritelong(x,y)
#define fwriteshortb(x,y) fwriteshortrev(x,y)
#define fwriteshortl(x,y) fwriteshort(x,y)
#define fgetlongb(x) fgetlongrev(x)
#define fgetlongl(x) fgetlong(x)
#define fgetshortb(x) fgetshortrev(x)
#define fgetshortl(x) fgetshort(x)
#else
#define fwritelongb(x,y) fwritelong(x,y)
#define fwritelongl(x,y) fwritelongrev(x,y)
#define fwriteshortb(x,y) fwriteshort(x,y)
#define fwriteshortl(x,y) fwriteshortrev(x,y)
#define fgetlongb(x) fgetlong(x)
#define fgetlongl(x) fgetlongrev(x)
#define fgetshortb(x) fgetshort(x)
#define fgetshortl(x) fgetshortrev(x)
#endif

/* Prefs manager */

typedef enum {PREFREAD,PREFWRITE,PREFDEFAULT} PrefState_e;

typedef char * (BURGERCALL *PrefsRecordProc_t)(char *,struct PrefsRecord_t *,PrefState_e);	/* Callback proc for readers */

typedef struct IniIndex_t {	/* Win95 style prefs file image header struct */
	const char *Header;		/* Header to scan for image data */
	const char *ImagePtr;	/* Pointer to the image in memory */
	Word32 ImageLength;	/* Length of the image in memory */
} IniIndex_t;

typedef struct PrefsRecord_t {	/* A single search record */
	char *EntryName;	/* Ascii of entry */
	PrefsRecordProc_t Proc;	/* Callback to process the data */
	void *DataPtr;		/* Pointer to data to store */
	void *Default;		/* Default data pointer (Or possible value) */
	Word Count;			/* Size in elements of buffer */
} PrefsRecord_t;

typedef struct PrefsTemplate_t {	/* Prefs file description record */
	char *Header;		/* Ascii for the header */
	Word Count;			/* Number of entries to scan */
	PrefsRecord_t *ArrayPtr;	/* Array of "Count" entries */
} PrefsTemplate_t;

extern char NibbleToAscii[16];		/* 0-F in upper case */
extern char NibbleToAsciiLC[16];	/* 0-f in lower case */
#define LongWordToAscii(Input,AsciiPtr) LongWordToAscii2(Input,AsciiPtr,0)
extern char * BURGERCALL LongWordToAscii2(Word32 Input,char *AsciiPtr,Word Printing);
#define longToAscii(Input,AsciiPtr) longToAscii2(Input,AsciiPtr,0)
extern char * BURGERCALL longToAscii2(long Input,char *AsciiPtr,Word Printing);
#define AsciiToLongWord(AsciiPtr) AsciiToLongWord2(AsciiPtr,0)
extern Word32 BURGERCALL AsciiToLongWord2(const char *AsciiPtr,char **DestPtr);
#define LongWordToAsciiHex(Input,AsciiPtr) LongWordToAsciiHex2(Input,AsciiPtr,0)
extern char * BURGERCALL LongWordToAsciiHex2(Word32 Input,char *AsciiPtr,Word Printing);
extern char * BURGERCALL ParseBeyondEOL(const char *Input);
extern char * BURGERCALL ParseBeyondWhiteSpace(const char *Input);
extern char * BURGERCALL StoreAString(char *WorkPtr,const char *Input);
extern char * BURGERCALL StoreALongWordAscii(char *WorkPtr,Word32 Input);
extern char * BURGERCALL StoreALongWordAsciiHex(char *WorkPtr,Word32 Input);
extern char * BURGERCALL StoreAlongAscii(char *WorkPtr,long Input);
extern char * BURGERCALL StoreAParsedString(char *WorkPtr,const char *Input);
extern char * BURGERCALL GetAParsedString(const char *WorkPtr,char *DestPtr,Word Size);
#define PrefsWordProc PrefsLongWordProc
#define PrefsWordArrayProc PrefsLongWordArrayProc
extern char * BURGERCALL PrefsLongWordProc(char *WorkPtr,PrefsRecord_t *RecordPtr,PrefState_e Pass);
extern char * BURGERCALL PrefsShortProc(char *WorkPtr,PrefsRecord_t *RecordPtr,PrefState_e Pass);
extern char * BURGERCALL PrefsLongWordArrayProc(char *WorkPtr,PrefsRecord_t *RecordPtr,PrefState_e Pass);
extern char * BURGERCALL PrefsShortArrayProc(char *WorkPtr,PrefsRecord_t *RecordPtr,PrefState_e Pass);
extern char * BURGERCALL PrefsStringProc(char *WorkPtr,PrefsRecord_t *RecordPtr,PrefState_e Pass);
extern Word BURGERCALL ScanIniImage(IniIndex_t *IndexPtr,PrefsRecord_t *Record);
extern Word32 BURGERCALL LongWordFromIniImage(const char *Header,const char *EntryName,const char *Input,Word32 InputLength);
extern void * BURGERCALL PrefsCreateFileImage(PrefsTemplate_t *MyTemplate,Word32 *LengthPtr);
extern Word BURGERCALL PrefsWriteFile(PrefsTemplate_t *MyTemplate,const char *FileName,Word AppendFlag);
extern void BURGERCALL PrefsParseFile(PrefsTemplate_t *MyTemplate,const char *FilePtr,Word32 Length);
extern Word BURGERCALL PrefsReadFile(PrefsTemplate_t *MyTemplate,const char *FileName);

/* Pref File Manager */

typedef struct PrefFile_t PrefFile_t;
typedef struct PrefFileSection_t PrefFileSection_t;
typedef struct PrefFileEntry_t PrefFileEntry_t;

extern PrefFile_t * BURGERCALL PrefFileNew(void);
extern PrefFile_t * BURGERCALL PrefFileNewFromMemory(const char *Data,Word32 Length);
extern PrefFile_t * BURGERCALL PrefFileNewFromFile(const char *FileName);
extern PrefFile_t * BURGERCALL PrefFileNewFromFileAlways(const char *FileName);
extern void BURGERCALL PrefFileDelete(PrefFile_t *Input);
extern Word BURGERCALL PrefFileSaveFile(PrefFile_t *Input,const char *FileName);
extern PrefFileSection_t * BURGERCALL PrefFileFindSection(PrefFile_t *Input,const char *SectionName);
extern PrefFileSection_t * BURGERCALL PrefFileFindSectionAlways(PrefFile_t *Input,const char *SectionName);
extern char *BURGERCALL PrefFileGetList(PrefFile_t *Input);
extern PrefFileEntry_t * BURGERCALL PrefFileSectionFindEntry(PrefFileSection_t *Input,const char *EntryName);
extern void BURGERCALL PrefFileDeleteSection(PrefFile_t *Input,const char *SectionName);
extern void BURGERCALL PrefFileDeletePrefFileSection(PrefFile_t *Input,PrefFileSection_t *SectionPtr);
extern PrefFileSection_t * BURGERCALL PrefFileAddSection(PrefFile_t *Input,const char *SectionName);
extern Word BURGERCALL PrefFileIsEntryPresent(PrefFile_t *Input,const char *SectionName,const char *EntryName);
extern char *BURGERCALL PrefFileSectionGetList(PrefFileSection_t *Input);
extern char *BURGERCALL PrefFileSectionGetRaw(PrefFileSection_t *Input,const char *EntryName);
extern Word BURGERCALL PrefFileSectionGetBoolean(PrefFileSection_t *Input,const char *EntryName,Word Default);
extern Word BURGERCALL PrefFileSectionGetWord(PrefFileSection_t *Input,const char *EntryName,Word Default,Word Min,Word Max);
extern int BURGERCALL PrefFileSectionGetInt(PrefFileSection_t *Input,const char *EntryName,int Default,int Min,int Max);
extern float BURGERCALL PrefFileSectionGetFloat(PrefFileSection_t *Input,const char *EntryName,float Default,float Min,float Max);
extern double BURGERCALL PrefFileSectionGetDouble(PrefFileSection_t *Input,const char *EntryName,double Default,double Min,double Max);
extern void BURGERCALL PrefFileSectionGetString(PrefFileSection_t *Input,const char *EntryName,const char *Default,char *Buffer,Word BufferSize);
extern void BURGERCALL PrefFileSectionGetDualString(PrefFileSection_t *Input,const char *EntryName,const char *Default,char *Buffer,Word BufferSize,const char *Default2,char *Buffer2,Word BufferSize2);
extern void BURGERCALL PrefFileSectionGetMem(PrefFileSection_t *Input,const char *EntryName,const Word8 *Default,Word8 *Buffer,Word BufferSize);
extern void BURGERCALL PrefFileSectionGetWordArray(PrefFileSection_t *Input,const char *EntryName,const Word *Default,Word *Buffer,Word Count);
extern void BURGERCALL PrefFileSectionAddEntry(PrefFileSection_t *Input,const char *EntryName,const char *Default);
extern void BURGERCALL PrefFileSectionPutRaw(PrefFileSection_t *Input,const char *EntryName,const char *RawString);
extern void BURGERCALL PrefFileSectionPutBoolean(PrefFileSection_t *Input,const char *EntryName,Word Data);
extern void BURGERCALL PrefFileSectionPutWord(PrefFileSection_t *Input,const char *EntryName,Word Data);
extern void BURGERCALL PrefFileSectionPutWordHex(PrefFileSection_t *Input,const char *EntryName,Word Data);
extern void BURGERCALL PrefFileSectionPutInt(PrefFileSection_t *Input,const char *EntryName,int Data);
extern void BURGERCALL PrefFileSectionPutFloat(PrefFileSection_t *Input,const char *EntryName,float Data);
extern void BURGERCALL PrefFileSectionPutDouble(PrefFileSection_t *Input,const char *EntryName,double Data);
extern void BURGERCALL PrefFileSectionPutString(PrefFileSection_t *Input,const char *EntryName,const char *Data);
extern void BURGERCALL PrefFileSectionPutDualString(PrefFileSection_t *Input,const char *EntryName,const char *Data,const char *Data2);
extern void BURGERCALL PrefFileSectionPutMem(PrefFileSection_t *Input,const char *EntryName,const Word8 *Data,Word Length);
extern void BURGERCALL PrefFileSectionPutWordArray(PrefFileSection_t *Input,const char *EntryName,const Word *DataPtr,Word Count);

/* Memory streaming routines */

typedef struct StreamHandle_t {
	Word8 **DataHandle;	/* Handle of the buffer for streaming */
	Word32 Mark;		/* Current file mark */
	Word32 BufferSize;	/* Maximum memory in buffer */
	Word32 EOFMark;	/* Size of the valid data */
	Word ErrorFlag;		/* Set to TRUE if an error occured */
} StreamHandle_t;

extern StreamHandle_t * BURGERCALL StreamHandleNewPut(void);
extern StreamHandle_t * BURGERCALL StreamHandleNewGet(void **GetHandle);
extern StreamHandle_t * BURGERCALL StreamHandleNewGetPtr(void *GetPtr,Word32 Size);
extern StreamHandle_t * BURGERCALL StreamHandleNewGetFile(const char *FileName);
extern void BURGERCALL StreamHandleInitPut(StreamHandle_t *Input);
extern void BURGERCALL StreamHandleInitGet(StreamHandle_t *Input,void **GetHandle);
extern void BURGERCALL StreamHandleInitGetPtr(StreamHandle_t *Input,void *GetPtr,Word32 Size);
extern Word BURGERCALL StreamHandleInitGetFile(StreamHandle_t *Input,const char *FileName);
extern void BURGERCALL StreamHandleDestroy(StreamHandle_t *Input);
extern void BURGERCALL StreamHandleDelete(StreamHandle_t *Input);
extern void ** BURGERCALL StreamHandleDetachHandle(StreamHandle_t *Input);
extern void BURGERCALL StreamHandleEndSave(StreamHandle_t *Input);
extern Word BURGERCALL StreamHandleSaveFile(StreamHandle_t *Input,const char *FileName);
extern Word BURGERCALL StreamHandleSaveTextFile(StreamHandle_t *Input,const char *FileName);
extern Word BURGERCALL StreamHandleGetByte(StreamHandle_t *Input);
extern Word BURGERCALL StreamHandleGetShort(StreamHandle_t *Input);
extern Word32 BURGERCALL StreamHandleGetLong(StreamHandle_t *Input);
extern float BURGERCALL StreamHandleGetFloat(StreamHandle_t *Input);
extern double BURGERCALL StreamHandleGetDouble(StreamHandle_t *Input);
extern void BURGERCALL StreamHandleGetVector2D(StreamHandle_t *Input,struct Vector2D_t *Output);
extern void BURGERCALL StreamHandleGetVector3D(StreamHandle_t *Input,struct Vector3D_t *Output);
extern void BURGERCALL StreamHandleGetMatrix3D(StreamHandle_t *Input,struct Matrix3D_t *Output);
extern void BURGERCALL StreamHandleGetEuler(StreamHandle_t *Input,struct Euler_t *Output);
extern void BURGERCALL StreamHandleGetLWRect(StreamHandle_t *Input,struct LWRect_t *Output);
extern void BURGERCALL StreamHandleGetLWPoint(StreamHandle_t *Input,struct LWPoint_t *Output);
extern Word BURGERCALL StreamHandleGetShortMoto(StreamHandle_t *Input);
extern Word32 BURGERCALL StreamHandleGetLongMoto(StreamHandle_t *Input);
extern float BURGERCALL StreamHandleGetFloatMoto(StreamHandle_t *Input);
extern double BURGERCALL StreamHandleGetDoubleMoto(StreamHandle_t *Input);
extern void BURGERCALL StreamHandleGetVector2DMoto(StreamHandle_t *Input,struct Vector2D_t *Output);
extern void BURGERCALL StreamHandleGetVector3DMoto(StreamHandle_t *Input,struct Vector3D_t *Output);
extern void BURGERCALL StreamHandleGetMatrix3DMoto(StreamHandle_t *Input,struct Matrix3D_t *Output);
extern void BURGERCALL StreamHandleGetEulerMoto(StreamHandle_t *Input,struct Euler_t *Output);
extern void BURGERCALL StreamHandleGetMem(StreamHandle_t *Input,void *DestPtr,Word32 Length);
extern void * BURGERCALL StreamHandleGetString(StreamHandle_t *Input,Word Flags);
extern void BURGERCALL StreamHandleGetString2(StreamHandle_t *Input,Word Flags,char *DestBuffer,Word32 MaxLength);
extern void BURGERCALL StreamHandlePutByte(StreamHandle_t *Input,Word Val);
extern void BURGERCALL StreamHandlePutShort(StreamHandle_t *Input,Word Val);
extern void BURGERCALL StreamHandlePutLong(StreamHandle_t *Input,Word32 Val);
extern void BURGERCALL StreamHandlePutFloat(StreamHandle_t *Input,float Val);
extern void BURGERCALL StreamHandlePutDouble(StreamHandle_t *Input,double Val);
extern void BURGERCALL StreamHandlePutVector2D(StreamHandle_t *Input,const struct Vector2D_t *Val);
extern void BURGERCALL StreamHandlePutVector3D(StreamHandle_t *Input,const struct Vector3D_t *Val);
extern void BURGERCALL StreamHandlePutMatrix3D(StreamHandle_t *Input,const struct Matrix3D_t *Val);
extern void BURGERCALL StreamHandlePutEuler(StreamHandle_t *Input,const struct Euler_t *Val);
extern void BURGERCALL StreamHandlePutShortMoto(StreamHandle_t *Input,Word Val);
extern void BURGERCALL StreamHandlePutLongMoto(StreamHandle_t *Input,Word32 Val);
extern void BURGERCALL StreamHandlePutFloatMoto(StreamHandle_t *Input,float Val);
extern void BURGERCALL StreamHandlePutDoubleMoto(StreamHandle_t *Input,double Val);
extern void BURGERCALL StreamHandlePutVector2DMoto(StreamHandle_t *Input,const struct Vector2D_t *Val);
extern void BURGERCALL StreamHandlePutVector3DMoto(StreamHandle_t *Input,const struct Vector3D_t *Val);
extern void BURGERCALL StreamHandlePutMatrix3DMoto(StreamHandle_t *Input,const struct Matrix3D_t *Val);
extern void BURGERCALL StreamHandlePutEulerMoto(StreamHandle_t *Input,const struct Euler_t *Val);
extern void BURGERCALL StreamHandlePutMem(StreamHandle_t *Input,const void *SrcPtr,Word32 Length);
extern void BURGERCALL StreamHandlePutString(StreamHandle_t *Input,const void *SrcPtr);
extern void BURGERCALL StreamHandlePutStringNoZero(StreamHandle_t *Input,const void *SrcPtr);
#define StreamHandleGetMark(x) (x)->Mark
extern void BURGERCALL StreamHandleSetMark(StreamHandle_t *Input,Word32 NewMark);
#define StreamHandleGetSize(x) (x)->EOFMark
extern void BURGERCALL StreamHandleSetSize(StreamHandle_t *Input,Word32 Size);
#define StreamHandleGetErrorFlag(x) (x)->ErrorFlag
#define StreamHandleClearErrorFlag(x) (x)->ErrorFlag=FALSE
#define StreamHandleSetErrorFlag(x) (x)->ErrorFlag=TRUE
extern void BURGERCALL StreamHandleSkip(StreamHandle_t *Input,long Offset);
extern void BURGERCALL StreamHandleSkipString(StreamHandle_t *Input);
extern void * BURGERCALL StreamHandleLock(StreamHandle_t *Input);
extern void * BURGERCALL StreamHandleLockExpand(StreamHandle_t *Input,Word32 Size);
extern void BURGERCALL StreamHandleUnlock(StreamHandle_t *Input,const void *EndPtr);
extern void BURGERCALL StreamHandleExpand(StreamHandle_t *Input,Word32 Size);

/* "C" convenience routines */

#define SWITCH_NORMAL 0x0
#define SWITCH_CALLBACK 0x1
#define SWITCH_WORD 0x2

#define ASCIILEADINGZEROS 0x8000U
#define ASCIINONULL 0x4000U

#define EXTRACTSTRDELIMITLF 0x01
#define EXTRACTSTRNOTRANSLATE 0x02
#define EXTRACTSTRHANDLE 0x04

typedef void (BURGERCALL *SwitchCallBackProc)(char *Input);

typedef struct Switch_t {	/* Used by Switches */
	char *StrPtr;	/* Ascii to check */
	Word32 Value;	/* Preset value to use (Or parm count) */
	void *ValuePtr;	/* Pointer to Word32 or function callback */
	Word Flags;		/* Flags for handler */
} Switch_t;

typedef struct MD2_t {	/* MD2 context */
	Word8 state[16];		/* state */
	Word8 checksum[16];	/* checksum */
	Word8 buffer[16];	/* input buffer */
	Word count;			/* number of bytes, modulo 16 */
} MD2_t;

typedef struct MD4_t {	/* MD4 context. */
	Word32 state[4];	/* Current 128 bit value */
	Word32 countlo;	/* Number of bytes processed (64 bit value) */
	Word32 counthi;
	Word8 buffer[64];	/* input buffer for processing */
} MD4_t;

typedef struct MD5_t {	/* MD5 context. */
	Word32 state[4];	/* Current 128 bit value */
	Word32 countlo;	/* Number of bytes processed (64 bit value) */
	Word32 counthi;
	Word8 buffer[64];	/* input buffer for processing */
} MD5_t;

extern Word8 ReverseBits[256];		/* Table to reverse bits in a byte */
extern Word BURGERCALL Switches(Word argc,char *argv[],const Switch_t *SwitchList);
extern char * BURGERCALL midstr(char *Dest,const char *Source,Word Start,Word Length);
extern char * BURGERCALL stristr(const char *Input, const char *SubStr);
extern void BURGERCALL CStr2PStr(char *DestPtr,const char *SrcPtr);
extern void BURGERCALL PStr2CStr(char *DestPtr,const char *SrcPtr);
extern Word32 BURGERCALL BCDToNum(Word32 Input);
extern Word32 BURGERCALL NumToBCD(Word32 Input);
extern Word32 BURGERCALL PowerOf2(Word32 Input);
extern Word32 BURGERCALL CalcMoreCRC32(const Word8 *buffPtr,Word32 buffSize,Word32 crc);
#define CalcCRC32(buffPtr,buffSize) CalcMoreCRC32(buffPtr,buffSize,0)
extern Word32 BURGERCALL CalcMoreAdler(const Word8 *Buffer,Word32 Length,Word32 crc);
#define CalcAdler(buffPtr,buffSize) CalcMoreAdler(buffPtr,buffSize,1)
extern Word BURGERCALL CalcMoreAdler16(const Word8 *Buffer,Word32 Length,Word CheckSum);
#define CalcAdler16(buffPtr,buffSize) CalcMoreAdler16(buffPtr,buffSize,1)
extern void BURGERCALL MD2Init(MD2_t *Input);
extern void BURGERCALL MD2Update(MD2_t *Input,const Word8 *BufferPtr,Word32 Length);
extern void BURGERCALL MD2Final(Word8 *Output,MD2_t *Input);
extern void BURGERCALL MD2Quick(Word8 *Output,const Word8 *BufferPtr,Word32 Length);
extern void BURGERCALL MD4Init(MD4_t *Input);
extern void BURGERCALL MD4Update(MD4_t *Input,const Word8 *BufferPtr,Word32 Length);
extern void BURGERCALL MD4Final(Word8 *Output,MD4_t *Input);
extern void BURGERCALL MD4Quick(Word8 *Output,const Word8 *BufferPtr,Word32 Length);
extern void BURGERCALL MD5Init(MD5_t *Input);
extern void BURGERCALL MD5Update(MD5_t *Input,const Word8 *BufferPtr,Word32 Length);
extern void BURGERCALL MD5Final(Word8 *Output,MD5_t *Input);
extern void BURGERCALL MD5Quick(Word8 *Output,const Word8 *BufferPtr,Word32 Length);
extern void * BURGERCALL ExtractAString(const char *SrcPtr,Word32 *BufSize,Word Flags);
extern void BURGERCALL ExtractAString2(const char *SrcPtr,Word32 *BufSize,Word Flags,char *DestPtr,Word32 DestSize);
extern void BURGERCALL FastMemCpy(void *DestPtr,const void *SrcPtr,Word32 Length);
#define FastMemCpyAlign(Dest,Src,Length) memcpy(Dest,Src,Length)
extern void BURGERCALL FastMemSet(void *DestPtr,Word Fill,Word32 Length);
extern void BURGERCALL FastMemSet16(void *DestPtr,Word Fill,Word32 Length);
extern Word BURGERCALL FastStrLen(const char *Input);
extern int BURGERCALL FastStrncmp(const char *Input1,const char *Input2,Word MaxLength);
inline void MemZero(Word8 *DestPtr,Word32 Size) { FastMemSet((void *)DestPtr,0,Size); }

#if defined(__MWERKS__) || defined(__MRC__) || defined(__BEOS__)
extern int BURGERCALL memicmp(const char *StrPtr1,const char *StrPtr2,Word Length);
#if !defined(_MSL_NEEDS_EXTRAS)
extern char * BURGERCALL strupr(char *Source);
extern char * BURGERCALL strlwr(char *Source);
extern char * BURGERCALL strdup(const char *source);
extern int BURGERCALL stricmp(const char *StrPtr1,const char *StrPtr2);
extern int BURGERCALL strnicmp(const char *StrPtr1,const char *StrPtr2,Word Len);
#endif
#endif

/* The point of DebugString is that it goes away in release code */

extern void BURGERCALL DebugXString(const char *String);
extern void BURGERCALL DebugXshort(short i);
extern void BURGERCALL DebugXShort(Word16 i);
extern void BURGERCALL DebugXlong(long i);
extern void BURGERCALL DebugXLongWord(Word32 i);
extern void BURGERCALL DebugXDouble(double i);
extern void BURGERCALL DebugXPointer(const void *i);
extern void ANSICALL DebugXMessage(const char *String,...);
#define DebugXWord(x) DebugXLongWord(x)

#if _DEBUG			/* Allow in debug code */
#define DebugString(x) DebugXString(x)
#define Debugshort(x) DebugXshort(x)
#define DebugShort(x) DebugXShort(x)
#define Debuglong(x) DebugXlong(x)
#define DebugLongWord(x) DebugXLongWord(x)
#define DebugDouble(x) DebugXDouble(x)
#define DebugPointer(x) DebugXPointer(x)
#define DebugWord(x) DebugXLongWord((Word32)x)
#define DebugMessage DebugXMessage
#else
#define DebugString(x)		/* Remove from release code */
#define Debugshort(x)
#define DebugShort(x)
#define Debuglong(x)
#define DebugLongWord(x)
#define DebugDouble(x)
#define DebugPointer(x)
#define DebugMessage 1 ? (void)0 : DebugXMessage
#define DebugWord(x)
#endif

#define DEBUGTRACE_MEMORYLEAKS 1 /* Test and display memory leaks */
#define DEBUGTRACE_REZLOAD 2	/* Print the name of a resource file being loaded */
#define DEBUGTRACE_FILELOAD 4	/* Print the name of a file being loaded */
#define DEBUGTRACE_WARNINGS 8	/* Print possible errors */
#define DEBUGTRACE_NETWORK 0x10	/* Network commands */
#define DEBUGTRACE_THEWORKS 0x1F	/* GIMME everything! */

typedef Word (BURGERCALL *SystemProcessCallBackProc)(const char *Input);

extern Word DebugTraceFlag;	/* Set these flag for debug spew */
extern Word BombFlag;		/* If true then bomb on ANY error */
extern Word IAmExiting;		/* TRUE if in a shutdown state */
extern char ErrorMsg[512];		/* Last message printed */
extern void ANSICALL Fatal(const char *FatalMsg,...);
extern void ANSICALL NonFatal(const char *ErrorMsg,...);
extern Word BURGERCALL SetErrBombFlag(Word Flag);
extern void BURGERCALL Halt(void);
extern void BURGERCALL SaveJunk(const void *Data,Word32 Length);
extern void BURGERCALL OkAlertMessage(const char *Title,const char *Message);
extern Word BURGERCALL OkCancelAlertMessage(const char *Title,const char *Message);
extern Word BURGERCALL SystemProcessFilenames(SystemProcessCallBackProc Proc);
extern Word BURGERCALL GetQuickTimeVersion(void);
extern Word BURGERCALL LaunchURL(const char *URLPtr);
extern Word BURGERCALL BurgerlibVersion(void);

typedef struct LibRef_t LibRef_t;
extern LibRef_t * BURGERCALL LibRefInit(const char *LibName);
extern void BURGERCALL LibRefDelete(LibRef_t *LibRef);
extern void * BURGERCALL LibRefGetProc(LibRef_t *LibRef,const char *ProcName);
extern void *BURGERCALL LibRefGetFunctionInLib(const char *LibName,const char *ProcName);

extern void BURGERCALL PrintHexDigit(Word Val);
extern void BURGERCALL PrintHexByte(Word Val);
extern void BURGERCALL PrintHexShort(Word Val);
extern void BURGERCALL PrintHexLongWord(Word32 Val);

#if defined(__INTEL__)
#define FastMemCpy(x,y,z) memcpy(x,y,z)
#endif

#undef strlen			/* For some compilers, this is intrinsic */
#define strlen(x) FastStrLen(x)

#ifdef __POWERPC__
#define strncmp(x,y,z) FastStrncmp(x,y,z)
#endif

/* Intel convience routines */

#if defined(__INTEL__)

typedef enum {
	IntelFP24=0,
	IntelFP56=2,
	IntelFP64=3
} IntelFP_e;

extern IntelFP_e BURGERCALL IntelSetFPPrecision(IntelFP_e Input);

#endif

/* String Handlers */

typedef struct LWString_t {
	char *DataPtr;			/* Pointer to actual string */
	Word StrLength;			/* Length of the string */
	Word BufferLength;		/* Length of the string buffer */
	char Bogus;				/* Bogus string for no memory (Failsafe) */
	char Pad1,Pad2,Pad3;	/* Make sure I am long word aligned */
} LWString_t;

extern void BURGERCALL StrStripLeadingSpaces(char* Input);
extern void BURGERCALL StrStripLeadingWhiteSpace(char* Input);
extern void BURGERCALL StrStripTrailingSpaces(char* Input);
extern void BURGERCALL StrStripTrailingWhiteSpace(char* Input);
extern void BURGERCALL StrStripLeadingAndTrailingSpaces(char* Input);
extern void BURGERCALL StrStripLeadingAndTrailingWhiteSpace(char* Input);
extern void BURGERCALL StrStripAllBut(char* Input,const char* ListPtr);
extern void BURGERCALL StrStripAll(char* Input,const char* ListPtr);
extern void BURGERCALL StrStripTrailing(char* Input, const char* ListPtr);
extern void BURGERCALL StrStripLeading(char* Input, const char* ListPtr);
extern char * BURGERCALL StrParseToDelimiter(const char *Input);
extern void BURGERCALL StrParse(struct LinkedList_t *ListPtr,const char *Input);
extern void BURGERCALL StrGetComputerName(char* Output,Word OutputSize);
extern void BURGERCALL StrGetUserName(char* Output,Word OutputSize);
extern char* BURGERCALL StrGetFileExtension(const char *Input);
extern void BURGERCALL StrSetFileExtension(char* Input,const char* NewExtension);
extern char* BURGERCALL StrCopy(const char *Input);
extern char* BURGERCALL StrCopyPad(const char *Input,Word Padding);
extern void ** BURGERCALL StrCopyHandle(const char *Input);
extern void ** BURGERCALL StrCopyPadHandle(const char *Input,Word Padding);
extern char* BURGERCALL DebugStrCopy(const char *Input,const char *File,Word Line);
extern char* BURGERCALL DebugStrCopyPad(const char *Input,Word Padding,const char *File,Word Line);
extern void ** BURGERCALL DebugStrCopyHandle(const char *Input,const char *File,Word Line);
extern void ** BURGERCALL DebugStrCopyPadHandle(const char *Input,Word Padding,const char *File,Word Line);

#if _DEBUG
#define StrCopy(x) DebugStrCopy(x,__FILE__,__LINE__)
#define StrCopyPad(x,y) DebugStrCopyPad(x,y,__FILE__,__LINE__)
#define StrCopyHandle(x) DebugStrCopyHandle(x,__FILE__,__LINE__)
#define StrCopyPadHandle(x,y) DebugStrCopyPadHandle(x,y,__FILE__,__LINE__)
#endif

/* Linked list handler */

struct LinkedListEntry_t;
typedef void (BURGERCALL *LinkedListDeleteProcPtr)(struct LinkedListEntry_t *);
typedef Word (BURGERCALL *LinkedListTraverseProcPtr)(void *);

typedef struct LinkedListEntry_t {
	struct LinkedListEntry_t *Next;		/* Pointer to the next entry */
	struct LinkedListEntry_t *Prev;		/* Pointer to the previous entry */
	void *Data;							/* Pointer to the data */
	LinkedListDeleteProcPtr KillProc;	/* Destructor */
} LinkedListEntry_t;

typedef struct LinkedList_t {
	LinkedListEntry_t *First;			/* First entry in the list */
	LinkedListEntry_t *Last;			/* Last entry in the list */
	Word Count;							/* Number of entries in the list */
} LinkedList_t;

#define LINKLIST_ABORT 1
#define LINKLIST_DELETE 2

typedef int (BURGERCALL *LinkedListSortProc)(const void *,const void *);

extern void BURGERCALL LinkedListEntryDeallocProc(LinkedListEntry_t *Input);
extern void BURGERCALL LinkedListEntryDeallocNull(LinkedListEntry_t *Input);
extern void BURGERCALL LinkedListInit(LinkedList_t *Input);
extern void BURGERCALL LinkedListDestroy(LinkedList_t *Input);
extern LinkedList_t * BURGERCALL LinkedListNew(void);
extern void BURGERCALL LinkedListDelete(LinkedList_t *Input);
extern void BURGERCALL LinkedListDeleteFirstEntry(LinkedList_t *Input);
extern void BURGERCALL LinkedListDeleteLastEntry(LinkedList_t *Input);
extern void BURGERCALL LinkedListDeleteEntryByData(LinkedList_t *Input,const void *DataPtr);
extern void BURGERCALL LinkedListDeleteEntry(LinkedList_t *Input,LinkedListEntry_t *EntryPtr);
extern void BURGERCALL LinkedListRemoveEntry(LinkedList_t *Input,LinkedListEntry_t *EntryPtr);
extern Word BURGERCALL LinkedListContains(const LinkedList_t *Input,const void *Data);
extern LinkedListEntry_t * BURGERCALL LinkedListGetEntry(const LinkedList_t *Input,Word EntryNum);
extern LinkedListEntry_t * BURGERCALL LinkedListGetEntryByData(const LinkedList_t *Input,const void *DataPtr);
extern void * BURGERCALL LinkedListGetEntryData(const LinkedList_t *Input,Word EntryNum);
extern Word BURGERCALL LinkedListFindString(const LinkedList_t *Input,const char *TextPtr);
extern LinkedListEntry_t * BURGERCALL LinkedListFindStringEntry(const LinkedList_t *Input,const char *TextPtr);
extern LinkedListEntry_t *BURGERCALL LinkedListTraverseForward(LinkedList_t *Input,LinkedListTraverseProcPtr Proc);
extern LinkedListEntry_t *BURGERCALL LinkedListTraverseBackward(LinkedList_t *Input,LinkedListTraverseProcPtr Proc);
extern void BURGERCALL LinkedListSort(LinkedList_t *Input,LinkedListSortProc Proc);
extern void BURGERCALL LinkedListAddEntryEnd(LinkedList_t *Input,LinkedListEntry_t *EntryPtr);
extern void BURGERCALL LinkedListAddEntryBegin(LinkedList_t *Input,LinkedListEntry_t *EntryPtr);
extern void BURGERCALL LinkedListAddEntryAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,LinkedListEntry_t *NewPtr);
extern void BURGERCALL LinkedListAddEntryBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,LinkedListEntry_t *NewPtr);
extern void BURGERCALL LinkedListAddNewEntryEnd(LinkedList_t *Input,void *Data);
extern void BURGERCALL LinkedListAddNewEntryBegin(LinkedList_t *Input,void *Data);
extern void BURGERCALL LinkedListAddNewEntryAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data);
extern void BURGERCALL LinkedListAddNewEntryBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data);
extern void BURGERCALL LinkedListAddNewEntryProcEnd(LinkedList_t *Input,void *Data,LinkedListDeleteProcPtr Kill);
extern void BURGERCALL LinkedListAddNewEntryProcBegin(LinkedList_t *Input,void *Data,LinkedListDeleteProcPtr Kill);
extern void BURGERCALL LinkedListAddNewEntryProcAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,LinkedListDeleteProcPtr Kill);
extern void BURGERCALL LinkedListAddNewEntryProcBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,LinkedListDeleteProcPtr Kill);
extern void BURGERCALL LinkedListAddNewEntryMemEnd(LinkedList_t *Input,void *Data);
extern void BURGERCALL LinkedListAddNewEntryMemBegin(LinkedList_t *Input,void *Data);
extern void BURGERCALL LinkedListAddNewEntryMemAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data);
extern void BURGERCALL LinkedListAddNewEntryMemBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data);
extern void BURGERCALL LinkedListAddNewEntryStringEnd(LinkedList_t *Input,const char *Data);
extern void BURGERCALL LinkedListAddNewEntryStringBegin(LinkedList_t *Input,const char *Data);
extern void BURGERCALL LinkedListAddNewEntryStringAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,const char *Data);
extern void BURGERCALL LinkedListAddNewEntryStringBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,const char *Data);
extern void BURGERCALL DebugLinkedListAddNewEntryEnd(LinkedList_t *Input,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryBegin(LinkedList_t *Input,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryProcEnd(LinkedList_t *Input,void *Data,LinkedListDeleteProcPtr Kill,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryProcBegin(LinkedList_t *Input,void *Data,LinkedListDeleteProcPtr Kill,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryProcAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,LinkedListDeleteProcPtr Kill,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryProcBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,LinkedListDeleteProcPtr Kill,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryMemEnd(LinkedList_t *Input,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryMemBegin(LinkedList_t *Input,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryMemAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryMemBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,void *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryStringEnd(LinkedList_t *Input,const char *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryStringBegin(LinkedList_t *Input,const char *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryStringAfter(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,const char *Data,const char *File,Word Line);
extern void BURGERCALL DebugLinkedListAddNewEntryStringBefore(LinkedList_t *Input,LinkedListEntry_t *EntryPtr,const char *Data,const char *File,Word Line);

#if _DEBUG
#define LinkedListAddNewEntryEnd(x,y) DebugLinkedListAddNewEntryEnd(x,y,__FILE__,__LINE__)
#define LinkedListAddNewEntryBegin(x,y) DebugLinkedListAddNewEntryBegin(x,y,__FILE__,__LINE__)
#define LinkedListAddNewEntryAfter(x,y,z) DebugLinkedListAddNewEntryAfter(x,y,z,__FILE__,__LINE__)
#define LinkedListAddNewEntryBefore(x,y,z) DebugLinkedListAddNewEntryBefore(x,y,z,__FILE__,__LINE__)
#define LinkedListAddNewEntryProcEnd(x,y,z) DebugLinkedListAddNewEntryProcEnd(x,y,z,__FILE__,__LINE__)
#define LinkedListAddNewEntryProcBegin(x,y,z) DebugLinkedListAddNewEntryProcBegin(x,y,z,__FILE__,__LINE__)
#define LinkedListAddNewEntryProcAfter(x,y,z,w) DebugLinkedListAddNewEntryProcAfter(x,y,z,w,__FILE__,__LINE__)
#define LinkedListAddNewEntryProcBefore(x,y,z,w) DebugLinkedListAddNewEntryProcBefore(x,y,z,w,__FILE__,__LINE__)
#define LinkedListAddNewEntryMemEnd(x,y) DebugLinkedListAddNewEntryMemEnd(x,y,__FILE__,__LINE__)
#define LinkedListAddNewEntryMemBegin(x,y) DebugLinkedListAddNewEntryMemBegin(x,y,__FILE__,__LINE__)
#define LinkedListAddNewEntryMemAfter(x,y,z) DebugLinkedListAddNewEntryMemAfter(x,y,z,__FILE__,__LINE__)
#define LinkedListAddNewEntryMemBefore(x,y,z) DebugLinkedListAddNewEntryMemBefore(x,y,z,__FILE__,__LINE__)
#define LinkedListAddNewEntryStringEnd(x,y) DebugLinkedListAddNewEntryStringEnd(x,y,__FILE__,__LINE__)
#define LinkedListAddNewEntryStringBegin(x,y) DebugLinkedListAddNewEntryStringBegin(x,y,__FILE__,__LINE__)
#define LinkedListAddNewEntryStringAfter(x,y,z) DebugLinkedListAddNewEntryStringAfter(x,y,z,__FILE__,__LINE__)
#define LinkedListAddNewEntryStringBefore(x,y,z) DebugLinkedListAddNewEntryStringBefore(x,y,z,__FILE__,__LINE__)
#endif

#define LinkedListGetSize(x) (x)->Count
#define LinkedListGetFirst(x) (x)->First
#define LinkedListGetLast(x) (x)->Last
#define LinkedListGetFirstData(x) (x)->First->Data
#define LinkedListGetLastData(x) (x)->Last->Data

/* List handlers */

typedef struct {
	char* FirstItemTextPtr;	/* Pointer to the string array */
	Word ItemSize;			/* Size of each string element */
	Word NumItems;			/* Number of elements */
} ListFixed_t;

typedef struct {
	char** FirstItemTextArrayPtr;	/* Pointer to the "C" string pointer array */
	Word ItemSize;			/* Size of each element */
	Word NumItems;			/* Number of elements */
} ListStatic_t;

typedef struct {
	void **ArrayHandle;		/* Handle to the array of "C" string pointers */
	Word NumItems;			/* Number of elements */
} ListDynamic_t;

typedef struct {
	int MinVal;				/* Lowest value integer */
	int MaxVal;				/* Highest value integer */
	char WorkString[16];	/* ASCII version of an integer (Used bu GetString()) */
} ListIntRange_t;

typedef struct {
	Fixed32 MinVal;			/* Lowest value fixed */
	Fixed32 MaxVal;			/* Highest value fixed */
	Fixed32 StepVal;		/* Step range */
	char WorkString[32];	/* Ascii version of a fixed point number */
} ListFixedRange_t;

typedef struct {
	float MinVal;			/* Lowest value float */
	float MaxVal;			/* Highest value float */
	float StepVal;			/* Step range */
	char WorkString[32];	/* Ascii version of a floating point number */
} ListFloatRange_t;

extern void BURGERCALL ListFixedInit(ListFixed_t *Input,char *FirstItem,Word Size,Word Count);
#define ListFixedDestroy(x)
extern Word BURGERCALL ListFixedFind(const ListFixed_t *Input,const char *ItemText);
extern char * BURGERCALL ListFixedGetString(const ListFixed_t *Input,Word Index);

extern void BURGERCALL ListStaticInit(ListStatic_t *Input,char **FirstItem,Word Size,Word Count);
#define ListStaticDestroy(x)
extern Word BURGERCALL ListStaticFind(const ListStatic_t *Input,const char *ItemText);
extern char * BURGERCALL ListStaticGetString(const ListStatic_t *Input,Word Index);

extern void BURGERCALL ListDynamicInit(ListDynamic_t *Input);
extern Word BURGERCALL ListDynamicFind(const ListDynamic_t *Input,const char *ItemText);
extern char * BURGERCALL ListDynamicGetString(const ListDynamic_t *Input,Word Index);
extern void BURGERCALL ListDynamicDestroy(ListDynamic_t *Input);
extern void BURGERCALL ListDynamicAdd(ListDynamic_t *Input,char *ItemText);
extern void BURGERCALL ListDynamicRemoveString(ListDynamic_t *Input,const char *ItemText);
extern void BURGERCALL ListDynamicRemoveIndex(ListDynamic_t *Input,Word Index);

extern void BURGERCALL ListIntRangeInit(ListIntRange_t *Input,int MinVal,int MaxVal);
#define ListIntRangeDestroy(x)
extern Word BURGERCALL ListIntRangeFind(const ListIntRange_t *Input,const char *ItemText);
extern char * BURGERCALL ListIntRangeGetString(ListIntRange_t *Input,Word Index);

extern void BURGERCALL ListFixedRangeInit(ListFixedRange_t *Input,Fixed32 MinVal,Fixed32 MaxVal,Fixed32 Step);
#define ListFixedRangeDestroy(x)
extern Fixed32 BURGERCALL ListFixedRangeFind(const ListFixedRange_t *Input,const char *ItemText);
extern char * BURGERCALL ListFixedRangeGetString(ListFixedRange_t *Input,Fixed32 Index);

extern void BURGERCALL ListFloatRangeInit(ListFloatRange_t *Input,float MinVal,float MaxVal,float Step);
#define ListFloatRangeDestroy(x)
extern float BURGERCALL ListFloatRangeFind(const ListFloatRange_t *Input,const char *ItemText);
extern char * BURGERCALL ListFloatRangeGetString(ListFloatRange_t *Input,float Index);

/* Timed code execution handler */

typedef Word (BURGERCALL *RunQueueProc_t)(void **,Word EventNum);	/* Run queue code */
typedef Word (BURGERCALL *RunQueuePollCallback_t)(void **);	/* Poll callback */

typedef struct RunQueue_t {
	struct RunQueue_t **NextQueue;		/* Next time event code routine in list */
	struct RunQueue_t **PrevQueue;		/* Previous entry for linked list */
	RunQueueProc_t Proc;	/* Event procedure */
	Word TimeQuantum;		/* Time in ticks before execution */
	Word ElapsedTime;		/* Fractional time in ticks */
	Word Priority;			/* Execution priority */
	Word IDNum;				/* ID used by application (-1 for master) */
} RunQueue_t;

enum {RQTIME,RQINIT,RQDESTROY,RQSAVE,RQLOAD,RQDEBUG,RQUSER};

extern RunQueue_t ** BURGERCALL MasterRunQueueNew(void);
extern void BURGERCALL MasterRunQueueInit(RunQueue_t **MasterQueueHandle);
extern void BURGERCALL MasterRunQueueDelete(RunQueue_t **MasterQueueHandle);
extern void BURGERCALL MasterRunQueueDestroy(RunQueue_t **MasterQueueHandle);
extern void BURGERCALL MasterRunQueueExecute(RunQueue_t **MasterQueueHandle,Word TimeQuantum);
extern void BURGERCALL MasterRunQueueDump(RunQueue_t **MasterQueueHandle);

extern Word BURGERCALL RunQueueCallProc(RunQueue_t **RunQueueHandle,Word Event);
extern Word BURGERCALL RunQueueDefaultProc(void **RunQueueHandle,Word Event);
extern RunQueue_t ** BURGERCALL RunQueueNew(RunQueue_t **MasterQueueHandle,Word MemSize,
	Word TimeQuantum,Word Priority,Word IDNum,RunQueueProc_t Proc);
extern void BURGERCALL RunQueueDelete(RunQueue_t **RunQueueHandle);
extern Word BURGERCALL RunQueueDeleteProc(void **RunQueueHandle,Word Event);
extern void BURGERCALL RunQueueDeleteDefer(RunQueue_t **RunQueueHandle);
extern void BURGERCALL RunQueueLink(RunQueue_t **MasterQueueHandle,RunQueue_t **RunQueueHandle,Word Priority);
extern void BURGERCALL RunQueueUnlink(RunQueue_t **RunQueueHandle);
extern RunQueue_t ** BURGERCALL RunQueueFindIDNum(RunQueue_t **RunQueueHandle,Word IDNum);
extern Word BURGERCALL RunQueuePoll(RunQueue_t **MasterQueueHandle,RunQueuePollCallback_t CallBack,Word IDNum);

/* Dialog controls */

typedef enum {BUTTON_OUTSIDE,BUTTON_INSIDE,BUTTON_DOWN,BUTTON_RELEASED,BUTTON_CLICKED} DialogControlAction_e;

typedef void (BURGERCALL *DialogControlProc)(struct DialogControl_t *Input);
typedef void (BURGERCALL *DialogControlDrawProc)(struct DialogControl_t *Input, int x, int y);
typedef void (BURGERCALL *DialogControlEventProc)(struct DialogControl_t *Input,DialogControlAction_e type);
typedef DialogControlAction_e (BURGERCALL *DialogControlCheckProc)(struct DialogControl_t *Input, int x,int y,Word buttons,Word Key);

typedef void (BURGERCALL *DialogProc)(struct Dialog_t *Input);
typedef void (BURGERCALL *DialogDrawProc)(struct Dialog_t *Input, int x, int y);
typedef Bool (BURGERCALL *DialogEventProc)(struct Dialog_t *Input,int x,int y,Word buttons,Word Key);

typedef void (BURGERCALL *DialogControlGenericListDrawProc)(struct DialogControlGenericList_t *Input, void *Data, int x, int y, int Width, int Height, Bool highlighted);

typedef struct DialogControl_t {
	LBRect Bounds;							/* Bounds rect for the control */
	Word8 Active;							/* TRUE if an active control */
	Word8 Invisible;							/* TRUE if invisible (Not drawn) */
	Word8 Focus;		 						/* Flag for if the button was down */
	Word8 Inside;							/* Mouse currently inside */
	DialogControlDrawProc Draw;					/* Draw the control */
	DialogControlProc Delete;				/* Delete the control */
	DialogControlEventProc Event;			/* An event occured (Callback) */
	DialogControlCheckProc Check;			/* Check if I hit a hot spot */
	void *RefCon;							/* User data */
	Word HotKey;							/* Ascii for the Hot Key */
	int Id;
} DialogControl_t;

typedef struct DialogControlList_t {
	Word NumButtons;						/* Number of VALID buttons */
	Word MemListSize;						/* Size of buttonlist for memory allocation */
	Word Dormant;							/* True if waiting for a mouse up but no control has focus */
	DialogControl_t *FocusControl;			/* Control that has focus */
	DialogControl_t **ControlList;			/* Pointers to the button list */
} DialogControlList_t;

typedef struct Dialog_t {
	DialogControlList_t MyList;
	LBRect Bounds;
	Word8 Invisible;
	Word FillColor;
	Word OutlineColor;
	DialogDrawProc Draw;						/* Draw the control */
	DialogProc Delete;						/* Delete the control */
	DialogEventProc Event;					/* An event occured (Callback) */
} Dialog_t;

typedef struct DialogList_t {
	Word NumDialogs;						/* Number of VALID buttons */
	Dialog_t *FrontDialog;					/* Control that has focus */
	Dialog_t **DialogList;					/* Pointers to the button list */
} DialogList_t;

typedef struct DialogControlButton_t {		/* Simple button */
	DialogControl_t Root;					/* Base class */
	int x,y;								/* Coords to draw the shape */
	struct ScreenShape_t *Art[3];				/* Shape number indexes */
} DialogControlButton_t;

typedef struct DialogControlTextButton_t {	/* Simple text string button */
	DialogControl_t Root;					/* Base class */
	//int x;								/* Coords to draw the string */
	const char *TextPtr;					/* Text string */
	struct FontRef_t *FontPtr;				/* Font to draw with */
	struct FontRef_t *FontPtr2;				/* Highlight Font to draw with */
	struct ScreenShape_t **Art;				/* Shapes to use (Array of 4) */
} DialogControlTextButton_t;

typedef struct DialogControlCheckBox_t {	/* Simple check box */
	DialogControl_t Root;					/* Base class */
	DialogControlEventProc Proc;			/* Pass through so I can handle the check events */
	struct ScreenShape_t **Art;				/* Shapes to use (Array of 4) */
//	int CheckY;								/* Coords to draw the checkbox */
//	int TextX,TextY;						/* Coords to draw the text */
	const char *TextPtr;					/* Text string */
	struct FontRef_t *FontPtr;				/* Font to draw with */
	Word Checked;							/* True if checked */
} DialogControlCheckBox_t;

typedef struct DialogControlSliderBar_t {	/* Slider bar */
	DialogControl_t Root;					/* Base class */
	struct ScreenShape_t **Art;				/* Shapes to use (Array of 5) */
	int BarY;								/* Coords to draw the slider bar */
	int ThumbX,ThumbY;						/* Current position of the thumb */
	Word ThumbAnchor;						/* Offset into the thumb for "Grabbing" */
	Word ThumbMinX;							/* Minimum offset from the left side */
	Word BarWidth;							/* Range of pixels the thumb can move */
	Word Range;								/* Range of the value (0-Range inclusive) */
	Word Value;								/* Current value */
} DialogControlSliderBar_t;

typedef struct DialogControlRepeatButton_t {	/* Simple repeater button */
	DialogControl_t Root;					/* Base class */
	int x,y;								/* Coords to draw the shape */
	struct ScreenShape_t *Art[3];				/* Shape number indexes */
	Word32 TimeMark;						/* Time mark when clicked */
	Word32 RepeatDelay;					/* Time before another click */
} DialogControlRepeatButton_t;

typedef struct DialogControlVScrollSlider_t {	/* Slider bar */
	DialogControl_t Root;					/* Base class */
	struct ScreenShape_t **Art;				/* Shapes to use (Array of 11) */
	int ThumbY;								/* Current position of the thumb */
	Word ThumbSize;							/* Size of the thumb in pixels */
	Word ThumbAnchor;						/* Offset into the thumb for "Grabbing" */
	Word BarHeight;							/* Range of pixels the thumb can move */
	Word Range;								/* Range of the value (0-Range inclusive) */
	Word Step;								/* Motion to step if clicked in a dead region */
	Word Value;								/* Current value */
	Word32 TimeMark;						/* Time mark when clicked */
	Word32 RepeatDelay;					/* Time before another click */
} DialogControlVScrollSlider_t;

typedef struct DialogControlVScroll_t {		/* Simple button */
	DialogControl_t Root;					/* Base class */
	DialogControlList_t MyList;				/* List of controls */
	DialogControlVScrollSlider_t *Slider;	/* Slider for up and down */
	Word ButtonStep;						/* Step for button press */
	Word Value;								/* Current value */
} DialogControlVScroll_t;

typedef struct DialogControlTextBox_t {		/* Simple box of text */
	DialogControl_t Root;					/* Base class */
	struct FontRef_t *FontPtr;				/* Font to use */
	const char *TextPtr;					/* Current text */
	struct FontWidthLists_t *TextDescription;	/* Format info for the text */
	struct ScreenShape_t **Art;				/* Art for the scroll bar */
	DialogControlVScroll_t *Slider;			/* Scroll bar if present */
	Word Value;								/* Top visible Y coordinate */
	Word OutlineColor;						/* Color to draw the outline in */
	Bool ScrollBarNormalArrowStyle;		/* The scroll bar style to use */
	Bool AllowSlider;
} DialogControlTextBox_t;

typedef struct DialogControlStaticText_t {		/* Simple box of text */
	DialogControl_t Root;					/* Base class */
	struct FontRef_t *FontPtr;				/* Font to use */
	const char *TextPtr;					/* Current text */
	struct FontWidthLists_t *TextDescription;	/* Format info for the text */
} DialogControlStaticText_t;

typedef struct DialogControlTextList_t {	/* Simple list of text */
	DialogControl_t Root;					/* Base class */
	struct FontRef_t *FontPtr;				/* Font to use */
	struct FontRef_t *FontPtr2;				/* Font to use (highlight) */
	DialogControlVScroll_t *Slider;			/* Scroll bar if present */
	struct ScreenShape_t **Art;				/* Scroll bar art */
	LinkedList_t List;						/* List of text */
	Word ScrollValue;						/* Which entry to display for scrolling */
	Word Value;								/* Which entry is valid */
	Word OutlineColor;						/* Color to draw the outline in */
	Word FillColor;							/* Color to draw background with */
	Word SelColor;							/* Color to draw highlighted row with */
	Bool ScrollBarNormalArrowStyle;		/* The scroll bar style to use */
} DialogControlTextList_t;

typedef struct DialogControlGenericList_t {	/* Simple list of text */
	DialogControl_t Root;					/* Base class */
	DialogControlVScroll_t *Slider;			/* Scroll bar if present */
	struct ScreenShape_t **Art;				/* Scroll bar art */
	LinkedList_t List;						/* List of text */
	Word ScrollValue;						/* Which entry to display for scrolling */
	Word Value;								/* Which entry is valid */
	Word OutlineColor;						/* Color to draw the outline in */
	Word FillColor;							/* Color to draw background with */
	Word SelColor;							/* Color to draw highlighted row with */
	Word CellHeight;
	Bool ScrollBarNormalArrowStyle;		/* The scroll bar style to use */
	DialogControlGenericListDrawProc CellDraw;
} DialogControlGenericList_t;

typedef struct DialogControlTextMenu_t {	/* Menu control (Used by pop-up menus) */
	DialogControl_t Root;					/* Base class */
	LinkedList_t *ListPtr;					/* List of text (I don't control the list) */
	struct FontRef_t *FontPtr;				/* Font to use */
	struct FontRef_t *FontPtr2;				/* Font to use (highlight) */
	Word Value;								/* Entry selected for highlight */
	Word CursorValue;						/* Entry the the cursor is over */
	int ScrollValue;						/* Scroll factor */
	Word CellHeight;						/* Number of cells high */
	Word NormColor;							/* Color to fill normally */
	Word CursorColor;						/* Color to fill for cursor highlight */
	Word BoxColor1;							/* Color for shadow box */
	Word BoxColor2;							/* Second color for shadow box */
	struct ScreenShape_t **Art;				/* Art for up and down arrows */
	Word32 TimeMark;						/* Time mark when clicked */
} DialogControlTextMenu_t;

typedef struct DialogControlPopupMenu_t {	/* Simple list of text */
	DialogControl_t Root;					/* Base class */
	struct FontRef_t *FontPtr;				/* Font to use */
	struct FontRef_t *FontPtr2;				/* Font to use (highlight) */
	DialogControlTextMenu_t *MenuPtr;		/* Pop up menu */
	LinkedList_t List;						/* List of text */
	Word Value;								/* Which entry is valid */
	Word NormColor;							/* Color to fill normally */
	Word CursorColor;						/* Color to fill for cursor highlight */
	Word BoxColor1;							/* Color for shadow box */
	Word BoxColor2;							/* Second color for shadow box */
	struct ScreenShape_t **Art;				/* Art list for controls */
} DialogControlPopupMenu_t;

#define DIALOGLINEEDIT_MAX_LEN 256
#define DIALOGLINEEDIT_ALPHAONLY 1
#define DIALOGLINEEDIT_NUMBERONLY 2
#define DIALOGLINEEDIT_CAPS 4
#define DIALOGLINEEDIT_SPACEOK 8

typedef struct DialogControlLineEdit_t {	/* Line Edit Control */
	DialogControl_t Root;					/* Base class */
	struct FontRef_t *FontRef;				/* Font to use */
	char Value[DIALOGLINEEDIT_MAX_LEN];	/* Current input */
	Word Length;							/* Length of valid input */
	Word CurPos;							/* Cursor index */
	Word Insert;							/* TRUE if insert mode */
	Word MaxLen;							/* Maximum length */
	Word CursorColor;						/* Color to draw the cursor line with */
	Word Flags;								/* Flags for line edit mode */
	Bool CursorFlag;						/* Whether to show the cursor */
} DialogControlLineEdit_t;

typedef struct DialogControlPicture_t {		/* Simple button */
	DialogControl_t Root;					/* Base class */
	struct ScreenShape_t *Art;				/* Shape number indexes */
} DialogControlPicture_t;

extern Word32 DialogControlTextBoxOutlineColor;		/* Color for outlines */
extern Word32 DialogControlTextBoxFillColor;
extern Word32 DialogControlTextBoxSelectedRowColor;
extern Word32 DialogControlShadowColor1;
extern Word32 DialogControlShadowColor2;
extern Word32 DialogControlMenuBackgroundColor;
extern Word32 DialogControlMenuSelectColor;

extern DialogList_t DialogMasterList;	/* Convenience; you usually don't need more than one */

extern void BURGERCALL DialogControlDelete(DialogControl_t *Input);
extern DialogControlAction_e BURGERCALL DialogControlCheck(DialogControl_t *Input,int x,int y,Word buttons,Word Key);
extern void BURGERCALL DialogControlDeleteProc(DialogControl_t *Input);
extern void BURGERCALL DialogControlMove(DialogControl_t *Input,int xoffset,int yoffset);
extern void BURGERCALL DialogControlMoveTo(DialogControl_t *Input,int x,int y);

extern void BURGERCALL DialogInit(Dialog_t *Input);
extern void BURGERCALL DialogDestroy(Dialog_t *Input);
extern DialogControl_t * BURGERCALL DialogCheck(Dialog_t *Input,int x,int y,Word Buttons,Word Key);
extern void BURGERCALL DialogDraw(Dialog_t *Input);
extern DialogControl_t * BURGERCALL DialogModal(Dialog_t *Input);
extern void BURGERCALL DialogMove(Dialog_t *Input,int xoffset,int yoffset);
extern void BURGERCALL DialogMoveTo(Dialog_t *Input,int x,int y);
extern Word32 DialogInitParseMacDLOG(Dialog_t *Input, const Word8 *DLOGData, char *name);
extern Word DialogInitParseMacDITL(Dialog_t *Input, Word8 *DITLData);

extern void BURGERCALL DialogListInit(DialogList_t *Input);
extern void BURGERCALL DialogListDestroy(DialogList_t *Input);
extern void BURGERCALL DialogListAddDialog(DialogList_t *Input,Dialog_t *DialogPtr);
extern void BURGERCALL DialogListRemoveDialog(DialogList_t *Input,Dialog_t *DialogPtr);
extern void BURGERCALL DialogListDraw(DialogList_t *Input, Bool RefreshAll);
extern DialogControl_t * BURGERCALL DialogListModal(DialogList_t *Input, Bool RefreshAll, Dialog_t **OutDialog);

extern void BURGERCALL DialogControlListInit(DialogControlList_t *Input);
extern void BURGERCALL DialogControlListDestroy(DialogControlList_t *Input);
extern DialogControl_t * BURGERCALL DialogControlListCheck(DialogControlList_t *Input,int x,int y,Word Buttons,Word Key);
extern void BURGERCALL DialogControlListAddControl(DialogControlList_t *Input,DialogControl_t *ControlPtr);
extern void BURGERCALL DialogControlListDraw(DialogControlList_t *Input, int x, int y);
extern DialogControl_t * BURGERCALL DialogControlListControlById(DialogControlList_t *Input, int Id);
extern DialogControlButton_t * BURGERCALL DialogControlListAddNewButton(DialogControlList_t *Input,struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControlTextButton_t * BURGERCALL DialogControlListAddNewTextButton(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,const char *TextPtr,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControlCheckBox_t * BURGERCALL DialogControlListAddNewCheckBox(DialogControlList_t *Input,struct ScreenShape_t **ShapeArray,const char *TextPtr,struct FontRef_t *FontPtr,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc,Word Checked);
extern DialogControlSliderBar_t * BURGERCALL DialogControlListAddSliderBar(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range);
extern DialogControlRepeatButton_t * BURGERCALL DialogControlListAddRepeatButton(DialogControlList_t *Input,struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControlVScrollSlider_t * BURGERCALL DialogControlListAddVScrollSlider(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step);
extern DialogControlVScroll_t *DialogControlListAddVScroll(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step,Word ButtonStep,Bool NormalArrowStyle);
extern DialogControlTextBox_t *BURGERCALL DialogControlListAddTextBox(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,Word Value,struct FontRef_t *FontPtr,Bool NormalArrowStyle,Bool AllowSlider);
extern DialogControlStaticText_t *BURGERCALL DialogControlListAddStaticText(DialogControlList_t *Input,const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,struct FontRef_t *FontPtr);
extern DialogControlTextList_t *BURGERCALL DialogControlListAddTextList(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc,Bool ScrollBarNormalArrowStyle);
extern DialogControlTextMenu_t *BURGERCALL DialogControlListAddTextMenu(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,int x,int y,struct LinkedList_t *ListPtr,Word Value,DialogControlEventProc EventProc);
extern DialogControlPopupMenu_t *BURGERCALL DialogControlListAddPopupMenu(DialogControlList_t *Input,struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc);
extern DialogControlLineEdit_t *BURGERCALL DialogControlListAddLineEdit(DialogControlList_t *Input, struct FontRef_t* FontPtr, const LBRect *Bounds, Word MaxLen, Word32 CursorColor, Word Flags, DialogControlEventProc EventProc);
extern DialogControlPicture_t *BURGERCALL DialogControlListAddPicture(DialogControlList_t *Input, struct ScreenShape_t *Art, const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);

extern void BURGERCALL DialogControlInit(DialogControl_t *Input,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControl_t *BURGERCALL DialogControlNew(const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);

extern void BURGERCALL DialogControlButtonInit(DialogControlButton_t *Input,struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControlButton_t *BURGERCALL DialogControlButtonNew(struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern void BURGERCALL DialogControlButtonSetShapes(DialogControlButton_t *Input,struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3);

extern void BURGERCALL DialogControlTextButtonInit(DialogControlTextButton_t *Input,struct ScreenShape_t **ShapeArray,const char *Text,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControlTextButton_t *BURGERCALL DialogControlTextButtonNew(struct ScreenShape_t **ShapeArray,const char *Text,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern void BURGERCALL DialogControlTextButtonSetText(DialogControlTextButton_t *Input,const char *TextPtr,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2);
extern void BURGERCALL DialogControlTextButtonSetArt(DialogControlTextButton_t *Input,struct ScreenShape_t **ShapeArray);

extern void BURGERCALL DialogControlCheckBoxInit(DialogControlCheckBox_t *Input,struct ScreenShape_t **ShapeArray,const char *Text,struct FontRef_t *FontPtr,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc,Word Checked);
extern DialogControlCheckBox_t *BURGERCALL DialogControlCheckBoxNew(struct ScreenShape_t **ShapeArray,const char *Text,struct FontRef_t *FontPtr,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc,Word Checked);
extern void BURGERCALL DialogControlCheckBoxSetText(DialogControlCheckBox_t *Input,struct ScreenShape_t **ShapeArray,const char *Text,struct FontRef_t *FontPtr);

extern void BURGERCALL DialogControlSliderBarInit(DialogControlSliderBar_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range);
extern DialogControlSliderBar_t * BURGERCALL DialogControlSliderBarNew(struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range);
extern void BURGERCALL DialogControlSliderBarSetValue(DialogControlSliderBar_t *Input,Word NewValue);
extern void BURGERCALL DialogControlSliderBarSetParms(DialogControlSliderBar_t *Input,Word Range);
extern void BURGERCALL DialogControlSliderBarSetArt(DialogControlSliderBar_t *Input,struct ScreenShape_t **ArtArray);

extern void BURGERCALL DialogControlRepeatButtonInit(DialogControlRepeatButton_t *Input,struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControlRepeatButton_t * BURGERCALL DialogControlRepeatButtonNew(struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern void BURGERCALL DialogControlRepeatButtonSetArt(DialogControlRepeatButton_t *Input,struct ScreenShape_t *Shape1,struct ScreenShape_t *Shape2,struct ScreenShape_t *Shape3);

extern void BURGERCALL DialogControlVScrollSliderInit(DialogControlVScrollSlider_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step);
extern DialogControlVScrollSlider_t * BURGERCALL DialogControlVScrollSliderNew(struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step);
extern void BURGERCALL DialogControlVScrollSliderSetValue(DialogControlVScrollSlider_t *Input,Word NewValue);
extern void BURGERCALL DialogControlVScrollSliderSetParms(DialogControlVScrollSlider_t *Input,Word Range,Word Step);

extern void BURGERCALL DialogControlVScrollSetValue(DialogControlVScroll_t *Input,Word NewValue);
extern void BURGERCALL DialogControlVScrollSetParms(DialogControlVScroll_t *Input,Word Range,Word Step,Word ButtonStep);
extern void BURGERCALL DialogControlVScrollInit(DialogControlVScroll_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step,Word ButtonStep, Bool NormalArrowStyle);
extern DialogControlVScroll_t * BURGERCALL DialogControlVScrollNew(struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,Word Value,Word Range,Word Step,Word ButtonStep,Bool NormalArrowStyle);

extern void BURGERCALL DialogControlTextBoxSetValue(DialogControlTextBox_t *Input,Word NewValue);
extern void BURGERCALL DialogControlTextBoxSetText(DialogControlTextBox_t *Input,const char *TextPtr,struct FontRef_t *FontPtr);
extern void BURGERCALL DialogControlTextBoxInit(DialogControlTextBox_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,Word Value,struct FontRef_t *FontPtr,Bool ScrollBarNormalArrowStyle,Bool AllowSlider);
extern DialogControlTextBox_t * BURGERCALL DialogControlTextBoxNew(struct ScreenShape_t **ArtArray,const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,Word Value,struct FontRef_t *FontPtr,Bool ScrollBarNormalArrowStyle,Bool AllowSlider);

extern void BURGERCALL DialogControlStaticTextSetText(DialogControlStaticText_t *Input,const char *TextPtr,struct FontRef_t *FontPtr);
extern void BURGERCALL DialogControlStaticTextInit(DialogControlStaticText_t *Input,const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,struct FontRef_t *FontPtr);
extern DialogControlStaticText_t * BURGERCALL DialogControlStaticTextNew(const LBRect *Bounds,DialogControlEventProc EventProc,const char *TextPtr,struct FontRef_t *FontPtr);

extern void BURGERCALL DialogControlTextListSetValue(DialogControlTextList_t *Input,Word NewValue);
extern Word BURGERCALL DialogControlTextListAddText(DialogControlTextList_t *Input,const char *TextPtr,Word EntryNum);
extern void BURGERCALL DialogControlTextListRemoveText(DialogControlTextList_t *Input,Word EntryNum);
extern void BURGERCALL DialogControlTextListRemoveAllText(DialogControlTextList_t *Input);
extern Word BURGERCALL DialogControlTextListFindText(DialogControlTextList_t *Input,const char *TextPtr);
extern void BURGERCALL DialogControlTextListInit(DialogControlTextList_t *Input,struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc,Bool ScrollBarNormalArrowStyle);
extern DialogControlTextList_t *BURGERCALL DialogControlTextListNew(struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc,Bool ScrollBarNormalArrowStyle);

extern void BURGERCALL DialogControlGenericListSetValue(DialogControlGenericList_t *Input,Word NewValue);
extern Word BURGERCALL DialogControlGenericListAddRow(DialogControlGenericList_t *Input,void *DataPtr,Word EntryNum);
extern void BURGERCALL DialogControlGenericListRemoveRow(DialogControlGenericList_t *Input,Word EntryNum);
extern void BURGERCALL DialogControlGenericListRemoveAllRows(DialogControlGenericList_t *Input);
extern void BURGERCALL DialogControlGenericListInit(DialogControlGenericList_t *Input,struct ScreenShape_t **ArtArray,const LBRect *Bounds,Word CellHeight,DialogControlEventProc EventProc,DialogControlGenericListDrawProc CellDrawProc,Bool ScrollBarNormalArrowStyle);
extern DialogControlGenericList_t *BURGERCALL DialogControlGenericListNew(struct ScreenShape_t **ArtArray,const LBRect *Bounds,Word CellHeight,DialogControlEventProc EventProc,DialogControlGenericListDrawProc CellDrawProc,Bool ScrollBarNormalArrowStyle);

extern void BURGERCALL DialogControlTextMenuInit(DialogControlTextMenu_t *Input,struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,int x,int y,LinkedList_t *ListPtr,Word Value,DialogControlEventProc EventProc);
extern DialogControlTextMenu_t *BURGERCALL DialogControlTextMenuNew(struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,int x,int y,LinkedList_t *ListPtr,Word Value,DialogControlEventProc EventProc);

extern void BURGERCALL DialogControlPopupMenuSetValue(DialogControlPopupMenu_t *Input,Word NewValue);
extern Word BURGERCALL DialogControlPopupMenuAddText(DialogControlPopupMenu_t *Input,const char *TextPtr,Word EntryNum);
extern void BURGERCALL DialogControlPopupMenuRemoveText(DialogControlPopupMenu_t *Input,Word EntryNum);
extern void BURGERCALL DialogControlPopupMenuRemoveAllText(DialogControlPopupMenu_t *Input);
extern Word BURGERCALL DialogControlPopupMenuFindText(DialogControlPopupMenu_t *Input,const char *TextPtr);
extern void BURGERCALL DialogControlPopupMenuInit(DialogControlPopupMenu_t *Input,struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc);
extern DialogControlPopupMenu_t *BURGERCALL DialogControlPopupMenuNew(struct ScreenShape_t **ArtArray,struct FontRef_t *FontPtr,struct FontRef_t *FontPtr2,const LBRect *Bounds,DialogControlEventProc EventProc);

extern void BURGERCALL DialogControlLineEditReset(DialogControlLineEdit_t *Input);
extern void BURGERCALL DialogControlLineEditSetText(DialogControlLineEdit_t *Input,const char *text);
extern void BURGERCALL DialogControlLineEditGetText(DialogControlLineEdit_t *Input, char* Buffer, Word BufferSize );
extern void BURGERCALL DialogControlLineEditEnableCursor(DialogControlLineEdit_t* Input, Bool EnableCursor );
extern void BURGERCALL DialogControlLineEditSetInsertMode(DialogControlLineEdit_t* Input, Bool InsertMode );
extern Bool BURGERCALL DialogControlLineEditGetInsertMode(DialogControlLineEdit_t* Input );
extern Bool BURGERCALL DialogControlLineEditOnKeyPress(DialogControlLineEdit_t *Input,Word InKey);
extern void BURGERCALL DialogControlLineEditInit(DialogControlLineEdit_t *Input, struct FontRef_t* FontPtr, const LBRect *Bounds, Word MaxLen, Word32 CursorColor, Word Flags, DialogControlEventProc EventProc );
extern DialogControlLineEdit_t *BURGERCALL DialogControlLineEditNew(struct FontRef_t* FontPtr, const LBRect *Bounds, Word MaxLen, Word32 CursorColor, Word Flags, DialogControlEventProc EventProc);

extern void BURGERCALL DialogControlPictureInit(DialogControlPicture_t *Input,struct ScreenShape_t *Art,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern DialogControlPicture_t *BURGERCALL DialogControlPictureNew(struct ScreenShape_t *Art,const LBRect *Bounds,Word HotKey,DialogControlEventProc EventProc);
extern void BURGERCALL DialogControlPictureSetArt(DialogControlPicture_t *Input,struct ScreenShape_t *Art);

/* Sound handlers */

#define SOUND_EXCLUSIVE 0x80000000UL
#define SOUND_COOKIE 0x40000000UL

#define SOUNDTYPEBYTE 0		/* WAVE 8 bit data */
#define SOUNDTYPECHAR 1		/* AIFF 8 bit data */
#define SOUNDTYPELSHORT 2	/* Little endian short */
#define SOUNDTYPEBSHORT 3	/* Big endian short */
#define SOUNDTYPEADPCM 4	/* MS ADPCM compression */
#define SOUNDTYPEDVIPCM 5	/* Intel DVI ADPCM compression */
#define SOUNDTYPEMP3 6		/* MP3 Audio */
#define SOUNDTYPEULAW 7		/* MuLaw */
#define SOUNDTYPEALAW 8		/* ALaw */
#define SOUNDTYPEMACE3 9	/* Mace 3:1 */
#define SOUNDTYPEMACE6 10	/* Mace 6:1 */
#define SOUNDTYPEOGG 11		/* OGG/Vorbis Audio */
#define SOUNDTYPESTEREO 0x8000	/* Stereo data */
#define SOUNDTYPEDOUBLEBUFFER 0x4000	/* Double buffered */

/* SystemState defines */

#define SfxActive 1
#define MusicActive 2
#define PauseActive 4

typedef void (BURGERCALL *SndCompleteProcPtr)(void *);
typedef char *(BURGERCALL *MakeSongProc)(Word);

typedef struct RawSound_t {
	Word8 *SoundPtr;			/* Pointer to the data */
	Word32 SoundLength;	/* Length of the data in bytes */
	Word32 LoopStart;		/* Sample to start from */
	Word32 LoopEnd;		/* Sample to end the loop (0 for no looping) */
	SndCompleteProcPtr Proc;	/* Completion routine */
	void *Data;				/* Data to pass for completion routine */
	Word DataType;			/* Type of data found */
	Word SampleRate;		/* Samples per second to play */
	Word8 *CompPtr;			/* Used by compression */
	Word Extra1;			/* Used by compression */
	Word Extra2;			/* Used by compression */
	Word Extra3;			/* Used by compression */
	Word32 SampleCount;	/* Output from ParseSoundFileImage */
	float TimeInSeconds;	/* Output from ParseSoundFileImage */
} RawSound_t;

#define MUSICCODECINIT 0
#define MUSICCODECDESTROY 1
#define MUSICCODECDECODE 2
#define MUSICCODECRESET 3

typedef Word32 (BURGERCALL *DecodeCallbackProc)(void *,Word8 *,Word32);
typedef Word32 (BURGERCALL *DecodeCodecProc)(struct DigitalMusicReadState_t *,Word,Word8 *,Word32);

typedef struct DigitalMusicReadState_t {	/* Private state structure */
	DecodeCallbackProc ReadProc;	/* Read data proc */
	void *CallBackParm;				/* Parm for the read callback */
	DecodeCodecProc CodecProc;		/* Decompressor */
	void *CompressStatePtr;			/* Extra data needed by codec */
	Word32 FileOffset;			/* Offset to where in the file data starts at */
	Word32 SoundLength;			/* Size of data to play (Decompressed) */
	Word32 BytesPlayed;			/* Number of bytes processed */
	Word DataType;					/* Type of input data (Codec index) */
} DigitalMusicReadState_t;

typedef int (BURGERCALL *MADImportPtr)(const Word8 *DataPtr,Word32 Length,struct MADMusic *MadFile);

extern Word SystemState;	/* Global game state flags */
extern Word SfxVolume;		/* Master volume of game sound effects */
extern Word *SoundCookiePtr;	/* Writeback handle for audio channel */
extern Word PanPosition;	/* Initial pan position of sound to play */
extern Word SoundLoopFlag;	/* True if you want the sound to loop */
extern int SoundFrequencyAdjust;	/* Pitch bend */
extern struct RezHeader_t *SoundRezHeader;	/* Resource file for sound loading */
extern MakeSongProc DigitalMusicNameCallback;

extern void BURGERCALL ModMusicInit(void);
extern void BURGERCALL ModMusicShutdown(void);
extern void BURGERCALL ModMusicImporter(MADImportPtr ImportPtr);
extern Word BURGERCALL ModMusicPlay(Word SongNum);
extern Word BURGERCALL ModMusicPlayByFilename(const char *FileName);
extern Word BURGERCALL ModMusicPlayByPtr(const Word8 *DataPtr,Word32 Length);
extern void BURGERCALL ModMusicStop(void);
extern void BURGERCALL ModMusicPause(void);
extern void BURGERCALL ModMusicResume(void);
extern void BURGERCALL ModMusicReset(void);
extern Word BURGERCALL ModMusicGetVolume(void);
extern void BURGERCALL ModMusicSetVolume(Word NewVolume);
extern int BURGERCALL ModMusicS3M(const Word8 *DataPtr,Word32 Length,struct MADMusic *MadFile);
extern int BURGERCALL ModMusicMADI(const Word8 *DataPtr,Word32 Length,struct MADMusic *MadFile);
extern int BURGERCALL ModMusicIT(const Word8 *DataPtr,Word32 Length,struct MADMusic *MadFile);
extern int BURGERCALL ModMusicXM(const Word8 *DataPtr,Word32 Length,struct MADMusic *MadFile);

extern Word32 BURGERCALL DigitalMusicByte(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicChar(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicULaw(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicALaw(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicLShort(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicBShort(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicMace3(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicMace6(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicADPCM(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word32 BURGERCALL DigitalMusicOgg(DigitalMusicReadState_t *Input,Word Command,Word8 *DestPtr,Word32 Length);
extern Word BURGERCALL DigitalMusicGetSilenceVal(Word Type);
extern Word BURGERCALL DigitalMusicDecode(DigitalMusicReadState_t *Input,Word8 *DestBuffer,Word32 Length);
extern Word BURGERCALL DigitalMusicReadStateInit(DigitalMusicReadState_t *Output,struct RawSound_t *Input,Word8 *ImagePtr,Word32 MaxSize,DecodeCallbackProc Proc,void *Parm);
extern void BURGERCALL DigitalMusicReadStateDestroy(DigitalMusicReadState_t *Input);
extern void BURGERCALL DigitalMusicReset(DigitalMusicReadState_t *Input);
extern void BURGERCALL DigitalMusicInit(void);
extern void BURGERCALL DigitalMusicShutdown(void);
extern Word BURGERCALL DigitalMusicIsPlaying(void);
extern Word BURGERCALL DigitalMusicGetFrequency(void);
extern Word BURGERCALL DigitalMusicGetVolume(void);
extern void BURGERCALL DigitalMusicSetFilenameProc(MakeSongProc Proc);
extern void BURGERCALL DigitalMusicSetFrequency(Word Freq);
extern void BURGERCALL DigitalMusicSetVolume(Word Volume);
extern void BURGERCALL DigitalMusicPlay(Word SongNum);
extern void BURGERCALL DigitalMusicPause(void);
extern void BURGERCALL DigitalMusicResume(void);

extern void BURGERCALL InitSoundPlayer(void);
extern void BURGERCALL KillSoundPlayer(void);
extern void BURGERCALL StopASound(Word32 SoundCookie);
extern Word BURGERCALL PlayASound(Word32 SoundNum);
extern Word BURGERCALL PlayARawSound(RawSound_t *Input);
extern Word BURGERCALL ParseSoundFileImage(RawSound_t *Output,const void *Input,Word32 Length);
extern double BURGERCALL ConvertAiffExtended(const void *Input);
extern void * BURGERCALL FindIffChunk(const void *Input,Word32 Name,Word32 Length);
extern void BURGERCALL PauseAllSounds(void);
extern void BURGERCALL ResumeAllSounds(void);
extern void BURGERCALL StopAllSounds(void);
extern void BURGERCALL SetMaxSounds(Word Max);
extern Word BURGERCALL GetMaxSounds(void);
extern Word BURGERCALL GetNumSoundsPlaying(void);
extern Word BURGERCALL GetSfxVolume(void);
extern void BURGERCALL SetSfxVolume(Word NewVolume);
extern Word BURGERCALL IsASoundPlaying(Word32 SoundCookie);
extern Word BURGERCALL GetASoundFrequency(Word32 SoundCookie);
extern void BURGERCALL SetASoundFrequency(Word32 SoundCookie,Word Frequency);
extern Word BURGERCALL GetASoundVolume(Word32 SoundCookie);
extern void BURGERCALL SetASoundVolume(Word32 SoundCookie,Word Volume);
extern Word BURGERCALL GetASoundPan(Word32 SoundCookie);
extern void BURGERCALL SetASoundPan(Word32 SoundCookie,Word Pan);
extern void BURGERCALL SoundSetCallback(Word32 SoundCookie,SndCompleteProcPtr Proc,void *Data);

/* CD Audio Manager */

#define REDBOOKMODEBUSY 0
#define REDBOOKMODESTOPPED 1
#define REDBOOKMODEPLAYING 2
#define REDBOOKMODEPAUSED 3
#define REDBOOKMODEOPENED 4

#define REDBOOKPLAY 0
#define REDBOOKPLAYONCE 1
#define REDBOOKPLAYLOOP 2

#define FRAMESPERSEC 75

#if defined(__WIN32__)	/* Windows 95 version */
typedef struct {			/* Members are private!! */
	Word32 FromMark;		/* Starting mark for loop */
	Word32 ToMark;		/* Destination mark used for resume */
	Word32 OpenDeviceID;	/* Win95 MMSystem device */
	void *ThreadID;			/* ID for the thread processing the CD */
	Word MixerDevice;		/* Device for audio control */
	Word MixerFlags;		/* Volume enable flag for mixer */
	Word LoopingTimerID;	/* Timer proc for looping a CD file */
	Bool Paused;			/* True if paused (Public) */
	Bool Active;			/* True if properly initialized (Public) */
	Bool Timer;			/* True if loop thread is spawned */
	Bool Mixer;			/* True if volume control is present */
} Redbook_t;

#elif defined(__MAC__)	/* MacOS version */
typedef struct {			/* Members are private!! */
	Word32 FromMark;		/* Starting mark for loop */
	Word32 ToMark;		/* Destination mark used for resume */
	Word32 TimeMark;		/* Timer for thread */
	void *ProcPtr;			/* IOCompletionUPP for async calls */
	Word8 ParamData[84*4];	/* MacOS ParamBlockRec (And a pad byte) */
	short OpenDeviceID;		/* MacOS device reference */
	Bool Paused;			/* True if paused (Public) */
	Bool Active;			/* True if properly initialized (Public) */
	Bool Timer;			/* True if loop thread is spawned */
} Redbook_t;
#else					/* DOS version */
typedef struct {			/* Members are private!! */
	Word32 FromMark;		/* Starting mark for loop */
	Word32 ToMark;		/* Destination mark used for resume */
	Word32 TimeMark;		/* Timer for thread */
	Word OpenDeviceID;		/* Dos device number */
	Bool Paused;			/* True if paused (Public) */
	Bool Active;			/* True if properly initialized (Public) */
	Bool Timer;			/* True if loop thread is spawned */
} Redbook_t;

#endif

extern Redbook_t * BURGERCALL RedbookNew(void);
extern Word BURGERCALL RedbookInit(Redbook_t *Input);
extern void BURGERCALL RedbookDelete(Redbook_t *Input);
extern void BURGERCALL RedbookDestroy(Redbook_t *Input);
extern void BURGERCALL RedbookPlay(Redbook_t *Input,Word Mode,Word Track);
extern void BURGERCALL RedbookPlayRange(Redbook_t *Input,Word Mode,Word StartTrack,Word32 StartPosition,Word EndTrack,Word32 EndPosition);
extern Word BURGERCALL RedbookGetCurrentTrack(Redbook_t *Input);
extern Word BURGERCALL RedbookGetTrackCount(Redbook_t *Input);
extern Word32 BURGERCALL RedbookGetTrackLength(Redbook_t *Input,Word Track);
extern Word32 BURGERCALL RedbookGetTrackStart(Redbook_t *Input,Word TrackNum);
extern void BURGERCALL RedbookStop(Redbook_t *Input);
extern void BURGERCALL RedbookGetVolume(Redbook_t *Input,Word *Left,Word *Right);
extern void BURGERCALL RedbookSetVolume(Redbook_t *Input,Word Left,Word Right);
extern Word BURGERCALL RedbookGetPlayStatus(Redbook_t *Input);
extern void BURGERCALL RedbookPause(Redbook_t *Input);
extern void BURGERCALL RedbookResume(Redbook_t *Input);
extern Word32 BURGERCALL RedbookGetPosition(Redbook_t *Input);
extern void BURGERCALL RedbookOpenDrawer(Redbook_t *Input);
extern void BURGERCALL RedbookCloseDrawer(Redbook_t *Input);
extern Word32 BURGERCALL RedbookMakeTMSF(Word Track,Word32 Position);
extern Word32 BURGERCALL RedbookMakePosition(Word Minutes,Word Seconds,Word Frames);
extern void BURGERCALL RedbookSetLoopStart(Redbook_t *Input,Word Track,Word32 Position);
extern void BURGERCALL RedbookSetLoopEnd(Redbook_t *Input,Word Track,Word32 Position);
extern Word BURGERCALL RedbookLogCDByName(Redbook_t *Input,const char *VolumeName);

/* Profile manager */

typedef enum {FPU_NONE,FPU_287,FPU_387,FPU_PENTIUM,
	FPU_601,FPU_603,FPU_68881} FPU_e;
typedef enum {CPU_UNKNOWN,CPU_386,CPU_486,CPU_586,CPU_686,
	CPU_601,CPU_603,CPU_604,CPU_750,
	CPU_68000,CPU_68020,CPU_68030,CPU_68040} CPU_e;
typedef enum {MMX_NONE,MMX_PENTIUM,MMX_K6,MMX_ALTIVEC} MMX_e;
typedef enum {VENDOR_UNKNOWN,VENDOR_INTEL,VENDOR_AMD,VENDOR_UMC,VENDOR_CYRIX,VENDOR_NEXGEN,
	VENDOR_IBM,VENDOR_HITACHI,VENDOR_MOTOROLA,VENDOR_ARM,VENDOR_MIPS} CPUVendor_e;

typedef struct CPUFeatures_t {
	CPU_e CPUFamily;	/* Class of CPU */
	FPU_e FPUFamily;	/* Class of FPU */
	MMX_e MMXFamily;	/* Extended instructions */
	CPUVendor_e Vendor;	/* Who made the chip? */
	char VerboseName[64];	/* Cpu name and feature string */
#if defined(__INTEL__)
	Word32 Features;	/* CPU ID features list */
	Word Revision;		/* CPU stepping flag */
	Word Model;			/* CPU model */
	Word Type;			/* CPU type */
	char VendorID[13];	/* Cpu vendor string */
#endif
} CPUFeatures_t;

typedef struct Profile_t {
	const char *Name;	/* Name of fragment being profiled */
	struct Profile_t *Next;	/* Next in linked list */
	Word32 Mark;		/* Current time mark */
	Word32 TimeIn;	/* Time inside the proc */
	Word32 TimeOut;	/* Time outside the proc */
	Word HitCount;		/* Number of times entered */
	Word RecurseCount;	/* Recursion flag */
	Word Initialized;	/* TRUE if initialized */
} Profile_t;

extern Profile_t *ProfileRoot;	/* Root pointer for linked list of Profile_t's */

extern void BURGERCALL CPUFeaturesGet(CPUFeatures_t *Input);
extern Profile_t * BURGERCALL ProfileNew(const char *Name);
extern void BURGERCALL ProfileInit(Profile_t *Input,const char *Name);
extern void BURGERCALL ProfileDelete(Profile_t *Input);
extern void BURGERCALL ProfileDestroy(Profile_t *Input);
extern void BURGERCALL ProfileEntry(Profile_t *Input);
extern void BURGERCALL ProfileExit(Profile_t *Input);
extern double BURGERCALL ProfileGetSecondsIn(const Profile_t *Input);
extern double BURGERCALL ProfileGetSecondsOut(const Profile_t *Input);
extern double BURGERCALL ProfileGetMicrosecondsIn(const Profile_t *Input);
extern double BURGERCALL ProfileGetMicrosecondsOut(const Profile_t *Input);
extern void BURGERCALL ProfileReset(Profile_t *Input);
extern void BURGERCALL ProfileResetAll(void);
extern Word BURGERCALL ProfileIsAvailable(void);

/* Flic player */

#define MOVIEDISKBASED 0
#define MOVIERAMBASED 1

typedef struct FlicMovie_t {
	FILE *fp;				/* Input file stream */
	Word8 *Buffer;			/* Input file buffer */
	Word32 BufferSize;	/* Size of the input buffer */
	StreamHandle_t MyInput;	/* Ram based input buffer */
	Word32 MovieSpeed;	/* Speed to play a movie */
	Word32 TickStart;		/* Tick the movie began */
	Word32 TickStop;		/* Tick the movie was paused at */
	Word32 FirstFrameMark;	/* Offset to the first movie frame */
	Word32 LoopFrameMark;	/* Offset to the loop frame */
	Word32 FileOffset;	/* Offset into the file for seeking */
	Image_t FrameImage;		/* Current image of the movie */
	Word CurrentFrameNumber;	/* Current frame being shown */
	Word MaxFrameNumber;	/* Frames in the movie */
	Bool Active;			/* True if this movie is in progress */
	Bool Paused;			/* True if the movie is paused */
	Bool Completed;		/* True if the movie has completed */
	Bool AllowFrameSkipping;	/* True if I can skip frames */
	Bool Looping;		/* True if it loops */
} FlicMovie_t;

extern FlicMovie_t * BURGERCALL FlicMovieNew(const char *FileName,Word Flags,Word32 FileOffset);
extern Word BURGERCALL FlicMovieInit(FlicMovie_t *Input,const char *FileName,Word Flags,Word32 FileOffset);
extern void BURGERCALL FlicMovieDelete(FlicMovie_t *Input);
extern void BURGERCALL FlicMovieDestroy(FlicMovie_t *Input);
#define FlicMovieIsPlaying(Input) ((Input)->Active)
#define FlicMovieGetWidth(Input) ((Input)->FrameImage.Width)
#define FlicMovieGetHeight(Input) ((Input)->FrameImage.Height)
extern Word BURGERCALL FlicMovieDataRead(FlicMovie_t *Input,void *Data,Word32 Length);
extern void BURGERCALL FlicMovieReset(FlicMovie_t *Input);
extern Word BURGERCALL FlicMovieStart(FlicMovie_t *Input);
extern void BURGERCALL FlicMovieStop(FlicMovie_t *Input);
extern void BURGERCALL FlicMoviePause(FlicMovie_t *Input);
extern void BURGERCALL FlicMovieResume(FlicMovie_t *Input);
extern void BURGERCALL FlicMovieSetLoopFlag(FlicMovie_t *Input,Word LoopFlag);
extern void BURGERCALL FlicMovieSetSpeed(FlicMovie_t *Input,Word FramesPerSecond);
extern void BURGERCALL FlicMovieSetToFrame(FlicMovie_t *Input,Word FrameNumber);
extern Image_t * BURGERCALL FlicMovieGetImage(FlicMovie_t *Input);
extern Image_t * BURGERCALL FlicMovieUpdate(FlicMovie_t *Input);

/* DPaint Anim player */

typedef struct DPaintAnimMovie_t {
	FILE *fp;				/* Input file stream */
	Word8 *Buffer;			/* Input file buffer */
	Word8 *DictionaryBuffer;	/* Dictionary buffer */
	StreamHandle_t MyInput;	/* Ram based input buffer */
	Word32 MovieSpeed;	/* Speed to play a movie */
	Word32 TickStart;		/* Tick the movie began */
	Word32 TickStop;		/* Tick the movie was paused at */
	Word32 FileOffset;	/* Offset into the file for seeking */
	Word DictionarySize;	/* Size of the dictionary in records */
	Image_t FrameImage;		/* Current image of the movie */
	Word CurrentFrameNumber;	/* Current frame being shown */
	Word MaxFrameNumber;	/* Frames in the movie */
	Bool Active;			/* True if this movie is in progress */
	Bool Paused;			/* True if the movie is paused */
	Bool Completed;		/* True if the movie has completed */
	Bool AllowFrameSkipping;	/* True if I can skip frames */
	Bool Looping;		/* True if it loops */
} DPaintAnimMovie_t;

extern DPaintAnimMovie_t * BURGERCALL DPaintAnimMovieNew(const char *FileName,Word Flags,Word32 FileOffset);
extern Word BURGERCALL DPaintAnimMovieInit(DPaintAnimMovie_t *Input,const char *FileName,Word Flags,Word32 FileOffset);
extern void BURGERCALL DPaintAnimMovieDelete(DPaintAnimMovie_t *Input);
extern void BURGERCALL DPaintAnimMovieDestroy(DPaintAnimMovie_t *Input);
#define DPaintAnimMovieIsPlaying(Input) ((Input)->Active)
#define DPaintAnimMovieGetWidth(Input) ((Input)->FrameImage.Width)
#define DPaintAnimMovieGetHeight(Input) ((Input)->FrameImage.Height)
extern Word BURGERCALL DPaintAnimMovieDataRead(DPaintAnimMovie_t *Input,void *Data,Word32 Length);
extern void BURGERCALL DPaintAnimMovieReset(DPaintAnimMovie_t *Input);
extern Word BURGERCALL DPaintAnimMovieStart(DPaintAnimMovie_t *Input);
extern void BURGERCALL DPaintAnimMovieStop(DPaintAnimMovie_t *Input);
extern void BURGERCALL DPaintAnimMoviePause(DPaintAnimMovie_t *Input);
extern void BURGERCALL DPaintAnimMovieResume(DPaintAnimMovie_t *Input);
extern void BURGERCALL DPaintAnimMovieSetLoopFlag(DPaintAnimMovie_t *Input,Word LoopFlag);
extern void BURGERCALL DPaintAnimMovieSetSpeed(DPaintAnimMovie_t *Input,Word FramesPerSecond);
extern Image_t * BURGERCALL DPaintAnimMovieGetImage(DPaintAnimMovie_t *Input);
extern Image_t * BURGERCALL DPaintAnimMovieUpdate(DPaintAnimMovie_t *Input);

/* GUID Manager */

#ifndef GUID_DEFINED		/* Used to be compatible with Win95 */
#define GUID_DEFINED
typedef struct _GUID {
	Word32 Data1;			/* Same names as Win95 */
	Word16 Data2;
	Word16 Data3;
	Word8 Data4[8];
} GUID;
#endif

typedef struct EthernetAddress_t {
    Word8 eaddr[6];      /* 6 bytes of ethernet hardware address */
} EthernetAddress_t;

extern GUID GUIDBlank;		/* Empty GUID */
extern void BURGERCALL GUIDInit(GUID *Output);
extern void BURGERCALL GUIDToString(char *Output,const GUID *Input);
extern Word BURGERCALL GUIDFromString(GUID *Output,const char *Input);
extern Word BURGERCALL GUIDHash(const GUID *Input);
extern Word BURGERCALL GUIDIsEqual(const GUID *Input1,const GUID *Input2);
extern int BURGERCALL GUIDCompare(const GUID *Input1,const GUID *Input2);
extern void BURGERCALL GUIDGetTime(LongWord64_t *Output);
extern Word BURGERCALL GUIDGetEthernetAddr(EthernetAddress_t *Output);
extern Word BURGERCALL GUIDGenRandomEthernet(EthernetAddress_t *Output);

/* Network Manager */

#define NET_MAXPACKETSIZE 3072
typedef enum {NET_PROVIDER_IPX,NET_PROVIDER_TCP,NET_PROVIDER_APPLETALK,NET_PROVIDER_COUNT} NetProvider_e;
typedef enum {SOCKETMODE_UNUSED,SOCKETMODE_LISTEN,SOCKETMODE_LISTENPACKET,SOCKETMODE_CONNECTED,
	SOCKETMODE_ACCEPTING,SOCKETMODE_COUNT} SocketMode_e;
typedef Word (BURGERCALL * NetListenProc)(struct NetHandle_t *);

typedef struct NetAddr_t {	/* Used for a network address */
	Word Provider;			/* Network provider (TCP/IPX/APPLETALK) */
	union {
		struct {			/* TCP/UDP data */
			Word Port;		/* TCP/IP, UDP Port */
			Word32 IP;	/* Internet IP (Network order) */
		} TCP;
		struct {			/* IPX/SPX data */
			Word8 Net[4];	/* IPX/SPX Network */
			Word8 Node[6];	/* IPX/SPX Node address */
			Word Socket;	/* IPX/SPX Socket */
		} IPX;
		struct {			/* Appletalk data */
			Word Network;	/* Appletalk network */
			Word NodeID;	/* Appletalk node */
			Word Socket;	/* Appletalk socket */
			Word DDPType;	/* Appletalk protocol ID */
		} APPLETALK;
	};
} NetAddr_t;

typedef struct NetPacket_t {
	NetAddr_t Origin;		/* Who sent this packet? */
	Word32 Length;		/* Length of data */
	Word8 *Data;				/* Data coming in */
} NetPacket_t;

typedef struct NetHandle_t NetHandle_t;

extern Word BURGERCALL NetInit(void);
extern void BURGERCALL NetShutdown(void);
extern NetHandle_t * BURGERCALL NetHandleNewListenPacket(NetAddr_t *Input);
extern NetHandle_t * BURGERCALL NetHandleNewListenStream(NetAddr_t *Input,NetListenProc Proc);
extern NetHandle_t * BURGERCALL NetHandleNewConnect(NetAddr_t *Input,Word Timeout);
extern void BURGERCALL NetHandleDelete(NetHandle_t *Input);
extern Word BURGERCALL NetHandleIsItClosed(NetHandle_t *Input);
extern long BURGERCALL NetHandleRead(NetHandle_t *Input,void *Buffer,long BufSize);
extern long BURGERCALL NetHandleWrite(NetHandle_t *Input,const void *Buffer,long Length,Word BlockFlag);
extern Word BURGERCALL NetHandleSendPacket(NetHandle_t *Input,NetAddr_t *DestAddr,const void *Buffer,Word Length);
extern NetPacket_t * BURGERCALL NetHandleGetPacket(NetHandle_t *Input);
extern Word BURGERCALL NetStringToTCPAddress(NetAddr_t *Output,const char *TCPName);
extern void BURGERCALL NetAddressToString(char *Output,Word Size,NetAddr_t *Input,Word PortFlag);
extern Word BURGERCALL NetAddressCompare(const NetAddr_t *First,const NetAddr_t *Second);
extern Word BURGERCALL NetIsProviderPresent(NetProvider_e Provider);
extern NetHandle_t * BURGERCALL NetGetPacketSendHandle(NetProvider_e Provider);
extern void BURGERCALL NetGetPeerAddress(NetAddr_t *Output,NetHandle_t *Input);
extern void BURGERCALL NetGetLocalAddress(NetAddr_t *Output,NetHandle_t *Input);
extern NetHandle_t * BURGERCALL NetFindHandleFromSocket(Word32 Socket);
extern NetHandle_t * BURGERCALL NetFindHandleByMode(SocketMode_e Mode);
extern void BURGERCALL NetAddHandleToList(NetHandle_t *Input);
#if defined(__WIN32__)
extern void BURGERCALL Win95WinSockToNetAddress(NetAddr_t *Output,struct sockaddr *Input);
extern void BURGERCALL Win95NetToWinSockAddress(struct sockaddr *Output,NetAddr_t *Input);
#elif defined(__MAC__)
extern void BURGERCALL MacOTToNetAddress(NetAddr_t *Output,struct OTAddress *Input);
extern void BURGERCALL MacNetToOTAddress(struct OTAddress *Output,NetAddr_t *Input);
#endif

/* IBM Specific convience routines */

#if defined(__MSDOS__)
/*********************************

	This is used by the x32 dos extender so I can address
	low memory in an IBM PC

*********************************/

extern void MCGAOn(void);
extern void MCGAOff(void);
extern Word SVGAOn(void);
extern void UpdateSVGA(void *Offscreen);
extern void UpdateSVGARectList(void *Offscreen,LBRectList *RectList);

typedef struct Regs16 {
	Word16 ax;
	Word16 bx;
	Word16 cx;
	Word16 dx;
	Word16 si;
	Word16 di;
	Word16 bp;
	Word16 ds;
	Word16 es;
	Word16 flags;
} Regs16;

extern Word BURGERCALL AreLongFilenamesAllowed(void);
extern int BURGERCALL Int86x(Word InterNum,Regs16 *InRegs,Regs16 *OutRegs);
extern int BURGERCALL Call86(Word32 Address,Regs16 *InRegs,Regs16 *OutRegs);
extern void CallInt10(Word EAX);
extern Word Int14(Word EAX,Word EDX);
extern Word Int17(Word EAX,Word EDX);
extern void SetBothInts(Word IrqNum,void far *CodePtr);
extern void SetProtInt(Word IrqNum,void far *CodePtr);
extern void SetRealInt(Word IrqNum,Word32 CodePtr);
extern void far *GetProtInt(Word IrqNum);
extern Word32 GetRealInt(Word IrqNum);
extern void *MapPhysicalAddress(void *Input,Word32 Length);
extern void BURGERCALL Win95AppendFilenameToDirectory(char *Directory,const char *Filename);
#endif

/**********************************

	Win 95 specific functions

**********************************/

#if defined(__WIN32__)
typedef struct Direct3DTexture_t {
	struct IDirectDrawSurface *Surface;		/* Pointer to the DirectDraw Surface */
	struct IDirect3DTexture *TexturePtr;	/* Pointer to the Direct3D Texture reference */
	Word32 TextureHandle;					/* The Direct3D texture handle for rendering */
} Direct3DTexture_t;

extern Word TickWakeUpFlag;		/* TRUE if I should get tick events */
extern Word Win95NoProcess;		/* Shut down ProcessSystemEvents */
extern Word Win95MouseButton;	/* Mouse button contents */
extern Word Win95MouseX;		/* Virtual mouse X */
extern Word Win95MouseY;		/* Virtual mouse Y */
extern int Win95MouseWheel;		/* Virtual mouse Z */
extern int Win95MouseXDelta;	/* Mouse motion */
extern int Win95MouseYDelta;
extern int Win95LastMouseX;		/* Previous mouse position for delta motion */
extern int Win95LastMouseY;
extern int Win95DestScreenX;	/* X coord to draw screen to */
extern int Win95DestScreenY;	/* Y coord to draw screen to */
extern void *Win95MainWindow;	/* Main window to perform all operations on */
extern struct IDirectDrawSurface *Win95FrontBuffer;	/* Currently displayed screen */
extern struct IDirectDrawSurface *Win95BackBuffer;	/* My work buffer */
extern struct IDirectDrawSurface *Win95WorkBuffer;	/* Which buffer am I using? */
extern struct IDirectDrawSurface *Direct3DZBufferPtr;		/* ZBuffer for 3D */
extern struct IDirectDrawPalette *Win95WindowPalettePtr;	/* Pointer to game palette */
extern struct IDirectDraw *DirectDrawPtr;			/* Pointer to direct draw instance */
extern struct IDirectDraw2 *DirectDraw2Ptr;			/* Pointer to the direct draw 2 instance */
extern struct IDirectDraw4 *DirectDraw4Ptr;			/* Pointer to the direct draw 4 instance */
extern struct IDirectDrawClipper *Win95WindowClipperPtr;	/* Clipper for primary surface */
extern Word Direct3DZBufferBitDepth;					/* Bits per pixel for D3D ZBuffer */
extern Word Win95ApplsActive;	/* True if the application is active */
extern Word BURGERCALL Win95ProcessMessage(struct tagMSG *MessagePtr);
extern void BURGERCALL ProcessSystemEvents(void);
extern Word8 *Win95VideoPointer;	/* Locked videopointer for hardware blits */
extern void *Win95Instance;		/* HINSTANCE of the current app */
extern Bool Win95UseBackBuffer;	/* True if the backbuffer is used */
extern void *Win95LockedMemory;		/* Copy of the video pointer */
extern void *BurgerHBitMap;			/* HBITMAP for window mode */
extern Word8 *BurgerHBitMapPtr;		/* Pointer to HBITMAP Memory */
extern void *BurgerHPalette;		/* HPALETTE for window mode */

extern Word BURGERCALL InitWin95Window(const char *Title,void *Instance,long (__stdcall *Proc)(struct HWND__*,Word,Word,long));
extern Word BURGERCALL GetDirectXVersion(void);
extern void BURGERCALL Win95AppendFilenameToDirectory(char *Directory,const char *Filename);
extern Word BURGERCALL Win95AddGroupToProgramMenu(const char *GroupName);
extern Word BURGERCALL Win95DeleteGroupFromProgramMenu(const char *GroupName);
extern Word BURGERCALL Win95Allow1Instance(const char *LockName);
extern long BURGERCALL CallDirectDrawCreate(struct _GUID *lpGUID,struct IDirectDraw **lplpDD,void *pUnkOuter);
extern long BURGERCALL CallDirectDrawEnumerateA(int (__stdcall *lpCallback)(struct _GUID *,char *,char *,void*),void *lpContext);
extern long BURGERCALL CallDirectDrawEnumerateExA(int (__stdcall *)(struct _GUID *,char*,char*,void*,void *),void *lpContext,Word32 Flags);
extern Word BURGERCALL Win95VideoGetGuid(struct _GUID **OutPtr,struct _GUID *Output,Word DevNum);

extern Word BURGERCALL Direct3DTextureInitImage(Direct3DTexture_t *Input,const struct Image_t *ImagePtr);
extern Direct3DTexture_t * BURGERCALL Direct3DTextureNewImage(const struct Image_t *ImagePtr);
extern void BURGERCALL Direct3DTextureDelete(Direct3DTexture_t *Input);
extern void BURGERCALL Direct3DTextureDestroy(Direct3DTexture_t *Input);
extern void BURGERCALL Direct3DTextureDraw2D(Direct3DTexture_t *Input,int x,int y,Word Width,Word Height);
extern void BURGERCALL Direct3DTextureDraw2DSubRect(Direct3DTexture_t *Input,int x,int y,Word Width,Word Height,const float *UVPtr);

#endif

/**********************************

	Mac OS specific functions

**********************************/

#if defined(__MAC__) || defined(__MACOSX__)
#if !defined(__MACOSX__)
typedef Word (BURGERCALL *MacEventInterceptProcPtr)(struct EventRecord *MyEventPtr);

extern struct GDevice **VideoDevice;	/* Video device for graphic output */
#if !TARGET_API_MAC_CARBON
extern struct CGrafPort *VideoGWorld;	/* Grafport to offscreen buffer */
extern struct GrafPort *VideoWindow;	/* Window to display to */
#define MacDialogRef struct GrafPort *
#else
extern struct OpaqueGrafPtr *VideoGWorld;	/* Grafport to offscreen buffer */
extern struct OpaqueWindowPtr *VideoWindow;	/* Window to display to */
#define MacDialogRef struct OpaqueDialogPtr*
#endif
#if TARGET_RT_MAC_CFM
extern struct OpaqueDSpContextReference *MacContext;	/* Reference to a reserved DrawSprocket reference */
extern Bool MacDrawSprocketActive;	/* Has draw sprocket been started up? */
#endif
extern Bool MacUseBackBuffer;		/* Which buffer is active */
extern Bool MacSystemTaskFlag;		/* If TRUE, then SystemTask() is called */
extern MacEventInterceptProcPtr MacEventIntercept;	/* Intercept for DoMacEvent() */
extern short MacVRefNum;		/* Volume reference used by Mac OS */
extern long MacDirID;			/* Directory reference used by MacOS */
extern short MacVRefNum2;		/* Volume reference used by copy and rename */
extern long MacDirID2;			/* Directory reference used by copy and rename */
extern short MacCacheVRefNum;	/* Cached VRef num for currently logged directory */
extern long MacCacheDirID;		/* Cached Dir ID for currently logged directory */
extern Word BURGERCALL MacMakeOffscreenGWorld(Word Width,Word Height,Word Depth,Word Flags);
extern void * BURGERCALL GetFullPathFromMacID(long dirID,short vRefNum);
extern short BURGERCALL OpenAMacResourceFile(const char *PathName,char Permission);
extern Word BURGERCALL CreateAMacResourceFile(const char *PathName);
extern char *BURGERCALL GetFullPathFromMacFSSpec(const struct FSSpec *Input);
extern void BURGERCALL MacOSFileSecondsToTimeDate(struct TimeDate_t *Output,Word32 Time);
extern void BURGERCALL MacOSPurgeDirCache(void);
extern Word BURGERCALL DoMacEvent(Word Mask,struct EventRecord *MyRecord);
extern Word BURGERCALL FixMacKey(struct EventRecord *MyRecord);
extern Word BURGERCALL MacOSIsTrapAvailable(int TrapNum);
extern Word BURGERCALL MacOSIsQuickTimePowerPlugAvailable(void);
extern Word BURGERCALL MacOSGetSoundManagerVersion(void);
extern Word BURGERCALL MacOSGetInputSprocketVersion(void);
extern Word BURGERCALL MacOSGetDrawSprocketVersion(void);
extern Word BURGERCALL MacOSGetAppleShareVersion(void);
extern void BURGERCALL MacOSSetExtensionsPrefix(Word PrefixNum);
extern long BURGERCALL MacOpenFileForRead(const char *Filename);
extern Word BURGERCALL MacOpenControlPanel(Word32 type,const char *defaultname,Word Continue);
extern Word BURGERCALL MacLaunch(short vref,long dirID, const char *name,Word Continue);
extern void BURGERCALL MacOSKillProcess(struct ProcessSerialNumber *victim);
extern void BURGERCALL MacOSKillAllProcesses(void);
extern struct LibRef_t * BURGERCALL MacOSGetInterfaceLib(void);
extern struct LibRef_t * BURGERCALL MacOSDriverLoaderLib(void);
extern Word BURGERCALL MacOSDialogControlGetValue(MacDialogRef dialog,Word item);
extern void BURGERCALL MacOSDialogControlSetValue(MacDialogRef dialog,Word item,Word Value);
extern void BURGERCALL MacOSDialogControlToggleValue(MacDialogRef dialog,Word item);
#if TARGET_API_MAC_CARBON
extern void * BURGERCALL MacOSMachOToCFM(void *ProcPtr);
extern void BURGERCALL MacOSMachOToCFMDelete(void *MachPtr);
#endif
#endif

extern void BURGERCALL MacOSInitTools(void);
extern void BURGERCALL MacOSEjectCD(void);
extern Word BURGERCALL MacOSIsAltivecPresent(void);
extern Word BURGERCALL MacOSGetOSVersion(void);

#if TARGET_API_MAC_CARBON || defined(__MACOSX__)

typedef struct MacOSXFramework_t {	/* Data for a MacOSX library reference */
	struct __CFBundle *LibBundle;	/* Master bundle reference */
} MacOSXFramework_t;

extern MacOSXFramework_t *BURGERCALL MacOSGetFoundationFramework(void);
extern Word BURGERCALL MacOSXFrameworkInit(MacOSXFramework_t *Input,const char *FrameWorkName);
extern void * BURGERCALL MacOSXFrameworkGetProc(MacOSXFramework_t *Input,const char *ProcNam);
extern void BURGERCALL MacOSXFrameworkDestroy(MacOSXFramework_t *Input);
extern void BURGERCALL MacOSXTimeDateFromUTCTime(struct TimeDate_t *Output,const struct UTCDateTime *Input);
#endif

#endif

#if defined(__INTEL__) && !defined(__BEOS__)
#include <poppack.h>
#elif defined(__MWERKS__)
#pragma options align=reset
#endif

#if defined(__MWERKS__)
#pragma ANSI_strict reset
#pragma enumsalwaysint reset
#endif

#ifdef __cplusplus
}
#endif

#endif

