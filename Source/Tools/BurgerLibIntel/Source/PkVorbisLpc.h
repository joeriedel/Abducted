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

  function: LPC low level routines
  last mod: $Id: PkVorbisLpc.h,v 1.4 2003/11/22 10:43:19 burger Exp $

 ********************************************************************/

#ifndef __PKVORBISLPC_H__
#define __PKVORBISLPC_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __PKVORBISSMALLFT_H__
#include "PkVorbisSmallft.h"
#endif

typedef struct lpc_lookup {
	/* en/decode lookups */
	drft_lookup fft;
	int ln;
	int m;
} lpc_lookup;

#ifdef __cplusplus
extern "C" {
#endif

extern float BURGERCALL vorbis_lpc_from_data(float *data,float *lpc,int n,int m);
extern float BURGERCALL vorbis_lpc_from_curve(float *curve,float *lpc,lpc_lookup *l);
extern void BURGERCALL lpc_init(lpc_lookup *l,long mapped, int m);
extern void BURGERCALL lpc_clear(lpc_lookup *l);
extern void BURGERCALL vorbis_lpc_predict(float *coeff,float *prime,int m,float *data,long n);

#ifdef __cplusplus
}
#endif

#endif
