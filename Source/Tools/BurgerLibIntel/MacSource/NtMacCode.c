/**********************************

	Burgerlib Network code
	Mac OS version

**********************************/

#include "NtNet.h"

#if defined(__MAC__)
#include <BREndian.hpp>
#include "ClStdLib.h"
#include "MmMemory.h"
#include "InInput.h"
#include "TkTick.h"
#include "PfPrefs.h"
#include "StString.h"
#include <Files.h>
#include <Gestalt.h>
#include <OpenTransport.h>
#include <OpenTptInternet.h>
#include <OpenTptAppletalk.h>

#ifdef __cplusplus
extern "C" {
static void NetKbhitProc(void *Temp);
}
#endif

#if TARGET_API_MAC_CARBON
OTClientContextPtr MacOTContext;
#endif

static OTNotifyUPP MyEventHandler;

/* Packet protocols */

static char *MacOSNetTypeNamesPacket[NET_PROVIDER_COUNT] = {"ipx",kUDPName,kDDPName};

/* Stream protocols */

static char *MacOSNetTypeNamesStream[NET_PROVIDER_COUNT] = {"spx",kTCPName,kADSPName};

/* Listen protocols */

static char *MacOSNetTypeNamesStreamListen[NET_PROVIDER_COUNT] = {"spx","tilisten, tcp",kADSPName};

/* Gestalt masks */

static Word TypeMasks[NET_PROVIDER_COUNT] = {
	0 /*gestaltOpenTptIPXSPXPresentMask */ ,		/* IPX is not supported in Open Transport */
	gestaltOpenTptTCPPresentMask,
	0 /*gestaltOpenTptAppleTalkPresentMask */};		/* Not done yet, maybe later! */

/* These MUST be in static locked memory, or BAD things happen! */

/**********************************

	Open Transport ASync event handler

**********************************/

static pascal void EventHandler(void*NetAddrPtr, OTEventCode code, OTResult result, void*cookie)
{
	switch (code) {
	case T_DATA:
		((NetHandle_t *)NetAddrPtr)->DataFlag = TRUE;
		break;
	case T_ACCEPTCOMPLETE:	
	case T_UNBINDCOMPLETE:
	case T_BINDCOMPLETE:	
	case T_LISTEN:
		((NetHandle_t *)NetAddrPtr)->AckFlag = TRUE;
		break;
	case T_DISCONNECT:
		((NetHandle_t *)NetAddrPtr)->Mode = SOCKETMODE_UNUSED;
		break;
		
	case T_UDERR:
		OTRcvUDErr(((NetHandle_t *)NetAddrPtr)->SocketRef,0);		/* Clear the error */
		
//	case T_CONNECT:
//	case T_EXDATA:
//	case T_ERROR:
//	case T_ORDREL:
//	case T_GODATA:
//	case T_GOEXDATA:	
//	case T_REQUEST:	
//	case T_REPLY:	
//	case T_PASSCON:	
//	case T_RESET:
//	case T_REPLYCOMPLETE:	
//	case T_DISCONNECTCOMPLETE:	
//	case T_OPTMGMTCOMPLETE:	
//	case T_OPENCOMPLETE:	
//	case T_GETPROTADDRCOMPLETE:	
//	case T_RESOLVEADDRCOMPLETE:	
//	case T_GETINFOCOMPLETE:	
//	case T_SYNCCOMPLETE:
//	case T_MEMORYRELEASED:
//	case T_REGNAMECOMPLETE:
//	case T_DELNAMECOMPLETE:
//	case T_LKUPNAMECOMPLETE:
//	case T_LKUPNAMERESULT:
//	default:;
	}
}

/**********************************

	This is used to create a new connection from a listening
	socket. When the listening socket gets a connection request,
	accept it...

**********************************/

