#include "McMac.h"

/**********************************

	MacOS Only!!!
	
	There was some oversight on the part of apple not
	to include PPC versions of these routines.
	So here they are.
	
	Addendum 2001 : There is now ControlStripLib from Apple
	so this is now obsolete. I only keep this file around for 
	example code for PPC <-> 68k calls
	
**********************************/

#if defined(__POWERPC__) && defined(__MAC__) && 0

#include <ControlStrip.h>

#if !TARGET_API_MAC_CARBON
/**********************************

	Is the control strip visible?
	
**********************************/


static Word16 SBIsControlStripVisible68K[] = {	// Bool Foo(void);
	0x554F,	// SUBQ.W    #$2,A7
	0x7000,	// MOVEQ     #$00,D0
	0xAAF2,	// DC.W      $AAF2          ; TB 02F2
	0x101F,	// MOVE.B    (A7)+,D0
	0x4E75	// RTS
};

pascal Bool SBIsControlStripVisible(void)
{
	return CallUniversalProc((UniversalProcPtr)&SBIsControlStripVisible68K[0],kRegisterBased|RESULT_SIZE(SIZE_CODE(1)));
}

/**********************************

	Hide or show the control strip
	
**********************************/

static Word16 SBShowHideControlStrip68K[] = {	// pascal void Foo(Bool);
	0x1F2F,0x0004,	// MOVE.B 4(A7),-(A7)
	0x303C,0x0101,	// MOV #$101,D0
	0xAAF2,			// DC.W      $AAF2          ; TB 02F2
	0x4E74,0x0002	// RTD #2
};

pascal void SBShowHideControlStrip(Bool showIt)
{
	CallUniversalProc((UniversalProcPtr)&SBShowHideControlStrip68K[0],kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(1)),showIt);
}

/**********************************

	Safe to access Startup Disk?
	
**********************************/

static Word16 SBSafeToAccessStartupDisk68K[] = {	// Bool Foo(void);
	0x554F,	// SUBQ.W    #$2,A7
	0x7000,	// MOVEQ     #$00,D0
	0xAAF2,	// DC.W      $AAF2          ; TB 02F2
	0x101F,	// MOVE.B    (A7)+,D0
	0x4E75	// RTS
};

pascal Bool SBSafeToAccessStartupDisk(void)
{
	return CallUniversalProc((UniversalProcPtr)&SBSafeToAccessStartupDisk68K[0],kRegisterBased|RESULT_SIZE(SIZE_CODE(1)));
}

/**********************************

	I'll do these later if anyone REALLY needs them!
	
**********************************/

//pascal short SBOpenModuleResourceFile(OSType fileCreator);	THREEWORDINLINE(0x303C, 0x0203, 0xAAF2);
//pascal OSErr SBLoadPreferences(ConstStr255Param prefsResourceName,Handle *preferences) THREEWORDINLINE(0x303C, 0x0404, 0xAAF2);
//pascal OSErr SBSavePreferences(ConstStr255Param prefsResourceName,Handle preferences) THREEWORDINLINE(0x303C, 0x0405, 0xAAF2);
//pascal void SBGetDetachedIndString(StringPtr theString,Handle stringList,short whichString) THREEWORDINLINE(0x303C, 0x0506, 0xAAF2);
//pascal OSErr SBGetDetachIconSuite(Handle *theIconSuite,short theResID, unsigned long selector) THREEWORDINLINE(0x303C, 0x0507, 0xAAF2);
//pascal short SBTrackPopupMenu(const Rect *moduleRect,MenuHandle theMenu) THREEWORDINLINE(0x303C, 0x0408, 0xAAF2);
//pascal short SBTrackSlider(const Rect *moduleRect,short ticksOnSlider,short initialValue) THREEWORDINLINE(0x303C, 0x0409, 0xAAF2);
//pascal OSErr SBShowHelpString(const Rect *moduleRect,StringPtr helpString) THREEWORDINLINE(0x303C, 0x040A, 0xAAF2);
//pascal short SBGetBarGraphWidth(short barCount) THREEWORDINLINE(0x303C, 0x010B, 0xAAF2);
//pascal void SBDrawBarGraph(short level,short barCount,short direction, Point barGraphTopLeft) THREEWORDINLINE(0x303C, 0x050C, 0xAAF2);
//pascal void SBModalDialogInContext (ModalFilterUPP filterProc,short *itemHit) THREEWORDINLINE(0x303C, 0x040D, 0xAAF2);
//pascal OSErr SBGetControlStripFontID(short *fontID) THREEWORDINLINE(0x303C, 0x020E, 0xAAF2);
//pascal OSErr SBSetControlStripFontID(short fontID) THREEWORDINLINE(0x303C, 0x010F, 0xAAF2);
//pascal OSErr SBGetControlStripFontSize(short *fontSize) THREEWORDINLINE(0x303C, 0x0210, 0xAAF2);
//pascal OSErr SBSetControlStripFontSize(short fontSize) THREEWORDINLINE(0x303C, 0x0111, 0xAAF2);
//pascal OSErr SBGetShowHideHotKey(short *modifiers, unsigned char *keyCode)	THREEWORDINLINE(0x303C, 0x0412, 0xAAF2);
//pascal OSErr SBSetShowHideHotKey(short modifiers, unsigned char	keyCode) THREEWORDINLINE(0x303C, 0x0213, 0xAAF2);
//pascal OSErr SBIsShowHideHotKeyEnabled(Bool * enabled) THREEWORDINLINE(0x303C, 0x0214, 0xAAF2);
//pascal OSErr SBEnableShowHideHotKey(Bool enabled) THREEWORDINLINE(0x303C, 0x0115, 0xAAF2);
//pascal short SBHitTrackSlider(const Rect *moduleRect,short ticksOnSlider,short initialValue,Bool *hit) THREEWORDINLINE(0x303C, 0x0616, 0xAAF2);

#endif
#endif