#include "Text.h"
#include <string.h>

/*******************************

	Scan the token list for a match and return
	the token number or -1 for no match

*******************************/

Word VarToken(Byte *Token,Byte **TokenList)
{
	Word i;
	i = 0;				/* Init the token index */
	do {
		if (!stricmp((char *)Token,(char *)(TokenList[0]))) {		/* Match string? */
			return i;		 /* Return the token # */
		}
		++i;		/* Next index */
		++TokenList;
	} while (TokenList[0]);		/* No more tokens? */
	return -1;		/* Return BAD token number */
}