static NetHandle_t * BURGERCALL NetHandleNewConnection(NetHandle_t *Origin,TCall *FooCall)
{
	OSStatus err;
	OTConfigurationRef config;
	EndpointRef Ref;
	NetHandle_t *NewRef;

	NewRef = (NetHandle_t*)AllocAPointerClear(sizeof(NetHandle_t));
	if (NewRef) {
		config = OTCreateConfiguration(MacOSNetTypeNamesStream[Origin->LocalAddr.Provider]);
		if (config) {
#if TARGET_API_MAC_CARBON
			Ref = OTOpenEndpointInContext(config,0,0,&err,MacOTContext);	/* Make the endpoint */
#else
			Ref = OTOpenEndpoint(config,0,0,&err);	/* Make the endpoint */
#endif
			if (!err) {					/* No error? */
				TBind BindOut;			/* Destination binding */
				InetAddress Out;		/* Size of the maximum structure */
										
				BindOut.addr.maxlen = sizeof(Out);	/* Prepare output */
				BindOut.addr.len = sizeof(Out);
				BindOut.addr.buf = (Word8 *)&Out;
				BindOut.qlen = 0;
						
				if (!OTBind(Ref,0,&BindOut)) {		/* Default binding */
					if (OTInstallNotifier(Ref,MyEventHandler,NewRef)==kOTNoError) {
						OTSetAsynchronous(Ref);
						err = OTAccept(Origin->SocketRef,Ref,FooCall);
						if (err==kOTNoError) {		/* I connected? */
							NewRef->SocketRef = Ref;
							NewRef->Mode=SOCKETMODE_CONNECTED;		/* Connected to a remote server */
							FastMemCpy(&NewRef->LocalAddr,&Origin->LocalAddr,sizeof(NetAddr_t));
							MacOTToNetAddress(&NewRef->RemoteAddr,(OTAddress *)FooCall->addr.buf);
							NetAddHandleToList(NewRef);										// Insert socket in our linked list
							return NewRef;	/* Save the reference */
						}
					}
				}
				OTSetSynchronous(Ref);
				OTUnbind(Ref);	/* Remove the binding */
				OTRemoveNotifier(Ref);
			}
			OTCloseProvider(Ref);	/* Dispose of the reference */
		}
		DeallocAPointer(NewRef);
	}
	return 0;	/* Too bad... */
}

/**********************************

	This is the polling foreground task
	for getting network connect requests.
	The main app, can't process these from an interrupt
	so I use this to give it to the app at main thread time

**********************************/

static void NetKbhitProc(void *Temp)
{
	NetHandle_t *WorkHandle;
	WorkHandle = NetRootSocket.NextPtr;		/* Start here... */
	if (WorkHandle) {			/* Anything here? */
		OSErr err;

		do {
			NetHandle_t *Next;

			Next = WorkHandle->NextPtr;		/* Save it */
			if (WorkHandle->Mode == SOCKETMODE_LISTEN) {
				WorkHandle->CallInfo.addr.buf = WorkHandle->PacketBuffer;
				WorkHandle->CallInfo.addr.len = sizeof(WorkHandle->PacketBuffer)/2;
				WorkHandle->CallInfo.addr.maxlen = sizeof(WorkHandle->PacketBuffer)/2;
				WorkHandle->CallInfo.opt.buf = 0;
				WorkHandle->CallInfo.opt.len = 0;
				WorkHandle->CallInfo.opt.maxlen = 0;
				WorkHandle->CallInfo.udata.buf = WorkHandle->PacketBuffer+(sizeof(WorkHandle->PacketBuffer)/2);
				WorkHandle->CallInfo.udata.len = sizeof(WorkHandle->PacketBuffer)/2;
				WorkHandle->CallInfo.udata.maxlen = sizeof(WorkHandle->PacketBuffer)/2;
				
				err = OTListen(WorkHandle->SocketRef,&WorkHandle->CallInfo);
				if (!err) {
					NetHandle_t *FooHand;
					FooHand = NetHandleNewConnection(WorkHandle,&WorkHandle->CallInfo);
					if (FooHand) {
						if (WorkHandle->ProcPtr(FooHand)) {
							NetHandleDelete(FooHand);		/* Disconnect */
						}
					}
					break;
				} else if (err==kOTLookErr) {
					OTResult lookErr;
					lookErr = OTLook(WorkHandle->SocketRef);
					if (lookErr == T_DISCONNECT) {
						OTRcvDisconnect(WorkHandle->SocketRef,0);
					}
				}
			}
			WorkHandle = Next;
		} while (WorkHandle);			/* Any more left? */
	}
}

/**********************************

	Start up the networking code
	Return FALSE if ok, or TRUE if no networking
	is allowed

**********************************/

Word BURGERCALL NetInit(void)
{
	long OTFlags;
	Word i;

	if (!NetInited) {		/* Already started? */
		if (Gestalt(gestaltOpenTpt,&OTFlags) ||	/* Uh, open transport SHOULD return this! */
			!(OTFlags&gestaltOpenTptPresentMask) ||
#if TARGET_API_MAC_CARBON
			InitOpenTransportInContext(kInitOTForApplicationMask,&MacOTContext)
#else
			InitOpenTransport()
#endif			
			) {
			return TRUE;		/* Not present or bad init */
		}
		NetInited = TRUE;		/* Ok, Open transport is present */
		i = 0;
		do {
			if (OTFlags&TypeMasks[i]) {	/* Is Appletalk present? */
				NetProviderPresent[i] = TRUE;
			}
		} while (++i<NET_PROVIDER_COUNT);	/* All done? */
		KeyboardAddRoutine(NetKbhitProc,0);	/* Monitor me */
		MyEventHandler = NewOTNotifyUPP(EventHandler);
	}
	return FALSE;			/* I am OK! */
}

