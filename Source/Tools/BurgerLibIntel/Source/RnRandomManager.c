#include "RnRandom.h"
#include "MmMemory.h"
#include "TkTick.h"
#include "ClStdLib.h"

/**********************************

	Default random number array

**********************************/

static Word baseRndArray[17] = {	/* Default array */
	1,1,2,3,5,8,13,21,54,75,129,204,323,527,850,1377,2227
};


/**********************************

	(Primary) Random number generator data

**********************************/

Random_t RndMain = {
	0,		/* Seed */
	16,		/* First index */
	4,		/* Second index */
	{1,1,2,3,5,8,13,21,54,75,129,204,323,527,850,1377,2227} /* Polynomial */
};

/**********************************

	(Primary) Random number generator data

**********************************/

Random_t RndAux = {
	0,		/* Seed */
	16,		/* First index */
	4,		/* Second index */
	{1,1,2,3,5,8,13,21,54,75,129,204,323,527,850,1377,2227} /* Polynomial */
};

/**********************************

	Create a new random number generator instance

**********************************/

Random_t * BURGERCALL RndNew(Word NewSeed)
{
	Random_t *NewPtr;
	NewPtr = (Random_t *)AllocAPointer(sizeof(Random_t));
	if (NewPtr) {
		RndRandomize(NewPtr);		/* Init the data */
		if (NewSeed) {
			RndSetRandomSeed(NewPtr,NewSeed);	/* Set a new seed */
		}
	}
	return NewPtr;		/* Return the instance */
}

/**********************************

	Init the random number generator from a KNOWN state.
	This will allow games to record just the joystick
	movements and have random actions repeat for demo playback

**********************************/

void BURGERCALL RndRandomize(Random_t *Input)
{
	FastMemCpy((char *)&Input->RndArray[0],(char *)baseRndArray,sizeof(Input->RndArray));
	Input->Seed = 0;
	Input->Index1 = 16;
	Input->Index2 = 4;
	RndGetRandom(Input,0xFFFF);
}

/**********************************

	Init the random number generator with an "Anything goes"
	policy so programs will power up in an unknown state.
	Do NOT use this if you wish your title to have recordable demos.

	I use the formula that the tick timer runs at a constant time
	base but the machine in question does not. As a result. The
	number of times GetRandom is called is anyone's guess.

**********************************/

void BURGERCALL RndHardwareRandomize(Random_t *Input)
{
	Word32 TickMark;
	RndRandomize(Input);	/* Init the structure */
	TickMark = ReadTick();	/* Get a current tick mark */
	do {
		RndGetRandom(Input,0xFFFF);	/* Discard a number from the stream */
	} while (ReadTick()==TickMark);	/* Same time? */
}

/**********************************

	Get a random number. Return a number between 0-MaxVal inclusive

**********************************/

Word BURGERCALL RndGetRandom(Random_t *Input,Word MaxVal)
{
	Word NewVal;
	int i,j;

	if (!MaxVal) {	/* Zero? */
		return 0;	/* Don't bother */
	}
	++MaxVal;		/* +1 to force inclusive */
	i = Input->Index1;		/* Cache indexs */
	j = Input->Index2;
	NewVal = Input->RndArray[i] + Input->RndArray[j];	/* Get the delta seed */
	Input->RndArray[i] = NewVal;	/* Save in array */
	NewVal += Input->Seed;		/* Add to the base seed */
	Input->Seed = NewVal;		/* Save the seed */
	if (--i<0) {			/* Advance the indexs */
		i = 16;
	}
	if (--j<0) {
		j = 16;
	}
	Input->Index1 = i;		/* Save in statics */
	Input->Index2 = j;
	NewVal&=0xFFFFU;		/* Make sure they are shorts! */
	MaxVal&=0xFFFFU;
	if (!MaxVal) {		/* No adjustment? */
		return NewVal;	/* Return the random value */
	}
	return ((NewVal*MaxVal)>>16U);	/* Adjust the 16 bit value to range */
}

/**********************************

	Set the random number generator to a specific seed

**********************************/

void BURGERCALL RndSetRandomSeed(Random_t *Input,Word Seed)
{
	Word i;
	FastMemCpy((char *)&Input->RndArray[0],(char *)baseRndArray,sizeof(Input->RndArray));
	Input->Seed = 0-Seed;
	i = Seed&0xF;
	Input->Index1 = i;
	Input->Index2 = (i-12)&0xF;
	i = ((Seed>>8)&0x1f)+1;
	do {
		RndGetRandom(Input,0xFFFF);
	} while (--i);
}

/**********************************

	Return a random number between -Range and +Range (Inclusive)
	I will return a SIGNED value

**********************************/

int BURGERCALL RndGetRandomSigned(Random_t *Input,Word Range)
{
	return RndGetRandom(Input,Range<<1)-Range;		/* Get the random number */
}
