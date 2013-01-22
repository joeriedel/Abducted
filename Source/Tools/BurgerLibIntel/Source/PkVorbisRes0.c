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

 function: residue backend 0, 1 and 2 implementation
 last mod: $Id: PkVorbisRes0.c,v 1.8 2003/12/02 08:19:16 burger Exp $

 ********************************************************************/

/* Slow, slow, slow, simpleminded and did I mention it was slow?  The
   encode/decode loops are coded for clarity and performance is not
   yet even a nagging little idea lurking in the shadows.  Oh and BTW,
   it's slow. */

#include "PkVorbisBackends.h"
#include "PkVorbisCodebook.h"
#include "PkVorbisBitrate.h"
#include "PkVorbisCodec.h"
#include "PkVorbisCodecInternal.h"
#include "MmMemory.h"
#include "ClStdlib.h"
#include "FpFloat.h"
#include <math.h>
#if defined(__MAC__) || defined(__MACOSX__)
#include <alloca.h>
#else
#include <malloc.h>
#endif

#ifdef TRAIN_RES
#include <stdio.h>
#endif 

typedef struct vorbis_look_residue {
	vorbis_info_residue *info;
	int         map;
	int         parts;
	int         stages;
	codebook   *fullbooks;
	codebook   *phrasebook;
	codebook ***partbooks;
	int         partvals;
	int **decodemap;
	long      postbits;
	long      phrasebits;
	long      frames;
	int       qoffsets[BITTRACK_DIVISOR+1];

#ifdef TRAIN_RES
	long      *training_data[8][64];
	float      training_max[8][64];
	float      training_min[8][64];
	int       longp;
	float     tmin;
	float     tmax;
#endif
} vorbis_look_residue;

static vorbis_info_residue *res0_copy_info(vorbis_info_residue *info)
{
	vorbis_info_residue *ret=static_cast<vorbis_info_residue *>(AllocAPointer(sizeof(*ret)));
	FastMemCpy(ret,info,sizeof(*ret));
	return(ret);
}

static void res0_free_info(vorbis_info_residue *i){
  vorbis_info_residue *info=(vorbis_info_residue *)i;
  if(info){
    FastMemSet(info,0,sizeof(*info));
    DeallocAPointer(info);
  }
}

static void res0_free_look(vorbis_look_residue *i)
{
	int j;
	if(i){
		vorbis_look_residue *look=(vorbis_look_residue *)i;

#ifdef TRAIN_RES
		{
			int j,k,l;
			for(j=0;j<look->parts;j++){
				fprintf(stderr,"partition %d: ",j);
				for(k=0;k<8;k++)
				if(look->training_data[k][j]){
					char buffer[80];
					FILE *of;
					codebook *statebook=look->partbooks[j][k];

					/* long and short into the same bucket by current convention */
					sprintf(buffer,"res_part%d_pass%d.vqd",j,k);
					of=fopen(buffer,"a");

					for(l=0;l<statebook->entries;l++)
						fprintf(of,"%d:%ld\n",l,look->training_data[k][j][l]);
					fclose(of);
					fprintf(stderr,"%d(%.2f|%.2f) ",k,look->training_min[k][j],look->training_max[k][j]);
					DeallocAPointer(look->training_data[k][j]);
				}
				fprintf(stderr,"\n");
			}
		}
		fprintf(stderr,"min/max residue: %g::%g\n",look->tmin,look->tmax);

		fprintf(stderr,"residue bit usage %f:%f (%f total)\n",
			(float)look->phrasebits/look->frames,
			(float)look->postbits/look->frames,
			(float)(look->postbits+look->phrasebits)/look->frames);
	#endif


		/*vorbis_info_residue0 *info=look->info;

		fprintf(stderr,"%ld frames encoded in %ld phrasebits and %ld residue bits "
			"(%g/frame) \n",look->frames,look->phrasebits,
			look->resbitsflat,
			(look->phrasebits+look->resbitsflat)/(float)look->frames);

		for(j=0;j<look->parts;j++){
			long acc=0;
			fprintf(stderr,"\t[%d] == ",j);
			for(k=0;k<look->stages;k++) {
				if((info->secondstages[j]>>k)&1){
					fprintf(stderr,"%ld,",look->resbits[j][k]);
					acc+=look->resbits[j][k];
				}
			}
			fprintf(stderr,":: (%ld vals) %1.2fbits/sample\n",look->resvals[j],
				acc?(float)acc/(look->resvals[j]*info->grouping):0);
		}
		fprintf(stderr,"\n");*/

		for(j=0;j<look->parts;j++)
			DeallocAPointer(look->partbooks[j]);
		DeallocAPointer(look->partbooks);

//		for(j=0;j<look->partvals;j++)
//			DeallocAPointer(look->decodemap[j]);
		DeallocAPointer(look->decodemap);		/* It's a large array */
		FastMemSet(look,0,sizeof(*look));
		DeallocAPointer(look);
	}
}

