/**********************************

	Import an XM file

**********************************/

#include "SnMadMusic.h"
#include <BREndian.hpp>
#include "MmMemory.h"
#include "ClStdLib.h"

/***** Specification of the FastTracker 2- (.XM)-Format: *****/

/*
       The XM module format description for XM files version $0104.

       By Mr.H of Triton in 1994.
 ============================================================================

   ******************************
   *   The XM file structure:   *
   ******************************

   Offset Length Type

      0     17   (char) ID text: 'Extended module: '
     17     20   (char) Module name, padded with zeroes
     37      1   (char) $1a
     38     20   (char) Tracker name
     58      2   (word) Version number, hi-byte major and low-byte minor
                        The current format is version $0103

     60      4  (dword) Header size
     +4      2   (word) Song length (in patten order table)
     +6      2   (word) Restart position
     +8      2   (word) Number of channels (2,4,6,8,10,...,32)
    +10      2   (word) Number of patterns (max 256)
    +12      2   (word) Number of instruments (max 128)
    +14      2   (word) Flags: bit 0: 0 = Amiga frequency table (see below);
                                      1 = Linear frequency table
    +16      2   (word) Default tempo
    +18      2   (word) Default BPM
    +20    256   (byte) Pattern order table

                        Patterns:
                        ---------

      ?      4  (dword) Pattern header length
     +4      1   (byte) Packing type (always 0)
     +5      2   (word) Number of rows in pattern (1..256)
     +7      2   (word) Packed patterndata size

      ?      ?          Packed pattern data

                        Instruments:
                        ------------

      ?      4  (dword) Instrument size
     +4     22   (char) Instrument name
    +26      1   (byte) Instrument type (always 0)
    +27      2   (word) Number of samples in instrument

   If the number of samples > 0, then the this will follow:

   !     +29      4  (dword) Sample header size
   !     +33     96   (byte) Sample number for all notes
   !    +129     48   (byte) Points for volume envelope
   !    +177     48   (byte) Points for panning envelope
   !    +225      1   (byte) Number of volume points
   !    +226      1   (byte) Number of panning points
   !    +227      1   (byte) Volume sustain point
   !    +228      1   (byte) Volume loop start point
   !    +229      1   (byte) Volume loop end point
   !    +230      1   (byte) Panning sustain point
   !    +231      1   (byte) Panning loop start point
   !    +232      1   (byte) Panning loop end point
   !    +233      1   (byte) Volume type: bit 0: On; 1: Sustain; 2: Loop
   !    +234      1   (byte) Panning type: bit 0: On; 1: Sustain; 2: Loop
   !    +235      1   (byte) Vibrato type
   !    +236      1   (byte) Vibrato sweep
   !    +237      1   (byte) Vibrato depth
   !    +238      1   (byte) Vibrato rate
   !    +239      2   (word) Volume fadeout
   !    +241      2   (word) Reserved
   !
   !                         Sample headers:
   !                         ---------------
   !
   !       ?      4  (dword) Sample length
   !      +4      4  (dword) Sample loop start
   !      +8      4  (dword) Sample loop length
   !     +12      1   (byte) Volume
   !     +13      1   (byte) Finetune (signed byte -16..+15)
   !     +14      1   (byte) Type: Bit 0-1: 0 = No loop, 1 = Forward loop,
   !                                        2 = Ping-pong loop;
   !                                     4: 16-bit sampledata
   !     +15      1   (byte) Panning (0-255)
   !     +16      1   (byte) Relative note number (signed byte)
   !     +17      1   (byte) Reserved
   !     +18     22   (char) Sample name
   !
   !                         Sample data:
   !                         ------------
   !
   !       ?      ?          Sample data (signed): The samples are stored
   !                         as delta values. To convert to real data:
   !
   !                         old=0;
   !                         for i=1 to len
   !                            new=sample[i]+old;
   !                            sample[i]=new;
   !                            old=new;



   ***********************
   *   Pattern format:   *
   ***********************

   The patterns are stored as ordinary MOD patterns, except that each
   note is stored as 5 bytes:

      ?      1   (byte) Note (0-71, 0 = C-0)
     +1      1   (byte) Instrument (0-128)
     +2      1   (byte) Volume column byte (see below)
     +3      1   (byte) Effect type
     +4      1   (byte) Effect parameter

   A simle packing scheme is also adopted, so that the patterns not become
   TOO large: Since the MSB in the note value is never used, if is used for
   the compression. If the bit is set, then the other bits are interpreted
   as follows:

      bit 0 set: Note follows
          1 set: Instrument follows
          2 set: Volume column byte follows
          3 set: Effect follows
          4 set: Guess what!

   It is very simple, but far from optimal. If you want a better,
   you can always repack the patterns in your loader.



   ******************************
   *   Volumes and envelopes:   *
   ******************************

   The volume formula:

   FinalVol=(FadeOutVol/65536)*(EnvelopeVol/64)*(GlobalVol/64)*(Vol/64)*Scale;

   The panning formula:

   FinalPan=Pan+(EnvelopePan-32)*(128-Abs(Pan-128))/32;

      Envelope:
      ---------

   The envelopes are processed once per frame, instead of every frame where
   no new notes are read. This is also true for the instrument vibrato and
   the fadeout. Since I am so lazy and the tracker is rather self-explaining
   I am not going to write any more for the moment.


   ********************************
   *   Periods and frequencies:   *
   ********************************

   PatternNote = 0..95 (0 = C-0, 95 = B-7)

   FineTune = -128..+127 (-128 = -1 halftone, +127 = +127/128 halftones)
   RelativeTone = -96..95 (0 => C-4 = C-4)

   RealNote = PatternNote + RelativeTone; (0..118, 0 = C-0, 118 = A#9)

      Linear frequence table:
      -----------------------

   Period = 10*12*16*4 - Note*16*4 - FineTune/2;
   Frequency = 8363*2^((6*12*16*4 - Period) / (12*16*4));

      Amiga frequence table:
      ----------------------

   Period = (PeriodTab[(Note MOD 12)*8 + FineTune/16]*(1-Frac32(FineTune/16)) +
             PeriodTab[(Note MOD 12)*8 + FineTune/16]*(Frac32(FineTune/16)))
            *16/2^(Note DIV 12);
      (The period is interpolated for finer finetune values)
   Frequency = 8363*1712/Period;

   PeriodTab = Array[0..12*8-1] of Word = (
      907,900,894,887,881,875,868,862,856,850,844,838,832,826,820,814,
      808,802,796,791,785,779,774,768,762,757,752,746,741,736,730,725,
      720,715,709,704,699,694,689,684,678,675,670,665,660,655,651,646,
      640,636,632,628,623,619,614,610,604,601,597,592,588,584,580,575,
      570,567,563,559,555,551,547,543,538,535,532,528,524,520,516,513,
      508,505,502,498,494,491,487,484,480,477,474,470,467,463,460,457);


   *************************
   *   Standard effects:   *
   *************************

      0      Appregio
      1  (*) Porta up
      2  (*) Porta down
      3  (*) Tone porta
      4  (*) Vibrato
      5  (*) Tone porta+Volume slide
      6  (*) Vibrato+Volume slide
      7  (*) Tremolo
      8      Set panning
      9      Sample offset
      A  (*) Volume slide
      B      Position jump
      C      Set volume
      D      Pattern break
      E1 (*) Fine porta up
      E2 (*) Fine porta down
      E3     Set gliss control
      E4     Set vibrato control
      E5     Set finetune
      E6     Set loop begin/loop
      E7     Set tremolo control
      E9     Retrig note
      EA (*) Fine volume slide up
      EB (*) Fine volume slide down
      EC     Note cut
      ED     Note delay
      EE     Pattern delay
      F      Set tempo/BPM
      G      Set global volume
      H  (*) Global volume slide
      K      Key off
      L      Set envelope position
      P  (*) Panning slide
      R  (*) Multi retrig note
      T      Tremor
      X1 (*) Extra fine porta up
      X2 (*) Extra fine porta down

      (*) = If the command byte is zero, the last nonzero byte for the
            command should be used.

   *********************************
   *   Effects in volume column:   *
   *********************************

   All effects in the volume column should work as the standard effects.
   The volume column is interpreted before the standard effects, so
   some standard effects may override volume column effects.

   Value      Meaning

      0       Do nothing
    $10-$50   Set volume Value-$10
      :          :        :
      :          :        :
    $60-$6f   Volume slide down
    $70-$7f   Volume slide up
    $80-$8f   Fine volume slide down
    $90-$9f   Fine volume slide up
    $a0-$af   Set vibrato speed
    $b0-$bf   Vibrato
    $c0-$cf   Set panning
    $d0-$df   Panning slide left
    $e0-$ef   Panning slide right
    $f0-$ff   Tone porta


 ============================================================================
*/

