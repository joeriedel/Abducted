#include "McMac.h"

/**********************************

	MacOS ONLY!!!!
	
	This code existed in CursorDevicesGlue.o
	Now it's here so that I can profile and debug it.
	Also the MRC compiler is much better than the .o file
	Added support so this can be called from carbon apps
	
**********************************/

#if defined(__POWERPC__) && defined(__MAC__)

#include <CursorDevices.h>
#include <Gestalt.h>
#include <Patches.h>
#include <Traps.h>
#include <MacErrors.h>

/* Since Carbon doesn't define these in the headers, */
/* I have to define them here */

#if TARGET_API_MAC_CARBON
#include "ClStdLib.h"
enum {
	kOSTrapType = 0,
	kToolboxTrapType = 1
};
typedef SignedByte TrapType;
#define F(x) (*x)		/* Function pointer */
#define E				/* Remove the extern keyword */
#define REF LocalsPtr->	/* Convert a call to a proc pointer */
#else

/* Classic, allows these functions */

#define F(x) x			/* Use the function as is */
#define E extern		/* Allow the extern keyword */
#define REF				/* Call the proc directly */
#endif

/* ANSI "C" please... */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Locals_t {
	OSErr NoBugs;		/* If TRUE then it's ok to use the ROM */
	Word8 CursorTest;	/* If TRUE then Gestalt was called */
	Word8 Padding;		/* Longword align */
#if !TARGET_API_MAC_CARBON
} Locals_t;				/* End here for Classic */
#endif

/* These are the prototypes to the code in ROM */
/* Or they are proc pointers in Carbon */

	E pascal OSErr F(CrsrDevMove)(CursorDevicePtr ourDevice,long deltaX, long deltaY);
	E pascal OSErr F(CrsrDevMoveTo)(CursorDevicePtr ourDevice,long absX, long absY);
	E pascal OSErr F(CrsrDevFlush)(CursorDevicePtr ourDevice);
	E pascal OSErr F(CrsrDevButtons)(CursorDevicePtr ourDevice,short buttons);
	E pascal OSErr F(CrsrDevButtonDown)(CursorDevicePtr ourDevice);
	E pascal OSErr F(CrsrDevButtonUp)(CursorDevicePtr ourDevice);
	E pascal OSErr F(CrsrDevButtonOp)(CursorDevicePtr ourDevice,short buttonNumber,ButtonOpcode opcode,long data);
	E pascal OSErr F(CrsrDevSetButtons)(CursorDevicePtr ourDevice,short numberOfButtons);
	E pascal OSErr F(CrsrDevSetAcceleration)(CursorDevicePtr ourDevice,Fixed acceleration);
	E pascal OSErr F(CrsrDevDoubleTime)(CursorDevicePtr ourDevice,long durationTicks);
	E pascal OSErr F(CrsrDevUnitsPerInch)(CursorDevicePtr ourDevice,Fixed resolution);
	E pascal OSErr F(CrsrDevNextDevice)(CursorDevicePtr *ourDevice);
	E pascal OSErr F(CrsrDevNewDevice)(CursorDevicePtr *ourDevice);
	E pascal OSErr F(CrsrDevDisposeDevice)(CursorDevicePtr ourDevice);
#if TARGET_API_MAC_CARBON
	pascal long (*CallUniversalProc)(UniversalProcPtr theProcPtr,ProcInfoType procInfo,...);
	pascal UniversalProcPtr (*NGetTrapAddress)(UInt16 trapNum,TrapType tTyp);
} Locals_t;		/* End here for Carbon */

/* These are the MacOS prototypes */
/* Included here since Carbon makes them go away */

extern pascal OSErr CursorDeviceMove(CursorDevicePtr ourDevice,long deltaX, long deltaY);
extern pascal OSErr CursorDeviceMoveTo(CursorDevicePtr ourDevice,long absX, long absY);
extern pascal OSErr CursorDeviceFlush(CursorDevicePtr ourDevice);
extern pascal OSErr CursorDeviceButtons(CursorDevicePtr ourDevice,short buttons);
extern pascal OSErr CursorDeviceButtonDown(CursorDevicePtr ourDevice);
extern pascal OSErr CursorDeviceButtonUp(CursorDevicePtr ourDevice);
extern pascal OSErr CursorDeviceButtonOp(CursorDevicePtr ourDevice,short buttonNumber,ButtonOpcode opcode,long data);
extern pascal OSErr CursorDeviceSetButtons(CursorDevicePtr ourDevice,short numberOfButtons);
extern pascal OSErr CursorDeviceSetAcceleration(CursorDevicePtr ourDevice,Fixed acceleration);
extern pascal OSErr CursorDeviceDoubleTime(CursorDevicePtr ourDevice,long durationTicks);
extern pascal OSErr CursorDeviceUnitsPerInch(CursorDevicePtr ourDevice,Fixed resolution);
extern pascal OSErr CursorDeviceNextDevice(CursorDevicePtr *ourDevice);
extern pascal OSErr CursorDeviceNewDevice(CursorDevicePtr *ourDevice);
extern pascal OSErr CursorDeviceDisposeDevice(CursorDevicePtr ourDevice);
#endif
#ifdef __cplusplus
}
#endif

