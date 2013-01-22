/*
 * jdpostct.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the decompression postprocessing controller.
 * This controller manages the upsampling, color conversion, and color
 * quantization/reduction steps; specifically, it controls the buffering
 * between upsample/color conversion and color quantization/reduction.
 *
 * If no color quantization/reduction is required, then this module has no
 * work to do, and it just hands off to the upsample/color conversion code.
 * An integrated upsample/convert/quantize process would replace this module
 * entirely.
 */

#include "JQuant.hpp"
#define JPEG_INTERNALS
#include "jpeglib.h"
#include "JUtils.hpp"

/* Private buffer controller object */

typedef struct {
  struct jpeg_d_post_controller pub; /* public fields */

  /* Color quantization source buffer: this holds output data from
   * the upsample/color conversion step to be passed to the quantizer.
   * For two-pass color quantization, we need a full-image buffer;
   * for one-pass operation, a strip buffer is sufficient.
   */
  struct jvirt_sarray_control * whole_image;	/* virtual array, or NULL if one-pass */
  JSAMPLE ** buffer;		/* strip buffer, or current strip of virtual */
  Word strip_height;	/* buffer size in rows */
  /* for two-pass mode only: */
  Word starting_row;	/* row # of first row in current strip */
  Word next_row;		/* index of next row to fill/empty in strip */
} my_post_controller;

typedef my_post_controller * my_post_ptr;


/* Forward declarations */
static void BURGERCALL post_process_1pass(CJPegDecompress * cinfo,JSAMPLE * * * input_buf, Word *in_row_group_ctr,Word in_row_groups_avail,JSAMPLE * * output_buf, Word *out_row_ctr,Word out_rows_avail);
static void BURGERCALL post_process_prepass(CJPegDecompress * cinfo,JSAMPLE * * * input_buf, Word *in_row_group_ctr,Word in_row_groups_avail,JSAMPLE * * output_buf, Word *out_row_ctr,Word out_rows_avail);
static void BURGERCALL post_process_2pass(CJPegDecompress * cinfo,JSAMPLE * * * input_buf, Word *in_row_group_ctr,Word in_row_groups_avail,JSAMPLE * * output_buf, Word *out_row_ctr,Word out_rows_avail);

/*
 * Initialize for a processing pass.
 */

static void BURGERCALL start_pass_dpost (CJPegDecompress * cinfo, J_BUF_MODE pass_mode)
{
  my_post_ptr post = (my_post_ptr) cinfo->post;

  switch (pass_mode) {
  case JBUF_PASS_THRU:
    if (cinfo->m_QuantizeColors) {
      /* Single-pass processing with color quantization. */
      post->pub.post_process_data = post_process_1pass;
      /* We could be doing buffered-image output before starting a 2-pass
       * color quantization; in that case, jinit_d_post_controller did not
       * allocate a strip buffer.  Use the virtual-array buffer as workspace.
       */
      if (post->buffer == NULL) {
	post->buffer = (*cinfo->mem->access_virt_sarray)
	  ((JPeg70::CCommonManager *) cinfo, post->whole_image,
	   (Word) 0, post->strip_height, TRUE);
      }
    } else {
      /* For single-pass processing without color quantization,
       * I have no work to do; just call the upsampler directly.
       */
      post->pub.post_process_data = cinfo->upsample->upsample;
    }
    break;
  case JBUF_SAVE_AND_PASS:
    /* First pass of 2-pass quantization */
    if (post->whole_image == NULL)
      cinfo->FatalError(JPeg70::JERR_BAD_BUFFER_MODE);
    post->pub.post_process_data = post_process_prepass;
    break;
  case JBUF_CRANK_DEST:
    /* Second pass of 2-pass quantization */
    if (post->whole_image == NULL)
     cinfo->FatalError(JPeg70::JERR_BAD_BUFFER_MODE);
    post->pub.post_process_data = post_process_2pass;
    break;
  default:
    cinfo->FatalError(JPeg70::JERR_BAD_BUFFER_MODE);
    break;
  }
  post->starting_row = post->next_row = 0;
}


/*
 * Process some data in the one-pass (strip buffer) case.
 * This is used for color precision reduction as well as one-pass quantization.
 */

static void BURGERCALL post_process_1pass (CJPegDecompress * cinfo,
		    JSAMPLE * * * input_buf, Word *in_row_group_ctr,
		    Word in_row_groups_avail,
		    JSAMPLE * * output_buf, Word *out_row_ctr,
		    Word out_rows_avail)
{
  my_post_ptr post = (my_post_ptr) cinfo->post;
  Word num_rows, max_rows;

  /* Fill the buffer, but not more than what we can dump out in one go. */
  /* Note we rely on the upsampler to detect bottom of image. */
  max_rows = out_rows_avail - *out_row_ctr;
  if (max_rows > post->strip_height)
    max_rows = post->strip_height;
  num_rows = 0;
  (*cinfo->upsample->upsample) (cinfo,
		input_buf, in_row_group_ctr, in_row_groups_avail,
		post->buffer, &num_rows, max_rows);
  /* Quantize and emit data. */
  cinfo->m_CQuantizePtr->ColorQuantize(post->buffer, output_buf + *out_row_ctr, (int) num_rows);
  *out_row_ctr += num_rows;
}


