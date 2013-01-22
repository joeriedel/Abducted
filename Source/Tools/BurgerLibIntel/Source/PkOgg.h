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

 function: toplevel libogg include
 last mod: $Id: PkOgg.h,v 1.6 2003/12/01 16:12:30 burger Exp $

 ********************************************************************/
#ifndef __PKOGG_H__
#define __PKOGG_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __LL64BIT_H__
#include "Ll64Bit.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct oggpack_buffer {
	Word8 *ptr;			/* Pointer to the current byte scanned */
	Word8 *buffer;		/* Pointer to the work buffer */
	Word32 endbyte;	/* Bytes accepted */
	Word32 storage;	/* Size of the buffer in bytes */
	Word endbit;		/* Work bit (0-7) */
} oggpack_buffer;

/* ogg_page is used to encapsulate the data in one Ogg bitstream page *****/

typedef struct ogg_page {
	Word8 *header;			/* Pointer to the data header */
	Word8 *body;				/* Pointer to the data */
	Word32 header_len;	/* Size of the header */
	Word32 body_len;		/* Size of the data */
} ogg_page;

/* ogg_stream_state contains the current encode/decode state of a logical
   Ogg bitstream **********************************************************/

typedef struct ogg_stream_state {
	Word8 *body_data;		/* bytes from packet bodies */
	long body_storage;		/* storage elements allocated */
	long body_fill;			/* elements stored; fill mark */
	long body_returned;		/* elements of fill returned */

	int *lacing_vals;      /* The values that will go to the segment table */
	LongWord64_t *granule_vals;	/* granulepos values for headers. Not compact */
								/* this way, but it is simple coupled to the lacing fifo */
	long lacing_storage;
	long lacing_fill;
	long lacing_packet;
	long lacing_returned;

	Word8 header[282];		/* working space for header encode */
	Word8 Padding[2];		/* Long align */
	int header_fill;

	int e_o_s;				/* set when we have buffered the last packet in the logical bitstream */
	int b_o_s;				/* set after we've written the initial page of a logical bitstream */
	long serialno;
	long pageno;
	LongWord64_t packetno;	/* sequence number for decode; the framing
								knows where there's a hole in the data,
								but we need coupling so that the codec
								(which is in a seperate abstraction
								layer) also knows about the gap */
	LongWord64_t granulepos;
} ogg_stream_state;

/* ogg_packet is used to encapsulate the data and metadata belonging
to a single raw Ogg/Vorbis packet *************************************/

typedef struct ogg_packet {
	Word8 *packet;
	long bytes;
	long b_o_s;
	long e_o_s;
	LongWord64_t granulepos;
	LongWord64_t packetno;		/* sequence number for decode; the framing
								knows where there's a hole in the data,
								but we need coupling so that the codec
								(which is in a seperate abstraction
								layer) also knows about the gap */
} ogg_packet;

typedef struct ogg_sync_state {
	Word8 *data;
	int storage;
	int fill;
	int returned;
	int unsynced;
	int headerbytes;
	int bodybytes;
} ogg_sync_state;

/* PkOggBitsize.c */

extern void BURGERCALL oggpack_writeinit(oggpack_buffer *b);
extern void BURGERCALL oggpack_reset(oggpack_buffer *b);
extern void BURGERCALL oggpack_writeclear(oggpack_buffer *b);
extern void BURGERCALL oggpack_readinit(oggpack_buffer *b,Word8 *buf,Word32 bytes);
extern void BURGERCALL oggpack_write(oggpack_buffer *b,Word32 value,Word bits);
extern Word32 BURGERCALL oggpack_look(oggpack_buffer *b,Word bits);
extern Word BURGERCALL oggpack_look1(oggpack_buffer *b);
extern Word32 BURGERCALL oggpack_look_huff(oggpack_buffer *b,Word bits);
extern void BURGERCALL oggpack_adv(oggpack_buffer *b,Word bits);
extern void BURGERCALL oggpack_adv1(oggpack_buffer *b);
extern int BURGERCALL oggpack_adv_huff(oggpack_buffer *b,Word bits);
extern Word32 BURGERCALL oggpack_read(oggpack_buffer *b,Word bits);
extern Word BURGERCALL oggpack_read1(oggpack_buffer *b);
extern Word32 BURGERCALL oggpack_bytes(oggpack_buffer *b);
extern Word32 BURGERCALL oggpack_bits(oggpack_buffer *b);
extern Word8 *BURGERCALL oggpack_get_buffer(oggpack_buffer *b);

/* PkOggFraming.c */

extern Word BURGERCALL ogg_page_version(ogg_page *og);
extern Word BURGERCALL ogg_page_continued(ogg_page *og);
extern Word BURGERCALL ogg_page_bos(ogg_page *og);
extern Word BURGERCALL ogg_page_eos(ogg_page *og);
extern LongWord64_t BURGERCALL ogg_page_granulepos(ogg_page *og);
extern Word32 BURGERCALL ogg_page_serialno(ogg_page *og);
extern Word32 BURGERCALL ogg_page_pageno(ogg_page *og);
extern Word BURGERCALL ogg_page_packets(ogg_page *og);
extern int BURGERCALL ogg_stream_init(ogg_stream_state *os,int serialno);
extern void BURGERCALL ogg_stream_clear(ogg_stream_state *os);
extern void BURGERCALL ogg_stream_destroy(ogg_stream_state *os);
extern void BURGERCALL ogg_page_checksum_set(ogg_page *og);
extern int BURGERCALL ogg_stream_packetin(ogg_stream_state *os, ogg_packet *op);
extern int BURGERCALL ogg_stream_flush(ogg_stream_state *os, ogg_page *og);
extern int BURGERCALL ogg_stream_pageout(ogg_stream_state *os, ogg_page *og);
extern int BURGERCALL ogg_stream_eos(ogg_stream_state *os);
extern int BURGERCALL ogg_sync_init(ogg_sync_state *oy);
extern int BURGERCALL ogg_sync_clear(ogg_sync_state *oy);
extern int BURGERCALL ogg_sync_destroy(ogg_sync_state *oy);
extern char *BURGERCALL ogg_sync_buffer(ogg_sync_state *oy, long size);
extern int BURGERCALL ogg_sync_wrote(ogg_sync_state *oy, long bytes);
extern long BURGERCALL ogg_sync_pageseek(ogg_sync_state *oy,ogg_page *og);
extern int BURGERCALL ogg_sync_pageout(ogg_sync_state *oy, ogg_page *og);
extern int BURGERCALL ogg_stream_pagein(ogg_stream_state *os, ogg_page *og);
extern int BURGERCALL ogg_sync_reset(ogg_sync_state *oy);
extern int BURGERCALL ogg_stream_reset(ogg_stream_state *os);
extern int BURGERCALL ogg_stream_packetout(ogg_stream_state *os,ogg_packet *op);
extern int BURGERCALL ogg_stream_packetpeek(ogg_stream_state *os,ogg_packet *op);
extern void BURGERCALL ogg_packet_clear(ogg_packet *op);

#ifdef __cplusplus
}
#endif

#endif
