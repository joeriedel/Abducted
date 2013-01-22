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

 function: channel mapping 0 implementation
 last mod: $Id: PkVorbisMapping0.c,v 1.4 2003/12/01 16:12:31 burger Exp $

 ********************************************************************/

#include "PkVorbisBackends.h"
#include "PkVorbisSmallft.h"
#include "PkVorbisCodecInternal.h"
#include "PkVorbisCodec.h"
#include "PkVorbisRegistry.h"
#include "PkVorbisScales.h"
#include "PkVorbisMdct.h"
#include "MmMemory.h"
#include "ClStdlib.h"
#include <math.h>
#if defined(__MAC__) || defined(__MACOSX__)
#include <alloca.h>
#else
#include <malloc.h>
#endif

/* simplistic, wasteful way of doing this (unique lookup for each
   mode/submapping); there should be a central repository for
   identical lookups.  That will require minor work, so I'm putting it
   off as low priority.

   Why a lookup for each backend in a given mode?  Because the
   blocksize is set by the mode, and low backend lookups may require
   parameters from other areas of the mode/mapping */

typedef struct vorbis_look_mapping {
	drft_lookup fft_look;
	vorbis_info_mode *mode;
	vorbis_info_mapping *map;

	struct vorbis_look_time **time_look;
	struct vorbis_look_floor **floor_look;

	struct vorbis_look_residue **residue_look;
	vorbis_look_psy *psy_look[2];

	vorbis_func_time **time_func;
	vorbis_func_floor **floor_func;
	vorbis_func_residue **residue_func;

	int ch;
	long lastframe; /* if a different mode is called, we need to invalidate decay */
} vorbis_look_mapping;

static vorbis_info_mapping *mapping0_copy_info(vorbis_info_mapping *info){
  vorbis_info_mapping *ret=static_cast<vorbis_info_mapping *>(AllocAPointer(sizeof(*ret)));
  FastMemCpy(ret,info,sizeof(*ret));
  return(ret);
}

static void mapping0_free_info(vorbis_info_mapping *info){
  if(info){
    FastMemSet(info,0,sizeof(*info));
    DeallocAPointer(info);
  }
}

static void mapping0_free_look(vorbis_look_mapping *l){
  int i;
 
  if(l){
    drft_clear(&l->fft_look);

    for(i=0;i<l->map->submaps;i++){
      l->time_func[i]->free_look(l->time_look[i]);
      l->floor_func[i]->free_look(l->floor_look[i]);
      l->residue_func[i]->free_look(l->residue_look[i]);
    }
    if(l->psy_look[1] && l->psy_look[1]!=l->psy_look[0]){
      _vp_psy_clear(l->psy_look[1]);
      DeallocAPointer(l->psy_look[1]);
    }
    if(l->psy_look[0]){
      _vp_psy_clear(l->psy_look[0]);
      DeallocAPointer(l->psy_look[0]);
    }
    DeallocAPointer(l->time_func);
    DeallocAPointer(l->floor_func);
    DeallocAPointer(l->residue_func);
    DeallocAPointer(l->time_look);
    DeallocAPointer(l->floor_look);
    DeallocAPointer(l->residue_look);
    FastMemSet(l,0,sizeof(*l));
    DeallocAPointer(l);
  }
}

