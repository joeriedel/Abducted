/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#ifndef __JPEG70TYPES_H__
#define __JPEG70TYPES_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __cplusplus
#error Requires C++ compilation
#endif

struct jpeg_memory_mgr;

#define JPEG_LIB_VERSION 70		/* Version 7 */
#define DCTSIZE 8				/* The basic DCT block is 8x8 samples */
#define DCTSIZE2 64				/* DCTSIZE squared; # of elements in a block */
#define NUM_QUANT_TBLS 4		/* Quantization tables are numbered 0..3 */
#define NUM_HUFF_TBLS 4			/* Huffman tables are numbered 0..3 */
#define NUM_ARITH_TBLS 16		/* Arith-coding tables are numbered 0..15 */
#define MAX_COMPS_IN_SCAN 4		/* JPEG limit on # of components in one scan */
#define MAX_SAMP_FACTOR 4		/* JPEG limit on sampling factors */
#define C_MAX_BLOCKS_IN_MCU 10	/* compressor's limit on blocks per MCU */
#define D_MAX_BLOCKS_IN_MCU 10	/* decompressor's limit on blocks per MCU */
#define MAX_COMPONENTS 10		/* maximum number of image components */
#define JPEG_MAX_DIMENSION 65500L  /* a tad under 64K to prevent overflows */
#define BITS_IN_JSAMPLE 8		/* use 8 or 12 */

#if BITS_IN_JSAMPLE == 8
typedef Word8 JSAMPLE;
typedef Word8 JSample_t;
#define GETJSAMPLE(value) ((int) (value))
#define MAXJSAMPLE	255
#define CENTERJSAMPLE	128
#endif
#if BITS_IN_JSAMPLE == 12
typedef Word16 JSAMPLE;
typedef Word16 JSample_t;
#define GETJSAMPLE(value)  ((int) (value))
#define MAXJSAMPLE	4095
#define CENTERJSAMPLE	2048
#endif

#define RANGE_MASK (MAXJSAMPLE * 4 + 3) /* 2 bits wider than legal samples */

/* Don't remove */

#define C_ARITH_CODING_SUPPORTED    /* Arithmetic coding back end? */
#define D_ARITH_CODING_SUPPORTED    /* Arithmetic coding back end? */
#define UPSAMPLE_SCALING_SUPPORTED  /* Output rescaling at upsample stage? */

/* JPeg Marker codes */

#define JPEG_RST0 0xD0	/* RST0 marker code */
#define JPEG_EOI 0xD9	/* EOI marker code */
#define JPEG_APP0 0xE0	/* APP0 marker code */
#define JPEG_COM 0xFE	/* COM marker code */

