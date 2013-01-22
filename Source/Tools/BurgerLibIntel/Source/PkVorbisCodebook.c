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

 function: basic codebook pack/unpack/code/decode operations
 last mod: $Id: PkVorbisCodebook.c,v 1.5 2003/12/01 16:12:31 burger Exp $

 ********************************************************************/

#include "PkVorbisCodebook.h"
#include "ClStdlib.h"
#include "MmMemory.h"
#include <stdlib.h>

/* packs the given codebook into the bitstream **************************/

int BURGERCALL vorbis_staticbook_pack(const static_codebook *c,oggpack_buffer *opb)
{
	long i,j;
	int ordered=0;

	/* first the basic parameters */
	oggpack_write(opb,0x564342,24);
	oggpack_write(opb,c->dim,16);
	oggpack_write(opb,c->entries,24);

	/* pack the codewords.  There are two packings; length ordered and
	length random.  Decide between the two now. */

	for(i=1;i<c->entries;i++) {
		if(c->lengthlist[i-1]==0 || c->lengthlist[i]<c->lengthlist[i-1]) {
			break;
		}
	}
	if(i==c->entries) {
		ordered=1;
	}
	if(ordered){
		/* length ordered.  We only need to say how many codewords of
		each length.  The actual codewords are generated
		deterministically */

		long count=0;
		oggpack_write(opb,1,1);  /* ordered */
		oggpack_write(opb,c->lengthlist[0]-1,5); /* 1 to 32 */

		for(i=1;i<c->entries;i++){
			long vorthis=c->lengthlist[i];
			long last=c->lengthlist[i-1];
			if(vorthis>last){
				for(j=last;j<vorthis;j++){
					oggpack_write(opb,i-count,_ilog(c->entries-count));
					count=i;
				}
			}
		}
		oggpack_write(opb,i-count,_ilog(c->entries-count));

	}else{
		/* length random.  Again, we don't code the codeword itself, just
		the length.  This time, though, we have to encode each length */
		oggpack_write(opb,0,1);   /* unordered */

		/* algortihmic mapping has use for 'unused entries', which we tag
		here.  The algorithmic mapping happens as usual, but the unused
		entry has no codeword. */
		for(i=0;i<c->entries;i++)
			if(c->lengthlist[i]==0)break;

		if(i==c->entries){
			oggpack_write(opb,0,1); /* no unused entries */
			for(i=0;i<c->entries;i++)
				oggpack_write(opb,c->lengthlist[i]-1,5);
		}else{
			oggpack_write(opb,1,1); /* we have unused entries; thus we tag */
			for(i=0;i<c->entries;i++){
				if(c->lengthlist[i]==0){
					oggpack_write(opb,0,1);
				}else{
					oggpack_write(opb,1,1);
					oggpack_write(opb,c->lengthlist[i]-1,5);
				}
			}
		}
	}

	/* is the entry number the desired return value, or do we have a
	mapping? If we have a mapping, what type? */
	oggpack_write(opb,c->maptype,4);
	switch(c->maptype){
	case 0:
		/* no mapping */
		break;
	case 1:case 2:
		/* implicitly populated value mapping */
		/* explicitly populated value mapping */

		if(!c->quantlist){
			/* no quantlist?  error */
			return(-1);
		}

		/* values that define the dequantization */
		oggpack_write(opb,c->q_min,32);
		oggpack_write(opb,c->q_delta,32);
		oggpack_write(opb,c->q_quant-1,4);
		oggpack_write(opb,c->q_sequencep,1);

		{
			int quantvals;
			switch(c->maptype){
			case 1:
				/* a single column of (c->entries/c->dim) quantized values for
				building a full value list algorithmically (square lattice) */
				quantvals=_book_maptype1_quantvals(c);
				break;
			case 2:
				/* every value (c->entries*c->dim total) specified explicitly */
				quantvals=c->entries*c->dim;
				break;
			default: /* NOT_REACHABLE */
				quantvals=-1;
			}

			/* quantized values */
			for(i=0;i<quantvals;i++)
				oggpack_write(opb,labs(c->quantlist[i]),c->q_quant);
		}
		break;
	default:
		/* error case; we don't have any other map types now */
		return(-1);
	}
	return(0);
}

/* unpacks a codebook from the packet buffer into the codebook struct,
   readies the codebook auxiliary structures for decode *************/
