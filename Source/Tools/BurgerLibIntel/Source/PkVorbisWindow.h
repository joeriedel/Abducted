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

 function: window functions
 last mod: $Id: PkVorbisWindow.h,v 1.5 2003/11/22 10:43:19 burger Exp $

 ********************************************************************/

#ifndef __PKVORBISWINDOW_H__
#define __PKVORBISWINDOW_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef PK_PI
#define PK_PI (3.1415926536f)
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern float *BURGERCALL _vorbis_window(int type,int window,int left,int right);

#ifdef __cplusplus
}
#endif

#endif
