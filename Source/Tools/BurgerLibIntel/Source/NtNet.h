/**********************************

	Burgerlib network support

**********************************/

#ifndef __NTNET_H__
#define __NTNET_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#if defined(__MAC__)
#include <OpenTransport.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct NetHandle_t;

/* Protocols supported (Not all are available) */

#define NET_MAXPACKETSIZE 3072
typedef enum {NET_PROVIDER_IPX,NET_PROVIDER_TCP,NET_PROVIDER_APPLETALK,NET_PROVIDER_COUNT} NetProvider_e;
typedef enum {SOCKETMODE_UNUSED,SOCKETMODE_LISTEN,SOCKETMODE_LISTENPACKET,SOCKETMODE_CONNECTED,
	SOCKETMODE_ACCEPTING,SOCKETMODE_COUNT} SocketMode_e;
typedef Word (BURGERCALL * NetListenProc)(struct NetHandle_t *);

typedef struct NetAddr_t {	/* Used for a network address */
	Word Provider;			/* Network provider (TCP/IPX/APPLETALK) */
	union {
		struct {			/* TCP/UDP data */
			Word Port;		/* TCP/IP, UDP Port */
			Word32 IP;	/* Internet IP (Network order) */
		} TCP;
		struct {			/* IPX/SPX data */
			Word8 Net[4];	/* IPX/SPX Network */
			Word8 Node[6];	/* IPX/SPX Node address */
			Word Socket;	/* IPX/SPX Socket */
		} IPX;
		struct {			/* Appletalk data */
			Word Network;	/* Appletalk network */
			Word NodeID;	/* Appletalk node */
			Word Socket;	/* Appletalk socket */
			Word DDPType;	/* Appletalk protocol ID */
		} APPLETALK;
	} U;
} NetAddr_t;

typedef struct NetPacket_t {
	NetAddr_t Origin;		/* Who sent this packet? */
	Word32 Length;		/* Length of data */
	Word8 *Data;				/* Data coming in */
} NetPacket_t;

//typedef struct NetHandle_t NetHandle_t;	/* Public definition */

typedef struct NetHandle_t {
	struct NetHandle_t *NextPtr;	/* Next NetHandle_t in the active list */
#if defined(__MAC__)
	class TEndpoint* SocketRef;			/* Socket referece for native OS (Ptr, long) */
#else
	Word32 SocketRef;			/* Socket referece for native OS (Ptr, long) */
#endif
	NetAddr_t LocalAddr;		/* Address of the socket locally */
	NetAddr_t RemoteAddr;		/* Address of the peer socket (Connect only) */
	NetListenProc ProcPtr;		/* Callback for new connections */
	SocketMode_e Mode;			/* Mode of the socket */
	NetPacket_t LastPacket;		/* Last packet that arrived */
#if defined(__MAC__)
	TCall CallInfo;				/* Buffer for connection */
	Word8 AckFlag;				/* Flag for acking an event */
	Word8 DataFlag;				/* TRUE if data is present */
	Word8 AcceptFlag;			/* Is there a pending listen command? */
	Word8 PollProc;				/* Polling proc attached? */
#endif
	Word8 PacketBuffer[NET_MAXPACKETSIZE];	/* Packet buffer */
} NetHandle_t;

extern Word NetInited;						/* True if the network is present */
extern Bool NetProviderPresent[NET_PROVIDER_COUNT];		/* True if the provider code is present */
extern NetHandle_t *DefaultSockets[NET_PROVIDER_COUNT];	/* These are the default sockets */
extern NetHandle_t NetRootSocket;			/* Root handle for linked list */

/* Public prototypes */

extern Word BURGERCALL NetInit(void);
extern void BURGERCALL NetShutdown(void);
extern NetHandle_t * BURGERCALL NetHandleNewListenPacket(NetAddr_t *Input);
extern NetHandle_t * BURGERCALL NetHandleNewListenStream(NetAddr_t *Input,NetListenProc Proc);
extern NetHandle_t * BURGERCALL NetHandleNewConnect(NetAddr_t *Input,Word Timeout);
extern void BURGERCALL NetHandleDelete(NetHandle_t *Input);
extern Word BURGERCALL NetHandleIsItClosed(NetHandle_t *Input);
extern long BURGERCALL NetHandleRead(NetHandle_t *Input,void *Buffer,long BufSize);
extern long BURGERCALL NetHandleWrite(NetHandle_t *Input,const void *Buffer,long Length,Word BlockFlag);
extern Word BURGERCALL NetHandleSendPacket(NetHandle_t *Input,NetAddr_t *DestAddr,const void *Buffer,Word Length);
extern NetPacket_t * BURGERCALL NetHandleGetPacket(NetHandle_t *Input);
extern Word BURGERCALL NetStringToTCPAddress(NetAddr_t *Output,const char *TCPName);
extern void BURGERCALL NetAddressToString(char *Output,Word Size,NetAddr_t *Input,Word PortFlag);
extern Word BURGERCALL NetAddressCompare(const NetAddr_t *First,const NetAddr_t *Second);
extern Word BURGERCALL NetIsProviderPresent(NetProvider_e Provider);
extern NetHandle_t * BURGERCALL NetGetPacketSendHandle(NetProvider_e Provider);
extern void BURGERCALL NetGetPeerAddress(NetAddr_t *Output,NetHandle_t *Input);
extern void BURGERCALL NetGetLocalAddress(NetAddr_t *Output,NetHandle_t *Input);
extern NetHandle_t * BURGERCALL NetFindHandleFromSocket(Word32 Socket);
extern NetHandle_t * BURGERCALL NetFindHandleByMode(SocketMode_e Mode);
extern void BURGERCALL NetAddHandleToList(NetHandle_t *Input);
#if defined(__WIN32__)
extern void BURGERCALL Win95WinSockToNetAddress(NetAddr_t *Output,struct sockaddr *Input);
extern void BURGERCALL Win95NetToWinSockAddress(struct sockaddr *Output,NetAddr_t *Input);
#elif defined(__MAC__)
extern void BURGERCALL MacOTToNetAddress(NetAddr_t *Output,struct OTAddress *Input);
extern void BURGERCALL MacNetToOTAddress(struct OTAddress *Output,NetAddr_t *Input);
#endif

#ifdef __cplusplus
}
#endif

#endif
