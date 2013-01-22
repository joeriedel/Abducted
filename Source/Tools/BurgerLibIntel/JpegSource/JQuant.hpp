/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#ifndef __JQUANT_HPP__
#define __JQUANT_HPP__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __JPEG70TYPES_H__
#include "JPeg70Types.h"
#endif

class CJPegDecompress;

namespace JPeg70 {

/* Color quantization or color precision reduction */
class CColorQuantizer {
public:
	CColorQuantizer(CJPegDecompress *CInfoPtr);
	virtual ~CColorQuantizer();
	virtual void BURGERCALL StartPass(Word is_pre_scan);
	inline void BURGERCALL ColorQuantize(JSample_t ** input_buf,JSample_t **output_buf,int num_rows) { m_ColorQuantize(this,input_buf,output_buf,num_rows); }
	virtual void BURGERCALL FinishPass(void);
	virtual void BURGERCALL NewColorMap(void);
	void (BURGERCALL *m_ColorQuantize)(CColorQuantizer *ThisPtr,JSample_t **input_buf,JSample_t **output_buf,int num_rows);
	CJPegDecompress *m_Parent;		/* Parent dispatcher */
private:
	static void BURGERCALL Dormant(CColorQuantizer *ThisPtr,JSample_t **input_buf,JSample_t ** output_buf,int num_rows);
};

}
#endif
