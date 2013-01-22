/*
 * jpeglib.h
 *
 * Copyright (C) 1991-1998, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file defines the application interface for the JPEG library.
 * Most applications using the library need only include this file,
 * and perhaps jerror.h if they want to know the exact error codes.
 */

#ifndef __JPEGLIB_H__
#define __JPEGLIB_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __JPEG70TYPES_H__
#include "JPeg70Types.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

namespace JPeg70 {
class CColorDeconverter;
class CColorQuantizer;
}
/* Master record for a compression instance */

class CJPegCompress : public JPeg70::CCommonManager {
public:
	CJPegCompress();
	~CJPegCompress();
	double input_gamma;		/* image gamma of input image */

	/* Destination for compressed data */
	struct jpeg_destination_mgr * dest;

	/* Description of source image --- these fields must be filled in by
	* outer application before starting compression.  in_color_space must
	* be correct before you can even call jpeg_set_defaults().
	*/

	Word image_width;	/* input image width */
	Word image_height;	/* input image height */
	int input_components;		/* # of color components in input image */
	JPeg70::ColorSpace_e in_color_space;	/* colorspace of input image */

	/* Compression parameters --- these fields must be set before calling
	* jpeg_start_compress().  We recommend calling jpeg_set_defaults() to
	* initialize everything to reasonable defaults, then changing anything
	* the application specifically wants to change.  That way you won't get
	* burnt when new parameters are added.  Also note that there are several
	* helper routines to simplify changing parameters.
	*/

	int data_precision;		/* bits of precision in image data */

	int m_NumComponents;		/* # of color components in JPEG image */
	JPeg70::ColorSpace_e m_JPegColorSpace; /* colorspace of JPEG image */

	JPeg70::ComponentInfo_t * m_CompInfoPtr;
	/* m_CompInfoPtr[i] describes component that appears i'th in SOF */

	JPeg70::QuantTable_t * quant_tbl_ptrs[NUM_QUANT_TBLS];
	/* ptrs to coefficient quantization tables, or NULL if not defined */

	JPeg70::HuffTable_t * dc_huff_tbl_ptrs[NUM_HUFF_TBLS];
	JPeg70::HuffTable_t * ac_huff_tbl_ptrs[NUM_HUFF_TBLS];
	/* ptrs to Huffman coding tables, or NULL if not defined */

	Word8 arith_dc_L[NUM_ARITH_TBLS]; /* L values for DC arith-coding tables */
	Word8 arith_dc_U[NUM_ARITH_TBLS]; /* U values for DC arith-coding tables */
	Word8 arith_ac_K[NUM_ARITH_TBLS]; /* Kx values for AC arith-coding tables */

	int num_scans;		/* # of entries in scan_info array */
	const JPeg70::ScanInfo_t * scan_info; /* script for multi-scan file, or NULL */
	/* The default value of scan_info is NULL, which causes a single-scan
	* sequential JPEG file to be emitted.  To create a multi-scan file,
	* set num_scans and scan_info to point to an array of scan definitions.
	*/

	Word8 raw_data_in;		/* TRUE=caller supplies downsampled data */
	Word8 arith_code;		/* TRUE=arithmetic coding, FALSE=Huffman */
	Word8 optimize_coding;	/* TRUE=optimize entropy encoding parms */
	Word8 CCIR601_sampling;	/* TRUE=first samples are cosited */
	int smoothing_factor;		/* 1..100, or 0 for no input smoothing */
	JPeg70::DCTMethod_e dct_method;	/* DCT algorithm selector */

	/* The restart interval can be specified in absolute MCUs by setting
	* restart_interval, or in MCU rows by setting restart_in_rows
	* (in which case the correct restart_interval will be figured
	* for each scan).
	*/
	Word restart_interval;		/* MCUs per restart, or 0 for no restart */
	int restart_in_rows;		/* if > 0, MCU rows per restart interval */

	/* Parameters controlling emission of special markers. */

	Word8 write_JFIF_header;	/* should a JFIF marker be written? */
	Word8 JFIF_major_version;	/* What to write for the JFIF version number */
	Word8 JFIF_minor_version;
	/* These three values are not used by the JPEG code, merely copied */
	/* into the JFIF APP0 marker.  density_unit can be 0 for unknown, */
	/* 1 for dots/inch, or 2 for dots/cm.  Note that the pixel aspect */
	/* ratio is defined by X_density/Y_density even when density_unit=0. */
	Word8 density_unit;		/* JFIF code for pixel size units */
	Word16 X_density;		/* Horizontal pixel density */
	Word16 Y_density;		/* Vertical pixel density */