/**********************************

	Shut down the network code

**********************************/

void BURGERCALL NetShutdown(void)
{
	NetHandle_t *WorkHandle;	/* Temp */
	Word i;

	KeyboardRemoveRoutine(NetKbhitProc,0);		/* Remove the foreground routine */

	i = 0;
	do {
		DefaultSockets[i] = 0;		/* Kill off the default sockets */
	} while (++i<NET_PROVIDER_COUNT);
	
	/* This code is to dispose of all the open sockets so we don't */
	/* crash because a thread was left laying around */
		
	WorkHandle = NetRootSocket.NextPtr;		/* Start here... */
	if (WorkHandle) {			/* Anything here? */
		do {
			NetHandle_t *Next;
			Next = WorkHandle->NextPtr;		/* Save it */
			NetHandleDelete(WorkHandle);	/* Remove from the list */
			WorkHandle = Next;
		} while (WorkHandle);			/* Any more left? */
		NetRootSocket.NextPtr = 0;		/* Make sure the list is dead */
	}

	/* With all the sockets gone, I can dispose */
	/* of the proc pointer */
			
	if (MyEventHandler) {
		DisposeOTNotifyUPP(MyEventHandler);
		MyEventHandler = 0;
	}

	/* Now we shut down system specific stuff */
	
	if (NetInited) {			/* Was it initialized */
#if TARGET_API_MAC_CARBON
		CloseOpenTransportInContext(MacOTContext);
		MacOTContext = 0;
#else
		CloseOpenTransport();	/* I have to check since it may not be linked in! */
#endif
		NetInited = FALSE;		/* Bye */
	}
}

/**********************************

	Open up a NetHandle_t structure for being able
	to send (And receive) packets from a specific address.
	If you send a protocol ID instead, use the default address

**********************************/

NetHandle_t * BURGERCALL NetHandleNewListenPacket(NetAddr_t *Input)
{
	OSStatus err;
	OTConfigurationRef config;
	EndpointRef Ref;
	NetAddr_t TempAddr;
		
	NetHandle_t *NewRef;
	NewRef = (NetHandle_t*)AllocAPointerClear(sizeof(NetHandle_t));
	if (NewRef) {
		if ((Word)Input<NET_PROVIDER_COUNT) {			/* Default address */
			FastMemSet(&TempAddr,0,sizeof(TempAddr));	/* Blank it out */
			TempAddr.Provider = (Word)Input;
			Input = &TempAddr;			/* Use the default address */
		}

		config = OTCreateConfiguration(MacOSNetTypeNamesPacket[Input->Provider]);
		if (config) {
#if TARGET_API_MAC_CARBON
			Ref = OTOpenEndpointInContext(config,0,0,&err,MacOTContext);	/* Make the endpoint */
#else
			Ref = OTOpenEndpoint(config,0,0,&err);	/* Make the endpoint */
#endif
			if (Ref && err==kOTNoError) {					/* No error? */

				TBind BindIn,BindOut;
				InetAddress In,Out;		/* Size of the maximum structure */
			
				if (OTInstallNotifier(Ref,MyEventHandler,NewRef)==kOTNoError) {
					OTSetAsynchronous(Ref);

					MacNetToOTAddress((OTAddress *)&In,Input);	/* Convert to native address */
					
					BindIn.addr.maxlen = sizeof(In);	/* Prepare input */
					BindIn.addr.len = sizeof(In);
					BindIn.addr.buf = (Word8 *)&In;
					BindIn.qlen = 0;
					
					BindOut.addr.maxlen = sizeof(Out);	/* Prepare output */
					BindOut.addr.len = sizeof(Out);
					BindOut.addr.buf = (Word8 *)&Out;
					BindOut.qlen = 0;
					
					if (!OTBind(Ref,&BindIn,&BindOut)) {		/* Default binding */
						while (!NewRef->AckFlag) {
							KeyboardKbhit();		/* Kill some time */
						}
						NewRef->AckFlag = FALSE;
						NewRef->SocketRef = Ref;
						NewRef->Mode=SOCKETMODE_LISTENPACKET;		/* Packet listen mode */
						if (Input->Provider==NET_PROVIDER_TCP) {
							InetInterfaceInfo myInfo;
							if (!OTInetGetInterfaceInfo(&myInfo,0)) {
								Out.fHost = myInfo.fAddress;
							}
						}
						MacOTToNetAddress(&NewRef->LocalAddr,(OTAddress *)&Out);
						NetAddHandleToList(NewRef);					/* Add to the list */
						return NewRef;	/* Save the reference */
					}
					OTSetSynchronous(Ref);
					OTUnbind(Ref);	/* Remove the binding */
					OTRemoveNotifier(Ref);
				}
			}
			OTCloseProvider(Ref);	/* Dispose of the reference */
		}
		DeallocAPointer(NewRef);
	}
	return 0;	/* Too bad... */
}