namespace JPeg70 {

/* Known color spaces. */

typedef enum {
	RGB_RED,		/* Red index */
	RGB_GREEN,		/* Green index */
	RGB_BLUE,		/* Blue index */
	RGB_PIXELSIZE=3	/* Bytes per 24 bits per pixel */
} RGBColors_e;

typedef enum {
	CS_UNKNOWN,			/* error/unspecified */
	CS_GRAYSCALE,		/* monochrome */
	CS_RGB,				/* red/green/blue */
	CS_YCBCR,			/* Y/Cb/Cr (also known as YUV) */
	CS_CMYK,			/* C/M/Y/K */
	CS_YCCK,			/* Y/Cb/Cr/K */
	CS_MAXINT=0x7FFFFFFF	/* Ensure ENUM is 32 bit */
} ColorSpace_e;

/* DCT/IDCT algorithm options. */

typedef enum {
	DCT_ISLOW,				/* slow but accurate integer algorithm */
	DCT_IFAST,				/* faster, less accurate integer method */
	DCT_FLOAT,				/* floating-point: accurate, fast on fast HW */
	DCT_MAXINT=0x7FFFFFFF	/* Ensure ENUM is 32 bit */
} DCTMethod_e;

/* Dithering options for decompression. */

typedef enum {
	DITHER_NONE,		/* no dithering */
	DITHER_ORDERED,		/* simple ordered dither */
	DITHER_FS,			/* Floyd-Steinberg error diffusion dither */
	DITHER_MAXINT=0x7FFFFFFF	/* Ensure ENUM is 32 bit */
} DitherMode_e;

typedef enum {
	CSTATE_NONE=	0,		/* No state */
	CSTATE_START=	100,	/* after create_compress */
	CSTATE_SCANNING=101,	/* start_compress done,write_scanlines OK */
	CSTATE_RAW_OK=	102,	/* start_compress done,write_raw_data OK */
	CSTATE_WRCOEFS=	103,	/* jpeg_write_coefficients done */
	DSTATE_START=	200,	/* after create_decompress */
	DSTATE_INHEADER=201,	/* reading header markers,no SOS yet */
	DSTATE_READY=	202,	/* found SOS,ready for start_decompress */
	DSTATE_PRELOAD=	203,	/* reading multiscan file in start_decompress*/
	DSTATE_PRESCAN=	204,	/* performing dummy pass for 2-pass quant */
	DSTATE_SCANNING=205,	/* start_decompress done,read_scanlines OK */
	DSTATE_RAW_OK=	206,	/* start_decompress done,read_raw_data OK */
	DSTATE_BUFIMAGE=207,	/* expecting jpeg_start_output */
	DSTATE_BUFPOST=	208,	/* looking for SOS/EOI in jpeg_finish_output */
	DSTATE_RDCOEFS=	209,	/* reading file in jpeg_read_coefficients */
	DSTATE_STOPPING=210,	/* looking for EOI in jpeg_finish_decompress */
	DSTATE_MAXINT=0x7FFFFFFF
} GlobalState_e;

/* Error codes */

typedef enum {
	JMSG_NOMESSAGE,
	JERR_ARITH_NOTIMPL,
	JERR_BAD_ALIGN_TYPE,
	JERR_BAD_ALLOC_CHUNK,
	JERR_BAD_BUFFER_MODE,
	JERR_BAD_COMPONENT_ID,
	JERR_BAD_DCT_COEF,
	JERR_BAD_DCTSIZE,
	JERR_BAD_HUFF_TABLE,
	JERR_BAD_IN_COLORSPACE,
	JERR_BAD_J_COLORSPACE,
	JERR_BAD_LENGTH,
	JERR_BAD_LIB_VERSION,
	JERR_BAD_MCU_SIZE,
	JERR_BAD_POOL_ID,
	JERR_BAD_PRECISION,
	JERR_BAD_PROGRESSION,
	JERR_BAD_PROG_SCRIPT,
	JERR_BAD_SAMPLING,
	JERR_BAD_SCAN_SCRIPT,
	JERR_BAD_STATE,
	JERR_BAD_STRUCT_SIZE,
	JERR_BAD_VIRTUAL_ACCESS,
	JERR_BUFFER_SIZE,
	JERR_CANT_SUSPEND,
	JERR_CCIR601_NOTIMPL,
	JERR_COMPONENT_COUNT,
	JERR_CONVERSION_NOTIMPL,
	JERR_DAC_INDEX,
	JERR_DAC_VALUE,
	JERR_DHT_INDEX,
	JERR_DQT_INDEX,
	JERR_EMPTY_IMAGE,
	JERR_EMS_READ,
	JERR_EMS_WRITE,
	JERR_EOI_EXPECTED,
	JERR_FILE_READ,
	JERR_FILE_WRITE,
	JERR_FRACT_SAMPLE_NOTIMPL,
	JERR_HUFF_CLEN_OVERFLOW,
	JERR_HUFF_MISSING_CODE,
	JERR_IMAGE_TOO_BIG,
	JERR_INPUT_EMPTY,
	JERR_INPUT_EOF,
	JERR_MISMATCHED_QUANT_TABLE,
	JERR_MISSING_DATA,
	JERR_MODE_CHANGE,
	JERR_NOTIMPL,
	JERR_NOT_COMPILED,
	JERR_NO_BACKING_STORE,
	JERR_NO_HUFF_TABLE,
	JERR_NO_IMAGE,
	JERR_NO_QUANT_TABLE,
	JERR_NO_SOI,
	JERR_OUT_OF_MEMORY,
	JERR_QUANT_COMPONENTS,
	JERR_QUANT_FEW_COLORS,
	JERR_QUANT_MANY_COLORS,
	JERR_SOF_DUPLICATE,
	JERR_SOF_NO_SOS,
	JERR_SOF_UNSUPPORTED,
	JERR_SOI_DUPLICATE,
	JERR_SOS_NO_SOF,
	JERR_TFILE_CREATE,
	JERR_TFILE_READ,
	JERR_TFILE_SEEK,
	JERR_TFILE_WRITE,
	JERR_TOO_LITTLE_DATA,
	JERR_UNKNOWN_MARKER,
	JERR_VIRTUAL_BUG,
	JERR_WIDTH_OVERFLOW,
	JERR_XMS_READ,
	JERR_XMS_WRITE,
	JMSG_COPYRIGHT,
	JMSG_VERSION,
	JTRC_16BIT_TABLES,
	JTRC_ADOBE,
	JTRC_APP0,
	JTRC_APP14,
	JTRC_DAC,
	JTRC_DHT,
	JTRC_DQT,
	JTRC_DRI,
	JTRC_EMS_CLOSE,
	JTRC_EMS_OPEN,
	JTRC_EOI,
	JTRC_HUFFBITS,
	JTRC_JFIF,
	JTRC_JFIF_BADTHUMBNAILSIZE,
	JTRC_JFIF_EXTENSION,
	JTRC_JFIF_THUMBNAIL,
	JTRC_MISC_MARKER,
	JTRC_PARMLESS_MARKER,
	JTRC_QUANTVALS,
	JTRC_QUANT_3_NCOLORS,
	JTRC_QUANT_NCOLORS,
	JTRC_QUANT_SELECTED,
	JTRC_RECOVERY_ACTION,
	JTRC_RST,
	JTRC_SMOOTH_NOTIMPL,
	JTRC_SOF,
	JTRC_SOF_COMPONENT,
	JTRC_SOI,
	JTRC_SOS,
	JTRC_SOS_COMPONENT,
	JTRC_SOS_PARAMS,
	JTRC_TFILE_CLOSE,
	JTRC_TFILE_OPEN,
	JTRC_THUMB_JPEG,
	JTRC_THUMB_PALETTE,
	JTRC_THUMB_RGB,
	JTRC_UNKNOWN_IDS,
	JTRC_XMS_CLOSE,
	JTRC_XMS_OPEN,
	JWRN_ADOBE_XFORM,
	JWRN_BOGUS_PROGRESSION,
	JWRN_EXTRANEOUS_DATA,
	JWRN_HIT_MARKER,
	JWRN_HUFF_BAD_CODE,
	JWRN_JFIF_MAJOR,
	JWRN_JPEG_EOF,
	JWRN_MUST_RESYNC,
	JWRN_NOT_SEQUENTIAL,
	JWRN_TOO_MUCH_DATA,
	JMSG_LASTMSGCODE,
	JERR_MAXINT=0x7FFFFFFF
} ErrorCode_e;

typedef SWord16 Block_t[DCTSIZE2];	/* one block of coefficients (128 bytes) */

/* Forward declarations */


/* DCT coefficient quantization tables. */

typedef struct QuantTable_t {
	Word16 quantval[DCTSIZE2];	/* quantization step for each coefficient */
	Word8 sent_table;			/* TRUE when table has been output */
	Word8 Padding[3];			/* Not used */
} QuantTable_t;

/* Huffman coding tables. */

typedef struct HuffTable_t {		
	Word8 bits[17];		/* bits[k] = # of symbols with codes of length k bits; bits[0] is unused */
	Word8 sent_table;		/* TRUE when table has been output */
	Word8 Padding[2];		/* Align to longword */
	Word8 huffval[256];		/* The symbols,in order of incr code length */
} HuffTable_t;

/* Basic info about one component (color channel). */

typedef struct ComponentInfo_t {
	int component_id;		/* identifier for this component (0..255) */
	int component_index;	/* its index in SOF or cinfo->m_CompInfoPtr[] */
	int h_samp_factor;		/* horizontal sampling factor (1..4) */
	int v_samp_factor;		/* vertical sampling factor (1..4) */
	int quant_tbl_no;		/* quantization table selector (0..3) */
	int dc_tbl_no;			/* DC entropy table selector (0..3) */
	int ac_tbl_no;			/* AC entropy table selector (0..3) */
	Word width_in_blocks;	/* Component's size in DCT (8x8) blocks */
	Word height_in_blocks;
	int DCT_scaled_size;	/* Always DCTSIZE for compression or size of one DCT block for decompression */
	Word downsampled_width;	/* actual width in samples */
	Word downsampled_height;	/* actual height in samples */
	Word8 component_needed;	/* do we need the value of this component? */
	Word8 Padding[3];		/* Not used */
	int MCU_width;			/* number of blocks per MCU,horizontally */
	int MCU_height;			/* number of blocks per MCU,vertically */
	int MCU_blocks;			/* MCU_width * MCU_height */
	int MCU_sample_width;	/* MCU width in samples, MCU_width*DCT_scaled_size */
	int last_col_width;		/* # of non-dummy blocks across in last MCU */
	int last_row_height;	/* # of non-dummy blocks down in last MCU */
	QuantTable_t *quant_table;	/* Saved quantization table for component; NULL if none yet saved. */
	union {
		float *Float;		/* Private per-component storage for DCT or IDCT subsystem. */
		SWord32 *Int;
	} DctTablePtr;
} ComponentInfo_t;

/* The script for encoding a multiple-scan file is an array of these: */

typedef struct ScanInfo_t {
	int comps_in_scan;		/* number of components encoded in this scan */
	int component_index[MAX_COMPS_IN_SCAN]; /* their SOF/m_CompInfoPtr[] indexes */
	int Ss,Se;				/* progressive JPEG spectral selection parms */
	int Ah,Al;				/* progressive JPEG successive approx. parms */
} ScanInfo_t;

/* The decompressor can save APPn and COM markers in a list of these: */

typedef struct Marker_t {
	struct Marker_t *next;	/* next in list,or NULL */
	Word8 marker;				/* marker code: JPEG_COM, or JPEG_APP0+n */
	Word8 Padding[3];			/* Not used */
	Word original_length;		/* # bytes of data in the file */
	Word data_length;			/* # bytes of data saved at data[] */
	Word8 * data;				/* the data contained in the marker */
} Marker_t;


/* Master manager for compression and decompression */

class CCommonManager;

/* Base code is in JUtils.hpp */

class CCommonManager {
public:
	CCommonManager();
	virtual ~CCommonManager();

