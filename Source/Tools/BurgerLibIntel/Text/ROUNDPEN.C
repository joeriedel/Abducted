#include "Text.h"
#include <math.h>

/****************************************

	Round a floating point number to the nearest penny

****************************************/

float RoundCent(float Val)
{
	float Temp;
	Temp = fmod(Val,0.01);	/* Isolate the fraction beyond the penny */
	Val-=Temp;			/* Hack off the fraction */
	if (Temp>=0.005) { /* Round it up? */
		Val+=0.01;	 /* Add 1 penny */
	}
	return Val;	 /* Value rounded to nearest penny */
}

