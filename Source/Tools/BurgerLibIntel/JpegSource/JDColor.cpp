/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#include "JDColor.hpp"
#define JPEG_INTERNALS
#include "jpeglib.h"
#include "JUtils.hpp"

namespace JPeg70 {

/*******************************

	YCbCr -> RGB conversion: most common case

	YCbCr is defined per CCIR 601-1, except that Cb and Cr are
	normalized to the range 0..MAXJSAMPLE rather than -0.5 .. 0.5.
	The conversion equations to be implemented are therefore
		R = Y                + 1.40200 * Cr
		G = Y - 0.34414 * Cb - 0.71414 * Cr
		B = Y + 1.77200 * Cb
	where Cb and Cr represent the incoming values less CENTERJSAMPLE.
	(These numbers are derived from TIFF 6.0 section 21, dated 3-June-92.)

	To avoid floating-point arithmetic, we represent the fractional constants
	as integers scaled up by 2^16 (about 4 digits precision); we have to divide
	the products by 2^16, with appropriate rounding, to get the correct answer.
	Notice that Y, being an integral input, does not contribute any fraction
	so it need not participate in the rounding.

	For even more speed, we avoid doing any multiplications in the inner loop
	by precalculating the constants times Cb and Cr for all possible values.
	For 8-bit JSAMPLEs this is very reasonable (only 256 entries per table);
	for 12-bit samples it is still acceptable.  It's not very reasonable for
	16-bit samples, but if you want lossless storage you shouldn't be changing
	colorspace anyway.
	The Cr=>R and Cb=>B values can be rounded to integers in advance; the
	values for the G calculation are left scaled up, since we must add them
	together before rounding.

*******************************/

#define SCALEBITS 16	/* speediest right-shift on some machines */
#define ONE_HALF ((Fixed32) 1 << (SCALEBITS-1))
#define FIX(xx) ((Fixed32) ((xx) * (1L<<SCALEBITS) + 0.5f))

/*******************************

	Initialize tables for YCC->RGB colorspace conversion.
	Return FALSE on no error

*******************************/

/* There is a bug in CodeWarrior 8.3 which will make */
/* bad code, (It optimized in the >>) this HACK is a workaround */

#if defined(__MWERKS__) && __MWERKS__==0x3003
#define HACK (volatile Fixed32)
#else
#define HACK
#endif

#define CRRSTEP 0x166E9		/* FIX(1.40200f) */
#define CBBSTEP 0x1C5A2		/* FIX(1.77200f) */
#define CRGSTEP 0x0B6D2		/* FIX(0.71414f) */
#define CBGSTEP 0x0581A		/* FIX(0.34414f) */

void BURGERCALL CColorDeconverter::BuildYCCRGBTable(void)
{
	Fixed32 *CrRTablePtr = new Fixed32[(MAXJSAMPLE+1)*4];
	if (CrRTablePtr) {
		/* Copy the pointers to the globals */
		
		m_TablePtr = CrRTablePtr;
		
		/* Make the local pointers */

		Fixed32 *CbBTablePtr = &CrRTablePtr[(MAXJSAMPLE+1)*1];
		Fixed32 *CrGTablePtr = &CrRTablePtr[(MAXJSAMPLE+1)*2];
		Fixed32 *CbGTablePtr = &CrRTablePtr[(MAXJSAMPLE+1)*3];
		
		/* Now, create the color scale tables */
		
		Word i = 0;
		Fixed32 CrR = (CRRSTEP*-CENTERJSAMPLE)+ONE_HALF;
		Fixed32 CbB = (CBBSTEP*-CENTERJSAMPLE)+ONE_HALF;
		Fixed32 CrG = (CRGSTEP*CENTERJSAMPLE);
		Fixed32 CbG = (CBGSTEP*CENTERJSAMPLE)+ONE_HALF;
		do {
			/* i is the actual input pixel value, in the range 0..MAXJSAMPLE */
			/* The Cb or Cr value we are thinking of is x = i - CENTERJSAMPLE */
			/* Cr=>R value is nearest int to 1.40200 * x */
			CrRTablePtr[i] = (HACK CrR >> SCALEBITS);

			/* Cb=>B value is nearest int to 1.77200 * x */
			CbBTablePtr[i] = (HACK CbB >> SCALEBITS);

			/* Cr=>G value is scaled-up -0.71414 * x */
			CrGTablePtr[i] = CrG;

			/* Cb=>G value is scaled-up -0.34414 * x */
			/* We also add in ONE_HALF so that need not do it in inner loop */
			CbGTablePtr[i] = CbG;

			CrR += CRRSTEP;
			CbB += CBBSTEP;
			CrG -= CRGSTEP;
			CbG -= CBGSTEP;
		} while (++i<=MAXJSAMPLE);
	} else {
		m_ColorConvert = Bogus;		/* Shut down conversion */
		m_Parent->FatalError(JERR_OUT_OF_MEMORY,sizeof(Fixed32)*(MAXJSAMPLE+1)*4);
	}
}

/*******************************

	Convert some rows of samples to the output colorspace.

	Note that we change from noninterleaved, one-plane-per-component format
	to interleaved-pixel format.  The output buffer is therefore three times
	as wide as the input buffer.
	A starting row offset is provided only for the input buffer.  The caller
	can easily adjust the passed output_buf value to accommodate any row
	offset required on that side.

*******************************/

void BURGERCALL CColorDeconverter::YCCRGBConvert(CColorDeconverter *ThisPtr,JSample_t ***input_buf,Word input_row,JSample_t **output_buf,int num_rows)
{
	if (num_rows>0) {
		Word num_cols = ThisPtr->m_Parent->output_width;
		if (num_cols) {
			/* copy these pointers into registers if possible */
			const JSample_t *range_limit = ThisPtr->m_Parent->sample_range_limit;
			const Fixed32 *Crrtab = ThisPtr->GetCrRTable();
			if (Crrtab) {			/* Failsafe */
				const Fixed32 *Cbbtab = ThisPtr->GetCbBTable();
				const Fixed32 *Crgtab = ThisPtr->GetCrGTable();
				const Fixed32 *Cbgtab = ThisPtr->GetCbGTable();

				do {
					const JSample_t *inptr0 = input_buf[0][input_row];
					const JSample_t *inptr1 = input_buf[1][input_row];
					const JSample_t *inptr2 = input_buf[2][input_row];
					++input_row;
					JSample_t *outptr = output_buf[0];
					++output_buf;
					Word col = 0;
					do {
						Word y = inptr0[col];
						Word cb = inptr1[col];
						Word cr = inptr2[col];
						/* Range-limiting is essential due to noise introduced by DCT losses. */
						outptr[RGB_RED] = range_limit[y + Crrtab[cr]];
						outptr[RGB_GREEN] = range_limit[y + (((Cbgtab[cb] + Crgtab[cr])>>SCALEBITS))];
						outptr[RGB_BLUE] =  range_limit[y + Cbbtab[cb]];
						outptr += RGB_PIXELSIZE;
					} while (++col<num_cols);
				} while (--num_rows);
			}
		}
	}
}

/*******************************

	Cases other than YCbCr -> RGB

	Color conversion for no colorspace change: just copy the data,
	converting from separate-planes to interleaved representation.

*******************************/

void BURGERCALL CColorDeconverter::NullConvert(CColorDeconverter *ThisPtr,JSample_t *** input_buf,Word input_row,JSample_t ** output_buf,int num_rows)
{
	if (num_rows>0) {
		int num_components = ThisPtr->m_Parent->m_NumComponents;
		if (num_components>0) {
			Word num_cols = ThisPtr->m_Parent->output_width;
			if (num_cols) {
				do {
					int ci = num_components;
					do {
						JSample_t *inptr = input_buf[ci][input_row];
						JSample_t *outptr = output_buf[0] + ci;
						Word count = num_cols;
						do {
							outptr[0] = inptr[0];
							++inptr;
							outptr += num_components;
						} while (--count);
					} while (++ci<num_components);
					input_row++;
					output_buf++;
				} while (--num_rows);
			}
		}
	}
}

/*******************************

	Color conversion for grayscale: just copy the data.
	This also works for YCbCr -> grayscale conversion, in which
	we just copy the Y (luminance) component and ignore chrominance.

*******************************/

void BURGERCALL CColorDeconverter::GrayscaleConvert(CColorDeconverter *ThisPtr,JSample_t *** input_buf,Word input_row,JSample_t ** output_buf,int num_rows)
{
	CopySampleRows(input_buf[0]+input_row,output_buf,num_rows,ThisPtr->m_Parent->output_width);
}

/*******************************

	Convert grayscale to RGB: just duplicate the graylevel three times.
	This is provided to support applications that don't want to cope
	with grayscale as a separate case.

*******************************/

void BURGERCALL CColorDeconverter::GrayRGBConvert(CColorDeconverter *ThisPtr,JSample_t *** input_buf,Word input_row,JSample_t ** output_buf,int num_rows)
{
	if (num_rows>0) {
		Word num_cols = ThisPtr->m_Parent->output_width;
		if (num_cols) {
			JSample_t **InputPtr1;
			InputPtr1 = input_buf[0]+input_row;		/* Index to the table */
			do {
				JSample_t *inptr = InputPtr1[0];
				++InputPtr1;
				JSample_t *outptr = output_buf[0];
				++output_buf;
				Word col = num_cols;
				do {
					JSample_t Temp;
					Temp = inptr[0];
					++inptr;
					outptr[RGB_RED] = Temp;
					outptr[RGB_GREEN] = Temp;
					outptr[RGB_BLUE] = Temp;
					outptr += RGB_PIXELSIZE;
				} while (--col);
			} while (--num_rows);
		}
	}
}


/*******************************

	Adobe-style YCCK->CMYK conversion.
	We convert YCbCr to R=1-C, G=1-M, and B=1-Y using the same
	conversion as above, while passing K (black) unchanged.
	We assume build_ycc_rgb_table has been called.

*******************************/

void BURGERCALL CColorDeconverter::YCCKCMYKConvert(CColorDeconverter *ThisPtr,JSample_t *** input_buf,Word input_row,JSample_t ** output_buf,int num_rows)
{
	if (num_rows>1) {
		Word num_cols = ThisPtr->m_Parent->output_width;
		if (num_cols) {
			/* copy these pointers into registers if possible */
			JSample_t *range_limit = ThisPtr->m_Parent->sample_range_limit+MAXJSAMPLE;
			Fixed32 * Crrtab = ThisPtr->GetCrRTable();
			if (Crrtab) {						/* This is a failsafe */
				Fixed32 * Cbbtab = ThisPtr->GetCbBTable();
				Fixed32 * Crgtab = ThisPtr->GetCrGTable();
				Fixed32 * Cbgtab = ThisPtr->GetCbGTable();

				do {
					JSample_t *inptr0 = input_buf[0][input_row];
					JSample_t *inptr1 = input_buf[1][input_row];
					JSample_t *inptr2 = input_buf[2][input_row];
					JSample_t *inptr3 = input_buf[3][input_row];
					++input_row;
					JSample_t *outptr = output_buf[0];
					++output_buf;
					Word col = 0;
					do {
						Word y = inptr0[col];
						Word cb = inptr1[col];
						Word cr = inptr2[col];
						/* Range-limiting is essential due to noise introduced by DCT losses. */
						outptr[0] = range_limit[0- (y + Crrtab[cr])];	/* red */
						outptr[1] = range_limit[0- (y + (((Cbgtab[cb] + Crgtab[cr])>>SCALEBITS)))]; /* Green */
						outptr[2] = range_limit[0- (y + Cbbtab[cb])];	/* blue */
						/* K passes through unchanged */
						outptr[3] = inptr3[col];
						outptr += 4;
					} while (++col<num_cols);
				} while (--num_rows);
			}
		}
	}
}

/*******************************

	Does nothing by design
	Used as a failsafe for error conditions

*******************************/

void BURGERCALL CColorDeconverter::Bogus(CColorDeconverter *,JSample_t ***,Word ,JSample_t **,int)
{
}

/*******************************

	Empty method for start_pass.

*******************************/

#if 0		/* Inlined */
void BURGERCALL CColorDeconverter::StartPass(void)
{
	/* no work needed */
}
#endif

/*******************************

	Module initialization routine for output colorspace conversion.

*******************************/

CColorDeconverter::CColorDeconverter(CJPegDecompress *CInfoPtr)
:	m_Parent(CInfoPtr),
	m_TablePtr(0),
	m_ColorConvert(Bogus)	
{
	/* Make sure m_NumComponents agrees with m_JPegColorSpace */
	
	switch (CInfoPtr->m_JPegColorSpace) {
	case CS_GRAYSCALE:
		if (CInfoPtr->m_NumComponents != 1) {
			CInfoPtr->FatalError(JERR_BAD_J_COLORSPACE);
		}
		break;

	case CS_RGB:
	case CS_YCBCR:
		if (CInfoPtr->m_NumComponents != 3) {
			CInfoPtr->FatalError(JERR_BAD_J_COLORSPACE);
		}
		break;

	case CS_CMYK:
	case CS_YCCK:
		if (CInfoPtr->m_NumComponents != 4) {
			CInfoPtr->FatalError(JERR_BAD_J_COLORSPACE);
		}
		break;

	default:			/* JCS_UNKNOWN can be anything */
		if (CInfoPtr->m_NumComponents < 1) {
			CInfoPtr->FatalError(JERR_BAD_J_COLORSPACE);
		}
		break;
	}

	/* Set m_ColorComponents and conversion method based on requested space.
	* Also clear the component_needed flags for any unused components,
	* so that earlier pipeline stages can avoid useless computation.
	*/


	switch (CInfoPtr->m_OutputColorSpace) {
	case CS_GRAYSCALE:
		CInfoPtr->m_ColorComponents = 1;
		if (CInfoPtr->m_JPegColorSpace == CS_GRAYSCALE ||
			CInfoPtr->m_JPegColorSpace == CS_YCBCR) {
			m_ColorConvert = GrayscaleConvert;
			/* For color->grayscale conversion, only the Y (0) component is needed */
			int ci;
			for (ci = 1; ci < CInfoPtr->m_NumComponents; ci++) {
				CInfoPtr->m_CompInfoPtr[ci].component_needed = FALSE;
			}
		} else {
			CInfoPtr->FatalError(JERR_CONVERSION_NOTIMPL);
		}
		break;

	case CS_RGB:
		CInfoPtr->m_ColorComponents = RGB_PIXELSIZE;
		if (CInfoPtr->m_JPegColorSpace == CS_YCBCR) {
			m_ColorConvert = YCCRGBConvert;
			BuildYCCRGBTable();				/* Can alter m_ColorConvert on error */
		} else if (CInfoPtr->m_JPegColorSpace == CS_GRAYSCALE) {
			m_ColorConvert = GrayRGBConvert;
		} else if (CInfoPtr->m_JPegColorSpace == CS_RGB && RGB_PIXELSIZE == 3) {
			m_ColorConvert = NullConvert;
		} else
			CInfoPtr->FatalError(JERR_CONVERSION_NOTIMPL);
		break;

	case CS_CMYK:
		CInfoPtr->m_ColorComponents = 4;
		if (CInfoPtr->m_JPegColorSpace == CS_YCCK) {
			m_ColorConvert = YCCKCMYKConvert;
			BuildYCCRGBTable();				/* Can alter m_ColorConvert on error */
		} else if (CInfoPtr->m_JPegColorSpace == CS_CMYK) {
			m_ColorConvert = NullConvert;
		} else {
			CInfoPtr->FatalError(JERR_CONVERSION_NOTIMPL);
		}
		break;

	default:
		/* Permit null conversion to same output space */
		if (CInfoPtr->m_OutputColorSpace == CInfoPtr->m_JPegColorSpace) {
			CInfoPtr->m_ColorComponents = CInfoPtr->m_NumComponents;
			m_ColorConvert = NullConvert;
		} else {				/* unsupported non-null conversion */
			CInfoPtr->FatalError(JERR_CONVERSION_NOTIMPL);
		}
		break;
	}

	if (CInfoPtr->m_QuantizeColors) {
		CInfoPtr->m_OutputComponents = 1; /* single colormapped output component */
	} else {
 		CInfoPtr->m_OutputComponents = CInfoPtr->m_ColorComponents;
	}
}

/*******************************

	Clean up after myself

*******************************/

CColorDeconverter::~CColorDeconverter()
{
	if (m_TablePtr) {			/* Was the table initialized? */
		delete [] m_TablePtr;	/* Free the memory */
	}
}

}