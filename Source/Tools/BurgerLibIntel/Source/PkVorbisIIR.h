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

  function: Direct Form I, II IIR filters, plus some specializations
  last mod: $Id: PkVorbisIIR.h,v 1.4 2003/11/22 10:43:19 burger Exp $

 ********************************************************************/

#ifndef __PKVORBISIIR_H__
#define __PKVORBISIIR_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

typedef struct IIRState_t {
	int stages;
	float *coeff_A;
	float *coeff_B;
	float *z_A;
	int ring;
	float gain;
} IIRState_t;

#ifdef __cplusplus
extern "C" {
#endif

extern void BURGERCALL IIRStateInit(IIRState_t *s,int stages,float gain,const float *A,const float *B);
extern void BURGERCALL IIRStateDestroy(IIRState_t *s);
extern void BURGERCALL IIRStateReset(IIRState_t *s);
extern float BURGERCALL IIRStateFilter(IIRState_t *s,float in);
extern float BURGERCALL IIRStateFilterBand(IIRState_t *s,float in);

#ifdef __cplusplus
}
#endif

#endif