/* Clean up (But don't kill REF) */
#undef E
#undef F

/**********************************

	This code isn't called much, so don't inline
	anything to conserve space
	
**********************************/

#if defined(__MRC__)
#if TARGET_API_MAC_CARBON
#pragma noinline_func LoadFrags
#endif
#pragma noinline_func CheckCursorCalls,GetCursorTrap
#elif defined(__MWERKS__)
#pragma dont_inline on
#endif

static Locals_t Locals;		/* Data for me */

/**********************************

	For Carbon, I need to link in InterfaceLib
	manually to be able to use the Cursor manager
	from a Carbon app in Classic.
	
	Note: This will fail on MacOSX. Hence the paranoid
	code checking.
	
**********************************/

#if TARGET_API_MAC_CARBON
static void LoadFrags(Locals_t *LocalsPtr)
{
	LibRef_t *Lib;
	
	Lib = MacOSGetInterfaceLib();
	if (Lib) {									/* Will succeed in classic */
		if (!LocalsPtr->NoBugs) {				/* I Will call 680x0 code, sigh */
			REF CallUniversalProc = (pascal long(*)(UniversalProcPtr,ProcInfoType,...))LibRefGetProc(Lib,"CallUniversalProc");
			REF NGetTrapAddress = (pascal UniversalProcPtr(*)(UInt16,TrapType))LibRefGetProc(Lib,"NGetTrapAddress");
			if (REF CallUniversalProc && REF NGetTrapAddress) {
				return;
			}
		} else {
		
			/* Hooray, I'll call native PowerPC code */
			
			REF CrsrDevMove = (pascal OSErr(*)(CursorDevice*,long,long))LibRefGetProc(Lib,"CrsrDevMove");
			REF CrsrDevMoveTo = (pascal OSErr(*)(CursorDevice*,long,long))LibRefGetProc(Lib,"CrsrDevMoveTo");
			REF CrsrDevFlush = (pascal OSErr(*)(CursorDevice*))LibRefGetProc(Lib,"CrsrDevFlush");
			REF CrsrDevButtons = (pascal OSErr(*)(CursorDevice*,short))LibRefGetProc(Lib,"CrsrDevButtons");
			REF CrsrDevButtonDown = (pascal OSErr(*)(CursorDevice*))LibRefGetProc(Lib,"CrsrDevButtonDown");
			REF CrsrDevButtonUp = (pascal OSErr(*)(CursorDevice*))LibRefGetProc(Lib,"CrsrDevButtonUp");
			REF CrsrDevButtonOp = (pascal OSErr(*)(CursorDevice*,short,short,long))LibRefGetProc(Lib,"CrsrDevButtonOp");
			REF CrsrDevSetButtons = (pascal OSErr(*)(CursorDevice*,short))LibRefGetProc(Lib,"CrsrDevSetButtons");
			REF CrsrDevSetAcceleration = (pascal OSErr(*)(CursorDevice*,long))LibRefGetProc(Lib,"CrsrDevSetAcceleration");
			REF CrsrDevDoubleTime = (pascal OSErr(*)(CursorDevice*,long))LibRefGetProc(Lib,"CrsrDevDoubleTime");
			REF CrsrDevUnitsPerInch = (pascal OSErr(*)(CursorDevice*,long))LibRefGetProc(Lib,"CrsrDevUnitsPerInch");
			REF CrsrDevNextDevice = (pascal OSErr(*)(CursorDevice**))LibRefGetProc(Lib,"CrsrDevNextDevice");
			REF CrsrDevNewDevice = (pascal OSErr(*)(CursorDevice**))LibRefGetProc(Lib,"CrsrDevNewDevice");
			REF CrsrDevDisposeDevice = (pascal OSErr(*)(CursorDevice*))LibRefGetProc(Lib,"CrsrDevDisposeDevice");
			if (REF CrsrDevMove && REF CrsrDevMoveTo &&
				REF CrsrDevFlush && REF CrsrDevButtons && REF CrsrDevButtonDown && REF CrsrDevButtonUp &&
				REF CrsrDevButtonOp && REF CrsrDevSetButtons && REF CrsrDevSetAcceleration && REF CrsrDevDoubleTime &&
				REF CrsrDevUnitsPerInch && REF CrsrDevNextDevice && REF CrsrDevNewDevice && REF CrsrDevDisposeDevice) {
				return;
			}
		}
	}
	LocalsPtr->NoBugs = cfragNoSymbolErr;		/* Failure */
}
#endif

