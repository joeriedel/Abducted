/**********************************

	Burgerlib network code
	Win95 version

**********************************/

#include "NtNet.h"

#if defined(__WIN32__)
#include <BREndian.hpp>
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>
#include <wsipx.h>

/* This is an ugly hack because WatcomC doesn't have this header */

#if !defined(__WATCOMC__)
#include <atalkwsh.h>
#else
typedef struct sockaddr_at {
    USHORT    sat_family;
    USHORT    sat_net;
    UCHAR     sat_node;
    UCHAR     sat_socket;
} SOCKADDR_AT, *PSOCKADDR_AT;
#endif

#include "W9Win95.h"
#include "PfPrefs.h"
#include "InInput.h"
#include "ClStdLib.h"
#include "MmMemory.h"
#include "StString.h"

/**********************************

	Start up the networking code
	Return FALSE if ok, or TRUE if no networking
	is allowed

**********************************/

static void *Win95NetSocketSemaphore;		/* Semaphore for network multi-threading */
static void *Win95NetThread;				/* Handle to the network thread */
static struct HWND__ *Win95NetWindowPtr;	/* Pointer to the window for the network thread */
static Word Win95NetAddrFamily[3]={AF_IPX,AF_INET,AF_APPLETALK};	/* Native Address families */
static long Win95NetTypes[3]={ NSPROTO_IPX, 0,0 };		/* Packet based protocols */

static void BURGERCALL NetKbhitProc(void * /* i */)
{
}

