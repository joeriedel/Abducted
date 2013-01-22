#define DIRECTINPUT_VERSION 0x700
#include "InInput.h"

#if defined(__WIN32__)
#include "MmMemory.h"
#include "ClStdLib.h"
#include "W9Win95.h"
#include "FmFile.h"
#include "RzRez.h"
#define WIN32_LEAN_AND_MEAN
#include <dinput.h>
#include <IForce2.h>

#if DIRECTINPUT_VERSION <= 0x700
#define DEVTYPE_JOY DIDEVTYPE_JOYSTICK
#else
#define DEVTYPE_JOY DI8DEVCLASS_GAMECTRL
#endif

struct ForceFeedbackEffect_t {
	IDirectInputEffect **EffectPtr;		/* Pointer to an array of effect pointers */
	ForceFeedbackData_t *SourceFile;	/* Pointer to the file reference */
	int Components;						/* Number of parts present */
};

struct ForceFeedbackData_t {
	HIFORCEPROJECT force_file;			/* Refence to a force feedback data file */
};

struct ForceFeedback_t {
	IDirectInput *InterfacePtr;			/* Gives us access to DirectInput COM shit */
	IDirectInputDevice2 *DevicePtr;		/* Pointer to the feedback device */
};

/**********************************

	Scan all devices until a joystick match is found
	
**********************************/

static BOOL CALLBACK ForceFeedbackEnumDevicesProc(LPCDIDEVICEINSTANCE cur_device_instance,
	LPVOID user_data)
{
	if (user_data) {		/* Make sure the pointer is valid */
		if (GET_DIDEVICE_TYPE(cur_device_instance->dwDevType) == DEVTYPE_JOY) {
			FastMemCpy(user_data,cur_device_instance, sizeof(DIDEVICEINSTANCE));
			return DIENUM_STOP;		/* Don't scan anymore */
		}
	}
	return DIENUM_CONTINUE;		/* Continue scanning */
}

/**********************************

	Init forcefeedback
	
**********************************/

ForceFeedback_t *ForceFeedbackNew(void)
{
	ForceFeedback_t *Result;
	DIDEVICEINSTANCE guid_device_instance;
	LPDIRECTINPUTDEVICE dummy_directinput1_ptr;

	/* Allocate the force feedback structure */

	Result = (ForceFeedback_t *)AllocAPointer(sizeof(ForceFeedback_t));
	if (Result) {

		/* Let's talk to direct input shall we? */

		if (DirectInputCreate(GetModuleHandle(0),DIRECTINPUT_VERSION,
				&Result->InterfacePtr,0)==DI_OK) {

			/* Scan for the joystick */

			FastMemSet(&guid_device_instance,0,sizeof(guid_device_instance));
			if (IDirectInput_EnumDevices(Result->InterfacePtr,
				DEVTYPE_JOY,ForceFeedbackEnumDevicesProc,
				&guid_device_instance,DIEDFL_FORCEFEEDBACK | DIEDFL_ATTACHEDONLY)==DI_OK) {

				if (guid_device_instance.dwDevType) {			/* Find anything? */

				// OK, found a ff joystick. Create the temp DirectInput 1 Device object
				// (which we need in order to access DirectInput 2 interfaces)

					dummy_directinput1_ptr = 0;		/* Init */
					if (IDirectInput_CreateDevice(Result->InterfacePtr,
						guid_device_instance.guidInstance,
						&dummy_directinput1_ptr,0)==DI_OK) {
						HRESULT ErrCode;

					/// Need DirectInputDevice2 for force feedback, so get ptr to it
						ErrCode = IDirectInput_QueryInterface(dummy_directinput1_ptr,
							IID_IDirectInputDevice2,(void **)&Result->DevicePtr);

						/// No longer need temp DirectInput 1 obj, so release it (yuck!)
						IDirectInput_Release (dummy_directinput1_ptr);

						if (ErrCode==DI_OK) {		/* Now act on the result */

						/// Now we tell our device to act like a specific devie - i.e. a
						/// friggin' joystick!
							if (IDirectInputDevice2_SetDataFormat(Result->DevicePtr,&c_dfDIJoystick)==DI_OK) {

						/// Set the cooperative level
								if (IDirectInputDevice2_SetCooperativeLevel (
									Result->DevicePtr,(HWND)Win95MainWindow,
									DISCL_EXCLUSIVE | DISCL_FOREGROUND)==DI_OK) {
						/// Finally, "acquire" the joystick
									if (IDirectInputDevice2_Acquire(Result->DevicePtr)==DI_OK) {
										/// Make sure it's force feedback
										DIDEVCAPS joy_caps;
										FastMemSet(&joy_caps,0,sizeof(joy_caps));
										joy_caps.dwSize = sizeof (joy_caps);
										IDirectInputDevice2_GetCapabilities(Result->DevicePtr,&joy_caps);		
										if (joy_caps.dwFlags & DIDC_FORCEFEEDBACK) {
											return Result;			/* It's cool!!! */
										}
										IDirectInputDevice2_Unacquire(Result->DevicePtr);
									}
								}
							}
							IDirectInputDevice2_Release(Result->DevicePtr);
						}
					}
				}
			}
			IDirectInput_Release(Result->InterfacePtr);
		}
		DeallocAPointer(Result);
	}	
	return 0;
}


