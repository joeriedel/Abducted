/**********************************

	Calls to the integer math toolbox
	This is COMPLETELY machine independent

**********************************/

#include "FxFixed.h"
#include "FpFloat.h"
#include "PfPrefs.h"
#include "Ll64Bit.h"
#include <stdlib.h>

/******************************

	Return the low 16 bits of a 32 bit unsigned long

******************************/

Word BURGERCALL IMLoWord(Word32 Input)
{
	return (Word16)Input;		/* Return just the low 16 bits */
}

/****************************

	Return the hi word of a 32 bit number (Unsigned)

****************************/

Word BURGERCALL IMHiWord(Word32 Input)
{
	return Input>>16;
}

/**********************************

	Convert a Word16 into a Word32
	that contains the ASCII value of the short...
	The result is BIG Endian so that it can be directly stored
	into an ASCII buffer without an Endian swap

	Input : 0x1234
	Output : '1','2','3','4'

**********************************/

Word32 BURGERCALL IMHexIt(Word Val)
{
	Word32 Result;
	Word i;

	i = 4;			/* 4 digits to process */
	Result = 0;		/* Init the result */
	do {
#if defined(__BIGENDIAN__)	/* For Motorola processors */
		Word32 Temp;
		Temp = NibbleToAscii[Val&0x0F];	/* Isolate a hex digit */
		Temp <<= 24;		/* Shift to the upper area */
		Result >>= 8;		/* Shift down the result */
#else
		Word Temp;
		Temp = NibbleToAscii[Val&0x0F];	/* Isolate a hex digit */
		Result <<= 8;		/* Make room for the ASCII */
#endif
		Result = Result|Temp;	/* Make the new result */
		Val>>=4;			/* Remove the low 4 bits */
	} while (--i);		/* All done? */
	return Result;		/* Exit */
}


/**********************************

	Multiply 2 Shorts and
	return the Word32 result

**********************************/

Word32 BURGERCALL IMMultiply(Word InputA,Word InputB)
{
	return InputA*InputB;
}

/**********************************

	Calculate the square root of a 32 bit unsigned
	value. The maximum value is 46341 for the square root
	of 0x7FFFFFFF. This routine is 100% accurate.

**********************************/

#if !defined(__INTEL__) && !defined(__POWERPC__)
Word BURGERCALL IMIntSqrt(Word32 x)
{
	Word32 r, nr;

	/* This is simplified since most are constants */

	r = 0;
	if (x>=0x40000000) {
		r = 0x40000000;
		x -= 0x40000000;
	}

	/* Here is where the fun begins */

	nr = r + 0x10000000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x10000000;
		x -= nr;
	}

	nr = r + 0x4000000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x4000000;
		x -= nr;
	}

	nr = r + 0x1000000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x1000000;
		x -= nr;
	}

	nr = r + 0x400000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x400000;
		x -= nr;
	}

	nr = r + 0x100000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x100000;
		x -= nr;
	}

	nr = r + 0x40000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x40000;
		x -= nr;
	}

	nr = r + 0x10000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x10000;
		x -= nr;
	}

	nr = r + 0x4000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x4000;
		x -= nr;
	}

	nr = r + 0x1000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x1000;
		x -= nr;
	}

	nr = r + 0x400;
	r>>= 1;
	if (x>=nr) {
		r =r+0x400;
		x -= nr;
	}

	nr = r + 0x100;
	r>>= 1;
	if (x>=nr) {
		r =r+0x100;
		x -= nr;
	}

	nr = r + 0x40;
	r>>= 1;
	if (x>=nr) {
		r =r+0x40;
		x -= nr;
	}

	nr = r + 0x10;
	r>>= 1;
	if (x>=nr) {
		r =r+0x10;
		x -= nr;
	}

	nr = r + 0x4;
	r>>= 1;
	if (x>=nr) {
		r =r+0x4;
		x -= nr;
	}

	nr = r + 0x1;
	r>>= 1;
	if (x>=nr) {
		r =r+0x1;
		x -= nr;
	}

	/* Big finish! */

	if (x>r) {
		r += 1;
	}
	return r;
}
#endif

/**********************************

	Calculate the square root of a 32 bit unsigned
	value. The input and output are 16.16 fixed
	point. This routine is 100% accurate.

**********************************/

