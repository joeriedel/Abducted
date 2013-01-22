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

 function: window functions
 last mod: $Id: PkVorbisWindow.c,v 1.5 2003/12/01 16:12:32 burger Exp $

 ********************************************************************/

#include "PkVorbisWindow.h"
#include "MmMemory.h"
#include <math.h>

float *BURGERCALL _vorbis_window(int type, int window,int left,int right)
{
	if (!type) {
		float *ret=static_cast<float *>(AllocAPointerClear(window*sizeof(*ret)));
		if (ret) {
		/* The 'vorbis window' (window 0) is sin(sin(x)*sin(x)*2pi) */
			int leftbegin=window/4-left/2;
			int rightbegin=window-window/4-right/2;
			int i;
			float x;
			float ftemp;
			float fi;
			float *Output;
			
			Output = ret+leftbegin;
			ftemp = (1.0f/(float)left)*(PK_PI/2.0f);
			fi = .5f;
			for(i=0;i<left;i++){
				x=fi*ftemp;
				x=(float)sin(x);
				x*=x;
				x*=(PK_PI/2.f);
				Output[0]=(float)sin(x);
				++Output;
				fi+=1.0f;
			}

			for(i=leftbegin+left;i<rightbegin;i++) {
				Output[0]=1.0f;
				++Output;
			}

			ftemp = (float)right;
			fi = ftemp-.5f;
			ftemp = (1.0f/ftemp)*(PK_PI/2.0f);
			for(i=0;i<right;i++){
				x=fi*ftemp;
				x=(float)sin(x);
				x*=x;
				x*=(PK_PI/2.f);
				Output[0]=(float)sin(x);
				++Output;
				fi -= 1.0f;
			}
			return ret;
		}
	}
	return 0;
}

