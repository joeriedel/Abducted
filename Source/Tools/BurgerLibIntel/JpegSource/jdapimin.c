/*
 * jdapimin.c
 *
 * Copyright (C) 1994-1998, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains application interface code for the decompression half
 * of the JPEG library.  These are the "minimum" API routines that may be
 * needed in either the normal full-decompression case or the
 * transcoding-only case.
 *
 * Most of the routines intended to be called directly by an application
 * are in this file or in jdapistd.c.  But also see jcomapi.c for routines
 * shared by compression and decompression, and jdtrans.c for the transcoding
 * case.
 */

#include "JDColor.hpp"
#include "JQuant.hpp"
#define JPEG_INTERNALS
#include "jpeglib.h"


/*
 * Initialization of a JPEG decompression object.
 * The error manager must already be set up (in case memory manager fails).
 */

void BURGERCALL jpeg_CreateDecompress (CJPegDecompress * cinfo, int version, long /* structsize */)
{
  int i;

  /* Guard against version mismatches between library and caller. */
  cinfo->mem = NULL;		/* so jpeg_destroy knows mem mgr not called */
  if (version != JPEG_LIB_VERSION)
    cinfo->FatalError(JPeg70::JERR_BAD_LIB_VERSION, JPEG_LIB_VERSION, version);
//  if (structsize != sizeof(struct jpeg_decompress_struct))
//    cinfo->FatalError(JPeg70::JERR_BAD_STRUCT_SIZE, 
//	     (int) sizeof(struct jpeg_decompress_struct), (int) structsize);

  /* For debugging purposes, we zero the whole master structure.
   * But the application has already set the err pointer, and may have set
   * client_data, so we have to save and restore those fields.
   * Note: if application hasn't set client_data, tools like Purify may
   * complain here.
   */
  {
//    FastMemSet(cinfo,0, sizeof(struct jpeg_decompress_struct));
  }
  cinfo->m_IsDecompressor = TRUE;

  /* Initialize a memory manager instance for this object */
  jinit_memory_mgr((JPeg70::CCommonManager *) cinfo);

  /* Zero out pointers to permanent structures. */
  cinfo->src = NULL;

  for (i = 0; i < NUM_QUANT_TBLS; i++)
    cinfo->quant_tbl_ptrs[i] = NULL;

  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    cinfo->dc_huff_tbl_ptrs[i] = NULL;
    cinfo->ac_huff_tbl_ptrs[i] = NULL;
  }

  /* Initialize marker processor so application can override methods
   * for COM, APPn markers before calling jpeg_read_header.
   */
  cinfo->marker_list = NULL;
  jinit_marker_reader(cinfo);

  /* And initialize the overall input controller. */
  jinit_input_controller(cinfo);

  /* OK, I'm ready */
  cinfo->m_GlobalState = JPeg70::DSTATE_START;
}


/*
 * Destruction of a JPEG decompression object
 */

void BURGERCALL jpeg_destroy_decompress (CJPegDecompress * cinfo)
{
  jpeg_destroy((JPeg70::CCommonManager *) cinfo); /* use common routine */
}


/*
 * Abort processing of a JPEG decompression operation,
 * but don't destroy the object itself.
 */

void BURGERCALL jpeg_abort_decompress (CJPegDecompress * cinfo)
{
  jpeg_abort((JPeg70::CCommonManager *) cinfo); /* use common routine */
}


/*
 * Set default decompression parameters.
 */