#if !defined(__INTEL__) && !defined(__POWERPC__)
Word32 BURGERCALL IMFixSqrt(Word32 x)
{
	Word32 r, nr;

	/* This is simplified since most are constants */

	r = 0;
	if (x>=0x40000000) {
		r = 0x40000000;
		x -= 0x40000000;
	}

	/* Here is where the fun begins */

	nr = r + 0x10000000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x10000000;
		x -= nr;
	}

	nr = r + 0x4000000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x4000000;
		x -= nr;
	}

	nr = r + 0x1000000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x1000000;
		x -= nr;
	}

	nr = r + 0x400000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x400000;
		x -= nr;
	}

	nr = r + 0x100000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x100000;
		x -= nr;
	}

	nr = r + 0x40000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x40000;
		x -= nr;
	}

	nr = r + 0x10000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x10000;
		x -= nr;
	}

	nr = r + 0x4000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x4000;
		x -= nr;
	}

	nr = r + 0x1000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x1000;
		x -= nr;
	}

	nr = r + 0x400;
	r>>= 1;
	if (x>=nr) {
		r =r+0x400;
		x -= nr;
	}

	nr = r + 0x100;
	r>>= 1;
	if (x>=nr) {
		r =r+0x100;
		x -= nr;
	}

	nr = r + 0x40;
	r>>= 1;
	if (x>=nr) {
		r =r+0x40;
		x -= nr;
	}

	nr = r + 0x10;
	r>>= 1;
	if (x>=nr) {
		r =r+0x10;
		x -= nr;
	}

	nr = r + 0x4;
	r>>= 1;
	if (x>=nr) {
		r =r+0x4;
		x -= nr;
	}

	nr = r + 0x1;
	r>>= 1;
	if (x>=nr) {
		r =r+0x1;
		x -= nr;
	}

	/* At this point, I've got the integer square root */
	/* I need another 16 bits of precision */

	r <<= 16;
	x <<= 16;

	/* Now, iterate another 8 times for the final 16 bits */

	nr = r + 0x4000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x4000;
		x -= nr;
	}

	nr = r + 0x1000;
	r>>= 1;
	if (x>=nr) {
		r =r+0x1000;
		x -= nr;
	}

	nr = r + 0x400;
	r>>= 1;
	if (x>=nr) {
		r =r+0x400;
		x -= nr;
	}

	nr = r + 0x100;
	r>>= 1;
	if (x>=nr) {
		r =r+0x100;
		x -= nr;
	}

	nr = r + 0x40;
	r>>= 1;
	if (x>=nr) {
		r =r+0x40;
		x -= nr;
	}

	nr = r + 0x10;
	r>>= 1;
	if (x>=nr) {
		r =r+0x10;
		x -= nr;
	}

	nr = r + 0x4;
	r>>= 1;
	if (x>=nr) {
		r =r+0x4;
		x -= nr;
	}

	nr = r + 0x1;
	r>>= 1;
	if (x>=nr) {
		r =r+0x1;
		x -= nr;
	}

	/* Big finish! */

	if (x>r) {
		r += 1;
	}
	return r;
}
#endif

/**********************************

	Multiply two 32 bit values and divide
	the 64 bit result by a third value. This way I
	get a high accuracy ratio of Value*(Numerator/Denominator)

**********************************/

#if !defined(__INTEL__) && !defined(__68K__) && !defined(__POWERPC__)
long BURGERCALL IMIntMulRatioFast(long Mul1,long Mul2,long Div)
{
	LongWord64_t Foo;
	LongWord64MulLongTo64(&Foo,Mul1,Mul2);
	LongWord64DivideByLong(&Foo,Div);
	return LongWord64ToLong(&Foo);
}
#endif

/******************************

	Mul two pairs of 32 bit numbers and return the upper 32 bits

******************************/

#if !defined(__INTEL__) && !defined(__POWERPC__)
long BURGERCALL IntDblMulAdd(long Mul1,long Mul2,long Mul3,long Mul4)
{
	LongWord64_t Temp,Temp2;
	LongWord64MulLongTo64(&Temp,Mul1,Mul2);
	LongWord64MulLongTo64(&Temp2,Mul1,Mul2);
	LongWord64Add(&Temp,&Temp2);
	return LongWord64ToHiLong(&Temp);	/* Return the value */
}
#endif

/******************************

	Mul two 32 bit numbers and return the upper 32 bits

******************************/

#if !defined(__INTEL__) && !defined(__POWERPC__)
long BURGERCALL IntMulHigh32(long Mul1,long Mul2)
{
	LongWord64_t Temp;
	LongWord64MulLongTo64(&Temp,Mul1,Mul2);
	return LongWord64ToHiLong(&Temp);	/* Return the value */
}
#endif

/**********************************

	Multiply two 16.16 fixed point numbers
	Handle overflow

**********************************/

