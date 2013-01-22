#include "ImImage.h"
#include "ClStdLib.h"
#include "MMMemory.h"
#define __BURGER__
#include <jpeglib.h>

/**********************************

	Decode a JPeg file

**********************************/

typedef struct JPegLocals_t {
	const void *in_ptr;
	int in_length;
	Word in_overrun;
	Word8 eoibuf[4];
} JPegLocals_t;

JPegLocals_t Locals;

static void BURGERCALL init_source(CJPegDecompress* cinfo )
{
	JPegLocals_t *LocalsPtr;
	jpeg_source_mgr *SrcPtr;
	
	LocalsPtr = &Locals;
	SrcPtr = cinfo->src;
	LocalsPtr->in_overrun = FALSE;
	SrcPtr->next_input_byte = (const Word8 *)LocalsPtr->in_ptr;
	SrcPtr->bytes_in_buffer = LocalsPtr->in_length;
}

static Word8 BURGERCALL fill_input_buffer(CJPegDecompress* cinfo )
{
	JPegLocals_t *LocalsPtr;
	jpeg_source_mgr *SrcPtr;

	LocalsPtr = &Locals;
	SrcPtr = cinfo->src;
	
	LocalsPtr->in_overrun = TRUE;
	LocalsPtr->eoibuf[0] = 0xff;
	LocalsPtr->eoibuf[1] = JPEG_EOI;
	SrcPtr->next_input_byte = LocalsPtr->eoibuf;
	SrcPtr->bytes_in_buffer = 2;
	
	return TRUE;
}


static void BURGERCALL skip_input_data(CJPegDecompress* cinfo, long num_bytes )
{
	jpeg_source_mgr *SrcPtr;
	SrcPtr = cinfo->src;
	if (num_bytes >= SrcPtr->bytes_in_buffer) {
		fill_input_buffer( cinfo );
	} else {
		SrcPtr->next_input_byte += num_bytes;
		SrcPtr->bytes_in_buffer -= num_bytes;
	}
}


static void BURGERCALL term_source(CJPegDecompress* cinfo )
{
	cinfo = cinfo; // shut up compiler
	// nothing necessary
}

static struct jpeg_source_mgr jSrc = {
	0,	// next_input_byte
	0,		// bytes_in_buffer
	init_source,
	fill_input_buffer,
	skip_input_data,
	jpeg_resync_to_restart,	// use the default
	term_source
};

#if 0
static void BURGERCALL init_destination( jpeg_compress_struct* cinfo )
{
	out_total = 0;
	cinfo->dest->next_output_byte = ((Word8 *)out_ptr);
	cinfo->dest->free_in_buffer = out_length;
}

static Word8 BURGERCALL empty_output_buffer( jpeg_compress_struct* cinfo )
{
	out_total += out_length;
	cinfo->dest->next_output_byte = ((Word8 *)out_ptr);
	cinfo->dest->free_in_buffer = out_length;
	return TRUE;
}

static void BURGERCALL term_destination( jpeg_compress_struct* cinfo )
{
	out_total += out_length - cinfo->dest->free_in_buffer;
}

static struct jpeg_destination_mgr jDst = {
	0,		// next_output_byte
	0,		// free_in_buffer
	init_destination,
	empty_output_buffer,
	term_destination
};
#endif