typedef struct XMHeader_t {
	char id[17];			// ID text: 'Extended module: '
	char songname[21];		// Module name, padded with zeroes and 0x1a at the end
	char trackername[20];	// Tracker name
	Word16 version;			// (word) Version number, hi-byte major and low-byte minor
	Word32 headersize;	// Header size
	Word16 songlength;		// (word) Song length (in patten order table)
	Word16 restart;			// (word) Restart position
	Word16 numchn;			// (word) Number of channels (2,4,6,8,10,...,32)
	Word16 numpat;			// (word) Number of patterns (max 256)
	Word16 numins;			// (word) Number of instruments (max 128)
	Word16 flags;			// (word) Flags: bit 0: 0 = Amiga frequency table (see below) 1 = Linear frequency table
	Word16 tempo;			// (word) Default tempo
	Word16 bpm;				// (word) Default BPM
	Word8 orders[256];		// (byte) Pattern order table
} XMHeader_t;

typedef struct XMINSTHEADER {
	Word32 size;			// (dword) Instrument size
	char name[22];			// (char) Instrument name
	Word16 Padding;
	Word type;				// (byte) Instrument type (always 0)
	Word numsmp;			// (word) Number of samples in instrument
	Word32 ssize;			//
} XMINSTHEADER;

typedef struct XMPATCHHEADER {
	Word8 what[96];			// (byte) Sample number for all notes
	Word16 volenv[24];		// (byte) Points for volume envelope
	Word16 panenv[24];		// (byte) Points for panning envelope
	Word8 volpts;			// (byte) Number of volume points
	Word8 panpts;			// (byte) Number of panning points
	Word8 volsus;			// (byte) Volume sustain point
	Word8 volbeg;			// (byte) Volume loop start point
	Word8 volend;			// (byte) Volume loop end point
	Word8 pansus;			// (byte) Panning sustain point
	Word8 panbeg;			// (byte) Panning loop start point
	Word8 panend;			// (byte) Panning loop end point
	Word8 volflg;			// (byte) Volume type: bit 0: On; 1: Sustain; 2: Loop
	Word8 panflg;			// (byte) Panning type: bit 0: On; 1: Sustain; 2: Loop
	Word8 vibflg;			// (byte) Vibrato type
	Word8 vibsweep;			// (byte) Vibrato sweep
	Word8 vibdepth;			// (byte) Vibrato depth
	Word8 vibrate;			// (byte) Vibrato rate
	Word16 volfade;			// (word) Volume fadeout
	Word16 reserved[11];		// (word) Reserved
} XMPATCHHEADER;