	struct jpeg_memory_mgr *mem;	/* Memory manager module */

	Word8 m_IsDecompressor;	/* So common code can tell which is which */
	Word8 PaddingX[3];		/* Not used */
	GlobalState_e m_GlobalState;		/* For checking call sequence validity */

	/* Memory functions */
	JSample_t ** BURGERCALL AllocSArray(Word Width,Word Heigth);

	/* Error handler */
	
	int m_ErrMsgCode;			/* Last error message ID */
	const char *const *m_MessageTablePtr;		/* Library errors */
	int m_LastMessage;			/* Table contains strings 0..last_jpeg_message */
	const char *const *m_AddonMessageTablePtr; /* Non-library errors */
	int m_FirstAddonMessage;	/* code for first string in addon table */
	int m_LastAddonMessage;		/* code for last string in addon table */
	int m_TraceLevel;			/* max msg_level that will be displayed */
	int m_NumWarnings;			/* number of corrupt-data warnings */
	union {
		int i[8];				/* Array of values to return */
		char s[128];			/* String message */
	} m_ErrMsgParms;

	virtual void BURGERCALL ResetErrorManager(void);
	virtual void BURGERCALL OutputMessage(void);
	virtual void BURGERCALL EmitMessage(int MsgLevel,int ErrorCode);
	virtual void BURGERCALL FatalError(int ErrorCode);
	virtual void BURGERCALL FormatMessage(char *TextBuffer,Word BufferSize);
	/* Memory manager */
	