static vorbis_look_mapping *mapping0_look(vorbis_dsp_state *vd,vorbis_info_mode *vm,
			  vorbis_info_mapping *m){
  int i;
  vorbis_info          *vi=vd->vi;
  codec_setup_info     *ci=static_cast<codec_setup_info *>(vi->codec_setup);
  vorbis_look_mapping *look=static_cast<vorbis_look_mapping *>(AllocAPointerClear(sizeof(*look)));
  vorbis_info_mapping *info=look->map=(vorbis_info_mapping *)m;
  look->mode=vm;
  
  look->time_look=static_cast<vorbis_look_time **>(AllocAPointerClear(info->submaps*sizeof(*look->time_look)));
  look->floor_look=static_cast<vorbis_look_floor **>(AllocAPointerClear(info->submaps*sizeof(*look->floor_look)));

  look->residue_look=static_cast<vorbis_look_residue **>(AllocAPointerClear(info->submaps*sizeof(*look->residue_look)));

  look->time_func=static_cast<vorbis_func_time **>(AllocAPointerClear(info->submaps*sizeof(*look->time_func)));
  look->floor_func=static_cast<vorbis_func_floor **>(AllocAPointerClear(info->submaps*sizeof(*look->floor_func)));
  look->residue_func=static_cast<vorbis_func_residue **>(AllocAPointerClear(info->submaps*sizeof(*look->residue_func)));
  
  for(i=0;i<info->submaps;i++){
    int timenum=info->timesubmap[i];
    int floornum=info->floorsubmap[i];
    int resnum=info->residuesubmap[i];

    look->time_func[i]=_time_P[ci->time_type[timenum]];
    look->time_look[i]=look->time_func[i]->
      look(vd,vm,ci->time_param[timenum]);
    look->floor_func[i]=_floor_P[ci->floor_type[floornum]];
    look->floor_look[i]=look->floor_func[i]->
      look(vd,vm,ci->floor_param[floornum]);
    look->residue_func[i]=_residue_P[ci->residue_type[resnum]];
    look->residue_look[i]=look->residue_func[i]->
      look(vd,vm,ci->residue_param[resnum]);
    
  }
  if(ci->psys && vd->analysisp){
    if(info->psy[0] != info->psy[1]){

      int psynum=info->psy[0];
      look->psy_look[0]=static_cast<vorbis_look_psy *>(AllocAPointerClear(sizeof(*look->psy_look[0])));      
      _vp_psy_init(look->psy_look[0],ci->psy_param[psynum],
		   &ci->psy_g_param,
		   ci->blocksizes[vm->blockflag]/2,vi->rate);

      psynum=info->psy[1];
      look->psy_look[1]=static_cast<vorbis_look_psy *>(AllocAPointerClear(sizeof(*look->psy_look[1])));      
      _vp_psy_init(look->psy_look[1],ci->psy_param[psynum],
		   &ci->psy_g_param,
		   ci->blocksizes[vm->blockflag]/2,vi->rate);
    }else{

      int psynum=info->psy[0];
      look->psy_look[0]=static_cast<vorbis_look_psy *>(AllocAPointerClear(sizeof(*look->psy_look[0])));      
      look->psy_look[1]=look->psy_look[0];
      _vp_psy_init(look->psy_look[0],ci->psy_param[psynum],&ci->psy_g_param,
		   ci->blocksizes[vm->blockflag]/2,vi->rate);

    }
  }

  look->ch=vi->channels;

  if(vd->analysisp)drft_init(&look->fft_look,ci->blocksizes[vm->blockflag]);
  return(look);
}

static int ilog2(unsigned int v)
{
	int ret=0;
	while(v>1){
		ret++;
		v>>=1;
	}
	return(ret);
}

static void mapping0_pack(vorbis_info *vi,vorbis_info_mapping *vm,
			  oggpack_buffer *opb){
  int i;
  vorbis_info_mapping *info=(vorbis_info_mapping *)vm;

  /* another 'we meant to do it this way' hack...  up to beta 4, we
     packed 4 binary zeros here to signify one submapping in use.  We
     now redefine that to mean four bitflags that indicate use of
     deeper features; bit0:submappings, bit1:coupling,
     bit2,3:reserved. This is backward compatable with all actual uses
     of the beta code. */

  if(info->submaps>1){
    oggpack_write(opb,1,1);
    oggpack_write(opb,info->submaps-1,4);
  }else
    oggpack_write(opb,0,1);

  if(info->coupling_steps>0){
    oggpack_write(opb,1,1);
    oggpack_write(opb,info->coupling_steps-1,8);
    
    for(i=0;i<info->coupling_steps;i++){
      oggpack_write(opb,info->coupling_mag[i],ilog2(vi->channels));
      oggpack_write(opb,info->coupling_ang[i],ilog2(vi->channels));
    }
  }else
    oggpack_write(opb,0,1);
  
  oggpack_write(opb,0,2); /* 2,3:reserved */

  /* we don't write the channel submappings if we only have one... */
  if(info->submaps>1){
    for(i=0;i<vi->channels;i++)
      oggpack_write(opb,info->chmuxlist[i],4);
  }
  for(i=0;i<info->submaps;i++){
    oggpack_write(opb,info->timesubmap[i],8);
    oggpack_write(opb,info->floorsubmap[i],8);
    oggpack_write(opb,info->residuesubmap[i],8);
  }
}

