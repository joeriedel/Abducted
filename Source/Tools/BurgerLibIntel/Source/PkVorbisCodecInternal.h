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

 function: libvorbis codec headers
 last mod: $Id: PkVorbisCodecInternal.h,v 1.4 2003/12/01 16:12:31 burger Exp $

 ********************************************************************/

#ifndef __PKVORBISCODECINTERNAL_H__
#define __PKVORBISCODECINTERNAL_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __PKVORBISBITRATE_H__
#include "PkVorbisBitrate.h"
#endif

#ifndef __PKVORBISPSY_H__
#include "PkVorbisPsy.h"
#endif

#define BLOCKTYPE_IMPULSE    0
#define BLOCKTYPE_PADDING    1
#define BLOCKTYPE_TRANSITION 0 
#define BLOCKTYPE_LONG       1

typedef struct vorbis_block_internal{
	float  **pcmdelay;  /* this is a pointer into local storage */ 
	float  ampmax;
	int    blocktype;
	Word32 *packet_markers;
} vorbis_block_internal;

/* mode ************************************************************/
typedef struct vorbis_info_mode {
	int blockflag;
	int windowtype;
	int transformtype;
	int mapping;
} vorbis_info_mode;

typedef struct backend_lookup_state {
	/* local lookup storage */
	struct envelope_lookup *ve; /* envelope lookup */    
	float **window[2][2][2]; /* block, leadin, leadout, type */
	struct mdct_lookup **transform[2];    /* block, type */
	struct codebook *fullbooks;
	struct vorbis_look_psy_global *psy_g_look;

	/* backend lookups are tied to the mode, not the backend or naked mapping */
	int modebits;
	struct vorbis_look_mapping   **mode;

	/* local storage, only used on the encoding side.  This way the
	application does not need to worry about freeing some packets'
	memory and not others'; packet storage is always tracked.
	Cleared next call to a _dsp_ function */
	unsigned char *header;
	unsigned char *header1;
	unsigned char *header2;

	bitrate_manager_state bms;

} backend_lookup_state;

/* high level configuration information for setting things up
   step-by-step with the detaile vorbis_encode_ctl interface */

typedef struct highlevel_block {
	double tone_mask_quality;
	double tone_peaklimit_quality;

	double noise_bias_quality;
	double noise_compand_quality;

	double ath_quality;

} highlevel_block;

typedef struct highlevel_encode_setup {
	double base_quality;       /* these have to be tracked by the ctl */
	double base_quality_short; /* interface so that the right books get */
	double base_quality_long;  /* chosen... */

	int short_block_p;
	int long_block_p;
	int impulse_block_p;

	int stereo_couple_p;
	int stereo_backfill_p;
	int residue_backfill_p;

	double stereo_point_kHz[2];
	double lowpass_kHz[2];

	double ath_floating_dB;
	double ath_absolute_dB;

	double amplitude_track_dBpersec;
	double trigger_quality;
	int    stereo_point_dB;

	highlevel_block blocktype[4]; /* impulse, padding, trans, long */

} highlevel_encode_setup;

/* codec_setup_info contains all the setup information specific to the
   specific compression/decompression mode in progress (eg,
   psychoacoustic settings, channel setup, options, codebook
   etc).  
*********************************************************************/

typedef struct codec_setup_info {
	/* Vorbis supports only short and long blocks, but allows the
		encoder to choose the sizes */

	long blocksizes[2];

	/* modes are the primary means of supporting on-the-fly different
	blocksizes, different channel mappings (LR or M/A),
	different residue backends, etc.  Each mode consists of a
	blocksize flag and a mapping (along with the mapping setup */

	int        modes;
	int        maps;
	int        times;
	int        floors;
	int        residues;
	int        books;
	int        psys;     /* encode only */

	vorbis_info_mode       *mode_param[64];
	int                     map_type[64];
	struct vorbis_info_mapping    *map_param[64];
	int                     time_type[64];
	struct vorbis_info_time       *time_param[64];
	int                     floor_type[64];
	struct vorbis_info_floor      *floor_param[64];
	int                     residue_type[64];
	struct vorbis_info_residue    *residue_param[64];
	struct static_codebook        *book_param[256];

	struct vorbis_info_psy        *psy_param[64]; /* encode only */
	vorbis_info_psy_global psy_g_param;

	bitrate_manager_info   bi;
	highlevel_encode_setup hi;

	int    passlimit[32];     /* iteration limit per couple/quant pass */
	int    coupling_passes;
} codec_setup_info;

#endif