int BURGERCALL vorbis_staticbook_unpack(oggpack_buffer *opb,static_codebook *s)
{
	long i,j;
	FastMemSet(s,0,sizeof(*s));
	s->allocedp=1;

	/* make sure alignment is correct */
	if(oggpack_read(opb,24)!=0x564342)goto _eofout;

	/* first the basic parameters */
	s->dim=oggpack_read(opb,16);
	s->entries=oggpack_read(opb,24);
	if(s->entries==-1)goto _eofout;

	/* codeword ordering.... length ordered or unordered? */
	switch((int)oggpack_read1(opb)){
	case 0:
		/* unordered */
		s->lengthlist=static_cast<long *>(AllocAPointer(sizeof(*s->lengthlist)*s->entries));

		/* allocated but unused entries? */
		if(oggpack_read1(opb)){
			/* yes, unused entries */

			for(i=0;i<s->entries;i++){
				if(oggpack_read1(opb)){
					long num=oggpack_read(opb,5);
					if(num==-1) {
						goto _eofout;
					}
					s->lengthlist[i]=num+1;
				} else {
					s->lengthlist[i]=0;
				}
			}
		} else {
			/* all entries used; no tagging */
			for(i=0;i<s->entries;i++){
				long num=oggpack_read(opb,5);
				if(num==-1)goto _eofout;
				s->lengthlist[i]=num+1;
			}
		}
		break;
	case 1:
		/* ordered */
		{
			long length=oggpack_read(opb,5)+1;
			s->lengthlist=static_cast<long *>(AllocAPointer(sizeof(*s->lengthlist)*s->entries));

			for(i=0;i<s->entries;){
				long num=oggpack_read(opb,_ilog(s->entries-i));
				if(num==-1)goto _eofout;
				for(j=0;j<num && i<s->entries;j++,i++)
					s->lengthlist[i]=length;
				length++;
			}
		}
		break;
	default:
		/* EOF */
		return(-1);
	}

	/* Do we have a mapping to unpack? */
	switch((s->maptype=oggpack_read(opb,4))){
	case 0:
		/* no mapping */
		break;
	case 1: case 2:
		/* implicitly populated value mapping */
		/* explicitly populated value mapping */

		s->q_min=oggpack_read(opb,32);
		s->q_delta=oggpack_read(opb,32);
		s->q_quant=oggpack_read(opb,4)+1;
		s->q_sequencep=oggpack_read1(opb);

		{
			int quantvals=0;
			switch(s->maptype){
			case 1:
				quantvals=_book_maptype1_quantvals(s);
				break;
			case 2:
				quantvals=s->entries*s->dim;
				break;
			}

			/* quantized values */
			s->quantlist=static_cast<long *>(AllocAPointer(sizeof(*s->quantlist)*quantvals));
			for(i=0;i<quantvals;i++)
				s->quantlist[i]=oggpack_read(opb,s->q_quant);

			if(quantvals&&s->quantlist[quantvals-1]==-1)goto _eofout;
		}
		break;
	default:
		goto _errout;
	}

	/* all set */
	return(0);

_errout:
_eofout:
	vorbis_staticbook_clear(s);
	return(-1); 
}

/* returns the number of bits ************************************************/
int BURGERCALL vorbis_book_encode(codebook *book, int a, oggpack_buffer *b)
{
	oggpack_write(b,book->codelist[a],book->c->lengthlist[a]);
	return(book->c->lengthlist[a]);
}

/* One the encode side, our vector writers are each designed for a
specific purpose, and the encoder is not flexible without modification:

The LSP vector coder uses a single stage nearest-match with no
interleave, so no step and no error return.  This is specced by floor0
and doesn't change.

Residue0 encoding interleaves, uses multiple stages, and each stage
peels of a specific amount of resolution from a lattice (thus we want
to match by threshold, not nearest match).  Residue doesn't *have* to
be encoded that way, but to change it, one will need to add more
infrastructure on the encode side (decode side is specced and simpler) */

/* floor0 LSP (single stage, non interleaved, nearest match) */
/* returns entry number and *modifies a* to the quantization value *****/
int BURGERCALL vorbis_book_errorv(codebook *book,float *a)
{
	int dim=book->dim,k;
	int best=_best(book,a,1);
	for(k=0;k<dim;k++)
		a[k]=(book->valuelist+best*dim)[k];
	return(best);
}

/* returns the number of bits and *modifies a* to the quantization value *****/
int BURGERCALL vorbis_book_encodev(codebook *book,int best,float *a,oggpack_buffer *b)
{
	int k,dim=book->dim;
	for(k=0;k<dim;k++)
		a[k]=(book->valuelist+best*dim)[k];
	return(vorbis_book_encode(book,best,b));
}

/* res0 (multistage, interleave, lattice) */
/* returns the number of bits and *modifies a* to the remainder value ********/
int BURGERCALL vorbis_book_encodevs(codebook *book,float *a,oggpack_buffer *b,int step,int addmul)
{
	int best=vorbis_book_besterror(book,a,step,addmul);
	return(vorbis_book_encode(book,best,b));
}

