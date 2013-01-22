/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

	This file contains 2-pass color quantization (color mapping) routines.
	These routines provide selection of a custom color map for an image,
	followed by mapping of the image to that color map, with optional
	Floyd-Steinberg dithering.
	It is also possible to use just the second pass to map to an arbitrary
	externally-given color map.

	Note: ordered dithering is not supported, since there isn't any fast
	way to compute intercolor distances; it's unclear that ordered dither's
	fundamental assumptions even hold with an irregularly spaced color map.

*******************************/

#include "JQuant2.hpp"
#include "jpeglib.h"
#include <Burger.h>

namespace JPeg70 {

/*******************************

	This module implements the well-known Heckbert paradigm for color
	quantization.  Most of the ideas used here can be traced back to
	Heckbert's seminal paper
	Heckbert, Paul.  "Color Image Quantization for Frame Buffer Display",
	Proc. SIGGRAPH '82, Computer Graphics v.16 #3 (July 1982), pp 297-304.

	In the first pass over the image, we accumulate a histogram showing the
	usage count of each possible color.  To keep the histogram to a reasonable
	size, we reduce the precision of the input; typical practice is to retain
	5 or 6 bits per color, so that 8 or 4 different input values are counted
	in the same histogram cell.

	Next, the color-selection step begins with a box representing the whole
	color space, and repeatedly splits the "largest" remaining box until we
	have as many boxes as desired colors.  Then the mean color in each
	remaining box becomes one of the possible output colors.

	The second pass over the image maps each input pixel to the closest output
	color (optionally after applying a Floyd-Steinberg dithering correction).
	This mapping is logically trivial, but making it go fast enough requires
	considerable care.

	Heckbert-style quantizers vary a good deal in their policies for choosing
	the "largest" box and deciding where to cut it.  The particular policies
	used here have proved out well in experimental comparisons, but better ones
	may yet be found.

	In earlier versions of the IJG code, this module quantized in YCbCr color
	space, processing the raw upsampled data without a color conversion step.
	This allowed the color conversion math to be done only once per colormap
	entry, not once per pixel.  However, that optimization precluded other
	useful optimizations (such as merging color conversion with upsampling)
	and it also interfered with desired capabilities such as quantizing to an
	externally-supplied colormap.  We have therefore abandoned that approach.
	The present code works in the post-conversion color space, typically RGB.

	To improve the visual quality of the results, we actually work in scaled
	RGB space, giving G distances more weight than R, and R in turn more than
	B.  To do everything in integer math, we must use integer scale factors.
	The 2/3/1 scale factors used here correspond loosely to the relative
	weights of the colors in the NTSC grayscale equation.
	If you want to use this code to quantize a non-RGB color space, you'll
	probably need to change these scale factors.

*******************************/

/*******************************

	First we have the histogram data structure and routines for creating it.

	The number of bits of precision can be adjusted by changing these symbols.
	We recommend keeping 6 bits for G and 5 each for R and B.
	If you have plenty of memory and cycles, 6 bits all around gives marginally
	better results; if you are short of memory, 5 bits all around will save
	some space but degrade the results.
	To maintain a fully accurate histogram, we'd need to allocate a "long"
	(preferably unsigned long) for each cell.  In practice this is overkill;
	we can get by with 16 bits per cell.  Few of the cell counts will overflow,
	and clamping those that do overflow to the maximum value will give close-
	enough results.  This reduces the recommended histogram size from 256Kb
	to 128Kb, which is a useful savings on PC-class machines.
	(In the second pass the histogram space is re-used for pixel mapping data;
	in that capacity, each cell must be able to store zero to the number of
	desired colors.  16 bits/cell is plenty for that too.)
	Since the JPEG code is intended to run in small memory model on 80x86
	machines, we can't just allocate the histogram in one chunk.  Instead
	of a true 3-D array, we use a row of pointers to 2-D arrays.  Each
	pointer corresponds to a C0 value (typically 2^5 = 32 pointers) and
	each 2-D array has 2^6*2^5 = 2048 or 2^6*2^6 = 4096 entries.  Note that
	on 80x86 machines, the pointer row is in near memory but the actual
	arrays are in far memory (same arrangement as we use for image arrays).

*******************************/

/*******************************

	Declarations for Floyd-Steinberg dithering.

	Errors are accumulated into the array fserrors[], at a resolution of
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

	The fserrors[] array has (#columns + 2) entries; the extra entry at
	each end saves us from special-casing the first and last pixels.
	Each entry is three values long, one value for each color component.

	Note: on a wide image, we might not have enough room in a PC's near data
	segment to hold the error array; so it is allocated with alloc_large.

*******************************/

/*******************************

	Prescan some rows of pixels.
	In this module the prescan simply updates the histogram, which has been
	initialized to zeroes by start_pass.
	An output_buf parameter is required by the method signature, but no data
	is actually output (in fact the buffer controller is probably passing a
	NULL pointer).

*******************************/

void BURGERCALL CColorQuantizer2Pass::PrescanQuantize(CColorQuantizer *ThisPtr2,JSample_t **input_buf,JSample_t ** /* output_buf */, int num_rows)
{
	if (num_rows>0) {
		CColorQuantizer2Pass *ThisPtr = reinterpret_cast<CColorQuantizer2Pass*>(ThisPtr2);
		Word width = ThisPtr->m_Parent->output_width;
		if (width) {
			int row=0;
			do {
				JSample_t*ptr = input_buf[row];
				Word col = width;
				do {
					/* get pixel value and index into the histogram */
					HistCell_t *histp = &ThisPtr->m_Histogram[ptr[0]>>C0_SHIFT][ptr[1]>>C1_SHIFT][ptr[2]>>C2_SHIFT];
					/* increment, check for overflow and undo increment if so. */
					if (++(histp[0]) <= 0) {
						(histp[0])--;
					}
					ptr += 3;
				} while (--col);
			} while (++row<num_rows);
		}
	}
}

/*******************************

	Next we have the really interesting routines: selection of a colormap
	given the completed histogram.
	These routines work with a list of "boxes", each representing a rectangular
	subset of the input color space (to histogram precision).

	Find the splittable box with the largest color population
	Returns NULL if no splittable boxes remain

*******************************/

inline CColorQuantizer2Pass::Box_t *CColorQuantizer2Pass::FindBiggestColorPop(Box_t *boxlist,Word numboxes)
{
	Box_t *which = 0;
	if (numboxes) {
		SWord32 maxc = 0;
		do {
			if (boxlist->colorcount > maxc && boxlist->volume > 0) {
				which = boxlist;
				maxc = boxlist->colorcount;
			}
			++boxlist;
		} while (--numboxes);
	}
	return which;
}

/*******************************

	Find the splittable box with the largest (scaled) volume
	Returns NULL if no splittable boxes remain

*******************************/

inline CColorQuantizer2Pass::Box_t * CColorQuantizer2Pass::FindBiggestVolume(Box_t *boxlist,Word numboxes)
{
	Box_t * which = 0;
	if (numboxes) {
		SWord32 maxv = 0;	
		do {
			if (boxlist->volume > maxv) {
				which = boxlist;
				maxv = boxlist->volume;
			}
			++boxlist;
		} while (--numboxes);
	}
	return which;
}

/*******************************

	Shrink the min/max bounds of a box to enclose only nonzero elements,
	and recompute its volume and population

*******************************/

void CColorQuantizer2Pass::UpdateBox(Box_t * boxp)
{
	int c0min = boxp->c0min;
	int c0max = boxp->c0max;
	int c1min = boxp->c1min;
	int c1max = boxp->c1max;
	int c2min = boxp->c2min;
	int c2max = boxp->c2max;

	int c0,c1,c2;
	if (c0max > c0min) {
		c0 = c0min;
		do {
			for (c1 = c1min; c1 <= c1max; c1++) {
				HistCell_t *histp1 = &m_Histogram[c0][c1][c2min];
				for (c2 = c2min; c2 <= c2max; c2++) {
					if (histp1[0]) {
						boxp->c0min = c0min = c0;
						goto have_c0min;
					}
					++histp1;
				}
			}
		} while (++c0<=c0max);
	}

have_c0min:
	if (c0max > c0min) {
		c0 = c0max;
		do {
			for (c1 = c1min; c1 <= c1max; c1++) {
				HistCell_t *histp2 = &m_Histogram[c0][c1][c2min];
				for (c2 = c2min; c2 <= c2max; c2++) {
					if (histp2[0]) {
						boxp->c0max = c0max = c0;
						goto have_c0max;
					}
					++histp2;
				}
			}
		} while (--c0>=c0min);
	}

have_c0max:
	if (c1max > c1min) {
		c1 = c1min;
		do {
			for (c0 = c0min; c0 <= c0max; c0++) {
				HistCell_t *histp3 = &m_Histogram[c0][c1][c2min];
				for (c2 = c2min; c2 <= c2max; c2++) {
					if (histp3[0]) {
						boxp->c1min = c1min = c1;
						goto have_c1min;
					}
					++histp3;
				}
			}
		} while (++c1<=c1max);
	}
	
have_c1min:
	if (c1max > c1min) {
		c1 = c1max;
		do {
			for (c0 = c0min; c0 <= c0max; c0++) {
				HistCell_t *histp4 = &m_Histogram[c0][c1][c2min];
				for (c2 = c2min; c2 <= c2max; c2++) {
					if (histp4[0]) {
						boxp->c1max = c1max = c1;
						goto have_c1max;
					}
					++histp4;
				}
			}
		} while (--c1>=c1min);
	}
		
have_c1max:
	if (c2max > c2min) {
		c2 = c2min;
		do {
			for (c0 = c0min; c0 <= c0max; c0++) {
				HistCell_t *histp5 = &m_Histogram[c0][c1min][c2];
				for (c1 = c1min; c1 <= c1max; c1++) {
					if (histp5[0]) {
						boxp->c2min = c2min = c2;
						goto have_c2min;
					}
					histp5 += HIST_C2_ELEMS;
				}
			}
		} while (++c2<=c2max);
	}
			
have_c2min:
	if (c2max > c2min) {
		c2 = c2max;
		do {
			for (c0 = c0min; c0 <= c0max; c0++) {
				HistCell_t *histp6 = &m_Histogram[c0][c1min][c2];
				for (c1 = c1min; c1 <= c1max; c1++) {
					if (histp6[0]) {
						boxp->c2max = c2max = c2;
						goto have_c2max;
					}
					histp6 += HIST_C2_ELEMS;
				}
			}
		} while (--c2>=c2min);
	}

have_c2max:

	/* Update box volume.
	* We use 2-norm rather than real volume here; this biases the method
	* against making long narrow boxes, and it has the side benefit that
	* a box is splittable iff norm > 0.
	* Since the differences are expressed in histogram-cell units,
	* we have to shift back to JSAMPLE units to get consistent distances;
	* after which, we scale according to the selected distance scale factors.
	*/
	SWord32 dist0 = ((c0max - c0min) << C0_SHIFT) * C0_SCALE;
	SWord32 dist1 = ((c1max - c1min) << C1_SHIFT) * C1_SCALE;
	SWord32 dist2 = ((c2max - c2min) << C2_SHIFT) * C2_SCALE;
	boxp->volume = dist0*dist0 + dist1*dist1 + dist2*dist2;

	/* Now scan remaining volume of box and compute population */
	SWord32 ccount = 0;
	for (c0 = c0min; c0 <= c0max; c0++) {
		for (c1 = c1min; c1 <= c1max; c1++) {
			HistCell_t *histp7 = &m_Histogram[c0][c1][c2min];
			for (c2 = c2min; c2 <= c2max; c2++) {
				if (histp7[0]) {
					ccount++;
				}
				++histp7;
			}
		}
	}
	boxp->colorcount = ccount;
}

/*******************************

	Repeatedly select and split the largest box until we have enough boxes

*******************************/

Word CColorQuantizer2Pass::MedianCut(Box_t *boxlist,Word numboxes,Word desired_colors)
{
	int n,lb;
	int cmax;

	if (numboxes < desired_colors) {
		do {
			/* Select box to split.
			* Current algorithm: by population for first half, then by volume.
			*/
			Box_t *b1;
			if (numboxes*2 <= desired_colors) {
				b1 = FindBiggestColorPop(boxlist,numboxes);
			} else {
				b1 = FindBiggestVolume(boxlist,numboxes);
			}
			if (!b1) {		/* no splittable boxes left! */
				break;
			}
			Box_t *b2 = &boxlist[numboxes];	/* where new box will go */
			/* Copy the color bounds to the new box. */
			b2->c0max = b1->c0max;
			b2->c1max = b1->c1max;
			b2->c2max = b1->c2max;
			b2->c0min = b1->c0min;
			b2->c1min = b1->c1min;
			b2->c2min = b1->c2min;
			/* Choose which axis to split the box on.
			* Current algorithm: longest scaled axis.
			* See notes in update_box about scaling distances.
			*/
			int c0 = ((b1->c0max - b1->c0min) << C0_SHIFT) * C0_SCALE;
			int c1 = ((b1->c1max - b1->c1min) << C1_SHIFT) * C1_SCALE;
			int c2 = ((b1->c2max - b1->c2min) << C2_SHIFT) * C2_SCALE;
			/* We want to break any ties in favor of green, then red, blue last.
			* This code does the right thing for R,G,B or B,G,R color orders only.
			*/
			#if 1		/* RGB */
			cmax = c1;
			n = 1;
			if (c0 > cmax) {
				cmax = c0;
				n = 0;
			}
			if (c2 > cmax) {
				n = 2;
			}
			#else
			cmax = c1;
			n = 1;
			if (c2 > cmax) {
				cmax = c2;
				n = 2;
			}
			if (c0 > cmax) {
				n = 0;
			}
			#endif
			/* Choose split point along selected axis, and update box bounds.
			* Current algorithm: split at halfway point.
			* (Since the box has been shrunk to minimum volume,
			* any split will produce two nonempty subboxes.)
			* Note that lb value is max for lower box, so must be < old max.
			*/
			switch (n) {
			case 0:
				lb = (b1->c0max + b1->c0min) >> 1;
				b1->c0max = lb;
				b2->c0min = lb+1;
				break;
			case 1:
				lb = (b1->c1max + b1->c1min) >> 1;
				b1->c1max = lb;
				b2->c1min = lb+1;
				break;
			default:		/* case 2 */
				lb = (b1->c2max + b1->c2min) >> 1;
				b1->c2max = lb;
				b2->c2min = lb+1;
				break;
			}
			/* Update stats for boxes */
			UpdateBox(b1);
			UpdateBox(b2);
		} while (++numboxes<desired_colors);
	}
	return numboxes;
}

/*******************************

	Compute representative color for a box, put it in colormap[icolor]
	Current algorithm: mean weighted by pixels (not colors)
	Note it is important to get the rounding correct!

*******************************/

void CColorQuantizer2Pass::ComputeColor(Box_t * boxp,Word icolor)
{
	int c0,c1,c2;
	SWord32 total = 0;
	SWord32 c0total = 0;
	SWord32 c1total = 0;
	SWord32 c2total = 0;

	int c0min = boxp->c0min;
	int c0max = boxp->c0max;
	int c1min = boxp->c1min;
	int c1max = boxp->c1max;
	int c2min = boxp->c2min;
	int c2max = boxp->c2max;

	for (c0 = c0min; c0 <= c0max; c0++) {
		for (c1 = c1min; c1 <= c1max; c1++) {
			HistCell_t *histp = &m_Histogram[c0][c1][c2min];
			for (c2 = c2min; c2 <= c2max; c2++) {
				SWord32 count = histp[0];
				if (count) {
					total += count;
					c0total += ((c0 << C0_SHIFT) + ((1<<C0_SHIFT)>>1)) * count;
					c1total += ((c1 << C1_SHIFT) + ((1<<C1_SHIFT)>>1)) * count;
					c2total += ((c2 << C2_SHIFT) + ((1<<C2_SHIFT)>>1)) * count;
				}
				++histp;
			}
		}
	}
	m_Parent->colormap[0][icolor] = static_cast<JSample_t>((c0total + (total>>1)) / total);
	m_Parent->colormap[1][icolor] = static_cast<JSample_t>((c1total + (total>>1)) / total);
	m_Parent->colormap[2][icolor] = static_cast<JSample_t>((c2total + (total>>1)) / total);
}

/*******************************

	Master routine for color selection

*******************************/

void CColorQuantizer2Pass::SelectColors(Word desired_colors)
{
	Box_t * boxlist;
	Word numboxes;
	Word i;

	/* Allocate workspace for box list */
	boxlist = new Box_t[desired_colors];
	/* Initialize one box containing whole space */
	numboxes = 1;
	boxlist[0].c0min = 0;
	boxlist[0].c0max = MAXJSAMPLE >> C0_SHIFT;
	boxlist[0].c1min = 0;
	boxlist[0].c1max = MAXJSAMPLE >> C1_SHIFT;
	boxlist[0].c2min = 0;
	boxlist[0].c2max = MAXJSAMPLE >> C2_SHIFT;
	/* Shrink it to actually-used volume and set its statistics */
	UpdateBox(boxlist);
	/* Perform median-cut to produce final box list */
	numboxes = MedianCut(boxlist, numboxes, desired_colors);
	/* Compute the representative color for each box, fill colormap */
	for (i = 0; i < numboxes; i++) {
		ComputeColor(&boxlist[i],i);
	}
	m_Parent->m_ActualNumberOfColors = numboxes;
	m_Parent->EmitTrace(1,JTRC_QUANT_SELECTED,static_cast<int>(numboxes));
}

/*******************************

	These routines are concerned with the time-critical task of mapping input
	colors to the nearest color in the selected colormap.

	We re-use the histogram space as an "inverse color map", essentially a
	cache for the results of nearest-color searches.  All colors within a
	histogram cell will be mapped to the same colormap entry, namely the one
	closest to the cell's center.  This may not be quite the closest entry to
	the actual input color, but it's almost as good.  A zero in the cache
	indicates we haven't found the nearest color for that cell yet; the array
	is cleared to zeroes before starting the mapping pass.  When we find the
	nearest color for a cell, its colormap index plus one is recorded in the
	cache for future use.  The pass2 scanning routines call FillInverseCMap
	when they need to use an unfilled entry in the cache.

	Our method of efficiently finding nearest colors is based on the "locally
	sorted search" idea described by Heckbert and on the incremental distance
	calculation described by Spencer W. Thomas in chapter III.1 of Graphics
	Gems II (James Arvo, ed.  Academic Press, 1991).  Thomas points out that
	the distances from a given colormap entry to each cell of the histogram can
	be computed quickly using an incremental method: the differences between
	distances to adjacent cells themselves differ by a constant.  This allows a
	fairly fast implementation of the "brute force" approach of computing the
	distance from every colormap entry to every histogram cell.  Unfortunately,
	it needs a work array to hold the best-distance-so-far for each histogram
	cell (because the inner loop has to be over cells, not colormap entries).
	The work array elements have to be INT32s, so the work array would need
	256Kb at our recommended precision.  This is not feasible in DOS machines.

	To get around these problems, we apply Thomas' method to compute the
	nearest colors for only the cells within a small subbox of the histogram.
	The work array need be only as big as the subbox, so the memory usage
	problem is solved.  Furthermore, we need not fill subboxes that are never
	referenced in pass2; many images use only part of the color gamut, so a
	fair amount of work is saved.  An additional advantage of this
	approach is that we can apply Heckbert's locality criterion to quickly
	eliminate colormap entries that are far away from the subbox; typically
	three-fourths of the colormap entries are rejected by Heckbert's criterion,
	and we need not compute their distances to individual cells in the subbox.
	The speed of this approach is heavily influenced by the subbox size: too
	small means too much overhead, too big loses because Heckbert's criterion
	can't eliminate as many colormap entries.  Empirically the best subbox
	size seems to be about 1/512th of the histogram (1/8th in each direction).

	Thomas' article also describes a refined method which is asymptotically
	faster than the brute-force method, but it is also far more complex and
	cannot efficiently be applied to small subboxes.  It is therefore not
	useful for programs intended to be portable to DOS machines.  On machines
	with plenty of memory, filling the whole histogram in one shot with Thomas'
	refined method might be faster than the present code --- but then again,
	it might not be any faster, and it's certainly more complicated.

*******************************/

/*******************************

	The next three routines implement inverse colormap filling.  They could
	all be folded into one big routine, but splitting them up this way saves
	some stack space (the mindist[] and bestdist[] arrays need not coexist)
	and may allow some compilers to produce better code by registerizing more
	inner-loop variables.

	Locate the colormap entries close enough to an update box to be candidates
	for the nearest entry to some cell(s) in the update box.  The update box
	is specified by the center coordinates of its first cell.  The number of
	candidate colormap entries is returned, and their colormap indexes are
	placed in colorlist[].
	This routine uses Heckbert's "locally sorted search" criterion to select
	the colors that need further consideration.

*******************************/

int CColorQuantizer2Pass::FindNearbyColors(int minc0, int minc1, int minc2,JSample_t *colorlist)
{
	Word numcolors = m_Parent->m_ActualNumberOfColors;
	SWord32 mindist[MAXNUMCOLORS];	/* min distance to colormap entry i */

	/* Compute true coordinates of update box's upper corner and center.
	* Actually we compute the coordinates of the center of the upper-corner
	* histogram cell, which are the upper bounds of the volume we care about.
	* Note that since ">>" rounds down, the "center" values may be closer to
	* min than to max; hence comparisons to them must be "<=", not "<".
	*/
	int maxc0 = minc0 + ((1 << BOX_C0_SHIFT) - (1 << C0_SHIFT));
	int centerc0 = (minc0 + maxc0) >> 1;
	int maxc1 = minc1 + ((1 << BOX_C1_SHIFT) - (1 << C1_SHIFT));
	int centerc1 = (minc1 + maxc1) >> 1;
	int maxc2 = minc2 + ((1 << BOX_C2_SHIFT) - (1 << C2_SHIFT));
	int centerc2 = (minc2 + maxc2) >> 1;

	/* For each color in colormap, find:
	*  1. its minimum squared-distance to any point in the update box
	*     (zero if color is within update box);
	*  2. its maximum squared-distance to any point in the update box.
	* Both of these can be found by considering only the corners of the box.
	* We save the minimum distance for each color in mindist[];
	* only the smallest maximum distance is of interest.
	*/
	SWord32 minmaxdist = 0x7FFFFFFFL;

	for (Word i = 0; i < numcolors; i++) {
		/* We compute the squared-c0-distance term, then add in the other two. */
		int x = m_Parent->colormap[0][i];
		SWord32 min_dist, max_dist, tdist;
		if (x < minc0) {
			tdist = (x - minc0) * C0_SCALE;
			min_dist = tdist*tdist;
			tdist = (x - maxc0) * C0_SCALE;
			max_dist = tdist*tdist;
		} else if (x > maxc0) {
			tdist = (x - maxc0) * C0_SCALE;
			min_dist = tdist*tdist;
			tdist = (x - minc0) * C0_SCALE;
			max_dist = tdist*tdist;
		} else {
			/* within cell range so no contribution to min_dist */
			min_dist = 0;
			if (x <= centerc0) {
				tdist = (x - maxc0) * C0_SCALE;
				max_dist = tdist*tdist;
			} else {
				tdist = (x - minc0) * C0_SCALE;
				max_dist = tdist*tdist;
			}
		}

		x = m_Parent->colormap[1][i];
		if (x < minc1) {
			tdist = (x - minc1) * C1_SCALE;
			min_dist += tdist*tdist;
			tdist = (x - maxc1) * C1_SCALE;
			max_dist += tdist*tdist;
		} else if (x > maxc1) {
			tdist = (x - maxc1) * C1_SCALE;
			min_dist += tdist*tdist;
			tdist = (x - minc1) * C1_SCALE;
			max_dist += tdist*tdist;
		} else {
			/* within cell range so no contribution to min_dist */
			if (x <= centerc1) {
				tdist = (x - maxc1) * C1_SCALE;
				max_dist += tdist*tdist;
			} else {
				tdist = (x - minc1) * C1_SCALE;
				max_dist += tdist*tdist;
			}
		}

		x = m_Parent->colormap[2][i];
		if (x < minc2) {
			tdist = (x - minc2) * C2_SCALE;
			min_dist += tdist*tdist;
			tdist = (x - maxc2) * C2_SCALE;
			max_dist += tdist*tdist;
		} else if (x > maxc2) {
			tdist = (x - maxc2) * C2_SCALE;
			min_dist += tdist*tdist;
			tdist = (x - minc2) * C2_SCALE;
			max_dist += tdist*tdist;
		} else {
			/* within cell range so no contribution to min_dist */
			if (x <= centerc2) {
				tdist = (x - maxc2) * C2_SCALE;
				max_dist += tdist*tdist;
			} else {
				tdist = (x - minc2) * C2_SCALE;
				max_dist += tdist*tdist;
			}
		}

		mindist[i] = min_dist;	/* save away the results */
		if (max_dist < minmaxdist) {
			minmaxdist = max_dist;
		}
	}

	/* Now we know that no cell in the update box is more than minmaxdist
	* away from some colormap entry.  Therefore, only colors that are
	* within minmaxdist of some part of the box need be considered.
	*/
	int ncolors = 0;
	for (Word ti = 0; ti < numcolors; ti++) {
		if (mindist[ti] <= minmaxdist) {
			colorlist[ncolors] = static_cast<JSample_t>(ti);
			++ncolors;
		}
	}
	return ncolors;
}

/*******************************

	Find the closest colormap entry for each cell in the update box,
	given the list of candidate colors prepared by find_nearby_colors.
	Return the indexes of the closest entries in the bestcolor[] array.
	This routine uses Thomas' incremental distance calculation method to
	find the distance from a colormap entry to successive cells in the box.

*******************************/

void CColorQuantizer2Pass::FindBestColors(int minc0,int minc1,int minc2,int numcolors, JSample_t *colorlist,JSample_t *bestcolor)
{
	if (numcolors>0) {
		/* This array holds the distance to the nearest-so-far color for each cell */
		SWord32 bestdist[BOX_C0_ELEMS * BOX_C1_ELEMS * BOX_C2_ELEMS];

		/* Initialize best-distance for each cell of the update box */
		SWord32 *bptr = bestdist;
		Word kk = BOX_C0_ELEMS*BOX_C1_ELEMS*BOX_C2_ELEMS;
		do {
			bptr[0] = 0x7FFFFFFFL;
			++bptr;
		} while (--kk);

		/* For each color selected by find_nearby_colors,
		* compute its distance to the center of each cell in the box.
		* If that's less than best-so-far, update best distance and color number.
		*/

		/* Nominal steps between cell centers ("x" in Thomas article) */

		do {
			Word IColor = colorlist[0];
			++colorlist;
			/* Compute (square of) distance from minc0/c1/c2 to this color */
			SWord32 inc0 = (minc0 - m_Parent->colormap[0][IColor]) * C0_SCALE;
			SWord32 dist0 = inc0*inc0;
			SWord32 inc1 = (minc1 - m_Parent->colormap[1][IColor]) * C1_SCALE;
			dist0 += inc1*inc1;
			SWord32 inc2 = (minc2 - m_Parent->colormap[2][IColor]) * C2_SCALE;
			dist0 += inc2*inc2;
			/* Form the initial difference increments */
			inc0 = (inc0 * (2 * STEP_C0)) + (STEP_C0 * STEP_C0);
			inc1 = (inc1 * (2 * STEP_C1)) + (STEP_C1 * STEP_C1);
			inc2 = (inc2 * (2 * STEP_C2)) + (STEP_C2 * STEP_C2);
			/* Now loop over all cells in box, updating distance per Thomas method */
			bptr = bestdist;
			JSample_t *cptr = bestcolor;
			Word ic0 = BOX_C0_ELEMS;
			do {
				SWord32 dist1 = dist0;
				SWord32 xx1 = inc1;
				Word ic1 = BOX_C1_ELEMS;
				do {
					SWord32 dist2 = dist1;
					SWord32 xx2 = inc2;
					Word ic2 = BOX_C2_ELEMS;
					do {
						if (dist2 < bptr[0]) {
							bptr[0] = dist2;
							cptr[0] = static_cast<JSample_t>(IColor);
						}
						dist2 += xx2;
						xx2 += (2 * STEP_C2 * STEP_C2);
						++bptr;
						++cptr;
					} while (--ic2);
					dist1 += xx1;
					xx1 += (2 * STEP_C1 * STEP_C1);
				} while (--ic1);
				dist0 += inc0;
				inc0 += (2 * STEP_C0 * STEP_C0);
			} while (--ic0);
		} while (--numcolors);
	}
}

/*******************************

	Fill the inverse-colormap entries in the update box that contains 
	histogram cell c0/c1/c2.  (Only that one cell MUST be filled, but 
	we can fill as many others as we wish.) 

*******************************/

void CColorQuantizer2Pass::FillInverseCMap(int c0,int c1,int c2)
{
	/* This array lists the candidate colormap indexes. */
	JSample_t colorlist[MAXNUMCOLORS];
	/* This array holds the actually closest colormap index for each cell. */
	JSample_t bestcolor[BOX_C0_ELEMS * BOX_C1_ELEMS * BOX_C2_ELEMS];

	/* Convert cell coordinates to update box ID */
	c0 >>= BOX_C0_LOG;
	c1 >>= BOX_C1_LOG;
	c2 >>= BOX_C2_LOG;

	/* Compute true coordinates of update box's origin corner.
	* Actually we compute the coordinates of the center of the corner
	* histogram cell, which are the lower bounds of the volume we care about.
	*/
	int minc0 = (c0 << BOX_C0_SHIFT) + ((1 << C0_SHIFT) >> 1);
	int minc1 = (c1 << BOX_C1_SHIFT) + ((1 << C1_SHIFT) >> 1);
	int minc2 = (c2 << BOX_C2_SHIFT) + ((1 << C2_SHIFT) >> 1);

	/* Determine which colormap entries are close enough to be candidates
	* for the nearest entry to some cell in the update box.
	*/
	int numcolors = FindNearbyColors(minc0, minc1, minc2, colorlist);

	/* Determine the actually nearest colors. */
	FindBestColors(minc0, minc1, minc2, numcolors, colorlist,bestcolor);

	/* Save the best color numbers (plus 1) in the main cache array */
	c0 <<= BOX_C0_LOG;		/* convert ID back to base cell indexes */
	c1 <<= BOX_C1_LOG;
	c2 <<= BOX_C2_LOG;
	JSample_t *cptr = bestcolor;
	Word ic0 = BOX_C0_ELEMS;
	do {
		Word ic1 = 0;
		do {
			HistCell_t *cachep = &m_Histogram[c0][c1+ic1][c2];
			Word ic2 = BOX_C2_ELEMS;
			do {
				cachep[0] = static_cast<HistCell_t>(cptr[0] + 1);
				++cptr;
				++cachep;
			} while (--ic2);
		} while (++ic1<BOX_C1_ELEMS);
		++c0;
	} while (--ic0);
}

/*******************************

	Map some rows of pixels to the output colormapped representation.
	This version performs no dithering
	
*******************************/

void BURGERCALL CColorQuantizer2Pass::Pass2NoDither(CColorQuantizer *ThisPtr2,JSample_t ** input_buf,JSample_t **output_buf, int num_rows)
{
	if (num_rows>0) {
		CColorQuantizer2Pass* ThisPtr = reinterpret_cast<CColorQuantizer2Pass*>(ThisPtr2);
		Word width = ThisPtr->m_Parent->output_width;
		if (width) {
			int row = 0;
			do {
				JSample_t *inptr = input_buf[row];
				JSample_t *outptr = output_buf[row];
				Word col = width;
				do {
					/* get pixel value and index into the cache */
					int c0 = inptr[0] >> C0_SHIFT;
					int c1 = inptr[1] >> C1_SHIFT;
					int c2 = inptr[2] >> C2_SHIFT;
					inptr+=3;
					HistCell_t *cachep = &ThisPtr->m_Histogram[c0][c1][c2];
					/* If we have not seen this color before, find nearest colormap entry */
					/* and update the cache */
					if (!cachep[0]) {
						ThisPtr->FillInverseCMap(c0,c1,c2);
					}
					/* Now emit the colormap index for this cell */
					outptr[0] = static_cast<JSample_t>(cachep[0] - 1);
					++outptr;
				} while (--col);
			} while (++row<num_rows);
		}
	}
}

/*******************************

	This version performs Floyd-Steinberg dithering
	
*******************************/

void BURGERCALL CColorQuantizer2Pass::Pass2FSDither(CColorQuantizer *ThisPtr2,JSample_t **input_buf, JSample_t ** output_buf, int num_rows)
{
	if (num_rows>0) {
		CColorQuantizer2Pass* ThisPtr = reinterpret_cast<CColorQuantizer2Pass*>(ThisPtr2);
		Word width = ThisPtr->m_Parent->output_width;
		if (width) {
			JSample_t *range_limit = ThisPtr->m_Parent->sample_range_limit;
			int *error_limit = ThisPtr->m_ErrorLimiter;
			JSample_t * colormap0 = ThisPtr->m_Parent->colormap[0];
			JSample_t * colormap1 = ThisPtr->m_Parent->colormap[1];
			JSample_t * colormap2 = ThisPtr->m_Parent->colormap[2];

			int row = 0;
			do {
				JSample_t *inptr = input_buf[row];
				JSample_t *outptr = output_buf[row];
				int dir;			/* +1 or -1 depending on direction */
				int dir3;			/* 3*dir, for advancing inptr & errorptr */
				SWord32 * errorptr;	/* => fserrors[] at column before current */
				if (ThisPtr->m_OnOddRow) {
					/* work right to left in this row */
					inptr += (width-1) * 3;	/* so point to rightmost pixel */
					outptr += width-1;
					dir = -1;
					dir3 = -3;
					errorptr = ThisPtr->m_FSErrors + (width+1)*3; /* => entry after last column */
					ThisPtr->m_OnOddRow = FALSE; /* flip for next time */
				} else {
					/* work left to right in this row */
					dir = 1;
					dir3 = 3;
					errorptr = ThisPtr->m_FSErrors; /* => entry before first real column */
					ThisPtr->m_OnOddRow = TRUE; /* flip for next time */
				}
				/* Preset error values: no error propagated to first pixel from left */
				SWord32 cur0 = 0;
				SWord32 cur1 = 0;
				SWord32 cur2 = 0;
				/* and no error propagated to row below yet */
				SWord32 belowerr0 = 0;
				SWord32 belowerr1 = 0;
				SWord32 belowerr2 = 0;
				SWord32 bpreverr0 = 0;
				SWord32 bpreverr1 = 0;
				SWord32 bpreverr2 = 0;

				Word col = width;
				do {
					/* curN holds the error propagated from the previous pixel on the
					* current line.  Add the error propagated from the previous line
					* to form the complete error correction term for this pixel, and
					* round the error term (which is expressed * 16) to an integer.
					* RIGHT_SHIFT rounds towards minus infinity, so adding 8 is correct
					* for either sign of the error value.
					* Note: errorptr points to *previous* column's array entry.
					*/
					cur0 = ((cur0 + errorptr[dir3+0] + 8)>> 4);
					cur1 = ((cur1 + errorptr[dir3+1] + 8)>> 4);
					cur2 = ((cur2 + errorptr[dir3+2] + 8)>> 4);
					/* Limit the error using transfer function set by InitErrorLimit().
					* See comments with InitErrorLimit() for rationale.
					*/
					cur0 = error_limit[cur0];
					cur1 = error_limit[cur1];
					cur2 = error_limit[cur2];
					/* Form pixel value + error, and range-limit to 0..MAXJSAMPLE.
					* The maximum error is +- MAXJSAMPLE (or less with error limiting);
					* this sets the required size of the range_limit array.
					*/
					cur0 += inptr[0];
					cur1 += inptr[1];
					cur2 += inptr[2];
					cur0 = range_limit[cur0];
					cur1 = range_limit[cur1];
					cur2 = range_limit[cur2];
					/* Index into the cache with adjusted pixel value */
					HistCell_t *cachep = &ThisPtr->m_Histogram[cur0>>C0_SHIFT][cur1>>C1_SHIFT][cur2>>C2_SHIFT];
					/* If we have not seen this color before, find nearest colormap */
					/* entry and update the cache */
					if (!cachep[0]) {
						ThisPtr->FillInverseCMap(cur0>>C0_SHIFT,cur1>>C1_SHIFT,cur2>>C2_SHIFT);
					}
					/* Now emit the colormap index for this cell */
					{
						Word pixcode = cachep[0] - 1U;
						outptr[0] = static_cast<JSample_t>(pixcode);
						/* Compute representation error for this pixel */
						cur0 -= colormap0[pixcode];
						cur1 -= colormap1[pixcode];
						cur2 -= colormap2[pixcode];
					}
					/* Compute error fractions to be propagated to adjacent pixels.
					* Add these into the running sums, and simultaneously shift the
					* next-line error sums left by 1 column.
					*/
					{
						SWord32 bnexterr = cur0;	/* Process component 0 */
						SWord32 delta = cur0 * 2;
						cur0 += delta;		/* form error * 3 */
						errorptr[0] = (bpreverr0 + cur0);
						cur0 += delta;		/* form error * 5 */
						bpreverr0 = belowerr0 + cur0;
						belowerr0 = bnexterr;
						cur0 += delta;		/* form error * 7 */
						bnexterr = cur1;	/* Process component 1 */
						delta = cur1 * 2;
						cur1 += delta;		/* form error * 3 */
						errorptr[1] = (bpreverr1 + cur1);
						cur1 += delta;		/* form error * 5 */
						bpreverr1 = belowerr1 + cur1;
						belowerr1 = bnexterr;
						cur1 += delta;		/* form error * 7 */
						bnexterr = cur2;	/* Process component 2 */
						delta = cur2 * 2;
						cur2 += delta;		/* form error * 3 */
						errorptr[2] = (bpreverr2 + cur2);
						cur2 += delta;		/* form error * 5 */
						bpreverr2 = belowerr2 + cur2;
						belowerr2 = bnexterr;
						cur2 += delta;		/* form error * 7 */
					}
					/* At this point curN contains the 7/16 error value to be propagated
					* to the next pixel on the current line, and all the errors for the
					* next line have been shifted over.  We are therefore ready to move on.
					*/
					inptr += dir3;		/* Advance pixel pointers to next column */
					outptr += dir;
					errorptr += dir3;		/* advance errorptr to current column */
				} while (--col);
				/* Post-loop cleanup: we must unload the final error values into the
				* final fserrors[] entry.  Note we need not unload belowerrN because
				* it is for the dummy column before or after the actual array.
				*/
				errorptr[0] = bpreverr0; /* unload prev errs into array */
				errorptr[1] = bpreverr1;
				errorptr[2] = bpreverr2;
			} while (++row<num_rows);
		}
	}
}

/*******************************

	Initialize the error-limiting transfer function (lookup table).
	The raw F-S error computation can potentially compute error values of up to
	+- MAXJSAMPLE.  But we want the maximum correction applied to a pixel to be
	much less, otherwise obviously wrong pixels will be created.  (Typical
	effects include weird fringes at color-area boundaries, isolated bright
	pixels in a dark area, etc.)  The standard advice for avoiding this problem
	is to ensure that the "corners" of the color cube are allocated as output
	colors; then repeated errors in the same direction cannot cause cascading
	error buildup.  However, that only prevents the error from getting
	completely out of hand; Aaron Giles reports that error limiting improves
	the results even with corner colors allocated.
	A simple clamping of the error values to about +- MAXJSAMPLE/8 works pretty
	well, but the smoother transfer function used below is even better.  Thanks
	to Aaron Giles for this idea.

	Allocate and fill in the error_limiter table
	
*******************************/

void CColorQuantizer2Pass::InitErrorLimit(void)
{
	int out;
	int out2;

	int *table = new int[((MAXJSAMPLE*2)+1)];
	table += MAXJSAMPLE;		/* so can index -MAXJSAMPLE .. +MAXJSAMPLE */
	m_ErrorLimiter = table;

#define STEPSIZE ((MAXJSAMPLE+1)/16)
	/* Map errors 1:1 up to +- MAXJSAMPLE/16 */
	out = 0;
	out2 = 0;
	int in = 0;
	int*table2 = table;
	do {
		table[0] = out;
		table2[0] = out2;
		++table;
		--table2;
		++out;
		--out2;
	} while (++in<STEPSIZE);
	/* Map errors 1:2 up to +- 3*MAXJSAMPLE/16 */
	do {
		table[0] = out;
		table2[0] = -out;
		++table;
		--table2;
		out += (in&1);
	} while (++in<(STEPSIZE*3));
	/* Clamp the rest to final out value (which is (MAXJSAMPLE+1)/8) */
	out2 = -out;
	do {
		table[0] = out;
		table2[0] = out2;
		++table;
		--table2;
	} while (++in<=MAXJSAMPLE);
#undef STEPSIZE
}

/*******************************

	Initialize for each processing pass.
	
*******************************/

void BURGERCALL CColorQuantizer2Pass::StartPass(Word is_pre_scan)
{
	CJPegDecompress *cinfo = m_Parent;

	/* Only F-S dithering or no dithering is supported. */
	/* If user asks for ordered dither, give him F-S. */
	if (cinfo->dither_mode != DITHER_NONE) {
		cinfo->dither_mode = DITHER_FS;
	}
	if (is_pre_scan) {
		/* Set up method pointers */
		m_ColorQuantize = PrescanQuantize;
		m_DoPass1 = TRUE;
		m_NeedsZeroed = TRUE; /* Always zero histogram */
	} else {
		/* Set up method pointers */
		if (cinfo->dither_mode == DITHER_FS) {
			m_ColorQuantize = Pass2FSDither;
		} else {
			m_ColorQuantize = Pass2NoDither;
		}
		m_DoPass1 = FALSE;

		/* Make sure color count is acceptable */
		Word nc = cinfo->m_ActualNumberOfColors;
		if (nc < 1) {
			cinfo->FatalError(JERR_QUANT_FEW_COLORS, 1);
		}
		if (nc > MAXNUMCOLORS) {
			cinfo->FatalError(JERR_QUANT_MANY_COLORS, MAXNUMCOLORS);
		}
		if (cinfo->dither_mode == DITHER_FS) {
			Word arraysize = ((cinfo->output_width + 2) * 3);
		/* Allocate Floyd-Steinberg workspace if we didn't already. */
			if (!m_FSErrors) {
				m_FSErrors = new SWord32[arraysize];
			}
		/* Initialize the propagated errors to zero. */
			MemZero(reinterpret_cast<Word8*>(m_FSErrors),arraysize*sizeof(SWord32));
			/* Make the error-limit table if we didn't already. */
			if (!m_ErrorLimiter) {
				InitErrorLimit();
			}
			m_OnOddRow = FALSE;
		}
	}
	/* Zero the histogram or inverse color map, if necessary */
	if (m_NeedsZeroed) {
		MemZero(reinterpret_cast<Word8*>(m_Histogram),sizeof(m_Histogram));
		m_NeedsZeroed = FALSE;
	}
}

/*******************************

	Finish up at the end of each pass.
	
*******************************/

void BURGERCALL CColorQuantizer2Pass::FinishPass(void)
{
	if (m_DoPass1) {
		/* Select the representative colors and fill in cinfo->colormap */
		m_Parent->colormap = m_SVColorMapPtr;
		SelectColors(m_Desired);
		/* Force next pass to zero the color index table */
		m_NeedsZeroed = TRUE;
	}
}

/*******************************

	Switch to a new external colormap between output passes.
	
*******************************/

void BURGERCALL CColorQuantizer2Pass::NewColorMap(void)
{
	/* Reset the inverse color map */
	m_NeedsZeroed = TRUE;
}

/*******************************

	Module initialization routine for 2-pass color quantization.
	
*******************************/

CColorQuantizer2Pass::CColorQuantizer2Pass(CJPegDecompress *CInfoPtr)
	: CColorQuantizer(CInfoPtr),
	m_SVColorMapPtr(0),
	m_FSErrors(0),
	m_ErrorLimiter(0),
	m_DoPass1(FALSE)
{
	/* Make sure jdmaster didn't give me a case I can't handle */
	if (CInfoPtr->m_ColorComponents != 3) {
		CInfoPtr->FatalError(JERR_NOTIMPL);
	}
	/* Allocate the histogram/inverse colormap storage */
	m_NeedsZeroed = TRUE; /* histogram is garbage now */

	/* Allocate storage for the completed colormap, if required.
	* We do this now since it is storage and may affect
	* the memory manager's space calculations.
	*/
	if (CInfoPtr->enable_2pass_quant) {
		/* Make sure color count is acceptable */
		Word desired = static_cast<Word>(CInfoPtr->desired_number_of_colors);
		/* Lower bound on # of colors ... somewhat arbitrary as long as > 0 */
		if (desired < 8) {
			CInfoPtr->FatalError(JERR_QUANT_FEW_COLORS, 8);
		}
		/* Make sure colormap indexes can be represented by JSAMPLEs */
		if (desired > MAXNUMCOLORS) {
			CInfoPtr->FatalError(JERR_QUANT_MANY_COLORS, MAXNUMCOLORS);
		}
		m_SVColorMapPtr = CInfoPtr->AllocSArray((Word)desired,3);
		m_Desired = desired;
	}
	/* Only F-S dithering or no dithering is supported. */
	/* If user asks for ordered dither, give him F-S. */
	if (CInfoPtr->dither_mode != DITHER_NONE)
		CInfoPtr->dither_mode = DITHER_FS;

	/* Allocate Floyd-Steinberg workspace if necessary.
	* This isn't really needed until pass 2, but again it is storage.
	* Although we will cope with a later change in dither_mode,
	* we do not promise to honor max_memory_to_use if dither_mode changes.
	*/
	if (CInfoPtr->dither_mode == DITHER_FS) {
		m_FSErrors = new SWord32[(CInfoPtr->output_width + 2) * 3];
		/* Might as well create the error-limiting table too. */
		InitErrorLimit();
	}
}

/*******************************

	Shutdown routine for 2-pass color quantization.
	
*******************************/

CColorQuantizer2Pass::~CColorQuantizer2Pass()
{
	if (m_SVColorMapPtr) {
		delete [] m_SVColorMapPtr;
	}
	if (m_FSErrors) {
		delete [] m_FSErrors;
	}
	if (m_ErrorLimiter) {
		delete [] (m_ErrorLimiter-MAXJSAMPLE);
	}
}

}