static int ilog(unsigned int v)
{
	int ret=0;
	if (v) {
		do {
			ret++;
			v>>=1;
		} while (v);
	}
	return(ret);
}

static int icount(unsigned int v)
{
	int ret=0;
	if (v) {
		do {
			ret+=v&1;
			v>>=1;
		} while (v);
	}
	return(ret);
}


static void res0_pack(vorbis_info_residue *vr,oggpack_buffer *opb){
  vorbis_info_residue *info=(vorbis_info_residue *)vr;
  int j,acc=0;
  oggpack_write(opb,info->begin,24);
  oggpack_write(opb,info->end,24);

  oggpack_write(opb,info->grouping-1,24);  /* residue vectors to group and 
					     code with a partitioned book */
  oggpack_write(opb,info->partitions-1,6); /* possible partition choices */
  oggpack_write(opb,info->groupbook,8);  /* group huffman book */

  /* secondstages is a bitmask; as encoding progresses pass by pass, a
     bitmask of one indicates this partition class has bits to write
     this pass */
  for(j=0;j<info->partitions;j++){
    if(ilog(info->secondstages[j])>3){
      /* yes, this is a minor hack due to not thinking ahead */
      oggpack_write(opb,info->secondstages[j],3); 
      oggpack_write(opb,1,1);
      oggpack_write(opb,info->secondstages[j]>>3,5); 
    }else
      oggpack_write(opb,info->secondstages[j],4); /* trailing zero */
    acc+=icount(info->secondstages[j]);
  }
  for(j=0;j<acc;j++)
    oggpack_write(opb,info->booklist[j],8);

}

/* vorbis_info is for range checking */
static vorbis_info_residue *res0_unpack(vorbis_info *vi,oggpack_buffer *opb){
  int j,acc=0;
  vorbis_info_residue *info=static_cast<vorbis_info_residue *>(AllocAPointerClear(sizeof(*info)));
  codec_setup_info     *ci=static_cast<codec_setup_info *>(vi->codec_setup);

  info->begin=oggpack_read(opb,24);
  info->end=oggpack_read(opb,24);
  info->grouping=oggpack_read(opb,24)+1;
  info->partitions=oggpack_read(opb,6)+1;
  info->groupbook=oggpack_read(opb,8);

  for(j=0;j<info->partitions;j++){
    int cascade=oggpack_read(opb,3);
    if(oggpack_read1(opb))
      cascade|=(oggpack_read(opb,5)<<3);
    info->secondstages[j]=cascade;

    acc+=icount(cascade);
  }
  for(j=0;j<acc;j++)
    info->booklist[j]=oggpack_read(opb,8);

  if(info->groupbook>=ci->books)goto errout;
  for(j=0;j<acc;j++)
    if(info->booklist[j]>=ci->books)goto errout;

  return(info);
 errout:
  res0_free_info(info);
  return(NULL);
}