/**********************************

	Open up a NetHandle_t structure for being able
	to send (And receive) packets from a specific address.
	If you send a protocol ID instead, use the default address

**********************************/

NetHandle_t * BURGERCALL NetHandleNewListenStream(NetAddr_t *Input,NetListenProc Proc)
{
	OSStatus err;
	OTConfigurationRef config;
	EndpointRef Ref;
	NetHandle_t *NewRef;

	NewRef = (NetHandle_t*)AllocAPointerClear(sizeof(NetHandle_t));
	if (NewRef) {
		config = OTCreateConfiguration(MacOSNetTypeNamesStreamListen[Input->Provider]);
		if (config) {
#if TARGET_API_MAC_CARBON
			Ref = OTOpenEndpointInContext(config,0,0,&err,MacOTContext);	/* Make the endpoint */
#else
			Ref = OTOpenEndpoint(config,0,0,&err);	/* Make the endpoint */
#endif
			if (Ref && err==kOTNoError) {					/* No error? */
				TBind BindIn,BindOut;
				InetAddress In,Out;		/* Size of the maximum structure */
				
				if (OTInstallNotifier(Ref,MyEventHandler,NewRef)==kOTNoError) {
					OTSetAsynchronous(Ref);
					MacNetToOTAddress((OTAddress *)&In,Input);	/* Convert to native address */
							
					BindIn.addr.maxlen = sizeof(In);	/* Prepare input */
					BindIn.addr.len = sizeof(In);
					BindIn.addr.buf = (Word8 *)&In;
					BindIn.qlen = 32;
							
					BindOut.addr.maxlen = sizeof(Out);	/* Prepare output */
					BindOut.addr.len = sizeof(Out);
					BindOut.addr.buf = (Word8 *)&Out;
					BindOut.qlen = 0;
							
					if (!OTBind(Ref,&BindIn,&BindOut)) {		/* Default binding */
						NewRef->SocketRef = Ref;
						NewRef->ProcPtr = Proc;
						NewRef->Mode=SOCKETMODE_LISTEN;		/* Packet listen mode */
						if (Input->Provider==NET_PROVIDER_TCP) {
							InetInterfaceInfo myInfo;
							if (!OTInetGetInterfaceInfo(&myInfo,0)) {
								Out.fHost = myInfo.fAddress;
							}
						}
						MacOTToNetAddress(&NewRef->LocalAddr,(OTAddress *)&Out);
						NetAddHandleToList(NewRef);					/* Add to the list */
						return NewRef;	/* Save the reference */
					}
					OTSetSynchronous(Ref);
					OTUnbind(Ref);	/* Remove the binding */
					OTRemoveNotifier(Ref);
				}
			}
			OTCloseProvider(Ref);	/* Dispose of the reference */
		}
		DeallocAPointer(NewRef);
	}
	return 0;	/* Too bad... */
}

/**********************************

	Open up a NetHandle_t structure for being able
	to send (And receive) packets from a specific address.
	If you send a protocol ID instead, use the default address

**********************************/

