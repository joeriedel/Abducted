/**********************************************************************
	Copyright (c) 1999 Immersion Corporation

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

  FILE:		ffcerrors.h

  PURPOSE:	Error codes returned in FFC; Error handling in FFC

  STARTED:	2/28/99 by Jeff Mallett

  NOTES/REVISIONS:
     3/2/99 jrm (Jeff Mallett): Force-->Feel renaming
	 3/6/99 jrm: Added user error handling control
	 3/15/99 jrm: __declspec(dllimport/dllexport) the whole class

**********************************************************************/

#ifndef INCLUDED_FFCERRORS_H
#define INCLUDED_FFCERRORS_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef _FFCDLL_
#define DLLFFC __declspec(dllimport)
#else
#define DLLFFC __declspec(dllexport)
#endif

#ifndef INCLUDED_WINERROR_H
#include <winerror.h>
#endif

/****************************************************************************
 *
 *      Error Codes
 *
 ****************************************************************************/

typedef enum {
    FFC_ERR_OK							=   0,

	FFC_ERR_UNKNOWN_ERROR				=   1,

	FFC_ERR_ALLOCATION_FAILED			=   2,
	FFC_ERR_INVALID_PARAMETER			=   3,
	FFC_ERR_NULL_PARAMETER				=   4,
	FFC_ERR_WRONG_FORM					=   5,

	FFC_ERR_DEVICE_IS_NULL				=   6,
	FFC_ERR_INVALID_GUID				=   7,
	FFC_ERR_EFFECT_NOT_INITIALIZED		=   8,

	//									=   9,

	FFC_ERR_CANT_CREATE_EFFECT			=  10,
	FFC_ERR_CANT_CREATE_EFFECT_FROM_IFR	=  11,
	FFC_ERR_NO_EFFECTS_FOUND			=  12,
	FFC_ERR_EFFECT_IS_COMPOUND			=  13,

	FFC_ERR_PROJECT_ALREADY_OPEN		=  14,
	FFC_ERR_PROJECT_NOT_OPEN			=  15
} FFC_ERROR_CODE;

typedef enum {
	FFC_OUTPUT_ERR_TO_DEBUG				= 0x0001,
	FFC_OUTPUT_ERR_TO_DIALOG			= 0x0002
} FFC_ERROR_HANDLING_FLAGS;


/****************************************************************************
 *
 *      Macros
 *
 ****************************************************************************/

//
// ------ PUBLIC MACROS ------ 
//
#define FFC_GET_LAST_ERROR			CFFCErrors::GetLastErrorCode()
#define FFC_SET_ERROR_HANDLING		CFFCErrors::SetErrorHandling


//
// ------ PRIVATE MACROS ------ 
//
#define FFC_SET_ERROR(err)			CFFCErrors::SetErrorCode(err)
#define FFC_CLEAR_ERROR				FFC_SET_ERROR(FFC_ERR_OK)



/****************************************************************************
 *
 *      CFFCErrors
 *
 ****************************************************************************/
// All members are static.  Don't bother instantiating an object of this class.

//
// ------ PUBLIC INTERFACE ------ 
//
        
class DLLFFC CFFCErrors
{
    //
    // ATTRIBUTES
    //

	public:

	static HRESULT
	GetLastErrorCode()
		{ return m_Err; }

	static void
	SetErrorHandling(unsigned long dwFlags)
		{ m_dwErrHandlingFlags = dwFlags; }


//
// ------ PRIVATE INTERFACE ------ 
//

	// Internally used by FFC classes
	static void
	SetErrorCode(
		HRESULT err
		);

    //
    // HELPERS
    //

	protected:

    //
    // INTERNAL DATA
    //

	private:

	static HRESULT m_Err;
	static unsigned long m_dwErrHandlingFlags;
};


#endif