	/* State variable: index of next scanline to be written to
	* jpeg_write_scanlines().  Application may use this to control its
	* processing loop, e.g., "while (next_scanline < image_height)".
	*/

	Word next_scanline;	/* 0 .. image_height-1  */

	Word8 write_Adobe_marker;	/* should an Adobe marker be written? */

	/* Remaining fields are known throughout compressor, but generally
	* should not be touched by a surrounding application.
	*/

	/*
	* These fields are computed during compression startup
	*/
	Word8 progressive_mode;	/* TRUE if scan script uses progressive mode */
	Word8 Padding1[2];		/* Align to long */
	int max_h_samp_factor;	/* largest h_samp_factor */
	int max_v_samp_factor;	/* largest v_samp_factor */

	Word total_iMCU_rows;	/* # of iMCU rows to be input to coef ctlr */
	/* The coefficient controller receives data in units of MCU rows as defined
	* for fully interleaved scans (whether the JPEG file is interleaved or not).
	* There are v_samp_factor * DCTSIZE sample rows of each component in an
	* "iMCU" (interleaved MCU) row.
	*/

	/*
	* These fields are valid during any one scan.
	* They describe the components and MCUs actually appearing in the scan.
	*/
	int comps_in_scan;		/* # of JPEG components in this scan */
	JPeg70::ComponentInfo_t * cur_comp_info[MAX_COMPS_IN_SCAN];
	/* *cur_comp_info[i] describes component that appears i'th in SOS */

	Word MCUs_per_row;	/* # of MCUs across the image */
	Word MCU_rows_in_scan;	/* # of MCU rows in the image */

	int blocks_in_MCU;		/* # of DCT blocks per MCU */
	int MCU_membership[C_MAX_BLOCKS_IN_MCU];
	/* MCU_membership[i] is index in cur_comp_info of component owning */
	/* i'th block in an MCU */

	int Ss, Se, Ah, Al;		/* progressive JPEG parameters for scan */

	/*
	* Links to compression subobjects (methods and private variables of modules)
	*/
	struct jpeg_comp_master * master;
	struct jpeg_c_main_controller * main;
	struct jpeg_c_prep_controller * prep;
	struct jpeg_c_coef_controller * coef;
	struct jpeg_marker_writer * marker;
	struct jpeg_color_converter * cconvert;
	struct jpeg_downsampler * downsample;
	struct jpeg_forward_dct * fdct;
	struct jpeg_entropy_encoder * entropy;
	JPeg70::ScanInfo_t * script_space; /* workspace for jpeg_simple_progression */
	int script_space_size;
};

/* Master record for a decompression instance */

class CJPegDecompress : public JPeg70::CCommonManager {
public:
	CJPegDecompress();
	virtual ~CJPegDecompress();
	inline JSample_t *IDCTRangeLimit(void) { return sample_range_limit + CENTERJSAMPLE;}
	double output_gamma;		/* image gamma wanted in output */
	
	/* Source of compressed data */
	struct jpeg_source_mgr * src;

	/* Basic description of image --- filled in by jpeg_read_header(). */
	/* Application may inspect these values to decide how to process image. */

	Word image_width;	/* nominal image width (from SOF marker) */
	Word image_height;	/* nominal image height */
	int m_NumComponents;		/* # of color components in JPEG image */
	JPeg70::ColorSpace_e m_JPegColorSpace; /* colorspace of JPEG image */

	/* Decompression processing parameters --- these fields must be set before
	* calling jpeg_start_decompress().  Note that jpeg_read_header() initializes
	* them to default values.
	*/

	JPeg70::ColorSpace_e m_OutputColorSpace; /* colorspace for output */

	Word scale_num, scale_denom; /* fraction by which to scale image */

	Word8 buffered_image;	/* TRUE=multiple output passes */
	Word8 raw_data_out;		/* TRUE=downsampled data wanted */
	Word8 do_fancy_upsampling;	/* TRUE=apply fancy upsampling */
	Word8 do_block_smoothing;	/* TRUE=apply interblock smoothing */

	JPeg70::DCTMethod_e dct_method;	/* IDCT algorithm selector */