NetHandle_t * BURGERCALL NetHandleNewConnect(NetAddr_t *Input,Word TimeOut)
{
	OSStatus err;
	OTConfigurationRef config;
	EndpointRef Ref;
	NetHandle_t *NewRef;

	NewRef = (NetHandle_t*)AllocAPointerClear(sizeof(NetHandle_t));
	if (NewRef) {
		config = OTCreateConfiguration(MacOSNetTypeNamesStream[Input->Provider]);
		if (config) {
#if TARGET_API_MAC_CARBON
			Ref = OTOpenEndpointInContext(config,0,0,&err,MacOTContext);	/* Make the endpoint */
#else
			Ref = OTOpenEndpoint(config,0,0,&err);	/* Make the endpoint */
#endif
			if (!err) {					/* No error? */
				TBind BindOut;			/* Destination binding */
				InetAddress In,Out;		/* Size of the maximum structure */
				
				MacNetToOTAddress((OTAddress *)&In,Input);	/* Convert to native address */
						
				BindOut.addr.maxlen = sizeof(Out);	/* Prepare output */
				BindOut.addr.len = sizeof(Out);
				BindOut.addr.buf = (Word8 *)&Out;
				BindOut.qlen = 0;
						
				if (!OTBind(Ref,0,&BindOut)) {		/* Default binding */

					if (OTInstallNotifier(Ref,MyEventHandler,NewRef)==kOTNoError) {
						TCall sndCall;

						OTSetAsynchronous(Ref);
						FastMemSet(&sndCall,0,sizeof(sndCall));
						sndCall.addr.buf = (Word8 *)&In;
						sndCall.addr.len = sizeof(In);
						err = OTConnect(Ref,&sndCall,0);
						if (err==kOTNoDataErr) {		/* Not time yet? */
							Word32 Mark;
							TimeOut = TimeOut*TICKSPERSEC;		/* Convert to ticks */
							Mark = ReadTick();
							do {
								err = OTRcvConnect(Ref,0);	/* Connect yet? */
								if (err == kOTLookErr) {
									OTResult lookError = OTLook(Ref);
									if (lookError == T_DISCONNECT) {
										/* don't wait if connect failed */
										TimeOut = 0;
									}
								}
							} while (err && TimeOut && (ReadTick()-Mark)<TimeOut);
						}
					
						if (err==kOTNoError) {		/* I connected? */
							NewRef->SocketRef = Ref;
							NewRef->Mode=SOCKETMODE_CONNECTED;		/* Connected to a remote server */
							FastMemCpy(&NewRef->RemoteAddr,Input,sizeof(NetAddr_t));
							MacOTToNetAddress(&NewRef->LocalAddr,(OTAddress *)&Out);
							NetAddHandleToList(NewRef);										// Insert socket in our linked list
							return NewRef;	/* Save the reference */
						}
					}
				}
				OTSetSynchronous(Ref);
				OTUnbind(Ref);	/* Remove the binding */
				OTRemoveNotifier(Ref);
			}
			OTCloseProvider(Ref);	/* Dispose of the reference */
		}
		DeallocAPointer(NewRef);
	}
	return 0;	/* Too bad... */
}

/**********************************

	Dispose of a network handle.
	I will close the connection and discard all associated memory
	
**********************************/

void BURGERCALL NetHandleDelete(NetHandle_t *Input)
{
	NetHandle_t *WorkHandle;		/* Handle for traverinsing the linked list */
	NetHandle_t *Next;
	Word i;
	
	if (Input) {		/* Is the handle valid? */

		/* This failsafe was added to prevent idiots from disposing a NetHandle_t */
		/* that was obtained from NetGetPacketSendHandle(). */
		/* A message will be sent in the debug build */
		
		i = 0;
		do {
			if (Input==DefaultSockets[i]) {
				DebugString("Error, you deleted a NetHandle_t that was obtained from NetGetPacketSendHandle()\n");
				return;
			}
		} while (++i<NET_PROVIDER_COUNT);

		if (Input->Mode == SOCKETMODE_CONNECTED) {
			OTSndOrderlyDisconnect(Input->SocketRef);
		}
		OTSetSynchronous(Input->SocketRef);
		OTUnbind(Input->SocketRef);		/* Discard the bindings */
		OTRemoveNotifier(Input->SocketRef);	/* Discard the callback */
		OTCloseProvider(Input->SocketRef);	/* Shut down the socket */
				
		/* Now I unlink myself from the socket linked list */

		WorkHandle=&NetRootSocket;		/* I need the root pointer so I can unlink */
		do {
			Next=WorkHandle->NextPtr;	/* Get the next pointer */
			if (Next==Input) {			/* Match? */
				WorkHandle->NextPtr = Next->NextPtr;	/* Unlink from the list */
				break;			/* Get out of the loop */
			}
			WorkHandle=Next;	/* Follow the list */
		} while (WorkHandle);	/* More? */
		DeallocAPointer(Input);					/* Kill the socket handle */	
	}
}

/**********************************

	Send a packet of data to a specific address.
	If the handle is a NULL pointer, then
	send it using the default packet address.

**********************************/

