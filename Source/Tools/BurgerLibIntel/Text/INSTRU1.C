#include "Text.h"

/************************************

	Accept input from an array of input controls
	such as line edits, number choices etc...
	This does not accept callbacks, use InputStruct2
	for callback access

************************************/

Word InputStruct(EntryList_t *EntryPtr)
{
	return InputStruct2(EntryPtr,0);
}
