/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

	This file contains inverse-DCT routines that produce reduced-size output:
	either 4x4, 2x2, or 1x1 pixels from an 8x8 DCT block.

	The implementation is based on the Loeffler, Ligtenberg and Moschytz (LL&M)
	algorithm used in jidctint.c.  We simply replace each 8-to-8 1-D IDCT step
	with an 8-to-4 step that produces the four averages of two adjacent outputs
	(or an 8-to-2 step producing two averages of four outputs, for 2x2 output).
	These steps were derived by computing the corresponding values at the end
	of the normal LL&M code, then simplifying as much as possible.

	1x1 is trivial: just take the DC coefficient divided by 8.

	See jidctint.c for additional comments.

*******************************/

#include "JIDCTRed.hpp"
#include "JPeglib.h"

namespace JPeg70 {

/* This module is specialized to the case DCTSIZE = 8. */

#if DCTSIZE != 8
	Sorry, this code only copes with 8x8 DCTs. /* deliberate syntax err */
#endif

/* Scaling is the same as in jidctint.c. */

#if BITS_IN_JSAMPLE == 8
#define CONST_BITS  13
#define PASS1_BITS  2
#else
#define CONST_BITS  13
#define PASS1_BITS  1		/* lose a little precision to avoid overflow */
#endif

/* Some C compilers fail to reduce "FIX(constant)" at compile time, thus
 * causing a lot of useless floating-point operations at run time.
 * To get around this we use the following pre-calculated constants.
 * If you change CONST_BITS you may want to add appropriate values.
 * (With a reasonable C compiler, you can just rely on the FIX() macro...)
 */

#if CONST_BITS == 13
#define FIX_0_211164243  ((SWord32)  1730)	/* FIX(0.211164243) */
#define FIX_0_509795579  ((SWord32)  4176)	/* FIX(0.509795579) */
#define FIX_0_601344887  ((SWord32)  4926)	/* FIX(0.601344887) */
#define FIX_0_720959822  ((SWord32)  5906)	/* FIX(0.720959822) */
#define FIX_0_765366865  ((SWord32)  6270)	/* FIX(0.765366865) */
#define FIX_0_850430095  ((SWord32)  6967)	/* FIX(0.850430095) */
#define FIX_0_899976223  ((SWord32)  7373)	/* FIX(0.899976223) */
#define FIX_1_061594337  ((SWord32)  8697)	/* FIX(1.061594337) */
#define FIX_1_272758580  ((SWord32)  10426)	/* FIX(1.272758580) */
#define FIX_1_451774981  ((SWord32)  11893)	/* FIX(1.451774981) */
#define FIX_1_847759065  ((SWord32)  15137)	/* FIX(1.847759065) */
#define FIX_2_172734803  ((SWord32)  17799)	/* FIX(2.172734803) */
#define FIX_2_562915447  ((SWord32)  20995)	/* FIX(2.562915447) */
#define FIX_3_624509785  ((SWord32)  29692)	/* FIX(3.624509785) */
#else
#define FIX_0_211164243  FIX(0.211164243)
#define FIX_0_509795579  FIX(0.509795579)
#define FIX_0_601344887  FIX(0.601344887)
#define FIX_0_720959822  FIX(0.720959822)
#define FIX_0_765366865  FIX(0.765366865)
#define FIX_0_850430095  FIX(0.850430095)
#define FIX_0_899976223  FIX(0.899976223)
#define FIX_1_061594337  FIX(1.061594337)
#define FIX_1_272758580  FIX(1.272758580)
#define FIX_1_451774981  FIX(1.451774981)
#define FIX_1_847759065  FIX(1.847759065)
#define FIX_2_172734803  FIX(2.172734803)
#define FIX_2_562915447  FIX(2.562915447)
#define FIX_3_624509785  FIX(3.624509785)
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