/* also responsible for range checking */
static vorbis_info_mapping *mapping0_unpack(vorbis_info *vi,oggpack_buffer *opb){
  int i;
  vorbis_info_mapping *info=static_cast<vorbis_info_mapping *>(AllocAPointerClear(sizeof(*info)));
  codec_setup_info     *ci=static_cast<codec_setup_info *>(vi->codec_setup);
  FastMemSet(info,0,sizeof(*info));

  if(oggpack_read1(opb))
    info->submaps=oggpack_read(opb,4)+1;
  else
    info->submaps=1;

  if(oggpack_read1(opb)){
    info->coupling_steps=oggpack_read(opb,8)+1;

    for(i=0;i<info->coupling_steps;i++){
      int testM=info->coupling_mag[i]=oggpack_read(opb,ilog2(vi->channels));
      int testA=info->coupling_ang[i]=oggpack_read(opb,ilog2(vi->channels));

      if(testM<0 || 
	 testA<0 || 
	 testM==testA || 
	 testM>=vi->channels ||
	 testA>=vi->channels) goto err_out;
    }

  }

  if(oggpack_read(opb,2)>0)goto err_out; /* 2,3:reserved */
    
  if(info->submaps>1){
    for(i=0;i<vi->channels;i++){
      info->chmuxlist[i]=oggpack_read(opb,4);
      if(info->chmuxlist[i]>=info->submaps)goto err_out;
    }
  }
  for(i=0;i<info->submaps;i++){
    info->timesubmap[i]=oggpack_read(opb,8);
    if(info->timesubmap[i]>=ci->times)goto err_out;
    info->floorsubmap[i]=oggpack_read(opb,8);
    if(info->floorsubmap[i]>=ci->floors)goto err_out;
    info->residuesubmap[i]=oggpack_read(opb,8);
    if(info->residuesubmap[i]>=ci->residues)goto err_out;
  }

  return info;

 err_out:
  mapping0_free_info(info);
  return(NULL);
}

