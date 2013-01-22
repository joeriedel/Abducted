#include "ClStdLib.h"
#include <BREndian.hpp>
#include "MmMemory.h"
#include "PfPrefs.h"
#include <stdlib.h>

/**********************************

	Table to reverse the bit order of a byte

**********************************/

Word8 ReverseBits[256] = {
	0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,
	0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
	0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,
	0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
	0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,
	0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
	0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,
	0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
	0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,
	0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
	0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,
	0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
	0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,
	0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
	0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,
	0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
	0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,
	0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
	0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,
	0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
	0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,
	0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
	0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,
	0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
	0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,
	0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
	0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,
	0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
	0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,
	0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
	0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,
	0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF
};

/**********************************

	Convert a BCD value into a hex value
	I assume the LSB is the lowest input nibble.
	Unpredictable results if the input value is NOT BCD!

**********************************/

Word32 BURGERCALL BCDToNum(Word32 Input)
{
	Word32 Output;
	Output = Input&0xF;			/* Get the first nibble */
	Input>>=4;
	Output = ((Input&0xF)*10)+Output;
	Input>>=4;
	if (Input) {		/* Bigger than a byte? */
		Output = ((Input&0xF)*100)+Output;
		Input>>=4;
		Output = ((Input&0xF)*1000)+Output;
		Input>>=4;
		if (Input) {	/* Bigger than a short? */
			Output = ((Input&0xF)*10000)+Output;
			Input>>=4;
			Output = ((Input&0xF)*100000)+Output;
			Input>>=4;
			if (Input) {	/* 3 bytes? */
				Output = ((Input&0xF)*1000000)+Output;
				Input>>=4;
				Output = (Input*10000000)+Output;
			}
		}
	}
	return Output;
}

/**********************************

	Convert a hex value into a BCD value
	I assume the LSB is the lowest input nibble.

**********************************/

Word32 BURGERCALL NumToBCD(Word32 Input)
{
	Word32 Output;
	Word32 Temp;

	if (Input>=99999999) {		/* Too large? */
		return 0x99999999;		/* Peg it! */
	}
	Output = 0;			/* Init output */
	if (Input>=100000) {
		Temp = Input/10000000;
		Output = Temp<<28;
		Input -= Temp*10000000;
		Temp = Input/1000000;
		Output = Temp<<24;
		Input -= Temp*1000000;
	}
	if (Input>=10000) {
		Temp = Input/100000;
		Output |= Temp<<20;
		Input -= Temp*100000;
		Temp = Input/10000;
		Output |= Temp<<16;
		Input -= Temp*10000;
	}
	if (Input>=100) {
		Temp = Input/1000;
		Output |= Temp<<12;
		Input -= Temp*1000;
		Temp = Input/100;
		Output |= Temp<<8;
		Input -= Temp*100;
	}

	Temp = Input/10;
	Output |= Temp<<4;
	Input -= Temp*10;

	Output |= Input;
	return Output;
}

/**********************************

	Take an arbitrary value and round it up to
	the nearest power of 2
	If the input is 0x40000001 to 0xFFFFFFFF, I return 0x80000000
	Zero will return zero

**********************************/

#if !defined(__POWERPC__) && !defined(__INTEL__)
Word32 PowerOf2(Word32 Input)
{
	if (Input<=0x10000) {				/* Which half? */
		if (Input<=0x100) {				/* Half again */
			if (Input<=0x10) {
				if (Input<=4) {			/* 4/0-2 */
					if (Input<=2) {
						return Input;	/* Return 0,1,2 */
					}
					return 4;
				}
				if (Input<=8) {			/* 8/0x10 */
					return 8;
				}
				return 0x10;
			}
			if (Input<=0x40) {
				if (Input<=0x20) {		/* 0x20/0x40 */
					return 0x20;
				}
				return 0x40;
			}
			if (Input<=0x80) {			/* 0x80/0x100 */
				return 0x80;
			}
			return 0x100;
		}
		if (Input<=0x1000) {
			if (Input<=0x400) {			/* 0x200/0x400 */
				if (Input<=0x200) {
					return 0x200;
				}
				return 0x400;
			}
			if (Input<=0x800) {			/* 0x800/0x1000 */
				return 0x800;
			}
			return 0x1000;
		}
		if (Input<=0x4000) {
			if (Input<=0x2000) {		/* 0x2000/0x4000 */
				return 0x2000;
			}
			return 0x4000;
		}
		if (Input<=0x8000) {			/* 0x8000/0x10000 */
			return 0x8000;
		}
		return 0x10000;
	}
	if (Input<=0x1000000) {
		if (Input<=0x100000) {
			if (Input<=0x40000) {		/* 0x20000/0x40000 */
				if (Input<=0x20000) {
					return 0x20000;
				}
				return 0x40000;
			}
			if (Input<=0x80000) {		/* 0x80000/0x100000 */
				return 0x80000;
			}
			return 0x100000;
		}
		if (Input<=0x400000) {
			if (Input<=0x200000) {		/* 0x200000/0x400000 */
				return 0x200000;
			}
			return 0x400000;
		}
		if (Input<=0x800000) {			/* 0x800000/0x1000000 */
			return 0x800000;
		}
		return 0x1000000;
	}
	if (Input<=0x10000000) {
		if (Input<=0x4000000) {			/* 0x2000000/0x4000000 */
			if (Input<=0x2000000) {
				return 0x2000000;
			}
			return 0x4000000;
		}
		if (Input<=0x8000000) {			/* 0x8000000/0x10000000 */
			return 0x8000000;
		}
		return 0x10000000;
	}
	if (Input<=0x40000000) {
		if (Input<=0x20000000) {		/* 0x20000000/0x40000000 */
			return 0x20000000;
		}
		return 0x40000000;
	}
	return 0x80000000U;					/* For everything else... */
}
#endif

/**********************************

	Convert a "C" string into a Pascal string.
	Note: I will remove the trailing zero with a length prefix byte
	and move the data up one byte.

**********************************/

void BURGERCALL CStr2PStr(char *DestPtr,const char *SrcPtr)
{
	char Temp1;
	Word Len;
	char *NewStr;		/* Running pointer */

	NewStr = DestPtr;		/* Save return value */
	++NewStr;
	Temp1 = SrcPtr[0];		/* Prefetch first char */
	++SrcPtr;
	Len = 0;				/* Get the length of the string */
	if (Temp1) {			/* End of string? */
		do {
			char Temp2;

			/* Grab the next character to prevent an overwrite if the source */
			/* and dest buffers are the same */

			Temp2 = SrcPtr[0];	
			++SrcPtr;
			NewStr[0] = Temp1;	/* Save to dest string */
			++NewStr;
			if (++Len>=255) {
				break;
			}
			Temp1 = Temp2;	/* Get source string */
		} while (Temp1);	/* Still more? */
	}
	DestPtr[0] = (Word8)Len;	/* Save the length byte for PString */
}

