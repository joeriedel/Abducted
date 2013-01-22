/*
 * jdct.h
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This include file contains common declarations for the forward and
 * inverse DCT modules.  These declarations are private to the DCT managers
 * (jcdctmgr.c, jddctmgr.c) and the individual DCT algorithms.
 * The individual DCT algorithms are kept in separate files to ease 
 * machine-dependent tuning (e.g., assembly coding).
 */

#ifndef __JDCT_H__
#define __JDCT_H__

#ifndef __BURGER__
#include <Burger.h>
#endif

/*
 * A forward DCT routine is given a pointer to a work area of type SWord32[];
 * the DCT is to be performed in-place in that buffer.  Type SWord32 is int
 * for 8-bit samples, long for 12-bit samples.  (NOTE: Floating-point DCT
 * implementations use an array of type float, instead.)
 * The DCT inputs are expected to be signed (range +-CENTERJSAMPLE).
 * The DCT outputs are returned scaled up by a factor of 8; they therefore
 * have a range of +-8K for 8-bit data, +-128K for 12-bit data.  This
 * convention improves accuracy in integer implementations and saves some
 * work in floating-point ones.
 * Quantization of the output coefficients is done by jcdctmgr.c.
 */

typedef void (BURGERCALL *forward_DCT_method_ptr)(SWord32 * data);
typedef void (BURGERCALL *float_DCT_method_ptr)(float * data);


/*
 * An inverse DCT routine is given a pointer to the input JPeg70Block_t and a pointer
 * to an output sample array.  The routine must dequantize the input data as
 * well as perform the IDCT; for dequantization, it uses the multiplier table
 * pointed to by compptr->dct_table.  The output data is to be placed into the
 * sample array starting at a specified column.  (Any row offset needed will
 * be applied to the array pointer before it is passed to the IDCT code.)
 * Note that the number of samples emitted by the IDCT routine is
 * DCT_scaled_size * DCT_scaled_size.
 */

/* typedef inverse_DCT_method_ptr is declared in jpegint.h */

/*
 * Each IDCT routine has its own ideas about the best dct_table element type.
 */

#if BITS_IN_JSAMPLE == 8
#define IFAST_SCALE_BITS  2	/* fractional bits in scale factors */
#else
#define IFAST_SCALE_BITS  13	/* fractional bits in scale factors */
#endif

/*
 * Macros for handling fixed-point arithmetic; these are used by many
 * but not all of the DCT/IDCT modules.
 *
 * All values are expected to be of type long.
 * Fractional constants are scaled left by CONST_BITS bits.
 * CONST_BITS is defined within each module using these macros,
 * and may differ from one module to the next.
 */

#define CONST_SCALE (1L << CONST_BITS)

/* Convert a positive real constant to an integer scaled by CONST_SCALE.
 * Caution: some C compilers fail to reduce "FIX(constant)" at compile time,
 * thus causing a lot of useless floating-point operations at run time.
 */

#define FIX(x)	((long) ((x) * CONST_SCALE + 0.5f))

/* Descale and correctly round an long value that's scaled by N bits.
 * We assume RIGHT_SHIFT rounds towards minus infinity, so adding
 * the fudge factor is correct for either sign of X.
 */

#define DESCALE(x,n)  (((x) + (1 << ((n)-1)))>>n)

#endif
