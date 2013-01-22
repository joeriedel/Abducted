/**********************************

	Mac specific code for GUID manager
	
**********************************/

#include "GuGuid.h"

#if defined(__MAC__)
#include "ClStdLib.h"
#include <ENET.h>					/* Macintosh routines for getting enet addr */
#include <Slots.h>
#include <Gestalt.h>
#include <Devices.h>
#include <Folders.h>
#include <Timer.h>

#if defined(__POWERPC__)			/* PowerPC Macintosh specific */
#include <NameRegistry.h>
#endif

/*
 *  Define constant designation difference in Unix and DTSS base times:
 *  DTSS UTC base time is October 15, 1582.
 *  Unix base time is January 1, 1970.
 */

#define uuid_c_os_base_time_diff_lo 0x13814000
#define uuid_c_os_base_time_diff_hi 0x01B21DD2

#define UUID_C_100NS_PER_SEC 10000000

enum {
	kUnsupported 		= 0,
	kPDMMachine			= 1,
	kPCIMachine			= 2,
	kCommSlotMachine	= 3,
	kPCIComm2Machine 	= 4
};

typedef struct RandomMacStuff {
	UnsignedWide curprocessor;	/* How long ago turned on? */
	unsigned long curtime;		/* Current clock time */
	long vSysDirID;
	long vCreateDate;			/* Boot volume attributes */
	long vLastBackup;
	short vBootVol;
	short vAttrib;
	short vFileCount;
	short vDirStart;
	short vDirLength;
	short vAllocBlocks;
	long vAllocSize;
	long vClumpSize;
	long vNextFile;
	short vBlockMap;
	short vFreeBlocks;
	Point mousePos;				/* Current mouse pointer pos */
} RandomMacStuff;


#define kPDMEnetROMBase	0x50f08000

static Word got_first_time;			/* Read time at least once? */
static LongWord64_t TimeAdjust;		/* Time adjust for time input */
static EthernetAddress_t GUIDSavedENetAddr;		/* Currently saves ethernet address */

/**********************************

	Power mac specific code to get the ethernet address
	
**********************************/

#if defined(__POWERPC__)			/* PowerPC Macintosh specific */

/**********************************

	Determine the type of Power Mac I am,
	and returns a constand indicating the type
	of built in ethernet support there is.
	
**********************************/

static Word GUIDDoesMacHaveBuiltInEthernet(void)
{
	long response;
	Word result;
	
	Gestalt(gestaltMachineType, &response);		/* What type am I? */
	switch (response) {
	case gestaltPowerMac8100_120:
	case gestaltAWS9150_80:
	case gestaltPowerMac8100_110:
	case gestaltPowerMac7100_80:
	case gestaltPowerMac8100_100:
	case gestaltAWS9150_120:
	case gestaltPowerMac8100_80:
	case gestaltPowerMac6100_60:
	case gestaltPowerMac6100_66:
	case gestaltPowerMac7100_66:
		result = kPDMMachine;
		break;
		
	case gestaltPowerMac9500:
	case gestaltPowerMac7500:
	case gestaltPowerMac8500:
	case gestaltPowerBook3400:
	case gestaltPowerBookG3:
	case gestaltPowerMac7200:
	case gestaltPowerMac7300:
	case gestaltPowerBookG3Series:
	case gestaltPowerBookG3Series2:
	case gestaltPowerMacG3:
	case gestaltPowerMacNewWorld:
		result = kPCIMachine;
		break;
			
	case gestaltPowerMac5200:
	case gestaltPowerMac6200:
		result = kCommSlotMachine;
		break;
			
	case gestaltPowerMac6400:
	case gestaltPowerMac5400:
	case gestaltPowerMac5500:
	case gestaltPowerMac6500:
	case gestaltPowerMac4400_160:
	case gestaltPowerMac4400:
		result = kPCIComm2Machine;
		break;
	default:
		if (!Gestalt(gestaltNameRegistryVersion, (long*) &response)) {
			result = kPCIMachine;
		} else {
			result = kUnsupported;		/* I give up! */
		}
	}
	return result;
}

/**********************************

	This pulls the ENet MAC address from the ROMs on machines which
	store the MAC address there.
	(Returns an error on MacOSX since I can't access ROM memory)
	
**********************************/

