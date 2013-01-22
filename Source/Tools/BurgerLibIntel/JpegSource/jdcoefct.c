/*
 * jdcoefct.c
 *
 * Copyright (C) 1994-1997, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the coefficient buffer controller for decompression.
 * This controller is the top level of the JPEG decompressor proper.
 * The coefficient buffer lies between entropy decoding and inverse-DCT steps.
 *
 * In buffered-image mode, this controller is the interface between
 * input-oriented processing and output-oriented processing.
 * Also, the input side (only) is used when reading a file for transcoding.
 */

#define JPEG_INTERNALS
#include "jpeglib.h"
#include "JUtils.hpp"

/* Private buffer controller object */

typedef struct {
	struct jpeg_d_coef_controller pub;		/* public fields */

  /* These variables keep track of the current location of the input side. */
  /* cinfo->input_iMCU_row is also used for this. */
  Word MCU_ctr;		/* counts MCUs processed in current row */
  int MCU_vert_offset;		/* counts MCU rows within iMCU row */
  int MCU_rows_per_iMCU_row;	/* number of such rows needed */

  /* The output side's location is represented by cinfo->output_iMCU_row. */

  /* In single-pass modes, it's sufficient to buffer just one MCU.
   * We allocate a workspace of D_MAX_BLOCKS_IN_MCU coefficient blocks,
   * and let the entropy decoder write into that workspace each time.
   * (On 80x86, the workspace is even though it's not really very big;
   * this is to keep the module interfaces unchanged when a large coefficient
   * buffer is necessary.)
   * In multi-pass modes, this array points to the current MCU's blocks
   * within the virtual arrays; it is used only by the input side.
   */
  JPeg70::Block_t * MCU_buffer[D_MAX_BLOCKS_IN_MCU];

  /* In multi-pass modes, we need a virtual block array for each component. */
  struct jvirt_barray_control * whole_image[MAX_COMPONENTS];

  /* When doing block smoothing, we latch coefficient Al values here */
  int * coef_bits_latch;
#define SAVED_COEFS  6		/* we save coef_bits[0..5] */
} my_coef_controller;

typedef my_coef_controller * my_coef_ptr;

/* Forward declarations */
static int BURGERCALL decompress_onepass(CJPegDecompress * cinfo, JSAMPLE * * * output_buf);
static int BURGERCALL decompress_data(CJPegDecompress * cinfo, JSAMPLE * * * output_buf);
static Word8 BURGERCALL smoothing_ok (CJPegDecompress * cinfo);
static int BURGERCALL decompress_smooth_data(CJPegDecompress * cinfo, JSAMPLE * * * output_buf);


static void start_iMCU_row (CJPegDecompress * cinfo)
/* Reset within-iMCU-row counters for a new row (input side) */
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;

  /* In an interleaved scan, an MCU row is the same as an iMCU row.
   * In a noninterleaved scan, an iMCU row has v_samp_factor MCU rows.
   * But at the bottom of the image, process only what's left.
   */
  if (cinfo->comps_in_scan > 1) {
    coef->MCU_rows_per_iMCU_row = 1;
  } else {
    if (cinfo->input_iMCU_row < (cinfo->total_iMCU_rows-1))
      coef->MCU_rows_per_iMCU_row = cinfo->cur_comp_info[0]->v_samp_factor;
    else
      coef->MCU_rows_per_iMCU_row = cinfo->cur_comp_info[0]->last_row_height;
  }

  coef->MCU_ctr = 0;
  coef->MCU_vert_offset = 0;
}


/*
 * Initialize for an input processing pass.
 */

static void BURGERCALL start_input_pass (CJPegDecompress * cinfo)
{
  cinfo->input_iMCU_row = 0;
  start_iMCU_row(cinfo);
}


/*
 * Initialize for an output processing pass.
 */

static void BURGERCALL start_output_pass (CJPegDecompress * cinfo)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;

  /* If multipass, check to see whether to use block smoothing on this pass */
  if (coef->pub.coef_arrays != NULL) {
    if (cinfo->do_block_smoothing && smoothing_ok(cinfo))
      coef->pub.decompress_data = decompress_smooth_data;
    else
      coef->pub.decompress_data = decompress_data;
  }
  cinfo->output_iMCU_row = 0;
}


