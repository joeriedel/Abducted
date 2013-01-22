/*******************************

	Burger's Universal library WIN95 version
	This is for Watcom 10.5 and higher...
	Also support for MSVC 4.0

*******************************/

#ifndef __MSDOS_H__
#define __MSDOS_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#if defined(__MSDOS__)

#ifdef __cplusplus
extern "C" {
#endif

/*********************************

	This is used by the x32 dos extender so I can address
	low memory in an IBM PC

*********************************/

extern void BURGERCALL MCGAOn(void);
extern void BURGERCALL MCGAOff(void);
extern Word BURGERCALL SVGAOn(void);
extern void BURGERCALL UpdateSVGA(void *Offscreen);

extern void *_x32_zero_base_ptr;
extern Word16 _x32_zero_base_selector;
#define ZeroBase ((Word8 *)_x32_zero_base_ptr)

typedef struct Regs16 {
	Word16 ax;
	Word16 bx;
	Word16 cx;
	Word16 dx;
	Word16 si;
	Word16 di;
	Word16 bp;
	Word16 ds;
	Word16 es;
	Word16 flags;
} Regs16;

extern int BURGERCALL Int86x(Word InterNum,Regs16 *InRegs,Regs16 *OutRegs);
extern int BURGERCALL Call86(Word32 Address,Regs16 *InRegs,Regs16 *OutRegs);
extern Word32 BURGERCALL AllocRealMemory(Word32 Size);
extern void BURGERCALL DeallocRealMemory(Word32 RealPtr);
extern void * BURGERCALL RealToProtectedPtr(Word32 RealPtr);
extern Word32 BURGERCALL GetRealBufferPtr(void);
extern void * BURGERCALL GetRealBufferProtectedPtr(void);
extern void CallInt10(Word EAX);
extern Word Int14(Word EAX,Word EDX);
extern Word Int17(Word EAX,Word EDX);
extern void SetBothInts(Word IrqNum,void far *CodePtr);
extern void SetProtInt(Word IrqNum,void far *CodePtr);
extern void SetRealInt(Word IrqNum,Word32 CodePtr);
extern void far *GetProtInt(Word IrqNum);
extern Word32 GetRealInt(Word IrqNum);
extern void *MapPhysicalAddress(void *Input,Word32 Length);

#ifdef __cplusplus
}
#endif

#endif
#endif
