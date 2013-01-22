/*******************************

	Burger's Universal library WIN95 version
	This is for Watcom 10.5 and higher...
	Also support for MSVC 4.0

*******************************/

#ifndef __OCOSCURSOR_H__
#define __OCOSCURSOR_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* OS Cursor handler */

extern Bool OSCursorVisibleFlag;
extern Word OSCursorLastCursor;
#if defined(__MAC__)
extern struct CCrsr **OSCursorPreviousCur;
#endif

extern void BURGERCALL OSCursorSet(Word Curnum);
extern void BURGERCALL OSCursorShow(void);
extern void BURGERCALL OSCursorHide(void);
extern void BURGERCALL OSCursorReset(void);
extern Word BURGERCALL OSCursorPresent(void);
extern Word BURGERCALL OSCursorIsVisible(void);
extern Word BURGERCALL OSCursorNumber(void);

#ifdef __cplusplus
}
#endif


#endif
