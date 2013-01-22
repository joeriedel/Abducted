/*
 *  ALGORITHM
 *
 *      The "deflation" process depends on being able to identify portions
 *      of the input text which are identical to earlier input (within a
 *      sliding window trailing behind the input currently being processed).
 *
 *      The most straightforward technique turns out to be the fastest for
 *      most input files: try all possible matches and select the longest.
 *      The key feature of this algorithm is that insertions into the string
 *      dictionary are very simple and thus fast, and deletions are avoided
 *      completely. Insertions are performed at each input character, whereas
 *      string matches are performed only when the previous match ends. So it
 *      is preferable to spend more time in matches to allow very fast string
 *      insertions and avoid deletions. The matching algorithm for small
 *      strings is inspired from that of Rabin & Karp. A brute force approach
 *      is used to find longer strings when a small match has been found.
 *      A similar algorithm is used in comic (by Jan-Mark Wams) and freeze
 *      (by Leonid Broukhis).
 *         A previous version of this file used a more sophisticated algorithm
 *      (by Fiala and Greene) which is guaranteed to run in linear amortized
 *      time, but has a larger average cost, uses more memory and is patented.
 *      However the F&G algorithm may be faster for some highly redundant
 *      files if the parameter max_chain_length (described below) is too large.
 *
 *  ACKNOWLEDGEMENTS
 *
 *      The idea of lazy evaluation of matches is due to Jan-Mark Wams, and
 *      I found it in 'freeze' written by Leonid Broukhis.
 *      Thanks to many people for bug reports and testing.
 *
 *  REFERENCES
 *
 *      Deutsch, L.P.,"DEFLATE Compressed Data Format Specification".
 *      Available in ftp://ds.internic.net/rfc/rfc1951.txt
 *
 *      A description of the Rabin and Karp algorithm is given in the book
 *         "Algorithms" by R. Sedgewick, Addison-Wesley, p252.
 *
 *      Fiala,E.R., and Greene,D.H.
 *         Data Compression with Finite Windows, Comm.ACM, 32,4 (1989) 490-595
 *
 */

#include "PkPack.h"
#include "MmMemory.h"
#include "ClStdLib.h"

#define MAX_WBITS 15
#define MAX_MEM_LEVEL 9

#define Z_NO_FLUSH      0
#define Z_PARTIAL_FLUSH 1 /* will be removed, use Z_SYNC_FLUSH instead */
#define Z_SYNC_FLUSH    2
#define Z_FULL_FLUSH    3
#define Z_FINISH        4
/* Allowed flush values; see deflate() below for details */

#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)
/* Return codes for the compression/decompression functions. Negative
 * values are errors, positive values are used for special but normal events.
 */

#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)
/* compression levels */

#define Z_FILTERED            1
#define Z_HUFFMAN_ONLY        2
#define Z_DEFAULT_STRATEGY    0
/* compression strategy; see deflateInit2() below for details */

#define Z_BINARY   0
#define Z_ASCII    1
#define Z_UNKNOWN  2
/* Possible values of the data_type field */

#define Z_DEFLATED   8

#define TOO_FAR 4096	/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */
#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)	/* Minimum amount of lookahead, except at the end of the input file. */

typedef struct ZStream_t {
    Word8 *next_in;		/* next input byte */
    Word32 avail_in;	/* number of bytes available at next_in */
    Word32 total_in;	/* total nb of input bytes read so far */

    Word8 *next_out;		/* next output byte should be put there */
    Word32 avail_out;	/* remaining free space at next_out */
    Word32 total_out;	/* total nb of bytes output so far */

    void *state;		/* not visible by applications */
	Word32 adler;		/* adler32 value of the uncompressed data */
} ZStream_t;

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
/* The three kinds of block type */

#define MIN_MATCH  3
#define MAX_MATCH  258
/* The minimum and maximum match lengths */

#define PRESET_DICT 0x20	/* preset dictionary flag in zlib header */
#define LENGTH_CODES 29		/* number of length codes, not counting the special END_BLOCK code */
#define LITERALS  256		/* number of literal bytes 0..255 */
#define L_CODES (LITERALS+1+LENGTH_CODES)	/* number of Literal or Length codes, including the END_BLOCK code */
#define D_CODES   30		/* number of distance codes */
#define BL_CODES  19		/* number of codes used to transfer the bit lengths */
#define HEAP_SIZE (2*L_CODES+1)	/* maximum heap size */
#define MAX_BITS 15			/* All codes must not exceed MAX_BITS bits */
#define INIT_STATE    42
#define BUSY_STATE   113
#define FINISH_STATE 666
/* Stream status */


/* Data structure describing a single value and its code string. */
typedef struct CodeData_t {
    union {
        Word16 freq;       /* frequency count */
        Word16 code;       /* bit string */
    } fc;
    union {
        Word16 dad;        /* father node in Huffman tree */
        Word16 len;        /* length of bit string */
    } dl;
} CodeData_t;

typedef struct StaticTreeDesc_t {
	const CodeData_t *static_tree;	/* static tree or NULL */
	const int *extra_bits;			/* extra bits for each code or NULL */
	int extra_base;					/* base index for extra_bits */
	int elems;						/* max number of elements in the tree */
	int max_length;					/* max bit length for the codes */
} StaticTreeDesc_t;

typedef struct TreeDesc_t {
    CodeData_t *dyn_tree;			/* the dynamic tree */
    int max_code;					/* largest code with non zero frequency */
    const StaticTreeDesc_t *stat_desc;	/* the corresponding static tree */
} TreeDesc_t;

/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. Word is used only for parameter passing.
 */

typedef struct DeflateState_t {
	ZStream_t *strm;		/* pointer back to this zlib stream */
	Word8 *pending_buf;		/* output still pending */
	Word32 pending_buf_size;	/* size of pending_buf */
	Word8 *pending_out;		/* next pending byte to output to the stream */
	int status;				/* as the name implies */
	int pending;			/* nb of bytes in the pending buffer */
	int noheader;			/* suppress zlib header and adler32 */
	int last_flush;			/* value of flush param for previous deflate call */

	/* used by deflate.c: */

	Word8 *window;

	/* Sliding window. Input bytes are read into the second half of the window,
	* and move to the first half later to keep a dictionary of at least wSize
	* bytes. With this organization, matches are limited to a distance of
	* wSize-MAX_MATCH bytes, but this ensures that IO is always
	* performed with a length multiple of the block size. Also, it limits
	* the window size to 64K, which is quite useful on MSDOS.
	* To do: use the user input buffer as sliding window.
	*/

	Word32 window_size;
	/* Actual size of window: 2*wSize, except when the user input buffer
	* is directly used as sliding window.
	*/

	Word16 *prev;
	/* Link to older string with same hash index. To limit the size of this
	* array to 64K, this link is maintained only for the last 32K strings.
	* An index in this array is thus a window index modulo 32K.
	*/

	Word16 *head; /* Heads of the hash chains or 0. */

	Word ins_h;          /* hash index of string to be inserted */
	Word hash_size;      /* number of elements in hash table */
	Word hash_bits;      /* log2(hash_size) */
	Word hash_mask;      /* hash_size-1 */

	Word match_length;           /* length of best match */
	Word hash_shift;
	/* Number of bits by which ins_h must be shifted at each input
	* step. It must be such that after MIN_MATCH steps, the oldest
	* byte no longer takes part in the hash key, that is:
	*   hash_shift * MIN_MATCH >= hash_bits
	*/

	long block_start;
	/* Window position at the beginning of the current output block. Gets
	* negative when the window is moved backwards.
	*/

	Word prev_match;             /* previous match */
	int match_available;         /* set if previous match exists */
	Word strstart;               /* start of string to insert */
	Word match_start;            /* start of matching string */
	Word lookahead;              /* number of valid bytes ahead in window */
    Word prev_length;
    
    /* Length of the best match at previous step. Matches not greater than this
     * are discarded. This is used in the lazy match evaluation.
     */

    Word max_chain_length;
    /* To speed up deflation, hash chains are never searched beyond this
     * length.  A higher limit improves compression ratio but degrades the
     * speed.
     */

    Word max_lazy_match;
    /* Attempt to find a better match only when the current match is strictly
     * smaller than this value. This mechanism is used only for compression
     * levels >= 4.
     */
#define max_insert_length  max_lazy_match
    /* Insert new strings in the hash table only if the match length is not
     * greater than this length. This saves time but degrades compression.
     * max_insert_length is used only for compression levels <= 3.
     */

    Word good_match;
    Word w_size;        /* LZ77 window size (32K by default) */
    Word w_bits;        /* log2(w_size)  (8..16) */
    Word w_mask;        /* w_size - 1 */
    /* Use a faster search when the previous match is longer than this */

    int nice_match; /* Stop searching when current match exceeds this */
    Word lit_bufsize;

                /* used by trees.c: */
    /* Didn't use ct_data typedef below to supress compiler warning */
    CodeData_t dyn_ltree[HEAP_SIZE];   /* literal and length tree */
	CodeData_t dyn_dtree[2*D_CODES+1]; /* distance tree */
	CodeData_t bl_tree[2*BL_CODES+1];  /* Huffman tree for bit lengths */

	TreeDesc_t l_desc;				/* desc. for literal tree */
	TreeDesc_t d_desc;				/* desc. for distance tree */
	TreeDesc_t bl_desc;				/* desc. for bit length tree */

    Word16 bl_count[MAX_BITS+1];		/* MAX_BIT = 15, so this is long aligned */

    /* number of codes at each bit length for an optimal tree */

    int heap[2*L_CODES+1];      /* heap used to build the Huffman trees */
    int heap_len;               /* number of elements in the heap */
    int heap_max;               /* element of largest frequency */
    /* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
     * The same heap array is used to build all trees. */

    Word8 depth[2*L_CODES+1];	/* Depth of each subtree used as tie breaker for trees of equal frequency */
    Word8 data_type;			/* UNKNOWN, BINARY or ASCII */
    Word8 method;			/* STORED (for zip only) or DEFLATED */
    Word8 *l_buf;          /* buffer for literals or lengths */

    /* Size of match buffer for literals/lengths.  There are 4 reasons for
     * limiting lit_bufsize to 64K:
     *   - frequencies can be kept in 16 bit counters
     *   - if compression is not successful for the first block, all input
     *     data is still in the window so we can still emit a stored block even
     *     when input comes from standard input.  (This can also be done for
     *     all blocks if lit_bufsize is not greater than 32K.)
     *   - if compression is not successful for a file smaller than 64K, we can
     *     even emit a stored file instead of a stored block (saving 5 bytes).
     *     This is applicable only for zip (not gzip or zlib).
     *   - creating new Huffman trees less frequently may not provide fast
     *     adaptation to changes in the input data statistics. (Take for
     *     example a binary file with poorly compressible code followed by
     *     a highly compressible string table.) Smaller buffer sizes give
     *     fast adaptation but have of course the overhead of transmitting
     *     trees more frequently.
     *   - I can't count above 4
     */

    Word last_lit;      /* running index in l_buf */
    int bi_valid;

    Word16 *d_buf;
    /* Buffer for distances. To simplify the code, d_buf and l_buf have
     * the same number of elements. To use different lengths, an extra flag
     * array would be necessary.
     */

    Word32 opt_len;        /* bit length of current block with optimal trees */
    Word32 static_len;     /* bit length of current block with static trees */
    Word matches;       /* number of string matches in current block */
    int last_eob_len;   /* bit length of EOB code for last block */

    /* Output buffer. bits are inserted starting at the bottom (least
     * significant bits).
     */
	Word bi_buf;
    /* Number of valid bits in bi_buf.  All bits above the last valid bit
     * are always zero.
     */

} DeflateState_t;

