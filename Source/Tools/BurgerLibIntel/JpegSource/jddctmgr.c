/*
 * jddctmgr.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the inverse-DCT management logic.
 * This code selects a particular IDCT implementation to be used,
 * and it performs related housekeeping chores.  No code in this file
 * is executed per IDCT step, only during output pass setup.
 *
 * Note that the IDCT routines are responsible for performing coefficient
 * dequantization as well as the IDCT proper.  This module sets up the
 * dequantization multiplier table needed by the IDCT routine.
 */

#include "JIDCTFlt.hpp"
#include "JIDCTFst.hpp"
#include "JIDCTInt.hpp"
#include "JIDCTRed.hpp"

#define JPEG_INTERNALS
#include "jpeglib.h"
#include "jdct.h"		/* Private declarations for DCT subsystem */


/*
 * The decompressor input side (jdinput.c) saves away the appropriate
 * quantization table for each component at the start of the first scan
 * involving that component.  (This is necessary in order to correctly
 * decode files that reuse Q-table slots.)
 * When we are ready to make an output pass, the saved Q-table is converted
 * to a multiplier table that will actually be used by the IDCT routine.
 * The multiplier table contents are IDCT-method-dependent.  To support
 * application changes in IDCT method between scans, we can remake the
 * multiplier tables if necessary.
 * In buffered-image mode, the first output pass may occur before any data
 * has been seen for some components, and thus before their Q-tables have
 * been saved away.  To handle this case, multiplier tables are preset
 * to zeroes; the result of the IDCT will be a neutral gray level.
 */


/* Private subobject for this module */

typedef struct {
  struct jpeg_inverse_dct pub;	/* public fields */

  /* This array contains the IDCT method code that each multiplier table
   * is currently set up for, or -1 if it's not yet set up.
   * The actual multiplier tables are pointed to by dct_table in the
   * per-component m_CompInfoPtr structures.
   */
  int cur_method[MAX_COMPONENTS];
} my_idct_controller;

typedef my_idct_controller * my_idct_ptr;


/* Allocated multiplier tables: big enough for any supported variant */

typedef union {
  SWord32 islow_array[DCTSIZE2];
  SWord32 ifast_array[DCTSIZE2];
  float float_array[DCTSIZE2];
} multiplier_table;


/* The current scaled-IDCT routines require ISLOW-style multiplier tables,
 * so be sure to compile that code if either ISLOW or SCALING is requested.
 */
#define PROVIDE_ISLOW_TABLES


/*
 * Prepare for an output pass.
 * Here we select the proper IDCT routine for each component and build
 * a matching multiplier table.
 */

