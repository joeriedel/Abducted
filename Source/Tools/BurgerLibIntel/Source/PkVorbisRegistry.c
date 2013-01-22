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
 last mod: $Id: PkVorbisRegistry.c,v 1.1 2002/06/17 19:31:45 burger Exp $

 ********************************************************************/

#include "PkVorbisRegistry.h"
#include "PkVorbisBackends.h"

/* seems like major overkill now; the backend numbers will grow into
	the infrastructure soon enough */

vorbis_func_time *_time_P[]={
	&time0_exportbundle,
};

vorbis_func_floor     *_floor_P[]={
	&floor0_exportbundle,
	&floor1_exportbundle,
};

vorbis_func_residue   *_residue_P[]={
	&residue0_exportbundle,
	&residue1_exportbundle,
	&residue2_exportbundle,
};

vorbis_func_mapping   *_mapping_P[]={
	&mapping0_exportbundle,
};

/* make Windows happy; can't access the registry directly outside of
   libvorbis, and vorbisenc needs a few functions */
void residue_free_info(vorbis_info_residue *r,int type)
{
	_residue_P[type]->free_info(r);
}

