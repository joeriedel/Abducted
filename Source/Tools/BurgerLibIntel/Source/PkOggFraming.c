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

 function: code raw [Vorbis] packets into framed OggSquish stream and
           decode Ogg streams back into raw packets
 last mod: $Id: PkOggFraming.c,v 1.6 2003/12/01 16:12:30 burger Exp $

 note: The CRC code is directly derived from public domain code by
 Ross Williams (ross@guest.adelaide.edu.au).  See docs/framing.html
 for details.

 ********************************************************************/

#include "PkOgg.h"
#include "ClStdlib.h"
#include "MmMemory.h"

/**********************************

	A complete description of Ogg framing exists in docs/framing.html

**********************************/

#define STREAM_DEFAULT_SIZE (16*1024)		/* Size of the data stream */
#define LACING_DEFAULT_SIZE 1024			/* Number of lacing units */

/**********************************

	Return the version number

**********************************/

Word BURGERCALL ogg_page_version(ogg_page *og)
{
	return((Word)(og->header[4]));
}

/**********************************

	Return TRUE if the page is continued

**********************************/

Word BURGERCALL ogg_page_continued(ogg_page *og)
{
	return((Word)(og->header[5]&0x01));
}


/**********************************

	Return the "beginning of stream" flag

**********************************/

Word BURGERCALL ogg_page_bos(ogg_page *og)
{
	return((Word)(og->header[5]&0x02));
}

/**********************************

	Return the "end of stream" flag

**********************************/

Word BURGERCALL ogg_page_eos(ogg_page *og)
{
	return((Word)(og->header[5]&0x04));
}

/**********************************

	Return the file position to this page

**********************************/

LongWord64_t BURGERCALL ogg_page_granulepos(ogg_page *og)
{
	Word8 *PagePtr;
	Word32 TempLo;
	Word32 TempHi;
	LongWord64_t granulepos;

	PagePtr = og->header;
	TempHi = PagePtr[13];
	TempHi = (TempHi<<8)|PagePtr[12];
	TempHi = (TempHi<<8)|PagePtr[11];
	TempHi = (TempHi<<8)|PagePtr[10];
	TempLo = PagePtr[9];
	TempLo = (TempLo<<8)|PagePtr[8];
	TempLo = (TempLo<<8)|PagePtr[7];
	TempLo = (TempLo<<8)|PagePtr[6];
	LongWord64FromLong2(&granulepos,TempLo,TempHi);
	return granulepos;
}

/**********************************

	Return the page serial number

**********************************/

Word32 BURGERCALL ogg_page_serialno(ogg_page *og)
{
	Word8 *PagePtr;
	Word32 Temp;
	PagePtr = og->header;
	Temp = PagePtr[17];
	Temp = (Temp<<8)|PagePtr[16];
	Temp = (Temp<<8)|PagePtr[15];
	Temp = (Temp<<8)|PagePtr[14];
	return Temp;
}
 
/**********************************

	Return the page number

**********************************/

Word32 BURGERCALL ogg_page_pageno(ogg_page *og)
{
	Word8 *PagePtr;
	Word32 Temp;
	PagePtr = og->header;
	Temp = PagePtr[21];
	Temp = (Temp<<8)|PagePtr[20];
	Temp = (Temp<<8)|PagePtr[19];
	Temp = (Temp<<8)|PagePtr[18];
	return Temp;
}


/**********************************

	returns the number of packets that are completed on this page (if
	the leading packet is begun on a previous page, but ends on this
	page, it's counted 

	NOTE:
	If a page consists of a packet begun on a previous page, and a new
	packet begun (but not completed) on this page, the return will be:
	ogg_page_packets(page)   ==1, 
	ogg_page_continued(page) !=0

	If a page happens to be a single packet that was begun on a
	previous page, and spans to the next page (in the case of a three or
	more page packet), the return will be: 
	ogg_page_packets(page)   ==0, 
	ogg_page_continued(page) !=0


**********************************/

Word BURGERCALL ogg_page_packets(ogg_page *og)
{
	Word n;
	Word count;
	
	n=og->header[26];		/* Get the count */
	count=0;
	if (n) {
		Word8 *SrcPtr;
		SrcPtr = &og->header[27];		/* Pointer to the packet array */
		do {
			if (SrcPtr[0]<255) {		/* Acceptable? */
				count++;				/* Add to the count */
			}
			++SrcPtr;
		} while (--n);					/* All done? */
	}
	return count;						/* Return the acceptable count */
}


