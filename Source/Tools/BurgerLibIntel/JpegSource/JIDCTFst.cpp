/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

	This file contains a fast, not so accurate integer implementation of the
	inverse DCT (Discrete Cosine Transform).  In the IJG code, this routine
	must also perform dequantization of the input coefficients.

	A 2-D IDCT can be done by 1-D IDCT on each column followed by 1-D IDCT
	on each row (or vice versa, but it's more convenient to emit a row at
	a time).  Direct algorithms are also available, but they are much more
	complex and seem not to be any faster when reduced to code.

	This implementation is based on Arai, Agui, and Nakajima's algorithm for
	scaled DCT.  Their original paper (Trans. IEICE E-71(11):1095) is in
	Japanese, but the algorithm is described in the Pennebaker & Mitchell
	JPEG textbook (see REFERENCES section in file README).  The following code
	is based directly on figure 4-8 in P&M.
	While an 8-point DCT cannot be done in less than 11 multiplies, it is
	possible to arrange the computation so that many of the multiplies are
	simple scalings of the final outputs.  These multiplies can then be
	folded into the multiplications or divisions by the JPEG quantization
	table entries.  The AA&N method leaves only 5 multiplies and 29 adds
	to be done in the DCT itself.
	The primary disadvantage of this method is that with fixed-point math,
	accuracy is lost due to imprecise representation of the scaled
	quantization values.  The smaller the quantization table entry, the less
	precise the scaled value, so this implementation does worse with high-
	quality-setting files than with low-quality ones.

*******************************/

#include "JIDCTFst.hpp"
#include "jpeglib.h"