	Perform dequantization and inverse DCT on one block of coefficients,
	producing a reduced-size 4x4 output block.

*******************************/

void BURGERCALL IDCT4x4(CJPegDecompress *cinfo,ComponentInfo_t *compptr,SWord16 *coef_block,JSample_t ** output_buf,Word output_col)
{
	SWord32 workspace[DCTSIZE*4];	/* buffers data between passes */

	/* Pass 1: process columns from input, store into work array. */

	SWord16 *inptr = coef_block;
	SWord32 *quantptr = compptr->DctTablePtr.Int;
	SWord32 *wsptr = workspace;
	Word Count = DCTSIZE;
	do {
		/* Don't bother to process column 4, because second pass won't use it */
		if (Count != DCTSIZE-4) {
			if (inptr[DCTSIZE*1] == 0 && inptr[DCTSIZE*2] == 0 &&
				inptr[DCTSIZE*3] == 0 && inptr[DCTSIZE*5] == 0 &&
				inptr[DCTSIZE*6] == 0 && inptr[DCTSIZE*7] == 0) {
				/* AC terms all zero; we need not examine term 4 for 4x4 output */
				SWord32 dcval = Dequantize(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]) << PASS1_BITS;

				wsptr[DCTSIZE*0] = dcval;
				wsptr[DCTSIZE*1] = dcval;
				wsptr[DCTSIZE*2] = dcval;
				wsptr[DCTSIZE*3] = dcval;
			} else {

				/* Even part */

				SWord32 tmp20 = Dequantize(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);
				tmp20 <<= (CONST_BITS+1);

				SWord32 z22 = Dequantize(inptr[DCTSIZE*2], quantptr[DCTSIZE*2]);
				SWord32 z23 = Dequantize(inptr[DCTSIZE*6], quantptr[DCTSIZE*6]);

				SWord32 tmp22 = MyMul(z22, FIX_1_847759065) + MyMul(z23, - FIX_0_765366865);

				SWord32 tmp10 = tmp20 + tmp22;
				SWord32 tmp12 = tmp20 - tmp22;

				/* Odd part */

				SWord32 z1 = Dequantize(inptr[DCTSIZE*7], quantptr[DCTSIZE*7]);
				SWord32 z2 = Dequantize(inptr[DCTSIZE*5], quantptr[DCTSIZE*5]);
				SWord32 z3 = Dequantize(inptr[DCTSIZE*3], quantptr[DCTSIZE*3]);
				SWord32 z4 = Dequantize(inptr[DCTSIZE*1], quantptr[DCTSIZE*1]);

				SWord32 tmp0 = MyMul(z1, - FIX_0_211164243) /* sqrt(2) * (c3-c1) */
					+ MyMul(z2, FIX_1_451774981) /* sqrt(2) * (c3+c7) */
					+ MyMul(z3, - FIX_2_172734803) /* sqrt(2) * (-c1-c5) */
					+ MyMul(z4, FIX_1_061594337); /* sqrt(2) * (c5+c7) */

				SWord32 tmp2 = MyMul(z1, - FIX_0_509795579) /* sqrt(2) * (c7-c5) */
					+ MyMul(z2, - FIX_0_601344887) /* sqrt(2) * (c5-c1) */
					+ MyMul(z3, FIX_0_899976223) /* sqrt(2) * (c3-c7) */
					+ MyMul(z4, FIX_2_562915447); /* sqrt(2) * (c1+c3) */

				/* Final output stage */

				wsptr[DCTSIZE*0] = Descale(tmp10 + tmp2, CONST_BITS-PASS1_BITS+1);
				wsptr[DCTSIZE*3] = Descale(tmp10 - tmp2, CONST_BITS-PASS1_BITS+1);
				wsptr[DCTSIZE*1] = Descale(tmp12 + tmp0, CONST_BITS-PASS1_BITS+1);
				wsptr[DCTSIZE*2] = Descale(tmp12 - tmp0, CONST_BITS-PASS1_BITS+1);
			}
		}
		++inptr;
		++quantptr;
		++wsptr;
	} while (--Count);

	/* Pass 2: process 4 rows from work array, store into output array. */

