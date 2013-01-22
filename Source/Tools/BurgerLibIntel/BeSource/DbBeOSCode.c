#include "ClStdLib.h"

/********************************

	BeOS Release version

********************************/

#if defined(__BEOS__)
#include <Alert.h>			/* Include the Be headers */

/**********************************

	Show a simple alert dialog

**********************************/

void BURGERCALL OkAlertMessage(const char *Title,const char *Message)
{
	BAlert *MyAlert = new BAlert(Title,Message,"Ok");
	MyAlert->Go();			/* Execute the message */
							/* The alert self-destructed */
}

/**********************************

	Show an Cancel/Ok alert dialog

**********************************/

Word BURGERCALL OkCancelAlertMessage(const char *Title,const char *Message)
{
	BAlert *MyAlert = new BAlert(Title,Message,"Cancel","Ok");
	return !MyAlert->Go();			/* Execute the message */
									/* The alert self-destructed */
}

#endif