	/* the following are ignored if not m_QuantizeColors: */
	JPeg70::DitherMode_e dither_mode;	/* type of color dithering to use */
	int desired_number_of_colors;	/* max # colors to use in created colormap */

	Word8 two_pass_quantize;	/* TRUE=use two-pass color quantization */
	Word8 m_QuantizeColors;		/* TRUE=colormapped output wanted */
	/* these are significant only in buffered-image mode: */
	Word8 enable_1pass_quant;	/* enable future use of 1-pass quantizer */
	Word8 enable_external_quant;/* enable future use of external colormap */

	/* Description of actual output image that will be returned to application.
	* These fields are computed by jpeg_start_decompress().
	* You can also use jpeg_calc_output_dimensions() to determine these values
	* in advance of calling jpeg_start_decompress().
	*/

	Word output_width;	/* scaled image width */
	Word output_height;	/* scaled image height */
	int m_ColorComponents;	/* # of color components in m_OutputColorSpace */
	int m_OutputComponents;	/* # of color components returned */
	/* m_OutputComponents is 1 (a colormap index) when quantizing colors;
	* otherwise it equals m_ColorComponents.
	*/
	int rec_outbuf_height;	/* min recommended height of scanline buffer */
	/* If the buffer passed to jpeg_read_scanlines() is less than this many rows
	* high, space and time will be wasted due to unnecessary data copying.
	* Usually rec_outbuf_height will be 1 or 2, at most 4.
	*/

	/* When quantizing colors, the output colormap is described by these fields.
	* The application can supply a colormap by setting colormap non-NULL before
	* calling jpeg_start_decompress; otherwise a colormap is created during
	* jpeg_start_decompress or jpeg_start_output.
	* The map has m_ColorComponents rows and actual_number_of_colors columns.
	*/
	Word m_ActualNumberOfColors;	/* number of entries in use */
	JSAMPLE **colormap;		/* The color map as a 2-D pixel array (Externally managed) */

	/* State variables: these variables indicate the progress of decompression.
	* The application may examine these but must not modify them.
	*/

	/* Row index of next scanline to be read from jpeg_read_scanlines().
	* Application may use this to control its processing loop, e.g.,
	* "while (output_scanline < output_height)".
	*/
	Word output_scanline;	/* 0 .. output_height-1  */

	/* Current input scan number and number of iMCU rows completed in scan.
	* These indicate the progress of the decompressor input side.
	*/
	int input_scan_number;	/* Number of SOS markers seen so far */
	Word input_iMCU_row;	/* Number of iMCU rows completed */

	/* The "output scan number" is the notional scan being displayed by the
	* output side.  The decompressor will not allow output scan/row number
	* to get ahead of input scan/row, but it can fall arbitrarily far behind.
	*/
	int output_scan_number;	/* Nominal scan number being displayed */
	Word output_iMCU_row;	/* Number of iMCU rows read */

	/* Current progression status.  coef_bits[c][i] indicates the precision
	* with which component c's DCT coefficient i (in zigzag order) is known.
	* It is -1 when no data has yet been received, otherwise it is the point
	* transform (shift) value for the most recent scan of the coefficient
	* (thus, 0 at completion of the progression).
	* This pointer is NULL when reading a non-progressive file.
	*/
	int (*coef_bits)[DCTSIZE2];	/* -1 or current Al value for each coef */

	/* Internal JPEG parameters --- the application usually need not look at
	* these fields.  Note that the decompressor output side may not use
	* any parameters that can change between scans.
	*/

	/* Quantization and Huffman tables are carried forward across input
	* datastreams when processing abbreviated JPEG datastreams.
	*/

	JPeg70::QuantTable_t * quant_tbl_ptrs[NUM_QUANT_TBLS];
	/* ptrs to coefficient quantization tables, or NULL if not defined */

	JPeg70::HuffTable_t * dc_huff_tbl_ptrs[NUM_HUFF_TBLS];
	JPeg70::HuffTable_t * ac_huff_tbl_ptrs[NUM_HUFF_TBLS];
	/* ptrs to Huffman coding tables, or NULL if not defined */

	/* These parameters are never carried across datastreams, since they
	* are given in SOF/SOS markers or defined to be reset by SOI.
	*/

	int data_precision;		/* bits of precision in image data */