//
// Network window procedure
//
static LRESULT CALLBACK netWindowProc(HWND /* hwnd */, Word16 /* msg */, WPARAM /* wParam */, LPARAM /* lParam */)
{
#if 0
	Word16  wsaErr=WSAGETSELECTERROR(lParam); 						  // WinSock error code
	Word16  wsaEvent=WSAGETSELECTEVENT(lParam);						  // Winsock event code
	
	// Check for out-of-range messages
	if ((msg<WSM_LOW) || (msg>WSM_HIGH)) {
		return(DefWindowProc(hwnd, msg, wParam, lParam));
	}
	// Check for simple (reply when happens) messages
	if (msg>=WSM_SIMPLE) {											  // Simple message? 
		// Set message type and result so the calling function can exit
		netAMsgResult=wsaErr;											// Set message result
		netAsyncMsg=msg;												// Set processed message
		return(0);
	}
	
	// This message isn't one of those simple ones, start with locking sockets
	WaitForSingleObject(Win95NetSocketSemaphore, INFINITE);					  // Hold Mutex
	
	//
	// #******************#
	// *				  *
	// *  STREAM SOCKETS  *
	// *				  *
	// #******************#
	//
	
	if (msg==WSM_STREAM) {										  // Stream socket event?
		// Receive notification on a stream (SPX/TCP) socket
				
		//
		// READ
		//
		if (wsaEvent==FD_READ) {											// Ready for reading
			PSTRMBUF  inBuf;											  // -> Input buffer
			NetHandle_t * readSock; 										  // Socket we're reading from
			long	  recvBytes;										  // Received bytes
			
			// Find socket in linked list
			if ((readSock=NetFindHandleFromSocket((SOCKET)wParam))==NULL) breakmsg;   // Find socket and quit if not found.
			
			// Check if readSock has a buffer we can fill out a little more
			inBuf=(PSTRMBUF)readSock->inTail;						// Buffer at the end of list
			if ((inBuf==NULL) || (isFull(inBuf)))					// No buffer, or buffer full?
			{
				// Allocate new buffer (all zeroes)
				inBuf=AllocAPointerClear(sizeof(STRMBUF)+TCP_BUFSIZE);		// Get new network buffer
				
				// Insert new buffer into linked list
				if (readSock->inHead==NULL) 						// First element?
				{
					// If the linked list doesn't have a head (so, it is empty), we're both head and tail
					readSock->inHead=(GenBuf_t *)inBuf;				// We're the head of the buffer list...
					readSock->inTail=(GenBuf_t *)inBuf;				// ...as well as the tail
				}
				else
				{
					// We pad ourselves to the end of the list. The current tail element's pNext will point to us,
					// the tail pointer will be set to point to us
					readSock->inTail->NextPtr=inBuf;					// Last element points to us
					readSock->inTail=(GenBuf_t *)inBuf;				// Now we're the last element
				}
			}
			
			// Receive data (pad at the end of the buffer, up to the amount to fill up the buffer)
			recvBytes=recv((SOCKET)wParam, (char *)(inBuf->Data+inBuf->Length), TCP_BUFSIZE-inBuf->Length, 0);
			if (recvBytes!=SOCKET_ERROR)
			{
				inBuf->Length+=recvBytes;							// Add to total received
				readSock->InputBytes += recvBytes;					// Add bytes in input buffer
			}
		//
		// CLOSE
		//
		} else if (wsaEvent==FD_CLOSE) {
			NetHandle_t * closeSock;										  // -> Socket closing down
			
			if ((closeSock=NetFindHandleFromSocket((SOCKET)wParam))==NULL) {
				breakmsg;  // Find socket in linked list
			}
			closeSock->Closed=TRUE; 									  // Set closed flag
		//
		// ACCEPT
		//
		} else if (wsaEvent==FD_ACCEPT) {
			NetHandle_t * acceptSock;										  // Accepting socket
			SOCKET	  newSocket;										  // Newly connected (accepted) socket
			NetHandle_t * newUserSock;										  // -> New socket's structure
			SOCKADDR  newAddr;											  // New socket address
			int 	  newAddrLen=sizeof(newAddr);						  // Length of new socket address
			
			// A new socket is incoming on a listening port, we should accept the new socket and call the user
			// callback so they can register the connection (note that this has to be done with extreme care as
			// the system is multithreading).
			
			// Accept socket and set new socket's parameters
			if ((newSocket=accept((SOCKET)wParam, &newAddr, &newAddrLen))==INVALID_SOCKET) breakmsg;
			if ((acceptSock=NetFindHandleFromSocket((SOCKET)wParam))==NULL) breakmsg; // -> Listening socket which is hosting accept()
			WSAAsyncSelect(newSocket, Win95NetWindowPtr, WSM_STREAM, FD_READ | FD_WRITE | FD_CLOSE);
			
			// Create a new socket structure for this socket and add to the linked list of sockets
			//
			// We set the AcceptProc of this socket to that of the listening socket. We set the global variable
			// "netPendingAccepts" to indicate that there are pending connections. KeyboardKbhit() calls
			// netAcceptPendingConnections() which checks this flag and (if applicable) scans for USERSOCKS
			// of type SOCKMODE_ACCEPTING. After Socket->AcceptProc() is called, the mode of this socket is
			// set to SOCKMODE_CONNECTED.
			newUserSock=AllocAPointerClear(sizeof(NetHandle_t)); 				// Get memory for new user socket
			newUserSock->dwSocket=newSocket;						// Set socket number
			newUserSock->Mode=SOCKETMODE_ACCEPTING;					// This socket needs accepting
			newUserSock->maxInBuffers=acceptSock->maxInBuffers; 	// Set maximum amount of input queue
			newUserSock->AcceptProc=acceptSock->AcceptProc; 		// Clone accept proc
			Win95WinSockToNetAddress(&newUserSock->RemoteAddr,&newAddr);		// Convert address to our format
			NetAddHandleToList(newUserSock);							// Insert socket in our linked list
			netPendingAccepts=TRUE; 								// We're waiting for pending accepts
		//
		// CONNECT
		//
		} else if (wsaEvent==FD_CONNECT) {
			// Set flag so that netConnect() can continue, further processing of this socket is done in netConnect()
			netAMsgResult=(lParam>>16)&0xFFFF;							// Set message result
			netAsyncMsg=WSM_CONNECTED;								// Set processed message
		}
	}
	//
	// #******************#
	// *                  *
	// *  PACKET SOCKETS  *
	// *                  *
	// #******************#
	//
	else if (msg==WSM_PACKET) 										// Packet socket event?
	{
		// Receive notification on a packet (IPX/UDP) socket
				
		//
		// READ
		//
		if (wsaEvent==FD_READ)										  // Ready for reading?
		{
			PPKTINBUF newInBuf;											// -> New input buffer
			NetHandle_t * readSock;											// Socket we're reading from
			SOCKADDR	readAddr;											// Address we're reading from
			int		readAddrLen=sizeof(readAddr);						// Size of address structure
			
			// Find socket in linked list
			if ((readSock=NetFindHandleFromSocket((SOCKET)wParam))==NULL) breakmsg;	// Find socket and quit if not found.
			
			// Create a new buffer at the head of the list and receive the packet
			newInBuf=AllocAPointerClear(sizeof(PKTINBUF)+NET_DGRAMSIZE);			// Get new network buffer
			newInBuf->p.Length=recvfrom((SOCKET)wParam, (char *)newInBuf->p.Data, NET_DGRAMSIZE, 0, &readAddr, &readAddrLen);
			// Check if packet was received allright, if not, remove buffer and return
			if ((newInBuf->p.Length==0) || (newInBuf->p.Length==SOCKET_ERROR))
			{
				DeallocAPointer(newInBuf);									// Remove network buffer
				breakmsg;
			}
			
			// Update linked list
			if (readSock->inHead==NULL)								// First element?
			{
				// If the linked list doesn't have a head (so, it is empty), we're both head and tail
				readSock->inHead=(GenBuf_t *)newInBuf;					// We're the head of the buffer list...
				readSock->inTail=(GenBuf_t *)newInBuf;					// ...as well as the tail
			}
			else
			{
				// We pad ourselves to the end of the list. The current tail element's pNext will point to us,
				// the tail pointer will be set to point to us
				readSock->inTail->NextPtr=newInBuf; 					// Last element points to us
				readSock->inTail=(GenBuf_t *)newInBuf;					// Now we're the last element
			}
			
			// If there are a lot of buffers in the input buffer queue, signal a warning
			readSock->InputPackets++;								// More packets
			readSock->InputBytes+=newInBuf->p.Length;				// More bytes
#if _DEBUG
			if (((readSock->InputPackets & 511)==0) && (readSock->InputPackets))
				printf("Packets are stacking up (%u now)", readSock->InputPackets);
#endif
			// Check for buffer limit (exceeding?)
			if (readSock->InputPackets > readSock->maxInBuffers)
			{
				// Kill last packet in list
				GenBuf_t *head=readSock->inHead;						// Head of input buffer
				readSock->InputPackets--;							// Less data in output queue
				readSock->InputBytes-=((PPKTINBUF)head)->p.Length;	// Less bytes
				readSock->inHead=head->NextPtr; 						// "Shrink" start of linked list
				DeallocAPointer(head);										// Remove head of list
			}
			
			newInBuf->p.TimeStamp=ReadTick();						// Timestamp network buffer was receive
			Win95WinSockToNetAddress(&newInBuf->p.From,&readAddr);				// Convert socket address to network address
		}
	}
	
	// Common exit code (release mutex and return 0)
ExitWinSockMessage:
	ReleaseMutex(Win95NetSocketSemaphore); 									// Done!
#endif
	return(0);
}