/**********************************

	Delete the force feedback reference
	
**********************************/

void BURGERCALL ForceFeedbackDelete(ForceFeedback_t *RefPtr)
{
	if (RefPtr) {
		IDirectInputDevice2_Unacquire(RefPtr->DevicePtr);
		IDirectInputDevice2_Release(RefPtr->DevicePtr);
		IDirectInput_Release(RefPtr->InterfacePtr);
		DeallocAPointer(RefPtr);
	}
}

/**********************************

	In the event of a system context switch,
	reaquire the focus.

**********************************/

void BURGERCALL ForceFeedbackReacquire(ForceFeedback_t *RefPtr)
{
	if (RefPtr) {
		IDirectInputDevice2_Acquire(RefPtr->DevicePtr);
	}
}

/**********************************

	Open a force feedback effect file
	and create a ForceFeedbackFile_t out of it...
	
**********************************/

ForceFeedbackData_t * BURGERCALL ForceFeedbackDataNew(ForceFeedback_t *RefPtr,const char *FilenamePtr)
{
	ForceFeedbackData_t *Result;
	char PathName[FULLPATHSIZE];		/* Full pathname */

	if (RefPtr && FilenamePtr && FilenamePtr[0]) {
		Result = (ForceFeedbackData_t *)AllocAPointer(sizeof (ForceFeedbackData_t));
		if (Result) {

			ExpandAPathToBufferNative(PathName,FilenamePtr);		/* Convert to native file system */
			Result->force_file = IFLoadProjectFile(PathName,RefPtr->DevicePtr);
			if (Result->force_file) {		/* Did it open? */
				return Result;				/* Got it! */
			}
			DeallocAPointer(Result);		/* Kill my structure! */
		}
	}
	return 0;		/* Nothing loaded */
}

/**********************************

	Open a force feedback effect file
	and create a ForceFeedbackFile_t out of it...
	
**********************************/

ForceFeedbackData_t * BURGERCALL ForceFeedbackDataNewRez(ForceFeedback_t *RefPtr,RezHeader_t *RezRef,Word RezNum)
{
	ForceFeedbackData_t *Result;
	void *DataPtr;

	if (RefPtr && RezRef) {
		Result = (ForceFeedbackData_t *)AllocAPointer(sizeof (ForceFeedbackData_t));
		if (Result) {
			DataPtr = ResourceLoad(RezRef,RezNum);		/* Load in the IFR file */
			if (DataPtr) {
				Result->force_file = IFLoadProjectPointer(DataPtr,RefPtr->DevicePtr);
				ResourceRelease(RezRef,RezNum);			/* Release the IFR file */
				if (Result->force_file) {		/* Did it open? */
					return Result;				/* Got it! */
				}
			}
			DeallocAPointer(Result);		/* Kill my structure! */
		}
	}
	return 0;		/* Nothing loaded */
}

/**********************************

	Release an effect file record and dispose of its memory
	
**********************************/

void BURGERCALL ForceFeedbackDataDelete(ForceFeedbackData_t *FilePtr)
{
	if (FilePtr) {		/* Is the record valid? */
		if (FilePtr->force_file) {		/* Is there a file allocated? */
			IFReleaseProject(FilePtr->force_file);	/* Release the file */
		}
		DeallocAPointer(FilePtr);	/* Dispose of the struct */
	}
}

