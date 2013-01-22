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

 function: basic shared codebook operations
 last mod: $Id: PkVorbisSharedBook.c,v 1.4 2003/12/01 16:12:32 burger Exp $

 ********************************************************************/

#include "PkVorbisCodebook.h"
#include "MmMemory.h"
#include "ClStdlib.h"
#include "FpFloat.h"
#include <math.h>

/**** pack/unpack helpers ******************************************/

int BURGERCALL _ilog(Word v)
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

/* 32 bit float (not IEEE; nonnormalized mantissa +
   biased exponent) : neeeeeee eeemmmmm mmmmmmmm mmmmmmmm 
   Why not IEEE?  It's just not that important here. */

#define VQ_FEXP 10
#define VQ_FMAN 21
#define VQ_FEXP_BIAS 768 /* bias toward values smaller than 1. */

/* doesn't currently guard under/overflow */
long BURGERCALL _float32_pack(float val)
{
	int sign=0;
	long exp;
	long mant;
	if (val<0){
		sign=0x80000000;
		val= -val;
	}
	exp= (long)floor(log(val)/log(2.f));
	mant=(long)floor(ldexp(val,(VQ_FMAN-1)-exp)+0.5f);
	exp=(exp+VQ_FEXP_BIAS)<<VQ_FMAN;
	return(sign|exp|mant);
}

float BURGERCALL _float32_unpack(long val)
{
	double mant=val&0x1fffff;
	int sign=val&0x80000000;
	long exp =(val&0x7fe00000L)>>VQ_FMAN;
	if(sign) {
		mant= -mant;
	}
	return (float)(ldexp(mant,exp-(VQ_FMAN-1)-VQ_FEXP_BIAS));
}

/* given a list of word lengths, generate a list of codewords.  Works
   for length ordered or unordered, always assigns the lowest valued
   codewords first.  Extended to handle unused entries (length 0) */
static long * BURGERCALL _make_words(long *l,long n)
{
	long i,j;
	long marker[33];
	long *r=static_cast<long *>(AllocAPointer(n*sizeof(*r)));
	if (r) {
		FastMemSet(marker,0,sizeof(marker));

		for(i=0;i<n;i++){
			long length=l[i];
			if(length>0){
				long entry=marker[length];

				/* when we claim a node for an entry, we also claim the nodes
				below it (pruning off the imagined tree that may have dangled
				from it) as well as blocking the use of any nodes directly
				above for leaves */

				/* update ourself */
				if(length<32 && (entry>>length)){
					/* error condition; the lengths must specify an overpopulated tree */
					DeallocAPointer(r);
					return(NULL);
				}
				r[i]=entry;

				/* Look to see if the next shorter marker points to the node
				above. if so, update it and repeat.  */
				{
					for(j=length;j>0;j--){

						if(marker[j]&1){
						/* have to jump branches */
							if(j==1)
								marker[1]++;
							else
								marker[j]=marker[j-1]<<1;
							break; /* invariant says next upper marker would already
							have been moved if it was on the same path */
						}
						marker[j]++;
					}
				}

				/* prune the tree; the implicit invariant says all the longer
				markers were dangling from our just-taken node.  Dangle them
				from our *new* node. */
				for(j=length+1;j<33;j++) {
					if((marker[j]>>1) == entry){
						entry=marker[j];
						marker[j]=marker[j-1]<<1;
					} else {
						break;
					}
				}    
			}
		}
			/* bitreverse the words because our bitwise packer/unpacker is LSb endian */
		for(i=0;i<n;i++){
			long temp=0;
			for(j=0;j<l[i];j++){
				temp<<=1;
				temp|=(r[i]>>j)&1;
			}
			r[i]=temp;
		}
	}
	return(r);
}

/* build the decode helper tree from the codewords */

