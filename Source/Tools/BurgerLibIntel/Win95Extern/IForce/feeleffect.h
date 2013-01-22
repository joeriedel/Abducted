/**********************************************************************
	Copyright (c) 1997 Immersion Corporation

	Permission to use, copy, modify, distribute, and sell this
	software and its documentation may be granted without fee;
	interested parties are encouraged to request permission from
		Immersion Corporation
		2158 Paragon Drive
		San Jose, CA 95131
		408-467-1900

	IMMERSION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
	IN NO EVENT SHALL IMMERSION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
	LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
	NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
	CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

  FILE:		FeelEffect.h

  PURPOSE:	Base Effect Class for Feelit API Foundation Classes

  STARTED:	10/10/97

  NOTES/REVISIONS:
     3/2/99 jrm (Jeff Mallett): Force-->Feel renaming
	 3/2/99 jrm: Added GetIsCompatibleGUID and feel_to_DI_GUID
	 3/15/99 jrm: __declspec(dllimport/dllexport) the whole class

**********************************************************************/


#ifndef INCLUDED_FEELEFFECT_H
#define INCLUDED_FEELEFFECT_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef _FFCDLL_
#define DLLFFC __declspec(dllimport)
#else
#define DLLFFC __declspec(dllexport)
#endif

#ifndef INCLUDED_FEELBASETYPES_H
#include <feelbasetypes.h>
#endif

#ifndef INCLUDED_FEELDEVICE_H
#include <feeldevice.h>
#endif

class CFeelProject;


//================================================================
// Constants
//================================================================

#define     FEEL_EFFECT_AXIS_X             1
#define     FEEL_EFFECT_AXIS_Y             2
#define     FEEL_EFFECT_AXIS_BOTH          3
#define     FEEL_EFFECT_AXIS_DIRECTIONAL   4
#define     FEEL_EFFECT_DONT_CHANGE        MINLONG
#define     FEEL_EFFECT_DONT_CHANGE_PTR    MAXDWORD
const POINT FEEL_EFFECT_DONT_CHANGE_POINT = { 0xFFFFFFFF, 0xFFFFFFFF };
const POINT FEEL_EFFECT_MOUSE_POS_AT_START = { MAXLONG, MAXLONG };

#define     FEEL_EFFECT_DEFAULT_ENVELOPE   NULL
#define     FEEL_EFFECT_DEFAULT_DIRECTION_X    1
#define     FEEL_EFFECT_DEFAULT_DIRECTION_Y    1
#define     FEEL_EFFECT_DEFAULT_ANGLE          0

//
// FORCE --> FEEL Wrappers
//
#define     FORCE_EFFECT_AXIS_X					FEEL_EFFECT_AXIS_X
#define     FORCE_EFFECT_AXIS_Y					FEEL_EFFECT_AXIS_Y
#define     FORCE_EFFECT_AXIS_BOTH				FEEL_EFFECT_AXIS_BOTH
#define     FORCE_EFFECT_AXIS_DIRECTIONAL		FEEL_EFFECT_AXIS_DIRECTIONAL
#define     FORCE_EFFECT_DONT_CHANGE			FEEL_EFFECT_DONT_CHANGE
#define     FORCE_EFFECT_DONT_CHANGE_PTR		FEEL_EFFECT_DONT_CHANGE_PTR
#define     FORCE_EFFECT_DONT_CHANGE_POINT		FEEL_EFFECT_DONT_CHANGE_POINT
#define     FORCE_EFFECT_MOUSE_POS_AT_START		FEEL_EFFECT_MOUSE_POS_AT_START

#define     FORCE_EFFECT_DEFAULT_ENVELOPE		FEEL_EFFECT_DEFAULT_ENVELOPE
#define     FORCE_EFFECT_DEFAULT_DIRECTION_X	FEEL_EFFECT_DEFAULT_DIRECTION_X
#define     FORCE_EFFECT_DEFAULT_DIRECTION_Y	FEEL_EFFECT_DEFAULT_DIRECTION_Y
#define     FORCE_EFFECT_DEFAULT_ANGLE			FEEL_EFFECT_DEFAULT_ANGLE



// GENERIC_EFFECT_PTR
// This is really a pointer to a child of CFeelEffect.
typedef class CFeelEffect * 	GENERIC_EFFECT_PTR;