static vorbis_look_residue *res0_look(vorbis_dsp_state *vd,vorbis_info_mode *vm,
	vorbis_info_residue *vr)
{
	vorbis_info_residue *info=(vorbis_info_residue *)vr;
	vorbis_look_residue *look=static_cast<vorbis_look_residue *>(AllocAPointerClear(sizeof(*look)));
	backend_lookup_state *be=static_cast<backend_lookup_state *>(vd->backend_state);

	int j,k,acc=0;
	int dim;
	int maxstage=0;
	look->info=info;
	look->map=vm->mapping;

	look->parts=info->partitions;
	look->fullbooks=be->fullbooks;
	look->phrasebook=be->fullbooks+info->groupbook;
	dim=look->phrasebook->dim;

	look->partbooks=static_cast<codebook ***>(AllocAPointerClear(look->parts*sizeof(*look->partbooks)));

	for(j=0;j<look->parts;j++){
		int stages=ilog(info->secondstages[j]);
		if(stages){
			if(stages>maxstage)maxstage=stages;
			look->partbooks[j]=static_cast<codebook **>(AllocAPointerClear(stages*sizeof(*look->partbooks[j])));
			for(k=0;k<stages;k++)
				if(info->secondstages[j]&(1<<k)){
					look->partbooks[j][k]=be->fullbooks+info->booklist[acc++];
#ifdef TRAIN_RES
					look->training_data[k][j]=AllocAPointerClear(look->partbooks[j][k]->entries*sizeof(***look->training_data));
#endif
				}
		}
	}

	look->partvals=(int)floor(pow((float)look->parts,(float)dim)+0.5f);
	look->stages=maxstage;
	look->decodemap=static_cast<int **>(AllocAPointerClear((look->partvals*sizeof(*look->decodemap))+(look->partvals*dim*sizeof(look->decodemap[0]))));
	{
		int *WorkPtr;
		
		WorkPtr = (int *)((Word8 *)look->decodemap+(look->partvals*sizeof(*look->decodemap)));
		for(j=0;j<look->partvals;j++){
			long val=j;
			long mult=look->partvals/look->parts;
			look->decodemap[j]=WorkPtr;
			for(k=0;k<dim;k++){
				long deco=val/mult;
				val-=deco*mult;
				mult/=look->parts;
				WorkPtr[k]=deco;
			}
			WorkPtr+=dim;
		}
	}
	{
		int samples_per_partition=info->grouping;
		int n=info->end-info->begin,i;
		int partvals=n/samples_per_partition;

		for(i=0;i<BITTRACK_DIVISOR;i++)
			look->qoffsets[i]=partvals*(i+1)/BITTRACK_DIVISOR;
		look->qoffsets[i]=9999999;
	}
	return(look);
}


#if 0
/* does not guard against invalid settings; eg, a subn of 16 and a
   subgroup request of 32.  Max subn of 128 */
static int _interleaved_testhack(float *vec,int n,vorbis_look_residue0 *look,
				 int auxparts,int auxpartnum){
  vorbis_info_residue0 *info=look->info;
  int i,j=0;
  float max,localmax=0.f;
  float temp[128];
  float entropy[8];

  /* setup */
  for(i=0;i<n;i++)temp[i]=fabs(vec[i]);

  /* handle case subgrp==1 outside */
  for(i=0;i<n;i++)
    if(temp[i]>localmax)localmax=temp[i];
  max=localmax;

  for(i=0;i<n;i++)temp[i]=rint(temp[i]);
  
  while(1){
    entropy[j]=localmax;
    n>>=1;
    if(!n)break;
    j++;

    for(i=0;i<n;i++){
      temp[i]+=temp[i+n];
    }
    localmax=0.f;
    for(i=0;i<n;i++)
      if(temp[i]>localmax)localmax=temp[i];
  }

  for(i=0;i<auxparts-1;i++)
    if(auxpartnum<info->blimit[i] &&
       entropy[info->subgrp[i]]<=info->entmax[i] &&
       max<=info->ampmax[i])
      break;

  return(i);
}
#endif


static int _testhack(float *vec,int n,vorbis_look_residue *look,
		     int auxparts,int auxpartnum){
  vorbis_info_residue *info=look->info;
  int i;
  float max=0.f;
  float temp[128];
  float entropy=0.f;

  /* setup */
  for(i=0;i<n;i++)temp[i]=FloatAbs(vec[i]);

  for(i=0;i<n;i++)
    if(temp[i]>max)max=temp[i];

	for(i=0;i<n;i++) {
		temp[i]=(float)floor(temp[i]+0.5f);
	}
  for(i=0;i<n;i++)
    entropy+=temp[i];

  for(i=0;i<auxparts-1;i++)
    if(auxpartnum<info->blimit[i] &&
       entropy<=info->entmax[i] &&
       max<=info->ampmax[i])
      break;

  return(i);
}

#ifdef TRAIN_RES
#define ACC acc
#else
#define ACC
#endif

static int _interleaved_encodepart(oggpack_buffer *opb,float *vec, int n,
				   codebook *book,long *ACC){
  int i,bits=0;
  int dim=book->dim;
  int step=n/dim;

  for(i=0;i<step;i++){
    int entry=vorbis_book_besterror(book,vec+i,step,0);

#ifdef TRAIN_RES
    acc[entry]++;
#endif

    bits+=vorbis_book_encode(book,entry,opb);
  }

  return(bits);
}
 