#if 0
/**********************************

	helper to initialize lookup for direct-table CRC (illustrative; we
	use the static init below)

**********************************/

static Word32 BURGERCALL _ogg_crc_entry(unsigned long index)
{
	int i;
	Word32 r;

	r = index << 24;
	for (i=0; i<8; i++) {
		if (r & 0x80000000UL) {
			r = (r << 1) ^ 0x04c11db7; /* The same as the ethernet generator
										polynomial, although we use an
										unreflected alg and an init/final
										of 0, not 0xffffffff */
		} else {
			r<<=1;
		}
	}
	return r;
}
#endif

static const Word32 crc_lookup[256]={
	0x00000000,0x04c11db7,0x09823b6e,0x0d4326d9,
	0x130476dc,0x17c56b6b,0x1a864db2,0x1e475005,
	0x2608edb8,0x22c9f00f,0x2f8ad6d6,0x2b4bcb61,
	0x350c9b64,0x31cd86d3,0x3c8ea00a,0x384fbdbd,
	0x4c11db70,0x48d0c6c7,0x4593e01e,0x4152fda9,
	0x5f15adac,0x5bd4b01b,0x569796c2,0x52568b75,
	0x6a1936c8,0x6ed82b7f,0x639b0da6,0x675a1011,
	0x791d4014,0x7ddc5da3,0x709f7b7a,0x745e66cd,
	0x9823b6e0,0x9ce2ab57,0x91a18d8e,0x95609039,
	0x8b27c03c,0x8fe6dd8b,0x82a5fb52,0x8664e6e5,
	0xbe2b5b58,0xbaea46ef,0xb7a96036,0xb3687d81,
	0xad2f2d84,0xa9ee3033,0xa4ad16ea,0xa06c0b5d,
	0xd4326d90,0xd0f37027,0xddb056fe,0xd9714b49,
	0xc7361b4c,0xc3f706fb,0xceb42022,0xca753d95,
	0xf23a8028,0xf6fb9d9f,0xfbb8bb46,0xff79a6f1,
	0xe13ef6f4,0xe5ffeb43,0xe8bccd9a,0xec7dd02d,
	0x34867077,0x30476dc0,0x3d044b19,0x39c556ae,
	0x278206ab,0x23431b1c,0x2e003dc5,0x2ac12072,
	0x128e9dcf,0x164f8078,0x1b0ca6a1,0x1fcdbb16,
	0x018aeb13,0x054bf6a4,0x0808d07d,0x0cc9cdca,
	0x7897ab07,0x7c56b6b0,0x71159069,0x75d48dde,
	0x6b93dddb,0x6f52c06c,0x6211e6b5,0x66d0fb02,
	0x5e9f46bf,0x5a5e5b08,0x571d7dd1,0x53dc6066,
	0x4d9b3063,0x495a2dd4,0x44190b0d,0x40d816ba,
	0xaca5c697,0xa864db20,0xa527fdf9,0xa1e6e04e,
	0xbfa1b04b,0xbb60adfc,0xb6238b25,0xb2e29692,
	0x8aad2b2f,0x8e6c3698,0x832f1041,0x87ee0df6,
	0x99a95df3,0x9d684044,0x902b669d,0x94ea7b2a,
	0xe0b41de7,0xe4750050,0xe9362689,0xedf73b3e,
	0xf3b06b3b,0xf771768c,0xfa325055,0xfef34de2,
	0xc6bcf05f,0xc27dede8,0xcf3ecb31,0xcbffd686,
	0xd5b88683,0xd1799b34,0xdc3abded,0xd8fba05a,
	0x690ce0ee,0x6dcdfd59,0x608edb80,0x644fc637,
	0x7a089632,0x7ec98b85,0x738aad5c,0x774bb0eb,
	0x4f040d56,0x4bc510e1,0x46863638,0x42472b8f,
	0x5c007b8a,0x58c1663d,0x558240e4,0x51435d53,
	0x251d3b9e,0x21dc2629,0x2c9f00f0,0x285e1d47,
	0x36194d42,0x32d850f5,0x3f9b762c,0x3b5a6b9b,
	0x0315d626,0x07d4cb91,0x0a97ed48,0x0e56f0ff,
	0x1011a0fa,0x14d0bd4d,0x19939b94,0x1d528623,
	0xf12f560e,0xf5ee4bb9,0xf8ad6d60,0xfc6c70d7,
	0xe22b20d2,0xe6ea3d65,0xeba91bbc,0xef68060b,
	0xd727bbb6,0xd3e6a601,0xdea580d8,0xda649d6f,
	0xc423cd6a,0xc0e2d0dd,0xcda1f604,0xc960ebb3,
	0xbd3e8d7e,0xb9ff90c9,0xb4bcb610,0xb07daba7,
	0xae3afba2,0xaafbe615,0xa7b8c0cc,0xa379dd7b,
	0x9b3660c6,0x9ff77d71,0x92b45ba8,0x9675461f,
	0x8832161a,0x8cf30bad,0x81b02d74,0x857130c3,
	0x5d8a9099,0x594b8d2e,0x5408abf7,0x50c9b640,
	0x4e8ee645,0x4a4ffbf2,0x470cdd2b,0x43cdc09c,
	0x7b827d21,0x7f436096,0x7200464f,0x76c15bf8,
	0x68860bfd,0x6c47164a,0x61043093,0x65c52d24,
	0x119b4be9,0x155a565e,0x18197087,0x1cd86d30,
	0x029f3d35,0x065e2082,0x0b1d065b,0x0fdc1bec,
	0x3793a651,0x3352bbe6,0x3e119d3f,0x3ad08088,
	0x2497d08d,0x2056cd3a,0x2d15ebe3,0x29d4f654,
	0xc5a92679,0xc1683bce,0xcc2b1d17,0xc8ea00a0,
	0xd6ad50a5,0xd26c4d12,0xdf2f6bcb,0xdbee767c,
	0xe3a1cbc1,0xe760d676,0xea23f0af,0xeee2ed18,
	0xf0a5bd1d,0xf464a0aa,0xf9278673,0xfde69bc4,
	0x89b8fd09,0x8d79e0be,0x803ac667,0x84fbdbd0,
	0x9abc8bd5,0x9e7d9662,0x933eb0bb,0x97ffad0c,
	0xafb010b1,0xab710d06,0xa6322bdf,0xa2f33668,
	0xbcb4666d,0xb8757bda,0xb5365d03,0xb1f740b4};

