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

 function: maintain the info structure, info <-> header packets
 last mod: $Id: PkVorbisInfo.c,v 1.6 2003/12/02 08:19:16 burger Exp $

 ********************************************************************/

/* general handling of the header and the vorbis_info structure (and
   substructures) */

#include "PkVorbisCodec.h"
#include "PkVorbisCodecInternal.h"
#include "PkVorbisRegistry.h"
#include "PkVorbisBackends.h"
#include "PkVorbisCodebook.h"
#include "ClStdlib.h"
#include "MmMemory.h"
#include <ctype.h>

/* helpers */
static int BURGERCALL ilog2(Word v)
{
	int ret=0;
	while(v>1){
		ret++;
		v>>=1;
	}
	return(ret);
}

static void BURGERCALL _v_writestring(oggpack_buffer *o,char *s, int bytes)
{
	while(bytes--){
		oggpack_write(o,*s++,8);
	}
}

static void BURGERCALL _v_readstring(oggpack_buffer *o,char *buf,int bytes)
{
	while(bytes--){
		*buf++=(char)oggpack_read(o,8);
	}
}

void BURGERCALL vorbis_comment_init(vorbis_comment *vc){
	FastMemSet(vc,0,sizeof(*vc));
}

void BURGERCALL vorbis_comment_add(vorbis_comment *vc,char *comment){
  vc->user_comments=static_cast<char **>(ResizeAPointer(vc->user_comments,
			    (vc->comments+2)*sizeof(*vc->user_comments)));
  vc->comment_lengths=static_cast<int *>(ResizeAPointer(vc->comment_lengths,
      			    (vc->comments+2)*sizeof(*vc->comment_lengths)));
  vc->comment_lengths[vc->comments]=strlen(comment);
  vc->user_comments[vc->comments]=static_cast<char *>(AllocAPointer(vc->comment_lengths[vc->comments]+1));
  strcpy(vc->user_comments[vc->comments], comment);
  vc->comments++;
  vc->user_comments[vc->comments]=NULL;
}

void BURGERCALL vorbis_comment_add_tag(vorbis_comment *vc, char *tag, char *contents){
  char *comment=static_cast<char *>(AllocAPointer(strlen(tag)+strlen(contents)+2)); /* +2 for = and \0 */
  strcpy(comment, tag);
  strcat(comment, "=");
  strcat(comment, contents);
  vorbis_comment_add(vc, comment);
  DeallocAPointer(comment);
}

/* This is more or less the same as strncasecmp - but that doesn't exist
 * everywhere, and this is a fairly trivial function, so we include it */
static int BURGERCALL tagcompare(const char *s1, const char *s2, int n){
  int c=0;
  while(c < n){
    if(toupper(s1[c]) != toupper(s2[c]))
      return !0;
    c++;
  }
  return 0;
}

char *BURGERCALL vorbis_comment_query(vorbis_comment *vc, char *tag, int count)
{
	long i;
	int found = 0;
	int taglen = strlen(tag)+1; /* +1 for the = we append */
	char *fulltag = static_cast<char *>(AllocAPointer(taglen+ 1));

	strcpy(fulltag, tag);
	strcat(fulltag, "=");

	for(i=0;i<vc->comments;i++){
		if(!tagcompare(vc->user_comments[i], fulltag, taglen)){
			if(count == found) {
				/* We return a pointer to the data, not a copy */
				DeallocAPointer(fulltag);
				return vc->user_comments[i] + taglen;
			} else {
				found++;
			}
		}
	}
	DeallocAPointer(fulltag);
	return NULL; /* didn't find anything */
}

int BURGERCALL vorbis_comment_query_count(vorbis_comment *vc, char *tag){
	int i,count=0;
	int taglen = strlen(tag)+1; /* +1 for the = we append */
	char *fulltag = static_cast<char *>(AllocAPointer(taglen+1));
	strcpy(fulltag,tag);
	strcat(fulltag, "=");

	for(i=0;i<vc->comments;i++){
		if(!tagcompare(vc->user_comments[i], fulltag, taglen)) {
			count++;
		}
	}
	DeallocAPointer(fulltag);
	return count;
}