/* Decode side is specced and easier, because we don't need to find
   matches using different criteria; we simply read and map.  There are
   two things we need to do 'depending':
   
   We may need to support interleave.  We don't really, but it's
   convenient to do it here rather than rebuild the vector later.

   Cascades may be additive or multiplicitive; this is not inherent in
   the codebook, but set in the code using the codebook.  Like
   interleaving, it's easiest to do it here.  
   addmul==0 -> declarative (set the value)
   addmul==1 -> additive
   addmul==2 -> multiplicitive */

/* returns the entry number or -1 on eof *************************************/
long BURGERCALL vorbis_book_decode(codebook *book, oggpack_buffer *b)
{
	long ptr=0;
	decode_aux *t=book->decode_tree;
	int lok = oggpack_look(b, t->tabn);

	if (lok >= 0) {
		ptr = t->tab[lok];
		oggpack_adv(b, t->tabl[lok]);
		if (ptr <= 0) {
			return -ptr;
		}
	}

	do {
		switch((int)oggpack_read1(b)){
		case 0:
			ptr=t->ptr0[ptr];
			break;
		case 1:
			ptr=t->ptr1[ptr];
			break;
		case -1:
			return(-1);
		}
	} while(ptr>0);
	return(-ptr);
}

/* returns 0 on OK or -1 on eof *************************************/
long BURGERCALL vorbis_book_decodevs_add(codebook *book,float *a,oggpack_buffer *b,int n)
{
	int i,j,o;
	int step;
	long *entry;
	float **t;
	
	step=n/book->dim;
	if (step) {
		entry = (long *)AllocAPointer((sizeof(*entry)*step)+(sizeof(*t)*step));
		if (entry) {
			t = (float **)((Word8 *)entry+(sizeof(*entry)*step));

			for (i = 0; i < step; i++) {
				entry[i]=vorbis_book_decode(book,b);
				if(entry[i]==-1) {
					DeallocAPointer(entry);
					return(-1);
				}
				t[i] = book->valuelist+entry[i]*book->dim;
			}
			for(i=0,o=0;i<book->dim;i++,o+=step) {
				for (j=0;j<step;j++) {
					a[o+j]+=t[j][i];
				}
			}
			DeallocAPointer(entry);
		}
	}
	return(0);
}

long BURGERCALL vorbis_book_decodev_add(codebook *book,float *a,oggpack_buffer *b,int n){
	int i,j,entry;
	float *t;

	if(book->dim>8){
		for(i=0;i<n;){
			entry = vorbis_book_decode(book,b);
			if(entry==-1)return(-1);
			t     = book->valuelist+entry*book->dim;
			for (j=0;j<book->dim;)
				a[i++]+=t[j++];
		}
	}else{
		for(i=0;i<n;){
			entry = vorbis_book_decode(book,b);
			if(entry==-1)return(-1);
			t     = book->valuelist+entry*book->dim;
			j=0;
			switch((int)book->dim){
			case 8:
				a[i++]+=t[j++];
			case 7:
				a[i++]+=t[j++];
			case 6:
				a[i++]+=t[j++];
			case 5:
				a[i++]+=t[j++];
			case 4:
				a[i++]+=t[j++];
			case 3:
				a[i++]+=t[j++];
			case 2:
				a[i++]+=t[j++];
			case 1:
				a[i++]+=t[j++];
			case 0:
				break;
			}
		}
	}    
	return(0);
}

long BURGERCALL vorbis_book_decodev_set(codebook *book,float *a,oggpack_buffer *b,int n)
{
	int i,j,entry;
	float *t;

	for(i=0;i<n;){
		entry = vorbis_book_decode(book,b);
		if(entry==-1)return(-1);
		t = book->valuelist+entry*book->dim;
		for (j=0;j<book->dim;)
			a[i++]=t[j++];
	}
	return(0);
}

long BURGERCALL vorbis_book_decodevv_add(codebook *book,float **a,long offset,int ch,oggpack_buffer *b,int n)
{
	long i,j,entry;
	int chptr=0;

	for(i=offset/ch;i<(offset+n)/ch;){
		entry = vorbis_book_decode(book,b);
		if(entry==-1) {
			return(-1);
		}
		{
			const float *t = book->valuelist+entry*book->dim;
			for (j=0;j<book->dim;j++){
				a[chptr++][i]+=t[j];
				if(chptr==ch){
					chptr=0;
					i++;
				}
			}
		}
	}
	return(0);
}