typedef struct XMWAVHEADER{
	Word32 length;		// (dword) Sample length
	Word32 loopstart;		// (dword) Sample loop start
	Word32 looplength;	// (dword) Sample loop length
	Word8 volume;			// (byte) Volume
	char finetune;			// (byte) Finetune (signed byte -128..+127)
	Word8 type;				// (byte) Type: Bit 0-1: 0 = No loop, 1 = Forward loop,
							// 2 = Ping-pong loop;
							// 4: 16-bit sampledata
	Word8 panning;			// (byte) Panning (0-255)
	char relnote;			// (byte) Relative note number (signed byte)
	Word8 reserved;			// (byte) Reserved
	char samplename[22];	// (char) Sample name
} XMWAVHEADER;

#if 0
typedef struct XMPatHeader_t {
	Word32 size;			// (dword) Pattern header length
	Word8 packing;			// (byte) Packing type (always 0)
	Word16 numrows;			// (word) Number of rows in pattern (1..256)
	Word16 packsize;			// (word) Packed patterndata size
} XMPatHeader_t;

typedef struct MTMNOTE {
	Word8 a,b,c;
} MTMNOTE;
#endif

typedef struct XMNote_t {
	Word8 note,ins,vol,eff,dat;
} XMNote_t;

#define HEADERSIZE 276
#define PHSIZE 9
#define IHSIZE 263
#define IHSIZESMALL 33
#define IHSSIZE 40

static const long finetune[ 16] = {
	7895,	7941,	7985,	8046,	8107,	8169,	8232,	8280,
	8363,	8413,	8463,	8529,	8581,	8651,	8723,	8757
};

