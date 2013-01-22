/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#include "JIDCTFlt.hpp"
#include "jpeglib.h"

namespace JPeg70 {

/*******************************

	This file contains a floating-point implementation of the
	inverse DCT (Discrete Cosine Transform).  In the IJG code, this routine
	must also perform dequantization of the input coefficients.

	This implementation should be more accurate than either of the integer
	IDCT implementations.  However, it may not give the same results on all
	machines because of differences in roundoff behavior.  Speed will depend
	on the hardware's floating point capacity.

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
	The primary disadvantage of this method is that with a fixed-point
	implementation, accuracy is lost due to imprecise representation of the
	scaled quantization values.  However, that problem does not arise if
	we use floating point arithmetic.

*******************************/

#if DCTSIZE != 8
	Sorry, this code only copes with 8x8 DCTs. /* deliberate syntax err */
#endif

/*******************************

	Float to index conversion with good type
	checking

*******************************/

inline Word DescaleMask(float x)
{
	int j = static_cast<int>(x)+(1<<(3-1));
	return static_cast<Word>((j>>3)&RANGE_MASK);
}

/*******************************

	Perform dequantization and inverse DCT on one block of coefficients.

*******************************/

void BURGERCALL IDCTFloat(CJPegDecompress *cinfo, ComponentInfo_t *compptr,SWord16 *coef_block,JSample_t **output_buf,Word output_col)
{
	float workspace[DCTSIZE2]; /* buffers data between passes */

	/* Pass 1: process columns from input, store into work array. */

	SWord16 *inptr = coef_block;
	float *quantptr = compptr->DctTablePtr.Float;
	float *wsptr = workspace;
	Word Count = DCTSIZE;
	do {
		/* Due to	quantization, we will usually find that	many of	the	input
		*	coefficients are zero, especially the AC terms.	 We	can	exploit	this
		*	by short-circuiting	the	IDCT calculation for any column	in which all
		*	the	AC terms are zero.	In that	case each output is	equal to the
		*	DC coefficient (with scale factor as needed).
		*	With typical images	and	quantization tables, half or more of the
		*	column DCT calculations	can	be simplified this way.
		*/

		if (inptr[DCTSIZE*1] == 0 && inptr[DCTSIZE*2] == 0 &&
			inptr[DCTSIZE*3] == 0 && inptr[DCTSIZE*4] == 0 &&
			inptr[DCTSIZE*5] == 0 && inptr[DCTSIZE*6] == 0 &&
			inptr[DCTSIZE*7] == 0) {
				/* AC	terms all zero */
				float dcval = (inptr[DCTSIZE*0]*quantptr[DCTSIZE*0]);

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

				float tmp0 = (inptr[DCTSIZE*0]* quantptr[DCTSIZE*0]);
				float tmp1 = (inptr[DCTSIZE*2]* quantptr[DCTSIZE*2]);
				float tmp2 = (inptr[DCTSIZE*4]* quantptr[DCTSIZE*4]);
				float tmp3 = (inptr[DCTSIZE*6]* quantptr[DCTSIZE*6]);

				float tmp10	= tmp0 + tmp2;	/* phase 3 */
				float tmp11	= tmp0 - tmp2;

				float tmp13	= tmp1 + tmp3;	/* phases 5-3 */
				float tmp12	= ((tmp1 - tmp3) * 1.414213562f) - tmp13; /* 2*c4 */

				tmp0 = tmp10 + tmp13;	/* phase 2 */
				tmp3 = tmp10 - tmp13;
				tmp1 = tmp11 + tmp12;
				tmp2 = tmp11 - tmp12;

				/* Odd part */

				float tmp4 = (inptr[DCTSIZE*1]* quantptr[DCTSIZE*1]);
				float tmp5 = (inptr[DCTSIZE*3]* quantptr[DCTSIZE*3]);
				float tmp6 = (inptr[DCTSIZE*5]* quantptr[DCTSIZE*5]);
				float tmp7 = (inptr[DCTSIZE*7]* quantptr[DCTSIZE*7]);

				float z13 =	tmp6 + tmp5;		/* phase 6 */
				float z10 =	tmp6 - tmp5;
				float z11 =	tmp4 + tmp7;
				float z12 =	tmp4 - tmp7;

				tmp7 = z11 + z13;		/* phase 5 */
				tmp11	= (z11 - z13) *	1.414213562f; /* 2*c4 */

				float z5 = (z10	+ z12) * 1.847759065f;	/* 2*c2	*/
				tmp10	= 1.082392200f	* z12 -	z5;	/* 2*(c2-c6) */
				tmp12	= -2.613125930f * z10 + z5; /*	-2*(c2+c6) */

				tmp6 = tmp12 - tmp7;	/* phase 2 */
				tmp5 = tmp11 - tmp6;
				tmp4 = tmp10 + tmp5;

				wsptr[DCTSIZE*0] = tmp0 + tmp7;
				wsptr[DCTSIZE*7] = tmp0 - tmp7;
				wsptr[DCTSIZE*1] = tmp1 + tmp6;
				wsptr[DCTSIZE*6] = tmp1 - tmp6;
				wsptr[DCTSIZE*2] = tmp2 + tmp5;
				wsptr[DCTSIZE*5] = tmp2 - tmp5;
				wsptr[DCTSIZE*4] = tmp3 + tmp4;
				wsptr[DCTSIZE*3] = tmp3 - tmp4;
			}
			++inptr;			/* advance pointers	to next	column */
			++quantptr;
			++wsptr;
		} while (--Count);

	/* Pass 2: process rows from work	array, store into output array.	*/
	/* Note that we must descale the results by a	factor of 8	== 2**3. */

	JSample_t *range_limit = cinfo->IDCTRangeLimit();
	wsptr = workspace;
	Count = DCTSIZE;
	do {
		JSample_t *outptr = output_buf[0] + output_col;
		++output_buf;
		/* Rows of zeroes	can	be exploited in	the	same way as	we did with	columns.
		*	However, the column	calculation	has	created	many nonzero AC	terms, so
		*	the	simplification applies less	often (typically 5%	to 10% of the time).
		*	And	testing	floats for zero	is relatively expensive, so	we don't bother.
		*/

		/* Even part */

		float btmp20 = wsptr[0] + wsptr[4];
		float btmp21 = wsptr[0] - wsptr[4];

		float btmp23 = wsptr[2] + wsptr[6];
		float btmp22 = ((wsptr[2] - wsptr[6]) * 1.414213562f) - btmp23;

		float btmp0 = btmp20 + btmp23;
		float btmp3 = btmp20 - btmp23;
		float btmp1 = btmp21 + btmp22;
		float btmp2 = btmp21 - btmp22;

		/* Odd part */

		float bz13 = wsptr[5] + wsptr[3];
		float bz10 = wsptr[5] - wsptr[3];
		float bz11 = wsptr[1] + wsptr[7];
		float bz12 = wsptr[1] - wsptr[7];

		float btmp7 = bz11 + bz13;
		float btmp11 = (bz11 - bz13) * 1.414213562f;

		float bz5 = (bz10 + bz12) * 1.847759065f; /* 2*c2 */
		float btmp10 = (1.082392200f * bz12) - bz5; /* 2*(c2-c6)	*/
		float btmp12 = (-2.613125930f * bz10) + bz5; /* -2*(c2+c6)	*/

		float btmp6 = btmp12 - btmp7;
		float btmp5 = btmp11 - btmp6;
		float btmp4 = btmp10 + btmp5;

		/* Final output stage: scale down	by a factor	of 8 and range-limit */

		outptr[0] = range_limit[DescaleMask(btmp0 + btmp7)];
		outptr[7] = range_limit[DescaleMask(btmp0 - btmp7)];
		outptr[1] = range_limit[DescaleMask(btmp1 + btmp6)];
		outptr[6] = range_limit[DescaleMask(btmp1 - btmp6)];
		outptr[2] = range_limit[DescaleMask(btmp2 + btmp5)];
		outptr[5] = range_limit[DescaleMask(btmp2 - btmp5)];
		outptr[4] = range_limit[DescaleMask(btmp3 + btmp4)];
		outptr[3] =	range_limit[DescaleMask(btmp3 - btmp4)];

		wsptr += DCTSIZE;		/* advance pointer to next row */
	} while (--Count);
}

}