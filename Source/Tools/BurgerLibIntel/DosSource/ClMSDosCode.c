/**********************************

	Misc standard lib functions

**********************************/

#include <BRTypes.h>

#if defined(__MSDOS__) || defined(__WATCOMC__)
#include "MSDos.h"
#include "ClStdLib.h"

/**********************************

	This routine will allow a DOS application to call a real mode procedure
	routine via the current DOS extender.

	This code is for the X32 dos extender ONLY

**********************************/


#if defined(__MSDOS__)
extern void CallMe(Word32 Address);

#pragma aux CallMe = /* Invoke the X32 call proc routine */ \
	"PUSH EBP" \
	"MOV EAX,0x250EH"	\
	"XOR ECX,ECX" \
	"INT 021H" \
	"POP EBP" \
	parm [ebx] \
	modify [eax ebx ecx edx esi edi]

static Word8 RealCode[] = {
	0xB8,0x00,0x01,		// MOV AX,0100 (0)
	0x8E,0xD8,			// MOV DS,AX (3)
	0xB8,0x00,0x01,		// MOV AX,0100 (5)
	0x8E,0xC0,			// MOV ES,AX (8)
	0xB8,0x00,0x01,		// MOV AX,0100 (10)
	0xBB,0x00,0x01,		// MOV BX,0100 (13)
	0xB9,0x00,0x01,		// MOV CX,0100 (16)
	0xBA,0x00,0x01,		// MOV DX,0100 (19)
	0xBF,0x00,0x01,		// MOV DI,0100 (22)
	0xBE,0x00,0x01,		// MOV SI,0100 (25)
	0xBD,0x00,0x01,		// MOV BP,0100 (28)
	0x9A,0x33,0x12,0x34,0x12,	// CALL 1234:1233 (31)
	0x2E,				// CS: (36)
	0x8C,0x1E,0x00,0x01,	// MOV [0100],DS (37)
	0x2E,				// CS: (41)
	0x8C,0x06,0x02,0x01,	// MOV [0102],ES (42)
	0x2E,				// CS: (46)
	0xA3,0x00,0x01,		// MOV [100],AX (47)
	0x2E,				// CS: (50)
	0x89,0x1E,0x00,0x01,	// MOV [0100],BX (51)
	0x2E,					// CS: (55)
	0x89,0x0E,0x00,0x01,    // MOV [0100],CX
	0x2E,					// CS: (60)
	0x89,0x16,0x00,0x01,    // MOV [0100],DX
	0x2E,					// CS: (65)
	0x89,0x3E,0x00,0x01,    // MOV [0100],DI
	0x2E,					// CS: (70)
	0x89,0x36,0x00,0x01,    // MOV [0100],SI
	0x2E,			   		// CS: (75)
	0x89,0x2E,0x00,0x01,	// MOV [0100],BP
	0x9C,				// PUSHF (80)
	0x58,				// POP AX (81)
	0x2E,				// CS: (82)
	0xA3,0x00,0x01,		// MOV [100],AX (83)
	0xCB					// RETF (86)
};

int Call86(Word32 Address,Regs16 *InRegs,Regs16 *OutRegs)
{
	Word32 TempMem;
	Word8 *TrueMem;

	TempMem = AllocRealMemory(120);		/* Get real memory */
	if (TempMem) {
		TrueMem = (Word8 *)RealToProtectedPtr(TempMem);
		FastMemCpy(TrueMem,RealCode,sizeof(RealCode));

		((Word16 *)(TrueMem+1))[0] = InRegs->ds;		/* Pass the input registers */
		((Word16 *)(TrueMem+6))[0] = InRegs->es;
		((Word16 *)(TrueMem+11))[0] = InRegs->ax;
		((Word16 *)(TrueMem+14))[0] = InRegs->bx;
		((Word16 *)(TrueMem+17))[0] = InRegs->cx;
		((Word16 *)(TrueMem+20))[0] = InRegs->dx;
		((Word16 *)(TrueMem+23))[0] = InRegs->di;
		((Word16 *)(TrueMem+26))[0] = InRegs->si;
		((Word16 *)(TrueMem+29))[0] = InRegs->bp;
		((Word32 *)(TrueMem+32))[0] = Address;

		((Word16 *)(TrueMem+39))[0] = (Word16)TempMem+100;	/* Set the return data */
		((Word16 *)(TrueMem+44))[0] = (Word16)TempMem+102;
		((Word16 *)(TrueMem+48))[0] = (Word16)TempMem+104;
		((Word16 *)(TrueMem+53))[0] = (Word16)TempMem+106;
		((Word16 *)(TrueMem+58))[0] = (Word16)TempMem+108;
		((Word16 *)(TrueMem+63))[0] = (Word16)TempMem+110;
		((Word16 *)(TrueMem+68))[0] = (Word16)TempMem+112;
		((Word16 *)(TrueMem+73))[0] = (Word16)TempMem+114;
		((Word16 *)(TrueMem+78))[0] = (Word16)TempMem+116;
		((Word16 *)(TrueMem+84))[0] = (Word16)TempMem+118;
		CallMe(TempMem);		/* Call it */

		OutRegs->ds = ((Word16 *)(TrueMem+100))[0];	/* Get the result */
		OutRegs->es = ((Word16 *)(TrueMem+102))[0];
		OutRegs->ax = ((Word16 *)(TrueMem+104))[0];
		OutRegs->bx = ((Word16 *)(TrueMem+106))[0];
		OutRegs->cx = ((Word16 *)(TrueMem+108))[0];
		OutRegs->dx = ((Word16 *)(TrueMem+110))[0];
		OutRegs->di = ((Word16 *)(TrueMem+112))[0];
		OutRegs->si = ((Word16 *)(TrueMem+114))[0];
		OutRegs->bp = ((Word16 *)(TrueMem+116))[0];
		OutRegs->flags = ((Word16 *)(TrueMem+118))[0];
		DeallocRealMemory(TempMem);		/* Release the memory */
		return OutRegs->ax;
	}
	FastMemSet(OutRegs,0,sizeof(Regs16));
	OutRegs->flags = 1;
	return 0;
}