namespace JPeg70 {

/*******************************
	
	This module is specialized to the case DCTSIZE = 8.
 
*******************************/

#if DCTSIZE != 8
	Sorry, this code only copes with 8x8 DCTs. /* deliberate syntax err */
#endif

/*******************************

	Scaling decisions are generally the same as in the LL&M algorithm;
	see jidctint.c for more details.  However, we choose to descale
	(right shift) multiplication products as soon as they are formed,
	rather than carrying additional fractional bits into subsequent additions.
	This compromises accuracy slightly, but it lets us save a few shifts.
	More importantly, 16-bit arithmetic is then adequate (for 8-bit samples)
	everywhere except in the multiplications proper; this saves a good deal
	of work on 16-bit-int machines.

	The dequantized coefficients are not integers because the AA&N scaling
	factors have been incorporated.  We represent them scaled up by PASS1_BITS,
	so that the first and second IDCT rounds have the same input scaling.
	For 8-bit JSAMPLEs, we choose IFAST_SCALE_BITS = PASS1_BITS so as to
	avoid a descaling shift; this compromises accuracy rather drastically
	for small quantization table entries, but it saves a lot of shifts.
	For 12-bit JSAMPLEs, there's no hope of using 16x16 multiplies anyway,
	so we use a much larger scaling factor to preserve accuracy.

	A final compromise is to represent the multiplicative constants to only
	8 fractional bits, rather than 13.  This saves some shifting work on some
	machines, and may also reduce the cost of multiplication (since there
	are fewer one-bits in the constants).

*******************************/

#if BITS_IN_JSAMPLE == 8
#define CONST_BITS  8
#define PASS1_BITS  2
#else
#define CONST_BITS  8
#define PASS1_BITS  1	/* lose a little precision to avoid overflow */
#endif

/*******************************

	Some C compilers fail to reduce "FIX(constant)" at compile time, thus
	causing a lot of useless floating-point operations at run time.
	To get around this we use the following pre-calculated constants.
	If you change CONST_BITS you may want to add appropriate values.
	(With a reasonable C compiler, you can just rely on the FIX() macro...)

*******************************/

#if CONST_BITS == 8
#define FIX_1_082392200  ((SWord32)  277)		/* FIX(1.082392200) */
#define FIX_1_414213562  ((SWord32)  362)		/* FIX(1.414213562) */
#define FIX_1_847759065  ((SWord32)  473)		/* FIX(1.847759065) */
#define FIX_2_613125930  ((SWord32)  669)		/* FIX(2.613125930) */
#else
#define FIX_1_082392200  FIX(1.082392200)
#define FIX_1_414213562  FIX(1.414213562)
#define FIX_1_847759065  FIX(1.847759065)
#define FIX_2_613125930  FIX(2.613125930)
#endif

inline SWord32 Descale(SWord32 x,Word n)
{
	return (x + (1<<(n-1)))>>n;
}

inline SWord32 MyMul(SWord32 x,SWord32 y)
{
	return Descale(x*y,CONST_BITS);
}

/*******************************

	Dequantize a coefficient by multiplying it by the multiplier-table
	entry; produce a SWord32 result.  For 8-bit data a 16x16->16
	multiplication will do.  For 12-bit data, the multiplier table is
	declared long, so a 32-bit multiply will be used.

*******************************/

inline SWord32 Dequantize(SWord32 coef,SWord32 quantval)
{
#if BITS_IN_JSAMPLE == 8
	return (coef * quantval);
#else
	return Descale(coef*quantval,IFAST_SCALE_BITS-PASS1_BITS);
#endif
}

/*******************************

	Perform dequantization and inverse DCT on one block of coefficients.

*******************************/

void BURGERCALL IDCTFast(CJPegDecompress *cinfo,ComponentInfo_t *compptr,SWord16 *coef_block,JSample_t ** output_buf,Word output_col)
{
	SWord32 workspace[DCTSIZE2];	/* buffers data between passes */

	/* Pass 1: process columns from input, store into work array. */

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
			SWord32 dcval = Dequantize(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);
	
			wsptr[DCTSIZE*0] = dcval;
			wsptr[DCTSIZE*1] = dcval;
			wsptr[DCTSIZE*2] = dcval;
			wsptr[DCTSIZE*3] = dcval;
			wsptr[DCTSIZE*4] = dcval;
			wsptr[DCTSIZE*5] = dcval;
			wsptr[DCTSIZE*6] = dcval;
			wsptr[DCTSIZE*7] = dcval;
		} else {

			/* Even part */

			SWord32 tmp0 = Dequantize(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);
			SWord32 tmp1 = Dequantize(inptr[DCTSIZE*2], quantptr[DCTSIZE*2]);
			SWord32 tmp2 = Dequantize(inptr[DCTSIZE*4], quantptr[DCTSIZE*4]);
			SWord32 tmp3 = Dequantize(inptr[DCTSIZE*6], quantptr[DCTSIZE*6]);

			SWord32 tmp10 = tmp0 + tmp2;	/* phase 3 */
			SWord32 tmp11 = tmp0 - tmp2;

			SWord32 tmp13 = tmp1 + tmp3;	/* phases 5-3 */
			SWord32 tmp12 = MyMul(tmp1 - tmp3, FIX_1_414213562) - tmp13; /* 2*c4 */

			tmp0 = tmp10 + tmp13;	/* phase 2 */
			tmp3 = tmp10 - tmp13;
			tmp1 = tmp11 + tmp12;
			tmp2 = tmp11 - tmp12;

			/* Odd part */

			SWord32 tmp4 = Dequantize(inptr[DCTSIZE*1], quantptr[DCTSIZE*1]);
			SWord32 tmp5 = Dequantize(inptr[DCTSIZE*3], quantptr[DCTSIZE*3]);
			SWord32 tmp6 = Dequantize(inptr[DCTSIZE*5], quantptr[DCTSIZE*5]);
			SWord32 tmp7 = Dequantize(inptr[DCTSIZE*7], quantptr[DCTSIZE*7]);

			SWord32 z13 = tmp6 + tmp5;		/* phase 6 */
			SWord32 z10 = tmp6 - tmp5;
			SWord32 z11 = tmp4 + tmp7;
			SWord32 z12 = tmp4 - tmp7;

			tmp7 = z11 + z13;		/* phase 5 */
			tmp11 = MyMul(z11 - z13, FIX_1_414213562); /* 2*c4 */

			SWord32 z5 = MyMul(z10 + z12, FIX_1_847759065); /* 2*c2 */
			tmp10 = MyMul(z12, FIX_1_082392200) - z5; /* 2*(c2-c6) */
			tmp12 = MyMul(z10, - FIX_2_613125930) + z5; /* -2*(c2+c6) */

			tmp6 = tmp12 - tmp7;	/* phase 2 */
			tmp5 = tmp11 - tmp6;
			tmp4 = tmp10 + tmp5;

			wsptr[DCTSIZE*0] = (tmp0 + tmp7);
			wsptr[DCTSIZE*7] = (tmp0 - tmp7);
			wsptr[DCTSIZE*1] = (tmp1 + tmp6);
			wsptr[DCTSIZE*6] = (tmp1 - tmp6);
			wsptr[DCTSIZE*2] = (tmp2 + tmp5);
			wsptr[DCTSIZE*5] = (tmp2 - tmp5);
			wsptr[DCTSIZE*4] = (tmp3 + tmp4);
			wsptr[DCTSIZE*3] = (tmp3 - tmp4);
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
			JSample_t dcval = range_limit[Descale(wsptr[0], PASS1_BITS+3) & RANGE_MASK];

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
			/* Even part */

			SWord32 btmp10 = (wsptr[0] + wsptr[4]);
			SWord32 btmp11 = (wsptr[0] - wsptr[4]);

			SWord32 btmp13 = (wsptr[2] + wsptr[6]);
			SWord32 btmp12 = MyMul(wsptr[2] - wsptr[6], FIX_1_414213562) - btmp13;

			SWord32 btmp0 = btmp10 + btmp13;
			SWord32 btmp3 = btmp10 - btmp13;
			SWord32 btmp1 = btmp11 + btmp12;
			SWord32 btmp2 = btmp11 - btmp12;

			/* Odd part */

			SWord32 bz13 = wsptr[5] + wsptr[3];
			SWord32 bz10 = wsptr[5] - wsptr[3];
			SWord32 bz11 = wsptr[1] + wsptr[7];
			SWord32 bz12 = wsptr[1] - wsptr[7];

			SWord32 btmp7 = bz11 + bz13;		/* phase 5 */
			SWord32 btmp21 = MyMul(bz11 - bz13, FIX_1_414213562); /* 2*c4 */

			SWord32 bz5 = MyMul(bz10 + bz12, FIX_1_847759065); /* 2*c2 */
			SWord32 btmp20 = MyMul(bz12, FIX_1_082392200) - bz5; /* 2*(c2-c6) */
			SWord32 btmp22 = MyMul(bz10, - FIX_2_613125930) + bz5; /* -2*(c2+c6) */

			SWord32 btmp6 = btmp22 - btmp7;	/* phase 2 */
			SWord32 btmp5 = btmp21 - btmp6;
			SWord32 btmp4 = btmp20 + btmp5;

			/* Final output stage: scale down by a factor of 8 and range-limit */

			outptr[0] = range_limit[Descale(btmp0 + btmp7, PASS1_BITS+3)&RANGE_MASK];
			outptr[7] = range_limit[Descale(btmp0 - btmp7, PASS1_BITS+3)&RANGE_MASK];
			outptr[1] = range_limit[Descale(btmp1 + btmp6, PASS1_BITS+3)&RANGE_MASK];
			outptr[6] = range_limit[Descale(btmp1 - btmp6, PASS1_BITS+3)&RANGE_MASK];
			outptr[2] = range_limit[Descale(btmp2 + btmp5, PASS1_BITS+3)&RANGE_MASK];
			outptr[5] = range_limit[Descale(btmp2 - btmp5, PASS1_BITS+3)&RANGE_MASK];
			outptr[4] = range_limit[Descale(btmp3 + btmp4, PASS1_BITS+3)&RANGE_MASK];
			outptr[3] = range_limit[Descale(btmp3 - btmp4, PASS1_BITS+3)&RANGE_MASK];
		}
		wsptr += DCTSIZE;		/* advance pointer to next row */
	} while (--Count2);
}

}
