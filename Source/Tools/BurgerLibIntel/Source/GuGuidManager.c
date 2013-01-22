/**********************************

	Create and work with Win95 style GUID structures
	
**********************************/

/**********************************

	(c) Copyright 1989 OPEN SOFTWARE FOUNDATION, INC.
	(c) Copyright 1989 HEWLETT-PACKARD COMPANY
	(c) Copyright 1989 DIGITAL EQUIPMENT CORPORATION
	To anyone who acknowledges that this file is provided "AS IS"
	without any express or implied warranty:
	                permission to use, copy, modify, and distribute this
	file for any purpose is hereby granted without fee, provided that
	the above copyright notices and this notice appears in all source
	code copies, and that none of the names of Open Software
	Foundation, Inc., Hewlett-Packard Company, or Digital Equipment
	Corporation be used in advertising or publicity pertaining to
	distribution of the software without specific, written prior
	permission.  Neither Open Software Foundation, Inc., Hewlett-
	Packard Company, nor Digital Equipment Corporation makes any
	representations about the suitability of this software for any
	purpose.

	Internal structure of universal unique IDs (UUIDs).
	
	There are three "variants" of UUIDs that this code knows about.  The
	variant #0 is what was defined in the 1989 HP/Apollo Network Computing
	Architecture (NCA) specification and implemented in NCS 1.x and DECrpc
	v1.  Variant #1 is what was defined for the joint HP/DEC specification
	for the OSF (in DEC's "UID Architecture Functional Specification Version
	X1.0.4") and implemented in NCS 2.0, DECrpc v2, and OSF 1.0 DCE RPC.
	Variant #2 is defined by Microsoft.
	
	This code creates only variant #1 UUIDs.
	 
	The three UUID variants can exist on the same wire because they have
	distinct values in the 3 MSB bits of octet 8 (see table below).  Do
	NOT confuse the version number with these 3 bits.  (Note the distinct
	use of the terms "version" and "variant".) Variant #0 had no version
	field in it.  Changes to variant #1 (should any ever need to be made)
	can be accomodated using the current form's 4 bit version field.
	 
	The UUID record structure MUST NOT contain padding between fields.
	The total size = 128 bits.
	
	To minimize confusion about bit assignment within octets, the UUID
	record definition is defined only in terms of fields that are integral
	numbers of octets.
	
	Depending on the network data representation, the multi-octet unsigned
	integer fields are subject to byte swapping when communicated between
	dissimilar endian machines.  Note that all three UUID variants have
	the same record structure; this allows this byte swapping to occur.
	(The ways in which the contents of the fields are generated can and
	do vary.)
	
	The following information applies to variant #1 UUIDs:
	
	The lowest addressed octet contains the global/local bit and the
	unicast/multicast bit, and is the first octet of the address transmitted
	on an 802.3 LAN.
	
	The adjusted time stamp is split into three fields, and the clockSeq
	is split into two fields.
	
	|<------------------------- 32 bits -------------------------->|
	+--------------------------------------------------------------+
	|                     low 32 bits of time                      |  0-3  .time_low
	+-------------------------------+-------------------------------
	|     mid 16 bits of time       |  4-5               .time_mid
	+-------+-----------------------+
	| vers. |   hi 12 bits of time  |  6-7               .time_hi_and_version
	+-------+-------+---------------+
	|Res|  clkSeqHi |  8                                 .clock_seq_hi_and_reserved
	+---------------+
	|   clkSeqLow   |  9                                 .clock_seq_low
	+---------------+----------...-----+
	|            node ID               |  8-16           .node
	+--------------------------...-----+
	
	--------------------------------------------------------------------------
	
	The structure layout of all three UUID variants is fixed for all time.
	I.e., the layout consists of a 32 bit int, 2 16 bit ints, and 8 8
	bit ints.  The current form version field does NOT determine/affect
	the layout.  This enables us to do certain operations safely on the
	variants of UUIDs without regard to variant; this increases the utility
	of this code even as the version number changes (i.e., this code does
	NOT need to check the version field).
	
	The "Res" field in the octet #8 is the so-called "reserved" bit-field
	and determines whether or not the uuid is a old, current or other
	UUID as follows:
	
	     MS-bit  2MS-bit  3MS-bit      Variant
	     ---------------------------------------------
	        0       x        x       0 (NCS 1.5)
	        1       0        x       1 (DCE 1.0 RPC)
	        1       1        0       2 (Microsoft)
	        1       1        1       unspecified
	
	--------------------------------------------------------------------------
	
	Internal structure of variant #0 UUIDs
	
	The first 6 octets are the number of 4 usec units of time that have
	passed since 1/1/80 0000 GMT.  The next 2 octets are reserved for
	future use.  The next octet is an address family.  The next 7 octets
	are a host ID in the form allowed by the specified address family.
	
	Note that while the family field (octet 8) was originally conceived
	of as being able to hold values in the range [0..255], only [0..13]
	were ever used.  Thus, the 2 MSB of this field are always 0 and are
	used to distinguish old and current UUID forms.
	
	+--------------------------------------------------------------+
	|                    high 32 bits of time                      |  0-3  .time_high
	+-------------------------------+-------------------------------
	|     low 16 bits of time       |  4-5               .time_low
	+-------+-----------------------+
	|         reserved              |  6-7               .reserved
	+---------------+---------------+
	|    family     |   8                                .family
	+---------------+----------...-----+
	|            node ID               |  9-16           .node
	+--------------------------...-----+

**********************************/

