/*
 * jcmainct.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the main buffer controller for compression.
 * The main buffer lies between the pre-processor and the JPEG
 * compressor proper; it holds downsampled data in the JPEG colorspace.
 */

#define JPEG_INTERNALS
#include "jpeglib.h"

/* Private buffer controller object */

typedef struct {
  struct jpeg_c_main_controller pub; /* public fields */

  Word cur_iMCU_row;	/* number of current iMCU row */
  Word rowgroup_ctr;	/* counts row groups received in iMCU row */
  Word8 suspended;		/* remember if we suspended output */
  J_BUF_MODE pass_mode;		/* current operating mode */

  /* If using just a strip buffer, this points to the entire set of buffers
   * (we allocate one for each component).  In the full-image case, this
   * points to the currently accessible strips of the virtual arrays.
   */
  JSAMPLE * * buffer[MAX_COMPONENTS];
} my_main_controller;

typedef my_main_controller * my_main_ptr;


/* Forward declarations */
static void BURGERCALL process_data_simple_main(CJPegCompress * cinfo, JSAMPLE * * input_buf,Word *in_row_ctr, Word in_rows_avail);

/*
 * Initialize for a processing pass.
 */

static void BURGERCALL start_pass_main (CJPegCompress * cinfo, J_BUF_MODE pass_mode)
{
  my_main_ptr main = (my_main_ptr) cinfo->main;

  /* Do nothing in raw-data mode. */
  if (cinfo->raw_data_in)
    return;

  main->cur_iMCU_row = 0;	/* initialize counters */
  main->rowgroup_ctr = 0;
  main->suspended = FALSE;
  main->pass_mode = pass_mode;	/* save mode for use by process_data */

  switch (pass_mode) {
  case JBUF_PASS_THRU:
    main->pub.process_data = process_data_simple_main;
    break;
  default:
    cinfo->FatalError(JPeg70::JERR_BAD_BUFFER_MODE);
    break;
  }
}


/*
 * Process some data.
 * This routine handles the simple pass-through mode,
 * where we have only a strip buffer.
 */

static void BURGERCALL process_data_simple_main (CJPegCompress * cinfo,
			  JSAMPLE * * input_buf, Word *in_row_ctr,
			  Word in_rows_avail)
{
  my_main_ptr main = (my_main_ptr) cinfo->main;

  while (main->cur_iMCU_row < cinfo->total_iMCU_rows) {
    /* Read input data if we haven't filled the main buffer yet */
    if (main->rowgroup_ctr < DCTSIZE)
      (*cinfo->prep->pre_process_data) (cinfo,
					input_buf, in_row_ctr, in_rows_avail,
					main->buffer, &main->rowgroup_ctr,
					(Word) DCTSIZE);

    /* If we don't have a full iMCU row buffered, return to application for
     * more data.  Note that preprocessor will always pad to fill the iMCU row
     * at the bottom of the image.
     */
    if (main->rowgroup_ctr != DCTSIZE)
      return;

    /* Send the completed row to the compressor */
    if (! (*cinfo->coef->compress_data) (cinfo, main->buffer)) {
      /* If compressor did not consume the whole row, then we must need to
       * suspend processing and return to the application.  In this situation
       * we pretend we didn't yet consume the last input row; otherwise, if
       * it happened to be the last row of the image, the application would
       * think we were done.
       */
      if (! main->suspended) {
	(*in_row_ctr)--;
	main->suspended = TRUE;
      }
      return;
    }
    /* We did finish the row.  Undo our little suspension hack if a previous
     * call suspended; then mark the main buffer empty.
     */
    if (main->suspended) {
      (*in_row_ctr)++;
      main->suspended = FALSE;
    }
    main->rowgroup_ctr = 0;
    main->cur_iMCU_row++;
  }
}


/*
 * Initialize main buffer controller.
 */

void BURGERCALL jinit_c_main_controller (CJPegCompress * cinfo, Word8 need_full_buffer)
{
  my_main_ptr main;
  int ci;
  JPeg70::ComponentInfo_t *compptr;

  main = (my_main_ptr)
    (*cinfo->mem->alloc_small) ((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE,
				sizeof(my_main_controller));
  cinfo->main = (struct jpeg_c_main_controller *) main;
  main->pub.start_pass = start_pass_main;

  /* We don't need to create a buffer in raw-data mode. */
  if (cinfo->raw_data_in)
    return;

  /* Create the buffer.  It holds downsampled data, so each component
   * may be of a different size.
   */
  if (need_full_buffer) {
    cinfo->FatalError(JPeg70::JERR_BAD_BUFFER_MODE);
  } else {
    /* Allocate a strip buffer for each component */
    for (ci = 0, compptr = cinfo->m_CompInfoPtr; ci < cinfo->m_NumComponents;
	 ci++, compptr++) {
      main->buffer[ci] = (*cinfo->mem->alloc_sarray)((JPeg70::CCommonManager *) cinfo, JPOOL_IMAGE,compptr->width_in_blocks * DCTSIZE,(Word) (compptr->v_samp_factor * DCTSIZE));
    }
  }
}
