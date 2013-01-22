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

 function: registry for time, floor, res backends and channel mappings
 last mod: $Id: PkVorbisRegistry.h,v 1.3 2003/11/22 10:43:19 burger Exp $

 ********************************************************************/

#ifndef __PKVORBISREGISTERY_H__
#define __PKVORBISREGISTERY_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#define VI_TRANSFORMB 1
#define VI_WINDOWB 1
#define VI_TIMEB 1
#define VI_FLOORB 2
#define VI_RESB 3
#define VI_MAPB 1

extern struct vorbis_func_time *_time_P[];
extern struct vorbis_func_floor *_floor_P[];
extern struct vorbis_func_residue *_residue_P[];
extern struct vorbis_func_mapping *_mapping_P[];

#ifdef __cplusplus
extern "C" {
#endif

extern void residue_free_info(struct vorbis_info_residue *r,int type);

#ifdef __cplusplus
}
#endif

#endif
