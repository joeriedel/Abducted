/**********************************

	MacOS ONLY!!!!
	
	This code existed in OpenTptInetPPC.o
	Now it's here so that I can profile and debug it.
	Also the MRC compiler is much better than the .o file
	
**********************************/

#include "McMac.h"

#if defined(__POWERPC__) && defined(__MAC__) && !TARGET_API_MAC_CARBON

#include <Files.h>
#include <OpenTptInternet.h>

/* These are the prototypes to the code in ROM */

#ifdef __cplusplus
extern "C" {
#endif
extern pascal OSStatus	OTAsyncOpenInternetServicesPriv(OTConfigurationRef cfig,
	OTOpenFlags oflag, OTNotifyProcPtr proc, void* contextPtr,Word8 *extraPtr);
extern pascal InetSvcRef OTOpenInternetServicesPriv(OTConfigurationRef cfig,
	OTOpenFlags oflag, OSStatus* err,Word8 *extraPtr);
extern Word8 __gOTClientRecord[];		/* Pointer to an application structure */
#ifdef __cplusplus
}
#endif

/**********************************

	Call OTAsyncOpenInternetServices
	
**********************************/

pascal OSStatus	OTAsyncOpenInternetServices(OTConfigurationRef cfig, OTOpenFlags oflag, 
	OTNotifyProcPtr proc, void* contextPtr)
{
	return OTAsyncOpenInternetServicesPriv(cfig,oflag,proc,contextPtr,__gOTClientRecord);
}

/**********************************

	Call OTOpenInternetServices
	
**********************************/

pascal InetSvcRef OTOpenInternetServices(OTConfigurationRef cfig, OTOpenFlags oflag, OSStatus* err)
{
	return OTOpenInternetServicesPriv(cfig,oflag,err,__gOTClientRecord);
}

#endif
