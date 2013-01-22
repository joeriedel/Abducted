/*
 * jpegint.h
 *
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file provides common declarations for the various JPEG modules.
 * These declarations are considered internal to the JPEG library; most
 * applications using the library shouldn't need to include this file.
 */

#ifndef __JPEGINT_H__
#define __JPEGINT_H__

#ifndef __BURGER__
#include <Burger.h>
#endif

/* Declarations for both compression & decompression */

typedef enum {			/* Operating modes for buffer controllers */
	JBUF_PASS_THRU,		/* Plain stripwise operation */
	/* Remaining modes require a full-image buffer to have been created */
	JBUF_SAVE_SOURCE,	/* Run source subobject only, save output */
	JBUF_CRANK_DEST,	/* Run dest subobject only, using saved data */
	JBUF_SAVE_AND_PASS	/* Run both subobjects, save output */
} J_BUF_MODE;

/* Master control module */
struct jpeg_comp_master {
	void(BURGERCALL *prepare_for_pass)(CJPegCompress * cinfo);
	void(BURGERCALL *pass_startup)(CJPegCompress * cinfo);
	void(BURGERCALL *finish_pass)(CJPegCompress * cinfo);

	/* State variables made visible to other modules */
	Word8 call_pass_startup;	/* True if pass_startup must be called */
	Word8 is_last_pass;		/* True during last pass */
	Word8 Padding[2];
};

/* Main buffer control (downsampled-data buffer) */
struct jpeg_c_main_controller {
  void(BURGERCALL *start_pass)(CJPegCompress * cinfo, J_BUF_MODE pass_mode);
  void(BURGERCALL *process_data)(CJPegCompress * cinfo,
			       JSAMPLE * * input_buf, Word *in_row_ctr,
			       Word in_rows_avail);
};

/* Compression preprocessing (downsampling input buffer control) */
struct jpeg_c_prep_controller {
  void(BURGERCALL *start_pass)(CJPegCompress * cinfo, J_BUF_MODE pass_mode);
  void(BURGERCALL *pre_process_data)(CJPegCompress * cinfo,
				   JSAMPLE * * input_buf,
				   Word *in_row_ctr,
				   Word in_rows_avail,
				   JSAMPLE * * * output_buf,
				   Word *out_row_group_ctr,
				   Word out_row_groups_avail);
};

/* Coefficient buffer control */
struct jpeg_c_coef_controller {
  void(BURGERCALL *start_pass)(CJPegCompress * cinfo, J_BUF_MODE pass_mode);
  Word8(BURGERCALL *compress_data)(CJPegCompress * cinfo,JSAMPLE * * * input_buf);
};

/* Colorspace conversion */
struct jpeg_color_converter {
  void(BURGERCALL *start_pass)(CJPegCompress * cinfo);
  void(BURGERCALL *color_convert)(CJPegCompress * cinfo,
				JSAMPLE * * input_buf, JSAMPLE * * * output_buf,
				Word output_row, int num_rows);
};

/* Downsampling */
struct jpeg_downsampler {
	void(BURGERCALL *start_pass)(CJPegCompress * cinfo);
	void(BURGERCALL *downsample)(CJPegCompress * cinfo,
		JSAMPLE * * * input_buf, Word in_row_index,
		JSAMPLE * * * output_buf,
		Word out_row_group_index);

	Word8 need_context_rows;	/* TRUE if need rows above & below */
	Word8 Padding[3];
};

/* Forward DCT (also controls coefficient quantization) */
struct jpeg_forward_dct {
  void(BURGERCALL *start_pass)(CJPegCompress * cinfo);
  /* perhaps this should be an array??? */
  void(BURGERCALL *forward_DCT)(CJPegCompress * cinfo,
			      JPeg70::ComponentInfo_t * compptr,
			      JSAMPLE * * sample_data, JPeg70::Block_t * coef_blocks,
			      Word start_row, Word start_col,
			      Word num_blocks);
};

/* Entropy encoding */
struct jpeg_entropy_encoder {
  void(BURGERCALL *start_pass)(CJPegCompress * cinfo, Word8 gather_statistics);
  Word8(BURGERCALL *encode_mcu)(CJPegCompress * cinfo, JPeg70::Block_t * *MCU_data);
  void(BURGERCALL *finish_pass)(CJPegCompress * cinfo);
};