long BURGERCALL NetHandleRead(NetHandle_t *Input,void *Buffer,long Length)
{
	OSStatus err;		/* Temp error */
	unsigned long junkFlags;
	
	if (Input->Mode!=SOCKETMODE_CONNECTED) {	/* Must have a connection! */
		return -1;
	}
	
	if (Length) {		/* Any data being read? */
		err = OTRcv(Input->SocketRef,Buffer,Length,&junkFlags);
		if (err!=kOTNoDataErr) {		/* Error? */
			if (err<0) {
				if (err==kOTLookErr) {	/* Should I look? */
					err = OTLook(Input->SocketRef);
					switch (err) {
					case T_DISCONNECT:
						OTRcvDisconnect(Input->SocketRef,0);
						Input->Mode = SOCKETMODE_UNUSED;
						break;
					case T_ORDREL:
						if (OTRcvOrderlyDisconnect(Input->SocketRef)==0) {
							OTSndOrderlyDisconnect(Input->SocketRef);
						}
						Input->Mode = SOCKETMODE_UNUSED;
					}
				}
				return -1;
			}
			return err;		/* Bytes read */
		}
	}
	return 0;		/* Nothing read */
}

/**********************************

	Send a packet of data to a specific address.
	If the handle is a NULL pointer, then
	send it using the default packet address.

**********************************/

long BURGERCALL NetHandleWrite(NetHandle_t *Input,const void *Buffer,long Length,Word BlockFlag)
{
	OSStatus err;		/* Temp error */
	long Sent;
	
	if (Input->Mode!=SOCKETMODE_CONNECTED) {
		return -1;
	}
	Sent = 0;
	if (Length) {
		do {
			err = OTSnd(Input->SocketRef,(void *)Buffer,Length,0);	/* Transmit */
			if (err<0) {
				if (err!=kOTFlowErr) {		/* Not filled? */
					if (err==kOTLookErr) {	/* Should I look? */
						err = OTLook(Input->SocketRef);
						switch (err) {
						case T_DISCONNECT:
							OTRcvDisconnect(Input->SocketRef,0);
							Input->Mode = SOCKETMODE_UNUSED;
							break;
						case T_ORDREL:
							if (OTRcvOrderlyDisconnect(Input->SocketRef)==0) {
								OTSndOrderlyDisconnect(Input->SocketRef);
							}
							Input->Mode = SOCKETMODE_UNUSED;
						}
					}
					return -1;		/* Darn */
				}
				err = 0;		/* Try again later if filled */
			}
			Sent += err;
			if (!BlockFlag) {
				break;
			}
			Length -= err;
			Buffer = (Word8 *)Buffer+err;
		} while (Length);
	}
	return Sent;		/* Success! */
}

/**********************************

	Send a packet of data to a specific address.
	If the handle is a NULL pointer, then
	send it using the default packet address.

**********************************/

Word BURGERCALL NetHandleSendPacket(NetHandle_t *Input,NetAddr_t *DestAddr,const void *Buffer,Word Length)
{
	TUnitData d;		/* Data for unit */
	InetAddress Dest;	/* Destination address */
	OSStatus err;		/* Temp error */
	
	if (Length && Buffer) {		/* Any data to send? */
		if (!Input) {		/* Use default? */
			Input = NetGetPacketSendHandle((NetProvider_e)DestAddr->Provider);	/* Get the default */
			if (!Input) {
				return TRUE;	/* Error! */
			}
		}	
		MacNetToOTAddress((OTAddress *)&Dest,DestAddr);	/* Convert to native address */
	
		d.addr.len = sizeof(Dest);	/* Data for dest address */
		d.addr.maxlen = sizeof(Dest);
		d.addr.buf = (Word8 *)&Dest;
	
		d.opt.len = 0;		/* Nothing for transmit */
		d.opt.maxlen = 0;
		d.opt.buf = 0;
	
		d.udata.len = Length;		/* Data to send */
		d.udata.maxlen = Length;
		d.udata.buf = (Word8 *)Buffer;	/* Pointer to buffer */
	
		err = OTSndUData(Input->SocketRef,&d);	/* Transmit */
		if (err != kOTNoError) {
			if (err == kOTLookErr) {		/* Should I call OTLook? */
				if (OTLook(Input->SocketRef)==T_UDERR) {	/* Underrun? */
					OTRcvUDErr(Input->SocketRef,0);
				}
			}
			return TRUE;		/* Darn */
		}
	}
	return FALSE;		/* Success! */
}

/**********************************

	Receive a connectionless packet from
	a specific socket
	
**********************************/

