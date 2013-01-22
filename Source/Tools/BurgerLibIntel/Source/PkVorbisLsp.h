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

  function: LSP (also called LSF) conversion routines
  last mod: $Id: PkVorbisLsp.h,v 1.5 2003/11/22 10:43:19 burger Exp $

 ********************************************************************/

#ifndef __PKVORBISLSP_H__
#define __PKVORBISLSP_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef PK_PI
#define PK_PI (3.1415926536f)
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void BURGERCALL vorbis_lsp_to_curve(float *curve,int *map,int n,int ln,float *lsp,int m,float amp,float ampoffset);
extern int BURGERCALL vorbis_lpc_to_lsp(float *lpc,float *lsp,int m);

#ifdef __cplusplus
}
#endif

#endif
