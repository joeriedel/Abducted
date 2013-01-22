/*******************************

	Debugging manager
	
*******************************/

#ifndef __BRDEBUG_HPP__
#define __BRDEBUG_HPP__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#if _DEBUG
#define BRASSERT(f) if (!(f)) { Fatal("Assertion at %s, %i", __FILE__, __LINE__);}
#else
#define BRASSERT(f)
#endif

#endif