void BURGERCALL vorbis_comment_clear(vorbis_comment *vc){
  if(vc){
    long i;
    for(i=0;i<vc->comments;i++)
      if(vc->user_comments[i])DeallocAPointer(vc->user_comments[i]);
    if(vc->user_comments)DeallocAPointer(vc->user_comments);
	if(vc->comment_lengths)DeallocAPointer(vc->comment_lengths);
    if(vc->vendor)DeallocAPointer(vc->vendor);
  }
  FastMemSet(vc,0,sizeof(*vc));
}

/* blocksize 0 is guaranteed to be short, 1 is guarantted to be long.
   They may be equal, but short will never ge greater than long */
int BURGERCALL vorbis_info_blocksize(vorbis_info *vi,int zo){
  codec_setup_info *ci = static_cast<codec_setup_info *>(vi->codec_setup);
  return ci ? ci->blocksizes[zo] : -1;
}

/* used by synthesis, which has a full, alloced vi */
void BURGERCALL vorbis_info_init(vorbis_info *vi)
{
	FastMemSet(vi,0,sizeof(*vi));
	vi->codec_setup=AllocAPointerClear(sizeof(codec_setup_info));
}

void BURGERCALL vorbis_info_clear(vorbis_info *vi){
  codec_setup_info     *ci=static_cast<codec_setup_info *>(vi->codec_setup);
  int i;

  if(ci){

    for(i=0;i<ci->modes;i++)
      if(ci->mode_param[i])DeallocAPointer(ci->mode_param[i]);

    for(i=0;i<ci->maps;i++) /* unpack does the range checking */
      _mapping_P[ci->map_type[i]]->free_info(ci->map_param[i]);

    for(i=0;i<ci->times;i++) /* unpack does the range checking */
      _time_P[ci->time_type[i]]->free_info(ci->time_param[i]);

    for(i=0;i<ci->floors;i++) /* unpack does the range checking */
      _floor_P[ci->floor_type[i]]->free_info(ci->floor_param[i]);
    
    for(i=0;i<ci->residues;i++) /* unpack does the range checking */
      _residue_P[ci->residue_type[i]]->free_info(ci->residue_param[i]);

    for(i=0;i<ci->books;i++){
      if(ci->book_param[i]){
	/* knows if the book was not alloced */
	vorbis_staticbook_destroy(ci->book_param[i]);
      }
    }
    
    for(i=0;i<ci->psys;i++)
      _vi_psy_free(ci->psy_param[i]);

    DeallocAPointer(ci);
  }

  FastMemSet(vi,0,sizeof(*vi));
}

/* Header packing/unpacking ********************************************/

static int BURGERCALL _vorbis_unpack_info(vorbis_info *vi,oggpack_buffer *opb){
	codec_setup_info     *ci=static_cast<codec_setup_info *>(vi->codec_setup);
	if(!ci)return(OV_EFAULT);

	vi->version=oggpack_read(opb,32);
	if(vi->version!=0)return(OV_EVERSION);

	vi->channels=oggpack_read(opb,8);
	vi->rate=oggpack_read(opb,32);

	vi->bitrate_upper=oggpack_read(opb,32);
	vi->bitrate_nominal=oggpack_read(opb,32);
	vi->bitrate_lower=oggpack_read(opb,32);

	ci->blocksizes[0]=1<<oggpack_read(opb,4);
	ci->blocksizes[1]=1<<oggpack_read(opb,4);

	if(vi->rate<1)goto err_out;
	if(vi->channels<1)goto err_out;
	if(ci->blocksizes[0]<8)goto err_out; 
	if(ci->blocksizes[1]<ci->blocksizes[0])goto err_out;

	if(oggpack_read1(opb)!=1)goto err_out; /* EOP check */

	return(0);
err_out:
	vorbis_info_clear(vi);
	return(OV_EBADHEADER);
}