/* Marker writing */
struct jpeg_marker_writer {
  void(BURGERCALL *write_file_header)(CJPegCompress * cinfo);
  void(BURGERCALL *write_frame_header)(CJPegCompress * cinfo);
  void(BURGERCALL *write_scan_header)(CJPegCompress * cinfo);
  void(BURGERCALL *write_file_trailer)(CJPegCompress * cinfo);
  void(BURGERCALL *write_tables_only)(CJPegCompress * cinfo);
  /* These routines are exported to allow insertion of extra markers */
  /* Probably only COM and APPn markers should be written this way */
  void(BURGERCALL *write_marker_header)(CJPegCompress * cinfo, int marker,
				      unsigned int datalen);
  void(BURGERCALL *write_marker_byte)(CJPegCompress * cinfo, int val);
};


/* Declarations for decompression modules */

/* Master control module */
struct jpeg_decomp_master {
  void(BURGERCALL *prepare_for_output_pass)(CJPegDecompress * cinfo);
  void(BURGERCALL *finish_output_pass)(CJPegDecompress * cinfo);

  /* State variables made visible to other modules */
  Word8 is_dummy_pass;	/* True during 1st pass for 2-pass quant */
  Word8 Padding[3];
};

/* Input control module */
struct jpeg_input_controller {
	int(BURGERCALL *consume_input)(CJPegDecompress * cinfo);
	void(BURGERCALL *reset_input_controller)(CJPegDecompress * cinfo);
	void(BURGERCALL *start_input_pass)(CJPegDecompress * cinfo);
	void(BURGERCALL *finish_input_pass)(CJPegDecompress * cinfo);

	/* State variables made visible to other modules */
	Word8 has_multiple_scans;	/* True if file has multiple scans */
	Word8 eoi_reached;		/* True when EOI has been consumed */\
	Word8 Padding[2];
};

/* Main buffer control (downsampled-data buffer) */
struct jpeg_d_main_controller {
  void(BURGERCALL *start_pass)(CJPegDecompress * cinfo, J_BUF_MODE pass_mode);
  void(BURGERCALL *process_data)(CJPegDecompress * cinfo,
			       JSAMPLE * * output_buf, Word *out_row_ctr,
			       Word out_rows_avail);
};

/* Coefficient buffer control */
struct jpeg_d_coef_controller {
	void(BURGERCALL *start_input_pass)(CJPegDecompress * cinfo);
	int(BURGERCALL *consume_data)(CJPegDecompress * cinfo);
	void(BURGERCALL *start_output_pass)(CJPegDecompress * cinfo);
	int(BURGERCALL *decompress_data)(CJPegDecompress * cinfo,JSAMPLE * * * output_buf);
	/* Pointer to array of coefficient virtual arrays, or NULL if none */
	struct jvirt_barray_control * *coef_arrays;
};

/* Decompression postprocessing (color quantization buffer control) */
struct jpeg_d_post_controller {
  void(BURGERCALL *start_pass)(CJPegDecompress * cinfo, J_BUF_MODE pass_mode);
  void(BURGERCALL *post_process_data)(CJPegDecompress * cinfo,
				    JSAMPLE * * * input_buf,
				    Word *in_row_group_ctr,
				    Word in_row_groups_avail,
				    JSAMPLE * * output_buf,
				    Word *out_row_ctr,
				    Word out_rows_avail);
};

/* Marker reading & parsing */
struct jpeg_marker_reader {
  void(BURGERCALL *reset_marker_reader)(CJPegDecompress * cinfo);
  /* Read markers until SOS or EOI.
   * Returns same codes as are defined for jpeg_consume_input:
   * JPEG_SUSPENDED, JPEG_REACHED_SOS, or JPEG_REACHED_EOI.
   */
  int (BURGERCALL *read_markers)(CJPegDecompress * cinfo);
  /* Read a restart marker --- exported for use by entropy decoder only */
  jpeg_marker_parser_method read_restart_marker;