	JPeg70::ComponentInfo_t * m_CompInfoPtr;
	/* m_CompInfoPtr[i] describes component that appears i'th in SOF */

	Word8 enable_2pass_quant;	/* enable future use of 2-pass quantizer */
	Word8 progressive_mode;	/* TRUE if SOFn specifies progressive mode */
	Word8 arith_code;		/* TRUE=arithmetic coding, FALSE=Huffman */
	Word8 saw_JFIF_marker;	/* TRUE iff a JFIF APP0 marker was found */

	Word8 arith_dc_L[NUM_ARITH_TBLS]; /* L values for DC arith-coding tables */
	Word8 arith_dc_U[NUM_ARITH_TBLS]; /* U values for DC arith-coding tables */
	Word8 arith_ac_K[NUM_ARITH_TBLS]; /* Kx values for AC arith-coding tables */

	Word restart_interval; /* MCUs per restart interval, or 0 for no restart */

	/* These fields record data obtained from optional markers recognized by
	* the JPEG library.
	*/
	/* Data copied from JFIF marker; only valid if saw_JFIF_marker is TRUE: */
	Word8 JFIF_major_version;	/* JFIF version number */
	Word8 JFIF_minor_version;
	Word8 density_unit;		/* JFIF code for pixel size units */
	Word8 saw_Adobe_marker;	/* TRUE iff an Adobe APP14 marker was found */

	Word16 X_density;		/* Horizontal pixel density */
	Word16 Y_density;		/* Vertical pixel density */
	
	Word8 Adobe_transform;	/* Color transform code from Adobe marker */
	Word8 CCIR601_sampling;	/* TRUE=first samples are cosited */
	Word8 Padding1[2];
	
	/* Aside from the specific data retained from APPn markers known to the
	* library, the uninterpreted contents of any or all APPn and COM markers
	* can be saved in a list for examination by the application.
	*/
	JPeg70::Marker_t * marker_list; /* Head of list of saved markers */

	/* Remaining fields are known throughout decompressor, but generally
	* should not be touched by a surrounding application.
	*/

	/*
	* These fields are computed during decompression startup
	*/
	int max_h_samp_factor;	/* largest h_samp_factor */
	int max_v_samp_factor;	/* largest v_samp_factor */

	int min_DCT_scaled_size;	/* smallest DCT_scaled_size of any component */

	Word total_iMCU_rows;	/* # of iMCU rows in image */
	/* The coefficient controller's input and output progress is measured in
	* units of "iMCU" (interleaved MCU) rows.  These are the same as MCU rows
	* in fully interleaved JPEG scans, but are used whether the scan is
	* interleaved or not.  We define an iMCU row as v_samp_factor DCT block
	* rows of each component.  Therefore, the IDCT output contains
	* v_samp_factor*DCT_scaled_size sample rows of a component per iMCU row.
	*/

	JSAMPLE * sample_range_limit; /* table for fast range-limiting */

	/*
	* These fields are valid during any one scan.
	* They describe the components and MCUs actually appearing in the scan.
	* Note that the decompressor output side must not use these fields.
	*/
	int comps_in_scan;		/* # of JPEG components in this scan */
	JPeg70::ComponentInfo_t * cur_comp_info[MAX_COMPS_IN_SCAN];
	/* *cur_comp_info[i] describes component that appears i'th in SOS */

	Word MCUs_per_row;	/* # of MCUs across the image */
	Word MCU_rows_in_scan;	/* # of MCU rows in the image */

	int blocks_in_MCU;		/* # of DCT blocks per MCU */
	int MCU_membership[D_MAX_BLOCKS_IN_MCU];
	/* MCU_membership[i] is index in cur_comp_info of component owning */
	/* i'th block in an MCU */

	int Ss, Se, Ah, Al;		/* progressive JPEG parameters for scan */

	/* This field is shared between entropy decoder and marker parser.
	* It is either zero or the code of a JPEG marker that has been
	* read from the data source, but has not yet been processed.
	*/
	int unread_marker;

	/*
	* Links to decompression subobjects (methods, private variables of modules)
	*/
	struct jpeg_decomp_master * master;
	struct jpeg_d_main_controller * main;
	struct jpeg_d_coef_controller * coef;
	struct jpeg_d_post_controller * post;
	struct jpeg_input_controller * inputctl;
	struct jpeg_marker_reader * marker;
	struct jpeg_entropy_decoder * entropy;
	struct jpeg_inverse_dct * idct;
	struct jpeg_upsampler * upsample;
	JPeg70::CColorDeconverter *m_ColorDeconverterPtr;
	JPeg70::CColorQuantizer *m_CQuantizePtr;
};


