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

 function: time backend 0 (dummy)
 last mod: $Id: PkVorbisTime0.c,v 1.4 2003/12/08 11:05:54 burger Exp $

 ********************************************************************/

#include "PkVorbisBackends.h"
#include "PkOgg.h"
#include "PkVorbisCodec.h"
#include "PkVorbisCodecInternal.h"

typedef struct vorbis_look_time {
	int dummy;
} vorbis_look_time;

static vorbis_info_time foo = {
	0
};

static vorbis_look_time foo2 = {
	0
};

static void time0_pack(vorbis_info_time * /* i */ ,oggpack_buffer * /* opb */)
{
}

static vorbis_info_time *time0_unpack(vorbis_info * /* vi */,oggpack_buffer * /* opb */)
{
	return &foo;
}

static vorbis_info_time *time0_copy_info (vorbis_info_time * /* vi */)
{
	return &foo;
}

static vorbis_look_time *time0_look (vorbis_dsp_state * /* vd */,vorbis_info_mode * /* mi */,vorbis_info_time * /* i */)
{
	return &foo2;
}

static void time0_free_info(vorbis_info_time * /* i */)
{
}

static void time0_free_look(vorbis_look_time * /* i */)
{
}

static int time0_forward(vorbis_block * /* vb */,vorbis_look_time * /* i */,float * /* input */,float * /* output */)
{
	return 0;
}

static int time0_inverse(vorbis_block * /* vb */,vorbis_look_time * /* i */,float * /* input */,float * /* output */)
{
	return 0;
}

	/* export hooks */
vorbis_func_time time0_exportbundle={
	&time0_pack,&time0_unpack,&time0_look,&time0_copy_info,&time0_free_info,
	&time0_free_look,&time0_forward,&time0_inverse
};