/**********************************

	Extract a note structure

**********************************/

static const Word8 *XM_ReadNote(XMNote_t *n,const Word8 *InputPtr)
{
	Word cmp;
	
	cmp = InputPtr[0];
	if (cmp&0x80) {
		Word8 Temp;
		++InputPtr;
		
		if (cmp&1) {
			Temp = InputPtr[0];
			++InputPtr;
		} else {
			Temp = 0xFF;
		}
		n->note = Temp;
		
		if (cmp&2) {
			Temp = InputPtr[0];
			++InputPtr;
		} else {
			Temp = 0;
		}
		n->ins = Temp;
		
		if (cmp&4) {
			Temp = InputPtr[0];
			++InputPtr;
		} else {
			Temp = 0xFF;
		}
		n->vol = Temp;
		
		if (cmp&8) {
			Temp = InputPtr[0];
			++InputPtr;
		} else {
			Temp = 0xFF;
		}
		n->eff = Temp;
		
		if (cmp&16) {
			Temp = InputPtr[0];
			++InputPtr;
		} else {
			Temp = 0xFF;
		}
		n->dat = Temp;
	} else {
		n->note	= static_cast<Word8>(cmp);
		n->ins = InputPtr[1];
		n->vol = InputPtr[2];
		n->eff = InputPtr[3];
		n->dat = InputPtr[4];
		InputPtr+=5;
	}
	return InputPtr;
}

/**********************************

	Extract an effect structure

**********************************/

static void XM_Convert2MAD(XMNote_t *xmtrack,Cmd_t *aCmd)
{
	Word8 note,ins,vol,eff,dat;

	note = xmtrack->note;
	if (!note || note == 0xFF) {
		note = 0xFF;
	} else {
		note--;
	}
	ins = xmtrack->ins;
	vol = xmtrack->vol;
	eff = xmtrack->eff;
	dat = xmtrack->dat;

	aCmd->ins = ins;
	aCmd->note = note;
	aCmd->vol = vol;
	if( note == 96) {		// KEY OFF
		aCmd->note = 0xFE;
	}
	if( eff <= 0x0F) {
		aCmd->cmd = eff;
		aCmd->arg = dat;
		if (dat == 0xFF) {
			aCmd->arg = 0;
		}
	} else {
		aCmd->cmd = 0;
		aCmd->arg = 0;
	}

	switch(eff) {
//	case 'G'-55:                    // G - set global volume
//		break;

//	case 'H'-55:                    // H - global volume slide
//		break;

	case 'K'-55:                    // K - keyoff	
		aCmd->ins = 00;
		aCmd->note = 0xFE;
		break;

//	case 'L'-55:                    // L - set envelope position
//		break;

//	case 'P'-55:                    // P - panning slide
//		break;

//	case 'R'-55:                    // R - multi retrig note
//		break;

//	case 'T'-55:
//		break;

//	case 'X'-55:
//		if((dat>>4)==1) {				// X1 extra fine porta up
//		} else {						// X2 extra fine porta down
//		}
//		break;

	default:
		break;
	}
}

/**********************************

	Read in the data patterns

**********************************/