/**********************************

	Init the output stream

**********************************/

int BURGERCALL ogg_stream_init(ogg_stream_state *os,int serialno)
{
	if (os) {								/* Am I nuts?!?!? */
		FastMemSet(os,0,sizeof(*os));		/* Zap the structure */
		os->serialno=serialno;				/* Save the serial number */
		os->body_storage=STREAM_DEFAULT_SIZE;			/* Default storage */
		if ((os->body_data=static_cast<Word8 *>(AllocAPointer(STREAM_DEFAULT_SIZE*sizeof(*os->body_data))))!=0) {
			os->lacing_storage=LACING_DEFAULT_SIZE;
			if ((os->lacing_vals=static_cast<int *>(AllocAPointer(LACING_DEFAULT_SIZE*sizeof(*os->lacing_vals))))!=0) {
				if ((os->granule_vals=static_cast<SWord64 *>(AllocAPointer(LACING_DEFAULT_SIZE*sizeof(*os->granule_vals))))!=0) {
					return 0;
				}
				DeallocAPointer(os->lacing_vals);	/* Release the memory */
			}
			DeallocAPointer(os->body_data);		/* Release the memory */
		}
		FastMemSet(os,0,sizeof(*os));		/* Zap the structure */
	}
	return(-1);		/* Ok, I'm boned */
} 

/**********************************

	Clear does not free os, only the non-flat storage within 

**********************************/

void BURGERCALL ogg_stream_clear(ogg_stream_state *os)
{
	if (os) {
		DeallocAPointer(os->body_data);
		DeallocAPointer(os->lacing_vals);
		DeallocAPointer(os->granule_vals);
		FastMemSet(os,0,sizeof(*os));    
	}
} 

/**********************************

	Delete the structure completely

**********************************/

void BURGERCALL ogg_stream_destroy(ogg_stream_state *os)
{
	if (os) {
		ogg_stream_clear(os);
		DeallocAPointer(os);
	}
} 

/**********************************

	Helpers for ogg_stream_encode; this keeps the structure and
	what's happening fairly clear

**********************************/

static void BURGERCALL _os_body_expand(ogg_stream_state *os,Word needed)
{
	long Storage;
	Storage = os->body_storage;		/* Get into a register temp */
	if (Storage<=(os->body_fill+(int)needed)) {		/* Am I ok? */
		Storage+=(needed+1024);		/* Increase the buffer */
		os->body_storage = Storage;	/* Save the new size */
		os->body_data=static_cast<Word8 *>(ResizeAPointer(os->body_data,Storage*sizeof(*os->body_data)));
	}
}

