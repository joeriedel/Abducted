/*******************************

	PowerPC inlines

*******************************/

#ifndef __BRENDIANPPC_HPP__
#define __BRENDIANPPC_HPP__

#ifndef __BRENDIAN_HPP__
#error Must be included from BREndian.hpp only!
#endif

namespace Burger {
inline Word16 BURGERCALL SwapEndian(Word16 a);
inline SWord16 BURGERCALL SwapEndian(SWord16 a);
inline Word32 BURGERCALL SwapEndian(Word32 a);
inline float BURGERCALL SwapEndian(float a);
inline double BURGERCALL SwapEndian(double a);
inline Word16 BURGERCALL SwapEndian(const Word16 *a);
inline SWord16 BURGERCALL SwapEndian(const SWord16 *a);
inline Word32 BURGERCALL SwapEndian(const Word32 *a);
inline float BURGERCALL SwapEndian(const float *a);
inline double BURGERCALL SwapEndian(const double *a);
inline void BURGERCALL SwapEndianMem(Word16 *a);
inline void BURGERCALL SwapEndianMem(Word32 *a);
inline void BURGERCALL SwapEndianMem(double *a);

inline Word16 BURGERCALL SwapEndian(Word16 a)
{
	return __lhbrx(&a,0);
}

inline SWord16 BURGERCALL SwapEndian(SWord16 a)
{
	return __lhbrx(&a,0);
}

inline Word32 BURGERCALL SwapEndian(Word32 a)
{
	return __lwbrx(&a,0);
}

inline float BURGERCALL SwapEndian(float a)
{
	float f;
	reinterpret_cast<Word32 *>(&f)[0] = __lwbrx(reinterpret_cast<Word32 *>(&a),0);
	return f;
}

inline double BURGERCALL SwapEndian(double a)
{
	double d;
	reinterpret_cast<Word32 *>(&d)[0] = __lwbrx(const_cast<Word32 *>(reinterpret_cast<const Word32 *>(&a)+1),0);
	reinterpret_cast<Word32 *>(&d)[1] = __lwbrx(const_cast<Word32 *>(reinterpret_cast<const Word32 *>(&a)),0);
	return d;
}

inline Word16 BURGERCALL SwapEndian(const Word16 *a)
{
	return __lhbrx(const_cast<Word16 *>(a),0);
}

inline SWord16 BURGERCALL SwapEndian(const SWord16 *a)
{
	return __lhbrx(const_cast<SWord16 *>(a),0);
}

inline Word32 BURGERCALL SwapEndian(const Word32 *a)
{
	return __lwbrx(const_cast<Word32 *>(a),0);
}

inline float BURGERCALL SwapEndian(const float *a)
{
	float f;
	reinterpret_cast<Word32 *>(&f)[0] = __lwbrx(const_cast<Word32 *>(reinterpret_cast<const Word32 *>(a)),0);
	return f;
}

inline double BURGERCALL SwapEndian(const double *a)
{
	double d;
	reinterpret_cast<Word32 *>(&d)[0] = __lwbrx(const_cast<Word32 *>(reinterpret_cast<const Word32 *>(a)+1),0);
	reinterpret_cast<Word32 *>(&d)[1] = __lwbrx(const_cast<Word32 *>(reinterpret_cast<const Word32 *>(a)),0);
	return d;
}

inline void BURGERCALL SwapEndianMem(Word16 *a)
{
	a[0] = __lhbrx(a,0);
}

inline void BURGERCALL SwapEndianMem(Word32 *a)
{
	a[0] = __lwbrx(a,0);
}

inline void BURGERCALL SwapEndianMem(double *a)
{
	Word32 b;
	b = __lwbrx(reinterpret_cast<Word32 *>(a)+1,0);
	reinterpret_cast<Word32 *>(a)[1] = __lwbrx(reinterpret_cast<Word32 *>(a),0);
	reinterpret_cast<Word32 *>(a)[0] = b;
}

}
#endif