static OSStatus GUIDGetPDMBuiltInEnetAddr(Word8 *Output)
{
	Word i;
	Word8 *val;
	long osver;
	OSErr err;
	
	err = Gestalt(gestaltSystemVersion,&osver);
	if (err) {
		return err;
	}
	if ((osver & 0x0000FFFF) >= 0x0850) {
		return -1;	/* Bogus error code */
	}
	
	/* Now do it. */

	i = 6;
	val = (Word8 *)kPDMEnetROMBase;
	do {
		Output[0] = ReverseBits[val[0]];		/* Reverse the bits */
		++Output;
		val+=16;
	} while (--i);
	return noErr;
}

/**********************************

	Later PCI PowerPC Macintosh systems have a "Name Registry" (No!
	they are making the Mac like Win95!!) which contains the ENet MAC
	address. This searches for the address
	
**********************************/

static char LocalName[] = "local-mac-address";

static OSStatus GUIDGetPCIBuiltInEnetAddr(Word8 *Output)
{
#if !TARGET_API_MAC_CARBON
	OSStatus err;
	RegEntryIter cookie;
	RegEntryID theFoundEntry;
	Word8 enetAddr[6];
	Bool done;
	RegPropertyValueSize theSize;

	err = RegistryEntryIDInit( &theFoundEntry );		/* Init the registry */
	if (!err) {
		err = RegistryEntryIterateCreate( &cookie );
		if (!err) {
			done = FALSE;
			err = RegistryEntrySearch( &cookie, kRegIterDescendants, &theFoundEntry, &done,
				LocalName, nil, 0);

			if (!err) {
				theSize = sizeof(enetAddr);
				err = RegistryPropertyGet(&theFoundEntry, LocalName, &enetAddr, &theSize );
				if (!err) {
					FastMemCpy(Output,enetAddr,sizeof(enetAddr));
				}
			}
			RegistryEntryIterateDispose( &cookie );
		}
	}
	return err;
#else
	return -1;
#endif
}

/**********************************

	This attempts to get the ENet MAC address from a different
	location in the name registry--some models use a different token
	than the one searched for above.
	
**********************************/

#if 0			/* Not used, maybe later */
static char LocalNamePCI[] = "ASNT,ethernet-address";

static OSStatus GUIDGetPCIComm2EnetAddr(Word8 *Output)
{
	OSStatus err;
	RegEntryIter cookie;
	RegEntryID theFoundEntry;
	Word8 *enetAddr;
	Bool done;
	RegPropertyValueSize theSize;

	err = RegistryEntryIDInit( &theFoundEntry );
	if (!err) {
		err = RegistryEntryIterateCreate( &cookie );
		if (!err) {
			done = FALSE;
			err = RegistryEntrySearch( &cookie, kRegIterDescendants, &theFoundEntry, &done,	
				LocalNamePCI, nil, 0);

			if (!err) {
				theSize = sizeof(enetAddr);
				err = RegistryPropertyGet(&theFoundEntry, LocalNamePCI, &enetAddr, &theSize );
				if (!err) {
					FastMemCpy(Output,enetAddr,6);
				}
			}
			RegistryEntryIterateDispose( &cookie );
		}
	}
	return err;
}
#endif

/**********************************

	This implements the routines necessary for pulling the Ethernet
	Address directly from the hardware of certain PowerPC Macintosh
	system. This doesn't work with MacOS X, so I first test to make
	sure I'm on qualifying hardware running an operating system that
	won't barf if I start poking around inside the Apple ROMs.
	
**********************************/

static Word GUIDGetHEthernetAddr(EthernetAddress_t *Output)
{
	OSStatus err;
	Word cputype;
	Word8 enetaddr[6];

	cputype = GUIDDoesMacHaveBuiltInEthernet();
	switch (cputype) {
	case kPDMMachine:
		err = GUIDGetPDMBuiltInEnetAddr(enetaddr);
		if (err) {
			return err;
		}
		break;
	case kPCIMachine:
		err = GUIDGetPCIBuiltInEnetAddr(enetaddr);
		if (err) {
			return err;
		}
		break;
//	case kCommSlotMachine:
//	case kPCIComm2Machine:
		/* This may or may not have an ethernet card in a slot. */
		/* Punt, and hope my driver code below handles this. */
//		return -1;			/* Bogus error code */
	default:
		/* This is not a supported machine. Punt. */
		/* If the driver code below gets an ENet MAC addr, great */
		return -1;
	}

	/*
	*	If we get here, our hardware technique worked. Copy the
	*	address to the uuid_address_t structure
	*/

	FastMemCpy(&Output->eaddr[0],&enetaddr[0],6);
	return noErr;
}

