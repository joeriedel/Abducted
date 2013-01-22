/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#ifndef __JFDCTFST_HPP__
#define __JFDCTFST_HPP__

#ifndef __JPEG70TYPES_H__
#include "JPEG70TYPES.h"
#endif

namespace JPeg70 {

extern void BURGERCALL FDCTIFast(SWord32 * data);

}

#endif