//================================================================
// CFeelEffect
//================================================================

//
// ------ PUBLIC INTERFACE ------ 
//

class DLLFFC CFeelEffect  
{
    //
    // CONSTRUCTOR/DESTRUCTOR
    //

    public:
    
    // Constructor
	CFeelEffect(
		const GUID& rguidEffect
		);

	// Destructor
	virtual
	~CFeelEffect();

    //
    // ATTRIBUTES
    //

    public:

	LPIFEEL_EFFECT
    GetEffect() 
		{ return m_piFeelitEffect; }

    BOOL 
    GetStatus(
		DWORD* pdwStatus
		);

    GUID
	GetGUID()
		{ return m_guidEffect; }

	virtual BOOL
	GetIsCompatibleGUID(
		GUID & /* guid */
		)
		{ return true; }
 
	// Allocates an object of the correct FFC class from the given GUID
	static GENERIC_EFFECT_PTR
	NewObjectFromGUID(
		GUID &guid
		);

    BOOL
    ChangeBaseParams( 
        LONG lDirectionX,
        LONG lDirectionY,
        DWORD dwDuration = FEEL_EFFECT_DONT_CHANGE,
        LPFEEL_ENVELOPE pEnvelope = (LPFEEL_ENVELOPE) FEEL_EFFECT_DONT_CHANGE_PTR,
        DWORD dwSamplePeriod = FEEL_EFFECT_DONT_CHANGE,
        DWORD dwGain = FEEL_EFFECT_DONT_CHANGE,
        DWORD dwTriggerButton = FEEL_EFFECT_DONT_CHANGE,
        DWORD dwTriggerRepeatInterval = FEEL_EFFECT_DONT_CHANGE
        );

    BOOL
    ChangeBaseParamsPolar( 
        LONG lAngle,
        DWORD dwDuration = FEEL_EFFECT_DONT_CHANGE,
        LPFEEL_ENVELOPE pEnvelope = (LPFEEL_ENVELOPE) FEEL_EFFECT_DONT_CHANGE_PTR,
        DWORD dwSamplePeriod = FEEL_EFFECT_DONT_CHANGE,
        DWORD dwGain = FEEL_EFFECT_DONT_CHANGE,
        DWORD dwTriggerButton = FEEL_EFFECT_DONT_CHANGE,
        DWORD dwTriggerRepeatInterval = FEEL_EFFECT_DONT_CHANGE
        );

    BOOL
    ChangeDirection( 
        LONG lDirectionX,
        LONG lDirectionY 
        );

    BOOL
    ChangeDirection( 
        LONG lAngle
        );


    BOOL
    SetEnvelope(
        DWORD dwAttackLevel,
        DWORD dwAttackTime,
        DWORD dwFadeLevel,
        DWORD dwFadeTime
        );

    BOOL
    SetEnvelope(
        LPFEEL_ENVELOPE pEnvelope
        );


    //
    // OPERATIONS
    //

    public:

    virtual BOOL
    Initialize( 
        CFeelDevice* pDevice, 
        const FEEL_EFFECT &effect
        );

	virtual BOOL 
	InitializeFromProject(
		CFeelProject &project,
		LPCSTR lpszEffectName,
		CFeelDevice* pDevice = NULL
	);

	virtual BOOL 
    Start(
        DWORD dwIterations = 1,
        DWORD dwFlags = 0
        );
    
    virtual BOOL 
    Stop();
   

//
// ------ PRIVATE INTERFACE ------ 
//

	//
    // HELPERS
    //

    protected:

    BOOL 
    initialize( 
        CFeelDevice* pDevice
        );

	HRESULT
	set_parameters_on_device(
		DWORD dwFlags
		);

	void 
	feel_to_DI_GUID( 
		GUID &guid
		);

    void
    reset();

    void
    reset_effect_struct();


    //
    // INTERNAL DATA
    //

    protected:

    FEEL_EFFECT m_Effect;
    DWORD m_dwaAxes[2];
    LONG m_laDirections[2];

    GUID m_guidEffect;
    BOOL m_bIsPlaying;
	DWORD m_dwDeviceType;
    LPIFEEL_DEVICE m_piFeelitDevice; // Might also be holding LPDIRECTINPUTDEVICE2
    LPIFEEL_EFFECT m_piFeelitEffect;
	DWORD m_cAxes; // Number of axes
};



#endif
