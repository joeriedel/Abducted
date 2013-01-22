/**********************************

	This code is used to decompress Microsoft
	ADPCM compressed audio data

**********************************/

#include "PkPack.h"

typedef struct ADPCMState_t {
	long sample1;	/* Left audio sample */
	long sample2;	/* Right audio sample */
	long Coef1;		/* Coef 1 constant */
	long Coef2;		/* Coef 2 constant */
	long index;		/* Index into step size table */
 } ADPCMState_t;

/* Coeffient tables */

static const long gaiP4[16+16+7+7] = {
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230,
//static const long deltaCodeTbl[16] = {
	0,1,2,3,4,5,6,7,-8,-7,-6,-5,-4,-3,-2,-1,
//static const long gaiCoef1[7] = {
	256, 512, 0, 192, 240, 460,  392,
//static const long gaiCoef2[7] = {
	0, -256,  0,  64,   0,-208, -232};

/**********************************

	Decode a sample and update the state tables
	Each sample is 4 bits in size

**********************************/

static INLINECALL int MsAdpcmDecode(Word deltaCode,ADPCMState_t *state)
{
	long idelta;		/* Previous index */

	/** Compute next Adaptive Scale Factor (ASF) **/

	deltaCode = deltaCode&0x0f;
	idelta = state->index;
	{
		long newindex;
		newindex = (gaiP4[deltaCode] * idelta) >> 8;
		if (newindex < 16) {
			newindex = 16;
		}
		state->index = newindex;
	}
	idelta = gaiP4[deltaCode+16]*idelta;

	/** Predict next sample **/

	{
		long predict;
		long sample;
		predict = (state->sample2 * state->Coef2);	/* Get first coef */
		sample = state->sample1;		/* Copy to temp */
		state->sample2 = sample;		/* Move to first */
		sample = (predict + (sample * state->Coef1)) >> 8;

		idelta = idelta + sample;	/* Get the output value */
	}
	if (idelta > 32767) {		/* In bounds for a short? */
		idelta = 32767;
	} else if (idelta <= -32768) {
		idelta = -32768;
	}
	state->sample1 = idelta;
	return idelta;
}

/**********************************

	Decode the a block of samples

**********************************/