#endif

/**********************************

	Perform the ANSI function strlen()
	This is in optimized Pentium assembly
	The algorithm is explained in the generic "C" version

**********************************/

extern size_t strlenasm(const char *);
#pragma aux strlenasm = \
	"push ebx" \
	"mov ebx,eax" \
	"and eax,3" \
	"push ecx" \
	"neg eax" \
	"push edx" \
	"jz LL" \
	"mov edx,0xFFFFFFFF" \
	"lea ecx,[32+eax*8]" \
	"shr edx,cl" \
	"mov ecx,[ebx+eax]" \
	"add eax,4" \
	"or ecx,edx" \
	"mov edx,ecx" \
	"xor ecx,0xFFFFFFFF" \
	"sub edx,0x01010101" \
	"and ecx,0x80808080" \
	"test ecx,edx" \
	"jnz L0" \
"LL:" \
	"mov edx,[ebx+eax]" \
	"mov ecx,[ebx+eax]" \
	"add eax,4" \
	"xor ecx,0xFFFFFFFF" \
	"sub edx,0x01010101" \
	"and ecx,0x80808080" \
	"test ecx,edx" \
	"jz LL" \
"L0:" \
	"add edx,0x01010101" \
	"test edx,0xFF" \
	"jz L4" \
	"test edx,0xFF00" \
	"jz L3" \
	"test edx,0xFF0000" \
	"jz L2" \
	"dec eax" \
	"pop edx" \
	"pop ecx" \
	"pop ebx" \
	"ret" \
"L2:" \
	"sub eax,2" \
	"pop edx" \
	"pop ecx" \
	"pop ebx" \
	"ret" \
"L3:" \
	"sub eax,3" \
	"pop edx" \
	"pop ecx" \
	"pop ebx" \
	"ret" \
"L4:" \
	"sub eax,4" \
	"pop edx" \
	"pop ecx" \
	"pop ebx" \
	value [eax] parm [eax] modify exact [eax]

Word BURGERCALL FastStrLen(const char *Input)
{
	return strlenasm(Input);
}

/**********************************

	Change the floating point precision in
	Intel processors to allow fast divides for
	routines that really need it.
	I return the previous state so you can quickly
	change it back.

**********************************/

extern IntelFP_e FooIntelSetPrecision(IntelFP_e Input);
#pragma aux FooIntelSetPrecision = \
	"push	edx" \
	"push	ecx" \
	"push	eax" \
	"shl	eax,8" /* Move to the Pentium bits area */ \
	"fnstcw	[esp]" /* Get the current status word */ \
	"mov ecx,dword ptr [esp]" /* Get the previous value */ \
	"mov edx,dword ptr [esp]" \
	"and ecx,0x0300"	/* Mask for return value */ \
	"and edx,0xFCFF"	/* Mask off unused bits */ \
	"shr ecx,8"			/* Convert to enum */ 	\
	"or	edx,eax" 		/* Blend in the bits */ \
	"mov eax,ecx"		/* Get the result */ \
	"mov dword ptr [esp],edx"	/* Store in memory */ \
	"fldcw [esp]"	/* Save the new status register */ \
	"pop	ecx" \
	"pop	ecx" \
	"pop	edx" \
	value [eax] parm [eax] modify exact [eax]

IntelFP_e BURGERCALL IntelSetFPPrecision(IntelFP_e Input)
{
	return FooIntelSetPrecision(Input);
}

/**********************************

	Take an arbitrary value and round it up to
	the nearest power of 2
	If the input is 0x40000001 to 0xFFFFFFFF, I return 0x80000000
	Zero will return zero

**********************************/

extern Word32 BURGERCALL WatPowerOf2(Word32 Input);

#pragma aux WatPowerOf2 = \
	"cmp eax,0x40000000"		/* Too large a number? */ \
	"ja Maximum"				/* Return the maximum */ \
	"cmp eax,2"					/* Too small a number? */ \
	"jbe Minimum"				/* Return as is */ \
	"dec eax"					/* Move down a bit */ \
	"bsr	ecx,eax"			/* Find the highest set bit (0-30) */ \
	"mov eax,2"					/* Shift up the nearest power of two */ \
	"shl eax,cl"				/* Return the result */ \
"Minimum:"  \
	"ret"						/* Result in EAX */ \
"Maximum:" \
	"mov eax,0x80000000"		/* Maximum value */ \
	"ret" \
	value [eax] parm [eax] modify [eax ecx]

Word32 BURGERCALL PowerOf2(Word32 Input)
{
	return WatPowerOf2(Input);
}

#endif
