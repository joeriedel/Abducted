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

 function: basic shared codebook operations
 last mod: $Id: PkVorbisCodebook.h,v 1.4 2003/11/22 10:43:19 burger Exp $

 ********************************************************************/

#ifndef __PKVORBISCODEBOOK_H__
#define __PKVORBISCODEBOOK_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __PKOGG_H__
#include "PkOgg.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* This structure encapsulates huffman and VQ style encoding books; it
   doesn't do anything specific to either.

   valuelist/quantlist are nonNULL (and q_* significant) only if
   there's entry->value mapping to be done.

   If encode-side mapping must be done (and thus the entry needs to be
   hunted), the auxiliary encode pointer will point to a decision
   tree.  This is true of both VQ and huffman, but is mostly useful
   with VQ.

*/

typedef struct static_codebook{
	long   dim;            /* codebook dimensions (elements per vector) */
	long   entries;        /* codebook entries */
	long  *lengthlist;     /* codeword lengths in bits */

	/* mapping ***************************************************************/
	int    maptype;        /* 0=none
	1=implicitly populated values from map column 
	2=listed arbitrary values */

	/* The below does a linear, single monotonic sequence mapping. */
	long     q_min;       /* packed 32 bit float; quant value 0 maps to minval */
	long     q_delta;     /* packed 32 bit float; val 1 - val 0 == delta */
	int      q_quant;     /* bits: 0 < quant <= 16 */
	int      q_sequencep; /* bitflag */

	long     *quantlist;  /* map == 1: (int)(entries^(1/dim)) element column map
							map == 2: list of dim*entries quantized entry vals */

	/* encode helpers ********************************************************/
	struct encode_aux_nearestmatch *nearest_tree;
	struct encode_aux_threshmatch  *thresh_tree;
	struct encode_aux_pigeonhole  *pigeon_tree;

	int allocedp;
} static_codebook;

/* this structures an arbitrary trained book to quickly find the
	nearest cell match */
typedef struct encode_aux_nearestmatch {
	/* pre-calculated partitioning tree */
	long   *ptr0;
	long   *ptr1;

	long   *p;         /* decision points (each is an entry) */
	long   *q;         /* decision points (each is an entry) */
	long   aux;        /* number of tree entries */
	long   alloc;       
} encode_aux_nearestmatch;

/* assumes a maptype of 1; encode side only, so that's OK */
typedef struct encode_aux_threshmatch{
	float *quantthresh;
	long   *quantmap;
	int     quantvals; 
	int     threshvals; 
} encode_aux_threshmatch;

typedef struct encode_aux_pigeonhole{
	float min;
	float del;

	int  mapentries;
	int  quantvals;
	long *pigeonmap;

	long fittotal;
	long *fitlist;
	long *fitmap;
	long *fitlength;
} encode_aux_pigeonhole;

typedef struct decode_aux{
	long   *tab;
	int    *tabl;
	int    tabn;

	long   *ptr0;
	long   *ptr1;
	long   aux;        /* number of tree entries */
} decode_aux;

typedef struct codebook{
	long dim;           /* codebook dimensions (elements per vector) */
	long entries;       /* codebook entries */
	const static_codebook *c;

	float  *valuelist;  /* list of dim*entries actual entry values */
	long   *codelist;   /* list of bitstream codewords for each entry */
	struct decode_aux *decode_tree;

	long zeroentry;
} codebook;

/* In PkVorbisCodebook.c */

extern int BURGERCALL vorbis_staticbook_pack(const static_codebook *c,oggpack_buffer *b);
extern int BURGERCALL vorbis_staticbook_unpack(oggpack_buffer *b,static_codebook *c);
extern int BURGERCALL vorbis_book_encode(codebook *book, int a, oggpack_buffer *b);
extern int BURGERCALL vorbis_book_errorv(codebook *book, float *a);
extern int BURGERCALL vorbis_book_encodev(codebook *book, int best,float *a,oggpack_buffer *b);
extern int BURGERCALL vorbis_book_encodevs(codebook *book, float *a, oggpack_buffer *b,int step,int stagetype);
extern long BURGERCALL vorbis_book_decode(codebook *book, oggpack_buffer *b);
extern long BURGERCALL vorbis_book_decodevs_add(codebook *book, float *a, oggpack_buffer *b,int n);
extern long BURGERCALL vorbis_book_decodev_add(codebook *book, float *a, oggpack_buffer *b,int n);
extern long BURGERCALL vorbis_book_decodev_set(codebook *book, float *a, oggpack_buffer *b,int n);
extern long BURGERCALL vorbis_book_decodevv_add(codebook *book, float **a,long off,int ch, oggpack_buffer *b,int n);

/* In PkVorbisSharedBook.c */

extern int BURGERCALL _ilog(Word v);
extern long BURGERCALL _float32_pack(float val);
extern float BURGERCALL _float32_unpack(long val);
extern long BURGERCALL _book_maptype1_quantvals(const static_codebook *b);
extern float *BURGERCALL _book_unquantize(const static_codebook *b);
extern void BURGERCALL vorbis_staticbook_clear(static_codebook *b);
extern void BURGERCALL vorbis_staticbook_destroy(static_codebook *b);
extern void BURGERCALL vorbis_book_clear(codebook *b);
extern int BURGERCALL vorbis_book_init_encode(codebook *dest,const static_codebook *source);
extern int BURGERCALL vorbis_book_init_decode(codebook *dest,const static_codebook *source);
extern int BURGERCALL _best(codebook *book, float *a, int step);
extern int BURGERCALL vorbis_book_besterror(codebook *book,float *a,int step,int addmul);
extern long BURGERCALL vorbis_book_codeword(codebook *book,int entry);
extern long BURGERCALL vorbis_book_codelen(codebook *book,int entry);

#ifdef __cplusplus
}
#endif

#endif
