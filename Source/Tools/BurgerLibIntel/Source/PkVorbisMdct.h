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

 function: modified discrete cosine transform prototypes
 last mod: $Id: PkVorbisMdct.h,v 1.6 2003/11/22 10:43:19 burger Exp $

 ********************************************************************/

#ifndef __PKVORBISMDCT_H__
#define __PKVORBISMDCT_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

/*#define MDCT_INTEGERIZED  <- be warned there could be some hurt left here*/
#ifdef MDCT_INTEGERIZED

#define DATA_TYPE int
#define REG_TYPE  register int
#define TRIGBITS 14
#define cPI3_8 6270
#define cPI2_8 11585
#define cPI1_8 15137

#define FLOAT_CONV(x) ((int)((x)*(1<<TRIGBITS)+.5))
#define MULT_NORM(x) ((x)>>TRIGBITS)
#define HALVE(x) ((x)>>1)

#else

#define DATA_TYPE float
#define REG_TYPE  float
#define cPI3_8 .38268343236508977175F
#define cPI2_8 .70710678118654752441F
#define cPI1_8 .92387953251128675613F

#define FLOAT_CONV(x) (x)
#define MULT_NORM(x) (x)
#define HALVE(x) ((x)*.5f)

#endif

#ifndef PK_PI
#define PK_PI (3.1415926536f)
#endif

typedef struct mdct_lookup {
	int n;
	int log2n;
	DATA_TYPE *trig;
	int *bitrev;
	DATA_TYPE scale;
} mdct_lookup;

#ifdef __cplusplus
extern "C" {
#endif

extern void BURGERCALL mdct_init(mdct_lookup *lookup,int n);
extern void BURGERCALL mdct_clear(mdct_lookup *l);
extern void BURGERCALL mdct_backward(mdct_lookup *init, DATA_TYPE *input, DATA_TYPE *output);
extern void BURGERCALL mdct_forward(mdct_lookup *init, DATA_TYPE *input, DATA_TYPE *output);

#ifdef __cplusplus
}
#endif

#endif