#if !defined(__POWERPC__) && !defined(__INTEL__)

Fixed32 BURGERCALL IMFixMul(Fixed32 Val1,Fixed32 Val2)
{
	Fixed32 Flag;

#if defined(__MAC__)
	register Word Sixteen;
	Sixteen = 16;
#else
#define Sixteen 16
#endif

	Flag = Val1^Val2;	/* Save the flag for the result sign */
	Val1 = abs(Val1);	/* Get the absolute value */
	Val2 = abs(Val2);	/* Get the absolute value */

	if ((Word32)Val1<65536UL) {
		if ((Word32)Val2<65536UL) {		/* 16 * 16 */
			Val1 = (Word32)Val1*(Word32)Val2;	/* Do the mul */
			Val1 = (Word32)Val1>>Sixteen;		/* Convert to fixed point */
			if (Flag>=0) {		/* Should I convert */
				return Val1;
			}
			return -Val1;		/* Return the result */
		}
		{
		Word32 Temp;
		/* 16 * 32 */
		Temp = (Word16)Val2;
		Temp = Temp*(Word32)Val1;	/* Get fraction result */
		Val2 = ((Word32)Val2) >> Sixteen;
		Val1 = (Word32)Val1*(Word32)Val2;		/* Mul to a fixed point number */
		if (Flag>=0) {
			Temp>>=Sixteen;
			return Val1 + Temp;
		}
		if ((Word16)Temp) {
			++Val1;
		}
		Temp>>=Sixteen;
		Val1 = Val1+Temp;
		return -Val1;
		}
	}
	if ((Word32)Val2<65536UL) {		/* 32 * 16 */
		Word32 Temp;
		Temp = (Word16)Val1;
		Temp = Temp*(Word32)Val2;	/* Get fraction result */
		Val1 = ((Word32)Val1)>>Sixteen;
		Val1 = (Word32)Val1*(Word32)Val2;		/* Mul to a fixed point number */
		if (Flag>=0) {
			Temp>>=Sixteen;
			return Val1+Temp;
		}
		if ((Word16)Temp) {
			++Val1;
		}
		Temp>>=Sixteen;
		Val1 = Val1+Temp;
		return -Val1;
	}
	{
		Word32 Val1Low,Val2Low;
		Word32 Temp,Temp2;

		Val1Low = (Word16)Val1;		/* Get the low 16 bits */
		Val2Low = (Word16)Val2;
		Val1 = ((Word32)Val1)>>Sixteen;	/* Get the integer of the the inputs */
		Val2 = ((Word32)Val2)>>Sixteen;
		Temp2 = Val1;				/* Init the result */
		Temp2 = Temp2*(Word32)Val2;		/* 16.0 * 16.0 */
		if (Temp2>=0x8000) {		/* Too large a result? */
			goto Overflow;
		}
		Temp2 = Temp2<<Sixteen;		/* Convert to integer */
		Temp = Val1Low;
		Temp = Temp*Val2Low;		/* 0.16 * 0.16 */
		Val2 = (Word32)Val2*Val1Low;	/* 0.16 * 16.0 */
		Val1 = (Word32)Val1*Val2Low;	/* 16.0 * 0.16 */
		Temp2 = Temp2+Val2;
		Temp2 = Temp2+Val1;
		if (Flag>=0) {
			Temp>>=Sixteen;
			return Temp2+Temp;
		}
		if ((Word16)Temp) {
			++Temp2;
		}
		Temp>>=Sixteen;
		Temp2 = Temp2+Temp;
		return -Temp2;
	}
Overflow:
	if (Flag>=0) {
		return 0x7FFFFFFF;
	}
	return 0x80000000;
}

#endif

/**********************************

	Multiply a fixed point number without error checking

**********************************/

#if !defined(__POWERPC__) && !defined(__INTEL__)
Fixed32 IMFixMulFast(Fixed32 a,Fixed32 b)
{
	LongWord64_t Foo;
	LongWord64MulLongTo64(&Foo,a,b);
	return Foo>>16;
}
#endif


/**********************************

	Divide a 16:16 fixed point number by
	another 16:16 fixed point number

	I handle overflow and divide by zero

**********************************/

#if !defined(__POWERPC__) && !defined(__INTEL__)