static decode_aux * BURGERCALL _make_decode_tree(codebook *c)
{
	const static_codebook *s=c->c;
	long top=0,i,j,n;
	long *ptr0;
	long *ptr1;
	decode_aux *t=static_cast<decode_aux *>(AllocAPointer(sizeof(*t)));
	if (t) {
		t->ptr0=ptr0=static_cast<long *>(AllocAPointerClear(c->entries*2*sizeof(*ptr0)));
		if (ptr0) {
			t->ptr1=ptr1=static_cast<long *>(AllocAPointerClear(c->entries*2*sizeof(*ptr1)));
			if (ptr1) {
				long *codelist=_make_words(s->lengthlist,s->entries);
				if (codelist) {
					t->aux=c->entries*2;

					for(i=0;i<c->entries;i++){
						if(s->lengthlist[i]>0){
							long ptr=0;
							for(j=0;j<s->lengthlist[i]-1;j++){
								int bit=(codelist[i]>>j)&1;
								if(!bit){
									if(!ptr0[ptr]) {
										ptr0[ptr]= ++top;
									}
									ptr=ptr0[ptr];
								}else{
									if(!ptr1[ptr]) {
										ptr1[ptr]= ++top;
									}
									ptr=ptr1[ptr];
								}
							}
							if(!((codelist[i]>>j)&1)) {
								ptr0[ptr]=-i;
							} else {
								ptr1[ptr]=-i;
							}
						}
					}
					DeallocAPointer(codelist);

					t->tabn = _ilog(c->entries)-4; /* this is magic */
					if(t->tabn<5) {
						t->tabn=5;
					}
					n = 1<<t->tabn;
					t->tab = static_cast<long *>(AllocAPointer(n*sizeof(*t->tab)));
					t->tabl = static_cast<int *>(AllocAPointer(n*sizeof(*t->tabl)));
					if (t->tab && t->tabl) {
						for (i = 0; i < n; i++) {
							long p = 0;
							for (j = 0; j < t->tabn && (p > 0 || j == 0); j++) {
								if (i & (1 << j)) {
									p = ptr1[p];
								} else {
									p = ptr0[p];
								}
							}
							/* now j == length, and p == -code */
							t->tab[i] = p;
							t->tabl[i] = j;
						}
						return(t);
					}
					DeallocAPointer(t->tab);
					DeallocAPointer(t->tabl);
				}
				DeallocAPointer(ptr1);
			}
			DeallocAPointer(ptr0);
		}
		DeallocAPointer(t);
	}
	return 0;
}

/* there might be a straightforward one-line way to do the below
   that's portable and totally safe against roundoff, but I haven't
   thought of it.  Therefore, we opt on the side of caution */

long BURGERCALL _book_maptype1_quantvals(const static_codebook *b)
{
	long vals=(long)floor(pow((float)b->entries,1.f/b->dim));

	/* the above *should* be reliable, but we'll not assume that FP is
	ever reliable when bitstream sync is at stake; verify via integer
	means that vals really is the greatest value of dim for which
	vals^b->bim <= b->entries */
	/* treat the above as an initial guess */
	while(1){
		long acc=1;
		long acc1=1;
		int i;
		for(i=0;i<b->dim;i++){
			acc*=vals;
			acc1*=vals+1;
		}
		if (acc<=b->entries && acc1>b->entries){
			return(vals);
		}
		if(acc>b->entries){
			vals--;
		} else {
			vals++;
		}
	}
}

/* unpack the quantized list of values for encode/decode ***********/
/* we need to deal with two map types: in map type 1, the values are
   generated algorithmically (each column of the vector counts through
   the values in the quant vector). in map type 2, all the values came
   in in an explicit list.  Both value lists must be unpacked */
float *BURGERCALL _book_unquantize(const static_codebook *b)
{
	long j,k;
	if(b->maptype==1 || b->maptype==2){
		int quantvals;
		float mindel=_float32_unpack(b->q_min);
		float delta=_float32_unpack(b->q_delta);
		float *r=static_cast<float *>(AllocAPointerClear(b->entries*b->dim*sizeof(*r)));

		if (r) {			/* Got the memory? */
		
			/* maptype 1 and 2 both use a quantized value vector, but
			different sizes */
			switch(b->maptype){
			case 1:
				/* most of the time, entries%dimensions == 0, but we need to be
				well defined.  We define that the possible vales at each
				scalar is values == entries/dim.  If entries%dim != 0, we'll
				have 'too few' values (values*dim<entries), which means that
				we'll have 'left over' entries; left over entries use zeroed
				values (and are wasted).  So don't generate codebooks like
				that */
				quantvals=_book_maptype1_quantvals(b);
				for(j=0;j<b->entries;j++){
					float last=0.f;
					int indexdiv=1;
					for(k=0;k<b->dim;k++){
						int index= (j/indexdiv)%quantvals;
						float val=(float)b->quantlist[index];
						val=FloatAbs(val)*delta+mindel+last;
						if(b->q_sequencep) {
							last=val;
						}
						r[j*b->dim+k]=val;
						indexdiv*=quantvals;
					}
				}
				break;
			case 2:
				for(j=0;j<b->entries;j++){
					float last=0.f;
					for(k=0;k<b->dim;k++){
						float val=(float)b->quantlist[j*b->dim+k];
						val=FloatAbs(val)*delta+mindel+last;
						if(b->q_sequencep) {
							last=val;	  
						}
						r[j*b->dim+k]=val;
					}
				}
				break;
			}
			return(r);
		}
	}
	return(NULL);
}

