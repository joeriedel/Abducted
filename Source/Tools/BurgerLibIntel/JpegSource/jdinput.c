/*
 * jdinput.c
 *
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains input control logic for the JPEG decompressor.
 * These routines are concerned with controlling the decompressor's input
 * processing (marker reading and coefficient decoding).  The actual input
 * reading is done in jdmarker.c, jdhuff.c, and jdphuff.c.
 */

#define JPEG_INTERNALS
#include "jpeglib.h"
#include "JUtils.hpp"

/* Private state */

typedef struct {
  struct jpeg_input_controller pub; /* public fields */

  Word8 inheaders;		/* TRUE until first SOS is reached */
} my_input_controller;

typedef my_input_controller * my_inputctl_ptr;


/* Forward declarations */
static int BURGERCALL consume_markers (CJPegDecompress * cinfo);


/*
 * Routines to calculate various quantities related to the size of the image.
 */

static void initial_setup (CJPegDecompress * cinfo)
/* Called once, when first SOS marker is reached */
{
  int ci;
  JPeg70::ComponentInfo_t *compptr;

  /* Make sure image isn't bigger than I can handle */
  if ((long) cinfo->image_height > (long) JPEG_MAX_DIMENSION ||
      (long) cinfo->image_width > (long) JPEG_MAX_DIMENSION)
    cinfo->FatalError(JPeg70::JERR_IMAGE_TOO_BIG, (unsigned int) JPEG_MAX_DIMENSION);

  /* For now, precision must match compiled-in value... */
  if (cinfo->data_precision != BITS_IN_JSAMPLE)
    cinfo->FatalError(JPeg70::JERR_BAD_PRECISION, cinfo->data_precision);

  /* Check that number of components won't exceed internal array sizes */
  if (cinfo->m_NumComponents > MAX_COMPONENTS)
    cinfo->FatalError(JPeg70::JERR_COMPONENT_COUNT, cinfo->m_NumComponents,
	     MAX_COMPONENTS);

  /* Compute maximum sampling factors; check factor validity */
  cinfo->max_h_samp_factor = 1;
  cinfo->max_v_samp_factor = 1;
  for (ci = 0, compptr = cinfo->m_CompInfoPtr; ci < cinfo->m_NumComponents;
       ci++, compptr++) {
    if (compptr->h_samp_factor<=0 || compptr->h_samp_factor>MAX_SAMP_FACTOR ||
	compptr->v_samp_factor<=0 || compptr->v_samp_factor>MAX_SAMP_FACTOR)
      cinfo->FatalError(JPeg70::JERR_BAD_SAMPLING);
    cinfo->max_h_samp_factor = MAX(cinfo->max_h_samp_factor,
				   compptr->h_samp_factor);
    cinfo->max_v_samp_factor = MAX(cinfo->max_v_samp_factor,
				   compptr->v_samp_factor);
  }

  /* We initialize DCT_scaled_size and min_DCT_scaled_size to DCTSIZE.
   * In the full decompressor, this will be overridden by jdmaster.c;
   * but in the transcoder, jdmaster.c is not used, so we must do it here.
   */
  cinfo->min_DCT_scaled_size = DCTSIZE;

  /* Compute dimensions of components */
  for (ci = 0, compptr = cinfo->m_CompInfoPtr; ci < cinfo->m_NumComponents;
       ci++, compptr++) {
    compptr->DCT_scaled_size = DCTSIZE;
    /* Size in DCT blocks */
    compptr->width_in_blocks = (Word)
      JPeg70::DivRoundUp((Word32) cinfo->image_width * (Word32) compptr->h_samp_factor,
		    (Word32) (cinfo->max_h_samp_factor * DCTSIZE));
    compptr->height_in_blocks = (Word)
      JPeg70::DivRoundUp((Word32) cinfo->image_height * (Word32) compptr->v_samp_factor,
		    (Word32) (cinfo->max_v_samp_factor * DCTSIZE));
    /* downsampled_width and downsampled_height will also be overridden by
     * jdmaster.c if we are doing full decompression.  The transcoder library
     * doesn't use these values, but the calling application might.
     */
    /* Size in samples */
    compptr->downsampled_width = (Word)
      JPeg70::DivRoundUp((Word32) cinfo->image_width * (Word32) compptr->h_samp_factor,
		    (Word32) cinfo->max_h_samp_factor);
    compptr->downsampled_height = (Word)
      JPeg70::DivRoundUp((Word32) cinfo->image_height * (Word32) compptr->v_samp_factor,
		    (Word32) cinfo->max_v_samp_factor);
    /* Mark component needed, until color conversion says otherwise */
    compptr->component_needed = TRUE;
    /* Mark no quantization table yet saved for component */
    compptr->quant_table = NULL;
  }

  /* Compute number of fully interleaved MCU rows. */
  cinfo->total_iMCU_rows = (Word)
    JPeg70::DivRoundUp((Word32) cinfo->image_height,
		  (Word32) (cinfo->max_v_samp_factor*DCTSIZE));

  /* Decide whether file contains multiple scans */
  if (cinfo->comps_in_scan < cinfo->m_NumComponents || cinfo->progressive_mode)
    cinfo->inputctl->has_multiple_scans = TRUE;
  else
    cinfo->inputctl->has_multiple_scans = FALSE;
}