Fixed32 BURGERCALL IMFixDiv(Fixed32 Numerator,Fixed32 Denominator)
{
	Fixed32 Sign;
	Fixed32 Result;
	Word32 Test;
	Word i;

	Sign = Numerator^Denominator;		/* Get the result's sign */
	Numerator = abs(Numerator);			/* I use absolute values */
	Denominator = abs(Denominator);
	if ((((Word32)Numerator)>>15)<((Word32)Denominator)) {	/* Detect overflow */
		i = 32;							/* Create 32 bits of fun */
		Result = 0;						/* Assume good result */
		Test = ((Word32)Numerator)>>16;	/* Grab the first 16 bits */
		Numerator = ((Word32)Numerator)<<16;	/* This way I do a 48 bit divide */
		do {
			Result = Result<<1;		/* Shift up the answer */
			Test = Test<<1;			/* Shift up the modulo */
			if (Numerator&0x80000000) {	/* Rol the value (64 bits) */
				++Test;				/* Shift it in */
			}
			Numerator = ((Word32)Numerator)<<1;	/* Zap the bit */
			if (Test>=(Word32)Denominator) {	/* 1 bit divide */
				Test-=Denominator;		/* Adjust the modulo */
				++Result;				/* Set the low bit */
			}
		} while (--i);		/* All 32 bits done? */
		if (Sign>=0) {		/* Positive answer? */
			return Result;	/* Return as is.. */
		}
		return -Result;	/* Negate the answer */
	}
	if (Sign<0) {		/* Overflow negative? */
		return 0x80000000;
	}
	return 0x7FFFFFFF;	/* Or positive? */
}

#endif

/**********************************

	Divide a 16.16 fixed point number
	without bounds checking.

	Note : This is a dangerous routine. A divide
	by zero or an overflow can cause an exception error!
	Use at your own risk!

**********************************/

#if !defined(__POWERPC__) && !defined(__INTEL__)
Fixed32 IMFixDivFast(Fixed32 a,Fixed32 b)
{
	LongWord64_t Foo;
	Foo = ((LongWord64_t)a<<16)/(LongWord64_t)b;
	return Foo>>16;
}
#endif

/**********************************

	Convert a fixed point number into the
	equivalent integer

**********************************/

int BURGERCALL IMFixRound(Fixed32 Input)
{
	if (Input&0x8000) {
		Input+=0x10000;
	}
	return Input>>16;
}

/**************************

	Convert a signed long into a 32 bit 16.16 fixed point
	number.
	If the converted number is out of bounds, then it will be set
	to the maximum allowed (0x7FFFFFFF if positive or 0x80000000 if
	negative).

**************************/

Fixed32 BURGERCALL IMLong2Fix(long Input)
{
	if (Input<0x8000) {
		if (Input>=-0x8000) {
			return Input>>16;
		}
		return 0x80000000;
	}
	return 0x7FFFFFFF;
}

/***************************

	Return the reciprocal of a fixed point number

***************************/

#if !defined(__POWERPC__) && !defined(__INTEL__)
Fixed32 BURGERCALL IMFixReciprocal(Fixed32 Input)
{
	if (Input!=-1) {
		if ((Word32)Input>=2) {		/* Prevent a divide by zero */
			Word32 Foo;
			Foo = abs(Input);
			Foo = 0x80000000UL/(Foo>>1);
			if (Input<0) {
				return -Foo;
			}
			return Foo;
		}
		return 0x7FFFFFFF;
	}
	return 0x80000000;
}
#endif

/**********************************

	Get the arctangent from the slope of two inputs

**********************************/

#define SIGNED 1
#define REVERSE 2
#define HALF 4

Fixed32 BURGERCALL IMFixATan2(long Input1,long Input2)
{
	Frac32 Result;
	Word Flags;

	Flags = 0;

	if (Input1<0) {		/* Get the absolute value */
		Flags |= REVERSE;	/* Sub result from Pi */
		Input1 = -Input1;
	}
	if (Input2<0) {		/* Get the absolute value */
		Flags = SIGNED;	/* Negate the result */
		Input2 = -Input2;
	}
	if (Input1>=Input2) {	/* Is input 1 larger? */
		Flags |= HALF;
	}
	Result = 0;		/* Assume zilch */

	if (Input1 && Input2) {	/* Both must be non zero */
		if (Flags&HALF) {		/* Divide from the smaller */
			Input2 = IMFracDiv(Input2,Input1);
		} else {
			Input2 = IMFracDiv(Input1,Input2);
		}
		Input1 = IMFracMul(Input2,Input2);		/* Get the square of the fraction */
		Result = IMFracMul(Input1,0xFF3FFE62)+0x035E92FE;
		Result = IMFracMul(Input1,Result)+0xF88C77F2;
		Result = IMFracMul(Input1,Result)+0x0C62F72C;
		Result = IMFracMul(Input1,Result)+0xEAB64EBE;
		Result = IMFracMul(Input1,Result)+0x3FFFA073;
		Input1 = IMFracMul(Input2,Result);
		Result = Input1>>14;		/* Use the upper 18 bits */
		if (Input1&(1<<13)) {		/* Round up */
			++Result;
		}
	}
	if (!(Flags&HALF)) {		/* Alter by Pi */
		Result = ((PiFixed/2)-1)-Result;
	}
	if (Flags&REVERSE) {		/* Reverse from Pi */
		Result = PiFixed-Result;
	}
	if (Flags&SIGNED) {		/* Negate the result */
		Result = -Result;
	}
	return Result;		/* I have the answer */
}

