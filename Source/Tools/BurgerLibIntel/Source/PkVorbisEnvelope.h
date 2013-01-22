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

 function: PCM data envelope analysis and manipulation
 last mod: $Id: PkVorbisEnvelope.h,v 1.4 2003/11/22 10:43:19 burger Exp $

 ********************************************************************/

#ifndef __PKVORBISENVELOPE_H__
#define __PKVORBISENVELOPE_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

typedef struct envelope_lookup {
	int ch;
	int winlength;
	int searchstep;
	float minenergy;
	struct IIRState_t *iir;
	float **filtered;
	long storage;
	long current;
	long mark;
	long prevmark;
	long cursor;
} envelope_lookup;

#ifdef __cplusplus
extern "C" {
#endif

extern void BURGERCALL _ve_envelope_init(envelope_lookup *e,struct vorbis_info *vi);
extern void BURGERCALL _ve_envelope_clear(envelope_lookup *e);
extern long BURGERCALL _ve_envelope_search(struct vorbis_dsp_state *v);
extern int BURGERCALL _ve_envelope_mark(struct vorbis_dsp_state *v);
extern void BURGERCALL _ve_envelope_shift(envelope_lookup *e,long shift);

#ifdef __cplusplus
}
#endif

#endif

