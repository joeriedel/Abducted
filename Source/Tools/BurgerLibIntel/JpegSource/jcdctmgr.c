/*
 * jcdctmgr.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the forward-DCT management logic.
 * This code selects a particular DCT implementation to be used,
 * and it performs related housekeeping chores including coefficient
 * quantization.
 */

#include "JFDCTFlt.hpp"
#include "JFDCTFst.hpp"
#include "JFDCTSlw.hpp"

#define JPEG_INTERNALS
#include "jpeglib.h"
#include "jdct.h"		/* Private declarations for DCT subsystem */


/* Private subobject for this module */

typedef struct {
	struct jpeg_forward_dct pub;	/* public fields */

	/* Pointer to the DCT routine actually in use */
	forward_DCT_method_ptr do_dct;

	/* The actual post-DCT divisors --- not identical to the quant table
	* entries, because of scaling (especially for an unnormalized DCT).
	* Each table is given in normal array order.
	*/
	SWord32 * divisors[NUM_QUANT_TBLS];

	/* Same as above for the floating-point case. */
	float * float_divisors[NUM_QUANT_TBLS];
} my_fdct_controller;

typedef my_fdct_controller * my_fdct_ptr;


/*
 * Initialize for a processing pass.
 * Verify that all referenced Q-tables are present, and set up
 * the divisor table for each one.
 * In the current implementation, DCT of all components is done during
 * the first pass, even if only some components will be output in the
 * first scan.  Hence all components should be examined here.
 */

