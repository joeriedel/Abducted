/*******************************

	Handle endian swapping

*******************************/

#ifndef __BRENDIAN_HPP__
#define __BRENDIAN_HPP__

#ifndef __cplusplus
#error Requires C++ compilation
#endif

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#if defined(__68K__) && defined(__MWERKS__)
#include <BREndian68k.hpp>
#elif defined(__POWERPC__) && defined(__MWERKS__)
#include <BREndianPPC.hpp>
#elif defined(__INTEL__) && (defined(__WATCOMC__) || defined(__MWERKS__) || defined(__ICL))
#include <BREndianIntel.hpp>
#else
namespace Burger {
extern Word16 BURGERCALL SwapEndian(Word16 a);
extern SWord16 BURGERCALL SwapEndian(SWord16 a);
extern Word32 BURGERCALL SwapEndian(Word32 a);
extern float BURGERCALL SwapEndian(float a);
extern double BURGERCALL SwapEndian(double a);
extern Word16 BURGERCALL SwapEndian(const Word16 *a);
extern SWord16 BURGERCALL SwapEndian(const SWord16 *a);
extern Word32 BURGERCALL SwapEndian(const Word32 *a);
extern float BURGERCALL SwapEndian(const float *a);
extern double BURGERCALL SwapEndian(const double *a);
extern void BURGERCALL SwapEndianMem(Word16 *a);
extern void BURGERCALL SwapEndianMem(Word32 *a);
extern void BURGERCALL SwapEndianMem(double *a);
}
#endif

namespace Burger {

inline SWord32 BURGERCALL SwapEndian(SWord32 a) { return static_cast<SWord32>(SwapEndian(static_cast<Word32>(a))); }
inline SWord32 BURGERCALL SwapEndian(const SWord32 *a) { return static_cast<SWord32>(SwapEndian(reinterpret_cast<const Word32 *>(a))); }
inline void BURGERCALL SwapEndianMem(SWord16 *a) { SwapEndianMem(reinterpret_cast<Word16 *>(a)); }
inline void BURGERCALL SwapEndianMem(SWord32 *a) { SwapEndianMem(reinterpret_cast<Word32 *>(a)); }
inline void BURGERCALL SwapEndianMem(float *a) { SwapEndianMem(reinterpret_cast<Word32 *>(a)); }

#if defined(__LITTLEENDIAN__)
inline Word16 BURGERCALL LoadLittle(Word16 a) { return a; }
inline SWord16 BURGERCALL LoadLittle(SWord16 a) { return a; }
inline Word32 BURGERCALL LoadLittle(Word32 a) { return a; }
inline SWord32 BURGERCALL LoadLittle(SWord32 a) { return a; }
inline float BURGERCALL LoadLittle(float a) { return a; }
inline double BURGERCALL LoadLittle(double a) { return a; }
inline Word16 BURGERCALL LoadLittle(const Word16*a) { return a[0]; }
inline SWord16 BURGERCALL LoadLittle(const SWord16*a) { return a[0]; }
inline Word32 BURGERCALL LoadLittle(const Word32*a) { return a[0]; }
inline SWord32 BURGERCALL LoadLittle(const SWord32 *a) { return a[0]; }
inline float BURGERCALL LoadLittle(const float *a) { return a[0]; }
inline double BURGERCALL LoadLittle(const double *a) { return a[0]; }
inline Word16 BURGERCALL LoadBig(Word16 a) { return SwapEndian(a); }
inline SWord16 BURGERCALL LoadBig(SWord16 a) { return SwapEndian(a); }
inline Word32 BURGERCALL LoadBig(Word32 a) { return SwapEndian(a); }
inline SWord32 BURGERCALL LoadBig(SWord32 a) { return SwapEndian(a); }
inline float BURGERCALL LoadBig(float a) { return SwapEndian(a); }
inline double BURGERCALL LoadBig(double a) { return SwapEndian(a); }
inline Word16 BURGERCALL LoadBig(const Word16*a) { return SwapEndian(a); }
inline SWord16 BURGERCALL LoadBig(const SWord16*a) { return SwapEndian(a); }
inline Word32 BURGERCALL LoadBig(const Word32*a) { return SwapEndian(a); }
inline SWord32 BURGERCALL LoadBig(const SWord32 *a) { return SwapEndian(a); }
inline float BURGERCALL LoadBig(const float *a) { return SwapEndian(a); }
inline double BURGERCALL LoadBig(const double *a) { return SwapEndian(a); }
#else
inline Word16 BURGERCALL LoadBig(Word16 a) { return a; }
inline SWord16 BURGERCALL LoadBig(SWord16 a) { return a; }
inline Word32 BURGERCALL LoadBig(Word32 a) { return a; }
inline SWord32 BURGERCALL LoadBig(SWord32 a) { return a; }
inline float BURGERCALL LoadBig(float a) { return a; }
inline double BURGERCALL LoadBig(double a) { return a; }
inline Word16 BURGERCALL LoadBig(const Word16 *a) { return a[0]; }
inline SWord16 BURGERCALL LoadBig(const SWord16 *a) { return a[0]; }
inline Word32 BURGERCALL LoadBig(const Word32 *a) { return a[0]; }
inline SWord32 BURGERCALL LoadBig(const SWord32 *a) { return a[0]; }
inline float BURGERCALL LoadBig(const float *a) { return a[0]; }
inline double BURGERCALL LoadBig(const double *a) { return a[0]; }
inline Word16 BURGERCALL LoadLittle(Word16 a) { return SwapEndian(a); }
inline SWord16 BURGERCALL LoadLittle(SWord16 a) { return SwapEndian(a); }
inline Word32 BURGERCALL LoadLittle(Word32 a) { return SwapEndian(a); }
inline SWord32 BURGERCALL LoadLittle(SWord32 a) { return SwapEndian(a); }
inline float BURGERCALL LoadLittle(float a) { return SwapEndian(a); }
inline double BURGERCALL LoadLittle(double a) { return SwapEndian(a); }
inline Word16 BURGERCALL LoadLittle(const Word16 *a) { return SwapEndian(a); }
inline SWord16 BURGERCALL LoadLittle(const SWord16 *a) { return SwapEndian(a); }
inline Word32 BURGERCALL LoadLittle(const Word32 *a) { return SwapEndian(a); }
inline SWord32 BURGERCALL LoadLittle(const SWord32 *a) { return SwapEndian(a); }
inline float BURGERCALL LoadLittle(const float *a) { return SwapEndian(a); }
inline double BURGERCALL LoadLittle(const double *a) { return SwapEndian(a); }
#endif

}

#endif