/**********************************

	Load a force feedback effect from the effects file
	
**********************************/

ForceFeedbackEffect_t * BURGERCALL ForceFeedbackEffectNew(
	ForceFeedbackData_t *FilePtr,const char *EffectNamePtr)
{
	ForceFeedbackEffect_t *Result;			/* Pointer to the data */
	IDirectInputEffect **loaded_effect;		/* Temp data */

	if (FilePtr && EffectNamePtr && FilePtr->force_file) {
		Result = (ForceFeedbackEffect_t *)AllocAPointer(sizeof(ForceFeedbackEffect_t));
		if (Result) {
			loaded_effect = IFCreateEffects(FilePtr->force_file,EffectNamePtr,&Result->Components);
			if (loaded_effect) {
				Result->EffectPtr = loaded_effect;		/* Init the structure */
				Result->SourceFile = FilePtr;
				return Result;
			}
			DeallocAPointer(Result);		/* Kill the memory... */
		}
	}
	return 0;		/* Effect is not found! */
}

/**********************************

	Kill an effect file
	
**********************************/

void BURGERCALL ForceFeedbackEffectDelete(ForceFeedbackEffect_t *Input)
{
	if (Input) {
		if (Input->EffectPtr) {
			IFReleaseEffects(Input->SourceFile,Input->EffectPtr);
		}
		DeallocAPointer(Input);
	}
}

/**********************************

	Start a force feedback event
	
**********************************/

Word BURGERCALL ForceFeedbackEffectPlay(ForceFeedbackEffect_t *Input)
{
	if (Input && Input->EffectPtr) {
		int i;
		i = Input->Components;
		if (i) {
			IDirectInputEffect **EffectPtr;
			EffectPtr = Input->EffectPtr;
			do {
				IDirectInputEffect_Start(EffectPtr[0],1,0);
				++EffectPtr;
			} while (--i);
			return FALSE;
		}
	}
	return TRUE;		/* Error! */
}

/**********************************

	If the force feedback event is playing, stop it
	
**********************************/

void BURGERCALL ForceFeedbackEffectStop(ForceFeedbackEffect_t *Input)
{
	if (Input && Input->EffectPtr) {
		int i;
		i = Input->Components;
		if (i) {
			IDirectInputEffect **EffectPtr;
			EffectPtr = Input->EffectPtr;
			do {
				IDirectInputEffect_Stop(EffectPtr[0]);	/* Stop the effect */
				++EffectPtr;
			} while (--i);
		}
	}
}

/**********************************

	Return TRUE if the effect is currently playing
	
**********************************/

Word BURGERCALL ForceFeedbackEffectIsPlaying(ForceFeedbackEffect_t *Input)
{
	DWORD force_flags;

	if (Input && Input->EffectPtr) {				/* Valid? */
		IDirectInputEffect_GetEffectStatus(Input->EffectPtr[0],&force_flags);
		if (force_flags & DIEGES_PLAYING) {
			return TRUE;
		}
	}
	return FALSE;
}

/**********************************

	Set the gain of the effect
	
**********************************/

void BURGERCALL ForceFeedbackEffectSetGain(ForceFeedbackEffect_t *Input,long NewGain)
{
	if (Input && Input->EffectPtr) {				/* Valid? */
		DIEFFECT ParmList;
		FastMemSet(&ParmList,0,sizeof(ParmList));
		if (NewGain>10000) {
			NewGain = 10000;
		}
		if (NewGain<-10000) {
			NewGain = -10000;
		}
		ParmList.dwGain = NewGain;
		ParmList.dwSize = sizeof(ParmList);
		IDirectInputEffect_SetParameters(Input->EffectPtr[0],&ParmList,DIEP_GAIN);
	}
}

/**********************************

	Set the duration of the effect in microseconds
	
**********************************/

void BURGERCALL ForceFeedbackEffectSetDuration(ForceFeedbackEffect_t *Input,Word32 NewDuration)
{
	if (Input && Input->EffectPtr) {				/* Valid? */
		DIEFFECT ParmList;
		FastMemSet(&ParmList,0,sizeof(ParmList));
		ParmList.dwDuration = NewDuration;
		ParmList.dwSize = sizeof(ParmList);
		IDirectInputEffect_SetParameters(Input->EffectPtr[0],&ParmList,DIEP_DURATION);
	}
}

#endif