/* no time mapping implementation for now */
static long seq=0;
static int mapping0_forward(vorbis_block *vb,vorbis_look_mapping *l){
  vorbis_dsp_state      *vd=vb->vd;
  vorbis_info           *vi=vd->vi;
  codec_setup_info      *ci=static_cast<codec_setup_info *>(vi->codec_setup);
  backend_lookup_state  *b=static_cast<backend_lookup_state *>(vb->vd->backend_state);
  bitrate_manager_state *bm=&b->bms;
  vorbis_look_mapping  *look=(vorbis_look_mapping *)l;
  vorbis_info_mapping  *info=look->map;
  vorbis_info_mode      *mode=look->mode;
  vorbis_block_internal *vbi=(vorbis_block_internal *)vb->internal;
  int                    n=vb->pcmend;
  int i,j;
  float *window=b->window[vb->W][vb->lW][vb->nW][mode->windowtype];
  int   *nonzero=(int *)alloca(sizeof(*nonzero)*vi->channels);

  float *work=static_cast<float *>(_vorbis_block_alloc(vb,n*sizeof(*work)));

  float global_ampmax=vbi->ampmax;
  float *local_ampmax=(float *)alloca(sizeof(*local_ampmax)*vi->channels);
  int blocktype=vbi->blocktype;

  /* we differentiate between short and long block types to help the
     masking engine; the window shapes also matter.
     impulse block (a short block in which an impulse occurs)
     padding block (a short block that pads between a transitional 
          long block and an impulse block, or vice versa)
     transition block (the wqeird one; a long block with the transition 
          window; affects bass/midrange response and that must be 
	  accounted for in masking) 
     long block (run of the mill long block)
  */

  for(i=0;i<vi->channels;i++){
    float scale=4.f/n;
    float scale_dB;

    /* the following makes things clearer to *me* anyway */
    float *pcm     =vb->pcm[i]; 
    float *fft     =work;
    float *logfft  =pcm+n/2;

    /*float *res     =pcm;
    float *mdct    =pcm;
    float *codedflr=pcm+n/2;
    float *logmax  =work;
    float *logmask =work+n/2;*/

    scale_dB=todB(&scale);
    _analysis_output("pcm",seq+i,pcm,n,0,0);

    /* window the PCM data */
    for(j=0;j<n;j++)
      fft[j]=pcm[j]*=window[j];
    
    /*_analysis_output("windowed",seq+i,pcm,n,0,0);*/

    /* transform the PCM data */
    /* only MDCT right now.... */
    mdct_forward(b->transform[vb->W][0],pcm,pcm);
    
    /* FFT yields more accurate tonal estimation (not phase sensitive) */
    drft_forward(&look->fft_look,fft);
    fft[0]*=scale;
    logfft[0]=todB(fft);
    local_ampmax[i]=logfft[0];
    for(j=1;j<n-1;j+=2){
      float temp=fft[j]*fft[j]+fft[j+1]*fft[j+1];
      temp=logfft[(j+1)>>1]=scale_dB+.5f*todB(&temp);
      if(temp>local_ampmax[i])local_ampmax[i]=temp;
    }

    if(local_ampmax[i]>0.f)local_ampmax[i]=0.f;
    if(local_ampmax[i]>global_ampmax)global_ampmax=local_ampmax[i];

    _analysis_output("fft",seq+i,logfft,n/2,1,0);
  }

  for(i=0;i<vi->channels;i++){
    int submap=info->chmuxlist[i];

    /* the following makes things clearer to *me* anyway */
    float *mdct    =vb->pcm[i]; 
    float *res     =mdct;
    float *codedflr=mdct+n/2;
    float *logfft  =mdct+n/2;

    float *logmdct =work;
    float *logmax  =mdct+n/2;
    float *logmask =work+n/2;

    for(j=0;j<n/2;j++)
      logmdct[j]=todB(mdct+j);
    _analysis_output("mdct",seq+i,logmdct,n/2,1,0);


    /* perform psychoacoustics; do masking */
    _vp_compute_mask(look->psy_look[blocktype],
		     logfft, /* -> logmax */
		     logmdct,
		     logmask,
		     global_ampmax,
		     local_ampmax[i],
		     (float)bm->avgnoise);

    _analysis_output("mask",seq+i,logmask,n/2,1,0);
    /* perform floor encoding */
    nonzero[i]=look->floor_func[submap]->
      forward(vb,look->floor_look[submap],
	      mdct,
	      logmdct,
	      logmask,
	      logmax,

	      codedflr);


    _vp_remove_floor(look->psy_look[blocktype],
		     mdct,
		     codedflr,
		     res);

    /*for(j=0;j<n/2;j++)
      if(fabs(res[j])>1200){
	analysis_noisy=TRUE;
	fprintf(stderr,"%ld ",seq+i);
	}*/

    _analysis_output("codedflr",seq+i,codedflr,n/2,1,1);
      
  }

  vbi->ampmax=global_ampmax;

  /* partition based prequantization and channel coupling */
  /* Steps in prequant and coupling:

     classify by |mag| across all pcm vectors 

     down-couple/down-quantize from perfect residue ->  quantized vector 
     
     do{ 
        encode quantized vector; add encoded values to 'so-far' vector
        more? [not yet at bitrate/not yet at target]
          yes{
              down-couple/down-quantize from perfect-'so-far' -> 
	        quantized vector; when subtracting coupling, 
		account for +/- out-of-phase component
          }no{  
              break
          }
     }
     done.

     quantization in each iteration is done (after circular normalization 
     in coupling) using a by-iteration quantization granule value.
  */
   
  {
    float  **pcm=vb->pcm;
    float  **quantized=(float **)alloca(sizeof(*quantized)*vi->channels);
    float  **sofar=(float **)alloca(sizeof(*sofar)*vi->channels);

    long  ***classifications=(long ***)alloca(sizeof(*classifications)*info->submaps);
    float ***qbundle=(float ***)alloca(sizeof(*qbundle)*info->submaps);
    float ***pcmbundle=(float ***)alloca(sizeof(*pcmbundle)*info->submaps);
    float ***sobundle=(float ***)alloca(sizeof(*sobundle)*info->submaps);
    int    **zerobundle=(int **)alloca(sizeof(*zerobundle)*info->submaps);
    int     *chbundle=(int *)alloca(sizeof(*chbundle)*info->submaps);
    int      chcounter=0;

    /* play a little loose with this abstraction */
    int   quant_passes=ci->coupling_passes;

    for(i=0;i<vi->channels;i++){
      quantized[i]=static_cast<float *>(_vorbis_block_alloc(vb,n*sizeof(*sofar[i])));
      sofar[i]=quantized[i]+n/2;
      FastMemSet(sofar[i],0,sizeof(*sofar[i])*n/2);
    }

    qbundle[0]=(float **)alloca(sizeof(*qbundle[0])*vi->channels);
    pcmbundle[0]=(float **)alloca(sizeof(*pcmbundle[0])*vi->channels);
    sobundle[0]=(float **)alloca(sizeof(*sobundle[0])*vi->channels);
    zerobundle[0]=(int *)alloca(sizeof(*zerobundle[0])*vi->channels);

    /* initial down-quantized coupling */
    
    if(info->coupling_steps==0){
      /* this assumes all or nothing coupling right now.  it should pass
	 through any channels left uncoupled, but it doesn't do that now */
      for(i=0;i<vi->channels;i++){
	float *lpcm=pcm[i];
	float *lqua=quantized[i];
	for(j=0;j<n/2;j++)
	  lqua[j]=lpcm[j];
      }
    }else{
      _vp_quantize_couple(look->psy_look[blocktype],
			  info,
			  pcm,
			  sofar,
			  quantized,
			  nonzero,
			  0);
    }

    for(i=0;i<vi->channels;i++)
      _analysis_output("quant",seq+i,quantized[i],n/2,1,0);

  
    /* classify, by submap */

    for(i=0;i<info->submaps;i++){
      int ch_in_bundle=0;
      qbundle[i]=qbundle[0]+chcounter;
      sobundle[i]=sobundle[0]+chcounter;
      zerobundle[i]=zerobundle[0]+chcounter;

      for(j=0;j<vi->channels;j++){
	if(info->chmuxlist[j]==i){
	  if(nonzero[j])
	    zerobundle[i][ch_in_bundle]=1;
	  else
	    zerobundle[i][ch_in_bundle]=0;
	  qbundle[i][ch_in_bundle]=quantized[j];
	  pcmbundle[i][ch_in_bundle]=pcm[j];
	  sobundle[i][ch_in_bundle++]=sofar[j];
	}
      }
      chbundle[i]=ch_in_bundle;
      chcounter+=ch_in_bundle;

      classifications[i]=look->residue_func[i]->
	vorbclass(vb,look->residue_look[i],pcmbundle[i],zerobundle[i],chbundle[i]);
    }

    /* actual encoding loop; we pack all the iterations to collect
       management data */

    for(i=0;i<quant_passes;){

      /* perform residue encoding of this pass's quantized residue
         vector, according residue mapping */
    
      for(j=0;j<info->submaps;j++){
	look->residue_func[j]->
	  forward(vb,look->residue_look[j],
		  qbundle[j],sobundle[j],zerobundle[j],chbundle[j],
		  i,classifications[j],vbi->packet_markers);
	
      }
      i++;
	
      if(i<quant_passes){
	/* down-couple/down-quantize from perfect-'so-far' -> 
	 new quantized vector */
	if(info->coupling_steps==0){
	  /* this assumes all or nothing coupling right now.  it should pass
	     through any channels left uncoupled, but it doesn't do that now */
	  int k;
	  for(k=0;k<vi->channels;k++){
	    float *lpcm=pcm[k];
	    float *lsof=sofar[k];
	    float *lqua=quantized[k];
	    for(j=0;j<n/2;j++)
	      lqua[j]=lpcm[j]-lsof[j];
	  }
	}else{

	  _vp_quantize_couple(look->psy_look[blocktype],
			      info,
			      pcm,
			      sofar,
			      quantized,
			      nonzero,
			      i);
	}
      }
    }
    seq+=vi->channels;
  } 

  look->lastframe=(long)vb->sequence;
  return(0);
}