typedef enum {
    need_more,      /* block not completed, need more input or more output */
    block_done,     /* block flush performed */
    finish_started, /* finish started, need only more output at next deflate */
    finish_done     /* finish done, accept no more input or output */
} block_state;

#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#define MAX_DIST(s)  ((s)->w_size-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

#define d_code(dist) ((dist) < 256 ? _dist_code[dist] : _dist_code[256+((dist)>>7)])
/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. _dist_code[256] and _dist_code[257] are never
 * used.
 */

/* Inline versions of _tr_tally for speed: */

#define _tr_tally_lit(s, c, flush) \
	{ Word cc = (c); \
	s->d_buf[s->last_lit] = 0; \
	s->l_buf[s->last_lit] = (Word8)cc; \
	++s->last_lit; \
	++s->dyn_ltree[cc].fc.freq; \
	flush = (s->last_lit == s->lit_bufsize-1); \
	}
#define _tr_tally_dist(s, distance, length, flush) \
	{ Word len = (length); \
	Word dist = (distance); \
	s->d_buf[s->last_lit] = static_cast<Word16>(dist); \
	s->l_buf[s->last_lit] = static_cast<Word8>(len); \
	++s->last_lit; \
	--dist; \
	s->dyn_ltree[_length_code[len]+LITERALS+1].fc.freq++; \
	s->dyn_dtree[d_code(dist)].fc.freq++; \
	flush = (s->last_lit == s->lit_bufsize-1); \
	}

#define MAX_BL_BITS 7		/* Bit length codes must not exceed MAX_BL_BITS bits */
#define END_BLOCK 256		/* end of block literal code */
#define REP_3_6      16		/* repeat previous bit length 3-6 times (2 bits of repeat count) */
#define REPZ_3_10    17		/* repeat a zero length 3-10 times  (3 bits of repeat count) */
#define REPZ_11_138  18		/* repeat a zero length 11-138 times  (7 bits of repeat count) */
static const int extra_lbits[LENGTH_CODES] /* extra bits for each length code */
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

static const int extra_dbits[D_CODES] /* extra bits for each distance code */
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static const int extra_blbits[BL_CODES]/* extra bits for each bit length code */
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

static const Word8 bl_order[BL_CODES]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
/* The lengths of the bit length codes are sent in order of decreasing
 * probability, to avoid transmitting the lengths for unused bit length codes.
 */

#define Buf_size (8 * 2*sizeof(char))
/* Number of bits used within bi_buf. (bi_buf might be implemented on
 * more than 16 bits on some systems.)
 */

/* ===========================================================================
 * Local data. These are initialized only once.
 */

#define DIST_CODE_LEN  512 /* see definition of array dist_code below */

/* header created automatically with -DGEN_TREES_H */