/* "Object" declarations for JPEG modules that may be supplied or called
 * directly by the surrounding application.
 * As with all objects in the JPEG library, these structs only define the
 * publicly visible methods and state variables of a module.  Additional
 * private fields may exist after the public ones.
 */


/* Data destination object for compression */

typedef struct jpeg_destination_mgr {
	Word8 * next_output_byte;	/* => next byte to write in buffer */
	long free_in_buffer;		/* # of byte spaces remaining in buffer */

	void(BURGERCALL *init_destination)(CJPegCompress * cinfo);
	Word8(BURGERCALL *empty_output_buffer)(CJPegCompress * cinfo);
	void(BURGERCALL *term_destination)(CJPegCompress * cinfo);
} jpeg_destination_mgr;

/* Data source object for decompression */

typedef struct jpeg_source_mgr {
	const Word8 * next_input_byte; /* => next byte to read from buffer */
	long bytes_in_buffer;	/* # of bytes remaining in buffer */

	void(BURGERCALL *init_source)(CJPegDecompress * cinfo);
	Word8(BURGERCALL *fill_input_buffer)(CJPegDecompress * cinfo);
	void(BURGERCALL *skip_input_data)(CJPegDecompress * cinfo, long num_bytes);
	Word8(BURGERCALL *resync_to_restart)(CJPegDecompress * cinfo, int desired);
	void(BURGERCALL *term_source)(CJPegDecompress * cinfo);
} jpeg_source_mgr;


/* Memory manager object.
 * Allocates "small" objects (a few K total), "large" objects (tens of K),
 * and "really big" objects (virtual arrays with backing store if needed).
 * The memory manager does not allow individual objects to be freed; rather,
 * each created object is assigned to a pool, and whole pools can be freed
 * at once.  This is faster and more convenient than remembering exactly what
 * to free, especially where malloc()/free() are not too speedy.
 * NB: alloc routines never return NULL.  They exit to error_exit if not
 * successful.
 */

#define JPOOL_PERMANENT	0	/* lasts until master record is destroyed */
#define JPOOL_IMAGE	1	/* lasts until done with image/datastream */
#define JPOOL_NUMPOOLS	2

typedef struct jpeg_memory_mgr {
	/* Method pointers */
	void *(BURGERCALL *alloc_small)(JPeg70::CCommonManager * cinfo, int pool_id,long sizeofobject);
	void *(BURGERCALL *alloc_large)(JPeg70::CCommonManager * cinfo, int pool_id,long sizeofobject);
	JSAMPLE * *(BURGERCALL *alloc_sarray)(JPeg70::CCommonManager * cinfo, int pool_id,Word samplesperrow,Word numrows);
	struct jvirt_sarray_control *(BURGERCALL *request_virt_sarray)(JPeg70::CCommonManager * cinfo,int pool_id,Word8 pre_zero,Word samplesperrow,Word numrows,Word maxaccess);
	struct jvirt_barray_control * (BURGERCALL *request_virt_barray)(JPeg70::CCommonManager * cinfo,int pool_id,Word8 pre_zero,Word blocksperrow,Word numrows,Word maxaccess);
	void (BURGERCALL *realize_virt_arrays)(JPeg70::CCommonManager * cinfo);
	JSAMPLE * *(BURGERCALL *access_virt_sarray)(JPeg70::CCommonManager * cinfo,struct jvirt_sarray_control * ptr,Word start_row,Word num_rows,Word8 writable);
	JPeg70::Block_t * *(BURGERCALL *access_virt_barray)(JPeg70::CCommonManager * cinfo,struct jvirt_barray_control * ptr,Word start_row,Word num_rows,Word8 writable);
	void(BURGERCALL *free_pool)(JPeg70::CCommonManager * cinfo, int pool_id);
	void(BURGERCALL *self_destruct)(JPeg70::CCommonManager * cinfo);
} jpeg_memory_mgr;


/* Routine signature for application-supplied marker processing methods.
 * Need not pass marker code since it is stored in cinfo->unread_marker.
 */
typedef Word8 (BURGERCALL *jpeg_marker_parser_method)(CJPegDecompress * cinfo);

