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

 function: bitrate tracking and management
 last mod: $Id: PkVorbisBitrate.h,v 1.5 2003/12/01 16:12:31 burger Exp $

 ********************************************************************/

#ifndef __PKVORBISBITRATE_H__
#define __PKVORBISBITRATE_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

/* encode side bitrate tracking */
#define BITTRACK_DIVISOR 16
#define BITTRACK_BPT     6
typedef struct bitrate_manager_state {
	Word32  *queue_binned;
	Word32  *queue_actual;
	int queue_size;

	int queue_head;
	int queue_bins;

	long *avg_binacc;
	int avg_center;
	int avg_tail;
	Word32 avg_centeracc;
	Word32 avg_sampleacc;
	Word32 avg_sampledesired;
	Word32 avg_centerdesired;

	long          *minmax_binstack;
	long          *minmax_posstack;
	long          *minmax_limitstack;
	long           minmax_stackptr;

	long           minmax_acctotal;
	int            minmax_tail;
	Word32   minmax_sampleacc;
	Word32   minmax_sampledesired;

	int            next_to_flush;
	int            last_to_flush;

	double         avgfloat;
	double         avgnoise;
	double         noisetrigger_request;
	long           noisetrigger_postpone;

	/* unfortunately, we need to hold queued packet data somewhere */
	struct oggpack_buffer *queue_packet_buffers;
	struct ogg_packet *queue_packets;

} bitrate_manager_state;

typedef struct bitrate_manager_info {
	/* detailed bitrate management setup */
	double queue_avg_time;
	double queue_avg_center;
	double queue_minmax_time;
	double queue_hardmin;
	double queue_hardmax;
	double queue_avgmin;
	double queue_avgmax;

	double avgfloat_initial; /* set by mode */
	double avgfloat_minimum; /* set by mode */
	double avgfloat_downslew_max;
	double avgfloat_upslew_max;
	double avgfloat_noise_lowtrigger;
	double avgfloat_noise_hightrigger;
	double avgfloat_noise_minval;
	double avgfloat_noise_maxval;
} bitrate_manager_info;

#ifdef __cplusplus
extern "C" {
#endif

extern void BURGERCALL vorbis_bitrate_init(struct vorbis_info *vi,bitrate_manager_state *bs);
extern void BURGERCALL vorbis_bitrate_clear(bitrate_manager_state *bs);
extern int BURGERCALL vorbis_bitrate_managed(struct vorbis_block *vb);
extern int BURGERCALL vorbis_bitrate_maxmarkers(void);
extern int BURGERCALL vorbis_bitrate_addblock(struct vorbis_block *vb);
extern int BURGERCALL vorbis_bitrate_flushpacket(struct vorbis_dsp_state *vd,struct ogg_packet *op);

#ifdef __cplusplus
}
#endif

#endif