static const CodeData_t static_ltree[L_CODES+2] = {
{{ 12},{  8}}, {{140},{  8}}, {{ 76},{  8}}, {{204},{  8}}, {{ 44},{  8}},
{{172},{  8}}, {{108},{  8}}, {{236},{  8}}, {{ 28},{  8}}, {{156},{  8}},
{{ 92},{  8}}, {{220},{  8}}, {{ 60},{  8}}, {{188},{  8}}, {{124},{  8}},
{{252},{  8}}, {{  2},{  8}}, {{130},{  8}}, {{ 66},{  8}}, {{194},{  8}},
{{ 34},{  8}}, {{162},{  8}}, {{ 98},{  8}}, {{226},{  8}}, {{ 18},{  8}},
{{146},{  8}}, {{ 82},{  8}}, {{210},{  8}}, {{ 50},{  8}}, {{178},{  8}},
{{114},{  8}}, {{242},{  8}}, {{ 10},{  8}}, {{138},{  8}}, {{ 74},{  8}},
{{202},{  8}}, {{ 42},{  8}}, {{170},{  8}}, {{106},{  8}}, {{234},{  8}},
{{ 26},{  8}}, {{154},{  8}}, {{ 90},{  8}}, {{218},{  8}}, {{ 58},{  8}},
{{186},{  8}}, {{122},{  8}}, {{250},{  8}}, {{  6},{  8}}, {{134},{  8}},
{{ 70},{  8}}, {{198},{  8}}, {{ 38},{  8}}, {{166},{  8}}, {{102},{  8}},
{{230},{  8}}, {{ 22},{  8}}, {{150},{  8}}, {{ 86},{  8}}, {{214},{  8}},
{{ 54},{  8}}, {{182},{  8}}, {{118},{  8}}, {{246},{  8}}, {{ 14},{  8}},
{{142},{  8}}, {{ 78},{  8}}, {{206},{  8}}, {{ 46},{  8}}, {{174},{  8}},
{{110},{  8}}, {{238},{  8}}, {{ 30},{  8}}, {{158},{  8}}, {{ 94},{  8}},
{{222},{  8}}, {{ 62},{  8}}, {{190},{  8}}, {{126},{  8}}, {{254},{  8}},
{{  1},{  8}}, {{129},{  8}}, {{ 65},{  8}}, {{193},{  8}}, {{ 33},{  8}},
{{161},{  8}}, {{ 97},{  8}}, {{225},{  8}}, {{ 17},{  8}}, {{145},{  8}},
{{ 81},{  8}}, {{209},{  8}}, {{ 49},{  8}}, {{177},{  8}}, {{113},{  8}},
{{241},{  8}}, {{  9},{  8}}, {{137},{  8}}, {{ 73},{  8}}, {{201},{  8}},
{{ 41},{  8}}, {{169},{  8}}, {{105},{  8}}, {{233},{  8}}, {{ 25},{  8}},
{{153},{  8}}, {{ 89},{  8}}, {{217},{  8}}, {{ 57},{  8}}, {{185},{  8}},
{{121},{  8}}, {{249},{  8}}, {{  5},{  8}}, {{133},{  8}}, {{ 69},{  8}},
{{197},{  8}}, {{ 37},{  8}}, {{165},{  8}}, {{101},{  8}}, {{229},{  8}},
{{ 21},{  8}}, {{149},{  8}}, {{ 85},{  8}}, {{213},{  8}}, {{ 53},{  8}},
{{181},{  8}}, {{117},{  8}}, {{245},{  8}}, {{ 13},{  8}}, {{141},{  8}},
{{ 77},{  8}}, {{205},{  8}}, {{ 45},{  8}}, {{173},{  8}}, {{109},{  8}},
{{237},{  8}}, {{ 29},{  8}}, {{157},{  8}}, {{ 93},{  8}}, {{221},{  8}},
{{ 61},{  8}}, {{189},{  8}}, {{125},{  8}}, {{253},{  8}}, {{ 19},{  9}},
{{275},{  9}}, {{147},{  9}}, {{403},{  9}}, {{ 83},{  9}}, {{339},{  9}},
{{211},{  9}}, {{467},{  9}}, {{ 51},{  9}}, {{307},{  9}}, {{179},{  9}},
{{435},{  9}}, {{115},{  9}}, {{371},{  9}}, {{243},{  9}}, {{499},{  9}},
{{ 11},{  9}}, {{267},{  9}}, {{139},{  9}}, {{395},{  9}}, {{ 75},{  9}},
{{331},{  9}}, {{203},{  9}}, {{459},{  9}}, {{ 43},{  9}}, {{299},{  9}},
{{171},{  9}}, {{427},{  9}}, {{107},{  9}}, {{363},{  9}}, {{235},{  9}},
{{491},{  9}}, {{ 27},{  9}}, {{283},{  9}}, {{155},{  9}}, {{411},{  9}},
{{ 91},{  9}}, {{347},{  9}}, {{219},{  9}}, {{475},{  9}}, {{ 59},{  9}},
{{315},{  9}}, {{187},{  9}}, {{443},{  9}}, {{123},{  9}}, {{379},{  9}},
{{251},{  9}}, {{507},{  9}}, {{  7},{  9}}, {{263},{  9}}, {{135},{  9}},
{{391},{  9}}, {{ 71},{  9}}, {{327},{  9}}, {{199},{  9}}, {{455},{  9}},
{{ 39},{  9}}, {{295},{  9}}, {{167},{  9}}, {{423},{  9}}, {{103},{  9}},
{{359},{  9}}, {{231},{  9}}, {{487},{  9}}, {{ 23},{  9}}, {{279},{  9}},
{{151},{  9}}, {{407},{  9}}, {{ 87},{  9}}, {{343},{  9}}, {{215},{  9}},
{{471},{  9}}, {{ 55},{  9}}, {{311},{  9}}, {{183},{  9}}, {{439},{  9}},
{{119},{  9}}, {{375},{  9}}, {{247},{  9}}, {{503},{  9}}, {{ 15},{  9}},
{{271},{  9}}, {{143},{  9}}, {{399},{  9}}, {{ 79},{  9}}, {{335},{  9}},
{{207},{  9}}, {{463},{  9}}, {{ 47},{  9}}, {{303},{  9}}, {{175},{  9}},
{{431},{  9}}, {{111},{  9}}, {{367},{  9}}, {{239},{  9}}, {{495},{  9}},
{{ 31},{  9}}, {{287},{  9}}, {{159},{  9}}, {{415},{  9}}, {{ 95},{  9}},
{{351},{  9}}, {{223},{  9}}, {{479},{  9}}, {{ 63},{  9}}, {{319},{  9}},
{{191},{  9}}, {{447},{  9}}, {{127},{  9}}, {{383},{  9}}, {{255},{  9}},
{{511},{  9}}, {{  0},{  7}}, {{ 64},{  7}}, {{ 32},{  7}}, {{ 96},{  7}},
{{ 16},{  7}}, {{ 80},{  7}}, {{ 48},{  7}}, {{112},{  7}}, {{  8},{  7}},
{{ 72},{  7}}, {{ 40},{  7}}, {{104},{  7}}, {{ 24},{  7}}, {{ 88},{  7}},
{{ 56},{  7}}, {{120},{  7}}, {{  4},{  7}}, {{ 68},{  7}}, {{ 36},{  7}},
{{100},{  7}}, {{ 20},{  7}}, {{ 84},{  7}}, {{ 52},{  7}}, {{116},{  7}},
{{  3},{  8}}, {{131},{  8}}, {{ 67},{  8}}, {{195},{  8}}, {{ 35},{  8}},
{{163},{  8}}, {{ 99},{  8}}, {{227},{  8}}
};

static const CodeData_t static_dtree[D_CODES] = {
{{ 0},{ 5}}, {{16},{ 5}}, {{ 8},{ 5}}, {{24},{ 5}}, {{ 4},{ 5}},
{{20},{ 5}}, {{12},{ 5}}, {{28},{ 5}}, {{ 2},{ 5}}, {{18},{ 5}},
{{10},{ 5}}, {{26},{ 5}}, {{ 6},{ 5}}, {{22},{ 5}}, {{14},{ 5}},
{{30},{ 5}}, {{ 1},{ 5}}, {{17},{ 5}}, {{ 9},{ 5}}, {{25},{ 5}},
{{ 5},{ 5}}, {{21},{ 5}}, {{13},{ 5}}, {{29},{ 5}}, {{ 3},{ 5}},
{{19},{ 5}}, {{11},{ 5}}, {{27},{ 5}}, {{ 7},{ 5}}, {{23},{ 5}}
};

static const Word8 _dist_code[DIST_CODE_LEN] = {
 0,  1,  2,  3,  4,  4,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8,
 8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10, 10, 10, 10, 10,
10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13,
13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15,
15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  0,  0, 16, 17,
18, 18, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22,
23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27,
27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29
};

static const Word8 _length_code[MAX_MATCH-MIN_MATCH+1]= {
 0,  1,  2,  3,  4,  5,  6,  7,  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 12, 12,
13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16,
17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19,
19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22,
22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23,
23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26,
26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 28
};

static const int base_length[LENGTH_CODES] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 32, 40, 48, 56,
64, 80, 96, 112, 128, 160, 192, 224, 0
};

static const int base_dist[D_CODES] = {
    0,     1,     2,     3,     4,     6,     8,    12,    16,    24,
   32,    48,    64,    96,   128,   192,   256,   384,   512,   768,
 1024,  1536,  2048,  3072,  4096,  6144,  8192, 12288, 16384, 24576
};

static const StaticTreeDesc_t static_l_desc =
	{static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_BITS};

static const StaticTreeDesc_t static_d_desc =
	{static_dtree, extra_dbits, 0,          D_CODES, MAX_BITS};

static const StaticTreeDesc_t static_bl_desc =
	{(const CodeData_t *)0, extra_blbits, 0,   BL_CODES, MAX_BL_BITS};


#define send_code(s, c, tree) send_bits(s, tree[c].fc.code, tree[c].dl.len)
   /* Send a code of the given tree. c and tree must not have side effects */

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
#define send_bits(s, value, length) \
	{ int len = (length); \
	int val = (value); \
	Word Bits; \
	Bits = s->bi_buf | (val<<s->bi_valid);\
	if (s->bi_valid > (int)(Buf_size - len)) {\
		Word8 *SendPtr; \
		Word index; \
		index = s->pending; \
		SendPtr = &s->pending_buf[index]; \
		s->pending = index+2; \
		SendPtr[0] = ((Word8)(Bits & 0xff)); \
		SendPtr[1] = ((Word8)(Bits >> 8)); \
		Bits = (Word16)val >> (Buf_size - s->bi_valid);\
		len -= Buf_size;\
	} \
	s->bi_buf = Bits; \
	s->bi_valid += len;\
}

#define MAX(a,b) (a >= b ? a : b)
/* the arguments must not have side effects */

/**********************************

	Reverse the first len bits of a code, using straightforward code (a faster
	* method would use a table)
	* IN assertion: 1 <= len <= 15

**********************************/

static Word bi_reverse(Word code,Word len)
{
	Word res;
	res = 0;
	do {
		res = res+res;
		res += (code & 1);
		code >>= 1;
	} while (--len);
	return res;
}


/**********************************

	Put a short in the pending buffer. The 16-bit value is put in MSB order.
	IN assertion: the stream state is correct and there is enough room in
	pending_buf.

**********************************/

static void putShortMSB(DeflateState_t *s,Word b)
{
	Word8 *Output;
	Word i;
	i = s->pending;
	Output=&s->pending_buf[i];
	s->pending = i+2;
	Output[0] = (Word8)(b>>8);
	Output[1] = (Word8)(b);
}

/**********************************

	Flush the bit buffer, keeping at most 7 bits in it.

**********************************/

static void bi_flush(DeflateState_t *s)
{
	Word8 *Output;
	Word i;
	Word Bucket;
	Word Count;
	Count = s->bi_valid;
	if (Count>=8) {
		i = s->pending;
		Output=&s->pending_buf[i];
		++i;
		Bucket = s->bi_buf;
		Output[0] = (Word8)Bucket;
		Count -= 8;
		Bucket = Bucket>>8;
		if (Count == 8) {			/* Clear it out completely? */
			Output[1] = (Word8)Bucket;
			Count = 0;
			Bucket = 0;
			++i;
		}
		s->bi_valid = Count;
		s->bi_buf = Bucket;
		s->pending = i;
	}
}

