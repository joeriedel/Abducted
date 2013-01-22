/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#ifndef __JQUANT1_HPP__
#define __JQUANT1_HPP__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __JPEG70TYPES_H__
#include "JPeg70Types.h"
#endif

#ifndef __JQUANT_HPP__
#include "JQuant.hpp"
#endif

namespace JPeg70 {

/* NB: if ODITHER_SIZE is not a power of 2, ODITHER_MASK uses will break */

/* Color quantization or color precision reduction */

class CColorQuantizer1Pass : public CColorQuantizer {
public:
	CColorQuantizer1Pass(CJPegDecompress *CInfoPtr);
	virtual ~CColorQuantizer1Pass();
	virtual void BURGERCALL StartPass(Word is_pre_scan);
private:
	enum {
		MAX_Q_COMPS=4,					/* max components I can handle */
		ODITHER_SIZE=16,				/* dimension of dither matrix */
		ODITHER_CELLS=(ODITHER_SIZE*ODITHER_SIZE),	/* # cells in matrix */
		ODITHER_MASK=(ODITHER_SIZE-1)};	/* mask for wrapping around counters */

	typedef int ODitherMatrix_t[ODITHER_SIZE][ODITHER_SIZE];
	typedef int (*ODitherMatrixPtr)[ODITHER_SIZE];
	
	inline Word SelectNColors(int *Ncolors);
	inline int OutputValue(int j,int maxj);
	inline int LargestInputValue(int j,int maxj);
	void CreateColorMap(void);
	void CreateColorIndex(void);
	inline ODitherMatrixPtr MakeODitherArray(int ncolors);
	void CreateODitherTables(void);
	static void BURGERCALL ColorQuantize(CColorQuantizer *ThisPtr,JSample_t **input_buf,JSample_t **output_buf, int num_rows);
	static void BURGERCALL ColorQuantize3(CColorQuantizer *ThisPtr,JSample_t **input_buf,JSample_t ** output_buf,int num_rows);
	static void BURGERCALL QuantizeOrdDither(CColorQuantizer *ThisPtr,JSample_t ** input_buf,JSample_t ** output_buf,int num_rows);
	static void BURGERCALL Quantize3OrdDither(CColorQuantizer *ThisPtr,JSample_t ** input_buf,JSample_t ** output_buf,int num_rows);
	static void BURGERCALL QuantizeFSDither(CColorQuantizer *ThisPtr,JSample_t ** input_buf,JSample_t ** output_buf,int num_rows);
	void AllocFSWorkspace(void);

	JSample_t **m_SvColorMapPtr;	/* The color map as a 2-D pixel array */
	JSample_t **m_ColorIndex;		/* Precomputed mapping for speed */
	Word m_SvActual;				/* number of entries in use of m_SvColorMap */
	int m_NColors[MAX_Q_COMPS];		/* # of values alloced to each component */
	Word m_RowIndex;				/* cur row's vertical index in dither matrix */
	ODitherMatrixPtr m_ODither[MAX_Q_COMPS]; /* one dither array per component */
	/* Variables for Floyd-Steinberg dithering */
	SWord32 *m_FSErrors[MAX_Q_COMPS]; /* accumulated errors */
	Word8 m_OnOddRow;				/* flag to remember which row we are on */
	Word8 m_IsPadded;				/* is the colorindex padded for odither? */
	Word8 m_PaddingX[2];
	static const Word8 BaseDitherMatrix[ODITHER_SIZE][ODITHER_SIZE];
};

}
#endif
