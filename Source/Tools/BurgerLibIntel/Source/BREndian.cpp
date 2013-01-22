#include <BREndian.hpp>

namespace Burger {

/**********************************

	Swap the endian

**********************************/

//#if !(defined(__68K__) && defined(__MWERKS__)) && !(defined(__POWERPC__) && defined(__MWERKS__)) && !(defined(__INTEL__) && (defined(__WATCOMC__) || defined(__MWERKS__) || defined(__ICL)))

/**********************************

	Okay, since the Microsoft Visual "C" compiler
	sucks so bad in inline assembly (As C++ inline funtions)
	I simply gave up and wrote them as assembly function.
	Sadly, the calling overhead is FASTER than inline
	due to the register ineffiencies in the creation and
	use of temporary memory variables to
	pass data to an assembly fragment.

	Please note my use of grabbing return addresses
	in registers, this is to prevent stalls with Address
	Generation Interlocks on PIII and higher CPUs,
	
**********************************/

#if defined(_MSC_VER)

__declspec(naked) Word16 BURGERCALL SwapEndian(Word16 a)
{
	_asm {
		mov ax,cx
		ror ax,8
		ret
	}
}

__declspec(naked) SWord16 BURGERCALL SwapEndian(SWord16 a)
{
	_asm {
		mov ax,cx
		ror ax,8
		ret
	}
}

__declspec(naked) Word32 BURGERCALL SwapEndian(Word32 a)
{
	_asm {
		mov eax,ecx
		bswap eax
		ret
	}
}

__declspec(naked) float BURGERCALL SwapEndian(float a)
{
	_asm {
		mov eax,dword ptr [esp+4]
		bswap eax
		mov edx,dword ptr [esp]
		mov dword ptr [esp+4],eax
		fld dword ptr [esp+4]
		add esp,8
		jmp edx
	}
}

__declspec(naked) double BURGERCALL SwapEndian(double a)
{
	_asm {
		mov eax,dword ptr [esp+4]
		mov edx,dword ptr [esp+8]
		bswap eax
		mov ecx,dword ptr [esp]
		bswap edx
		mov dword ptr [esp+8],eax
		mov dword ptr [esp+4],edx
		fld qword ptr [esp+4]
		add esp,12
		jmp ecx
	}
}

__declspec(naked) Word16 BURGERCALL SwapEndian(const Word16 *a)
{
	_asm {
		mov ax,word ptr [ecx]
		ror ax,8
		ret
	}
}

__declspec(naked) SWord16 BURGERCALL SwapEndian(const SWord16 *a)
{
	_asm {
		mov ax,word ptr [ecx]
		ror ax,8
		ret
	}
}

__declspec(naked) Word32 BURGERCALL SwapEndian(const Word32 *a)
{
	_asm {
		mov eax,dword ptr [ecx]
		bswap eax
		ret
	}
}

__declspec(naked) float BURGERCALL SwapEndian(const float *a)
{
	_asm {
		mov eax,dword ptr [ecx]
		mov ecx,dword ptr [esp]
		bswap eax
		mov dword ptr [esp],eax
		fld dword ptr [esp]
		pop eax
		jmp ecx
	}
}

__declspec(naked) double BURGERCALL SwapEndian(const double *a)
{
	_asm {
		sub esp,8
		mov eax,dword ptr [ecx]
		mov edx,dword ptr [ecx+4]
		bswap eax
		mov ecx,dword ptr [esp+8]
		bswap edx
		mov dword ptr [esp+4],eax
		mov dword ptr [esp],edx
		fld	qword ptr [esp]
		add esp,12
		jmp ecx
	}
}

__declspec(naked) void BURGERCALL SwapEndianMem(Word16 *a)
{
	_asm {
		mov ax,word ptr [ecx]
		ror ax,8
		mov word ptr [ecx],ax
		ret
	}
}

__declspec(naked) void BURGERCALL SwapEndianMem(Word32 *a)
{
	_asm {
		mov eax,dword ptr [ecx]
		bswap eax
		mov dword ptr [ecx],eax
		ret
	}
}

__declspec(naked) void BURGERCALL SwapEndianMem(double *a)
{
	_asm {
		mov eax,dword ptr [ecx]
		mov edx,dword ptr [ecx+4]
		bswap eax
		bswap edx
		mov dword ptr [ecx+4],eax
		mov dword ptr [ecx],edx
		ret
	}
}

#else

/**********************************

	Swap endian code in pure C++

**********************************/

/**********************************

	Function:
		SWord32 BURGERCALL Burger::SwapEndian(SWord32 Val)

	Header:
		Reverse the endian of a 32 bit integer

	Synopsis:
		Given a 32 bit value in an integer register, swap the bytes
		so that 0x12345678 becomes 0x78563412

	Input:
		Val = Value to endian convert

	Returns:
		The input with the bytes swapped, no errors are possible.

	Notes:
		This function is inlined to actually use Burger::SwapEndian(Word32)
		
	Also:
		Burger::LoadLittle(SWord32), Burger::LoadBig(SWord32), 
		Burger::SwapEndian(Word32), Burger::SwapEndian(const SWord32 *)
		

**********************************/


/**********************************

	Function:
		Word16 BURGERCALL Burger::SwapEndian(Word16 Val)

	Header:
		Reverse the endian of a 16 bit integer

	Synopsis:
		Given a 16 bit value in an integer register, swap the bytes
		so that 0x1234 becomes 0x3412

	Input:
		Val = Value to endian convert

	Returns:
		The input with the bytes swapped, no errors are possible.

	Notes:
		This function is inlined on WATCOM and Metrowerks 68K compilers.
		
	Also:
		Burger::LoadLittle(Word16), Burger::LoadBig(Word16), 
		Burger::SwapEndian(SWord16), Burger::SwapEndian(const Word16 *)

**********************************/

Word16 BURGERCALL Burger::SwapEndian(Word16 a)
{
	Word Temp;
	Word Val;
	Val = a;
	Temp = (Val>>8)&0xFF;
	Temp |= (Val<<8)&0xFF00;
	return static_cast<Word16>(Temp);
}

/**********************************

	Swap the endian of a short

**********************************/

SWord16 BURGERCALL Burger::SwapEndian(SWord16 a)
{
	Word Temp;
	Word Val;
	Val = static_cast<Word16>(a);
	Temp = (Val>>8)&0xFF;
	Temp |= (Val<<8)&0xFF00;
	return static_cast<SWord16>(Temp);
}

/**********************************

	Function:
		Word32 BURGERCALL Burger::SwapEndian(Word32 Val)

	Header:
		Reverse the endian of a 32 bit integer

	Synopsis:
		Given a 32 bit value in an integer register, swap the bytes
		so that 0x12345678 becomes 0x78563412

	Input:
		Val = Value to endian convert

	Returns:
		The input with the bytes swapped, no errors are possible.

	Notes:
		This function is inlined on WATCOM and Metrowerks 68K compilers.
		
	Also:
		Burger::LoadLittle(Word32), Burger::LoadBig(Word32), 
		Burger::SwapEndian(SWord32), Burger::SwapEndian(const Word32 *)
		

**********************************/

Word32 BURGERCALL Burger::SwapEndian(Word32 a)
{
	Word32 Temp;
	Temp = (a>>24)&0xFF;
	Temp |= (a>>8)&0xFF00;
	Temp |= (a<<8)&0xFF0000;
	Temp |= a<<24;
	return Temp;
}


float BURGERCALL Burger::SwapEndian(float a)
{
	Word8 Temp1,Temp2;

	Temp1 = reinterpret_cast<const Word8 *>(&a)[0];
	Temp2 = reinterpret_cast<const Word8 *>(&a)[3];
	reinterpret_cast<Word8 *>(&a)[3] = Temp1;	/* Perform the swap */
	reinterpret_cast<Word8 *>(&a)[0] = Temp2;
	Temp1 = reinterpret_cast<const Word8 *>(&a)[1];
	Temp2 = reinterpret_cast<const Word8 *>(&a)[2];
	reinterpret_cast<Word8 *>(&a)[2] = Temp1;
	reinterpret_cast<Word8 *>(&a)[1] = Temp2;
	return a;					/* Return the float */
}

double BURGERCALL Burger::SwapEndian(double a)
{
	Word8 Temp1,Temp2;

	Temp1 = reinterpret_cast<const Word8 *>(&a)[0];
	Temp2 = reinterpret_cast<const Word8 *>(&a)[7];
	reinterpret_cast<Word8 *>(&a)[7] = Temp1;	/* Perform the swap */
	reinterpret_cast<Word8 *>(&a)[0] = Temp2;
	Temp1 = reinterpret_cast<const Word8 *>(&a)[1];
	Temp2 = reinterpret_cast<const Word8 *>(&a)[6];
	reinterpret_cast<Word8 *>(&a)[6] = Temp1;
	reinterpret_cast<Word8 *>(&a)[1] = Temp2;
	Temp1 = reinterpret_cast<const Word8 *>(&a)[2];
	Temp2 = reinterpret_cast<const Word8 *>(&a)[5];
	reinterpret_cast<Word8 *>(&a)[5] = Temp1;
	reinterpret_cast<Word8 *>(&a)[2] = Temp2;
	Temp1 = reinterpret_cast<const Word8 *>(&a)[3];
	Temp2 = reinterpret_cast<const Word8 *>(&a)[4];
	reinterpret_cast<Word8 *>(&a)[4] = Temp1;
	reinterpret_cast<Word8 *>(&a)[3] = Temp2;
	return a;					/* Return the double */
}

Word16 BURGERCALL Burger::SwapEndian(const Word16 *a)
{
	Word Temp;
	Word Val;
	Val = a[0];
	Temp = (Val>>8)&0xFF;
	Temp |= (Val<<8)&0xFF00;
	return static_cast<Word16>(Temp);
}

SWord16 BURGERCALL Burger::SwapEndian(const SWord16 *a)
{
	Word Temp;
	Word Val;
	Val = reinterpret_cast<const Word16 *>(a)[0];
	Temp = (Val>>8)&0xFF;
	Temp |= (Val<<8)&0xFF00;
	return static_cast<SWord16>(Temp);
}

Word32 BURGERCALL Burger::SwapEndian(const Word32 *a)
{
	Word32 Val;		
	Word32 Temp;
	Val = a[0];
	Temp = (Val>>24)&0xFF;
	Temp |= (Val>>8)&0xFF00;
	Temp |= (Val<<8)&0xFF0000;
	Temp |= Val<<24;
	return Temp;
}

float BURGERCALL Burger::SwapEndian(const float *a)
{
	Word8 Temp1,Temp2;
	float MemFloat;		/* This MUST be cast as a float to be float aligned! */

	Temp1 = reinterpret_cast<const Word8 *>(a)[0];
	Temp2 = reinterpret_cast<const Word8 *>(a)[1];
	reinterpret_cast<Word8 *>(&MemFloat)[3] = Temp1;	/* Perform the swap */
	reinterpret_cast<Word8 *>(&MemFloat)[2] = Temp2;
	Temp1 = reinterpret_cast<const Word8 *>(a)[2];
	Temp2 = reinterpret_cast<const Word8 *>(a)[3];
	reinterpret_cast<Word8 *>(&MemFloat)[1] = Temp1;
	reinterpret_cast<Word8 *>(&MemFloat)[0] = Temp2;
	return MemFloat;					/* Return the float */
}

/**********************************

	Swap the endian of an 8 byte double.
	Due to the fact that doubles will generate exceptions
	when read or written, and I cannot guarantee the input
	pointer is double aligned, I have to do this routine
	the hard way. A byte at a time. Or I get a massive slowdown
	due to exception handlers.

**********************************/

double BURGERCALL Burger::SwapEndian(const double *a)
{
	Word8 Temp1,Temp2;
	double MemFloat;		/* This MUST be cast as a double to be double aligned! */

	Temp1 = reinterpret_cast<const Word8 *>(a)[0];
	Temp2 = reinterpret_cast<const Word8 *>(a)[1];
	reinterpret_cast<Word8 *>(&MemFloat)[7] = Temp1;	/* Perform the swap */
	reinterpret_cast<Word8 *>(&MemFloat)[6] = Temp2;
	Temp1 = reinterpret_cast<const Word8 *>(a)[2];
	Temp2 = reinterpret_cast<const Word8 *>(a)[3];
	reinterpret_cast<Word8 *>(&MemFloat)[5] = Temp1;
	reinterpret_cast<Word8 *>(&MemFloat)[4] = Temp2;
	Temp1 = reinterpret_cast<const Word8 *>(a)[4];
	Temp2 = reinterpret_cast<const Word8 *>(a)[5];
	reinterpret_cast<Word8 *>(&MemFloat)[3] = Temp1;
	reinterpret_cast<Word8 *>(&MemFloat)[2] = Temp2;
	Temp1 = reinterpret_cast<const Word8 *>(a)[6];
	Temp2 = reinterpret_cast<const Word8 *>(a)[7];
	reinterpret_cast<Word8 *>(&MemFloat)[1] = Temp1;
	reinterpret_cast<Word8 *>(&MemFloat)[0] = Temp2;
	return MemFloat;					/* Return the double */
}


void BURGERCALL Burger::SwapEndianMem(Word16 *a)
{
	Word8 Temp1,Temp2;
	Temp1 = reinterpret_cast<Word8*>(a)[0];
	Temp2 = reinterpret_cast<Word8*>(a)[1];
	reinterpret_cast<Word8*>(a)[1] = Temp1;
	reinterpret_cast<Word8*>(a)[0] = Temp2;
}

void BURGERCALL Burger::SwapEndianMem(Word32 *a)
{
	Word8 Temp1,Temp2;
	Temp1 = reinterpret_cast<Word8*>(a)[0];
	Temp2 = reinterpret_cast<Word8*>(a)[3];
	reinterpret_cast<Word8*>(a)[3] = Temp1;
	reinterpret_cast<Word8*>(a)[0] = Temp2;
	Temp1 = reinterpret_cast<Word8*>(a)[1];
	Temp2 = reinterpret_cast<Word8*>(a)[2];
	reinterpret_cast<Word8*>(a)[2] = Temp1;
	reinterpret_cast<Word8*>(a)[1] = Temp2;
}

void BURGERCALL Burger::SwapEndianMem(double *a)
{
	Word8 Temp1,Temp2;
	Temp1 = reinterpret_cast<Word8*>(a)[0];
	Temp2 = reinterpret_cast<Word8*>(a)[7];
	reinterpret_cast<Word8*>(a)[7] = Temp1;
	reinterpret_cast<Word8*>(a)[0] = Temp2;
	Temp1 = reinterpret_cast<Word8*>(a)[1];
	Temp2 = reinterpret_cast<Word8*>(a)[6];
	reinterpret_cast<Word8*>(a)[6] = Temp1;
	reinterpret_cast<Word8*>(a)[1] = Temp2;
	Temp1 = reinterpret_cast<Word8*>(a)[2];
	Temp2 = reinterpret_cast<Word8*>(a)[5];
	reinterpret_cast<Word8*>(a)[5] = Temp1;
	reinterpret_cast<Word8*>(a)[2] = Temp2;
	Temp1 = reinterpret_cast<Word8*>(a)[3];
	Temp2 = reinterpret_cast<Word8*>(a)[4];
	reinterpret_cast<Word8*>(a)[4] = Temp1;
	reinterpret_cast<Word8*>(a)[3] = Temp2;
}

#endif
//#endif

} // Burger