/*
 * Process some data in the first pass of 2-pass quantization.
 */

static void BURGERCALL post_process_prepass (CJPegDecompress * cinfo,
		      JSAMPLE * * * input_buf, Word *in_row_group_ctr,
		      Word in_row_groups_avail,
		      JSAMPLE * * /* output_buf */, Word *out_row_ctr,
		      Word /* out_rows_avail */)
{
  my_post_ptr post = (my_post_ptr) cinfo->post;
  Word old_next_row, num_rows;

  /* Reposition virtual buffer if at start of strip. */
  if (post->next_row == 0) {
    post->buffer = (*cinfo->mem->access_virt_sarray)
	((JPeg70::CCommonManager *) cinfo, post->whole_image,
	 post->starting_row, post->strip_height, TRUE);
  }

  /* Upsample some data (up to a strip height's worth). */
  old_next_row = post->next_row;
  (*cinfo->upsample->upsample) (cinfo,
		input_buf, in_row_group_ctr, in_row_groups_avail,
		post->buffer, &post->next_row, post->strip_height);

  /* Allow quantizer to scan new data.  No data is emitted, */
  /* but we advance out_row_ctr so outer loop can tell when we're done. */
  if (post->next_row > old_next_row) {
    num_rows = post->next_row - old_next_row;
    cinfo->m_CQuantizePtr->ColorQuantize(post->buffer + old_next_row,(JSAMPLE * *) NULL, (int) num_rows);
    *out_row_ctr += num_rows;
  }

  /* Advance if we filled the strip. */
  if (post->next_row >= post->strip_height) {
    post->starting_row += post->strip_height;
    post->next_row = 0;
  }
}


/*
 * Process some data in the second pass of 2-pass quantization.
 */

static void BURGERCALL post_process_2pass (CJPegDecompress * cinfo,
		    JSAMPLE * * * /* input_buf */, Word * /*in_row_group_ctr */,
		    Word /* in_row_groups_avail */,
		    JSAMPLE ** output_buf, Word *out_row_ctr,
		    Word out_rows_avail)
{
  my_post_ptr post = (my_post_ptr) cinfo->post;
  Word num_rows, max_rows;

  /* Reposition virtual buffer if at start of strip. */
  if (post->next_row == 0) {
    post->buffer = (*cinfo->mem->access_virt_sarray)
	((JPeg70::CCommonManager *) cinfo, post->whole_image,
	 post->starting_row, post->strip_height, FALSE);
  }

  /* Determine number of rows to emit. */
  num_rows = post->strip_height - post->next_row; /* available in strip */
  max_rows = out_rows_avail - *out_row_ctr; /* available in output area */
  if (num_rows > max_rows)
    num_rows = max_rows;
  /* We have to check bottom of image here, can't depend on upsampler. */
  max_rows = cinfo->output_height - post->starting_row;
  if (num_rows > max_rows)
    num_rows = max_rows;

  /* Quantize and emit data. */
  cinfo->m_CQuantizePtr->ColorQuantize(post->buffer + post->next_row, output_buf + *out_row_ctr,(int) num_rows);
  *out_row_ctr += num_rows;

  /* Advance if we filled the strip. */
  post->next_row += num_rows;
  if (post->next_row >= post->strip_height) {
    post->starting_row += post->strip_height;
    post->next_row = 0;
  }
}

/*
 * Initialize postprocessing controller.
 */

void BURGERCALL jinit_d_post_controller (CJPegDecompress * cinfo, Word8 need_full_buffer)
{
  my_post_ptr post;

  post = (my_post_ptr)(*cinfo->mem->alloc_small) ((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE,sizeof(my_post_controller));
  cinfo->post = (struct jpeg_d_post_controller *) post;
  post->pub.start_pass = start_pass_dpost;
  post->whole_image = NULL;	/* flag for no virtual arrays */
  post->buffer = NULL;		/* flag for no strip buffer */

  /* Create the quantization buffer, if needed */
  if (cinfo->m_QuantizeColors) {
    /* The buffer strip height is max_v_samp_factor, which is typically
     * an efficient number of rows for upsampling to return.
     * (In the presence of output rescaling, we might want to be smarter?)
     */
    post->strip_height = (Word) cinfo->max_v_samp_factor;
    if (need_full_buffer) {
      /* Two-pass color quantization: need full-image storage. */
      /* We round up the number of rows to a multiple of the strip height. */
      post->whole_image = (*cinfo->mem->request_virt_sarray)((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE, FALSE,
	 cinfo->output_width * cinfo->m_ColorComponents,
	 (Word) JPeg70::RoundUp((Word32) cinfo->output_height,(Word32) post->strip_height), post->strip_height);
    } else {
      /* One-pass color quantization: just make a strip buffer. */
      post->buffer = (*cinfo->mem->alloc_sarray)((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE,cinfo->output_width * cinfo->m_ColorComponents,post->strip_height);
    }
  }
}
