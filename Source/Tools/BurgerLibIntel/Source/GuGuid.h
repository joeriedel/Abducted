/**********************************

	Create and work with Win95 style GUID structures

**********************************/

#ifndef __GUGUID_H__
#define __GUGUID_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifndef __LL64BIT_H__
#include "Ll64Bit.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GUID_DEFINED		/* Used to be compatible with Win95 */
#define GUID_DEFINED
typedef struct _GUID {
	Word32 Data1;			/* Same names as Win95 */
	Word16 Data2;
	Word16 Data3;
	Word8 Data4[8];
} GUID;
#endif

typedef struct EthernetAddress_t {
    Word8 eaddr[6];      /* 6 bytes of ethernet hardware address */
} EthernetAddress_t;

extern GUID GUIDBlank;		/* Empty GUID */
extern void BURGERCALL GUIDInit(GUID *Output);
extern void BURGERCALL GUIDToString(char *Output,const GUID *Input);
extern Word BURGERCALL GUIDFromString(GUID *Output,const char *Input);
extern Word BURGERCALL GUIDHash(const GUID *Input);
extern Word BURGERCALL GUIDIsEqual(const GUID *Input1,const GUID *Input2);
extern int BURGERCALL GUIDCompare(const GUID *Input1,const GUID *Input2);
extern Word BURGERCALL GUIDGetAddress(EthernetAddress_t *Output);
extern void BURGERCALL GUIDGetTime(LongWord64_t *Output);
extern Word BURGERCALL GUIDGetEthernetAddr(EthernetAddress_t *Output);
extern Word BURGERCALL GUIDGenRandomEthernet(EthernetAddress_t *Output);

#ifdef __cplusplus
}
#endif

#endif

