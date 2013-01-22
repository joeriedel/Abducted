#include "ClStdLib.h"

#if defined(__POWERPC__) && defined(__MWERKS__) && defined(__MACOSX__)
//
// Assembly for Standard "C" library manager
// Power PC version
// Optimized and written by Bill Heineman
//

//
// Word FastStrLen(const char * str);
// I will check 4 bytes at a time by subtracting $01010101
// from the longword and detecting whether each and every byte changed
// sign from positive to negative. This can be done in a vector operation
//

Word asm FastStrLen(register const char *)
{
//
// First I need to longword align the pointer
//

		clrlwi     r12,r3,30		//Mask the low 2 bits from the pointer (0-3)
		rlwinm     r0,r3,3,27,29	//Copy the low 2 bit up and mul by 8 (0,8,16,24)
		neg        r12,r12			//0,-1,-2,-3
		li         r9,-1			//Init the mask
		lwzx       r8,r12,r3		//Fetch the longword aligned data
		subfic     r0,r0,32			//Convert 0,8,16,24 to 32,24,16,8
		lis        r6,0x101			//Store $01010101 in r6
		slw        r9,r9,r0			//Shift off the or mask (Could be 32 for zero)
		ori        r6,r6,0x0101		//Finish the load of $01010101
		or         r8,r8,r9			//Force the invalid values to $FF
		slwi       r5,r6,7			//Load r5 with $80808080

		subfc      r10,r6,r8		//Subtract $01010101 from text
		andc       r11,r5,r8		//And the bit compliment from $80808080
		and.       r10,r10,r11		//Perform the test
		addi       r12,r12,4		//Assume 1-4 bytes to be valid
		bne        strlenexit		//First time hit!
strlenl:
		lwzx       r8,r12,r3		//Fetch 4 bytes
		addi       r12,r12,4		//Increase the string length by 4
		subfc      r10,r6,r8		//Subtract $01010101 from the value
		andc       r11,r5,r8		//And the bit compliment from $80808080
		and.       r10,r10,r11		//Mask only those that changed from positive to negative
		beq        strlenl			//Any changed?
strlenexit:							//I've got a change. Wrap up
		clrrwi.    r10,r8,24		//Check the first byte
		subi       r3,r12,4			//Offset by zero
		beqlr						//Exit now
		rlwinm.    r10,r8,0,8,15	//Check the second byte
		subi       r3,r12,3
		beqlr
		rlwinm.    r10,r8,0,16,23	//Check the third byte
		subi       r3,r12,2
		beqlr
		subi       r3,r12,1			//Assume the forth byte did it.
		blr
}

//
// size_t strncmp(const char * str1,const char *str2,size_t len);
// Quickly scan through the two strings for a match
//
#if 0
asm int strncmp(register const char *str1,register const char *str2,register size_t len)
{	
//
// First I need to longword align the pointer
//

		mr.			r0,r5			//Get the maximum count
		mtctr		r5				//Store the count in the counter register
		ble			strncmp0		//If less than or equal to 0 for the length, quit now!
		lbz			r8,0(r3)		//Get the first character
		lbz			r7,0(r4)		//Get the second character
		mr			r6,r3
		cmpwi		cr1,r8,0		//End of the source string?
		subfc.		r3,r7,r8		//Perform the compare
		bdzf		cr1_EQ,strncmp1	//Is it not the end of the string and not counter
strncmpx:
		bnelr						//Are they the same? (If not, then continue)
		lbzu       r8,1(r6)			//Get the next character
		lbzu       r7,1(r4)
		cmpwi      cr1,r8,0			//End of source string?
		subfc.     r3,r7,r8			//Compare
		bdnzf      cr1_EQ,strncmpx	//Loop if not the end of the string
strncmp1:
		blr							//Exit with r3 being -, + or zero
strncmp0:
		li         r3,0
		blr
}
#endif

#endif