static void per_scan_setup (CJPegDecompress * cinfo)
/* Do computations that are needed before processing a JPEG scan */
/* cinfo->comps_in_scan and cinfo->cur_comp_info[] were set from SOS marker */
{
  int ci, mcublks, tmp;
  JPeg70::ComponentInfo_t *compptr;
  
  if (cinfo->comps_in_scan == 1) {
    
    /* Noninterleaved (single-component) scan */
    compptr = cinfo->cur_comp_info[0];
    
    /* Overall image size in MCUs */
    cinfo->MCUs_per_row = compptr->width_in_blocks;
    cinfo->MCU_rows_in_scan = compptr->height_in_blocks;
    
    /* For noninterleaved scan, always one block per MCU */
    compptr->MCU_width = 1;
    compptr->MCU_height = 1;
    compptr->MCU_blocks = 1;
    compptr->MCU_sample_width = compptr->DCT_scaled_size;
    compptr->last_col_width = 1;
    /* For noninterleaved scans, it is convenient to define last_row_height
     * as the number of block rows present in the last iMCU row.
     */
    tmp = (int) (compptr->height_in_blocks % compptr->v_samp_factor);
    if (tmp == 0) tmp = compptr->v_samp_factor;
    compptr->last_row_height = tmp;
    
    /* Prepare array describing MCU composition */
    cinfo->blocks_in_MCU = 1;
    cinfo->MCU_membership[0] = 0;
    
  } else {
    
    /* Interleaved (multi-component) scan */
    if (cinfo->comps_in_scan <= 0 || cinfo->comps_in_scan > MAX_COMPS_IN_SCAN)
      cinfo->FatalError(JPeg70::JERR_COMPONENT_COUNT, cinfo->comps_in_scan,
	       MAX_COMPS_IN_SCAN);
    
    /* Overall image size in MCUs */
    cinfo->MCUs_per_row = (Word)
      JPeg70::DivRoundUp((Word32) cinfo->image_width,
		    (Word32) (cinfo->max_h_samp_factor*DCTSIZE));
    cinfo->MCU_rows_in_scan = (Word)
      JPeg70::DivRoundUp((Word32) cinfo->image_height,
		    (Word32) (cinfo->max_v_samp_factor*DCTSIZE));
    
    cinfo->blocks_in_MCU = 0;
    
    for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
      compptr = cinfo->cur_comp_info[ci];
      /* Sampling factors give # of blocks of component in each MCU */
      compptr->MCU_width = compptr->h_samp_factor;
      compptr->MCU_height = compptr->v_samp_factor;
      compptr->MCU_blocks = compptr->MCU_width * compptr->MCU_height;
      compptr->MCU_sample_width = compptr->MCU_width * compptr->DCT_scaled_size;
      /* Figure number of non-dummy blocks in last MCU column & row */
      tmp = (int) (compptr->width_in_blocks % compptr->MCU_width);
      if (tmp == 0) tmp = compptr->MCU_width;
      compptr->last_col_width = tmp;
      tmp = (int) (compptr->height_in_blocks % compptr->MCU_height);
      if (tmp == 0) tmp = compptr->MCU_height;
      compptr->last_row_height = tmp;
      /* Prepare array describing MCU composition */
      mcublks = compptr->MCU_blocks;
      if (cinfo->blocks_in_MCU + mcublks > D_MAX_BLOCKS_IN_MCU)
	cinfo->FatalError(JPeg70::JERR_BAD_MCU_SIZE);
      while (mcublks-- > 0) {
	cinfo->MCU_membership[cinfo->blocks_in_MCU++] = ci;
      }
    }
    
  }
}