#undef breakmsg

//
// WinSock thread
//
// The parameter is a HANDLE of an event. When we set this event, the main program will
// know we have initialised correctly.
//

static Word32 ANSICALL NetThreadProc(Word32 Param)
{
	MSG TempMsg;		/* Message to process */
	BOOL Var;			/* Result code */
			
	/* Create the window to handle the network messages */
	
	Win95NetWindowPtr=CreateWindow("DDWinSock","WinSock Window",0,0,0,0,0,0,0,(HINSTANCE)Win95Instance,0);	
	
	/* Tell NetInit() that I am ready */

	SetEvent((HANDLE)Param);
	if (Win95NetWindowPtr) {			/* Am I ok? */
		do {
			Var = GetMessage(&TempMsg,Win95NetWindowPtr,0,0);	/* Get a message */
			if (Var==1) {		/* Valid message? */
				DispatchMessage(&TempMsg);		/* Send to the window proc */
			}
		} while (Var);		/* Message or error? */
	}
	return 0;		/* Exit now */
}


//
// Initialise WinSock
//

static Word ClassInited;

Word BURGERCALL NetInit(void)
{
	WSADATA WinSockData;		/* Data about WinSock */
	HANDLE TempEvent;			/* Handle to get the thread init event */
	
	if (NetInited) {			/* Already started? */
		return FALSE;
	}	

	if (!ClassInited) {			/* Do I already have a thread going? */
		WNDCLASS cls;				/* Window class for event callbacks */
		FastMemSet(&cls,0, sizeof(cls));
		cls.lpfnWndProc = (WNDPROC)netWindowProc;
		cls.hInstance = (HINSTANCE)Win95Instance;
		cls.lpszClassName = "DDWinSock";
		if (!RegisterClass(&cls)) {
			return TRUE;
		}
		ClassInited = TRUE;
	}

	/* First I will init WinSock */
	
	if (WSAStartup(0x0101, &WinSockData)) {
		return TRUE;
	}
	
	/* I requested version 1.1, even if it is a higher version, it will return 1.1 */

	if (WinSockData.wVersion==0x101) {

		/* Let's create a semaphore for multi-threading */
	
		Win95NetSocketSemaphore=CreateMutex(0,0,0);
		if (Win95NetSocketSemaphore) {

			/* Now I create a bogus window to pass all the async calls to */
			/* this way, If I am on a parallel processing machine, I can use */
			/* a different processor to handle this code and offload from the */
			/* primary CPU. Neat eh? */
		
			TempEvent=CreateEvent(0,0,0,0);		/* Create a message to sync with */
			if (TempEvent) {					/* Did I make it? */
				DWORD TempThreadID;

				/* Now create the thread that will handle all network events */
				/* This is an async thread on another CPU */
				/* The thread startup routine will create the window */

				Win95NetThread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)NetThreadProc,TempEvent,0,&TempThreadID);// Create WinSock thread
				if (Win95NetThread) {
					SetThreadPriority(Win95NetThread,THREAD_PRIORITY_TIME_CRITICAL);	// Thread must be low-latency
					WaitForSingleObject(TempEvent,INFINITE);				// Wait for thread to initialise
				}
				CloseHandle(TempEvent);				// Close synchronisation event

				if (Win95NetThread) {
//					if (Win95NetWindowPtr) {

						/* Open a packet port for all providers */
						/* There are used by NetSendPacket() */
						KeyboardAddRoutine(NetKbhitProc,0);
						NetProviderPresent[NET_PROVIDER_TCP] = TRUE;
						NetProviderPresent[NET_PROVIDER_IPX] = TRUE;
						NetInited=TRUE; 								// Network support is up!
						return FALSE;
//					}
//					CloseHandle(Win95NetThread);
//					Win95NetThread = 0;
				}
			}
			CloseHandle(Win95NetSocketSemaphore);
			Win95NetSocketSemaphore = 0;
		}
	}
	WSACleanup();		/* Clean up my mess */
	return TRUE;		/* Error! */
}

