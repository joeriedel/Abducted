/**********************************

	Inflate for Burgerlib

	Original code is Copyright 1995-1998 Mark Adler
	Burgerlib version by Bill Heineman

**********************************/

#include "PkPack.h"
#include "MmMemory.h"
#include "ClStdLib.h"

/**********************************

	Standard defines. Do NOT CHANGE
	under penalty of DEATH!

**********************************/

#define MAX_WBITS 15
#define PRESET_DICT 0x20	/* preset dictionary flag in zlib header */
#define Z_DEFLATED 8

#define Z_NO_FLUSH 0
#define Z_PARTIAL_FLUSH 1	/* will be removed, use Z_SYNC_FLUSH instead */
#define Z_SYNC_FLUSH 2
#define Z_FULL_FLUSH 3
#define Z_FINISH 4

#define Z_OK 0
#define Z_STREAM_END 1
#define Z_NEED_DICT 2
#define Z_ERRNO (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR (-3)
#define Z_MEM_ERROR (-4)
#define Z_BUF_ERROR (-5)
#define Z_VERSION_ERROR (-6)

/* Maximum size of dynamic tree. The maximum found in a long but non-
	exhaustive search was 1004 huft structures (850 for length/literals
	and 154 for distances, the latter actually the result of an
	exhaustive search). The actual maximum is not known, but the
	value below is more than safe. */
#define MANY 1440

typedef Word32 (*check_func)(const Word8 *buf,Word32 len,Word32 check);

typedef enum {
	TYPE,	/* get type bits (3, including end bit) */
	LENS,	/* get lengths for stored */
	STORED,	/* processing stored block */
	TABLE,	/* get table lengths */
	BTREE,	/* get bit lengths tree for a dynamic block */
	DTREE,	/* get length, distance trees for a dynamic block */
	CODES,	/* processing fixed or dynamic block */
	DRY,	/* output remaining window bytes */
	DONE,	/* finished last block, done */
	BAD		/* got a data error--stuck here */
} InflateBlockMode_e;

typedef struct InflateHuft_t {
	union {
		struct {
			Word8 Exop;	/* number of extra bits or operation */
			Word8 Bits;	/* number of bits in this code or subcode */
		} what;
		Word pad;		/* pad structure to a power of 2 (4 bytes for */
	} word;				/* 16-bit, 8 bytes for 32-bit int's) */
	Word base;			/* literal, length base, distance base, */
						/* or table offset */
} InflateHuft_t;


typedef enum {	/* waiting for "i:"=input, "o:"=output, "x:"=nothing */
	START,		/* x: set up for LEN */
	LEN,		/* i: get length/literal/eob next */
	LENEXT,		/* i: getting length extra (have base) */
	DIST,		/* i: get distance next */
	DISTEXT,	/* i: getting distance extra */
	COPY,		/* o: copying bytes in window, waiting for space */
	LIT,		/* o: got literal, waiting for output space */
	WASH,		/* o: got eob, possibly still output waiting */
	END,		/* x: got eob and all data flushed */
	BADCODE		/* x: got error */
} InflateCodesMode_e;

typedef struct InflateCodesState_t {	/* inflate codes private state */
	InflateCodesMode_e mode;	/* current inflate_codes mode */

	/* mode dependent information */
	Word len;
	union {
		struct {
			InflateHuft_t *tree;	/* pointer into tree */
			Word need;			/* bits needed */
		} code;			/* if LEN or DIST, where in tree */
		Word lit;		/* if LIT, literal */
		struct {
			Word get;	/* bits to get for extra */
			Word dist;	/* distance back to copy from */
		} copy;			/* if EXT or COPY, where and how much */
	} sub;				/* submode */

	/* mode independent information */
	InflateHuft_t *ltree;	/* literal/length/eob tree */
	InflateHuft_t *dtree;	/* distance tree */
	Word8 lbits;			/* ltree bits decoded per branch */
	Word8 dbits;			/* dtree bits decoder per branch */
} InflateCodesState_t;

/* inflate blocks semi-private state */
typedef struct InflateBlocksState_t {

	/* mode */
	InflateBlockMode_e mode;	/* current inflate_block mode */

	/* mode dependent information */
	union {
		Word left;					/* if STORED, bytes left to copy */
		struct {
			Word table;				/* table lengths (14 bits) */
			Word index;				/* index into blens (or border) */
			Word *blens;			/* bit lengths of codes */
			Word bb;				/* bit length tree depth */
			InflateHuft_t *tb;		/* bit length decoding tree */
		} trees;					/* if DTREE, decoding info for trees */
		struct {
			InflateCodesState_t *codes;
		} decode;		/* if CODES, current state */
	} sub;				/* submode */
	Word last;			/* true if this block is the last block */

		/* mode independent information */
	Word bitk;				/* bits in bit buffer */
	Word32 bitb;			/* bit buffer */
	Word8 *end;				/* one byte after sliding window */
	Word8 *read;				/* window read pointer */
	Word8 *write;			/* window write pointer */
	Word32 check;			/* check on output */
	Word8 WindowTbl[1<<MAX_WBITS];		/* sliding window */
	InflateHuft_t HuftTbl[MANY];	/* single malloc for tree space */
} InflateBlocksState_t;

typedef enum {
	METHOD,	/* waiting for method byte */
	FLAG,	/* waiting for flag byte */
	DICT4,	/* four dictionary check bytes to go */
	DICT3,	/* three dictionary check bytes to go */
	DICT2,	/* two dictionary check bytes to go */
	DICT1,	/* one dictionary check byte to go */
	DICT0,	/* waiting for inflateSetDictionary */
	BLOCKS,	/* decompressing blocks */
	CHECK4,	/* four check bytes to go */
	CHECK3,	/* three check bytes to go */
	CHECK2,	/* two check bytes to go */
	CHECK1,	/* one check byte to go */
	XDONE,	/* finished check, done */
	XBAD	/* got an error--stay here */
} InflateMode_e;

/* inflate private state */

typedef struct InflateState_t {
	InflateMode_e mode;		/* current inflate mode */

	/* mode dependent information */
	union {
		Word method;		/* if FLAGS, method byte */
		struct {
			Word32 was;	/* computed check value */
			Word32 need;	/* stream check value */
		} check;			/* if CHECK, check values to compare */
		Word marker;		/* if BAD, inflateSync's marker bytes count */
	} sub;					/* submode */
	/* mode independent information */
	InflateBlocksState_t BlockState;	/* current inflate_blocks state */
} InflateState_t;

/**********************************

	Static data for decompression

**********************************/

/**********************************

	Masks for lower bits
	And'ing with mask[n] masks the lower n bits

**********************************/