/**********************************

	Actually perform the sine and cosine calculations
	The skew uses only the last 3 bits.
	Bit #0 determines either an moving towards 45 degrees (0) or a away
		from 45 degrees (1)
	Bit #1 determines whether I am going from 0 to 90 degrees (0) or
		90 to 180 degrees (1)
	Bit #2 determine if this should be a positive result (0) or a
		negative result (1)

	All other bits are don't cares since a sine/cosine formula generates
	a repeating sine wave.

**********************************/

static Frac32 BURGERCALL DoSine(Fixed32 Value,Word Skew)
{
	Frac32 Result;
	Fixed32 Square;

	Square = Value/(PiFixed/4);		/* First get the integer for rounding */

/* Now get the actual angle (Degrees mod 360) */

	Value = Value - ((PiFixed/4)*Square);	/* Poor man's modulo */
	Skew += Square;		/* Adjust the skew based on the integer from division */

	if (Skew&1) {		/* Reverse skew? */
		Value = (PiFixed/4)-Value;		/* Reverse the input value (45 degrees) */
	}
	Value <<= 13;		/* Convert to frac */
	Square = IMFracMul(Value,Value);	/* Get the square */
	if ((Skew+1)&2) {		/* Am I skewed towards the peak? */

	/* This formula will handle 45-135 degrees sine values */

		Result = IMFracMul(Square,0xFA6E2A42)+0x2AA7F29A;
		Result = IMFracMul(Result,Square)+0x800011A7;
		Result = IMFracMul(Result,Square)+0x40000000;
	} else {

	/* This formula will handle 0-45 and 135-180 degrees sine values */

		Result = IMFracMul(Square,0x10A208E5)+0xAAB3314D;
		Result = IMFracMul(Result,Square)+0x7FFFD609;
		Result = IMFracMul(Result,Value);
	}
	if (Skew&4) {		/* Negate the result? */
		Result = -Result;	/* Negate it! */
	}
	return Result;		/* Return the answer */
}

/**********************************

	Calculate a cosine from a input in fixed
	radians.
	Returns the cosine in a Frac32

**********************************/

Frac32 BURGERCALL IMFracCos(Fixed32 Value)
{
	if (Value<0) {		/* Force a positive value */
		Value = -Value;
	}
	return DoSine(Value,2);	/* Skew the result by 180 degrees */
}

/**********************************

	Calculate a sine from a input in fixed
	radians.
	Returns the sine in a Frac32

**********************************/

Frac32 BURGERCALL IMFracSin(Fixed32 Value)
{
	if (Value<0) {		/* Less than zero? */
		return DoSine(-Value,4);	/* Force positive and negate the result */
	}
	return DoSine(Value,0);		/* Allow the formula to go ahead */
}

/**********************************

	Multiply two 2.30 fixed point numbers

**********************************/

#if !defined(__INTEL__)
Fixed32 BURGERCALL IMFracMul(Frac32 Val1,Frac32 Val2)
{
	LongWord64_t a,b;

	a = Val1;
	b = Val2;
	a = a*b;

	if (a>=0x2000000000000000) {
		return 0x7FFFFFFF;
	}
	if (a<=-0x2000000000000000) {
		return 0x80000000;
	}
	return (a>>30);
}
#endif

/**********************************

	Divide a 2:30 fixed point number by
	another 2:30 fixed point number

	I handle overflow and divide by zero

**********************************/