/* Word16 forms of external names for systems with brain-damaged linkers.
 * We shorten external names to be unique in the first six letters, which
 * is good enough for all known systems.
 * (If your compiler itself needs names to be unique in less than 15 
 * characters, you are out of luck.  Get a better compiler.)
 */

/* Initialization of JPEG compression objects.
 * jpeg_create_compress() and jpeg_create_decompress() are the exported
 * names that applications should call.  These expand to calls on
 * jpeg_CreateCompress and jpeg_CreateDecompress with additional information
 * passed for version mismatch checking.
 * NB: you must set up the error-manager BEFORE calling jpeg_create_xxx.
 */
#define jpeg_create_compress(cinfo) \
    jpeg_CreateCompress((cinfo), JPEG_LIB_VERSION,(long) sizeof(CJPegCompress))
#define jpeg_create_decompress(cinfo) \
    jpeg_CreateDecompress((cinfo), JPEG_LIB_VERSION,(long) sizeof(CJPegDecompress))
extern void BURGERCALL jpeg_CreateCompress (CJPegCompress * cinfo,int version, long structsize);
extern void BURGERCALL jpeg_CreateDecompress (CJPegDecompress * cinfo,int version, long structsize);

/* Destruction of JPEG compression objects */
extern void BURGERCALL jpeg_destroy_compress (CJPegCompress * cinfo);
extern void BURGERCALL jpeg_destroy_decompress (CJPegDecompress * cinfo);

/* Default parameter setup for compression */
extern void BURGERCALL jpeg_set_defaults(CJPegCompress * cinfo);
/* Compression parameter setup aids */
extern void BURGERCALL jpeg_set_colorspace(CJPegCompress * cinfo,JPeg70::ColorSpace_e colorspace);
extern void BURGERCALL jpeg_default_colorspace (CJPegCompress * cinfo);
extern void BURGERCALL jpeg_set_quality (CJPegCompress * cinfo, int quality,Word8 force_baseline);
extern void BURGERCALL jpeg_set_linear_quality (CJPegCompress * cinfo,int scale_factor,Word8 force_baseline);
extern void BURGERCALL jpeg_add_quant_table (CJPegCompress * cinfo, int which_tbl,const Word *basic_table,int scale_factor,Word8 force_baseline);
extern int BURGERCALL jpeg_quality_scaling (int quality);
extern void BURGERCALL jpeg_simple_progression (CJPegCompress * cinfo);
extern void BURGERCALL jpeg_suppress_tables (CJPegCompress * cinfo,Word8 suppress);
extern JPeg70::QuantTable_t * BURGERCALL jpeg_alloc_quant_table(JPeg70::CCommonManager * cinfo);
extern JPeg70::HuffTable_t * BURGERCALL jpeg_alloc_huff_table (JPeg70::CCommonManager * cinfo);

/* Main entry points for compression */
extern void BURGERCALL jpeg_start_compress (CJPegCompress * cinfo,Word8 write_all_tables);
extern Word BURGERCALL jpeg_write_scanlines (CJPegCompress * cinfo,JSAMPLE * * scanlines,Word num_lines);
extern void BURGERCALL jpeg_finish_compress(CJPegCompress * cinfo);

/* Replaces jpeg_write_scanlines when writing raw downsampled data. */
extern Word BURGERCALL jpeg_write_raw_data(CJPegCompress * cinfo,JSAMPLE * * * data,Word num_lines);

/* Write a special marker.  See libjpeg.doc concerning safe usage. */
extern void BURGERCALL jpeg_write_marker(CJPegCompress * cinfo, int marker,const Word8 * dataptr, Word datalen);
/* Same, but piecemeal. */
extern void BURGERCALL jpeg_write_m_header(CJPegCompress * cinfo, int marker, Word datalen);
extern void BURGERCALL jpeg_write_m_byte(CJPegCompress * cinfo, int val);

/* Alternate compression function: just write an abbreviated table file */
extern void BURGERCALL jpeg_write_tables(CJPegCompress * cinfo);