void BURGERCALL vorbis_staticbook_clear(static_codebook *b)
{
	if (b->allocedp){
		DeallocAPointer(b->quantlist);
		DeallocAPointer(b->lengthlist);
		if(b->nearest_tree){
			DeallocAPointer(b->nearest_tree->ptr0);
			DeallocAPointer(b->nearest_tree->ptr1);
			DeallocAPointer(b->nearest_tree->p);
			DeallocAPointer(b->nearest_tree->q);
			FastMemSet(b->nearest_tree,0,sizeof(*b->nearest_tree));
			DeallocAPointer(b->nearest_tree);
		}
		if(b->thresh_tree){
			DeallocAPointer(b->thresh_tree->quantthresh);
			DeallocAPointer(b->thresh_tree->quantmap);
			FastMemSet(b->thresh_tree,0,sizeof(*b->thresh_tree));
			DeallocAPointer(b->thresh_tree);
		}
		FastMemSet(b,0,sizeof(*b));
	}
}

void BURGERCALL vorbis_staticbook_destroy(static_codebook *b)
{
	if(b->allocedp){
		vorbis_staticbook_clear(b);
		DeallocAPointer(b);
	}
}

void BURGERCALL vorbis_book_clear(codebook *b)
{
	/* static book is not cleared; we're likely called on the lookup and
	the static codebook belongs to the info struct */
	if(b->decode_tree){
		DeallocAPointer(b->decode_tree->tab);
		DeallocAPointer(b->decode_tree->tabl);
		DeallocAPointer(b->decode_tree->ptr0);
		DeallocAPointer(b->decode_tree->ptr1);
		FastMemSet(b->decode_tree,0,sizeof(*b->decode_tree));
		DeallocAPointer(b->decode_tree);
	}
	DeallocAPointer(b->valuelist);
	DeallocAPointer(b->codelist);
	FastMemSet(b,0,sizeof(*b));
}

int BURGERCALL vorbis_book_init_encode(codebook *c,const static_codebook *s)
{
	long j,k;
	FastMemSet(c,0,sizeof(*c));
	c->c=s;
	c->entries=s->entries;
	c->dim=s->dim;
	c->codelist=_make_words(s->lengthlist,s->entries);
	c->valuelist=_book_unquantize(s);

	/* set the 'zero entry' */
	c->zeroentry=-1;
	if(c->valuelist){
		for(j=0;j<s->entries;j++){
			int flag=1;
			for(k=0;k<s->dim;k++){
				if(FloatAbs(c->valuelist[j*s->dim+k])>1e-12f){
					flag=0;
					break;
				}
			}
			if(flag) {
				c->zeroentry=j;
			}
		}
	}
	return(0);
}

int BURGERCALL vorbis_book_init_decode(codebook *c,const static_codebook *s)
{
	FastMemSet(c,0,sizeof(*c));
	c->c=s;
	c->entries=s->entries;
	c->dim=s->dim;
	c->valuelist=_book_unquantize(s);
	c->decode_tree=_make_decode_tree(c);
	if (c->decode_tree) {
		return(0);
	}
	vorbis_book_clear(c);
	return(-1);
}

static float BURGERCALL _dist(int el,const float *ref,const float *b,int step)
{
	int i;
	float acc=0.f;
	for(i=0;i<el;i++){
		float val=(ref[i]-b[i*step]);
		acc+=val*val;
	}
	return(acc);
}

