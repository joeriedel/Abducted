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

 function: linear scale -> dB, Bark and Mel scales
 last mod: $Id: PkVorbisScales.h,v 1.3 2003/11/22 10:43:19 burger Exp $

 ********************************************************************/

#ifndef __PKVORBISSCALES_H__
#define __PKVORBISSCALES_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#define todB(x) (*(x)==0?-400.f:(float)log(*(x)**(x))*4.34294480f)
#define todB_nn(x) (*(x)==0.f?-400.f:(float)log(*(x))*8.6858896f)
#define fromdB(x) ((float)exp((x)*.11512925f))  

/* The bark scale equations are approximations, since the original
   table was somewhat hand rolled.  The below are chosen to have the
   best possible fit to the rolled tables, thus their somewhat odd
   appearance (these are more accurate and over a longer range than
   the oft-quoted bark equations found in the texts I have).  The
   approximations are valid from 0 - 30kHz (nyquist) or so.

   all f in Hz, z in Bark */

#define toBARK(n)   (13.1f*(float)atan(.00074f*(n))+2.24f*(float)atan((n)*(n)*1.85e-8f)+1e-4f*(n))
#define fromBARK(z) (102.f*(z)-2.f*(float)pow(z,2.f)+.4f*(float)pow(z,3.f)+(float)pow(1.46f,z)-1.f)
#define toMEL(n)    ((float)log(1.f+(n)*.001f)*1442.695f)
#define fromMEL(m)  (1000.f*(float)exp((m)*(1.0f/1442.695f))-1000.f)

/* Frequency to octave.  We arbitrarily declare 63.5 Hz to be octave 0.0 */

#define toOC(n) ((float)log(n)*1.442695f-5.965784f)
#define fromOC(o) ((float)exp(((o)+5.965784f)*.693147f))

#endif

