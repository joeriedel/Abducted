/**********************************

	Burgerlib net manager

**********************************/

#include "NtNet.h"
#include "PfPrefs.h"
#include "ClStdLib.h"
#include "MmMemory.h"

Word NetInited;										/* True if Network is initialized */
Bool NetProviderPresent[NET_PROVIDER_COUNT];		/* True if the provider code is present */
NetHandle_t *DefaultSockets[NET_PROVIDER_COUNT];	/* These are the default endpoints */
NetHandle_t NetRootSocket;							/* Root handle for linked list */

/**********************************

	Machine specific code. This is the generic stubs

**********************************/

#if !defined(__MAC__) && !defined(__WIN32__)

/**********************************

	Start up the networking code
	Return FALSE if ok, or TRUE if no networking
	is allowed

**********************************/

Word BURGERCALL NetInit(void)
{
	return TRUE;		/* Bad bad bad bad bad */
}

/**********************************

	Shut down the network code

**********************************/

void BURGERCALL NetShutdown(void)
{
}

/**********************************

	Open up a NetHandle_t structure for being able
	to send (And receive) packets from a specific address.
	If you send a protocol ID instead, use the default address

**********************************/

NetHandle_t * BURGERCALL NetHandleNewListenPacket(NetAddr_t * /* Server */)
{
	return 0;		/* Always bomb */
}

/**********************************

	Open up a NetHandle_t structure for being able
	to send (And receive) packets from a specific address.
	If you send a protocol ID instead, use the default address

**********************************/

NetHandle_t * BURGERCALL NetHandleNewListenStream(NetAddr_t * /*Input*/,NetListenProc /*Proc*/)
{
	return 0;		/* Always bomb */
}

/**********************************

	Open up a NetHandle_t structure for being able
	to send (And receive) packets from a specific address.
	If you send a protocol ID instead, use the default address

**********************************/

NetHandle_t * BURGERCALL NetHandleNewConnect(NetAddr_t * /* Server */,Word /* TimeOut */)
{
	return 0;		/* Always bomb */
}

/**********************************

	Dispose of a network handle.
	I will close the connection and discard all associated memory

**********************************/

void BURGERCALL NetHandleDelete(NetHandle_t *Input)
{
	DeallocAPointer(Input);
}

/**********************************

	Send a packet of data to a specific address.
	If the handle is a NULL pointer, then
	send it using the default packet address.

**********************************/

long BURGERCALL NetHandleRead(NetHandle_t * /* Input */,void * /* Buffer */,long /* Length */)
{
	return -1;		/* ERROR! */
}

/**********************************

	Send a packet of data to a specific address.
	If the handle is a NULL pointer, then
	send it using the default packet address.

**********************************/

long BURGERCALL NetHandleWrite(NetHandle_t * /*Input*/,const void * /*Buffer*/,long /*Length */,Word /*BlockFlag*/)
{
	return -1;		/* ERROR! */
}

/**********************************

	Send a packet of data to a specific address.
	If the handle is a NULL pointer, then
	send it using the default packet address.

**********************************/

Word BURGERCALL NetHandleSendPacket(NetHandle_t * /* Input*/,NetAddr_t * /*DestAddr*/,const void * /*Buffer*/,Word /*Length*/)
{
	return TRUE;		/* ERROR! */
}

/**********************************

	Receive a connectionless packet from
	a specific socket

**********************************/

NetPacket_t * BURGERCALL NetHandleGetPacket(NetHandle_t * /*Input*/)
{
	return 0;		/* Always fail! */
}

/**********************************

	Given a DNS entry, return the IP address.
	206.55.132.145
	206.55.132.180:80
	www.logicware.com
	logicware.com:80

**********************************/

Word BURGERCALL NetStringToTCPAddress(NetAddr_t *Output,const char * /*HostName*/)
{
	Output->Provider = NET_PROVIDER_TCP;		/* It's a TCP/IP address */
	Output->U.TCP.IP = 0;
	Output->U.TCP.Port = 0;
	return TRUE;		/* Error!! */
}

#endif


/**********************************

	The following are generic routines available across all platforms!

**********************************/


/**********************************

	Return TRUE if a handle is shutdown or invalid

**********************************/

Word BURGERCALL NetHandleIsItClosed(NetHandle_t *Input)
{
	if (!Input || Input->Mode==SOCKETMODE_UNUSED) {		/* Bad handle or closed? */
		return TRUE;			/* I'm dead */
	}
	return FALSE;		/* This handle is open */
}

