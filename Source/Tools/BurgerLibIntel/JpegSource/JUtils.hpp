/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman

*******************************/

#ifndef __JUTILS_HPP__
#define __JUTILS_HPP__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __JPEG70TYPES_H__
#include "JPeg70Types.h"
#endif

namespace JPeg70 {

extern const Word NaturalOrder[];		/* zigzag coef order to natural order */
extern Word32 BURGERCALL DivRoundUp(Word32 a,Word32 b);
extern Word32 BURGERCALL RoundUp(Word32 a,Word32 b);
extern void BURGERCALL CopySampleRows(JSample_t **input_array,JSample_t **output_array,int Height,Word Width);

}
#endif
