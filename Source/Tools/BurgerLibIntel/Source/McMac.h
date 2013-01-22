#ifndef __MCMAC_H__
#define __MCMAC_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#if defined(__MAC__) || defined(__MACOSX__)

#if !defined(__MACOSX__)
#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif

/* This truly bites!! */

#if !TARGET_API_MAC_CARBON
#define GetPortPixMap(x) (x)->portPixMap
#define GetPortBounds(MyPort,WorkRect) (WorkRect)[0] = MyPort->portRect
#define GetPixBounds(MyPort,WorkRect) (WorkRect)[0] = (MyPort[0])->bounds
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Spotlight API for Mac Debugging */

#if defined(__MAC__) && defined(__POWERPC__)
#define slValidateReadWrite		'vr_w'
#define slInitMemory			'imem'
#define slInitStack				'istk'
#define slRptRsrcFailures		'rsrc'
#define slRptMemoryLeaks		'leak'
#define slTBValidation			'tchk'
#define slDebuggerOptions		'dbug'
#define slLogging				'logg'
extern void SLInit(void);
extern void SLEnable(void);
extern void SLDisable(void);
extern void SLEnterInterrupt(void);
extern void SLLeaveInterrupt(void);
extern void SLResetLeaks(void);
extern pascal signed short SLInitialize(struct CFragSystem7InitBlock *theInitBlock);
extern pascal void SLStart(void);
#endif

typedef Word (BURGERCALL *MacEventInterceptProcPtr)(struct EventRecord *MyEventPtr);

extern struct GDevice **VideoDevice;	/* Video device for graphic output */
#if !TARGET_API_MAC_CARBON && !defined(__MACOSX__)
extern struct CGrafPort *VideoGWorld;	/* Grafport to offscreen buffer */
extern struct GrafPort *VideoWindow;	/* Window to display to */
#define MacDialogRef struct GrafPort *
#else
extern struct OpaqueGrafPtr *VideoGWorld;	/* Grafport to offscreen buffer */
extern struct OpaqueWindowPtr *VideoWindow;	/* Window to display to */
#define MacDialogRef struct OpaqueDialogPtr*
#endif
#if TARGET_RT_MAC_CFM || defined(__MACOSX__)
extern struct OpaqueDSpContextReference *MacContext;	/* Reference to a reserved DrawSprocket reference */
extern Bool MacDrawSprocketActive;	/* Has draw sprocket been started up? */
#endif
extern Bool MacUseBackBuffer;		/* Which buffer is active */
extern Bool MacSystemTaskFlag;		/* If TRUE, then SystemTask() is called */
extern MacEventInterceptProcPtr MacEventIntercept;	/* Intercept for DoMacEvent() */
extern short MacVRefNum;		/* Volume reference used by Mac OS */
extern long MacDirID;			/* Directory reference used by MacOS */
extern short MacVRefNum2;		/* Volume reference used by copy and rename */
extern long MacDirID2;			/* Directory reference used by copy and rename */
extern short MacCacheVRefNum;	/* Cached VRef num for currently logged directory */
extern long MacCacheDirID;		/* Cached Dir ID for currently logged directory */
extern Word BURGERCALL MacMakeOffscreenGWorld(Word Width,Word Height,Word Depth,Word Flags);
extern void * BURGERCALL GetFullPathFromMacID(long dirID,short vRefNum);
extern short BURGERCALL OpenAMacResourceFile(const char *PathName,char Permission);
extern Word BURGERCALL CreateAMacResourceFile(const char *PathName);
extern char *BURGERCALL GetFullPathFromMacFSSpec(const struct FSSpec *Input);
extern void BURGERCALL MacOSFileSecondsToTimeDate(struct TimeDate_t *Output,Word32 Time);
extern void BURGERCALL MacOSPurgeDirCache(void);
extern Word BURGERCALL DoMacEvent(Word Mask,struct EventRecord *MyRecord);
extern Word BURGERCALL FixMacKey(struct EventRecord *MyRecord);
extern Word BURGERCALL MacOSIsTrapAvailable(int TrapNum);
extern Word BURGERCALL MacOSIsQuickTimePowerPlugAvailable(void);
extern Word BURGERCALL MacOSGetSoundManagerVersion(void);
extern Word BURGERCALL MacOSGetInputSprocketVersion(void);
extern Word BURGERCALL MacOSGetDrawSprocketVersion(void);
extern Word BURGERCALL MacOSGetAppleShareVersion(void);
extern void BURGERCALL MacOSSetExtensionsPrefix(Word PrefixNum);
extern long BURGERCALL MacOpenFileForRead(const char *Filename);
extern Word BURGERCALL MacOpenControlPanel(Word32 type,const char *defaultname,Word Continue);
extern Word BURGERCALL MacLaunch(short vref,long dirID, const char *name,Word Continue);
extern void BURGERCALL MacOSKillProcess(struct ProcessSerialNumber *victim);
extern void BURGERCALL MacOSKillAllProcesses(void);
extern struct LibRef_t * BURGERCALL MacOSGetInterfaceLib(void);
extern struct LibRef_t * BURGERCALL MacOSDriverLoaderLib(void);
extern Word BURGERCALL MacOSDialogControlGetValue(MacDialogRef dialog,Word item);
extern void BURGERCALL MacOSDialogControlSetValue(MacDialogRef dialog,Word item,Word Value);
extern void BURGERCALL MacOSDialogControlToggleValue(MacDialogRef dialog,Word item);
#if TARGET_API_MAC_CARBON && defined(__MAC__)
extern void * BURGERCALL MacOSMachOToCFM(void *ProcPtr);
extern void BURGERCALL MacOSMachOToCFMDelete(void *MachPtr);
#endif

extern void BURGERCALL MacOSInitTools(void);
extern void BURGERCALL MacOSEjectCD(void);
extern Word BURGERCALL MacOSIsAltivecPresent(void);
extern Word BURGERCALL MacOSGetOSVersion(void);

#if TARGET_API_MAC_CARBON || defined(__MACOSX__)

typedef struct MacOSXFramework_t {	/* Data for a MacOSX library reference */
	struct __CFBundle *LibBundle;	/* Master bundle reference */
} MacOSXFramework_t;

extern MacOSXFramework_t *BURGERCALL MacOSGetFoundationFramework(void);
extern Word BURGERCALL MacOSXFrameworkInit(MacOSXFramework_t *Input,const char *FrameWorkName);
extern void * BURGERCALL MacOSXFrameworkGetProc(MacOSXFramework_t *Input,const char *ProcNam);
extern void BURGERCALL MacOSXFrameworkDestroy(MacOSXFramework_t *Input);
extern void BURGERCALL MacOSXTimeDateFromUTCTime(struct TimeDate_t *Output,const struct UTCDateTime *Input);
#endif

#ifdef __cplusplus
}
#endif

#endif
#endif