/**********************************

	This is a local routine
	Return TRUE if the CDM code is not fixed
	
**********************************/

static OSErr CheckCursorCalls(Locals_t *LocalsPtr)
{
	long gestaltAnswer;
	
	if (!LocalsPtr->CursorTest) {			/* Was it tested? */
		LocalsPtr->NoBugs = FALSE;			/* Assume ok */
		if (!Gestalt('bugx',&gestaltAnswer)) {		/* Get the bug check */
			if (gestaltAnswer&0x100000) {
				LocalsPtr->NoBugs = TRUE;	/* The fix is present */
			}
		}
#if TARGET_API_MAC_CARBON
		LoadFrags(LocalsPtr);			/* Load in the procs (Could fail) */
#endif
		LocalsPtr->CursorTest = TRUE;	/* Don't call gestalt again */
	}
	return LocalsPtr->NoBugs;			/* Return the result of gestalt */
}

/**********************************

	Get the trap address
	
**********************************/

static UniversalProcPtr GetCursorTrap(Locals_t *LocalsPtr)
{
	return REF NGetTrapAddress(_CursorDeviceDispatch,kToolboxTrapType);
}

/**********************************

	Call CursorDeviceMove
	
**********************************/

pascal OSErr CursorDeviceMove(CursorDevicePtr ourDevice,long deltaX, long deltaY)
{
	OSErr Flag;
	Locals_t *LocalsPtr;

	LocalsPtr = &Locals;
	Flag = CheckCursorCalls(LocalsPtr);
#if TARGET_API_MAC_CARBON
	if (Flag<0) {
		return Flag;
	}
#endif
	if (Flag) {
		return REF CrsrDevMove(ourDevice,deltaX,deltaY);
	}
	return REF CallUniversalProc(GetCursorTrap(LocalsPtr),
		kD0DispatchedPascalStackBased |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
		STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long))) |
		STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(CursorDevicePtr))) |
		STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long))) |
		STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long))),
		0,ourDevice,deltaX,deltaY);
}

/**********************************

	Call CursorDeviceMoveTo
	
**********************************/

pascal OSErr CursorDeviceMoveTo(CursorDevicePtr ourDevice,long absX, long absY)
{
	OSErr Flag;
	Locals_t *LocalsPtr;

	LocalsPtr = &Locals;
	Flag = CheckCursorCalls(LocalsPtr);
#if TARGET_API_MAC_CARBON
	if (Flag<0) {
		return Flag;
	}
#endif
	if (Flag) {
		return REF CrsrDevMoveTo(ourDevice,absX,absY);
	}
	return REF CallUniversalProc(GetCursorTrap(LocalsPtr),
		kD0DispatchedPascalStackBased |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
		STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long))) |
		STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(CursorDevicePtr))) |
		STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long))) |
		STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long))),
		1,ourDevice,absX,absY);
}

/**********************************

	Call CursorDeviceFlush
	
**********************************/

pascal OSErr CursorDeviceFlush(CursorDevicePtr ourDevice)
{
	OSErr Flag;
	Locals_t *LocalsPtr;

	LocalsPtr = &Locals;
	Flag = CheckCursorCalls(LocalsPtr);
#if TARGET_API_MAC_CARBON
	if (Flag<0) {
		return Flag;
	}
#endif
	if (Flag) {
		return REF CrsrDevFlush(ourDevice);
	}
	return REF CallUniversalProc(GetCursorTrap(LocalsPtr),
		kD0DispatchedPascalStackBased |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) | 
		STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long))) | 
		STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(CursorDevicePtr))),
		2,ourDevice);
}

/**********************************

	Call CursorDeviceButtons
	
**********************************/