static const Word8 *XMReadPattern(MADMusic *theMAD,const Word8 *InputPtr,Word NumPat)
{
	Word t, u, v;
	Word NumChan;
	
	/*****************/
	/** PATTERNS **/
	/*****************/

	FastMemSet(theMAD->partition,0,sizeof(theMAD->partition));
	NumChan = theMAD->header->numChn;
	
	for (t=0;t<NumPat;t++) {
		PatData_t *tempMusicPat;
		Word PatternSize;
		XMNote_t xmpat;
		Cmd_t *aCmd;
		Word16 PackSize;
//		Word Packing;
		
//		Packing = InputPtr[4];
		PatternSize = Burger::LoadLittle((Word16 *)(&InputPtr[5]));
		PackSize = Burger::LoadLittle((Word16 *)(&InputPtr[7]));

		InputPtr += Burger::LoadLittle((Word32 *)InputPtr);

		/*
			Gr8.. when packsize is 0, don't try to load a pattern.. it's empty.
			This bug was discovered thanks to Khyron's module..
		*/

		if (PackSize > 0) {
			tempMusicPat = (PatData_t*) AllocAPointerClear( sizeof( PatHeader_t) + theMAD->header->numChn * PatternSize * sizeof(Cmd_t));
			if (!tempMusicPat) {
				return 0;		/* Crap! */
			}
			theMAD->partition[t] = tempMusicPat;
			tempMusicPat->header.size = PatternSize;

			for (u = 0; u < PatternSize ; u++) {
				for (v = 0; v < NumChan; v++) {
					InputPtr = XM_ReadNote(&xmpat,InputPtr);
					XM_Convert2MAD(&xmpat,&tempMusicPat->Cmds[(PatternSize * v) + u]);
				}
			}
		} else {
			tempMusicPat = (PatData_t*) AllocAPointerClear(sizeof( PatHeader_t) + theMAD->header->numChn *sizeof(Cmd_t));
			if (!tempMusicPat) {
				return 0;
			}
			theMAD->partition[t] = tempMusicPat;
			tempMusicPat->header.size = 1;
			
			for (v = 0 ; v < NumChan; v++) {
				aCmd = &tempMusicPat->Cmds[v];
				aCmd->ins = 0;
				aCmd->note = 0xFF;
				aCmd->cmd = 0;
				aCmd->arg = 0;
				aCmd->vol = 0xFF;
				aCmd->Filler = 0;
			}
		}
	}
	return InputPtr;
}

/**********************************

	Read in the instruments

**********************************/

