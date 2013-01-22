/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#ifndef __JERROR_H__
#define __JERROR_H__

#ifndef __JPEG70TYPES_H__
#include "JPeg70Types.h"
#endif

#ifndef __cplusplus
#error Requires C++ compilation
#endif

namespace JPeg70 {

extern const char *const StandardMessageTable[JMSG_LASTMSGCODE];

}

#endif