static int _encodepart(oggpack_buffer *opb,float *vec, int n,
		       codebook *book,long *ACC){
  int i,bits=0;
  int dim=book->dim;
  int step=n/dim;

  for(i=0;i<step;i++){
    int entry=vorbis_book_besterror(book,vec+i*dim,1,0);

#ifdef TRAIN_RES
    acc[entry]++;
#endif

    bits+=vorbis_book_encode(book,entry,opb);
  }

  return(bits);
}

static long **_01class(vorbis_block *vb,vorbis_look_residue *vl,
		       float **input,int ch,
		       int (*classify)(float *,int,vorbis_look_residue *,
				       int,int)){
  long i,j;
  vorbis_look_residue *look=(vorbis_look_residue *)vl;
  vorbis_info_residue *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int possible_partitions=info->partitions;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  long **partword=static_cast<long **>(_vorbis_block_alloc(vb,ch*sizeof(*partword)));

  /* we find the partition type for each partition of each
     channel.  We'll go back and do the interleaved encoding in a
     bit.  For now, clarity */
 
  for(i=0;i<ch;i++){
    partword[i]=static_cast<long *>(_vorbis_block_alloc(vb,n/samples_per_partition*sizeof(*partword[i])));
    FastMemSet(partword[i],0,n/samples_per_partition*sizeof(*partword[i]));
  }

  for(i=0;i<partvals;i++){
    for(j=0;j<ch;j++)
      /* do the partition decision based on the 'entropy'
         int the block */
      partword[j][i]=
	classify(input[j]+i*samples_per_partition+info->begin,
		 samples_per_partition,look,possible_partitions,i);
  
  }

#ifdef TRAIN_RES
  look->longp=vb->W;
  {
    FILE *of;
    char buffer[80];
  
    for(i=0;i<ch;i++){
      sprintf(buffer,"resaux_%s.vqd",(vb->mode?"long":"short"));
      of=fopen(buffer,"a");
      for(j=0;j<partvals;j++)
	fprintf(of,"%ld, ",partword[i][j]);
      fprintf(of,"\n");
      fclose(of);
    }
  }
#endif
  look->frames++;

  return(partword);
}

static long **_2class(vorbis_block *vb,vorbis_look_residue *vl,
		       float **input,int ch,
		       int (*classify)(float *,int,vorbis_look_residue *,
				       int,int)){
  long i,j,k,l;
  vorbis_look_residue *look=(vorbis_look_residue *)vl;
  vorbis_info_residue *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int possible_partitions=info->partitions;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  long **partword=static_cast<long **>(_vorbis_block_alloc(vb,sizeof(*partword)));
  float *work=(float *)alloca(sizeof(*work)*samples_per_partition);

#ifdef TRAIN_RES
  FILE *of;
  char buffer[80];
#endif
  
  partword[0]=static_cast<long *>(_vorbis_block_alloc(vb,n*ch/samples_per_partition*sizeof(*partword[0])));
  FastMemSet(partword[0],0,n*ch/samples_per_partition*sizeof(*partword[0]));

  for(i=0,j=0,k=0,l=info->begin;i<partvals;i++){
    for(k=0;k<samples_per_partition;k++){
      work[k]=input[j][l];
      j++;
      if(j>=ch){
	j=0;
	l++;
      }
    }

    partword[0][i]=
      classify(work,samples_per_partition,look,possible_partitions,i);


  }  

#ifdef TRAIN_RES
  look->longp=vb->W;
  sprintf(buffer,"resaux_%s.vqd",(vb->mode?"long":"short"));
  of=fopen(buffer,"a");
  for(i=0;i<partvals;i++)
    fprintf(of,"%ld, ",partword[0][i]);
  fprintf(of,"\n");
  fclose(of);
#endif

  look->frames++;

  return(partword);
}

