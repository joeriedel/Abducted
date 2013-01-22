/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2001             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

  function: packing variable sized words into an octet stream
  last mod: $Id: PkOggBitWise.c,v 1.7 2003/12/02 08:19:16 burger Exp $

 ********************************************************************/

/* We're 'LSb' endian; if we write a word but read individual bits,
   then we'll read the lsb first */

#include "PkOgg.h"
#include "ClStdlib.h"
#include "MmMemory.h"

#define BUFFER_INCREMENT 4096		/* Size to increment the buffer growth */

static const Word32 mask[]= {
	0x00000000,0x00000001,0x00000003,0x00000007,0x0000000f,
	0x0000001f,0x0000003f,0x0000007f,0x000000ff,0x000001ff,
	0x000003ff,0x000007ff,0x00000fff,0x00001fff,0x00003fff,
	0x00007fff,0x0000ffff,0x0001ffff,0x0003ffff,0x0007ffff,
	0x000fffff,0x001fffff,0x003fffff,0x007fffff,0x00ffffff,
	0x01ffffff,0x03ffffff,0x07ffffff,0x0fffffff,0x1fffffff,
	0x3fffffff,0x7fffffff,0xffffffff };

/**********************************

	Init a oggpack_buffer for writing

**********************************/

void BURGERCALL oggpack_writeinit(oggpack_buffer *b)
{
	Word8 *WorkPtr;
	FastMemSet(b,0,sizeof(*b));		/* Blank the structure */
	
	WorkPtr=static_cast<Word8 *>(AllocAPointer(BUFFER_INCREMENT));		/* Get my buffer */
	if (WorkPtr) {					/* Got the memory? */
		b->ptr=WorkPtr;				/* Store it */
		b->buffer = WorkPtr;
		b->storage=BUFFER_INCREMENT;
		WorkPtr[0]=0;				/* Init the first byte */
	}
}

/**********************************

	Reset the oggpack_buffer to the begining

**********************************/

void BURGERCALL oggpack_reset(oggpack_buffer *b)
{
	Word8 *WorkPtr;
	WorkPtr = b->buffer;		/* Get the starting byte address */
	b->ptr=WorkPtr;				/* This is my work address */
	if (WorkPtr) {
		WorkPtr[0]=0;			/* Zap the first byte */
	}
	b->endbit=0;
	b->endbyte=0;		/* Kill the bits */
}

/**********************************

	Clear out the structure and release all memory

**********************************/

void BURGERCALL oggpack_writeclear(oggpack_buffer *b)
{
	DeallocAPointer(b->buffer);		/* Release memory (NULL is ok) */
	FastMemSet(b,0,sizeof(*b));		/* Zap the buffer */
}

/**********************************

	Clear out the structure and release all memory

**********************************/

void BURGERCALL oggpack_readinit(oggpack_buffer *b,Word8 *buf,Word32 bytes)
{
	FastMemSet(b,0,sizeof(*b));		/* Kill the structure */
	b->buffer=buf;
	b->ptr=buf;			/* Store the buffer address */
	b->storage=bytes;				/* Store the buffer size */
}

/**********************************

	Store data in the bit stream (Maximum 32 bits)

**********************************/