#include "GuGuid.h"
#include "PfPrefs.h"
#include "ClStdLib.h"

/* local defines used in uuid bit-diddling */

#define RAND_MASK 0x3fff			/* same as CLOCK_SEQ_LAST */

#define TIME_MID_MASK               0x0000ffff
#define TIME_HIGH_MASK              0x0fff0000
#define TIME_HIGH_SHIFT_COUNT       16

/*
 *	The following was modified in order to prevent overlap because
 *	our clock is (theoretically) accurate to 1 Microsecond.
 */

#define MAX_TIME_ADJUST             9			/* Max adjust before tick */

#define CLOCK_SEQ_LOW_MASK          0xff
#define CLOCK_SEQ_HIGH_MASK         0x3f00
#define CLOCK_SEQ_HIGH_SHIFT_COUNT  8
#define CLOCK_SEQ_FIRST             1
#define CLOCK_SEQ_LAST              0x3fff      /* same as RAND_MASK */
#define CLOCK_SEQ_BUMP(seq) ((seq) = ((seq) + 1) & CLOCK_SEQ_LAST)		/* Power of 2 */

#define UUID_C_100NS_PER_USEC           10

#define UUID_VERSION_BITS (1 << 12)
#define UUID_RESERVED_BITS 0x80

static EthernetAddress_t saved_addr;	/* Cache of the MAC address */
static Word got_address;			/* Is the mac address valid? */
static Word last_addr_result;		/* What was the last error code? */
static Word32 rand_m;				/* multiplier */
static Word32 rand_ia;    	    /* adder #1 */
static Word32 rand_ib;			/* adder #2 */
static Word32 rand_irand;			/* random value */
static Word GUIDInitDone;			/* True if initialized */
static LongWord64_t GLastTime;		/* Last time received */
static Word GTimeAdjust;			/* Time adjustment */
static Word16 GClockSeq;				/* Next clock sequence */

GUID GUIDBlank;			/* Empty GUID */

/**********************************

	Return TRUE if the ASCII char is 0-9, A-F or a-f or zero
	
**********************************/

static INLINECALL Word GUIDCharIsHex(Word Input)
{
	if (!Input) {
		goto Good;		/* End of string? */
	}
	if (Input<'0') {		/* 0-9 */
		goto Bad;
	}
	if (Input<='9') {
		goto Good;
	}
	if (Input<'A') {		/* A-F */
		goto Bad;
	}
	if (Input<='F') {
		goto Good;
	}
	if (Input<'a') {		/* a-f */
		goto Bad;
	}
	if (Input<='f') {
Good:;
		return TRUE;	/* Acceptable char */
	}
Bad:;
	return FALSE;		/* No good! */
}

/**********************************

	Parse a hex strip and return the value
	(Before I parse, skip over whitespace)
	
**********************************/