/**********************************

	Small expand

**********************************/

static void BURGERCALL _os_lacing_expand(ogg_stream_state *os,Word needed)
{
	long Storage;
	Storage = os->lacing_storage;
	if (Storage<=(os->lacing_fill+(int)needed)) {
		Storage+=(needed+32);
		os->lacing_storage = Storage;	/* Save the new size */
		os->lacing_vals=static_cast<int *>(ResizeAPointer(os->lacing_vals,Storage*sizeof(*os->lacing_vals)));
		os->granule_vals=static_cast<SWord64 *>(ResizeAPointer(os->granule_vals,Storage*sizeof(*os->granule_vals)));
	}
}


/**********************************

	checksum the page
	Direct table CRC; note that this will be faster in the future if we
	perform the checksum silmultaneously with other copies

**********************************/

void BURGERCALL ogg_page_checksum_set(ogg_page *og)
{
	if (og) {
		Word32 crc_reg;
		Word8 *DataPtr;
		const Word8 *WorkPtr;
		Word i;

		DataPtr = og->header;
		/* safety; needed for API behavior, but not framing code */
		DataPtr[22]=0;		/* In case I am CRC'ing the header */
		DataPtr[23]=0;
		DataPtr[24]=0;
		DataPtr[25]=0;

		crc_reg = 0;			/* Init the checksum */
		
		/* Checksum for the header */
		
		i = og->header_len;
		if (i) {
			WorkPtr = DataPtr;
			do {
				crc_reg=(crc_reg<<8)^crc_lookup[((crc_reg >> 24)&0xff)^WorkPtr[0]];
				++WorkPtr;
			} while (--i);
		}
		
		/* Checksum for the body */
		
		i = og->body_len;
		if (i) {
			WorkPtr = og->body;
			do {
				crc_reg=(crc_reg<<8)^crc_lookup[((crc_reg >> 24)&0xff)^WorkPtr[0]];
				++WorkPtr;
			} while (--i);
		}
		DataPtr[22]=(Word8)crc_reg;
		DataPtr[23]=(Word8)(crc_reg>>8);
		DataPtr[24]=(Word8)(crc_reg>>16);
		DataPtr[25]=(Word8)(crc_reg>>24);
	}
}

/**********************************

	submit data to the internal buffer of the framing engine

**********************************/

int BURGERCALL ogg_stream_packetin(ogg_stream_state *os,ogg_packet *op)
{
	int i;
	int lacing_vals;
		
	lacing_vals=op->bytes/255+1;

	{
		long Returned;
		Returned = os->body_returned;
		if (Returned){
			/* advance packet data according to the body_returned pointer. We
				had to keep it around to return a pointer into the buffer last call */
			long BodyFill;
			BodyFill = os->body_fill-Returned;
			os->body_fill=BodyFill;
			if (BodyFill) {
				memmove(os->body_data,os->body_data+Returned,BodyFill);
			}
			os->body_returned=0;		/* Zap the returned flag */
		}
	}
	
	/* make sure we have the buffer storage */
	
	_os_body_expand(os,op->bytes);
	_os_lacing_expand(os,lacing_vals);

	/* Copy in the submitted packet.  Yes, the copy is a waste; this is
	the liability of overly clean abstraction for the time being.  It
	will actually be fairly easy to eliminate the extra copy in the future */

	FastMemCpy(os->body_data+os->body_fill,op->packet,op->bytes);
	os->body_fill+=op->bytes;

	/* Store lacing vals for this packet */
	{
		int *LacingPtr;
		LongWord64_t *GranPtr;

		i = os->lacing_fill;
		LacingPtr = &os->lacing_vals[i];
		GranPtr = &os->granule_vals[i];
		if (lacing_vals>1) {
			LongWord64_t PosTemp;

			PosTemp = os->granulepos;
			i = lacing_vals-1;
			do {
				LacingPtr[0]=255;
				++LacingPtr;
				GranPtr[0]=PosTemp;
				++GranPtr;
			} while (--i);
		}
		LacingPtr[0]=(op->bytes)%255;
		os->granulepos=GranPtr[0]=op->granulepos;
	}
	/* flag the first segment as the beginning of the packet */
	os->lacing_vals[os->lacing_fill]|= 0x100;
	os->lacing_fill+=lacing_vals;

	/* for the sake of completeness */

	os->packetno++;
	if (op->e_o_s) {		/* Make sure that this is TRUE or FALSE */
		os->e_o_s=1;
	}
	return 0;				/* I'm ok */
}