/**********************************

	Shut down the network code

**********************************/

void NetShutdown(void)
{
	NetHandle_t *WorkHandle;	/* Temp */

	KeyboardRemoveRoutine(NetKbhitProc,0);		/* Remove the foreground routine */

	/* This code is to dispose of all the open sockets so we don't */
	/* crash because a thread was left laying around */
		
	WorkHandle = NetRootSocket.NextPtr;
	if (WorkHandle) {
		do {
			NetHandle_t *Next;
			Next = WorkHandle->NextPtr;
			NetHandleDelete(WorkHandle);
			WorkHandle = Next;
		} while (WorkHandle);
		NetRootSocket.NextPtr = 0;		/* Make sure the list is dead */
	}
		
	/* Now we shut down system specific stuff */
		
	WSACleanup();		/* Kill Winsock */
			
	if (Win95NetSocketSemaphore) {			/* Was a semaphore allocated? */
		CloseHandle(Win95NetSocketSemaphore);	/* Kill it */
		Win95NetSocketSemaphore=0;			/* Gone */
	}
	if (Win95NetThread) {		/* Was there a window thread allocated? */
		CloseHandle(Win95NetThread);	/* Bye bye */
		Win95NetThread = 0;		/* Gone */
	}
	NetInited=FALSE;		/* Network support is dead */
}

/**********************************

	Open up a NetHandle_t structure for being able
	to send (And receive) packets from a specific address.
	If you send a protocol ID instead, use the default address

**********************************/