pascal OSErr CursorDeviceButtons(CursorDevicePtr ourDevice,short buttons)
{
	OSErr Flag;
	Locals_t *LocalsPtr;

	LocalsPtr = &Locals;
	Flag = CheckCursorCalls(LocalsPtr);
#if TARGET_API_MAC_CARBON
	if (Flag<0) {
		return Flag;
	}
#endif
	if (Flag) {
		return REF CrsrDevButtons(ourDevice,buttons);
	}
	return REF CallUniversalProc(GetCursorTrap(LocalsPtr),
		kD0DispatchedPascalStackBased |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
		STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long))) |
		STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(CursorDevicePtr))) |
		STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short))),
		3,ourDevice,buttons);
}

/**********************************

	Call CursorDeviceButtonDown
	
**********************************/

pascal OSErr CursorDeviceButtonDown(CursorDevicePtr ourDevice)
{
	OSErr Flag;
	Locals_t *LocalsPtr;

	LocalsPtr = &Locals;
	Flag = CheckCursorCalls(LocalsPtr);
#if TARGET_API_MAC_CARBON
	if (Flag<0) {
		return Flag;
	}
#endif
	if (Flag) {
		return REF CrsrDevButtonDown(ourDevice);
	}
	return REF CallUniversalProc(GetCursorTrap(LocalsPtr),
		kD0DispatchedPascalStackBased |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) | 
		STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long))) | 
		STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(CursorDevicePtr))),
		4,ourDevice);
}

/**********************************

	Call CursorDeviceButtonUp
	
**********************************/

pascal OSErr CursorDeviceButtonUp(CursorDevicePtr ourDevice)
{
	OSErr Flag;
	Locals_t *LocalsPtr;

	LocalsPtr = &Locals;
	Flag = CheckCursorCalls(LocalsPtr);
#if TARGET_API_MAC_CARBON
	if (Flag<0) {
		return Flag;
	}
#endif
	if (Flag) {
		return REF CrsrDevButtonUp(ourDevice);
	}
	return REF CallUniversalProc(GetCursorTrap(LocalsPtr),
		kD0DispatchedPascalStackBased |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) | 
		STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long))) | 
		STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(CursorDevicePtr))),
		5,ourDevice);
}

/**********************************

	Call CursorDeviceButtonOp
	
**********************************/

pascal OSErr CursorDeviceButtonOp(CursorDevicePtr ourDevice,short buttonNumber,ButtonOpcode opcode,long data)
{
	OSErr Flag;
	Locals_t *LocalsPtr;

	LocalsPtr = &Locals;
	Flag = CheckCursorCalls(LocalsPtr);
#if TARGET_API_MAC_CARBON
	if (Flag<0) {
		return Flag;
	}
#endif
	if (Flag) {
		return REF CrsrDevButtonOp(ourDevice,buttonNumber,opcode,data);
	}
	return REF CallUniversalProc(GetCursorTrap(LocalsPtr),
		kD0DispatchedPascalStackBased |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
		STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long))) |
		STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(CursorDevicePtr))) |
		STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short))) |
		STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(ButtonOpcode))) |
		STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(long))),
		6,ourDevice,buttonNumber,opcode,data);
}

/**********************************

	Call CursorDeviceSetButtons
	
**********************************/

pascal OSErr CursorDeviceSetButtons(CursorDevicePtr ourDevice,short numberOfButtons)
{
	OSErr Flag;
	Locals_t *LocalsPtr;

	LocalsPtr = &Locals;
	Flag = CheckCursorCalls(LocalsPtr);
#if TARGET_API_MAC_CARBON
	if (Flag<0) {
		return Flag;
	}
#endif
	if (Flag) {
		return REF CrsrDevSetButtons(ourDevice,numberOfButtons);
	}
	return REF CallUniversalProc(GetCursorTrap(LocalsPtr),
		kD0DispatchedPascalStackBased |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
		STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long))) |
		STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(CursorDevicePtr))) |
		STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short))),
		7,ourDevice,numberOfButtons);
}

/**********************************

	Call CursorDeviceSetAcceleration
	
**********************************/