/**********************************

	Flush the bit buffer and align the output on a byte boundary

**********************************/

static void bi_windup(DeflateState_t *s)
{
	Word8 *Output;
	Word i;
	Word Bucket;
	Word Count;

	Count = s->bi_valid;
	if (Count) {
		i = s->pending;
		Output=&s->pending_buf[i];
		++i;
		Bucket = s->bi_buf;
		Output[0] = (Word8)Bucket;
		if (Count > 8) {			/* Clear it out completely? */
			Output[1] = (Word8)(Bucket>>8);
			++i;
		}
		s->pending = i;
	}
	s->bi_valid = 0;
	s->bi_buf = 0;
}

/**********************************

	Copy a stored block, storing first the length and its
	one's complement if requested.

**********************************/

static void copy_block(DeflateState_t *s,char * buf,Word len)
{
	Word8 *Output;
	Word i;
	
	bi_windup(s);        /* align on byte boundary */
	s->last_eob_len = 8; /* enough lookahead for inflate */

	i = s->pending;
	Output=&s->pending_buf[i];
	{
		Word Temp2;
		Output[0] = (Word8)(len);
		Output[1] = (Word8)(len >> 8);
		Temp2 = ~len;
		Output[2] = (Word8)(Temp2);
		Output[3] = (Word8)(Temp2 >> 8);
	}
	s->pending = i+4+len;
	if (len) {
		Output+=4;
		do {
			Output[0] = buf[0];
			++buf;
			++Output;
		} while (--len);
	}
}

/**********************************

	Set the data type to ASCII or BINARY, using a crude approximation:
	binary if more than 20% of the bytes are <= 6 or >= 128, ascii otherwise.
	IN assertion: the fields freq of dyn_ltree are set and the total of all
	frequencies does not exceed 64K (to fit in an int on 16 bit machines).

**********************************/

static void set_data_type(DeflateState_t *s)
{
	Word n;
	Word ascii_freq;
	Word bin_freq;

	n = 0;
	ascii_freq = 0;
	bin_freq = 0;
	do {
		bin_freq += s->dyn_ltree[n].fc.freq;
	} while (++n<7);

	do {
		ascii_freq += s->dyn_ltree[n].fc.freq;
	} while (++n<128);
	
	do {
		bin_freq += s->dyn_ltree[n].fc.freq;
	} while (++n<LITERALS);
	s->data_type = (Word8)(bin_freq > (ascii_freq >> 2) ? Z_BINARY : Z_ASCII);
}

/**********************************

	Init a new deflate block

**********************************/

static void init_block(DeflateState_t *s)
{
    Word n; /* iterates over tree elements */

	/* Initialize the trees. */
	n = 0;
	do {
		s->dyn_ltree[n].fc.freq = 0;
	} while (++n<L_CODES);
	n = 0;
	do {
		s->dyn_dtree[n].fc.freq = 0;
	} while (++n<D_CODES);
	n = 0;
	do {
		s->bl_tree[n].fc.freq = 0;
	} while (++n<BL_CODES);

	s->dyn_ltree[END_BLOCK].fc.freq = 1;
	s->opt_len = s->static_len = 0L;
	s->last_lit = s->matches = 0;
}

/**********************************

	Initialize the tree data structures for a new zlib stream.

**********************************/

static void _tr_init(DeflateState_t *s)
{
    s->l_desc.dyn_tree = s->dyn_ltree;
    s->l_desc.stat_desc = &static_l_desc;

    s->d_desc.dyn_tree = s->dyn_dtree;
    s->d_desc.stat_desc = &static_d_desc;

    s->bl_desc.dyn_tree = s->bl_tree;
    s->bl_desc.stat_desc = &static_bl_desc;

    s->bi_buf = 0;
    s->bi_valid = 0;
    s->last_eob_len = 8; /* enough lookahead for inflate */

    /* Initialize the first block of the first file: */
    init_block(s);
}

#define SMALLEST 1
/* Index within the heap array of least frequent node in the Huffman tree */

/**********************************

	Restore the heap property by moving down the tree starting at node k,
	exchanging a node with the smallest of its two sons if necessary, stopping
	when the heap property is re-established (each father smaller than its
	two sons).

**********************************/

