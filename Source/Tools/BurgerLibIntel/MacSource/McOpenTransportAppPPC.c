/**********************************

	MacOS ONLY!!!!
	
	This code existed in OpenTransportAppPPC.o
	Now it's here so that I can profile and debug it.
	Also the MRC compiler is much better than the .o file
	
**********************************/

#include "LWMac.h"

#if defined(__POWERPC__) && defined(__MAC__)

#include <OpenTransport.h>

/* These are the prototypes to the code in ROM */

extern OSStatus InitOpenTransportCommon(Word x);
extern void CloseOpenTransportPriv(Word8 *extraPtr);
extern pascal OSStatus OTRegisterAsClientPriv(OTClientName name, OTNotifyProcPtr proc,Word8 *extraPtr);
extern pascal OSStatus OTUnregisterAsClientPriv(Word8 *extraPtr);
extern void *OTAllocMemPriv(size_t memsize,Word8 *extraPtr);
extern pascal void* OTAllocPriv(EndpointRef ref, OTStructType structType,UInt32 fields, OSStatus* err,Word8 *extraPtr);

extern Word8 __gOTClientRecord[];		/* Pointer to an application structure */

/**********************************

	Call InitOpenTransportUtilities
	
**********************************/

pascal OSStatus InitOpenTransportUtilities(void)
{
	return InitOpenTransportCommon(1);
}

/**********************************

	Call CloseOpenTransport
	
**********************************/

pascal void CloseOpenTransport(void)
{
	CloseOpenTransportPriv(__gOTClientRecord);
}

/**********************************

	Call OTRegisterAsClient
	
**********************************/

pascal OSStatus	OTRegisterAsClient(OTClientName name, OTNotifyProcPtr proc)
{
	return OTRegisterAsClientPriv(name,proc,__gOTClientRecord);
}

/**********************************

	Call OTUnregisterAsClient
	
**********************************/

pascal OSStatus	OTUnregisterAsClient(void)
{
	return OTUnregisterAsClientPriv(__gOTClientRecord);
}

/**********************************

	Call OTAllocMem
	
**********************************/

void *OTAllocMem(size_t memsize)
{
	return OTAllocMemPriv(memsize,__gOTClientRecord);
}

/**********************************

	Call InitOpenTransport
	
**********************************/

pascal OSStatus InitOpenTransport(void)
{
	return InitOpenTransportCommon(3);
}

/**********************************

	Call OTAlloc
	
**********************************/

pascal void* OTAlloc(EndpointRef ref, OTStructType structType,
	UInt32 fields, OSStatus* err)
{
	return OTAllocPriv(ref,structType,fields,err,__gOTClientRecord);
}

extern pascal ProviderRef	OTOpenProvider(OTConfiguration*, OTOpenFlags, OSStatus*);
extern pascal MapperRef	OTOpenMapper(OTConfiguration* config, OTOpenFlags oflag,
									 OSStatus* err);	
extern pascal EndpointRef	
						OTOpenEndpoint(OTConfiguration* config, OTOpenFlags oflag,
									   TEndpointInfo* info, OSStatus* err);
extern pascal OSStatus		OTAsyncOpenProvider(OTConfiguration*, OTOpenFlags,
												OTNotifyProcPtr, void*);
extern pascal OSStatus 	OTAsyncOpenMapper(OTConfiguration* config, OTOpenFlags oflag,
										  OTNotifyProcPtr proc, void* contextPtr);			
extern pascal OSStatus	OTAsyncOpenEndpoint(OTConfiguration* config,
											OTOpenFlags oflag, TEndpointInfo* info,
											OTNotifyProcPtr proc, void* contextPtr);
extern pascal ProviderRef	OTTransferProviderOwnership(ProviderRef ref, 
														OTClient prevOwner,
														OSStatus* errPtr);
extern pascal OTClient	OTWhoAmI(void);
extern pascal OSStatus		OTCloseProvider(ProviderRef ref);

#endif