/**********************************

	Convert a Pascal string into a "C" string.
	Since I get the length byte immediately, I can do a fast
	Word32 copy to move the bulk of the string.

**********************************/

void BURGERCALL PStr2CStr(char *DestPtr,const char *SrcPtr)
{
	Word Temp1,Temp2;

	Temp1 = ((Word8 *)SrcPtr)[0];	/* Get the string length */
	SrcPtr++;
	Temp2 = Temp1>>2;			/* Get the longword count */
	if (Temp2) {		/* Any longwords? */
		do {
			Word32 Temp;
			Temp = ((Word32 *)SrcPtr)[0];
			SrcPtr+=4;
			((Word32 *)DestPtr)[0] = Temp;	/* Copy a byte */
			DestPtr+=4;
		} while (--Temp2);	/* Count down */
	}
	Temp1 = Temp1&3;		/* Any bytes left? */
	if (Temp1) {
		do {
			DestPtr[0] = SrcPtr[0];		/* Copy a byte */
			++SrcPtr;
			++DestPtr;
		} while (--Temp1);	/* Count down */
	}
	DestPtr[0] = 0;		/* Terminating zero */
}

/**********************************

	I will read a string from an input stream.
	The I will allocate memory and return the string in the
	buffer.

**********************************/

void * BURGERCALL ExtractAString(const char *SrcPtr,Word32 *BufSize,Word Flags)
{
	void *Result;		/* Result pointer or handle */
	Word32 Length;	/* Input buffer length */
	Word32 NewLength;	/* Size of the result */
	const Word8 *WorkPtr;	/* Work source pointer */
	Word8 *DestPtr;		/* Work destination pointer */
	Word Temp;			/* Duh... */

	Length = *BufSize;		/* Get the length of the input buffer */
	if (!Length) {			/* Is there any input? */
		goto Abort;
	}
	WorkPtr = (Word8 *)SrcPtr;	/* Init the true source pointer */
	NewLength = 1;			/* 1 byte for the terminating zero */
	do {
		Temp = WorkPtr[0];	/* Get a char from input */
		++WorkPtr;			/* Inc source */
		if (!(Flags&EXTRACTSTRNOTRANSLATE)) {
			if (Temp==13) {		/* Is it a CR? */
				Temp=10;		/* Convert to a LF */
				if (Length>=2 && WorkPtr[0]==10) {	/* CR/LF (PC) */
					++WorkPtr;	/* Discard the LF */
					--Length;	/* Reduce from maximum */
				}
			}
		}
		if (Flags&EXTRACTSTRDELIMITLF) {	/* End on a LF? */
			if (Temp==10) {		/* LF? (Unix) */
				break;
			}
		}
		if (!Temp) {		/* End the string now? */
			break;
		}
		++NewLength;
	} while (--Length);		/* Scan for more data */

	Length = WorkPtr-(const Word8 *)SrcPtr;	/* Bytes removed from input */
	*BufSize = Length;			/* Number of bytes parsed */

	/* Let's allocate the result buffer */

	if (Flags&EXTRACTSTRHANDLE) {		/* Handle? */
		Result = (void *)AllocAHandle(NewLength);
		if (!Result) {
			goto Abort;
		}
		DestPtr = (Word8*)(*((void **)Result));
	} else {
		Result = AllocAPointer(NewLength);	/* Pointer? */
		if (!Result) {
			goto Abort;
		}
		DestPtr = (Word8 *)Result;
	}

	/* Now let's save the data */

	WorkPtr = (Word8 *)SrcPtr;	/* Perform the loop again */
	do {
		Temp = WorkPtr[0];	/* Get a char from input */
		++WorkPtr;
		if (!(Flags&EXTRACTSTRNOTRANSLATE)) {
			if (Temp==13) {		/* Is it a CR? */
				Temp=10;		/* Convert to a LF */
				if (Length>=2 && WorkPtr[0]==10) {	/* CR/LF (PC) */
					++WorkPtr;	/* Discard the LF */
					--Length;
				}
			}
		}
		DestPtr[0] = static_cast<Word8>(Temp);		/* Store the char */
		++DestPtr;
	} while (--Length);

	if (Temp) {				/* The last char MUST be a zero! */
		--DestPtr;
		DestPtr[0] = 0;		/* Force a zero */
	}
	return Result;			/* Return either a handle or a pointer */

Abort:
	return 0;		/* Return zip for error */
}

/**********************************

	I will read a string from an input stream and
	store it into a static buffer.
	I properly handle the EXTRACTSTRNOTRANSLATE and
	EXTRACTSTRDELIMITLF defines.

**********************************/

void BURGERCALL ExtractAString2(const char *SrcPtr,Word32 *BufSize,Word Flags,char *DestPtr,Word32 DestSize)
{
	Word32 Length;	/* Input buffer length */
	const Word8 *WorkPtr;	/* Work source pointer */
	Word Temp;			/* Duh... */

	Length = *BufSize;		/* Get the length of the input buffer */
	if (Length) {			/* Is there any input? */
		if (DestSize) {		/* Is there output? */
			--DestSize;		/* Reduce the buffer by 1 */
			DestPtr[DestSize] = 0;	/* Force the last char to be zero */
		}
		WorkPtr = (Word8 *)SrcPtr;	/* Init the true source pointer */
		do {
			Temp = WorkPtr[0];	/* Get a char from input */
			++WorkPtr;			/* Inc source */
			if (!(Flags&EXTRACTSTRNOTRANSLATE)) {
				if (Temp==13) {		/* Is it a CR? */
					Temp=10;		/* Convert to a LF */
					if (Length>=2 && WorkPtr[0]==10) {	/* CR/LF (PC) */
						++WorkPtr;	/* Discard the LF */
						--Length;	/* Reduce from maximum */
					}
				}
			}
			if (Flags&EXTRACTSTRDELIMITLF) {	/* End on a LF? */
				if (Temp==10) {		/* LF? (Unix) */
					break;
				}
			}
			if (!Temp) {		/* End the string now? */
				break;
			}

			if (DestSize) {		/* Any data in the dest buffer? */
				--DestSize;		/* Remove value */
				DestPtr[0] = static_cast<char>(Temp);	/* Store char */
				++DestPtr;		/* Inc the pointer */
			}
		} while (--Length);		/* Scan for more data */

		if (DestSize) {		/* Did I underrun? */
			DestPtr[0] = 0;	/* Force a "C" string. */
		}
		*BufSize = WorkPtr-(const Word8 *)SrcPtr;			/* Number of bytes parsed */
	}
}

#if (defined(__MAC__) && !defined(__MSL__)) || defined(__BEOS__) || defined(__MACOSX__) || (defined(__INTEL__) && defined(__MWERKS__) && !defined(_MSL_NEEDS_EXTRAS))

/**********************************

	Convert a string to upper case

**********************************/

