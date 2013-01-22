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

	Infinite Impulse Response Filter
	function: Direct Form II IIR filters, plus some specializations
	last mod: $Id: PkVorbisIIR.c,v 1.5 2003/12/01 16:12:31 burger Exp $

 ********************************************************************/

/* LPC is actually a degenerate case of form I/II filters, but we need
   both */

#include "PkVorbisiir.h"
#include "ClStdlib.h"
#include "MmMemory.h"

void BURGERCALL IIRStateInit(IIRState_t *s,int stages,float gain,const float *A,const float *B)
{
	s->ring = 0;
	s->stages=stages;
	s->gain=1.f/gain;
	s->coeff_A=static_cast<float *>(MemoryNewPointerCopy(A,stages*sizeof(*s->coeff_A)));
	s->coeff_B=static_cast<float *>(MemoryNewPointerCopy(B,(stages+1)*sizeof(*s->coeff_B)));
	s->z_A=static_cast<float *>(AllocAPointerClear(stages*2*sizeof(*s->z_A)));
}

void BURGERCALL IIRStateDestroy(IIRState_t *s)
{
	if (s) {
		DeallocAPointer(s->coeff_A);
		DeallocAPointer(s->coeff_B);
		DeallocAPointer(s->z_A);
		FastMemSet(s,0,sizeof(*s));
	}
}

void BURGERCALL IIRStateReset(IIRState_t *s)
{
	FastMemSet(s->z_A,0,sizeof(*s->z_A)*s->stages*2);
}

float BURGERCALL IIRStateFilter(IIRState_t *s,float input)
{
	int i;
	int stages;
	float newA;
	float newB;
	float *zA;
	const float *ZInput;
	float *AInput;
	const float *BInput;
	
	zA=s->z_A+s->ring;

	newA = input*s->gain;
	newB = 0;
	stages = s->stages;
	ZInput = zA;
	AInput = s->coeff_A;
	BInput = s->coeff_B;

	if (stages) {
		do {
			float ztemp;
			ztemp = ZInput[0];
			++ZInput;
			newA += AInput[0] * ztemp;
			++AInput;
			newB += BInput[0] * ztemp;
			++BInput;
		} while (--stages);
	}

	newB += newA*BInput[0];

	zA[0]=AInput[0]=newA;
	i = s->ring+1;
	if(i>=s->stages) {
		i=0;
	}
	s->ring = i;
	return newB;
}

/* this assumes the symmetrical structure of the feed-forward stage of
   a typical bandpass to save multiplies */
   
float BURGERCALL IIRStateFilterBand(IIRState_t *s,float input)
{
	int i;
	int stages=s->stages;
	int stages2=stages>>1;
	float newA;
	float newB;
	float *zA;
	
	zA=s->z_A+s->ring;
	newA = input*s->gain;
	newA += s->coeff_A[0] * zA[0];
	newB = 0;
	for (i=1;i<stages2;i++){
		newA += s->coeff_A[i] * zA[i];
		newB += s->coeff_B[i] * (zA[i]-zA[stages-i]);
	}
	newB+= s->coeff_B[i] * zA[i];
	for (;i<stages;i++) {
		newA+= s->coeff_A[i] * zA[i];
	}
	newB+=newA-zA[0];

	zA[0]=zA[stages]=newA;
	if (++s->ring>=stages) {
		s->ring=0;
	}
	return(newB);
}