static const Word8 *XMReadInstruments(MADMusic *theMAD,const Word8 *InputPtr,Word NumIns)
{
	int t, u, i, x;

	/*****************/
	/** INSTRUMENTS **/
	/*****************/
	
	theMAD->fid = ( InstrData*) AllocAPointerClear( sizeof( InstrData) * (long) MAXINSTRU);
	if( !theMAD->fid) {
		return 0;
	}
	theMAD->sample = ( sData_t**) AllocAPointerClear( sizeof( sData_t*) * (long) MAXINSTRU * (long) MAXSAMPLE);
	if( !theMAD->sample) {
		return 0;
	}
	
	i = 0;
	do {
		theMAD->fid[ i].firstSample = (short)(i * MAXSAMPLE);
	} while (++i<MAXINSTRU);
	
	i = 0;
	do {
		x = 0;
		do {
			theMAD->sample[i*MAXSAMPLE+x] = 0;
		} while (++x<MAXSAMPLE);
		theMAD->fid[i].numSamples	= 0;
	} while (++i<MAXINSTRU);
	
	for (t = 0; t < (int)NumIns; t++) {
		XMINSTHEADER ih;
		InstrData *curIns = &theMAD->fid[t];
		const Word8 *theXMReadCopy;

		theXMReadCopy = InputPtr;
		ih.size = Burger::LoadLittle((Word32 *)InputPtr);
		ih.type = InputPtr[26];
		ih.numsmp = Burger::LoadLittle((Word16 *)(&InputPtr[27]));
		FastMemCpy(ih.name,InputPtr+4,22);
		InputPtr += 29;

		FastMemCpy(curIns->name,ih.name,22);
		curIns->numSamples	= static_cast<Word8>(ih.numsmp);

		if (ih.numsmp > 0) {
			XMPATCHHEADER	pth;

			ih.ssize = Burger::LoadLittle((Word32*)InputPtr);
			FastMemCpy(pth.what,InputPtr+4,96);
			FastMemCpy(pth.volenv,InputPtr+100,48);
			FastMemCpy(pth.panenv,InputPtr+148,48);
			pth.volpts = InputPtr[196];
			pth.panpts = InputPtr[197];
			pth.volsus = InputPtr[198];
			pth.volbeg = InputPtr[199];
			pth.volend = InputPtr[200];
			pth.pansus = InputPtr[201];
			pth.panbeg = InputPtr[202];
			pth.panend = InputPtr[203];
			pth.volflg = InputPtr[204];
			pth.panflg = InputPtr[205];
			pth.vibflg = InputPtr[206];
			pth.vibsweep = InputPtr[207];
			pth.vibdepth = InputPtr[208];
			pth.vibrate = InputPtr[209];
			pth.volfade = Burger::LoadLittle((Word16 *)(&InputPtr[210]));
			FastMemCpy(pth.reserved,InputPtr+212,22);
			InputPtr += 234;

			FastMemCpy(curIns->what,pth.what,96);
			
			// Volume Env
			
			x = 0;
			do {
				curIns->volEnv[x].pos = Burger::LoadLittle(&pth.volenv[x*2]);	// 
				curIns->volEnv[x].val = Burger::LoadLittle(&pth.volenv[x*2+1]);	// 00...64
			} while (++x<12);
		
			curIns->volSize	= pth.volpts;
			curIns->volType	= pth.volflg;
			curIns->volSus	= pth.volsus;
			curIns->volBeg	= pth.volbeg;
			curIns->volEnd	= pth.volend;
			
			if( curIns->volBeg >= curIns->volSize) {
				curIns->volBeg = curIns->volSize-1;
			}
			if( curIns->volEnd >= curIns->volSize) {
				curIns->volEnd = curIns->volSize-1;
			}
			curIns->volFade	= pth.volfade;
			
			// Panning Env
			
			x = 0;
			do {
				curIns->pannEnv[x].pos = Burger::LoadLittle(&pth.panenv[x*2]);	// 
				curIns->pannEnv[x].val = Burger::LoadLittle(&pth.panenv[x*2+1]);	// 00...64
			} while (++x<12);
			
			curIns->pannSize = pth.panpts;
			curIns->pannType = pth.panflg;
			curIns->pannSus	= pth.pansus;
			curIns->pannBeg	= pth.panbeg;
			curIns->pannEnd	= pth.panend;
			
			if( curIns->pannBeg >= curIns->pannSize) {
				curIns->pannBeg = curIns->pannSize-1;
			}
			if( curIns->pannEnd >= curIns->pannSize) {
				curIns->pannEnd = curIns->pannSize-1;
			}
		//	curIns->panFade	= pth.panfade;
		}
		InputPtr = theXMReadCopy + ih.size;
		
		/** Read samples */
		
		for( u = 0 ; (Word)u < ih.numsmp ; u++) {
			XMWAVHEADER wh;
			
			theXMReadCopy = InputPtr;
			wh.length = Burger::LoadLittle((Word32 *)InputPtr);
			wh.loopstart = Burger::LoadLittle((Word32 *)(&InputPtr[4]));
			wh.looplength = Burger::LoadLittle((Word32 *)(&InputPtr[8]));
			wh.volume = InputPtr[12];
			wh.finetune = InputPtr[13];
			wh.type = InputPtr[14];
			wh.panning = InputPtr[15];
			wh.relnote = InputPtr[16];
			wh.reserved = InputPtr[17];
			FastMemCpy(wh.samplename,InputPtr+18,22);
			InputPtr = theXMReadCopy + ih.ssize;
			
			{
				sData_t	*curData;
				
				curData = theMAD->sample[ t*MAXSAMPLE + u] = (sData_t*) AllocAPointerClear( sizeof( sData_t));
				
				curData->size = wh.length;
				curData->loopBeg = wh.loopstart;
				curData->loopSize = wh.looplength;
				
				curData->vol = wh.volume;
				curData->c2spd = (Word16)finetune[(wh.finetune + 128)/16];
				curData->loopType = 0;
				curData->amp = 8;
				
				if (wh.type & 0x10)	{		// 16 Bits
					curData->amp = 16;
				}
				if (wh.type & 0x2) {	// Ping Pong
					curData->loopType = ePingPongLoop;
				}
				
				if( !(wh.type & 0x3)) {
					curData->loopBeg = 0;
					curData->loopSize = 0;
				}
				
			//	curData->panning	= wh.panning;
				curData->relNote	= wh.relnote;
				FastMemCpy(curData->name,wh.samplename,22);
			}
		}

		/** Read samples data **/
		
		for( u = 0 ; u < curIns->numSamples ; u++) {
			sData_t	*curData = theMAD->sample[ t*MAXSAMPLE + u];
			const Word8 *SrcData;
			Word8 *DestData;
			
			DestData = (Word8 *)AllocAPointer( curData->size);
			if (!DestData) {
				return 0;
			}
			curData->data = (char *)DestData;
			SrcData = InputPtr;
			InputPtr += curData->size;
			
			if (curData->amp == 16) {				
				/* Delta to Real */
				Word oldV;
				Word32 xL;
					
				xL = curData->size>>1;
				if (xL) {
					oldV = 0;
					do {
						oldV = Burger::LoadLittle((Word16 *)SrcData) + oldV;
						((Word16 *)DestData)[0] = (Word16)oldV;
						SrcData+=2;
						DestData+=2;
					} while (--xL);
				}
			} else {
				/* Delta to Real */
				Word oldV;
				Word32 xL;
				xL = curData->size;
				if (xL) {
					oldV = 0;
					do {
						oldV = SrcData[0] + oldV;
						DestData[0] = (Word8)oldV;
						++SrcData;
						++DestData;
					} while (--xL);
				}
			}
		}
	}
	return InputPtr;
}

