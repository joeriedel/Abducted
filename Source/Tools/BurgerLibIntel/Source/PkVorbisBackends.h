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

 function: libvorbis backend and mapping structures; needed for 
           static mode headers
 last mod: $Id: PkVorbisBackends.h,v 1.5 2003/12/01 16:12:30 burger Exp $

 ********************************************************************/

/* this is exposed up here because we need it for static modes.
   Lookups for each backend aren't exposed because there's no reason
   to do so */

#ifndef __PKVORBISBACKENDS_H__
#define __PKVORBISBACKENDS_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef PK_PI
#define PK_PI (3.1415926536f)
#endif

/* this would all be simpler/shorter with templates, but.... */
/* Transform backend generic *************************************/

/* only mdct right now.  Flesh it out more if we ever transcend mdct
   in the transform domain */

/* Time backend generic ******************************************/
typedef struct vorbis_func_time {
	void (*pack) (struct vorbis_info_time *,struct oggpack_buffer *);
	struct vorbis_info_time *(*unpack)(struct vorbis_info *,struct oggpack_buffer *);
	struct vorbis_look_time *(*look)  (struct vorbis_dsp_state *,struct vorbis_info_mode *,struct vorbis_info_time *);
	struct vorbis_info_time *(*copy_info)(struct vorbis_info_time *);
	void (*free_info) (struct vorbis_info_time *);
	void (*free_look) (struct vorbis_look_time *);
	int  (*forward)   (struct vorbis_block *,struct vorbis_look_time *,float *,float *);
	int  (*inverse)   (struct vorbis_block *,struct vorbis_look_time *,float *,float *);
} vorbis_func_time;

typedef struct vorbis_info_time {
	int dummy;
} vorbis_info_time;

/* Floor backend generic *****************************************/
typedef struct vorbis_func_floor {
	void (*pack) (struct vorbis_info_floor *,struct oggpack_buffer *);
	struct vorbis_info_floor *(*unpack)(struct vorbis_info *,struct oggpack_buffer *);
	struct vorbis_look_floor *(*look)  (struct vorbis_dsp_state *,struct vorbis_info_mode *,struct vorbis_info_floor *);
	struct vorbis_info_floor *(*copy_info)(struct vorbis_info_floor *);
	void (*free_info) (struct vorbis_info_floor *);
	void (*free_look) (struct vorbis_look_floor *);
	int  (*forward)   (struct vorbis_block *,struct vorbis_look_floor *,float *, const float *, /* in */
		const float *, const float *, /* in */ float *);                     /* out */
	void *(*inverse1)  (struct vorbis_block *,struct vorbis_look_floor *);
	int   (*inverse2)  (struct vorbis_block *,struct vorbis_look_floor *,void *buffer,float *);
} vorbis_func_floor;

typedef struct vorbis_info_floor {
	int order;
	long rate;
	long barkmap;

	int ampbits;
	int ampdB;

	int numbooks; /* <= 16 */
	int books[16];

	float lessthan;     /* encode-only config setting hacks for libvorbis */
	float greaterthan;  /* encode-only config setting hacks for libvorbis */
} vorbis_info_floor;

#define VIF_POSIT 63
#define VIF_CLASS 16
#define VIF_PARTS 31

typedef struct vorbis_info_floor1 {
	int   partitions;                /* 0 to 31 */
	int   partitionclass[VIF_PARTS]; /* 0 to 15 */

	int   class_dim[VIF_CLASS];        /* 1 to 8 */
	int   class_subs[VIF_CLASS];       /* 0,1,2,3 (bits: 1<<n poss) */
	int   class_book[VIF_CLASS];       /* subs ^ dim entries */
	int   class_subbook[VIF_CLASS][8]; /* [VIF_CLASS][subs] */


	int   mult;                      /* 1 2 3 or 4 */ 
	int   postlist[VIF_POSIT+2];    /* first two implicit */ 

	/* encode side analysis parameters */
	float maxover;     
	float maxunder;  
	float maxerr;    

	int   twofitminsize;
	int   twofitminused;
	int   twofitweight;  
	float twofitatten;
	int   unusedminsize;
	int   unusedmin_n;
	int   n;
} vorbis_info_floor1;