int BURGERCALL _best(codebook *book, float *a, int step)
{
	encode_aux_nearestmatch *nt=book->c->nearest_tree;
	encode_aux_threshmatch *tt=book->c->thresh_tree;
	encode_aux_pigeonhole *pt=book->c->pigeon_tree;
	int dim=book->dim;
	int ptr=0,k,o;
	/*int savebest=-1;
	float saverr;*/

	/* do we have a threshhold encode hint? */
	if(tt){
		int index=0;
		/* find the quant val of each scalar */
		for(k=0,o=step*(dim-1);k<dim;k++,o-=step){
			int i;
			/* linear search the quant list for now; it's small and although
			with > ~8 entries, it would be faster to bisect, this would be
			a misplaced optimization for now */
			for(i=0;i<tt->threshvals-1;i++)
				if(a[o]<tt->quantthresh[i])break;

				index=(index*tt->quantvals)+tt->quantmap[i];
		}
		/* regular lattices are easy :-) */
		if(book->c->lengthlist[index]>0) /* is this unused?  If so, we'll
			use a decision tree after all
			and fall through*/
			return(index);
	}

	/* do we have a pigeonhole encode hint? */
	if(pt){
		const static_codebook *c=book->c;
		int i,besti=-1;
		float best=0.f;
		int entry=0;

		/* dealing with sequentialness is a pain in the ass */
		if(c->q_sequencep){
			int pv;
			long mul=1;
			float qlast=0;
			for(k=0,o=0;k<dim;k++,o+=step){
				pv=(int)((a[o]-qlast-pt->min)/pt->del);
				if(pv<0 || pv>=pt->mapentries)break;
				entry+=pt->pigeonmap[pv]*mul;
				mul*=pt->quantvals;
				qlast+=pv*pt->del+pt->min;
			}
		} else{
			for(k=0,o=step*(dim-1);k<dim;k++,o-=step){
				int pv=(int)((a[o]-pt->min)/pt->del);
				if(pv<0 || pv>=pt->mapentries)break;
				entry=entry*pt->quantvals+pt->pigeonmap[pv];
			}
		}

		/* must be within the pigeonholable range; if we quant outside (or
		in an entry that we define no list for), brute force it */
		if(k==dim && pt->fitlength[entry]){
			/* search the abbreviated list */
			long *list=pt->fitlist+pt->fitmap[entry];
			for(i=0;i<pt->fitlength[entry];i++){
				float vorthis=_dist(dim,book->valuelist+list[i]*dim,a,step);
				if(besti==-1 || vorthis<best){
					best=vorthis;
					besti=list[i];
				}
			}

			return(besti); 
		}
	}

	if(nt){
		/* optimized using the decision tree */
		while(1){
			float c=0.f;
			float *p=book->valuelist+nt->p[ptr];
			float *q=book->valuelist+nt->q[ptr];

			for(k=0,o=0;k<dim;k++,o+=step)
				c+=(p[k]-q[k])*(a[o]-(p[k]+q[k])*.5f);

			if(c>0.f) /* in A */
				ptr= -nt->ptr0[ptr];
			else     /* in B */
				ptr= -nt->ptr1[ptr];
			if(ptr<=0)break;
		}
		return(-ptr);
	}

	/* brute force it! */
	{
		const static_codebook *c=book->c;
		int i,besti=-1;
		float best=0.f;
		float *e=book->valuelist;
		for(i=0;i<book->entries;i++){
			if(c->lengthlist[i]>0){
				float vorthis=_dist(dim,e,a,step);
				if(besti==-1 || vorthis<best){
					best=vorthis;
					besti=i;
				}
			}
			e+=dim;
		}

		/*if(savebest!=-1 && savebest!=besti) {
			fprintf(stderr,"brute force/pigeonhole disagreement:\n"
				"original:");
			for(i=0;i<dim*step;i+=step) {
				fprintf(stderr,"%g,",a[i]);
			}
			fprintf(stderr,"\n"
				"pigeonhole (entry %d, err %g):",savebest,saverr);
			for(i=0;i<dim;i++) {
				fprintf(stderr,"%g,",
				(book->valuelist+savebest*dim)[i]);
			}
			fprintf(stderr,"\n"
				"bruteforce (entry %d, err %g):",besti,best);
			for(i=0;i<dim;i++) {
				fprintf(stderr,"%g,",(book->valuelist+besti*dim)[i]);
			}
			fprintf(stderr,"\n");
		}
		*/
		return(besti);
	}
}

/* returns the entry number and *modifies a* to the remainder value ********/
int BURGERCALL vorbis_book_besterror(codebook *book,float *a,int step,int addmul)
{
	int dim=book->dim,i,o;
	int best=_best(book,a,step);
	switch(addmul){
	case 0:
		for(i=0,o=0;i<dim;i++,o+=step) {
			a[o]-=(book->valuelist+best*dim)[i];
		}
		break;
	case 1:
		for(i=0,o=0;i<dim;i++,o+=step) {
			float val=(book->valuelist+best*dim)[i];
			if(val==0){
				a[o]=0;
			}else{
				a[o]/=val;
			}
		}
		break;
	}
	return(best);
}

long BURGERCALL vorbis_book_codeword(codebook *book,int entry)
{
	return book->codelist[entry];
}

long BURGERCALL vorbis_book_codelen(codebook *book,int entry)
{
	return book->c->lengthlist[entry];
}