static void pqdownheap(DeflateState_t *s, CodeData_t *tree,int k)
{
	int v;
	int j;  /* left son of k */
	int *HeapPtr;
	int n,m;
	Word8 *DepthPtr;

	HeapPtr = s->heap;
	v = HeapPtr[k];
	j = k << 1;  /* left son of k */
	DepthPtr = s->depth;
	if (j <= s->heap_len) {
		do {
			/* Set j to the smallest of the two sons: */
			n = HeapPtr[j+1];
			m = HeapPtr[j];
			if ((j < s->heap_len) &&
				((tree[n].fc.freq < tree[m].fc.freq) ||
				(tree[n].fc.freq == tree[m].fc.freq && DepthPtr[n] <= DepthPtr[m]))) {
				++j;
/*				m = HeapPtr[j];	*/	/* See below */
				m = n;				/* n == HeapPtr[j+1] */
			}
			/* Exit if v is smaller than both sons */
			if (((tree[v].fc.freq < tree[m].fc.freq) ||
				(tree[v].fc.freq == tree[m].fc.freq && DepthPtr[v] <= DepthPtr[m]))) {
				break;
			}
			/* Exchange v with the smallest son */
			HeapPtr[k] = HeapPtr[j]; 
			k = j;

			/* And continue down the tree, setting j to the left son of k */
			j <<= 1;
		} while (j <= s->heap_len);
	}
	HeapPtr[k] = v;
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
static void compress_block(DeflateState_t *s, CodeData_t *ltree, CodeData_t *dtree)
{
    Word dist;      /* distance of matched string */
    int lc;             /* match length or unmatched char (if dist == 0) */
    Word lx = 0;    /* running index in l_buf */
    Word code;      /* the code to send */
    int extra;          /* number of extra bits to send */

    if (s->last_lit != 0) do {
        dist = s->d_buf[lx];
        lc = s->l_buf[lx++];
        if (dist == 0) {
            send_code(s, lc, ltree); /* send a literal byte */
        } else {
            /* Here, lc is the match length - MIN_MATCH */
            code = _length_code[lc];
            send_code(s, code+LITERALS+1, ltree); /* send the length code */
            extra = extra_lbits[code];
            if (extra != 0) {
                lc -= base_length[code];
                send_bits(s, lc, extra);       /* send the extra length bits */
            }
            dist--; /* dist is now the match distance - 1 */
            code = d_code(dist);

            send_code(s, code, dtree);       /* send the distance code */
            extra = extra_dbits[code];
            if (extra != 0) {
                dist -= base_dist[code];
                send_bits(s, dist, extra);   /* send the extra distance bits */
            }
        } /* literal or match pair ? */

        /* Check that the overlay between pending_buf and d_buf+l_buf is ok: */

    } while (lx < s->last_lit);

    send_code(s, END_BLOCK, ltree);
    s->last_eob_len = ltree[END_BLOCK].dl.len;
}

/* ===========================================================================
 * Read a new buffer from the current input stream, update the adler32
 * and total number of bytes read.  All deflate() input goes through
 * this function so some applications may wish to modify it to avoid
 * allocating a large strm->next_in buffer and copying from it.
 * (See also flush_pending()).
 */
static int read_buf(ZStream_t* strm, Word8 *buf,Word size)
{
	Word len = strm->avail_in;

	if (len > size) {
		len = size;
	}
	if (len) {
		strm->avail_in  -= len;
		if (!((DeflateState_t *)strm->state)->noheader) {
			strm->adler = CalcMoreAdler(strm->next_in, len,strm->adler);
		}
		FastMemCpy(buf, strm->next_in, len);
		strm->next_in  += len;
		strm->total_in += len;
	}
	return (int)len;
}

/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to to UPDATE_HASH are made with consecutive
 *    input characters, so that a running hash key can be computed from the
 *    previous key instead of complete recalculation each time.
 */
#define UPDATE_HASH(s,h,c) (h = (((h)<<s->hash_shift) ^ (c)) & s->hash_mask)


/* ===========================================================================
 * Insert string str in the dictionary and set match_head to the previous head
 * of the hash chain (the most recent string with same hash key). Return
 * the previous length of the hash chain.
 * If this file is compiled with -DFASTEST, the compression level is forced
 * to 1, and no hash chains are maintained.
 * IN  assertion: all calls to to INSERT_STRING are made with consecutive
 *    input characters and the first MIN_MATCH bytes of str are valid
 *    (except for the last MIN_MATCH-1 bytes of the input file).
 */
#define INSERT_STRING(s, str, match_head) \
   (UPDATE_HASH(s, s->ins_h, s->window[(str) + (MIN_MATCH-1)]), \
    match_head = s->head[s->ins_h], s->prev[(str) & s->w_mask] = static_cast<Word16>(match_head), \
    s->head[s->ins_h] = static_cast<Word16>(str))

/* ===========================================================================
 * Initialize the hash table (avoiding 64K overflow for 16 bit systems).
 * prev[] will be initialized on the fly.
 */
#define CLEAR_HASH(s) \
    s->head[s->hash_size-1] = 0; \
    FastMemSet(reinterpret_cast<Word8 *>(s->head),0,static_cast<Word>((s->hash_size-1)*sizeof(*s->head)));

/* ===========================================================================
 * Fill the window when the lookahead becomes insufficient.
 * Updates strstart and lookahead.
 *
 * IN assertion: lookahead < MIN_LOOKAHEAD
 * OUT assertions: strstart <= window_size-MIN_LOOKAHEAD
 *    At least one byte has been read, or avail_in == 0; reads are
 *    performed for at least two bytes (required for the zip translate_eol
 *    option -- not supported here).
 */
static void fill_window(DeflateState_t *s)
{
	register Word n, m;
	register Word16 *p;
	unsigned more;    /* Amount of free space at the end of the window. */
	Word wsize = s->w_size;

	do {
		more = (unsigned)(s->window_size -(Word32)s->lookahead -(Word32)s->strstart);

		/* Deal with !@#$% 64K limit: */
		if (more == 0 && s->strstart == 0 && s->lookahead == 0) {
			more = wsize;
	
		} else if (more == (unsigned)(-1)) {
			/* Very unlikely, but possible on 16 bit machine if strstart == 0
			* and lookahead == 1 (input done one byte at time)
			*/
			more--;

			/* If the window is almost full and there is insufficient lookahead,
			* move the upper half to the lower one to make room in the upper half.
			*/
		} else if (s->strstart >= wsize+MAX_DIST(s)) {

			FastMemCpy(s->window, s->window+wsize, (unsigned)wsize);
			s->match_start -= wsize;
			s->strstart    -= wsize; /* we now have strstart >= MAX_DIST */
			s->block_start -= (long) wsize;

		/* Slide the hash table (could be avoided with 32 bit values
		at the expense of memory usage). We slide even when level == 0
		to keep the hash table consistent if we switch back to level > 0
		later. (Using level 0 permanently is not an optimal usage of
		zlib, so we don't care about this pathological case.)
		*/
			n = s->hash_size;		
			p = &s->head[n];
			do {
				m = *--p;
				*p = (Word16)(m >= wsize ? m-wsize : 0);
			} while (--n);

			n = wsize;
			p = &s->prev[n];
			do {
				m = *--p;
				*p = (Word16)(m >= wsize ? m-wsize : 0);
				/* If n is not on any hash chain, prev[n] is garbage but
				* its value will never be used.
				*/
			} while (--n);
			more += wsize;
		}
		if (s->strm->avail_in == 0) return;

		/* If there was no sliding:
		*    strstart <= WSIZE+MAX_DIST-1 && lookahead <= MIN_LOOKAHEAD - 1 &&
		*    more == window_size - lookahead - strstart
		* => more >= window_size - (MIN_LOOKAHEAD-1 + WSIZE + MAX_DIST-1)
		* => more >= window_size - 2*WSIZE + 2
		* In the BIG_MEM or MMAP case (not yet supported),
		*   window_size == input_size + MIN_LOOKAHEAD  &&
		*   strstart + s->lookahead <= input_size => more >= MIN_LOOKAHEAD.
		* Otherwise, window_size == 2*WSIZE so more >= 2.
		* If there was sliding, more >= WSIZE. So in all cases, more >= 2.
		*/

		n = read_buf(s->strm, s->window + s->strstart + s->lookahead, more);
		s->lookahead += n;

		/* Initialize the hash value now that we have some input: */
		if (s->lookahead >= MIN_MATCH) {
			s->ins_h = s->window[s->strstart];
			UPDATE_HASH(s, s->ins_h, s->window[s->strstart+1]);
#if MIN_MATCH != 3
			Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
		}
		/* If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
		* but this is not important since only literal bytes will be emitted.
		*/
	} while (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_in != 0);
}

/* ===========================================================================
 * Compute the optimal bit lengths for a tree and update the total bit length
 * for the current block.
 * IN assertion: the fields freq and dad are set, heap[heap_max] and
 *    above are the tree nodes sorted by increasing frequency.
 * OUT assertions: the field len is set to the optimal bit length, the
 *     array bl_count contains the frequencies for each bit length.
 *     The length opt_len is updated; static_len is also updated if stree is
 *     not null.
 */
static void gen_bitlen(DeflateState_t *s, TreeDesc_t *desc)
{
    CodeData_t *tree        = desc->dyn_tree;
    int max_code         = desc->max_code;
    const CodeData_t *stree = desc->stat_desc->static_tree;
    const int *extra    = desc->stat_desc->extra_bits;
    int base             = desc->stat_desc->extra_base;
    int max_length       = desc->stat_desc->max_length;
    int h;              /* heap index */
    int n, m;           /* iterate over the tree elements */
    int bits;           /* bit length */
    int xbits;          /* extra bits */
    Word16 f;              /* frequency */
    int overflow = 0;   /* number of elements with bit length too large */

	bits = 0;
	do {
    	s->bl_count[bits] = 0;
	} while (++bits<=MAX_BITS);
	/* In a first pass, compute the optimal bit lengths (which may
	* overflow in the case of the bit length tree).
	*/
	tree[s->heap[s->heap_max]].dl.len = 0; /* root of the heap */

	for (h = s->heap_max+1; h < HEAP_SIZE; h++) {
		n = s->heap[h];
		bits = tree[tree[n].dl.dad].dl.len + 1;
		if (bits > max_length) {
			bits = max_length;
			overflow++;
		}
		tree[n].dl.len = (Word16)bits;
		/* We overwrite tree[n].dl.dad which is no longer needed */

		if (n > max_code) {
			continue; /* not a leaf node */
		}
		s->bl_count[bits]++;
		xbits = 0;
		if (n >= base) {
			xbits = extra[n-base];
		}
		f = tree[n].fc.freq;
		s->opt_len += (Word32)f * (bits + xbits);
		if (stree) {
			s->static_len += (Word32)f * (stree[n].dl.len + xbits);
		}
	}
    if (overflow == 0) {
    	return;
	}
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    do {
        bits = max_length-1;
        while (s->bl_count[bits] == 0) {
        	bits--;
        }
        s->bl_count[bits]--;      /* move one leaf down the tree */
        s->bl_count[bits+1] += 2; /* move one overflow item as its brother */
        s->bl_count[max_length]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        overflow -= 2;
    } while (overflow > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    for (bits = max_length; bits != 0; bits--) {
        n = s->bl_count[bits];
        while (n != 0) {
            m = s->heap[--h];
            if (m > max_code) continue;
            if (tree[m].dl.len != (Word) bits) {
                s->opt_len += ((long)bits - (long)tree[m].dl.len)
                              *(long)tree[m].fc.freq;
                tree[m].dl.len = (Word16)bits;
            }
            n--;
        }
    }
}


/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
static void gen_codes (CodeData_t *tree, int max_code, Word16 *bl_count)
{
    Word16 next_code[MAX_BITS+1]; /* next code value for each bit length */
    Word16 code = 0;              /* running code value */
    int bits;                  /* bit index */
    int n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    for (bits = 1; bits <= MAX_BITS; bits++) {
        next_code[bits] = code = (code + bl_count[bits-1]) << 1;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */

    for (n = 0;  n <= max_code; n++) {
        int len = tree[n].dl.len;
        if (len) {
	        /* Now reverse the bits */
    	    tree[n].fc.code = static_cast<Word16>(bi_reverse(next_code[len]++, len));
		}
    }
}

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
static void build_tree(DeflateState_t *s,TreeDesc_t *desc)
{
    CodeData_t *tree         = desc->dyn_tree;
    const CodeData_t *stree  = desc->stat_desc->static_tree;
    int elems             = desc->stat_desc->elems;
    int n, m;          /* iterate over heap elements */
    int max_code = -1; /* largest code with non zero frequency */
    int node;          /* new node being created */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n+1].
     * heap[0] is not used.
     */
    s->heap_len = 0, s->heap_max = HEAP_SIZE;

    for (n = 0; n < elems; n++) {
        if (tree[n].fc.freq != 0) {
            s->heap[++(s->heap_len)] = max_code = n;
            s->depth[n] = 0;
        } else {
            tree[n].dl.len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    while (s->heap_len < 2) {
        node = s->heap[++(s->heap_len)] = (max_code < 2 ? ++max_code : 0);
        tree[node].fc.freq = 1;
        s->depth[node] = 0;
        s->opt_len--; if (stree) s->static_len -= stree[node].dl.len;
        /* node is 0 or 1 so it does not have extra bits */
    }
    desc->max_code = max_code;

    /* The elements heap[heap_len/2+1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    for (n = s->heap_len/2; n >= 1; n--) pqdownheap(s, tree, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    node = elems;              /* next internal node of the tree */
    do {
		n = s->heap[SMALLEST];
		s->heap[SMALLEST] = s->heap[s->heap_len--];
		pqdownheap(s, tree, SMALLEST);		/* n = node of least frequency */
        m = s->heap[SMALLEST]; /* m = node of next least frequency */

        s->heap[--(s->heap_max)] = n; /* keep the nodes sorted by frequency */
        s->heap[--(s->heap_max)] = m;

        /* Create a new node father of n and m */
        tree[node].fc.freq = tree[n].fc.freq + tree[m].fc.freq;
        s->depth[node] = (Word8) (MAX(s->depth[n], s->depth[m]) + 1);
        tree[n].dl.dad = tree[m].dl.dad = (Word16)node;
        /* and insert the new node in the heap */
        s->heap[SMALLEST] = node++;
        pqdownheap(s, tree, SMALLEST);

    } while (s->heap_len >= 2);

    s->heap[--(s->heap_max)] = s->heap[SMALLEST];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen(s, (TreeDesc_t *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes ((CodeData_t *)tree, max_code, s->bl_count);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree.
 */
static void scan_tree (DeflateState_t *s, CodeData_t *tree,int max_code)
{
    int n;                     /* iterates over all tree elements */
    int prevlen = -1;          /* last emitted length */
    int curlen;                /* length of current code */
    int nextlen = tree[0].dl.len; /* length of next code */
    int count = 0;             /* repeat count of the current code */
    int max_count = 7;         /* max repeat count */
    int min_count = 4;         /* min repeat count */

    if (nextlen == 0) {
    	max_count = 138;
    	min_count = 3;
    }
    tree[max_code+1].dl.len = (Word16)0xffff; /* guard */

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = tree[n+1].dl.len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            s->bl_tree[curlen].fc.freq += count;
        } else if (curlen != 0) {
            if (curlen != prevlen) s->bl_tree[curlen].fc.freq++;
            s->bl_tree[REP_3_6].fc.freq++;
        } else if (count <= 10) {
            s->bl_tree[REPZ_3_10].fc.freq++;
        } else {
            s->bl_tree[REPZ_11_138].fc.freq++;
        }
        count = 0; prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
        } else {
            max_count = 7, min_count = 4;
        }
    }
}

/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
static int build_bl_tree(DeflateState_t *s)
{
    int max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    scan_tree(s, (CodeData_t *)s->dyn_ltree, s->l_desc.max_code);
    scan_tree(s, (CodeData_t *)s->dyn_dtree, s->d_desc.max_code);

    /* Build the bit length tree: */
    build_tree(s, (TreeDesc_t *)(&(s->bl_desc)));
    /* opt_len now includes the length of the tree representations, except
     * the lengths of the bit lengths codes and the 5+5+4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    for (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        if (s->bl_tree[bl_order[max_blindex]].dl.len != 0) break;
    }
    /* Update opt_len to include the bit length tree and counts */
    s->opt_len += 3*(max_blindex+1) + 5+5+4;

    return max_blindex;
}

/* ===========================================================================
 * Send a stored block
 */
static void _tr_stored_block(DeflateState_t *s,char *buf,Word32 stored_len,int eof)
{
    send_bits(s, (STORED_BLOCK<<1)+eof, 3);  /* send block type */
    copy_block(s, buf, (Word)stored_len); /* with header */
}


/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
static void send_tree (DeflateState_t *s, CodeData_t *tree, int max_code)
{
    int n;                     /* iterates over all tree elements */
    int prevlen = -1;          /* last emitted length */
    int curlen;                /* length of current code */
    int nextlen = tree[0].dl.len; /* length of next code */
    int count = 0;             /* repeat count of the current code */
    int max_count = 7;         /* max repeat count */
    int min_count = 4;         /* min repeat count */

    /* tree[max_code+1].dl.len = -1; */  /* guard already set */
    if (nextlen == 0) {
    	max_count = 138;
    	min_count = 3;
	}
    for (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = tree[n+1].dl.len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            do { send_code(s, curlen, s->bl_tree); } while (--count != 0);

        } else if (curlen != 0) {
            if (curlen != prevlen) {
                send_code(s, curlen, s->bl_tree); count--;
            }
            send_code(s, REP_3_6, s->bl_tree);
            send_bits(s, count-3, 2);

        } else if (count <= 10) {
            send_code(s, REPZ_3_10, s->bl_tree);
            send_bits(s, count-3, 3);

        } else {
            send_code(s, REPZ_11_138, s->bl_tree);
            send_bits(s, count-11, 7);
        }
        count = 0; prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138;
            min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6;
            min_count = 3;
        } else {
            max_count = 7;
            min_count = 4;
        }
    }
}

/* ===========================================================================
 * Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
 */
static void send_all_trees(DeflateState_t *s,int lcodes,int dcodes,int blcodes)
{
    int rank;                    /* index in bl_order */

    send_bits(s, lcodes-257, 5); /* not +255 as stated in appnote.txt */
    send_bits(s, dcodes-1,   5);
    send_bits(s, blcodes-4,  4); /* not -3 as stated in appnote.txt */
    for (rank = 0; rank < blcodes; rank++) {
        send_bits(s, s->bl_tree[bl_order[rank]].dl.len, 3);
    }

    send_tree(s, (CodeData_t *)s->dyn_ltree, lcodes-1); /* literal tree */

    send_tree(s, (CodeData_t *)s->dyn_dtree, dcodes-1); /* distance tree */
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and output the encoded block to the zip file.
 */
static void _tr_flush_block(DeflateState_t *s, char *buf,Word32 stored_len,int eof)
{
    Word32 opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    int max_blindex = 0;  /* index of last bit length code of non zero freq */

    /* Build the Huffman trees unless a stored block is forced */

	 /* Check if the file is ascii or binary */
	if (s->data_type == Z_UNKNOWN) {
		set_data_type(s);
	}
	/* Construct the literal and distance trees */
	build_tree(s, (TreeDesc_t *)(&(s->l_desc)));

	build_tree(s, (TreeDesc_t *)(&(s->d_desc)));
	/* At this point, opt_len and static_len are the total bit lengths of
	 * the compressed block data, excluding the tree representations.
	 */

	/* Build the bit length tree for the above two trees, and get the index
	 * in bl_order of the last bit length code to send.
	 */
	max_blindex = build_bl_tree(s);

	/* Determine the best encoding. Compute first the block length in bytes*/
	opt_lenb = (s->opt_len+3+7)>>3;
	static_lenb = (s->static_len+3+7)>>3;


	if (static_lenb <= opt_lenb) {
		opt_lenb = static_lenb;
    }

    if (stored_len+4 <= opt_lenb && buf != (char*)0) {
                       /* 4: two words for the lengths */
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        _tr_stored_block(s, buf, stored_len, eof);

    } else if (static_lenb == opt_lenb) {
        send_bits(s, (STATIC_TREES<<1)+eof, 3);
        compress_block(s, (CodeData_t *)static_ltree, (CodeData_t *)static_dtree);
    } else {
        send_bits(s, (DYN_TREES<<1)+eof, 3);
        send_all_trees(s, s->l_desc.max_code+1, s->d_desc.max_code+1,
                       max_blindex+1);
        compress_block(s, (CodeData_t *)s->dyn_ltree, (CodeData_t *)s->dyn_dtree);
    }
    /* The above check is made mod 2^32, for files larger than 512 MB
     * and uLong implemented on 32 bits.
     */
    init_block(s);

    if (eof) {
        bi_windup(s);
    }
}

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#define FLUSH_BLOCK_ONLY(s, eof) { \
   _tr_flush_block(s, (s->block_start >= 0L ? \
                   (char *)&s->window[(unsigned)s->block_start] : \
                   (char *)0), \
		(Word32)((long)s->strstart - s->block_start), \
		(eof)); \
   s->block_start = s->strstart; \
   flush_pending(s->strm); \
}

/* Same but force premature exit if necessary. */
#define FLUSH_BLOCK(s, eof) { \
   FLUSH_BLOCK_ONLY(s, eof); \
   if (s->strm->avail_out == 0) return (eof) ? finish_started : need_more; \
}

/* =========================================================================
 * Flush as much pending output as possible. All deflate() output goes
 * through this function so some applications may wish to modify it
 * to avoid allocating a large strm->next_out buffer and copying into it.
 * (See also read_buf()).
 */
static void flush_pending(ZStream_t* strm)
{
    Word len = ((DeflateState_t *)strm->state)->pending;

    if (len > strm->avail_out) {
    	len = strm->avail_out;
    }
    if (len) {
		FastMemCpy(strm->next_out, ((DeflateState_t *)strm->state)->pending_out, len);
		strm->next_out  += len;
		((DeflateState_t *)strm->state)->pending_out  += len;
		strm->total_out += len;
		strm->avail_out  -= len;
		((DeflateState_t *)strm->state)->pending -= len;
		if (((DeflateState_t *)strm->state)->pending == 0) {
			((DeflateState_t *)strm->state)->pending_out = ((DeflateState_t *)strm->state)->pending_buf;
		}
	}
}


/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 * OUT assertion: the match length is not greater than s->lookahead.
 */

static Word longest_match(DeflateState_t *s, Word cur_match)
{
    unsigned chain_length = s->max_chain_length;/* max hash chain length */
    register Word8 *scan = s->window + s->strstart; /* current string */
    register Word8 *match;                       /* matched string */
    register int len;                           /* length of current match */
    int best_len = s->prev_length;              /* best match length so far */
    int nice_match = s->nice_match;             /* stop if match long enough */
    Word limit = s->strstart > (Word)MAX_DIST(s) ?
        s->strstart - (Word)MAX_DIST(s) : 0;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */
    Word16 *prev = s->prev;
    Word wmask = s->w_mask;

    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    register Word8 *strend = s->window + s->strstart + MAX_MATCH - 1;
    register Word16 scan_start = *(Word16*)scan;
    register Word16 scan_end   = *(Word16*)(scan+best_len-1);


    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */

    /* Do not waste too much time if we already have a good match: */
    if (s->prev_length >= s->good_match) {
        chain_length >>= 2;
    }
    /* Do not look for matches beyond the end of the input. This is necessary
     * to make deflate deterministic.
     */
    if ((Word)nice_match > s->lookahead) nice_match = s->lookahead;

    do {
        match = s->window + cur_match;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2:
         */
        /* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
        if (*(Word16*)(match+best_len-1) != scan_end ||
            *(Word16*)match != scan_start) continue;

        /* It is not necessary to compare scan[2] and match[2] since they are
         * always equal when the other bytes match, given that the hash keys
         * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
         * strstart+3, +5, ... up to strstart+257. We check for insufficient
         * lookahead only every 4th comparison; the 128th check will be made
         * at strstart+257. If MAX_MATCH-2 is not a multiple of 8, it is
         * necessary to put more guard bytes at the end of the window, or
         * to check more often for insufficient lookahead.
         */
        scan++, match++;
        do {
        } while (*(Word16*)(scan+=2) == *(Word16*)(match+=2) &&
                 *(Word16*)(scan+=2) == *(Word16*)(match+=2) &&
                 *(Word16*)(scan+=2) == *(Word16*)(match+=2) &&
                 *(Word16*)(scan+=2) == *(Word16*)(match+=2) &&
                 scan < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window+strstart+257 */
        if (*scan == *match) scan++;

        len = (MAX_MATCH - 1) - (int)(strend-scan);
        scan = strend - (MAX_MATCH-1);


        if (len > best_len) {
            s->match_start = cur_match;
            best_len = len;
            if (len >= nice_match) break;
            scan_end = *(Word16*)(scan+best_len-1);
        }
    } while ((cur_match = prev[cur_match & wmask]) > limit
             && --chain_length != 0);

    if ((Word)best_len <= s->lookahead) return (Word)best_len;
    return s->lookahead;
}

/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
static block_state deflate_slow(DeflateState_t *s,int flush)
{
    Word hash_head = 0;    /* head of hash chain */
    int bflush;              /* set if current block must be flushed */

    /* Process the input block. */
    for (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        if (s->lookahead < MIN_LOOKAHEAD) {
            fill_window(s);
            if (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
	        return need_more;
	    }
            if (s->lookahead == 0) break; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart+2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        if (s->lookahead >= MIN_MATCH) {
            INSERT_STRING(s, s->strstart, hash_head);
        }

        /* Find the longest match, discarding those <= prev_length.
         */
        s->prev_length = s->match_length, s->prev_match = s->match_start;
        s->match_length = MIN_MATCH-1;

        if (hash_head != 0 && s->prev_length < s->max_lazy_match &&
            s->strstart - hash_head <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->match_length = longest_match (s, hash_head);
            /* longest_match() sets match_start */

            if (s->match_length <= 5 && (
                 (s->match_length == MIN_MATCH &&
                  s->strstart - s->match_start > TOO_FAR))) {

                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                s->match_length = MIN_MATCH-1;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        if (s->prev_length >= MIN_MATCH && s->match_length <= s->prev_length) {
            Word max_insert = s->strstart + s->lookahead - MIN_MATCH;
            /* Do not insert strings in hash table beyond this. */

            _tr_tally_dist(s, s->strstart -1 - s->prev_match,
			   s->prev_length - MIN_MATCH, bflush);

            /* Insert in hash table all strings up to the end of the match.
             * strstart-1 and strstart are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            s->lookahead -= s->prev_length-1;
            s->prev_length -= 2;
            do {
                if (++s->strstart <= max_insert) {
                    INSERT_STRING(s, s->strstart, hash_head);
                }
            } while (--s->prev_length != 0);
            s->match_available = 0;
            s->match_length = MIN_MATCH-1;
            s->strstart++;

            if (bflush) FLUSH_BLOCK(s, 0);

        } else if (s->match_available) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
	    _tr_tally_lit(s, s->window[s->strstart-1], bflush);
	    if (bflush) {
                FLUSH_BLOCK_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            if (s->strm->avail_out == 0) return need_more;
        } else {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            s->match_available = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    if (s->match_available) {
        _tr_tally_lit(s, s->window[s->strstart-1], bflush);
        s->match_available = 0;
    }
    FLUSH_BLOCK(s, flush == Z_FINISH);
    return flush == Z_FINISH ? finish_done : block_done;
}

/* ===========================================================================
 * Send one empty static block to give enough lookahead for inflate.
 * This takes 10 bits, of which 7 may remain in the bit buffer.
 * The current inflate code requires 9 bits of lookahead. If the
 * last two codes for the previous block (real code plus EOB) were coded
 * on 5 bits or less, inflate may have only 5+3 bits of lookahead to decode
 * the last real code. In this case we send two empty static blocks instead
 * of one. (There are no problems if the previous block is stored or fixed.)
 * To simplify the code, we assume the worst case of last real code encoded
 * on one bit only.
 */
static void _tr_align(DeflateState_t *s)
{
    send_bits(s, STATIC_TREES<<1, 3);
    send_code(s, END_BLOCK, static_ltree);
    bi_flush(s);
    /* Of the 10 bits for the empty block, we have already sent
     * (10 - bi_valid) bits. The lookahead for the last real code (before
     * the EOB of the previous block) was thus at least one plus the length
     * of the EOB plus what we have just sent of the empty static block.
     */
    if (1 + s->last_eob_len + 10 - s->bi_valid < 9) {
        send_bits(s, STATIC_TREES<<1, 3);
        send_code(s, END_BLOCK, static_ltree);
        bi_flush(s);
    }
    s->last_eob_len = 7;
}




/* ===========================================================================
 *  Function prototypes.
 */


/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */
typedef struct config_s {
   Word good_length; /* reduce lazy search above this match length */
   Word max_lazy;    /* do not perform lazy search above this match length */
   Word nice_length; /* quit search above this match length */
   Word max_chain;
} config;

static const config configuration_table = 
	{32, 258, 258, 4096}; /* maximum compression */

/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
 * For deflate_fast() (levels <= 3) good is ignored and lazy has a different
 * meaning.
 */


/* ========================================================================= */
static int deflateEnd (ZStream_t* strm)
{
	int status;

	if (strm == 0 || strm->state == 0) {
		return Z_STREAM_ERROR;
	}
	status = ((DeflateState_t *)strm->state)->status;
	if (status != INIT_STATE && status != BUSY_STATE &&
		status != FINISH_STATE) {
		return Z_STREAM_ERROR;
	}

	/* Deallocate in reverse order of allocations: */
	DeallocAPointer(((DeflateState_t *)strm->state)->pending_buf);
	DeallocAPointer(((DeflateState_t *)strm->state)->head);
	DeallocAPointer(((DeflateState_t *)strm->state)->prev);
	DeallocAPointer(((DeflateState_t *)strm->state)->window);

	DeallocAPointer(strm->state);
	strm->state = 0;

	return status == BUSY_STATE ? Z_DATA_ERROR : Z_OK;
}
/* ===========================================================================
 * Initialize the "longest match" routines for a new zlib stream
 */
static void lm_init (DeflateState_t *s)
{
    s->window_size = (Word32)2L*s->w_size;

    CLEAR_HASH(s);

    /* Set the default configuration parameters:
     */
    s->max_lazy_match   = configuration_table.max_lazy;
    s->good_match       = configuration_table.good_length;
    s->nice_match       = configuration_table.nice_length;
    s->max_chain_length = configuration_table.max_chain;

    s->strstart = 0;
    s->block_start = 0L;
    s->lookahead = 0;
    s->match_length = s->prev_length = MIN_MATCH-1;
    s->match_available = 0;
    s->ins_h = 0;
}

/* ========================================================================= */
static int  deflateReset (ZStream_t* strm)
{
    DeflateState_t *s;

    if (strm == 0 || strm->state == 0) return Z_STREAM_ERROR;

    strm->total_in = strm->total_out = 0;

    s = (DeflateState_t *)strm->state;
    s->pending = 0;
    s->pending_out = s->pending_buf;

    if (s->noheader < 0) {
        s->noheader = 0; /* was set to -1 by deflate(..., Z_FINISH); */
    }
    s->status = s->noheader ? BUSY_STATE : INIT_STATE;
    strm->adler = 1;
    s->last_flush = Z_NO_FLUSH;

    _tr_init(s);
    lm_init(s);

    return Z_OK;
}


/* ========================================================================= */
static int  deflateInit(ZStream_t* strm)
{
	DeflateState_t *s;

    Word16 *overlay;
    /* We overlay pending_buf and d_buf+l_buf. This works since the average
     * output size for (length,distance) codes is <= 24 bits. */
    if (!strm) {
    	return Z_STREAM_ERROR;
	}

    s = (DeflateState_t *)AllocAPointerClear(sizeof(DeflateState_t));
    if (s == 0) return Z_MEM_ERROR;
    strm->state = (DeflateState_t *)s;
    s->strm = strm;

    s->noheader = 0;
    s->w_bits = MAX_WBITS;
    s->w_size = 1 << s->w_bits;
    s->w_mask = s->w_size - 1;

    s->hash_bits = MAX_MEM_LEVEL + 7;
    s->hash_size = 1 << s->hash_bits;
    s->hash_mask = s->hash_size - 1;
    s->hash_shift =  ((s->hash_bits+MIN_MATCH-1)/MIN_MATCH);

    s->window = (Word8 *)AllocAPointerClear(s->w_size*2*sizeof(Word8));
    s->prev = (Word16 *)AllocAPointerClear(s->w_size*sizeof(Word16));
    s->head = (Word16 *)AllocAPointerClear(s->hash_size*sizeof(Word16));

    s->lit_bufsize = 1 << (MAX_MEM_LEVEL + 6); /* 16K elements by default */

    overlay = (Word16 *) AllocAPointerClear(s->lit_bufsize*(sizeof(Word16)+2));
    s->pending_buf = (Word8 *) overlay;
    s->pending_buf_size = (Word32)s->lit_bufsize * (sizeof(Word16)+2L);

    if (s->window == 0 || s->prev == 0 || s->head == 0 ||
        s->pending_buf == 0) {
        deflateEnd (strm);
        return Z_MEM_ERROR;
    }
    s->d_buf = overlay + s->lit_bufsize/sizeof(Word16);
    s->l_buf = s->pending_buf + (1+sizeof(Word16))*s->lit_bufsize;

    s->method = (Word8)Z_DEFLATED;

    return deflateReset(strm);
}

/* ========================================================================= */
static int  deflate (ZStream_t* strm, int flush)
{
	int old_flush; /* value of flush param for previous deflate call */
	DeflateState_t *s;

	if (strm == 0 || strm->state == 0 ||
		flush > Z_FINISH || flush < 0) {
		return Z_STREAM_ERROR;
	}
	s = (DeflateState_t *)strm->state;

	if (strm->next_out == 0 ||
		(strm->next_in == 0 && strm->avail_in != 0) ||
		(s->status == FINISH_STATE && flush != Z_FINISH)) {
		return Z_STREAM_ERROR;
	}
	if (strm->avail_out == 0) return Z_BUF_ERROR;

	s->strm = strm; /* just in case */
	old_flush = s->last_flush;
	s->last_flush = flush;

	/* Write the zlib header */
	if (s->status == INIT_STATE) {

		Word header = (Z_DEFLATED + ((s->w_bits-8)<<4)) << 8;
		Word level_flags = (Z_BEST_COMPRESSION-1) >> 1;

		if (level_flags > 3) level_flags = 3;
		header |= (level_flags << 6);
		if (s->strstart != 0) header |= PRESET_DICT;
		header += 31 - (header % 31);

		s->status = BUSY_STATE;
		putShortMSB(s, header);

		/* Save the adler32 of the preset dictionary: */
		if (s->strstart != 0) {
			putShortMSB(s, (Word)(strm->adler >> 16));
			putShortMSB(s, (Word)(strm->adler & 0xffff));
		}
		strm->adler = 1L;
	}

	/* Flush as much pending output as possible */
	if (s->pending != 0) {
		flush_pending(strm);
		if (strm->avail_out == 0) {
			/* Since avail_out is 0, deflate will be called again with
			* more output space, but possibly with both pending and
			* avail_in equal to zero. There won't be anything to do,
			* but this is not an error situation so make sure we
			* return OK instead of BUF_ERROR at next call of deflate:
			*/
			s->last_flush = -1;
			return Z_OK;
		}

		/* Make sure there is something to do and avoid duplicate consecutive
		* flushes. For repeated and useless calls with Z_FINISH, we keep
		* returning Z_STREAM_END instead of Z_BUFF_ERROR.
		*/
	} else if (strm->avail_in == 0 && flush <= old_flush &&
		flush != Z_FINISH) {
		return Z_BUF_ERROR;
	}

	/* User must not provide more input after the first FINISH: */
	if (s->status == FINISH_STATE && strm->avail_in != 0) {
		return Z_BUF_ERROR;
	}

	/* Start a new block or continue the current one.
	*/
	if (strm->avail_in != 0 || s->lookahead != 0 ||
		(flush != Z_NO_FLUSH && s->status != FINISH_STATE)) {
		block_state bstate;

		bstate = deflate_slow(s, flush);

		if (bstate == finish_started || bstate == finish_done) {
			s->status = FINISH_STATE;
		}
		if (bstate == need_more || bstate == finish_started) {
			if (strm->avail_out == 0) {
				s->last_flush = -1; /* avoid BUF_ERROR next call, see above */
			}
			return Z_OK;
			/* If flush != Z_NO_FLUSH && avail_out == 0, the next call
			* of deflate should use the same flush parameter to make sure
			* that the flush is complete. So we don't have to output an
			* empty block here, this will be done at next call. This also
			* ensures that for a very small output buffer, we emit at most
			* one empty block.
			*/
		}
		if (bstate == block_done) {
			if (flush == Z_PARTIAL_FLUSH) {
				_tr_align(s);
			} else { /* FULL_FLUSH or SYNC_FLUSH */
				_tr_stored_block(s, (char*)0, 0L, 0);
				/* For a full flush, this empty block will be recognized
				* as a special marker by inflate_sync().
				*/
				if (flush == Z_FULL_FLUSH) {
					CLEAR_HASH(s);             /* forget history */
				}
			}
			flush_pending(strm);
			if (strm->avail_out == 0) {
				s->last_flush = -1; /* avoid BUF_ERROR at next call, see above */
				return Z_OK;
			}
		}
	}
	if (flush != Z_FINISH) return Z_OK;
	if (s->noheader) return Z_STREAM_END;

	/* Write the zlib trailer (adler32) */
	putShortMSB(s, (Word)(strm->adler >> 16));
	putShortMSB(s, (Word)(strm->adler & 0xffff));
	flush_pending(strm);
	/* If avail_out is zero, the application will call deflate again
	* to flush the rest.
	*/
	s->noheader = -1; /* write the trailer only once! */
	return s->pending != 0 ? Z_OK : Z_STREAM_END;
}

/* ===========================================================================
     Compresses the source buffer into the destination buffer. The level
   parameter has the same meaning as in deflateInit.  sourceLen is the byte
   length of the source buffer. Upon entry, destLen is the total size of the
   destination buffer, which must be at least 0.1% larger than sourceLen plus
   12 bytes. Upon exit, destLen is the actual size of the compressed buffer.

     compress2 returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_BUF_ERROR if there was not enough room in the output buffer,
   Z_STREAM_ERROR if the level parameter is invalid.
*/

void ** BURGERCALL EncodeInflate(Word8 *source,Word32 sourceLen)
{
	ZStream_t stream;
	int err;
	void **Result;
	Word32 DestLen;
	
	DestLen = sourceLen+(sourceLen/100)+12;
	Result = AllocAHandle(DestLen);
	if (Result) {
		stream.next_in = (Word8*)source;
		stream.avail_in = (Word)sourceLen;
		stream.next_out = (Word8 *)LockAHandle(Result);
		stream.avail_out = DestLen;
	
		if (deflateInit(&stream)==Z_OK) {
			err = deflate(&stream,Z_FINISH);
			deflateEnd(&stream);
			if (err==Z_OK || err==Z_STREAM_END) {
				UnlockAHandle(Result);
				Result = ResizeAHandle(Result,stream.total_out);
				return Result;
			}
		}
		DeallocAHandle(Result);
	}
	return 0;
}