/* Residue backend generic *****************************************/
typedef struct vorbis_func_residue {
	void (*pack)(struct vorbis_info_residue *,struct oggpack_buffer *);
	struct vorbis_info_residue *(*unpack)(struct vorbis_info *,struct oggpack_buffer *);
	struct vorbis_look_residue *(*look)  (struct vorbis_dsp_state *,struct vorbis_info_mode *,struct vorbis_info_residue *);
	struct vorbis_info_residue *(*copy_info)(struct vorbis_info_residue *);
	void (*free_info)(struct vorbis_info_residue *);
	void (*free_look)(struct vorbis_look_residue *);
	long **(*vorbclass)(struct vorbis_block *,struct vorbis_look_residue *,float **,int *,int);
	int (*forward)(struct vorbis_block *,struct vorbis_look_residue *,float **,float **,int *,int,int,long **,Word32*);
	int (*inverse)(struct vorbis_block *,struct vorbis_look_residue *,float **,int *,int);
} vorbis_func_residue;

typedef struct vorbis_info_residue {
	/* block-partitioned VQ coded straight residue */
	long begin;
	long end;

	/* first stage (lossless partitioning) */
	int grouping;         /* group n vectors per partition */
	int partitions;       /* possible codebooks for a partition */
	int groupbook;        /* huffbook for partitioning */
	int secondstages[64]; /* expanded out to pointers in lookup */
	int booklist[256];    /* list of second stage books */

	/* encode-only heuristic settings */
	float entmax[64];       /* book entropy threshholds*/
	float ampmax[64];       /* book amp threshholds*/
	int subgrp[64];       /* book heuristic subgroup size */
	int blimit[64];       /* subgroup position limits */
} vorbis_info_residue;

/* Mapping backend generic *****************************************/
typedef struct vorbis_func_mapping {
	void (*pack)  (struct vorbis_info *,struct vorbis_info_mapping *,struct oggpack_buffer *);
	struct vorbis_info_mapping *(*unpack)(struct vorbis_info *,struct oggpack_buffer *);
	struct vorbis_look_mapping *(*look)  (struct vorbis_dsp_state *,struct vorbis_info_mode *,struct vorbis_info_mapping *);
	struct vorbis_info_mapping *(*copy_info)(struct vorbis_info_mapping *);
	void (*free_info)(struct vorbis_info_mapping *);
	void (*free_look)(struct vorbis_look_mapping *);
	int (*forward)(struct vorbis_block *vb,struct vorbis_look_mapping *);
	int (*inverse)(struct vorbis_block *vb,struct vorbis_look_mapping *);
} vorbis_func_mapping;

typedef struct vorbis_info_mapping {
	int submaps;  /* <= 16 */
	int chmuxlist[256];   /* up to 256 channels in a Vorbis stream */

	int timesubmap[16];    /* [mux] */
	int floorsubmap[16];   /* [mux] submap to floors */
	int residuesubmap[16]; /* [mux] submap to residue */

	int psy[2]; /* by blocktype; impulse/padding for short,
		transition/normal for long */

	int coupling_steps;
	int coupling_mag[256];
	int coupling_ang[256];
} vorbis_info_mapping;

/* In PkVorbisRes0.c */

extern vorbis_func_residue residue0_exportbundle;
extern vorbis_func_residue residue1_exportbundle;
extern vorbis_func_residue residue2_exportbundle;

/* In PkVorbisFloor0.c */

extern vorbis_func_floor floor0_exportbundle;

/* In PkVorbisFloor1.c */

extern vorbis_func_floor floor1_exportbundle;

/* In PkVorbisTime0.c */

extern vorbis_func_time time0_exportbundle;

/* In PkVorbisMapping0.c */

extern vorbis_func_mapping mapping0_exportbundle;

#endif