char * BURGERCALL strupr(char *StrPtr)
{
	Word temp1;
	char *NewStr;

	temp1 = ((Word8 *)StrPtr)[0];		/* Prefetch first char */
	if (temp1) {			/* End of string? */
		NewStr = StrPtr;		/* Get a temp pointer */
		do {
			if (temp1>='a' && temp1<('z'+1)) {
				temp1 = temp1-32;
			}
			NewStr[0] = (char)temp1;	/* Convert to upper case */
			temp1 = ((Word8 *)NewStr)[1];	/* Get the next char */
			++NewStr;
		} while (temp1);		/* Still more? */
	}
	return StrPtr;		/* Return the original pointer */
}

/**********************************

	Convert a string to lower case

**********************************/

char * BURGERCALL strlwr(char *StrPtr)
{
	Word temp1;
	char *NewStr;

	temp1 = ((Word8 *)StrPtr)[0];		/* Prefetch first char */
	if (temp1) {			/* End of string? */
		NewStr = StrPtr;		/* Get a temp pointer */
		do {
			if (temp1>='A' && temp1<('Z'+1)) {
				temp1 = temp1+32;
			}
			NewStr[0] = (char)temp1;	/* Convert to lower case */
			temp1 = ((Word8 *)NewStr)[1];	/* Get the next char */
			++NewStr;
		} while (temp1);		/* Still more? */
	}
	return StrPtr;		/* Return the original pointer */
}

/**********************************

	Make a copy of a string

**********************************/

#if !defined(__MACOSX__)
char * BURGERCALL strdup(const char *Input)
{
	char *Result;
	Result = (char *)malloc(strlen(Input)+1);
	if (Result) {
		strcpy(Result,Input);
	}
	return Result;
}
#endif

/**********************************

	Compare two strings (Case insensitive)

**********************************/

int BURGERCALL stricmp(const char *StrPtr1,const char *StrPtr2)
{
	int temp1,temp2;

	do {
		temp1 = StrPtr1[0];		/* Convert to upper case */
		++StrPtr1;
		temp2 = StrPtr2[0];		/* Convert to upper case */
		++StrPtr2;
		if (temp1>='A' && temp1<('Z'+1)) {
			temp1 = temp1+32;
		}
		if (temp2>='A' && temp2<('Z'+1)) {
			temp2 = temp2+32;
		}
		if (temp1 < temp2) {		/* Match? */
			goto Less;			/* Less than */
		}
		if (temp1 != temp2) {	/* Greater than? */
			goto More;		/* Greater than... */
		}
	} while (temp1);		/* End of string? (And match!!) */
	return 0;			/* Perfect match! */
Less:;
	return -1;		/* Return less */
More:;
	return 1;		/* Return more */
}

/**********************************

	Compare two strings (Case insensitive)

**********************************/

#if !defined(__INTEL__)
int BURGERCALL strnicmp(const char *StrPtr1,const char *StrPtr2,Word32 Len)
#else
int BURGERCALL strnicmp(const char *StrPtr1,const char *StrPtr2,Word Len)
#endif
{
	int temp1,temp2;

	if (Len) {
		do {
			temp1 = StrPtr1[0];		/* Convert to upper case */
			++StrPtr1;
			temp2 = StrPtr2[0];		/* Convert to upper case */
			++StrPtr2;
			if (temp1>='A' && temp1<('Z'+1)) {
				temp1 = temp1+32;
			}
			if (temp2>='A' && temp2<('Z'+1)) {
				temp2 = temp2+32;
			}
			if (temp1 < temp2) {		/* Match? */
				goto Less;			/* Less than */
			}
			if (temp1 != temp2) {	/* Greater than? */
				goto More;		/* Greater than... */
			}
		} while (--Len && temp1);		/* End of string? (And match!!) */
	}
	return 0;			/* Perfect match! */
Less:;
	return -1;		/* Return less */
More:;
	return 1;		/* Return more */
}
#endif

/**********************************

	Compare two strings (Case insensitive)

**********************************/

#if defined(__MAC__) || defined(__BEOS__) || defined(__MACOSX__) || (defined(__INTEL__) && defined(__MWERKS__))

int BURGERCALL memicmp(const char *StrPtr1,const char *StrPtr2,Word Length)
{
	int temp1,temp2;

	if (Length) {
		do {
			temp1 = StrPtr1[0];		/* Convert to upper case */
			++StrPtr1;
			temp2 = StrPtr2[0];		/* Convert to upper case */
			++StrPtr2;
			if (temp1>='A' && temp1<('Z'+1)) {
				temp1 = temp1+32;
			}
			if (temp2>='A' && temp2<('Z'+1)) {
				temp2 = temp2+32;
			}
			if (temp1 < temp2) {		/* Match? */
				goto Less;			/* Less than */
			}
			if (temp1 != temp2) {	/* Greater than? */
				goto More;		/* Greater than... */
			}
		} while (--Length);		/* End of string? (And match!!) */
	}
	return 0;			/* Perfect match! */
Less:;
	return -1;		/* Return less */
More:;
	return 1;		/* Return more */
}
#endif

/**********************************

	Clip a section of a string into a destination string

**********************************/

char * BURGERCALL midstr(char *DestPtr,const char *SourcePtr,Word Start,Word Length)
{
	Word i;

	i = strlen(SourcePtr);		/* Get the length of the source string */
	if ((Start+Length)>=i) {		/* Is the string big enough? */
		Length = i-Start;
		if ((int)Length<0) {
			Length = 0;
		}
	}
	if (Length) {
		FastMemCpy(DestPtr,&SourcePtr[Start],Length);	/* Copy the string */
	}
	DestPtr[Length] = 0;		/* Terminate the string */
	return DestPtr;
}

/**********************************

	Perform a strstr() but case insensitive

**********************************/

char * BURGERCALL stristr(const char *Input, const char *SubStr)
{
	Word Temp;
	Temp = ((Word8 *)Input)[0];		/* Get the first character */
	if (Temp) {						/* Do I even bother? */
		do {
			Word i;
			Word Temp2;
			i = 0;					/* Init the index */
			do {
				Temp2 = ((Word8 *)SubStr)[i];	/* Get the first char of the test string */
				if (!Temp2) {					/* Match? */
					return (char *)Input;		/* I matched here! */
				}
				Temp = ((Word8 *)Input)[i];		/* Get the source string */
				++i;							/* Ack the char */
				if (Temp2>='A' && Temp2<='Z') {	/* Convert to lower case */
					Temp2 += 32;
				}
				if (Temp>='A' && Temp<='Z') {	/* Convert to lower case */
					Temp += 32;
				}
			} while (Temp == Temp2);			/* Match? */
			++Input;							/* Next main string char */
			Temp = ((Word8 *)Input)[0];			/* Next entry */
		} while (Temp);							/* Source string still ok? */
	}
	return 0;		/* No string match */
}

