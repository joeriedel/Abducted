/**********************************

	64 bit type manager

**********************************/

#ifndef __LL64BIT_H__
#define __LL64BIT_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LONGWORD64NATIVE
#if defined(__MWERKS__) || defined(__MRC__) || defined(__GNUC__)
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

#ifdef __cplusplus
}
#endif


#endif