static void BURGERCALL start_pass (CJPegDecompress * cinfo)
{
  my_idct_ptr idct = (my_idct_ptr) cinfo->idct;
  int ci, i;
  JPeg70::ComponentInfo_t *compptr;
  int method = 0;
  inverse_DCT_method_ptr method_ptr = NULL;
  JPeg70::QuantTable_t * qtbl;

  for (ci = 0, compptr = cinfo->m_CompInfoPtr; ci < cinfo->m_NumComponents;
       ci++, compptr++) {
    /* Select the proper IDCT routine for this component's scaling */
    switch (compptr->DCT_scaled_size) {
    case 1:
      method_ptr = JPeg70::IDCT1x1;
      method = JPeg70::DCT_ISLOW;	/* jidctred uses islow-style table */
      break;
    case 2:
      method_ptr = JPeg70::IDCT2x2;
      method = JPeg70::DCT_ISLOW;	/* jidctred uses islow-style table */
      break;
    case 4:
      method_ptr = JPeg70::IDCT4x4;
      method = JPeg70::DCT_ISLOW;	/* jidctred uses islow-style table */
      break;
    case DCTSIZE:
      switch (cinfo->dct_method) {
      case JPeg70::DCT_ISLOW:
	method_ptr = JPeg70::IDCTISlow;
	method = JPeg70::DCT_ISLOW;
	break;
      case JPeg70::DCT_IFAST:
	method_ptr = JPeg70::IDCTFast;
	method = JPeg70::DCT_IFAST;
	break;
      case JPeg70::DCT_FLOAT:
	method_ptr = JPeg70::IDCTFloat;
	method = JPeg70::DCT_FLOAT;
	break;
      default:
	cinfo->FatalError(JPeg70::JERR_NOT_COMPILED);
	break;
      }
      break;
    default:
      cinfo->FatalError(JPeg70::JERR_BAD_DCTSIZE, compptr->DCT_scaled_size);
      break;
    }
    idct->pub.inverse_DCT[ci] = method_ptr;
    /* Create multiplier table from quant table.
     * However, we can skip this if the component is uninteresting
     * or if we already built the table.  Also, if no quant table
     * has yet been saved for the component, we leave the
     * multiplier table all-zero; we'll be reading zeroes from the
     * coefficient controller's buffer anyway.
     */
    if (! compptr->component_needed || idct->cur_method[ci] == method)
      continue;
    qtbl = compptr->quant_table;
    if (qtbl == NULL)		/* happens if no data yet for component */
      continue;
    idct->cur_method[ci] = method;
    switch (method) {
#ifdef PROVIDE_ISLOW_TABLES
    case JPeg70::DCT_ISLOW:
      {
	/* For LL&M IDCT method, multipliers are equal to raw quantization
	 * coefficients, but are stored as ints to ensure access efficiency.
	 */
	SWord32 * ismtbl = compptr->DctTablePtr.Int;
	for (i = 0; i < DCTSIZE2; i++) {
	  ismtbl[i] = (SWord32) qtbl->quantval[i];
	}
      }
      break;
#endif
    case JPeg70::DCT_IFAST:
      {
	/* For AA&N IDCT method, multipliers are equal to quantization
	 * coefficients scaled by scalefactor[row]*scalefactor[col], where
	 *   scalefactor[0] = 1
	 *   scalefactor[k] = cos(k*PI/16) * sqrt(2)    for k=1..7
	 * For integer operation, the multiplier table is to be scaled by
	 * IFAST_SCALE_BITS.
	 */
	SWord32 * ifmtbl = compptr->DctTablePtr.Int;
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
	

	for (i = 0; i < DCTSIZE2; i++) {
	  ifmtbl[i] = (SWord32)
	    DESCALE(((long) qtbl->quantval[i]*(long) aanscales[i]),CONST_BITS-IFAST_SCALE_BITS);
	}
      }
      break;
    case JPeg70::DCT_FLOAT:
      {
	/* For float AA&N IDCT method, multipliers are equal to quantization
	 * coefficients scaled by scalefactor[row]*scalefactor[col], where
	 *   scalefactor[0] = 1
	 *   scalefactor[k] = cos(k*PI/16) * sqrt(2)    for k=1..7
	 */
	float * fmtbl = compptr->DctTablePtr.Float;
	int row, col;
	static const double aanscalefactor[DCTSIZE] = {
	  1.0, 1.387039845, 1.306562965, 1.175875602,
	  1.0, 0.785694958, 0.541196100, 0.275899379
	};

	i = 0;
	for (row = 0; row < DCTSIZE; row++) {
	  for (col = 0; col < DCTSIZE; col++) {
	    fmtbl[i] = (float)((double) qtbl->quantval[i] *aanscalefactor[row] * aanscalefactor[col]);
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
 * Initialize IDCT manager.
 */

void BURGERCALL jinit_inverse_dct (CJPegDecompress * cinfo)
{
	my_idct_ptr idct;
	int ci;
	JPeg70::ComponentInfo_t *compptr;

	idct = (my_idct_ptr)(*cinfo->mem->alloc_small) ((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE,sizeof(my_idct_controller));
	cinfo->idct = (struct jpeg_inverse_dct *) idct;
	idct->pub.start_pass = start_pass;

	for (ci = 0, compptr = cinfo->m_CompInfoPtr; ci < cinfo->m_NumComponents;ci++, compptr++) {
		/* Allocate and pre-zero a multiplier table for each component */
		compptr->DctTablePtr.Int = (SWord32 *)(*cinfo->mem->alloc_small) ((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE,sizeof(multiplier_table));
		FastMemSet(compptr->DctTablePtr.Int,0, sizeof(multiplier_table));
		/* Mark multiplier table not yet set up for any method */
		idct->cur_method[ci] = -1;
	}
}
