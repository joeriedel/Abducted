#include "Text.h"

/****************************************

	Convert an ASCII 0-9, A-F, a-f into
	a hex nibble.
	Return -1 if invalid.

****************************************/

Word AscHex(Word Nibble)
{
	Nibble-='0';			/* Convert 0-9 */
	if (Nibble<10) {
		return Nibble;		/* Return 0-9 */
	}
	if (Nibble>=0x11 && Nibble<0x37 && (Nibble<0x17 || Nibble>=0x31)) {
		return (Nibble-7)&0xf;		/* Return 10-15 */
	}
	return -1;				/* Input is foobar'd */
}

