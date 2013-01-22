/*******************************

	Copyright (C) 1991-1998, Thomas G. Lane.
	This file is part of the Independent JPEG Group's software.
	For conditions of distribution and use, see the accompanying README file.

	Alterations (C) 2003, Bill Heineman
	
	This file has the base ColorQuantizer class. It
	it just a placeholder, since the real work is
	done in the derived classes

*******************************/

#include "JQuant.hpp"
#include "jpeglib.h"

namespace JPeg70 {

/*******************************

	Default constructor

*******************************/

CColorQuantizer::CColorQuantizer(CJPegDecompress *CInfoPtr)
	: m_Parent(CInfoPtr),
	m_ColorQuantize(Dormant)
{
}

/*******************************

	Default destructor

*******************************/

CColorQuantizer::~CColorQuantizer()
{
}

/*******************************

	Default start pass function

*******************************/

void BURGERCALL CColorQuantizer::StartPass(Word)
{
}

/*******************************

	Default finish pass function

*******************************/

void BURGERCALL CColorQuantizer::FinishPass(void)
{
}

/*******************************

	Default new color map function

*******************************/

void BURGERCALL CColorQuantizer::NewColorMap(void)
{
	m_Parent->FatalError(JERR_MODE_CHANGE);		/* Shouldn't be called on a 1 pass filter */
}

/*******************************

	Default Color function in case of a fatal error.
	This way, the code will run, but do nothing

*******************************/

void BURGERCALL CColorQuantizer::Dormant(CColorQuantizer *,JSample_t **,JSample_t **,int)
{
}

}