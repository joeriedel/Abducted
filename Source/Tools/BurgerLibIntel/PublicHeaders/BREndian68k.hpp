/*******************************

	680x0 inlines

*******************************/

#ifndef __BRENDIAN68K_HPP__
#define __BRENDIAN68K_HPP__

#ifndef __BRENDIAN_HPP__
#error Must be included from BREndian.hpp only!
#endif

namespace Burger {

inline Word16 BURGERCALL SwapEndian(Word16:__D0):__D0 =
{
	0xE058		/* ROR.W #8,D0 */
};

inline SWord16 BURGERCALL SwapEndian(SWord16:__D0):__D0 =
{
	0xE058		/* ROR.W #8,D0 */
};

inline Word32 BURGERCALL SwapEndian(Word32:__D0):__D0 =
{
	0xE058,		/* ROR.W #8,D0 */
	0x4840,		/* SWAP D0 */
	0xE058		/* ROR.W #8,D0 */
};

inline float BURGERCALL SwapEndian(float a)
{
	Word32 b;
	b = SwapEndian(reinterpret_cast<Word32*>(&a)[0]);
	return reinterpret_cast<float*>(&b)[0];
}

inline double BURGERCALL SwapEndian(double a)
{
	Word32 b;
	b = SwapEndian(reinterpret_cast<Word32*>(&a)[0]);
	reinterpret_cast<Word32 *>(&a)[0] = SwapEndian(reinterpret_cast<Word32*>(&a)[1]);
	reinterpret_cast<Word32 *>(&a)[1] = b;
	return a;
}

inline Word16 BURGERCALL SwapEndian(const Word16 *a:__a1):__D0 = 
{
	0x3011,		/* MOVE.W (a1),D0 */
	0xE058		/* ROR.W #8,D0 */
};

inline SWord16 BURGERCALL SwapEndian(const SWord16 *a:__a1):__D0 =
{
	0x3011,		/* MOVE.W (a1),D0 */
	0xE058		/* ROR.W #8,D0 */
};

inline Word32 BURGERCALL SwapEndian(const Word32 *a:__a1):__D0 = 
{
	0x3011,		/* MOVE.W (a1),D0 */
	0xE058,		/* ROR.W #8,D0 */
	0x4840,		/* SWAP D0 */
	0xE058		/* ROR.W #8,D0 */
};

inline float BURGERCALL SwapEndian(const float *a)
{
	float b;
	reinterpret_cast<Word32 *>(&b)[0] = SwapEndian(reinterpret_cast<const Word32*>(a)[0]);
	return b;
}

inline double BURGERCALL SwapEndian(const double *a)
{
	Word32 b;
	double c;
	b = SwapEndian(reinterpret_cast<const Word32*>(a)[0]);
	reinterpret_cast<Word32 *>(&c)[0] = SwapEndian(reinterpret_cast<const Word32*>(a)[1]);
	reinterpret_cast<Word32 *>(&c)[1] = b;
	return c;
}

inline void BURGERCALL SwapEndianMem(Word16 *a)
{
	a[0] = SwapEndian(a[0]);
}

inline void BURGERCALL SwapEndianMem(Word32 *a)
{
	a[0] = SwapEndian(a[0]);
}

inline void BURGERCALL SwapEndianMem(double *a)
{
	Word32 hi,lo;
	lo = reinterpret_cast<Word32 *>(a)[0];
	hi = reinterpret_cast<Word32 *>(a)[1];
	lo = SwapEndian(lo);
	hi = SwapEndian(hi);
	reinterpret_cast<Word32 *>(a)[1] = lo;
	reinterpret_cast<Word32 *>(a)[0] = hi;
}

}

#endif