/*
 * Decompress and return some data in the single-pass case.
 * Always attempts to emit one fully interleaved MCU row ("iMCU" row).
 * Input and output must run in lockstep since we have only a one-MCU buffer.
 * Return value is JPEG_ROW_COMPLETED, JPEG_SCAN_COMPLETED, or JPEG_SUSPENDED.
 *
 * NB: output_buf contains a plane for each component in image,
 * which we index according to the component's SOF position.
 */

static int BURGERCALL decompress_onepass (CJPegDecompress * cinfo, JSAMPLE * * * output_buf)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  Word MCU_col_num;	/* index of current MCU within row */
  Word last_MCU_col = cinfo->MCUs_per_row - 1;
  Word last_iMCU_row = cinfo->total_iMCU_rows - 1;
  int blkn, ci, xindex, yindex, yoffset, useful_width;
  JSAMPLE * * output_ptr;
  Word start_col, output_col;
  JPeg70::ComponentInfo_t *compptr;
  inverse_DCT_method_ptr inverse_DCT;

  /* Loop to process as much as one whole iMCU row */
  for (yoffset = coef->MCU_vert_offset; yoffset < coef->MCU_rows_per_iMCU_row;
       yoffset++) {
    for (MCU_col_num = coef->MCU_ctr; MCU_col_num <= last_MCU_col;
	 MCU_col_num++) {
      /* Try to fetch an MCU.  Entropy decoder expects buffer to be zeroed. */
      FastMemSet((void *) coef->MCU_buffer[0],0,(Word32)(cinfo->blocks_in_MCU * sizeof(JPeg70::Block_t)));
      if (! (*cinfo->entropy->decode_mcu) (cinfo, coef->MCU_buffer)) {
	/* Suspension forced; update state counters and exit */
	coef->MCU_vert_offset = yoffset;
	coef->MCU_ctr = MCU_col_num;
	return JPEG_SUSPENDED;
      }
      /* Determine where data should go in output_buf and do the IDCT thing.
       * We skip dummy blocks at the right and bottom edges (but blkn gets
       * incremented past them!).  Note the inner loop relies on having
       * allocated the MCU_buffer[] blocks sequentially.
       */
      blkn = 0;			/* index of current DCT block within MCU */
      for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
	compptr = cinfo->cur_comp_info[ci];
	/* Don't bother to IDCT an uninteresting component. */
	if (! compptr->component_needed) {
	  blkn += compptr->MCU_blocks;
	  continue;
	}
	inverse_DCT = cinfo->idct->inverse_DCT[compptr->component_index];
	useful_width = (MCU_col_num < last_MCU_col) ? compptr->MCU_width
						    : compptr->last_col_width;
	output_ptr = output_buf[compptr->component_index] +
	  yoffset * compptr->DCT_scaled_size;
	start_col = MCU_col_num * compptr->MCU_sample_width;
	for (yindex = 0; yindex < compptr->MCU_height; yindex++) {
	  if (cinfo->input_iMCU_row < last_iMCU_row ||
	      yoffset+yindex < compptr->last_row_height) {
	    output_col = start_col;
	    for (xindex = 0; xindex < useful_width; xindex++) {
	      (*inverse_DCT) (cinfo, compptr,
			      (SWord16 *) coef->MCU_buffer[blkn+xindex],
			      output_ptr, output_col);
	      output_col += compptr->DCT_scaled_size;
	    }
	  }
	  blkn += compptr->MCU_width;
	  output_ptr += compptr->DCT_scaled_size;
	}
      }
    }
    /* Completed an MCU row, but perhaps not an iMCU row */
    coef->MCU_ctr = 0;
  }
  /* Completed the iMCU row, advance counters for next one */
  cinfo->output_iMCU_row++;
  if (++(cinfo->input_iMCU_row) < cinfo->total_iMCU_rows) {
    start_iMCU_row(cinfo);
    return JPEG_ROW_COMPLETED;
  }
  /* Completed the scan */
  (*cinfo->inputctl->finish_input_pass) (cinfo);
  return JPEG_SCAN_COMPLETED;
}