/**********************************

	This will flush remaining packets into a page (returning nonzero),
	even if there is not enough data to trigger a flush normally
	(undersized page). If there are no packets or partial packets to
	flush, ogg_stream_flush returns 0.  Note that ogg_stream_flush will
	try to flush a normal sized page like ogg_stream_pageout; a call to
	ogg_stream_flush does not gurantee that all packets have flushed.
	Only a return value of 0 from ogg_stream_flush indicates all packet
	data is flushed into pages.

	ogg_stream_page will flush the last page in a stream even if it's
	undersized; you almost certainly want to use ogg_stream_pageout
	(and *not* ogg_stream_flush) unless you need to flush an undersized
	page in the middle of a stream for some reason.

**********************************/

int BURGERCALL ogg_stream_flush(ogg_stream_state *os,ogg_page *og)
{
	int i;
	int vals;
	int maxvals;
	int bytes;
	LongWord64_t granule_pos;

	maxvals=os->lacing_fill;
	if (maxvals>255) {
		maxvals = 255;
	}
	if (!maxvals) {
		return 0;
	}

	/* construct a page */
	/* decide how many segments to include */

	/* If this is the initial header case, the first page must only include
	the initial header packet */
	if (os->b_o_s==0) {  /* 'initial header page' case */
		granule_pos=0;
		for(vals=0;vals<maxvals;vals++){
			if((os->lacing_vals[vals]&0x0ff)<255){
				vals++;
				break;
			}
		}
	} else{
		long acc;
		acc = 0;
		granule_pos=os->granule_vals[0];
		
		for(vals=0;vals<maxvals;vals++){
			if(acc>4096) {
				break;
			}
			acc+=os->lacing_vals[vals]&0x0ff;
			granule_pos=os->granule_vals[vals];
		}
	}

	/* construct the header in temp storage */
	FastMemCpy(os->header,"OggS",4);

	/* stream structure version */
	os->header[4]=0x00;

	/* continued packet flag? */
	i=0x00;
	if ((os->lacing_vals[0]&0x100)==0) {
		i|=0x01;
	}
	/* first page flag? */
	if (os->b_o_s==0) {
		i|=0x02;
	}
	/* last page flag? */
	if(os->e_o_s && os->lacing_fill==vals) {
		i|=0x04;
	}
	os->header[5] = (Word8)i;
	os->b_o_s=1;

	/* 64 bits of PCM position */
	i = 6;
	do {
		os->header[i]=(Word8)(granule_pos&0xff);
		granule_pos>>=8;
	} while (++i<14);

	/* 32 bits of stream serial number */
	{
		long serialno=os->serialno;
//		i = 14;
		do {
			os->header[i]=(Word8)(serialno&0xff);
			serialno>>=8;
		} while (++i<18);
	}

	/* 32 bits of page counter (we have both counter and page header
	because this val can roll over) */
	if(os->pageno==-1) {
		os->pageno=0; /* because someone called stream_reset; this would be a
			strange thing to do in an encode stream, but it has
			plausible uses */
		{
			long pageno=os->pageno++;
//			i = 18;
			do {
				os->header[i]=(Word8)(pageno&0xff);
				pageno>>=8;
			} while (++i<22);
		}
	}

	/* zero for computation; filled in later */
	os->header[22]=0;
	os->header[23]=0;
	os->header[24]=0;
	os->header[25]=0;

	/* segment table */
	os->header[26]=(Word8)vals;
	bytes = 0;
	for(i=0;i<vals;i++) {
		bytes+=os->header[i+27]=(Word8)os->lacing_vals[i];
	}
	/* set pointers in the ogg_page struct */
	og->header=os->header;
	og->header_len=os->header_fill=vals+27;
	og->body=os->body_data+os->body_returned;
	og->body_len=bytes;

	/* advance the lacing data and set the body_returned pointer */

	os->lacing_fill-=vals;
	memmove(os->lacing_vals,os->lacing_vals+vals,os->lacing_fill*sizeof(*os->lacing_vals));
	memmove(os->granule_vals,os->granule_vals+vals,os->lacing_fill*sizeof(*os->granule_vals));
	os->body_returned+=bytes;

	/* calculate the checksum */

	ogg_page_checksum_set(og);

	/* done */
	return 1;
}


/**********************************

	This constructs pages from buffered packet segments.  The pointers
	returned are to static buffers; do not free. The returned buffers are
	good only until the next call (using the same ogg_stream_state)

**********************************/

