/*******************************

	Handle endian swapping

*******************************/

#ifndef __BRENDIAN_H__
#define __BRENDIAN_H__

#ifndef __cplusplus
#error Requires C++ compilation
#endif

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __BRENDIAN_HPP__
#include <BREndian.hpp>
#endif

using Burger::SwapEndian;
using Burger::SwapEndianMem;
using Burger::LoadLittle;
using Burger::LoadBig;

#endif