pascal OSErr CursorDeviceSetAcceleration(CursorDevicePtr ourDevice,Fixed acceleration)
{
	OSErr Flag;
	Locals_t *LocalsPtr;

	LocalsPtr = &Locals;
	Flag = CheckCursorCalls(LocalsPtr);
#if TARGET_API_MAC_CARBON
	if (Flag<0) {
		return Flag;
	}
#endif
	if (Flag) {
		return REF CrsrDevSetAcceleration(ourDevice,acceleration);
	}
	return REF CallUniversalProc(GetCursorTrap(LocalsPtr),
		kD0DispatchedPascalStackBased |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
		STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long))) |
		STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(CursorDevicePtr))) |
		STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Fixed))),
		8,ourDevice,acceleration);
}

/**********************************

	Call CursorDeviceDoubleTime
	
**********************************/

pascal OSErr CursorDeviceDoubleTime(CursorDevicePtr ourDevice,long durationTicks)
{
	OSErr Flag;
	Locals_t *LocalsPtr;

	LocalsPtr = &Locals;
	Flag = CheckCursorCalls(LocalsPtr);
#if TARGET_API_MAC_CARBON
	if (Flag<0) {
		return Flag;
	}
#endif
	if (Flag) {
		return REF CrsrDevDoubleTime(ourDevice,durationTicks);
	}
	return REF CallUniversalProc(GetCursorTrap(LocalsPtr),
		kD0DispatchedPascalStackBased |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
		STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long))) |
		STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(CursorDevicePtr))) |
		STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long))),
		9,ourDevice,durationTicks);
}

/**********************************

	Call CursorDeviceUnitsPerInch
	
**********************************/

pascal OSErr CursorDeviceUnitsPerInch(CursorDevicePtr ourDevice,Fixed resolution)
{
	OSErr Flag;
	Locals_t *LocalsPtr;

	LocalsPtr = &Locals;
	Flag = CheckCursorCalls(LocalsPtr);
#if TARGET_API_MAC_CARBON
	if (Flag<0) {
		return Flag;
	}
#endif
	if (Flag) {
		return REF CrsrDevUnitsPerInch(ourDevice,resolution);
	}
	return REF CallUniversalProc(GetCursorTrap(LocalsPtr),
		kD0DispatchedPascalStackBased |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
		STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long))) |
		STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(CursorDevicePtr))) |
		STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Fixed))),
		10,ourDevice,resolution);
}

/**********************************

	Call CursorDeviceNextDevice
	
**********************************/

pascal OSErr CursorDeviceNextDevice(CursorDevicePtr *ourDevice)
{
	OSErr Flag;
	Locals_t *LocalsPtr;

	LocalsPtr = &Locals;
	Flag = CheckCursorCalls(LocalsPtr);
#if TARGET_API_MAC_CARBON
	if (Flag<0) {
		return Flag;
	}
#endif
	if (Flag) {
		return REF CrsrDevNextDevice(ourDevice);
	}
	return REF CallUniversalProc(GetCursorTrap(LocalsPtr),
		kD0DispatchedPascalStackBased |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) | 
		STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long))) | 
		STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(CursorDevicePtr *))),
		11,ourDevice);
}

/**********************************

	Call CursorDeviceNewDevice
	
**********************************/

pascal OSErr CursorDeviceNewDevice(CursorDevicePtr *ourDevice)
{
	OSErr Flag;
	Locals_t *LocalsPtr;

	LocalsPtr = &Locals;
	Flag = CheckCursorCalls(LocalsPtr);
#if TARGET_API_MAC_CARBON
	if (Flag<0) {
		return Flag;
	}
#endif
	if (Flag) {
		return REF CrsrDevNewDevice(ourDevice);
	}
	return REF CallUniversalProc(GetCursorTrap(LocalsPtr),
		kD0DispatchedPascalStackBased |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) | 
		STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long))) | 
		STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(CursorDevicePtr *))),
		12,ourDevice);
}

/**********************************

	Call CursorDeviceDisposeDevice
	
**********************************/

pascal OSErr CursorDeviceDisposeDevice(CursorDevicePtr ourDevice)
{
	OSErr Flag;
	Locals_t *LocalsPtr;

	LocalsPtr = &Locals;
	Flag = CheckCursorCalls(LocalsPtr);
#if TARGET_API_MAC_CARBON
	if (Flag<0) {
		return Flag;
	}
#endif
	if (Flag) {
		return REF CrsrDevDisposeDevice(ourDevice);
	}
	return REF CallUniversalProc(GetCursorTrap(LocalsPtr),
		kD0DispatchedPascalStackBased |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) | 
		STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long))) | 
		STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(CursorDevicePtr))),
		13,ourDevice);
}

#endif