static Word32 GUIDFromHex(const char **input,Word digits)
{
	Word32 Value;
	Word c;
	const char *Input;
	
	Input = input[0];
	while (!GUIDCharIsHex(((Word8 *)Input)[0])) {
		++Input;
	}
	Value = 0;				/* Init the value */
	do {
		c = ((Word8 *)Input)[0];
		if (!c) {			/* End of string??? */
			break;			/* Abort NOW */
		}
		++Input;			/* Accept this char */
		c -= '0';			/* Convert '0'-'9' to 0-9 */
		if (c>=10) {		/* Not in range? */
			c -= ('A'-'0');		/* Convert 'A'-'F' to 0-5 */
			if (c>=6) {
				c -= 'a'-'A';	/* Convert 'a'-'f' to 0-5 */
				if (c>=6) {
					break;		/* Not valid! */
				}
			}
			c+=10;				/* Convert 0-5 to 10-15 */
		}
		Value = (Value << 4) | c;		/* Blend */
	} while (--digits);
	input[0] = Input;				/* Save the string place */
	return Value;
}

/**********************************

	Convert a GUID to a string
	The format is...
	8641FBDE-7F8F-11D4-AAC5-000A27DD93F2
	
**********************************/

void BURGERCALL GUIDToString(char *Output,const GUID *Input)
{
	Word i;
	
	if (!Input) {
		Input = &GUIDBlank;		/* Oh crap... */
	}
	Output = LongWordToAsciiHex2(Input->Data1,Output,ASCIILEADINGZEROS|8);
	Output[0] = '-';
	Output = LongWordToAsciiHex2(Input->Data2,Output+1,ASCIILEADINGZEROS|4);
	Output[0] = '-';
	Output = LongWordToAsciiHex2(Input->Data3,Output+1,ASCIILEADINGZEROS|4);
	Output[0] = '-';
	Output = LongWordToAsciiHex2(Input->Data4[0],Output+1,ASCIILEADINGZEROS|2);
	Output = LongWordToAsciiHex2(Input->Data4[1],Output,ASCIILEADINGZEROS|2);
	Output[0] = '-';
	++Output;
	i = 0;
	do {
		Output = LongWordToAsciiHex2(Input->Data4[i+2],Output,ASCIILEADINGZEROS|2);
	} while (++i<6);
	Output[0] = 0;			/* End the "C" string */
}

/**********************************

	Using the DCE code from the DEC/HP, I will
	create a 16 bit hash value from the GUID
	
**********************************/

Word BURGERCALL GUIDHash(const GUID *Input)
{
	int c0,c1;
	int x,y;
	
	if (!Input) {
		Input = &GUIDBlank;		/* Oh crap */
	}
	c0 = 0;
	c1 = 0;
	
	x = sizeof(GUID);
	do {
		c0 = c0 + ((Word8 *)Input)[0];	/* Simple add of all 16 bytes */
		c1 = c1 + c0;					/* Adjust the hash */
		Input = (const GUID *)(((Word8 *)Input)+1);
	} while (--x);
	
	c1 = (short)c1;
	x = -c1 % 255;			/* Make a modulo to hash it out */
	if (x < 0) {
		x += 255;			/* Make positive */
	}
	x = x&0xFF;
	
	c1 = (short)(c1-c0);	/* The hash is 16 bit! */
	y = c1 % 255;			/* Make modulo */
	if (y < 0) {
		y += 255;			/* Make positive */
	}
	return ((y&0xFF) << 8) | x;		/* Return the hash */
}

/**********************************

	Simple check to see if two GUID's are
	the same, return TRUE if so
	(I am going to cheat), just check the 16 bytes
	
**********************************/

Word BURGERCALL GUIDIsEqual(const GUID *Input1, const GUID *Input2)
{
	if (!Input1) {
		Input1 = &GUIDBlank;
	}
	if (!Input2) {
		Input2 = &GUIDBlank;
	}
	if ((((Word32 *)Input1)[0] == ((Word32 *)Input2)[0]) &&
		(((Word32 *)Input1)[1] == ((Word32 *)Input2)[1]) &&
		(((Word32 *)Input1)[2] == ((Word32 *)Input2)[2]) &&
		(((Word32 *)Input1)[3] == ((Word32 *)Input2)[3])) {
		return TRUE;
	}
	return FALSE;
}

/**********************************

	Simple check to see if two GUID's are
	the same, less than or greater than
	Return 0 for equal, -1 if Input1<Input2
	and 1 if Input1>Input2
	
**********************************/