#if !defined(__INTEL__)
Frac32 BURGERCALL IMFracDiv(Frac32 Numerator,Frac32 Denominator)
{
	Fixed32 Sign;
	Fixed32 Result;
	Word32 Test;
	Word i;

	Sign = Numerator^Denominator;		/* Get the result's sign */
	Numerator = abs(Numerator);			/* I use absolute values */
	Denominator = abs(Denominator);
	if ((((Word32)Numerator)>>1)<((Word32)Denominator)) {	/* Detect overflow */
		i = 32;							/* Create 32 bits of fun */
		Result = 0;						/* Assume good result */
		Test = ((Word32)Numerator)>>2;	/* Grab the first 16 bits */
		Numerator = ((Word32)Numerator)<<30;	/* This way I do a 48 bit divide */
		do {
			Result = Result<<1;		/* Shift up the answer */
			Test = Test<<1;			/* Shift up the modulo */
			if (Numerator&0x80000000) {	/* Rol the value (64 bits) */
				++Test;				/* Shift it in */
			}
			Numerator = ((Word32)Numerator)<<1;	/* Zap the bit */
			if (Test>=(Word32)Denominator) {	/* 1 bit divide */
				Test-=Denominator;		/* Adjust the modulo */
				++Result;				/* Set the low bit */
			}
		} while (--i);		/* All 32 bits done? */
		if (Sign>=0) {		/* Positive answer? */
			return Result;	/* Return as is.. */
		}
		return -Result;	/* Negate the answer */
	}
	if (Sign<0) {		/* Overflow negative? */
		return 0x80000000;
	}
	return 0x7FFFFFFF;	/* Or positive? */
}
#endif

/**********************************

	Convert a fixed point value to an int
	using the "C" standard for rounding
	(Round to zero)

**********************************/

int FixedToInt(Fixed32 Input)
{
	if (Input<0) {			/* Negative? */
		Input += 0xFFFF;	/* Push closer to zero */
	}
	return Input>>16;		/* Perform the conversion */
}

/**********************************

	The 3D vector code for fixed point vectors

**********************************/

/**********************************

	Init a FixedVector3D_t

**********************************/

void BURGERCALL FixedVector3DInit(FixedVector3D_t *Input,Fixed32 x,Fixed32 y,Fixed32 z)
{
	Input->x = x;
	Input->y = y;
	Input->z = z;
}

/**********************************

	Set the vector to zero

**********************************/

void BURGERCALL FixedVector3DZero(FixedVector3D_t *Output)
{
	Output->x = 0;
	Output->y = 0;
	Output->z = 0;
}

/**********************************

	Convert a floating point vector into fixed point

**********************************/

void BURGERCALL FixedVector3DFromVector3D(FixedVector3D_t *Output,const Vector3D_t *Input)
{
	Output->x = FLOATTOFIXED(Input->x);
	Output->y = FLOATTOFIXED(Input->y);
	Output->z = FLOATTOFIXED(Input->z);
}

/**********************************

	Convert a floating point vector into integers

**********************************/

void BURGERCALL IntVector3DFromVector3D(FixedVector3D_t *Output,const Vector3D_t *Input)
{
	Output->x = (int)Input->x;
	Output->y = (int)Input->y;
	Output->z = (int)Input->z;
}

/**********************************

	Negate a fixed vector

**********************************/

void BURGERCALL FixedVector3DNegate(FixedVector3D_t *Input)
{
	Input->x = -Input->x;
	Input->y = -Input->y;
	Input->z = -Input->z;
}

/**********************************

	Negate and copy a vector

**********************************/

void BURGERCALL FixedVector3DNegate2(FixedVector3D_t *Output,const FixedVector3D_t *Input)
{
	Output->x = -Input->x;
	Output->y = -Input->y;
	Output->z = -Input->z;
}

/**********************************

	Add two vectors

**********************************/

void BURGERCALL FixedVector3DAdd(FixedVector3D_t *Output,const FixedVector3D_t *Input)
{
	Output->x += Input->x;
	Output->y += Input->y;
	Output->z += Input->z;
}

/**********************************

	Add and copy two vectors

**********************************/

void BURGERCALL FixedVector3DAdd3(FixedVector3D_t *Output,const FixedVector3D_t *Input1,const FixedVector3D_t *Input2)
{
	Output->x = Input1->x+Input2->x;
	Output->y = Input1->y+Input2->y;
	Output->z = Input1->z+Input2->z;
}

/**********************************

	Subtract two vectors

**********************************/

void BURGERCALL FixedVector3DSub(FixedVector3D_t *Output,const FixedVector3D_t *Input)
{
	Output->x -= Input->x;
	Output->y -= Input->y;
	Output->z -= Input->z;
}

/**********************************

	Subtract and copy two vectors

**********************************/

void BURGERCALL FixedVector3DSub3(FixedVector3D_t *Output,const FixedVector3D_t *Input1,const FixedVector3D_t *Input2)
{
	Output->x = Input1->x - Input2->x;
	Output->y = Input1->y - Input2->y;
	Output->z = Input1->z - Input2->z;
}