static void default_decompress_parms (CJPegDecompress * cinfo)
{
  /* Guess the input colorspace, and set output colorspace accordingly. */
  /* (Wish JPEG committee had provided a real way to specify this...) */
  /* Note application may override our guesses. */
  switch (cinfo->m_NumComponents) {
  case 1:
    cinfo->m_JPegColorSpace = JPeg70::CS_GRAYSCALE;
    cinfo->m_OutputColorSpace = JPeg70::CS_GRAYSCALE;
    break;
    
  case 3:
    if (cinfo->saw_JFIF_marker) {
      cinfo->m_JPegColorSpace = JPeg70::CS_YCBCR; /* JFIF implies YCbCr */
    } else if (cinfo->saw_Adobe_marker) {
      switch (cinfo->Adobe_transform) {
      case 0:
	cinfo->m_JPegColorSpace = JPeg70::CS_RGB;
	break;
      case 1:
	cinfo->m_JPegColorSpace = JPeg70::CS_YCBCR;
	break;
      default:
	cinfo->EmitWarning(JPeg70::JWRN_ADOBE_XFORM, cinfo->Adobe_transform);
	cinfo->m_JPegColorSpace = JPeg70::CS_YCBCR; /* assume it's YCbCr */
	break;
      }
    } else {
      /* Saw no special markers, try to guess from the component IDs */
      int cid0 = cinfo->m_CompInfoPtr[0].component_id;
      int cid1 = cinfo->m_CompInfoPtr[1].component_id;
      int cid2 = cinfo->m_CompInfoPtr[2].component_id;

      if (cid0 == 1 && cid1 == 2 && cid2 == 3)
	cinfo->m_JPegColorSpace = JPeg70::CS_YCBCR; /* assume JFIF w/out marker */
      else if (cid0 == 82 && cid1 == 71 && cid2 == 66)
	cinfo->m_JPegColorSpace = JPeg70::CS_RGB; /* ASCII 'R', 'G', 'B' */
      else {
	cinfo->EmitTrace(1, JPeg70::JTRC_UNKNOWN_IDS, cid0, cid1, cid2);
	cinfo->m_JPegColorSpace = JPeg70::CS_YCBCR; /* assume it's YCbCr */
      }
    }
    /* Always guess RGB is proper output colorspace. */
    cinfo->m_OutputColorSpace = JPeg70::CS_RGB;
    break;
    
  case 4:
    if (cinfo->saw_Adobe_marker) {
      switch (cinfo->Adobe_transform) {
      case 0:
	cinfo->m_JPegColorSpace = JPeg70::CS_CMYK;
	break;
      case 2:
	cinfo->m_JPegColorSpace = JPeg70::CS_YCCK;
	break;
      default:
	cinfo->EmitWarning(JPeg70::JWRN_ADOBE_XFORM, cinfo->Adobe_transform);
	cinfo->m_JPegColorSpace = JPeg70::CS_YCCK; /* assume it's YCCK */
	break;
      }
    } else {
      /* No special markers, assume straight CMYK. */
      cinfo->m_JPegColorSpace = JPeg70::CS_CMYK;
    }
    cinfo->m_OutputColorSpace = JPeg70::CS_CMYK;
    break;
    
  default:
    cinfo->m_JPegColorSpace = JPeg70::CS_UNKNOWN;
    cinfo->m_OutputColorSpace = JPeg70::CS_UNKNOWN;
    break;
  }

  /* Set defaults for other decompression parameters. */
  cinfo->scale_num = 1;		/* 1:1 scaling */
  cinfo->scale_denom = 1;
  cinfo->output_gamma = 1.0;
  cinfo->buffered_image = FALSE;
  cinfo->raw_data_out = FALSE;
  cinfo->dct_method = JPeg70::DCT_ISLOW;
  cinfo->do_fancy_upsampling = TRUE;
  cinfo->do_block_smoothing = TRUE;
  cinfo->m_QuantizeColors = FALSE;
  /* We set these in case application only sets m_QuantizeColors. */
  cinfo->dither_mode = JPeg70::DITHER_FS;
  cinfo->two_pass_quantize = TRUE;
  cinfo->desired_number_of_colors = 256;
  cinfo->colormap = 0;
  /* Initialize for no mode change in buffered-image mode. */
  cinfo->enable_1pass_quant = FALSE;
  cinfo->enable_external_quant = FALSE;
  cinfo->enable_2pass_quant = FALSE;
}