Word BURGERCALL ADPCMDecodeBlock(ADPCMUnpackState_t *StatePtr)
{
	Word32 Temp;			/* Temp Var */
	Word8 *bytePtr;
	short *OutputPtr;
	Word samplesThisBlock;

	/* Pull in the packet and check the header */

	Temp = StatePtr->BlockSize;
	if (StatePtr->SrcLength<Temp) {
		Temp = StatePtr->SrcLength;
	}
	StatePtr->SrcLength-=Temp;
	bytePtr = StatePtr->SrcPtr;
	StatePtr->SrcPtr=&bytePtr[Temp];			/* Adjust the source pointer */

	if (Temp < StatePtr->BlockSize) {	/* Partial block? */

	/* If it looks like a valid header is around then try and */
	/* work with partial blocks. Specs say it should be null */
	/* padded but I guess this is better then trailing quiet. */

		if (Temp < (7 * StatePtr->Channels)) {
			return 0;		/* No bytes decoded! */
		}
		samplesThisBlock = (StatePtr->BlockSize - (6 * StatePtr->Channels));
	} else {
		samplesThisBlock = StatePtr->SamplesPerBlock;
	}

	/* Now, I decompress differently for mono or stereo */

	if (StatePtr->Channels==1) {
		/* Mono */
		/* Read the four-byte header for each channel */

		/* Reset the decompressor */
		Temp = bytePtr[0];	/* Left */
		/* 7 should be variable from AVI/WAV header */
		if (Temp < 7) {
			ADPCMState_t MonoState;	/* One decompressor state for each channel */
			MonoState.Coef1 = gaiP4[Temp+32];
			MonoState.Coef2 = gaiP4[Temp+(32+7)];

			Temp = bytePtr[1];
			Temp |= ((Word)bytePtr[2])<<8;
			MonoState.index = (long)((short)Temp);		/* Store it */

			Temp = bytePtr[3];
			Temp |= ((Word)bytePtr[4])<<8;
			MonoState.sample1 = (long)((short)Temp);

			Temp = bytePtr[5];
			Temp |= ((Word)bytePtr[6])<<8;
			MonoState.sample2 = (long)((short)Temp);

			OutputPtr = StatePtr->OutputPtr;

			/* Decode two samples for the header */
			OutputPtr[0] = (short)Temp;		//state[0].sample2;
			OutputPtr[1] = (short)MonoState.sample1;

			bytePtr += 7;
			OutputPtr += 2;

			/* Decompress nybbles. Minus 2 included in header */

			if (samplesThisBlock>2) {
				Word remaining;
				remaining = (samplesThisBlock-2)>>1;
				do {
					Temp = bytePtr[0];
					++bytePtr;
					OutputPtr[0] = MsAdpcmDecode((Temp>>4),&MonoState);
					OutputPtr[1] = MsAdpcmDecode(Temp,&MonoState);
					OutputPtr+=2;
				} while (--remaining);
			}
			StatePtr->OutputPtr = OutputPtr;
			return samplesThisBlock*2;
		}
		return 0;
	}
	/* Stereo */

	/* Read the four-byte header for each channel */

	/* Reset the decompressor */
	Temp = bytePtr[0];	/* Left */
	/* 7 should be variable from AVI/WAV header */
	if (Temp < 7) {
		ADPCMState_t state[2];	/* One decompressor state for each channel */
		state[0].Coef1 = gaiP4[Temp+32];
		state[0].Coef2 = gaiP4[Temp+(32+7)];

		Temp = bytePtr[1];	/* Right */
		if (Temp < 7) {
			state[1].Coef1 = gaiP4[Temp+32];
			state[1].Coef2 = gaiP4[Temp+(32+7)];

			Temp = bytePtr[2];
			Temp |= ((Word)bytePtr[3])<<8;
			state[0].index = (long)((short)Temp);

			Temp = bytePtr[4];
			Temp |= ((Word)bytePtr[5])<<8;
			state[1].index = (long)((short)Temp);

			Temp = bytePtr[6];
			Temp |= ((Word)bytePtr[7])<<8;
			state[0].sample1 = (long)((short)Temp);

			Temp = bytePtr[8];
			Temp |= ((Word)bytePtr[9])<<8;
			state[1].sample1 = (long)((short)Temp);

			Temp = bytePtr[10];
			Temp |= ((Word)bytePtr[11])<<8;
			state[0].sample2 = (long)((short)Temp);

			Temp = bytePtr[12];
			Temp |= ((Word)bytePtr[13])<<8;
			state[1].sample2 = (long)((short)Temp);

			OutputPtr = StatePtr->OutputPtr;

			/* Decode two samples for the header */
			OutputPtr[0] = (short)state[0].sample2;
			OutputPtr[1] = (short)state[1].sample2;
			OutputPtr[2] = (short)state[0].sample1;
			OutputPtr[3] = (short)state[1].sample1;
			OutputPtr+=4;
			bytePtr+=14;
			/* Decompress nybbles. Minus 2 included in header */
			if (samplesThisBlock>2) {
				Word remaining2;
				remaining2 = samplesThisBlock-2;
				do {
					Temp = bytePtr[0];
					++bytePtr;
					OutputPtr[0] = MsAdpcmDecode((Temp>>4),&state[0]);
					OutputPtr[1] = MsAdpcmDecode(Temp,&state[1]);
					OutputPtr+=2;
				} while (--remaining2);
			}
			StatePtr->OutputPtr = OutputPtr;
			return samplesThisBlock*4;
		}
	}
	return 0;
}