  /* State of marker reader --- nominally internal, but applications
   * supplying COM or APPn handlers might like to know the state.
   */
  Word8 saw_SOI;		/* found SOI? */
  Word8 saw_SOF;		/* found SOF? */
  Word8 Padding[2];
  int next_restart_num;		/* next restart number expected (0-7) */
  unsigned int discarded_bytes;	/* # of bytes skipped looking for a marker */
};

/* Entropy decoding */
struct jpeg_entropy_decoder {
	void(BURGERCALL *start_pass)(CJPegDecompress * cinfo);
	Word8(BURGERCALL *decode_mcu)(CJPegDecompress * cinfo,JPeg70::Block_t * *MCU_data);

	/* This is here to share code between baseline and progressive decoders; */
	/* other modules probably should not use it */
	Word8 insufficient_data;	/* set TRUE after emitting warning */
	Word8 Padding[3];
};

/* Inverse DCT (also performs dequantization) */
typedef void(BURGERCALL *inverse_DCT_method_ptr)
		(CJPegDecompress * cinfo, JPeg70::ComponentInfo_t * compptr,
		 SWord16 * coef_block,
		 JSAMPLE * * output_buf, Word output_col);

struct jpeg_inverse_dct {
  void(BURGERCALL *start_pass)(CJPegDecompress * cinfo);
  /* It is useful to allow each component to have a separate IDCT method. */
  inverse_DCT_method_ptr inverse_DCT[MAX_COMPONENTS];
};

/* Upsampling (note that upsampler must also call color converter) */
struct jpeg_upsampler {
  void(BURGERCALL *start_pass)(CJPegDecompress * cinfo);
  void(BURGERCALL *upsample)(CJPegDecompress * cinfo,
			   JSAMPLE * * * input_buf,
			   Word *in_row_group_ctr,
			   Word in_row_groups_avail,
			   JSAMPLE * * output_buf,
			   Word *out_row_ctr,
			   Word out_rows_avail);

  Word8 need_context_rows;	/* TRUE if need rows above & below */
};


/* Miscellaneous useful macros */

#undef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#undef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))

/* Compression module initialization routines */
extern void BURGERCALL jinit_compress_master (CJPegCompress * cinfo);
extern void BURGERCALL jinit_c_master_control(CJPegCompress * cinfo,Word8 transcode_only);
extern void BURGERCALL jinit_c_main_controller (CJPegCompress * cinfo,Word8 need_full_buffer);
extern void BURGERCALL jinit_c_prep_controller (CJPegCompress * cinfo,Word8 need_full_buffer);
extern void BURGERCALL jinit_c_coef_controller (CJPegCompress * cinfo,Word8 need_full_buffer);
extern void BURGERCALL jinit_color_converter (CJPegCompress * cinfo);
extern void BURGERCALL jinit_downsampler (CJPegCompress * cinfo);
extern void BURGERCALL jinit_forward_dct (CJPegCompress * cinfo);
extern void BURGERCALL jinit_huff_encoder (CJPegCompress * cinfo);
extern void BURGERCALL jinit_phuff_encoder (CJPegCompress * cinfo);
extern void BURGERCALL jinit_marker_writer (CJPegCompress * cinfo);
/* Decompression module initialization routines */
extern void BURGERCALL jinit_master_decompress (CJPegDecompress * cinfo);
extern void BURGERCALL jinit_d_main_controller (CJPegDecompress * cinfo,Word8 need_full_buffer);
extern void BURGERCALL jinit_d_coef_controller (CJPegDecompress * cinfo,Word8 need_full_buffer);
extern void BURGERCALL jinit_d_post_controller (CJPegDecompress * cinfo,Word8 need_full_buffer);
extern void BURGERCALL jinit_input_controller (CJPegDecompress * cinfo);
extern void BURGERCALL jinit_marker_reader (CJPegDecompress * cinfo);
extern void BURGERCALL jinit_huff_decoder (CJPegDecompress * cinfo);
extern void BURGERCALL jinit_phuff_decoder (CJPegDecompress * cinfo);
extern void BURGERCALL jinit_inverse_dct (CJPegDecompress * cinfo);
extern void BURGERCALL jinit_upsampler (CJPegDecompress * cinfo);
extern void BURGERCALL jinit_merged_upsampler (CJPegDecompress * cinfo);
/* Memory manager initialization */
extern void jinit_memory_mgr (JPeg70::CCommonManager * cinfo);

#endif