/*
 * Decompression startup: read start of JPEG datastream to see what's there.
 * Need only initialize JPEG object and supply a data source before calling.
 *
 * This routine will read as far as the first SOS marker (ie, actual start of
 * compressed data), and will save all tables and parameters in the JPEG
 * object.  It will also initialize the decompression parameters to default
 * values, and finally return JPEG_HEADER_OK.  On return, the application may
 * adjust the decompression parameters and then call jpeg_start_decompress.
 * (Or, if the application only wanted to determine the image parameters,
 * the data need not be decompressed.  In that case, call jpeg_abort or
 * jpeg_destroy to release any temporary space.)
 * If an abbreviated (tables only) datastream is presented, the routine will
 * return JPEG_HEADER_TABLES_ONLY upon reaching EOI.  The application may then
 * re-use the JPEG object to read the abbreviated image datastream(s).
 * It is unnecessary (but OK) to call jpeg_abort in this case.
 * The JPEG_SUSPENDED return code only occurs if the data source module
 * requests suspension of the decompressor.  In this case the application
 * should load more source data and then re-call jpeg_read_header to resume
 * processing.
 * If a non-suspending data source is used and require_image is TRUE, then the
 * return code need not be inspected since only JPEG_HEADER_OK is possible.
 *
 * This routine is now just a front end to jpeg_consume_input, with some
 * extra error checking.
 */

int jpeg_read_header (CJPegDecompress * cinfo, Word8 require_image)
{
  int retcode;

  if (cinfo->m_GlobalState != JPeg70::DSTATE_START &&
      cinfo->m_GlobalState != JPeg70::DSTATE_INHEADER)
   cinfo->FatalError(JPeg70::JERR_BAD_STATE, cinfo->m_GlobalState);

  retcode = jpeg_consume_input(cinfo);

  switch (retcode) {
  case JPEG_REACHED_SOS:
    retcode = JPEG_HEADER_OK;
    break;
  case JPEG_REACHED_EOI:
    if (require_image)		/* Complain if application wanted an image */
      cinfo->FatalError(JPeg70::JERR_NO_IMAGE);
    /* Reset to start state; it would be safer to require the application to
     * call jpeg_abort, but we can't change it now for compatibility reasons.
     * A side effect is to free any temporary memory (there shouldn't be any).
     */
    jpeg_abort((JPeg70::CCommonManager *) cinfo); /* sets state = DSTATE_START */
    retcode = JPEG_HEADER_TABLES_ONLY;
    break;
  case JPEG_SUSPENDED:
    /* no work */
    break;
  }

  return retcode;
}


/*
 * Consume data in advance of what the decompressor requires.
 * This can be called at any time once the decompressor object has
 * been created and a data source has been set up.
 *
 * This routine is essentially a state machine that handles a couple
 * of critical state-transition actions, namely initial setup and
 * transition from header scanning to ready-for-start_decompress.
 * All the actual input is done via the input controller's consume_input
 * method.
 */

int BURGERCALL jpeg_consume_input (CJPegDecompress * cinfo)
{
  int retcode = JPEG_SUSPENDED;

  /* NB: every possible DSTATE value should be listed in this switch */
  switch (cinfo->m_GlobalState) {
  case JPeg70::DSTATE_START:
    /* Start-of-datastream actions: reset appropriate modules */
    (*cinfo->inputctl->reset_input_controller) (cinfo);
    /* Initialize application's data source module */
    (*cinfo->src->init_source) (cinfo);
    cinfo->m_GlobalState = JPeg70::DSTATE_INHEADER;
    /*FALLTHROUGH*/
  case JPeg70::DSTATE_INHEADER:
    retcode = (*cinfo->inputctl->consume_input) (cinfo);
    if (retcode == JPEG_REACHED_SOS) { /* Found SOS, prepare to decompress */
      /* Set up default parameters based on header data */
      default_decompress_parms(cinfo);
      /* Set global state: ready for start_decompress */
      cinfo->m_GlobalState = JPeg70::DSTATE_READY;
    }
    break;
  case JPeg70::DSTATE_READY:
    /* Can't advance past first SOS until start_decompress is called */
    retcode = JPEG_REACHED_SOS;
    break;
  case JPeg70::DSTATE_PRELOAD:
  case JPeg70::DSTATE_PRESCAN:
  case JPeg70::DSTATE_SCANNING:
  case JPeg70::DSTATE_RAW_OK:
  case JPeg70::DSTATE_BUFIMAGE:
  case JPeg70::DSTATE_BUFPOST:
  case JPeg70::DSTATE_STOPPING:
    retcode = (*cinfo->inputctl->consume_input) (cinfo);
    break;
  default:
    cinfo->FatalError(JPeg70::JERR_BAD_STATE, cinfo->m_GlobalState);
  }
  return retcode;
}