/*
 * Dummy consume-input routine for single-pass operation.
 */

static int BURGERCALL dummy_consume_data (CJPegDecompress * /* cinfo */)
{
  return JPEG_SUSPENDED;	/* Always indicate nothing was done */
}

/*
 * Consume input data and store it in the full-image coefficient buffer.
 * We read as much as one fully interleaved MCU row ("iMCU" row) per call,
 * ie, v_samp_factor block rows for each component in the scan.
 * Return value is JPEG_ROW_COMPLETED, JPEG_SCAN_COMPLETED, or JPEG_SUSPENDED.
 */

static int BURGERCALL consume_data (CJPegDecompress * cinfo)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  Word MCU_col_num;	/* index of current MCU within row */
  int blkn, ci, xindex, yindex, yoffset;
  Word start_col;
  JPeg70::Block_t * * buffer[MAX_COMPS_IN_SCAN];
  JPeg70::Block_t * buffer_ptr;
  JPeg70::ComponentInfo_t *compptr;

  /* Align the virtual buffers for the components used in this scan. */
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    buffer[ci] = (*cinfo->mem->access_virt_barray)
      ((JPeg70::CCommonManager *) cinfo, coef->whole_image[compptr->component_index],
       cinfo->input_iMCU_row * compptr->v_samp_factor,
       (Word) compptr->v_samp_factor, TRUE);
    /* Note: entropy decoder expects buffer to be zeroed,
     * but this is handled automatically by the memory manager
     * because we requested a pre-zeroed array.
     */
  }

  /* Loop to process one whole iMCU row */
  for (yoffset = coef->MCU_vert_offset; yoffset < coef->MCU_rows_per_iMCU_row;
       yoffset++) {
    for (MCU_col_num = coef->MCU_ctr; MCU_col_num < cinfo->MCUs_per_row;
	 MCU_col_num++) {
      /* Construct list of pointers to DCT blocks belonging to this MCU */
      blkn = 0;			/* index of current DCT block within MCU */
      for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
	compptr = cinfo->cur_comp_info[ci];
	start_col = MCU_col_num * compptr->MCU_width;
	for (yindex = 0; yindex < compptr->MCU_height; yindex++) {
	  buffer_ptr = buffer[ci][yindex+yoffset] + start_col;
	  for (xindex = 0; xindex < compptr->MCU_width; xindex++) {
	    coef->MCU_buffer[blkn++] = buffer_ptr++;
	  }
	}
      }
      /* Try to fetch the MCU. */
      if (! (*cinfo->entropy->decode_mcu) (cinfo, coef->MCU_buffer)) {
	/* Suspension forced; update state counters and exit */
	coef->MCU_vert_offset = yoffset;
	coef->MCU_ctr = MCU_col_num;
	return JPEG_SUSPENDED;
      }
    }
    /* Completed an MCU row, but perhaps not an iMCU row */
    coef->MCU_ctr = 0;
  }
  /* Completed the iMCU row, advance counters for next one */
  if (++(cinfo->input_iMCU_row) < cinfo->total_iMCU_rows) {
    start_iMCU_row(cinfo);
    return JPEG_ROW_COMPLETED;
  }
  /* Completed the scan */
  (*cinfo->inputctl->finish_input_pass) (cinfo);
  return JPEG_SCAN_COMPLETED;
}


/*
 * Decompress and return some data in the multi-pass case.
 * Always attempts to emit one fully interleaved MCU row ("iMCU" row).
 * Return value is JPEG_ROW_COMPLETED, JPEG_SCAN_COMPLETED, or JPEG_SUSPENDED.
 *
 * NB: output_buf contains a plane for each component in image.
 */