/* Decompression startup: read start of JPEG datastream to see what's there */
extern int jpeg_read_header(CJPegDecompress * cinfo,Word8 require_image);
/* Return value is one of: */
#define JPEG_SUSPENDED		0 /* Suspended due to lack of input data */
#define JPEG_HEADER_OK		1 /* Found valid image datastream */
#define JPEG_HEADER_TABLES_ONLY	2 /* Found valid table-specs-only datastream */
/* If you pass require_image = TRUE (normal case), you need not check for
 * a TABLES_ONLY return code; an abbreviated file will cause an error exit.
 * JPEG_SUSPENDED is only possible if you use a data source module that can
 * give a suspension return (the stdio source module doesn't).
 */

/* Main entry points for decompression */
extern Word8 BURGERCALL jpeg_start_decompress(CJPegDecompress * cinfo);
extern Word BURGERCALL jpeg_read_scanlines(CJPegDecompress * cinfo,JSAMPLE * * scanlines,Word max_lines);
extern Word8 BURGERCALL jpeg_finish_decompress (CJPegDecompress * cinfo);

/* Replaces jpeg_read_scanlines when reading raw downsampled data. */
extern Word BURGERCALL jpeg_read_raw_data (CJPegDecompress * cinfo,JSAMPLE * * * * data,Word max_lines);

/* Additional entry points for buffered-image mode. */
extern Word8 BURGERCALL jpeg_has_multiple_scans (CJPegDecompress * cinfo);
extern Word8 BURGERCALL jpeg_start_output (CJPegDecompress * cinfo,int scan_number);
extern Word8 BURGERCALL jpeg_finish_output (CJPegDecompress * cinfo);
extern Word8 BURGERCALL jpeg_input_complete (CJPegDecompress * cinfo);
extern void BURGERCALL jpeg_new_colormap (CJPegDecompress * cinfo);
extern int BURGERCALL jpeg_consume_input (CJPegDecompress * cinfo);
/* Return value is one of: */
/* #define JPEG_SUSPENDED	0    Suspended due to lack of input data */
#define JPEG_REACHED_SOS	1 /* Reached start of new scan */
#define JPEG_REACHED_EOI	2 /* Reached end of image */
#define JPEG_ROW_COMPLETED	3 /* Completed one iMCU row */
#define JPEG_SCAN_COMPLETED	4 /* Completed last iMCU row of a scan */

/* Precalculate output dimensions for current decompression parameters. */
extern void BURGERCALL jpeg_calc_output_dimensions (CJPegDecompress * cinfo);

/* Control saving of COM and APPn markers into marker_list. */
extern void BURGERCALL jpeg_save_markers(CJPegDecompress * cinfo, int marker_code,unsigned int length_limit);

/* Install a special processing method for COM or APPn markers. */
extern void BURGERCALL jpeg_set_marker_processor(CJPegDecompress * cinfo, int marker_code,jpeg_marker_parser_method routine);

/* Read or write raw DCT coefficients --- useful for lossless transcoding. */
extern struct jvirt_barray_control * * BURGERCALL jpeg_read_coefficients(CJPegDecompress * cinfo);
extern void BURGERCALL jpeg_write_coefficients(CJPegCompress * cinfo,struct jvirt_barray_control * * coef_arrays);
extern void BURGERCALL jpeg_copy_critical_parameters (CJPegDecompress * srcinfo,CJPegCompress * dstinfo);

/* If you choose to abort compression or decompression before completing
 * jpeg_finish_(de)compress, then you need to clean up to release memory,
 * temporary files, etc.  You can just call jpeg_destroy_(de)compress
 * if you're done with the JPEG object, but if you want to clean it up and
 * reuse it, call this:
 */
extern void BURGERCALL jpeg_abort_compress (CJPegCompress * cinfo);
extern void BURGERCALL jpeg_abort_decompress (CJPegDecompress * cinfo);

/* Generic versions of jpeg_abort and jpeg_destroy that work on either
 * flavor of JPEG object.  These may be more convenient in some places.
 */
extern void BURGERCALL jpeg_abort (JPeg70::CCommonManager * cinfo);
extern void BURGERCALL jpeg_destroy(JPeg70::CCommonManager * cinfo);

/* Default restart-marker-resync procedure for use by data source modules */
extern Word8 BURGERCALL jpeg_resync_to_restart (CJPegDecompress * cinfo,int desired);

#ifdef __cplusplus
}
#endif

#ifdef JPEG_INTERNALS
#ifndef __JPEGINT_H__
#include "jpegint.h"		/* fetch private declarations */
#endif

#endif

#endif /* JPEGLIB_H */
