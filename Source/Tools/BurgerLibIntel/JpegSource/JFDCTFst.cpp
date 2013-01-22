/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#include "JFDCTFst.hpp"

namespace JPeg70 {

/*******************************


	This file contains a fast, not so accurate integer implementation of the
	forward DCT (Discrete Cosine Transform).

	A 2-D DCT can be done by 1-D DCT on each row followed by 1-D DCT
	on each column.  Direct algorithms are also available, but they are
	much more complex and seem not to be any faster when reduced to code.

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

#if DCTSIZE != 8
	Sorry, this code only copes with 8x8 DCTs. /* deliberate syntax err */
#endif


/*******************************

	Scaling decisions are generally the same as in the LL&M algorithm;
	see jfdctint.c for more details.  However, we choose to descale
	(right shift) multiplication products as soon as they are formed,
	rather than carrying additional fractional bits into subsequent additions.
	This compromises accuracy slightly, but it lets us save a few shifts.
	More importantly, 16-bit arithmetic is then adequate (for 8-bit samples)
	everywhere except in the multiplications proper; this saves a good deal
	of work on 16-bit-int machines.

	Again to save a few shifts, the intermediate results between pass 1 and
	pass 2 are not upscaled, but are represented only to integral precision.

	A final compromise is to represent the multiplicative constants to only
	8 fractional bits, rather than 13.  This saves some shifting work on some
	machines, and may also reduce the cost of multiplication (since there
	are fewer one-bits in the constants).

*******************************/

#define CONST_BITS 8

/*******************************

	Some C compilers fail to reduce "FIX(constant)" at compile time, thus
	causing a lot of useless floating-point operations at run time.
	To get around this we use the following pre-calculated constants.
	If you change CONST_BITS you may want to add appropriate values.
	(With a reasonable C compiler, you can just rely on the FIX() macro...)

*******************************/

#if CONST_BITS == 8
#define FIX_0_382683433  ( 0x62)	/* FIX(0.382683433) */
#define FIX_0_541196100  ( 0x8B)	/* FIX(0.541196100) */
#define FIX_0_707106781  ( 0xB5)	/* FIX(0.707106781) */
#define FIX_1_306562965  (0x14E)	/* FIX(1.306562965) */
#else
#define FIX_0_382683433  (SWord32)(0.382683433*(1<<CONST_BITS)+0.5f)
#define FIX_0_541196100  (SWord32)(0.541196100*(1<<CONST_BITS)+0.5f)
#define FIX_0_707106781  (SWord32)(0.707106781*(1<<CONST_BITS)+0.5f)
#define FIX_1_306562965  (SWord32)(1.306562965*(1<<CONST_BITS)+0.5f)
#endif

/* Multiply a SWord32 variable by an long constant, and immediately */
/* descale to yield a SWord32 result. */

inline SWord32 JFixMul(SWord32 a,SWord32 b)
{
	return ((a*b)+(1<<(CONST_BITS-1)))>>CONST_BITS;
}

/*******************************

	Perform the forward DCT on one block of samples.

*******************************/

void BURGERCALL FDCTIFast(SWord32 *data)
{
	/* Pass 1: process rows. */

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

		/* Even part */

		SWord32 tmp10 = tmp0 + tmp3;	/* phase 2 */
		SWord32 tmp13 = tmp0 - tmp3;
		SWord32 tmp11 = tmp1 + tmp2;
		SWord32 tmp12 = tmp1 - tmp2;

		dataptr[0] = tmp10 + tmp11; /* phase 3 */
		dataptr[4] = tmp10 - tmp11;

		SWord32 z1 = JFixMul(tmp12 + tmp13, FIX_0_707106781); /* c4 */
		dataptr[2] = tmp13 + z1;	/* phase 5 */
		dataptr[6] = tmp13 - z1;

		/* Odd part */

		tmp10 = tmp4 + tmp5;	/* phase 2 */
		tmp11 = tmp5 + tmp6;
		tmp12 = tmp6 + tmp7;

		/* The rotator is modified from fig 4-8 to avoid extra negations. */
		SWord32 z5 = JFixMul(tmp10 - tmp12, FIX_0_382683433); /* c6 */
		SWord32 z2 = JFixMul(tmp10, FIX_0_541196100) + z5; /* c2-c6 */
		SWord32 z4 = JFixMul(tmp12, FIX_1_306562965) + z5; /* c2+c6 */
		SWord32 z3 = JFixMul(tmp11, FIX_0_707106781); /* c4 */

		SWord32 z11 = tmp7 + z3;		/* phase 5 */
		SWord32 z13 = tmp7 - z3;

		dataptr[5] = z13 + z2;	/* phase 6 */
		dataptr[3] = z13 - z2;
		dataptr[1] = z11 + z4;
		dataptr[7] = z11 - z4;

		dataptr += DCTSIZE;		/* advance pointer to next row */
	} while (--Count);

	/* Pass 2: process columns. */

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

		/* Even part */

		SWord32 btmp10 = btmp0 + btmp3;	/* phase 2 */
		SWord32 btmp13 = btmp0 - btmp3;
		SWord32 btmp11 = btmp1 + btmp2;
		SWord32 btmp12 = btmp1 - btmp2;

		dataptr[DCTSIZE*0] = btmp10 + btmp11; /* phase 3 */
		dataptr[DCTSIZE*4] = btmp10 - btmp11;

		SWord32 bz1 = JFixMul(btmp12 + btmp13, FIX_0_707106781); /* c4 */
		dataptr[DCTSIZE*2] = btmp13 + bz1; /* phase 5 */
		dataptr[DCTSIZE*6] = btmp13 - bz1;

		/* Odd part */

		btmp10 = btmp4 + btmp5;	/* phase 2 */
		btmp11 = btmp5 + btmp6;
		btmp12 = btmp6 + btmp7;

		/* The rotator is modified from fig 4-8 to avoid extra negations. */
		SWord32 bz5 = JFixMul(btmp10 - btmp12, FIX_0_382683433); /* c6 */
		SWord32 bz2 = JFixMul(btmp10, FIX_0_541196100) + bz5; /* c2-c6 */
		SWord32 bz4 = JFixMul(btmp12, FIX_1_306562965) + bz5; /* c2+c6 */
		SWord32 bz3 = JFixMul(btmp11, FIX_0_707106781); /* c4 */

		SWord32 bz11 = btmp7 + bz3;		/* phase 5 */
		SWord32 bz13 = btmp7 - bz3;

		dataptr[DCTSIZE*5] = bz13 + bz2; /* phase 6 */
		dataptr[DCTSIZE*3] = bz13 - bz2;
		dataptr[DCTSIZE*1] = bz11 + bz4;
		dataptr[DCTSIZE*7] = bz11 - bz4;

		++dataptr;			/* advance pointer to next column */
	} while (--Count);
}

}