static int BURGERCALL _vorbis_unpack_comment(vorbis_comment *vc,oggpack_buffer *opb){
	int i;
	int vendorlen=oggpack_read(opb,32);
	if(vendorlen>=0) {
		vc->vendor=static_cast<char *>(AllocAPointerClear(vendorlen+1));
		_v_readstring(opb,vc->vendor,vendorlen);
		vc->comments=oggpack_read(opb,32);
		if(vc->comments>=0) {
			vc->user_comments=static_cast<char **>(AllocAPointerClear((vc->comments+1)*sizeof(*vc->user_comments)));
			vc->comment_lengths=static_cast<int *>(AllocAPointerClear((vc->comments+1)*sizeof(*vc->comment_lengths)));

			for(i=0;i<vc->comments;i++){
				int len=oggpack_read(opb,32);
				if(len<0)goto err_out;
				vc->comment_lengths[i]=len;
				vc->user_comments[i]=static_cast<char *>(AllocAPointerClear(len+1));
				_v_readstring(opb,vc->user_comments[i],len);
			}	  
			if (oggpack_read1(opb)==1) {/* EOP check */
				return(0);
			}
		}
	}
err_out:
	vorbis_comment_clear(vc);
	return(OV_EBADHEADER);
}

/* all of the real encoding details are here.  The modes, books,
   everything */
static int BURGERCALL _vorbis_unpack_books(vorbis_info *vi,oggpack_buffer *opb){
  codec_setup_info     *ci=static_cast<codec_setup_info *>(vi->codec_setup);
  int i;
  if(!ci)return(OV_EFAULT);

  /* codebooks */
  ci->books=oggpack_read(opb,8)+1;
  /*ci->book_param=AllocAPointerClear(ci->books*sizeof(*ci->book_param));*/
  for(i=0;i<ci->books;i++){
    ci->book_param[i]=static_cast<static_codebook *>(AllocAPointerClear(sizeof(*ci->book_param[i])));
    if(vorbis_staticbook_unpack(opb,ci->book_param[i]))goto err_out;
  }

  /* time backend settings */
  ci->times=oggpack_read(opb,6)+1;
  /*ci->time_type=AllocAPointer(ci->times*sizeof(*ci->time_type));*/
  /*ci->time_param=AllocAPointerClear(ci->times*sizeof(void *));*/
  for(i=0;i<ci->times;i++){
    ci->time_type[i]=oggpack_read(opb,16);
    if(ci->time_type[i]<0 || ci->time_type[i]>=VI_TIMEB)goto err_out;
    ci->time_param[i]=_time_P[ci->time_type[i]]->unpack(vi,opb);
    if(!ci->time_param[i])goto err_out;
  }

  /* floor backend settings */
  ci->floors=oggpack_read(opb,6)+1;
  /*ci->floor_type=AllocAPointer(ci->floors*sizeof(*ci->floor_type));*/
  /*ci->floor_param=AllocAPointerClear(ci->floors*sizeof(void *));*/
  for(i=0;i<ci->floors;i++){
    ci->floor_type[i]=oggpack_read(opb,16);
    if(ci->floor_type[i]<0 || ci->floor_type[i]>=VI_FLOORB)goto err_out;
    ci->floor_param[i]=_floor_P[ci->floor_type[i]]->unpack(vi,opb);
    if(!ci->floor_param[i])goto err_out;
  }

  /* residue backend settings */
  ci->residues=oggpack_read(opb,6)+1;
  /*ci->residue_type=AllocAPointer(ci->residues*sizeof(*ci->residue_type));*/
  /*ci->residue_param=AllocAPointerClear(ci->residues*sizeof(void *));*/
  for(i=0;i<ci->residues;i++){
    ci->residue_type[i]=oggpack_read(opb,16);
    if(ci->residue_type[i]<0 || ci->residue_type[i]>=VI_RESB)goto err_out;
    ci->residue_param[i]=_residue_P[ci->residue_type[i]]->unpack(vi,opb);
    if(!ci->residue_param[i])goto err_out;
  }

  /* map backend settings */
  ci->maps=oggpack_read(opb,6)+1;
  /*ci->map_type=AllocAPointer(ci->maps*sizeof(*ci->map_type));*/
  /*ci->map_param=AllocAPointerClear(ci->maps*sizeof(void *));*/
  for(i=0;i<ci->maps;i++){
    ci->map_type[i]=oggpack_read(opb,16);
    if(ci->map_type[i]<0 || ci->map_type[i]>=VI_MAPB)goto err_out;
    ci->map_param[i]=_mapping_P[ci->map_type[i]]->unpack(vi,opb);
    if(!ci->map_param[i])goto err_out;
  }
  
  /* mode settings */
  ci->modes=oggpack_read(opb,6)+1;
  /*vi->mode_param=AllocAPointerClear(vi->modes*sizeof(void *));*/
  for(i=0;i<ci->modes;i++){
    ci->mode_param[i]=static_cast<vorbis_info_mode *>(AllocAPointerClear(sizeof(*ci->mode_param[i])));
    ci->mode_param[i]->blockflag=oggpack_read1(opb);
    ci->mode_param[i]->windowtype=oggpack_read(opb,16);
    ci->mode_param[i]->transformtype=oggpack_read(opb,16);
    ci->mode_param[i]->mapping=oggpack_read(opb,8);

    if(ci->mode_param[i]->windowtype>=VI_WINDOWB)goto err_out;
    if(ci->mode_param[i]->transformtype>=VI_WINDOWB)goto err_out;
    if(ci->mode_param[i]->mapping>=ci->maps)goto err_out;
  }
  
  if(oggpack_read1(opb)!=1)goto err_out; /* top level EOP check */

  return(0);
 err_out:
  vorbis_info_clear(vi);
  return(OV_EBADHEADER);
}

