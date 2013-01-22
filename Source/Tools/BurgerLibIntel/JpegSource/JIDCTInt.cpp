/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

	This file contains a slow-but-accurate integer implementation of the
	inverse DCT (Discrete Cosine Transform).  In the IJG code, this routine
	must also perform dequantization of the input coefficients.

	A 2-D IDCT can be done by 1-D IDCT on each column followed by 1-D IDCT
	on each row (or vice versa, but it's more convenient to emit a row at
	a time).  Direct algorithms are also available, but they are much more
	complex and seem not to be any faster when reduced to code.

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

#include "JIDCTInt.hpp"
#include "JPeglib.h"

namespace JPeg70 {

/*******************************

	This module is specialized to the case DCTSIZE = 8.

*******************************/

#if DCTSIZE != 8
	Sorry, this code only copes with 8x8 DCTs. /* deliberate syntax err */
#endif

/*******************************

	The poop on this scaling stuff is as follows:

	Each 1-D IDCT step produces outputs which are a factor of sqrt(N)
	larger than the true IDCT outputs.  The final outputs are therefore
	a factor of N larger than desired; since N=8 this can be cured by
	a simple right shift at the end of the algorithm.  The advantage of
	this arrangement is that we save two multiplications per 1-D IDCT,
	because the y0 and y4 inputs need not be divided by sqrt(N).

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
	with the recommended scaling.  (To scale up 12-bit sample data further, an
	intermediate long array would be needed.)

	To avoid overflow of the 32-bit intermediate results in pass 2, we must
	have BITS_IN_JSAMPLE + CONST_BITS + PASS1_BITS <= 26.  Error analysis
	shows that the values given below are the most effective.

*******************************/

#if BITS_IN_JSAMPLE == 8
#define CONST_BITS  13
#define PASS1_BITS  2
#else
#define CONST_BITS  13
#define PASS1_BITS  1		/* lose a little precision to avoid overflow */
#endif

/*******************************

	Some C compilers fail to reduce "FIX(constant)" at compile time, thus
	causing a lot of useless floating-point operations at run time.
	To get around this we use the following pre-calculated constants.
	If you change CONST_BITS you may want to add appropriate values.
	(With a reasonable C compiler, you can just rely on the FIX() macro...)

*******************************/

#if CONST_BITS == 13
#define FIX_0_298631336  ((SWord32)  2446)	/* FIX(0.298631336) */
#define FIX_0_390180644  ((SWord32)  3196)	/* FIX(0.390180644) */
#define FIX_0_541196100  ((SWord32)  4433)	/* FIX(0.541196100) */
#define FIX_0_765366865  ((SWord32)  6270)	/* FIX(0.765366865) */
#define FIX_0_899976223  ((SWord32)  7373)	/* FIX(0.899976223) */
#define FIX_1_175875602  ((SWord32)  9633)	/* FIX(1.175875602) */
#define FIX_1_501321110  ((SWord32)  12299)	/* FIX(1.501321110) */
#define FIX_1_847759065  ((SWord32)  15137)	/* FIX(1.847759065) */
#define FIX_1_961570560  ((SWord32)  16069)	/* FIX(1.961570560) */
#define FIX_2_053119869  ((SWord32)  16819)	/* FIX(2.053119869) */
#define FIX_2_562915447  ((SWord32)  20995)	/* FIX(2.562915447) */
#define FIX_3_072711026  ((SWord32)  25172)	/* FIX(3.072711026) */
#else
#define FIX_0_298631336  FIX(0.298631336)
#define FIX_0_390180644  FIX(0.390180644)
#define FIX_0_541196100  FIX(0.541196100)
#define FIX_0_765366865  FIX(0.765366865)
#define FIX_0_899976223  FIX(0.899976223)
#define FIX_1_175875602  FIX(1.175875602)
#define FIX_1_501321110  FIX(1.501321110)
#define FIX_1_847759065  FIX(1.847759065)
#define FIX_1_961570560  FIX(1.961570560)
#define FIX_2_053119869  FIX(2.053119869)
#define FIX_2_562915447  FIX(2.562915447)
#define FIX_3_072711026  FIX(3.072711026)
#endif

/*******************************

	Multiply an long variable by an long constant to yield an long result.
	For 8-bit samples with the recommended scaling, all the variable
	and constant values involved are no more than 16 bits wide, so a
	16x16->32 bit multiply can be used instead of a full 32x32 multiply.
	For 12-bit samples, a full 32-bit multiplication will be needed.

*******************************/

inline SWord32 MyMul(SWord32 x,SWord32 y)
{
	return x*y;
}

/*******************************

	Dequantize a coefficient by multiplying it by the multiplier-table
	entry; produce an int result.  In this module, both inputs and result
	are 16 bits or less, so either int or short multiply will work.

*******************************/

inline SWord32 Dequantize(SWord32 coef,SWord32 quantval)
{
	return coef*quantval;
}

inline SWord32 Descale(SWord32 x,Word n)
{
	return (x + (1 << (n-1)))>>n;
}

/*******************************

	Perform dequantization and inverse DCT on one block of coefficients.

*******************************/


void BURGERCALL IDCTISlow(CJPegDecompress *cinfo,ComponentInfo_t *compptr,SWord16 * coef_block,JSample_t ** output_buf,Word output_col)
{
	SWord32 workspace[DCTSIZE2];	/* buffers data between passes */

	/* Pass 1: process columns from input, store into work array. */
	/* Note results are scaled up by sqrt(8) compared to a true IDCT; */
	/* furthermore, we scale the results by 2**PASS1_BITS. */

	SWord16 *inptr = coef_block;
	SWord32 *quantptr = compptr->DctTablePtr.Int;
	SWord32 *wsptr = workspace;
	Word Count = DCTSIZE;
	do {
		/* Due to quantization, we will usually find that many of the input
		* coefficients are zero, especially the AC terms.  We can exploit this
		* by short-circuiting the IDCT calculation for any column in which all
		* the AC terms are zero.  In that case each output is equal to the
		* DC coefficient (with scale factor as needed).
		* With typical images and quantization tables, half or more of the
		* column DCT calculations can be simplified this way.
		*/

		if (inptr[DCTSIZE*1] == 0 && inptr[DCTSIZE*2] == 0 &&
			inptr[DCTSIZE*3] == 0 && inptr[DCTSIZE*4] == 0 &&
			inptr[DCTSIZE*5] == 0 && inptr[DCTSIZE*6] == 0 &&
			inptr[DCTSIZE*7] == 0) {
			/* AC terms all zero */
			SWord32 dcval = Dequantize(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]) << PASS1_BITS;

			wsptr[DCTSIZE*0] = dcval;
			wsptr[DCTSIZE*1] = dcval;
			wsptr[DCTSIZE*2] = dcval;
			wsptr[DCTSIZE*3] = dcval;
			wsptr[DCTSIZE*4] = dcval;
			wsptr[DCTSIZE*5] = dcval;
			wsptr[DCTSIZE*6] = dcval;
			wsptr[DCTSIZE*7] = dcval;
		} else {

			/* Even part: reverse the even part of the forward DCT. */
			/* The rotator is sqrt(2)*c(-6). */

			SWord32 z32 = Dequantize(inptr[DCTSIZE*2], quantptr[DCTSIZE*2]);
			SWord32 z33 = Dequantize(inptr[DCTSIZE*6], quantptr[DCTSIZE*6]);

			SWord32 z31 = MyMul(z32 + z33, FIX_0_541196100);
			SWord32 tmp2 = z31 + MyMul(z33, - FIX_1_847759065);
			SWord32 tmp3 = z31 + MyMul(z32, FIX_0_765366865);

			SWord32 z42 = Dequantize(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);
			SWord32 z43 = Dequantize(inptr[DCTSIZE*4], quantptr[DCTSIZE*4]);

			SWord32 tmp0 = (z42 + z43) << CONST_BITS;
			SWord32 tmp1 = (z42 - z43) << CONST_BITS;

			SWord32 tmp10 = tmp0 + tmp3;
			SWord32 tmp13 = tmp0 - tmp3;
			SWord32 tmp11 = tmp1 + tmp2;
			SWord32 tmp12 = tmp1 - tmp2;

			/* Odd part per figure 8; the matrix is unitary and hence its
			* transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
			*/

			tmp0 = Dequantize(inptr[DCTSIZE*7], quantptr[DCTSIZE*7]);
			tmp1 = Dequantize(inptr[DCTSIZE*5], quantptr[DCTSIZE*5]);
			tmp2 = Dequantize(inptr[DCTSIZE*3], quantptr[DCTSIZE*3]);
			tmp3 = Dequantize(inptr[DCTSIZE*1], quantptr[DCTSIZE*1]);

			SWord32 z1 = tmp0 + tmp3;
			SWord32 z2 = tmp1 + tmp2;
			SWord32 z3 = tmp0 + tmp2;
			SWord32 z4 = tmp1 + tmp3;
			SWord32 z5 = MyMul(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */

			tmp0 = MyMul(tmp0, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
			tmp1 = MyMul(tmp1, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
			tmp2 = MyMul(tmp2, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
			tmp3 = MyMul(tmp3, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
			z1 = MyMul(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
			z2 = MyMul(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
			z3 = MyMul(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
			z4 = MyMul(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */

			z3 += z5;
			z4 += z5;

			tmp0 += z1 + z3;
			tmp1 += z2 + z4;
			tmp2 += z2 + z3;
			tmp3 += z1 + z4;

			/* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

			wsptr[DCTSIZE*0] = Descale(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
			wsptr[DCTSIZE*7] = Descale(tmp10 - tmp3, CONST_BITS-PASS1_BITS);
			wsptr[DCTSIZE*1] = Descale(tmp11 + tmp2, CONST_BITS-PASS1_BITS);
			wsptr[DCTSIZE*6] = Descale(tmp11 - tmp2, CONST_BITS-PASS1_BITS);
			wsptr[DCTSIZE*2] = Descale(tmp12 + tmp1, CONST_BITS-PASS1_BITS);
			wsptr[DCTSIZE*5] = Descale(tmp12 - tmp1, CONST_BITS-PASS1_BITS);
			wsptr[DCTSIZE*3] = Descale(tmp13 + tmp0, CONST_BITS-PASS1_BITS);
			wsptr[DCTSIZE*4] = Descale(tmp13 - tmp0, CONST_BITS-PASS1_BITS);
		}
		++inptr;			/* advance pointers to next column */
		++quantptr;
		++wsptr;
	} while (--Count);

	/* Pass 2: process rows from work array, store into output array. */
	/* Note that we must descale the results by a factor of 8 == 2**3, */
	/* and also undo the PASS1_BITS scaling. */

	JSample_t *range_limit = cinfo->IDCTRangeLimit();
	wsptr = workspace;
	Word Count2 = DCTSIZE;
	do {
		JSample_t *outptr = output_buf[0] + output_col;
		++output_buf;
		/* Rows of zeroes can be exploited in the same way as we did with columns.
		* However, the column calculation has created many nonzero AC terms, so
		* the simplification applies less often (typically 5% to 10% of the time).
		* On machines with very fast multiplication, it's possible that the
		* test takes more time than it's worth.  In that case this section
		* may be commented out.
		*/

		#ifndef NO_ZERO_ROW_TEST
		if (wsptr[1] == 0 && wsptr[2] == 0 && wsptr[3] == 0 && wsptr[4] == 0 &&
			wsptr[5] == 0 && wsptr[6] == 0 && wsptr[7] == 0) {
			/* AC terms all zero */
			JSample_t dcval = range_limit[Descale(wsptr[0],PASS1_BITS+3)& RANGE_MASK];
			outptr[0] = dcval;
			outptr[1] = dcval;
			outptr[2] = dcval;
			outptr[3] = dcval;
			outptr[4] = dcval;
			outptr[5] = dcval;
			outptr[6] = dcval;
			outptr[7] = dcval;
		} else 
		#endif
		{
			/* Even part: reverse the even part of the forward DCT. */
			/* The rotator is sqrt(2)*c(-6). */

			SWord32 bz32 = wsptr[2];
			SWord32 bz33 = wsptr[6];

			SWord32 bz31 = MyMul(bz32 + bz33, FIX_0_541196100);
			SWord32 btmp2 = bz31 + MyMul(bz33, - FIX_1_847759065);
			SWord32 btmp3 = bz31 + MyMul(bz32, FIX_0_765366865);

			SWord32 btmp0 = (wsptr[0] + wsptr[4]) << CONST_BITS;
			SWord32 btmp1 = (wsptr[0] - wsptr[4]) << CONST_BITS;

			SWord32 btmp10 = btmp0 + btmp3;
			SWord32 btmp13 = btmp0 - btmp3;
			SWord32 btmp11 = btmp1 + btmp2;
			SWord32 btmp12 = btmp1 - btmp2;

			/* Odd part per figure 8; the matrix is unitary and hence its
			* transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
			*/

			btmp0 = wsptr[7];
			btmp1 = wsptr[5];
			btmp2 = wsptr[3];
			btmp3 = wsptr[1];

			SWord32 bz1 = btmp0 + btmp3;
			SWord32 bz2 = btmp1 + btmp2;
			SWord32 bz3 = btmp0 + btmp2;
			SWord32 bz4 = btmp1 + btmp3;
			SWord32 bz5 = MyMul(bz3 + bz4, FIX_1_175875602); /* sqrt(2) * c3 */

			btmp0 = MyMul(btmp0, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
			btmp1 = MyMul(btmp1, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
			btmp2 = MyMul(btmp2, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
			btmp3 = MyMul(btmp3, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
			bz1 = MyMul(bz1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
			bz2 = MyMul(bz2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
			bz3 = MyMul(bz3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
			bz4 = MyMul(bz4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */

			bz3 += bz5;
			bz4 += bz5;

			btmp0 += bz1 + bz3;
			btmp1 += bz2 + bz4;
			btmp2 += bz2 + bz3;
			btmp3 += bz1 + bz4;

			/* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

			outptr[0] = range_limit[Descale(btmp10 + btmp3,CONST_BITS+PASS1_BITS+3)& RANGE_MASK];
			outptr[7] = range_limit[Descale(btmp10 - btmp3,CONST_BITS+PASS1_BITS+3)& RANGE_MASK];
			outptr[1] = range_limit[Descale(btmp11 + btmp2,CONST_BITS+PASS1_BITS+3)& RANGE_MASK];
			outptr[6] = range_limit[Descale(btmp11 - btmp2,CONST_BITS+PASS1_BITS+3)& RANGE_MASK];
			outptr[2] = range_limit[Descale(btmp12 + btmp1,CONST_BITS+PASS1_BITS+3)& RANGE_MASK];
			outptr[5] = range_limit[Descale(btmp12 - btmp1,CONST_BITS+PASS1_BITS+3)& RANGE_MASK];
			outptr[3] = range_limit[Descale(btmp13 + btmp0,CONST_BITS+PASS1_BITS+3)& RANGE_MASK];
			outptr[4] = range_limit[Descale(btmp13 - btmp0,CONST_BITS+PASS1_BITS+3)& RANGE_MASK];
		}
		wsptr += DCTSIZE;		/* advance pointer to next row */
	} while (--Count2);
}

}