/**********************************

	Perform the importation

**********************************/

static int BURGERCALL ConvertXM2Mad(const Word8 *theXM,Word32 /* XMSize */, MADMusic *theMAD)
{
	int i;
	long inOutCount, x;
	XMHeader_t *HeaderPtr;
	Word NumIns;
	Word SngLength;
	Word NumPat;
	MADSpec *MadHeaderPtr;
	
	/********************/
	/** READ XM HEADER **/
	/********************/
	
	HeaderPtr = (XMHeader_t *)theXM;

	NumIns = Burger::LoadLittle(&HeaderPtr->numins);
	if (NumIns>MAXINSTRU) {
		NumIns = MAXINSTRU;
	}
	
	/********************/
	/** MAD ALLOCATION **/
	/********************/
	
	inOutCount = sizeof( MADSpec);
	MadHeaderPtr = (MADSpec*) AllocAPointerClear( inOutCount);
	theMAD->header = MadHeaderPtr;
	if (!MadHeaderPtr) {
		return MADNeedMemory;
	}
	MadHeaderPtr->MAD = 'MADI';
	
	i = 0;
	do {
		MadHeaderPtr->name[i] = HeaderPtr->songname[i];
		if (!MadHeaderPtr->name[i]) {
			break;
		}
	} while (++i<20);
	
	SngLength = Burger::LoadLittle(&HeaderPtr->songlength);
	NumPat = Burger::LoadLittle(&HeaderPtr->numpat);
	MadHeaderPtr->speed = Burger::LoadLittle(&HeaderPtr->tempo);
	MadHeaderPtr->tempo = Burger::LoadLittle(&HeaderPtr->bpm);
	MadHeaderPtr->numChn = (Word8)Burger::LoadLittle(&HeaderPtr->numchn);
	MadHeaderPtr->numPat = static_cast<Word8>(NumPat);
	MadHeaderPtr->numPointers = static_cast<Word8>(SngLength);
	
	if (SngLength > 128) {
		MadHeaderPtr->numPointers = 128;
	}
	strcpy(MadHeaderPtr->infos,"Converted...");
	
	for( i = 0; i < (int)SngLength; i++) {
		Word Point1;
		Point1 = HeaderPtr->orders[ i];
		if (Point1 >= NumPat) {
			Point1 = NumPat-1;
		}
		MadHeaderPtr->oPointers[i] = (Word8)Point1;
	}
	
	x = 1;
	i = 0;
	do {
		if (x > 0) {
			MadHeaderPtr->chanPan[i] = MAX_PANNING/4;
		} else {
			MadHeaderPtr->chanPan[i] = MAX_PANNING - MAX_PANNING/4;
		}
		if(--x == -2) {
			x = 2;
		}
		MadHeaderPtr->chanVol[i] = MAX_VOLUME;
	} while (++i<MAXTRACK);
	
	MadHeaderPtr->generalVol = 64;
	MadHeaderPtr->generalSpeed = 80;
	MadHeaderPtr->generalPitch = 80;

	theXM = XMReadPattern(theMAD,theXM+sizeof(XMHeader_t),NumPat);
	if (theXM) {
		theXM = XMReadInstruments(theMAD,theXM,NumIns);
		if (theXM) {
			return 0;
		}
	}
	MADMusicDestroy(theMAD);
	return MADNeedMemory;
}

/**********************************

	Import an XM file

**********************************/

int BURGERCALL ModMusicXM(const Word8 *DataPtr,Word32 Length,MADMusic *MadFile)
{
	int Result;
	Result = MADFileNotSupportedByThisPlug;		/* Not supported */
	if (!memcmp(DataPtr,"Extended Module: ",17) && Burger::LoadLittle(&((XMHeader_t*)DataPtr)->version) == 0x104) {
		Result = ConvertXM2Mad(DataPtr,Length,MadFile);
	}
	return Result;		
}