	JSample_t *range_limit = cinfo->IDCTRangeLimit();
	wsptr = workspace;
	Word Count2 = 4;
	do {
		JSample_t *outptr = output_buf[0] + output_col;
		++output_buf;
		/* It's not clear whether a zero row test is worthwhile here ... */

		#ifndef NO_ZERO_ROW_TEST
		if (wsptr[1] == 0 && wsptr[2] == 0 && wsptr[3] == 0 &&
			wsptr[5] == 0 && wsptr[6] == 0 && wsptr[7] == 0) {
			/* AC terms all zero */
			JSample_t dcval = range_limit[Descale(wsptr[0], PASS1_BITS+3) & RANGE_MASK];
			outptr[0] = dcval;
			outptr[1] = dcval;
			outptr[2] = dcval;
			outptr[3] = dcval;
		} else
		#endif
		{
			/* Even part */

			SWord32 tmp20 = wsptr[0] << (CONST_BITS+1);

			SWord32 tmp22 = MyMul(wsptr[2], FIX_1_847759065)
				+ MyMul(wsptr[6], - FIX_0_765366865);

			SWord32 tmp10 = tmp20 + tmp22;
			SWord32 tmp12 = tmp20 - tmp22;

			/* Odd part */

			SWord32 z1 = wsptr[7];
			SWord32 z2 = wsptr[5];
			SWord32 z3 = wsptr[3];
			SWord32 z4 = wsptr[1];

			SWord32 tmp0 = MyMul(z1, - FIX_0_211164243) /* sqrt(2) * (c3-c1) */
				+ MyMul(z2, FIX_1_451774981) /* sqrt(2) * (c3+c7) */
				+ MyMul(z3, - FIX_2_172734803) /* sqrt(2) * (-c1-c5) */
				+ MyMul(z4, FIX_1_061594337); /* sqrt(2) * (c5+c7) */

			SWord32 tmp2 = MyMul(z1, - FIX_0_509795579) /* sqrt(2) * (c7-c5) */
				+ MyMul(z2, - FIX_0_601344887) /* sqrt(2) * (c5-c1) */
				+ MyMul(z3, FIX_0_899976223) /* sqrt(2) * (c3-c7) */
				+ MyMul(z4, FIX_2_562915447); /* sqrt(2) * (c1+c3) */

			/* Final output stage */

			outptr[0] = range_limit[Descale(tmp10 + tmp2,CONST_BITS+PASS1_BITS+3+1)& RANGE_MASK];
			outptr[3] = range_limit[Descale(tmp10 - tmp2,CONST_BITS+PASS1_BITS+3+1)& RANGE_MASK];
			outptr[1] = range_limit[Descale(tmp12 + tmp0,CONST_BITS+PASS1_BITS+3+1)& RANGE_MASK];
			outptr[2] = range_limit[Descale(tmp12 - tmp0,CONST_BITS+PASS1_BITS+3+1)& RANGE_MASK];
		}
		wsptr += DCTSIZE;		/* advance pointer to next row */
	} while (--Count2);
}


/*******************************

	Perform dequantization and inverse DCT on one block of coefficients,
	producing a reduced-size 2x2 output block.

*******************************/