/**********************************

	Perform the ANSI function strlen()
	I grab 4 bytes at a time and the data is
	long word aligned. I test 4 bytes at a time by
	subtracting 0x01010101 from the longword and
	test for any high bits the transitioned from
	clear to set (0x00 -> 0xFF)

**********************************/

#if !defined(__INTEL__) && !defined(__POWERPC__)

Word BURGERCALL FastStrLen(const char *Input)
{
	Word i;
	Word32 Temp;
	Word32 Temp2,Temp3;

	i = (Word)Input&3;		/* Get the address and see if it is already aligned */
	if (i) {				/* Nope, perform a phony first fetch */
		i = 0-i;			/* -1,-2,-3 */
		Temp = ((Word32 *)(Input+i))[0];	/* Get the first longword */
#if defined(__LITTLEENDIAN__)
		Temp |= 0xFFFFFFFF>>(32+(i*8));		/* Create mask for unused bytes */
#else
		Temp |= 0xFFFFFFFF<<(32+(i*8));
#endif
		i+=4;				/* Accept 4 bytes in length */
		Temp2 = Temp-0x01010101;			/* Perform the subtraction */
		Temp3 = (~Temp)&0x80808080;			/* Make all CLEAR high bits set */
		if (Temp2&Temp3) {					/* Test for CLEAR to SET transitions */
			goto Skip1;						/* If TRUE then a transition occured */
		}
	}
	/* This is the main loop */
	do {
		Temp = ((Word32 *)(Input+i))[0];	/* Fetch the longword */
		i += 4;								/* Accept the 4 bytes */
		Temp2 = Temp-0x01010101;			/* Perform the subtraction */
		Temp3 = (~Temp)&0x80808080;			/* Get the mask for CLEAR bits */
	} while (!(Temp3&Temp2));				/* If zero, then no transitions occured */
Skip1:;
#if defined(__LITTLEENDIAN__)
	if (!(Temp&0xFF)) {						/* Was the first byte the zero one? */
		return i-4;							/* Adjust result and exit */
	}
	if (!(Temp&0xFF00)) {					/* Word8 #2 zero? */
		return i-3;
	}
	if (!(Temp&0xFF0000)) {					/* Word8 #3 zero? */
		return i-2;
	}
#else
	if (!(Temp&0xFF000000)) {
		return i-4;
	}
	if (!(Temp&0xFF0000)) {
		return i-3;
	}
	if (!(Temp&0xFF00)) {
		return i-2;
	}
#endif
	return i-1;								/* I'll just assume byte #4 is zero */
}

#endif

/**********************************

	Actually remove entries from the command list

**********************************/

static Word BURGERCALL RemoveParm(char *argv[],Word Index,Word Count,Word argc)
{
	if (!Count) {		/* Why am I here? */
		return argc;	/* Don't remove anything (Failsafe) */
	}
	if ((Index+Count) >= argc) {	/* Off the end? */
		return Index;			/* Just truncate the table to the end */
	}
	argc-=Count;	/* Get the adjusted count */
	Count+=Index;	/* Index to the NEXT entry */
	do {
		argv[Index] = argv[Count];	/* Copy the entry */
		++Count;			/* Next source */
	} while (++Index<argc);	/* All done? */
	return argc;		/* Return the new count */
}

/**********************************

	Traverse the command list for input switches

**********************************/

Word BURGERCALL Switches(Word argc,char *argv[],const Switch_t *SwitchList)
{
	Word i;
	Word Letter;
	Word CommandLen;
	char *SrcText;
	const Switch_t *WorkSwitch;

	i = 1;
	while (i<argc) {		/* As long as there are parms left */
		Letter = argv[i][0];	/* Get the first char */

		if (Letter=='-' || Letter=='/') {	/* Valid prefix char? */
			SrcText = &argv[i][1];		/* Get second char (Upper case) */
			WorkSwitch = SwitchList;		/* Reset the table */
			for (;;) {
				char *Command;
				Command = WorkSwitch->StrPtr;	/* Get command */
				if (!Command) {		/* End of the list? */
					break;
				}
				CommandLen = strlen(Command);
				if (!memicmp(Command,SrcText,CommandLen)) {	/* Match? */
					Word Parms;
					SrcText += CommandLen;
					if (WorkSwitch->Flags & SWITCH_CALLBACK) {
						Parms = WorkSwitch->Value;
						if (!Parms) {
							Parms = 1;
						}
						if (!SrcText[0] && (i+1)<argc) {	/* Seperate parm? */
							SrcText = argv[i+1];
							++Parms;
						}
						((SwitchCallBackProc)WorkSwitch->ValuePtr)(SrcText);
					} else if (WorkSwitch->Flags & SWITCH_WORD) {
						Parms = 1;
						if (!SrcText[0] && (i+1)<argc) {	/* Seperate parm? */
							SrcText = argv[i+1];
							++Parms;
						}
						((Word32 *)WorkSwitch->ValuePtr)[0] = AsciiToLongWord(SrcText);
					} else {
						((Word32 *)WorkSwitch->ValuePtr)[0] = WorkSwitch->Value;
						Parms = 1;		/* Remove 1 parm */
					}
					argc = RemoveParm(argv,i,Parms,argc);
					--i;		/* Undo the future ++i */
					break;
				}
				++WorkSwitch;	/* Next entry */
			}
		}
		++i;	/* Parse the next parm */
	}
	return argc;		/* Return the NEW count */
}

/**********************************

	Compute the (Mark) Adler-32 checksum

	The lower 16 bits is a simple additive checksum with
	a starting value of 1.

	The upper 16 bits is a factorial additive checksum based on the
	additive checksum with a starting value of 0

	This is based on the source provided from Mark Adler
	in the zlib source archive.

**********************************/

/* Note : Do NOT alter these defines or the checksum */
/* will not be the same as found in deflate/inflate gzip */
/* archives. This is a bad thing. */

#define BASE 65521L /* largest prime smaller than 65536 */
#define NMAX 5552	/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

Word32 BURGERCALL CalcMoreAdler(const Word8 *Buffer,Word32 Length,Word32 CheckSum)
{
	Word32 Additive;

	if (Buffer) {		/* Do I want the default value? */
		if (Length) {		/* Ok to do the deed? */
			Additive = (Word16)CheckSum;	/* Get the additive checksum */
			CheckSum = (Word16)(CheckSum>>16); 	/* Get the factorial checksum */
			do {
				Word i;
				i = NMAX;						/* Assume maximum */
				if (Length<NMAX) {				/* Not enough */
					i = (Word)Length;			/* Use the length */
				}
				Length -= i;					/* Remove the length */
				do {
					Additive += Buffer[0];		/* Add to the additive checksum */
					++Buffer;
					CheckSum += Additive;		/* Add the checksum to the factorial */
				} while (--i);
				Additive %= BASE;	/* Force to fit in a short */
				CheckSum %= BASE;	/* Force to fit in a short */
			} while (Length);		/* All done? */
			CheckSum = (CheckSum<<16)+Additive;	/* Blend */
		}
		return CheckSum;
	}
	return 1;		/* Return the default value */
}