NetHandle_t * BURGERCALL NetHandleNewListenPacket(NetAddr_t *Server)
{
	NetHandle_t *NewRef;
	SOCKADDR servAddr;
	int TempInt;
	NetAddr_t TempAddr;

	if ((Word)Server<NET_PROVIDER_COUNT) {		/* Default address? */
		FastMemSet(&TempAddr,0,sizeof(TempAddr));
		TempAddr.Provider = (Word)Server;
		Server = &TempAddr;
	}

	Win95NetToWinSockAddress(&servAddr,Server);
	NewRef=static_cast<NetHandle_t *>(AllocAPointerClear(sizeof(NetHandle_t)));	
	NewRef->SocketRef=socket(Win95NetAddrFamily[Server->Provider],SOCK_DGRAM,Win95NetTypes[Server->Provider]);
	TempInt = TRUE;
	setsockopt(NewRef->SocketRef, SOL_SOCKET, SO_BROADCAST, (char *)&TempInt, sizeof(TempInt));
	if (NewRef->SocketRef!=INVALID_SOCKET) {
		if (bind(NewRef->SocketRef, &servAddr, sizeof(servAddr))!=SOCKET_ERROR) {
			DWORD TempLong;
			TempLong = TRUE;
			if (!ioctlsocket(NewRef->SocketRef,FIONBIO,&TempLong)) {	/* Ok to not block! */
				NewRef->Mode=SOCKETMODE_LISTENPACKET;		/* The packet is listening! */
				FastMemCpy(&NewRef->LocalAddr,Server,sizeof(NetAddr_t));
				if (Server==&TempAddr) {		/* Using the default address? */
					TempInt = sizeof(servAddr);
					getsockname(NewRef->SocketRef,&servAddr,&TempInt);	/* Get the address */
					Win95WinSockToNetAddress(&NewRef->LocalAddr,&servAddr);	/* Save it */
				}

				/* This crappy piece of code is to get my local IP address instead */
				/* of a 0.0.0.0 */
				
				if (Server->Provider == NET_PROVIDER_TCP && !Server->U.TCP.IP) {
					HOSTENT *WorkPtr;
					char Buffer[256];
					if (!gethostname(Buffer,sizeof(Buffer))) {		/* Get my name */
						WorkPtr = gethostbyname(Buffer);			/* Convert to address */
						if (WorkPtr) {
							NewRef->LocalAddr.U.TCP.IP = Burger::LoadBig(((Word32 *)WorkPtr->h_addr_list[0])[0]);
						}
					}
				}
				NetAddHandleToList(NewRef);		/* Add to the active list! */
				return NewRef;
			}
		}
		closesocket(NewRef->SocketRef);		/* Discard the socket */
		DeallocAPointer(NewRef);			/* Kill my pointer */
	}
	return 0;		/* Darn! */
}

/**********************************

	Open up a NetHandle_t structure for being able
	to send (And receive) packets from a specific address.
	If you send a protocol ID instead, use the default address

**********************************/

NetHandle_t * BURGERCALL NetHandleNewListenStream(NetAddr_t * /* Input */,NetListenProc /* Proc */)
{
	return 0;		/* Always bomb */
}

/**********************************

	Open up a NetHandle_t structure for being able
	to send (And receive) packets from a specific address.
	If you send a protocol ID instead, use the default address

**********************************/

NetHandle_t * BURGERCALL NetHandleNewConnect(NetAddr_t *Server,Word /* TimeOut */)
{
	NetHandle_t *NewRef;
	SOCKADDR servAddr;
	SOCKADDR clAddr;
	int TempInt;
//	NetAddr_t TempAddr;

	FastMemSet(&clAddr,0,sizeof(clAddr));
	clAddr.sa_family = static_cast<Word16>(Win95NetAddrFamily[Server->Provider]);
	Win95NetToWinSockAddress(&servAddr,Server);
	NewRef=static_cast<NetHandle_t*>(AllocAPointerClear(sizeof(NetHandle_t)));	
	NewRef->SocketRef=socket(Win95NetAddrFamily[Server->Provider],SOCK_STREAM,Win95NetTypes[Server->Provider]);
	TempInt = TRUE;
//	setsockopt(NewRef->SocketRef, SOL_SOCKET, SO_BROADCAST, (char *)&TempInt, sizeof(TempInt));
	if (NewRef->SocketRef!=INVALID_SOCKET) {
		if (bind(NewRef->SocketRef, &clAddr, sizeof(clAddr))!=SOCKET_ERROR) {
			DWORD TempLong;
			TempLong = TRUE;
			if (!ioctlsocket(NewRef->SocketRef,FIONBIO,&TempLong)) {	/* Ok to not block! */
				NewRef->Mode=SOCKETMODE_LISTEN;		/* The packet is listening! */
				FastMemCpy(&NewRef->LocalAddr,Server,sizeof(NetAddr_t));
				TempInt = sizeof(servAddr);
				getsockname(NewRef->SocketRef,&servAddr,&TempInt);	/* Get the address */
				Win95WinSockToNetAddress(&NewRef->LocalAddr,&servAddr);	/* Save it */

				NetAddHandleToList(NewRef);		/* Add to the active list! */
				return NewRef;
			}
		}
		closesocket(NewRef->SocketRef);		/* Discard the socket */
		DeallocAPointer(NewRef);			/* Kill my pointer */
	}
	return 0;		/* Darn! */
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

		WaitForSingleObject(Win95NetSocketSemaphore,INFINITE);	/* Invoke the semaphore */
		closesocket(Input->SocketRef);		/* Stop the socket from executing */
		
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
		ReleaseMutex(Win95NetSocketSemaphore);	/* Multi-tasking can occur now */
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
	if (Input->Mode != SOCKETMODE_CONNECTED) {
		return -1;
	}
	if (Length) {
		do {
			long Sent;
			Sent = recv(Input->SocketRef,static_cast<char *>(Buffer),Length,0);
			if (Sent==(long)Length) {
				break;
			}
			if ((Sent==SOCKET_ERROR) && (WSAGetLastError()==WSAEWOULDBLOCK)) {
			} else if (Sent==SOCKET_ERROR) {
				return TRUE;
			} else {
				Buffer=(void *)(((Word8 *)Buffer)-Sent);
				Length-=Sent;
			}
		} while (Length);
		return TRUE;	/* Darn */
	}
	return FALSE;	/* Success! */
}

