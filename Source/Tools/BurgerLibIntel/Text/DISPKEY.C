#include "Text.h"

/*********************************

	Accept text input and dispatch
	if a key in the list was hit.

*********************************/

void DispatchKey(Word Flag,DispatchKey_t *Vectors)
{
	Word NewKey;
	DispatchKey_t *VecPtr;

	for (;;) {
		if (Flag&DKCursor) {		/* Shall I display a cursor? */
			NewKey = GetACursorKey();	/* Show cursor */
		} else {
			NewKey = KeyboardGet();		/* No cursor */
		}
		if ((Flag&DKUpperCase) && (NewKey>='a' && NewKey<('z'+1))) {	/* Force input to upper case? */
			NewKey &= 0xFFDF;		/* Force upper case */
		}
		VecPtr = Vectors;			/* Init pointer */
		for (;;) {
			if (VecPtr->Key==NewKey && VecPtr->Code) {	/* Key match and code present? */
				VecPtr->Code();		/* Call the code */
				if (!NewKey) {		/* Null event? */
					break;			/* Stay here */
				}
				return;				/* Exit */
			}
			if (!VecPtr->Key) {		/* End of the list? */
				break;				/* Abort */
			}
			++VecPtr; 				/* Next entry */
		}
	}
}