int BURGERCALL ogg_stream_pageout(ogg_stream_state *os, ogg_page *og)
{

	if ((os->e_o_s&&os->lacing_fill) ||				/* 'were done, now flush' case */
		os->body_fill-os->body_returned > 4096 ||	/* 'page nominal size' case */
		os->lacing_fill>=255 ||						/* 'segment table full' case */
		(os->lacing_fill&&!os->b_o_s)) {			/* 'initial header page' case */
		return ogg_stream_flush(os,og);
	}
	/* not enough data to construct a page and not end of stream */
	return 0;
}

/**********************************

	Return the End of Stream flag

**********************************/

int BURGERCALL ogg_stream_eos(ogg_stream_state *os)
{
	return os->e_o_s;
}

/**********************************

	This has two layers to place more of the multi-serialno and paging
	control in the application's hands.  First, we expose a data buffer
	using ogg_sync_buffer().  The app either copies into the
	buffer, or passes it directly to read(), etc.  We then call
	ogg_sync_wrote() to tell how many bytes we just added.

	Pages are returned (pointers into the buffer in ogg_sync_state)
	by ogg_sync_pageout().  The page is then submitted to
	ogg_stream_pagein() along with the appropriate
	ogg_stream_state* (ie, matching serialno).  We then get raw
	packets out calling ogg_stream_packetout() with a
	ogg_stream_state.  See the 'frame-prog.txt' docs for details and
	example code.

**********************************/

/**********************************

	initialize the struct to a known state

**********************************/

int BURGERCALL ogg_sync_init(ogg_sync_state *oy)
{
	if (oy) {
		FastMemSet(oy,0,sizeof(*oy));
	}
	return(0);
}

/**********************************

	clear non-flat storage within

**********************************/

int BURGERCALL ogg_sync_clear(ogg_sync_state *oy)
{
	if (oy) {
		DeallocAPointer(oy->data);
		FastMemSet(oy,0,sizeof(*oy));
	}
	return(0);
}

/**********************************

	delete the sync structure

**********************************/

int BURGERCALL ogg_sync_destroy(ogg_sync_state *oy)
{
	if(oy){
		ogg_sync_clear(oy);
		DeallocAPointer(oy);
	}
	return(0);
}

/**********************************

	Get a pointer to a buffer to read new data to.

**********************************/

char *BURGERCALL ogg_sync_buffer(ogg_sync_state *oy,long size)
{

	/* first, clear out any space that has been previously returned */
	if (oy->returned){
		oy->fill-=oy->returned;
		if (oy->fill>0) {
			memmove(oy->data,oy->data+oy->returned,oy->fill);
		}
		oy->returned=0;
	}

	if (size>(oy->storage-oy->fill)) {
		/* We need to extend the internal buffer */
		long newsize=size+oy->fill+4096; /* an extra page to be nice */

		oy->data=static_cast<Word8 *>(ResizeAPointer(oy->data,newsize));
		oy->storage=newsize;
	}

	/* expose a segment at least as large as requested at the fill mark */
	return ((char *)oy->data+oy->fill);
}

int BURGERCALL ogg_sync_wrote(ogg_sync_state *oy, long bytes)
{
	bytes += oy->fill;
	if (bytes>oy->storage) {
		return -1;
	}
	oy->fill=bytes;
	return 0;
}

/**********************************

	sync the stream.  This is meant to be useful for finding page
	boundaries.

	return values for this:
	-n) skipped n bytes
	0) page not ready; more data (no bytes skipped)
	n) page synced at current location; page length n bytes

**********************************/

