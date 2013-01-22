/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#ifndef __JQUANT2_HPP__
#define __JQUANT2_HPP__

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

/* Color quantization or color precision reduction */

class CColorQuantizer2Pass : public CColorQuantizer {
public:
	CColorQuantizer2Pass(CJPegDecompress *CInfoPtr);
	virtual ~CColorQuantizer2Pass();
	virtual void BURGERCALL StartPass(Word is_pre_scan);
	virtual void BURGERCALL FinishPass(void);
	virtual void BURGERCALL NewColorMap(void);
private:
	enum {
		R_SCALE=2,		/* scale R distances by this much */
		G_SCALE=3,		/* scale G distances by this much */
		B_SCALE=1,		/* and B by this much */
#if 1		/* R,G,B */
		C0_SCALE=R_SCALE,
		C1_SCALE=G_SCALE,
		C2_SCALE=B_SCALE,
#else		/* B,G,R */
		C0_SCALE=B_SCALE,
		C1_SCALE=G_SCALE,
		C2_SCALE=R_SCALE,
#endif
		MAXNUMCOLORS=(MAXJSAMPLE+1), /* maximum size of colormap */
		HIST_C0_BITS=5,		/* bits of precision in R/B histogram */
		HIST_C1_BITS=6,		/* bits of precision in G histogram */
		HIST_C2_BITS=5,		/* bits of precision in B/R histogram */
		HIST_C0_ELEMS=(1<<HIST_C0_BITS),
		HIST_C1_ELEMS=(1<<HIST_C1_BITS),
		HIST_C2_ELEMS=(1<<HIST_C2_BITS),
		C0_SHIFT=(BITS_IN_JSAMPLE-HIST_C0_BITS),
		C1_SHIFT=(BITS_IN_JSAMPLE-HIST_C1_BITS),
		C2_SHIFT=(BITS_IN_JSAMPLE-HIST_C2_BITS),
		BOX_C0_LOG=(HIST_C0_BITS-3),
		BOX_C1_LOG=(HIST_C1_BITS-3),
		BOX_C2_LOG=(HIST_C2_BITS-3),
		BOX_C0_ELEMS=(1<<BOX_C0_LOG), /* # of hist cells in update box */
		BOX_C1_ELEMS=(1<<BOX_C1_LOG),
		BOX_C2_ELEMS=(1<<BOX_C2_LOG),
		BOX_C0_SHIFT=(C0_SHIFT + BOX_C0_LOG),
		BOX_C1_SHIFT=(C1_SHIFT + BOX_C1_LOG),
		BOX_C2_SHIFT=(C2_SHIFT + BOX_C2_LOG),
		STEP_C0=((1<<C0_SHIFT) * C0_SCALE),		/* Step for finding best colors */
		STEP_C1=((1<<C1_SHIFT) * C1_SCALE),
		STEP_C2=((1<<C2_SHIFT) * C2_SCALE)
	};
	typedef Word16 HistCell_t;	/* histogram cell; prefer an unsigned type */
	typedef struct Box_t {
		/* The bounds of the box (inclusive); expressed as histogram indexes */
		int c0min, c0max;
		int c1min, c1max;
		int c2min, c2max;
		SWord32 volume;		/* The volume (actually 2-norm) of the box */
		SWord32 colorcount;	/* The number of nonzero histogram cells within this box */
	} Box_t;

	static void BURGERCALL PrescanQuantize(CColorQuantizer *ThisPtr,JSample_t **input_buf,JSample_t **output_buf, int num_rows);
	inline static Box_t *FindBiggestColorPop(Box_t *boxlist,Word numboxes);
	inline static Box_t *FindBiggestVolume(Box_t *boxlist,Word numboxes);
	void UpdateBox(Box_t *boxp);
	Word MedianCut(Box_t *boxlist,Word numboxes,Word desired_colors);
	void ComputeColor(Box_t * boxp,Word icolor);
	void SelectColors(Word desired_colors);
	int FindNearbyColors(int minc0, int minc1, int minc2,JSample_t *colorlist);
	void FindBestColors(int minc0, int minc1, int minc2,int numcolors, JSample_t *colorlist,JSample_t *bestcolor);
	void FillInverseCMap(int c0,int c1,int c2);
	static void BURGERCALL Pass2NoDither(CColorQuantizer *ThisPtr,JSample_t **input_buf,JSample_t **output_buf, int num_rows);
	static void BURGERCALL Pass2FSDither(CColorQuantizer *ThisPtr,JSample_t **input_buf,JSample_t **output_buf, int num_rows);
	void InitErrorLimit(void)	;

	JSample_t **m_SVColorMapPtr;	/* colormap allocated at init time */
	SWord32 *m_FSErrors;	/* accumulated errors */
	int *m_ErrorLimiter;	/* table for clamping the applied error */
	Word m_Desired;			/* desired # of colors = size of colormap */
	HistCell_t m_Histogram[HIST_C0_ELEMS][HIST_C1_ELEMS][HIST_C2_ELEMS];		/* The histogram */
	Word8 m_OnOddRow;		/* flag to remember which row we are on */
	Word8 m_NeedsZeroed;	/* TRUE if next pass must zero histogram */
	Word8 m_DoPass1;		/* Perform pass1 */
	Word8 m_PaddingX;
};

}
#endif
