/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

	This file contains 1-pass color quantization (color mapping) routines.
	These routines provide mapping to a fixed color map using equally spaced
	color values.  Optional Floyd-Steinberg or ordered dithering is available.

*******************************/

#include "JQuant1.hpp"
#include "jpeglib.h"
#include <Burger.h>

namespace JPeg70 {

/*******************************

	The main purpose of 1-pass quantization is to provide a fast, if not very
	high quality, colormapped output capability.  A 2-pass quantizer usually
	gives better visual quality; however, for quantized grayscale output this
	quantizer is perfectly adequate.  Dithering is highly recommended with this
	quantizer, though you can turn it off if you really want to.

	In 1-pass quantization the colormap must be chosen in advance of seeing the
	image.  We use a map consisting of all combinations of Ncolors[i] color
	values for the i'th component.  The Ncolors[] values are chosen so that
	their product, the total number of colors, is no more than that requested.
	(In most cases, the product will be somewhat less.)

	Since the colormap is orthogonal, the representative value for each color
	component can be determined without considering the other components;
	then these indexes can be combined into a colormap index by a standard
	N-dimensional-array-subscript calculation.  Most of the arithmetic involved
	can be precalculated and stored in the lookup table colorindex[].
	colorindex[i][j] maps pixel value j in component i to the nearest
	representative value (grid plane) for that component; this index is
	multiplied by the array stride for component i, so that the
	index of the colormap entry closest to a given pixel value is just
	sum( colorindex[component-number][pixel-component-value] )
	Aside from being fast, this scheme allows for variable spacing between
	representative values with no additional lookup cost.

	If gamma correction has been applied in color conversion, it might be wise
	to adjust the color grid spacing so that the representative colors are
	equidistant in linear space.  At this writing, gamma correction is not
	implemented by jdcolor, so nothing is done here.

	Declarations for ordered dithering.

	We use a standard 16x16 ordered dither array.  The basic concept of ordered
	dithering is described in many references, for instance Dale Schumacher's
	chapter II.2 of Graphics Gems II (James Arvo, ed. Academic Press, 1991).
	In place of Schumacher's comparisons against a "threshold" value, we add a
	"dither" value to the input pixel and then round the result to the nearest
	output value.  The dither value is equivalent to (0.5 - threshold) times
	the distance between output values.  For ordered dithering, we assume that
	the output colors are equally spaced; if not, results will probably be
	worse, since the dither may be too much or too little at a given point.

	The normal calculation would be to form pixel value + dither, range-limit
	this to 0..MAXJSAMPLE, and then index into the colorindex table as usual.
	We can skip the separate range-limiting step by extending the colorindex
	table in both directions.

*******************************/

/*******************************

	Bayer's order-4 dither array.  Generated by the code given in
	Stephen Hawley's article "Ordered Dithering" in Graphics Gems I.
	The values in this array must range from 0 to ODITHER_CELLS-1.
	
*******************************/

const Word8 CColorQuantizer1Pass::BaseDitherMatrix[ODITHER_SIZE][ODITHER_SIZE] = {
	{   0,192, 48,240, 12,204, 60,252,  3,195, 51,243, 15,207, 63,255 },
	{ 128, 64,176,112,140, 76,188,124,131, 67,179,115,143, 79,191,127 },
	{  32,224, 16,208, 44,236, 28,220, 35,227, 19,211, 47,239, 31,223 },
	{ 160, 96,144, 80,172,108,156, 92,163, 99,147, 83,175,111,159, 95 },
	{   8,200, 56,248,  4,196, 52,244, 11,203, 59,251,  7,199, 55,247 },
	{ 136, 72,184,120,132, 68,180,116,139, 75,187,123,135, 71,183,119 },
	{  40,232, 24,216, 36,228, 20,212, 43,235, 27,219, 39,231, 23,215 },
	{ 168,104,152, 88,164,100,148, 84,171,107,155, 91,167,103,151, 87 },
	{   2,194, 50,242, 14,206, 62,254,  1,193, 49,241, 13,205, 61,253 },
	{ 130, 66,178,114,142, 78,190,126,129, 65,177,113,141, 77,189,125 },
	{  34,226, 18,210, 46,238, 30,222, 33,225, 17,209, 45,237, 29,221 },
	{ 162, 98,146, 82,174,110,158, 94,161, 97,145, 81,173,109,157, 93 },
	{  10,202, 58,250,  6,198, 54,246,  9,201, 57,249,  5,197, 53,245 },
	{ 138, 74,186,122,134, 70,182,118,137, 73,185,121,133, 69,181,117 },
	{  42,234, 26,218, 38,230, 22,214, 41,233, 25,217, 37,229, 21,213 },
	{ 170,106,154, 90,166,102,150, 86,169,105,153, 89,165,101,149, 85 }
};

static const int RGB_order[3] = { RGB_GREEN, RGB_RED, RGB_BLUE };

/*******************************

	Declarations for Floyd-Steinberg dithering.

	Errors are accumulated into the array m_FSErrors[], at a resolution of
	1/16th of a pixel count.  The error at a given pixel is propagated
	to its not-yet-processed neighbors using the standard F-S fractions,
	...	(here)	7/16
		3/16	5/16	1/16
	We work left-to-right on even rows, right-to-left on odd rows.

	We can get away with a single array (holding one row's worth of errors)
	by using it to store the current row's errors at pixel columns not yet
	processed, but the next row's errors at columns already processed.  We
	need only a few extra variables to hold the errors immediately around the
	current column.  (If we are lucky, those variables are in registers, but
	even if not, they're probably cheaper to access than array elements are.)

	The m_FSErrors[] array is indexed [component#][position].
	We provide (#columns + 2) entries per component; the extra entry at each
	end saves us from special-casing the first and last pixels.

	Note: on a wide image, we might not have enough room in a PC's near data
	segment to hold the error array; so it is allocated with alloc_large.

*******************************/

/*******************************

	Policy-making subroutines for CreateColorMap and CreateColorIndex.
	These routines determine the colormap to be used.  The rest of the module
	only assumes that the colormap is orthogonal.

	* SelectNColors decides how to divvy up the available colors
		among the components.
	* output_value defines the set of representative values for a component.
	* largest_input_value defines the mapping from input values to
		representative values for a component.
	Note that the latter two routines may impose different policies for
	different components, though this is not currently done.

*******************************/

/*******************************

	Determine allocation of desired colors to components,
	and fill in Ncolors[] array to indicate choice.
	Return value is total number of colors (product of Ncolors[] values).

*******************************/

inline Word CColorQuantizer1Pass::SelectNColors(int *Ncolors)
{
	int nc = m_Parent->m_ColorComponents; /* number of color components */
	Word max_colors = static_cast<Word>(m_Parent->desired_number_of_colors);

	/* We can allocate at least the nc'th root of max_colors per component. */
	/* Compute floor(nc'th root of max_colors). */

	Word iroot = 1;
	Word temp;
	do {
		iroot++;
		temp = iroot;		/* set temp = iroot ** nc */
		for (int i3 = 1; i3 < nc; i3++) {
			temp *= iroot;
		}
	} while (temp <= max_colors); /* repeat till iroot exceeds root */
	iroot--;			/* now iroot = floor(root) */

	/* Must have at least 2 color values per component */
	if (iroot < 2) {
		m_Parent->FatalError(JERR_QUANT_FEW_COLORS,static_cast<int>(temp));
		return 0;
	}
	/* Initialize to iroot color values for each component */
	Word total_colors = 1;
	for (int i2 = 0; i2 < nc; i2++) {
		Ncolors[i2] = static_cast<int>(iroot);
		total_colors *= iroot;
	}
	/* We may be able to increment the count for one or more components without
	* exceeding max_colors, though we know not all can be incremented.
	* Sometimes, the first component can be incremented more than once!
	* (Example: for 16 colors, we start at 2*2*2, go to 3*2*2, then 4*2*2.)
	* In RGB colorspace, try to increment G first, then R, then B.
	*/
	Bool changed = FALSE;
	do {
		for (int i = 0; i < nc; i++) {
			int j = (m_Parent->m_OutputColorSpace == CS_RGB ? RGB_order[i] : i);
			/* calculate new total_colors if Ncolors[j] is incremented */
			Word temp2 = total_colors / Ncolors[j];
			temp2 *= Ncolors[j]+1;	/* done in long arith to avoid oflo */
			if (temp2 > max_colors) {
				break;			/* won't fit, done with this pass */
			}
			Ncolors[j]++;		/* OK, apply the increment */
			total_colors = temp2;
			changed = TRUE;
		}
	} while (changed);
	return total_colors;
}

/*******************************

	Return j'th output value, where j will range from 0 to maxj
	The output values must fall in 0..MAXJSAMPLE in increasing order

	We always provide values 0 and MAXJSAMPLE for each component;
	any additional values are equally spaced between these limits.
	(Forcing the upper and lower values to the limits ensures that
	dithering can't produce a color outside the selected gamut.)

*******************************/

inline int CColorQuantizer1Pass::OutputValue(int j, int maxj)
{
	return ((j * MAXJSAMPLE + maxj/2) / maxj);
}


/*******************************

	Return largest input value that should map to j'th output value
	Must have largest(j=0) >= 0, and largest(j=maxj) >= MAXJSAMPLE

*******************************/

inline int CColorQuantizer1Pass::LargestInputValue(int j, int maxj)
{
	/* Breakpoints are halfway between values returned by output_value */
	return (((2*j + 1) * MAXJSAMPLE + maxj) / (2*maxj));
}

/*******************************

	Create the colormap.

*******************************/

void CColorQuantizer1Pass::CreateColorMap(void)
{
	/* Select number of colors for each component */
	Word total_colors = SelectNColors(m_NColors);

	/* Report selected color counts */
	if (m_Parent->m_ColorComponents == 3) {
		m_Parent->EmitTrace(1,JTRC_QUANT_3_NCOLORS,static_cast<int>(total_colors),m_NColors[0],m_NColors[1],m_NColors[2]);
	} else {
		m_Parent->EmitTrace(1,JTRC_QUANT_NCOLORS,static_cast<int>(total_colors));
	}
	/* Allocate and fill in the colormap. */
	/* The colors are ordered in the map in standard row-major order, */
	/* i.e. rightmost (highest-indexed) color changes most rapidly. */

	JSample_t **colormap = m_Parent->AllocSArray(total_colors,(Word)m_Parent->m_ColorComponents);
	if (colormap) {
		/* blksize is number of adjacent repeated entries for a component */
		/* blkdist is distance between groups of identical entries for a component */
		Word blkdist = total_colors;

		int nc = m_Parent->m_ColorComponents;
		for (int i = 0; i < nc; i++) {
			/* fill in colormap entries for i'th color component */
			int nci = m_NColors[i]; /* # of distinct values for this color */
			Word blksize = blkdist / nci;
			for (int j = 0; j < nci; j++) {
				/* Compute j'th output value (out of nci) for component */
				int val = OutputValue(j, nci-1);
				/* Fill in all colormap entries that have this value of this component */
				for (Word ptr = j * blksize; ptr < total_colors; ptr += blkdist) {
					/* fill in blksize entries beginning at ptr */
					for (Word k = 0; k < blksize; k++) {
						colormap[i][ptr+k] = static_cast<JSample_t>(val);
					}
				}
			}
			blkdist = blksize;		/* blksize of this color is blkdist of next */
		}
	}
	/* Save the colormap in private storage,
	* where it will survive color quantization mode changes.
	*/
	m_SvColorMapPtr = colormap;
	m_SvActual = total_colors;
}

/*******************************

	Create the color index table.

*******************************/

void CColorQuantizer1Pass::CreateColorIndex(void)
{
	int pad;

	/* For ordered dither, we pad the color index tables by MAXJSAMPLE in
	* each direction (input index values can be -MAXJSAMPLE .. 2*MAXJSAMPLE).
	* This is not necessary in the other dithering modes.  However, we
	* flag whether it was done in case user changes dithering mode.
	*/
	if (m_Parent->dither_mode == DITHER_ORDERED) {
		pad = MAXJSAMPLE*2;
		m_IsPadded = TRUE;
	} else {
		pad = 0;
		m_IsPadded = FALSE;
	}

	m_ColorIndex = m_Parent->AllocSArray((Word)(MAXJSAMPLE+1 + pad),(Word) m_Parent->m_ColorComponents);
	if (m_ColorIndex) {
		/* blksize is number of adjacent repeated entries for a component */
		Word blksize = m_SvActual;
		int nc = m_Parent->m_ColorComponents;
		for (int i = 0; i < nc; i++) {
			/* fill in colorindex entries for i'th color component */
			int nci = m_NColors[i]; /* # of distinct values for this color */
			blksize = blksize / nci;

			/* adjust colorindex pointers to provide padding at negative indexes. */
			if (pad) {
				m_ColorIndex[i] += MAXJSAMPLE;
			}
			/* in loop, val = index of current output value, */
			/* and k = largest j that maps to current val */
			JSample_t *indexptr = m_ColorIndex[i];
			int val = 0;
			int k = LargestInputValue(0, nci-1);
			for (int j = 0; j <= MAXJSAMPLE; j++) {
				while (j > k) {				/* advance val if past boundary */
					k = LargestInputValue(++val, nci-1);
				}
				/* premultiply so that no multiplication needed in main processing */
				indexptr[j] = (JSample_t) (val * blksize);
			}
			/* Pad at both ends if necessary */
			if (pad) {
				int m=1;
				do {
					indexptr[-m] = indexptr[0];
					indexptr[MAXJSAMPLE+m] = indexptr[MAXJSAMPLE];
				} while (++m<=MAXJSAMPLE);
			}
		}
	}
}

/*******************************

	Create an ordered-dither array for a component having ncolors
	distinct output values.
	
*******************************/

inline CColorQuantizer1Pass::ODitherMatrixPtr CColorQuantizer1Pass::MakeODitherArray(int ncolors)
{
	ODitherMatrixPtr odither = new ODitherMatrix_t;
	if (odither) {
		/* The inter-value distance for this color is MAXJSAMPLE/(ncolors-1).
		* Hence the dither value for the matrix cell with fill order f
		* (f=0..N-1) should be (N-1-2*f)/(2*N) * MAXJSAMPLE/(ncolors-1).
		* On 16-bit-int machine, be careful to avoid overflow.
		*/
		int den = 2 * ODITHER_CELLS * (ncolors - 1);
		Word j = 0;
		do {
			Word k = 0;
			do {
				int num = (((ODITHER_CELLS-1) - 2*(static_cast<int>(BaseDitherMatrix[j][k])))) * MAXJSAMPLE;
				/* Ensure round towards zero despite C's lack of consistency
				* about rounding negative values in integer division... */
				odither[j][k] = (num<0 ? -((-num)/den) : num/den);
			} while (++k<ODITHER_SIZE);
		} while (++j<ODITHER_SIZE);
	} else {
		m_Parent->FatalError(JERR_OUT_OF_MEMORY,sizeof(ODitherMatrix_t));
	}
	return odither;
}

/*******************************

	Create the ordered-dither tables.
	Components having the same number of representative colors may 
	share a dither table.
	
*******************************/

void CColorQuantizer1Pass::CreateODitherTables(void)
{
	int nc = m_Parent->m_ColorComponents;
	for (int i = 0; i < nc; i++) {
		int nci = m_NColors[i]; /* # of distinct values for this color */
		ODitherMatrixPtr odither = 0;		/* search for matching prior component */
		for (int j = 0; j < i; j++) {
			if (nci == m_NColors[j]) {
				odither = m_ODither[j];
				break;
			}
		}
		if (!odither) {	/* need a new table? */
			odither = MakeODitherArray(nci);
		}
		m_ODither[i] = odither;
	}
}

/*******************************

	Map some rows of pixels to the output colormapped representation.
	
	General case, no dithering

*******************************/

void BURGERCALL CColorQuantizer1Pass::ColorQuantize(CColorQuantizer *ThisPtr,JSample_t **input_buf,JSample_t **output_buf, int num_rows)
{
	if (num_rows>0) {
		Word width = ThisPtr->m_Parent->output_width;
		if (width) {
			JSample_t ** colorindex = reinterpret_cast<CColorQuantizer1Pass*>(ThisPtr)->m_ColorIndex;
			int nc = ThisPtr->m_Parent->m_ColorComponents;
			int row = 0;
			do {
				JSample_t *ptrin = input_buf[row];
				JSample_t *ptrout = output_buf[row];
				Word col = width;
				do {
					Word pixcode = 0;
					for (int ci=0; ci < nc; ci++) {
						pixcode += colorindex[ci][ptrin[0]];
						++ptrin;
					}	
					ptrout[0] = static_cast<JSample_t>(pixcode);
					++ptrout;
				} while (--col);
			} while (++row<num_rows);
		}
	}
}

/*******************************

	Fast path for m_ColorComponents==3, no dithering

*******************************/

void BURGERCALL CColorQuantizer1Pass::ColorQuantize3(CColorQuantizer *ThisPtr2,JSample_t **input_buf,JSample_t ** output_buf,int num_rows)
{
	if (num_rows>0) {
		CColorQuantizer1Pass *ThisPtr = reinterpret_cast<CColorQuantizer1Pass*>(ThisPtr2);
		Word width = ThisPtr->m_Parent->output_width;
		if (width) {
			JSample_t **ColorIndex = ThisPtr->m_ColorIndex;
			JSample_t *colorindex0 = ColorIndex[0];
			JSample_t *colorindex1 = ColorIndex[1];
			JSample_t *colorindex2 = ColorIndex[2];
			int row = 0;
			do {
				JSample_t *ptrin = input_buf[row];
				JSample_t *ptrout = output_buf[row];
				Word col = width;
				do {
					Word pixcode = colorindex0[ptrin[0]];
					pixcode += colorindex1[ptrin[1]];
					pixcode += colorindex2[ptrin[2]];
					ptrin+=3;
					ptrout[0] = static_cast<JSample_t>(pixcode);
					++ptrout;
				} while (--col);
			} while (++row<num_rows);
		}
	}
}


/*******************************

	General case, with ordered dithering

*******************************/

void BURGERCALL CColorQuantizer1Pass::QuantizeOrdDither(CColorQuantizer *ThisPtr2,JSample_t ** input_buf,JSample_t ** output_buf,int num_rows)
{
	if (num_rows>0) {
		CColorQuantizer1Pass *ThisPtr = reinterpret_cast<CColorQuantizer1Pass *>(ThisPtr2);
		Word width = ThisPtr->m_Parent->output_width;
		if (width) {
			int nc = ThisPtr->m_Parent->m_ColorComponents;
			int row = 0;
			Word row_index = ThisPtr->m_RowIndex;
			do {
				/* Initialize output values to 0 so can process components separately */
				MemZero(reinterpret_cast<Word8*>(output_buf[row]),(width * sizeof(JSample_t)));
				for (int ci = 0; ci < nc; ci++) {
					JSample_t *input_ptr = input_buf[row] + ci;
					JSample_t *output_ptr = output_buf[row];
					JSample_t *colorindex_ci = ThisPtr->m_ColorIndex[ci];
					int *dither = ThisPtr->m_ODither[ci][row_index];
					Word col_index = 0;
					Word col = width;
					do {
						/* Form pixel value + dither, range-limit to 0..MAXJSAMPLE,
						* select output value, accumulate into output code for this pixel.
						* Range-limiting need not be done explicitly, as we have extended
						* the colorindex table to produce the right answers for out-of-range
						* inputs.  The maximum dither is +- MAXJSAMPLE; this sets the
						* required amount of padding.
						*/
						output_ptr[0] += colorindex_ci[input_ptr[0]+dither[col_index]];
						input_ptr += nc;
						++output_ptr;
						col_index = (col_index + 1) & ODITHER_MASK;
					} while (--col);
				}
				/* Advance row index for next row */
				row_index = (row_index + 1) & ODITHER_MASK;
			} while (++row<num_rows);
			ThisPtr->m_RowIndex = row_index;
		}
	}
}

/*******************************

	Fast path for m_ColorComponents==3, with ordered dithering

*******************************/

void BURGERCALL CColorQuantizer1Pass::Quantize3OrdDither(CColorQuantizer *ThisPtr2,JSample_t ** input_buf,JSample_t ** output_buf,int num_rows)
{
	if (num_rows>0) {
		CColorQuantizer1Pass *ThisPtr = reinterpret_cast<CColorQuantizer1Pass *>(ThisPtr2);
		Word width = ThisPtr->m_Parent->output_width;
		if (width) {
			JSample_t **ColorIndex = ThisPtr->m_ColorIndex;
			JSample_t *colorindex0 = ColorIndex[0];
			JSample_t *colorindex1 = ColorIndex[1];
			JSample_t *colorindex2 = ColorIndex[2];
			int row = 0;
			Word row_index = ThisPtr->m_RowIndex;
			do {
				JSample_t *input_ptr = input_buf[row];
				JSample_t *output_ptr = output_buf[row];
				int *dither0 = ThisPtr->m_ODither[0][row_index];
				int *dither1 = ThisPtr->m_ODither[1][row_index];
				int *dither2 = ThisPtr->m_ODither[2][row_index];
				Word col_index = 0;
				Word col = width;
				do {
					Word pixcode  = colorindex0[input_ptr[0] + dither0[col_index]];
					pixcode += colorindex1[input_ptr[1] + dither1[col_index]];
					pixcode += colorindex2[input_ptr[2] + dither2[col_index]];
					input_ptr+=3;
					output_ptr[0] = static_cast<JSample_t>(pixcode);
					++output_ptr;
					col_index = (col_index + 1) & ODITHER_MASK;
				} while (--col);
				row_index = (row_index + 1) & ODITHER_MASK;
			} while (++row<num_rows);
			ThisPtr->m_RowIndex = row_index;
		}
	}
}


/*******************************

	General case, with Floyd-Steinberg dithering

*******************************/

void BURGERCALL CColorQuantizer1Pass::QuantizeFSDither(CColorQuantizer *ThisPtr2,JSample_t ** input_buf,JSample_t ** output_buf,int num_rows)
{
	if (num_rows>0) {
		CColorQuantizer1Pass *ThisPtr = reinterpret_cast<CColorQuantizer1Pass *>(ThisPtr2);
		Word width = ThisPtr->m_Parent->output_width;
		if (width) {
			int nc = ThisPtr->m_Parent->m_ColorComponents;
			JSample_t *range_limit = ThisPtr->m_Parent->sample_range_limit;

			int row = 0;
			do {
				/* Initialize output values to 0 so can process components separately */
				MemZero(reinterpret_cast<Word8*>(output_buf[row]),width * sizeof(JSample_t));
				for (int ci = 0; ci < nc; ci++) {
					JSample_t *input_ptr = input_buf[row] + ci;
					JSample_t *output_ptr = output_buf[row];
					int dir;			/* 1 for left-to-right, -1 for right-to-left */
					int dirnc;			/* dir * nc */
					SWord32 * errorptr;	/* => m_FSErrors[] at column before current */
					if (ThisPtr->m_OnOddRow) {
						/* work right to left in this row */
						input_ptr += (width-1) * nc; /* so point to rightmost pixel */
						output_ptr += width-1;
						dir = -1;
						dirnc = -nc;
						errorptr = ThisPtr->m_FSErrors[ci] + (width+1); /* => entry after last column */
					} else {
						/* work left to right in this row */
						dir = 1;
						dirnc = nc;
						errorptr = ThisPtr->m_FSErrors[ci]; /* => entry before first column */
					}
					JSample_t *colorindex_ci = ThisPtr->m_ColorIndex[ci];
					JSample_t *colormap_ci = ThisPtr->m_SvColorMapPtr[ci];
					/* Preset error values: no error propagated to first pixel from left */
					/* and no error propagated to row below yet */
					SWord32 cur=0;				/* current error or pixel value */
					SWord32 belowerr = 0;		/* error for pixel below cur */
					SWord32 bpreverr = 0;		/* error for below/prev col */
					Word col = width;
					do {
						/* cur holds the error propagated from the previous pixel on the
						* current line.  Add the error propagated from the previous line
						* to form the complete error correction term for this pixel, and
						* round the error term (which is expressed * 16) to an integer.
						* RIGHT_SHIFT rounds towards minus infinity, so adding 8 is correct
						* for either sign of the error value.
						* Note: errorptr points to *previous* column's array entry.
						*/
						cur = ((cur + errorptr[dir] + 8)>> 4);
						/* Form pixel value + error, and range-limit to 0..MAXJSAMPLE.
						* The maximum error is +- MAXJSAMPLE; this sets the required size
						* of the range_limit array.
						*/
						cur += input_ptr[0];
						cur = range_limit[cur];
						/* Select output value, accumulate into output code for this pixel */
						Word pixcode = colorindex_ci[cur];
						output_ptr[0] += static_cast<JSample_t>(pixcode);
						/* Compute actual representation error at this pixel */
						/* Note: we can do this even though we don't have the final */
						/* pixel code, because the colormap is orthogonal. */
						cur -= colormap_ci[pixcode];
						/* Compute error fractions to be propagated to adjacent pixels.
						* Add these into the running sums, and simultaneously shift the
						* next-line error sums left by 1 column.
						*/
						SWord32 bnexterr = cur;
						SWord32 delta = cur * 2;
						cur += delta;		/* form error * 3 */
						errorptr[0] = (bpreverr + cur);
						cur += delta;		/* form error * 5 */
						bpreverr = belowerr + cur;
						belowerr = bnexterr;
						cur += delta;		/* form error * 7 */
						/* At this point cur contains the 7/16 error value to be propagated
						* to the next pixel on the current line, and all the errors for the
						* next line have been shifted over. We are therefore ready to move on.
						*/
						input_ptr += dirnc;	/* advance input ptr to next column */
						output_ptr += dir;	/* advance output ptr to next column */
						errorptr += dir;	/* advance errorptr to current column */
					} while (--col);
					/* Post-loop cleanup: we must unload the final error value into the
					* final m_FSErrors[] entry.  Note we need not unload belowerr because
					* it is for the dummy column before or after the actual array.
					*/
					errorptr[0] = bpreverr; /* unload prev err into array */
				}
				ThisPtr->m_OnOddRow ^= TRUE;
			} while (++row<num_rows);
		}
	}
}

/*******************************

	Allocate workspace for Floyd-Steinberg errors.

*******************************/

void CColorQuantizer1Pass::AllocFSWorkspace(void)
{
	Word arraysize = (m_Parent->output_width + 2);
	int nc = m_Parent->m_ColorComponents;
	for (int i = 0; i < nc; i++) {
		m_FSErrors[i] = new SWord32[arraysize];
	}
}

/*******************************

	Initialize for one-pass color quantization.

*******************************/

void BURGERCALL CColorQuantizer1Pass::StartPass(Word /* is_pre_scan */)
{
	/* Install my colormap. */
	m_Parent->colormap = m_SvColorMapPtr;
	m_Parent->m_ActualNumberOfColors = m_SvActual;

	/* Initialize for desired dithering mode. */
	switch (m_Parent->dither_mode) {
	case DITHER_NONE:
		if (m_Parent->m_ColorComponents == 3) {
			m_ColorQuantize = ColorQuantize3;
		} else {
			m_ColorQuantize = ColorQuantize;
		}
		break;
	case DITHER_ORDERED:
		if (m_Parent->m_ColorComponents == 3) {
			m_ColorQuantize = Quantize3OrdDither;
		} else {
			m_ColorQuantize = QuantizeOrdDither;
		}
		m_RowIndex = 0;		/* initialize state for ordered dither */
		/* If user changed to ordered dither from another mode,
		* we must recreate the color index table with padding.
		* This will cost extra space, but probably isn't very likely.
		*/
		if (!m_IsPadded) {
			CreateColorIndex();
		}
		/* Create ordered-dither tables if we didn't already. */
		if (!m_ODither[0]) {
			CreateODitherTables();
		}
		break;
	case DITHER_FS:
		m_ColorQuantize = QuantizeFSDither;
		m_OnOddRow = FALSE; /* initialize state for F-S dither */
		/* Allocate Floyd-Steinberg workspace if didn't already. */
		if (!m_FSErrors[0]) {
			AllocFSWorkspace();
		}
		/* Initialize the propagated errors to zero. */
		{
			Word TheArraySize = ((m_Parent->output_width + 2) * sizeof(SWord32));
			int nc = m_Parent->m_ColorComponents; 
			for (int i = 0; i < nc; i++) {
				MemZero(reinterpret_cast<Word8*>(m_FSErrors[i]),TheArraySize);
			}
		}
		break;
	default:
		m_Parent->FatalError(JERR_NOT_COMPILED);
		break;
	}
}

/*******************************

	Module initialization routine for 1-pass color quantization.
	
*******************************/

CColorQuantizer1Pass::CColorQuantizer1Pass(CJPegDecompress *CInfoPtr)
	: CColorQuantizer(CInfoPtr),
	m_SvColorMapPtr(0),
	m_ColorIndex(0)
{
	MemZero(reinterpret_cast<Word8*>(m_FSErrors),sizeof(m_FSErrors));	/* Flag FS workspace not allocated */
	MemZero(reinterpret_cast<Word8*>(m_ODither),sizeof(m_ODither));	/* Also flag odither arrays not allocated */

	/* Make sure my internal arrays won't overflow */
	if (CInfoPtr->m_ColorComponents > MAX_Q_COMPS) {
		CInfoPtr->FatalError(JERR_QUANT_COMPONENTS,MAX_Q_COMPS);
	}
	/* Make sure colormap indexes can be represented by JSAMPLEs */
	if (CInfoPtr->desired_number_of_colors > (MAXJSAMPLE+1)) {
		CInfoPtr->FatalError(JERR_QUANT_MANY_COLORS, MAXJSAMPLE+1);
	}
	/* Create the colormap and color index table. */
	CreateColorMap();
	CreateColorIndex();

	/* Allocate Floyd-Steinberg workspace now if requested.
	* We do this now since it is storage and may affect the memory
	* manager's space calculations.  If the user changes to FS dither
	* mode in a later pass, we will allocate the space then, and will
	* possibly overrun the max_memory_to_use setting.
	*/
	if (CInfoPtr->dither_mode == DITHER_FS) {
		AllocFSWorkspace();
	}
}

/*******************************

	Dispose of all the data found
	
*******************************/

CColorQuantizer1Pass::~CColorQuantizer1Pass()
{
	if (m_SvColorMapPtr) {		/* Release the saved color map */
		delete [] m_SvColorMapPtr;
	}
	if (m_ColorIndex) {			/* Release the color indexs */
		delete [] m_ColorIndex;
	}
	Word i=0;
	do {
		if (m_ODither[i]) {		/* Release the dither table */
			delete [] m_ODither[i];
		}
		if (m_FSErrors[i]) {	/* Release the Floyd-Steinberg error tables */
			delete [] m_FSErrors[i];
		}
	} while (++i<MAX_Q_COMPS);
}

}