long BURGERCALL ogg_sync_pageseek(ogg_sync_state *oy,ogg_page *og)
{
	Word8 *page=oy->data+oy->returned;
	Word8 *next;
	long bytes=oy->fill-oy->returned;

	if(oy->headerbytes==0){
		int headerbytes,i;
		if(bytes<27) {
			return(0); /* not enough for a header */
		}
		/* verify capture pattern */
		if(memcmp(page,"OggS",4)) {
			goto sync_fail;
		}
		headerbytes=page[26]+27;
		if(bytes<headerbytes) {
			return(0); /* not enough for header + seg table */
		}
	/* count up body length in the segment table */

		for(i=0;i<page[26];i++) {
			oy->bodybytes+=page[27+i];
		}
		oy->headerbytes=headerbytes;
	}

	if (oy->bodybytes+oy->headerbytes>bytes) {
		return(0);
	}
	/* The whole test page is buffered.  Verify the checksum */
	{
		/* Grab the checksum bytes, set the header field to zero */
		char chksum[4];
		ogg_page log;

		FastMemCpy(chksum,page+22,4);
		FastMemSet(page+22,0,4);

		/* set up a temp page struct and recompute the checksum */
		log.header=page;
		log.header_len=oy->headerbytes;
		log.body=page+oy->headerbytes;
		log.body_len=oy->bodybytes;
		ogg_page_checksum_set(&log);

		/* Compare */
		if(memcmp(chksum,page+22,4)){
			/* D'oh.  Mismatch! Corrupt page (or miscapture and not a page at all) */
			/* replace the computed checksum with the one actually read in */
			FastMemCpy(page+22,chksum,4);
			/* Bad checksum. Lose sync */
			goto sync_fail;
		}
	}

	/* yes, have a whole page all ready to go */
	{
		Word8 *page=oy->data+oy->returned;
		long bytes;

		if(og){
			og->header=page;
			og->header_len=oy->headerbytes;
			og->body=page+oy->headerbytes;
			og->body_len=oy->bodybytes;
		}

		oy->unsynced=0;
		oy->returned+=(bytes=oy->headerbytes+oy->bodybytes);
		oy->headerbytes=0;
		oy->bodybytes=0;
		return(bytes);
	}

sync_fail:

	oy->headerbytes=0;
	oy->bodybytes=0;

	/* search for possible capture */
	next=static_cast<Word8 *>(memchr(page+1,'O',bytes-1));
	if(!next) {
		next=oy->data+oy->fill;
	}
	oy->returned=next-oy->data;
	return(-(next-page));
}

/**********************************

	sync the stream and get a page.  Keep trying until we find a page.
	Supress 'sync errors' after reporting the first.

	return values:
	-1) recapture (hole in data)
	0) need more data
	1) page returned

	Returns pointers into buffered data; invalidated by next call to
	_stream, _clear, _init, or _buffer

**********************************/

int BURGERCALL ogg_sync_pageout(ogg_sync_state *oy, ogg_page *og)
{
	/* all we need to do is verify a page at the head of the stream
		buffer.  If it doesn't verify, we look for the next potential frame */

	while(1){
		long ret=ogg_sync_pageseek(oy,og);
		if(ret>0){
			/* have a page */
			return(1);
		}
		if(ret==0){
			/* need more data */
			return(0);
		}
		/* head did not start a synced page... skipped some bytes */
		if(!oy->unsynced){
			oy->unsynced=1;
			return(-1);
		}
	/* loop. keep looking */
	}
}

/**********************************

	add the incoming page to the stream state; we decompose the page
	into packet segments here as well.

**********************************/

int BURGERCALL ogg_stream_pagein(ogg_stream_state *os, ogg_page *og)
{
	Word8 *header=og->header;
	Word8 *body=og->body;
	long bodysize=og->body_len;
	int segptr=0;

	int version=ogg_page_version(og);
	int continued=ogg_page_continued(og);
	int bos=ogg_page_bos(og);
	int eos=ogg_page_eos(og);
	LongWord64_t granulepos=ogg_page_granulepos(og);
	int serialno=ogg_page_serialno(og);
	long pageno=ogg_page_pageno(og);
	int segments=header[26];

	/* clean up 'returned data' */
	{
		long lr=os->lacing_returned;
		long br=os->body_returned;

		/* body data */
		if(br) {
			os->body_fill-=br;
			if(os->body_fill) {
				memmove(os->body_data,os->body_data+br,os->body_fill);
			}
			os->body_returned=0;
		}

		if(lr){
			/* segment table */
			if(os->lacing_fill-lr){
				memmove(os->lacing_vals,os->lacing_vals+lr,
					(os->lacing_fill-lr)*sizeof(*os->lacing_vals));
				memmove(os->granule_vals,os->granule_vals+lr,
					(os->lacing_fill-lr)*sizeof(*os->granule_vals));
			}
			os->lacing_fill-=lr;
			os->lacing_packet-=lr;
			os->lacing_returned=0;
		}
	}

	/* check the serial number */
	if(serialno!=os->serialno) {
		return(-1);
	}
	if(version>0) {
		return(-1);
	}
	_os_lacing_expand(os,segments+1);

	/* are we in sequence? */
	if(pageno!=os->pageno){
		int i;

		/* unroll previous partial packet (if any) */
		for(i=os->lacing_packet;i<os->lacing_fill;i++) {
			os->body_fill-=os->lacing_vals[i]&0xff;
		}
		os->lacing_fill=os->lacing_packet;

		/* make a note of dropped data in segment table */
		if(os->pageno!=-1){
			os->lacing_vals[os->lacing_fill++]=0x400;
			os->lacing_packet++;
		}

		/* are we a 'continued packet' page?  If so, we'll need to skip
		some segments */
		if(continued){
			bos=0;
			for(;segptr<segments;segptr++){
				int val=header[27+segptr];
				body+=val;
				bodysize-=val;
				if(val<255){
					segptr++;
					break;
				}
			}
		}
	}

	if(bodysize){
		_os_body_expand(os,bodysize);
		FastMemCpy(os->body_data+os->body_fill,body,bodysize);
		os->body_fill+=bodysize;
	}

	{
		int saved=-1;
		while(segptr<segments){
			int val=header[27+segptr];
			os->lacing_vals[os->lacing_fill]=val;
			os->granule_vals[os->lacing_fill]=-1;
			if(bos){
				os->lacing_vals[os->lacing_fill]|=0x100;
				bos=0;
			}
			if(val<255) {
				saved=os->lacing_fill;
			}
			os->lacing_fill++;
			segptr++;

			if(val<255) {
				os->lacing_packet=os->lacing_fill;
			}
		}

		/* set the granulepos on the last granuleval of the last full packet */
		if(saved!=-1){
			os->granule_vals[saved]=granulepos;
		}
	}

	if(eos){
		os->e_o_s=1;
		if(os->lacing_fill>0) {
			os->lacing_vals[os->lacing_fill-1]|=0x200;
		}
	}
	os->pageno=pageno+1;
	return(0);
}