static int BURGERCALL decompress_data (CJPegDecompress * cinfo, JSAMPLE * * * output_buf)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  Word last_iMCU_row = cinfo->total_iMCU_rows - 1;
  Word block_num;
  int ci, block_row, block_rows;
  JPeg70::Block_t * * buffer;
  JPeg70::Block_t * buffer_ptr;
  JSAMPLE * * output_ptr;
  Word output_col;
  JPeg70::ComponentInfo_t *compptr;
  inverse_DCT_method_ptr inverse_DCT;

  /* Force some input to be done if we are getting ahead of the input. */
  while (cinfo->input_scan_number < cinfo->output_scan_number ||
	 (cinfo->input_scan_number == cinfo->output_scan_number &&
	  cinfo->input_iMCU_row <= cinfo->output_iMCU_row)) {
    if ((*cinfo->inputctl->consume_input)(cinfo) == JPEG_SUSPENDED)
      return JPEG_SUSPENDED;
  }

  /* OK, output from the virtual arrays. */
  for (ci = 0, compptr = cinfo->m_CompInfoPtr; ci < cinfo->m_NumComponents;
       ci++, compptr++) {
    /* Don't bother to IDCT an uninteresting component. */
    if (! compptr->component_needed)
      continue;
    /* Align the virtual buffer for this component. */
    buffer = (*cinfo->mem->access_virt_barray)
      ((JPeg70::CCommonManager *) cinfo, coef->whole_image[ci],
       cinfo->output_iMCU_row * compptr->v_samp_factor,
       (Word) compptr->v_samp_factor, FALSE);
    /* Count non-dummy DCT block rows in this iMCU row. */
    if (cinfo->output_iMCU_row < last_iMCU_row)
      block_rows = compptr->v_samp_factor;
    else {
      /* NB: can't use last_row_height here; it is input-side-dependent! */
      block_rows = (int) (compptr->height_in_blocks % compptr->v_samp_factor);
      if (block_rows == 0) block_rows = compptr->v_samp_factor;
    }
    inverse_DCT = cinfo->idct->inverse_DCT[ci];
    output_ptr = output_buf[ci];
    /* Loop over all DCT blocks to be processed. */
    for (block_row = 0; block_row < block_rows; block_row++) {
      buffer_ptr = buffer[block_row];
      output_col = 0;
      for (block_num = 0; block_num < compptr->width_in_blocks; block_num++) {
	(*inverse_DCT) (cinfo, compptr, (SWord16 *) buffer_ptr,
			output_ptr, output_col);
	buffer_ptr++;
	output_col += compptr->DCT_scaled_size;
      }
      output_ptr += compptr->DCT_scaled_size;
    }
  }

  if (++(cinfo->output_iMCU_row) < cinfo->total_iMCU_rows)
    return JPEG_ROW_COMPLETED;
  return JPEG_SCAN_COMPLETED;
}

/*
 * This code applies interblock smoothing as described by section K.8
 * of the JPEG standard: the first 5 AC coefficients are estimated from
 * the DC values of a DCT block and its 8 neighboring blocks.
 * We apply smoothing only for progressive JPEG decoding, and only if
 * the coefficients it can estimate are not yet known to full precision.
 */

/* Natural-order array positions of the first 5 zigzag-order coefficients */
#define Q01_POS  1
#define Q10_POS  8
#define Q20_POS  16
#define Q11_POS  9
#define Q02_POS  2

/*
 * Determine whether block smoothing is applicable and safe.
 * We also latch the current states of the coef_bits[] entries for the
 * AC coefficients; otherwise, if the input side of the decompressor
 * advances into a new scan, we might think the coefficients are known
 * more accurately than they really are.
 */

