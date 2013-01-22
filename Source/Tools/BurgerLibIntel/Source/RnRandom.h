/*******************************

	Random number generator

*******************************/

#ifndef __RNRANDOM_H__
#define __RNRANDOM_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Random number generator */

typedef struct Random_t {
	Word Seed;		/* Random number seed */
	Word Index1;	/* First lookup index */
	Word Index2;	/* Second lookup index */
	Word RndArray[17];	/* Array of seed values (Polynomial) */
} Random_t;

/* Public data */

extern Random_t RndMain;		/* Main random number instance */
extern Random_t RndAux;			/* Aux random number instance */
extern Random_t * BURGERCALL RndNew(Word NewSeed);
#define RndDelete(Input) DeallocAPointer(Input)
extern void BURGERCALL RndRandomize(Random_t *Input);
extern void BURGERCALL RndHardwareRandomize(Random_t *Input);
extern Word BURGERCALL RndGetRandom(Random_t *Input,Word Range);
extern void BURGERCALL RndSetRandomSeed(Random_t *Input,Word Seed);
extern int BURGERCALL RndGetRandomSigned(Random_t *Input,Word Range);
#define Randomize() RndRandomize(&RndMain)
#define HardwareRandomize() RndHardwareRandomize(&RndMain)
#define GetRandom(Range) RndGetRandom(&RndMain,Range)
#define SetRandomSeed(Seed) RndSetRandomSeed(&RndMain,Seed)
#define GetRandomSigned(Range) RndGetRandomSigned(&RndMain,Range)
#define AuxRandomize() RndRandomize(&RndAux)
#define AuxHardwareRandomize() RndHardwareRandomize(&RndAux)
#define AuxGetRandom(Range) RndGetRandom(&RndAux,Range)
#define AuxSetRandomSeed(Seed) RndSetRandomSeed(&RndAux,Seed)
#define AuxGetRandomSigned(Range) RndGetRandomSigned(&RndAux,Range)

#ifdef __cplusplus
}
#endif


#endif