/**********************************

	Given a network address, convert it into a
	readable string.

	If PortFlag is TRUE then append the port address.

	If TCP/IP = "206.55.132.145:80"
	If IPX "12341234:123412341234:1234"
	If APPLETALK 0000:00:00

**********************************/

void BURGERCALL NetAddressToString(char *Output,Word Size,NetAddr_t *Input,Word PortFlag)
{
	Word i;
	char TempBuf[128];
	char *WorkPtr;

	if (Size) {		/* Is there an output buffer? */
		WorkPtr = TempBuf;
		switch(Input->Provider) {
		case NET_PROVIDER_TCP:			/* 206.55.132.145:80 */
			WorkPtr = LongWordToAscii(Input->U.TCP.IP>>24,WorkPtr);
			WorkPtr[0] = '.';
			WorkPtr = LongWordToAscii((Input->U.TCP.IP>>16)&0xFF,WorkPtr+1);
			WorkPtr[0] = '.';
			WorkPtr = LongWordToAscii((Input->U.TCP.IP>>8)&0xFF,WorkPtr+1);
			WorkPtr[0] = '.';
			WorkPtr = LongWordToAscii(Input->U.TCP.IP&0xFF,WorkPtr+1);
			if (PortFlag) {
				WorkPtr[0] = ':';		/* Add the port value */
				WorkPtr = LongWordToAscii(Input->U.TCP.Port,WorkPtr+1);
			}
			break;

		case NET_PROVIDER_IPX:		/* 00000000:000000000000:0000 */

			i = 0;		/* Show the IPX net address */
			do {
				WorkPtr = LongWordToAsciiHex2(Input->U.IPX.Net[i],WorkPtr,ASCIILEADINGZEROS|2);
			} while (++i<4);
			WorkPtr[0] = ':';		/* Seperator */
			++WorkPtr;
			i = 0;
			do {
				WorkPtr = LongWordToAsciiHex2(Input->U.IPX.Node[i],WorkPtr,ASCIILEADINGZEROS|2);	// Convert node number
			} while (++i<6);
			if (PortFlag) {
				WorkPtr[0] = ':';		/* Seperator */
				++WorkPtr;
				WorkPtr = LongWordToAsciiHex2(Input->U.IPX.Socket,WorkPtr,ASCIILEADINGZEROS|4);
			}
			break;
		case NET_PROVIDER_APPLETALK:	/* 0000:00:00 */
			WorkPtr = LongWordToAsciiHex2(Input->U.APPLETALK.Network,WorkPtr,ASCIILEADINGZEROS|4);
			WorkPtr[0] = ':';
			WorkPtr = LongWordToAsciiHex2(Input->U.APPLETALK.NodeID,WorkPtr+1,ASCIILEADINGZEROS|2);
			WorkPtr[0] = ':';
			WorkPtr = LongWordToAsciiHex2(Input->U.APPLETALK.Socket,WorkPtr+1,ASCIILEADINGZEROS|2);
			break;

		default:
			FastMemCpy(WorkPtr,"Unknown network address",23+1);		/* Failsafe */
			WorkPtr = TempBuf+23;
		}
		--Size;			/* Make room for the ending zero */
		i = WorkPtr-TempBuf;		/* Length of the string */
		if (i>Size) {	/* Is the string too big? */
			i = Size;
		}
		Output[i] = 0;		/* End the buffer */
		FastMemCpy(Output,TempBuf,i);		/* Copy to the final buffer */
	}
}

/**********************************

	Compare two network addresses for equality
	Do not use this routine to test for less/greater.
	Since I support 3 network modes, I need three compare routines

	Return FALSE (like memcmp()) for equality and TRUE if not equal.

**********************************/

Word BURGERCALL NetAddressCompare(const NetAddr_t *First,const NetAddr_t *Second)
{
	if (First->Provider==Second->Provider) {		/* Same provider? */
		switch (First->Provider) {
		case NET_PROVIDER_TCP:
			if (First->U.TCP.IP == Second->U.TCP.IP &&
				First->U.TCP.Port == Second->U.TCP.Port) {
				return FALSE;		/* Match */
			}
			break;
		case NET_PROVIDER_IPX:
			if (((Word32 *)&First->U.IPX.Net[0])[0] == ((Word32 *)&Second->U.IPX.Net[0])[0] &&
				((Word32 *)&First->U.IPX.Node[0])[0] == ((Word32 *)&Second->U.IPX.Node[0])[0] &&
				((Word16 *)&First->U.IPX.Node[4])[0] == ((Word16 *)&Second->U.IPX.Node[4])[0]
				) {
				return FALSE;		/* Match */
			}
			break;
		case NET_PROVIDER_APPLETALK:
			if (First->U.APPLETALK.Network == Second->U.APPLETALK.Network &&
				First->U.APPLETALK.NodeID == Second->U.APPLETALK.NodeID &&
				First->U.APPLETALK.Socket == Second->U.APPLETALK.Socket &&
				First->U.APPLETALK.DDPType == Second->U.APPLETALK.DDPType) {
				return FALSE;		/* Match */
			}
		}
	}
	return TRUE;		/* Not a match! */
}