static Word8 BURGERCALL smoothing_ok (CJPegDecompress * cinfo)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  Word8 smoothing_useful = FALSE;
  int ci, coefi;
  JPeg70::ComponentInfo_t *compptr;
  JPeg70::QuantTable_t * qtable;
  int * coef_bits;
  int * coef_bits_latch;

  if (! cinfo->progressive_mode || cinfo->coef_bits == NULL)
    return FALSE;

  /* Allocate latch area if not already done */
  if (coef->coef_bits_latch == NULL)
    coef->coef_bits_latch = (int *)
      (*cinfo->mem->alloc_small) ((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE,
				  cinfo->m_NumComponents *
				  (SAVED_COEFS * sizeof(int)));
  coef_bits_latch = coef->coef_bits_latch;

  for (ci = 0, compptr = cinfo->m_CompInfoPtr; ci < cinfo->m_NumComponents;
       ci++, compptr++) {
    /* All components' quantization values must already be latched. */
    if ((qtable = compptr->quant_table) == NULL)
      return FALSE;
    /* Verify DC & first 5 AC quantizers are nonzero to avoid zero-divide. */
    if (qtable->quantval[0] == 0 ||
	qtable->quantval[Q01_POS] == 0 ||
	qtable->quantval[Q10_POS] == 0 ||
	qtable->quantval[Q20_POS] == 0 ||
	qtable->quantval[Q11_POS] == 0 ||
	qtable->quantval[Q02_POS] == 0)
      return FALSE;
    /* DC values must be at least partly known for all components. */
    coef_bits = cinfo->coef_bits[ci];
    if (coef_bits[0] < 0)
      return FALSE;
    /* Block smoothing is helpful if some AC coefficients remain inaccurate. */
    for (coefi = 1; coefi <= 5; coefi++) {
      coef_bits_latch[coefi] = coef_bits[coefi];
      if (coef_bits[coefi] != 0)
	smoothing_useful = TRUE;
    }
    coef_bits_latch += SAVED_COEFS;
  }

  return smoothing_useful;
}


/*
 * Variant of decompress_data for use when doing block smoothing.
 */

