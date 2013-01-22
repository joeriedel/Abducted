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

  FILE:		feeldxdevice.h

  PURPOSE:	Abstraction of DirectX Force Feedback device

  STARTED:	10/10/97

  NOTES/REVISIONS:
     3/2/99 jrm (Jeff Mallett): Force-->Feel renaming
	 3/15/99 jrm: __declspec(dllimport/dllexport) the whole class

**********************************************************************/

#ifndef INCLUDED_FEELDXDEVICE_H
#define INCLUDED_FEELDXDEVICE_H

#ifndef _FFCDLL_
#define DLLFFC __declspec(dllimport)
#else
#define DLLFFC __declspec(dllexport)
#endif

#ifndef INCLUDED_FEELDEVICE_H
#include <feeldevice.h>
#endif


//================================================================
// CFeelDXDevice
//================================================================

//
// ------ PUBLIC INTERFACE ------ 
//

class DLLFFC CFeelDXDevice : public CFeelDevice
{

    //
    // CONSTRUCTOR/DESCTRUCTOR
    //

    public:
    
    // Constructor
	CFeelDXDevice();

	// Destructor
    virtual
	~CFeelDXDevice();


    //
    // ATTRIBUTES
    //

    public:

	virtual LPIFEEL_API
    GetAPI() 
	{ return (LPIFEEL_API) m_piApi; } // actually LPDIRECTINPUT

	virtual LPIFEEL_DEVICE
    GetDevice() 
	{ return (LPIFEEL_DEVICE) m_piDevice; } // actually LPDIRECTINPUTDEVICE2


    //
    // OPERATIONS
    //

	public:

	BOOL
	Initialize(
		HINSTANCE hinstApp,
		HWND hwndApp,
		LPDIRECTINPUT pDI = NULL,
		LPDIRECTINPUTDEVICE2 piDevice = NULL
		);


//
// ------ PRIVATE INTERFACE ------ 
//

	//
	// HELPERS
	//

	protected:

	virtual void
	reset();


    //
    // INTERNAL DATA
    //

    protected:

	BOOL m_bpDIPreExist;
	BOOL m_bpDIDevicePreExist;

	LPDIRECTINPUT m_piApi;
	LPDIRECTINPUTDEVICE2 m_piDevice;
};

#endif
