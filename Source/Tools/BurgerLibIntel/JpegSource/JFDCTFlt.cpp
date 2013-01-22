/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#include "JFDCTFlt.hpp"

namespace JPeg70 {

/*******************************

	This file contains a floating-point implementation of the
	forward DCT (Discrete Cosine Transform).
	
	This implementation should be more accurate than either of the integer
	DCT implementations.  However, it may not give the same results on all
	machines because of differences in roundoff behavior.  Speed will depend
	on the hardware's floating point capacity.
	
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
	
	The primary disadvantage of this method is that with a fixed-point
	implementation, accuracy is lost due to imprecise representation of the
	scaled quantization values.  However, that problem does not arise if
	we use floating point arithmetic.

*******************************/

#if DCTSIZE != 8
	Sorry, this code only copes with 8x8 DCTs. /* deliberate syntax err */
#endif

void BURGERCALL FDCTFloat(float *data)
{
	/* Pass 1: process rows. */

	float *dataptr = data;
	Word Count = DCTSIZE;
	do {
		float tmp0 = dataptr[0] + dataptr[7];
		float tmp7 = dataptr[0] - dataptr[7];
		float tmp1 = dataptr[1] + dataptr[6];
		float tmp6 = dataptr[1] - dataptr[6];
		float tmp2 = dataptr[2] + dataptr[5];
		float tmp5 = dataptr[2] - dataptr[5];
		float tmp3 = dataptr[3] + dataptr[4];
		float tmp4 = dataptr[3] - dataptr[4];

		/* Even part */

		float tmp10 = tmp0 + tmp3;	/* phase 2 */
		float tmp13 = tmp0 - tmp3;
		float tmp11 = tmp1 + tmp2;
		float tmp12 = tmp1 - tmp2;

		dataptr[0] = tmp10 + tmp11; /* phase 3 */
		dataptr[4] = tmp10 - tmp11;

		float z1 = (tmp12 + tmp13) * 0.707106781f; /* c4 */
		dataptr[2] = tmp13 + z1;	/* phase 5 */
		dataptr[6] = tmp13 - z1;

		/* Odd part */

		tmp10 = tmp4 + tmp5;	/* phase 2 */
		tmp11 = tmp5 + tmp6;
		tmp12 = tmp6 + tmp7;

		/* The rotator is modified from fig 4-8 to avoid extra negations. */
		float z5 = (tmp10 - tmp12) * 0.382683433f; /* c6 */
		float z2 = 0.541196100f * tmp10 + z5; /* c2-c6 */
		float z4 = 1.306562965f * tmp12 + z5; /* c2+c6 */
		float z3 = tmp11 * 0.707106781f; /* c4 */

		float z11 = tmp7 + z3;		/* phase 5 */
		float z13 = tmp7 - z3;

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
		float btmp0 = dataptr[DCTSIZE*0] + dataptr[DCTSIZE*7];
		float btmp7 = dataptr[DCTSIZE*0] - dataptr[DCTSIZE*7];
		float btmp1 = dataptr[DCTSIZE*1] + dataptr[DCTSIZE*6];
		float btmp6 = dataptr[DCTSIZE*1] - dataptr[DCTSIZE*6];
		float btmp2 = dataptr[DCTSIZE*2] + dataptr[DCTSIZE*5];
		float btmp5 = dataptr[DCTSIZE*2] - dataptr[DCTSIZE*5];
		float btmp3 = dataptr[DCTSIZE*3] + dataptr[DCTSIZE*4];
		float btmp4 = dataptr[DCTSIZE*3] - dataptr[DCTSIZE*4];

		/* Even part */

		float btmp10 = btmp0 + btmp3;	/* phase 2 */
		float btmp13 = btmp0 - btmp3;
		float btmp11 = btmp1 + btmp2;
		float btmp12 = btmp1 - btmp2;

		dataptr[DCTSIZE*0] = btmp10 + btmp11; /* phase 3 */
		dataptr[DCTSIZE*4] = btmp10 - btmp11;

		float bz1 = (btmp12 + btmp13) * 0.707106781f; /* c4 */
		dataptr[DCTSIZE*2] = btmp13 + bz1; /* phase 5 */
		dataptr[DCTSIZE*6] = btmp13 - bz1;

		/* Odd part */

		btmp10 = btmp4 + btmp5;	/* phase 2 */
		btmp11 = btmp5 + btmp6;
		btmp12 = btmp6 + btmp7;

		/* The rotator is modified from fig 4-8 to avoid extra negations. */
		float bz5 = (btmp10 - btmp12) * 0.382683433f; /* c6 */
		float bz2 = 0.541196100f * btmp10 + bz5; /* c2-c6 */
		float bz4 = 1.306562965f * btmp12 + bz5; /* c2+c6 */
		float bz3 = btmp11 * 0.707106781f; /* c4 */

		float bz11 = btmp7 + bz3;		/* phase 5 */
		float bz13 = btmp7 - bz3;

		dataptr[DCTSIZE*5] = bz13 + bz2; /* phase 6 */
		dataptr[DCTSIZE*3] = bz13 - bz2;
		dataptr[DCTSIZE*1] = bz11 + bz4;
		dataptr[DCTSIZE*7] = bz11 - bz4;

		++dataptr;			/* advance pointer to next column */
	} while (--Count);
}

}