static int mapping0_inverse(vorbis_block *vb,vorbis_look_mapping *l){
  vorbis_dsp_state     *vd=vb->vd;
  vorbis_info          *vi=vd->vi;
  codec_setup_info     *ci=static_cast<codec_setup_info *>(vi->codec_setup);
  backend_lookup_state *b=static_cast<backend_lookup_state *>(vd->backend_state);
  vorbis_look_mapping *look=(vorbis_look_mapping *)l;
  vorbis_info_mapping *info=look->map;
  vorbis_info_mode     *mode=look->mode;
  int                   i,j;
  long                  n=vb->pcmend=ci->blocksizes[vb->W];

  float *window=b->window[vb->W][vb->lW][vb->nW][mode->windowtype];
  float **pcmbundle=(float **)alloca(sizeof(*pcmbundle)*vi->channels);
  int    *zerobundle=(int *)alloca(sizeof(*zerobundle)*vi->channels);

  int   *nonzero  =(int *)alloca(sizeof(*nonzero)*vi->channels);
  void **floormemo=(void **)alloca(sizeof(*floormemo)*vi->channels);
  
  /* time domain information decode (note that applying the
     information would have to happen later; we'll probably add a
     function entry to the harness for that later */
  /* NOT IMPLEMENTED */

  /* recover the spectral envelope; store it in the PCM vector for now */
  for(i=0;i<vi->channels;i++){
    int submap=info->chmuxlist[i];
    floormemo[i]=look->floor_func[submap]->
      inverse1(vb,look->floor_look[submap]);
    if(floormemo[i])
      nonzero[i]=1;
    else
      nonzero[i]=0;      
    FastMemSet(vb->pcm[i],0,sizeof(*vb->pcm[i])*n/2);
  }

  /* channel coupling can 'dirty' the nonzero listing */
  for(i=0;i<info->coupling_steps;i++){
    if(nonzero[info->coupling_mag[i]] ||
       nonzero[info->coupling_ang[i]]){
      nonzero[info->coupling_mag[i]]=1; 
      nonzero[info->coupling_ang[i]]=1; 
    }
  }

  /* recover the residue into our working vectors */
  for(i=0;i<info->submaps;i++){
    int ch_in_bundle=0;
    for(j=0;j<vi->channels;j++){
      if(info->chmuxlist[j]==i){
	if(nonzero[j])
	  zerobundle[ch_in_bundle]=1;
	else
	  zerobundle[ch_in_bundle]=0;
	pcmbundle[ch_in_bundle++]=vb->pcm[j];
      }
    }
    
    look->residue_func[i]->inverse(vb,look->residue_look[i],
				   pcmbundle,zerobundle,ch_in_bundle);
  }

  /* channel coupling */
  for(i=info->coupling_steps-1;i>=0;i--){
    float *pcmM=vb->pcm[info->coupling_mag[i]];
    float *pcmA=vb->pcm[info->coupling_ang[i]];

    for(j=0;j<n/2;j++){
      float mag=pcmM[j];
      float ang=pcmA[j];

      if(mag>0)
	if(ang>0){
	  pcmM[j]=mag;
	  pcmA[j]=mag-ang;
	}else{
	  pcmA[j]=mag;
	  pcmM[j]=mag+ang;
	}
      else
	if(ang>0){
	  pcmM[j]=mag;
	  pcmA[j]=mag+ang;
	}else{
	  pcmA[j]=mag;
	  pcmM[j]=mag-ang;
	}
    }
  }

  /* compute and apply spectral envelope */
  for(i=0;i<vi->channels;i++){
    float *pcm=vb->pcm[i];
    int submap=info->chmuxlist[i];
    look->floor_func[submap]->
      inverse2(vb,look->floor_look[submap],floormemo[i],pcm);
  }

  /* transform the PCM data; takes PCM vector, vb; modifies PCM vector */
  /* only MDCT right now.... */
  for(i=0;i<vi->channels;i++){
    float *pcm=vb->pcm[i];
    mdct_backward(b->transform[vb->W][0],pcm,pcm);
  }

  /* window the data */
  for(i=0;i<vi->channels;i++){
    float *pcm=vb->pcm[i];
    if(nonzero[i])
      for(j=0;j<n;j++)
	pcm[j]*=window[j];
    else
      for(j=0;j<n;j++)
	pcm[j]=0.f;

  }
	    
  /* all done! */
  return(0);
}

/* export hooks */
vorbis_func_mapping mapping0_exportbundle={
	&mapping0_pack,
	&mapping0_unpack,
	&mapping0_look,
	&mapping0_copy_info,
	&mapping0_free_info,
	&mapping0_free_look,
	&mapping0_forward,
	&mapping0_inverse
};