/* The Vorbis header is in three packets; the initial small packet in
   the first page that identifies basic parameters, a second packet
   with bitstream comments and a third packet that holds the
   codebook. */

int BURGERCALL vorbis_synthesis_headerin(vorbis_info *vi,vorbis_comment *vc,ogg_packet *op){
  oggpack_buffer opb;
  
  if(op){
    oggpack_readinit(&opb,op->packet,op->bytes);

    /* Which of the three types of header is this? */
    /* Also verify header-ness, vorbis */
    {
      char buffer[6];
      int packtype=oggpack_read(&opb,8);
      FastMemSet(buffer,0,6);
      _v_readstring(&opb,buffer,6);
      if(memcmp(buffer,"vorbis",6)){
	/* not a vorbis header */
	return(OV_ENOTVORBIS);
      }
      switch(packtype){
      case 0x01: /* least significant *bit* is read first */
	if(!op->b_o_s){
	  /* Not the initial packet */
	  return(OV_EBADHEADER);
	}
	if(vi->rate!=0){
	  /* previously initialized info header */
	  return(OV_EBADHEADER);
	}

	return(_vorbis_unpack_info(vi,&opb));

      case 0x03: /* least significant *bit* is read first */
	if(vi->rate==0){
	  /* um... we didn't get the initial header */
	  return(OV_EBADHEADER);
	}

	return(_vorbis_unpack_comment(vc,&opb));

      case 0x05: /* least significant *bit* is read first */
	if(vi->rate==0 || vc->vendor==NULL){
	  /* um... we didn;t get the initial header or comments yet */
	  return(OV_EBADHEADER);
	}

	return(_vorbis_unpack_books(vi,&opb));

      default:
		/* Not a valid vorbis header type */
		return(OV_EBADHEADER);
      }
    }
  }
  return(OV_EBADHEADER);
}

