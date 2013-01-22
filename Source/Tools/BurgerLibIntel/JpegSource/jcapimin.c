/*
 * jcapimin.c
 *
 * Copyright (C) 1994-1998, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains application interface code for the compression half
 * of the JPEG library.  These are the "minimum" API routines that may be
 * needed in either the normal full-compression case or the transcoding-only
 * case.
 *
 * Most of the routines intended to be called directly by an application
 * are in this file or in jcapistd.c.  But also see jcparam.c for
 * parameter-setup helper routines, jcomapi.c for routines shared by
 * compression and decompression, and jctrans.c for the transcoding case.
 */

#define JPEG_INTERNALS
#include "jpeglib.h"


/*
 * Initialization of a JPEG compression object.
 * The error manager must already be set up (in case memory manager fails).
 */

void BURGERCALL jpeg_CreateCompress (CJPegCompress * cinfo, int version, long /* structsize */)
{
  int i;

  /* Guard against version mismatches between library and caller. */
  cinfo->mem = NULL;		/* so jpeg_destroy knows mem mgr not called */
  if (version != JPEG_LIB_VERSION)
    cinfo->FatalError(JPeg70::JERR_BAD_LIB_VERSION, JPEG_LIB_VERSION, version);
//  if (structsize != sizeof(CJPegCompress))
//  cinfo->FatalError(JPeg70::JERR_BAD_STRUCT_SIZE, 
//	     (int) sizeof(CJPegCompress), (int) structsize);

  /* For debugging purposes, we zero the whole master structure.
   * But the application has already set the err pointer, and may have set
   * client_data, so we have to save and restore those fields.
   * Note: if application hasn't set client_data, tools like Purify may
   * complain here.
   */
  {
//    FastMemSet(cinfo,0, sizeof(struct jpeg_compress_struct));
  }
  cinfo->m_IsDecompressor = FALSE;

  /* Initialize a memory manager instance for this object */
  jinit_memory_mgr((JPeg70::CCommonManager *) cinfo);

  /* Zero out pointers to permanent structures. */
  cinfo->dest = NULL;

  cinfo->m_CompInfoPtr = NULL;

  for (i = 0; i < NUM_QUANT_TBLS; i++)
    cinfo->quant_tbl_ptrs[i] = NULL;

  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    cinfo->dc_huff_tbl_ptrs[i] = NULL;
    cinfo->ac_huff_tbl_ptrs[i] = NULL;
  }

  cinfo->script_space = NULL;

  cinfo->input_gamma = 1.0;	/* in case application forgets */

  /* OK, I'm ready */
  cinfo->m_GlobalState = JPeg70::CSTATE_START;
}


/*
 * Destruction of a JPEG compression object
 */

void BURGERCALL jpeg_destroy_compress (CJPegCompress * cinfo)
{
  jpeg_destroy((JPeg70::CCommonManager *) cinfo); /* use common routine */
}


/*
 * Abort processing of a JPEG compression operation,
 * but don't destroy the object itself.
 */

void BURGERCALL jpeg_abort_compress (CJPegCompress * cinfo)
{
  jpeg_abort((JPeg70::CCommonManager *) cinfo); /* use common routine */
}


/*
 * Forcibly suppress or un-suppress all quantization and Huffman tables.
 * Marks all currently defined tables as already written (if suppress)
 * or not written (if !suppress).  This will control whether they get emitted
 * by a subsequent jpeg_start_compress call.
 *
 * This routine is exported for use by applications that want to produce
 * abbreviated JPEG datastreams.  It logically belongs in jcparam.c, but
 * since it is called by jpeg_start_compress, we put it here --- otherwise
 * jcparam.o would be linked whether the application used it or not.
 */

void BURGERCALL jpeg_suppress_tables (CJPegCompress * cinfo, Word8 suppress)
{
  int i;
  JPeg70::QuantTable_t * qtbl;
  JPeg70::HuffTable_t * htbl;

  for (i = 0; i < NUM_QUANT_TBLS; i++) {
    if ((qtbl = cinfo->quant_tbl_ptrs[i]) != NULL)
      qtbl->sent_table = suppress;
  }

  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    if ((htbl = cinfo->dc_huff_tbl_ptrs[i]) != NULL)
      htbl->sent_table = suppress;
    if ((htbl = cinfo->ac_huff_tbl_ptrs[i]) != NULL)
      htbl->sent_table = suppress;
  }
}


/*
 * Finish JPEG compression.
 *
 * If a multipass operating mode was selected, this may do a great deal of
 * work including most of the actual output.
 */

