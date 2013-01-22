/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#include "JFDCTSlw.hpp"

namespace JPeg70 {

/*******************************

	This file contains a slow-but-accurate integer implementation of the
	forward DCT (Discrete Cosine Transform).

	A 2-D DCT can be done by 1-D DCT on each row followed by 1-D DCT
	on each column.  Direct algorithms are also available, but they are
	much more complex and seem not to be any faster when reduced to code.

	This implementation is based on an algorithm described in
	C. Loeffler, A. Ligtenberg and G. Moschytz, "Practical Fast 1-D DCT
	Algorithms with 11 Multiplications", Proc. Int'l. Conf. on Acoustics,
	Speech, and Signal Processing 1989 (ICASSP '89), pp. 988-991.
	The primary algorithm described there uses 11 multiplies and 29 adds.
	We use their alternate method with 12 multiplies and 32 adds.
	The advantage of this method is that no data path contains more than one
	multiplication; this allows a very simple and accurate implementation in
	scaled fixed-point arithmetic, with a minimal number of shifts.

*******************************/

#if DCTSIZE != 8
	Sorry, this code only copes with 8x8 DCTs. /* deliberate syntax err */
#endif

/*******************************

	The poop on this scaling stuff is as follows:

	Each 1-D DCT step produces outputs which are a factor of sqrt(N)
	larger than the true DCT outputs.  The final outputs are therefore
	a factor of N larger than desired; since N=8 this can be cured by
	a simple right shift at the end of the algorithm.  The advantage of
	this arrangement is that we save two multiplications per 1-D DCT,
	because the y0 and y4 outputs need not be divided by sqrt(N).
	In the IJG code, this factor of 8 is removed by the quantization step
	(in jcdctmgr.c), NOT in this module.

	We have to do addition and subtraction of the integer inputs, which
	is no problem, and multiplication by fractional constants, which is
	a problem to do in integer arithmetic.  We multiply all the constants
	by CONST_SCALE and convert them to integer constants (thus retaining
	CONST_BITS bits of precision in the constants).  After doing a
	multiplication we have to divide the product by CONST_SCALE, with proper
	rounding, to produce the correct output.  This division can be done
	cheaply as a right shift of CONST_BITS bits.  We postpone shifting
	as long as possible so that partial sums can be added together with
	full fractional precision.

	The outputs of the first pass are scaled up by PASS1_BITS bits so that
	they are represented to better-than-integral precision.  These outputs
	require BITS_IN_JSAMPLE + PASS1_BITS + 3 bits; this fits in a 16-bit word
	with the recommended scaling.  (For 12-bit sample data, the intermediate
	array is long anyway.)

	To avoid overflow of the 32-bit intermediate results in pass 2, we must
	have BITS_IN_JSAMPLE + CONST_BITS + PASS1_BITS <= 26.  Error analysis
	shows that the values given below are the most effective.

********************************/

#define CONST_BITS  13

/*******************************

	Some C compilers fail to reduce "FIX(constant)" at compile time, thus
	causing a lot of useless floating-point operations at run time.
	To get around this we use the following pre-calculated constants.
	If you change CONST_BITS you may want to add appropriate values.
	(With a reasonable C compiler, you can just rely on the FIX() macro...)

********************************/

#if CONST_BITS == 13
#define FIX_0_298631336  ( 2446)	/* FIX(0.298631336) */
#define FIX_0_390180644  ( 3196)	/* FIX(0.390180644) */
#define FIX_0_541196100  ( 4433)	/* FIX(0.541196100) */
#define FIX_0_765366865  ( 6270)	/* FIX(0.765366865) */
#define FIX_0_899976223  ( 7373)	/* FIX(0.899976223) */
#define FIX_1_175875602  ( 9633)	/* FIX(1.175875602) */
#define FIX_1_501321110  (12299)	/* FIX(1.501321110) */
#define FIX_1_847759065  (15137)	/* FIX(1.847759065) */
#define FIX_1_961570560  (16069)	/* FIX(1.961570560) */
#define FIX_2_053119869  (16819)	/* FIX(2.053119869) */
#define FIX_2_562915447  (20995)	/* FIX(2.562915447) */
#define FIX_3_072711026  (25172)	/* FIX(3.072711026) */
#else
#define FIX_0_298631336  (SWord32)(0.298631336*(1<<CONST_BITS)+0.5f)
#define FIX_0_390180644  (SWord32)(0.390180644*(1<<CONST_BITS)+0.5f)
#define FIX_0_541196100  (SWord32)(0.541196100*(1<<CONST_BITS)+0.5f)
#define FIX_0_765366865  (SWord32)(0.765366865*(1<<CONST_BITS)+0.5f)
#define FIX_0_899976223  (SWord32)(0.899976223*(1<<CONST_BITS)+0.5f)
#define FIX_1_175875602  (SWord32)(1.175875602*(1<<CONST_BITS)+0.5f)
#define FIX_1_501321110  (SWord32)(1.501321110*(1<<CONST_BITS)+0.5f)
#define FIX_1_847759065  (SWord32)(1.847759065*(1<<CONST_BITS)+0.5f)
#define FIX_1_961570560  (SWord32)(1.961570560*(1<<CONST_BITS)+0.5f)
#define FIX_2_053119869  (SWord32)(2.053119869*(1<<CONST_BITS)+0.5f)
#define FIX_2_562915447  (SWord32)(2.562915447*(1<<CONST_BITS)+0.5f)
#define FIX_3_072711026  (SWord32)(3.072711026*(1<<CONST_BITS)+0.5f)
#endif

inline SWord32 JFixMul(SWord32 a,SWord32 b)
{
	return a*b;
}

inline SWord32 Descale(SWord32 x,Word n)
{
	return (((x) + (1 << ((n)-1)))>>n);
}

/*******************************

	Perform the forward DCT on one block of samples.

********************************/
	
#if BITS_IN_JSAMPLE == 8
#define PASS1_BITS 2
#else
#define PASS1_BITS 1
#endif

void BURGERCALL FDCTISlow(SWord32 *data)
{
	/* Pass 1: process rows. */
	/* Note results are scaled up by sqrt(8) compared to a true DCT; */
	/* furthermore, we scale the results by 2**PASS1_BITS. */

	SWord32 *dataptr = data;
	Word Count = DCTSIZE;
	do {
		SWord32 tmp0 = dataptr[0] + dataptr[7];
		SWord32 tmp7 = dataptr[0] - dataptr[7];
		SWord32 tmp1 = dataptr[1] + dataptr[6];
		SWord32 tmp6 = dataptr[1] - dataptr[6];
		SWord32 tmp2 = dataptr[2] + dataptr[5];
		SWord32 tmp5 = dataptr[2] - dataptr[5];
		SWord32 tmp3 = dataptr[3] + dataptr[4];
		SWord32 tmp4 = dataptr[3] - dataptr[4];

		/* Even part per LL&M figure 1 --- note that published figure is faulty;
			rotator "sqrt(2)*c1" should be "sqrt(2)*c6". */

		SWord32 tmp10 = tmp0 + tmp3;
		SWord32 tmp13 = tmp0 - tmp3;
		SWord32 tmp11 = tmp1 + tmp2;
		SWord32 tmp12 = tmp1 - tmp2;

		dataptr[0] = ((tmp10 + tmp11) << PASS1_BITS);
		dataptr[4] = ((tmp10 - tmp11) << PASS1_BITS);

		SWord32 z6 = JFixMul(tmp12 + tmp13, FIX_0_541196100);
		dataptr[2] = Descale(z6 + JFixMul(tmp13, FIX_0_765366865),CONST_BITS-PASS1_BITS);
		dataptr[6] = Descale(z6 + JFixMul(tmp12, - FIX_1_847759065),CONST_BITS-PASS1_BITS);

		/* Odd part per figure 8 --- note paper omits factor of sqrt(2).
		* cK represents cos(K*pi/16).
		* i0..i3 in the paper are tmp4..tmp7 here.
		*/

		SWord32 z1 = tmp4 + tmp7;
		SWord32 z2 = tmp5 + tmp6;
		SWord32 z3 = tmp4 + tmp6;
		SWord32 z4 = tmp5 + tmp7;
		SWord32 z5 = JFixMul(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */

		tmp4 = JFixMul(tmp4, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
		tmp5 = JFixMul(tmp5, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
		tmp6 = JFixMul(tmp6, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
		tmp7 = JFixMul(tmp7, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
		z1 = JFixMul(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
		z2 = JFixMul(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
		z3 = JFixMul(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
		z4 = JFixMul(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */

		z3 += z5;
		z4 += z5;

		dataptr[7] = Descale(tmp4 + z1 + z3, CONST_BITS-PASS1_BITS);
		dataptr[5] = Descale(tmp5 + z2 + z4, CONST_BITS-PASS1_BITS);
		dataptr[3] = Descale(tmp6 + z2 + z3, CONST_BITS-PASS1_BITS);
		dataptr[1] = Descale(tmp7 + z1 + z4, CONST_BITS-PASS1_BITS);

		dataptr += DCTSIZE;		/* advance pointer to next row */
	} while (--Count);

	/* Pass 2: process columns.
	* We remove the PASS1_BITS scaling, but leave the results scaled up
	* by an overall factor of 8.
	*/

	dataptr = data;
	Count = DCTSIZE;
	do {
		SWord32 btmp0 = dataptr[DCTSIZE*0] + dataptr[DCTSIZE*7];
		SWord32 btmp7 = dataptr[DCTSIZE*0] - dataptr[DCTSIZE*7];
		SWord32 btmp1 = dataptr[DCTSIZE*1] + dataptr[DCTSIZE*6];
		SWord32 btmp6 = dataptr[DCTSIZE*1] - dataptr[DCTSIZE*6];
		SWord32 btmp2 = dataptr[DCTSIZE*2] + dataptr[DCTSIZE*5];
		SWord32 btmp5 = dataptr[DCTSIZE*2] - dataptr[DCTSIZE*5];
		SWord32 btmp3 = dataptr[DCTSIZE*3] + dataptr[DCTSIZE*4];
		SWord32 btmp4 = dataptr[DCTSIZE*3] - dataptr[DCTSIZE*4];

		/* Even part per LL&M figure 1 --- note that published figure is faulty;
		* rotator "sqrt(2)*c1" should be "sqrt(2)*c6".
		*/

		SWord32 btmp10 = btmp0 + btmp3;
		SWord32 btmp13 = btmp0 - btmp3;
		SWord32 btmp11 = btmp1 + btmp2;
		SWord32 btmp12 = btmp1 - btmp2;

		dataptr[DCTSIZE*0] = Descale(btmp10 + btmp11, PASS1_BITS);
		dataptr[DCTSIZE*4] = Descale(btmp10 - btmp11, PASS1_BITS);

		SWord32 bz6 = JFixMul(btmp12 + btmp13, FIX_0_541196100);
		dataptr[DCTSIZE*2] = Descale(bz6 + JFixMul(btmp13, FIX_0_765366865),CONST_BITS+PASS1_BITS);
		dataptr[DCTSIZE*6] = Descale(bz6 + JFixMul(btmp12, - FIX_1_847759065),CONST_BITS+PASS1_BITS);

		/* Odd part per figure 8 --- note paper omits factor of sqrt(2).
		* cK represents cos(K*pi/16).
		* i0..i3 in the paper are tmp4..tmp7 here.
		*/

		SWord32 bz1 = btmp4 + btmp7;
		SWord32 bz2 = btmp5 + btmp6;
		SWord32 bz3 = btmp4 + btmp6;
		SWord32 bz4 = btmp5 + btmp7;
		SWord32 bz5 = JFixMul(bz3 + bz4, FIX_1_175875602); /* sqrt(2) * c3 */

		btmp4 = JFixMul(btmp4, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
		btmp5 = JFixMul(btmp5, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
		btmp6 = JFixMul(btmp6, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
		btmp7 = JFixMul(btmp7, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
		bz1 = JFixMul(bz1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
		bz2 = JFixMul(bz2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
		bz3 = JFixMul(bz3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
		bz4 = JFixMul(bz4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */

		bz3 += bz5;
		bz4 += bz5;

		dataptr[DCTSIZE*7] = Descale(btmp4 + bz1 + bz3,CONST_BITS+PASS1_BITS);
		dataptr[DCTSIZE*5] = Descale(btmp5 + bz2 + bz4,CONST_BITS+PASS1_BITS);
		dataptr[DCTSIZE*3] = Descale(btmp6 + bz2 + bz3,CONST_BITS+PASS1_BITS);
		dataptr[DCTSIZE*1] = Descale(btmp7 + bz1 + bz4,CONST_BITS+PASS1_BITS);

		++dataptr;			/* advance pointer to next column */
	} while (--Count);
}

}