#endif

/**********************************

		This attempts to open the .ENET driver and get the card information
	directly from that driver. Punt if I fail.

		This uses the method described in IM:Networking, on the section on
	using the EtherNet driver. (It turns out the algorithms outlined in
	other places for opening the .ENET driver were not as reliable.)

		Note that I do not close the driver. The reason for this is that
	other services may be using the .ENET driver. In particular on earlier
	versions of the MacOS, both parts of Open Transport and AppleTalk sit
	on top of the .ENET driver. And closing the .ENET driver leads to some
	really bad results.
	
**********************************/

Word BURGERCALL GUIDGetEthernetAddr(EthernetAddress_t *Output)
{
#if !TARGET_API_MAC_CARBON
	char buffer[78];
	EParamBlock theEPB;

	SpBlock sp;
	ParamBlockRec pb;
	short refNum;
	OSErr err;
#endif
	
	/*
	 *	Before I try the driver, first try getting the hardware
	 *	address without the driver. This only works on certain
	 *	models of PowerPC hardware.
	 */
	
#if defined(__POWERPC__)
	if (!GUIDGetHEthernetAddr(Output)) {
		return noErr;			/* Got it! */
	}
#endif
	
#if !TARGET_API_MAC_CARBON
	/*
	 *	Couldn't get it from the hardware, so I punt and try
	 *	to get it from the driver instead.
	 */
	
	refNum = 0;

	FastMemSet(&sp,0,sizeof(sp));
	FastMemSet(&pb,0,sizeof(pb));
		
	sp.spParamData = 1;
	sp.spCategory = 4;		// network card
	sp.spCType = 1;			// ethernet
	sp.spTBMask = 3;
	sp.spSlot = 0;
	
	/* Let's first try the built in driver */
	
	while ((err = SNextTypeSRsrc(&sp)) == noErr) {
		pb.slotDevParam.ioNamePtr = "\p.ENET";
		pb.slotDevParam.ioSPermssn = fsCurPerm;
		pb.slotDevParam.ioSlot = sp.spSlot;
		pb.slotDevParam.ioID = sp.spID;
		err = OpenSlot(&pb,FALSE);
		if (!err) {
			break;
		}
	}
	
	if (err) {
		/*
		 *	We didn't find a thing--try the ENET0 driver instead.
		 *	This is an alternative driver which is supplied for
		 *	non-NuBUS and slot-managed ethernet devices.
		 */
		
		err = MacOpenDriver("\p.ENET0",&refNum);
		if (err) {
			return err;
		}
	} else {
		refNum = pb.slotDevParam.ioSRefNum;
	}
	FastMemSet(buffer,0,sizeof(buffer));
	theEPB.ioRefNum = refNum;
	theEPB.u.EParms1.ePointer = buffer;
	theEPB.u.EParms1.eBuffSize = 78;
	theEPB.ioNamePtr = NULL;
	err = EGetInfo(&theEPB,0);

	FastMemCpy(Output,buffer,sizeof(EthernetAddress_t));
	return err;
#else
	return -1;
#endif
}

/**********************************

		This generates a random 48-bit string using some random factors
	from the Macintosh operating system, using the MD5 messaging
	algorithm to generate the bits.
	
**********************************/