static int _01forward(vorbis_block *vb,vorbis_look_residue *vl,
		      float **input,int ch,
		      int pass,long **partword,
		      int (*encode)(oggpack_buffer *,float *,int,
				    codebook *,long *),
		      Word32 *stats){
  long i,j,k,s;
  vorbis_look_residue *look=(vorbis_look_residue *)vl;
  vorbis_info_residue *info=look->info;

  vorbis_dsp_state      *vd=vb->vd;
  vorbis_info           *vi=vd->vi;
  codec_setup_info      *ci=static_cast<codec_setup_info *>(vi->codec_setup);


  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int possible_partitions=info->partitions;
  int partitions_per_word=look->phrasebook->dim;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  long resbits[128];
  long resvals[128];

#ifdef TRAIN_RES
  for(i=0;i<ch;i++)
    for(j=info->begin;j<info->end;j++){
      if(input[i][j]>look->tmax)look->tmax=input[i][j];
      if(input[i][j]<look->tmin)look->tmin=input[i][j];
    }
#endif

  FastMemSet(resbits,0,sizeof(resbits));
  FastMemSet(resvals,0,sizeof(resvals));
  
  /* we code the partition words for each channel, then the residual
     words for a partition per channel until we've written all the
     residual words for that partition word.  Then write the next
     partition channel words... */

  for(s=(pass==0?0:ci->passlimit[pass-1]);s<ci->passlimit[pass];s++){
    int bin=0;
    Word32 *qptr=NULL;
    if(stats)qptr=stats+s*BITTRACK_DIVISOR;

    for(i=0;i<partvals;){

      /* first we encode a partition codeword for each channel */
      if(s==0){
	for(j=0;j<ch;j++){
	  long val=partword[j][i];
	  for(k=1;k<partitions_per_word;k++){
	    val*=possible_partitions;
	    if(i+k<partvals)
	      val+=partword[j][i+k];
	  }	

	  /* training hack */
	  if(val<look->phrasebook->entries)
	    look->phrasebits+=vorbis_book_encode(look->phrasebook,val,&vb->opb);
#ifdef TRAIN_RES
	  else
	    fprintf(stderr,"!");
#endif
	
	}
      }
      
      /* now we encode interleaved residual values for the partitions */
      for(k=0;k<partitions_per_word && i<partvals;k++,i++){
	long offset=i*samples_per_partition+info->begin;
	
	if(qptr)while(i>=look->qoffsets[bin])
	  qptr[bin++]=oggpack_bits(&vb->opb);

	for(j=0;j<ch;j++){
	  if(s==0)resvals[partword[j][i]]+=samples_per_partition;
	  if(info->secondstages[partword[j][i]]&(1<<s)){
	    codebook *statebook=look->partbooks[partword[j][i]][s];
	    if(statebook){
	      int ret;
	      long *accumulator=NULL;

#ifdef TRAIN_RES
	      accumulator=look->training_data[s][partword[j][i]];
	      {
		int l;
		float *samples=input[j]+offset;
		for(l=0;l<samples_per_partition;l++){
		  if(samples[l]<look->training_min[s][partword[j][i]])
		    look->training_min[s][partword[j][i]]=samples[l];
		  if(samples[l]>look->training_max[s][partword[j][i]])
		    look->training_max[s][partword[j][i]]=samples[l];
		}
	      }
#endif
	      
	      ret=encode(&vb->opb,input[j]+offset,samples_per_partition,
			 statebook,accumulator);

	      look->postbits+=ret;
	      resbits[partword[j][i]]+=ret;
	    }
	  }
	}
      }
      if(qptr)while(i>=look->qoffsets[bin])
	qptr[bin++]=oggpack_bits(&vb->opb);
    }
  }

  /*{
    long total=0;
    long totalbits=0;
    fprintf(stderr,"%d :: ",vb->mode);
    for(k=0;k<possible_partitions;k++){
      fprintf(stderr,"%ld/%1.2g, ",resvals[k],(float)resbits[k]/resvals[k]);
      total+=resvals[k];
      totalbits+=resbits[k];
      }
    
    fprintf(stderr,":: %ld:%1.2g\n",total,(double)totalbits/total);
    }*/
  return(0);
}