void BURGERCALL oggpack_write(oggpack_buffer *b,Word32 value,Word bits)
{

	/* Do I need to resize the buffer? */
	Word32 Offset,Size;
	Word8 *WorkPtr;
	
	Size = b->storage;		/* Current size */
	Offset = b->endbyte;	/* Current used size */
	
	if ((Offset+4)>=Size) {		/* Buffer too small? */
		Size += BUFFER_INCREMENT;
		WorkPtr=static_cast<Word8 *>(ResizeAPointer(b->buffer,Size));
		b->buffer = WorkPtr;
		
		if (!WorkPtr) {			/* Allocation failure? */
			b->storage = 0;		/* Zap the structure */
			b->ptr = 0;
			b->endbit = 0;
			b->endbyte = 0;
			return;				/* Oh, you are so screwed!!! */
		}
		
		b->storage=Size;		/* New data size */
		b->ptr=WorkPtr+Offset;	/* New work pointer */
	}

	if (bits>32) {				/* Failsafe */
		bits = 32;
	}
	
	value&=mask[bits];			/* Hack off unneeded bits */
	Size = b->endbit;			/* Get the bits already in the queue (0-7) */
	bits+=Size;					/* Bits to process */
	WorkPtr = b->ptr;			/* Dest pointer */
	WorkPtr[0]|=static_cast<Word8>(value<<Size);	/* Blend bits */

	if (bits>=8) {				/* Am I in the next byte? */
		WorkPtr[1]=(Word8)(value>>(8-Size));		/* Grab the next 8 bits */
		if (bits>=16){			/* Still more? */
			WorkPtr[2]=(Word8)(value>>(16-Size));
			if (bits>=24) {		/* Even more? */
				WorkPtr[3]=(Word8)(value>>(24-Size));  
				if (bits>=32) {		/* Final overflow */
					if (Size) {
						WorkPtr[4]=(Word8)(value>>(32-Size));
					} else {
						WorkPtr[4]=0;
					}
				}
			}
		}
	}
	
	/* Let's wrap up... */
	
	Size = bits>>3;			/* Number of BYTES taken */
	b->endbyte = Offset + Size;	/* Adjust the pointer */
	b->ptr=WorkPtr+Size;
	b->endbit=bits&7;		/* Number of bits remaining */
}

/**********************************

	Grab a stream of bits, but DON'T remove the bits from the stream

**********************************/

Word32 BURGERCALL oggpack_look(oggpack_buffer *b,Word bits)
{
	Word32 ret;
	Word32 m;
	Word EndBit;
	const Word8 *WorkPtr;
	
	if (bits>32) {		/* Failsafe */
		bits = 32;
	}
	
	m=mask[bits];		/* Get the mask */
	EndBit = b->endbit;
	bits+=EndBit;		/* Total number of bits to process */
	
	if ((b->endbyte+4)>=b->storage) {		/* Off the end? */
		/* not the main path */
		if ((b->endbyte+((bits-1)>>3))>=b->storage) {
			return (Word32)(-1);
		}
	}

	WorkPtr = b->ptr;			/* Get the work pointer */
	ret=WorkPtr[0]>>EndBit;		/* Get the first byte */
	if (bits>8) {				/* More than 8 bits tp process? */
		ret|=WorkPtr[1]<<(8-EndBit);
		if (bits>16) {
			ret|=WorkPtr[2]<<(16-EndBit);  
			if (bits>24) {
				ret|=WorkPtr[3]<<(24-EndBit);  
				if (bits>32) {		/* Anything left? */
					ret|=WorkPtr[4]<<(32-EndBit);
				}
			}
		}
	}
	return (m&ret);		/* Mask off the unused bits */
}

/**********************************

	Look at a single bit from the stream, but DON'T remove the bit from the stream

**********************************/

Word BURGERCALL oggpack_look1(oggpack_buffer *b)
{
	if (b->endbyte<b->storage) {
		return ((b->ptr[0]>>b->endbit)&1);
	}
	return (Word)-1;
}

/**********************************

	Read in bits without advancing the bitptr; bits <= 8
	we never return 'out of bits'; we'll handle it on _adv
	
**********************************/

Word32 BURGERCALL oggpack_look_huff(oggpack_buffer *b,Word bits)
{
	Word32 ret;
	Word32 m;
	Word EndBit;
	const Word8 *WorkPtr;
		
	if (bits>8) {
		bits = 8;
	}
	m=mask[bits];
	EndBit = b->endbit;
	bits+=EndBit;
	WorkPtr = b->ptr;
	ret=WorkPtr[0]>>EndBit;
	if (bits>8) {
		ret|=WorkPtr[1]<<(8-EndBit);  
	}
	return (m&ret);		/* Return the byte */
}

/**********************************

	Accept a number of bits
	
**********************************/