Word BURGERCALL GUIDGenRandomEthernet(EthernetAddress_t *Output)
{
	RandomMacStuff stuff;		/* Crap to fill in */
	ParamBlockRec param;
	MD5_t context;
	Word8 md5res[16];			/* MD5 hash value */
	Word i;
	
	i = 0;
	do {
		if (GUIDSavedENetAddr.eaddr[i]) {
			FastMemCpy(Output,&GUIDSavedENetAddr,6);
			return noErr;
		}
	} while (++i<6);
	
	GetDateTime(&stuff.curtime);			/* Current time */
	Microseconds(&stuff.curprocessor);		/* Current fast time */
	GetMouse(&stuff.mousePos);				/* Read the mouse */
	FindFolder(kOnSystemDisk,				/* Boot volume number */
			   kSystemFolderType,
			   kDontCreateFolder,
			   &stuff.vBootVol,
			   &stuff.vSysDirID);
	
	param.volumeParam.ioVolIndex = 0;		/* Hard drive info */
	param.volumeParam.ioNamePtr = NULL;
	param.volumeParam.ioVRefNum = stuff.vBootVol;

#if !TARGET_API_MAC_CARBON
	if (!PBGetVInfo(&param,false)) {
		stuff.vCreateDate  = param.volumeParam.ioVCrDate;
		stuff.vLastBackup  = param.volumeParam.ioVLsBkUp;
		stuff.vAttrib      = param.volumeParam.ioVAtrb;
		stuff.vFileCount   = param.volumeParam.ioVNmFls;
		stuff.vDirStart    = param.volumeParam.ioVDirSt;
		stuff.vDirLength   = param.volumeParam.ioVBlLn;
		stuff.vAllocBlocks = param.volumeParam.ioVNmAlBlks;
		stuff.vAllocSize   = param.volumeParam.ioVAlBlkSiz;
		stuff.vClumpSize   = param.volumeParam.ioVClpSiz;
		stuff.vBlockMap    = param.volumeParam.ioAlBlSt;
		stuff.vNextFile    = param.volumeParam.ioVNxtFNum;
		stuff.vFreeBlocks  = param.volumeParam.ioVFrBlk;
	}
#endif	
	/* Now make a hash value using MD5 */
		
	MD5Init(&context);
	MD5Update(&context,(Word8 *)&stuff,sizeof(stuff));
	MD5Final(md5res,&context);
	
	/*
	 *	And copy over the bits
	 */
	
	i = 0;
	do {
		Output->eaddr[i] = md5res[i+5];			/* Copy the address */
	} while (++i<6);
	Output->eaddr[0] |= 0x80;					/* Set high bit of this thing */	
	FastMemCpy(&GUIDSavedENetAddr,Output,6);	/* Save it */
	return noErr;
}

/**********************************

	Get the current time in 100ns increments
	
**********************************/

void BURGERCALL GUIDGetTime(LongWord64_t *Output)
{
	unsigned long read_time;
	LongWord64_t tmp;
	LongWord64_t micro;
    LongWord64_t usecs,utc,os_basetime_diff;
    MachineLocation os_mach;
    long delta_gmt;

	/*
	 *	Calculate the time in 100ns units since system boot
	 */
	
	if (!got_first_time) {
		got_first_time = TRUE;
		GetDateTime(&read_time);			// time in seconds
		
		ReadLocation(&os_mach);				// offset time to GMT
		delta_gmt = os_mach.u.gmtDelta;
		delta_gmt &= 0x00FFFFFF;
		if (delta_gmt & 0x00800000) {
			delta_gmt |= 0xFF000000;
		}
		read_time -= delta_gmt;
		
		Microseconds((UnsignedWide *)&micro);				// time in 100ns units
		LongWord64FromLong(&tmp,10);
		LongWord64Mul(&micro,&tmp);
		
		read_time -= 2082844800;			/* seconds from 1904 to 1970 */
		LongWord64MulLongTo64(&TimeAdjust,read_time, UUID_C_100NS_PER_SEC);
		LongWord64Sub(&TimeAdjust,&micro);		/* Final time adjust value */
	}
	
	/* Now get the current time in microseconds and add to boot time */
	
	Microseconds((UnsignedWide *)&usecs);
	LongWord64FromLong(&tmp,10);
	LongWord64Mul(&usecs,&tmp);
	LongWord64Add3(&utc,&usecs,&TimeAdjust);
	
	/* Offset between DTSS formatted times and Unix formatted times. */
	
	LongWord64FromLong2(&os_basetime_diff,uuid_c_os_base_time_diff_lo,uuid_c_os_base_time_diff_hi);
	LongWord64Add3(Output,&utc,&os_basetime_diff);
}

#endif