static int BURGERCALL decompress_smooth_data (CJPegDecompress * cinfo, JSAMPLE * * * output_buf)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  Word last_iMCU_row = cinfo->total_iMCU_rows - 1;
  Word block_num, last_block_column;
  int ci, block_row, block_rows, access_rows;
  JPeg70::Block_t * * buffer;
  JPeg70::Block_t * buffer_ptr;
  JPeg70::Block_t *prev_block_row;
  JPeg70::Block_t *next_block_row;
  JSAMPLE * * output_ptr;
  Word output_col;
  JPeg70::ComponentInfo_t *compptr;
  inverse_DCT_method_ptr inverse_DCT;
  Word8 first_row, last_row;
  JPeg70::Block_t workspace;
  int *coef_bits;
  JPeg70::QuantTable_t *quanttbl;
  long Q00,Q01,Q02,Q10,Q11,Q20, num;
  int DC1,DC2,DC3,DC4,DC5,DC6,DC7,DC8,DC9;
  int Al, pred;

  /* Force some input to be done if we are getting ahead of the input. */
  while (cinfo->input_scan_number <= cinfo->output_scan_number &&
	 ! cinfo->inputctl->eoi_reached) {
    if (cinfo->input_scan_number == cinfo->output_scan_number) {
      /* If input is working on current scan, we ordinarily want it to
       * have completed the current row.  But if input scan is DC,
       * we want it to keep one row ahead so that next block row's DC
       * values are up to date.
       */
      Word delta = (cinfo->Ss == 0) ? 1U : 0;
      if (cinfo->input_iMCU_row > cinfo->output_iMCU_row+delta)
	break;
    }
    if ((*cinfo->inputctl->consume_input)(cinfo) == JPEG_SUSPENDED)
      return JPEG_SUSPENDED;
  }

  /* OK, output from the virtual arrays. */
  for (ci = 0, compptr = cinfo->m_CompInfoPtr; ci < cinfo->m_NumComponents;
       ci++, compptr++) {
    /* Don't bother to IDCT an uninteresting component. */
    if (! compptr->component_needed)
      continue;
    /* Count non-dummy DCT block rows in this iMCU row. */
    if (cinfo->output_iMCU_row < last_iMCU_row) {
      block_rows = compptr->v_samp_factor;
      access_rows = block_rows * 2; /* this and next iMCU row */
      last_row = FALSE;
    } else {
      /* NB: can't use last_row_height here; it is input-side-dependent! */
      block_rows = (int) (compptr->height_in_blocks % compptr->v_samp_factor);
      if (block_rows == 0) block_rows = compptr->v_samp_factor;
      access_rows = block_rows; /* this iMCU row only */
      last_row = TRUE;
    }
    /* Align the virtual buffer for this component. */
    if (cinfo->output_iMCU_row > 0) {
      access_rows += compptr->v_samp_factor; /* prior iMCU row too */
      buffer = (*cinfo->mem->access_virt_barray)
	((JPeg70::CCommonManager *) cinfo, coef->whole_image[ci],
	 (cinfo->output_iMCU_row - 1) * compptr->v_samp_factor,
	 (Word) access_rows, FALSE);
      buffer += compptr->v_samp_factor;	/* point to current iMCU row */
      first_row = FALSE;
    } else {
      buffer = (*cinfo->mem->access_virt_barray)
	((JPeg70::CCommonManager *) cinfo, coef->whole_image[ci],
	 (Word) 0, (Word) access_rows, FALSE);
      first_row = TRUE;
    }
    /* Fetch component-dependent info */
    coef_bits = coef->coef_bits_latch + (ci * SAVED_COEFS);
    quanttbl = compptr->quant_table;
    Q00 = quanttbl->quantval[0];
    Q01 = quanttbl->quantval[Q01_POS];
    Q10 = quanttbl->quantval[Q10_POS];
    Q20 = quanttbl->quantval[Q20_POS];
    Q11 = quanttbl->quantval[Q11_POS];
    Q02 = quanttbl->quantval[Q02_POS];
    inverse_DCT = cinfo->idct->inverse_DCT[ci];
    output_ptr = output_buf[ci];
    /* Loop over all DCT blocks to be processed. */
    for (block_row = 0; block_row < block_rows; block_row++) {
      buffer_ptr = buffer[block_row];
      if (first_row && block_row == 0)
	prev_block_row = buffer_ptr;
      else
	prev_block_row = buffer[block_row-1];
      if (last_row && block_row == block_rows-1)
	next_block_row = buffer_ptr;
      else
	next_block_row = buffer[block_row+1];
      /* We fetch the surrounding DC values using a sliding-register approach.
       * Initialize all nine here so as to do the right thing on narrow pics.
       */
      DC1 = DC2 = DC3 = (int) prev_block_row[0][0];
      DC4 = DC5 = DC6 = (int) buffer_ptr[0][0];
      DC7 = DC8 = DC9 = (int) next_block_row[0][0];
      output_col = 0;
      last_block_column = compptr->width_in_blocks - 1;
      for (block_num = 0; block_num <= last_block_column; block_num++) {
	/* Fetch current DCT block into workspace so we can modify it. */
	FastMemCpy(workspace,buffer_ptr,1*(DCTSIZE2 * sizeof(SWord16)));
	/* Update DC values */
	if (block_num < last_block_column) {
	  DC3 = (int) prev_block_row[1][0];
	  DC6 = (int) buffer_ptr[1][0];
	  DC9 = (int) next_block_row[1][0];
	}
	/* Compute coefficient estimates per K.8.
	 * An estimate is applied only if coefficient is still zero,
	 * and is not known to be fully accurate.
	 */
	/* AC01 */
	if ((Al=coef_bits[1]) != 0 && workspace[1] == 0) {
	  num = 36 * Q00 * (DC4 - DC6);
	  if (num >= 0) {
	    pred = (int) (((Q01<<7) + num) / (Q01<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	  } else {
	    pred = (int) (((Q01<<7) - num) / (Q01<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	    pred = -pred;
	  }
	  workspace[1] = (SWord16) pred;
	}
	/* AC10 */
	if ((Al=coef_bits[2]) != 0 && workspace[8] == 0) {
	  num = 36 * Q00 * (DC2 - DC8);
	  if (num >= 0) {
	    pred = (int) (((Q10<<7) + num) / (Q10<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	  } else {
	    pred = (int) (((Q10<<7) - num) / (Q10<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	    pred = -pred;
	  }
	  workspace[8] = (SWord16) pred;
	}
	/* AC20 */
	if ((Al=coef_bits[3]) != 0 && workspace[16] == 0) {
	  num = 9 * Q00 * (DC2 + DC8 - 2*DC5);
	  if (num >= 0) {
	    pred = (int) (((Q20<<7) + num) / (Q20<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	  } else {
	    pred = (int) (((Q20<<7) - num) / (Q20<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	    pred = -pred;
	  }
	  workspace[16] = (SWord16) pred;
	}
	/* AC11 */
	if ((Al=coef_bits[4]) != 0 && workspace[9] == 0) {
	  num = 5 * Q00 * (DC1 - DC3 - DC7 + DC9);
	  if (num >= 0) {
	    pred = (int) (((Q11<<7) + num) / (Q11<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	  } else {
	    pred = (int) (((Q11<<7) - num) / (Q11<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	    pred = -pred;
	  }
	  workspace[9] = (SWord16) pred;
	}
	/* AC02 */
	if ((Al=coef_bits[5]) != 0 && workspace[2] == 0) {
	  num = 9 * Q00 * (DC4 + DC6 - 2*DC5);
	  if (num >= 0) {
	    pred = (int) (((Q02<<7) + num) / (Q02<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	  } else {
	    pred = (int) (((Q02<<7) - num) / (Q02<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	    pred = -pred;
	  }
	  workspace[2] = (SWord16) pred;
	}
	/* OK, do the IDCT */
	(*inverse_DCT) (cinfo, compptr, (SWord16 *) workspace,
			output_ptr, output_col);
	/* Advance for next column */
	DC1 = DC2; DC2 = DC3;
	DC4 = DC5; DC5 = DC6;
	DC7 = DC8; DC8 = DC9;
	buffer_ptr++, prev_block_row++, next_block_row++;
	output_col += compptr->DCT_scaled_size;
      }
      output_ptr += compptr->DCT_scaled_size;
    }
  }

  if (++(cinfo->output_iMCU_row) < cinfo->total_iMCU_rows)
    return JPEG_ROW_COMPLETED;
  return JPEG_SCAN_COMPLETED;
}


/*
 * Initialize coefficient buffer controller.
 */

void BURGERCALL jinit_d_coef_controller (CJPegDecompress * cinfo, Word8 need_full_buffer)
{
  my_coef_ptr coef;

  coef = (my_coef_ptr)
    (*cinfo->mem->alloc_small) ((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE,
				sizeof(my_coef_controller));
  cinfo->coef = (struct jpeg_d_coef_controller *) coef;
  coef->pub.start_input_pass = start_input_pass;
  coef->pub.start_output_pass = start_output_pass;
  coef->coef_bits_latch = NULL;

  /* Create the coefficient buffer. */
  if (need_full_buffer) {
    /* Allocate a full-image virtual array for each component, */
    /* padded to a multiple of samp_factor DCT blocks in each direction. */
    /* Note we ask for a pre-zeroed array. */
    int ci, access_rows;
    JPeg70::ComponentInfo_t *compptr;

    for (ci = 0, compptr = cinfo->m_CompInfoPtr; ci < cinfo->m_NumComponents;
	 ci++, compptr++) {
      access_rows = compptr->v_samp_factor;
      /* If block smoothing could be used, need a bigger window */
      if (cinfo->progressive_mode)
	access_rows *= 3;
      coef->whole_image[ci] = (*cinfo->mem->request_virt_barray)
	((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE, TRUE,
	 (Word) JPeg70::RoundUp((Word32) compptr->width_in_blocks,(Word32) compptr->h_samp_factor),
	 (Word) JPeg70::RoundUp((Word32) compptr->height_in_blocks,(Word32) compptr->v_samp_factor),
	 (Word) access_rows);
    }
    coef->pub.consume_data = consume_data;
    coef->pub.decompress_data = decompress_data;
    coef->pub.coef_arrays = coef->whole_image; /* link to virtual arrays */
  } else {
    /* We only need a single-MCU buffer. */
    JPeg70::Block_t * buffer;
    int i;

    buffer = (JPeg70::Block_t *)(*cinfo->mem->alloc_large) ((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE,
				  D_MAX_BLOCKS_IN_MCU * sizeof(JPeg70::Block_t));
    for (i = 0; i < D_MAX_BLOCKS_IN_MCU; i++) {
      coef->MCU_buffer[i] = buffer + i;
    }
    coef->pub.consume_data = dummy_consume_data;
    coef->pub.decompress_data = decompress_onepass;
    coef->pub.coef_arrays = NULL; /* flag for no virtual arrays */
  }
}
