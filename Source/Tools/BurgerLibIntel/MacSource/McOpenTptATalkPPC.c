/**********************************

	MacOS ONLY!!!!
	
	This code existed in OpenTptInetPPC.o
	Now it's here so that I can profile and debug it.
	Also the MRC compiler is much better than the .o file
	
**********************************/

#include "McMac.h"

#if defined(__POWERPC__) && defined(__MAC__) && !TARGET_API_MAC_CARBON

#include <Files.h>
#include <OpenTptAppletalk.h>

/* These are the prototypes to the code in ROM */

#ifdef __cplusplus
extern "C" {
#endif
extern pascal OSStatus OTAsyncOpenAppleTalkServicesPriv(OTConfigurationRef cfig,
	OTOpenFlags flags,OTNotifyProcPtr, void* contextPtr,Word8 *extraPtr);
extern pascal ATSvcRef	OTOpenAppleTalkServicesPriv(OTConfigurationRef cfig,
	OTOpenFlags flags,OSStatus* err,Word8 *extraPtr);
extern Word8 __gOTClientRecord[];		/* Pointer to an application structure */
#ifdef __cplusplus
}
#endif

/**********************************

	Call OTAsyncOpenInternetServices
	
**********************************/

pascal OSStatus	OTAsyncOpenAppleTalkServices(OTConfigurationRef cfig,
	OTOpenFlags flags,OTNotifyProcPtr proc, void* contextPtr)
{
	return OTAsyncOpenAppleTalkServicesPriv(cfig,flags,proc,contextPtr,__gOTClientRecord);
}

/**********************************

	Call OTOpenInternetServices
	
**********************************/

pascal ATSvcRef	OTOpenAppleTalkServices(OTConfigurationRef cfig,
	OTOpenFlags flags,OSStatus* err)
{
	return OTOpenAppleTalkServicesPriv(cfig,flags,err,__gOTClientRecord);
}

#endif