	Word32 m_MaxNemoryToUse;	/* Memory cap */
	Word32 m_MaxAllocChunk;		/* Largest memory I can allocate */

	/* Progress of the decompress/compression */
	
public:
	virtual void BURGERCALL ShowProgress(void);
	Word m_PassCounter;		/* work units completed in this pass */
	Word m_PassLimit;		/* total number of work units in this pass */
	Word m_CompletedPasses;	/* passes completed so far */
	Word m_TotalPasses;		/* total number of passes expected */

	/* Helper functions */
	
public:
	inline void FatalError(int ErrorCode,int a)
	{
		m_ErrMsgParms.i[0] = a;
		FatalError(ErrorCode);
	}
	inline void FatalError(int ErrorCode,int a,int b)
	{
		m_ErrMsgParms.i[0] = a;
		m_ErrMsgParms.i[1] = b;
		FatalError(ErrorCode);
	}
	inline void FatalError(int ErrorCode,int a,int b,int c)
	{
		m_ErrMsgParms.i[0] = a;
		m_ErrMsgParms.i[1] = b;
		m_ErrMsgParms.i[2] = c;
		FatalError(ErrorCode);
	}
	inline void FatalError(int ErrorCode,int a,int b,int c,int d)
	{
		m_ErrMsgParms.i[0] = a;
		m_ErrMsgParms.i[1] = b;
		m_ErrMsgParms.i[2] = c;
		m_ErrMsgParms.i[3] = d;
		FatalError(ErrorCode);
	}
	inline void EmitWarning(int ErrorCode)
	{
		EmitMessage(-1,ErrorCode);
	}
	inline void EmitWarning(int ErrorCode,int a)
	{
		m_ErrMsgParms.i[0] = a;
		EmitMessage(-1,ErrorCode);
	}
	inline void EmitWarning(int ErrorCode,int a,int b)
	{
		m_ErrMsgParms.i[0] = a;
		m_ErrMsgParms.i[1] = b;
		EmitMessage(-1,ErrorCode);
	}
	inline void EmitTrace(int ErrorCode,int Lvl)
	{
		EmitMessage(Lvl,ErrorCode);
	}
	inline void EmitTrace(int ErrorCode,int Lvl,int a)
	{
		m_ErrMsgParms.i[0] = a;
		EmitMessage(Lvl,ErrorCode);
	}
	inline void EmitTrace(int ErrorCode,int Lvl,int a,int b)
	{
		m_ErrMsgParms.i[0] = a;
		m_ErrMsgParms.i[1] = b;
		EmitMessage(Lvl,ErrorCode);
	}
	inline void EmitTrace(int ErrorCode,int Lvl,int a,int b,int c)
	{
		m_ErrMsgParms.i[0] = a;
		m_ErrMsgParms.i[1] = b;
		m_ErrMsgParms.i[2] = c;
		EmitMessage(Lvl,ErrorCode);
	}
	inline void EmitTrace(int ErrorCode,int Lvl,int a,int b,int c,int d)
	{
		m_ErrMsgParms.i[0] = a;
		m_ErrMsgParms.i[1] = b;
		m_ErrMsgParms.i[2] = c;
		m_ErrMsgParms.i[3] = d;
		EmitMessage(Lvl,ErrorCode);
	}
	inline void EmitTrace(int ErrorCode,int Lvl,int a,int b,int c,int d,int e)
	{
		m_ErrMsgParms.i[0] = a;
		m_ErrMsgParms.i[1] = b;
		m_ErrMsgParms.i[2] = c;
		m_ErrMsgParms.i[3] = d;
		m_ErrMsgParms.i[4] = e;
		EmitMessage(Lvl,ErrorCode);
	}
	inline void EmitTrace(int ErrorCode,int Lvl,int a,int b,int c,int d,int e,int f)
	{
		m_ErrMsgParms.i[0] = a;
		m_ErrMsgParms.i[1] = b;
		m_ErrMsgParms.i[2] = c;
		m_ErrMsgParms.i[3] = d;
		m_ErrMsgParms.i[4] = e;
		m_ErrMsgParms.i[5] = f;
		EmitMessage(Lvl,ErrorCode);
	}
	inline void EmitTrace(int ErrorCode,int Lvl,int a,int b,int c,int d,int e,int f,int g)
	{
		m_ErrMsgParms.i[0] = a;
		m_ErrMsgParms.i[1] = b;
		m_ErrMsgParms.i[2] = c;
		m_ErrMsgParms.i[3] = d;
		m_ErrMsgParms.i[4] = e;
		m_ErrMsgParms.i[5] = f;
		m_ErrMsgParms.i[6] = g;
		EmitMessage(Lvl,ErrorCode);
	}
	inline void EmitTrace(int ErrorCode,int Lvl,int a,int b,int c,int d,int e,int f,int g,int h)
	{
		m_ErrMsgParms.i[0] = a;
		m_ErrMsgParms.i[1] = b;
		m_ErrMsgParms.i[2] = c;
		m_ErrMsgParms.i[3] = d;
		m_ErrMsgParms.i[4] = e;
		m_ErrMsgParms.i[5] = f;
		m_ErrMsgParms.i[6] = g;
		m_ErrMsgParms.i[7] = h;
		EmitMessage(Lvl,ErrorCode);
	}

};


}
#endif