/**********************************

	Compute the (Mark) Adler-16 checksum

	The lower 8 bits is a simple additive checksum with
	a starting value of 1.

	The upper 8 bits is a factorial additive checksum based on the
	additive checksum with a starting value of 0

	This is based on the source provided from Mark Adler
	in the zlib source archive.

**********************************/

/* Note : Do NOT alter these defines or the checksum */
/* will not be the same as found in deflate/inflate gzip */
/* archives. This is a bad thing. */

#define BASE16 251 /* largest prime smaller than 256 */
#define NMAX16 5802	/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

Word BURGERCALL CalcMoreAdler16(const Word8 *Buffer,Word32 Length,Word CheckSum)
{
	Word32 Additive;
	Word32 Factor;
	
	if (Buffer) {		/* Do I want the default value? */
		if (Length) {		/* Ok to do the deed? */
			Additive = (Word8)CheckSum;	/* Get the additive checksum */
			Factor = (Word8)(CheckSum>>8); 	/* Get the factorial checksum */
			do {
				Word i;
				i = NMAX16;		/* Assume maximum */
				if (Length<NMAX16) {	/* Not enough */
					i = (Word)Length;	/* Use the length */
				}
				Length -= i;	/* Remove the length */
				do {
					Additive += Buffer[0];	/* Add to the additive checksum */
					++Buffer;
					Factor += Additive;		/* Add the checksum to the factorial */
				} while (--i);
				Additive %= BASE16;	/* Force to fit in a byte */
				Factor %= BASE16;		/* Force to fit in a byte */
			} while (Length);		/* All done? */
			CheckSum = (Word)((Factor<<8)+Additive);	/* Blend */
		}
		return CheckSum;
	}
	return 1;		/* Return the default value */
}


/**********************************

	Calculates a 32-bit CRC,based on the obscurely-written
	ZModem source (if that doesn't make you hate Unix people,
	nothing will!).

	Copyright (C) 1986 Gary S. Brown. You may use this program,or
	code or tables extracted from it,as desired without restriction.

	Optimized and added to Burgerlib by Bill Heineman,1996

	First,the polynomial itself and its table of feedback terms. The
	polynomial is
	X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0
	Note that we take it "backwards" and put the highest-order term in
	the lowest-order bit. The X^32 term is "implied"; the LSB is the
	X^31 term,etc. The X^0 term (usually shown as "+1") results in
	the MSB being 1.

	Note that the usual hardware shift register implementation,which
	is what we're using (we're merely optimizing it by doing eight-bit
	chunks at a time) shifts bits into the lowest-order term. In our
	implementation,that means shifting towards the right. Why do we
	do it this way? Because the calculated CRC must be transmitted in
	order from highest-order term to lowest-order term. UARTs transmit
	characters in order from LSB to MSB. By storing the CRC this way,
	we hand it to the UART in the order low-byte to high-byte; the UART
	sends each low-bit to hight-bit; and the result is transmission bit
	by bit from highest- to lowest-order term without requiring any bit
	shuffling on our part. Reception works similarly.

	The feedback terms table consists of 256,32-bit entries. Notes:

	The table is generated by CRC32.c in the BurgerTools:TableGen archive
	It might not be obvious,but the feedback terms simply represent the
	results of eight shift/xor operations for all combinations of data
	and CRC register values.

	The values must be right-shifted by eight bits by the "updcrc"
	logic; the shift must be unsigned (bring in zeroes). On some
	hardware you could probably optimize the shift in assembler by
	using byte-swap instructions.

**********************************/

static const Word32 crcTable[256] = {
	0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,
	0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
	0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,
	0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
	0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,
	0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
	0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
	0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
	0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,
	0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
	0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,
	0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
	0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,
	0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
	0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,
	0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
	0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,
	0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
	0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,
	0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
	0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
	0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
	0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,
	0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
	0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,
	0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
	0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,
	0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
	0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,
	0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
	0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,
	0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
};

Word32 BURGERCALL CalcMoreCRC32(const Word8 *buffPtr,Word32 buffSize,Word32 crc)
{
	if (buffPtr) {
		if (buffSize) {		/* Ok to do the deed? */
			crc = ~crc;		/* Initialize the CRC */
			do {
				crc = crcTable[(Word8)(buffPtr[0]^crc)] ^ (crc>>8);
				++buffPtr;
			} while (--buffSize);
			crc = ~crc;		/* Complement the CRC */
		}
   		return crc;	/* Return the complement for ZModem */
	}
	return 0;
}

/**********************************

	This macro/inline function is used
	by the RSA hash routines

**********************************/

/* ROTATE_LEFT rotates x left n bits. */

/* Use inline assembly for DOS / Intel version */

#if defined(__WATCOMC__)
extern Word32 ROTATE_LEFT(Word32,Word);
#pragma aux ROTATE_LEFT = \
	"rol eax,cl" \
	parm [eax] [ecx] value [eax] modify [eax] exact
#else
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#endif

/**********************************

	MD2 message-digest algorithm

	Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All
	rights reserved.

	License to copy and use this software is granted for
	non-commercial Internet Privacy-Enhanced Mail provided that it is
	identified as the "RSA Data Security, Inc. MD2 Message Digest
	Algorithm" in all material mentioning or referencing this software
	or this function.

	RSA Data Security, Inc. makes no representations concerning either
	the merchantability of this software or the suitability of this
	software for any particular purpose. It is provided "as is"
	without express or implied warranty of any kind.

	These notices must be retained in any copies of any part of this
	documentation and/or software.

**********************************/

/* Permutation of 0..255 constructed from the digits of pi. It gives a
   "random" nonlinear byte substitution operation. */

static const Word8 PI_SUBST[256] = {
	 41, 46, 67,201,162,216,124,  1, 61, 54, 84,161,236,240,  6, 19,
     98,167,  5,243,192,199,115,140,152,147, 43,217,188, 76,130,202,
	 30,155, 87, 60,253,212,224, 22,103, 66,111, 24,138, 23,229, 18,
	190, 78,196,214,218,158,222, 73,160,251,245,142,187, 47,238,122,
	169,104,121,145, 21,178,  7, 63,148,194, 16,137, 11, 34, 95, 33,
	128,127, 93,154, 90,144, 50, 39, 53, 62,204,231,191,247,151,  3,
	255, 25, 48,179, 72,165,181,209,215, 94,146, 42,172, 86,170,198,
	 79,184, 56,210,150,164,125,182,118,252,107,226,156,116,  4,241,
	 69,157,112, 89,100,113,135, 32,134, 91,207,101,230, 45,168,  2,
	 27, 96, 37,173,174,176,185,246, 28, 70, 97,105, 52, 64,126, 15,
	 85, 71,163, 35,221, 81,175, 58,195, 92,249,206,186,197,234, 38,
	 44, 83, 13,110,133, 40,132,  9,211,223,205,244, 65,129, 77, 82,
	106,220, 55,200,108,193,171,250, 36,225,123,  8, 12,189,177, 74,
	120,136,149,139,227, 99,232,109,233,203,213,254, 59,  0, 29, 57,
	242,239,183, 14,102, 88,208,228,166,119,114,248,235,117, 75, 10,
	 49, 68, 80,180,143,237, 31, 26,219,153,141, 51,159, 17,131, 20
};