static void BURGERCALL start_pass_fdctmgr (CJPegCompress * cinfo)
{
  my_fdct_ptr fdct = (my_fdct_ptr) cinfo->fdct;
  int ci, qtblno, i;
  JPeg70::ComponentInfo_t *compptr;
  JPeg70::QuantTable_t * qtbl;
  SWord32 * dtbl;

  for (ci = 0, compptr = cinfo->m_CompInfoPtr; ci < cinfo->m_NumComponents;
       ci++, compptr++) {
    qtblno = compptr->quant_tbl_no;
    /* Make sure specified quantization table is present */
    if (qtblno < 0 || qtblno >= NUM_QUANT_TBLS ||
	cinfo->quant_tbl_ptrs[qtblno] == NULL)
      cinfo->FatalError(JPeg70::JERR_NO_QUANT_TABLE, qtblno);
    qtbl = cinfo->quant_tbl_ptrs[qtblno];
    /* Compute divisors for this quant table */
    /* We may do this more than once for same table, but it's not a big deal */
    switch (cinfo->dct_method) {
    case JPeg70::DCT_ISLOW:
      /* For LL&M IDCT method, divisors are equal to raw quantization
       * coefficients multiplied by 8 (to counteract scaling).
       */
      if (fdct->divisors[qtblno] == NULL) {
	fdct->divisors[qtblno] = (SWord32 *)
	  (*cinfo->mem->alloc_small) ((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE,
				      DCTSIZE2 * sizeof(SWord32));
      }
      dtbl = fdct->divisors[qtblno];
      for (i = 0; i < DCTSIZE2; i++) {
	dtbl[i] = ((SWord32) qtbl->quantval[i]) << 3;
      }
      break;
    case JPeg70::DCT_IFAST:
      {
	/* For AA&N IDCT method, divisors are equal to quantization
	 * coefficients scaled by scalefactor[row]*scalefactor[col], where
	 *   scalefactor[0] = 1
	 *   scalefactor[k] = cos(k*PI/16) * sqrt(2)    for k=1..7
	 * We apply a further scale factor of 8.
	 */
#define CONST_BITS 14
	static const short aanscales[DCTSIZE2] = {
	  /* precomputed values scaled up by 14 bits */
	  16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
	  22725, 31521, 29692, 26722, 22725, 17855, 12299,  6270,
	  21407, 29692, 27969, 25172, 21407, 16819, 11585,  5906,
	  19266, 26722, 25172, 22654, 19266, 15137, 10426,  5315,
	  16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
	  12873, 17855, 16819, 15137, 12873, 10114,  6967,  3552,
	   8867, 12299, 11585, 10426,  8867,  6967,  4799,  2446,
	   4520,  6270,  5906,  5315,  4520,  3552,  2446,  1247
	};
	

	if (fdct->divisors[qtblno] == NULL) {
	  fdct->divisors[qtblno] = (SWord32 *)
	    (*cinfo->mem->alloc_small) ((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE,
					DCTSIZE2 * sizeof(SWord32));
	}
	dtbl = fdct->divisors[qtblno];
	for (i = 0; i < DCTSIZE2; i++) {
	  dtbl[i] = (SWord32)
	    DESCALE(((long) qtbl->quantval[i]*(long) aanscales[i]),CONST_BITS-3);
	}
      }
      break;
    case JPeg70::DCT_FLOAT:
      {
	/* For float AA&N IDCT method, divisors are equal to quantization
	 * coefficients scaled by scalefactor[row]*scalefactor[col], where
	 *   scalefactor[0] = 1
	 *   scalefactor[k] = cos(k*PI/16) * sqrt(2)    for k=1..7
	 * We apply a further scale factor of 8.
	 * What's actually stored is 1/divisor so that the inner loop can
	 * use a multiplication rather than a division.
	 */
	float * fdtbl;
	int row, col;
	static const double aanscalefactor[DCTSIZE] = {
	  1.0, 1.387039845, 1.306562965, 1.175875602,
	  1.0, 0.785694958, 0.541196100, 0.275899379
	};

	if (fdct->float_divisors[qtblno] == NULL) {
	  fdct->float_divisors[qtblno] = (float *)
	    (*cinfo->mem->alloc_small) ((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE,
					DCTSIZE2 * sizeof(float));
	}
	fdtbl = fdct->float_divisors[qtblno];
	i = 0;
	for (row = 0; row < DCTSIZE; row++) {
	  for (col = 0; col < DCTSIZE; col++) {
	    fdtbl[i] = (float)
	      (1.0 / (((double) qtbl->quantval[i] *
		       aanscalefactor[row] * aanscalefactor[col] * 8.0)));
	    i++;
	  }
	}
      }
      break;
    default:
      cinfo->FatalError(JPeg70::JERR_NOT_COMPILED);
      break;
    }
  }
}


/*
 * Perform forward DCT on one or more blocks of a component.
 *
 * The input samples are taken from the sample_data[] array starting at
 * position start_row/start_col, and moving to the right for any additional
 * blocks. The quantized coefficients are returned in coef_blocks[].
 */

static void BURGERCALL forward_DCT (CJPegCompress * cinfo, JPeg70::ComponentInfo_t * compptr,
	     JSAMPLE * * sample_data, JPeg70::Block_t * coef_blocks,
	     Word start_row, Word start_col,
	     Word num_blocks)
/* This version is used for integer DCT implementations. */
{
  /* This routine is heavily used, so it's worth coding it tightly. */
  my_fdct_ptr fdct = (my_fdct_ptr) cinfo->fdct;
  SWord32 * divisors = fdct->divisors[compptr->quant_tbl_no];
  SWord32 workspace[DCTSIZE2];	/* work area for FDCT subroutine */
  Word bi;

  sample_data += start_row;	/* fold in the vertical offset once */

  for (bi = 0; bi < num_blocks; bi++, start_col += DCTSIZE) {
    /* Load data into workspace, applying unsigned->signed conversion */
    { register SWord32 *workspaceptr;
      register JSAMPLE * elemptr;
      register int elemr;

      workspaceptr = workspace;
      for (elemr = 0; elemr < DCTSIZE; elemr++) {
	elemptr = sample_data[elemr] + start_col;
#if DCTSIZE == 8		/* unroll the inner loop */
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	*workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
#else
	{ register int elemc;
	  for (elemc = DCTSIZE; elemc > 0; elemc--) {
	    *workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
	  }
	}
#endif
      }
    }

    /* Perform the DCT */
    fdct->do_dct(workspace);

    /* Quantize/descale the coefficients, and store into coef_blocks[] */
    { register SWord32 temp, qval;
      register int i;
      register SWord16 * output_ptr = coef_blocks[bi];

      for (i = 0; i < DCTSIZE2; i++) {
	qval = divisors[i];
	temp = workspace[i];
	/* Divide the coefficient value by qval, ensuring proper rounding.
	 * Since C does not specify the direction of rounding for negative
	 * quotients, we have to force the dividend positive for portability.
	 *
	 * In most files, at least half of the output values will be zero
	 * (at default quantization settings, more like three-quarters...)
	 * so we should ensure that this case is fast.  On many machines,
	 * a comparison is enough cheaper than a divide to make a special test
	 * a win.  Since both inputs will be nonnegative, we need only test
	 * for a < b to discover whether a/b is 0.
	 * If your machine's division is fast enough, define FAST_DIVIDE.
	 */
#ifdef FAST_DIVIDE
#define DIVIDE_BY(a,b)	a /= b
#else
#define DIVIDE_BY(a,b)	if (a >= b) a /= b; else a = 0
#endif
	if (temp < 0) {
	  temp = -temp;
	  temp += qval>>1;	/* for rounding */
	  DIVIDE_BY(temp, qval);
	  temp = -temp;
	} else {
	  temp += qval>>1;	/* for rounding */
	  DIVIDE_BY(temp, qval);
	}
	output_ptr[i] = (SWord16) temp;
      }
    }
  }
}


static void BURGERCALL forward_DCT_float (CJPegCompress * cinfo, JPeg70::ComponentInfo_t * compptr,
		   JSAMPLE * * sample_data, JPeg70::Block_t * coef_blocks,
		   Word start_row, Word start_col,
		   Word num_blocks)
/* This version is used for floating-point DCT implementations. */
{
  /* This routine is heavily used, so it's worth coding it tightly. */
  my_fdct_ptr fdct = (my_fdct_ptr) cinfo->fdct;
  float * divisors = fdct->float_divisors[compptr->quant_tbl_no];
  float workspace[DCTSIZE2]; /* work area for FDCT subroutine */
  Word bi;

  sample_data += start_row;	/* fold in the vertical offset once */

  for (bi = 0; bi < num_blocks; bi++, start_col += DCTSIZE) {
    /* Load data into workspace, applying unsigned->signed conversion */
    { register float *workspaceptr;
      register JSAMPLE * elemptr;
      register int elemr;

      workspaceptr = workspace;
      for (elemr = 0; elemr < DCTSIZE; elemr++) {
	elemptr = sample_data[elemr] + start_col;
#if DCTSIZE == 8		/* unroll the inner loop */
	*workspaceptr++ = (float)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	*workspaceptr++ = (float)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	*workspaceptr++ = (float)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	*workspaceptr++ = (float)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	*workspaceptr++ = (float)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	*workspaceptr++ = (float)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	*workspaceptr++ = (float)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	*workspaceptr++ = (float)(GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
#else
	{ register int elemc;
	  for (elemc = DCTSIZE; elemc > 0; elemc--) {
	    *workspaceptr++ = (float)
	      (GETJSAMPLE(*elemptr++) - CENTERJSAMPLE);
	  }
	}
#endif
      }
    }

    /* Perform the DCT */
    JPeg70::FDCTFloat(workspace);

    /* Quantize/descale the coefficients, and store into coef_blocks[] */
    { register float temp;
      register int i;
      register SWord16 * output_ptr = coef_blocks[bi];

      for (i = 0; i < DCTSIZE2; i++) {
	/* Apply the quantization and scaling factor */
	temp = workspace[i] * divisors[i];
	/* Round to nearest integer.
	 * Since C does not specify the direction of rounding for negative
	 * quotients, we have to force the dividend positive for portability.
	 * The maximum coefficient size is +-16K (for 12-bit data), so this
	 * code should work for either 16-bit or 32-bit ints.
	 */
	output_ptr[i] = (SWord16) ((int) (temp + (float) 16384.5) - 16384);
      }
    }
  }
}

/*
 * Initialize FDCT manager.
 */

void BURGERCALL jinit_forward_dct (CJPegCompress * cinfo)
{
	my_fdct_ptr fdct;
	int i;

	fdct = (my_fdct_ptr)
	(*cinfo->mem->alloc_small) ((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE,sizeof(my_fdct_controller));
	cinfo->fdct = (struct jpeg_forward_dct *) fdct;
	fdct->pub.start_pass = start_pass_fdctmgr;

	switch (cinfo->dct_method) {
	case JPeg70::DCT_ISLOW:
		fdct->pub.forward_DCT = forward_DCT;
		fdct->do_dct = JPeg70::FDCTISlow;
		break;
	case JPeg70::DCT_IFAST:
		fdct->pub.forward_DCT = forward_DCT;
		fdct->do_dct = JPeg70::FDCTIFast;
		break;
	case JPeg70::DCT_FLOAT:
		fdct->pub.forward_DCT = forward_DCT_float;
		break;
	default:
		cinfo->FatalError(JPeg70::JERR_NOT_COMPILED);
		break;
	}

	/* Mark divisor tables unallocated */
	for (i = 0; i < NUM_QUANT_TBLS; i++) {
		fdct->divisors[i] = NULL;
		fdct->float_divisors[i] = NULL;
	}
}