NetPacket_t * BURGERCALL NetHandleGetPacket(NetHandle_t *Input)
{
	TUnitData d;
	InetAddress inAddr;
	OSStatus err;
	OTFlags flags;
	
	d.addr.len = sizeof(inAddr);		/* Address to receive from */
	d.addr.maxlen = sizeof(inAddr);
	d.addr.buf = (Word8 *)&inAddr;
	
	d.opt.len = 0;				/* Nothing here */
	d.opt.maxlen = 0;
	d.opt.buf = 0;
	
	d.udata.len = sizeof(Input->PacketBuffer);		/* Buffer to receive data */
	d.udata.maxlen = sizeof(Input->PacketBuffer);
	d.udata.buf = Input->PacketBuffer;
	
	err = OTRcvUData(Input->SocketRef,&d,&flags);		/* Get the data if any */
	if (err==kOTNoError) {		/* Nothing? */
		if (d.udata.len) {		/* Never return zero length data */
			MacOTToNetAddress(&Input->LastPacket.Origin,(OTAddress *)&inAddr);
			Input->LastPacket.Length = d.udata.len;
			Input->LastPacket.Data = Input->PacketBuffer;
			return &Input->LastPacket;		/* I got it! */
		}
	} else if (err==kOTLookErr) {		/* Error? */
		if (OTLook(Input->SocketRef)==T_UDERR) {
			OTRcvUDErr(Input->SocketRef,0);
		}
	}
	return 0;		/* No data right now */
}

/**********************************

	Given a DNS entry, return the IP address.
	206.55.132.145
	206.55.132.180:80
	www.logicware.com
	logicware.com:80

**********************************/

typedef struct TMyOTInetSvcInfo {		/* Open Transport Internet services provider info */
	InetSvcRef ref;			/* provider reference */
	Bool complete;		/* true when asynch operation has completed */
	OTResult result;		/* result code */
	void *cookie;			/* cookie */
} TMyOTInetSvcInfo;

/**********************************

	This is called asynchronously by Open Transport

**********************************/

static pascal void MyOTInetSvcNotifyProc(void *svcInfo, OTEventCode code,
	OTResult result, void *cookie)
{
	switch (code) {
	case T_OPENCOMPLETE:
	case T_DNRSTRINGTOADDRCOMPLETE:
	case T_DNRADDRTONAMECOMPLETE:			/* Did the operation complete? */
		((TMyOTInetSvcInfo*)svcInfo)->complete = TRUE;
		((TMyOTInetSvcInfo*)svcInfo)->result = result; 
		((TMyOTInetSvcInfo*)svcInfo)->cookie = cookie;
		break;
	}
}

/**********************************

	Wait for the event to occur or the timeout

**********************************/

static OSErr MyOTInetSvcWait(TMyOTInetSvcInfo *svcInfo,Word Time)
{
	Word32 Mark;
	if (!svcInfo->complete) {		/* Not done yet? */
		Mark = ReadTick();			/* Get timer */
		do {
			KeyboardKbhit();		/* Give the system some time */
			if ((ReadTick()-Mark)>=Time) {
				return -666;			/* Timeout! */
			}
		} while (!svcInfo->complete);		/* Not yet! */
	}
	return 0;			/* No error */
}

/**********************************

	Here is where I lookup a DNS entry.
	I have to do it async since I want a timeout
	
**********************************/

Word BURGERCALL NetStringToTCPAddress(NetAddr_t *Output,const char *HostName)
{
	OSErr err;		/* Mac error code */
	TMyOTInetSvcInfo svcInfo;	/* Internet services */
	InetHostInfo hInfoOT;		/* Internet host address returned */
	char *ColonPtr;				/* Work string */
	char *TempPtr;
	OTNotifyUPP NotifyProc;
	
	Output->Provider = NET_PROVIDER_TCP;		/* It's a TCP/IP address */
	Output->U.TCP.IP = 0;				/* Init the defaults */
	Output->U.TCP.Port = 0;
	
	if (NetProviderPresent[NET_PROVIDER_TCP] && HostName && HostName[0]) {		/* Bad input name? */
		unsigned long TempVal;
		/* Find a colon for the port */
		
		TempPtr = StrCopy(HostName);			/* Make a copy of the string */

		if (TempPtr) {						/* Success? */
			ColonPtr = strchr(TempPtr,':');		/* Scan for the offending colon */
			if (ColonPtr) {
				ColonPtr[0] = 0;		/* Force a null string */
				Output->U.TCP.Port = AsciiToLongWord(ColonPtr+1);		/* Get the port */
			}

			/* First, see if I can resolve 206.55.132.145 */
			
			if (OTInetStringToHost(TempPtr,&TempVal) == noErr) {
				Output->U.TCP.IP = TempVal;
				DeallocAPointer(TempPtr);
				return FALSE;		/* It's OK right now! */
			}
			
			/* Hmm, it seems to be a real domain name, I must use DNS */
			
			NotifyProc = NewOTNotifyUPP(MyOTInetSvcNotifyProc);
			svcInfo.complete = FALSE;		/* Init the wait proc */
			if (
#if TARGET_API_MAC_CARBON
			!OTAsyncOpenInternetServicesInContext(kDefaultInternetServicesPath, 0,NotifyProc, &svcInfo,MacOTContext)
#else
			!OTAsyncOpenInternetServices(kDefaultInternetServicesPath, 0,NotifyProc, &svcInfo)
#endif
			) {	/* Start services */
				if (!MyOTInetSvcWait(&svcInfo,10*TICKSPERSEC)) {		/* Didn't time out? */
					svcInfo.ref = static_cast<TInternetServices *>(svcInfo.cookie);			/* Get the reference */
					svcInfo.complete = FALSE;				/* Init the flag */
					err = OTInetStringToAddress(svcInfo.ref,TempPtr,&hInfoOT);		/* Resolve DNS */
					if (err == noErr) {
						err = MyOTInetSvcWait(&svcInfo,10*TICKSPERSEC);		/* Timeout? */
					}
					OTCloseProvider(svcInfo.ref);		/* Release services */
					if (err == noErr) {					/* Error? */
						Output->U.TCP.IP = hInfoOT.addrs[0];		/* Get the address */
						DeallocAPointer(TempPtr);
						DisposeOTNotifyUPP(NotifyProc);
						return FALSE;			/* Ok */
					}
				}
			}
			DisposeOTNotifyUPP(NotifyProc);
			DeallocAPointer(TempPtr);
		}
	}
	return TRUE;		/* Error! */
}

