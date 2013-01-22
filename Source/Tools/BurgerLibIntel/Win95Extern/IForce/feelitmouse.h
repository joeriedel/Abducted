/**********************************

	The FEELit Mouse system

**********************************/

#ifndef INCLUDED_FEELITMOUSE_H
#define INCLUDED_FEELITMOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INCLUDED_FEELITAPI_H
#include <feelitapi.h>
#endif

#ifndef INCLUDED_FEELITIFR_H
#include <feelitifr.h>
#endif

#define	GUNSHOT		0
#define INERTIA		1
#define BUZZER		2
#define RELOAD		3
#define CHANGECLIP	4
#define MOVEPAPERTARGET 5
#define BUTTON		6


#define RELOADDOWN	0
#define RELOADUP	1

extern void FEELitInit(void);
extern void FEELitPlayFFEffect(int nEffectNum);	// Play FEELit forcefeedback effect.
extern void FEELitStopFFEffect(int nEffectNum);	// Play FEELit forcefeedback effect.
extern void FEELitSetWeaponWeight(int nWeight);
extern void FEELitSetWeaponKick(int nKick);
extern void FEELitSetReloadDir(int nDirection);
extern void FEELitReloadDown(void);
extern void FEELitChangeClip(void);
extern void FEELitReloadUp(void);
extern void FEELitStopGameForces(void);
extern void FEELitPlayMenuEnclosure(int Left, int Top, int Right, int Bottom );
extern void FEELitStopMenuForces(void);

#ifdef __cplusplus
}
#endif

#endif
