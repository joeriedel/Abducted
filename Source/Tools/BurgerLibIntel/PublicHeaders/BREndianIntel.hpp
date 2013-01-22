/*******************************

	Intel inlines

*******************************/

#ifndef __BRENDIANINTEL_HPP__
#define __BRENDIANINTEL_HPP__

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

/*******************************

	Metrowerks for Intel

*******************************/

#if defined(__MWERKS__)

inline Word16 BURGERCALL SwapEndian(Word16 a)
{
	register Word16 Temp;
	Temp = a;
	asm {
	ror		Temp,8
	}
	return Temp;
}

inline SWord16 BURGERCALL SwapEndian(SWord16 a)
{
	register SWord16 Temp;
	Temp = a;
	asm {
	ror		Temp,8
	}
	return Temp;
}

inline Word32 BURGERCALL SwapEndian(Word32 a)
{
	register Word32 Temp;
	Temp = a;
	asm {
	bswap	Temp
	}
	return Temp;
}

inline float BURGERCALL SwapEndian(float a)
{
	register Word32 Temp;
	float FTemp;
	Temp = reinterpret_cast<Word32 *>(&a)[0];
	asm {
	bswap	Temp;
	}
	reinterpret_cast<Word32 *>(&FTemp)[0] = Temp;
	return FTemp;
}

inline double BURGERCALL SwapEndian(double a)
{
	register Word32 Temp1,Temp2;
	double DTemp;
	Temp1 = reinterpret_cast<const Word32 *>(&a)[0];
	Temp2 = reinterpret_cast<const Word32 *>(&a)[1];
	asm {
	bswap	Temp1
	bswap	Temp2
	}
	reinterpret_cast<Word32 *>(&DTemp)[0] = Temp2;
	reinterpret_cast<Word32 *>(&DTemp)[1] = Temp1;
	return DTemp;
}

inline Word16 BURGERCALL SwapEndian(const Word16 *a)
{
	register Word16 Temp;
	Temp = a[0];
	asm {
	ror		Temp,8
	}
	return Temp;
}

inline SWord16 BURGERCALL SwapEndian(const SWord16 *a)
{
	register SWord16 Temp;
	Temp = a[0];
	asm {
	ror		Temp,8
	}
	return Temp;
}

inline Word32 BURGERCALL SwapEndian(const Word32 *a)
{
	register Word32 Temp;
	Temp = a[0];
	asm {
		bswap Temp
	}
	return Temp;
}

inline float BURGERCALL SwapEndian(const float *a)
{
	register Word32 Temp;
	float FTemp;
	Temp = reinterpret_cast<const Word32 *>(a)[0];
	asm {
	bswap	Temp
	}
	reinterpret_cast<Word32 *>(&FTemp)[0] = Temp;
	return FTemp;
}

inline double BURGERCALL SwapEndian(const double *a)
{
	register Word32 Temp1,Temp2;
	double DTemp;
	Temp1 = reinterpret_cast<const Word32 *>(a)[0];
	Temp2 = reinterpret_cast<const Word32 *>(a)[1];
	asm {
	bswap	Temp1
	bswap	Temp2
	}	
	reinterpret_cast<Word32 *>(&DTemp)[0] = Temp2;
	reinterpret_cast<Word32 *>(&DTemp)[1] = Temp1;
	return DTemp;
}

inline void BURGERCALL SwapEndianMem(Word16 *a)
{
	register Word16 Temp;
	Temp = a[0];
	asm {
	ror		Temp,8
	}
	a[0] = Temp;
}

inline void BURGERCALL SwapEndianMem(Word32 *a)
{
	register Word32 Temp;
	Temp = a[0];
	asm {
	bswap	Temp;
	}
	a[0] = Temp;
}

inline void BURGERCALL SwapEndianMem(double *a)
{
	register Word32 Temp1,Temp2;
	Temp1 = reinterpret_cast<Word32 *>(a)[0];
	Temp2 = reinterpret_cast<Word32 *>(a)[1];
	asm {
	bswap	Temp1;
	bswap	Temp2;
	}
	reinterpret_cast<Word32 *>(a)[0] = Temp2;
	reinterpret_cast<Word32 *>(a)[1] = Temp1;
}

/*******************************

	Open Watcom for Intel

*******************************/

#elif defined(__WATCOMC__)
}

/* Assembly MUST not be in a namespace */
extern "C" {
extern Word16 WatcomSwapShort(Word16 a);
extern Word32 WatcomSwapLong(Word32 a);
extern Word32 WatcomSwapLong2(Word32 a);

#pragma aux WatcomSwapLong = "BSWAP EAX" parm [eax] value [eax] modify exact [];
#pragma aux WatcomSwapLong2 = "BSWAP EDX" parm [edx] value [edx] modify exact [];
#pragma aux WatcomSwapShort = "ROR AX,8" parm [ax] value [ax] modify exact [];
}