/**********************************

	Send a packet of data to a specific address.
	If the handle is a NULL pointer, then
	send it using the default packet address.

**********************************/

long BURGERCALL NetHandleWrite(NetHandle_t *Input,const void *Buffer,long Length,Word /* BlockFlag */)
{	
	if (Length) {
		do {
			long Sent;
			Sent = send(Input->SocketRef,static_cast<const char *>(Buffer),Length,0);
			if (Sent==(long)Length) {
				break;
			}
			if ((Sent==SOCKET_ERROR) && (WSAGetLastError()==WSAEWOULDBLOCK)) {
			} else if (Sent==SOCKET_ERROR) {
				return TRUE;
			} else {
				Buffer=(void *)(((Word8 *)Buffer)-Sent);
				Length-=Sent;
			}
		} while (Length);
		return TRUE;	/* Darn */
	}
	return FALSE;	/* Success! */
}

/**********************************

	Send a packet of data to a specific address.
	If the handle is a NULL pointer, then
	send it using the default packet address.

**********************************/

Word BURGERCALL NetHandleSendPacket(NetHandle_t *Input,NetAddr_t *DestAddr,const void *Buffer,Word Length)
{
	SOCKADDR Dest;		/* Windows destination address */
	
	if (Length) {
		if (!Input) {		/* Use default? */
			Input = NetGetPacketSendHandle((NetProvider_e)DestAddr->Provider);	/* Get the default */
			if (!Input) {
				return TRUE;	/* Error! */
			}
		}	
		Win95NetToWinSockAddress(&Dest,DestAddr);	
		if (sendto(Input->SocketRef,static_cast<const char *>(Buffer),Length,0,&Dest,sizeof(Dest))!=(int)Length) {
			return TRUE;	/* Darn */
		}
	}
	return FALSE;	/* Success! */
}

/**********************************

	Receive a connectionless packet from
	a specific socket
	
**********************************/

NetPacket_t * BURGERCALL NetHandleGetPacket(NetHandle_t *Input)
{
	SOCKADDR ReadSocketAddr;		/* Address the packet game from */
	int DataSize;		/* Size of socket address */
	long Length;
	
	Win95NetToWinSockAddress(&ReadSocketAddr,&Input->LocalAddr);
	DataSize=sizeof(ReadSocketAddr);				/* Init the size */
	Length = recvfrom(Input->SocketRef, (char *)Input->PacketBuffer,NET_MAXPACKETSIZE,0,&ReadSocketAddr, &DataSize);
	if (Length && Length!=SOCKET_ERROR) {
		Win95WinSockToNetAddress(&Input->LastPacket.Origin,&ReadSocketAddr);
		Input->LastPacket.Length = Length;
		Input->LastPacket.Data = Input->PacketBuffer;
		return &Input->LastPacket;
	}
	return 0;		/* Nothing here! */
}

/**********************************

	Given a DNS entry, return the IP address.
	206.55.132.145
	206.55.132.180:80
	www.logicware.com
	logicware.com:80

**********************************/