void BURGERCALL jpeg_finish_compress (CJPegCompress * cinfo)
{
  Word iMCU_row;

  if (cinfo->m_GlobalState == JPeg70::CSTATE_SCANNING ||
      cinfo->m_GlobalState == JPeg70::CSTATE_RAW_OK) {
    /* Terminate first pass */
    if (cinfo->next_scanline < cinfo->image_height)
      cinfo->FatalError(JPeg70::JERR_TOO_LITTLE_DATA);
    (*cinfo->master->finish_pass) (cinfo);
  } else if (cinfo->m_GlobalState != JPeg70::CSTATE_WRCOEFS)
    cinfo->FatalError(JPeg70::JERR_BAD_STATE, cinfo->m_GlobalState);
  /* Perform any remaining passes */
  while (! cinfo->master->is_last_pass) {
    (*cinfo->master->prepare_for_pass) (cinfo);
    for (iMCU_row = 0; iMCU_row < cinfo->total_iMCU_rows; iMCU_row++) {
     
	cinfo->m_PassCounter = iMCU_row;
	cinfo->m_PassLimit = cinfo->total_iMCU_rows;
	cinfo->ShowProgress();
      
      /* We bypass the main controller and invoke coef controller directly;
       * all work is being done from the coefficient buffer.
       */
      if (! (*cinfo->coef->compress_data) (cinfo, (JSAMPLE * * *) NULL))
	cinfo->FatalError(JPeg70::JERR_CANT_SUSPEND);
    }
    (*cinfo->master->finish_pass) (cinfo);
  }
  /* Write EOI, do final cleanup */
  (*cinfo->marker->write_file_trailer) (cinfo);
  (*cinfo->dest->term_destination) (cinfo);
  /* We can use jpeg_abort to release memory and reset global_state */
  jpeg_abort((JPeg70::CCommonManager *) cinfo);
}


/*
 * Write a special marker.
 * This is only recommended for writing COM or APPn markers.
 * Must be called after jpeg_start_compress() and before
 * first call to jpeg_write_scanlines() or jpeg_write_raw_data().
 */

void BURGERCALL jpeg_write_marker (CJPegCompress * cinfo, int marker,
		   const Word8 *dataptr, unsigned int datalen)
{
  void(BURGERCALL *write_marker_byte)(CJPegCompress * info, int val);

  if (cinfo->next_scanline != 0 ||
      (cinfo->m_GlobalState != JPeg70::CSTATE_SCANNING &&
       cinfo->m_GlobalState != JPeg70::CSTATE_RAW_OK &&
       cinfo->m_GlobalState != JPeg70::CSTATE_WRCOEFS))
    cinfo->FatalError(JPeg70::JERR_BAD_STATE, cinfo->m_GlobalState);

  (*cinfo->marker->write_marker_header) (cinfo, marker, datalen);
  write_marker_byte = cinfo->marker->write_marker_byte;	/* copy for speed */
  while (datalen--) {
    (*write_marker_byte) (cinfo, *dataptr);
    dataptr++;
  }
}

/* Same, but piecemeal. */

void BURGERCALL jpeg_write_m_header (CJPegCompress * cinfo, int marker, unsigned int datalen)
{
  if (cinfo->next_scanline != 0 ||
      (cinfo->m_GlobalState != JPeg70::CSTATE_SCANNING &&
       cinfo->m_GlobalState != JPeg70::CSTATE_RAW_OK &&
       cinfo->m_GlobalState != JPeg70::CSTATE_WRCOEFS))
    cinfo->FatalError(JPeg70::JERR_BAD_STATE, cinfo->m_GlobalState);

  (*cinfo->marker->write_marker_header) (cinfo, marker, datalen);
}

void BURGERCALL jpeg_write_m_byte (CJPegCompress * cinfo, int val)
{
  (*cinfo->marker->write_marker_byte) (cinfo, val);
}


/*
 * Alternate compression function: just write an abbreviated table file.
 * Before calling this, all parameters and a data destination must be set up.
 *
 * To produce a pair of files containing abbreviated tables and abbreviated
 * image data, one would proceed as follows:
 *
 *		initialize JPEG object
 *		set JPEG parameters
 *		set destination to table file
 *		jpeg_write_tables(cinfo);
 *		set destination to image file
 *		jpeg_start_compress(cinfo, FALSE);
 *		write data...
 *		jpeg_finish_compress(cinfo);
 *
 * jpeg_write_tables has the side effect of marking all tables written
 * (same as jpeg_suppress_tables(..., TRUE)).  Thus a subsequent start_compress
 * will not re-emit the tables unless it is passed write_all_tables=TRUE.
 */

void BURGERCALL jpeg_write_tables (CJPegCompress * cinfo)
{
  if (cinfo->m_GlobalState != JPeg70::CSTATE_START)
    cinfo->FatalError(JPeg70::JERR_BAD_STATE, cinfo->m_GlobalState);

  /* (Re)initialize error mgr and destination modules */
  cinfo->ResetErrorManager();
  (*cinfo->dest->init_destination) (cinfo);
  /* Initialize the marker writer ... bit of a crock to do it here. */
  jinit_marker_writer(cinfo);
  /* Write them tables! */
  (*cinfo->marker->write_tables_only) (cinfo);
  /* And clean up. */
  (*cinfo->dest->term_destination) (cinfo);
  /*
   * In library releases up through v6a, we called jpeg_abort() here to free
   * any working memory allocated by the destination manager and marker
   * writer.  Some applications had a problem with that: they allocated space
   * of their own from the library memory manager, and didn't want it to go
   * away during write_tables.  So now we do nothing.  This will cause a
   * memory leak if an app calls write_tables repeatedly without doing a full
   * compression cycle or otherwise resetting the JPEG object.  However, that
   * seems less bad than unexpectedly freeing memory in the normal case.
   * An app that prefers the old behavior can call jpeg_abort for itself after
   * each call to jpeg_write_tables().
   */
}

CJPegCompress::CJPegCompress()
{
}

CJPegCompress::~CJPegCompress()
{
}