void BURGERCALL IDCT2x2(CJPegDecompress * cinfo,ComponentInfo_t *compptr,SWord16 *coef_block,JSample_t **output_buf,Word output_col)
{
	SWord32 workspace[DCTSIZE*2];	/* buffers data between passes */

	/* Pass 1: process columns from input, store into work array. */

	SWord16 *inptr = coef_block;
	SWord32 *quantptr = compptr->DctTablePtr.Int;
	SWord32 *wsptr = workspace;
	Word Count = DCTSIZE;
	do {
		/* Don't bother to process columns 2,4,6 */
		if (Count != DCTSIZE-2 && Count != DCTSIZE-4 && Count != DCTSIZE-6) {
			if (inptr[DCTSIZE*1] == 0 && inptr[DCTSIZE*3] == 0 &&
				inptr[DCTSIZE*5] == 0 && inptr[DCTSIZE*7] == 0) {
				/* AC terms all zero; we need not examine terms 2,4,6 for 2x2 output */
				SWord32 dcval = Dequantize(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]) << PASS1_BITS;

				wsptr[DCTSIZE*0] = dcval;
				wsptr[DCTSIZE*1] = dcval;
			} else {

				/* Even part */

				SWord32 z1 = Dequantize(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);
				SWord32 tmp10 = z1 << (CONST_BITS+2);
	
				/* Odd part */

				z1 = Dequantize(inptr[DCTSIZE*7], quantptr[DCTSIZE*7]);
				SWord32 tmp0 = MyMul(z1, - FIX_0_720959822); /* sqrt(2) * (c7-c5+c3-c1) */
				z1 = Dequantize(inptr[DCTSIZE*5], quantptr[DCTSIZE*5]);
				tmp0 += MyMul(z1, FIX_0_850430095); /* sqrt(2) * (-c1+c3+c5+c7) */
				z1 = Dequantize(inptr[DCTSIZE*3], quantptr[DCTSIZE*3]);
				tmp0 += MyMul(z1, - FIX_1_272758580); /* sqrt(2) * (-c1+c3-c5-c7) */
				z1 = Dequantize(inptr[DCTSIZE*1], quantptr[DCTSIZE*1]);
				tmp0 += MyMul(z1, FIX_3_624509785); /* sqrt(2) * (c1+c3+c5+c7) */

				/* Final output stage */	

				wsptr[DCTSIZE*0] = Descale(tmp10 + tmp0, CONST_BITS-PASS1_BITS+2);
				wsptr[DCTSIZE*1] = Descale(tmp10 - tmp0, CONST_BITS-PASS1_BITS+2);
			}
		}
		++inptr;
		++quantptr;
		++wsptr;
	} while (--Count);

	/* Pass 2: process 2 rows from work array, store into output array. */

	JSample_t *range_limit = cinfo->IDCTRangeLimit();
	wsptr = workspace;
	Word Count2 = 2;
	do {
		JSample_t *outptr = output_buf[0] + output_col;
		++output_buf;
		/* It's not clear whether a zero row test is worthwhile here ... */

		#ifndef NO_ZERO_ROW_TEST
		if (wsptr[1] == 0 && wsptr[3] == 0 && wsptr[5] == 0 && wsptr[7] == 0) {
			/* AC terms all zero */
			JSample_t dcval = range_limit[Descale(wsptr[0], PASS1_BITS+3)& RANGE_MASK];
			outptr[0] = dcval;
			outptr[1] = dcval;
		} else
		#endif
		{
			/* Even part */

			SWord32 tmp10 = wsptr[0] << (CONST_BITS+2);

			/* Odd part */

			SWord32 tmp0 = MyMul(wsptr[7], - FIX_0_720959822) /* sqrt(2) * (c7-c5+c3-c1) */
				+ MyMul(wsptr[5], FIX_0_850430095) /* sqrt(2) * (-c1+c3+c5+c7) */
				+ MyMul(wsptr[3], - FIX_1_272758580) /* sqrt(2) * (-c1+c3-c5-c7) */
				+ MyMul(wsptr[1], FIX_3_624509785); /* sqrt(2) * (c1+c3+c5+c7) */

			/* Final output stage */

			outptr[0] = range_limit[Descale(tmp10 + tmp0,CONST_BITS+PASS1_BITS+3+2)& RANGE_MASK];
			outptr[1] = range_limit[Descale(tmp10 - tmp0,CONST_BITS+PASS1_BITS+3+2)& RANGE_MASK];
		}
		wsptr += DCTSIZE;		/* advance pointer to next row */
	} while (--Count2);
}


/*******************************

	Perform dequantization and inverse DCT on one block of coefficients,
	producing a reduced-size 1x1 output block.

*******************************/

void BURGERCALL IDCT1x1(CJPegDecompress *cinfo,ComponentInfo_t *compptr,SWord16 *coef_block,JSample_t **output_buf,Word output_col)
{
	/* We hardly need an inverse DCT routine for this: just take the
	* average pixel value, which is one-eighth of the DC coefficient. */

	SWord32 *quantptr = compptr->DctTablePtr.Int;
	SWord32 dcval = Descale(Dequantize(coef_block[0], quantptr[0]),3);

	JSample_t *range_limit = cinfo->IDCTRangeLimit();
	output_buf[0][output_col] = range_limit[dcval & RANGE_MASK];
}

}