namespace Burger {
inline Word16 SwapEndian(Word16 a)
{
	return WatcomSwapShort(a);
}

inline SWord16 SwapEndian(SWord16 a)
{
	return static_cast<SWord16>(WatcomSwapShort(static_cast<Word16>(a)));
}

inline Word32 SwapEndian(Word32 a)
{
	return WatcomSwapLong(a);
}

inline float SwapEndian(float a)
{
	float FTemp;
	reinterpret_cast<Word32 *>(&FTemp)[0] = WatcomSwapLong(reinterpret_cast<Word32 *>(&a)[0]);
	return FTemp;
}

inline double BURGERCALL SwapEndian(double a)
{
	register Word32 Temp1,Temp2;
	double DTemp;
	Temp1 = reinterpret_cast<const Word32 *>(&a)[0];
	Temp2 = reinterpret_cast<const Word32 *>(&a)[1];
	reinterpret_cast<Word32 *>(&DTemp)[1] = WatcomSwapLong(Temp1);
	reinterpret_cast<Word32 *>(&DTemp)[0] = WatcomSwapLong2(Temp2);
	return DTemp;
}

inline Word16 BURGERCALL SwapEndian(const Word16 *a)
{
	return WatcomSwapShort(a[0]);
}

inline SWord16 BURGERCALL SwapEndian(const SWord16 *a)
{
	return static_cast<SWord16>(WatcomSwapShort(static_cast<Word16>(a[0])));
}

inline Word32 BURGERCALL SwapEndian(const Word32 *a)
{
	return WatcomSwapLong(a[0]);
}

inline float BURGERCALL SwapEndian(const float *a)
{
	float FTemp;
	reinterpret_cast<Word32 *>(&FTemp)[0] = WatcomSwapLong(reinterpret_cast<const Word32 *>(a)[0]);
	return FTemp;
}

inline double BURGERCALL SwapEndian(const double *a)
{
	register Word32 Temp1,Temp2;
	double DTemp;
	Temp1 = reinterpret_cast<const Word32 *>(a)[0];
	Temp2 = reinterpret_cast<const Word32 *>(a)[1];
	reinterpret_cast<Word32 *>(&DTemp)[1] = WatcomSwapLong(Temp1);
	reinterpret_cast<Word32 *>(&DTemp)[0] = WatcomSwapLong2(Temp2);
	return DTemp;
}

inline void BURGERCALL SwapEndianMem(Word16 *a)
{
	a[0] = WatcomSwapShort(a[0]);
}

inline void BURGERCALL SwapEndianMem(Word32 *a)
{
	a[0] = WatcomSwapLong(a[0]);
}

inline void BURGERCALL SwapEndianMem(double *a)
{
	register Word32 Temp1,Temp2;
	Temp1 = reinterpret_cast<Word32 *>(a)[0];
	Temp2 = reinterpret_cast<Word32 *>(a)[1];
	reinterpret_cast<Word32 *>(a)[1] = WatcomSwapLong(Temp1);
	reinterpret_cast<Word32 *>(a)[0] = WatcomSwapLong2(Temp2);
}

/*******************************

	Intel compiler

*******************************/

#elif defined(__ICL) 

inline Word16 BURGERCALL SwapEndian(Word16 a)
{
	a = (a>>8)+(a<<8);
	return a;
}

inline SWord16 BURGERCALL SwapEndian(SWord16 a)
{
	a = (static_cast<Word16>(a)>>8)+(static_cast<Word16>(a)<<8);
	return a;
}

inline Word32 BURGERCALL SwapEndian(Word32 a)
{
	return _bswap(a);
}

inline float BURGERCALL SwapEndian(float a)
{
	reinterpret_cast<Word32 *>(&a)[0] = _bswap(reinterpret_cast<Word32*>(&a)[0]);
	return a;
}

inline double BURGERCALL SwapEndian(double a)
{
	Word32 b;
	b = _bswap(reinterpret_cast<Word32*>(&a)[0]);
	reinterpret_cast<Word32 *>(&a)[0] = _bswap(reinterpret_cast<Word32*>(&a)[1]);
	reinterpret_cast<Word32 *>(&a)[1] = b;
	return a;
}

inline Word16 BURGERCALL SwapEndian(const Word16 *a)
{
	Word16 b;
	b = a[0];
	b = (b>>8)+(b<<8);
	return b;
}

inline SWord16 BURGERCALL SwapEndian(const SWord16 *a)
{
	Word16 b;
	b = a[0];
	b = (static_cast<Word16>(b)>>8)+(static_cast<Word16>(b)<<8);
	return b;
}

inline Word32 BURGERCALL SwapEndian(const Word32 *a)
{
	return _bswap(a[0]);
}

inline float BURGERCALL SwapEndian(const float *a)
{
	float b;
	reinterpret_cast<Word32 *>(&b)[0] = _bswap(reinterpret_cast<const Word32*>(a)[0]);
	return b;
}

inline double BURGERCALL SwapEndian(const double *a)
{
	Word32 b;
	double c;
	b = _bswap(reinterpret_cast<const Word32*>(a)[0]);
	reinterpret_cast<Word32 *>(&c)[0] = _bswap(reinterpret_cast<const Word32*>(a)[1]);
	reinterpret_cast<Word32 *>(&c)[1] = b;
	return c;
}

inline void BURGERCALL SwapEndianMem(Word16 *a)
{
	Word8 b,c;
	b = reinterpret_cast<Word8*>(a)[0];
	c = reinterpret_cast<Word8*>(a)[1];
	reinterpret_cast<Word8*>(a)[1] = b;
	reinterpret_cast<Word8*>(a)[0] = c;
}

inline void BURGERCALL SwapEndianMem(Word32 *a)
{
	a[0] = _bswap(a[0]);
}

inline void BURGERCALL SwapEndianMem(double *a)
{
	Word32 b;
	b = _bswap(reinterpret_cast<Word32*>(a)[0]);
	reinterpret_cast<Word32 *>(a)[0] = _bswap(reinterpret_cast<Word32*>(a)[1]);
	reinterpret_cast<Word32 *>(a)[1] = b;
}

#else
#error Unsupported Intel compiler, bug burger@contrabandent.com for the code
#endif

}

#endif