Word BURGERCALL NetStringToTCPAddress(NetAddr_t *Output,const char *HostName)
{
	Word32 IPAdr;
	HOSTENT *MyHost;
	char *ColonPtr;		/* Pointer to the port colon */
	char *TempPtr;

	Output->Provider = NET_PROVIDER_TCP;		/* It's a TCP/IP address */
	Output->U.TCP.IP = 0;				/* Init the defaults */
	Output->U.TCP.Port = 0;

	if (NetInited && HostName && HostName[0]) {
		TempPtr = StrCopy(HostName);
		if (TempPtr) {
			ColonPtr = strchr(TempPtr,':');		/* Scan for the offending colon */
			if (ColonPtr) {
				ColonPtr[0] = 0;		/* Force a null string */
				Output->U.TCP.Port = AsciiToLongWord(ColonPtr+1);
			}
			IPAdr=inet_addr(TempPtr);		/* Try 206.55.132.154 */
			if (IPAdr!=INADDR_NONE) {		/* Good? */
				goto ItsGood;
			}
			MyHost = gethostbyname(TempPtr);	/* Try www.logicware.com */
			if (MyHost) {
				IPAdr = ((Word32 *)MyHost->h_addr_list[0])[0];
	ItsGood:
				DeallocAPointer(TempPtr);
				Output->Provider = NET_PROVIDER_TCP;
				Output->U.TCP.IP = Burger::LoadBig(IPAdr);
				return FALSE;
			}
			DeallocAPointer(TempPtr);
		}
	}
	return TRUE;
}

/**********************************

	This is a Win95 convinence routine to convert a
	Burgerlib network address into a WinSock address

**********************************/

void BURGERCALL Win95NetToWinSockAddress(SOCKADDR *Output,NetAddr_t *Input)
{
	switch (Input->Provider) {
	case NET_PROVIDER_IPX:
		Output->sa_family = AF_IPX;
		((SOCKADDR_IPX *)Output)->sa_socket = Burger::LoadBig((Word16)Input->U.IPX.Socket);
		FastMemCpy(((SOCKADDR_IPX *)Output)->sa_netnum,Input->U.IPX.Net,10);
		break;
	case NET_PROVIDER_TCP:
		Output->sa_family = AF_INET;
		((SOCKADDR_IN *)Output)->sin_addr.s_addr = Burger::LoadBig(Input->U.TCP.IP);
		((SOCKADDR_IN *)Output)->sin_port = Burger::LoadBig((Word16)Input->U.TCP.Port);
		break;
	case NET_PROVIDER_APPLETALK:
		Output->sa_family = AF_APPLETALK;
		((SOCKADDR_AT *)Output)->sat_net = Burger::LoadBig((Word16)Input->U.APPLETALK.Network);
		((SOCKADDR_AT *)Output)->sat_node = static_cast<Word8>(Input->U.APPLETALK.NodeID);
		((SOCKADDR_AT *)Output)->sat_socket = static_cast<Word8>(Input->U.APPLETALK.Socket);
		break;
	default:
		FastMemSet(Output,0,sizeof(SOCKADDR));
	}
}

/**********************************

	This is a Win95 convinence routine to convert a
	WinSock address into a Burgerlib network address

**********************************/

void BURGERCALL Win95WinSockToNetAddress(NetAddr_t *Output,SOCKADDR *Input)
{
	switch (Input->sa_family) {
	case AF_INET:		/* TCP/IP address */
		Output->Provider=NET_PROVIDER_TCP;
		Output->U.TCP.Port=Burger::LoadBig(((SOCKADDR_IN *)Input)->sin_port);
		Output->U.TCP.IP=Burger::LoadBig(((SOCKADDR_IN *)Input)->sin_addr.s_addr);
		break;
	case AF_IPX:		/* IPX/SPX address */
		Output->Provider=NET_PROVIDER_IPX;
		Output->U.IPX.Socket=Burger::LoadBig(((SOCKADDR_IPX *)Input)->sa_socket);
		FastMemCpy(Output->U.IPX.Net,((SOCKADDR_IPX *)Input)->sa_netnum,10);
		break;
	case AF_APPLETALK:	/* Appletalk address */
		Output->Provider=NET_PROVIDER_APPLETALK;
		Output->U.APPLETALK.Network = Burger::LoadBig(((SOCKADDR_AT *)Input)->sat_net);
		Output->U.APPLETALK.NodeID = ((SOCKADDR_AT *)Input)->sat_node;
		Output->U.APPLETALK.Socket = ((SOCKADDR_AT *)Input)->sat_socket;
		break;
	default:
		FastMemSet(Output,0,sizeof(NetAddr_t));
	}
}

#endif