/**********************************

	Return TRUE if the protocol is present

**********************************/

Word BURGERCALL NetIsProviderPresent(NetProvider_e Provider)
{
	if (Provider<NET_PROVIDER_COUNT) {		/* Valid input? */
		if (NetProviderPresent[Provider]) {		/* Socket present? */
			return TRUE;		/* You can use this */
		}
	}
	return FALSE;		/* Protocol not ready */
}

/**********************************

	Return the default packet handle for a specific provider
	for packet transmission	or zero if the protocol is not supported

	Some machines will dial up the PPP connection if TCP/IP is asked for.
	Only use when you REALLY mean it!

**********************************/

NetHandle_t * BURGERCALL NetGetPacketSendHandle(NetProvider_e Provider)
{
	NetHandle_t *Temp;
	if (Provider<NET_PROVIDER_COUNT) {		/* Valid input? */
		Temp = DefaultSockets[Provider];	/* Return the socket for the default transmitter */
		if (!Temp) {						/* Not connected yet? */
			Temp = NetHandleNewListenPacket((NetAddr_t *)Provider);	/* Make the socket */
			DefaultSockets[Provider] = Temp;		/* Save it (May be zero anyway!) */
		}
		return Temp;
	}				/* The above COULD be zero if not supported! */
	return 0;		/* Bad news */
}

/**********************************

	Get the address of the destination socket when using
	a live connection

**********************************/

void BURGERCALL NetGetPeerAddress(NetAddr_t *Output,NetHandle_t *Input)
{
	if (Input->Mode==SOCKETMODE_CONNECTED) {	/* Is there a peer? */
		FastMemCpy(Output,&Input->RemoteAddr,sizeof(NetAddr_t));	/* Get the address */
		return;
	}
	FastMemSet(Output,0,sizeof(NetAddr_t));		/* Clear out the output */
}

/**********************************

	Get the address of the source (Local) socket when using
	a live connection

**********************************/

void BURGERCALL NetGetLocalAddress(NetAddr_t *Output,NetHandle_t *Input)
{
	if (Input->Mode!=SOCKETMODE_UNUSED) {
		FastMemCpy(Output,&Input->LocalAddr,sizeof(NetAddr_t));
		return;
	}
	FastMemSet(Output,0,sizeof(NetAddr_t));
}

/**********************************

	Scan the linked list of NetHandle_t's until the one
	with a specfic socket is found

**********************************/

NetHandle_t * BURGERCALL NetFindHandleFromSocket(Word32 Socket)
{
	NetHandle_t *SocketPtr;

	SocketPtr=NetRootSocket.NextPtr;		/* Get the first entry */
	if (SocketPtr) {						/* Valid? */
		do {
			if ((Word32)SocketPtr->SocketRef==Socket) {	/* Match? */
				break;						/* Return this entry */
			}
			SocketPtr = SocketPtr->NextPtr;	/* Follow the list */
		} while (SocketPtr);
	}
	return SocketPtr;					/* Return this entry */
}

/**********************************

	Find a socket handle that's in a specific mode
	this doesn't bother check what protocol it's using

**********************************/

NetHandle_t * BURGERCALL NetFindHandleByMode(SocketMode_e Mode)
{
	NetHandle_t *SocketPtr;

	SocketPtr=NetRootSocket.NextPtr;		/* Get the first entry */
	if (SocketPtr) {						/* Valid? */
		do {
			if (SocketPtr->Mode==Mode) {	/* Match? */
				break;						/* Return this entry */
			}
			SocketPtr = SocketPtr->NextPtr;	/* Follow the list */
		} while (SocketPtr);
	}
	return SocketPtr;					/* Return the found entry */
}

/**********************************

	This is an internal routine.
	I will take a newly created NetHandle and
	add it to the main internal linked list. This way
	I am aware of all network sockets that are present.
	I can clean up after you when you shut down!

**********************************/

void BURGERCALL NetAddHandleToList(NetHandle_t *Input)
{
	Input->NextPtr = NetRootSocket.NextPtr;	/* Save the new end link */
	NetRootSocket.NextPtr = Input;			/* Store myself as the "Next" */
}