/*
 * Save away a copy of the Q-table referenced by each component present
 * in the current scan, unless already saved during a prior scan.
 *
 * In a multiple-scan JPEG file, the encoder could assign different components
 * the same Q-table slot number, but change table definitions between scans
 * so that each component uses a different Q-table.  (The IJG encoder is not
 * currently capable of doing this, but other encoders might.)  Since we want
 * to be able to dequantize all the components at the end of the file, this
 * means that we have to save away the table actually used for each component.
 * We do this by copying the table at the start of the first scan containing
 * the component.
 * The JPEG spec prohibits the encoder from changing the contents of a Q-table
 * slot between scans of a component using that slot.  If the encoder does so
 * anyway, this decoder will simply use the Q-table values that were current
 * at the start of the first scan for the component.
 *
 * The decompressor output side looks only at the saved quant tables,
 * not at the current Q-table slots.
 */

static void latch_quant_tables (CJPegDecompress * cinfo)
{
  int ci, qtblno;
  JPeg70::ComponentInfo_t *compptr;
  JPeg70::QuantTable_t * qtbl;

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    /* No work if we already saved Q-table for this component */
    if (compptr->quant_table != NULL)
      continue;
    /* Make sure specified quantization table is present */
    qtblno = compptr->quant_tbl_no;
    if (qtblno < 0 || qtblno >= NUM_QUANT_TBLS ||
	cinfo->quant_tbl_ptrs[qtblno] == NULL)
      cinfo->FatalError(JPeg70::JERR_NO_QUANT_TABLE, qtblno);
    /* OK, save away the quantization table */
    qtbl = (JPeg70::QuantTable_t *)(*cinfo->mem->alloc_small) ((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE,sizeof(JPeg70::QuantTable_t));
    FastMemCpy(qtbl, cinfo->quant_tbl_ptrs[qtblno], sizeof(JPeg70::QuantTable_t));
    compptr->quant_table = qtbl;
  }
}


/*
 * Initialize the input modules to read a scan of compressed data.
 * The first call to this is done by jdmaster.c after initializing
 * the entire decompressor (during jpeg_start_decompress).
 * Subsequent calls come from consume_markers, below.
 */

static void BURGERCALL start_input_pass (CJPegDecompress * cinfo)
{
  per_scan_setup(cinfo);
  latch_quant_tables(cinfo);
  (*cinfo->entropy->start_pass) (cinfo);
  (*cinfo->coef->start_input_pass) (cinfo);
  cinfo->inputctl->consume_input = cinfo->coef->consume_data;
}


/*
 * Finish up after inputting a compressed-data scan.
 * This is called by the coefficient controller after it's read all
 * the expected data of the scan.
 */

static void BURGERCALL finish_input_pass (CJPegDecompress * cinfo)
{
  cinfo->inputctl->consume_input = consume_markers;
}