void BURGERCALL oggpack_adv(oggpack_buffer *b,Word bits)
{
	Word Bytes;
	bits+=b->endbit;
	Bytes = bits>>3;
	b->ptr+=Bytes;
	b->endbyte+=Bytes;
	b->endbit=bits&7;
}

/**********************************

	Accept a single bit
	
**********************************/

void BURGERCALL oggpack_adv1(oggpack_buffer *b)
{
	Word EndBit;
	EndBit = b->endbit+1;
	if (EndBit>7){
		EndBit=0;
		b->ptr++;
		b->endbyte++;
	}
	b->endbit = EndBit;
}

/**********************************

	Accept up to 8 bits. Check for overflow here
	
**********************************/

int BURGERCALL oggpack_adv_huff(oggpack_buffer *b,Word bits)
{
	Word Bytes;
	Word EndByte;
	EndByte = b->endbyte;
	bits += b->endbit;
	if ((EndByte+((bits-1)>>3))>=b->storage) {
		return (-1);
	}
	Bytes = bits>>3;		/* Convert bits to bytes */
	b->ptr+=Bytes;
	b->endbyte=EndByte+Bytes;
	b->endbit=bits&7;
	return 0;
}

/**********************************

	Accept up to 32 bits. Check for overflow here
	
**********************************/

Word32 BURGERCALL oggpack_read(oggpack_buffer *b,Word bits)
{
	Word32 ret;
	Word32 m;
	Word8 *WorkPtr;
	Word EndBit;
	
	if (bits>32) {		/* Failsafe */
		bits = 32;
	}
	m=mask[bits];		/* Mask for bits to return */
	EndBit = b->endbit;
	bits+=EndBit;
	WorkPtr = b->ptr;

	if ((b->endbyte+4)>=b->storage){
		/* not the main path */
		ret=(Word32)-1;
		if ((b->endbyte+((bits-1)>>3))>=b->storage) {
			goto overflow;
		}
	}
	ret=WorkPtr[0]>>EndBit;
	if (bits>8){
		ret|=WorkPtr[1]<<(8-EndBit);  
		if (bits>16){
			ret|=WorkPtr[2]<<(16-EndBit);  
			if (bits>24){
				ret|=WorkPtr[3]<<(24-EndBit);  
				if (bits>32 && b->endbit){
					ret|=WorkPtr[4]<<(32-EndBit);
				}
			}
		}
	}
	ret&=m;		/* Mask the result */
overflow:
	EndBit = bits>>3;
	b->ptr=WorkPtr+EndBit;
	b->endbyte+=EndBit;
	b->endbit=bits&7;
	return ret;
}

/**********************************

	Accept a single bit. Check for overflow here
	
**********************************/

Word BURGERCALL oggpack_read1(oggpack_buffer *b)
{
	Word ret;
	Word EndBit;
	Word32 EndByte;
	
	EndByte = b->endbyte;
	EndBit = b->endbit;			/* Grab in locals */
	if (EndByte>=b->storage) {
		ret = (Word)-1;			/* Result */
	} else {
		ret = (b->ptr[0]>>EndBit)&1;
	}
	if (++EndBit>=8) {
		++EndByte;
		EndBit=0;
		b->ptr++;
		b->endbyte = EndByte;
	}
	b->endbit = EndBit;
	return ret;		/* Return the result 0,1, or -1 */
}

/**********************************

	Return the number of bytes accepted
	
**********************************/

Word32 BURGERCALL oggpack_bytes(oggpack_buffer *b)
{
	return (b->endbyte+((b->endbit+7)>>3));
}

/**********************************

	Return the number of bits accepted
	
**********************************/

Word32 BURGERCALL oggpack_bits(oggpack_buffer *b)
{
	return ((b->endbyte<<3)+b->endbit);
}

/**********************************

	Return the data buffer pointer
	
**********************************/

Word8 *BURGERCALL oggpack_get_buffer(oggpack_buffer *b)
{
	return b->buffer;
}