/**********************************

	This is a MacOS convinence routine to convert a
	Burgerlib network address into an Open Transport address

**********************************/

void BURGERCALL MacNetToOTAddress(OTAddress *Output,NetAddr_t *Input)
{
	switch (Input->Provider) {
//	case NET_PROVIDER_IPX:
//		Output->fAddressType = AF_IPX;
//		((OTAddress *)Output)->fAddress[0] = Burger::LoadBig((Word16)Input->IPX.Socket);
//		FastMemCpy(((OTAddress *)Output)->fAddress[2],Input->IPX.Net,10);
//		break;
	case NET_PROVIDER_TCP:
		Output->fAddressType = AF_INET;
		((InetAddress *)Output)->fHost = Burger::LoadBig(Input->U.TCP.IP);
		((InetAddress *)Output)->fPort = Burger::LoadBig(static_cast<Word16>(Input->U.TCP.Port));
		break;
	case NET_PROVIDER_APPLETALK:
		Output->fAddressType = AF_ATALK_DDP;
		((DDPAddress *)Output)->fNetwork = Burger::LoadBig(static_cast<Word16>(Input->U.APPLETALK.Network));
		((DDPAddress *)Output)->fNodeID = Input->U.APPLETALK.NodeID;
		((DDPAddress *)Output)->fSocket = Input->U.APPLETALK.Socket;
		((DDPAddress *)Output)->fDDPType = 0;
		break;
	default:
		FastMemSet(Output,0,sizeof(OTAddress));
	}
}

/**********************************

	This is a MacOS convinence routine to convert a
	Open Transport address into a Burgerlib network address

**********************************/

void BURGERCALL MacOTToNetAddress(NetAddr_t *Output,OTAddress *Input)
{
	switch (Input->fAddressType) {
	case AF_INET:		/* TCP/IP address */
		Output->Provider=NET_PROVIDER_TCP;
		Output->U.TCP.Port=Burger::LoadBig(((InetAddress *)Input)->fPort);
		Output->U.TCP.IP=Burger::LoadBig(((InetAddress *)Input)->fHost);
		break;
//	case AF_IPX:		/* IPX/SPX address */
//		Output->Provider=NET_PROVIDER_IPX;
//		Output->IPX.Socket=Burger::LoadBig(((SOCKADDR_IPX *)Input)->sa_socket);
//		FastMemCpy(Output->IPX.Net,((SOCKADDR_IPX *)Input)->sa_netnum,10);
//		break;
	case AF_ATALK_DDP:	/* Appletalk address */
		Output->Provider=NET_PROVIDER_APPLETALK;
		Output->U.APPLETALK.Network = Burger::LoadBig(((DDPAddress *)Input)->fNetwork);
		Output->U.APPLETALK.NodeID = ((DDPAddress *)Input)->fNodeID;
		Output->U.APPLETALK.Socket = ((DDPAddress *)Input)->fSocket;
		break;
	default:
		FastMemSet(Output,0,sizeof(NetAddr_t));
	}
}

#endif