/*
 * Read JPEG markers before, between, or after compressed-data scans.
 * Change state as necessary when a new scan is reached.
 * Return value is JPEG_SUSPENDED, JPEG_REACHED_SOS, or JPEG_REACHED_EOI.
 *
 * The consume_input method pointer points either here or to the
 * coefficient controller's consume_data routine, depending on whether
 * we are reading a compressed data segment or inter-segment markers.
 */

static int BURGERCALL consume_markers (CJPegDecompress * cinfo)
{
  my_inputctl_ptr inputctl = (my_inputctl_ptr) cinfo->inputctl;
  int val;

  if (inputctl->pub.eoi_reached) /* After hitting EOI, read no further */
    return JPEG_REACHED_EOI;

  val = (*cinfo->marker->read_markers) (cinfo);

  switch (val) {
  case JPEG_REACHED_SOS:	/* Found SOS */
    if (inputctl->inheaders) {	/* 1st SOS */
      initial_setup(cinfo);
      inputctl->inheaders = FALSE;
      /* Note: start_input_pass must be called by jdmaster.c
       * before any more input can be consumed.  jdapimin.c is
       * responsible for enforcing this sequencing.
       */
    } else {			/* 2nd or later SOS marker */
      if (! inputctl->pub.has_multiple_scans)
	cinfo->FatalError(JPeg70::JERR_EOI_EXPECTED); /* Oops, I wasn't expecting this! */
      start_input_pass(cinfo);
    }
    break;
  case JPEG_REACHED_EOI:	/* Found EOI */
    inputctl->pub.eoi_reached = TRUE;
    if (inputctl->inheaders) {	/* Tables-only datastream, apparently */
      if (cinfo->marker->saw_SOF)
	cinfo->FatalError(JPeg70::JERR_SOF_NO_SOS);
    } else {
      /* Prevent infinite loop in coef ctlr's decompress_data routine
       * if user set output_scan_number larger than number of scans.
       */
      if (cinfo->output_scan_number > cinfo->input_scan_number)
	cinfo->output_scan_number = cinfo->input_scan_number;
    }
    break;
  case JPEG_SUSPENDED:
    break;
  }

  return val;
}


/*
 * Reset state to begin a fresh datastream.
 */

static void BURGERCALL reset_input_controller (CJPegDecompress * cinfo)
{
  my_inputctl_ptr inputctl = (my_inputctl_ptr) cinfo->inputctl;

  inputctl->pub.consume_input = consume_markers;
  inputctl->pub.has_multiple_scans = FALSE; /* "unknown" would be better */
  inputctl->pub.eoi_reached = FALSE;
  inputctl->inheaders = TRUE;
  /* Reset other modules */
  cinfo->ResetErrorManager();
  (*cinfo->marker->reset_marker_reader) (cinfo);
  /* Reset progression state -- would be cleaner if entropy decoder did this */
  cinfo->coef_bits = NULL;
}


/*
 * Initialize the input controller module.
 * This is called only once, when the decompression object is created.
 */

void BURGERCALL jinit_input_controller (CJPegDecompress * cinfo)
{
  my_inputctl_ptr inputctl;

  /* Create subobject in permanent pool */
  inputctl = (my_inputctl_ptr)
    (*cinfo->mem->alloc_small) ((JPeg70::CCommonManager *) cinfo, JPOOL_PERMANENT,
				sizeof(my_input_controller));
  cinfo->inputctl = (struct jpeg_input_controller *) inputctl;
  /* Initialize method pointers */
  inputctl->pub.consume_input = consume_markers;
  inputctl->pub.reset_input_controller = reset_input_controller;
  inputctl->pub.start_input_pass = start_input_pass;
  inputctl->pub.finish_input_pass = finish_input_pass;
  /* Initialize state: can't use reset_input_controller since we don't
   * want to try to reset other modules yet.
   */
  inputctl->pub.has_multiple_scans = FALSE; /* "unknown" would be better */
  inputctl->pub.eoi_reached = FALSE;
  inputctl->inheaders = TRUE;
}