/* a truncated packet here just means 'stop working'; it's not an error */
static int _01inverse(vorbis_block *vb,vorbis_look_residue *vl,
		      float **input,int ch,
		      long (BURGERCALL * decodepart)(codebook *, float *, 
					 oggpack_buffer *,int)){

  long i,j,k,l,s;
  vorbis_look_residue *look=(vorbis_look_residue *)vl;
  vorbis_info_residue *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int partitions_per_word=look->phrasebook->dim;
  int n=info->end-info->begin;
  
  int partvals=n/samples_per_partition;
  int partwords=(partvals+partitions_per_word-1)/partitions_per_word;
  int ***partword=(int ***)alloca(ch*sizeof(*partword));

  for(j=0;j<ch;j++)
    partword[j]=static_cast<int **>(_vorbis_block_alloc(vb,partwords*sizeof(*partword[j])));

  for(s=0;s<look->stages;s++){

    /* each loop decodes on partition codeword containing 
       partitions_pre_word partitions */
    for(i=0,l=0;i<partvals;l++){
      if(s==0){
	/* fetch the partition word for each channel */
	for(j=0;j<ch;j++){
	  int temp=vorbis_book_decode(look->phrasebook,&vb->opb);
	  if(temp==-1)goto eopbreak;
	  partword[j][l]=look->decodemap[temp];
	  if(partword[j][l]==NULL)goto errout;
	}
      }
      
      /* now we decode residual values for the partitions */
      for(k=0;k<partitions_per_word && i<partvals;k++,i++)
	for(j=0;j<ch;j++){
	  long offset=info->begin+i*samples_per_partition;
	  if(info->secondstages[partword[j][l][k]]&(1<<s)){
	    codebook *stagebook=look->partbooks[partword[j][l][k]][s];
	    if(stagebook){
	      if(decodepart(stagebook,input[j]+offset,&vb->opb,
			    samples_per_partition)==-1)goto eopbreak;
	    }
	  }
	}
    } 
  }
  
 errout:
 eopbreak:
  return(0);
}

/* residue 0 and 1 are just slight variants of one another. 0 is
   interleaved, 1 is not */
static long **res0_class(vorbis_block *vb,vorbis_look_residue *vl,
		  float **input,int *nonzero,int ch){
  /* we encode only the nonzero parts of a bundle */
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      input[used++]=input[i];
  if(used)
    /*return(_01class(vb,vl,input,used,_interleaved_testhack));*/
    return(_01class(vb,vl,input,used,_testhack));
  else
    return(0);
}

static int res0_forward(vorbis_block *vb,vorbis_look_residue *vl,
		 float **input,float **output,int *nonzero,int ch,
		 int pass, long **partword,Word32 *stats){
  /* we encode only the nonzero parts of a bundle */
  int i,j,used=0,n=vb->pcmend/2;
  for(i=0;i<ch;i++)
    if(nonzero[i]){
      for(j=0;j<n;j++)
	output[i][j]+=input[i][j];
      input[used++]=input[i];
    }
  if(used){
    int ret=_01forward(vb,vl,input,used,pass,partword,
		      _interleaved_encodepart,stats);
    used=0;
    for(i=0;i<ch;i++)
      if(nonzero[i]){
	for(j=0;j<n;j++)
	  output[i][j]-=input[used][j];
	used++;
      }
    return(ret);
  }else{
    for(i=0;i<vorbis_bitrate_maxmarkers();i++)
      stats[i]=oggpack_bits(&vb->opb);

    return(0);
  }
}

static int res0_inverse(vorbis_block *vb,vorbis_look_residue *vl,
		 float **input,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      input[used++]=input[i];
  if(used)
    return(_01inverse(vb,vl,input,used,vorbis_book_decodevs_add));
  else
    return(0);
}

static int res1_forward(vorbis_block *vb,vorbis_look_residue *vl,
		 float **input,float **output,int *nonzero,int ch,
		 int pass, long **partword, Word32 *stats){
  int i,j,used=0,n=vb->pcmend/2;
  for(i=0;i<ch;i++)
    if(nonzero[i]){
      for(j=0;j<n;j++)
	output[i][j]+=input[i][j];
      input[used++]=input[i];
    }

  if(used){
    int ret=_01forward(vb,vl,input,used,pass,partword,_encodepart,stats);
    used=0;
    for(i=0;i<ch;i++)
      if(nonzero[i]){
	for(j=0;j<n;j++)
	  output[i][j]-=input[used][j];
	used++;
      }
    return(ret);
  }else{
    for(i=0;i<vorbis_bitrate_maxmarkers();i++)
      stats[i]=oggpack_bits(&vb->opb);

    return(0);
  }
}

static long **res1_class(vorbis_block *vb,vorbis_look_residue *vl,
		  float **input,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      input[used++]=input[i];
  if(used)
    return(_01class(vb,vl,input,used,_testhack));
  else
    return(0);
}

