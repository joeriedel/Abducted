/**********************************

	Microsoft "C" version

**********************************/

#include "GrGraphics.h"

#if defined(__WIN32__) && !defined(__MWERKS__) && !defined(__WATCOMC__)

/**********************************

	Convert from 5:5:5 to 5:6:5 format.
	For accuracies sake, I will shift the 5
	bits of green into 6 bit by using this formula

	G = (G<<1)+(G&1);

	This way, 0x1f becomes 0x3F for full intensity instead
	of 0x3E if I used a simple shift.
	The extra cycle is negligable since the real bottleneck
	is shoving data onto the PCI bus, this is handled by
	combining the writes with a U/V paired write to do
	a 64 bit write using two integer registers on the
	intel platform.

	The routine assumes VideoWidth and VideoPointer are
	pointing to true hardware page and VideoOffscreen points
	to the offscreen buffer

**********************************/

__declspec(naked) void BURGERCALL Video555To565(void)
{
	_asm {
	push	ebx
	push	esi
	push	edi
	push	ebp
	sub	esp,8
	mov	ecx,[VideoOffscreen]
	mov	ebx,[VideoPointer]
	mov	eax,[ScreenHeight]
L1:
	mov	[esp],eax
	mov	[esp+4],ebx
	mov	ebp,[ScreenWidth]
L2:
	mov	eax,[ecx]
	mov	edx,[ecx+4]
	lea	edi,[eax+eax]
	lea	esi,[edx+edx]
	and	eax,0x3F003F
	and	edx,0x3F003F
	and	edi,0xFFC0FFC0
	and	esi,0xFFC0FFC0
	add	eax,edi
	add	edx,esi
	mov	[ebx],eax
	mov	[ebx+4],edx
	lea	ecx,[ecx+8]
	lea	ebx,[ebx+8]
	sub	ebp,4
	jne	L2
	mov	eax,[esp]
	mov	ebx,[esp+4]
	mov	edx,[VideoWidth]
	dec	eax
	lea	ebx,[edx+ebx]
	jne	L1
	pop	eax
	pop	eax
	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret
	}
}


#endif