/*
 * Have we finished reading the input file?
 */

Word8 BURGERCALL jpeg_input_complete (CJPegDecompress * cinfo)
{
  /* Check for valid jpeg object */
  if (cinfo->m_GlobalState < JPeg70::DSTATE_START ||
      cinfo->m_GlobalState > JPeg70::DSTATE_STOPPING)
    cinfo->FatalError(JPeg70::JERR_BAD_STATE, cinfo->m_GlobalState);
  return cinfo->inputctl->eoi_reached;
}


/*
 * Is there more than one scan?
 */

Word8 BURGERCALL jpeg_has_multiple_scans (CJPegDecompress * cinfo)
{
  /* Only valid after jpeg_read_header completes */
  if (cinfo->m_GlobalState < JPeg70::DSTATE_READY ||
      cinfo->m_GlobalState > JPeg70::DSTATE_STOPPING)
    cinfo->FatalError(JPeg70::JERR_BAD_STATE, cinfo->m_GlobalState);
  return cinfo->inputctl->has_multiple_scans;
}


/*
 * Finish JPEG decompression.
 *
 * This will normally just verify the file trailer and release temp storage.
 *
 * Returns FALSE if suspended.  The return value need be inspected only if
 * a suspending data source is used.
 */

Word8 BURGERCALL jpeg_finish_decompress (CJPegDecompress * cinfo)
{
  if ((cinfo->m_GlobalState == JPeg70::DSTATE_SCANNING ||
       cinfo->m_GlobalState == JPeg70::DSTATE_RAW_OK) && ! cinfo->buffered_image) {
    /* Terminate final pass of non-buffered mode */
    if (cinfo->output_scanline < cinfo->output_height)
      cinfo->FatalError(JPeg70::JERR_TOO_LITTLE_DATA);
    (*cinfo->master->finish_output_pass) (cinfo);
    cinfo->m_GlobalState = JPeg70::DSTATE_STOPPING;
  } else if (cinfo->m_GlobalState == JPeg70::DSTATE_BUFIMAGE) {
    /* Finishing after a buffered-image operation */
    cinfo->m_GlobalState = JPeg70::DSTATE_STOPPING;
  } else if (cinfo->m_GlobalState != JPeg70::DSTATE_STOPPING) {
    /* STOPPING = repeat call after a suspension, anything else is error */
    cinfo->FatalError(JPeg70::JERR_BAD_STATE, cinfo->m_GlobalState);
  }
  /* Read until EOI */
  while (! cinfo->inputctl->eoi_reached) {
    if ((*cinfo->inputctl->consume_input) (cinfo) == JPEG_SUSPENDED)
      return FALSE;		/* Suspend, come back later */
  }
  /* Do final cleanup */
  (*cinfo->src->term_source) (cinfo);
  /* We can use jpeg_abort to release memory and reset global_state */
  jpeg_abort((JPeg70::CCommonManager *) cinfo);
  return TRUE;
}

CJPegDecompress::CJPegDecompress()
	: m_ColorDeconverterPtr(0),
	m_CQuantizePtr(0)
{
}

CJPegDecompress::~CJPegDecompress()
{
	if (m_ColorDeconverterPtr) {
		delete m_ColorDeconverterPtr;
	}
	if (m_CQuantizePtr) {
		delete m_CQuantizePtr;
	}
}