static int res1_inverse(vorbis_block *vb,vorbis_look_residue *vl,
		 float **input,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      input[used++]=input[i];
  if(used)
    return(_01inverse(vb,vl,input,used,vorbis_book_decodev_add));
  else
    return(0);
}

static long **res2_class(vorbis_block *vb,vorbis_look_residue *vl,
		  float **input,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      input[used++]=input[i];
  if(used)
    return(_2class(vb,vl,input,used,_testhack));
  else
    return(0);
}

/* res2 is slightly more different; all the channels are interleaved
   into a single vector and encoded. */

static int res2_forward(vorbis_block *vb,vorbis_look_residue *vl,
		 float **input,float **output,int *nonzero,int ch,
		 int pass,long **partword,Word32 *stats){
  long i,j,k,n=vb->pcmend/2,used=0;

  /* don't duplicate the code; use a working vector hack for now and
     reshape ourselves into a single channel res1 */
  /* ugly; reallocs for each coupling pass :-( */
  float *work=static_cast<float *>(_vorbis_block_alloc(vb,ch*n*sizeof(*work)));
  for(i=0;i<ch;i++){
    float *pcm=input[i];
    if(nonzero[i])used++;
    for(j=0,k=i;j<n;j++,k+=ch)
      work[k]=pcm[j];
  }
  
  if(used){
    int ret=_01forward(vb,vl,&work,1,pass,partword,_encodepart,stats);
    /* update the sofar vector */
    for(i=0;i<ch;i++){
      float *pcm=input[i];
      float *sofar=output[i];
      for(j=0,k=i;j<n;j++,k+=ch)
	sofar[j]+=pcm[j]-work[k];

    }
    return(ret);
  }else{
    for(i=0;i<vorbis_bitrate_maxmarkers();i++)
      stats[i]=oggpack_bits(&vb->opb);

    return(0);
  }
}

/* duplicate code here as speed is somewhat more important */
static int res2_inverse(vorbis_block *vb,vorbis_look_residue *vl,
		 float **input,int *nonzero,int ch){
  long i,k,l,s;
  vorbis_look_residue *look=(vorbis_look_residue *)vl;
  vorbis_info_residue *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int partitions_per_word=look->phrasebook->dim;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  int partwords=(partvals+partitions_per_word-1)/partitions_per_word;
  int **partword=static_cast<int **>(_vorbis_block_alloc(vb,partwords*sizeof(*partword)));

  for(i=0;i<ch;i++)if(nonzero[i])break;
  if(i==ch)return(0); /* no nonzero vectors */

  for(s=0;s<look->stages;s++){
    for(i=0,l=0;i<partvals;l++){

      if(s==0){
	/* fetch the partition word */
	int temp=vorbis_book_decode(look->phrasebook,&vb->opb);
	if(temp==-1)goto eopbreak;
	partword[l]=look->decodemap[temp];
	if(partword[l]==NULL)goto errout;
      }

      /* now we decode residual values for the partitions */
      for(k=0;k<partitions_per_word && i<partvals;k++,i++)
	if(info->secondstages[partword[l][k]]&(1<<s)){
	  codebook *stagebook=look->partbooks[partword[l][k]][s];
	  
	  if(stagebook){
	    if(vorbis_book_decodevv_add(stagebook,input,
					i*samples_per_partition+info->begin,ch,
					&vb->opb,samples_per_partition)==-1)
	      goto eopbreak;
	  }
	}
    } 
  }
  
 errout:
 eopbreak:
  return(0);
}


vorbis_func_residue residue0_exportbundle={
	&res0_pack,
	&res0_unpack,
	&res0_look,
	&res0_copy_info,
	&res0_free_info,
	&res0_free_look,
	&res0_class,
	&res0_forward,
	&res0_inverse
};

vorbis_func_residue residue1_exportbundle={
	&res0_pack,
	&res0_unpack,
	&res0_look,
	&res0_copy_info,
	&res0_free_info,
	&res0_free_look,
	&res1_class,
	&res1_forward,
	&res1_inverse
};

vorbis_func_residue residue2_exportbundle={
	&res0_pack,
	&res0_unpack,
	&res0_look,
	&res0_copy_info,
	&res0_free_info,
	&res0_free_look,
	&res2_class,
	&res2_forward,
	&res2_inverse
};