/**********************************

	MD2 basic transformation. Transforms state and updates checksum
	based on block.

**********************************/

static void MD2Transform (Word8 *state,Word8 * checksum,const Word8 *block)
{
	Word i, j, t;
	Word8 x[48];

	/* Form encryption block from state, block, state ^ block. */

	FastMemCpy(x,state,16);
	FastMemCpy(x+16,block,16);
	i = 0;
	do {
		x[i+32] = state[i] ^ block[i];
	} while (++i<16);

	/* Encrypt block (18 rounds). */
	t = 0;
	i = 0;
	do {
		j = 0;
		do {
			t = x[j] ^= PI_SUBST[t];
		} while (++j<48);
		t = (t + i) & 0xff;
	} while (++i<18);

	/* Save new state */
	FastMemCpy(state,x,16);

	/* Update checksum. */
	t = checksum[15];
	i = 0;
	do {
		t = checksum[i] ^= PI_SUBST[block[i] ^ t];
	} while (++i<16);
}

/**********************************

	MD2 initialization. Begins an MD2 operation, writing a new context.

**********************************/

void BURGERCALL MD2Init(MD2_t *Input)
{
	FastMemSet(Input,0,sizeof(*Input));
}

/**********************************

	MD2 block update operation. Continues an MD2 message-digest
	operation, processing another message block, and updating the
	context.

**********************************/

void BURGERCALL MD2Update(MD2_t *context,const Word8 *input,Word32 inputLen)
{
	Word i, index;

	/* Update number of bytes mod 16 */

	index = context->count;
	context->count = (index + inputLen) & 0xf;

	i = 16 - index;

	/* Transform as many times as possible. */
	if (inputLen >= i) {
		FastMemCpy(&context->buffer[index],input,i);
		MD2Transform(context->state, context->checksum, context->buffer);

		if ((i+15)<inputLen) {
			do {
				MD2Transform (context->state, context->checksum, &input[i]);
				i+=16;
			} while ((i+15)<inputLen);
		}
		index = 0;
	} else {
		i = 0;
	}

	/* Buffer remaining input */
	FastMemCpy(&context->buffer[index],&input[i],inputLen-i);
}

/**********************************

	MD2 finalization. Ends an MD2 message-digest operation, writing the
	message digest and zeroizing the context.

**********************************/

void BURGERCALL MD2Final(Word8 *digest,MD2_t *context)
{
	Word index, padLen;
	Word8 Padding[32];

	/* Pad out to multiple of 16. */
	index = context->count;
	padLen = 16 - index;
	FastMemSet(Padding,padLen,padLen);
	MD2Update (context,Padding,padLen);

	/* Extend with checksum */
	MD2Update (context, context->checksum, 16);

	/* Store state in digest */
	FastMemCpy(digest,context->state,16);

	/* Zeroize sensitive information. */
	FastMemSet(context, 0, sizeof (*context));
}

/**********************************

	Quickly create an MD2 key

**********************************/

void BURGERCALL MD2Quick(Word8 *Output,const Word8 *BufferPtr,Word32 Length)
{
	MD2_t Context;
	MD2Init(&Context);
	MD2Update(&Context,BufferPtr,Length);
	MD2Final(Output,&Context);
}

/**********************************

	MD4 message-digest algorithm

	Copyright (C) 1990-2, RSA Data Security, Inc. All rights reserved.

	License to copy and use this software is granted provided that it
	is identified as the "RSA Data Security, Inc. MD4 Message-Digest
	Algorithm" in all material mentioning or referencing this software
	or this function.

	License is also granted to make and use derivative works provided
	that such works are identified as "derived from the RSA Data
	Security, Inc. MD4 Message-Digest Algorithm" in all material
	mentioning or referencing the derived work.

	RSA Data Security, Inc. makes no representations concerning either
	the merchantability of this software or the suitability of this
	software for any particular purpose. It is provided "as is"
	without express or implied warranty of any kind.

	These notices must be retained in any copies of any part of this
	documentation and/or software.

**********************************/

/* Constants for MD4Transform routine. */

#define S411 3
#define S412 7
#define S413 11
#define S414 19
#define S421 3
#define S422 5
#define S423 9
#define S424 13
#define S431 3
#define S432 9
#define S433 11
#define S434 15

/* F, G and H are basic MD4 functions.
 */
#define F4(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G4(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define H4(x, y, z) ((x) ^ (y) ^ (z))

/* FF, GG and HH are transformations for rounds 1, 2 and 3 */
/* Rotation is separate from addition to prevent recomputation */

#define FF4(a, b, c, d, x, s) { \
    (a) += F4((b), (c), (d)) + (x); \
    (a) = ROTATE_LEFT ((a), (s)); \
  }
#define GG4(a, b, c, d, x, s) { \
    (a) += G4((b), (c), (d)) + (x) + (Word32)0x5a827999; \
    (a) = ROTATE_LEFT ((a), (s)); \
  }
#define HH4(a, b, c, d, x, s) { \
    (a) += H4((b), (c), (d)) + (x) + (Word32)0x6ed9eba1; \
    (a) = ROTATE_LEFT ((a), (s)); \
  }

/* MD4 basic transformation. Transforms state based on block.
 */