/**********************************

	clear things to an initial state.  Good to call, eg, before seeking

**********************************/

int BURGERCALL ogg_sync_reset(ogg_sync_state *oy)
{
	oy->fill=0;
	oy->returned=0;
	oy->unsynced=0;
	oy->headerbytes=0;
	oy->bodybytes=0;
	return 0;
}

/**********************************

	reset the stream

**********************************/

int BURGERCALL ogg_stream_reset(ogg_stream_state *os)
{
	os->body_fill=0;
	os->body_returned=0;

	os->lacing_fill=0;
	os->lacing_packet=0;
	os->lacing_returned=0;

	os->header_fill=0;

	os->e_o_s=0;
	os->b_o_s=0;
	os->pageno=-1;
	os->packetno=0;
	os->granulepos=0;

	return(0);
}

/**********************************

	The last part of decode. We have the stream broken into packet
	segments.  Now we need to group them into packets (or return the
	out of sync markers)

**********************************/

static int BURGERCALL _packetout(ogg_stream_state *os,ogg_packet *op,Word adv)
{
	int ptr=os->lacing_returned;

	if(os->lacing_packet<=ptr) {
		return(0);
	}
	if(os->lacing_vals[ptr]&0x400){
		/* we need to tell the codec there's a gap; it might need to
		handle previous packet dependencies. */
		os->lacing_returned++;
		os->packetno++;
		return(-1);
	}

	if (op || adv) {	/* just using peek as an inexpensive way
						to ask if there's a whole packet
						waiting */
	/* Gather the whole packet. We'll have no holes or a partial packet */
		int size=os->lacing_vals[ptr]&0xff;
		int bytes=size;
		int eos=os->lacing_vals[ptr]&0x200; /* last packet of the stream? */
		int bos=os->lacing_vals[ptr]&0x100; /* first packet of the stream? */

		while(size==255){
			int val;
			val=os->lacing_vals[++ptr];
			size=val&0xff;
			if(val&0x200) {
				eos=0x200;
			}
			bytes+=size;
		}

		if (op) {
			op->e_o_s=eos;
			op->b_o_s=bos;
			op->packet=os->body_data+os->body_returned;
			op->packetno=os->packetno;
			op->granulepos=os->granule_vals[ptr];
			op->bytes=bytes;
		}
		if (adv) {
			os->body_returned+=bytes;
			os->lacing_returned=ptr+1;
			os->packetno++;
		}
	}
	return(1);
}

/**********************************

	Output a packet

**********************************/

int BURGERCALL ogg_stream_packetout(ogg_stream_state *os,ogg_packet *op)
{
	return _packetout(os,op,TRUE);
}

/**********************************

	Peek a packet

**********************************/

int BURGERCALL ogg_stream_packetpeek(ogg_stream_state *os,ogg_packet *op)
{
	return _packetout(os,op,FALSE);
}

/**********************************

	Clear a packet

**********************************/

void BURGERCALL ogg_packet_clear(ogg_packet *op)
{
	DeallocAPointer(op->packet);
	FastMemSet(op, 0, sizeof(*op));
}