static Word inflate_mask[17] = {
	0x0000,
	0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
	0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

/* Tables for deflate from PKZIP's appnote.txt. */

static const Word cplens[31] = { /* Copy lengths for literal codes 257..285 */
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
	35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
	/* see note #13 above about 258 */

static const Word cplext[31] = { /* Extra bits for literal codes 257..285 */
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
	3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 112, 112}; /* 112==invalid */

static const Word cpdist[30] = { /* Copy offsets for distance codes 0..29 */
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
	257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
	8193, 12289, 16385, 24577};

static const Word cpdext[30] = { /* Extra bits for distance codes */
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
	7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
	12, 12, 13, 13};

/**********************************

	Table for decoding fixed codes

**********************************/

#define FIXED_BL 9
#define FIXED_BD 5

static InflateHuft_t fixed_tl[] = {
	{{{96,7}},256}, {{{0,8}},80}, {{{0,8}},16}, {{{84,8}},115},
	{{{82,7}},31}, {{{0,8}},112}, {{{0,8}},48}, {{{0,9}},192},
	{{{80,7}},10}, {{{0,8}},96}, {{{0,8}},32}, {{{0,9}},160},
	{{{0,8}},0}, {{{0,8}},128}, {{{0,8}},64}, {{{0,9}},224},
	{{{80,7}},6}, {{{0,8}},88}, {{{0,8}},24}, {{{0,9}},144},
	{{{83,7}},59}, {{{0,8}},120}, {{{0,8}},56}, {{{0,9}},208},
	{{{81,7}},17}, {{{0,8}},104}, {{{0,8}},40}, {{{0,9}},176},
	{{{0,8}},8}, {{{0,8}},136}, {{{0,8}},72}, {{{0,9}},240},
	{{{80,7}},4}, {{{0,8}},84}, {{{0,8}},20}, {{{85,8}},227},
	{{{83,7}},43}, {{{0,8}},116}, {{{0,8}},52}, {{{0,9}},200},
	{{{81,7}},13}, {{{0,8}},100}, {{{0,8}},36}, {{{0,9}},168},
	{{{0,8}},4}, {{{0,8}},132}, {{{0,8}},68}, {{{0,9}},232},
	{{{80,7}},8}, {{{0,8}},92}, {{{0,8}},28}, {{{0,9}},152},
	{{{84,7}},83}, {{{0,8}},124}, {{{0,8}},60}, {{{0,9}},216},
	{{{82,7}},23}, {{{0,8}},108}, {{{0,8}},44}, {{{0,9}},184},
	{{{0,8}},12}, {{{0,8}},140}, {{{0,8}},76}, {{{0,9}},248},
	{{{80,7}},3}, {{{0,8}},82}, {{{0,8}},18}, {{{85,8}},163},
	{{{83,7}},35}, {{{0,8}},114}, {{{0,8}},50}, {{{0,9}},196},
	{{{81,7}},11}, {{{0,8}},98}, {{{0,8}},34}, {{{0,9}},164},
	{{{0,8}},2}, {{{0,8}},130}, {{{0,8}},66}, {{{0,9}},228},
	{{{80,7}},7}, {{{0,8}},90}, {{{0,8}},26}, {{{0,9}},148},
	{{{84,7}},67}, {{{0,8}},122}, {{{0,8}},58}, {{{0,9}},212},
	{{{82,7}},19}, {{{0,8}},106}, {{{0,8}},42}, {{{0,9}},180},
	{{{0,8}},10}, {{{0,8}},138}, {{{0,8}},74}, {{{0,9}},244},
	{{{80,7}},5}, {{{0,8}},86}, {{{0,8}},22}, {{{192,8}},0},
	{{{83,7}},51}, {{{0,8}},118}, {{{0,8}},54}, {{{0,9}},204},
	{{{81,7}},15}, {{{0,8}},102}, {{{0,8}},38}, {{{0,9}},172},
	{{{0,8}},6}, {{{0,8}},134}, {{{0,8}},70}, {{{0,9}},236},
	{{{80,7}},9}, {{{0,8}},94}, {{{0,8}},30}, {{{0,9}},156},
	{{{84,7}},99}, {{{0,8}},126}, {{{0,8}},62}, {{{0,9}},220},
	{{{82,7}},27}, {{{0,8}},110}, {{{0,8}},46}, {{{0,9}},188},
	{{{0,8}},14}, {{{0,8}},142}, {{{0,8}},78}, {{{0,9}},252},
	{{{96,7}},256}, {{{0,8}},81}, {{{0,8}},17}, {{{85,8}},131},
	{{{82,7}},31}, {{{0,8}},113}, {{{0,8}},49}, {{{0,9}},194},
	{{{80,7}},10}, {{{0,8}},97}, {{{0,8}},33}, {{{0,9}},162},
	{{{0,8}},1}, {{{0,8}},129}, {{{0,8}},65}, {{{0,9}},226},
	{{{80,7}},6}, {{{0,8}},89}, {{{0,8}},25}, {{{0,9}},146},
	{{{83,7}},59}, {{{0,8}},121}, {{{0,8}},57}, {{{0,9}},210},
	{{{81,7}},17}, {{{0,8}},105}, {{{0,8}},41}, {{{0,9}},178},
	{{{0,8}},9}, {{{0,8}},137}, {{{0,8}},73}, {{{0,9}},242},
	{{{80,7}},4}, {{{0,8}},85}, {{{0,8}},21}, {{{80,8}},258},
	{{{83,7}},43}, {{{0,8}},117}, {{{0,8}},53}, {{{0,9}},202},
	{{{81,7}},13}, {{{0,8}},101}, {{{0,8}},37}, {{{0,9}},170},
	{{{0,8}},5}, {{{0,8}},133}, {{{0,8}},69}, {{{0,9}},234},
	{{{80,7}},8}, {{{0,8}},93}, {{{0,8}},29}, {{{0,9}},154},
	{{{84,7}},83}, {{{0,8}},125}, {{{0,8}},61}, {{{0,9}},218},
	{{{82,7}},23}, {{{0,8}},109}, {{{0,8}},45}, {{{0,9}},186},
	{{{0,8}},13}, {{{0,8}},141}, {{{0,8}},77}, {{{0,9}},250},
	{{{80,7}},3}, {{{0,8}},83}, {{{0,8}},19}, {{{85,8}},195},
	{{{83,7}},35}, {{{0,8}},115}, {{{0,8}},51}, {{{0,9}},198},
	{{{81,7}},11}, {{{0,8}},99}, {{{0,8}},35}, {{{0,9}},166},
	{{{0,8}},3}, {{{0,8}},131}, {{{0,8}},67}, {{{0,9}},230},
	{{{80,7}},7}, {{{0,8}},91}, {{{0,8}},27}, {{{0,9}},150},
	{{{84,7}},67}, {{{0,8}},123}, {{{0,8}},59}, {{{0,9}},214},
	{{{82,7}},19}, {{{0,8}},107}, {{{0,8}},43}, {{{0,9}},182},
	{{{0,8}},11}, {{{0,8}},139}, {{{0,8}},75}, {{{0,9}},246},
	{{{80,7}},5}, {{{0,8}},87}, {{{0,8}},23}, {{{192,8}},0},
	{{{83,7}},51}, {{{0,8}},119}, {{{0,8}},55}, {{{0,9}},206},
	{{{81,7}},15}, {{{0,8}},103}, {{{0,8}},39}, {{{0,9}},174},
	{{{0,8}},7}, {{{0,8}},135}, {{{0,8}},71}, {{{0,9}},238},
	{{{80,7}},9}, {{{0,8}},95}, {{{0,8}},31}, {{{0,9}},158},
	{{{84,7}},99}, {{{0,8}},127}, {{{0,8}},63}, {{{0,9}},222},
	{{{82,7}},27}, {{{0,8}},111}, {{{0,8}},47}, {{{0,9}},190},
	{{{0,8}},15}, {{{0,8}},143}, {{{0,8}},79}, {{{0,9}},254},
	{{{96,7}},256}, {{{0,8}},80}, {{{0,8}},16}, {{{84,8}},115},
	{{{82,7}},31}, {{{0,8}},112}, {{{0,8}},48}, {{{0,9}},193},
	{{{80,7}},10}, {{{0,8}},96}, {{{0,8}},32}, {{{0,9}},161},
	{{{0,8}},0}, {{{0,8}},128}, {{{0,8}},64}, {{{0,9}},225},
	{{{80,7}},6}, {{{0,8}},88}, {{{0,8}},24}, {{{0,9}},145},
	{{{83,7}},59}, {{{0,8}},120}, {{{0,8}},56}, {{{0,9}},209},
	{{{81,7}},17}, {{{0,8}},104}, {{{0,8}},40}, {{{0,9}},177},
	{{{0,8}},8}, {{{0,8}},136}, {{{0,8}},72}, {{{0,9}},241},
	{{{80,7}},4}, {{{0,8}},84}, {{{0,8}},20}, {{{85,8}},227},
	{{{83,7}},43}, {{{0,8}},116}, {{{0,8}},52}, {{{0,9}},201},
	{{{81,7}},13}, {{{0,8}},100}, {{{0,8}},36}, {{{0,9}},169},
	{{{0,8}},4}, {{{0,8}},132}, {{{0,8}},68}, {{{0,9}},233},
	{{{80,7}},8}, {{{0,8}},92}, {{{0,8}},28}, {{{0,9}},153},
	{{{84,7}},83}, {{{0,8}},124}, {{{0,8}},60}, {{{0,9}},217},
	{{{82,7}},23}, {{{0,8}},108}, {{{0,8}},44}, {{{0,9}},185},
	{{{0,8}},12}, {{{0,8}},140}, {{{0,8}},76}, {{{0,9}},249},
	{{{80,7}},3}, {{{0,8}},82}, {{{0,8}},18}, {{{85,8}},163},
	{{{83,7}},35}, {{{0,8}},114}, {{{0,8}},50}, {{{0,9}},197},
	{{{81,7}},11}, {{{0,8}},98}, {{{0,8}},34}, {{{0,9}},165},
	{{{0,8}},2}, {{{0,8}},130}, {{{0,8}},66}, {{{0,9}},229},
	{{{80,7}},7}, {{{0,8}},90}, {{{0,8}},26}, {{{0,9}},149},
	{{{84,7}},67}, {{{0,8}},122}, {{{0,8}},58}, {{{0,9}},213},
	{{{82,7}},19}, {{{0,8}},106}, {{{0,8}},42}, {{{0,9}},181},
	{{{0,8}},10}, {{{0,8}},138}, {{{0,8}},74}, {{{0,9}},245},
	{{{80,7}},5}, {{{0,8}},86}, {{{0,8}},22}, {{{192,8}},0},
	{{{83,7}},51}, {{{0,8}},118}, {{{0,8}},54}, {{{0,9}},205},
	{{{81,7}},15}, {{{0,8}},102}, {{{0,8}},38}, {{{0,9}},173},
	{{{0,8}},6}, {{{0,8}},134}, {{{0,8}},70}, {{{0,9}},237},
	{{{80,7}},9}, {{{0,8}},94}, {{{0,8}},30}, {{{0,9}},157},
	{{{84,7}},99}, {{{0,8}},126}, {{{0,8}},62}, {{{0,9}},221},
	{{{82,7}},27}, {{{0,8}},110}, {{{0,8}},46}, {{{0,9}},189},
	{{{0,8}},14}, {{{0,8}},142}, {{{0,8}},78}, {{{0,9}},253},
	{{{96,7}},256}, {{{0,8}},81}, {{{0,8}},17}, {{{85,8}},131},
	{{{82,7}},31}, {{{0,8}},113}, {{{0,8}},49}, {{{0,9}},195},
	{{{80,7}},10}, {{{0,8}},97}, {{{0,8}},33}, {{{0,9}},163},
	{{{0,8}},1}, {{{0,8}},129}, {{{0,8}},65}, {{{0,9}},227},
	{{{80,7}},6}, {{{0,8}},89}, {{{0,8}},25}, {{{0,9}},147},
	{{{83,7}},59}, {{{0,8}},121}, {{{0,8}},57}, {{{0,9}},211},
	{{{81,7}},17}, {{{0,8}},105}, {{{0,8}},41}, {{{0,9}},179},
	{{{0,8}},9}, {{{0,8}},137}, {{{0,8}},73}, {{{0,9}},243},
	{{{80,7}},4}, {{{0,8}},85}, {{{0,8}},21}, {{{80,8}},258},
	{{{83,7}},43}, {{{0,8}},117}, {{{0,8}},53}, {{{0,9}},203},
	{{{81,7}},13}, {{{0,8}},101}, {{{0,8}},37}, {{{0,9}},171},
	{{{0,8}},5}, {{{0,8}},133}, {{{0,8}},69}, {{{0,9}},235},
	{{{80,7}},8}, {{{0,8}},93}, {{{0,8}},29}, {{{0,9}},155},
	{{{84,7}},83}, {{{0,8}},125}, {{{0,8}},61}, {{{0,9}},219},
	{{{82,7}},23}, {{{0,8}},109}, {{{0,8}},45}, {{{0,9}},187},
	{{{0,8}},13}, {{{0,8}},141}, {{{0,8}},77}, {{{0,9}},251},
	{{{80,7}},3}, {{{0,8}},83}, {{{0,8}},19}, {{{85,8}},195},
	{{{83,7}},35}, {{{0,8}},115}, {{{0,8}},51}, {{{0,9}},199},
	{{{81,7}},11}, {{{0,8}},99}, {{{0,8}},35}, {{{0,9}},167},
	{{{0,8}},3}, {{{0,8}},131}, {{{0,8}},67}, {{{0,9}},231},
	{{{80,7}},7}, {{{0,8}},91}, {{{0,8}},27}, {{{0,9}},151},
	{{{84,7}},67}, {{{0,8}},123}, {{{0,8}},59}, {{{0,9}},215},
	{{{82,7}},19}, {{{0,8}},107}, {{{0,8}},43}, {{{0,9}},183},
	{{{0,8}},11}, {{{0,8}},139}, {{{0,8}},75}, {{{0,9}},247},
	{{{80,7}},5}, {{{0,8}},87}, {{{0,8}},23}, {{{192,8}},0},
	{{{83,7}},51}, {{{0,8}},119}, {{{0,8}},55}, {{{0,9}},207},
	{{{81,7}},15}, {{{0,8}},103}, {{{0,8}},39}, {{{0,9}},175},
	{{{0,8}},7}, {{{0,8}},135}, {{{0,8}},71}, {{{0,9}},239},
	{{{80,7}},9}, {{{0,8}},95}, {{{0,8}},31}, {{{0,9}},159},
	{{{84,7}},99}, {{{0,8}},127}, {{{0,8}},63}, {{{0,9}},223},
	{{{82,7}},27}, {{{0,8}},111}, {{{0,8}},47}, {{{0,9}},191},
	{{{0,8}},15}, {{{0,8}},143}, {{{0,8}},79}, {{{0,9}},255}
};

static InflateHuft_t fixed_td[] = {
	{{{80,5}},1}, {{{87,5}},257}, {{{83,5}},17}, {{{91,5}},4097},
	{{{81,5}},5}, {{{89,5}},1025}, {{{85,5}},65}, {{{93,5}},16385},
	{{{80,5}},3}, {{{88,5}},513}, {{{84,5}},33}, {{{92,5}},8193},
	{{{82,5}},9}, {{{90,5}},2049}, {{{86,5}},129}, {{{192,5}},24577},
	{{{80,5}},2}, {{{87,5}},385}, {{{83,5}},25}, {{{91,5}},6145},
	{{{81,5}},7}, {{{89,5}},1537}, {{{85,5}},97}, {{{93,5}},24577},
	{{{80,5}},4}, {{{88,5}},769}, {{{84,5}},49}, {{{92,5}},12289},
	{{{82,5}},13}, {{{90,5}},3073}, {{{86,5}},193}, {{{192,5}},24577}
};


/* Table for deflate from PKZIP's appnote.txt. */

static const Word border[] = { /* Order of the bit length code lengths */
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};


/**********************************

	copy as much as possible from the sliding window to the output area

**********************************/

static int inflate_flush(PackState_t * z,int r)
{
	Word n;
	Word8 *p;
	Word8 *q;
	InflateBlocksState_t *s;

	s = &((InflateState_t *)z->Internal)->BlockState;

	/* local copies of source and destination pointers */
	p = z->OutPtr;
	q = s->read;

	/* compute number of bytes to copy as far as end of window */
	n = (Word)((q <= s->write ? s->write : s->end) - q);
	if (n > z->OutLen) {
		n = z->OutLen;
	}
	if (n && r == Z_BUF_ERROR) {
		r = Z_OK;
	}
	/* update counters */
	z->OutLen -= n;

	/* update check information */
	s->check = CalcMoreAdler(q,n,s->check);

	/* copy as far as end of window */
	FastMemCpy(p, q, n);
	p += n;
	q += n;

	/* see if more to copy at beginning of window */
	if (q == s->end) {
	/* wrap pointers */
		q = s->WindowTbl;
		if (s->write == s->end) {
			s->write = s->WindowTbl;
		}

	/* compute bytes to copy */
		n = (Word)(s->write - q);
		if (n > z->OutLen) {
			n = z->OutLen;
		}
		if (n && r == Z_BUF_ERROR) {
 			r = Z_OK;
		}
	/* update counters */
		z->OutLen -= n;

	/* update check information */
		s->check = CalcMoreAdler(q, n,s->check);

	/* copy */
		FastMemCpy(p, q, n);
		p += n;
		q += n;
	}

	/* update pointers */
	z->OutPtr = p;
	s->read = q;

	/* done */
	return r;
}

/* Called with number of bytes left to write in window at least 258
	(the maximum string length) and number of input bytes available
	at least ten. The ten bytes are six bytes for the longest length/
	distance pair plus four bytes for overloading the bit buffer. */

static int inflate_fast(Word bl,Word bd,InflateHuft_t *tl,
	InflateHuft_t *td,InflateBlocksState_t *s,PackState_t* z)
{
	InflateHuft_t *t;	/* temporary pointer */
	Word e;				/* extra bits or operation */
	Word32 b;			/* bit buffer */
	Word k;				/* bits in bit buffer */
	Word8 *p;			/* input data pointer */
	Word n;				/* bytes available there */
	Word8 *q;			/* output window write pointer */
	Word m;				/* bytes to end of window or read pointer */
	Word ml;			/* mask for literal/length tree */
	Word md;			/* mask for distance tree */
	Word c;				/* bytes to copy */
	Word d;				/* distance back to copy from */
	Word8 *r;			/* copy source pointer */
	int Result;

	/* load input, output, bit values */
	p=z->PackPtr;
	n=z->PackLen;
	b=s->bitb;
	k=s->bitk;
	q=s->write;
	m=(Word)(q<s->read?s->read-q-1:s->end-q);

	/* initialize masks */
	ml = inflate_mask[bl];
	md = inflate_mask[bd];

	/* do until not enough input or output space for fast loop */
	do {		/* assume called with m >= 258 && n >= 10 */
				/* get literal/length code */
		while(k<20) {
			--n;
			b|=((Word32)(*p++))<<k;
			k+=8;
		}				/* max bits for literal/length code */
		if ((e = (t = tl + ((Word)b & ml))->word.what.Exop) == 0) {
			b>>=t->word.what.Bits;
			k-=t->word.what.Bits;
			*q++ = (Word8)t->base;
			m--;
			continue;
		}
		do {
			b>>=t->word.what.Bits;
			k-=t->word.what.Bits;
			if (e & 16) {
				/* get extra bits for length */
				e &= 15;
				c = t->base + ((Word)b & inflate_mask[e]);
				b>>=e;
				k-=e;

				/* decode distance base of block to copy */
				while(k<15) {
					--n;
					b|=((Word32)(*p++))<<k;
					k+=8;
				}			/* max bits for distance code */
				e = (t = td + ((Word)b & md))->word.what.Exop;
				do {
					b>>=(t->word.what.Bits);
					k-=(t->word.what.Bits);
					if (e & 16) {
					/* get extra bits to add to distance base */
						e &= 15;
						while(k<e) {
							--n;
							b|=((Word32)(*p++))<<k;
							k+=8;
						}			/* get extra bits (up to 13) */
						d = t->base + ((Word)b & inflate_mask[e]);
						b>>= e;
						k-=e;

						/* do the copy */
						m -= c;
						if ((Word)(q - s->WindowTbl) >= d) {		/* offset before dest */
												/* just copy */
							r = q - d;
							q[0] = r[0];		/* minimum count is three, */
							q[1] = r[1];
							r+=2;
							q+=2;
							c-=2;				/* so unroll loop a little */
						} else {				/* else offset after destination */
							e = d - (Word)(q - s->WindowTbl); /* bytes from offset to end */
							r = s->end - e;		/* pointer to offset */
							if (c > e) {		/* if source crosses, */
								c -= e;			/* copy to end of window */
								do {
									q[0] = r[0];
									++r;
									++q;
								} while (--e);
								r = s->WindowTbl;		/* copy rest from start of window */
							}
						}
						do {						/* copy all or what's left */
							q[0] = r[0];
							++r;
							++q;
						} while (--c);
						break;
					} else if (!(e & 64)) {
						t += t->base;
						e = (t += ((Word)b & inflate_mask[e]))->word.what.Exop;
					} else {
						Result = Z_DATA_ERROR;
						goto ByeBye;
					}
				} while (1);
				break;
			}
			if ((e & 64) == 0) {
				t += t->base;
				if ((e = (t += ((Word)b & inflate_mask[e]))->word.what.Exop) == 0) {
					b>>=t->word.what.Bits;
					k-=t->word.what.Bits;
					*q++ = (Word8)t->base;
					m--;
					break;
				}
			} else {
				if (e & 32) {
					Result = Z_STREAM_END;
					goto ByeBye;
				}
				Result = Z_DATA_ERROR;
				goto ByeBye;
			}
		} while (1);
	} while (m >= 258 && n >= 10);

	/* not enough input or output--restore pointers and return */
	Result = Z_OK;
ByeBye:;
	c=z->PackLen-n;
	c=(k>>3)<c?k>>3:c;
	n+=c;
	p-=c;
	k-=c<<3;
	s->bitb=b;
	s->bitk=k;
	z->PackLen=n;
	z->PackPtr=p;
	s->write=q;
	return Result;
}

static InflateCodesState_t *inflate_codes_new(Word bl,Word bd,InflateHuft_t *tl,
	InflateHuft_t *td,PackState_t* /* z */)
{
	InflateCodesState_t *c;

	if ((c = (InflateCodesState_t *)AllocAPointerClear(sizeof(InflateCodesState_t))) != 0) {
		c->mode = START;
		c->lbits = (Word8)bl;
		c->dbits = (Word8)bd;
		c->ltree = tl;
		c->dtree = td;
	}
	return c;
}


static int inflate_codes(InflateBlocksState_t *s,PackState_t* z,int r)
{
	Word j;					/* temporary storage */
	InflateHuft_t *t;		/* temporary pointer */
	Word e;					/* extra bits or operation */
	Word32 b;				/* bit buffer */
	Word k;					/* bits in bit buffer */
	Word8 *p;				/* input data pointer */
	Word n;					/* bytes available there */
	Word8 *q;				/* output window write pointer */
	Word m;					/* bytes to end of window or read pointer */
	Word8 *f;				/* pointer to copy strings from */
	InflateCodesState_t *c = s->sub.decode.codes;		/* codes state */

	/* copy input/output information to locals (UPDATE macro restores) */
	p=z->PackPtr;
	n=z->PackLen;
	b=s->bitb;
	k=s->bitk;
	q=s->write;
	m=(Word)(q<s->read?s->read-q-1:s->end-q);

	/* process input and output based on current state */
	while (1) {
		switch (c->mode) {			/* waiting for "i:"=input, "o:"=output, "x:"=nothing */
		case START:					/* x: set up for LEN */
			if (m >= 258 && n >= 10) {
				s->bitb=b;
				s->bitk=k;
				z->PackLen=n;
				z->PackPtr=p;
				s->write=q;
				r = inflate_fast(c->lbits, c->dbits, c->ltree, c->dtree, s, z);
				p=z->PackPtr;
				n=z->PackLen;
				b=s->bitb;
				k=s->bitk;
				q=s->write;
				m=(Word)(q<s->read?s->read-q-1:s->end-q);
				if (r != Z_OK) {
					c->mode = r == Z_STREAM_END ? WASH : BADCODE;
					break;
				}
			}
			c->sub.code.need = c->lbits;
			c->sub.code.tree = c->ltree;
			c->mode = LEN;
		case LEN:			/* i: get length/literal/eob next */
			j = c->sub.code.need;
			while (k<j) {
				if (!n) {
					goto Abort;
				}
				r=Z_OK;
				--n;
				b|=((Word32)(*p++))<<k;
				k+=8;
			}
			t = c->sub.code.tree + ((Word)b & inflate_mask[j]);
			b>>=(t->word.what.Bits);
			k-=(t->word.what.Bits);
			e = (Word)(t->word.what.Exop);
			if (e == 0) {				/* literal */
				c->sub.lit = t->base;
				c->mode = LIT;
				break;
			}
			if (e & 16) {				/* length */
				c->sub.copy.get = e & 15;
				c->len = t->base;
				c->mode = LENEXT;
				break;
			}
			if ((e & 64) == 0) {		/* next table */
				c->sub.code.need = e;
				c->sub.code.tree = t + t->base;
				break;
			}
			if (e & 32) {				/* end of block */
				c->mode = WASH;
				break;
			}
			c->mode = BADCODE;			/* invalid code */
			r = Z_DATA_ERROR;
			goto Abort;
		case LENEXT:			/* i: getting length extra (have base) */
			j = c->sub.copy.get;
			while(k<j) {
				if (!n) {
					goto Abort;
				}
				r=Z_OK;
				--n;
				b|=((Word32)(*p++))<<k;
				k+=8;
			}
			c->len += (Word)b & inflate_mask[j];
			b>>=j;
			k-=j;
			c->sub.code.need = c->dbits;
			c->sub.code.tree = c->dtree;
			c->mode = DIST;
		case DIST:			/* i: get distance next */
			j = c->sub.code.need;
			while(k<j) {
				if(!n) {
					goto Abort;
				}
				r=Z_OK;
				--n;
				b|=((Word32)(*p++))<<k;
				k+=8;
			}
			t = c->sub.code.tree + ((Word)b & inflate_mask[j]);
			b>>=(t->word.what.Bits);
			k-=(t->word.what.Bits);
			e = (Word)(t->word.what.Exop);
			if (e & 16) {				/* distance */
				c->sub.copy.get = e & 15;
				c->sub.copy.dist = t->base;
				c->mode = DISTEXT;
				break;
			}
			if ((e & 64) == 0) {		/* next table */
				c->sub.code.need = e;
				c->sub.code.tree = t + t->base;
				break;
			}
			c->mode = BADCODE;			/* invalid code */
			r = Z_DATA_ERROR;
			goto Abort;

		case DISTEXT:			/* i: getting distance extra */
			j = c->sub.copy.get;
			while (k<j) {
				if (!n) {
					goto Abort;
				}
				r=Z_OK;
				--n;
				b|=((Word32)(*p++))<<k;
				k+=8;
			}
			c->sub.copy.dist += (Word)b & inflate_mask[j];
			b>>=j;
			k-=j;
			c->mode = COPY;
		case COPY:			/* o: copying bytes in window, waiting for space */
			f = (Word)(q - s->WindowTbl) < c->sub.copy.dist ?
			s->end - (c->sub.copy.dist - (q - s->WindowTbl)) :
				q - c->sub.copy.dist;
			while (c->len) {
				if (!m) {
					if((q==s->end)&&(s->read!=s->WindowTbl)) {
						q=s->WindowTbl;
						m=(Word)(q<s->read?s->read-q-1:s->end-q);
					}
					if(!m) {
						s->write=q;
						r=inflate_flush(z,r);
						q=s->write;
						m=(Word)(q<s->read?s->read-q-1:s->end-q);
						if(q==s->end&&s->read!=s->WindowTbl){
							q=s->WindowTbl;
							m=(Word)(q<s->read?s->read-q-1:s->end-q);
						}
						if (!m) {
							goto Abort;
						}
					}
				}
				r=Z_OK;
				*q++=(Word8)(*f++);
				m--;
				if (f == s->end) {
					f = s->WindowTbl;
				}
				c->len--;
			}
			c->mode = START;
			break;
		case LIT:			/* o: got literal, waiting for output space */
			if (!m){
				if(q==s->end&&s->read!=s->WindowTbl){
					q=s->WindowTbl;
					m=(Word)(q<s->read?s->read-q-1:s->end-q);
				}
				if(!m){
					s->write=q;
					r=inflate_flush(z,r);
					q=s->write;
					m=(Word)(q<s->read?s->read-q-1:s->end-q);
					if(q==s->end&&s->read!=s->WindowTbl){
						q=s->WindowTbl;
						m=(Word)(q<s->read?s->read-q-1:s->end-q);
					}
					if(!m) {
						goto Abort;
					}
				}
			}
			r=Z_OK;
			*q++=(Word8)(c->sub.lit);
			m--;
			c->mode = START;
			break;
		case WASH:				/* o: got eob, possibly more output */
			if (k > 7) {		/* return unused byte, if any */
				k -= 8;
				n++;
				p--;			/* can always return one */
			}
			s->write=q;
			r=inflate_flush(z,r);
			q=s->write;
			m=(Word)(q<s->read?s->read-q-1:s->end-q);
			if (s->read != s->write) {
				goto Abort;
			}
			c->mode = END;
		case END:
			r = Z_STREAM_END;
			goto Abort;
		case BADCODE:			/* x: got error */
			r = Z_DATA_ERROR;
			goto Abort;
		default:
			r = Z_STREAM_ERROR;
			goto Abort;
		}
	}
Abort:;
	s->bitb=b;
	s->bitk=k;
	z->PackLen=n;
	z->PackPtr=p;
	s->write=q;
	return inflate_flush(z,r);
}

/*
	If you use the zlib library in a product, an acknowledgment is welcome
	in the documentation of your product. If for some reason you cannot
	include such an acknowledgment, I would appreciate that you keep this
	copyright string in the executable of your product.
*/
/* simplify the use of the InflateHuft_t type with some defines */


/*
	Huffman code decoding is performed using a multi-level table lookup.
	The fastest way to decode is to simply build a lookup table whose
	size is determined by the longest code. However, the time it takes
	to build this table can also be a factor if the data being decoded
	is not very long. The most common codes are necessarily the
	shortest codes, so those codes dominate the decoding time, and hence
	the speed. The idea is you can have a shorter table that decodes the
	shorter, more probable codes, and then point to subsidiary tables for
	the longer codes. The time it costs to decode the longer codes is
	then traded against the time it takes to make longer tables.

	This results of this trade are in the variables lbits and dbits
	below. lbits is the number of bits the first level table for literal/
	length codes can decode in one step, and dbits is the same thing for
	the distance codes. Subsequent tables are also less than or equal to
	those sizes. These values may be adjusted either when all of the
	codes are shorter than that, in which case the longest code length in
	bits is used, or when the shortest code is *longer* than the requested
	table size, in which case the length of the shortest code in bits is
	used.

	There are two different values for the two tables, since they code a
	different number of possibilities each. The literal/length table
	codes 286 possible values, or in a flat code, a little over eight
	bits. The distance table codes 30 possible values, or a little less
	than five bits, flat. The optimum values for speed end up being
	about one bit more than those, so lbits is 8+1 and dbits is 5+1.
	The optimum values may differ though from machine to machine, and
	possibly even between compilers. Your mileage may vary.
*/


/* If BMAX needs to be larger than 16, then h and x[] should be uLong. */
#define BMAX 15		/* maximum bit length of any code */

static int huft_build(Word *b,Word n,Word s, const Word *d,const Word *e,
	InflateHuft_t **t,Word *m,InflateHuft_t *hp,Word *hn,Word *v)
/* Given a list of code lengths and a maximum table size, make a set of
	tables to decode that set of codes. Return Z_OK on success, Z_BUF_ERROR
	if the given code set is incomplete (the tables are still built in this
	case), Z_DATA_ERROR if the input is invalid (an over-subscribed set of
	lengths), or Z_MEM_ERROR if not enough memory. */
{

	Word a;						/* counter for codes of length k */
	Word c[BMAX+1];				/* bit length count table */
	Word f;						/* i repeats in table every f entries */
	int g;						/* maximum code length */
	int h;						/* table level */
	register Word i;			/* counter, current code */
	register Word j;			/* counter */
	register int k;				/* number of bits in current code */
	int l;						/* bits per table (returned in m) */
	Word mask;					/* (1 << w) - 1, to avoid cc -O bug on HP */
	register Word *p;			/* pointer into c[], b[], or v[] */
	InflateHuft_t *q;			/* points to current table */
	InflateHuft_t r;			/* table entry for structure assignment */
	InflateHuft_t *u[BMAX];		/* table stack */
	register int w;				/* bits before this table == (l * h) */
	Word x[BMAX+1];				/* bit offsets, then code stack */
	Word *xp;					/* pointer into x */
	int y;						/* number of dummy codes added */
	Word z;						/* number of entries in current table */


	/* Generate counts for each bit length */
	c[0] = 0;				/* clear c[]--assume BMAX+1 is 16 */
	c[1] = 0;
	c[2] = 0;
	c[3] = 0;
	c[4] = 0;
	c[5] = 0;
	c[6] = 0;
	c[7] = 0;
	c[8] = 0;
	c[9] = 0;
	c[10] = 0;
	c[11] = 0;
	c[12] = 0;
	c[13] = 0;
	c[14] = 0;
	c[15] = 0;
	p = b;
	i = n;
	do {
		c[p[0]]++;				/* assume all entries <= BMAX */
		++p;
	} while (--i);
	if (c[0] == n) {			/* null input--all zero length codes */
		*t = (InflateHuft_t *)0;
		*m = 0;
		return Z_OK;
	}


	/* Find minimum and maximum length, bound *m by those */
	l = *m;
	j = 1;
	do {
		if (c[j]) {
			break;
		}
	} while (++j<=BMAX);
	k = j;						/* minimum code length */
	if ((Word)l < j) {
		l = j;
	}
	i = BMAX;
	do {
		if (c[i]) {
			break;
		}
	} while (--i);
	g = i;						/* maximum code length */
	if ((Word)l > i) {
		l = i;
	}
	*m = l;


	/* Adjust last length count to fill out codes, if needed */
	for (y = 1 << j; j < i; j++, y <<= 1) {
		if ((y -= c[j]) < 0) {
			return Z_DATA_ERROR;
		}
	}
	if ((y -= c[i]) < 0) {
		return Z_DATA_ERROR;
	}
	c[i] += y;


	/* Generate starting offsets into the value table for each length */
	x[1] = j = 0;
	p = c + 1;
	xp = x + 2;
	while (--i) {					/* note that i == g from above */
		xp[0] = (j += p[0]);
		++p;
		++xp;
	}


	/* Make a table of values in order of bit lengths */
	p = b;
	i = 0;
	do {
		if ((j = p[0]) != 0) {
			v[x[j]++] = i;
		}
		++p;
	} while (++i < n);
	n = x[g];					/* set n to length of v */


	/* Generate the Huffman codes and for each, make the table entries */
	x[0] = i = 0;					/* first Huffman code is zero */
	p = v;						/* grab values in bit order */
	h = -1;						/* no tables yet--level -1 */
	w = -l;						/* bits decoded == (l * h) */
	u[0] = (InflateHuft_t *)0;		/* just to keep compilers happy */
	q = (InflateHuft_t *)0;		/* ditto */
	z = 0;						/* ditto */

	/* go through the bit lengths (k already is bits in shortest code) */
	for (; k <= g; k++) {
		a = c[k];
		while (a--) {
		/* here i is the Huffman code of length k bits for value *p */
		/* make tables up to required level */
			while (k > w + l) {
				h++;
				w += l;				/* previous table always l bits */

				/* compute minimum size table less than or equal to l bits */
				z = g - w;
				z = z > (Word)l ? l : z;			/* table size upper limit */
				if ((f = 1 << (j = k - w)) > a + 1) {		/* try a k-w bit table */
					/* too few codes for k-w bit table */
					f -= a + 1;				/* deduct codes from patterns left */
					xp = c + k;
					if (j < z)
						while (++j < z) {	/* try smaller tables up to z bits */
							if ((f <<= 1) <= *++xp)
								break;			/* enough codes to use up j bits */
							f -= *xp;			/* else deduct codes from patterns */
						}
				}
				z = 1 << j;				/* table entries for j-bit table */

				/* allocate new table */
				if (*hn + z > MANY)			/* (note: doesn't matter for fixed) */
					return Z_MEM_ERROR;		/* not enough memory */
				u[h] = q = hp + *hn;
				*hn += z;

				/* connect to last table, if there is one */
				if (h) {
					x[h] = i;				/* save pattern for backing up */
					r.word.what.Bits = (Word8)l;		/* bits to dump before this table */
					r.word.what.Exop = (Word8)j;		/* bits in this table */
					j = i >> (w - l);
					r.base = (Word)(q - u[h-1] - j);	/* offset to this table */
					u[h-1][j] = r;			/* connect to last table */
				} else
					*t = q;					/* first table is returned result */
			}

			/* set up table entry in r */
			r.word.what.Bits = (Word8)(k - w);
			if (p >= v + n)
				r.word.what.Exop = 128 + 64;		/* out of values--invalid code */
			else if (*p < s) {
				r.word.what.Exop = (Word8)(*p < 256 ? 0 : 32 + 64);		/* 256 is end-of-block */
				r.base = *p++;			/* simple code is just the value */
			} else {
				r.word.what.Exop = (Word8)(e[*p - s] + 16 + 64);/* non-simple--look up in lists */
				r.base = d[*p++ - s];
			}

			/* fill code-like entries with r */
			f = 1 << (k - w);
			for (j = i >> w; j < z; j += f)
				q[j] = r;

			/* backwards increment the k-bit code i */
			for (j = 1 << (k - 1); i & j; j >>= 1)
				i ^= j;
			i ^= j;

			/* backup over finished tables */
			mask = (1 << w) - 1;		/* needed on HP, cc -O bug */
			while ((i & mask) != x[h]) {
				h--;					/* don't need to update q */
				w -= l;
				mask = (1 << w) - 1;
			}
		}
	}
	/* Return Z_BUF_ERROR if we were given an incomplete table */
	return y != 0 && g != 1 ? Z_BUF_ERROR : Z_OK;
}


static int inflate_trees_bits(Word *c,Word *bb,InflateHuft_t **tb,InflateHuft_t *hp,PackState_t * /* z */)
{
	int r;
	Word hn = 0;		/* hufts used in space */
	Word *v;			/* work area for huft_build */

	if ((v = (Word*)AllocAPointerClear(19*sizeof(Word))) == 0) {
		return Z_MEM_ERROR;
	}
	r = huft_build(c, 19, 19,(Word*)0,(Word*)0,tb, bb, hp, &hn, v);
	if (r != Z_DATA_ERROR) {
		if (r == Z_BUF_ERROR || *bb == 0) {
			r = Z_DATA_ERROR;
		}
	}
	DeallocAPointer(v);
	return r;
}


static int inflate_trees_dynamic(Word nl,Word nd,Word *c,Word *bl,Word *bd,
	InflateHuft_t **tl,InflateHuft_t **td,InflateHuft_t * hp,PackState_t * /* z */)
{
	int r;
	Word hn = 0;			/* hufts used in space */
	Word *v;				/* work area for huft_build */

	/* allocate work area */
	if ((v = (Word*)AllocAPointerClear(288*sizeof(Word))) == 0)
		return Z_MEM_ERROR;

	/* build literal/length tree */
	r = huft_build(c, nl, 257, cplens, cplext, tl, bl, hp, &hn, v);
	if (r != Z_OK || *bl == 0) {
		if (r != Z_DATA_ERROR) {
			if (r != Z_MEM_ERROR) {
				r = Z_DATA_ERROR;
			}
		}
		DeallocAPointer(v);
		return r;
	}

	/* build distance tree */
	r = huft_build(c + nl, nd, 0, cpdist, cpdext, td, bd, hp, &hn, v);
	if (r != Z_OK || (*bd == 0 && nl > 257)) {
		if (r != Z_DATA_ERROR) {
			if (r == Z_BUF_ERROR) {
				r = Z_DATA_ERROR;
			} else if (r != Z_MEM_ERROR) {
				r = Z_DATA_ERROR;
			}
		}
		DeallocAPointer(v);
		return r;
	}

	/* done */
	DeallocAPointer(v);
	return Z_OK;
}



/*
	Notes beyond the 1.93a appnote.txt:

	1. Distance pointers never point before the beginning of the output
		stream.
	2. Distance pointers can point back across blocks, up to 32k away.
	3. There is an implied maximum of 7 bits for the bit length table and
		15 bits for the actual data.
	4. If only one code exists, then it is encoded using one bit. (Zero
		would be more efficient, but perhaps a little confusing.) If two
		codes exist, they are coded using one bit each (0 and 1).
	5. There is no way of sending zero distance codes--a dummy must be
		sent if there are none. (History: a pre 2.0 version of PKZIP would
		store blocks with no distance codes, but this was discovered to be
		too harsh a criterion.) Valid only for 1.93a. 2.04c does allow
		zero distance codes, which is sent as one code of zero bits in
		length.
	6. There are up to 286 literal/length codes. Code 256 represents the
		end-of-block. Note however that the static length tree defines
		288 codes just to fill out the Huffman codes. Codes 286 and 287
		cannot be used though, since there is no length base or extra bits
		defined for them. Similarily, there are up to 30 distance codes.
		However, static trees define 32 codes (all 5 bits) to fill out the
		Huffman codes, but the last two had better not show up in the data.
	7. Unzip can check dynamic Huffman blocks for complete code sets.
		The exception is that a single code would not be complete (see #4).
	8. The five bits following the block type is really the number of
		literal codes sent minus 257.
	9. Length codes 8,16,16 are interpreted as 13 length codes of 8 bits
		(1+6+6). Therefore, to output three times the length, you output
		three codes (1+1+1), whereas to output four times the same length,
		you only need two codes (1+3). Hmm.
	10. In the tree reconstruction algorithm, Code = Code + Increment
		only if BitLength(i) is not zero. (Pretty obvious.)
	11. Correction: 4 Bits: # of Bit Length codes - 4 (4 - 19)
	12. Note: length code 284 can represent 227-258, but length code 285
		really is 258. The last length deserves its own, short code
		since it gets used a lot in very redundant files. The length
		258 is special since 258 - 3 (the min match length) is 255.
	13. The literal/length and distance code bit lengths are read as a
		single stream of lengths. It is possible (and advantageous) for
		a repeat code (16, 17, or 18) to go across the boundary between
		the two sets of lengths.
*/

/**********************************

	Reset the decompression state.
	s->mode has the last mode to shut down

**********************************/

static void inflate_blocks_reset(PackState_t * z,Word32 *c)
{
	InflateBlocksState_t *s;

	s = &((InflateState_t *)z->Internal)->BlockState;
	if (c) {
		*c = s->check;		/* Return the checksum */
	}
	if (s->mode == BTREE || s->mode == DTREE) {
		DeallocAPointer(s->sub.trees.blens);
	}
	if (s->mode == CODES) {
		DeallocAPointer(s->sub.decode.codes);
	}
	s->mode = TYPE;
	s->bitk = 0;
	s->bitb = 0;
	s->read = s->write = s->WindowTbl;
	s->check = CalcMoreAdler(0,0,0);
}

/**********************************

	Here is where 99% of the decompression
	is performed.

**********************************/

static int inflate_blocks(PackState_t * z,int r)
{
	Word t;				/* temporary storage */
	Word32 b;			/* bit buffer */
	Word k;				/* bits in bit buffer */
	Word8 *p;			/* input data pointer */
	Word n;				/* bytes available there */
	Word8 *q;			/* output window write pointer */
	Word m;				/* bytes to end of window or read pointer */
	InflateBlocksState_t *s;

	s = &((InflateState_t *)z->Internal)->BlockState;
	/* copy input/output information to locals (UPDATE macro restores) */
	p=z->PackPtr;
	n=z->PackLen;
	b=s->bitb;
	k=s->bitk;
	q=s->write;
	m=(Word)(q<s->read?s->read-q-1:s->end-q);

	/* process input based on current state */
	while (1) {
		switch (s->mode) {
		case TYPE:
			while(k<3) {
				if (!n) {
					goto Abort;
				}
				r = Z_OK;
				--n;
				b|=((Word32)(*p++))<<k;
				k+=8;
			}
			t = (Word)b & 7;
			b>>=3;			/* Accept 3 bits */
			k-=3;
			s->last = t & 1;
			switch (t&6) {
			case 0:							/* stored */
				t = k & 7;					/* go to byte boundary */
				b>>=t;
				k-=t;
				s->mode = LENS;				/* get length of stored block */
				break;
			case 2:							/* fixed */
				s->sub.decode.codes = inflate_codes_new(FIXED_BL,FIXED_BD,fixed_tl,fixed_td, z);
				if (s->sub.decode.codes == 0) {
					r = Z_MEM_ERROR;
					goto Abort;
				}
				s->mode = CODES;
				break;
			case 4:							/* dynamic */
				s->mode = TABLE;
				break;
			case 6:							/* illegal */
				s->mode = BAD;
				r = Z_DATA_ERROR;
				goto Abort;
			}
			break;
		case LENS:
			while (k<32) {
				if(!n) {
					goto Abort;
				}
				r=Z_OK;
				--n;
				b|=((Word32)(*p++))<<k;
				k+=8;
			}
			if ((((~b) >> 16) & 0xffff) != (b & 0xffff)) {
				s->mode = BAD;
				r = Z_DATA_ERROR;
				goto Abort;
			}
			s->sub.left = (Word)b & 0xffff;
			b = k = 0;						/* dump bits */
			s->mode = s->sub.left ? STORED : (s->last ? DRY : TYPE);
			break;
		case STORED:
			if (!n) {
				goto Abort;
			}

			if (!m) {
				if(q==s->end&&s->read!=s->WindowTbl) {
					q=s->WindowTbl;
					m=(Word)(q<s->read?s->read-q-1:s->end-q);
				}
				if (!m) {
					s->write=q;
					r=inflate_flush(z,r);
					q=s->write;
					m=(Word)(q<s->read?s->read-q-1:s->end-q);
					if(q==s->end&&s->read!=s->WindowTbl) {
						q=s->WindowTbl;
						m=(Word)(q<s->read?s->read-q-1:s->end-q);
					}
					if (!m) {
						goto Abort;
					}
				}
			}
			r=Z_OK;
			t = s->sub.left;
			if (t > n) {
				t = n;
			}
			if (t > m) {
				t = m;
			}
			FastMemCpy(q, p, t);
			p += t;
			n -= t;
			q += t;
			m -= t;
			if ((s->sub.left -= t) != 0) {
				break;
			}
			s->mode = s->last ? DRY : TYPE;
			break;

		case TABLE:
			while (k<14) {
				if (!n) {
					goto Abort;
				}
				r=Z_OK;
				--n;
				b|=((Word32)(*p++))<<k;
				k+=8;
			}
			s->sub.trees.table = t = (Word)b & 0x3fff;
			if ((t & 0x1f) > 29 || ((t >> 5) & 0x1f) > 29) {
				s->mode = BAD;
				r = Z_DATA_ERROR;
				goto Abort;
			}
			t = 258 + (t & 0x1f) + ((t >> 5) & 0x1f);
			if ((s->sub.trees.blens = (Word*)AllocAPointerClear(t*sizeof(Word))) == 0) {
 				r = Z_MEM_ERROR;
				goto Abort;
			}
			b>>=14;
			k-=14;
			s->sub.trees.index = 0;
			s->mode = BTREE;
		case BTREE:
			while (s->sub.trees.index < 4 + (s->sub.trees.table >> 10)) {
				while(k<3) {
					if(!n) {
						goto Abort;
					}
					r=Z_OK;
					--n;
					b|=((Word32)(*p++))<<k;
					k+=8;
				}
				s->sub.trees.blens[border[s->sub.trees.index++]] = (Word)b & 7;
				b>>=3;
				k-=3;
			}
			while (s->sub.trees.index < 19) {
				s->sub.trees.blens[border[s->sub.trees.index++]] = 0;
			}
			s->sub.trees.bb = 7;
			t = inflate_trees_bits(s->sub.trees.blens, &s->sub.trees.bb,
				&s->sub.trees.tb, s->HuftTbl, z);
			if (t != Z_OK) {
				DeallocAPointer(s->sub.trees.blens);
				r = t;
				if (r == Z_DATA_ERROR) {
					s->mode = BAD;
				}
				goto Abort;
			}
			s->sub.trees.index = 0;
			s->mode = DTREE;
		case DTREE:
			while (t = s->sub.trees.table,
				s->sub.trees.index < 258 + (t & 0x1f) + ((t >> 5) & 0x1f)) {
				InflateHuft_t *h;
				Word i, j, c;

				t = s->sub.trees.bb;
				while (k<t) {
					if(!n) {
						goto Abort;
					}
					r=Z_OK;
					--n;
					b|=((Word32)(p[0]))<<k;
					k+=8;
					++p;
				}
				h = s->sub.trees.tb + ((Word)b & inflate_mask[t]);
				t = h->word.what.Bits;
				c = h->base;
				if (c < 16) {
					b>>=t;
					k-=t;
					s->sub.trees.blens[s->sub.trees.index++] = c;
				} else {	/* c == 16..18 */
					if (c==18) {
						i = 7;
						j = 11;
					} else {
						i = c-14;
						j = 3;
					}
					while(k<(t+i)) {
						if (!n) {
							goto Abort;
						}
						r=Z_OK;
						--n;
						b|=((Word32)(*p++))<<k;
						k+=8;
					}
					b>>=t;
					k-=t;
					j += (Word)b & inflate_mask[i];
					b>>=i;
					k-=i;
					i = s->sub.trees.index;
					t = s->sub.trees.table;
					if (i + j > 258 + (t & 0x1f) + ((t >> 5) & 0x1f) ||
						(c == 16 && i < 1)) {
						DeallocAPointer(s->sub.trees.blens);
						s->mode = BAD;
						r = Z_DATA_ERROR;
						goto Abort;
					}
					c = (c == 16) ? s->sub.trees.blens[i - 1] : 0;
					do {
						s->sub.trees.blens[i] = c;
						++i;
					} while (--j);
					s->sub.trees.index = i;
				}
			}
			s->sub.trees.tb = 0;
			{
				Word bl, bd;
				InflateHuft_t *tl,*td;
				InflateCodesState_t *c;

				bl = 9;			/* must be <= 9 for lookahead assumptions */
				bd = 6;			/* must be <= 9 for lookahead assumptions */
				t = s->sub.trees.table;
				t = inflate_trees_dynamic(257 + (t & 0x1f), 1 + ((t >> 5) & 0x1f),
					s->sub.trees.blens, &bl, &bd, &tl, &td,
					s->HuftTbl, z);
				DeallocAPointer(s->sub.trees.blens);
				if (t != Z_OK) {
					if (t == (Word)Z_DATA_ERROR) {
						s->mode = BAD;
					}
					r = t;
					goto Abort;
				}
				if ((c = inflate_codes_new(bl, bd, tl, td, z)) == 0) {
					r = Z_MEM_ERROR;
					goto Abort;
				}
				s->sub.decode.codes = c;
			}
			s->mode = CODES;
		case CODES:
			s->bitb=b;
			s->bitk=k;
			z->PackLen=n;
			z->PackPtr=p;
			s->write=q;
			if ((r = inflate_codes(s, z, r)) != Z_STREAM_END) {
				return inflate_flush(z, r);
			}
			r = Z_OK;
			DeallocAPointer(s->sub.decode.codes);
			p=z->PackPtr;
			n=z->PackLen;
			b=s->bitb;
			k=s->bitk;
			q=s->write;
			m=(Word)(q<s->read?s->read-q-1:s->end-q);
			if (!s->last) {
				s->mode = TYPE;
				break;
			}
			s->mode = DRY;
		case DRY:
			s->write=q;
			r=inflate_flush(z,r);
			q=s->write;
			m=(Word)(q<s->read?s->read-q-1:s->end-q);
			if (s->read != s->write) {
				goto Abort;
			}
			s->mode = DONE;
		case DONE:
			r = Z_STREAM_END;
			goto Abort;
		case BAD:
			r = Z_DATA_ERROR;
			goto Abort;
		default:
			r = Z_STREAM_ERROR;
			goto Abort;
		}
	}
Abort:;
	s->bitb=b;
	s->bitk=k;
	z->PackLen=n;
	z->PackPtr=p;
	s->write=q;
	return inflate_flush(z,r);
}

/**********************************

	Perform a METHOD reset

**********************************/

static int inflateReset(PackState_t *Input)
{
	if (Input->Internal) {
		((InflateState_t *)Input->Internal)->mode = METHOD;
		inflate_blocks_reset(Input, 0);
		return Z_OK;
	}
	return Z_STREAM_ERROR;
}


/**********************************

	Destroy the contents of a PackState_t
	structure

**********************************/

void BURGERCALL DInflateDestroy(PackState_t *Input)
{
	if (Input->Internal) {
		inflate_blocks_reset(Input,0);
		DeallocAPointer(Input->Internal);
		Input->Internal = 0;
	}
}


/**********************************

	Initialize a DInflate state

**********************************/

Word BURGERCALL DInflateInit(PackState_t *Input)
{
	InflateState_t *WorkPtr;
	
	WorkPtr = static_cast<InflateState_t *>(AllocAPointerClear(sizeof(InflateState_t)));
	if (WorkPtr) {
		Input->Internal = WorkPtr;

	/* create inflate_blocks state */

		WorkPtr->BlockState.end = &WorkPtr->BlockState.WindowTbl[1<<MAX_WBITS];
		WorkPtr->BlockState.mode = TYPE;
		inflate_blocks_reset(Input,0);	/* TYPE reset */
		inflateReset(Input);			/* METHOD reset */
		return FALSE;
	}
	return TRUE;
}

/**********************************

	This is the workhorse.
	Call this when you wish to continue
	decompression

**********************************/

Word DInflateMore(PackState_t * z)
{
	int r;					/* Current error state */
	Word Temp;				/* Bit bucket */
	Word32 PackLen;
	Word8 *PackPtr;

	PackLen = z->PackLen;
	PackPtr = z->PackPtr;

	if (z->Internal == 0 || !PackPtr || !PackLen) {
		return (Word)Z_STREAM_ERROR;
	}

	r = Z_BUF_ERROR;		/* Assume a buffer error */
	while (1) {
		switch (((InflateState_t *)z->Internal)->mode) {

		case METHOD:
			if (!PackLen) {
				goto Abort;
			}
			--PackLen;
			Temp = PackPtr[0];
			++PackPtr;
			((InflateState_t *)z->Internal)->sub.method = Temp;
			if ( ((Temp & 0xf) != Z_DEFLATED) ||
				(((Temp >> 4) + 8) > MAX_WBITS)) {

Bad5:
				((InflateState_t *)z->Internal)->mode = XBAD;
				((InflateState_t *)z->Internal)->sub.marker = 5;	/* can't try inflateSync */
				goto DataErr;
			}
			((InflateState_t *)z->Internal)->mode = FLAG;		/* Flag mode */
			r=Z_OK;

		case FLAG:
			if (!PackLen) {
				goto Abort;
			}
			--PackLen;
			Temp = PackPtr[0];
			++PackPtr;
			if (((((InflateState_t *)z->Internal)->sub.method << 8) + Temp) % 31) {
				goto Bad5;
			}
			if (!(Temp & PRESET_DICT)) {
				((InflateState_t *)z->Internal)->mode = BLOCKS;
				r=Z_OK;
				break;
			}
			((InflateState_t *)z->Internal)->mode = DICT4;
			r=Z_OK;
		case DICT4:
			if (!PackLen) {
				goto Abort;
			}
			--PackLen;
			((InflateState_t *)z->Internal)->sub.check.need = (Word32)(PackPtr[0]) << 24;
			++PackPtr;
			((InflateState_t *)z->Internal)->mode = DICT3;
			r=Z_OK;
		case DICT3:
			if (!PackLen) {
				goto Abort;
			}
			--PackLen;
			((InflateState_t *)z->Internal)->sub.check.need += (Word32)(PackPtr[0]) << 16;
			++PackPtr;
			((InflateState_t *)z->Internal)->mode = DICT2;
			r=Z_OK;
		case DICT2:
			if (!PackLen) {
				goto Abort;
			}
			--PackLen;
			((InflateState_t *)z->Internal)->sub.check.need += (Word32)(PackPtr[0]) << 8;
			++PackPtr;
			((InflateState_t *)z->Internal)->mode = DICT1;
			r=Z_OK;
		case DICT1:
			if (!PackLen) {
				goto Abort;
			}
			--PackLen;
			((InflateState_t *)z->Internal)->sub.check.need += (Word32)(PackPtr[0]);
			++PackPtr;
			((InflateState_t *)z->Internal)->mode = DICT0;
			r = Z_NEED_DICT;
			goto Abort;
		case DICT0:
Bad0:
			((InflateState_t *)z->Internal)->mode = XBAD;
			((InflateState_t *)z->Internal)->sub.marker = 0;		/* can try inflateSync */
			r = Z_STREAM_ERROR;
			goto Abort;

		case BLOCKS:
			z->PackLen = PackLen;
			z->PackPtr = PackPtr;
			r = inflate_blocks(z, r);
			PackLen = z->PackLen;
			PackPtr = z->PackPtr;
			if (r == Z_DATA_ERROR) {
				goto Bad0;
			}
			if (r != Z_STREAM_END) {
				goto Abort;
			}
			z->PackLen = PackLen;
			z->PackPtr = PackPtr;
			inflate_blocks_reset(z,&((InflateState_t *)z->Internal)->sub.check.was);
			PackLen = z->PackLen;
			PackPtr = z->PackPtr;
			((InflateState_t *)z->Internal)->mode = CHECK4;
			r = Z_OK;

		case CHECK4:
			if (!PackLen) {
				goto Abort;
			}
			--PackLen;
			((InflateState_t *)z->Internal)->sub.check.need = (Word32)(PackPtr[0]) << 24;
			++PackPtr;
			((InflateState_t *)z->Internal)->mode = CHECK3;
			r=Z_OK;
		case CHECK3:
			if (!PackLen) {
				goto Abort;
			}
			--PackLen;
			((InflateState_t *)z->Internal)->sub.check.need += (Word32)(PackPtr[0]) << 16;
			++PackPtr;
			((InflateState_t *)z->Internal)->mode = CHECK2;
			r=Z_OK;
		case CHECK2:
			if (!PackLen) {
				goto Abort;
			}
			--PackLen;
			((InflateState_t *)z->Internal)->sub.check.need += (Word32)(PackPtr[0]) << 8;
			++PackPtr;
			((InflateState_t *)z->Internal)->mode = CHECK1;
			r=Z_OK;
		case CHECK1:
			if (!PackLen) {
				goto Abort;
			}
			--PackLen;
			((InflateState_t *)z->Internal)->sub.check.need += (Word32)(PackPtr[0]);
			++PackPtr;

			if (((InflateState_t *)z->Internal)->sub.check.was != ((InflateState_t *)z->Internal)->sub.check.need) {
				goto Bad5;
			}
			((InflateState_t *)z->Internal)->mode = XDONE;
		case XDONE:
			r = Z_STREAM_END;
			goto Abort;
		case XBAD:
			goto DataErr;
		default:
			r = Z_STREAM_ERROR;
			goto Abort;
		}
	}
DataErr:;
	r = Z_DATA_ERROR;
Abort:;
	z->PackLen = PackLen;
	z->PackPtr = PackPtr;
	return r;
}


/**********************************

	Decompresses the source buffer into the destination buffer. sourceLen is
	the byte length of the source buffer. Upon entry, destLen is the total
	size of the destination buffer, which must be large enough to hold the
	entire uncompressed data. (The size of the uncompressed data must have
	been saved previously by the compressor and transmitted to the decompressor
	by some mechanism outside the scope of this compression library.)
	Upon exit, destLen is the actual size of the compressed buffer.

	This function can be used to decompress a whole file at once if the
	input file is mmap'ed.

	uncompress returns Z_OK if success, Z_MEM_ERROR if there was not
	enough memory, Z_BUF_ERROR if there was not enough room in the output
	buffer, or Z_DATA_ERROR if the input data was corrupted.

**********************************/

void BURGERCALL DInflateFast(Word8 *dest,Word8 *source,Word32 sourceLen)
{
	PackState_t stream;
	int err;

	stream.PackPtr = (Word8*)source;
	stream.PackLen = sourceLen;
	stream.OutPtr = dest;
	stream.OutLen = 0xFFFFFFFF;

	err = DInflateInit(&stream);
	if (err == Z_OK) {
		err = DInflateMore(&stream);
		DInflateDestroy(&stream);
	}
}

/**********************************

	This is used by the resource manager
	for data decompression

**********************************/

static PackState_t LastStream;

void BURGERCALL DInflate(Word8 *Dest,Word8 *Src,Word32 Length,Word32 PackedLen)
{
	int err;

	if (!PackedLen) {
		if (!Dest) {
			DInflateDestroy(&LastStream);
		}
		return;
	}
	LastStream.PackPtr = Src;
	LastStream.PackLen = PackedLen;
	if (Length) {
		LastStream.OutPtr = Dest;
		LastStream.OutLen = Length;
		err = DInflateInit(&LastStream);
		if (err!=Z_OK) {
			return;
		}
	}
	DInflateMore(&LastStream);
	return;
}