int BURGERCALL GUIDCompare(const GUID *Input1,const GUID *Input2)
{
	Word i;
	
	if (!Input1) {
		Input1 = &GUIDBlank;
	}
	if (!Input2) {
		Input2 = &GUIDBlank;
	}
	if (Input1->Data1 < Input2->Data1) {
		goto Less;
	}
	if (Input1->Data1 > Input2->Data1) {
		goto More;
	}
	if (Input1->Data2 < Input2->Data2) {
		goto Less;
	}
	if (Input1->Data2 > Input2->Data2) {
		goto More;
	}
	if (Input1->Data3 < Input2->Data3) {
		goto Less;
	}
	if (Input1->Data3 > Input2->Data3) {
		goto More;
	}
	i = 0;
	do {
		if (Input1->Data4[i] < Input2->Data4[i]) {
			goto Less;
		}
		if (Input1->Data4[i] > Input2->Data4[i]) {
			goto More;
		}
	} while (++i<8);
	return 0;		/* They are equal! */
Less:;
	return -1;		/* Input1<Input2 */
More:;
	return 1;		/* Input1>Input2 */
}

/**********************************

	Convert a GUID string of the format...
	8641FBDE-7F8F-11D4-AAC5-000A27DD93F2
	Into a GUID structure
	
**********************************/

Word BURGERCALL GUIDFromString(GUID *Output,const char *Input)
{
	Word i;
	GUID val;
	
	val.Data1 = GUIDFromHex(&Input,8);		/* Get the timestamp */
	val.Data2 = (Word16)GUIDFromHex(&Input,4);		/* The shorts */
	val.Data3 = (Word16)GUIDFromHex(&Input,4);
	i = 0;
	do {
		val.Data4[i] = (Word8)GUIDFromHex(&Input,2);	/* The last 8 bytes */
	} while (++i<8);
	
	while (!GUIDCharIsHex(((Word8 *)Input)[0])) {
		++Input;
	}
	if (((Word8 *)Input)[0]) {
		return TRUE;		/* Error */
	}
	Output[0] = val;
	return FALSE;			/* It's ok! */
}

/**********************************

	Get the MAC network address. If I can't
	generate a random address
	
**********************************/

Word BURGERCALL GUIDGetAddress(EthernetAddress_t *Output)
{
	Word temp;
	if (!got_address) {			/* Did I already get the address? */
		
		temp = GUIDGetEthernetAddr(&saved_addr);		/* Get the address */
		
		/*
		* Was this an error? If so, I need to generate a random
		* sequence to use in place of an Ethernet address.
		*/
		
		if (temp) {
			temp = GUIDGenRandomEthernet(&saved_addr);		/* Make random */
		}
		last_addr_result=temp;
		got_address = TRUE;
	}
	FastMemCpy(Output,&saved_addr,sizeof(EthernetAddress_t));
	return last_addr_result;
}

/**********************************

	Note: we return a value which is 'tuned' to our purposes.  Anyone
	using this routine should modify the return value accordingly.
	
**********************************/

static Word16 GUIDRandom(void)
{
	Word32 m,a,b,r;
	
	m = rand_m;
	a = rand_ia;
	b = rand_ib;
	r = rand_irand;
    m += 7;
    if (m >= 9973) {
    	m -= 9871;
    }
    rand_m = m;
    a += 1907;
    if (a >= 99991) {
    	a -= 89989;
    }
    rand_ia = a;
    b += 73939;
    if (b >= 224729) {
    	b -= 96233;
	}
	rand_ib = b;
    r = (r * m) + a + b;
    rand_irand = r;
    return (Word16)(((r>>16) ^ (r & RAND_MASK)));
}

/**********************************

	Init the random number generator
	Note: we "seed" the RNG with the bits from the clock and the PID
	
**********************************/

#define rand_m_init 971
#define rand_ia_init 11113
#define rand_ib_init 104322
#define rand_irand_init 4181

static void GUIDRandomInit(void)
{
    LongWord64_t t;
    Word16 *seedp;
    Word16 seed;

    /*
     * optimal/recommended starting values according to the reference
     */

    rand_m = rand_m_init;
    rand_ia = rand_ia_init;
    rand_ib = rand_ib_init;
    rand_irand = rand_irand_init;

    /*
     * Generating our 'seed' value
     *
     * We start with the current time, but, since the resolution of clocks is
     * system hardware dependent (eg. Ultrix is 10 msec.) and most likely
     * coarser than our resolution (10 usec) we 'mixup' the bits by xor'ing
     * all the bits together.  This will have the effect of involving all of
     * the bits in the determination of the seed value while remaining system
     * independent.  Then for good measure to ensure a unique seed when there
     * are multiple processes creating UUID's on a system, we add in the PID.
     */
    GUIDGetTime(&t);
    seedp = (Word16 *)(&t);
    seed = seedp[0];
    seed ^= seedp[1];
    seed ^= seedp[2];
    seed ^= seedp[3];
    rand_irand += seed;
}