/* pack side **********************************************************/

static int BURGERCALL _vorbis_pack_info(oggpack_buffer *opb,vorbis_info *vi){
  codec_setup_info     *ci=static_cast<codec_setup_info *>(vi->codec_setup);
  if(!ci)return(OV_EFAULT);

  /* preamble */  
  oggpack_write(opb,0x01,8);
  _v_writestring(opb,"vorbis", 6);

  /* basic information about the stream */
  oggpack_write(opb,0x00,32);
  oggpack_write(opb,vi->channels,8);
  oggpack_write(opb,vi->rate,32);

  oggpack_write(opb,vi->bitrate_upper,32);
  oggpack_write(opb,vi->bitrate_nominal,32);
  oggpack_write(opb,vi->bitrate_lower,32);

  oggpack_write(opb,ilog2(ci->blocksizes[0]),4);
  oggpack_write(opb,ilog2(ci->blocksizes[1]),4);
  oggpack_write(opb,1,1);

  return(0);
}

static int BURGERCALL _vorbis_pack_comment(oggpack_buffer *opb,vorbis_comment *vc){
  char temp[]="Xiphophorus libVorbis I 20011231";
  int bytes = strlen(temp);

  /* preamble */  
  oggpack_write(opb,0x03,8);
  _v_writestring(opb,"vorbis", 6);

  /* vendor */
  oggpack_write(opb,bytes,32);
  _v_writestring(opb,temp, bytes);
  
  /* comments */

  oggpack_write(opb,vc->comments,32);
  if(vc->comments){
    int i;
    for(i=0;i<vc->comments;i++){
      if(vc->user_comments[i]){
	oggpack_write(opb,vc->comment_lengths[i],32);
	_v_writestring(opb,vc->user_comments[i], vc->comment_lengths[i]);
      }else{
	oggpack_write(opb,0,32);
      }
    }
  }
  oggpack_write(opb,1,1);

  return(0);
}
 
static int BURGERCALL _vorbis_pack_books(oggpack_buffer *opb,vorbis_info *vi){
  codec_setup_info     *ci=static_cast<codec_setup_info *>(vi->codec_setup);
  int i;
  if(!ci)return(OV_EFAULT);

  oggpack_write(opb,0x05,8);
  _v_writestring(opb,"vorbis", 6);

  /* books */
  oggpack_write(opb,ci->books-1,8);
  for(i=0;i<ci->books;i++)
    if(vorbis_staticbook_pack(ci->book_param[i],opb))goto err_out;

  /* times */
  oggpack_write(opb,ci->times-1,6);
  for(i=0;i<ci->times;i++){
    oggpack_write(opb,ci->time_type[i],16);
    _time_P[ci->time_type[i]]->pack(ci->time_param[i],opb);
  }

  /* floors */
  oggpack_write(opb,ci->floors-1,6);
  for(i=0;i<ci->floors;i++){
    oggpack_write(opb,ci->floor_type[i],16);
    _floor_P[ci->floor_type[i]]->pack(ci->floor_param[i],opb);
  }

  /* residues */
  oggpack_write(opb,ci->residues-1,6);
  for(i=0;i<ci->residues;i++){
    oggpack_write(opb,ci->residue_type[i],16);
    _residue_P[ci->residue_type[i]]->pack(ci->residue_param[i],opb);
  }

  /* maps */
  oggpack_write(opb,ci->maps-1,6);
  for(i=0;i<ci->maps;i++){
    oggpack_write(opb,ci->map_type[i],16);
    _mapping_P[ci->map_type[i]]->pack(vi,ci->map_param[i],opb);
  }

  /* modes */
  oggpack_write(opb,ci->modes-1,6);
  for(i=0;i<ci->modes;i++){
    oggpack_write(opb,ci->mode_param[i]->blockflag,1);
    oggpack_write(opb,ci->mode_param[i]->windowtype,16);
    oggpack_write(opb,ci->mode_param[i]->transformtype,16);
    oggpack_write(opb,ci->mode_param[i]->mapping,8);
  }
  oggpack_write(opb,1,1);

  return(0);
err_out:
  return(-1);
} 

