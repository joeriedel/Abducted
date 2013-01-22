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

 function: fft transform
 last mod: $Id: PkVorbisSmallft.h,v 1.4 2003/11/22 10:43:19 burger Exp $

 ********************************************************************/

#ifndef __PKVORBISSMALLFT_H__
#define __PKVORBISSMALLFT_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

typedef struct drft_lookup {
	int n;
	float *trigcache;
	int *splitcache;
} drft_lookup;

#ifdef __cplusplus
extern "C" {
#endif

extern void BURGERCALL drft_forward(drft_lookup *l,float *data);
extern void BURGERCALL drft_backward(drft_lookup *l,float *data);
extern void BURGERCALL drft_init(drft_lookup *l,int n);
extern void BURGERCALL drft_clear(drft_lookup *l);

#ifdef __cplusplus
}
#endif

#endif