Word BURGERCALL ImageParseJPG(Image_t *Output,const Word8 *InputPtr,Word32 InputLength)
{
	CJPegDecompress cinfo;
	Word Width,Height;
	void *ImagePtr;
	Word Result;
	Word RowBytes;

	/* Let's start by zapping my structures */
	
	FastMemSet(Output,0,sizeof(Image_t));	
//	FastMemSet(&cinfo,0,sizeof(cinfo));
			
	/* Create a decompression manager */
	
	jpeg_create_decompress( &cinfo );
	
	/* Init the input file */
	
	Locals.in_ptr = InputPtr;
	Locals.in_length = InputLength;
	Locals.in_overrun = 0;
	cinfo.src = &jSrc;
	
	/* Valid? */
	
	Result = TRUE;
	if (jpeg_read_header(&cinfo,TRUE) == JPEG_HEADER_OK ) {
		Width = cinfo.image_width;
		Height = cinfo.image_height;
		if (Width && Height) {
			Word BytesPerPixel;
			
			Output->Width = Width;
			Output->Height = Height;
			
			if (cinfo.m_NumComponents==1) {
				Word xy;
				Word8 *PalPtr;
				BytesPerPixel = 1;
				Output->DataType = IMAGE8_PAL;
				PalPtr = (Word8*)AllocAPointer(768);
				if (PalPtr) {
					Output->PalettePtr = PalPtr;
					xy = 0;
					do {
						PalPtr[0] = (Word8)xy;
						PalPtr[1] = (Word8)xy;
						PalPtr[2] = (Word8)xy;
						PalPtr+=3;
					} while (++xy<256);
				}
			} else {
				BytesPerPixel = 3;
				Output->DataType = IMAGE888;
			}
			RowBytes = Width*BytesPerPixel;
			Output->RowBytes = RowBytes;

			cinfo.scale_denom = 1;			/* Scale factor (No scaling) */
			
		#if 1
			cinfo.dct_method = JPeg70::DCT_FLOAT;
			cinfo.do_fancy_upsampling = TRUE;
		#else
			cinfo.dct_method = JPeg70::DCT_IFAST;
			cinfo.do_fancy_upsampling = FALSE;
		#endif
			
			/* Allocate memory to start decompression */
			
			jpeg_start_decompress(&cinfo);
			
			ImagePtr = AllocAPointer(Height*RowBytes);
			if (ImagePtr) {
				Word8 **RowPtr;
				Output->ImagePtr = static_cast<Word8 *>(ImagePtr);
				RowPtr = (Word8 **)AllocAPointer(sizeof(Word8 *)*Height);
				if (RowPtr) {
					Word y;
					Word8 * *RowPtr2;
					Word8 *FooPtr;
					
					y = Height;
					RowPtr2 = RowPtr;
					FooPtr = static_cast<Word8 *>(ImagePtr);
					do {
						RowPtr2[0] = FooPtr;
						FooPtr += RowBytes;
						++RowPtr2;
					} while (--y);
			
					while (cinfo.output_scanline < Height) {
						if (jpeg_read_scanlines(&cinfo,&RowPtr[cinfo.output_scanline],Height - cinfo.output_scanline)<1) {
							break;
						}
					}
					DeallocAPointer(RowPtr);
					Result = FALSE;
				}
			}
			jpeg_finish_decompress(&cinfo);
		}
	}
	jpeg_destroy_decompress(&cinfo);
	
	/* Failsafe in case the image was created, but the palette was not */
	/* Really should be above, but what the heck! */
	
	if (Output->DataType==IMAGE8_PAL && !Output->PalettePtr) {
		Result = TRUE;		/* Force error */
	}
	if (Result) {
		ImageDestroy(Output);
	}
	return Result;
}

#if 0
// this will compress
// the in ptr data with jpeg compression
// and allocate the out ptr, and fill it in.
// to make the jpeg file, just dump the *out
// ptr to disk.
//
// something that would be useful in burger lib is to expose the
// routines that compress/decompress a chunk of image data to/from
// jpeg format. this would allow an application to build it's own
// image formats around the jpeg compression.
//
// infact, it might be even better to have optional image routines
// that write straight to a StreamHandle_t 

Bool compress_jpeg( void **out, int *out_len, const void *in, int width, int height, int rowbytes, int quality, Bool hq )
{
	int y;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	Word8 * *rows;
	
	if (rowbytes == 0) {
		rowbytes = 3 * width;
	}
		
	jpeg_my_std_error(&jerr);
	cinfo.err = &jerr;
	jpeg_create_compress(&cinfo);
	
	*out = AllocAPointer( width*height*3 );
	if( !(*out) )
		return TRUE;
	
	cinfo.dest = &jDst;
	out_ptr = *out;
	out_length = width*height*3;
	
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults( &cinfo );
	
	if (hq) {
		cinfo.dct_method = JDCT_FLOAT;
		cinfo.optimize_coding = TRUE;
	} else {
		cinfo.dct_method = JDCT_IFAST;
		cinfo.optimize_coding = FALSE;
	}
	
	jpeg_set_quality( &cinfo, quality, FALSE );
	
	jpeg_start_compress( &cinfo, TRUE );
	
	rows = (Word8 **)AllocAPointer(sizeof(Word8 *)*height);
	if(!rows)
	{
		jpeg_destroy_compress(&cinfo);
		DeallocAPointer( *out );
		*out = 0;
		return TRUE;
	}
	
	for (y = 0; y < height; ++y) {
		rows[y] = ((Word8 *)in) + y * rowbytes;
	}
	
	while (cinfo.next_scanline < cinfo.image_height) {
		if (jpeg_write_scanlines( &cinfo, rows + cinfo.next_scanline, height - cinfo.next_scanline ) < 1) break;
	}
	
	DeallocAPointer(rows);
	
	jpeg_finish_compress( &cinfo );
	*out_len = out_total;
	
	jpeg_destroy_compress(&cinfo);
	return FALSE;
}

#endif