#include "InInput.h"

#if !defined(__WIN32__)
#include "RzRez.h"

/**********************************

	Init forcefeedback
	
**********************************/

ForceFeedback_t *ForceFeedbackNew(void)
{
	return 0;
}


/**********************************

	Delete the force feedback reference
	
**********************************/

void BURGERCALL ForceFeedbackDelete(ForceFeedback_t * /* RefPtr */)
{
}

/**********************************

	In the event of a system context switch,
	reaquire the focus.

**********************************/

void BURGERCALL ForceFeedbackReacquire(ForceFeedback_t * /*RefPtr */)
{
}

/**********************************

	Open a force feedback effect file
	and create a ForceFeedbackFile_t out of it...
	
**********************************/

ForceFeedbackData_t * BURGERCALL ForceFeedbackDataNew(ForceFeedback_t * /* RefPtr */,const char * /* FilenamePtr */)
{
	return 0;		/* Nothing loaded */
}

/**********************************

	Open a force feedback effect file
	and create a ForceFeedbackFile_t out of it...
	
**********************************/

ForceFeedbackData_t * BURGERCALL ForceFeedbackDataNewRez(ForceFeedback_t * /* RefPtr */,RezHeader_t * /* RezRef */,Word /* RezNum */)
{
	return 0;		/* Nothing loaded */
}

/**********************************

	Release an effect file record and dispose of its memory
	
**********************************/

void BURGERCALL ForceFeedbackDataDelete(ForceFeedbackData_t * /* FilePtr */)
{
}

/**********************************

	Load a force feedback effect from the effects file
	
**********************************/

ForceFeedbackEffect_t * BURGERCALL ForceFeedbackEffectNew(ForceFeedbackData_t * /* FilePtr */,const char * /* EffectNamePtr */)
{
	return 0;		/* Effect is not found! */
}

/**********************************

	Kill an effect file
	
**********************************/

void BURGERCALL ForceFeedbackEffectDelete(ForceFeedbackEffect_t * /* Input */)
{
}

/**********************************

	Start a force feedback event
	
**********************************/

Word BURGERCALL ForceFeedbackEffectPlay(ForceFeedbackEffect_t * /* Input */)
{
	return TRUE;		/* Error! */
}

/**********************************

	If the force feedback event is playing, stop it
	
**********************************/

void BURGERCALL ForceFeedbackEffectStop(ForceFeedbackEffect_t * /* Input */)
{
}

/**********************************

	Return TRUE if the effect is currently playing
	
**********************************/

Word BURGERCALL ForceFeedbackEffectIsPlaying(ForceFeedbackEffect_t * /* Input */)
{
	return FALSE;
}

/**********************************

	Set the gain of the effect
	
**********************************/

void BURGERCALL ForceFeedbackEffectSetGain(ForceFeedbackEffect_t * /* Input */,long /* NewGain */)
{
}

/**********************************

	Set the duration of the effect in microseconds
	
**********************************/

void BURGERCALL ForceFeedbackEffectSetDuration(ForceFeedbackEffect_t * /* Input */,Word32 /* NewDuration */)
{
}

#endif