extern void BURGERCALL FixedVector3DMul(FixedVector3D_t *Output,Fixed32 Val);
extern void BURGERCALL FixedVector3DMul3(FixedVector3D_t *Output,const FixedVector3D_t *Input,Fixed32 Val);
extern Word BURGERCALL FixedVector3DEqual(const FixedVector3D_t *Input1,const FixedVector3D_t *Input2);
extern Word BURGERCALL FixedVector3DEqualWithinRange(const FixedVector3D_t *Input1,const FixedVector3D_t *Input2,Fixed32 Range);
extern Fixed32 BURGERCALL FixedVector3DGetAxis(const FixedVector3D_t *Input,Word Axis);
extern void BURGERCALL FixedVector3DSetAxis(FixedVector3D_t *Output,Word Axis,Fixed32 Val);

/**********************************

	Perform a dot product

**********************************/

Fixed32 BURGERCALL FixedVector3DDot(const FixedVector3D_t *Input1,const FixedVector3D_t *Input2)
{
	return (IMFixMulFast(Input1->x,Input2->x)+
		IMFixMulFast(Input1->y,Input2->y)+
		IMFixMulFast(Input1->z,Input2->z));
}

/**********************************

	Perform a cross product

**********************************/

void FixedVector3DCross(FixedVector3D_t *Out,const FixedVector3D_t *In1,const FixedVector3D_t *In2)
{
	Out->x = IMFixMulFast(In1->y,In2->z) - IMFixMulFast(In1->z,In2->y);
	Out->y = IMFixMulFast(In1->z,In2->x) - IMFixMulFast(In1->x,In2->z);
	Out->z = IMFixMulFast(In1->x,In2->y) - IMFixMulFast(In1->y,In2->x);
}

extern Fixed32 BURGERCALL FixedVector3DGetRadiusSqr(const FixedVector3D_t *Input);
extern Fixed32 BURGERCALL FixedVector3DGetRadius(const FixedVector3D_t *Input);
extern Fixed32 BURGERCALL FixedVector3DGetRadiusFast(const FixedVector3D_t *Input);
extern void BURGERCALL FixedVector3DSetRadius(FixedVector3D_t *Input,Fixed32 Len);
extern void BURGERCALL FixedVector3DNormalize(FixedVector3D_t *Input);
extern void BURGERCALL FixedVector3DNormalizeFast(FixedVector3D_t *Input);
extern void BURGERCALL FixedVector3DNormalizeToLen(FixedVector3D_t *Input,Fixed32 Len);

/**********************************

	Code to handle a 3D Matrix

**********************************/

void BURGERCALL FixedMatrix3DZero(FixedMatrix3D_t *Input)
{
	Fixed32 Temp;
	Temp = 0;			/* Force the compiler to use a register */
	Input->x.x = Temp;	/* Fill all the elements with zero */
	Input->x.y = Temp;
	Input->x.z = Temp;
	Input->y.x = Temp;
	Input->y.y = Temp;
	Input->y.z = Temp;
	Input->z.x = Temp;
	Input->z.y = Temp;
	Input->z.z = Temp;
}

/**********************************

	Initialize a 3D matrix so that
	it is inert (No change for transformations)

**********************************/

void BURGERCALL FixedMatrix3DIdentity(FixedMatrix3D_t *Input)
{
	Input->x.x = 1<<16;	/* Leave X alone */
	Input->x.y = 0;
	Input->x.z = 0;
	Input->y.x = 0;
	Input->y.y = 1<<16;	/* Leave Y alone */
	Input->y.z = 0;
	Input->z.x = 0;
	Input->z.y = 0;
	Input->z.z = 1<<16;	/* Leave Z alone */
}

/**********************************

	Convert a floating point matrix into fixed point

**********************************/

void BURGERCALL FixedMatrix3DFromMatrix3D(FixedMatrix3D_t *Output,const Matrix3D_t *Input)
{
	Output->x.x = FLOATTOFIXED(Input->x.x);
	Output->x.y = FLOATTOFIXED(Input->x.y);
	Output->x.z = FLOATTOFIXED(Input->x.z);
	Output->y.x = FLOATTOFIXED(Input->y.x);
	Output->y.y = FLOATTOFIXED(Input->y.y);
	Output->y.z = FLOATTOFIXED(Input->y.z);
	Output->z.x = FLOATTOFIXED(Input->z.x);
	Output->z.y = FLOATTOFIXED(Input->z.y);
	Output->z.z = FLOATTOFIXED(Input->z.z);
}

