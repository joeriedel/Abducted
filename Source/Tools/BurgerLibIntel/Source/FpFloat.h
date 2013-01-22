/*******************************

	Floating point manager

*******************************/

#ifndef __FPFLOAT_H__
#define __FPFLOAT_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Private */

struct FixedVector3D_t;
struct FixedMatrix3D_t;

extern Word32 BurgerSqrtTable[512*2];

/* Public */

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

#ifdef __cplusplus
}
#endif


#endif