int BURGERCALL vorbis_commentheader_out(vorbis_comment *vc,
    				      ogg_packet *op){

  oggpack_buffer opb;

  oggpack_writeinit(&opb);
  if(_vorbis_pack_comment(&opb,vc)) return OV_EIMPL;

  op->packet = static_cast<Word8 *>(AllocAPointer(oggpack_bytes(&opb)));
  FastMemCpy(op->packet, opb.buffer, oggpack_bytes(&opb));

  op->bytes=oggpack_bytes(&opb);
  op->b_o_s=0;
  op->e_o_s=0;
  op->granulepos=0;

  return 0;
}

int BURGERCALL vorbis_analysis_headerout(vorbis_dsp_state *v,
			      vorbis_comment *vc,
			      ogg_packet *op,
			      ogg_packet *op_comm,
			      ogg_packet *op_code){
  int ret=OV_EIMPL;
  vorbis_info *vi=v->vi;
  oggpack_buffer opb;
  backend_lookup_state *b=static_cast<backend_lookup_state *>(v->backend_state);

  if(!b){
    ret=OV_EFAULT;
    goto err_out;
  }

  /* first header packet **********************************************/

  oggpack_writeinit(&opb);
  if(_vorbis_pack_info(&opb,vi))goto err_out;

  /* build the packet */
  if(b->header)DeallocAPointer(b->header);
  b->header=static_cast<Word8 *>(AllocAPointer(oggpack_bytes(&opb)));
  FastMemCpy(b->header,opb.buffer,oggpack_bytes(&opb));
  op->packet=b->header;
  op->bytes=oggpack_bytes(&opb);
  op->b_o_s=1;
  op->e_o_s=0;
  op->granulepos=0;

  /* second header packet (comments) **********************************/

  oggpack_reset(&opb);
  if(_vorbis_pack_comment(&opb,vc))goto err_out;

  if(b->header1)DeallocAPointer(b->header1);
  b->header1=static_cast<Word8 *>(AllocAPointer(oggpack_bytes(&opb)));
  FastMemCpy(b->header1,opb.buffer,oggpack_bytes(&opb));
  op_comm->packet=b->header1;
  op_comm->bytes=oggpack_bytes(&opb);
  op_comm->b_o_s=0;
  op_comm->e_o_s=0;
  op_comm->granulepos=0;

  /* third header packet (modes/codebooks) ****************************/

  oggpack_reset(&opb);
  if(_vorbis_pack_books(&opb,vi))goto err_out;

  if(b->header2)DeallocAPointer(b->header2);
  b->header2=static_cast<Word8 *>(AllocAPointer(oggpack_bytes(&opb)));
  FastMemCpy(b->header2,opb.buffer,oggpack_bytes(&opb));
  op_code->packet=b->header2;
  op_code->bytes=oggpack_bytes(&opb);
  op_code->b_o_s=0;
  op_code->e_o_s=0;
  op_code->granulepos=0;

  oggpack_writeclear(&opb);
  return(0);
 err_out:
  oggpack_writeclear(&opb);
  FastMemSet(op,0,sizeof(*op));
  FastMemSet(op_comm,0,sizeof(*op_comm));
  FastMemSet(op_code,0,sizeof(*op_code));

  if(b->header)DeallocAPointer(b->header);
  if(b->header1)DeallocAPointer(b->header1);
  if(b->header2)DeallocAPointer(b->header2);
  b->header=NULL;
  b->header1=NULL;
  b->header2=NULL;
  return(ret);
}

