/*
 * jmemsys.h
 *
 * Copyright (C) 1992-1997, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This include file defines the interface between the system-independent
 * and system-dependent portions of the JPEG memory manager.  No other
 * modules need include it.  (The system-independent portion is jmemmgr.c;
 * there are several different versions of the system-dependent portion.)
 *
 * This file works as-is for the system-dependent memory managers supplied
 * in the IJG distribution.  You may need to modify it if you write a
 * custom memory manager.
 */

#ifndef __JMEMSYS_H__
#define __JMEMSYS_H__

#ifndef __BURGER__
#include <Burger.h>
#endif

/*
 * The macro MAX_ALLOC_CHUNK designates the maximum number of bytes that may
 * be requested in a single call to jpeg_get_large (and jpeg_get_small for that
 * matter, but that case should never come into play).  This macro is needed
 * to model the 64Kb-segment-size limit of far addressing on 80x86 machines.
 * On those machines, we expect that jconfig.h will provide a proper value.
 * On machines with 32-bit flat address spaces, any large constant may be used.
 *
 * NB: jmemmgr.c expects that MAX_ALLOC_CHUNK will be representable as type
 * long and will be a multiple of sizeof(align_type).
 */

#ifndef MAX_ALLOC_CHUNK		/* may be overridden in jconfig.h */
#define MAX_ALLOC_CHUNK  1000000000L
#endif

typedef struct backing_store_struct * backing_store_ptr;

typedef struct backing_store_struct {
	/* Methods for reading/writing/closing this backing-store object */
	void(BURGERCALL *read_backing_store)(JPeg70::CCommonManager * cinfo,backing_store_ptr info,void * buffer_address,long file_offset, long byte_count);
	void(BURGERCALL *write_backing_store)(JPeg70::CCommonManager * cinfo,backing_store_ptr info,void * buffer_address,long file_offset, long byte_count);
} backing_store_info;

#endif