static void MD4Transform(Word32 *state,const Word8*block)
{
	Word32 a;
	Word32 b;
	Word32 c;
	Word32 d;
	Word32 x[16];
	Word i;

	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];

	i = 0;
	do {
		x[i] = Burger::LoadLittle(&((Word32 *)block)[i]);
	} while (++i<16);

	/* Round 1 */
	FF4 (a, b, c, d, x[ 0], S411); /* 1 */
	FF4 (d, a, b, c, x[ 1], S412); /* 2 */
	FF4 (c, d, a, b, x[ 2], S413); /* 3 */
	FF4 (b, c, d, a, x[ 3], S414); /* 4 */
	FF4 (a, b, c, d, x[ 4], S411); /* 5 */
	FF4 (d, a, b, c, x[ 5], S412); /* 6 */
	FF4 (c, d, a, b, x[ 6], S413); /* 7 */
	FF4 (b, c, d, a, x[ 7], S414); /* 8 */
	FF4 (a, b, c, d, x[ 8], S411); /* 9 */
	FF4 (d, a, b, c, x[ 9], S412); /* 10 */
	FF4 (c, d, a, b, x[10], S413); /* 11 */
	FF4 (b, c, d, a, x[11], S414); /* 12 */
	FF4 (a, b, c, d, x[12], S411); /* 13 */
	FF4 (d, a, b, c, x[13], S412); /* 14 */
	FF4 (c, d, a, b, x[14], S413); /* 15 */
	FF4 (b, c, d, a, x[15], S414); /* 16 */

	/* Round 2 */
	GG4 (a, b, c, d, x[ 0], S421); /* 17 */
	GG4 (d, a, b, c, x[ 4], S422); /* 18 */
	GG4 (c, d, a, b, x[ 8], S423); /* 19 */
	GG4 (b, c, d, a, x[12], S424); /* 20 */
	GG4 (a, b, c, d, x[ 1], S421); /* 21 */
	GG4 (d, a, b, c, x[ 5], S422); /* 22 */
	GG4 (c, d, a, b, x[ 9], S423); /* 23 */
	GG4 (b, c, d, a, x[13], S424); /* 24 */
	GG4 (a, b, c, d, x[ 2], S421); /* 25 */
	GG4 (d, a, b, c, x[ 6], S422); /* 26 */
	GG4 (c, d, a, b, x[10], S423); /* 27 */
	GG4 (b, c, d, a, x[14], S424); /* 28 */
	GG4 (a, b, c, d, x[ 3], S421); /* 29 */
	GG4 (d, a, b, c, x[ 7], S422); /* 30 */
	GG4 (c, d, a, b, x[11], S423); /* 31 */
	GG4 (b, c, d, a, x[15], S424); /* 32 */

	/* Round 3 */
	HH4 (a, b, c, d, x[ 0], S431); /* 33 */
	HH4 (d, a, b, c, x[ 8], S432); /* 34 */
	HH4 (c, d, a, b, x[ 4], S433); /* 35 */
	HH4 (b, c, d, a, x[12], S434); /* 36 */
	HH4 (a, b, c, d, x[ 2], S431); /* 37 */
	HH4 (d, a, b, c, x[10], S432); /* 38 */
	HH4 (c, d, a, b, x[ 6], S433); /* 39 */
	HH4 (b, c, d, a, x[14], S434); /* 40 */
	HH4 (a, b, c, d, x[ 1], S431); /* 41 */
	HH4 (d, a, b, c, x[ 9], S432); /* 42 */
	HH4 (c, d, a, b, x[ 5], S433); /* 43 */
	HH4 (b, c, d, a, x[13], S434); /* 44 */
	HH4 (a, b, c, d, x[ 3], S431); /* 45 */
	HH4 (d, a, b, c, x[11], S432); /* 46 */
	HH4 (c, d, a, b, x[ 7], S433); /* 47 */
	HH4 (b, c, d, a, x[15], S434); /* 48 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
}

/**********************************

	MD4 initialization. Begins an MD4 operation, writing a new context.

**********************************/

void BURGERCALL MD4Init (MD4_t *Input)
{
	Input->countlo = 0;
	Input->counthi = 0;

	/* Load magic initialization constants. */

	Input->state[0] = 0x67452301;
	Input->state[1] = 0xefcdab89;
	Input->state[2] = 0x98badcfe;
	Input->state[3] = 0x10325476;
}

/**********************************

	MD4 block update operation. Continues an MD4 message-digest
	operation, processing another message block, and updating the
	context.

**********************************/

void BURGERCALL MD4Update(MD4_t *context,const Word8 *input,Word32 inputLen)
{
	Word i, index;

	/* Compute number of bytes mod 64 */

	index = (Word)(context->countlo & 0x3F);

	/* Update number of bits */

	if ((context->countlo += inputLen) < inputLen) {
		context->counthi++;
	}

	i = 64 - index;

	/* Transform as many times as possible. */

	if (inputLen >= i) {
		FastMemCpy(&context->buffer[index],input,i);
		MD4Transform(context->state, context->buffer);

		if ((i+63)<inputLen) {
			do {
				MD4Transform (context->state, &input[i]);
				i+=64;
			} while ((i+63)<inputLen);
		}
		index = 0;
	} else {
		i = 0;
	}

  /* Buffer remaining input */

	FastMemCpy(&context->buffer[index],&input[i],inputLen-i);
}

/**********************************

	MD4 finalization. Ends an MD4 message-digest operation, writing the
	the message digest and zeroizing the context.

**********************************/

void MD4Final(Word8 *Output,MD4_t *Input)
{
	Word8 bits[8];
	Word8 Padding[64];
	Word padLen;

	/* Save number of bits */

	((Word32 *)bits)[0] = Burger::LoadLittle((Input->countlo<<3));
	((Word32 *)bits)[1] = Burger::LoadLittle(((Input->counthi<<3)|(Input->countlo>>29)));

	/* Pad out to 56 mod 64. */

	padLen = ((55-((Word)Input->countlo))&0x3F)+1;		/* Convert to 1-64 */
	FastMemSet(&Padding[1],0,63);
	Padding[0] = 0x80;
	MD4Update(Input,Padding,padLen);

	/* Append length (before padding) */

	MD4Update(Input,bits,8);

	/* Store state in digest */
	((Word32 *)Output)[0] = Burger::LoadLittle(&Input->state[0]);
	((Word32 *)Output)[1] = Burger::LoadLittle(&Input->state[1]);
	((Word32 *)Output)[2] = Burger::LoadLittle(&Input->state[2]);
	((Word32 *)Output)[3] = Burger::LoadLittle(&Input->state[3]);

	/* Zeroize sensitive infromation */
	FastMemSet(Input,0,sizeof(*Input));
}

/**********************************

	Quickly create an MD4 key

**********************************/

void BURGERCALL MD4Quick(Word8 *Output,const Word8 *BufferPtr,Word32 Length)
{
	MD4_t Context;
	MD4Init(&Context);
	MD4Update(&Context,BufferPtr,Length);
	MD4Final(Output,&Context);
}

/**********************************

	MD5 message-digest algorithm

	Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
	rights reserved.

	License to copy and use this software is granted provided that it
	is identified as the "RSA Data Security, Inc. MD5 Message-Digest
	Algorithm" in all material mentioning or referencing this software
	or this function.

	License is also granted to make and use derivative works provided
	that such works are identified as "derived from the RSA Data
	Security, Inc. MD5 Message-Digest Algorithm" in all material
	mentioning or referencing the derived work.

	RSA Data Security, Inc. makes no representations concerning either
	the merchantability of this software or the suitability of this
	software for any particular purpose. It is provided "as is"
	without express or implied warranty of any kind.

	These notices must be retained in any copies of any part of this
	documentation and/or software.

**********************************/

/* Constants for MD5Transform routine. */

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
   Rotation is separate from addition to prevent recomputation.
*/
#define FF(a, b, c, d, x, s, ac) { \
    (a) += F ((b), (c), (d)) + (x) + (Word32)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
    (a) += G ((b), (c), (d)) + (x) + (Word32)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
    (a) += H ((b), (c), (d)) + (x) + (Word32)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
    (a) += I ((b), (c), (d)) + (x) + (Word32)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
  }