extern void BURGERCALL FixedMatrix3DSet(FixedMatrix3D_t *Output,Fixed32 yaw,Fixed32 pitch,Fixed32 roll);
extern void BURGERCALL FixedMatrix3DSetYaw(FixedMatrix3D_t *Output,Fixed32 yaw);
extern void BURGERCALL FixedMatrix3DSetPitch(FixedMatrix3D_t *Output,Fixed32 pitch);
extern void BURGERCALL FixedMatrix3DSetRoll(FixedMatrix3D_t *Output,Fixed32 roll);

/**********************************

	Reverse a matrix (Flip it compeletly around)
	Note : x.x, y.y and z.z don't change

**********************************/

void BURGERCALL FixedMatrix3DTranspose(FixedMatrix3D_t *Input)
{
	Fixed32 Temp1,Temp2;
	Temp1 = Input->x.y;		/* Swap x.y and y.x */
	Temp2 = Input->y.x;
	Input->x.y = Temp2;
	Input->y.x = Temp1;

	Temp1 = Input->x.z;		/* Swap x.z and z.x */
	Temp2 = Input->z.x;
	Input->x.z = Temp2;
	Input->z.x = Temp1;

	Temp1 = Input->y.z;		/* Swap y.z and z.y */
	Temp2 = Input->z.y;
	Input->y.z = Temp2;
	Input->z.y = Temp1;
}

/**********************************

	Reverse a matrix (Flip it compeletly around)

**********************************/

void BURGERCALL FixedMatrix3DTranspose2(FixedMatrix3D_t *Output,const FixedMatrix3D_t *Input)
{
	Fixed32 Temp1,Temp2;

	Temp1 = Input->x.x;	/* I am copying the matrix, this */
	Temp2 = Input->y.x;	/* is why I am copying the x.x, y.y */
	Output->x.x = Temp1;	/* and z.z entries */
	Output->x.y = Temp2;

	Temp1 = Input->z.x;
	Temp2 = Input->x.y;
	Output->x.z = Temp1;
	Output->y.x = Temp2;

	Temp1 = Input->y.y;
	Temp2 = Input->z.y;
	Output->y.y = Temp1;
	Output->y.z = Temp2;

	Temp1 = Input->x.z;
	Temp2 = Input->y.z;
	Output->z.x = Temp1;
	Output->z.y = Temp2;

	Temp1 = Input->z.z;	/* Last one */
	Output->z.z = Temp1;
}

extern void BURGERCALL FixedMatrix3DMul(FixedMatrix3D_t *Output,const FixedMatrix3D_t *Input);
extern void BURGERCALL FixedMatrix3DMul2(FixedMatrix3D_t *Output,const FixedMatrix3D_t *Input1,const FixedMatrix3D_t *Input2);

/**********************************

	Return the X vector from a matrix

**********************************/

void BURGERCALL FixedMatrix3DGetXVector(FixedVector3D_t *Output,const FixedMatrix3D_t *Input)
{
	Output->x = Input->x.x;
	Output->y = Input->y.x;
	Output->z = Input->z.x;
}

/**********************************

	Return the Y vector from a matrix

**********************************/

void BURGERCALL FixedMatrix3DGetYVector(FixedVector3D_t *Output,const FixedMatrix3D_t *Input)
{
	Output->x = Input->x.y;
	Output->y = Input->y.y;
	Output->z = Input->z.y;
}

/**********************************

	Return the Z vector from a matrix

**********************************/

void BURGERCALL FixedMatrix3DGetZVector(FixedVector3D_t *Output,const FixedMatrix3D_t *Input)
{
	Output->x = Input->x.z;
	Output->y = Input->y.z;
	Output->z = Input->z.z;
}

extern void BURGERCALL FixedMatrix3DMulVector(FixedVector3D_t *Output,const FixedMatrix3D_t *Input);
extern void BURGERCALL FixedMatrix3DMulVector2(FixedVector3D_t *Output,const FixedMatrix3D_t *Input,const FixedVector3D_t *Input2);
extern void BURGERCALL FixedMatrix3DMulVectorAddVector(FixedVector3D_t *Output,const FixedMatrix3D_t *Input,const FixedVector3D_t *Add);
extern void BURGERCALL FixedMatrix3DMulVectorAddVector2(FixedVector3D_t *Output,const FixedMatrix3D_t *Input,const FixedVector3D_t *Add,const FixedVector3D_t *InputV);

/**********************************

	Fixed32 point quaternions

**********************************/

void BURGERCALL FixedQuatIdentity(FixedQuat_t *Input)
{
	Input->x = 0;
	Input->y = 0;
	Input->z = 0;
	Input->w = FLOATTOFIXED(1.0f);
}

