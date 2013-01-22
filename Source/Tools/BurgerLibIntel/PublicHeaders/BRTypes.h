/*******************************

	This header contains the base types
	used in all of Burgerlib. It will also
	try to determine via compiler defines 
	exactly what features are supported in
	the compiler and what target OS you're
	building for.

*******************************/

#ifndef __BRTYPES_H__
#define __BRTYPES_H__

/* Watcom C 11.0 -> Wintel and MSDOS */

#if defined(__WATCOMC__)
#define __INTEL__		/* INTEL CPU */
#if defined(__DOS__)	/* Targeting MS-DOS? (-bt=DOS) */
#define __MSDOS__
#elif defined(__NT__)	/* Targeting WIN32? (-bt=NT) */
#define __WIN32__
#else
#error Watcom is not set with the proper bt=??? parameter. Add bt=NT to your WCC386 call
#endif

/* MrC for PowerPC (Classic MacOS) */

#elif defined(__MRC__)
#define __MAC__			/* __POWERPC__ is already defined */

/* GCC for PS2 (SN Systems) */

#elif defined(__GNUC__) && defined(__R5900__)
#define __PS2__
#define __MIPS__		/* __R5900__ is already defined */
#define longw64 long

/* GCC for GameCube (SN Systems) */

#elif defined(__GNUC__) && defined(__PPC__)
#define __GAMECUBE__
#define __POWERPC__

/* Visual C 7.0 for XBox (Requires a define) */

#elif defined(XBOX)
#define __XBOX__
#define __INTEL__
#define longw64 __int64

/* Metrowerks Codewarrior -> MacOS, MacOSX, Win32 */

#elif defined(__MWERKS__)	/* Codewarrior? Mac/BeOS/Win95 */
#if macintosh				/* MacOS? */
#define __MAC__
#if !defined(__POWERPC__)
#define __68K__				/* MacOS 68k */
extern double __fabs(double x);		/* Hack for a bad Metrowerks header 8.3 */
#endif

#elif defined(__MACH__)		/* MacOSX? */
#define __MACOSX__

#elif !defined(__be_os) || (__be_os != __dest_os) /* Metrowerks for Win95 */
#define __WIN32__
#define __INTEL__

#else						/* BeOS */
#define __BEOS__
#if !defined(__POWERPC__)
#define __INTEL__
#endif
#endif

/* External define for Project builder */

#elif defined(__MACOSX__)		/* MacOSX */
#define __POWERPC__

/* Assume a Visual Studio compatible target */

#elif defined(_MSC_VER)				/* I assume Microsquish C++ 5.0 */
#define ANSICALL __cdecl
#define BURGERCALL __fastcall		/* Use fastcall parms for MSVC++ */
#define __WIN32__
#define __INTEL__
#define LONGW64 __int64
#else
#error Not supported yet
#endif

/* Handle defaults */

#if !defined(BURGERCALL)
#define BURGERCALL
#endif

#if !defined(ANSICALL)
#define ANSICALL
#endif

#if !defined(BURGERMAXINT)
#define BURGERMAXINT 0x7FFFFFFF
#endif

#if !defined(LONGW64)
#define LONGW64 long long
#endif

/* Define the INLINE keyword for ANSI "C" compilers that support it */

#ifndef INLINECALL
#if defined(__cplusplus) || defined(__GNUC__) || defined(__MWERKS__)
#define INLINECALL inline		/* Use the C++ standard keyword */
#elif defined(__WIN32__) && !defined(__WATCOMC__)
#define INLINECALL __inline		/* Keyword for Visual C 6.0 */
#else
#define INLINECALL				/* Not supported */
#endif
#endif

/* Little endian machine? */

#if defined(__INTEL__) || defined(__PS2__)
#define __LITTLEENDIAN__
#else
#define __BIGENDIAN__
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

#undef NDEBUG
#if !defined(_DEBUG) && (defined(__MWERKS__) || defined(__MRC__))
#if __option(sym)	/* Ask code warrior about it */
#define _DEBUG 1
#endif
#endif

#ifndef _DEBUG
#define _DEBUG 0
#endif

#if !_DEBUG
#define NDEBUG 1	/* Do not allow assert.h to make code */
#endif

typedef signed char SWord8;
typedef unsigned char Word8;
typedef short SWord16;
typedef unsigned short Word16;
#if defined(__PS2__)
typedef int SWord32;
typedef unsigned int Word32;
#else
typedef long SWord32;
typedef unsigned long Word32;
#endif
typedef LONGW64 SWord64;
typedef unsigned LONGW64 Word64;
#undef LONGW64

#if defined(__PS2__)
typedef signed int SWord128 __attribute__((mode (TI)));
typedef unsigned int Word128 __attribute__((mode (TI)));
#else
typedef SWord64 SWord128[2];
typedef Word64 Word128[2];
#endif

typedef Word8 Bool;
typedef SWord32 Frac32;
typedef SWord32 Fixed32;
typedef unsigned int Word;

#endif