/**********************************

	Ensure *clkseq is up-to-date
	
	Note: clock_seq is architected to be 14-bits (unsigned) but
		I've put it in here as 16-bits since there isn't a
		14-bit unsigned integer type (yet)
	
**********************************/

static void GUIDNewSeq(Word16 *clkseq)
{
	Word16 Temp;
	/* A clkseq value of 0 indicates that it hasn't been initialized. */
	
	Temp = clkseq[0];
	if (!Temp) {
		/* with a volatile clock, we always init to a random number */
		Temp = GUIDRandom();
	}
	CLOCK_SEQ_BUMP(Temp);
	if (!Temp) {
		++Temp;
	}
	clkseq[0] = Temp;
}

/**********************************

	Init my data
	
**********************************/

static void GUIDInitSelf(void)
{
    GUIDRandomInit();				/* Init random numbers */
	GTimeAdjust = 0;				/* Don't adjust yet */		
    GUIDGetTime(&GLastTime);		/* Init my base time */
    GClockSeq = GUIDRandom();		/* Init my sequence */
    GUIDInitDone = TRUE;			/* Init is done */
}

/**********************************

	Create a new GUID
	
**********************************/

void BURGERCALL GUIDInit(GUID *Output)
{
	EthernetAddress_t eaddr;	/* Current MAC address */
	Word got_no_time;			/* Flag for loop */
	Word32 Temp;
	LongWord64_t time_now;
	
	if (!GUIDInitDone) {		/* Is this manager set? */
		GUIDInitSelf();			/* Init my variables */
	}
	
	GUIDGetAddress(&eaddr);		/* get our hardware network address */
	do {
        GUIDGetTime (&time_now);		/* get the current time */

        /*
         * do stuff like:
         *
         *  o check that our clock hasn't gone backwards and handle it
         *    accordingly with clock_seq
         *  o check that we're not generating uuid's faster than we
         *    can accommodate with our time_adjust fudge factor
         */
		got_no_time = FALSE;			/* Allow exit */
		switch (LongWord64Compare(&time_now, &GLastTime)) {
		default:
//		case -1:
			GUIDNewSeq(&GClockSeq);		/* Make a new sequence value */
		case 1:
			GTimeAdjust = 0;			/* Reset the time adjustment */
			break;
		case 0:			/* Too fast??! */
			if (GTimeAdjust == MAX_TIME_ADJUST) {
				got_no_time = TRUE;		/* spin your wheels while we wait for the clock to tick */
			} else {
				GTimeAdjust++;			/* Adjust the time and wait */
			}
			break;
		}
	} while (got_no_time);

    GLastTime = time_now;				/* Save the new time */
	if (GTimeAdjust) {					/* Should I add a fudge factor for high speed access? */
		LongWord64_t GTimeAdjust64;
		LongWord64FromLong(&GTimeAdjust64,GTimeAdjust);
		LongWord64Add(&time_now,&GTimeAdjust64);
	}

    /* now construct a uuid with the information we've gathered plus a few constants */

    Output->Data1 = LongWord64ToLong(&time_now);	/* Low 32 bits */
    Temp = LongWord64ToHiLong(&time_now);		/* High 32 bits */
    Output->Data2 = (Word16)(Temp & TIME_MID_MASK);

    Output->Data3 = (Word16)((Temp & TIME_HIGH_MASK) >> TIME_HIGH_SHIFT_COUNT);
    Output->Data3 |= UUID_VERSION_BITS;

    Output->Data4[1] = GClockSeq & CLOCK_SEQ_LOW_MASK;
    Output->Data4[0] = (GClockSeq & CLOCK_SEQ_HIGH_MASK) >> CLOCK_SEQ_HIGH_SHIFT_COUNT;
    Output->Data4[0] |= UUID_RESERVED_BITS;

    FastMemCpy(Output->Data4+2,&eaddr,6);
}

