/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#ifndef __JDCOLOR_HPP__
#define __JDCOLOR_HPP__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __JPEG70TYPES_H__
#include "JPeg70Types.h"
#endif

class CJPegDecompress;

namespace JPeg70 {

class CColorDeconverter {
public:
	CColorDeconverter(CJPegDecompress *CInfoPtr);
	~CColorDeconverter();
	void BURGERCALL StartPass(void) {}
	inline void ColorConvert(JSample_t ***input_buf, Word input_row,JSample_t **output_buf,int num_rows) { m_ColorConvert(this,input_buf,input_row,output_buf,num_rows); }
private:
	void BURGERCALL BuildYCCRGBTable(void);
	static void BURGERCALL YCCRGBConvert(CColorDeconverter *ThisPtr,JSample_t ***input_buf,Word input_row,JSample_t **output_buf,int num_rows);
	static void BURGERCALL NullConvert(CColorDeconverter *ThisPtr,JSample_t *** input_buf,Word input_row,JSample_t ** output_buf,int num_rows);
	static void BURGERCALL GrayscaleConvert(CColorDeconverter *ThisPtr,JSample_t *** input_buf,Word input_row,JSample_t ** output_buf,int num_rows);
	static void BURGERCALL GrayRGBConvert(CColorDeconverter *ThisPtr,JSample_t *** input_buf,Word input_row,JSample_t ** output_buf,int num_rows);
	static void BURGERCALL YCCKCMYKConvert(CColorDeconverter *ThisPtr,JSample_t *** input_buf,Word input_row,JSample_t ** output_buf,int num_rows);
	static void BURGERCALL Bogus(CColorDeconverter *ThisPtr,JSample_t *** input_buf,Word input_row,JSample_t ** output_buf,int num_rows);
	inline Fixed32 *GetCrRTable(void) { return &m_TablePtr[(MAXJSAMPLE+1)*0]; }
	inline Fixed32 *GetCbBTable(void) { return &m_TablePtr[(MAXJSAMPLE+1)*1]; }
	inline Fixed32 *GetCrGTable(void) { return &m_TablePtr[(MAXJSAMPLE+1)*2]; }
	inline Fixed32 *GetCbGTable(void) { return &m_TablePtr[(MAXJSAMPLE+1)*3]; }
	CJPegDecompress *m_Parent;		/* Parent dispatcher */
	Fixed32 *m_TablePtr;			/* Tables */
	void (BURGERCALL *m_ColorConvert)(CColorDeconverter *ThisPtr,JSample_t ***input_buf, Word input_row,JSample_t ** output_buf,int num_rows);
};

}
#endif
