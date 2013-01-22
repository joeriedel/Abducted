/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#ifndef __JIDCTINT_HPP__
#define __JIDCTINT_HPP__

#ifndef __JPEG70TYPES_H__
#include "JPEG70TYPES.h"
#endif

class CJPegDecompress;

namespace JPeg70 {

extern void BURGERCALL IDCTISlow(CJPegDecompress *cinfo,ComponentInfo_t *compptr,SWord16 * coef_block,JSample_t ** output_buf,Word output_col);

}

#endif