/**********************************

	MD5 basic transformation.
	Transforms state based on block.

**********************************/

static void BURGERCALL MD5Transform(Word32 *state,const Word8 *block)
{
	Word32 a;
	Word32 b;
	Word32 c;
	Word32 d;
	Word32 x[16];
	Word i;

	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];

	i = 0;
	do {
		x[i] = Burger::LoadLittle(&((Word32 *)block)[i]);
	} while (++i<16);

  /* Round 1 */
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
	FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
	FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
	FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
	FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

  /* Round 2 */
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
	GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
	GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
	GG (d, a, b, c, x[10], S22, 0x02441453); /* 22 */
	GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
	GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
	HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
	HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH (b, c, d, a, x[ 6], S34, 0x04881d05); /* 44 */
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */

	II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
	II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
	II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
	II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
	II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
	II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
}

/**********************************

	MD5 initialization.
	Begins an MD5 operation, writing a new context.

**********************************/

void BURGERCALL MD5Init(MD5_t *Input)
{
	Input->countlo = 0;
	Input->counthi = 0;

  /* Load magic initialization constants. */

	Input->state[0] = 0x67452301;
	Input->state[1] = 0xefcdab89;
	Input->state[2] = 0x98badcfe;
	Input->state[3] = 0x10325476;
}

/**********************************

	MD5 block update operation. Continues an MD5 message-digest
	operation, processing another message block, and updating the
	context.

**********************************/

void BURGERCALL MD5Update(MD5_t *context,const Word8 * input,Word32 inputLen)
{
	Word i, index;

	/* Compute number of bytes mod 64 */

	index = (Word)(context->countlo & 0x3F);

	/* Update number of bits (Perform a 64 bit add) */

	if ((context->countlo += inputLen) < inputLen) {
		context->counthi++;
	}
	i = 64 - index;

	/* Transform as many times as possible. */

	if (inputLen >= i) {		/* Should I copy or pack? */

		FastMemCpy(&context->buffer[index],input,i);
		MD5Transform(context->state,context->buffer);	/* Perform the checksum */

		/* Perform the checksum directly on the memory buffers */

		if ((i+63)<inputLen) {
			do {
	 			MD5Transform(context->state,&input[i]);
				i += 64;
			} while ((i+63) < inputLen);
		}
		index = 0;
	} else {
		i = 0;
	}

	/* Buffer remaining input */
	FastMemCpy(&context->buffer[index],&input[i],inputLen-i);
}

/**********************************

	MD5 finalization. Ends an MD5 message-digest operation, writing the
	the message digest and zeroizing the context.

**********************************/

void BURGERCALL MD5Final(Word8 *Output,MD5_t *Input)
{
	Word8 bits[8];			/* Bit count in little endian format */
	Word8 Padding[64];		/* Pad array, first byte is 0x80, rest 0 */
	Word padLen;

	/* Save number of bits */

	((Word32 *)bits)[0] = Burger::LoadLittle((Input->countlo<<3));
	((Word32 *)bits)[1] = Burger::LoadLittle(((Input->counthi<<3)|(Input->countlo>>29)));

	/* Pad out to 56 mod 64. */

	padLen = ((55-((Word)Input->countlo))&0x3f)+1;	/* Convert to 1-64 */

	FastMemSet(&Padding[1],0,63);
	Padding[0] = 0x80;
	MD5Update(Input,Padding,padLen);		/* Pad out to 64 byte chunk */

	/* Append length (before padding) */

	MD5Update(Input,bits,8);

	/* Store state in digest */
	((Word32 *)Output)[0] = Burger::LoadLittle(&Input->state[0]);
	((Word32 *)Output)[1] = Burger::LoadLittle(&Input->state[1]);
	((Word32 *)Output)[2] = Burger::LoadLittle(&Input->state[2]);
	((Word32 *)Output)[3] = Burger::LoadLittle(&Input->state[3]);

	/* Zeroize sensitive information. */
	FastMemSet(Input,0,sizeof(*Input));
}

/**********************************

	Quickly create an MD5 key

**********************************/

void BURGERCALL MD5Quick(Word8 *Output,const Word8 *BufferPtr,Word32 Length)
{
	MD5_t Context;
	MD5Init(&Context);
	MD5Update(&Context,BufferPtr,Length);
	MD5Final(Output,&Context);
}

/**********************************

	This routine will scan the operating system for files dropped into this
	applications icon. If any are present, each and every file will
	be passed to a routine to handle the event.
	If no file is present, do nothing.
	Once all the files are gone, return.

	The procedure returns FALSE for no error and the scan will continue,
	if an error is returned, the scan will abort.

**********************************/

#if !defined(__MAC__) && !defined(__WIN32__)
Word BURGERCALL SystemProcessFilenames(SystemProcessCallBackProc /* Proc */)
{
	return TRUE;
}
#endif

/**********************************

	By invoking DEEP magic, I will divine the version
	of QuickTimeX that is present.

	Returned values.
	0	    No QuickTime installed
	0x211   QuickTime 2.1.1 installed
	0x212	QuickTime 2.1.2 installed

**********************************/

#if !defined(__MAC__) && !defined(__WIN32__)
Word BURGERCALL GetQuickTimeVersion(void)
{
	return 0;
}
#endif

/**********************************

	Return the version of Burgerlib
	(Useful for a shared library version)

**********************************/

Word BURGERCALL BurgerlibVersion(void)
{
	return 0x102;		/* Version 1.0.2 */
}

/**********************************

	Load in a shared library

**********************************/

#if !defined(__MAC__) && !defined(__WIN32__)
LibRef_t * BURGERCALL LibRefInit(const char * /*LibName */)
{
	return 0;
}

/**********************************

	Release a shared library

**********************************/

void BURGERCALL LibRefDelete(LibRef_t * /* LibRef */)
{
}

/**********************************

	Grab a function from a shared library

**********************************/

void * BURGERCALL LibRefGetProc(LibRef_t * /* LibRef */,const char * /* ProcName */)
{
	return 0;
}

#endif


/**********************************

	Return a pointer to a DLL or shared library
	I do not release the library. This is a quick and dirty
	routine for getting things like DirectDraw and OpenTransport

**********************************/

void * BURGERCALL LibRefGetFunctionInLib(const char *LibName,const char *ProcName)
{
	LibRef_t *RefPtr;
	void *ProcPtr;

	RefPtr = LibRefInit(LibName);		/* Load the library */
	if (RefPtr) {						/* It's present! */
		ProcPtr =  LibRefGetProc(RefPtr,ProcName);		/* Load the function if found */
		if (ProcPtr) {
			return ProcPtr;				/* Return the proc pointer (NOTE: I don't release the reference!) */
		}
		LibRefDelete(RefPtr);			/* At least attempt to keep myself clean */
	}
	return 0;
}

