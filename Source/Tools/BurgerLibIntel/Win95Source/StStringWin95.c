#include "StString.h"

/**********************************

	Win 95 specific code

**********************************/

#if defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/**********************************

	Get the computer's name
	
**********************************/

void BURGERCALL StrGetComputerName(char* Output,Word OutputSize)
{
	DWORD NameSize;		/* Length to pass to Win95 */

	if (OutputSize) {
		NameSize = OutputSize;
		if (!GetComputerName(Output,&NameSize)) {	/* Get the name */

		/* The name wasn't present, use the default */

			--OutputSize;
			strncpy(Output,"Computer",OutputSize);	/* Copy to buffer */
			Output[OutputSize] = 0;		/* Make SURE it's zero terminated! */
		}
	}
}

/**********************************

	Get the user's name

**********************************/

void BURGERCALL StrGetUserName(char* Output,Word OutputSize)
{
	DWORD NameSize;		/* Length to pass to Win95 */

	if (OutputSize) {
		NameSize = OutputSize;
		if (!GetUserName(Output,&NameSize)) {	/* Get the name */

		/* The name wasn't present, use the default */

			--OutputSize;
			strncpy(Output,"User",OutputSize);	/* Copy to buffer */
			Output[OutputSize] = 0;		/* Make SURE it's zero terminated! */
		}
	}
}

#endif